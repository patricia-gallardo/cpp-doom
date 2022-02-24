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
//	Put all global tate variables here.
//

#include "doomstat.hpp"

static doomstat_t doomstat_s = {
  .nomonsters              = false,
  .respawnparm             = false,
  .fastparm                = false,
  .devparm                 = false,
  .gamemode                = indetermined,
  .gamemission             = doom,
  .gameversion             = exe_final2,
  .gamevariant             = vanilla,
  .gamedescription         = nullptr,
  .nervewadfile            = nullptr,
  .modifiedgame            = false,
  .startskill              = sk_noitems,
  .startepisode            = 0,
  .startmap                = 0,
  .startloadgame           = 0,
  .autostart               = false,
  .gameskill               = sk_noitems,
  .gameepisode             = 0,
  .gamemap                 = 0,
  .timelimit               = 0,
  .respawnmonsters         = false,
  .netgame                 = false,
  .deathmatch              = 0,
  .sfxVolume               = 8,
  .musicVolume             = 8,
  .snd_MusicDevice         = 0,     // Unused?
  .snd_SfxDevice           = 0,     // Unused?
  .snd_DesiredMusicDevice  = 0,     // Unused?
  .snd_DesiredSfxDevice    = 0,     // Unused?
  .statusbaractive         = false, // Unused?
  .automapactive           = false,
  .menuactive              = false,
  .paused                  = false,
  .viewactive              = false,
  .nodrawers               = false,
  .testcontrols            = false,
  .testcontrols_mousespeed = 0,
  .viewangleoffset         = 0,
  .consoleplayer           = 0,
  .displayplayer           = 0,
  .totalkills              = 0,
  .totalitems              = 0,
  .totalsecret             = 0,
  .extrakills              = 0,
  .levelstarttic           = 0,
  .totalleveltimes         = 0,
  .usergame                = false,
  .demoplayback            = false,
  .demorecording           = false,
  .lowres_turn             = false,
  .singledemo              = false,
  .gamestate               = GS_LEVEL,
  .players                 = {},
  .playeringame            = {},
  .deathmatchstarts        = {},
  .deathmatch_p            = nullptr,
  .playerstarts            = {},
  .playerstartsingame      = {},
  .wminfo                  = {},
  .savegamedir             = nullptr,
  .precache                = true,
  .wipegamestate           = GS_LEVEL,
  .mouseSensitivity        = 5,
  .mouseSensitivity_x2     = 5,
  .mouseSensitivity_y      = 5,
  .bodyqueslot             = 0,
  .skyflatnum              = 0,
  .rndindex                = 0,
  .netcmds                 = nullptr
};

doomstat_t *const g_doomstat_globals = &doomstat_s;
