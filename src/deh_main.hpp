//
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
//
// Dehacked entrypoint and common code
//

#pragma once

#include "deh_str.hpp"
#include "doomtype.hpp"
#include "sha1.hpp"

// These are the limits that dehacked uses (from dheinit.h in the dehacked
// source).  If these limits are exceeded, it does not generate an error, but
// a warning is displayed.

constexpr auto DEH_VANILLA_NUMSTATES = 966;
constexpr auto DEH_VANILLA_NUMSFX    = 107;

void DEH_ParseCommandLine();
int  DEH_LoadFile(cstring_view filename);
void DEH_AutoLoadPatches(cstring_view path);
int  DEH_LoadLump(int lumpnum, bool allow_long, bool allow_error);
int  DEH_LoadLumpByName(cstring_view name, bool allow_long, bool allow_error);

bool DEH_ParseAssignment(char * line, char ** variable_name, char ** value);

void DEH_Checksum(sha1_digest_t digest);

extern bool deh_allow_extended_strings;
extern bool deh_allow_long_strings;
extern bool deh_allow_long_cheats;
extern bool deh_apply_cheats;
