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
//
// DESCRIPTION:
//   Functions and definitions relating to the game type and operational
//   mode.
//

#include "d_mode.hpp"

// Valid game mode/mission combinations, with the number of
// episodes/maps for each.

static struct
{
  GameMission_t mission;
  GameMode_t    mode;
  int           episode;
  int           map;
} valid_modes[] = {
  {pack_chex,    retail,     1, 5 },
  { doom,        shareware,  1, 9 },
  { doom,        registered, 3, 9 },
  { doom,        retail,     4, 9 },
  { doom2,       commercial, 1, 32},
  { pack_tnt,    commercial, 1, 32},
  { pack_plut,   commercial, 1, 32},
  { pack_hacx,   commercial, 1, 32},
  { pack_nerve,  commercial, 1, 9 },
  { pack_master, commercial, 1, 21},
};

// Check that a gamemode+gamemission received over the network is valid.

bool D_ValidGameMode(int mission, int mode) {
  for (auto & valid_mode : valid_modes) {
    if (valid_mode.mode == mode && valid_mode.mission == mission) {
      return true;
    }
  }

  return false;
}

bool D_ValidEpisodeMap(GameMission_t mission, GameMode_t mode, int episode, int map) {
  // Find the table entry for this mission/mode combination.

  for (auto & valid_mode : valid_modes) {
    if (mission == valid_mode.mission
        && mode == valid_mode.mode) {
      return episode >= 1 && episode <= valid_mode.episode
             && map >= 1 && map <= valid_mode.map;
    }
  }

  // Unknown mode/mission combination

  return false;
}

// Get the number of valid episodes for the specified mission/mode.

int D_GetNumEpisodes(GameMission_t mission, GameMode_t mode) {
  int episode = 1;

  while (D_ValidEpisodeMap(mission, mode, episode, 1)) {
    ++episode;
  }

  return episode - 1;
}

// Table of valid versions

static struct {
  GameMission_t mission;
  GameVersion_t version;
} valid_versions[] = {
  {doom,     exe_doom_1_2   },
  { doom,    exe_doom_1_666 },
  { doom,    exe_doom_1_7   },
  { doom,    exe_doom_1_8   },
  { doom,    exe_doom_1_9   },
  { doom,    exe_hacx       },
  { doom,    exe_ultimate   },
  { doom,    exe_final      },
  { doom,    exe_final2     },
  { doom,    exe_chex       },
};

bool D_ValidGameVersion(GameMission_t mission, GameVersion_t version) {
  // All Doom variants can use the Doom versions.

  if (mission == doom2 || mission == pack_plut || mission == pack_tnt
      || mission == pack_hacx || mission == pack_chex
      || mission == pack_nerve || mission == pack_master) {
    mission = doom;
  }

  for (auto & valid_version : valid_versions) {
    if (valid_version.mission == mission
        && valid_version.version == version) {
      return true;
    }
  }

  return false;
}

// Does this mission type use ExMy form, rather than MAPxy form?

bool D_IsEpisodeMap(GameMission_t mission) {
  switch (mission) {
  case doom:
  case pack_chex:
    return true;

  case none:
  case doom2:
  case pack_hacx:
  case pack_tnt:
  case pack_plut:
  case pack_nerve:
  case pack_master:
  default:
    return false;
  }
}

const char * D_GameMissionString(GameMission_t mission) {
  switch (mission) {
  case none:
  default:
    return "none";
  case doom:
    return "doom";
  case doom2:
    return "doom2";
  case pack_tnt:
    return "tnt";
  case pack_plut:
    return "plutonia";
  case pack_hacx:
    return "hacx";
  case pack_chex:
    return "chex";
  }
}

const char * D_GameModeString(GameMode_t mode) {
  switch (mode) {
  case shareware:
    return "shareware";
  case registered:
    return "registered";
  case commercial:
    return "commercial";
  case retail:
    return "retail";
  case indetermined:
  default:
    return "unknown";
  }
}
