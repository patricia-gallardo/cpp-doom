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
//     Querying servers to find their current status.
//

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include "i_system.hpp"
#include "i_timer.hpp"
#include "m_misc.hpp"

#include "net_common.hpp"
#include "net_defs.hpp"
#include "net_io.hpp"
#include "net_packet.hpp"
#include "net_query.hpp"
#include "net_structrw.hpp"
#include "net_sdl.hpp"

// DNS address of the Internet master server.

#define MASTER_SERVER_ADDRESS "master.chocolate-doom.org:2342"

// Time to wait for a response before declaring a timeout.

#define QUERY_TIMEOUT_SECS 2

// Time to wait for secure demo signatures before declaring a timeout.

#define SIGNATURE_TIMEOUT_SECS 5

// Number of query attempts to make before giving up on a server.

#define QUERY_MAX_ATTEMPTS 3

enum query_target_type_t
{
  QUERY_TARGET_SERVER,   // Normal server target.
  QUERY_TARGET_MASTER,   // The master server.
  QUERY_TARGET_BROADCAST // Send a broadcast query
};

enum query_target_state_t
{
  QUERY_TARGET_QUEUED,    // Query not yet sent
  QUERY_TARGET_QUERIED,   // Query sent, waiting response
  QUERY_TARGET_RESPONDED, // Response received
  QUERY_TARGET_NO_RESPONSE
};

struct query_target_t {
  query_target_type_t  type;
  query_target_state_t state;
  net_addr_t *         addr;
  net_querydata_t      data;
  unsigned int         ping_time;
  unsigned int         query_time;
  unsigned int         query_attempts;
  bool                 printed;
};

static bool registered_with_master = false;
static bool got_master_response    = false;

static net_context_t *  query_context;
static query_target_t * targets;
static int              num_targets;

static bool query_loop_running = false;
static bool printed_header     = false;
static int  last_query_time    = 0;

static char * securedemo_start_message = nullptr;

// Resolve the master server address.

net_addr_t * NET_Query_ResolveMaster(net_context_t * context) {
  net_addr_t * addr = NET_ResolveAddress(context, MASTER_SERVER_ADDRESS);

  if (addr == nullptr) {
    fmt::fprintf(stderr, "Warning: Failed to resolve address "
                         "for master server: %s\n",
                 MASTER_SERVER_ADDRESS);
  }

  return addr;
}

// Send a registration packet to the master server to register
// ourselves with the global list.

void NET_Query_AddToMaster(net_addr_t * master_addr) {
  net_packet_t * packet = NET_NewPacket(10);
  NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_ADD);
  NET_SendPacket(master_addr, packet);
  NET_FreePacket(packet);
}

// Process a packet received from the master server.

void NET_Query_AddResponse(net_packet_t * packet) {
  unsigned int result = 0;

  if (!NET_ReadInt16(packet, &result)) {
    return;
  }

  if (result != 0) {
    // Only show the message once.

    if (!registered_with_master) {
      fmt::printf("Registered with master server at %s\n",
                  MASTER_SERVER_ADDRESS);
      registered_with_master = true;
    }
  } else {
    // Always show rejections.

    fmt::printf("Failed to register with master server at %s\n",
                MASTER_SERVER_ADDRESS);
  }

  got_master_response = true;
}

bool NET_Query_CheckAddedToMaster(bool * result) {
  // Got response from master yet?

  if (!got_master_response) {
    return false;
  }

  *result = registered_with_master;
  return true;
}

// Send a query to the master server.

static void NET_Query_SendMasterQuery(net_addr_t * addr) {
  net_packet_t * packet = NET_NewPacket(4);
  NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_QUERY);
  NET_SendPacket(addr, packet);
  NET_FreePacket(packet);

  // We also send a NAT_HOLE_PUNCH_ALL packet so that servers behind
  // NAT gateways will open themselves up to us.
  packet = NET_NewPacket(4);
  NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_NAT_HOLE_PUNCH_ALL);
  NET_SendPacket(addr, packet);
  NET_FreePacket(packet);
}

// Send a hole punch request to the master server for the server at the
// given address.
void NET_RequestHolePunch(net_context_t * context, net_addr_t * addr) {
  net_addr_t * master_addr = NET_Query_ResolveMaster(context);
  if (master_addr == nullptr) {
    return;
  }

  net_packet_t * packet = NET_NewPacket(32);
  NET_WriteInt16(packet, NET_MASTER_PACKET_TYPE_NAT_HOLE_PUNCH);
  NET_WriteString(packet, NET_AddrToString(addr));
  NET_SendPacket(master_addr, packet);

  NET_FreePacket(packet);
  NET_ReleaseAddress(master_addr);
}

