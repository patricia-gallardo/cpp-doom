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
// Network client code
//

#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "doomtype.hpp"
#include "d_ticcmd.hpp"
#include "sha1.hpp"
#include "net_defs.hpp"

bool NET_CL_Connect(net_addr_t * addr, net_connect_data_t * data);
void NET_CL_Disconnect();
void NET_CL_Run();
void NET_CL_Init();
void NET_CL_LaunchGame();
void NET_CL_StartGame(net_gamesettings_t * settings_param);
void NET_CL_SendTiccmd(ticcmd_t * ticcmd, int maketic);
bool NET_CL_GetSettings(net_gamesettings_t * _settings);
void NET_Init();

void NET_BindVariables();

struct net_client_globals_t {
  // true if the client code is in use
  bool net_client_connected {};
  // true if we have received waiting data from the server,
  // and the wait data that was received.
  bool           net_client_received_wait_data {};
  net_waitdata_t net_client_wait_data {};
  // Why did the server reject us?
  char * net_client_reject_reason {};
  // Waiting at the initial wait screen for the game to be launched?
  bool net_waiting_for_launch {};
  // Name that we send to the server
  char * net_player_name {};

  sha1_digest_t net_server_wad_sha1sum {};
  sha1_digest_t net_server_deh_sha1sum {};
  unsigned int  net_server_is_freedoom {};
  // Hash checksums of our wad directory and dehacked data.
  sha1_digest_t net_local_wad_sha1sum {};
  sha1_digest_t net_local_deh_sha1sum {};
  // Are we playing with the freedoom IWAD?
  unsigned int net_local_is_freedoom {};

  // Connected but not participating in the game (observer)
  bool drone {};
};

extern net_client_globals_t * const g_net_client_globals;

#endif /* #ifndef NET_CLIENT_H */
