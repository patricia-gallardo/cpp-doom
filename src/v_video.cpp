//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//

#include "SDL_version.h" // [crispy]

#include <cstdlib>
#include <cstring>
#include <cmath>

#include "i_system.hpp"

#include "doomtype.hpp"

#include "deh_str.hpp"
#include "i_input.hpp"
#include "i_swap.hpp"
#include "i_video.hpp"
#include "m_bbox.hpp"
#include "m_misc.hpp"
#ifdef CRISPY_TRUECOLOR
#include "v_trans.hpp"
#endif
#include "v_video.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"
#include "crispy.hpp"

#include "lump.hpp"
#include "memory.hpp"
#include "config.h"
#ifdef HAVE_LIBPNG
#include <png.h>
#endif

// TODO: There are separate RANGECHECK defines for different games, but this
// is common code. Fix this.
#define RANGECHECK

// Blending table used for fuzzpatch, etc.
// Only used in Heretic/Hexen

uint8_t *tinttable      = nullptr;
uint8_t *tranmap        = nullptr;
uint8_t *dp_translation = nullptr;
bool dp_translucent = false;
#ifdef CRISPY_TRUECOLOR
extern pixel_t *colormaps;
#endif

// villsa [STRIFE] Blending table used for Strife
uint8_t *xlatab = nullptr;

// The screen buffer that the v_video.c code draws to.

static pixel_t *dest_screen = nullptr;

int dirtybox[4];

// haleyjd 08/28/10: clipping callback function for patches.
// This is needed for Chocolate Strife, which clips patches to the screen.
static vpatchclipfunc_t patchclip_callback = nullptr;

//
// V_MarkRect
//
void V_MarkRect(int x, int y, int width, int height)
{
    // If we are temporarily using an alternate screen, do not
    // affect the update box.

    if (dest_screen == I_VideoBuffer)
    {
        M_AddToBox(dirtybox, x, y);
        M_AddToBox(dirtybox, x + width - 1, y + height - 1);
    }
}


//
// V_CopyRect
//
void V_CopyRect(int srcx, int srcy, pixel_t *source,
    int width, int height,
    int destx, int desty)
{
    srcx <<= crispy->hires;
    srcy <<= crispy->hires;
    width <<= crispy->hires;
    height <<= crispy->hires;
    destx <<= crispy->hires;
    desty <<= crispy->hires;

#ifdef RANGECHECK
    if (srcx < 0
        || srcx + width > SCREENWIDTH
        || srcy < 0
        || srcy + height > SCREENHEIGHT
        || destx < 0
        || destx /* + width */ > SCREENWIDTH
        || desty < 0
        || desty /* + height */ > SCREENHEIGHT)
    {
        I_Error("Bad V_CopyRect");
    }
#endif

    // [crispy] prevent framebuffer overflow
    if (destx + width > SCREENWIDTH)
        width = SCREENWIDTH - destx;
    if (desty + height > SCREENHEIGHT)
        height = SCREENHEIGHT - desty;

    V_MarkRect(destx, desty, width, height);

    pixel_t *src  = source + SCREENWIDTH * srcy + srcx;
    pixel_t *dest = dest_screen + SCREENWIDTH * desty + destx;

    for (; height > 0; height--)
    {
        std::memcpy(dest, src, static_cast<unsigned long>(width) * sizeof(*dest));
        src += SCREENWIDTH;
        dest += SCREENWIDTH;
    }
}

