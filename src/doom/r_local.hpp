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
//	Refresh (R_*) module, global header.
//	All the rendering/drawing stuff is here.
//

#ifndef __R_LOCAL__
#define __R_LOCAL__

// Binary Angles, sine/cosine/atan lookups.
#include "tables.hpp"

// Screen size related parameters.
#include "doomdef.hpp"

// Include the refresh/render data structs.
#include "r_data.hpp"

//
// Separate header file for each module.
//
#include "r_bsp.hpp"
#include "r_data.hpp"
#include "r_draw.hpp"
#include "r_main.hpp"
#include "r_plane.hpp"
#include "r_segs.hpp"
#include "r_things.hpp"

#endif // __R_LOCAL__
