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
//	Fixed point arithemtics, implementation.
//

#ifndef __M_FIXED__
#define __M_FIXED__

//
// Fixed point, 32bit as 16.16.
//
constexpr auto FRACBITS = 16;
constexpr auto FRACUNIT = (1 << FRACBITS);
template <typename T>
constexpr auto FIXED2DOUBLE(T x) {
  return (x / static_cast<double>(FRACUNIT));
}

using fixed_t = int;

fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);

#endif
