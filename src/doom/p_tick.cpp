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
//	Archiving: SaveGame I/O.
//	Thinker, Ticker.
//

#include "z_zone.hpp"
#include "p_local.hpp"
#include "s_musinfo.hpp" // [crispy] T_MAPMusic()

#include "doomstat.hpp"

int leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

//
// P_InitThinkers
//
void P_InitThinkers() {
  g_p_local_globals->thinkercap.prev = g_p_local_globals->thinkercap.next = &g_p_local_globals->thinkercap;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker(thinker_t * thinker) {
  g_p_local_globals->thinkercap.prev->next = thinker;
  thinker->next                            = &g_p_local_globals->thinkercap;
  thinker->prev                            = g_p_local_globals->thinkercap.prev;
  g_p_local_globals->thinkercap.prev       = thinker;
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker(thinker_t * thinker) {
  thinker->function = valid_hook(false);
}

//
// P_AllocateThinker
// Allocates memory and adds a new thinker at the end of the list.
//
void P_AllocateThinker(thinker_t *) {
}

//
// P_RunThinkers
//
void P_RunThinkers() {
  thinker_t *currentthinker, *nextthinker;

  currentthinker = g_p_local_globals->thinkercap.next;
  while (currentthinker != &g_p_local_globals->thinkercap) {
    action_hook invalid_hook = valid_hook(false);
    if (currentthinker->function == invalid_hook) {
      // time to remove it
      nextthinker                = currentthinker->next;
      currentthinker->next->prev = currentthinker->prev;
      currentthinker->prev->next = currentthinker->next;
      Z_Free(currentthinker);
    } else {
      call_thinker(currentthinker);
      nextthinker = currentthinker->next;
    }
    currentthinker = nextthinker;
  }

  // [crispy] support MUSINFO lump (dynamic music changing)
  T_MusInfo();
}

//
// P_Ticker
//

void P_Ticker() {
  int i;

  // run the tic
  if (g_doomstat_globals->paused)
    return;

  // pause if in menu and at least one tic has been run
  if (!g_doomstat_globals->netgame
      && g_doomstat_globals->menuactive
      && !g_doomstat_globals->demoplayback
      && g_doomstat_globals->players[g_doomstat_globals->consoleplayer].viewz != 1) {
    return;
  }

  for (i = 0; i < MAXPLAYERS; i++)
    if (g_doomstat_globals->playeringame[i])
      P_PlayerThink(&g_doomstat_globals->players[i]);

  P_RunThinkers();
  P_UpdateSpecials();
  P_RespawnSpecials();

  // for par times
  leveltime++;
}
