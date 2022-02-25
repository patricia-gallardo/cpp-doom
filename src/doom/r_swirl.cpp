//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2000, 2005-2014 Simon Howard
// Copyright(C) 2019 Fabian Greffrath
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
//	[crispy] add support for SMMU swirling flats
//

// [crispy] adapted from smmu/r_ripple.c, by Simon Howard

#include <tables.hpp>

#include <i_system.hpp>
#include <w_wad.hpp>
#include <z_zone.hpp>

#include "doomstat.hpp"
#include "lump.hpp"

// swirl factors determine the number of waves per flat width

// 1 cycle per 64 units
constexpr auto swirlfactor = (8192 / 64);

// 1 cycle per 32 units (2 in 64)
constexpr auto swirlfactor2 = (8192 / 32);

constexpr auto SEQUENCE = 1024;
constexpr auto FLATSIZE = (64 * 64);

static int * offsets;
static int * offset;

constexpr auto AMP   = 2;
constexpr auto AMP2  = 2;
constexpr auto SPEED = 40;

void R_InitDistortedFlats() {
  if (!offsets) {
    offsets = static_cast<decltype(offsets)>(I_Realloc(nullptr, SEQUENCE * FLATSIZE * sizeof(*offsets)));
    offset  = offsets;

    for (int i = 0; i < SEQUENCE; i++) {
      for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
          int sinvalue  = (y * swirlfactor + i * SPEED * 5 + 900) & 8191;
          int sinvalue2 = (x * swirlfactor2 + i * SPEED * 4 + 300) & 8191;
          int x1        = x + 128
                   + ((finesine[sinvalue] * AMP) >> FRACBITS)
                   + ((finesine[sinvalue2] * AMP2) >> FRACBITS);

          sinvalue  = (x * swirlfactor + i * SPEED * 3 + 700) & 8191;
          sinvalue2 = (y * swirlfactor2 + i * SPEED * 4 + 1200) & 8191;
          int y1    = y + 128
                   + ((finesine[sinvalue] * AMP) >> FRACBITS)
                   + ((finesine[sinvalue2] * AMP2) >> FRACBITS);

          x1 &= 63;
          y1 &= 63;

          offset[(y << 6) + x] = (y1 << 6) + x1;
        }
      }

      offset += FLATSIZE;
    }
  }
}

char * R_DistortedFlat(int flatnum) {
  static int  swirltic  = -1;
  static int  swirlflat = -1;
  static char distortedflat[FLATSIZE];

  if (swirltic != leveltime) {
    offset = offsets + ((leveltime & (SEQUENCE - 1)) * FLATSIZE);

    swirltic  = leveltime;
    swirlflat = -1;
  }

  if (swirlflat != flatnum) {
    auto * normalflat = cache_lump_num<char *>(flatnum, PU_STATIC);

    for (int i = 0; i < FLATSIZE; i++) {
      distortedflat[i] = normalflat[offset[i]];
    }

    W_ReleaseLumpNum(flatnum);

    swirlflat = flatnum;
  }

  return distortedflat;
}
