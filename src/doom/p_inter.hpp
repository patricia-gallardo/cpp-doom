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
//
//


#ifndef __P_INTER__
#define __P_INTER__


#include "dstrings.hpp"

boolean
                                   P_GivePower(player_t *, int);

// [crispy] show weapon pickup messages in multiplayer games
static constexpr const char *const WeaponPickupMessages[NUMWEAPONS] = {
  NULL, // wp_fist
  NULL, // wp_pistol
  GOTSHOTGUN,
  GOTCHAINGUN,
  GOTLAUNCHER,
  GOTPLASMA,
  GOTBFG9000,
  GOTCHAINSAW,
  GOTSHOTGUN2,
};

#endif
