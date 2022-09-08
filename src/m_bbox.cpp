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
//	Main loop menu stuff.
//	Random number LUT.
//	Default Config File.
//	PCX Screenshots.
//

#include <cstdint>
#include <limits>

#include "m_bbox.hpp"

void bounding_box_t::clear() {
  set(box_e::top, std::numeric_limits<int32_t>::min());
  set(box_e::right, std::numeric_limits<int32_t>::min());
  set(box_e::bottom, std::numeric_limits<int32_t>::max());
  set(box_e::left, std::numeric_limits<int32_t>::max());
}

void bounding_box_t::set(const box_e coord, const fixed_t value) {
  box[static_cast<fixed_t>(coord)] = value;
}

fixed_t bounding_box_t::get(const box_e coord) const {
  return box[static_cast<fixed_t>(coord)];
}

void bounding_box_t::add(const fixed_t x, const fixed_t y) {
  if (x < get(box_e::left))
    set(box_e::left, x);
  else if (x > get(box_e::right))
    set(box_e::right, x);
  if (y < get(box_e::bottom))
    set(box_e::bottom, y);
  else if (y > get(box_e::top))
    set(box_e::top, y);
}
