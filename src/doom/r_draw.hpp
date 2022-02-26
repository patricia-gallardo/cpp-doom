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
//	System specific interface stuff.
//

#pragma once

// The span blitting interface.
// Hook in assembler or system specific BLT
//  here.
void R_DrawColumn();
void R_DrawColumnLow();

// The Spectre/Invisibility effect.
void R_DrawFuzzColumn();
void R_DrawFuzzColumnLow();

// [crispy] draw fuzz effect independent of rendering frame rate
void R_SetFuzzPosTic();
void R_SetFuzzPosDraw();

// Draw with color translation tables,
//  for player sprite rendering,
//  Green/Red/Blue/Indigo shirts.
void R_DrawTranslatedColumn();
void R_DrawTranslatedColumnLow();

void R_DrawTLColumn();
void R_DrawTLColumnLow();

void R_VideoErase(unsigned ofs,
                  int      count);

// Span blitting for rows, floor/ceiling.
// No Sepctre effect needed.
void R_DrawSpan();

// Low resolution mode, 160x200?
void R_DrawSpanLow();

void R_InitBuffer(int width,
                  int height);

// Initialize color translation tables,
//  for player rendering etc.
void R_InitTranslationTables();

// Rendering function.
void R_FillBackScreen();

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder();

struct r_draw_t {
  //
  // R_DrawColumn
  // Source is the top of the column to scale.
  //
  lighttable_t * dc_colormap[2]; // [crispy] brightmaps
  int            dc_x;
  int            dc_yl;
  int            dc_yh;
  fixed_t        dc_iscale;
  fixed_t        dc_texturemid;
  int            dc_texheight; // [crispy] Tutti-Frutti fix
  uint8_t *      dc_brightmap;

  // first pixel in a column (possibly virtual)
  uint8_t * dc_source;

  //
  // R_DrawSpan
  // With DOOM style restrictions on view orientation,
  //  the floors and ceilings consist of horizontal slices
  //  or spans with constant z depth.
  // However, rotation around the world z axis is possible,
  //  thus this mapping, while simpler and faster than
  //  perspective correct texture mapping, has to traverse
  //  the texture at an angle in all but a few cases.
  // In consequence, flats are not stored by column (like walls),
  //  and the inner loop has to step in texture space u and v.
  //
  int ds_y;
  int ds_x1;
  int ds_x2;

  lighttable_t * ds_colormap[2];
  uint8_t *      ds_brightmap;

  fixed_t ds_xfrac;
  fixed_t ds_yfrac;
  fixed_t ds_xstep;
  fixed_t ds_ystep;

  // start of a 64*64 tile image
  uint8_t * ds_source;

  //
  // R_DrawTranslatedColumn
  // Used to draw player sprites
  //  with the green colorramp mapped to others.
  // Could be used with different translation
  //  tables, e.g. the lighter colored version
  //  of the BaronOfHell, the HellKnight, uses
  //  identical sprites, kinda brightened up.
  //
  uint8_t * translationtables;
  uint8_t * dc_translation;
};

extern r_draw_t * const g_r_draw_globals;
