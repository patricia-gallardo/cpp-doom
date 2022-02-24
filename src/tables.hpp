//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//	Lookup tables.
//	Do not try to look them up :-).
//	In the order of appearance:
//
//	int finetangent[4096]	- Tangens LUT.
//	 Should work with BAM fairly well (12 of 16bit,
//      effectively, by shifting).
//
//	int finesine[10240]		- Sine lookup.
//	 Guess what, serves as cosine, too.
//	 Remarkable thing is, how to use BAMs with this?
//
//	int tantoangle[2049]	- ArcTan LUT,
//	  maps tan(angle) to angle fast. Gotta search.
//

#ifndef __TABLES__
#define __TABLES__

#include "doomtype.hpp"

#include "m_fixed.hpp"

#define FINEANGLES 8192
#define FINEMASK   (FINEANGLES - 1)

// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT 19

// Effective size is 10240.
extern const fixed_t finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 pahse shift.
extern const fixed_t * finecosine;

// Effective size is 4096.
extern const fixed_t finetangent[FINEANGLES / 2];

// Gamma correction tables.
const uint8_t extern gammatable[5][256];

// Binary Angle Measument, BAM.

constexpr auto                  ANG45   = 0x20000000;
constexpr auto                  ANG90   = 0x40000000;
constexpr auto                  ANG180  = 0x80000000;
constexpr auto                  ANG270  = 0xc0000000;
[[maybe_unused]] constexpr auto ANG_MAX = 0xffffffff;

constexpr auto ANG1  = (ANG45 / 45);
constexpr auto ANG60 = (ANG180 / 3);

// Heretic code uses this definition as though it represents one
// degree, but it is not!  This is actually ~1.40 degrees.

[[maybe_unused]] constexpr auto ANG1_X = 0x01000000;

constexpr auto SLOPERANGE = 2048;
constexpr auto SLOPEBITS  = 11;
constexpr auto DBITS      = (FRACBITS - SLOPEBITS);

using angle_t = unsigned int;

// Effective size is 2049;
// The +1 size is to handle the case when x==y
//  without additional checking.
extern const angle_t tantoangle[SLOPERANGE + 1];

// Utility function,
//  called by R_PointToAngle.
int SlopeDiv(unsigned int num, unsigned int den);
int SlopeDivCrispy(unsigned int num, unsigned int den);

#endif
