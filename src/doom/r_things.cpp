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
//	Refresh of things, i.e. objects represented by sprites.
//

#include <cstdio>
#include <cstdlib>

#include <fmt/printf.h>

#include "deh_main.hpp"
#include "doomdef.hpp"

#include "i_swap.hpp"
#include "i_system.hpp"
#include "z_zone.hpp"
#include "w_wad.hpp"

#include "r_local.hpp"

#include "doomstat.hpp"

#include "lump.hpp"
#include "memory.hpp"
#include "p_local.hpp" // [crispy] MLOOKUNIT
#include "r_bmaps.hpp" // [crispy] R_BrightmapForTexName()
#include "v_trans.hpp" // [crispy] colored blood sprites

#define MINZ        (FRACUNIT * 4)
#define BASEYCENTER (ORIGHEIGHT / 2)

// void R_DrawColumn ();
// void R_DrawFuzzColumn ();

struct maskdraw_t {
  int x1;
  int x2;

  int column;
  int topclip;
  int bottomclip;
};

static degenmobj_t laserspot_m = { {} };
degenmobj_t *      laserspot   = &laserspot_m;

// [crispy] extendable, but the last char element must be zero,
// keep in sync with multiitem_t multiitem_crosshairtype[] in m_menu.c
static laserpatch_t laserpatch_m[] = {
  {'+',  "cross1", 0, 0, 0},
  { '^', "cross2", 0, 0, 0},
  { '.', "cross3", 0, 0, 0},
  { 0,   "",       0, 0, 0},
};
laserpatch_t * laserpatch = laserpatch_m;

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t pspritescale;
fixed_t pspriteiscale;

lighttable_t ** spritelights;

// constant arrays
//  used for psprite clipping and initializing clipping
int negonearray[MAXWIDTH];       // [crispy] 32-bit integer math
int screenheightarray[MAXWIDTH]; // [crispy] 32-bit integer math

//
// INITIALIZATION FUNCTIONS
//