//
// V_SetPatchClipCallback
//
// haleyjd 08/28/10: Added for Strife support.
// By calling this function, you can setup runtime error checking for patch
// clipping. Strife never caused errors by drawing patches partway off-screen.
// Some versions of vanilla DOOM also behaved differently than the default
// implementation, so this could possibly be extended to those as well for
// accurate emulation.
//
void V_SetPatchClipCallback(vpatchclipfunc_t func)
{
    patchclip_callback = func;
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen.
//

// [crispy] four different rendering functions
// for each possible combination of dp_translation and dp_translucent:
// (1) normal, opaque patch
static inline pixel_t drawpatchpx00(const pixel_t, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{
    return source;
}
#else
{
    return colormaps[source];
}
#endif
// (2) color-translated, opaque patch
static inline pixel_t drawpatchpx01(const pixel_t, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{
    return dp_translation[source];
}
#else
{
    return colormaps[dp_translation[source]];
}
#endif
// (3) normal, translucent patch
static inline pixel_t drawpatchpx10(const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{
    return tranmap[(dest << 8) + source];
}
#else
{
    return I_BlendOver(dest, colormaps[source]);
}
#endif
// (4) color-translated, translucent patch
static inline pixel_t drawpatchpx11(const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{
    return tranmap[(dest << 8) + dp_translation[source]];
}
#else
{
    return I_BlendOver(dest, colormaps[dp_translation[source]]);
}
#endif
// [crispy] array of function pointers holding the different rendering functions
using drawpatchpx_t = pixel_t (const pixel_t, const pixel_t);
static drawpatchpx_t *const drawpatchpx_a[2][2] = { { drawpatchpx11, drawpatchpx10 }, { drawpatchpx01, drawpatchpx00 } };

static fixed_t dx, dxi, dy, dyi;

void V_DrawPatch(int x, int y, patch_t *patch)
{
    // [crispy] four different rendering functions
    drawpatchpx_t *const drawpatchpx = drawpatchpx_a[!dp_translucent][!dp_translation];

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += DELTAWIDTH; // [crispy] horizontal widescreen offset

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if (patchclip_callback)
    {
        if (!patchclip_callback(patch, x, y))
            return;
    }

#ifdef RANGECHECK_NOTHANKS
    if (x < 0
        || x + SHORT(patch->width) > ORIGWIDTH
        || y < 0
        || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawPatch");
    }
#endif

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    pixel_t  *desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
    int       col     = 0;
    int       w       = SHORT(patch->width);

    for (; col < w << FRACBITS; x++, col += dxi, desttop++)
    {
        int topdelta = -1;

        // [crispy] too far left
        if (x < 0)
        {
            continue;
        }

        // [crispy] too far right / width
        if (x >= SCREENWIDTH)
        {
            break;
        }

        auto *column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            int srccol = 0;
            // [crispy] support for DeePsea tall patches
            if (column->topdelta <= topdelta)
            {
                topdelta += column->topdelta;
            }
            else
            {
                topdelta = column->topdelta;
            }

            int      top    = ((y + topdelta) * dy) >> FRACBITS;
            uint8_t *source = reinterpret_cast<uint8_t *>(column) + 3;
            pixel_t *dest   = desttop + ((topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            int      count  = (column->length * dy) >> FRACBITS;

            // [crispy] too low / height
            if (top + count > SCREENHEIGHT)
            {
                count = SCREENHEIGHT - top;
            }

            // [crispy] nothing left to draw?
            if (count < 1)
            {
                break;
            }

            while (count--)
            {
                // [crispy] too high
                if (top++ >= 0)
                {
                    *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                }
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(column) + column->length + 4);
        }
    }
}

void V_DrawPatchFullScreen(patch_t *patch, bool flipped)
{
    const short width  = SHORT(patch->width);
    const short height = SHORT(patch->height);

    dx  = (HIRESWIDTH << FRACBITS) / width;
    dxi = (width << FRACBITS) / HIRESWIDTH;
    dy  = (SCREENHEIGHT << FRACBITS) / height;
    dyi = (height << FRACBITS) / SCREENHEIGHT;

    patch->leftoffset = 0;
    patch->topoffset  = 0;

    // [crispy] fill pillarboxes in widescreen mode
    if (SCREENWIDTH != HIRESWIDTH)
    {
        V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
    }

    if (flipped)
    {
        V_DrawPatchFlipped(0, 0, patch);
    }
    else
    {
        V_DrawPatch(0, 0, patch);
    }

    dx  = (HIRESWIDTH << FRACBITS) / ORIGWIDTH;
    dxi = (ORIGWIDTH << FRACBITS) / HIRESWIDTH;
    dy  = (SCREENHEIGHT << FRACBITS) / ORIGHEIGHT;
    dyi = (ORIGHEIGHT << FRACBITS) / SCREENHEIGHT;
}

//
// V_DrawPatchFlipped
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//

void V_DrawPatchFlipped(int x, int y, patch_t *patch)
{
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += DELTAWIDTH; // [crispy] horizontal widescreen offset

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if (patchclip_callback)
    {
        if (!patchclip_callback(patch, x, y))
            return;
    }

#ifdef RANGECHECK_NOTHANKS
    if (x < 0
        || x + SHORT(patch->width) > ORIGWIDTH
        || y < 0
        || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawPatchFlipped");
    }
#endif

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    pixel_t *desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
    int col = 0;
    int w   = SHORT(patch->width);

    for (; col < w << FRACBITS; x++, col += dxi, desttop++)
    {
        int topdelta = -1;

        // [crispy] too far left
        if (x < 0)
        {
            continue;
        }

        // [crispy] too far right / width
        if (x >= SCREENWIDTH)
        {
            break;
        }

        auto *column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[w - 1 - (col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            int srccol = 0;
            // [crispy] support for DeePsea tall patches
            if (column->topdelta <= topdelta)
            {
                topdelta += column->topdelta;
            }
            else
            {
                topdelta = column->topdelta;
            }
            int      top    = ((y + topdelta) * dy) >> FRACBITS;
            uint8_t *source = reinterpret_cast<uint8_t *>(column) + 3;
            pixel_t *dest   = desttop + ((topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            int      count  = (column->length * dy) >> FRACBITS;

            // [crispy] too low / height
            if (top + count > SCREENHEIGHT)
            {
                count = SCREENHEIGHT - top;
            }

            // [crispy] nothing left to draw?
            if (count < 1)
            {
                break;
            }

            while (count--)
            {
                // [crispy] too high
                if (top++ >= 0)
                {
#ifndef CRISPY_TRUECOLOR
                    *dest = source[srccol >> FRACBITS];
#else
                    *dest = colormaps[source[srccol >> FRACBITS]];
#endif
                }
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(column) + column->length + 4);
        }
    }
}


//
// V_DrawPatchDirect
// Draws directly to the screen on the pc.
//

void V_DrawPatchDirect(int x, int y, patch_t *patch)
{
    V_DrawPatch(x, y, patch);
}

//
// V_DrawTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawTLPatch(int x, int y, patch_t *patch)
{
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
        || x + SHORT(patch->width) > ORIGWIDTH
        || y < 0
        || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawTLPatch");
    }

    pixel_t *desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
    int      col     = 0;
    int      w       = SHORT(patch->width);
    for (; col < w << FRACBITS; x++, col += dxi, desttop++)
    {
        auto *column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            int      srccol = 0;
            uint8_t *source = reinterpret_cast<uint8_t *>(column) + 3;
            pixel_t *dest   = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            int      count  = (column->length * dy) >> FRACBITS;

            while (count--)
            {
                *dest = tinttable[((*dest) << 8) + source[srccol >> FRACBITS]];
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(column) + column->length + 4);
        }
    }
}

//
// V_DrawXlaPatch
//
// villsa [STRIFE] Masks a column based translucent masked pic to the screen.
//

void V_DrawXlaPatch(int x, int y, patch_t *patch)
{
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (patchclip_callback)
    {
        if (!patchclip_callback(patch, x, y))
            return;
    }

    pixel_t *desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
    int      col     = 0;
    int      w       = SHORT(patch->width);

    for (; col < w << FRACBITS; x++, col += dxi, desttop++)
    {
        auto *column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            int      srccol = 0;
            uint8_t *source = reinterpret_cast<uint8_t *>(column) + 3;
            pixel_t *dest   = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            int      count  = (column->length * dy) >> FRACBITS;

            while (count--)
            {
                *dest = xlatab[*dest + (source[srccol >> FRACBITS] << 8)];
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(column) + column->length + 4);
        }
    }
}

//
// V_DrawAltTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawAltTLPatch(int x, int y, patch_t *patch)
{
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
        || x + SHORT(patch->width) > ORIGWIDTH
        || y < 0
        || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawAltTLPatch");
    }

    pixel_t *dest_top = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
    int      col      = 0;
    int      w        = SHORT(patch->width);

    for (; col < w << FRACBITS; x++, col += dxi, dest_top++)
    {
        auto *column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            int      srccol = 0;
            uint8_t *source = reinterpret_cast<uint8_t *>(column) + 3;
            pixel_t *dest   = dest_top + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            int      count  = (column->length * dy) >> FRACBITS;

            while (count--)
            {
                *dest = tinttable[((*dest) << 8) + source[srccol >> FRACBITS]];
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(column) + column->length + 4);
        }
    }
}

//
// V_DrawShadowedPatch
//
// Masks a column based masked pic to the screen.
//

void V_DrawShadowedPatch(int x, int y, patch_t *patch)
{
    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
        || x + SHORT(patch->width) > ORIGWIDTH
        || y < 0
        || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawShadowedPatch");
    }

    int col      = 0;
    pixel_t *desttop  = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
    pixel_t *desttop2 = dest_screen + (((y + 2) * dy) >> FRACBITS) * SCREENWIDTH + (((x + 2) * dx) >> FRACBITS);

    int w = SHORT(patch->width);
    for (; col < w << FRACBITS; x++, col += dxi, desttop++, desttop2++)
    {
        auto *column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            int      srccol = 0;
            uint8_t *source = reinterpret_cast<uint8_t *>(column) + 3;
            pixel_t *dest   = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            pixel_t *dest2  = desttop2 + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            int      count  = (column->length * dy) >> FRACBITS;

            while (count--)
            {
                *dest2 = tinttable[((*dest2) << 8)];
                dest2 += SCREENWIDTH;
                *dest = source[srccol >> FRACBITS];
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(column) + column->length + 4);
        }
    }
}

//
// Load tint table from TINTTAB lump.
//

void V_LoadTintTable()
{
    tinttable = cache_lump_name<uint8_t *>("TINTTAB", PU_STATIC);
}

//
// V_LoadXlaTable
//
// villsa [STRIFE] Load xla table from XLATAB lump.
//

void V_LoadXlaTable()
{
    xlatab = cache_lump_name<uint8_t *>("XLATAB", PU_STATIC);
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//

void V_DrawBlock(int x, int y, int width, int height, pixel_t *src)
{
#ifdef RANGECHECK
    if (x < 0
        || x + width > SCREENWIDTH
        || y < 0
        || y + height > SCREENHEIGHT)
    {
        I_Error("Bad V_DrawBlock");
    }
#endif

    V_MarkRect(x, y, width, height);

    pixel_t *dest = dest_screen + (y << crispy->hires) * SCREENWIDTH + x;

    while (height--)
    {
        std::memcpy(dest, src, static_cast<unsigned long>(width) * sizeof(*dest));
        src += width;
        dest += SCREENWIDTH;
    }
}

void V_DrawScaledBlock(int x, int y, int width, int height, pixel_t *src)
{
#ifdef RANGECHECK
    if (x < 0
        || x + width > ORIGWIDTH
        || y < 0
        || y + height > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawScaledBlock");
    }
#endif

    V_MarkRect(x, y, width, height);

    pixel_t *dest = dest_screen + (y << crispy->hires) * SCREENWIDTH + (x << crispy->hires);

    for (int i = 0; i < (height << crispy->hires); i++)
    {
        for (int j = 0; j < (width << crispy->hires); j++)
        {
            *(dest + i * SCREENWIDTH + j) = *(src + (i >> crispy->hires) * width + (j >> crispy->hires));
        }
    }
}

void V_DrawFilledBox(int x, int y, int w, int h, int c)
{
    pixel_t *buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (int y1 = 0; y1 < h; ++y1)
    {
        pixel_t *buf1 = buf;

        for (int x1 = 0; x1 < w; ++x1)
        {
            *buf1++ = static_cast<pixel_t>(c);
        }

        buf += SCREENWIDTH;
    }
}

void V_DrawHorizLine(int x, int y, int w, int c)
{
    // [crispy] prevent framebuffer overflows
    if (x + w > SCREENWIDTH)
        w = SCREENWIDTH - x;

    pixel_t *buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (int x1 = 0; x1 < w; ++x1)
    {
        *buf++ = static_cast<pixel_t>(c);
    }
}

void V_DrawVertLine(int x, int y, int h, int c)
{
    pixel_t *buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (int y1 = 0; y1 < h; ++y1)
    {
        *buf = static_cast<pixel_t>(c);
        buf += SCREENWIDTH;
    }
}

void V_DrawBox(int x, int y, int w, int h, int c)
{
    V_DrawHorizLine(x, y, w, c);
    V_DrawHorizLine(x, y + h - 1, w, c);
    V_DrawVertLine(x, y, h, c);
    V_DrawVertLine(x + w - 1, y, h, c);
}

//
// Draw a "raw" screen (lump containing raw data to blit directly
// to the screen)
//

void V_CopyScaledBuffer(pixel_t *dest, pixel_t *src, size_t size)
{
#ifdef RANGECHECK
    if (size > ORIGWIDTH * ORIGHEIGHT)
    {
        I_Error("Bad V_CopyScaledBuffer");
    }
#endif

    while (size--)
    {
        for (int i = 0; i <= crispy->hires; i++)
        {
            for (int j = 0; j <= crispy->hires; j++)
            {
                *(dest + (size << crispy->hires) + (crispy->hires * static_cast<int>(size / ORIGWIDTH) + i) * SCREENWIDTH + j) = *(src + size);
            }
        }
    }
}

void V_DrawRawScreen(pixel_t *raw)
{
    V_CopyScaledBuffer(dest_screen, raw, ORIGWIDTH * ORIGHEIGHT);
}

//
// V_Init
//
void V_Init()
{
    // [crispy] initialize resolution-agnostic patch drawing
    if (HIRESWIDTH && SCREENHEIGHT)
    {
        dx  = (HIRESWIDTH << FRACBITS) / ORIGWIDTH;
        dxi = (ORIGWIDTH << FRACBITS) / HIRESWIDTH;
        dy  = (SCREENHEIGHT << FRACBITS) / ORIGHEIGHT;
        dyi = (ORIGHEIGHT << FRACBITS) / SCREENHEIGHT;
    }
    // no-op!
    // There used to be separate screens that could be drawn to; these are
    // now handled in the upper layers.
}

// Set the buffer that the code draws to.

void V_UseBuffer(pixel_t *buffer)
{
    dest_screen = buffer;
}

// Restore screen buffer to the i_video screen buffer.

void V_RestoreBuffer()
{
    dest_screen = I_VideoBuffer;
}

//
// SCREEN SHOTS
//

typedef PACKED_STRUCT(
    {
        char manufacturer;
        char version;
        char encoding;
        char bits_per_pixel;

        unsigned short xmin;
        unsigned short ymin;
        unsigned short xmax;
        unsigned short ymax;

        unsigned short hres;
        unsigned short vres;

        unsigned char palette[48];

        char           reserved;
        char           color_planes;
        unsigned short bytes_per_line;
        unsigned short palette_type;

        char          filler[58];
        unsigned char data; // unbounded
    }) pcx_t;


//
// WritePCXfile
//

void WritePCXfile(char *filename, pixel_t *data,
    int width, int height, uint8_t *palette)
{
    pcx_t *pcx = zmalloc<pcx_t *>(static_cast<size_t>(width * height * 2 + 1000), PU_STATIC, nullptr);

    pcx->manufacturer   = 0x0a; // PCX id
    pcx->version        = 5;    // 256 color
    pcx->encoding       = 1;    // uncompressed
    pcx->bits_per_pixel = 8;    // 256 color
    pcx->xmin           = 0;
    pcx->ymin           = 0;
    pcx->xmax           = SHORT(width - 1);
    pcx->ymax           = SHORT(height - 1);
    pcx->hres           = SHORT(1);
    pcx->vres           = SHORT(1);
    std::memset(pcx->palette, 0, sizeof(pcx->palette));
    pcx->reserved       = 0; // PCX spec: reserved byte must be zero
    pcx->color_planes   = 1; // chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type   = SHORT(2); // not a grey scale
    std::memset(pcx->filler, 0, sizeof(pcx->filler));

    // pack the image
    uint8_t *pack = &pcx->data;

    for (int i = 0; i < width * height; i++)
    {
        if ((*data & 0xc0) != 0xc0)
            *pack++ = *data++;
        else
        {
            *pack++ = 0xc1;
            *pack++ = *data++;
        }
    }

    // write the palette
    *pack++ = 0x0c; // palette ID byte
    for (int i = 0; i < 768; i++)
        *pack++ = *palette++;

    // write output file
    int length = static_cast<int>(pack - reinterpret_cast<uint8_t *>(pcx));
    M_WriteFile(filename, pcx, length);

    Z_Free(pcx);
}

#ifdef HAVE_LIBPNG
//
// WritePNGfile
//

static void error_fn(png_structp, png_const_charp s)
{
    printf("libpng error: %s\n", s);
}

static void warning_fn(png_structp, png_const_charp s)
{
    printf("libpng warning: %s\n", s);
}

void WritePNGfile(char *filename, pixel_t *,
    int width, int height, uint8_t *palette)
{
    //  png_colorp pcolor;
    int j = 0;
    //  int w_factor, h_factor;

    extern void I_RenderReadPixels(uint8_t * *data, int *w, int *h, int *p);

    /*
    if (aspect_ratio_correct == 1)
    {
        // scale up to accommodate aspect ratio correction
        w_factor = 5;
        h_factor = 6;

        width *= w_factor;
        height *= h_factor;
    }
    else
    {
        w_factor = 1;
        h_factor = 1;
    }
*/

    FILE *handle = fopen(filename, "wb");
    if (!handle)
    {
        return;
    }

    png_structp ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr,
        error_fn, warning_fn);
    if (!ppng)
    {
        fclose(handle);
        return;
    }

    png_infop pinfo = png_create_info_struct(ppng);
    if (!pinfo)
    {
        fclose(handle);
        png_destroy_write_struct(&ppng, nullptr);
        return;
    }

    png_init_io(ppng, handle);

    I_RenderReadPixels(&palette, &width, &height, &j);
    uint8_t *rowbuf = palette; // [crispy] pointer abuse!

    png_set_IHDR(ppng, pinfo, static_cast<png_uint_32>(width), static_cast<png_uint_32>(height),
#if SDL_VERSION_ATLEAST(2, 0, 5)
        8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
#else
        8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
#endif
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    /*
    png_set_IHDR(ppng, pinfo, width, height,
                 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    pcolor = malloc(sizeof(*pcolor) * 256);
    if (!pcolor)
    {
        fclose(handle);
        png_destroy_write_struct(&ppng, &pinfo);
        return;
    }

    for (i = 0; i < 256; i++)
    {
        pcolor[i].red   = *(palette + 3 * i);
        pcolor[i].green = *(palette + 3 * i + 1);
        pcolor[i].blue  = *(palette + 3 * i + 2);
    }

    png_set_PLTE(ppng, pinfo, pcolor, 256);
    free(pcolor);
*/

    png_write_info(ppng, pinfo);

    /*
    rowbuf = malloc(width);

    if (rowbuf)
    {
        for (i = 0; i < SCREENHEIGHT; i++)
        {
            // expand the row 5x
            for (j = 0; j < SCREENWIDTH; j++)
            {
                std::memset(rowbuf + j * w_factor, *(data + i*SCREENWIDTH + j), w_factor);
            }

            // write the row 6 times
            for (j = 0; j < h_factor; j++)
            {
                png_write_row(ppng, rowbuf);
            }
        }

        free(rowbuf);
    }
*/

    for (int i = 0; i < height; i++)
    {
        png_write_row(ppng, rowbuf);
        rowbuf += j;
    }
    free(palette);

    png_write_end(ppng, pinfo);
    png_destroy_write_struct(&ppng, &pinfo);
    fclose(handle);
}
#endif

//
// V_ScreenShot
//

void V_ScreenShot(const char *format)
{
    char        lbmname[16]; // haleyjd 20110213: BUG FIX - 12 is too small!
    const char *ext = nullptr;

    // find a file name to save it to

#ifdef HAVE_LIBPNG
    extern int png_screenshots;
    if (png_screenshots)
    {
        ext = "png";
    }
    else
#endif
    {
        ext = "pcx";
    }

    int i = 0;
    for (i = 0; i <= 9999; i++) // [crispy] increase screenshot filename limit
    {
        M_snprintf(lbmname, sizeof(lbmname), format, i, ext);

        if (!M_FileExists(lbmname))
        {
            break; // file doesn't exist
        }
    }

    if (i == 10000) // [crispy] increase screenshot filename limit
    {
#ifdef HAVE_LIBPNG
        if (png_screenshots)
        {
            I_Error("V_ScreenShot: Couldn't create a PNG");
        }
        else
#endif
        {
            I_Error("V_ScreenShot: Couldn't create a PCX");
        }
    }

#ifdef HAVE_LIBPNG
    if (png_screenshots)
    {
        WritePNGfile(lbmname, I_VideoBuffer,
            SCREENWIDTH, SCREENHEIGHT,
            cache_lump_name<uint8_t *>(DEH_String("PLAYPAL"), PU_CACHE));
    }
    else
#endif
    {
        // save the pcx file
        WritePCXfile(lbmname, I_VideoBuffer,
            SCREENWIDTH, SCREENHEIGHT,
            cache_lump_name<uint8_t *>(DEH_String("PLAYPAL"), PU_CACHE));
    }
}

constexpr auto MOUSE_SPEED_BOX_WIDTH  = 120;
constexpr auto MOUSE_SPEED_BOX_HEIGHT = 9;
#define MOUSE_SPEED_BOX_X      (SCREENWIDTH - MOUSE_SPEED_BOX_WIDTH - 10)
constexpr auto MOUSE_SPEED_BOX_Y      = 15;

//
// V_DrawMouseSpeedBox
//

static void DrawAcceleratingBox(int speed)
{
#ifndef CRISPY_TRUECOLOR
    int red    = I_GetPaletteIndex(0xff, 0x00, 0x00);
    int white  = I_GetPaletteIndex(0xff, 0xff, 0xff);
    int yellow = I_GetPaletteIndex(0xff, 0xff, 0x00);
#else
    int red         = I_MapRGB(0xff, 0x00, 0x00);
    int white       = I_MapRGB(0xff, 0xff, 0xff);
    int yellow      = I_MapRGB(0xff, 0xff, 0x00);
#endif

    // Calculate the position of the red threshold line when calibrating
    // acceleration.  This is 1/3 of the way along the box.

    int redline_x = MOUSE_SPEED_BOX_WIDTH / 3;
    int original_speed = 0;
    int linelen = 0;
    if (speed >= mouse_threshold)
    {
        // Undo acceleration and get back the original mouse speed
        original_speed = speed - mouse_threshold;
        original_speed = static_cast<int>(static_cast<float>(original_speed) / mouse_acceleration);
        original_speed += mouse_threshold;

        linelen = (original_speed * redline_x) / mouse_threshold;
    }
    else
    {
        linelen = (speed * redline_x) / mouse_threshold;
    }

    // Horizontal "thermometer"
    if (linelen > MOUSE_SPEED_BOX_WIDTH - 1)
    {
        linelen = MOUSE_SPEED_BOX_WIDTH - 1;
    }

    if (linelen < redline_x)
    {
        V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1,
            MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
            linelen, white);
    }
    else
    {
        V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1,
            MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
            redline_x, white);
        V_DrawHorizLine(MOUSE_SPEED_BOX_X + redline_x,
            MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
            linelen - redline_x, yellow);
    }

    // Draw acceleration threshold line
    V_DrawVertLine(MOUSE_SPEED_BOX_X + redline_x, MOUSE_SPEED_BOX_Y + 1,
        MOUSE_SPEED_BOX_HEIGHT - 2, red);
}

// Highest seen mouse turn speed. We scale the range of the thermometer
// according to this value, so that it never exceeds the range. Initially
// this is set to a 1:1 setting where 1 pixel = 1 unit of speed.
static int max_seen_speed = MOUSE_SPEED_BOX_WIDTH - 1;

static void DrawNonAcceleratingBox(int speed)
{
#ifndef CRISPY_TRUECOLOR
    int white = I_GetPaletteIndex(0xff, 0xff, 0xff);
#else
    int white       = I_MapRGB(0xff, 0xff, 0xff);
#endif

    if (speed > max_seen_speed)
    {
        max_seen_speed = speed;
    }

    // Draw horizontal "thermometer":
    int linelen = speed * (MOUSE_SPEED_BOX_WIDTH - 1) / max_seen_speed;

    V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1,
        MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
        linelen, white);
}

