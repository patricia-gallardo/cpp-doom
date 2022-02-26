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
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//

#include <cstdio>
#include <cstdlib>

#include <fmt/printf.h>

#include "i_system.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "doomdef.hpp"
#include "doomstat.hpp"

#include "lump.hpp"
#include "r_bmaps.hpp" // [crispy] R_BrightmapForTexName()
#include "r_local.hpp"
#include "r_sky.hpp"
#include "r_swirl.hpp" // [crispy] R_DistortedFlat()

[[maybe_unused]] planefunction_t floorfunc;
[[maybe_unused]] planefunction_t ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
constexpr auto MAXVISPLANES = 128;
visplane_t *   visplanes    = nullptr;
visplane_t *   lastvisplane;
static int     numvisplanes;

// ?
constexpr auto MAXOPENINGS = MAXWIDTH * 64 * 4;
std::array<int, MAXOPENINGS> openings; // [crispy] 32-bit integer math
int *          lastopening;           // [crispy] 32-bit integer math

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
std::array<int, MAXWIDTH> floorclip;   // [crispy] 32-bit integer math
std::array<int, MAXWIDTH> ceilingclip; // [crispy] 32-bit integer math

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
std::array<int, MAXHEIGHT> spanstart;
[[maybe_unused]] int spanstop[MAXHEIGHT];

//
// texture mapping
//
lighttable_t ** planezlight;
fixed_t         planeheight;

fixed_t *                yslope;
fixed_t                  yslopes[LOOKDIRS][MAXHEIGHT];
[[maybe_unused]] fixed_t distscale[MAXWIDTH];
[[maybe_unused]] fixed_t basexscale;
[[maybe_unused]] fixed_t baseyscale;

fixed_t cachedheight[MAXHEIGHT];
fixed_t cacheddistance[MAXHEIGHT];
fixed_t cachedxstep[MAXHEIGHT];
fixed_t cachedystep[MAXHEIGHT];

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes() {
  // Doh!
}

//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
void R_MapPlane(int y,
                int x1,
                int x2) {
#ifdef RANGECHECK
  if (x2 < x1
      || x1 < 0
      || x2 >= g_r_state_globals->viewwidth
      || y > g_r_state_globals->viewheight) {
    I_Error("R_MapPlane: %i, %i at %i", x1, x2, y);
  }
#endif

  // [crispy] visplanes with the same flats now match up far better than before
  // adapted from prboom-plus/src/r_plane.c:191-239, translated to fixed-point math

  int dy = std::abs(centery - y);
  if (!dy) {
    return;
  }

  fixed_t distance = 0;
  if (planeheight != cachedheight[y]) {
    cachedheight[y] = planeheight;
    distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);
    g_r_draw_globals->ds_xstep = cachedxstep[y] = (FixedMul(viewsin, planeheight) / dy) << detailshift;
    g_r_draw_globals->ds_ystep = cachedystep[y] = (FixedMul(viewcos, planeheight) / dy) << detailshift;
  } else {
    distance                   = cacheddistance[y];
    g_r_draw_globals->ds_xstep = cachedxstep[y];
    g_r_draw_globals->ds_ystep = cachedystep[y];
  }

  int dx = x1 - centerx;

  g_r_draw_globals->ds_xfrac = g_r_state_globals->viewx + FixedMul(viewcos, distance) + dx * g_r_draw_globals->ds_xstep;
  g_r_draw_globals->ds_yfrac = -g_r_state_globals->viewy - FixedMul(viewsin, distance) + dx * g_r_draw_globals->ds_ystep;

  if (fixedcolormap)
    g_r_draw_globals->ds_colormap[0] = g_r_draw_globals->ds_colormap[1] = fixedcolormap;
  else {
    int index = distance >> LIGHTZSHIFT;

    if (index >= MAXLIGHTZ)
      index = MAXLIGHTZ - 1;

    g_r_draw_globals->ds_colormap[0] = planezlight[index];
    g_r_draw_globals->ds_colormap[1] = zlight[LIGHTLEVELS - 1][MAXLIGHTZ - 1];
  }

  g_r_draw_globals->ds_y  = y;
  g_r_draw_globals->ds_x1 = x1;
  g_r_draw_globals->ds_x2 = x2;

  // high or low detail
  spanfunc();
}

//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes() {
  angle_t angle;

  // opening / clipping determination
  for (int i = 0; i < g_r_state_globals->viewwidth; i++) {
    floorclip[i]   = g_r_state_globals->viewheight;
    ceilingclip[i] = -1;
  }

  lastvisplane = visplanes;
  lastopening  = openings.data();

  // texture calculation
  std::memset(cachedheight, 0, sizeof(cachedheight));

  // left to right mapping
  angle = (g_r_state_globals->viewangle - ANG90) >> ANGLETOFINESHIFT;

  // scale will be unit scale at SCREENWIDTH/2 distance
  basexscale = FixedDiv(finecosine[angle], centerxfrac);
  baseyscale = -FixedDiv(finesine[angle], centerxfrac);
}

