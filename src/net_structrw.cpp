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
// Reading and writing various structures into packets
//

#include <array>
#include <cstring>

#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_misc.hpp"
#include "net_packet.hpp"
#include "net_structrw.hpp"

// String names for the enum values in net_protocol_t, which are what is
// sent over the wire. Every enum value must have an entry in this list.
static struct
{
    net_protocol_t protocol;
    const char *   name;
} protocol_names[] = {
    { NET_PROTOCOL_CHOCOLATE_DOOM_0, "CHOCOLATE_DOOM_0" },
};

void NET_WriteConnectData(net_packet_t *packet, net_connect_data_t *data)
{
    NET_WriteInt8(packet, static_cast<unsigned int>(data->gamemode));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->gamemission));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->lowres_turn));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->drone));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->max_players));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->is_freedoom));
    NET_WriteSHA1Sum(packet, data->wad_sha1sum);
    NET_WriteSHA1Sum(packet, data->deh_sha1sum);
    NET_WriteInt8(packet, static_cast<unsigned int>(data->player_class));
}

bool NET_ReadConnectData(net_packet_t *packet, net_connect_data_t *data)
{
    return NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->gamemode))
           && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->gamemission))
           && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->lowres_turn))
           && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->drone))
           && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->max_players))
           && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->is_freedoom))
           && NET_ReadSHA1Sum(packet, data->wad_sha1sum)
           && NET_ReadSHA1Sum(packet, data->deh_sha1sum)
           && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->player_class));
}

void NET_WriteSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->ticdup));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->extratics));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->deathmatch));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->nomonsters));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->fast_monsters));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->respawn_monsters));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->episode));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->map));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->skill));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->gameversion));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->lowres_turn));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->new_sync));
    NET_WriteInt32(packet, static_cast<unsigned int>(settings->timelimit));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->loadgame));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->random));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->num_players));
    NET_WriteInt8(packet, static_cast<unsigned int>(settings->consoleplayer));

    for (int i = 0; i < settings->num_players; ++i)
    {
        NET_WriteInt8(packet, static_cast<unsigned int>(settings->player_classes[i]));
    }
}

bool NET_ReadSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    bool success = NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->ticdup))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->extratics))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->deathmatch))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->nomonsters))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->fast_monsters))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->respawn_monsters))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->episode))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->map))
              && NET_ReadSInt8(packet, &settings->skill)
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->gameversion))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->lowres_turn))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->new_sync))
              && NET_ReadInt32(packet, reinterpret_cast<unsigned int *>(&settings->timelimit))
              && NET_ReadSInt8(packet, reinterpret_cast<signed int *>(&settings->loadgame))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->random))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->num_players))
              && NET_ReadSInt8(packet, reinterpret_cast<signed int *>(&settings->consoleplayer));

    if (!success)
    {
        return false;
    }

    for (int i = 0; i < settings->num_players; ++i)
    {
        if (!NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&settings->player_classes[i])))
        {
            return false;
        }
    }

    return true;
}

bool NET_ReadQueryData(net_packet_t *packet, net_querydata_t *query)
{
    query->version = NET_ReadSafeString(packet);

    bool success = query->version != nullptr
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&query->server_state))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&query->num_players))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&query->max_players))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&query->gamemode))
              && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&query->gamemission));

    if (!success)
    {
        return false;
    }

    query->description = NET_ReadSafeString(packet);

    // We read the list of protocols supported by the server. However,
    // old versions of Chocolate Doom do not support this field; it is
    // okay if it cannot be successfully read.
    query->protocol = NET_ReadProtocolList(packet);

    return query->description != nullptr;
}

void NET_WriteQueryData(net_packet_t *packet, net_querydata_t *query)
{
    NET_WriteString(packet, query->version);
    NET_WriteInt8(packet, static_cast<unsigned int>(query->server_state));
    NET_WriteInt8(packet, static_cast<unsigned int>(query->num_players));
    NET_WriteInt8(packet, static_cast<unsigned int>(query->max_players));
    NET_WriteInt8(packet, static_cast<unsigned int>(query->gamemode));
    NET_WriteInt8(packet, static_cast<unsigned int>(query->gamemission));
    NET_WriteString(packet, query->description);

    // Write a list of all supported protocols. Note that the query->protocol
    // field is ignored here; it is only used when receiving.
    NET_WriteProtocolList(packet);
}

