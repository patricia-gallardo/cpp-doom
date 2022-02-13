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
//	Rendering main loop and setup functions,
//	 utility functions (BSP, geometry, trigonometry).
//	See tables.c, too.
//


#include <cstdlib>
#include <cassert>


#include "doomdef.hpp"
#include "doomstat.hpp" // [AM] leveltime, paused, menuactive
#include "d_loop.hpp"

#include "m_bbox.hpp"
#include "m_menu.hpp"

#include "i_system.hpp" // [crispy] I_Realloc()
#include "p_local.hpp"  // [crispy] MLOOKUNIT
#include "r_local.hpp"
#include "r_sky.hpp"
#include "st_stuff.hpp" // [crispy] ST_refreshBackground()


// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048


int viewangleoffset;

// increment every time a check is made
int validcount = 1;


lighttable_t *        fixedcolormap;
extern lighttable_t **walllights;

int centerx;
int centery;

fixed_t centerxfrac;
fixed_t centeryfrac;
fixed_t projection;

// just for profiling purposes
int framecount;

int sscount;
int linecount;
int loopcount;

fixed_t viewx;
fixed_t viewy;
fixed_t viewz;

angle_t viewangle;

fixed_t viewcos;
fixed_t viewsin;

player_t *viewplayer;

// 0 = high, 1 = low
int detailshift;

//
// precalculated math tables
//
angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
int viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t xtoviewangle[MAXWIDTH + 1];

// [crispy] parameterized for smooth diminishing lighting
lighttable_t ***scalelight      = nullptr;
lighttable_t ** scalelightfixed = nullptr;
lighttable_t ***zlight          = nullptr;

// bumped light from gun blasts
int extralight;

// [crispy] parameterized for smooth diminishing lighting
int LIGHTLEVELS;
int LIGHTSEGSHIFT;
int LIGHTBRIGHT;
int MAXLIGHTSCALE;
int LIGHTSCALESHIFT;
int MAXLIGHTZ;
int LIGHTZSHIFT;


void (*colfunc)();
void (*basecolfunc)();
void (*fuzzcolfunc)();
void (*transcolfunc)();
void (*tlcolfunc)();
void (*spanfunc)();