// [crispy] remove MAXVISPLANES Vanilla limit
static void R_RaiseVisplanes(visplane_t ** vp) {
  if (lastvisplane - visplanes == numvisplanes) {
    int          numvisplanes_old = numvisplanes;
    visplane_t * visplanes_old    = visplanes;

    numvisplanes = numvisplanes ? 2 * numvisplanes : MAXVISPLANES;
    visplanes    = static_cast<decltype(visplanes)>(I_Realloc(visplanes, static_cast<unsigned long>(numvisplanes) * sizeof(*visplanes)));
    std::memset(visplanes + numvisplanes_old, 0, (static_cast<unsigned long>(numvisplanes - numvisplanes_old)) * sizeof(*visplanes));

    lastvisplane                    = visplanes + numvisplanes_old;
    g_r_state_globals->floorplane   = visplanes + (g_r_state_globals->floorplane - visplanes_old);
    g_r_state_globals->ceilingplane = visplanes + (g_r_state_globals->ceilingplane - visplanes_old);

    if (numvisplanes_old)
      fmt::fprintf(stderr, "R_FindPlane: Hit MAXVISPLANES limit at %d, raised to %d.\n", numvisplanes_old, numvisplanes);

    // keep the pointer passed as argument in relation to the visplanes pointer
    if (vp)
      *vp = visplanes + (*vp - visplanes_old);
  }
}

//
// R_FindPlane
//
visplane_t *
    R_FindPlane(fixed_t height,
                int     picnum,
                int     lightlevel) {
  visplane_t * check;

  // [crispy] add support for MBF sky tranfers
  if (picnum == g_doomstat_globals->skyflatnum || static_cast<unsigned int>(picnum) & PL_SKYFLAT) {
    height     = 0; // all skys map together
    lightlevel = 0;
  }

  for (check = visplanes; check < lastvisplane; check++) {
    if (height == check->height
        && picnum == check->picnum
        && lightlevel == check->lightlevel) {
      break;
    }
  }

  if (check < lastvisplane)
    return check;

  R_RaiseVisplanes(&check); // [crispy] remove VISPLANES limit
  if (lastvisplane - visplanes == MAXVISPLANES && false)
    I_Error("R_FindPlane: no more visplanes");

  lastvisplane++;

  check->height     = height;
  check->picnum     = picnum;
  check->lightlevel = lightlevel;
  check->minx       = SCREENWIDTH;
  check->maxx       = -1;

  std::memset(check->top, 0xff, sizeof(check->top));

  return check;
}

//
// R_CheckPlane
//
visplane_t *
    R_CheckPlane(visplane_t * pl,
                 int          start,
                 int          stop) {
  int intrl;
  int intrh;
  int unionl;
  int unionh;
  int x;

  if (start < pl->minx) {
    intrl  = pl->minx;
    unionl = start;
  } else {
    unionl = pl->minx;
    intrl  = start;
  }

  if (stop > pl->maxx) {
    intrh  = pl->maxx;
    unionh = stop;
  } else {
    unionh = pl->maxx;
    intrh  = stop;
  }

  for (x = intrl; x <= intrh; x++)
    if (pl->top[x] != 0xffffffffu) // [crispy] hires / 32-bit integer math
      break;

  // [crispy] fix HOM if ceilingplane and floorplane are the same
  // visplane (e.g. both are skies)
  if (!(pl == g_r_state_globals->floorplane && markceiling && g_r_state_globals->floorplane == g_r_state_globals->ceilingplane)) {
    if (x > intrh) {
      pl->minx = unionl;
      pl->maxx = unionh;

      // use the same one
      return pl;
    }
  }

  // make a new visplane
  R_RaiseVisplanes(&pl); // [crispy] remove VISPLANES limit
  lastvisplane->height     = pl->height;
  lastvisplane->picnum     = pl->picnum;
  lastvisplane->lightlevel = pl->lightlevel;

  if (lastvisplane - visplanes == MAXVISPLANES && false) // [crispy] remove VISPLANES limit
    I_Error("R_CheckPlane: no more visplanes");

  pl       = lastvisplane++;
  pl->minx = start;
  pl->maxx = stop;

  std::memset(pl->top, 0xff, sizeof(pl->top));

  return pl;
}

