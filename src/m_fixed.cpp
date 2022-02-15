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
//	Fixed point implementation.
//

#include <cstdlib>

#include "doomtype.hpp"
#include "m_fixed.hpp"

// Fixme. __USE_C_FIXED__ or something.

fixed_t FixedMul(fixed_t a, fixed_t b)
{
    return static_cast<fixed_t>((static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> FRACBITS);
}


//
// FixedDiv, C version.
//

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((std::abs(a) >> 14) >= std::abs(b))
    {
        return (a ^ b) < 0 ? INT_MIN : INT_MAX;
    }
    else
    {
        int64_t result;

        result = (static_cast<int64_t>(a) << FRACBITS) / b;

        return static_cast<fixed_t>(result);
    }
}
