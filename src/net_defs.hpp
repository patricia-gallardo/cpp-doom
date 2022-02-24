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
//     Definitions for use in networking code.
//

#ifndef NET_DEFS_H
#define NET_DEFS_H

#include <cstdio>

#include "doomtype.hpp"
#include "d_ticcmd.hpp"
#include "sha1.hpp"

// Absolute maximum number of "nodes" in the game.  This is different to
// NET_MAXPLAYERS, as there may be observers that are not participating
// (eg. left/right monitors)

constexpr auto MAXNETNODES = 16;

// The maximum number of players, multiplayer/networking.
// This is the maximum supported by the networking code; individual games
// have their own values for MAXPLAYERS that can be smaller.

constexpr auto NET_MAXPLAYERS = 8;

// Maximum length of a player's name.

constexpr auto MAXPLAYERNAME = 30;

// Networking and tick handling related.

constexpr auto BACKUPTICS = 128;

using net_module_t  = struct _net_module_s;
using net_packet_t  = struct _net_packet_s;
using net_addr_t    = struct _net_addr_s;
using net_context_t = struct _net_context_s;

struct _net_packet_s {
  uint8_t *    data;
  size_t       len;
  size_t       alloced;
  unsigned int pos;
};

struct _net_module_s {
  // Initialize this module for use as a client

  bool (*InitClient)();

  // Initialize this module for use as a server

  bool (*InitServer)();

  // Send a packet

  void (*SendPacket)(net_addr_t * addr, net_packet_t * packet);

  // Check for new packets to receive
  //
  // Returns true if packet received

  bool (*RecvPacket)(net_addr_t ** addr, net_packet_t ** packet);

  // Converts an address to a string

  void (*AddrToString)(net_addr_t * addr, char * buffer, int buffer_len);

  // Free back an address when no longer in use

  void (*FreeAddress)(net_addr_t * addr);

  // Try to resolve a name to an address

  net_addr_t * (*ResolveAddress)(const char * addr);
};

// net_addr_t

struct _net_addr_s {
  net_module_t * module;
  int            refcount;
  void *         handle;
};

// Magic number sent when connecting to check this is a valid client
constexpr auto NET_MAGIC_NUMBER = 1454104972U;

// Old magic number used by Chocolate Doom versions before v3.0:
constexpr auto NET_OLD_MAGIC_NUMBER = 3436803284U;

// header field value indicating that the packet is a reliable packet

constexpr auto NET_RELIABLE_PACKET = (1 << 15);

// Supported protocols. If you're developing a fork of Chocolate
// Doom, you can add your own entry to this list while maintaining
// compatibility with Chocolate Doom servers. Higher-numbered enum values
// will be preferred when negotiating a protocol for the client and server
// to use, so the order matters.
// NOTE: The values in this enum do not have any special value outside of
// the program they're compiled in. What matters is the string representation.
enum net_protocol_t
{
  // Protocol introduced with Chocolate Doom v3.0. Each compatibility-
  // breaking change to the network protocol will produce a new protocol
  // number in this enum.
  NET_PROTOCOL_CHOCOLATE_DOOM_0,

  // Add your own protocol here; be sure to add a name for it to the list
  // in net_common.c too.

  NET_NUM_PROTOCOLS,
  NET_PROTOCOL_UNKNOWN,
};

// packet types

enum net_packet_type_t
{
  NET_PACKET_TYPE_SYN,
  NET_PACKET_TYPE_ACK [[maybe_unused]], // deprecated
  NET_PACKET_TYPE_REJECTED,
  NET_PACKET_TYPE_KEEPALIVE,
  NET_PACKET_TYPE_WAITING_DATA,
  NET_PACKET_TYPE_GAMESTART,
  NET_PACKET_TYPE_GAMEDATA,
  NET_PACKET_TYPE_GAMEDATA_ACK,
  NET_PACKET_TYPE_DISCONNECT,
  NET_PACKET_TYPE_DISCONNECT_ACK,
  NET_PACKET_TYPE_RELIABLE_ACK,
  NET_PACKET_TYPE_GAMEDATA_RESEND,
  NET_PACKET_TYPE_CONSOLE_MESSAGE,
  NET_PACKET_TYPE_QUERY,
  NET_PACKET_TYPE_QUERY_RESPONSE,
  NET_PACKET_TYPE_LAUNCH,
  NET_PACKET_TYPE_NAT_HOLE_PUNCH,
};

