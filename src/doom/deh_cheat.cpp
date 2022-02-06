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
// Parses "Cheat" sections in dehacked files
//

#include <cstdlib>
#include <cstring>

#include "doomtype.hpp"

#include "deh_defs.hpp"
#include "deh_io.hpp"
#include "deh_main.hpp"
#include "am_map.hpp"
#include "st_stuff.hpp"

typedef struct
{
    const char *name;
    cheatseq_t *seq;
} deh_cheat_t;

static deh_cheat_t allcheats[] = {
    { "Change music", &cheat_mus },
    { "Chainsaw", &cheat_choppers },
    { "God mode", &cheat_god },
    { "Ammo & Keys", &cheat_ammo },
    { "Ammo", &cheat_ammonokey },
    { "No Clipping 1", &cheat_noclip },
    { "No Clipping 2", &cheat_commercial_noclip },
    { "Invincibility", &cheat_powerup[0] },
    { "Berserk", &cheat_powerup[1] },
    { "Invisibility", &cheat_powerup[2] },
    { "Radiation Suit", &cheat_powerup[3] },
    { "Auto-map", &cheat_powerup[4] },
    { "Lite-Amp Goggles", &cheat_powerup[5] },
    { "BEHOLD menu", &cheat_powerup[6] },
    { "Level Warp", &cheat_clev },
    { "Player Position", &cheat_mypos },
    { "Map cheat", &cheat_amap },
};

static deh_cheat_t *FindCheatByName(char *name)
{
    size_t i;

    for (i = 0; i < arrlen(allcheats); ++i)
    {
        if (!strcasecmp(allcheats[i].name, name))
            return &allcheats[i];
    }

    return nullptr;
}

static void *DEH_CheatStart(deh_context_t *context, char *line)
{
    return nullptr;
}

static void DEH_CheatParseLine(deh_context_t *context, char *line, void *tag)
{
    deh_cheat_t *  cheat;
    char *         variable_name;
    char *         value;
    unsigned char *unsvalue;
    unsigned int   i;

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    unsvalue = (unsigned char *)value;

    cheat = FindCheatByName(variable_name);

    if (cheat == nullptr)
    {
        DEH_Warning(context, "Unknown cheat '%s'", variable_name);
        return;
    }

    // write the value into the cheat sequence

    i = 0;

    while (unsvalue[i] != 0 && unsvalue[i] != 0xff)
    {
        // If the cheat length exceeds the Vanilla limit, stop.  This
        // does not apply if we have the limit turned off.

        if (!deh_allow_long_cheats && i >= cheat->seq->sequence_len)
        {
            DEH_Warning(context, "Cheat sequence longer than supported by "
                                 "Vanilla dehacked");
            break;
        }

        if (deh_apply_cheats)
        {
            cheat->seq->sequence[i] = unsvalue[i];
        }
        ++i;

        // Absolute limit - don't exceed

        if (i >= MAX_CHEAT_LEN - cheat->seq->parameter_chars)
        {
            DEH_Error(context, "Cheat sequence too long!");
            return;
        }
    }

    if (deh_apply_cheats)
    {
        cheat->seq->sequence[i] = '\0';
    }
}

deh_section_t deh_section_cheat = {
    "Cheat",
    nullptr,
    DEH_CheatStart,
    DEH_CheatParseLine,
    nullptr,
    nullptr,
};