// Given the specified address, find the target associated.  If no
// target is found, and 'create' is true, a new target is created.

static query_target_t * GetTargetForAddr(net_addr_t * addr, bool create) {
  for (int i = 0; i < num_targets; ++i) {
    if (targets[i].addr == addr) {
      return &targets[i];
    }
  }

  if (!create) {
    return nullptr;
  }

  targets                 = static_cast<query_target_t *>(I_Realloc(targets, sizeof(query_target_t) * (static_cast<unsigned long>(num_targets + 1))));
  query_target_t * target = &targets[num_targets];
  target->type            = QUERY_TARGET_SERVER;
  target->state           = QUERY_TARGET_QUEUED;
  target->printed         = false;
  target->query_attempts  = 0;
  target->addr            = addr;
  NET_ReferenceAddress(addr);
  ++num_targets;

  return target;
}

static void FreeTargets() {
  for (int i = 0; i < num_targets; ++i) {
    NET_ReleaseAddress(targets[i].addr);
  }
  free(targets);
  targets     = nullptr;
  num_targets = 0;
}

// Transmit a query packet

static void NET_Query_SendQuery(net_addr_t * addr) {
  net_packet_t * request = NET_NewPacket(10);
  NET_WriteInt16(request, NET_PACKET_TYPE_QUERY);

  if (addr == nullptr) {
    NET_SendBroadcast(query_context, request);
  } else {
    NET_SendPacket(addr, request);
  }

  NET_FreePacket(request);
}

static void NET_Query_ParseResponse(net_addr_t * addr, net_packet_t * packet, net_query_callback_t callback, void * user_data) {
  // Read the header

  unsigned int packet_type = 0;
  if (!NET_ReadInt16(packet, &packet_type)
      || packet_type != NET_PACKET_TYPE_QUERY_RESPONSE) {
    return;
  }

  // Read query data

  net_querydata_t querydata;
  if (!NET_ReadQueryData(packet, &querydata)) {
    return;
  }

  // Find the target that responded.

  query_target_t * target = GetTargetForAddr(addr, false);

  // If the target is not found, it may be because we are doing
  // a LAN broadcast search, in which case we need to create a
  // target for the new responder.

  if (target == nullptr) {
    query_target_t * broadcast_target = GetTargetForAddr(nullptr, false);

    // Not in broadcast mode, unexpected response that came out
    // of nowhere. Ignore.

    if (broadcast_target == nullptr
        || broadcast_target->state != QUERY_TARGET_QUERIED) {
      return;
    }

    // Create new target.

    target             = GetTargetForAddr(addr, true);
    target->state      = QUERY_TARGET_QUERIED;
    target->query_time = broadcast_target->query_time;
  }

  if (target->state != QUERY_TARGET_RESPONDED) {
    target->state = QUERY_TARGET_RESPONDED;
    std::memcpy(&target->data, &querydata, sizeof(net_querydata_t));

    // Calculate RTT.

    target->ping_time = static_cast<unsigned int>(I_GetTimeMS()) - target->query_time;

    // Invoke callback to signal that we have a new address.

    callback(addr, &target->data, target->ping_time, user_data);
  }
}

// Parse a response packet from the master server.

static void NET_Query_ParseMasterResponse(net_addr_t * master_addr, net_packet_t * packet) {
  // Read the header.  We are only interested in query responses.

  unsigned int packet_type = 0;
  if (!NET_ReadInt16(packet, &packet_type)
      || packet_type != NET_MASTER_PACKET_TYPE_QUERY_RESPONSE) {
    return;
  }

  // Read a list of strings containing the addresses of servers
  // that the master knows about.

  for (;;) {
    char * addr_str = NET_ReadString(packet);

    if (addr_str == nullptr) {
      break;
    }

    // Resolve address and add to targets list if it is not already
    // there.

    net_addr_t * addr = NET_ResolveAddress(query_context, addr_str);
    if (addr != nullptr) {
      GetTargetForAddr(addr, true);
      NET_ReleaseAddress(addr);
    }
  }

  // Mark the master as having responded.

  query_target_t * target = GetTargetForAddr(master_addr, true);
  target->state           = QUERY_TARGET_RESPONDED;
}

