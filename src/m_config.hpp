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
//      Configuration file interface.
//

#pragma once

#include "cstring_view.hpp"
#include "doomtype.hpp"

void                   M_LoadDefaults();
void                   M_SaveDefaults();
void                   M_SaveDefaultsAlternate(cstring_view main, cstring_view extra);
void                   M_SetConfigDir(const char * dir);
void                   M_SetMusicPackDir();
void                   M_BindIntVariable(cstring_view name, int * variable);
void                   M_BindFloatVariable(cstring_view name, float * variable);
void                   M_BindStringVariable(cstring_view name, char ** variable);
bool                   M_SetVariable(cstring_view name, cstring_view value);
[[maybe_unused]] int   M_GetIntVariable(cstring_view name);
const char *           M_GetStringVariable(cstring_view name);
[[maybe_unused]] float M_GetFloatVariable(cstring_view name);
void                   M_SetConfigFilenames(cstring_view main_config, cstring_view extra_config);
char *                 M_GetSaveGameDir(cstring_view iwadname);
char *                 M_GetAutoloadDir(cstring_view iwadname);

extern char * configdir;
