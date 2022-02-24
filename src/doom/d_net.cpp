//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//	DOOM Network game communication and protocol,
//	all OS independend parts.
//

#include <cstddef>

#include <fmt/printf.h>

#include "d_main.hpp"
#include "m_argv.hpp"
#include "m_menu.hpp"
#include "m_misc.hpp"
#include "i_system.hpp"
#include "g_game.hpp"
#include "doomdef.hpp"
#include "doomstat.hpp"
#include "w_checksum.hpp"
#include "w_wad.hpp"
#include "deh_main.hpp"
#include "d_loop.hpp"

// Called when a player leaves the game

static void PlayerQuitGame(player_t * player) {
  static char exitmsg[80];
  auto        player_num = static_cast<unsigned int>(player - g_doomstat_globals->players);

  // Do this the same way as Vanilla Doom does, to allow dehacked
  // replacements of this message

  M_StringCopy(exitmsg, DEH_String("Player 1 left the game"), sizeof(exitmsg));

  exitmsg[7] = static_cast<char>(exitmsg[7] + player_num);

  g_doomstat_globals->playeringame[player_num]                           = false;
  g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = exitmsg;
  // [crispy] don't interpolate players who left the game
  player->mo->interp = false;

  // TODO: check if it is sensible to do this:

  if (g_doomstat_globals->demorecording) {
    G_CheckDemoStatus();
  }
}

static void RunTic(ticcmd_t * cmds, bool * ingame) {
  extern bool advancedemo;

  // Check for player quits.

  for (unsigned int i = 0; i < MAXPLAYERS; ++i) {
    if (!g_doomstat_globals->demoplayback && g_doomstat_globals->playeringame[i] && !ingame[i]) {
      PlayerQuitGame(&g_doomstat_globals->players[i]);
    }
  }

  g_doomstat_globals->netcmds = cmds;

  // check that there are players in the game.  if not, we cannot
  // run a tic.

  if (advancedemo)
    D_DoAdvanceDemo();

  G_Ticker();
}

static loop_interface_t doom_loop_interface = {
  D_ProcessEvents,
  G_BuildTiccmd,
  RunTic,
  M_Ticker
};

// Load game settings from the specified structure and
// set global variables.

static void LoadGameSettings(net_gamesettings_t * settings) {
  g_doomstat_globals->deathmatch    = settings->deathmatch;
  g_doomstat_globals->startepisode  = settings->episode;
  g_doomstat_globals->startmap      = settings->map;
  g_doomstat_globals->startskill    = static_cast<skill_t>(settings->skill);
  g_doomstat_globals->startloadgame = settings->loadgame;
  g_doomstat_globals->lowres_turn   = settings->lowres_turn;
  g_doomstat_globals->nomonsters    = settings->nomonsters;
  g_doomstat_globals->fastparm      = settings->fast_monsters;
  g_doomstat_globals->respawnparm   = settings->respawn_monsters;
  g_doomstat_globals->timelimit     = settings->timelimit;
  g_doomstat_globals->consoleplayer = settings->consoleplayer;

  if (g_doomstat_globals->lowres_turn) {
    fmt::printf("NOTE: Turning resolution is reduced; this is probably "
                "because there is a client recording a Vanilla demo.\n");
  }

  for (std::size_t i = 0; i < MAXPLAYERS; ++i) {
    g_doomstat_globals->playeringame[i] = static_cast<int>(i) < settings->num_players;
  }
}

// Save the game settings from global variables to the specified
// game settings structure.

