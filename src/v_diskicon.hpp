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
//	Disk load indicator.
//

#ifndef __V_DISKICON__
#define __V_DISKICON__

#include "crispy.hpp"

// Dimensions of the flashing "loading" disk icon

#define LOADING_DISK_W (16 << crispy->hires)
#define LOADING_DISK_H (16 << crispy->hires)

extern void V_EnableLoadingDisk(const char *lump_name, int xoffs, int yoffs);
extern void V_EnableLoadingDisk(const std::string & lump_name, int xoffs, int yoffs);
extern void V_BeginRead(size_t nbytes);
extern void V_DrawDiskIcon();
extern void V_RestoreDiskBackground();

#endif
