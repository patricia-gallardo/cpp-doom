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
//  Sky rendering. The DOOM sky is a texture map like any
//  wall, wrapping around. A 1024 columns equal 360 degrees.
//  The default sky map is 256 columns and repeats 4 times
//  on a 320 screen?
//
//

// Needed for FRACUNIT.
#include "m_fixed.hpp"

// Needed for Flat retrieval.
#include "r_data.hpp"

#include "r_sky.hpp"

//
// sky mapping
//
int skytexture = -1; // [crispy] initialize
int skytexturemid;

constexpr int normal_textureid { ORIGHEIGHT / 2 * FRACUNIT };
constexpr int stretched_texture_multiplier { -28 * FRACUNIT };

//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap() {
  // skyflatnum = R_FlatNumForName ( SKYFLATNAME );
  // [crispy] stretch sky
  if (skytexture == -1) {
    return;
  }

  // cpp-doom: should be == ?
  if ((crispy->stretchsky = crispy->freelook || crispy->mouselook || crispy->pitch)) {
    skytexturemid = stretched_texture_multiplier * (g_r_state_globals->textureheight[skytexture] >> FRACBITS) / SKYSTRETCH_HEIGHT;
  } else {
    skytexturemid = normal_textureid;
  }
}