static void NET_Query_ParsePacket(net_addr_t * addr, net_packet_t * packet, net_query_callback_t callback, void * user_data) {
  // This might be the master server responding.

  query_target_t * target = GetTargetForAddr(addr, false);

  if (target != nullptr && target->type == QUERY_TARGET_MASTER) {
    NET_Query_ParseMasterResponse(addr, packet);
  } else {
    NET_Query_ParseResponse(addr, packet, callback, user_data);
  }
}

static void NET_Query_GetResponse(net_query_callback_t callback,
                                  void *               user_data) {
  net_addr_t *   addr   = nullptr;
  net_packet_t * packet = nullptr;

  if (NET_RecvPacket(query_context, &addr, &packet)) {
    NET_Query_ParsePacket(addr, packet, callback, user_data);
    NET_ReleaseAddress(addr);
    NET_FreePacket(packet);
  }
}

// Find a target we have not yet queried and send a query.

static void SendOneQuery() {
  auto now = static_cast<unsigned int>(I_GetTimeMS());

  // Rate limit - only send one query every 50ms.

  if (now - static_cast<unsigned int>(last_query_time) < 50) {
    return;
  }

  int i = 0;
  for (i = 0; i < num_targets; ++i) {
    // Not queried yet?
    // Or last query timed out without a response?

    if (targets[i].state == QUERY_TARGET_QUEUED
        || (targets[i].state == QUERY_TARGET_QUERIED
            && now - targets[i].query_time > QUERY_TIMEOUT_SECS * 1000)) {
      break;
    }
  }

  if (i >= num_targets) {
    return;
  }

  // Found a target to query.  Send a query; how to do this depends on
  // the target type.

  switch (targets[i].type) {
  case QUERY_TARGET_SERVER:
    NET_Query_SendQuery(targets[i].addr);
    break;

  case QUERY_TARGET_BROADCAST:
    NET_Query_SendQuery(nullptr);
    break;

  case QUERY_TARGET_MASTER:
    NET_Query_SendMasterQuery(targets[i].addr);
    break;
  }

  // printf("Queried %s\n", NET_AddrToString(targets[i].addr));
  targets[i].state      = QUERY_TARGET_QUERIED;
  targets[i].query_time = now;
  ++targets[i].query_attempts;

  last_query_time = static_cast<int>(now);
}

// Time out servers that have been queried and not responded.

static void CheckTargetTimeouts() {
  auto now = static_cast<unsigned int>(I_GetTimeMS());

  for (int i = 0; i < num_targets; ++i) {
    /*
   fmt::printf("target %i: state %i, queries %i, query time %i\n",
           i, targets[i].state, targets[i].query_attempts,
           now - targets[i].query_time);
    */

    // We declare a target to be "no response" when we've sent
    // multiple query packets to it (QUERY_MAX_ATTEMPTS) and
    // received no response to any of them.

    if (targets[i].state == QUERY_TARGET_QUERIED
        && targets[i].query_attempts >= QUERY_MAX_ATTEMPTS
        && now - targets[i].query_time > QUERY_TIMEOUT_SECS * 1000) {
      targets[i].state = QUERY_TARGET_NO_RESPONSE;

      if (targets[i].type == QUERY_TARGET_MASTER) {
        fmt::fprintf(stderr, "NET_MasterQuery: no response "
                             "from master server.\n");
      }
    }
  }
}

// If all targets have responded or timed out, returns true.

static bool AllTargetsDone() {
  for (int i = 0; i < num_targets; ++i) {
    if (targets[i].state != QUERY_TARGET_RESPONDED
        && targets[i].state != QUERY_TARGET_NO_RESPONSE) {
      return false;
    }
  }

  return true;
}

// Polling function, invoked periodically to send queries and
// interpret new responses received from remote servers.
// Returns zero when the query sequence has completed and all targets
// have returned responses or timed out.

int NET_Query_Poll(net_query_callback_t callback, void * user_data) {
  CheckTargetTimeouts();

  // Send a query.  This will only send a single query at once.

  SendOneQuery();

  // Check for a response

  NET_Query_GetResponse(callback, user_data);

  return !AllTargetsDone();
}

// Stop the query loop

static void NET_Query_ExitLoop() {
  query_loop_running = false;
}

// Loop waiting for responses.
// The specified callback is invoked when a new server responds.

static void NET_Query_QueryLoop(net_query_callback_t callback, void * user_data) {
  query_loop_running = true;

  while (query_loop_running && NET_Query_Poll(callback, user_data)) {
    // Don't thrash the CPU

    I_Sleep(1);
  }
}

