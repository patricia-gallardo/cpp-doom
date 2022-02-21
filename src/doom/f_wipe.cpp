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
//	Mission begin melt/wipe screen special effect.
//

#include <cstring>

#include "z_zone.hpp"
#include "i_video.hpp"
#include "v_video.hpp"
#include "m_random.hpp"

#include "doomtype.hpp"

#include "memory.hpp"
#include "f_wipe.hpp"

//
//                       SCREEN WIPE PACKAGE
//

// when zero, stop the wipe
static bool go = false;

static pixel_t *wipe_scr_start;
static pixel_t *wipe_scr_end;
static pixel_t *wipe_scr;


void wipe_shittyColMajorXform(dpixel_t *array,
    int                                 width,
    int                                 height)
{
    int       x;
    int       y;
    dpixel_t *dest;

    dest = zmalloc<dpixel_t *>(static_cast<unsigned long>(width * height) * sizeof(*dest), PU_STATIC, 0);

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            dest[x * height + y] = array[y * width + x];

    std::memcpy(array, dest, static_cast<unsigned long>(width * height) * sizeof(*dest));

    Z_Free(dest);
}

int wipe_initColorXForm(int width, int height, int)
{
    std::memcpy(wipe_scr, wipe_scr_start, static_cast<unsigned long>(width * height) * sizeof(*wipe_scr));
    return 0;
}

int wipe_doColorXForm(int width,
    int                   height,
    int                   ticks)
{
    bool  changed;
    pixel_t *w;
    pixel_t *e;
    int      newval;

    changed = false;
    w       = wipe_scr;
    e       = wipe_scr_end;

    while (w != wipe_scr + width * height)
    {
        if (*w != *e)
        {
            if (*w > *e)
            {
                newval = *w - ticks;
                if (newval < *e)
                    *w = *e;
                else
                    *w = static_cast<pixel_t>(newval);
                changed = true;
            }
            else if (*w < *e)
            {
                newval = *w + ticks;
                if (newval > *e)
                    *w = *e;
                else
                    *w = static_cast<pixel_t>(newval);
                changed = true;
            }
        }
        w++;
        e++;
    }

    return !changed;
}

int wipe_exitColorXForm(int, int, int)
{
    return 0;
}


static int *y;

int wipe_initMelt(int width, int height, int)
{
    int i, r;

    // copy start screen to main screen
    std::memcpy(wipe_scr, wipe_scr_start, static_cast<unsigned long>(width * height) * sizeof(*wipe_scr));

    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform(reinterpret_cast<dpixel_t *>(wipe_scr_start), width / 2, height);
    wipe_shittyColMajorXform(reinterpret_cast<dpixel_t *>(wipe_scr_end), width / 2, height);

    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y    = zmalloc<int *>(static_cast<unsigned long>(width) * sizeof(int), PU_STATIC, 0);
    y[0] = -(M_Random() % 16);
    for (i = 1; i < width; i++)
    {
        r    = (M_Random() % 3) - 1;
        y[i] = y[i - 1] + r;
        if (y[i] > 0)
            y[i] = 0;
        else if (y[i] == -16)
            y[i] = -15;
    }

    return 0;
}

int wipe_doMelt(int width,
    int             height,
    int             ticks)
{
    int i;
    int j;
    int dy;
    int idx;

    dpixel_t *s;
    dpixel_t *d;
    bool   done = true;

    width /= 2;

    while (ticks--)
    {
        for (i = 0; i < width; i++)
        {
            if (y[i] < 0)
            {
                y[i]++;
                done = false;
            }
            else if (y[i] < height)
            {
                dy = (y[i] < 16) ? y[i] + 1 : (8 << crispy->hires);
                if (y[i] + dy >= height) dy = height - y[i];
                dpixel_t *pixel1 = reinterpret_cast<dpixel_t *>(wipe_scr_end);
                s   = &pixel1[i * height + y[i]];
                dpixel_t *pixel2 = reinterpret_cast<dpixel_t *>(wipe_scr);
                d   = &pixel2[y[i] * width + i];
                idx = 0;
                for (j = dy; j; j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }
                y[i] += dy;
                dpixel_t *pixel11 = reinterpret_cast<dpixel_t *>(wipe_scr_start);
                s   = &pixel11[i * height];
                dpixel_t *pixel22 = reinterpret_cast<dpixel_t *>(wipe_scr);
                d   = &pixel22[y[i] * width + i];
                idx = 0;
                for (j = height - y[i]; j; j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }
                done = false;
            }
        }
    }

    return done;
}

int wipe_exitMelt(int, int, int)
{
    Z_Free(y);
    Z_Free(wipe_scr_start);
    Z_Free(wipe_scr_end);
    return 0;
}

int wipe_StartScreen(int, int, int, int)
{
    wipe_scr_start = zmalloc<decltype(wipe_scr_start)>(
        static_cast<unsigned long>(SCREENWIDTH * SCREENHEIGHT) * sizeof(*wipe_scr_start), PU_STATIC, nullptr);
    I_ReadScreen(wipe_scr_start);
    return 0;
}

int wipe_EndScreen(int x,
    int                y_param,
    int                width,
    int                height)
{
    wipe_scr_end = zmalloc<decltype(wipe_scr_end)>(static_cast<unsigned long>(SCREENWIDTH * SCREENHEIGHT) * sizeof(*wipe_scr_end), PU_STATIC, nullptr);
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(x, y_param, width, height, wipe_scr_start); // restore start scr.
    return 0;
}

int wipe_ScreenWipe(int wipeno, int, int, int width, int height, int ticks)
{
    int rc;
    static int (*wipes[])(int, int, int) = {
        wipe_initColorXForm, wipe_doColorXForm, wipe_exitColorXForm,
        wipe_initMelt, wipe_doMelt, wipe_exitMelt
    };

    // initial stuff
    if (!go)
    {
        go = 1;
        // wipe_scr = zmalloc<pixel_t *>(width*height, PU_STATIC, 0); // DEBUG
        wipe_scr = g_i_video_globals->I_VideoBuffer;
        (*wipes[wipeno * 3])(width, height, ticks);
    }

    // do a piece of wipe-in
    V_MarkRect(0, 0, width, height);
    rc = (*wipes[wipeno * 3 + 1])(width, height, ticks);
    //  V_DrawBlock(x, y, 0, width, height, wipe_scr); // DEBUG

    // final stuff
    if (rc)
    {
        go = 0;
        (*wipes[wipeno * 3 + 2])(width, height, ticks);
    }

    return !go;
}