//
// R_MakeSpans
//
void R_MakeSpans(int          x,
                 unsigned int t1, // [crispy] 32-bit integer math
                 unsigned int b1, // [crispy] 32-bit integer math
                 unsigned int t2, // [crispy] 32-bit integer math
                 unsigned int b2) // [crispy] 32-bit integer math
{
  while (t1 < t2 && t1 <= b1) {
    R_MapPlane(static_cast<int>(t1), spanstart[t1], x - 1);
    t1++;
  }
  while (b1 > b2 && b1 >= t1) {
    R_MapPlane(static_cast<int>(b1), spanstart[b1], x - 1);
    b1--;
  }

  while (t2 < t1 && t2 <= b2) {
    spanstart[t2] = x;
    t2++;
  }
  while (b2 > b1 && b2 >= t2) {
    spanstart[b2] = x;
    b2--;
  }
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes() {
  visplane_t * pl;
  int          light;
  int          x;
  int          stop;
  int          angle;
  int          lumpnum;

#ifdef RANGECHECK
  if (ds_p - drawsegs > numdrawsegs)
    I_Error("R_DrawPlanes: drawsegs overflow (%" PRIiPTR ")",
            ds_p - drawsegs);

  if (lastvisplane - visplanes > numvisplanes)
    I_Error("R_DrawPlanes: visplane overflow (%" PRIiPTR ")",
            lastvisplane - visplanes);

  if (lastopening - openings.data() > MAXOPENINGS)
    I_Error("R_DrawPlanes: opening overflow (%" PRIiPTR ")",
            lastopening - openings.data());
#endif

  for (pl = visplanes; pl < lastvisplane; pl++) {
    const bool swirling = (g_r_state_globals->flattranslation[pl->picnum] == -1);

    if (pl->minx > pl->maxx)
      continue;

    // sky flat
    // [crispy] add support for MBF sky tranfers
    if (pl->picnum == g_doomstat_globals->skyflatnum || static_cast<unsigned int>(pl->picnum) & PL_SKYFLAT) {
      int     texture;
      angle_t an = g_r_state_globals->viewangle, flip;
      if (static_cast<unsigned int>(pl->picnum) & PL_SKYFLAT) {
        const line_t * l                = &g_r_state_globals->lines[static_cast<unsigned int>(pl->picnum) & ~PL_SKYFLAT];
        const side_t * s                = *l->sidenum + g_r_state_globals->sides;
        texture                         = g_r_state_globals->texturetranslation[s->toptexture];
        g_r_draw_globals->dc_texturemid = s->rowoffset - 28 * FRACUNIT;
        // [crispy] stretch sky
        if (crispy->stretchsky) {
          g_r_draw_globals->dc_texturemid = g_r_draw_globals->dc_texturemid * (g_r_state_globals->textureheight[texture] >> FRACBITS) / SKYSTRETCH_HEIGHT;
        }
        flip = (l->special == 272) ? 0u : ~0u;
        an += static_cast<unsigned int>(s->textureoffset);
      } else {
        texture                         = skytexture;
        g_r_draw_globals->dc_texturemid = skytexturemid;
        flip                            = 0;
      }
      g_r_draw_globals->dc_iscale = pspriteiscale >> detailshift;

      // Sky is allways drawn full bright,
      //  i.e. colormaps[0] is used.
      // Because of this hack, sky is not affected
      //  by INVUL inverse mapping.
      // [crispy] no brightmaps for sky
      g_r_draw_globals->dc_colormap[0] = g_r_draw_globals->dc_colormap[1] = g_r_state_globals->colormaps;
      //	    dc_texturemid = skytexturemid;
      g_r_draw_globals->dc_texheight = g_r_state_globals->textureheight[texture] >> FRACBITS; // [crispy] Tutti-Frutti fix
      // [crispy] stretch sky
      if (crispy->stretchsky)
        g_r_draw_globals->dc_iscale = g_r_draw_globals->dc_iscale * g_r_draw_globals->dc_texheight / SKYSTRETCH_HEIGHT;
      for (x = pl->minx; x <= pl->maxx; x++) {
        g_r_draw_globals->dc_yl = static_cast<int>(pl->top[x]);
        g_r_draw_globals->dc_yh = static_cast<int>(pl->bottom[x]);

        if (g_r_draw_globals->dc_yl <= g_r_draw_globals->dc_yh) // [crispy] 32-bit integer math
        {
          angle                       = ((an + g_r_state_globals->xtoviewangle[x]) ^ flip) >> ANGLETOSKYSHIFT;
          g_r_draw_globals->dc_x      = x;
          g_r_draw_globals->dc_source = R_GetColumn(texture, angle, false);
          colfunc();
        }
      }
      continue;
    }

    // regular flat
    lumpnum = g_r_state_globals->firstflat + (swirling ? pl->picnum : g_r_state_globals->flattranslation[pl->picnum]);
    // [crispy] add support for SMMU swirling flats
    g_r_draw_globals->ds_source =
        static_cast<uint8_t *>(swirling ? reinterpret_cast<unsigned char *>(R_DistortedFlat(lumpnum)) : cache_lump_num<uint8_t *>(lumpnum, PU_STATIC));
    g_r_draw_globals->ds_brightmap = R_BrightmapForFlatNum(lumpnum - g_r_state_globals->firstflat);

    planeheight = std::abs(pl->height - g_r_state_globals->viewz);
    light       = (pl->lightlevel >> LIGHTSEGSHIFT) + (extralight * LIGHTBRIGHT);

    if (light >= LIGHTLEVELS)
      light = LIGHTLEVELS - 1;

    if (light < 0)
      light = 0;

    planezlight = zlight[light];

    pl->top[pl->maxx + 1] = 0xffffffffu; // [crispy] hires / 32-bit integer math
    pl->top[pl->minx - 1] = 0xffffffffu; // [crispy] hires / 32-bit integer math

    stop = pl->maxx + 1;

    for (x = pl->minx; x <= stop; x++) {
      R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x]);
    }

    W_ReleaseLumpNum(lumpnum);
  }
}
