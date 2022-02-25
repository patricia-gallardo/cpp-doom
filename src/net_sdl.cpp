//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Networking module which uses SDL_net
//

#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_misc.hpp"
#include "memory.hpp"
#include "net_defs.hpp"
#include "net_io.hpp"
#include "net_packet.hpp"
#include "net_sdl.hpp"
#include "z_zone.hpp"

//
// NETWORKING
//

#include <SDL_net.h>

#define DEFAULT_PORT 2342

static bool        initted = false;
static int         port    = DEFAULT_PORT;
static UDPsocket   udpsocket;
static UDPpacket * recvpacket;

struct addrpair_t {
  net_addr_t net_addr;
  IPaddress  sdl_addr;
};

static addrpair_t ** addr_table;
static int           addr_table_size = -1;

// Initializes the address table

static void NET_SDL_InitAddrTable() {
  addr_table_size = 16;

  addr_table = zmalloc<decltype(addr_table)>(sizeof(addrpair_t *) * static_cast<unsigned long>(addr_table_size),
                                             PU_STATIC,
                                             0);
  std::memset(addr_table, 0, sizeof(addrpair_t *) * static_cast<unsigned long>(addr_table_size));
}

static bool AddressesEqual(IPaddress * a, IPaddress * b) {
  return a->host == b->host
         && a->port == b->port;
}

// Finds an address by searching the table.  If the address is not found,
// it is added to the table.

static net_addr_t * NET_SDL_FindAddress(IPaddress * addr) {
  int empty_entry = -1;

  if (addr_table_size < 0) {
    NET_SDL_InitAddrTable();
  }

  for (int i = 0; i < addr_table_size; ++i) {
    if (addr_table[i] != nullptr
        && AddressesEqual(addr, &addr_table[i]->sdl_addr)) {
      return &addr_table[i]->net_addr;
    }

    if (empty_entry < 0 && addr_table[i] == nullptr)
      empty_entry = i;
  }

  // Was not found in list.  We need to add it.

  // Is there any space in the table? If not, increase the table size

  if (empty_entry < 0) {
    // after reallocing, we will add this in as the first entry
    // in the new block of memory

    empty_entry = addr_table_size;

    // allocate a new array twice the size, init to 0 and copy
    // the existing table in.  replace the old table.

    int           new_addr_table_size = addr_table_size * 2;
    addrpair_t ** new_addr_table      = zmalloc<decltype(new_addr_table)>(sizeof(addrpair_t *) * static_cast<unsigned long>(new_addr_table_size),
                                                                     PU_STATIC,
                                                                     0);
    std::memset(new_addr_table, 0, sizeof(addrpair_t *) * static_cast<unsigned long>(new_addr_table_size));
    std::memcpy(new_addr_table, addr_table, sizeof(addrpair_t *) * static_cast<unsigned long>(addr_table_size));
    Z_Free(addr_table);
    addr_table      = new_addr_table;
    addr_table_size = new_addr_table_size;
  }

  // Add a new entry

  addrpair_t * new_entry       = zmalloc<decltype(new_entry)>(sizeof(addrpair_t), PU_STATIC, 0);
  new_entry->sdl_addr          = *addr;
  new_entry->net_addr.refcount = 0;
  new_entry->net_addr.handle   = &new_entry->sdl_addr;
  new_entry->net_addr.module   = &net_sdl_module;

  addr_table[empty_entry] = new_entry;

  return &new_entry->net_addr;
}

static void NET_SDL_FreeAddress(net_addr_t * addr) {
  for (int i = 0; i < addr_table_size; ++i) {
    if (addr == &addr_table[i]->net_addr) {
      Z_Free(addr_table[i]);
      addr_table[i] = nullptr;
      return;
    }
  }

  I_Error("NET_SDL_FreeAddress: Attempted to remove an unused address!");
}

static bool NET_SDL_InitClient() {
  if (initted)
    return true;

  //!
  // @category net
  // @arg <n>
  //
  // Use the specified UDP port for communications, instead of
  // the default (2342).
  //

  int p = M_CheckParmWithArgs("-port", 1);
  if (p > 0)
    port = std::atoi(myargv[p + 1]);

  SDLNet_Init();

  udpsocket = SDLNet_UDP_Open(0);

  if (udpsocket == nullptr) {
    I_Error("NET_SDL_InitClient: Unable to open a socket!");
  }

  recvpacket = SDLNet_AllocPacket(1500);

#ifdef DROP_PACKETS
  srand(time(nullptr));
#endif

  initted = true;

  return true;
}

