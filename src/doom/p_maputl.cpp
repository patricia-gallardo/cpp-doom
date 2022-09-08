//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2005, 2006 Andrey Budko
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
//	Movement/collision utility functions,
//	as used by function in p_map.c.
//	BLOCKMAP Iterator functions,
//	and some PIT_* functions to use for iteration.
//

#include <cstdlib>

#include <fmt/printf.h>

#include "i_system.hpp" // [crispy] I_Realloc()
#include "m_bbox.hpp"
#include "m_mapmath.hpp"

#include "doomdef.hpp"
#include "doomstat.hpp"
#include "p_local.hpp"
#include "p_blockmap.hpp"

// State.
#include "r_state.hpp"

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
int P_BoxOnLineSide(const bounding_box_t& tmbox,
                    line_t *  ld) {
  int p1 = 0;
  int p2 = 0;

  switch (ld->slopetype) {
  case ST_HORIZONTAL:
    p1 = tmbox.get(box_e::top) > ld->v1->y;
    p2 = tmbox.get(box_e::bottom) > ld->v1->y;
    if (ld->dx < 0) {
      p1 ^= 1;
      p2 ^= 1;
    }
    break;

  case ST_VERTICAL:
    p1 = tmbox.get(box_e::right) < ld->v1->x;
    p2 = tmbox.get(box_e::left) < ld->v1->x;
    if (ld->dy < 0) {
      p1 ^= 1;
      p2 ^= 1;
    }
    break;

  case ST_POSITIVE:
    p1 = map::point_on_line_side(tmbox.get(box_e::left), tmbox.get(box_e::top), ld->dx, ld->dy, ld->v1->x, ld->v1->y);
    p2 = map::point_on_line_side(tmbox.get(box_e::right), tmbox.get(box_e::bottom), ld->dx, ld->dy, ld->v1->x, ld->v1->y);
    break;

  case ST_NEGATIVE:
    p1 = map::point_on_line_side(tmbox.get(box_e::right), tmbox.get(box_e::top), ld->dx, ld->dy, ld->v1->x, ld->v1->y);
    p2 = map::point_on_line_side(tmbox.get(box_e::left), tmbox.get(box_e::bottom), ld->dx, ld->dy, ld->v1->x, ld->v1->y);
    break;
  }

  if (p1 == p2)
    return p1;
  return -1;
}

//
// P_PointOnDivlineSide
// Returns 0 or 1.
//
int P_PointOnDivlineSide(fixed_t     x,
                         fixed_t     y,
                         divline_t * line) {
  fixed_t dx;
  fixed_t dy;
  fixed_t left;
  fixed_t right;

  if (!line->dx) {
    if (x <= line->x)
      return line->dy > 0;

    return line->dy < 0;
  }
  if (!line->dy) {
    if (y <= line->y)
      return line->dx < 0;

    return line->dx > 0;
  }

  dx = (x - line->x);
  dy = (y - line->y);

  // try to quickly decide by looking at sign bits
  if ((static_cast<unsigned int>(line->dy ^ line->dx ^ dx ^ dy)) & 0x80000000) {
    if ((static_cast<unsigned int>(line->dy ^ dx)) & 0x80000000)
      return 1; // (left is negative)
    return 0;
  }

  left  = FixedMul(line->dy >> 8, dx >> 8);
  right = FixedMul(dy >> 8, line->dx >> 8);

  if (right < left)
    return 0; // front side
  return 1;   // back side
}

//
// P_MakeDivline
//
void P_MakeDivline(line_t *    li,
                   divline_t * dl) {
  dl->x  = li->v1->x;
  dl->y  = li->v1->y;
  dl->dx = li->dx;
  dl->dy = li->dy;
}

