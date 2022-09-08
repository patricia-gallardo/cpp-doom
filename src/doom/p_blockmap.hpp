//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2017 Fabian Greffrath
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
//	[crispy] Create Blockmap
//

#pragma once

#include "p_mobj.hpp"

struct p_local_blockmap_t {
  // BLOCKMAP
  // Created from axis aligned bounding box
  // of the map, a rectangular array of
  // blocks of size ...
  // Used to speed up collision detection
  // by spatial subdivision in 2D.
  //
  // Blockmap size.
  int32_t * blockmaplump {}; // offsets in blockmap are from here // [crispy] BLOCKMAP limit
  int32_t * blockmap {};     // int for larger maps // [crispy] BLOCKMAP limit
  int       bmapwidth {};
  int       bmapheight {}; // size in mapblocks

  // origin of block map
  fixed_t   bmaporgx {};
  fixed_t   bmaporgy {};   // origin of block map
  mobj_t ** blocklinks {}; // for thing chains
};

extern p_local_blockmap_t * const g_p_local_blockmap;