spriteframe_t sprtemp[29];
int           maxframe;
const char *  spritename;

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void R_InstallSpriteLump(int      lump,
                         unsigned frame,
                         char     rot,
                         bool     flipped) {
  int r;
  // [crispy] support 16 sprite rotations
  unsigned rotation = (rot >= 'A') ? static_cast<unsigned int>(rot - 'A' + 10) : (rot >= '0') ? static_cast<unsigned int>(rot - '0') :
                                                                                                17;

  if (frame >= 29 || rotation > 16) // [crispy] support 16 sprite rotations
    I_Error("R_InstallSpriteLump: "
            "Bad frame characters in lump %i",
            lump);

  if (static_cast<int>(frame) > maxframe)
    maxframe = static_cast<int>(frame);

  if (rotation == 0) {
    // the lump should be used for all rotations
    // [crispy] make non-fatal
    if (sprtemp[frame].rotate == false)
      fmt::fprintf(stderr, "R_InitSprites: Sprite %s frame %c has "
                           "multip rot=0 lump\n",
                   spritename,
                   'A' + frame);

    // [crispy] make non-fatal
    if (sprtemp[frame].rotate == 1)
      fmt::fprintf(stderr, "R_InitSprites: Sprite %s frame %c has rotations "
                           "and a rot=0 lump\n",
                   spritename,
                   'A' + frame);

    // [crispy] moved ...
    //	sprtemp[frame].rotate = false;
    for (r = 0; r < 8; r++) {
      // [crispy] only if not yet substituted
      if (sprtemp[frame].lump[r] == -1) {
        sprtemp[frame].lump[r] = static_cast<short>(lump - g_r_state_globals->firstspritelump);
        sprtemp[frame].flip[r] = static_cast<uint8_t>(flipped);
        // [crispy] ... here
        sprtemp[frame].rotate = false;
      }
    }
    return;
  }

  // the lump is only used for one rotation
  // [crispy] make non-fatal
  if (sprtemp[frame].rotate == false)
    fmt::fprintf(stderr, "R_InitSprites: Sprite %s frame %c has rotations "
                         "and a rot=0 lump\n",
                 spritename,
                 'A' + frame);

  // [crispy] moved ...
  //    sprtemp[frame].rotate = true;

  // make 0 based
  rotation--;
  if (sprtemp[frame].lump[rotation] != -1) {
    // [crispy] make non-fatal
    fmt::fprintf(stderr, "R_InitSprites: Sprite %s : %c : %c "
                         "has two lumps mapped to it\n",
                 spritename,
                 'A' + frame,
                 '1' + rotation);
    return;
  }

  sprtemp[frame].lump[rotation] = static_cast<short>(lump - g_r_state_globals->firstspritelump);
  sprtemp[frame].flip[rotation] = static_cast<uint8_t>(flipped);
  // [crispy] ... here
  sprtemp[frame].rotate = true;
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
void R_InitSpriteDefs(const char ** namelist) {
  //    int          frame;
  //    int          rotation;

  // count the number of sprite names
  const char ** check = namelist;
  while (*check != nullptr)
    check++;

  g_r_state_globals->numsprites = static_cast<int>(check - namelist);

  if (!g_r_state_globals->numsprites)
    return;

  g_r_state_globals->sprites = zmalloc<decltype(g_r_state_globals->sprites)>(static_cast<unsigned long>(g_r_state_globals->numsprites) * sizeof(*g_r_state_globals->sprites), PU_STATIC, nullptr);

  int start = g_r_state_globals->firstspritelump - 1;
  int end   = g_r_state_globals->lastspritelump + 1;

  // scan all the lump names for each of the names,
  //  noting the highest frame letter.
  // Just compare 4 characters as ints
  for (int i = 0; i < g_r_state_globals->numsprites; i++) {
    spritename = DEH_String(namelist[i]);
    std::memset(sprtemp, -1, sizeof(sprtemp));

    maxframe = -1;

    // scan the lumps,
    //  filling in the frames for whatever is found
    for (int l = start + 1; l < end; l++) {
      if (!strncasecmp(lumpinfo[l]->name, spritename, 4)) {
        int frame    = lumpinfo[l]->name[4] - 'A';
        int rotation = lumpinfo[l]->name[5];
        int patched;

        if (g_doomstat_globals->modifiedgame)
          patched = W_GetNumForName(lumpinfo[l]->name);
        else
          patched = l;

        R_InstallSpriteLump(patched, static_cast<unsigned int>(frame), static_cast<char>(rotation), false);

        if (lumpinfo[l]->name[6]) {
          frame    = lumpinfo[l]->name[6] - 'A';
          rotation = lumpinfo[l]->name[7];
          R_InstallSpriteLump(l, static_cast<unsigned int>(frame), static_cast<char>(rotation), true);
        }
      }
    }

    // check the frames that were found for completeness
    if (maxframe == -1) {
      g_r_state_globals->sprites[i].numframes = 0;
      continue;
    }

    maxframe++;

    for (int frame = 0; frame < maxframe; frame++) {
      switch (static_cast<int>(sprtemp[frame].rotate)) {
      case -1:
        // no rotations were found for that frame at all
        // [crispy] make non-fatal
        fmt::fprintf(stderr, "R_InitSprites: No patches found "
                             "for %s frame %c\n",
                     spritename,
                     frame + 'A');
        break;

      case 0:
        // only the first rotation is needed
        break;

      case 1:
        // must have all 8 frames
        int rotation;
        for (rotation = 0; rotation < 8; rotation++)
          if (sprtemp[frame].lump[rotation] == -1)
            I_Error("R_InitSprites: Sprite %s frame %c "
                    "is missing rotations",
                    spritename,
                    frame + 'A');

        // [crispy] support 16 sprite rotations
        sprtemp[frame].rotate = 2;
        for (; rotation < 16; rotation++)
          if (sprtemp[frame].lump[rotation] == -1) {
            sprtemp[frame].rotate = 1;
            break;
          }

        break;
      }
    }

    // allocate space for the frames present and copy sprtemp to it
    g_r_state_globals->sprites[i].numframes    = maxframe;
    g_r_state_globals->sprites[i].spriteframes = zmalloc<decltype(g_r_state_globals->sprites[i].spriteframes)>(static_cast<unsigned long>(maxframe) * sizeof(spriteframe_t), PU_STATIC, nullptr);
    std::memcpy(g_r_state_globals->sprites[i].spriteframes, sprtemp, static_cast<unsigned long>(maxframe) * sizeof(spriteframe_t));
  }
}

//
// GAME FUNCTIONS
//
vissprite_t * vissprites = nullptr;
vissprite_t * vissprite_p;
int           newvissprite;
static int    numvissprites;

//
// R_InitSprites
// Called at program start.
//
void R_InitSprites(const char ** namelist) {
  for (int i = 0; i < SCREENWIDTH; i++) {
    negonearray[i] = -1;
  }

  R_InitSpriteDefs(namelist);
}

//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites() {
  vissprite_p = vissprites;
}

//
// R_NewVisSprite
//
vissprite_t overflowsprite;

vissprite_t * R_NewVisSprite() {
  // [crispy] remove MAXVISSPRITE Vanilla limit
  if (vissprite_p == &vissprites[numvissprites]) {
    static int max;
    int        numvissprites_old = numvissprites;

    // [crispy] cap MAXVISSPRITES limit at 4096
    if (!max && numvissprites == 32 * MAXVISSPRITES) {
      fmt::fprintf(stderr, "R_NewVisSprite: MAXVISSPRITES limit capped at %d.\n", numvissprites);
      max++;
    }

    if (max)
      return &overflowsprite;

    numvissprites = numvissprites ? 2 * numvissprites : MAXVISSPRITES;
    vissprites    = static_cast<decltype(vissprites)>(I_Realloc(vissprites, static_cast<unsigned long>(numvissprites) * sizeof(*vissprites)));
    std::memset(vissprites + numvissprites_old, 0, (static_cast<unsigned long>(numvissprites - numvissprites_old)) * sizeof(*vissprites));

    vissprite_p = vissprites + numvissprites_old;

    if (numvissprites_old)
      fmt::fprintf(stderr, "R_NewVisSprite: Hit MAXVISSPRITES limit at %d, raised to %d.\n", numvissprites_old, numvissprites);
  }

  vissprite_p++;
  return vissprite_p - 1;
}

