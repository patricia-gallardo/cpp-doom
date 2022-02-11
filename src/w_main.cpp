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
//     Common code to parse command line, identifying WAD files to load.
//

#include <array>
#include <cstdlib>

#include "config.h"
#include "d_iwad.hpp"
#include "i_glob.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "w_main.hpp"
#include "w_merge.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

// Parse the command line, merging WAD files that are sppecified.
// Returns true if at least one file was added.
bool W_ParseCommandLine()
{
    bool modifiedgame = false;
    int     p;

    // Merged PWADs are loaded first, because they are supposed to be
    // modified IWADs.

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of deutex's -merge option, merging a PWAD
    // into the main IWAD.  Multiple files may be specified.
    //

    p = M_CheckParmWithArgs("-merge", 1);

    if (p > 0)
    {
        for (p = p + 1; p < myargc && myargv[p][0] != '-'; ++p)
        {
            char *filename;

            modifiedgame = true;

            filename = D_TryFindWADByName(myargv[p]);

            printf(" merging %s\n", filename);
            W_MergeFile(filename);
            free(filename);
        }
    }

    // NWT-style merging:

    // NWT's -merge option:

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of NWT's -merge option.  Multiple files
    // may be specified.

    p = M_CheckParmWithArgs("-nwtmerge", 1);

    if (p > 0)
    {
        for (p = p + 1; p < myargc && myargv[p][0] != '-'; ++p)
        {
            char *filename;

            modifiedgame = true;

            filename = D_TryFindWADByName(myargv[p]);

            printf(" performing NWT-style merge of %s\n", filename);
            W_NWTDashMerge(filename);
            free(filename);
        }
    }

    // Add flats

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of NWT's -af option, merging flats into
    // the main IWAD directory.  Multiple files may be specified.
    //

    p = M_CheckParmWithArgs("-af", 1);

    if (p > 0)
    {
        for (p = p + 1; p < myargc && myargv[p][0] != '-'; ++p)
        {
            char *filename;

            modifiedgame = true;

            filename = D_TryFindWADByName(myargv[p]);

            printf(" merging flats from %s\n", filename);
            W_NWTMergeFile(filename, W_NWT_MERGE_FLATS);
            free(filename);
        }
    }

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of NWT's -as option, merging sprites
    // into the main IWAD directory.  Multiple files may be specified.
    //

    p = M_CheckParmWithArgs("-as", 1);

    if (p > 0)
    {
        for (p = p + 1; p < myargc && myargv[p][0] != '-'; ++p)
        {
            char *filename;

            modifiedgame = true;
            filename     = D_TryFindWADByName(myargv[p]);

            printf(" merging sprites from %s\n", filename);
            W_NWTMergeFile(filename, W_NWT_MERGE_SPRITES);
            free(filename);
        }
    }

    //!
    // @arg <files>
    // @category mod
    //
    // Equivalent to "-af <files> -as <files>".
    //

    p = M_CheckParmWithArgs("-aa", 1);

    if (p > 0)
    {
        for (p = p + 1; p < myargc && myargv[p][0] != '-'; ++p)
        {
            char *filename;

            modifiedgame = true;

            filename = D_TryFindWADByName(myargv[p]);

            printf(" merging sprites and flats from %s\n", filename);
            W_NWTMergeFile(filename, W_NWT_MERGE_SPRITES | W_NWT_MERGE_FLATS);
            free(filename);
        }
    }

    //!
    // @arg <files>
    // @vanilla
    //
    // Load the specified PWAD files.
    //

    p = M_CheckParmWithArgs("-file", 1);
    if (p)
    {
        // the parms after p are wadfile/lump names,
        // until end of parms or another - preceded parm
        modifiedgame = true; // homebrew levels
        while (++p != myargc && myargv[p][0] != '-')
        {
            char *filename;

            filename = D_TryFindWADByName(myargv[p]);

            // [crispy] always merge arguments of "-file" parameter
            printf(" merging %s !\n", filename);
            W_MergeFile(filename);
            free(filename);
        }
    }

    //    W_PrintDirectory();

    return modifiedgame;
}

// Load all WAD files from the given directory.
void W_AutoLoadWADs(const char *path)
{
    glob_t *    glob;
    const char *filename;

    glob = I_StartMultiGlob(path, GLOB_FLAG_NOCASE | GLOB_FLAG_SORTED,
        "*.wad", "*.lmp", nullptr);
    for (;;)
    {
        filename = I_NextGlob(glob);
        if (filename == nullptr)
        {
            break;
        }
        printf(" [autoload] merging %s\n", filename);
        W_MergeFile(filename);
    }

    I_EndGlob(glob);
}

// Lump names that are unique to particular game types. This lets us check
// the user is not trying to play with the wrong executable, eg.
// chocolate-doom -iwad hexen.wad.
static const struct
{
    GameMission_t mission;
    const char *  lumpname;
} unique_lumps[] = {
    { doom, "POSSA1" },
    { heretic, "IMPXA1" },
    { hexen, "ETTNA1" },
    { strife, "AGRDA1" },
};

void W_CheckCorrectIWAD(GameMission_t mission)
{
    for (auto unique_lump : unique_lumps)
    {
        if (mission != unique_lump.mission)
        {
            lumpindex_t lumpnum = W_CheckNumForName(unique_lump.lumpname);

            if (lumpnum >= 0)
            {
                I_Error("\nYou are trying to use a %s IWAD file with "
                        "the %s%s binary.\nThis isn't going to work.\n"
                        "You probably want to use the %s%s binary.",
                    D_SuggestGameName(unique_lump.mission,
                        indetermined),
                    PROGRAM_PREFIX,
                    D_GameMissionString(mission),
                    PROGRAM_PREFIX,
                    D_GameMissionString(unique_lump.mission));
            }
        }
    }
}
