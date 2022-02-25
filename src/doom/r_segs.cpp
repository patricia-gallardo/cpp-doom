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
//	All the clipping: columns, horizontal spans, sky columns.
//

#include <cstdio>

#include <fmt/printf.h>

#include "doomdef.hpp"
#include "doomstat.hpp"
#include "i_system.hpp"
#include "r_bmaps.hpp" // [crispy] brightmaps
#include "r_local.hpp"

// OPTIMIZE: closed two sided lines as single sided

// True if any of the segs textures might be visible.
bool segtextured;

// False if the back side is the same plane.
bool markfloor;
bool markceiling;

bool maskedtexture;
int  toptexture;
int  bottomtexture;
int  midtexture;

//
// regular wall
//
int     rw_x;
int     rw_stopx;
angle_t rw_centerangle;
fixed_t rw_offset;
fixed_t rw_scale;
fixed_t rw_scalestep;
fixed_t rw_midtexturemid;
fixed_t rw_toptexturemid;
fixed_t rw_bottomtexturemid;

int worldtop;
int worldbottom;
int worldhigh;
int worldlow;

int64_t pixhigh; // [crispy] WiggleFix
int64_t pixlow;  // [crispy] WiggleFix
fixed_t pixhighstep;
fixed_t pixlowstep;

int64_t topfrac; // [crispy] WiggleFix
fixed_t topstep;

int64_t bottomfrac; // [crispy] WiggleFix
fixed_t bottomstep;

lighttable_t ** walllights;

int * maskedtexturecol; // [crispy] 32-bit integer math

// [crispy] WiggleFix: add this code block near the top of r_segs.c
//
// R_FixWiggle()
// Dynamic wall/texture rescaler, AKA "WiggleHack II"
//  by Kurt "kb1" Baumgardner ("kb") and Andrey "Entryway" Budko ("e6y")
//
//  [kb] When the rendered view is positioned, such that the viewer is
//   looking almost parallel down a wall, the result of the scale
//   calculation in R_ScaleFromGlobalAngle becomes very large. And, the
//   taller the wall, the larger that value becomes. If these large
//   values were used as-is, subsequent calculations would overflow,
//   causing full-screen HOM, and possible program crashes.
//
//  Therefore, vanilla Doom clamps this scale calculation, preventing it
//   from becoming larger than 0x400000 (64*FRACUNIT). This number was
//   chosen carefully, to allow reasonably-tight angles, with reasonably
//   tall sectors to be rendered, within the limits of the fixed-point
//   math system being used. When the scale gets clamped, Doom cannot
//   properly render the wall, causing an undesirable wall-bending
//   effect that I call "floor wiggle". Not a crash, but still ugly.
//
//  Modern source ports offer higher video resolutions, which worsens
//   the issue. And, Doom is simply not adjusted for the taller walls
//   found in many PWADs.
//
//  This code attempts to correct these issues, by dynamically
//   adjusting the fixed-point math, and the maximum scale clamp,
//   on a wall-by-wall basis. This has 2 effects:
//
//  1. Floor wiggle is greatly reduced and/or eliminated.
//  2. Overflow is no longer possible, even in levels with maximum
//     height sectors (65535 is the theoretical height, though Doom
//     cannot handle sectors > 32767 units in height.
//
//  The code is not perfect across all situations. Some floor wiggle can
//   still be seen, and some texture strips may be slightly misaligned in
//   extreme cases. These effects cannot be corrected further, without
//   increasing the precision of various renderer variables, and,
//   possibly, creating a noticable performance penalty.
//

static int max_rwscale = 64 * FRACUNIT;
static int heightbits  = 12;
static int heightunit  = (1 << 12);
static int invhgtbits  = 4;

static const struct
{
  int clamp;
  int heightbits;
} scale_values[8] = {
  {2048 * FRACUNIT,  12},
  { 1024 * FRACUNIT, 12},
  { 1024 * FRACUNIT, 11},
  { 512 * FRACUNIT,  11},
  { 512 * FRACUNIT,  10},
  { 256 * FRACUNIT,  10},
  { 256 * FRACUNIT,  9 },
  { 128 * FRACUNIT,  9 }
};

