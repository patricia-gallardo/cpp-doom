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

// i_cdmus.h

#ifndef __ICDMUS__
#define __ICDMUS__

[[maybe_unused]] constexpr auto CDERR_NOTINSTALLED   = 10;  // MSCDEX not installed
[[maybe_unused]] constexpr auto CDERR_NOAUDIOSUPPORT = 11;  // CD-ROM Doesn't support audio
[[maybe_unused]] constexpr auto CDERR_NOAUDIOTRACKS  = 12;  // Current CD has no audio tracks
[[maybe_unused]] constexpr auto CDERR_BADDRIVE       = 20;  // Bad drive number
[[maybe_unused]] constexpr auto CDERR_BADTRACK       = 21;  // Bad track number
[[maybe_unused]] constexpr auto CDERR_IOCTLBUFFMEM   = 22;  // Not enough low memory for IOCTL
[[maybe_unused]] constexpr auto CDERR_DEVREQBASE     = 100; // DevReq errors

[[maybe_unused]] extern int cd_Error;

[[maybe_unused]] int  I_CDMusInit();
[[maybe_unused]] void I_CDMusPrintStartup();
[[maybe_unused]] int  I_CDMusPlay(int track);
[[maybe_unused]] int  I_CDMusStop();
[[maybe_unused]] int  I_CDMusResume();
[[maybe_unused]] int  I_CDMusSetVolume(int volume);
[[maybe_unused]] int  I_CDMusFirstTrack();
[[maybe_unused]] int  I_CDMusLastTrack();
[[maybe_unused]] int  I_CDMusTrackLength(int track);

#endif
