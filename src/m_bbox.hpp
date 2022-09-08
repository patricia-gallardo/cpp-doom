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
//    Nil.
//

#pragma once

#include <climits>

#include "m_fixed.hpp"

// Bounding box coordinate storage.
enum class box_e
{
  top,
  bottom,
  left,
  right
}; // bbox coordinates

struct bounding_box_t {
  fixed_t box[4] {};

  void clear();
  void set(const box_e coord, const fixed_t value);
  fixed_t get(const box_e coord) const;
  void add(const fixed_t x, const fixed_t y);
};
