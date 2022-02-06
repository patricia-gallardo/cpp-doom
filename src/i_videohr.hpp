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
// DESCRIPTION:
//     SDL emulation of VGA 640x480x4 planar video mode,
//     for Hexen startup loading screen.
//

#ifndef I_VIDEOHR_H
#define I_VIDEOHR_H

bool I_SetVideoModeHR();
void    I_UnsetVideoModeHR();
void    I_SetWindowTitleHR(const char *title);
void    I_ClearScreenHR();
void    I_SlamBlockHR(int x, int y, int w, int h, const byte *src);
void    I_SlamHR(const byte *buffer);
void    I_InitPaletteHR();
void    I_SetPaletteHR(const byte *palette);
void    I_FadeToPaletteHR(const byte *palette);
void    I_BlackPaletteHR();
bool I_CheckAbortHR();

#endif /* #ifndef I_VIDEOHR_H */
