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
// Parses Text substitution sections in dehacked files
//

#include <cstring>
#include <string>

#include "doomtype.hpp"

#include "z_zone.hpp"

#include "deh_defs.hpp"
#include "deh_io.hpp"
#include "deh_main.hpp"

// [crispy] support INCLUDE NOTEXT directive in BEX files
bool bex_notext = false;

// Given a string length, find the maximum length of a
// string that can replace it.

static int TXT_MaxStringLength(int len)
{
    // Enough bytes for the string and the NUL terminator

    len += 1;

    // All strings in doom.exe are on 4-byte boundaries, so we may be able
    // to support a slightly longer string.
    // Extend up to the next 4-byte boundary

    len += (4 - (len % 4)) % 4;

    // Less one for the NUL terminator.

    return len - 1;
}

static void *DEH_TextStart(deh_context_t *context, char *line)
{
    int fromlen, tolen;
    int i;

    if (sscanf(line, "Text %i %i", &fromlen, &tolen) != 2)
    {
        DEH_Warning(context, "Parse error on section start");
        return nullptr;
    }

    // Only allow string replacements that are possible in Vanilla Doom.
    // Chocolate Doom is unforgiving!

    if (!deh_allow_long_strings && tolen > TXT_MaxStringLength(fromlen))
    {
        DEH_Error(context, "Replacement string is longer than the maximum "
                           "possible in doom.exe");
        return nullptr;
    }

    std::string from_text(fromlen + 1, '\0');
    std::string to_text(tolen + 1, '\0');

    // read in the "from" text

    for (i = 0; i < fromlen; ++i)
    {
        from_text[i] = DEH_GetChar(context);
    }
    from_text[fromlen] = '\0';

    // read in the "to" text

    for (i = 0; i < tolen; ++i)
    {
        to_text[i] = DEH_GetChar(context);
    }
    to_text[tolen] = '\0';

    if (!bex_notext)
    {
        DEH_AddStringReplacement(from_text.c_str(), to_text.c_str());
    }

    return nullptr;
}

static void DEH_TextParseLine(deh_context_t *context, char *line, void *tag)
{
    // not used
}

deh_section_t deh_section_text = {
    "Text",
    nullptr,
    DEH_TextStart,
    DEH_TextParseLine,
    nullptr,
    nullptr,
};