//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
int * mfloorclip;   // [crispy] 32-bit integer math
int * mceilingclip; // [crispy] 32-bit integer math

fixed_t spryscale;
int64_t sprtopscreen; // [crispy] WiggleFix

void R_DrawMaskedColumn(column_t * column) {
  int64_t topscreen;    // [crispy] WiggleFix
  int64_t bottomscreen; // [crispy] WiggleFix
  fixed_t basetexturemid;
  int     top = -1;

  basetexturemid                 = g_r_draw_globals->dc_texturemid;
  g_r_draw_globals->dc_texheight = 0; // [crispy] Tutti-Frutti fix

  for (; column->topdelta != 0xff;) {
    // [crispy] support for DeePsea tall patches
    if (column->topdelta <= top) {
      top += column->topdelta;
    } else {
      top = column->topdelta;
    }
    // calculate unclipped screen coordinates
    //  for post
    topscreen    = sprtopscreen + spryscale * top;
    bottomscreen = topscreen + spryscale * column->length;

    g_r_draw_globals->dc_yl = static_cast<int>((topscreen + FRACUNIT - 1) >> FRACBITS); // [crispy] WiggleFix
    g_r_draw_globals->dc_yh = static_cast<int>((bottomscreen - 1) >> FRACBITS);         // [crispy] WiggleFix

    if (g_r_draw_globals->dc_yh >= mfloorclip[g_r_draw_globals->dc_x])
      g_r_draw_globals->dc_yh = mfloorclip[g_r_draw_globals->dc_x] - 1;
    if (g_r_draw_globals->dc_yl <= mceilingclip[g_r_draw_globals->dc_x])
      g_r_draw_globals->dc_yl = mceilingclip[g_r_draw_globals->dc_x] + 1;

    if (g_r_draw_globals->dc_yl <= g_r_draw_globals->dc_yh) {
      g_r_draw_globals->dc_source     = reinterpret_cast<uint8_t *>(column) + 3;
      g_r_draw_globals->dc_texturemid = basetexturemid - (top << FRACBITS);
      // dc_source = (byte *)column + 3 - top;

      // Drawn by either R_DrawColumn
      //  or (SHADOW) R_DrawFuzzColumn.
      colfunc();
    }
    uint8_t * col_ptr = reinterpret_cast<uint8_t *>(column) + column->length + 4;
    column            = reinterpret_cast<column_t *>(col_ptr);
  }

  g_r_draw_globals->dc_texturemid = basetexturemid;
}

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void R_DrawVisSprite(vissprite_t * vis, int, int) {
  column_t * column;
  int        texturecolumn;
  fixed_t    frac;
  patch_t *  patch;

  patch = cache_lump_num<patch_t *>(vis->patch + g_r_state_globals->firstspritelump, PU_CACHE);

  // [crispy] brightmaps for select sprites
  g_r_draw_globals->dc_colormap[0] = vis->colormap[0];
  g_r_draw_globals->dc_colormap[1] = vis->colormap[1];
  g_r_draw_globals->dc_brightmap   = vis->brightmap;

  if (!g_r_draw_globals->dc_colormap[0]) {
    // nullptr colormap = shadow draw
    colfunc = fuzzcolfunc;
  } else if (static_cast<unsigned int>(vis->mobjflags) & MF_TRANSLATION) {
    colfunc                          = transcolfunc;
    g_r_draw_globals->dc_translation = g_r_draw_globals->translationtables - 256 + ((static_cast<unsigned int>(vis->mobjflags) & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
  }
  // [crispy] color-translated sprites (i.e. blood)
  else if (vis->translation) {
    colfunc                          = transcolfunc;
    g_r_draw_globals->dc_translation = vis->translation;
  }
  // [crispy] translucent sprites
  else if (crispy->translucency && static_cast<unsigned int>(vis->mobjflags) & MF_TRANSLUCENT) {
    if (!(static_cast<unsigned int>(vis->mobjflags) & (MF_NOGRAVITY | MF_COUNTITEM))
        || (static_cast<unsigned int>(vis->mobjflags) & MF_NOGRAVITY && crispy->translucency & TRANSLUCENCY_MISSILE)
        || (static_cast<unsigned int>(vis->mobjflags) & MF_COUNTITEM && crispy->translucency & TRANSLUCENCY_ITEM)) {
      colfunc = tlcolfunc;
    }
#ifdef CRISPY_TRUECOLOR
    blendfunc = vis->blendfunc;
#endif
  }

  g_r_draw_globals->dc_iscale     = std::abs(vis->xiscale) >> detailshift;
  g_r_draw_globals->dc_texturemid = vis->texturemid;
  frac                            = vis->startfrac;
  spryscale                       = vis->scale;
  sprtopscreen                    = centeryfrac - FixedMul(g_r_draw_globals->dc_texturemid, spryscale);

  for (g_r_draw_globals->dc_x = vis->x1; g_r_draw_globals->dc_x <= vis->x2; g_r_draw_globals->dc_x++, frac += vis->xiscale) {
    static bool error = false;
    texturecolumn     = frac >> FRACBITS;
#ifdef RANGECHECK
    if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width)) {
      // [crispy] make non-fatal
      if (!error) {
        fmt::fprintf(stderr, "R_DrawSpriteRange: bad texturecolumn\n");
        error = true;
      }
      continue;
    }
#endif
    uint8_t * col_ptr = reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[texturecolumn]);
    column            = reinterpret_cast<column_t *>(col_ptr);
    R_DrawMaskedColumn(column);
  }

  colfunc = basecolfunc;
