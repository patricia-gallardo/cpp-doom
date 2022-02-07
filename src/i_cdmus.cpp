//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
// Hexen CD interface.
//

#include <cstdio>

#include "SDL.h"

#include "doomtype.hpp"

#include "i_cdmus.hpp"

int cd_Error;

int I_CDMusInit()
{
    fprintf(stderr,
        "I_CDMusInit: CD music playback is no longer supported! "
        "Please use digital music packs instead:\n"
        "https://www.chocolate-doom.org/wiki/index.php/Digital_music_packs\n");
    return -1;
}

// We cannot print status messages inline during startup, they must
// be deferred until after I_CDMusInit has returned.

void I_CDMusPrintStartup()
{
}

int I_CDMusPlay(int)
{
    return 0;
}

int I_CDMusStop()
{
    return 0;
}

int I_CDMusResume()
{
    return 0;
}

int I_CDMusSetVolume(int)
{
    return 0;
}

int I_CDMusFirstTrack()
{
    return 0;
}

int I_CDMusLastTrack()
{
    return 0;
}

int I_CDMusTrackLength(int)
{
    return 0;
}
