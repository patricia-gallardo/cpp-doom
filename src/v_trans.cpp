//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014 Fabian Greffrath, Paul Haeberli
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
//
// Color translation tables
//

#include <cmath>

#include "doomtype.hpp"
#include "v_trans.hpp"

// [crispy] here used to be static color translation tables based on
// the ones found in Boom and MBF. Nowadays these are recalculated
// by means of actual color space conversions in r_data:R_InitColormaps().

// this one will be the identity matrix
static uint8_t cr_none[256];
// this one will be the ~50% darker matrix
static uint8_t cr_dark[256];
static uint8_t cr_gray[256];
static uint8_t cr_green[256];
static uint8_t cr_gold[256];
static uint8_t cr_red[256];
static uint8_t cr_blue[256];

// clang-format off
static const uint8_t cr_red2blue[256] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 207, 207, 46, 207,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 207, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };

static const uint8_t cr_red2green[256] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 127, 127, 46, 127,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 127, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 126, 127, 127,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };
// clang-format on

uint8_t * cr_colors[9] = {
  reinterpret_cast<uint8_t *>(&cr_none),
  reinterpret_cast<uint8_t *>(&cr_dark),
  reinterpret_cast<uint8_t *>(&cr_gray),
  reinterpret_cast<uint8_t *>(&cr_green),
  reinterpret_cast<uint8_t *>(&cr_gold),
  reinterpret_cast<uint8_t *>(&cr_red),
  reinterpret_cast<uint8_t *>(&cr_blue),
  const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(&cr_red2blue)),
  const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(&cr_red2green))
};

char ** crstr = 0;

/*
Date: Sun, 26 Oct 2014 10:36:12 -0700
From: paul haeberli <paulhaeberli@yahoo.com>
Subject: Re: colors and color conversions
To: Fabian Greffrath <fabian@greffrath.com>

Yes, this seems exactly like the solution I was looking for. I just
couldn't find code to do the HSV->RGB conversion. Speaking of the code,
would you allow me to use this code in my software? The Doom source code
is licensed under the GNU GPL, so this code yould have to be under a
compatible license.

    Yes. I'm happy to contribute this code to your project.  GNU GPL or anything
    compatible sounds fine.

Regarding the conversions, the procedure you sent me will leave grays
(r=g=b) untouched, no matter what I set as HUE, right? Is it possible,
then, to also use this routine to convert colors *to* gray?

    You can convert any color to an equivalent grey by setting the saturation
    to 0.0


    - Paul Haeberli
*/

constexpr auto CTOLERANCE = (0.0001);

struct vect {
  float x;
  float y;
  float z;
};

static void hsv_to_rgb(vect * hsv, vect * rgb) {
  float h = hsv->x;
  float s = hsv->y;
  float v = hsv->z;
  h *= static_cast<float>(360.0);
  if (s < CTOLERANCE) {
    rgb->x = v;
    rgb->y = v;
    rgb->z = v;
  } else {
    if (h >= 360.0)
      h -= static_cast<float>(360.0);
    h /= static_cast<float>(60.0);
    int   i = static_cast<int>(floor(h));
    float f = h - static_cast<float>(i);
    auto  p = static_cast<float>(v * (1.0 - s));
    auto  q = static_cast<float>(v * (1.0 - (s * f)));
    auto  t = static_cast<float>(v * (1.0 - (s * (1.0 - f))));
    switch (i) {
    case 0:
      rgb->x = v;
      rgb->y = t;
      rgb->z = p;
      break;
    case 1:
      rgb->x = q;
      rgb->y = v;
      rgb->z = p;
      break;
    case 2:
      rgb->x = p;
      rgb->y = v;
      rgb->z = t;
      break;
    case 3:
      rgb->x = p;
      rgb->y = q;
      rgb->z = v;
      break;
    case 4:
      rgb->x = t;
      rgb->y = p;
      rgb->z = v;
      break;
    case 5:
      rgb->x = v;
      rgb->y = p;
      rgb->z = q;
      break;
    }
  }
}