//
// R_AddPointToBox
// Expand a given bbox
// so that it encloses a given point.
//
void R_AddPointToBox(int x,
    int                  y,
    fixed_t *            box)
{
    if (x < box[BOXLEFT])
        box[BOXLEFT] = x;
    if (x > box[BOXRIGHT])
        box[BOXRIGHT] = x;
    if (y < box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    if (y > box[BOXTOP])
        box[BOXTOP] = y;
}


//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
int R_PointOnSide(fixed_t x,
    fixed_t               y,
    node_t *              node)
{
    fixed_t dx;
    fixed_t dy;
    fixed_t left;
    fixed_t right;

    if (!node->dx)
    {
        if (x <= node->x)
            return node->dy > 0;

        return node->dy < 0;
    }
    if (!node->dy)
    {
        if (y <= node->y)
            return node->dx < 0;

        return node->dx > 0;
    }

    dx = (x - node->x);
    dy = (y - node->y);

    // Try to quickly decide by looking at sign bits.
    if ((static_cast<unsigned int>(node->dy ^ node->dx ^ dx ^ dy)) & 0x80000000)
    {
        if ((static_cast<unsigned int>(node->dy ^ dx)) & 0x80000000)
        {
            // (left is negative)
            return 1;
        }
        return 0;
    }

    left  = FixedMul(node->dy >> FRACBITS, dx);
    right = FixedMul(dy, node->dx >> FRACBITS);

    if (right < left)
    {
        // front side
        return 0;
    }
    // back side
    return 1;
}


int R_PointOnSegSide(fixed_t x,
    fixed_t                  y,
    seg_t *                  line)
{
    fixed_t lx;
    fixed_t ly;
    fixed_t ldx;
    fixed_t ldy;
    fixed_t dx;
    fixed_t dy;
    fixed_t left;
    fixed_t right;

    lx = line->v1->x;
    ly = line->v1->y;

    ldx = line->v2->x - lx;
    ldy = line->v2->y - ly;

    if (!ldx)
    {
        if (x <= lx)
            return ldy > 0;

        return ldy < 0;
    }
    if (!ldy)
    {
        if (y <= ly)
            return ldx < 0;

        return ldx > 0;
    }

    dx = (x - lx);
    dy = (y - ly);

    // Try to quickly decide by looking at sign bits.
    if ((static_cast<unsigned int>(ldy ^ ldx ^ dx ^ dy)) & 0x80000000)
    {
        if ((static_cast<unsigned int>(ldy ^ dx)) & 0x80000000)
        {
            // (left is negative)
            return 1;
        }
        return 0;
    }

    left  = FixedMul(ldy >> FRACBITS, dx);
    right = FixedMul(dy, ldx >> FRACBITS);

    if (right < left)
    {
        // front side
        return 0;
    }
    // back side
    return 1;
}


//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table.

//


// [crispy] turned into a general R_PointToAngle() flavor
// called with either slope_div = SlopeDivCrispy() from R_PointToAngleCrispy()
// or slope_div = SlopeDiv() else
angle_t
    R_PointToAngleSlope(fixed_t x,
        fixed_t                 y,
        int (*slope_div)(unsigned int num, unsigned int den))
{
    x -= viewx;
    y -= viewy;

    if ((!x) && (!y))
        return 0;

    if (x >= 0)
    {
        // x >=0
        if (y >= 0)
        {
            // y>= 0

            if (x > y)
            {
                // octant 0
                return tantoangle[slope_div(static_cast<unsigned int>(y), static_cast<unsigned int>(x))];
            }
            else
            {
                // octant 1
                return ANG90 - 1 - tantoangle[slope_div(static_cast<unsigned int>(x), static_cast<unsigned int>(y))];
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x > y)
            {
                // octant 8
                // Subtracting from 0u avoids compiler warnings
                return 0u - tantoangle[slope_div(static_cast<unsigned int>(y), static_cast<unsigned int>(x))];
            }
            else
            {
                // octant 7
                return ANG270 + tantoangle[slope_div(static_cast<unsigned int>(x), static_cast<unsigned int>(y))];
            }
        }
    }
    else
    {
        // x<0
        x = -x;

        if (y >= 0)
        {
            // y>= 0
            if (x > y)
            {
                // octant 3
                return ANG180 - 1 - tantoangle[slope_div(static_cast<unsigned int>(y), static_cast<unsigned int>(x))];
            }
            else
            {
                // octant 2
                return ANG90 + tantoangle[slope_div(static_cast<unsigned int>(x), static_cast<unsigned int>(y))];
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x > y)
            {
                // octant 4
                return ANG180 + tantoangle[slope_div(static_cast<unsigned int>(y), static_cast<unsigned int>(x))];
            }
            else
            {
                // octant 5
                return ANG270 - 1 - tantoangle[slope_div(static_cast<unsigned int>(x), static_cast<unsigned int>(y))];
            }
        }
    }
    [[unreachable]];
}

angle_t
    R_PointToAngle(fixed_t x,
        fixed_t            y)
{
    return R_PointToAngleSlope(x, y, SlopeDiv);
}

// [crispy] overflow-safe R_PointToAngle() flavor
// called only from R_CheckBBox(), R_AddLine() and P_SegLengths()
angle_t
    R_PointToAngleCrispy(fixed_t x,
        fixed_t                  y)
{
    // [crispy] fix overflows for very long distances
    int64_t y_viewy = static_cast<int64_t>(y) - viewy;
    int64_t x_viewx = static_cast<int64_t>(x) - viewx;

    // [crispy] the worst that could happen is e.g. INT_MIN-INT_MAX = 2*INT_MIN
    if (x_viewx < INT_MIN || x_viewx > INT_MAX || y_viewy < INT_MIN || y_viewy > INT_MAX)
    {
        // [crispy] preserving the angle by halfing the distance in both directions
        x = static_cast<fixed_t >(x_viewx / 2 + viewx);
        y = static_cast<fixed_t >(y_viewy / 2 + viewy);
    }

    return R_PointToAngleSlope(x, y, SlopeDivCrispy);
}

angle_t
    R_PointToAngle2(fixed_t x1,
        fixed_t             y1,
        fixed_t             x2,
        fixed_t             y2)
{
    viewx = x1;
    viewy = y1;

    // [crispy] R_PointToAngle2() is never called during rendering
    return R_PointToAngleSlope(x2, y2, SlopeDiv);
}


fixed_t
    R_PointToDist(fixed_t x,
        fixed_t           y)
{
    int     angle;
    fixed_t dx;
    fixed_t dy;
    fixed_t temp;
    fixed_t dist;
    fixed_t frac;

    dx = std::abs(x - viewx);
    dy = std::abs(y - viewy);

    if (dy > dx)
    {
        temp = dx;
        dx   = dy;
        dy   = temp;
    }

    // Fix crashes in udm1.wad

    if (dx != 0)
    {
        frac = FixedDiv(dy, dx);
    }
    else
    {
        frac = 0;
    }

    angle = (tantoangle[frac >> DBITS] + ANG90) >> ANGLETOFINESHIFT;

    // use as cosine
    dist = FixedDiv(dx, finesine[angle]);

    return dist;
}


//
// R_InitPointToAngle
//
void R_InitPointToAngle()
{
    // UNUSED - now getting from tables.c
#if 0
    int	i;
    long	t;
    float	f;
//
// slope (tangent) to angle lookup
//
    for (i=0 ; i<=SLOPERANGE ; i++)
    {
	f = atan( (float)i/SLOPERANGE )/(3.141592657*2);
	t = 0xffffffff*f;
	tantoangle[i] = t;
    }
#endif
}


// [crispy] WiggleFix: move R_ScaleFromGlobalAngle function to r_segs.c,
// above R_StoreWallRange
#if 0
//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
fixed_t R_ScaleFromGlobalAngle (angle_t visangle)
{
    fixed_t		scale;
    angle_t		anglea;
    angle_t		angleb;
    int			sinea;
    int			sineb;
    fixed_t		num;
    int			den;

    // UNUSED
#if 0
{
    fixed_t		dist;
    fixed_t		z;
    fixed_t		sinv;
    fixed_t		cosv;
	
    sinv = finesine[(visangle-rw_normalangle)>>ANGLETOFINESHIFT];	
    dist = FixedDiv (rw_distance, sinv);
    cosv = finecosine[(viewangle-visangle)>>ANGLETOFINESHIFT];
    z = std::abs(FixedMul (dist, cosv));
    scale = FixedDiv(projection, z);
    return scale;
}
#endif

    anglea = ANG90 + (visangle-viewangle);
    angleb = ANG90 + (visangle-rw_normalangle);

    // both sines are allways positive
    sinea = finesine[anglea>>ANGLETOFINESHIFT];	
    sineb = finesine[angleb>>ANGLETOFINESHIFT];
    num = FixedMul(projection,sineb)<<detailshift;
    den = FixedMul(rw_distance,sinea);

    if (den > num>>FRACBITS)
    {
	scale = FixedDiv (num, den);

	if (scale > 64*FRACUNIT)
	    scale = 64*FRACUNIT;
	else if (scale < 256)
	    scale = 256;
    }
    else
	scale = 64*FRACUNIT;
	
    return scale;
}
#endif


// [AM] Interpolate between two angles.
angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale)
{
    if (nangle == oangle)
        return nangle;
    else if (nangle > oangle)
    {
        if (nangle - oangle < ANG270)
            return oangle + static_cast<angle_t>((nangle - oangle) * FIXED2DOUBLE(scale));
        else // Wrapped around
            return oangle - static_cast<angle_t>((oangle - nangle) * FIXED2DOUBLE(scale));
    }
    else // nangle < oangle
    {
        if (oangle - nangle < ANG270)
            return oangle - static_cast<angle_t>((oangle - nangle) * FIXED2DOUBLE(scale));
        else // Wrapped around
            return oangle + static_cast<angle_t>((nangle - oangle) * FIXED2DOUBLE(scale));
    }
}