void R_FixWiggle(sector_t * sector) {
  static int lastheight = 0;
  int        height     = (sector->interpceilingheight - sector->interpfloorheight) >> FRACBITS;

  // disallow negative heights. using 1 forces cache initialization
  if (height < 1)
    height = 1;

  // early out?
  if (height != lastheight) {
    lastheight = height;

    // initialize, or handle moving sector
    if (height != sector->cachedheight) {
      sector->cachedheight = height;
      sector->scaleindex   = 0;
      height >>= 7;

      // calculate adjustment
      while (height >>= 1)
        sector->scaleindex++;
    }

    // fine-tune renderer for this wall
    max_rwscale = scale_values[sector->scaleindex].clamp;
    heightbits  = scale_values[sector->scaleindex].heightbits;
    heightunit  = (1 << heightbits);
    invhgtbits  = FRACBITS - heightbits;
  }
}

//
// R_RenderMaskedSegRange
//
void R_RenderMaskedSegRange(drawseg_t * ds,
                            int         x1,
                            int         x2) {
  // Calculate light table.
  // Use different light tables
  //   for horizontal / vertical / diagonal. Diagonal?
  // OPTIMIZE: get rid of LIGHTSEGSHIFT globally
  curline     = ds->curline;
  frontsector = curline->frontsector;
  backsector  = curline->backsector;
  int texnum  = g_r_state_globals->texturetranslation[curline->sidedef->midtexture];

  int lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + (extralight * LIGHTBRIGHT);

  // [crispy] smoother fake contrast
  lightnum += curline->fakecontrast;
  /*
  if (curline->v1->y == curline->v2->y)
      lightnum--;
  else if (curline->v1->x == curline->v2->x)
      lightnum++;
*/

  if (lightnum < 0)
    walllights = scalelight[0];
  else if (lightnum >= LIGHTLEVELS)
    walllights = scalelight[LIGHTLEVELS - 1];
  else
    walllights = scalelight[lightnum];

  maskedtexturecol = ds->maskedtexturecol;

  rw_scalestep = ds->scalestep;
  spryscale    = ds->scale1 + (x1 - ds->x1) * rw_scalestep;
  mfloorclip   = ds->sprbottomclip;
  mceilingclip = ds->sprtopclip;

  // find positioning
  if (curline->linedef->flags & ML_DONTPEGBOTTOM) {
    g_r_draw_globals->dc_texturemid = frontsector->interpfloorheight > backsector->interpfloorheight ? frontsector->interpfloorheight : backsector->interpfloorheight;
    g_r_draw_globals->dc_texturemid = g_r_draw_globals->dc_texturemid + g_r_state_globals->textureheight[texnum] - g_r_state_globals->viewz;
  } else {
    g_r_draw_globals->dc_texturemid = frontsector->interpceilingheight < backsector->interpceilingheight ? frontsector->interpceilingheight : backsector->interpceilingheight;
    g_r_draw_globals->dc_texturemid = g_r_draw_globals->dc_texturemid - g_r_state_globals->viewz;
  }
  g_r_draw_globals->dc_texturemid += curline->sidedef->rowoffset;

  if (fixedcolormap)
    g_r_draw_globals->dc_colormap[0] = g_r_draw_globals->dc_colormap[1] = fixedcolormap;

  // draw the columns
  for (g_r_draw_globals->dc_x = x1; g_r_draw_globals->dc_x <= x2; g_r_draw_globals->dc_x++) {
    // calculate lighting
    if (maskedtexturecol[g_r_draw_globals->dc_x] != INT_MAX) // [crispy] 32-bit integer math
    {
      if (!fixedcolormap) {
        int index = spryscale >> (LIGHTSCALESHIFT + crispy->hires);

        if (index >= MAXLIGHTSCALE)
          index = MAXLIGHTSCALE - 1;

        // [crispy] no brightmaps for mid-textures
        g_r_draw_globals->dc_colormap[0] = g_r_draw_globals->dc_colormap[1] = walllights[index];
      }

      // [crispy] apply Killough's int64 sprtopscreen overflow fix
      // from winmbf/Source/r_segs.c:174-191
      // killough 3/2/98:
      //
      // This calculation used to overflow and cause crashes in Doom:
      //
      // sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
      //
      // This code fixes it, by using double-precision intermediate
      // arithmetic and by skipping the drawing of 2s normals whose
      // mapping to screen coordinates is totally out of range:

      {
        int64_t t = (static_cast<int64_t>(centeryfrac) << FRACBITS) - static_cast<int64_t>(g_r_draw_globals->dc_texturemid) * spryscale;

        if (t + static_cast<int64_t>(g_r_state_globals->textureheight[texnum]) * spryscale < 0 || t > static_cast<int64_t>(SCREENHEIGHT) << FRACBITS * 2) {
          spryscale += rw_scalestep; // [crispy] MBF had this in the for-loop iterator
          continue;                  // skip if the texture is out of screen's range
        }

        sprtopscreen = static_cast<int64_t>(t >> FRACBITS); // [crispy] WiggleFix
      }

      g_r_draw_globals->dc_iscale = static_cast<fixed_t>(0xffffffffu / static_cast<unsigned>(spryscale));

      // draw the texture
      auto * col_ptr = reinterpret_cast<uint8_t *>(R_GetColumn(texnum, maskedtexturecol[g_r_draw_globals->dc_x], false) - 3);
      auto * col     = reinterpret_cast<column_t *>(col_ptr);

      R_DrawMaskedColumn(col);
      maskedtexturecol[g_r_draw_globals->dc_x] = INT_MAX; // [crispy] 32-bit integer math
    }
    spryscale += rw_scalestep;
  }
}

