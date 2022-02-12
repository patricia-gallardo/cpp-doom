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
//      Network packet manipulation (net_packet_t)
//

#include "net_packet.hpp"
#include "memory.hpp"
#include "m_misc.hpp"
#include "z_zone.hpp"
#include <cctype>
#include <cstring>

static size_t total_packet_memory = 0;

net_packet_t *NET_NewPacket(int initial_size)
{
    net_packet_t *packet = zmalloc<net_packet_t *>(sizeof(net_packet_t), PU_STATIC, 0);

    if (initial_size == 0)
        initial_size = 256;

    packet->alloced = static_cast<size_t>(initial_size);
    packet->data    = zmalloc<uint8_t *>(static_cast<size_t>(initial_size), PU_STATIC, 0);
    packet->len     = 0;
    packet->pos     = 0;

    total_packet_memory += sizeof(net_packet_t) + static_cast<unsigned long>(initial_size);

    //printf("total packet memory: %i bytes\n", total_packet_memory);
    //printf("%p: allocated\n", packet);

    return packet;
}

// duplicates an existing packet

net_packet_t *NET_PacketDup(net_packet_t *packet)
{
    net_packet_t *newpacket = NET_NewPacket(static_cast<int>(packet->len));
    memcpy(newpacket->data, packet->data, packet->len);
    newpacket->len = packet->len;

    return newpacket;
}

void NET_FreePacket(net_packet_t *packet)
{
    //printf("%p: destroyed\n", packet);

    total_packet_memory -= sizeof(net_packet_t) + packet->alloced;
    Z_Free(packet->data);
    Z_Free(packet);
}

// Read a byte from the packet, returning true if read
// successfully

bool NET_ReadInt8(net_packet_t *packet, unsigned int *data)
{
    if (packet->pos + 1 > packet->len)
        return false;

    *data = packet->data[packet->pos];

    packet->pos += 1;

    return true;
}

// Read a 16-bit integer from the packet, returning true if read
// successfully

bool NET_ReadInt16(net_packet_t *packet, unsigned int *data)
{
    uint8_t *p;

    if (packet->pos + 2 > packet->len)
        return false;

    p = packet->data + packet->pos;

    *data = static_cast<unsigned int>((p[0] << 8) | p[1]);
    packet->pos += 2;

    return true;
}

// Read a 32-bit integer from the packet, returning true if read
// successfully

bool NET_ReadInt32(net_packet_t *packet, unsigned int *data)
{
    uint8_t *p;

    if (packet->pos + 4 > packet->len)
        return false;

    p = packet->data + packet->pos;

    *data = static_cast<unsigned int>((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
    packet->pos += 4;

    return true;
}

// Signed read functions

bool NET_ReadSInt8(net_packet_t *packet, signed int *data)
{
    if (NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(data)))
    {
        if (*data & (1 << 7))
        {
            *data &= ~(1 << 7);
            *data -= (1 << 7);
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool NET_ReadSInt16(net_packet_t *packet, signed int *data)
{
    if (NET_ReadInt16(packet, reinterpret_cast<unsigned int *>(data)))
    {
        if (*data & (1 << 15))
        {
            *data &= ~(1 << 15);
            *data -= (1 << 15);
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool NET_ReadSInt32(net_packet_t *packet, signed int *data)
{
    if (NET_ReadInt32(packet, reinterpret_cast<unsigned int *>(data)))
    {
        if (static_cast<unsigned int>(*data) & (1U << 31))
        {
            *data &= ~(1U << 31);
            *data -= (1U << 31);
        }
        return true;
    }
    else
    {
        return false;
    }
}

// Read a string from the packet.  Returns nullptr if a terminating
// NUL character was not found before the end of the packet.

char *NET_ReadString(net_packet_t *packet)
{
    char *start = reinterpret_cast<char *>(packet->data) + packet->pos;

    // Search forward for a NUL character

    while (packet->pos < packet->len && packet->data[packet->pos] != '\0')
    {
        ++packet->pos;
    }

    if (packet->pos >= packet->len)
    {
        // Reached the end of the packet

        return nullptr;
    }

    // packet->data[packet->pos] == '\0': We have reached a terminating
    // NULL.  Skip past this nullptr and continue reading immediately
    // after it.

    ++packet->pos;

    return start;
}

// Read a string from the packet, but (potentially) modify it to strip
// out any unprintable characters which could be malicious control codes.
// Note that this may modify the original packet contents.
char *NET_ReadSafeString(net_packet_t *packet)
{
    char *r, *w, *result;

    result = NET_ReadString(packet);
    if (result == nullptr)
    {
        return nullptr;
    }

    // w is always <= r, so we never produce a longer string than the original.
    w = result;
    for (r = result; *r != '\0'; ++r)
    {
        // TODO: This is a very naive way of producing a safe string; only
        // ASCII characters are allowed. Probably this should really support
        // UTF-8 characters as well.
        if (isprint(*r) || *r == '\n')
        {
            *w = *r;
            ++w;
        }
    }
    *w = '\0';

    return result;
}

// Dynamically increases the size of a packet

static void NET_IncreasePacket(net_packet_t *packet)
{
    total_packet_memory -= packet->alloced;

    packet->alloced *= 2;

    auto *newdata = zmalloc<uint8_t *>(packet->alloced, PU_STATIC, 0);

    memcpy(newdata, packet->data, packet->len);

    Z_Free(packet->data);
    packet->data = newdata;

    total_packet_memory += packet->alloced;
}

// Write a single byte to the packet

void NET_WriteInt8(net_packet_t *packet, unsigned int i)
{
    if (packet->len + 1 > packet->alloced)
        NET_IncreasePacket(packet);

    packet->data[packet->len] = static_cast<uint8_t>(i);
    packet->len += 1;
}

// Write a 16-bit integer to the packet

void NET_WriteInt16(net_packet_t *packet, unsigned int i)
{
    uint8_t *p;

    if (packet->len + 2 > packet->alloced)
        NET_IncreasePacket(packet);

    p = packet->data + packet->len;

    p[0] = (i >> 8) & 0xff;
    p[1] = i & 0xff;

    packet->len += 2;
}


// Write a single byte to the packet

void NET_WriteInt32(net_packet_t *packet, unsigned int i)
{
    uint8_t *p;

    if (packet->len + 4 > packet->alloced)
        NET_IncreasePacket(packet);

    p = packet->data + packet->len;

    p[0] = static_cast<uint8_t>((i >> 24) & 0xff);
    p[1] = (i >> 16) & 0xff;
    p[2] = (i >> 8) & 0xff;
    p[3] = i & 0xff;

    packet->len += 4;
}

void NET_WriteString(net_packet_t *packet, const char *string)
{
    size_t string_size = strlen(string) + 1;

    // Increase the packet size until large enough to hold the string

    while (packet->len + string_size > packet->alloced)
    {
        NET_IncreasePacket(packet);
    }

    uint8_t *p = packet->data + packet->len;

    M_StringCopy(reinterpret_cast<char *>(p), string, string_size);

    packet->len += string_size;
}