void NET_WriteTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff,
    bool lowres_turn)
{
    // Header

    NET_WriteInt8(packet, diff->diff);

    // Write the fields which are enabled:

    if (diff->diff & NET_TICDIFF_FORWARD)
        NET_WriteInt8(packet, static_cast<unsigned int>(diff->cmd.forwardmove));
    if (diff->diff & NET_TICDIFF_SIDE)
        NET_WriteInt8(packet, static_cast<unsigned int>(diff->cmd.sidemove));
    if (diff->diff & NET_TICDIFF_TURN)
    {
        if (lowres_turn)
        {
            NET_WriteInt8(packet, static_cast<unsigned int>(diff->cmd.angleturn / 256));
        }
        else
        {
            NET_WriteInt16(packet, static_cast<unsigned int>(diff->cmd.angleturn));
        }
    }
    if (diff->diff & NET_TICDIFF_BUTTONS)
        NET_WriteInt8(packet, diff->cmd.buttons);
    if (diff->diff & NET_TICDIFF_CONSISTANCY)
        NET_WriteInt8(packet, diff->cmd.consistancy);
    if (diff->diff & NET_TICDIFF_CHATCHAR)
        NET_WriteInt8(packet, diff->cmd.chatchar);
    if (diff->diff & NET_TICDIFF_RAVEN)
    {
        NET_WriteInt8(packet, diff->cmd.lookfly);
        NET_WriteInt8(packet, diff->cmd.arti);
    }
    if (diff->diff & NET_TICDIFF_STRIFE)
    {
        NET_WriteInt8(packet, diff->cmd.buttons2);
        NET_WriteInt16(packet, static_cast<unsigned int>(diff->cmd.inventory));
    }
}

bool NET_ReadTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff,
    bool lowres_turn)
{
    unsigned int val = 0;
    signed int   sval = 0;

    // Read header

    if (!NET_ReadInt8(packet, &diff->diff))
        return false;

    // Read fields

    if (diff->diff & NET_TICDIFF_FORWARD)
    {
        if (!NET_ReadSInt8(packet, &sval))
            return false;
        diff->cmd.forwardmove = static_cast<signed char>(sval);
    }

    if (diff->diff & NET_TICDIFF_SIDE)
    {
        if (!NET_ReadSInt8(packet, &sval))
            return false;
        diff->cmd.sidemove = static_cast<signed char>(sval);
    }

    if (diff->diff & NET_TICDIFF_TURN)
    {
        if (lowres_turn)
        {
            if (!NET_ReadSInt8(packet, &sval))
                return false;
            diff->cmd.angleturn = static_cast<short>(sval * 256);
        }
        else
        {
            if (!NET_ReadSInt16(packet, &sval))
                return false;
            diff->cmd.angleturn = static_cast<short>(sval);
        }
    }

    if (diff->diff & NET_TICDIFF_BUTTONS)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.buttons = static_cast<uint8_t>(val);
    }

    if (diff->diff & NET_TICDIFF_CONSISTANCY)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.consistancy = static_cast<uint8_t>(val);
    }

    if (diff->diff & NET_TICDIFF_CHATCHAR)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.chatchar = static_cast<uint8_t>(val);
    }

    if (diff->diff & NET_TICDIFF_RAVEN)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.lookfly = static_cast<uint8_t>(val);

        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.arti = static_cast<uint8_t>(val);
    }

    if (diff->diff & NET_TICDIFF_STRIFE)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.buttons2 = static_cast<uint8_t>(val);

        if (!NET_ReadInt16(packet, &val))
            return false;
        diff->cmd.inventory = static_cast<int>(val);
    }

    return true;
}

void NET_TiccmdDiff(ticcmd_t *tic1, ticcmd_t *tic2, net_ticdiff_t *diff)
{
    diff->diff = 0;
    diff->cmd  = *tic2;

    if (tic1->forwardmove != tic2->forwardmove)
        diff->diff |= NET_TICDIFF_FORWARD;
    if (tic1->sidemove != tic2->sidemove)
        diff->diff |= NET_TICDIFF_SIDE;
    if (tic1->angleturn != tic2->angleturn)
        diff->diff |= NET_TICDIFF_TURN;
    if (tic1->buttons != tic2->buttons)
        diff->diff |= NET_TICDIFF_BUTTONS;
    if (tic1->consistancy != tic2->consistancy)
        diff->diff |= NET_TICDIFF_CONSISTANCY;
    if (tic2->chatchar != 0)
        diff->diff |= NET_TICDIFF_CHATCHAR;

    // Heretic/Hexen-specific

    if (tic1->lookfly != tic2->lookfly || tic2->arti != 0)
        diff->diff |= NET_TICDIFF_RAVEN;

    // Strife-specific

    if (tic1->buttons2 != tic2->buttons2 || tic2->inventory != 0)
        diff->diff |= NET_TICDIFF_STRIFE;
}

