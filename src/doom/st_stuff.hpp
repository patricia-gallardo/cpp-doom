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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.hpp"
#include "d_event.hpp"
#include "m_cheat.hpp"
#include "i_video.hpp"

// Size of statusbar.
// Now sensitive for scaling.
constexpr auto ST_HEIGHT  = 32;
constexpr auto ST_WIDTH   = ORIGWIDTH;
constexpr auto ST_Y       = (ORIGHEIGHT - ST_HEIGHT);
constexpr auto CRISPY_HUD = 12;

// [crispy] Demo Timer widget
extern void ST_DrawDemoTimer(const int time);
extern int  defdemotics, deftotaldemotics;

//
// STATUS BAR
//

// Called by main loop.
bool ST_Responder(event_t *ev);

// Called by main loop.
void ST_Ticker();

// Called by main loop.
void ST_Drawer(bool fullscreen_param, bool refresh);

// Called when the console player is spawned on each level.
void ST_Start();

// Called by startup code.
void ST_Init();

// [crispy] forcefully initialize the status bar backing screen
extern void ST_refreshBackground(bool force);


// States for status bar code.
enum st_stateenum_t
{
    AutomapState,
    FirstPersonState
};


// States for the chat code.
enum st_chatstateenum_t
{
    StartChatState,
    WaitDestState [[maybe_unused]],
    GetChatState [[maybe_unused]]
};

struct st_stuff_t {
    // graphics are drawn to a backing screen and blitted to the real screen
    pixel_t *  st_backing_screen;
    cheatseq_t cheat_mus;
    cheatseq_t cheat_god;
    cheatseq_t cheat_ammo;
    cheatseq_t cheat_ammonokey;
    cheatseq_t cheat_noclip;
    cheatseq_t cheat_commercial_noclip;
    cheatseq_t cheat_powerup[8]; // [crispy] idbehold0
    cheatseq_t cheat_choppers;
    cheatseq_t cheat_clev;
    cheatseq_t cheat_mypos;
};

extern st_stuff_t *const g_st_stuff_globals;

#endif