enum net_master_packet_type_t
{
  NET_MASTER_PACKET_TYPE_ADD,
  NET_MASTER_PACKET_TYPE_ADD_RESPONSE,
  NET_MASTER_PACKET_TYPE_QUERY,
  NET_MASTER_PACKET_TYPE_QUERY_RESPONSE,
  NET_MASTER_PACKET_TYPE_GET_METADATA [[maybe_unused]],
  NET_MASTER_PACKET_TYPE_GET_METADATA_RESPONSE [[maybe_unused]],
  NET_MASTER_PACKET_TYPE_SIGN_START,
  NET_MASTER_PACKET_TYPE_SIGN_START_RESPONSE,
  NET_MASTER_PACKET_TYPE_SIGN_END,
  NET_MASTER_PACKET_TYPE_SIGN_END_RESPONSE,
  NET_MASTER_PACKET_TYPE_NAT_HOLE_PUNCH,
  NET_MASTER_PACKET_TYPE_NAT_HOLE_PUNCH_ALL,
};

// Settings specified when the client connects to the server.

struct net_connect_data_t {
  int           gamemode;
  int           gamemission;
  int           lowres_turn;
  int           drone;
  int           max_players;
  int           is_freedoom;
  sha1_digest_t wad_sha1sum;
  sha1_digest_t deh_sha1sum;
  int           player_class;
};

// Game settings sent by client to server when initiating game start,
// and received from the server by clients when the game starts.

struct net_gamesettings_t {
  int ticdup;
  int extratics;
  int deathmatch;
  int episode;
  int nomonsters;
  int fast_monsters;
  int respawn_monsters;
  int map;
  int skill;
  int gameversion;
  int lowres_turn;
  int new_sync;
  int timelimit;
  int loadgame;
  int random; // [Strife only]

  // These fields are only used by the server when sending a game
  // start message:

  int num_players;
  int consoleplayer;

  // Hexen player classes:

  int player_classes[NET_MAXPLAYERS];
};

constexpr auto NET_TICDIFF_FORWARD     = (1 << 0);
constexpr auto NET_TICDIFF_SIDE        = (1 << 1);
constexpr auto NET_TICDIFF_TURN        = (1 << 2);
constexpr auto NET_TICDIFF_BUTTONS     = (1 << 3);
constexpr auto NET_TICDIFF_CONSISTANCY = (1 << 4);
constexpr auto NET_TICDIFF_CHATCHAR    = (1 << 5);
constexpr auto NET_TICDIFF_RAVEN       = (1 << 6);
constexpr auto NET_TICDIFF_STRIFE      = (1 << 7);

struct net_ticdiff_t {
  unsigned int diff;
  ticcmd_t     cmd;
};

// Complete set of ticcmds from all players

struct net_full_ticcmd_t {
  signed int    latency;
  unsigned int  seq;
  bool          playeringame[NET_MAXPLAYERS];
  net_ticdiff_t cmds[NET_MAXPLAYERS];
};

// Data sent in response to server queries

struct net_querydata_t {
  const char *   version;
  int            server_state;
  int            num_players;
  int            max_players;
  int            gamemode;
  int            gamemission;
  const char *   description;
  net_protocol_t protocol;
};

// Data sent by the server while waiting for the game to start.

struct net_waitdata_t {
  int           num_players;
  int           num_drones;
  int           ready_players;
  int           max_players;
  int           is_controller;
  int           consoleplayer;
  char          player_names[NET_MAXPLAYERS][MAXPLAYERNAME];
  char          player_addrs[NET_MAXPLAYERS][MAXPLAYERNAME];
  sha1_digest_t wad_sha1sum;
  sha1_digest_t deh_sha1sum;
  int           is_freedoom;
};

#endif /* #ifndef NET_DEFS_H */