//
// R_InitTables
//
void R_InitTables()
{
    // UNUSED: now getting from tables.c
#if 0
    int		i;
    float	a;
    float	fv;
    int		t;
    
    // viewangle tangent table
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
	a = (i-FINEANGLES/4+0.5)*PI*2/FINEANGLES;
	fv = FRACUNIT*tan (a);
	t = fv;
	finetangent[i] = t;
    }
    
    // finesine table
    for (i=0 ; i<5*FINEANGLES/4 ; i++)
    {
	// OPTIMIZE: mirror...
	a = (i+0.5)*PI*2/FINEANGLES;
	t = FRACUNIT*sin (a);
	finesine[i] = t;
    }
#endif
}


//
// R_InitTextureMapping
//
void R_InitTextureMapping()
{
    int     i;
    int     x;
    int     t;
    fixed_t focallength;
    fixed_t focalwidth;

    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.
    //
    // Calc focallength
    //  so FIELDOFVIEW angles covers SCREENWIDTH.
    // [crispy] in widescreen mode, make sure the same number of horizontal
    // pixels shows the same part of the game scene as in regular rendering mode
    focalwidth  = crispy->widescreen ? ((HIRESWIDTH >> detailshift) / 2) << FRACBITS : centerxfrac;
    focallength = FixedDiv(focalwidth,
        finetangent[FINEANGLES / 4 + FIELDOFVIEW / 2]);

    for (i = 0; i < FINEANGLES / 2; i++)
    {
        if (finetangent[i] > FRACUNIT * 2)
            t = -1;
        else if (finetangent[i] < -FRACUNIT * 2)
            t = viewwidth + 1;
        else
        {
            t = FixedMul(finetangent[i], focallength);
            t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;

            if (t < -1)
                t = -1;
            else if (t > viewwidth + 1)
                t = viewwidth + 1;
        }
        viewangletox[i] = t;
    }

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.
    for (x = 0; x <= viewwidth; x++)
    {
        i = 0;
        while (viewangletox[i] > x)
            i++;
        xtoviewangle[x] = static_cast<angle_t>((i << ANGLETOFINESHIFT) - ANG90);
    }

    // Take out the fencepost cases from viewangletox.
    for (i = 0; i < FINEANGLES / 2; i++)
    {
        t = FixedMul(finetangent[i], focallength);
        t = centerx - t;

        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == viewwidth + 1)
            viewangletox[i] = viewwidth;
    }

    clipangle = xtoviewangle[0];
}