static void SaveGameSettings(net_gamesettings_t * settings) {
  // Fill in game settings structure with appropriate parameters
  // for the new game

  settings->deathmatch       = g_doomstat_globals->deathmatch;
  settings->episode          = g_doomstat_globals->startepisode;
  settings->map              = g_doomstat_globals->startmap;
  settings->skill            = g_doomstat_globals->startskill;
  settings->loadgame         = g_doomstat_globals->startloadgame;
  settings->gameversion      = g_doomstat_globals->gameversion;
  settings->nomonsters       = g_doomstat_globals->nomonsters;
  settings->fast_monsters    = g_doomstat_globals->fastparm;
  settings->respawn_monsters = g_doomstat_globals->respawnparm;
  settings->timelimit        = g_doomstat_globals->timelimit;

  settings->lowres_turn = (M_ParmExists("-record")
                           && !M_ParmExists("-longtics"))
                          || M_ParmExists("-shorttics");
}

static void InitConnectData(net_connect_data_t * connect_data) {
  connect_data->max_players = MAXPLAYERS;
  connect_data->drone       = false;

  //!
  // @category net
  //
  // Run as the left screen in three screen mode.
  //

  if (M_CheckParm("-left") > 0) {
    g_doomstat_globals->viewangleoffset = ANG90;
    connect_data->drone                 = true;
  }

  //!
  // @category net
  //
  // Run as the right screen in three screen mode.
  //

  if (M_CheckParm("-right") > 0) {
    g_doomstat_globals->viewangleoffset = static_cast<int>(ANG270);
    connect_data->drone                 = true;
  }

  //
  // Connect data
  //

  // Game type fields:

  connect_data->gamemode    = g_doomstat_globals->gamemode;
  connect_data->gamemission = g_doomstat_globals->gamemission;

  //!
  // @category demo
  //
  // Play with low turning resolution to emulate demo recording.
  //

  bool shorttics = M_ParmExists("-shorttics");

  // Are we recording a demo? Possibly set lowres turn mode

  connect_data->lowres_turn = (M_ParmExists("-record")
                               && !M_ParmExists("-longtics"))
                              || shorttics;

  // Read checksums of our WAD directory and dehacked information

  W_Checksum(connect_data->wad_sha1sum);
  DEH_Checksum(connect_data->deh_sha1sum);

  // Are we playing with the Freedoom IWAD?

  connect_data->is_freedoom = W_CheckNumForName("FREEDOOM") >= 0;
}

void D_ConnectNetGame() {
  net_connect_data_t connect_data;

  InitConnectData(&connect_data);
  g_doomstat_globals->netgame = D_InitNetGame(&connect_data);

  //!
  // @category net
  //
  // Start the game playing as though in a netgame with a single
  // player.  This can also be used to play back single player netgame
  // demos.
  //

  if (M_CheckParm("-solo-net") > 0) {
    g_doomstat_globals->netgame = true;
  }
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
void D_CheckNetGame() {
  net_gamesettings_t settings;

  if (g_doomstat_globals->netgame) {
    g_doomstat_globals->autostart = true;
  }

  D_RegisterLoopCallbacks(&doom_loop_interface);

  SaveGameSettings(&settings);
  D_StartNetGame(&settings, nullptr);
  LoadGameSettings(&settings);

  fmt::printf("startskill %i  deathmatch: %i  startmap: %i  startepisode: %i\n",
              g_doomstat_globals->startskill,
              g_doomstat_globals->deathmatch,
              g_doomstat_globals->startmap,
              g_doomstat_globals->startepisode);

  fmt::printf("player %i of %i (%i nodes)\n",
              g_doomstat_globals->consoleplayer + 1,
              settings.num_players,
              settings.num_players);

  // Show players here; the server might have specified a time limit

  if (g_doomstat_globals->timelimit > 0 && g_doomstat_globals->deathmatch) {
    // Gross hack to work like Vanilla:

    if (g_doomstat_globals->timelimit == 20 && M_CheckParm("-avg")) {
      fmt::printf("Austin Virtual Gaming: Levels will end "
                  "after 20 minutes\n");
    } else {
      fmt::printf("Levels will end after %d minute", g_doomstat_globals->timelimit);
      if (g_doomstat_globals->timelimit > 1)
        fmt::printf("s");
      fmt::printf(".\n");
    }
  }
}