//
// P_InterceptVector
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings
// and addlines traversers.
//
fixed_t
    P_InterceptVector(divline_t * v2,
                      divline_t * v1) {
#if 1
  fixed_t frac;
  fixed_t num;
  fixed_t den;

  den = FixedMul(v1->dy >> 8, v2->dx) - FixedMul(v1->dx >> 8, v2->dy);

  if (den == 0)
    return 0;
  //	I_Error ("P_InterceptVector: parallel");

  num =
      FixedMul((v1->x - v2->x) >> 8, v1->dy)
      + FixedMul((v2->y - v1->y) >> 8, v1->dx);

  frac = FixedDiv(num, den);

  return frac;
#else // UNUSED, float debug.
  float frac;
  float num;
  float den;
  float v1x;
  float v1y;
  float v1dx;
  float v1dy;
  float v2x;
  float v2y;
  float v2dx;
  float v2dy;

  v1x  = (float)v1->x / FRACUNIT;
  v1y  = (float)v1->y / FRACUNIT;
  v1dx = (float)v1->dx / FRACUNIT;
  v1dy = (float)v1->dy / FRACUNIT;
  v2x  = (float)v2->x / FRACUNIT;
  v2y  = (float)v2->y / FRACUNIT;
  v2dx = (float)v2->dx / FRACUNIT;
  v2dy = (float)v2->dy / FRACUNIT;

  den = v1dy * v2dx - v1dx * v2dy;

  if (den == 0)
    return 0; // parallel

  num  = (v1x - v2x) * v1dy + (v2y - v1y) * v1dx;
  frac = num / den;

  return frac * FRACUNIT;
#endif
}

void P_LineOpening(line_t * linedef_param) {
  if (linedef_param->sidenum[1] == NO_INDEX) // [crispy] extended nodes
  {
    // single sided line
    g_p_local_globals->openrange = 0;
    return;
  }

  sector_t * front = linedef_param->frontsector;
  sector_t * back  = linedef_param->backsector;

  if (front->ceilingheight < back->ceilingheight)
    g_p_local_globals->opentop = front->ceilingheight;
  else
    g_p_local_globals->opentop = back->ceilingheight;

  if (front->floorheight > back->floorheight) {
    g_p_local_globals->openbottom = front->floorheight;
    g_p_local_globals->lowfloor   = back->floorheight;
  } else {
    g_p_local_globals->openbottom = back->floorheight;
    g_p_local_globals->lowfloor   = front->floorheight;
  }

  g_p_local_globals->openrange = g_p_local_globals->opentop - g_p_local_globals->openbottom;
}

