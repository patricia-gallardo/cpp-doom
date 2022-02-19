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
// Text mode emulation in SDL
//

#ifndef TXT_IO_H
#define TXT_IO_H

#include "txt_main.hpp"

struct txt_saved_colors_t
{
    txt_color_t bgcolor;
    txt_color_t fgcolor;
};

void TXT_PutSymbol(int c);
void TXT_PutChar(int c);
void TXT_Puts(const char *s);
void TXT_GotoXY(int x, int y);
void TXT_GetXY(int *x, int *y);
void TXT_FGColor(txt_color_t color);
void TXT_BGColor(txt_color_t color, int blinking);
void TXT_SaveColors(txt_saved_colors_t *save);
void TXT_RestoreColors(txt_saved_colors_t *save);
[[maybe_unused]] void TXT_ClearScreen();

#endif /* #ifndef TXT_IO_H */