//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked
//  texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling
//  textures.
// CALLED: CORE LOOPING ROUTINE.
//
constexpr auto                  HEIGHTBITS = 12;
[[maybe_unused]] constexpr auto HEIGHTUNIT = (1 << HEIGHTBITS);

void R_RenderSegLoop() {
  angle_t angle;
  int     yl;
  int     yh;
  int     mid;
  fixed_t texturecolumn;
  int     top;
  int     bottom;

  for (; rw_x < rw_stopx; rw_x++) {
    // mark floor / ceiling areas
    yl = static_cast<int>((topfrac + heightunit - 1) >> heightbits); // [crispy] WiggleFix

    // no space above wall?
    if (yl < ceilingclip[rw_x] + 1)
      yl = ceilingclip[rw_x] + 1;

    if (markceiling) {
      top    = ceilingclip[rw_x] + 1;
      bottom = yl - 1;

      if (bottom >= floorclip[rw_x])
        bottom = floorclip[rw_x] - 1;

      if (top <= bottom) {
        g_r_state_globals->ceilingplane->top[rw_x]    = static_cast<unsigned int>(top);
        g_r_state_globals->ceilingplane->bottom[rw_x] = static_cast<unsigned int>(bottom);
      }
    }

    yh = static_cast<int>(bottomfrac >> heightbits); // [crispy] WiggleFix

    if (yh >= floorclip[rw_x])
      yh = floorclip[rw_x] - 1;

    if (markfloor) {
      top    = yh + 1;
      bottom = floorclip[rw_x] - 1;
      if (top <= ceilingclip[rw_x])
        top = ceilingclip[rw_x] + 1;
      if (top <= bottom) {
        g_r_state_globals->floorplane->top[rw_x]    = static_cast<unsigned int>(top);
        g_r_state_globals->floorplane->bottom[rw_x] = static_cast<unsigned int>(bottom);
      }
    }

    // texturecolumn and lighting are independent of wall tiers
    if (segtextured) {
      // calculate texture offset
      angle         = (rw_centerangle + g_r_state_globals->xtoviewangle[rw_x]) >> ANGLETOFINESHIFT;
      texturecolumn = rw_offset - FixedMul(finetangent[angle], g_r_state_globals->rw_distance);
      texturecolumn >>= FRACBITS;
      // calculate lighting
      int index = rw_scale >> (LIGHTSCALESHIFT + crispy->hires);

      if (index >= MAXLIGHTSCALE)
        index = MAXLIGHTSCALE - 1;

      // [crispy] optional brightmaps
      g_r_draw_globals->dc_colormap[0] = walllights[index];
      g_r_draw_globals->dc_colormap[1] = (!fixedcolormap && (crispy->brightmaps & BRIGHTMAPS_TEXTURES)) ? scalelight[LIGHTLEVELS - 1][MAXLIGHTSCALE - 1] : g_r_draw_globals->dc_colormap[0];
      g_r_draw_globals->dc_x           = rw_x;
      g_r_draw_globals->dc_iscale      = static_cast<fixed_t>(0xffffffffu / static_cast<unsigned>(rw_scale));
    } else {
      // purely to shut up the compiler

      texturecolumn = 0;
    }

    // draw the wall tiers
    if (midtexture) {
      // single sided line
      g_r_draw_globals->dc_yl         = yl;
      g_r_draw_globals->dc_yh         = yh;
      g_r_draw_globals->dc_texturemid = rw_midtexturemid;
      g_r_draw_globals->dc_source     = R_GetColumn(midtexture, texturecolumn, true);
      g_r_draw_globals->dc_texheight  = g_r_state_globals->textureheight[midtexture] >> FRACBITS; // [crispy] Tutti-Frutti fix
      g_r_draw_globals->dc_brightmap  = texturebrightmap[midtexture];
      colfunc();
      ceilingclip[rw_x] = g_r_state_globals->viewheight;
      floorclip[rw_x]   = -1;
    } else {
      // two sided line
      if (toptexture) {
        // top wall
        mid = static_cast<int>(pixhigh >> heightbits); // [crispy] WiggleFix
        pixhigh += pixhighstep;

        if (mid >= floorclip[rw_x])
          mid = floorclip[rw_x] - 1;

        if (mid >= yl) {
          g_r_draw_globals->dc_yl         = yl;
          g_r_draw_globals->dc_yh         = mid;
          g_r_draw_globals->dc_texturemid = rw_toptexturemid;
          g_r_draw_globals->dc_source     = R_GetColumn(toptexture, texturecolumn, true);
          g_r_draw_globals->dc_texheight  = g_r_state_globals->textureheight[toptexture] >> FRACBITS; // [crispy] Tutti-Frutti fix
          g_r_draw_globals->dc_brightmap  = texturebrightmap[toptexture];
          colfunc();
          ceilingclip[rw_x] = mid;
        } else
          ceilingclip[rw_x] = yl - 1;
      } else {
        // no top wall
        if (markceiling)
          ceilingclip[rw_x] = yl - 1;
      }

      if (bottomtexture) {
        // bottom wall
        mid = static_cast<int>((pixlow + heightunit - 1) >> heightbits); // [crispy] WiggleFix
        pixlow += pixlowstep;

        // no space above wall?
        if (mid <= ceilingclip[rw_x])
          mid = ceilingclip[rw_x] + 1;

        if (mid <= yh) {
          g_r_draw_globals->dc_yl         = mid;
          g_r_draw_globals->dc_yh         = yh;
          g_r_draw_globals->dc_texturemid = rw_bottomtexturemid;
          g_r_draw_globals->dc_source     = R_GetColumn(bottomtexture,
                                                    texturecolumn,
                                                    true);
          g_r_draw_globals->dc_texheight  = g_r_state_globals->textureheight[bottomtexture] >> FRACBITS; // [crispy] Tutti-Frutti fix
          g_r_draw_globals->dc_brightmap  = texturebrightmap[bottomtexture];
          colfunc();
          floorclip[rw_x] = mid;
        } else
          floorclip[rw_x] = yh + 1;
      } else {
        // no bottom wall
        if (markfloor)
          floorclip[rw_x] = yh + 1;
      }

      if (maskedtexture) {
        // save texturecol
        //  for backdrawing of masked mid texture
        maskedtexturecol[rw_x] = texturecolumn;
      }
    }

    rw_scale += rw_scalestep;
    topfrac += topstep;
    bottomfrac += bottomstep;
  }
}

