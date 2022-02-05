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
//	Items: key cards, artifacts, weapon, ammunition.
//


#ifndef __D_ITEMS__
#define __D_ITEMS__

#include "doomdef.hpp"
#include "info.hpp"

// Weapon info: sprite frames, ammunition use.
using weaponinfo_t = struct
{
  ammotype_t ammo;
  statenum_t upstate;
  statenum_t downstate;
  statenum_t readystate;
  statenum_t atkstate;
  statenum_t flashstate;
};

extern weaponinfo_t weaponinfo[NUMWEAPONS];

#endif
