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


#ifndef __AMMAP_H__
#define __AMMAP_H__

// For use if I do walls with outsides/insides
constexpr auto REDS        = 12 * 8;
constexpr auto REDRANGE    = 1; // 16
constexpr auto BLUES       = (256 - 4 * 16 + 8);
constexpr auto BLUERANGE   = 1; // 8
constexpr auto GREENS      = (33 * 8);
constexpr auto GREENRANGE  = 1; // 16
constexpr auto GRAYS       = (5 * 8);
constexpr auto GRAYSRANGE  = 1; // 16
constexpr auto BROWNS      = (14 * 8);
constexpr auto BROWNRANGE  = 1; // 16
constexpr auto YELLOWS     = 10 * 8;
constexpr auto YELLOWRANGE = 1;
constexpr auto BLACK       = 0;
constexpr auto WHITE       = 4 * 8;
constexpr auto PARCH       = 13 * 8 - 1;
constexpr auto BLOODRED    = 177;
constexpr auto BLUEKEY     = 157;
constexpr auto YELLOWKEY   = 137;
constexpr auto GREENKEY    = 198;

// Automap colors

#define AM_PLR1_COLOR 157       // Blue
#define AM_PLR2_COLOR 177       // Red
#define AM_PLR3_COLOR 137       // Yellow
#define AM_PLR4_COLOR 198       // Green
#define AM_PLR5_COLOR 215       // Jade
#define AM_PLR6_COLOR 32        // White
#define AM_PLR7_COLOR 106       // Hazel
#define AM_PLR8_COLOR 234       // Purple

#define BACKGROUND	PARCH
#define YOURCOLORS	WHITE
#define YOURRANGE	0
#define WALLCOLORS	REDS
#define WALLRANGE	REDRANGE
#define TSWALLCOLORS	GRAYS
#define TSWALLRANGE	GRAYSRANGE
#define FDWALLCOLORS	BROWNS
#define FDWALLRANGE	BROWNRANGE
#define CDWALLCOLORS	YELLOWS
#define CDWALLRANGE	YELLOWRANGE
#define THINGCOLORS	GREENS
#define THINGRANGE	GREENRANGE
#define SECRETWALLCOLORS WALLCOLORS
#define SECRETWALLRANGE WALLRANGE
#define GRIDCOLORS	(GRAYS + GRAYSRANGE/2)
#define GRIDRANGE	0
#define XHAIRCOLORS	GRAYS

// drawing stuff

#define AM_NUMMARKPOINTS 10

#define AM_MSGHEADER (('a'<<24)+('m'<<16))
#define AM_MSGENTERED (AM_MSGHEADER | ('e'<<8))
#define AM_MSGEXITED (AM_MSGHEADER | ('x'<<8))

#define INITSCALEMTOF (.2*FRACUNIT)     // scale on entry
// how much the automap moves window per tic in frame-buffer coordinates
#define F_PANINC	4       // moves 140 pixels in 1 second
// how much zoom-in per tic
#define M_ZOOMIN        (static_cast<int>(1.02*FRACUNIT)) // goes to 2x in 1 second
// how much zoom-out per tic
#define M_ZOOMOUT       (static_cast<int>(FRACUNIT/1.02)) // pulls out to 0.5x in 1 second

// translates between frame-buffer and map distances
#define FTOM(x) FixedMul(((x)<<16),scale_ftom)
#define MTOF(x) (FixedMul((x),scale_mtof)>>16)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))

// the following is crap
#define LINE_NEVERSEE ML_DONTDRAW

typedef struct
{
    int x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef vertex_t mpoint_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

typedef struct
{
    fixed_t slp, islp;
} islope_t;

// extern int f_x, f_y, f_w, f_h;

#endif
