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
//	BSP traversal, handling of LineSegs for rendering.
//

#include "doomdef.hpp"

#include "m_bbox.hpp"

#include "i_system.hpp"

#include "r_main.hpp"
#include "r_plane.hpp"
#include "r_things.hpp"

// State.
#include "doomstat.hpp"
#include "r_state.hpp"

//#include "r_local.hpp"

seg_t *    curline;
side_t *   sidedef;
line_t *   linedef;
sector_t * frontsector;
sector_t * backsector;

drawseg_t * drawsegs = nullptr;
drawseg_t * ds_p;
int         numdrawsegs = 0;

void R_StoreWallRange(int start,
                      int stop);

//
// R_ClearDrawSegs
//
void R_ClearDrawSegs() {
  ds_p = drawsegs;
}

//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
struct cliprange_t {
  int first;
  int last;
};

// We must expand MAXSEGS to the theoretical limit of the number of solidsegs
// that can be generated in a scene by the DOOM engine. This was determined by
// Lee Killough during BOOM development to be a function of the screensize.
// The simplest thing we can do, other than fix this bug, is to let the game
// render overage and then bomb out by detecting the overflow after the
// fact. -haleyjd
//#define MAXSEGS 32
#define MAXSEGS (MAXWIDTH / 2 + 1)

// newend is one past the last valid seg
cliprange_t * newend;
cliprange_t   solidsegs[MAXSEGS];

//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
//
void R_ClipSolidWallSegment(int first,
                            int last) {
  cliprange_t * next;
  cliprange_t * start;

  // Find the first range that touches the range
  //  (adjacent pixels are touching).
  start = solidsegs;
  while (start->last < first - 1)
    start++;

  if (first < start->first) {
    if (last < start->first - 1) {
      // Post is entirely visible (above start),
      //  so insert a new clippost.
      R_StoreWallRange(first, last);
      next = newend;
      newend++;

      while (next != start) {
        *next = *(next - 1);
        next--;
      }
      next->first = first;
      next->last  = last;
      return;
    }

    // There is a fragment above *start.
    R_StoreWallRange(first, start->first - 1);
    // Now adjust the clip size.
    start->first = first;
  }

  // Bottom contained in start?
  if (last <= start->last)
    return;

  next = start;
  while (last >= (next + 1)->first - 1) {
    // There is a fragment between two posts.
    R_StoreWallRange(next->last + 1, (next + 1)->first - 1);
    next++;

    if (last <= next->last) {
      // Bottom is contained in next.
      // Adjust the clip size.
      start->last = next->last;
      goto crunch;
    }
  }

  // There is a fragment after *next.
  R_StoreWallRange(next->last + 1, last);
  // Adjust the clip size.
  start->last = last;

  // Remove start+1 to next from the clip list,
  // because start now covers their area.
crunch:
  if (next == start) {
    // Post just extended past the bottom of one post.
    return;
  }

  while (next++ != newend) {
    // Remove a post.
    *++start = *next;
  }

  newend = start + 1;
}

//
// R_ClipPassWallSegment
// Clips the given range of columns,
//  but does not includes it in the clip list.
// Does handle windows,
//  e.g. LineDefs with upper and lower texture.
//
void R_ClipPassWallSegment(int first,
                           int last) {
  cliprange_t * start;

  // Find the first range that touches the range
  //  (adjacent pixels are touching).
  start = solidsegs;
  while (start->last < first - 1)
    start++;

  if (first < start->first) {
    if (last < start->first - 1) {
      // Post is entirely visible (above start).
      R_StoreWallRange(first, last);
      return;
    }

    // There is a fragment above *start.
    R_StoreWallRange(first, start->first - 1);
  }

  // Bottom contained in start?
  if (last <= start->last)
    return;

  while (last >= (start + 1)->first - 1) {
    // There is a fragment between two posts.
    R_StoreWallRange(start->last + 1, (start + 1)->first - 1);
    start++;

    if (last <= start->last)
      return;
  }

  // There is a fragment after *next.
  R_StoreWallRange(start->last + 1, last);
}

//
// R_ClearClipSegs
//
void R_ClearClipSegs() {
  solidsegs[0].first = -0x7fffffff;
  solidsegs[0].last  = -1;
  solidsegs[1].first = g_r_state_globals->viewwidth;
  solidsegs[1].last  = 0x7fffffff;
  newend             = solidsegs + 2;
}

