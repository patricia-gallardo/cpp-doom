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

#pragma once

#include "m_fixed.hpp"

namespace map {
fixed_t approx_distance(fixed_t dx, fixed_t dy);
bool    point_on_line_side(fixed_t x, fixed_t y, fixed_t line_dx, fixed_t line_dy, fixed_t line_x, fixed_t line_y);
}
