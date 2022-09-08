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
//    Bounding box
//

#pragma once

#include <climits>

#include "m_fixed.hpp"

// cppdoom todo, add namespace.

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

  void    clear();
  bool    is_outside(const bounding_box_t & other) const;
  void    set(const box_e coord, const fixed_t value);
  void    set_vertex(const fixed_t v1x, const fixed_t v1y, const fixed_t v2x, const fixed_t v2y);
  fixed_t get(const box_e coord) const;
  void    add(const fixed_t x, const fixed_t y);
};