#ifdef CRISPY_TRUECOLOR
  blendfunc = I_BlendOver;
#endif
}

//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
void R_ProjectSprite(mobj_t * thing) {
  fixed_t tr_x;
  fixed_t tr_y;

  fixed_t gxt;
  fixed_t gyt;
  fixed_t gzt; // [JN] killough 3/27/98

  fixed_t tx;
  fixed_t tz;

  fixed_t xscale;

  int x1;
  int x2;

  spritedef_t *   sprdef;
  spriteframe_t * sprframe;
  int             lump;

  unsigned rot;
  bool     flip;

  int index;

  vissprite_t * vis;

  angle_t ang;
  fixed_t iscale;

  fixed_t interpx;
  fixed_t interpy;
  fixed_t interpz;
  fixed_t interpangle;

  // [AM] Interpolate between current and last position,
  //      if prudent.
  if (crispy->uncapped &&
      // Don't interpolate if the mobj did something
      // that would necessitate turning it off for a tic.
      thing->interp == 1 &&
      // Don't interpolate during a paused state.
      leveltime > oldleveltime) {
    interpx     = thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic);
    interpy     = thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic);
    interpz     = thing->oldz + FixedMul(thing->z - thing->oldz, fractionaltic);
    interpangle = static_cast<fixed_t>(R_InterpolateAngle(thing->oldangle, thing->angle, fractionaltic));
  } else {
    interpx     = thing->x;
    interpy     = thing->y;
    interpz     = thing->z;
    interpangle = static_cast<fixed_t>(thing->angle);
  }

  // transform the origin point
  tr_x = interpx - g_r_state_globals->viewx;
  tr_y = interpy - g_r_state_globals->viewy;

  gxt = FixedMul(tr_x, viewcos);
  gyt = -FixedMul(tr_y, viewsin);

  tz = gxt - gyt;

  // thing is behind view plane?
  if (tz < MINZ)
    return;

  xscale = FixedDiv(projection, tz);

  gxt = -FixedMul(tr_x, viewsin);
  gyt = FixedMul(tr_y, viewcos);
  tx  = -(gyt + gxt);

  // too far off the side?
  if (std::abs(tx) > (tz << 2))
    return;

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
  if (static_cast<unsigned int>(thing->sprite) >= static_cast<unsigned int>(g_r_state_globals->numsprites))
    I_Error("R_ProjectSprite: invalid sprite number %i ",
            thing->sprite);
#endif
  sprdef = &g_r_state_globals->sprites[thing->sprite];
  // [crispy] the TNT1 sprite is not supposed to be rendered anyway
  if (!sprdef->numframes && thing->sprite == SPR_TNT1) {
    return;
  }
#ifdef RANGECHECK
  if ((thing->frame & FF_FRAMEMASK) >= sprdef->numframes)
    I_Error("R_ProjectSprite: invalid sprite frame %i : %i ",
            thing->sprite,
            thing->frame);
