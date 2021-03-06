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
//	Handle Sector base lighting effects.
//	Muzzle flash?
//

#include "m_random.hpp"
#include "p_local.hpp"
#include "z_zone.hpp"

// State.
#include "memory.hpp"
#include "r_state.hpp"

//
// FIRELIGHT FLICKER
//

//
// T_FireFlicker
//
void T_FireFlicker(fireflicker_t * flick) {
  if (--flick->count)
    return;

  int amount = (P_Random() & 3) * 16;

  if (flick->sector->lightlevel - amount < flick->minlight)
    flick->sector->lightlevel = static_cast<short>(flick->minlight);
  else
    flick->sector->lightlevel = static_cast<short>(flick->maxlight - amount);

  flick->count = 4;
}

//
// P_SpawnFireFlicker
//
void P_SpawnFireFlicker(sector_t * sector) {
  // Note that we are resetting sector attributes.
  // Nothing special about it during gameplay.
  sector->special = 0;

  fireflicker_t * flick = zmalloc<decltype(flick)>(sizeof(*flick), PU_LEVSPEC, 0);

  P_AddThinker(&flick->thinker);

  flick->thinker.function = T_FireFlicker;
  flick->sector           = sector;
  flick->maxlight         = sector->lightlevel;
  flick->minlight         = P_FindMinSurroundingLight(sector, sector->lightlevel) + 16;
  flick->count            = 4;
}

//
// BROKEN LIGHT FLASHING
//

//
// T_LightFlash
// Do flashing lights.
//
void T_LightFlash(lightflash_t * flash) {
  if (--flash->count)
    return;

  if (flash->sector->lightlevel == flash->maxlight) {
    flash->sector->lightlevel = static_cast<short>(flash->minlight);
    flash->count              = (P_Random() & flash->mintime) + 1;
  } else {
    flash->sector->lightlevel = static_cast<short>(flash->maxlight);
    flash->count              = (P_Random() & flash->maxtime) + 1;
  }
}

//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnLightFlash(sector_t * sector) {
  // nothing special about it during gameplay
  sector->special = 0;

  lightflash_t * flash = zmalloc<decltype(flash)>(sizeof(*flash), PU_LEVSPEC, 0);

  P_AddThinker(&flash->thinker);

  flash->thinker.function = T_LightFlash;
  flash->sector           = sector;
  flash->maxlight         = sector->lightlevel;

  flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
  flash->maxtime  = 64;
  flash->mintime  = 7;
  flash->count    = (P_Random() & flash->maxtime) + 1;
}

//
// STROBE LIGHT FLASHING
//

//
// T_StrobeFlash
//
void T_StrobeFlash(strobe_t * flash) {
  if (--flash->count)
    return;

  if (flash->sector->lightlevel == flash->minlight) {
    flash->sector->lightlevel = static_cast<short>(flash->maxlight);
    flash->count              = flash->brighttime;
  } else {
    flash->sector->lightlevel = static_cast<short>(flash->minlight);
    flash->count              = flash->darktime;
  }
}

//
// P_SpawnStrobeFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnStrobeFlash(sector_t * sector,
                        int        fastOrSlow,
                        int        inSync) {
  strobe_t * flash = zmalloc<decltype(flash)>(sizeof(*flash), PU_LEVSPEC, 0);

  P_AddThinker(&flash->thinker);

  flash->sector           = sector;
  flash->darktime         = fastOrSlow;
  flash->brighttime       = STROBEBRIGHT;
  flash->thinker.function = T_StrobeFlash;
  flash->maxlight         = sector->lightlevel;
  flash->minlight         = P_FindMinSurroundingLight(sector, sector->lightlevel);

  if (flash->minlight == flash->maxlight)
    flash->minlight = 0;

  // nothing special about it during gameplay
  sector->special = 0;

  if (!inSync)
    flash->count = (P_Random() & 7) + 1;
  else
    flash->count = 1;
}

//
// Start strobing lights (usually from a trigger)
//
void EV_StartLightStrobing(line_t * line) {
  sector_t * sec = nullptr;

  int secnum = -1;
  while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0) {
    sec = &g_r_state_globals->sectors[secnum];
    if (sec->specialdata)
      continue;

    P_SpawnStrobeFlash(sec, SLOWDARK, 0);
  }
}

//
// TURN LINE'S TAG LIGHTS OFF
//
void EV_TurnTagLightsOff(line_t * line) {
  int        min;
  sector_t * sector;
  sector_t * tsec;
  line_t *   templine;

  sector = g_r_state_globals->sectors;

  for (int j = 0; j < g_r_state_globals->numsectors; j++, sector++) {
    if (sector->tag == line->tag) {
      min = sector->lightlevel;
      for (int i = 0; i < sector->linecount; i++) {
        templine = sector->lines[i];
        tsec     = getNextSector(templine, sector);
        if (!tsec)
          continue;
        if (tsec->lightlevel < min)
          min = tsec->lightlevel;
      }
      sector->lightlevel = static_cast<short>(min);
    }
  }
}

//
// TURN LINE'S TAG LIGHTS ON
//
void EV_LightTurnOn(line_t * line,
                    int      bright) {
  sector_t * sector;
  sector_t * temp;
  line_t *   templine;

  sector = g_r_state_globals->sectors;

  for (int i = 0; i < g_r_state_globals->numsectors; i++, sector++) {
    if (sector->tag == line->tag) {
      // bright = 0 means to search
      // for highest light level
      // surrounding sector
      if (!bright) {
        for (int j = 0; j < sector->linecount; j++) {
          templine = sector->lines[j];
          temp     = getNextSector(templine, sector);

          if (!temp)
            continue;

          if (temp->lightlevel > bright)
            bright = temp->lightlevel;
        }
      }
      sector->lightlevel = static_cast<short>(bright);
    }
  }
}

//
// Spawn glowing light
//

void T_Glow(glow_t * g) {
  switch (g->direction) {
  case -1:
    // DOWN
    g->sector->lightlevel = static_cast<short>(g->sector->lightlevel - GLOWSPEED);
    if (g->sector->lightlevel <= g->minlight) {
      g->sector->lightlevel = static_cast<short>(g->sector->lightlevel + GLOWSPEED);
      g->direction          = 1;
    }
    break;

  case 1:
    // UP
    g->sector->lightlevel = static_cast<short>(g->sector->lightlevel + GLOWSPEED);
    if (g->sector->lightlevel >= g->maxlight) {
      g->sector->lightlevel = static_cast<short>(g->sector->lightlevel - GLOWSPEED);
      g->direction          = -1;
    }
    break;
  }
}

void P_SpawnGlowingLight(sector_t * sector) {
  glow_t * g = zmalloc<decltype(g)>(sizeof(*g), PU_LEVSPEC, 0);

  P_AddThinker(&g->thinker);

  g->sector           = sector;
  g->minlight         = P_FindMinSurroundingLight(sector, sector->lightlevel);
  g->maxlight         = sector->lightlevel;
  g->thinker.function = T_Glow;
  g->direction        = -1;

  sector->special = 0;
}
