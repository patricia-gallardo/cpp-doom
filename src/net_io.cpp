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
//     Network packet I/O.  Base layer for sending/receiving packets,
//     through the network module system
//

#include "memory.hpp"
#include "i_system.hpp"
#include "net_defs.hpp"
#include "net_io.hpp"
#include "z_zone.hpp"

#define MAX_MODULES 16

struct [[maybe_unused]] _net_context_s {
    net_module_t *modules[MAX_MODULES];
    int           num_modules;
};

net_addr_t net_broadcast_addr;

net_context_t *NET_NewContext()
{
    auto *context        = zmalloc<net_context_t *>(sizeof(net_context_t), PU_STATIC, 0);
    context->num_modules = 0;

    return context;
}

void NET_AddModule(net_context_t *context, net_module_t *module)
{
    if (context->num_modules >= MAX_MODULES)
    {
        I_Error("NET_AddModule: No more modules for context");
    }

    context->modules[context->num_modules] = module;
    ++context->num_modules;
}

net_addr_t *NET_ResolveAddress(net_context_t *context, const char *addr)
{
    for (int i = 0; i < context->num_modules; ++i)
    {
        net_addr_t *result = context->modules[i]->ResolveAddress(addr);

        if (result != nullptr)
        {
            NET_ReferenceAddress(result);
            return result;
        }
    }

    return nullptr;
}

void NET_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    addr->module->SendPacket(addr, packet);
}

void NET_SendBroadcast(net_context_t *context, net_packet_t *packet)
{
    for (int i = 0; i < context->num_modules; ++i)
    {
        context->modules[i]->SendPacket(&net_broadcast_addr, packet);
    }
}

bool NET_RecvPacket(net_context_t *context,
    net_addr_t **                     addr,
    net_packet_t **                   packet)
{
    // check all modules for new packets

    for (int i = 0; i < context->num_modules; ++i)
    {
        if (context->modules[i]->RecvPacket(addr, packet))
        {
            NET_ReferenceAddress(*addr);
            return true;
        }
    }

    return false;
}

// Note: this prints into a static buffer, calling again overwrites
// the first result

char *NET_AddrToString(net_addr_t *addr)
{
    static char buf[128];

    addr->module->AddrToString(addr, buf, sizeof(buf) - 1);

    return buf;
}

void NET_ReferenceAddress(net_addr_t *addr)
{
    if (addr == nullptr)
    {
        return;
    }
    ++addr->refcount;
    //printf("%s: +refcount=%d\n", NET_AddrToString(addr), addr->refcount);
}

void NET_ReleaseAddress(net_addr_t *addr)
{
    if (addr == nullptr)
    {
        return;
    }

    --addr->refcount;
    //printf("%s: -refcount=%d\n", NET_AddrToString(addr), addr->refcount);
    if (addr->refcount <= 0)
    {
        addr->module->FreeAddress(addr);
    }
}