#endif
  sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

  if (sprframe->rotate) {
    // choose a different rotation based on player view
    ang = R_PointToAngle(interpx, interpy);
    // [crispy] now made non-fatal
    if (sprframe->rotate == -1) {
      return;
    } else
      // [crispy] support 16 sprite rotations
      if (sprframe->rotate == 2) {
        const unsigned rot2 = (ang - static_cast<unsigned int>(interpangle) + static_cast<unsigned>(ANG45 / 4) * 17);
        rot                 = (rot2 >> 29) + ((rot2 >> 25) & 8);
      } else {
        rot = (ang - static_cast<unsigned int>(interpangle) + static_cast<unsigned>(ANG45 / 2) * 9) >> 29;
      }
    lump = sprframe->lump[rot];
    flip = static_cast<bool>(sprframe->flip[rot]);
  } else {
    // use single rotation for all views
    lump = sprframe->lump[0];
    flip = static_cast<bool>(sprframe->flip[0]);
  }

  // [crispy] randomly flip corpse, blood and death animation sprites
  if (crispy->flipcorpses && (thing->flags & MF_FLIPPABLE) && !(thing->flags & MF_SHOOTABLE) && (thing->health & 1)) {
    flip = !flip;
  }

  // calculate edges of the shape
  // [crispy] fix sprite offsets for mirrored sprites
  tx -= flip ? g_r_state_globals->spritewidth[lump] - g_r_state_globals->spriteoffset[lump] : g_r_state_globals->spriteoffset[lump];
  x1 = (centerxfrac + FixedMul(tx, xscale)) >> FRACBITS;

  // off the right side?
  if (x1 > g_r_state_globals->viewwidth)
    return;

  tx += g_r_state_globals->spritewidth[lump];
  x2 = ((centerxfrac + FixedMul(tx, xscale)) >> FRACBITS) - 1;

  // off the left side
  if (x2 < 0)
    return;

  // [JN] killough 4/9/98: clip things which are out of view due to height
  gzt = interpz + g_r_state_globals->spritetopoffset[lump];
  if (interpz > g_r_state_globals->viewz + FixedDiv(g_r_state_globals->viewheight << FRACBITS, xscale) || gzt < g_r_state_globals->viewz - FixedDiv((g_r_state_globals->viewheight << FRACBITS) - g_r_state_globals->viewheight, xscale)) {
    return;
  }

  // [JN] quickly reject sprites with bad x ranges
  if (x1 >= x2) {
    return;
  }

  // store information in a vissprite
  vis              = R_NewVisSprite();
  vis->translation = nullptr; // [crispy] no color translation
  vis->mobjflags   = thing->flags;
  vis->scale       = xscale << detailshift;
  vis->gx          = interpx;
  vis->gy          = interpy;
  vis->gz          = interpz;
  vis->gzt         = gzt; // [JN] killough 3/27/98
  vis->texturemid  = gzt - g_r_state_globals->viewz;
  vis->x1          = x1 < 0 ? 0 : x1;
  vis->x2          = x2 >= g_r_state_globals->viewwidth ? g_r_state_globals->viewwidth - 1 : x2;
  iscale           = FixedDiv(FRACUNIT, xscale);

  if (flip) {
    vis->startfrac = g_r_state_globals->spritewidth[lump] - 1;
    vis->xiscale   = -iscale;
  } else {
    vis->startfrac = 0;
    vis->xiscale   = iscale;
  }

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale * (vis->x1 - x1);
  vis->patch = lump;

  // get light level
  if (thing->flags & MF_SHADOW) {
    // shadow draw
    vis->colormap[0] = vis->colormap[1] = nullptr;
  } else if (fixedcolormap) {
    // fixed map
    vis->colormap[0] = vis->colormap[1] = fixedcolormap;
  } else if (thing->frame & FF_FULLBRIGHT) {
    // full bright
    vis->colormap[0] = vis->colormap[1] = g_r_state_globals->colormaps;
  }

  else {
    // diminished light
    index = xscale >> (LIGHTSCALESHIFT - detailshift + crispy->hires);

    if (index >= MAXLIGHTSCALE)
      index = MAXLIGHTSCALE - 1;

    // [crispy] brightmaps for select sprites
    vis->colormap[0] = spritelights[index];
    vis->colormap[1] = scalelight[LIGHTLEVELS - 1][MAXLIGHTSCALE - 1];
  }
  vis->brightmap = R_BrightmapForSprite(thing->sprite);

  // [crispy] colored blood
  if (crispy->coloredblood && (thing->type == MT_BLOOD || thing->state - states == S_GIBS) && thing->target) {
    // [crispy] Thorn Things in Hacx bleed green blood
    if (g_doomstat_globals->gamemission == pack_hacx) {
      if (thing->target->type == MT_BABY) {
        vis->translation = cr_colors[static_cast<int>(cr_t::CR_RED2GREEN)];
      }
    } else {
      // [crispy] Barons of Hell and Hell Knights bleed green blood
      if (thing->target->type == MT_BRUISER || thing->target->type == MT_KNIGHT) {
        vis->translation = cr_colors[static_cast<int>(cr_t::CR_RED2GREEN)];
      } else
        // [crispy] Cacodemons bleed blue blood
        if (thing->target->type == MT_HEAD) {
          vis->translation = cr_colors[static_cast<int>(cr_t::CR_RED2BLUE)];
        }
    }
  }

#ifdef CRISPY_TRUECOLOR
  // [crispy] translucent sprites
  if (thing->flags & MF_TRANSLUCENT) {
    vis->blendfunc = (thing->frame & FF_FULLBRIGHT) ? I_BlendAdd : I_BlendOver;
  }
#endif
}

extern void P_LineLaser(mobj_t * t1, angle_t angle, fixed_t distance, fixed_t slope);

uint8_t * R_LaserspotColor() {
  if (crispy->crosshairtarget) {
    // [crispy] the projected crosshair code calls P_LineLaser() itself
    if (crispy->crosshair == CROSSHAIR_STATIC) {
      P_LineLaser(g_r_state_globals->viewplayer->mo, g_r_state_globals->viewangle, 16 * 64 * FRACUNIT, PLAYER_SLOPE(g_r_state_globals->viewplayer));
    }
    if (g_p_local_globals->linetarget) {
      return cr_colors[static_cast<int>(cr_t::CR_GRAY)];
    }
  }

  // [crispy] keep in sync with st_stuff.c:ST_WidgetColor(hudcolor_health)
  if (crispy->crosshairhealth) {
    const int health = g_r_state_globals->viewplayer->health;

    // [crispy] Invulnerability powerup and God Mode cheat turn Health values gray
    if (g_r_state_globals->viewplayer->cheats & CF_GODMODE || g_r_state_globals->viewplayer->powers[pw_invulnerability])
      return cr_colors[static_cast<int>(cr_t::CR_GRAY)];
    else if (health < 25)
      return cr_colors[static_cast<int>(cr_t::CR_RED)];
    else if (health < 50)
      return cr_colors[static_cast<int>(cr_t::CR_GOLD)];
    else if (health <= 100)
      return cr_colors[static_cast<int>(cr_t::CR_GREEN)];
    else
      return cr_colors[static_cast<int>(cr_t::CR_BLUE)];
  }

  return nullptr;
}

