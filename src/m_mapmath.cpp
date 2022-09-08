//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2005-2006 Andrey Budko
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
//  Map related math functions
//

#include "m_mapmath.hpp"

#include <cmath>

namespace map {

//
// P_AproxDistance
// Gives an estimation of distance (not exact)
//
fixed_t approx_distance(fixed_t dx, fixed_t dy) {
  dx = std::abs(dx);
  dy = std::abs(dy);
  if (dx < dy)
    return dx + dy - (dx >> 1);
  return dx + dy - (dy >> 1);
}

//
// P_PointOnLineSide
//
bool point_on_line_side(fixed_t x,
                        fixed_t y,
                        fixed_t line_dx,
                        fixed_t line_dy,
                        fixed_t line_x,
                        fixed_t line_y) {
  if (!line_dx) {
    if (x <= line_x)
      return line_dy > 0;
    return line_dy < 0;
  }

  if (!line_dy) {
    if (y <= line_y)
      return line_dx < 0;
    return line_dx > 0;
  }

  const fixed_t dx = (x - line_x);
  const fixed_t dy = (y - line_y);

  const fixed_t left  = FixedMul(line_dy >> FRACBITS, dx);
  const fixed_t right = FixedMul(dy, line_dx >> FRACBITS);

  if (right < left)
    return false; // front side
  return true;    // back side
}

}