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

#pragma once

#include "doomtype.hpp"
#include "textscreen.hpp"

struct execute_context_t;

constexpr auto IWAD_DOOM2    = (1 << 0); /* doom2.wad */
constexpr auto IWAD_PLUTONIA = (1 << 1); /* plutonia.wad */
constexpr auto IWAD_TNT      = (1 << 2); /* tnt.wad */
constexpr auto IWAD_DOOM     = (1 << 3); /* doom.wad */
constexpr auto IWAD_DOOM1    = (1 << 4); /* doom1.wad */
constexpr auto IWAD_CHEX     = (1 << 5); /* chex.wad */

execute_context_t * NewExecuteContext();
void                AddCmdLineParameter(execute_context_t * context, const char * s, ...) PRINTF_ATTR(2, 3);
void                PassThroughArguments(execute_context_t * context);
int                 ExecuteDoom(execute_context_t * context);
int                 FindInstalledIWADs();
bool                OpenFolder(cstring_view path);

txt_window_action_t * TestConfigAction();