// [crispy] generate a vissprite for the laser spot
static void R_DrawLSprite() {
  fixed_t       xscale;
  fixed_t       tx, tz;
  vissprite_t * vis;

  static int       lump;
  static patch_t * patch;

  if (weaponinfo[g_r_state_globals->viewplayer->readyweapon].ammo == am_noammo || g_r_state_globals->viewplayer->playerstate != PST_LIVE)
    return;

  if (lump != laserpatch[crispy->crosshairtype].l) {
    lump  = laserpatch[crispy->crosshairtype].l;
    patch = cache_lump_num<patch_t *>(lump, PU_STATIC);
  }

  P_LineLaser(g_r_state_globals->viewplayer->mo, g_r_state_globals->viewangle, 16 * 64 * FRACUNIT, PLAYER_SLOPE(g_r_state_globals->viewplayer));

  if (action_hook_is_empty(laserspot->thinker.function))
    return;

  tz = FixedMul(laserspot->x - g_r_state_globals->viewx, viewcos) + FixedMul(laserspot->y - g_r_state_globals->viewy, viewsin);

  if (tz < MINZ)
    return;

  xscale = FixedDiv(projection, tz);
  // [crispy] the original patch has 5x5 pixels, cap the projection at 20x20
  xscale = (xscale > 4 * FRACUNIT) ? 4 * FRACUNIT : xscale;

  tx = -(FixedMul(laserspot->y - g_r_state_globals->viewy, viewcos) - FixedMul(laserspot->x - g_r_state_globals->viewx, viewsin));

  if (std::abs(tx) > (tz << 2))
    return;

  vis = R_NewVisSprite();
  std::memset(vis, 0, sizeof(*vis));                                                                  // [crispy] set all fields to nullptr, except ...
  vis->patch       = lump - g_r_state_globals->firstspritelump;                                       // [crispy] not a sprite patch
  vis->colormap[0] = vis->colormap[1] = fixedcolormap ? fixedcolormap : g_r_state_globals->colormaps; // [crispy] always full brightness
  vis->brightmap                      = g_r_draw_globals->dc_brightmap;
  vis->translation                    = R_LaserspotColor();
#ifdef CRISPY_TRUECOLOR
  vis->mobjflags |= MF_TRANSLUCENT;
  vis->blendfunc = I_BlendAdd;
#endif
  vis->xiscale    = FixedDiv(FRACUNIT, xscale);
  vis->texturemid = laserspot->z - g_r_state_globals->viewz;
  vis->scale      = xscale << detailshift;

  tx -= SHORT(patch->width / 2) << FRACBITS;
  vis->x1 = (centerxfrac + FixedMul(tx, xscale)) >> FRACBITS;
  tx += SHORT(patch->width) << FRACBITS;
  vis->x2 = ((centerxfrac + FixedMul(tx, xscale)) >> FRACBITS) - 1;

  if (vis->x1 < 0 || vis->x1 >= g_r_state_globals->viewwidth || vis->x2 < 0 || vis->x2 >= g_r_state_globals->viewwidth)
    return;

  R_DrawVisSprite(vis, vis->x1, vis->x2);
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites(sector_t * sec) {
  mobj_t * thing;
  int      lightnum;

  // BSP is traversed by subsector.
  // A sector might have been split into several
  //  subsectors during BSP building.
  // Thus we check whether its already added.
  if (sec->validcount == validcount)
    return;

  // Well, now it will be done.
  sec->validcount = validcount;

  lightnum = (sec->lightlevel >> LIGHTSEGSHIFT) + (extralight * LIGHTBRIGHT);

  if (lightnum < 0)
    spritelights = scalelight[0];
  else if (lightnum >= LIGHTLEVELS)
    spritelights = scalelight[LIGHTLEVELS - 1];
  else
    spritelights = scalelight[lightnum];

  // Handle all things in sector.
  for (thing = sec->thinglist; thing; thing = thing->snext)
    R_ProjectSprite(thing);
}

//
// R_DrawPSprite
//
void R_DrawPSprite(pspdef_t * psp, psprnum_t psprnum) // [crispy] differentiate gun from flash sprites
{
  fixed_t         tx;
  int             x1;
  int             x2;
  spritedef_t *   sprdef;
  spriteframe_t * sprframe;
  int             lump;
  bool            flip;
  vissprite_t *   vis;
  vissprite_t     avis;

  // decide which patch to use
#ifdef RANGECHECK
  if (static_cast<unsigned>(psp->state->sprite) >= static_cast<unsigned int>(g_r_state_globals->numsprites))
    I_Error("R_ProjectSprite: invalid sprite number %i ",
            psp->state->sprite);
#endif
  sprdef = &g_r_state_globals->sprites[psp->state->sprite];
  // [crispy] the TNT1 sprite is not supposed to be rendered anyway
  if (!sprdef->numframes && psp->state->sprite == SPR_TNT1) {
    return;
  }
#ifdef RANGECHECK
  if ((psp->state->frame & FF_FRAMEMASK) >= sprdef->numframes)
    I_Error("R_ProjectSprite: invalid sprite frame %i : %i ",
            psp->state->sprite,
            psp->state->frame);
#endif
  sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

  lump = sprframe->lump[0];
  flip = static_cast<bool>(sprframe->flip[0] ^ static_cast<uint8_t>(crispy->flipweapons));

  // calculate edges of the shape
  tx = psp->sx2 - (ORIGWIDTH / 2) * FRACUNIT;

  // [crispy] fix sprite offsets for mirrored sprites
  tx -= flip ? 2 * tx - g_r_state_globals->spriteoffset[lump] + g_r_state_globals->spritewidth[lump] : g_r_state_globals->spriteoffset[lump];
  x1 = (centerxfrac + FixedMul(tx, pspritescale)) >> FRACBITS;

  // off the right side
  if (x1 > g_r_state_globals->viewwidth)
    return;

  tx += g_r_state_globals->spritewidth[lump];
  x2 = ((centerxfrac + FixedMul(tx, pspritescale)) >> FRACBITS) - 1;

  // off the left side
  if (x2 < 0)
    return;

  // store information in a vissprite
  vis              = &avis;
  vis->translation = nullptr; // [crispy] no color translation
  vis->mobjflags   = 0;
  // [crispy] weapons drawn 1 pixel too high when player is idle
  vis->texturemid = (BASEYCENTER << FRACBITS) + FRACUNIT / 4 - (psp->sy2 + std::abs(psp->dy) - g_r_state_globals->spritetopoffset[lump]);
  vis->x1         = x1 < 0 ? 0 : x1;
  vis->x2         = x2 >= g_r_state_globals->viewwidth ? g_r_state_globals->viewwidth - 1 : x2;
  vis->scale      = pspritescale << detailshift;

  if (flip) {
    vis->xiscale   = -pspriteiscale;
    vis->startfrac = g_r_state_globals->spritewidth[lump] - 1;
  } else {
    vis->xiscale   = pspriteiscale;
    vis->startfrac = 0;
  }

  // [crispy] free look
  vis->texturemid += FixedMul(((centery - g_r_state_globals->viewheight / 2) << FRACBITS), pspriteiscale) >> detailshift;

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale * (vis->x1 - x1);

  vis->patch = lump;

  if (g_r_state_globals->viewplayer->powers[pw_invisibility] > 4 * 32
      || g_r_state_globals->viewplayer->powers[pw_invisibility] & 8) {
    // shadow draw
    vis->colormap[0] = vis->colormap[1] = nullptr;
  } else if (fixedcolormap) {
    // fixed color
    vis->colormap[0] = vis->colormap[1] = fixedcolormap;
  } else if (psp->state->frame & FF_FULLBRIGHT) {
    // full bright
    vis->colormap[0] = vis->colormap[1] = g_r_state_globals->colormaps;
  } else {
    // local light
    vis->colormap[0] = spritelights[MAXLIGHTSCALE - 1];
    vis->colormap[1] = scalelight[LIGHTLEVELS - 1][MAXLIGHTSCALE - 1];
  }
  vis->brightmap = R_BrightmapForState(static_cast<int>(psp->state - states));

  // [crispy] translucent gun flash sprites
  if (psprnum == ps_flash) {
    vis->mobjflags |= MF_TRANSLUCENT;
#ifdef CRISPY_TRUECOLOR
    vis->blendfunc = I_BlendOver; // I_BlendAdd;
#endif
  }

  R_DrawVisSprite(vis, vis->x1, vis->x2);
}

//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites() {
  int        i;
  int        lightnum;
  pspdef_t * psp;

  // get light level
  lightnum =
      (g_r_state_globals->viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT)
      + (extralight * LIGHTBRIGHT);

  if (lightnum < 0)
    spritelights = scalelight[0];
  else if (lightnum >= LIGHTLEVELS)
    spritelights = scalelight[LIGHTLEVELS - 1];
  else
    spritelights = scalelight[lightnum];

  // clip to screen bounds
  mfloorclip   = screenheightarray;
  mceilingclip = negonearray;

  if (crispy->crosshair == CROSSHAIR_PROJECTED)
    R_DrawLSprite();

  // add all active psprites
  for (i = 0, psp = g_r_state_globals->viewplayer->psprites;
       i < NUMPSPRITES;
       i++, psp++) {
    if (psp->state)
      R_DrawPSprite(psp, static_cast<psprnum_t>(i)); // [crispy] pass gun or flash sprite
  }
}

