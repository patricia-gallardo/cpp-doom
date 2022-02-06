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
//	System specific interface stuff.
//


#ifndef __D_MAIN__
#define __D_MAIN__

#include "doomdef.hpp"




// Read events from all input devices

void D_ProcessEvents ();
	

//
// BASE LEVEL
//
void D_PageTicker ();
void D_PageDrawer ();
void D_AdvanceDemo ();
void D_DoAdvanceDemo ();
void D_StartTitle ();
void D_QuitGame (); // [STRIFE]

void D_IntroTick(); // [STRIFE]
 
//
// GLOBAL VARIABLES
//

extern  gameaction_t    gameaction;
extern  bool         isregistered;   // villsa [STRIFE]
extern  bool        isdemoversion;  // haleyjd [STRIFE]
extern  bool         stonecold;      // villsa [STRIFE]
extern  bool         workparm;       // villsa [STRIFE]

#endif

