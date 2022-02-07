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
//  Refresh module, data I/O, caching, retrieval of graphics
//  by name.
//


#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.hpp"
#include "r_state.hpp"

#define LOOKDIRMIN 110 // [crispy] -110, actually
#define LOOKDIRMAX 90
#define LOOKDIRS   (LOOKDIRMIN + 1 + LOOKDIRMAX) // [crispy] lookdir range: -110..0..90

// Retrieve column data for span blitting.
uint8_t *R_GetColumn(int tex,
        int         col,
        bool     opaque);


// I/O, setting up the stuff.
void R_InitData();
void R_PrecacheLevel();


// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_FlatNumForName(const char *name);


// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName(const char *name);
int R_CheckTextureNumForName(const char *name);

#endif