//
// R_SortVisSprites
//
#ifdef HAVE_QSORT
// [crispy] use stdlib's qsort() function for sorting the vissprites[] array
static inline int cmp_vissprites(const void * a, const void * b) {
  const auto * vsa = static_cast<const vissprite_t *>(a);
  const auto * vsb = static_cast<const vissprite_t *>(b);

  const int ret = vsa->scale - vsb->scale;

  return static_cast<int>(ret ? ret : vsa->next - vsb->next);
}

void R_SortVisSprites() {
  int count = static_cast<int>(vissprite_p - vissprites);

  if (!count)
    return;

  // [crispy] maintain a stable sort for deliberately overlaid sprites
  for (vissprite_t * ds = vissprites; ds < vissprite_p; ds++) {
    ds->next = ds + 1;
  }

  qsort(vissprites, static_cast<size_t>(count), sizeof(*vissprites), cmp_vissprites);
}
#else
vissprite_t vsprsortedhead;

void R_SortVisSprites() {
  int           count;
  vissprite_t * ds;
  vissprite_t * best;
  vissprite_t   unsorted;
  fixed_t       bestscale;

  count = vissprite_p - vissprites;

  unsorted.next = unsorted.prev = &unsorted;

  if (!count)
    return;

  for (ds = vissprites; ds < vissprite_p; ds++) {
    ds->next = ds + 1;
    ds->prev = ds - 1;
  }

  vissprites[0].prev      = &unsorted;
  unsorted.next           = &vissprites[0];
  (vissprite_p - 1)->next = &unsorted;
  unsorted.prev           = vissprite_p - 1;

  // pull the vissprites out by scale

  vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
  for (int i = 0; i < count; i++) {
    bestscale = INT_MAX;
    best      = unsorted.next;
    for (ds = unsorted.next; ds != &unsorted; ds = ds->next) {
      if (ds->scale < bestscale) {
        bestscale = ds->scale;
        best      = ds;
      }
    }
    best->next->prev          = best->prev;
    best->prev->next          = best->next;
    best->next                = &vsprsortedhead;
    best->prev                = vsprsortedhead.prev;
    vsprsortedhead.prev->next = best;
    vsprsortedhead.prev       = best;
  }
}
#endif