void V_DrawMouseSpeedBox(int speed)
{
    extern int usemouse;
    // If the mouse is turned off, don't draw the box at all.
    if (!usemouse)
    {
        return;
    }

    // Get palette indices for colors for widget. These depend on the
    // palette of the game being played.

#ifndef CRISPY_TRUECOLOR
    int bgcolor     = I_GetPaletteIndex(0x77, 0x77, 0x77);
    int bordercolor = I_GetPaletteIndex(0x55, 0x55, 0x55);
    int black       = I_GetPaletteIndex(0x00, 0x00, 0x00);
#else
    int bgcolor     = I_MapRGB(0x77, 0x77, 0x77);
    int bordercolor = I_MapRGB(0x55, 0x55, 0x55);
    int black       = I_MapRGB(0x00, 0x00, 0x00);
#endif

    // Calculate box position

    V_DrawFilledBox(MOUSE_SPEED_BOX_X, MOUSE_SPEED_BOX_Y,
        MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bgcolor);
    V_DrawBox(MOUSE_SPEED_BOX_X, MOUSE_SPEED_BOX_Y,
        MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bordercolor);
    V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1, MOUSE_SPEED_BOX_Y + 4,
        MOUSE_SPEED_BOX_WIDTH - 2, black);

    // If acceleration is used, draw a box that helps to calibrate the
    // threshold point.
    if (mouse_threshold > 0 && fabs(mouse_acceleration - 1) > 0.01)
    {
        DrawAcceleratingBox(speed);
    }
    else
    {
        DrawNonAcceleratingBox(speed);
    }
}
