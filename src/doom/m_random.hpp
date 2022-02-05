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
//
//


#ifndef __M_RANDOM__
#define __M_RANDOM__


#include "doomtype.hpp"


// Returns a number from 0 to 255,
// from a lookup table.
int M_Random();

// As M_Random, but used only by the play simulation.
int P_Random();

// [crispy] our own private random function
int Crispy_Random();

// Fix randoms for demos.
void M_ClearRandom();

// Defined version of P_Random() - P_Random()
int P_SubRandom();
int Crispy_SubRandom();

#endif