//
// R_DrawSprite
//
void R_DrawSprite(vissprite_t * spr) {
  drawseg_t * ds;
  int         clipbot[MAXWIDTH]; // [crispy] 32-bit integer math
  int         cliptop[MAXWIDTH]; // [crispy] 32-bit integer math
  int         x;
  int         r1;
  int         r2;
  fixed_t     scale;
  fixed_t     lowscale;
  int         silhouette;

  for (x = spr->x1; x <= spr->x2; x++)
    clipbot[x] = cliptop[x] = -2;

  // Scan drawsegs from end to start for obscuring segs.
  // The first drawseg that has a greater scale
  //  is the clip seg.
  for (ds = ds_p - 1; ds >= drawsegs; ds--) {
    // determine if the drawseg obscures the sprite
    if (ds->x1 > spr->x2
        || ds->x2 < spr->x1
        || (!ds->silhouette
            && !ds->maskedtexturecol)) {
      // does not cover sprite
      continue;
    }

    r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
    r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

    if (ds->scale1 > ds->scale2) {
      lowscale = ds->scale2;
      scale    = ds->scale1;
    } else {
      lowscale = ds->scale1;
      scale    = ds->scale2;
    }

    if (scale < spr->scale
        || (lowscale < spr->scale
            && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline))) {
      // masked mid texture?
      if (ds->maskedtexturecol)
        R_RenderMaskedSegRange(ds, r1, r2);
      // seg is behind sprite
      continue;
    }

    // clip this piece of the sprite
    silhouette = ds->silhouette;

    if (spr->gz >= ds->bsilheight)
      silhouette &= ~SIL_BOTTOM;

    if (spr->gzt <= ds->tsilheight)
      silhouette &= ~SIL_TOP;

    if (silhouette == 1) {
      // bottom sil
      for (x = r1; x <= r2; x++)
        if (clipbot[x] == -2)
          clipbot[x] = ds->sprbottomclip[x];
    } else if (silhouette == 2) {
      // top sil
      for (x = r1; x <= r2; x++)
        if (cliptop[x] == -2)
          cliptop[x] = ds->sprtopclip[x];
    } else if (silhouette == 3) {
      // both
      for (x = r1; x <= r2; x++) {
        if (clipbot[x] == -2)
          clipbot[x] = ds->sprbottomclip[x];
        if (cliptop[x] == -2)
          cliptop[x] = ds->sprtopclip[x];
      }
    }
  }

  // all clipping has been performed, so draw the sprite

  // check for unclipped columns
  for (x = spr->x1; x <= spr->x2; x++) {
    if (clipbot[x] == -2)
      clipbot[x] = g_r_state_globals->viewheight;

    if (cliptop[x] == -2)
      cliptop[x] = -1;
  }

  mfloorclip   = clipbot;
  mceilingclip = cliptop;
  R_DrawVisSprite(spr, spr->x1, spr->x2);
}

//
// R_DrawMasked
//
void R_DrawMasked() {
  vissprite_t * spr;
  drawseg_t *   ds;

  R_SortVisSprites();

  if (vissprite_p > vissprites) {
    // draw all vissprites back to front
#ifdef HAVE_QSORT
    for (spr = vissprites;
         spr < vissprite_p;
         spr++)
#else
    for (spr = vsprsortedhead.next;
         spr != &vsprsortedhead;
         spr = spr->next)
#endif
    {

      R_DrawSprite(spr);
    }
  }

  // render any remaining masked mid textures
  for (ds = ds_p - 1; ds >= drawsegs; ds--)
    if (ds->maskedtexturecol)
      R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

  if (crispy->cleanscreenshot == 2)
    return;

  // draw the psprites on top of everything
  //  but does not draw on side views
  if (!g_doomstat_globals->viewangleoffset)
    R_DrawPlayerSprites();
}