static void rgb_to_hsv(vect * rgb, vect * hsv) {
  float h = 0.0;
  float s = 0.0;

  float r = rgb->x;
  float g = rgb->y;
  float b = rgb->z;
  /* find the cmax and cmin of r g b */
  float cmax = r;
  float cmin = r;
  cmax       = (g > cmax ? g : cmax);
  cmin       = (g < cmin ? g : cmin);
  cmax       = (b > cmax ? b : cmax);
  cmin       = (b < cmin ? b : cmin);
  float v    = cmax; /* value */
  if (cmax > CTOLERANCE)
    s = (cmax - cmin) / cmax;
  else {
    s = 0.0;
    h = 0.0;
  }
  if (s < CTOLERANCE)
    h = 0.0;
  else {
    float cdelta = cmax - cmin;
    float rc     = (cmax - r) / cdelta;
    float gc     = (cmax - g) / cdelta;
    float bc     = (cmax - b) / cdelta;
    if (r == cmax)
      h = bc - gc;
    else if (g == cmax)
      h = static_cast<float>(2.0 + rc - bc);
    else
      h = static_cast<float>(4.0 + gc - rc);
    h = static_cast<float>(h * 60.0);
    if (h < 0.0)
      h += static_cast<float>(360.0);
  }
  hsv->x = static_cast<float>(h / 360.0);
  hsv->y = s;
  hsv->z = v;
}

// [crispy] copied over from i_video.c
static int I_GetPaletteIndex2(uint8_t * palette, int r, int g, int b) {
  int best      = 0;
  int best_diff = std::numeric_limits<int32_t>::max();

  for (int i = 0; i < 256; ++i) {
    int diff = (r - palette[3 * i + 0]) * (r - palette[3 * i + 0])
               + (g - palette[3 * i + 1]) * (g - palette[3 * i + 1])
               + (b - palette[3 * i + 2]) * (b - palette[3 * i + 2]);

    if (diff < best_diff) {
      best      = i;
      best_diff = diff;
    }

    if (diff == 0) {
      break;
    }
  }

  return best;
}

[[maybe_unused]] uint8_t V_Colorize(uint8_t * playpal, int cr, uint8_t source, bool keepgray109) {
  vect rgb, hsv;

  // [crispy] preserve gray drop shadow in IWAD status bar numbers
  if (cr == static_cast<int>(cr_t::CR_NONE) || (keepgray109 && source == 109))
    return source;

  rgb.x = static_cast<float>(playpal[3 * source + 0] / 255.);
  rgb.y = static_cast<float>(playpal[3 * source + 1] / 255.);
  rgb.z = static_cast<float>(playpal[3 * source + 2] / 255.);

  rgb_to_hsv(&rgb, &hsv);

  if (cr == static_cast<int>(cr_t::CR_DARK))
    hsv.z *= static_cast<float>(0.5);
  else if (cr == static_cast<int>(cr_t::CR_GRAY))
    hsv.y = 0;
  else {
    // [crispy] hack colors to full saturation
    hsv.y = 1.0;

    if (cr == static_cast<int>(cr_t::CR_GREEN)) {
      //	    hsv.x = 135./360.;
      hsv.x = static_cast<float>((150. * hsv.z + 120. * (1. - hsv.z)) / 360.);
    } else if (cr == static_cast<int>(cr_t::CR_GOLD)) {
      //	    hsv.x = 45./360.;
      //	    hsv.x = (50. * hsv.z + 30. * (1. - hsv.z))/360.;
      hsv.x = static_cast<float>((7.0 + 53. * hsv.z) / 360.);
      hsv.y = static_cast<float>(1.0 - 0.4 * hsv.z);
      hsv.z = static_cast<float>(0.2 + 0.8 * hsv.z);
    } else if (cr == static_cast<int>(cr_t::CR_RED)) {
      hsv.x = 0.;
    } else if (cr == static_cast<int>(cr_t::CR_BLUE)) {
      hsv.x = static_cast<float>(240. / 360.);
    }
  }

  hsv_to_rgb(&hsv, &rgb);

  rgb.x *= static_cast<float>(255.);
  rgb.y *= static_cast<float>(255.);
  rgb.z *= static_cast<float>(255.);

  int retval = I_GetPaletteIndex2(playpal, static_cast<int>(rgb.x), static_cast<int>(rgb.y), static_cast<int>(rgb.z));
  return static_cast<uint8_t>(retval);
}