void NET_TiccmdPatch(ticcmd_t *src, net_ticdiff_t *diff, ticcmd_t *dest)
{
    std::memmove(dest, src, sizeof(ticcmd_t));

    // Apply the diff

    if (diff->diff & NET_TICDIFF_FORWARD)
        dest->forwardmove = diff->cmd.forwardmove;
    if (diff->diff & NET_TICDIFF_SIDE)
        dest->sidemove = diff->cmd.sidemove;
    if (diff->diff & NET_TICDIFF_TURN)
        dest->angleturn = diff->cmd.angleturn;
    if (diff->diff & NET_TICDIFF_BUTTONS)
        dest->buttons = diff->cmd.buttons;
    if (diff->diff & NET_TICDIFF_CONSISTANCY)
        dest->consistancy = diff->cmd.consistancy;

    if (diff->diff & NET_TICDIFF_CHATCHAR)
        dest->chatchar = diff->cmd.chatchar;
    else
        dest->chatchar = 0;

    // Heretic/Hexen specific:

    if (diff->diff & NET_TICDIFF_RAVEN)
    {
        dest->lookfly = diff->cmd.lookfly;
        dest->arti    = diff->cmd.arti;
    }
    else
    {
        dest->arti = 0;
    }

    // Strife-specific:

    if (diff->diff & NET_TICDIFF_STRIFE)
    {
        dest->buttons2  = diff->cmd.buttons2;
        dest->inventory = diff->cmd.inventory;
    }
    else
    {
        dest->inventory = 0;
    }
}

//
// net_full_ticcmd_t
//

bool NET_ReadFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, bool lowres_turn)
{
    // Latency

    if (!NET_ReadSInt16(packet, &cmd->latency))
    {
        return false;
    }

    // Regenerate playeringame from the "header" bitfield

    unsigned int bitfield = 0;
    if (!NET_ReadInt8(packet, &bitfield))
    {
        return false;
    }

    for (int i = 0; i < NET_MAXPLAYERS; ++i)
    {
        cmd->playeringame[i] = (bitfield & (1 << i)) != 0;
    }

    // Read cmds

    for (int i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            if (!NET_ReadTiccmdDiff(packet, &cmd->cmds[i], lowres_turn))
            {
                return false;
            }
        }
    }

    return true;
}

void NET_WriteFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, bool lowres_turn)
{
    // Write the latency

    NET_WriteInt16(packet, static_cast<unsigned int>(cmd->latency));

    // Write "header" byte indicating which players are active
    // in this ticcmd

    unsigned int bitfield = 0;

    for (int i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            bitfield |= 1 << i;
        }
    }

    NET_WriteInt8(packet, bitfield);

    // Write player ticcmds

    for (int i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            NET_WriteTiccmdDiff(packet, &cmd->cmds[i], lowres_turn);
        }
    }
}

void NET_WriteWaitData(net_packet_t *packet, net_waitdata_t *data)
{
    NET_WriteInt8(packet, static_cast<unsigned int>(data->num_players));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->num_drones));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->ready_players));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->max_players));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->is_controller));
    NET_WriteInt8(packet, static_cast<unsigned int>(data->consoleplayer));

    for (int i = 0; i < data->num_players && i < NET_MAXPLAYERS; ++i)
    {
        NET_WriteString(packet, data->player_names[i]);
        NET_WriteString(packet, data->player_addrs[i]);
    }

    NET_WriteSHA1Sum(packet, data->wad_sha1sum);
    NET_WriteSHA1Sum(packet, data->deh_sha1sum);
    NET_WriteInt8(packet, static_cast<unsigned int>(data->is_freedoom));
}

bool NET_ReadWaitData(net_packet_t *packet, net_waitdata_t *data)
{
    if (!NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->num_players))
        || !NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->num_drones))
        || !NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->ready_players))
        || !NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->max_players))
        || !NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->is_controller))
        || !NET_ReadSInt8(packet, &data->consoleplayer))
    {
        return false;
    }

    for (int i = 0; i < data->num_players && i < NET_MAXPLAYERS; ++i)
    {
        char *s = NET_ReadString(packet);

        if (s == nullptr || strlen(s) >= MAXPLAYERNAME)
        {
            return false;
        }

        M_StringCopy(data->player_names[i], s, MAXPLAYERNAME);

        s = NET_ReadString(packet);

        if (s == nullptr || strlen(s) >= MAXPLAYERNAME)
        {
            return false;
        }

        M_StringCopy(data->player_addrs[i], s, MAXPLAYERNAME);
    }

    return NET_ReadSHA1Sum(packet, data->wad_sha1sum)
           && NET_ReadSHA1Sum(packet, data->deh_sha1sum)
           && NET_ReadInt8(packet, reinterpret_cast<unsigned int *>(&data->is_freedoom));
}

