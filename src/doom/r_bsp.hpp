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
//	Refresh module, BSP traversal and handling.
//

#pragma once

extern seg_t *    curline;
extern side_t *   sidedef;
extern line_t *   linedef;
extern sector_t * frontsector;
extern sector_t * backsector;

extern int rw_x;
extern int rw_stopx;

extern bool segtextured;

// false if the back side is the same plane
extern bool markfloor;
extern bool markceiling;

extern bool skymap;

extern drawseg_t * drawsegs;
extern drawseg_t * ds_p;
extern int         numdrawsegs;

extern lighttable_t ** hscalelight;
extern lighttable_t ** vscalelight;
extern lighttable_t ** dscalelight;

using drawfunc_t = void (*)(int, int);

// BSP?
void R_ClearClipSegs();
void R_ClearDrawSegs();

void R_RenderBSPNode(int bspnum);