// [crispy] WiggleFix: move R_ScaleFromGlobalAngle function to r_segs.c,
// above R_StoreWallRange
fixed_t R_ScaleFromGlobalAngle(angle_t visangle) {
  int     anglea = static_cast<int>(ANG90 + (visangle - g_r_state_globals->viewangle));
  int     angleb = static_cast<int>(ANG90 + (visangle - g_r_state_globals->rw_normalangle));
  int     den    = FixedMul(g_r_state_globals->rw_distance, finesine[anglea >> ANGLETOFINESHIFT]);
  fixed_t num    = FixedMul(projection, finesine[angleb >> ANGLETOFINESHIFT]) << detailshift;
  fixed_t scale;

  if (den > (num >> 16)) {
    scale = FixedDiv(num, den);

    // [kb] When this evaluates True, the scale is clamped,
    //  and there will be some wiggling.
    if (scale > max_rwscale)
      scale = max_rwscale;
    else if (scale < 256)
      scale = 256;
  } else
    scale = max_rwscale;

  return scale;
}

//
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//
void R_StoreWallRange(int start,
                      int stop) {
  fixed_t        vtop;
  int            lightnum;
  int64_t        dx, dy, dx1, dy1, dist; // [crispy] fix long wall wobble
  const uint32_t len = curline->length;

  // [crispy] remove MAXDRAWSEGS Vanilla limit
  if (ds_p == &drawsegs[numdrawsegs]) {
    int numdrawsegs_old = numdrawsegs;

    numdrawsegs = numdrawsegs ? 2 * numdrawsegs : MAXDRAWSEGS;
    drawsegs    = static_cast<decltype(drawsegs)>(I_Realloc(drawsegs, static_cast<unsigned long>(numdrawsegs) * sizeof(*drawsegs)));
    std::memset(drawsegs + numdrawsegs_old, 0, (static_cast<unsigned long>(numdrawsegs - numdrawsegs_old)) * sizeof(*drawsegs));

    ds_p = drawsegs + numdrawsegs_old;

    if (numdrawsegs_old)
      fmt::fprintf(stderr, "R_StoreWallRange: Hit MAXDRAWSEGS limit at %d, raised to %d.\n", numdrawsegs_old, numdrawsegs);
  }

#ifdef RANGECHECK
  if (start >= g_r_state_globals->viewwidth || start > stop)
    I_Error("Bad R_RenderWallRange: %i to %i", start, stop);
#endif

  sidedef = curline->sidedef;
  linedef = curline->linedef;

  // mark the segment as visible for auto map
  linedef->flags |= ML_MAPPED;

  // [crispy] (flags & ML_MAPPED) is all we need to know for automap
  if (g_doomstat_globals->automapactive && !crispy->automapoverlay)
    return;

  // calculate rw_distance for scale calculation
  g_r_state_globals->rw_normalangle = curline->r_angle + ANG90; // [crispy] use re-calculated angle

  // [crispy] fix long wall wobble
  // thank you very much Linguica, e6y and kb1
  // http://www.doomworld.com/vb/post/1340718
  // shift right to avoid possibility of int64 overflow in rw_distance calculation
  dx                             = (static_cast<int64_t>(curline->v2->r_x) - curline->v1->r_x) >> 1;
  dy                             = (static_cast<int64_t>(curline->v2->r_y) - curline->v1->r_y) >> 1;
  dx1                            = (static_cast<int64_t>(g_r_state_globals->viewx) - curline->v1->r_x) >> 1;
  dy1                            = (static_cast<int64_t>(g_r_state_globals->viewy) - curline->v1->r_y) >> 1;
  dist                           = ((dy * dx1 - dx * dy1) / len) << 1;
  g_r_state_globals->rw_distance = static_cast<fixed_t>(std::clamp<int64_t>(dist, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));

  ds_p->x1 = rw_x = start;
  ds_p->x2        = stop;
  ds_p->curline   = curline;
  rw_stopx        = stop + 1;

  // [crispy] WiggleFix: add this line, in r_segs.c:R_StoreWallRange,
  // right before calls to R_ScaleFromGlobalAngle:
  R_FixWiggle(frontsector);

  // calculate scale at both ends and step
  ds_p->scale1 = rw_scale =
      R_ScaleFromGlobalAngle(g_r_state_globals->viewangle + g_r_state_globals->xtoviewangle[start]);

  if (stop > start) {
    ds_p->scale2    = R_ScaleFromGlobalAngle(g_r_state_globals->viewangle + g_r_state_globals->xtoviewangle[stop]);
    ds_p->scalestep = rw_scalestep =
        (ds_p->scale2 - rw_scale) / (stop - start);
  } else {
    // UNUSED: try to fix the stretched line bug
#if 0
	if (rw_distance < FRACUNIT/2)
	{
	    fixed_t		trx,try;
	    fixed_t		gxt,gyt;

	    trx = curline->v1->x - viewx;
	    try = curline->v1->y - viewy;
			
	    gxt = FixedMul(trx,viewcos); 
	    gyt = -FixedMul(try,viewsin); 
	    ds_p->scale1 = FixedDiv(projection, gxt-gyt)<<detailshift;
	}
#endif
    ds_p->scale2 = ds_p->scale1;
  }

  // calculate texture boundaries
  //  and decide if floor / ceiling marks are needed
  worldtop    = frontsector->interpceilingheight - g_r_state_globals->viewz;
  worldbottom = frontsector->interpfloorheight - g_r_state_globals->viewz;

  midtexture = toptexture = bottomtexture = 0;
  maskedtexture                           = false;
  ds_p->maskedtexturecol                  = nullptr;

  if (!backsector) {
    // single sided line
    midtexture = g_r_state_globals->texturetranslation[sidedef->midtexture];
    // a single sided line is terminal, so it must mark ends
    markfloor = markceiling = true;
    if (linedef->flags & ML_DONTPEGBOTTOM) {
      vtop = frontsector->interpfloorheight + g_r_state_globals->textureheight[sidedef->midtexture];
      // bottom of texture at bottom
      rw_midtexturemid = vtop - g_r_state_globals->viewz;
    } else {
      // top of texture at top
      rw_midtexturemid = worldtop;
    }
    rw_midtexturemid += sidedef->rowoffset;

    ds_p->silhouette    = SIL_BOTH;
    ds_p->sprtopclip    = screenheightarray;
    ds_p->sprbottomclip = negonearray;
    ds_p->bsilheight    = INT_MAX;
    ds_p->tsilheight    = INT_MIN;
  } else {
    // [crispy] fix sprites being visible behind closed doors
    // adapted from mbfsrc/R_BSP.C:234-257
    const bool doorclosed =
        // if door is closed because back is shut:
        backsector->interpceilingheight <= backsector->interpfloorheight
        // preserve a kind of transparent door/lift special effect:
        && (backsector->interpceilingheight >= frontsector->interpceilingheight || curline->sidedef->toptexture)
        && (backsector->interpfloorheight <= frontsector->interpfloorheight || curline->sidedef->bottomtexture)
        // properly render skies (consider door "open" if both ceilings are sky):
        && (backsector->ceilingpic != g_doomstat_globals->skyflatnum || frontsector->ceilingpic != g_doomstat_globals->skyflatnum);

    // two sided line
    ds_p->sprtopclip = ds_p->sprbottomclip = nullptr;
    ds_p->silhouette                       = 0;

    if (frontsector->interpfloorheight > backsector->interpfloorheight) {
      ds_p->silhouette = SIL_BOTTOM;
      ds_p->bsilheight = frontsector->interpfloorheight;
    } else if (backsector->interpfloorheight > g_r_state_globals->viewz) {
      ds_p->silhouette = SIL_BOTTOM;
      ds_p->bsilheight = INT_MAX;
      // ds_p->sprbottomclip = negonearray;
    }

    if (frontsector->interpceilingheight < backsector->interpceilingheight) {
      ds_p->silhouette |= SIL_TOP;
      ds_p->tsilheight = frontsector->interpceilingheight;
    } else if (backsector->interpceilingheight < g_r_state_globals->viewz) {
      ds_p->silhouette |= SIL_TOP;
      ds_p->tsilheight = INT_MIN;
      // ds_p->sprtopclip = screenheightarray;
    }

    if (backsector->interpceilingheight <= frontsector->interpfloorheight || doorclosed) {
      ds_p->sprbottomclip = negonearray;
      ds_p->bsilheight    = INT_MAX;
      ds_p->silhouette |= SIL_BOTTOM;
    }

    if (backsector->interpfloorheight >= frontsector->interpceilingheight || doorclosed) {
      ds_p->sprtopclip = screenheightarray;
      ds_p->tsilheight = INT_MIN;
      ds_p->silhouette |= SIL_TOP;
    }

    worldhigh = backsector->interpceilingheight - g_r_state_globals->viewz;
    worldlow  = backsector->interpfloorheight - g_r_state_globals->viewz;

    // hack to allow height changes in outdoor areas
    if (frontsector->ceilingpic == g_doomstat_globals->skyflatnum
        && backsector->ceilingpic == g_doomstat_globals->skyflatnum) {
      worldtop = worldhigh;
    }

    if (worldlow != worldbottom
        || backsector->floorpic != frontsector->floorpic
        || backsector->lightlevel != frontsector->lightlevel) {
      markfloor = true;
    } else {
      // same plane on both sides
      markfloor = false;
    }

    if (worldhigh != worldtop
        || backsector->ceilingpic != frontsector->ceilingpic
        || backsector->lightlevel != frontsector->lightlevel) {
      markceiling = true;
    } else {
      // same plane on both sides
      markceiling = false;
    }

    if (backsector->interpceilingheight <= frontsector->interpfloorheight
        || backsector->interpfloorheight >= frontsector->interpceilingheight) {
      // closed door
      markceiling = markfloor = true;
    }

    if (worldhigh < worldtop) {
      // top texture
      toptexture = g_r_state_globals->texturetranslation[sidedef->toptexture];
      if (linedef->flags & ML_DONTPEGTOP) {
        // top of texture at top
        rw_toptexturemid = worldtop;
      } else {
        vtop =
            backsector->interpceilingheight
            + g_r_state_globals->textureheight[sidedef->toptexture];

        // bottom of texture
        rw_toptexturemid = vtop - g_r_state_globals->viewz;
      }
    }
    if (worldlow > worldbottom) {
      // bottom texture
      bottomtexture = g_r_state_globals->texturetranslation[sidedef->bottomtexture];

      if (linedef->flags & ML_DONTPEGBOTTOM) {
        // bottom of texture at bottom
        // top of texture at top
        rw_bottomtexturemid = worldtop;
      } else // top of texture at top
        rw_bottomtexturemid = worldlow;
    }
    rw_toptexturemid += sidedef->rowoffset;
    rw_bottomtexturemid += sidedef->rowoffset;

    // allocate space for masked texture tables
    if (sidedef->midtexture) {
      // masked midtexture
      maskedtexture          = true;
      ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
      lastopening += rw_stopx - rw_x;
    }
  }

  // calculate rw_offset (only needed for textured lines)
  segtextured = midtexture | toptexture | bottomtexture | static_cast<int>(maskedtexture);

  if (segtextured) {

    // [crispy] fix long wall wobble
    rw_offset = static_cast<fixed_t>(((dx * dx1 + dy * dy1) / len) << 1);
    rw_offset += sidedef->textureoffset + curline->offset;
    rw_centerangle = ANG90 + g_r_state_globals->viewangle - g_r_state_globals->rw_normalangle;

    // calculate light table
    //  use different light tables
    //  for horizontal / vertical / diagonal
    // OPTIMIZE: get rid of LIGHTSEGSHIFT globally
    if (!fixedcolormap) {
      lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + (extralight * LIGHTBRIGHT);

      // [crispy] smoother fake contrast
      lightnum += curline->fakecontrast;
      /*
      if (curline->v1->y == curline->v2->y)
          lightnum--;
      else if (curline->v1->x == curline->v2->x)
          lightnum++;
*/

      if (lightnum < 0)
        walllights = scalelight[0];
      else if (lightnum >= LIGHTLEVELS)
        walllights = scalelight[LIGHTLEVELS - 1];
      else
        walllights = scalelight[lightnum];
    }
  }

  // if a floor / ceiling plane is on the wrong side
  //  of the view plane, it is definitely invisible
  //  and doesn't need to be marked.

  if (frontsector->interpfloorheight >= g_r_state_globals->viewz) {
    // above view plane
    markfloor = false;
  }

  if (frontsector->interpceilingheight <= g_r_state_globals->viewz
      && frontsector->ceilingpic != g_doomstat_globals->skyflatnum) {
    // below view plane
    markceiling = false;
  }

  // calculate incremental stepping values for texture edges
  worldtop >>= invhgtbits;
  worldbottom >>= invhgtbits;

  topstep    = -FixedMul(rw_scalestep, worldtop);
  auto    aa = static_cast<int64_t>(centeryfrac);
  int64_t bb = static_cast<int64_t>(worldtop) * rw_scale;
  topfrac    = (aa >> invhgtbits) - (bb >> FRACBITS); // [crispy] WiggleFix

  bottomstep  = -FixedMul(rw_scalestep, worldbottom);
  auto    aaa = static_cast<int64_t>(centeryfrac);
  int64_t bbb = static_cast<int64_t>(worldbottom) * rw_scale;
  bottomfrac  = (aaa >> invhgtbits) - (bbb >> FRACBITS); // [crispy] WiggleFix

  if (backsector) {
    worldhigh >>= invhgtbits;
    worldlow >>= invhgtbits;

    if (worldhigh < worldtop) {
      int64_t a   = static_cast<int64_t>(centeryfrac) >> invhgtbits;
      int64_t b   = static_cast<int64_t>(worldhigh) * rw_scale;
      pixhigh     = a - (b >> FRACBITS); // [crispy] WiggleFix
      pixhighstep = -FixedMul(rw_scalestep, worldhigh);
    }

    if (worldlow > worldbottom) {
      int64_t a  = static_cast<int64_t>(centeryfrac) >> invhgtbits;
      int64_t b  = static_cast<int64_t>(worldlow) * rw_scale;
      pixlow     = a - (b >> FRACBITS); // [crispy] WiggleFix
      pixlowstep = -FixedMul(rw_scalestep, worldlow);
    }
  }

  // render it
  if (markceiling)
    g_r_state_globals->ceilingplane = R_CheckPlane(g_r_state_globals->ceilingplane, rw_x, rw_stopx - 1);

  if (markfloor)
    g_r_state_globals->floorplane = R_CheckPlane(g_r_state_globals->floorplane, rw_x, rw_stopx - 1);

  R_RenderSegLoop();

  // save sprite clipping info
  if (((ds_p->silhouette & SIL_TOP) || maskedtexture)
      && !ds_p->sprtopclip) {
    std::memcpy(lastopening, ceilingclip + start, sizeof(*lastopening) * (static_cast<unsigned long>(rw_stopx - start)));
    ds_p->sprtopclip = lastopening - start;
    lastopening += rw_stopx - start;
  }

  if (((ds_p->silhouette & SIL_BOTTOM) || maskedtexture)
      && !ds_p->sprbottomclip) {
    std::memcpy(lastopening, floorclip + start, sizeof(*lastopening) * (static_cast<unsigned long>(rw_stopx - start)));
    ds_p->sprbottomclip = lastopening - start;
    lastopening += rw_stopx - start;
  }

  if (maskedtexture && !(ds_p->silhouette & SIL_TOP)) {
    ds_p->silhouette |= SIL_TOP;
    ds_p->tsilheight = INT_MIN;
  }
  if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM)) {
    ds_p->silhouette |= SIL_BOTTOM;
    ds_p->bsilheight = INT_MAX;
  }
  ds_p++;
}
