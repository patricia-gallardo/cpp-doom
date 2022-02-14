// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: v_video.h,v 1.9 1998/05/06 11:12:54 jim Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// DESCRIPTION:
//  Gamma correction LUT.
//  Color range translation support
//  Functions to draw patches (by post) directly to screen.
//  Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------

#ifndef __V_TRANS__
#define __V_TRANS__

#include "doomtype.hpp"

enum class cr_t
{
    CR_NONE,
    CR_DARK,
    CR_GRAY,
    CR_GREEN,
    CR_GOLD,
    CR_RED,
    CR_BLUE,
    CR_RED2BLUE,
    CR_RED2GREEN,
    CRMAX
};

[[maybe_unused]] constexpr auto CR_GREY = cr_t::CR_GRAY;

extern uint8_t *cr_colors[static_cast<int>(cr_t::CRMAX)];
extern char **crstr;

constexpr char cr_esc = '~';

#ifndef CRISPY_TRUECOLOR
extern uint8_t *tranmap;
#else
extern const pixel_t (*blendfunc)(const pixel_t fg, const pixel_t bg);
extern const pixel_t I_BlendAdd(const pixel_t bg, const pixel_t fg);
extern const pixel_t I_BlendDark(const pixel_t bg, const int d);
extern const pixel_t I_BlendOver(const pixel_t bg, const pixel_t fg);
#endif

#endif // __V_TRANS__
