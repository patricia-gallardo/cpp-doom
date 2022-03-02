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
//      Refresh/rendering module, shared data struct definitions.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-extensions"
#pragma once

#include <vector>

#pragma warning( push )
#pragma warning( disable : 4200 )

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.

// https://doomwiki.org/wiki/Picture_format
typedef PACKED_STRUCT(
    {
      short width; // bounding box size
      short height;
      short leftoffset;   // pixels to the left of origin
      short topoffset;    // pixels below the origin
      int   columnofs[]; // only [width] used
                          // the [0] is &columnofs[width]
    }) patch_t;

#pragma GCC diagnostic pop

#pragma warning( pop )

// posts are runs of non masked source pixels
typedef PACKED_STRUCT(
    {
      uint8_t topdelta; // -1 is the last post in a column
      uint8_t length;   // length data bytes follows
    }) post_t;

// column_t is a list of 0 or more post_t, (byte)-1 terminated
using column_t = post_t;

#pragma clang diagnostic pop