//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP 2

void R_InitLightTables()
{
    int i;
    int j;
    int level;
    int startmap_local;
    int scale;

    if (scalelight)
    {
        for (i = 0; i < LIGHTLEVELS; i++)
        {
            free(scalelight[i]);
        }
        free(scalelight);
    }

    if (scalelightfixed)
    {
        free(scalelightfixed);
    }

    if (zlight)
    {
        for (i = 0; i < LIGHTLEVELS; i++)
        {
            free(zlight[i]);
        }
        free(zlight);
    }

    // [crispy] smooth diminishing lighting
    if (crispy->smoothlight)
    {
        LIGHTLEVELS     = 32;
        LIGHTSEGSHIFT   = 3;
        LIGHTBRIGHT     = 2;
        MAXLIGHTSCALE   = 48;
        LIGHTSCALESHIFT = 12;
        MAXLIGHTZ       = 1024;
        LIGHTZSHIFT     = 17;
    }
    else
    {
        LIGHTLEVELS     = 16;
        LIGHTSEGSHIFT   = 4;
        LIGHTBRIGHT     = 1;
        MAXLIGHTSCALE   = 48;
        LIGHTSCALESHIFT = 12;
        MAXLIGHTZ       = 128;
        LIGHTZSHIFT     = 20;
    }

    scalelight      = static_cast<lighttable_t ***>(malloc(static_cast<unsigned long>(LIGHTLEVELS) * sizeof(*scalelight)));
    scalelightfixed = static_cast<lighttable_t **>(malloc(static_cast<unsigned long>(MAXLIGHTSCALE) * sizeof(*scalelightfixed)));
    zlight          = static_cast<lighttable_t ***>(malloc(static_cast<unsigned long>(LIGHTLEVELS) * sizeof(*zlight)));

    // Calculate the light levels to use
    //  for each level / distance combination.
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        zlight[i] =
            static_cast<lighttable_t **>(malloc(static_cast<unsigned long>(MAXLIGHTZ) * sizeof(**zlight)));

        startmap_local = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
        for (j = 0; j < MAXLIGHTZ; j++)
        {
            scale = FixedDiv((ORIGWIDTH / 2 * FRACUNIT), (j + 1) << LIGHTZSHIFT);
            scale >>= LIGHTSCALESHIFT;
            level = startmap_local - scale / DISTMAP;

            if (level < 0)
                level = 0;

            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;

            zlight[i][j] = colormaps + level * 256;
        }
    }
}