// [AM] Interpolate the passed sector, if prudent.
void R_MaybeInterpolateSector(sector_t * sector) {
  if (crispy->uncapped &&
      // Only if we moved the sector last tic.
      sector->oldgametic == gametic - 1) {
    // Interpolate between current and last floor/ceiling position.
    if (sector->floorheight != sector->oldfloorheight)
      sector->interpfloorheight = sector->oldfloorheight + FixedMul(sector->floorheight - sector->oldfloorheight, fractionaltic);
    else
      sector->interpfloorheight = sector->floorheight;
    if (sector->ceilingheight != sector->oldceilingheight)
      sector->interpceilingheight = sector->oldceilingheight + FixedMul(sector->ceilingheight - sector->oldceilingheight, fractionaltic);
    else
      sector->interpceilingheight = sector->ceilingheight;
  } else {
    sector->interpfloorheight   = sector->floorheight;
    sector->interpceilingheight = sector->ceilingheight;
  }
}

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//
void R_AddLine(seg_t * line) {
  int     x1;
  int     x2;
  angle_t angle1;
  angle_t angle2;
  angle_t span;
  angle_t tspan;

  curline = line;

  // OPTIMIZE: quickly reject orthogonal back sides.
  // [crispy] remove slime trails
  angle1 = R_PointToAngleCrispy(line->v1->r_x, line->v1->r_y);
  angle2 = R_PointToAngleCrispy(line->v2->r_x, line->v2->r_y);

  // Clip to view edges.
  // OPTIMIZE: make constant out of 2*clipangle (FIELDOFVIEW).
  span = angle1 - angle2;

  // Back side? I.e. backface culling?
  if (span >= ANG180)
    return;

  // Global angle needed by segcalc.
  g_r_state_globals->rw_angle1 = static_cast<int>(angle1);
  angle1 -= g_r_state_globals->viewangle;
  angle2 -= g_r_state_globals->viewangle;

  tspan = angle1 + g_r_state_globals->clipangle;
  if (tspan > 2 * g_r_state_globals->clipangle) {
    tspan -= 2 * g_r_state_globals->clipangle;

    // Totally off the left edge?
    if (tspan >= span)
      return;

    angle1 = g_r_state_globals->clipangle;
  }
  tspan = g_r_state_globals->clipangle - angle2;
  if (tspan > 2 * g_r_state_globals->clipangle) {
    tspan -= 2 * g_r_state_globals->clipangle;

    // Totally off the left edge?
    if (tspan >= span)
      return;
    // Subtracting from 0u avoids compiler warnings
    angle2 = 0u - g_r_state_globals->clipangle;
  }

  // The seg is in the view range,
  // but not necessarily visible.
  angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
  angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
  x1     = g_r_state_globals->viewangletox[angle1];
  x2     = g_r_state_globals->viewangletox[angle2];

  // Does not cross a pixel?
  if (x1 == x2)
    return;

  backsector = line->backsector;

  // Single sided line?
  if (!backsector)
    goto clipsolid;

  // [AM] Interpolate sector movement before
  //      running clipping tests.  Frontsector
  //      should already be interpolated.
  R_MaybeInterpolateSector(backsector);

  // Closed door.
  if (backsector->interpceilingheight <= frontsector->interpfloorheight
      || backsector->interpfloorheight >= frontsector->interpceilingheight)
    goto clipsolid;

  // Window.
  if (backsector->interpceilingheight != frontsector->interpceilingheight
      || backsector->interpfloorheight != frontsector->interpfloorheight)
    goto clippass;

  // Reject empty lines used for triggers
  //  and special events.
  // Identical floor and ceiling on both sides,
  // identical light levels on both sides,
  // and no middle texture.
  if (backsector->ceilingpic == frontsector->ceilingpic
      && backsector->floorpic == frontsector->floorpic
      && backsector->lightlevel == frontsector->lightlevel
      && curline->sidedef->midtexture == 0) {
    return;
  }

clippass:
  R_ClipPassWallSegment(x1, x2 - 1);
  return;

clipsolid:
  R_ClipSolidWallSegment(x1, x2 - 1);
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
int checkcoord[12][4] = {
  {3,  0, 2, 1 },
  { 3, 0, 2, 0 },
  { 3, 1, 2, 0 },
  { 0},
  { 2, 0,  2,  1  },
  { 0, 0, 0,0},
  { 3, 1, 3,  0    },
  { 0},
  { 2, 0, 3,1},
  { 2, 1, 3,  1     },
  { 2,   1, 3,0}
};

bool R_CheckBBox(fixed_t * bspcoord) {
  int boxx;
  int boxy;
  int boxpos;

  fixed_t x1;
  fixed_t y1;
  fixed_t x2;
  fixed_t y2;

  angle_t angle1;
  angle_t angle2;
  angle_t span;
  angle_t tspan;

  cliprange_t * start;

  int sx1;
  int sx2;

  // Find the corners of the box
  // that define the edges from current viewpoint.
  if (g_r_state_globals->viewx <= bspcoord[BOXLEFT])
    boxx = 0;
  else if (g_r_state_globals->viewx < bspcoord[BOXRIGHT])
    boxx = 1;
  else
    boxx = 2;

  if (g_r_state_globals->viewy >= bspcoord[BOXTOP])
    boxy = 0;
  else if (g_r_state_globals->viewy > bspcoord[BOXBOTTOM])
    boxy = 1;
  else
    boxy = 2;

  boxpos = (boxy << 2) + boxx;
  if (boxpos == 5)
    return true;

  x1 = bspcoord[checkcoord[boxpos][0]];
  y1 = bspcoord[checkcoord[boxpos][1]];
  x2 = bspcoord[checkcoord[boxpos][2]];
  y2 = bspcoord[checkcoord[boxpos][3]];

  // check clip list for an open space
  angle1 = R_PointToAngleCrispy(x1, y1) - g_r_state_globals->viewangle;
  angle2 = R_PointToAngleCrispy(x2, y2) - g_r_state_globals->viewangle;

  span = angle1 - angle2;

  // Sitting on a line?
  if (span >= ANG180)
    return true;

  tspan = angle1 + g_r_state_globals->clipangle;

  if (tspan > 2 * g_r_state_globals->clipangle) {
    tspan -= 2 * g_r_state_globals->clipangle;

    // Totally off the left edge?
    if (tspan >= span)
      return false;

    angle1 = g_r_state_globals->clipangle;
  }
  tspan = g_r_state_globals->clipangle - angle2;
  if (tspan > 2 * g_r_state_globals->clipangle) {
    tspan -= 2 * g_r_state_globals->clipangle;

    // Totally off the left edge?
    if (tspan >= span)
      return false;
    // Subtracting from 0u avoids compiler warnings
    angle2 = 0u - g_r_state_globals->clipangle;
  }

  // Find the first clippost
  //  that touches the source post
  //  (adjacent pixels are touching).
  angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
  angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
  sx1    = g_r_state_globals->viewangletox[angle1];
  sx2    = g_r_state_globals->viewangletox[angle2];

  // Does not cross a pixel.
  if (sx1 == sx2)
    return false;
  sx2--;

  start = solidsegs;
  while (start->last < sx2)
    start++;

  if (sx1 >= start->first
      && sx2 <= start->last) {
    // The clippost contains the new span.
    return false;
  }

  return true;
}

//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
void R_Subsector(int num) {
  int           count;
  seg_t *       line;
  subsector_t * sub;

#ifdef RANGECHECK
  if (num >= g_r_state_globals->numsubsectors)
    I_Error("R_Subsector: ss %i with numss = %i",
            num,
            g_r_state_globals->numsubsectors);
#endif

  g_r_state_globals->sscount++;
  sub         = &g_r_state_globals->subsectors[num];
  frontsector = sub->sector;
  count       = sub->numlines;
  line        = &g_r_state_globals->segs[sub->firstline];

  // [AM] Interpolate sector movement.  Usually only needed
  //      when you're standing inside the sector.
  R_MaybeInterpolateSector(frontsector);

  if (frontsector->interpfloorheight < g_r_state_globals->viewz) {
    g_r_state_globals->floorplane = R_FindPlane(frontsector->interpfloorheight,
                                                // [crispy] add support for MBF sky tranfers
                                                frontsector->floorpic == g_doomstat_globals->skyflatnum && static_cast<unsigned int>(frontsector->sky) & PL_SKYFLAT ? frontsector->sky :
                                                                                                                                                                      frontsector->floorpic,
                                                frontsector->lightlevel);
  } else
    g_r_state_globals->floorplane = nullptr;

  if (frontsector->interpceilingheight > g_r_state_globals->viewz
      || frontsector->ceilingpic == g_doomstat_globals->skyflatnum) {
    g_r_state_globals->ceilingplane = R_FindPlane(frontsector->interpceilingheight,
                                                  // [crispy] add support for MBF sky tranfers
                                                  frontsector->ceilingpic == g_doomstat_globals->skyflatnum && static_cast<unsigned int>(frontsector->sky) & PL_SKYFLAT ? frontsector->sky :
                                                                                                                                                                          frontsector->ceilingpic,
                                                  frontsector->lightlevel);
  } else
    g_r_state_globals->ceilingplane = nullptr;

  R_AddSprites(frontsector);

  while (count--) {
    R_AddLine(line);
    line++;
  }

  // check for solidsegs overflow - extremely unsatisfactory!
  if (newend > &solidsegs[32] && false)
    I_Error("R_Subsector: solidsegs overflow (vanilla may crash here)\n");
}

//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
void R_RenderBSPNode(int bspnum) {
  node_t * bsp;
  int      side;

  // Found a subsector?
  if (static_cast<unsigned int>(bspnum) & NF_SUBSECTOR) {
    if (bspnum == -1)
      R_Subsector(0);
    else
      R_Subsector(static_cast<unsigned int>(bspnum) & (~NF_SUBSECTOR));
    return;
  }

  bsp = &g_r_state_globals->nodes[bspnum];

  // Decide which side the view point is on.
  side = R_PointOnSide(g_r_state_globals->viewx, g_r_state_globals->viewy, bsp);

  // Recursively divide front space.
  R_RenderBSPNode(bsp->children[side]);

  // Possibly divide back space.
  if (R_CheckBBox(bsp->bbox[side ^ 1]))
    R_RenderBSPNode(bsp->children[side ^ 1]);
}
