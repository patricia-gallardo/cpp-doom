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

#include "doomstat.hpp"


int leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//



// Both the head and tail of the thinker list.
thinker_t   thinkercap;


//
// P_InitThinkers
//
// [STRIFE] Verified unmodified
//
void P_InitThinkers ()
{
    thinkercap.prev = thinkercap.next  = &thinkercap;
}




//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
// [STRIFE] Verified unmodified
//
void P_AddThinker (thinker_t* thinker)
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
}



//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
// [STRIFE] Verified unmodified
//
void P_RemoveThinker (thinker_t* thinker)
{
  // FIXME: NOP.
  action_hook invalid = valid_hook(false);
  thinker->function = invalid;
}



//
// P_AllocateThinker
// Allocates memory and adds a new thinker at the end of the list.
//
void P_AllocateThinker (thinker_t*)
{
}

//
// P_RunThinkers
//
// [STRIFE] Verified unmodified
//
void P_RunThinkers ()
{
    thinker_t *currentthinker, *nextthinker;

    currentthinker = thinkercap.next;
    action_hook invalid_hook = valid_hook(false);
    while (currentthinker != &thinkercap)
    {
        if ( currentthinker->function == invalid_hook )
        {
            // time to remove it
            nextthinker = currentthinker->next;
            currentthinker->next->prev = currentthinker->prev;
            currentthinker->prev->next = currentthinker->next;
            Z_Free (currentthinker);
        }
        else
        {
            if (currentthinker->function.index() == thinker_param_action_hook) {
                const auto & callback = std::get<thinker_param_action>(currentthinker->function);
                callback(currentthinker);

            }
            nextthinker = currentthinker->next;
        }

        currentthinker = nextthinker;
    }
}

//
// P_Ticker
//
// [STRIFE] Menu pause behavior modified
//
void P_Ticker ()
{
    int     i;
    
    // run the tic
    if (paused)
        return;

    // pause if in menu and at least one tic has been run
    // haleyjd 09/08/10 [STRIFE]: menuactive -> menupause
    if (!netgame 
        && menupause 
        && !demoplayback 
        && players[consoleplayer].viewz != 1)
    {
        return;
    }
    

    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i])
            P_PlayerThink (&players[i]);

    P_RunThinkers ();
    P_UpdateSpecials ();
    P_RespawnSpecials ();

    // for par times
    leveltime++;
}