//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//
void P_UnsetThingPosition(mobj_t * thing) {
  int blockx;
  int blocky;

  if (!(thing->flags & MF_NOSECTOR)) {
    // inert things don't need to be in blockmap?
    // unlink from subsector
    if (thing->snext)
      thing->snext->sprev = thing->sprev;

    if (thing->sprev)
      thing->sprev->snext = thing->snext;
    else
      thing->subsector->sector->thinglist = thing->snext;
  }

  if (!(thing->flags & MF_NOBLOCKMAP)) {
    // inert things don't need to be in blockmap
    // unlink from block map
    if (thing->bnext)
      thing->bnext->bprev = thing->bprev;

    if (thing->bprev)
      thing->bprev->bnext = thing->bnext;
    else {
      blockx = (thing->x - g_p_local_blockmap->bmaporgx) >> MAPBLOCKSHIFT;
      blocky = (thing->y - g_p_local_blockmap->bmaporgy) >> MAPBLOCKSHIFT;

      if (blockx >= 0 && blockx < g_p_local_blockmap->bmapwidth
          && blocky >= 0 && blocky < g_p_local_blockmap->bmapheight) {
        g_p_local_blockmap->blocklinks[blocky * g_p_local_blockmap->bmapwidth + blockx] = thing->bnext;
      }
    }
  }
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on it's x y.
// Sets thing->subsector properly
//
void P_SetThingPosition(mobj_t * thing) {
  subsector_t * ss;
  sector_t *    sec;
  int           blockx;
  int           blocky;
  mobj_t **     link;

  // link into subsector
  ss               = R_PointInSubsector(thing->x, thing->y);
  thing->subsector = ss;

  if (!(thing->flags & MF_NOSECTOR)) {
    // invisible things don't go into the sector links
    sec = ss->sector;

    thing->sprev = nullptr;
    thing->snext = sec->thinglist;

    if (sec->thinglist)
      sec->thinglist->sprev = thing;

    sec->thinglist = thing;
  }

  // link into blockmap
  if (!(thing->flags & MF_NOBLOCKMAP)) {
    // inert things don't need to be in blockmap
    blockx = (thing->x - g_p_local_blockmap->bmaporgx) >> MAPBLOCKSHIFT;
    blocky = (thing->y - g_p_local_blockmap->bmaporgy) >> MAPBLOCKSHIFT;

    if (blockx >= 0
        && blockx < g_p_local_blockmap->bmapwidth
        && blocky >= 0
        && blocky < g_p_local_blockmap->bmapheight) {
      link         = &g_p_local_blockmap->blocklinks[blocky * g_p_local_blockmap->bmapwidth + blockx];
      thing->bprev = nullptr;
      thing->bnext = *link;
      if (*link)
        (*link)->bprev = thing;

      *link = thing;
    } else {
      // thing is off the map
      thing->bnext = thing->bprev = nullptr;
    }
  }
}

//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock,
// call the passed PIT_* function.
// If the function returns false,
// exit with false without checking anything else.
//

//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines
// that are marked in multiple mapblocks,
// so increment validcount before the first call
// to P_BlockLinesIterator, then make one or more calls
// to it.
//
bool P_BlockLinesIterator(int x,
                          int y,
                          bool (*func)(line_t *)) {
  int       offset;
  int32_t * list; // [crispy] BLOCKMAP limit
  line_t *  ld;

  if (x < 0
      || y < 0
      || x >= g_p_local_blockmap->bmapwidth
      || y >= g_p_local_blockmap->bmapheight) {
    return true;
  }

  offset = y * g_p_local_blockmap->bmapwidth + x;

  offset = *(g_p_local_blockmap->blockmap + offset);

  for (list = g_p_local_blockmap->blockmaplump + offset; *list != -1; list++) {
    ld = &g_r_state_globals->lines[*list];

    if (ld->validcount == validcount)
      continue; // line has already been checked

    ld->validcount = validcount;

    if (!func(ld))
      return false;
  }
  return true; // everything was checked
}

//
// P_BlockThingsIterator
//
bool P_BlockThingsIterator(int x,
                           int y,
                           bool (*func)(mobj_t *)) {
  mobj_t * mobj;

  if (x < 0
      || y < 0
      || x >= g_p_local_blockmap->bmapwidth
      || y >= g_p_local_blockmap->bmapheight) {
    return true;
  }

  for (mobj = g_p_local_blockmap->blocklinks[y * g_p_local_blockmap->bmapwidth + x];
       mobj;
       mobj = mobj->bnext) {
    if (!func(mobj))
      return false;
  }
  return true;
}

//
// INTERCEPT ROUTINES
//
static intercept_t * intercepts; // [crispy] remove INTERCEPTS limit

// [crispy] remove INTERCEPTS limit
// taken from PrBoom+/src/p_maputl.c:422-433
static void check_intercept() {
  static size_t num_intercepts;
  const size_t  offset = static_cast<size_t>(g_p_local_globals->intercept_p - intercepts);

  if (offset >= num_intercepts) {
    num_intercepts                 = num_intercepts ? num_intercepts * 2 : MAXINTERCEPTS_ORIGINAL;
    intercepts                     = static_cast<decltype(intercepts)>(I_Realloc(intercepts, sizeof(*intercepts) * num_intercepts));
    g_p_local_globals->intercept_p = intercepts + offset;
  }
}

bool                 earlyout;
[[maybe_unused]] int ptflags;

static void InterceptsOverrun(int num_intercepts, intercept_t * intercept);

// [crispy] show mapthing number in INTERCEPTS overflow warnings
extern mobj_t * shootthing;

//
// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
// Returns true if earlyout and a solid line hit.
//
bool PIT_AddLineIntercepts(line_t * ld) {
  int       s1;
  int       s2;
  fixed_t   frac;
  divline_t dl;

  // avoid precision problems with two routines
  if (g_p_local_globals->trace.dx > FRACUNIT * 16
      || g_p_local_globals->trace.dy > FRACUNIT * 16
      || g_p_local_globals->trace.dx < -FRACUNIT * 16
      || g_p_local_globals->trace.dy < -FRACUNIT * 16) {
    s1 = P_PointOnDivlineSide(ld->v1->x, ld->v1->y, &g_p_local_globals->trace);
    s2 = P_PointOnDivlineSide(ld->v2->x, ld->v2->y, &g_p_local_globals->trace);
  } else {
    s1 = map::point_on_line_side(g_p_local_globals->trace.x, g_p_local_globals->trace.y, ld->dx, ld->dy, ld->v1->x, ld->v1->y);
    s2 = map::point_on_line_side(g_p_local_globals->trace.x + g_p_local_globals->trace.dx, g_p_local_globals->trace.y + g_p_local_globals->trace.dy, ld->dx, ld->dy, ld->v1->x, ld->v1->y);
  }

  if (s1 == s2)
    return true; // line isn't crossed

  // hit the line
  P_MakeDivline(ld, &dl);
  frac = P_InterceptVector(&g_p_local_globals->trace, &dl);

  if (frac < 0)
    return true; // behind source

  // try to early out the check
  if (earlyout
      && frac < FRACUNIT
      && !ld->backsector) {
    return false; // stop checking
  }

  check_intercept(); // [crispy] remove INTERCEPTS limit
  g_p_local_globals->intercept_p->frac    = frac;
  g_p_local_globals->intercept_p->isaline = true;
  g_p_local_globals->intercept_p->d.line  = ld;
  InterceptsOverrun(static_cast<int>(g_p_local_globals->intercept_p - intercepts), g_p_local_globals->intercept_p);
  // [crispy] intercepts overflow guard
  if (g_p_local_globals->intercept_p - intercepts == MAXINTERCEPTS_ORIGINAL + 1) {
    if (crispy->crosshair & CROSSHAIR_INTERCEPT)
      return false;
    else
      // [crispy] print a warning
      fmt::fprintf(stderr, "PIT_AddLineIntercepts: Triggered INTERCEPTS overflow!\n");
  }
  g_p_local_globals->intercept_p++;

  return true; // continue
}

//
// PIT_AddThingIntercepts
//
bool PIT_AddThingIntercepts(mobj_t * thing) {
  fixed_t x1;
  fixed_t y1;
  fixed_t x2;
  fixed_t y2;

  int s1;
  int s2;

  bool tracepositive;

  divline_t dl;

  fixed_t frac;

  tracepositive = (g_p_local_globals->trace.dx ^ g_p_local_globals->trace.dy) > 0;

  // check a corner to corner crossection for hit
  if (tracepositive) {
    x1 = thing->x - thing->radius;
    y1 = thing->y + thing->radius;

    x2 = thing->x + thing->radius;
    y2 = thing->y - thing->radius;
  } else {
    x1 = thing->x - thing->radius;
    y1 = thing->y - thing->radius;

    x2 = thing->x + thing->radius;
    y2 = thing->y + thing->radius;
  }

  s1 = P_PointOnDivlineSide(x1, y1, &g_p_local_globals->trace);
  s2 = P_PointOnDivlineSide(x2, y2, &g_p_local_globals->trace);

  if (s1 == s2)
    return true; // line isn't crossed

  dl.x  = x1;
  dl.y  = y1;
  dl.dx = x2 - x1;
  dl.dy = y2 - y1;

  frac = P_InterceptVector(&g_p_local_globals->trace, &dl);

  if (frac < 0)
    return true; // behind source

  check_intercept(); // [crispy] remove INTERCEPTS limit
  g_p_local_globals->intercept_p->frac    = frac;
  g_p_local_globals->intercept_p->isaline = false;
  g_p_local_globals->intercept_p->d.thing = thing;
  InterceptsOverrun(static_cast<int>(g_p_local_globals->intercept_p - intercepts), g_p_local_globals->intercept_p);
  // [crispy] intercepts overflow guard
  if (g_p_local_globals->intercept_p - intercepts == MAXINTERCEPTS_ORIGINAL + 1) {
    if (crispy->crosshair & CROSSHAIR_INTERCEPT)
      return false;
    else
      // [crispy] print a warning
      fmt::fprintf(stderr, "PIT_AddThingIntercepts: Triggered INTERCEPTS overflow!\n");
  }
  g_p_local_globals->intercept_p++;

  return true; // keep going
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//
bool P_TraverseIntercepts(traverser_t func, fixed_t maxfrac) {
  int count = static_cast<int>(g_p_local_globals->intercept_p - intercepts);

  intercept_t * in = 0; // shut up compiler warning

  while (count--) {
    fixed_t dist = std::numeric_limits<int32_t>::max();
    for (intercept_t * scan = intercepts; scan < g_p_local_globals->intercept_p; scan++) {
      if (scan->frac < dist) {
        dist = scan->frac;
        in   = scan;
      }
    }

    if (dist > maxfrac)
      return true; // checked everything in range

#if 0 // UNUSED
    {
	// don't check these yet, there may be others inserted
	in = scan = intercepts;
	for ( scan = intercepts ; scan<intercept_p ; scan++)
	    if (scan->frac > maxfrac)
		*in++ = *scan;
	intercept_p = in;
	return false;
    }
#endif

    if (!func(in))
      return false; // don't bother going farther

    in->frac = std::numeric_limits<int32_t>::max();
  }

  return true; // everything was traversed
}

extern fixed_t bulletslope;

// Intercepts Overrun emulation, from PrBoom-plus.
// Thanks to Andrey Budko (entryway) for researching this and his
// implementation of Intercepts Overrun emulation in PrBoom-plus
// which this is based on.

struct intercepts_overrun_t {
  int    len;
  void * addr;
  bool   int16_array;
};

// Intercepts memory table.  This is where various variables are located
// in memory in Vanilla Doom.  When the intercepts table overflows, we
// need to write to them.
//
// Almost all of the values to overwrite are 32-bit integers, except for
// playerstarts, which is effectively an array of 16-bit integers and
// must be treated differently.

static intercepts_overrun_t intercepts_overrun[] = {
  {4,    nullptr,                           false                     },
  { 4,   nullptr,                           /* &earlyout, */ false    },
  { 4,   nullptr,                           /* &intercept_p, */ false },
  { 4,   &g_p_local_globals->lowfloor,      false                     },
  { 4,   &g_p_local_globals->openbottom,    false                     },
  { 4,   &g_p_local_globals->opentop,       false                     },
  { 4,   &g_p_local_globals->openrange,     false                     },
  { 4,   nullptr,                           false                     },
  { 120, nullptr,                           /* &activeplats, */ false },
  { 8,   nullptr,                           false                     },
  { 4,   &bulletslope,                      false                     },
  { 4,   nullptr,                           /* &swingx, */ false      },
  { 4,   nullptr,                           /* &swingy, */ false      },
  { 4,   nullptr,                           false                     },
  { 40,  &g_doomstat_globals->playerstarts, true                      },
  { 4,   nullptr,                           /* &blocklinks, */ false  },
  { 4,   &g_p_local_blockmap->bmapwidth,    false },
  { 4,   nullptr,                           /* &blockmap, */ false    },
  { 4,   &g_p_local_blockmap->bmaporgx,     false },
  { 4,   &g_p_local_blockmap->bmaporgy,     false },
  { 4,   nullptr,                           /* &blockmaplump, */ false},
  { 4,   &g_p_local_blockmap->bmapheight,   false },
  { 0,   nullptr,                           false                     },
};

// Overwrite a specific memory location with a value.

static void InterceptsMemoryOverrun(int location, int value) {
  int    i, offset;
  int    index;
  void * addr;

  i      = 0;
  offset = 0;

  // Search down the array until we find the right entry

  while (intercepts_overrun[i].len != 0) {
    if (offset + intercepts_overrun[i].len > location) {
      addr = intercepts_overrun[i].addr;

      // Write the value to the memory location.
      // 16-bit and 32-bit values are written differently.

      if (addr != nullptr) {
        if (intercepts_overrun[i].int16_array) {
          index                 = (location - offset) / 2;
          auto * addr_short     = reinterpret_cast<short *>(addr);
          addr_short[index]     = static_cast<short>(value & 0xffff);
          addr_short[index + 1] = static_cast<short>((value >> 16) & 0xffff);
        } else {
          index           = (location - offset) / 4;
          int * addr_int  = reinterpret_cast<int *>(addr);
          addr_int[index] = value;
        }
      }

      break;
    }

    offset += intercepts_overrun[i].len;
    ++i;
  }
}

// Emulate overruns of the intercepts[] array.

static void InterceptsOverrun(int num_intercepts, intercept_t * intercept) {
  if (num_intercepts <= MAXINTERCEPTS_ORIGINAL) {
    // No overrun

    return;
  }

  int location = (num_intercepts - MAXINTERCEPTS_ORIGINAL - 1) * 12;

  // Overwrite memory that is overwritten in Vanilla Doom, using
  // the values from the intercept structure.
  //
  // Note: the ->d.{thing,line} member should really have its
  // address translated into the correct address value for
  // Vanilla Doom.

  InterceptsMemoryOverrun(location, intercept->frac);
  InterceptsMemoryOverrun(location + 4, intercept->isaline);
  InterceptsMemoryOverrun(location + 8, static_cast<int>(reinterpret_cast<intptr_t>(intercept->d.thing)));
}

//
// P_PathTraverse
// Traces a line from x1,y1 to x2,y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//
bool P_PathTraverse(fixed_t x1,
                    fixed_t y1,
                    fixed_t x2,
                    fixed_t y2,
                    int     flags,
                    bool (*trav)(intercept_t *)) {
  fixed_t xt1;
  fixed_t yt1;
  fixed_t xt2;
  fixed_t yt2;

  fixed_t xstep;
  fixed_t ystep;

  fixed_t partial;

  fixed_t xintercept;
  fixed_t yintercept;

  int mapx;
  int mapy;

  int mapxstep;
  int mapystep;

  int count;

  earlyout = (flags & PT_EARLYOUT) != 0;

  validcount++;
  g_p_local_globals->intercept_p = intercepts;

  if (((x1 - g_p_local_blockmap->bmaporgx) & (MAPBLOCKSIZE - 1)) == 0)
    x1 += FRACUNIT; // don't side exactly on a line

  if (((y1 - g_p_local_blockmap->bmaporgy) & (MAPBLOCKSIZE - 1)) == 0)
    y1 += FRACUNIT; // don't side exactly on a line

  g_p_local_globals->trace.x  = x1;
  g_p_local_globals->trace.y  = y1;
  g_p_local_globals->trace.dx = x2 - x1;
  g_p_local_globals->trace.dy = y2 - y1;

  x1 -= g_p_local_blockmap->bmaporgx;
  y1 -= g_p_local_blockmap->bmaporgy;
  xt1 = x1 >> MAPBLOCKSHIFT;
  yt1 = y1 >> MAPBLOCKSHIFT;

  x2 -= g_p_local_blockmap->bmaporgx;
  y2 -= g_p_local_blockmap->bmaporgy;
  xt2 = x2 >> MAPBLOCKSHIFT;
  yt2 = y2 >> MAPBLOCKSHIFT;

  if (xt2 > xt1) {
    mapxstep = 1;
    partial  = FRACUNIT - ((x1 >> MAPBTOFRAC) & (FRACUNIT - 1));
    ystep    = FixedDiv(y2 - y1, std::abs(x2 - x1));
  } else if (xt2 < xt1) {
    mapxstep = -1;
    partial  = (x1 >> MAPBTOFRAC) & (FRACUNIT - 1);
    ystep    = FixedDiv(y2 - y1, std::abs(x2 - x1));
  } else {
    mapxstep = 0;
    partial  = FRACUNIT;
    ystep    = 256 * FRACUNIT;
  }

  yintercept = (y1 >> MAPBTOFRAC) + FixedMul(partial, ystep);

  if (yt2 > yt1) {
    mapystep = 1;
    partial  = FRACUNIT - ((y1 >> MAPBTOFRAC) & (FRACUNIT - 1));
    xstep    = FixedDiv(x2 - x1, std::abs(y2 - y1));
  } else if (yt2 < yt1) {
    mapystep = -1;
    partial  = (y1 >> MAPBTOFRAC) & (FRACUNIT - 1);
    xstep    = FixedDiv(x2 - x1, std::abs(y2 - y1));
  } else {
    mapystep = 0;
    partial  = FRACUNIT;
    xstep    = 256 * FRACUNIT;
  }
  xintercept = (x1 >> MAPBTOFRAC) + FixedMul(partial, xstep);

  // Step through map blocks.
  // Count is present to prevent a round off error
  // from skipping the break.
  mapx = xt1;
  mapy = yt1;

  for (count = 0; count < 64; count++) {
    if (flags & PT_ADDLINES) {
      if (!P_BlockLinesIterator(mapx, mapy, PIT_AddLineIntercepts))
        return false; // early out
    }

    if (flags & PT_ADDTHINGS) {
      if (!P_BlockThingsIterator(mapx, mapy, PIT_AddThingIntercepts))
        return false; // early out
    }

    if (mapx == xt2
        && mapy == yt2) {
      break;
    }

    if ((yintercept >> FRACBITS) == mapy) {
      yintercept += ystep;
      mapx += mapxstep;
    } else if ((xintercept >> FRACBITS) == mapx) {
      xintercept += xstep;
      mapy += mapystep;
    }
  }
  // go through the sorted list
  return P_TraverseIntercepts(trav, FRACUNIT);
}