void NET_Query_Init() {
  if (query_context == nullptr) {
    query_context = NET_NewContext();
    NET_AddModule(query_context, &net_sdl_module);
    net_sdl_module.InitClient();
  }

  free(targets);
  targets     = nullptr;
  num_targets = 0;

  printed_header = false;
}

// Callback that exits the query loop when the first server is found.

static void NET_Query_ExitCallback(net_addr_t *, net_querydata_t *, unsigned int, void *) {
  NET_Query_ExitLoop();
}

// Search the targets list and find a target that has responded.
// If none have responded, returns NULL.

static query_target_t * FindFirstResponder() {
  for (int i = 0; i < num_targets; ++i) {
    if (targets[i].type == QUERY_TARGET_SERVER
        && targets[i].state == QUERY_TARGET_RESPONDED) {
      return &targets[i];
    }
  }

  return nullptr;
}

// Return a count of the number of responses.

static int GetNumResponses() {
  int result = 0;

  for (int i = 0; i < num_targets; ++i) {
    if (targets[i].type == QUERY_TARGET_SERVER
        && targets[i].state == QUERY_TARGET_RESPONDED) {
      ++result;
    }
  }

  return result;
}

int NET_StartLANQuery() {
  NET_Query_Init();

  // Add a broadcast target to the list.

  query_target_t * target = GetTargetForAddr(nullptr, true);
  target->type            = QUERY_TARGET_BROADCAST;

  return 1;
}

int NET_StartMasterQuery() {
  NET_Query_Init();

  // Resolve master address and add to targets list.

  net_addr_t * master = NET_Query_ResolveMaster(query_context);

  if (master == nullptr) {
    return 0;
  }

  query_target_t * target = GetTargetForAddr(master, true);
  target->type            = QUERY_TARGET_MASTER;
  NET_ReleaseAddress(master);

  return 1;
}

// -----------------------------------------------------------------------

static void formatted_printf(int wide, const char * s, ...) PRINTF_ATTR(2, 3);
static void formatted_printf(int wide, const char * s, ...) {
  va_list args;

  va_start(args, s);
  int i = vprintf(s, args);
  va_end(args);

  while (i < wide) {
    putchar(' ');
    ++i;
  }
}

static constexpr const char * GameDescription(int mode, int mission) {
  switch (mission) {
  case doom:
    if (mode == shareware)
      return "swdoom";
    else if (mode == registered)
      return "regdoom";
    else if (mode == retail)
      return "ultdoom";
    else
      return "doom";
  case doom2:
    return "doom2";
  case pack_tnt:
    return "tnt";
  case pack_plut:
    return "plutonia";
  case pack_chex:
    return "chex";
  case pack_hacx:
    return "hacx";
  case heretic:
    return "heretic";
  case hexen:
    return "hexen";
  case strife:
    return "strife";
  default:
    return "?";
  }
}

static void PrintHeader() {
  putchar('\n');
  formatted_printf(5, "Ping");
  formatted_printf(18, "Address");
  formatted_printf(8, "Players");
  puts("Description");

  for (int i = 0; i < 70; ++i)
    putchar('=');
  putchar('\n');
}

// Callback function that just prints information in a table.

static void NET_QueryPrintCallback(net_addr_t * addr, net_querydata_t * data, unsigned int ping_time, void *) {
  // If this is the first server, print the header.

  if (!printed_header) {
    PrintHeader();
    printed_header = true;
  }

  formatted_printf(5, "%4i", ping_time);
  formatted_printf(22, "%s", NET_AddrToString(addr));
  formatted_printf(4, "%i/%i ", data->num_players, data->max_players);

  if (data->gamemode != indetermined) {
    fmt::printf("(%s) ", GameDescription(data->gamemode, data->gamemission));
  }

  if (data->server_state) {
    fmt::printf("(game running) ");
  }

  fmt::printf("%s\n", data->description);
}

void NET_LANQuery() {
  if (NET_StartLANQuery()) {
    fmt::printf("\nSearching for servers on local LAN ...\n");

    NET_Query_QueryLoop(NET_QueryPrintCallback, nullptr);

    fmt::printf("\n%i server(s) found.\n", GetNumResponses());
    FreeTargets();
  }
}

void NET_MasterQuery() {
  if (NET_StartMasterQuery()) {
    fmt::printf("\nSearching for servers on Internet ...\n");

    NET_Query_QueryLoop(NET_QueryPrintCallback, nullptr);

    fmt::printf("\n%i server(s) found.\n", GetNumResponses());
    FreeTargets();
  }
}