//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
bool setsizeneeded;
int     setblocks;
int     setdetail;

// [crispy] lookup table for horizontal screen coordinates
int  flipscreenwidth[MAXWIDTH];
int *flipviewwidth;

void R_SetViewSize(int blocks,
    int                detail)
{
    setsizeneeded = true;
    setblocks     = blocks;
    setdetail     = detail;
}


//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize()
{
    fixed_t cosadj;
    fixed_t dy;
    int     i;
    int     j;
    int     level;
    int     startmap_local;

    setsizeneeded = false;

    // [crispy] make absolutely sure screenblocks is never < 11 in widescreen mode
    if (crispy->widescreen)
    {
        extern void M_SizeDisplay(int choice);

        while (setblocks < 11)
        {
            M_SizeDisplay(1);
            R_ExecuteSetViewSize();
            return;
        }
    }

    if (setblocks >= 11) // [crispy] Crispy HUD
    {
        scaledviewwidth = SCREENWIDTH;
        viewheight      = SCREENHEIGHT;
    }
    else
    {
        scaledviewwidth = (setblocks * 32) << crispy->hires;
        viewheight      = ((setblocks * 168 / 10) & ~7) << crispy->hires;
    }

    detailshift = setdetail;
    viewwidth   = scaledviewwidth >> detailshift;

    centery     = viewheight / 2;
    centerx     = viewwidth / 2;
    centerxfrac = centerx << FRACBITS;
    centeryfrac = centery << FRACBITS;
    projection  = MIN(centerxfrac, ((HIRESWIDTH >> detailshift) / 2) << FRACBITS);

    if (!detailshift)
    {
        colfunc = basecolfunc = R_DrawColumn;
        fuzzcolfunc           = R_DrawFuzzColumn;
        transcolfunc          = R_DrawTranslatedColumn;
        tlcolfunc             = R_DrawTLColumn;
        spanfunc              = R_DrawSpan;
    }
    else
    {
        colfunc = basecolfunc = R_DrawColumnLow;
        fuzzcolfunc           = R_DrawFuzzColumnLow;
        transcolfunc          = R_DrawTranslatedColumnLow;
        tlcolfunc             = R_DrawTLColumnLow;
        spanfunc              = R_DrawSpanLow;
    }

    R_InitBuffer(scaledviewwidth, viewheight);

    R_InitTextureMapping();

    // psprite scales
    pspritescale  = FRACUNIT * MIN(viewwidth, HIRESWIDTH >> detailshift) / ORIGWIDTH;
    pspriteiscale = FRACUNIT * ORIGWIDTH / MIN(viewwidth, HIRESWIDTH >> detailshift);

    // thing clipping
    for (i = 0; i < viewwidth; i++)
        screenheightarray[i] = viewheight;

    // planes
    for (i = 0; i < viewheight; i++)
    {
        // [crispy] re-generate lookup-table for yslope[] (free look)
        // whenever "detailshift" or "screenblocks" change
        const fixed_t num = MIN(viewwidth << detailshift, HIRESWIDTH) / 2 * FRACUNIT;
        for (j = 0; j < LOOKDIRS; j++)
        {
            dy            = ((i - (viewheight / 2 + ((j - LOOKDIRMIN) * (1 << crispy->hires)) * (screenblocks < 11 ? screenblocks : 11) / 10)) << FRACBITS) + FRACUNIT / 2;
            dy            = std::abs(dy);
            yslopes[j][i] = FixedDiv(num, dy);
        }
    }
    yslope = yslopes[LOOKDIRMIN];

    for (i = 0; i < viewwidth; i++)
    {
        cosadj       = std::abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv(FRACUNIT, cosadj);
    }

    // Calculate the light levels to use
    //  for each level / scale combination.
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        scalelight[i] = static_cast<lighttable_t **>(malloc(static_cast<unsigned long>(MAXLIGHTSCALE) * sizeof(**scalelight)));

        startmap_local = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
        for (j = 0; j < MAXLIGHTSCALE; j++)
        {
            level = startmap_local - j * HIRESWIDTH / MIN(viewwidth << detailshift, HIRESWIDTH) / DISTMAP;

            if (level < 0)
                level = 0;

            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;

            scalelight[i][j] = colormaps + level * 256;
        }
    }

    // [crispy] lookup table for horizontal screen coordinates
    for (i = 0, j = SCREENWIDTH - 1; i < SCREENWIDTH; i++, j--)
    {
        flipscreenwidth[i] = crispy->fliplevels ? j : i;
    }

    flipviewwidth = flipscreenwidth + (crispy->fliplevels ? (SCREENWIDTH - scaledviewwidth) : 0);

    // [crispy] forcefully initialize the status bar backing screen
    ST_refreshBackground(true);
}


