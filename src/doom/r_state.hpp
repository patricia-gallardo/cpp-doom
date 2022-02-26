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
//	Refresh/render internal state variables (global).
//

#pragma once

// Need data structure definitions.
#include "d_player.hpp"
#include "r_data.hpp"

//
// Refresh internal data structures,
//  for rendering.
//
struct r_state_t {

  // needed for texture pegging
  fixed_t * textureheight;

  // needed for pre rendering (fracs)
  fixed_t * spritewidth;

  fixed_t * spriteoffset;
  fixed_t * spritetopoffset;

  lighttable_t * colormaps;

  int viewwidth;
  int scaledviewwidth;
  int viewheight;

  // [crispy] lookup table for horizontal screen coordinates
  int   flipscreenwidth[MAXWIDTH];
  int * flipviewwidth;

  int firstflat;

  // for global animation
  int * flattranslation;
  int * texturetranslation;

  // Sprite....
  int firstspritelump;
  int lastspritelump;
  int numspritelumps;

  //
  // Lookup tables for map data.
  //

  // variables used to look up
  //  and range check thing_t sprites patches
  int           numsprites;
  spritedef_t * sprites;

  //
  // MAP related Lookup tables.
  // Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
  //

  int        numvertexes;
  vertex_t * vertexes;

  int     numsegs;
  seg_t * segs;

  int        numsectors;
  sector_t * sectors;

  int           numsubsectors;
  subsector_t * subsectors;

  int      numnodes;
  node_t * nodes;

  int      numlines;
  line_t * lines;

  int      numsides;
  side_t * sides;

  //
  // POV data.
  //
  fixed_t viewx;
  fixed_t viewy;
  fixed_t viewz;

  angle_t    viewangle;
  player_t * viewplayer;

  //
  // precalculated math tables
  //
  angle_t clipangle;

  // The viewangletox[viewangle + FINEANGLES/4] lookup
  // maps the visible view angles to screen X coordinates,
  // flattening the arc to a flat projection plane.
  // There will be many angles mapped to the same X.
  int viewangletox[FINEANGLES / 2];

  // The xtoviewangleangle[] table maps a screen pixel
  // to the lowest viewangle that maps back to x ranges
  // from clipangle to -clipangle.
  angle_t xtoviewangle[MAXWIDTH + 1];

  fixed_t rw_distance;
  angle_t rw_normalangle;

  // angle to line origin
  [[maybe_unused]] int rw_angle1;

  // Segs count?
  int sscount;

  visplane_t * floorplane;
  visplane_t * ceilingplane;
};

extern r_state_t * const g_r_state_globals;