void NET_QueryAddress(char * addr_str) {
  NET_Query_Init();

  net_addr_t * addr = NET_ResolveAddress(query_context, addr_str);

  if (addr == nullptr) {
    I_Error("NET_QueryAddress: Host '%s' not found!", addr_str);
  }

  // Add the address to the list of targets.

  query_target_t * target = GetTargetForAddr(addr, true);

  fmt::printf("\nQuerying '%s'...\n", addr_str);

  // Run query loop.

  NET_Query_QueryLoop(NET_Query_ExitCallback, nullptr);

  // Check if the target responded.

  if (target->state == QUERY_TARGET_RESPONDED) {
    NET_QueryPrintCallback(addr, &target->data, target->ping_time, nullptr);
    NET_ReleaseAddress(addr);
    FreeTargets();
  } else {
    I_Error("No response from '%s'", addr_str);
  }
}

net_addr_t * NET_FindLANServer() {
  NET_Query_Init();

  // Add a broadcast target to the list.

  query_target_t * target = GetTargetForAddr(nullptr, true);
  target->type            = QUERY_TARGET_BROADCAST;

  // Run the query loop, and stop at the first target found.

  NET_Query_QueryLoop(NET_Query_ExitCallback, nullptr);

  query_target_t * responder = FindFirstResponder();

  net_addr_t * result = nullptr;
  if (responder != nullptr) {
    result = responder->addr;
    NET_ReferenceAddress(result);
  } else {
    result = nullptr;
  }

  FreeTargets();
  return result;
}

// Block until a packet of the given type is received from the given
// address.

static net_packet_t * BlockForPacket(net_addr_t * addr, unsigned int packet_type, unsigned int timeout_ms) {
  auto start_time = static_cast<unsigned int>(I_GetTimeMS());

  while (I_GetTimeMS() < static_cast<int>(start_time + timeout_ms)) {
    net_addr_t *   packet_src = nullptr;
    net_packet_t * packet     = nullptr;
    if (!NET_RecvPacket(query_context, &packet_src, &packet)) {
      I_Sleep(20);
      continue;
    }

    // Caller doesn't need additional reference.
    NET_ReleaseAddress(packet_src);

    unsigned int read_packet_type = 0;
    if (packet_src == addr
        && NET_ReadInt16(packet, &read_packet_type)
        && packet_type == read_packet_type) {
      return packet;
    }

    NET_FreePacket(packet);
  }

  // Timeout - no response.

  return nullptr;
}

// Query master server for secure demo start seed value.

[[maybe_unused]] bool NET_StartSecureDemo(prng_seed_t seed) {
  NET_Query_Init();
  net_addr_t * master_addr = NET_Query_ResolveMaster(query_context);

  // Send request packet to master server.

  net_packet_t * request = NET_NewPacket(10);
  NET_WriteInt16(request, NET_MASTER_PACKET_TYPE_SIGN_START);
  NET_SendPacket(master_addr, request);
  NET_FreePacket(request);

  // Block for response and read contents.
  // The signed start message will be saved for later.

  net_packet_t * response = BlockForPacket(master_addr,
                                           NET_MASTER_PACKET_TYPE_SIGN_START_RESPONSE,
                                           SIGNATURE_TIMEOUT_SECS * 1000);

  bool result = false;

  if (response != nullptr) {
    if (NET_ReadPRNGSeed(response, seed)) {
      char * signature = NET_ReadString(response);

      if (signature != nullptr) {
        securedemo_start_message = M_StringDuplicate(signature);
        result                   = true;
      }
    }

    NET_FreePacket(response);
  }

  return result;
}

// Query master server for secure demo end signature.

[[maybe_unused]] char * NET_EndSecureDemo(sha1_digest_t demo_hash) {
  net_addr_t * master_addr = NET_Query_ResolveMaster(query_context);

  // Construct end request and send to master server.

  net_packet_t * request = NET_NewPacket(10);
  NET_WriteInt16(request, NET_MASTER_PACKET_TYPE_SIGN_END);
  NET_WriteSHA1Sum(request, demo_hash);
  NET_WriteString(request, securedemo_start_message);
  NET_SendPacket(master_addr, request);
  NET_FreePacket(request);

  // Block for response. The response packet simply contains a string
  // with the ASCII signature.

  net_packet_t * response = BlockForPacket(master_addr,
                                           NET_MASTER_PACKET_TYPE_SIGN_END_RESPONSE,
                                           SIGNATURE_TIMEOUT_SECS * 1000);

  if (response == nullptr) {
    return nullptr;
  }

  char * signature = NET_ReadString(response);

  NET_FreePacket(response);

  return signature;
}
