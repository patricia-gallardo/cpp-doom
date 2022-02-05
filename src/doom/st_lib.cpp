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
//	The status bar widget code.
//


#include <cstdio>

#include "deh_main.hpp"
#include "doomdef.hpp"

#include "z_zone.hpp"
#include "v_video.hpp"

#include "i_swap.hpp"
#include "i_system.hpp"

#include "w_wad.hpp"

#include "st_stuff.hpp"
#include "st_lib.hpp"
#include "r_local.hpp"

#include "lump.hpp"
#include "v_trans.hpp" // [crispy] colored status bar widgets

// in AM_map.c
extern boolean automapactive;
extern int     screenblocks;


//
// Hack display negative frags.
//  Loads and store the stminus lump.
//
patch_t *sttminus;

void STlib_init()
{
    if (W_CheckNumForName(DEH_String("STTMINUS")) >= 0)
        sttminus = cache_lump_name<patch_t *>(DEH_String("STTMINUS"), PU_STATIC);
    else
        sttminus = NULL;
}


// ?
void STlib_initNum(st_number_t *n,
    int                         x,
    int                         y,
    patch_t **                  pl,
    int *                       num,
    boolean *                   on,
    int                         width)
{
    n->x      = x;
    n->y      = y;
    n->oldnum = 0;
    n->width  = width;
    n->num    = num;
    n->on     = on;
    n->p      = pl;
}


//
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?
//
void STlib_drawNum(st_number_t *n,
    boolean                     refresh)
{

    int numdigits = n->width;
    int num       = *n->num;

    int w = SHORT(n->p[0]->width);
    int h = SHORT(n->p[0]->height);
    int x = n->x;

    int neg;

    // [crispy] redraw only if necessary
    if (n->oldnum == num && !refresh)
    {
        return;
    }

    n->oldnum = *n->num;

    neg = num < 0;

    if (neg)
    {
        if (numdigits == 2 && num < -9)
            num = -9;
        else if (numdigits == 3 && num < -99)
            num = -99;

        num = -num;
    }

    // clear the area
    x = n->x - numdigits * w;

    if (n->y - ST_Y < 0)
        I_Error("drawNum: n->y - ST_Y < 0");

    if (screenblocks < CRISPY_HUD || (automapactive && !crispy->automapoverlay && !crispy->widescreen))
        V_CopyRect(x, n->y - ST_Y, st_backing_screen, w * numdigits, h, x, n->y);

    // if non-number, do not draw it
    if (num == 1994)
        return;

    x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
        V_DrawPatch(x - w, n->y, n->p[0]);

    // draw the new number
    while (num && numdigits--)
    {
        x -= w;
        V_DrawPatch(x, n->y, n->p[num % 10]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg && sttminus)
        V_DrawPatch(x - 8, n->y, sttminus);
}


//
void STlib_updateNum(st_number_t *n,
    boolean                       refresh)
{
    if (*n->on) STlib_drawNum(n, refresh);
}


//
void STlib_initPercent(st_percent_t *p,
    int                              x,
    int                              y,
    patch_t **                       pl,
    int *                            num,
    boolean *                        on,
    patch_t *                        percent)
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;

    // [crispy] remember previous colorization
    p->oldtranslation = NULL;
}


void STlib_updatePercent(st_percent_t *per,
    int                                refresh)
{
    // [crispy] remember previous colorization
    if (per->oldtranslation != dp_translation)
    {
        refresh             = true;
        per->oldtranslation = dp_translation;
    }

    STlib_updateNum(&per->n, refresh); // [crispy] moved here

    if (crispy->coloredhud & COLOREDHUD_BAR)
        dp_translation = cr_colors[static_cast<int>(cr_t::CR_GRAY)];

    if (refresh && *per->n.on)
        V_DrawPatch(per->n.x, per->n.y, per->p);

    dp_translation = NULL;
}


void STlib_initMultIcon(st_multicon_t *i,
    int                                x,
    int                                y,
    patch_t **                         il,
    int *                              inum,
    boolean *                          on)
{
    i->x       = x;
    i->y       = y;
    i->oldinum = -1;
    i->inum    = inum;
    i->on      = on;
    i->p       = il;
}


void STlib_updateMultIcon(st_multicon_t *mi,
    boolean                              refresh)
{
    int w;
    int h;
    int x;
    int y;

    if (*mi->on
        && (mi->oldinum != *mi->inum || refresh)
        && (*mi->inum != -1))
    {
        if (mi->oldinum != -1)
        {
            x = mi->x - SHORT(mi->p[mi->oldinum]->leftoffset);
            y = mi->y - SHORT(mi->p[mi->oldinum]->topoffset);
            w = SHORT(mi->p[mi->oldinum]->width);
            h = SHORT(mi->p[mi->oldinum]->height);

            if (y - ST_Y < 0)
                I_Error("updateMultIcon: y - ST_Y < 0");

            if (screenblocks < CRISPY_HUD || (automapactive && !crispy->automapoverlay && !crispy->widescreen))
                V_CopyRect(x, y - ST_Y, st_backing_screen, w, h, x, y);
        }
        V_DrawPatch(mi->x, mi->y, mi->p[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}


void STlib_initBinIcon(st_binicon_t *b,
    int                              x,
    int                              y,
    patch_t *                        i,
    boolean *                        val,
    boolean *                        on)
{
    b->x      = x;
    b->y      = y;
    b->oldval = false;
    b->val    = val;
    b->on     = on;
    b->p      = i;
}


void STlib_updateBinIcon(st_binicon_t *bi,
    boolean                            refresh)
{
    int x;
    int y;
    int w;
    int h;

    if (*bi->on
        && (bi->oldval != *bi->val || refresh))
    {
        x = bi->x - SHORT(bi->p->leftoffset);
        y = bi->y - SHORT(bi->p->topoffset);
        w = SHORT(bi->p->width);
        h = SHORT(bi->p->height);

        if (y - ST_Y < 0)
            I_Error("updateBinIcon: y - ST_Y < 0");

        if (*bi->val)
            V_DrawPatch(bi->x, bi->y, bi->p);
        else if (screenblocks < CRISPY_HUD || (automapactive && !crispy->automapoverlay && !crispy->widescreen))
            V_CopyRect(x, y - ST_Y, st_backing_screen, w, h, x, y);

        bi->oldval = *bi->val;
    }
}
