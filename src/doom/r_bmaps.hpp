//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2017 Fabian Greffrath
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Brightmaps for wall textures
//

#pragma once

#include "doomtype.hpp"

extern void R_InitBrightmaps();

extern uint8_t * (*R_BrightmapForTexName)(cstring_view texname);
extern uint8_t * (*R_BrightmapForSprite)(const int type);
extern uint8_t * (*R_BrightmapForFlatNum)(const int num);
extern uint8_t * (*R_BrightmapForState)(const int state);

extern uint8_t ** texturebrightmap;