//
// R_Init
//


void R_Init()
{
    R_InitData();
    printf(".");
    R_InitPointToAngle();
    printf(".");
    R_InitTables();
    // viewwidth / viewheight / detailLevel are set by the defaults
    printf(".");

    R_SetViewSize(screenblocks, detailLevel);
    R_InitPlanes();
    printf(".");
    R_InitLightTables();
    printf(".");
    R_InitSkyMap();
    R_InitTranslationTables();
    printf(".");

    framecount = 0;
}


//
// R_PointInSubsector
//
subsector_t *
    R_PointInSubsector(fixed_t x,
        fixed_t                y)
{
    node_t *node;
    int     side;

    // single subsector is a special case
    if (!numnodes)
        return subsectors;

    int nodenum = numnodes - 1;

    while (!(static_cast<unsigned int>(nodenum) & NF_SUBSECTOR))
    {
        node    = &nodes[nodenum];
        side    = R_PointOnSide(x, y, node);
        nodenum = node->children[side];
    }

    return &subsectors[static_cast<unsigned int>(nodenum) & ~NF_SUBSECTOR];
}


//
// R_SetupFrame
//
void R_SetupFrame(player_t *player)
{
    int i;
    int tempCentery;
    int pitch;

    viewplayer = player;

    // [AM] Interpolate the player camera if the feature is enabled.
    if (crispy->uncapped &&
        // Don't interpolate on the first tic of a level,
        // otherwise oldviewz might be garbage.
        leveltime > 1 &&
        // Don't interpolate if the player did something
        // that would necessitate turning it off for a tic.
        player->mo->interp == 1 &&
        // Don't interpolate during a paused state
        leveltime > oldleveltime)
    {
        // Interpolate player camera from their old position to their current one.
        viewx     = player->mo->oldx + FixedMul(player->mo->x - player->mo->oldx, fractionaltic);
        viewy     = player->mo->oldy + FixedMul(player->mo->y - player->mo->oldy, fractionaltic);
        viewz     = static_cast<fixed_t>(player->oldviewz + static_cast<unsigned int>(FixedMul(static_cast<fixed_t>(static_cast<unsigned int>(player->viewz) - player->oldviewz), fractionaltic)));
        viewangle = R_InterpolateAngle(player->mo->oldangle, player->mo->angle, fractionaltic) + static_cast<unsigned int>(viewangleoffset);

        double  oldlookdir = player->oldlookdir + (player->lookdir - player->oldlookdir) * FIXED2DOUBLE(fractionaltic);
        fixed_t recoil     = player->oldrecoilpitch + FixedMul(player->recoilpitch - player->oldrecoilpitch, fractionaltic);
        pitch              = static_cast<int>(oldlookdir / MLOOKUNIT + recoil);
    }
    else
    {
        viewx     = player->mo->x;
        viewy     = player->mo->y;
        viewz     = player->viewz;
        viewangle = player->mo->angle + static_cast<unsigned int>(viewangleoffset);

        // [crispy] pitch is actual lookdir and weapon pitch
        pitch = player->lookdir / MLOOKUNIT + player->recoilpitch;
    }

    extralight = player->extralight;

    if (pitch > LOOKDIRMAX)
        pitch = LOOKDIRMAX;
    else if (pitch < -LOOKDIRMIN)
        pitch = -LOOKDIRMIN;

    // apply new yslope[] whenever "lookdir", "detailshift" or "screenblocks" change
    tempCentery = viewheight / 2 + (pitch * (1 << crispy->hires)) * (screenblocks < 11 ? screenblocks : 11) / 10;
    if (centery != tempCentery)
    {
        centery     = tempCentery;
        centeryfrac = centery << FRACBITS;
        yslope      = yslopes[LOOKDIRMIN + pitch];
    }

    viewsin = finesine[viewangle >> ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];

    sscount = 0;

    if (player->fixedcolormap)
    {
        fixedcolormap =
            colormaps
            + player->fixedcolormap * 256;

        walllights = scalelightfixed;

        for (i = 0; i < MAXLIGHTSCALE; i++)
            scalelightfixed[i] = fixedcolormap;
    }
    else
        fixedcolormap = 0;

    framecount++;
    validcount++;
}


