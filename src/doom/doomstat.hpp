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
//   All the global variables that store the internal state.
//   Theoretically speaking, the internal state of the engine
//    should be found by looking at the variables collected
//    here, and every relevant module will have to include
//    this header file.
//   In practice, things are a bit messy.
//

#pragma once

// We need globally shared data structures,
//  for defining the global state variables.
#include "d_loop.hpp"
#include "doomdata.hpp"

// We need the playr data structure as well.
#include "d_player.hpp"

// Game mode/mission
#include "d_mode.hpp"

#include "net_defs.hpp"

#include "crispy.hpp"

// Maintain single and multi player starting spots.
constexpr auto MAX_DEATHMATCH_STARTS = 10;

// TODO
extern int leveltime; // tics in game play for par

struct doomstat_t {

  // ------------------------
  // Command line parameters.
  //
  bool nomonsters;  // checkparm of -nomonsters
  bool respawnparm; // checkparm of -respawn
  bool fastparm;    // checkparm of -fast

  bool devparm; // DEBUG: launched with -devparm

  // -----------------------------------------------------
  // Game Mode - identify IWAD as shareware, retail etc.
  //
  GameMode_t    gamemode;
  GameMission_t gamemission;
  GameVersion_t gameversion;
  GameVariant_t gamevariant;
  const char *  gamedescription;
  char *        nervewadfile;

  // Convenience macro.
  // 'gamemission' can be equal to pack_chex or pack_hacx, but these are
  // just modified versions of doom and doom2, and should be interpreted
  // as the same most of the time.

  // Set if homebrew PWAD stuff has been added.
  bool modifiedgame;

  // -------------------------------------------
  // Selected skill type, map etc.
  //

  // Defaults for menu, methinks.
  skill_t startskill;
  int     startepisode;
  int     startmap;

  // Savegame slot to load on startup.  This is the value provided to
  // the -loadgame option.  If this has not been provided, this is -1.

  int startloadgame;

  bool autostart;

  // Selected by user.
  skill_t gameskill;
  int     gameepisode;
  int     gamemap;

  // If non-zero, exit the level after this number of minutes
  int timelimit;

  // Nightmare mode flag, single player.
  bool respawnmonsters;

  // Netgame? Only true if >1 player.
  bool netgame; // only true if packets are broadcast

  // 0=Cooperative; 1=Deathmatch; 2=Altdeath
  int deathmatch; // only if started as net death

  // -------------------------
  // Internal parameters for sound rendering.
  // These have been taken from the DOS version,
  //  but are not (yet) supported with Linux
  //  (e.g. no sound volume adjustment with menu.

  // From m_menu.c:
  //  Sound FX volume has default, 0 - 15
  //  Music volume has default, 0 - 15
  // These are multiplied by 8.

  // Maximum volume of a sound effect.
  // Internal default is max out of 0-15.
  int sfxVolume;

  // Maximum volume of music.
  int musicVolume;

  // Current music/sfx card - index useless
  //  w/o a reference LUT in a sound module.
  // Ideally, this would use indices found
  //  in: /usr/include/linux/soundcard.h
  [[maybe_unused]] int snd_MusicDevice;
  [[maybe_unused]] int snd_SfxDevice;
  // Config file? Same disclaimer as above.
  [[maybe_unused]] int snd_DesiredMusicDevice;
  [[maybe_unused]] int snd_DesiredSfxDevice;

  // -------------------------
  // Status flags for refresh.
  //

  // Depending on view size - no status bar?
  // Note that there is no way to disable the
  //  status bar explicitely.
  [[maybe_unused]] bool statusbaractive;

  // in AM_map.c
  bool automapactive; // In AutoMap mode?
  bool menuactive;    // Menu overlayed?
  bool paused;        // Game Pause?

  bool viewactive;

  bool nodrawers; // for comparative timing purposes

  bool testcontrols; // Invoked by setup to test controls
  int  testcontrols_mousespeed;

  // This one is related to the 3-screen display mode.
  // ANG90 = left side, ANG270 = right
  int viewangleoffset;

  // Player taking events, and displaying.
  int consoleplayer; // player taking events and displaying
  int displayplayer; // view being displayed

  // -------------------------------------
  // Scores, rating.
  // Statistics on a given map, for intermission.
  // for intermission
  int totalkills;
  int totalitems;
  int totalsecret;
  int extrakills; // [crispy] count spawned monsters

  // Timer, for scores.
  [[maybe_unused]] int levelstarttic;   // gametic at level start
  int                  totalleveltimes; // [crispy] CPhipps - total time for all completed levels

  // --------------------------------------
  // DEMO playback/recording related stuff.
  // No demo, there is a human player in charge?
  // Disable save/end game?
  bool usergame; // ok to save / end game

  //?
  bool demoplayback;
  bool demorecording;

  // Round angleturn in ticcmds to the nearest 256.  This is used when
  // recording Vanilla demos in netgames.

  bool lowres_turn; // low resolution turning for longtics

  // Quit after playing a demo from cmdline.
  bool singledemo;

  //?
  gamestate_t gamestate;

  //-----------------------------
  // Internal parameters, fixed.
  // These are set by the engine, and not changed
  //  according to user inputs. Partly load from
  //  WAD, partly set at startup time.

  // Bookkeeping on players - state.
  player_t players[MAXPLAYERS];

  // Alive? Disconnected?
  bool playeringame[MAXPLAYERS];

  // Player spawn spots for deathmatch.
  mapthing_t   deathmatchstarts[MAX_DEATHMATCH_STARTS];
  mapthing_t * deathmatch_p;

  // Player spawn spots.
  mapthing_t playerstarts[MAXPLAYERS];
  bool       playerstartsingame[MAXPLAYERS];
  // Intermission stats.
  // Parameters for world map / intermission.
  wbstartstruct_t wminfo;

  //-----------------------------------------
  // Internal parameters, used for engine.
  //

  // File handling stuff.
  // Location where savegames are stored
  char * savegamedir;

  // if true, load all graphics at level load
  bool precache; // if true, load all graphics at start

  // wipegamestate can be set to -1 to force a wipe on the next draw
  gamestate_t wipegamestate;

  int mouseSensitivity;
  int mouseSensitivity_x2; // [crispy] mouse sensitivity menu
  int mouseSensitivity_y;  // [crispy] mouse sensitivity menu

  int bodyqueslot;

  // Needed to store the number of the dummy sky flat.
  // Used for rendering,
  //  as well as tracking projectiles etc.
  int skyflatnum; // sky mapping

  // Netgame stuff (buffers and pointers, i.e. indices).

  int rndindex;

  ticcmd_t * netcmds;
};

extern doomstat_t * const g_doomstat_globals;

constexpr GameMission_t logical_gamemission() {
  GameMission_t mission = g_doomstat_globals->gamemission;
  if (mission == pack_chex)
    return doom;
  if (mission == pack_hacx)
    return doom2;
  return mission;
}