static bool NET_SDL_InitServer() {
  if (initted)
    return true;

  int p = M_CheckParmWithArgs("-port", 1);
  if (p > 0)
    port = std::atoi(myargv[p + 1]);

  SDLNet_Init();

  udpsocket = SDLNet_UDP_Open(static_cast<Uint16>(port));

  if (udpsocket == nullptr) {
    I_Error("NET_SDL_InitServer: Unable to bind to port %i", port);
  }

  recvpacket = SDLNet_AllocPacket(1500);
#ifdef DROP_PACKETS
  srand(time(nullptr));
#endif

  initted = true;

  return true;
}

static void NET_SDL_SendPacket(net_addr_t * addr, net_packet_t * packet) {
  UDPpacket sdl_packet;
  IPaddress ip;

  if (addr == &net_broadcast_addr) {
    SDLNet_ResolveHost(&ip, nullptr, static_cast<Uint16>(port));
    ip.host = INADDR_BROADCAST;
  } else {
    ip = *(reinterpret_cast<IPaddress *>(addr->handle));
  }

#if 0
    {
        static int this_second_sent = 0;
        static int lasttime;

        this_second_sent += packet->len + 64;

        if (I_GetTime() - lasttime > TICRATE)
        {
            fmt::printf("%i bytes sent in the last second\n", this_second_sent);
            lasttime = I_GetTime();
            this_second_sent = 0;
        }
    }
#endif

#ifdef DROP_PACKETS
  if ((rand() % 4) == 0)
    return;
#endif

  sdl_packet.channel = 0;
  sdl_packet.data    = packet->data;
  sdl_packet.len     = static_cast<int>(packet->len);
  sdl_packet.address = ip;

  if (!SDLNet_UDP_Send(udpsocket, -1, &sdl_packet)) {
    I_Error("NET_SDL_SendPacket: Error transmitting packet: %s",
            SDLNet_GetError());
  }
}

static bool NET_SDL_RecvPacket(net_addr_t ** addr, net_packet_t ** packet) {
  int result = SDLNet_UDP_Recv(udpsocket, recvpacket);

  if (result < 0) {
    I_Error("NET_SDL_RecvPacket: Error receiving packet: %s",
            SDLNet_GetError());
  }

  // no packets received

  if (result == 0)
    return false;

  // Put the data into a new packet structure

  *packet = NET_NewPacket(recvpacket->len);
  std::memcpy((*packet)->data, recvpacket->data, static_cast<size_t>(recvpacket->len));
  (*packet)->len = static_cast<size_t>(recvpacket->len);

  // Address

  *addr = NET_SDL_FindAddress(&recvpacket->address);

  return true;
}

void NET_SDL_AddrToString(net_addr_t * addr, char * buffer, int buffer_len) {
  auto *   ip         = reinterpret_cast<IPaddress *>(addr->handle);
  uint32_t host       = SDLNet_Read32(&ip->host);
  uint16_t port_local = SDLNet_Read16(&ip->port);

  M_snprintf(buffer, static_cast<size_t>(buffer_len), "%i.%i.%i.%i", (host >> 24) & 0xff, (host >> 16) & 0xff, (host >> 8) & 0xff, host & 0xff);

  // If we are using the default port we just need to show the IP address,
  // but otherwise we need to include the port. This is important because
  // we use the string representation in the setup tool to provided an
  // address to connect to.
  if (port_local != DEFAULT_PORT) {
    char portbuf[10];
    M_snprintf(portbuf, sizeof(portbuf), ":%i", port_local);
    M_StringConcat(buffer, portbuf, static_cast<size_t>(buffer_len));
  }
}

net_addr_t * NET_SDL_ResolveAddress(const char * address) {
  const auto * colon = std::strchr(address, ':');

  char * addr_hostname = M_StringDuplicate(address);
  int    addr_port     = 0;
  if (colon != nullptr) {
    addr_hostname[colon - address] = '\0';
    addr_port                      = std::atoi(colon + 1);
  } else {
    addr_port = port;
  }

  IPaddress ip;
  int       result = SDLNet_ResolveHost(&ip, addr_hostname, static_cast<Uint16>(addr_port));

  free(addr_hostname);

  if (result) {
    // unable to resolve

    return nullptr;
  } else {
    return NET_SDL_FindAddress(&ip);
  }
}

// Complete module

net_module_t net_sdl_module = {
  NET_SDL_InitClient,
  NET_SDL_InitServer,
  NET_SDL_SendPacket,
  NET_SDL_RecvPacket,
  NET_SDL_AddrToString,
  NET_SDL_FreeAddress,
  NET_SDL_ResolveAddress,
};
