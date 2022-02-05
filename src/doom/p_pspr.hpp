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
//  Sprite animation.
//


#ifndef __P_PSPR__
#define __P_PSPR__

// Basic data types.
// Needs fixed point, and BAM angles.
#include "m_fixed.hpp"
#include "tables.hpp"


//
// Needs to include the precompiled
//  sprite animation tables.
// Header generated by multigen utility.
// This includes all the data for thing animation,
// i.e. the Thing Atrributes table
// and the Frame Sequence table.
#include "info.hpp"


//
// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
//
#define FF_FULLBRIGHT 0x8000 // flag in thing->frame
#define FF_FRAMEMASK  0x7fff


//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
using psprnum_t = enum
{
    ps_weapon,
    ps_flash,
    NUMPSPRITES

};

struct pspdef_t {
    state_t *state; // a NULL state means not active
    int      tics;
    fixed_t  sx;
    fixed_t  sy;

    // [crispy] squat down weapon sprite
    fixed_t dy;
    // [crispy] variable weapon sprite bob
    fixed_t sx2;
    fixed_t sy2;
};

#endif