//
// R_RenderView
//
void R_RenderPlayerView(player_t *player)
{
    extern void V_DrawFilledBox(int x, int y, int w, int h, int c);
    extern void R_InterpolateTextureOffsets();

    R_SetupFrame(player);

    // Clear buffers.
    R_ClearClipSegs();
    R_ClearDrawSegs();
    R_ClearPlanes();
    R_ClearSprites();
    if (automapactive && !crispy->automapoverlay)
    {
        R_RenderBSPNode(numnodes - 1);
        return;
    }

    // [crispy] flashing HOM indicator
    V_DrawFilledBox(viewwindowx, viewwindowy,
        scaledviewwidth, viewheight,
#ifndef CRISPY_TRUECOLOR
        crispy->flashinghom ? (176 + (gametic % 16)) : 0);
#else
        colormaps[crispy->flashinghom ? (176 + (gametic % 16)) : 0]);
#endif

    // check for new console commands.
    NetUpdate();

    // [crispy] smooth texture scrolling
    R_InterpolateTextureOffsets();
    // The head node is the last node output.
    R_RenderBSPNode(numnodes - 1);

    // Check for new console commands.
    NetUpdate();

    R_DrawPlanes();

    // Check for new console commands.
    NetUpdate();

    // [crispy] draw fuzz effect independent of rendering frame rate
    R_SetFuzzPosDraw();
    R_DrawMasked();

    // Check for new console commands.
    NetUpdate();
}