static bool NET_ReadBlob(net_packet_t *packet, uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        unsigned int b = 0;
        if (!NET_ReadInt8(packet, &b))
        {
            return false;
        }

        buf[i] = static_cast<uint8_t>(b);
    }

    return true;
}

static void NET_WriteBlob(net_packet_t *packet, uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        NET_WriteInt8(packet, buf[i]);
    }
}

bool NET_ReadSHA1Sum(net_packet_t *packet, sha1_digest_t digest)
{
    return NET_ReadBlob(packet, digest, sizeof(sha1_digest_t));
}

void NET_WriteSHA1Sum(net_packet_t *packet, sha1_digest_t digest)
{
    NET_WriteBlob(packet, digest, sizeof(sha1_digest_t));
}

bool NET_ReadPRNGSeed(net_packet_t *packet, prng_seed_t seed)
{
    return NET_ReadBlob(packet, seed, sizeof(prng_seed_t));
}

void NET_WritePRNGSeed(net_packet_t *packet, prng_seed_t seed)
{
    NET_WriteBlob(packet, seed, sizeof(prng_seed_t));
}

static net_protocol_t ParseProtocolName(const char *name)
{
    for (auto & protocol_name : protocol_names)
    {
        if (!strcmp(protocol_name.name, name))
        {
            return protocol_name.protocol;
        }
    }

    return NET_PROTOCOL_UNKNOWN;
}

// NET_ReadProtocol reads a single string-format protocol name from the given
// packet, returning NET_PROTOCOL_UNKNOWN if the string describes an unknown
// protocol.
net_protocol_t NET_ReadProtocol(net_packet_t *packet)
{
    const char *name = NET_ReadString(packet);
    if (name == nullptr)
    {
        return NET_PROTOCOL_UNKNOWN;
    }

    return ParseProtocolName(name);
}

// NET_WriteProtocol writes a single string-format protocol name to a packet.
void NET_WriteProtocol(net_packet_t *packet, net_protocol_t protocol)
{
    for (auto & protocol_name : protocol_names)
    {
        if (protocol_name.protocol == protocol)
        {
            NET_WriteString(packet, protocol_name.name);
            return;
        }
    }

    // If you add an entry to the net_protocol_t enum, a corresponding entry
    // must be added to the protocol_names list.
    I_Error("NET_WriteProtocol: protocol %d missing from protocol_names "
            "list; please add it.",
        protocol);
}

// NET_ReadProtocolList reads a list of string-format protocol names from
// the given packet, returning a single protocol number. The protocol that is
// returned is the last protocol in the list that is a supported protocol. If
// no recognized protocols are read, NET_PROTOCOL_UNKNOWN is returned.
net_protocol_t NET_ReadProtocolList(net_packet_t *packet)
{
    unsigned int num_protocols = 0;
    if (!NET_ReadInt8(packet, &num_protocols))
    {
        return NET_PROTOCOL_UNKNOWN;
    }

    net_protocol_t result = NET_PROTOCOL_UNKNOWN;

    for (unsigned int i = 0; i < num_protocols; ++i)
    {
        const char *   name = NET_ReadString(packet);
        if (name == nullptr)
        {
            return NET_PROTOCOL_UNKNOWN;
        }

        net_protocol_t p = ParseProtocolName(name);
        if (p != NET_PROTOCOL_UNKNOWN)
        {
            result = p;
        }
    }

    return result;
}

// NET_WriteProtocolList writes a list of string-format protocol names into
// the given packet, all the supported protocols in the net_protocol_t enum.
// This is slightly different to other functions in this file, in that there
// is nothing the caller can "choose" to write; the built-in list of all
// protocols is always sent.
void NET_WriteProtocolList(net_packet_t *packet)
{
    NET_WriteInt8(packet, NET_NUM_PROTOCOLS);

    for (int i = 0; i < NET_NUM_PROTOCOLS; ++i)
    {
        NET_WriteProtocol(packet, static_cast<net_protocol_t>(i));
    }
}
