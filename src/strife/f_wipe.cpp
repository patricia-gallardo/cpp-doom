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

#include <string.h>

#include "z_zone.hpp"
#include "i_video.hpp"
#include "v_video.hpp"
#include "m_random.hpp"

#include "doomtype.hpp"

#include "r_defs.hpp"   // haleyjd [STRIFE]
#include "r_draw.hpp"

#include "f_wipe.hpp"
#include "memory.hpp"

//
//                       SCREEN WIPE PACKAGE
//

// when zero, stop the wipe
static bool	go = 0;

static uint8_t *wipe_scr_start;
static uint8_t *wipe_scr_end;
static uint8_t *wipe_scr;


void
wipe_shittyColMajorXform
( short*	array,
  int		width,
  int		height )
{
    int		x;
    int		y;
    short*	dest;

    dest = zmalloc<short*>(width*height*2, PU_STATIC, 0);

    for(y=0;y<height;y++)
	for(x=0;x<width;x++)
	    dest[x*height+y] = array[y*width+x];

    memcpy(array, dest, width*height*2);

    Z_Free(dest);

}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int wipe_initColorXForm(int width, int height, int)
{
    memcpy(wipe_scr, wipe_scr_start, width*height);
    return 0;
}

//
// wipe_doColorXForm
//
// haleyjd 08/26/10: [STRIFE]
// * Rogue modified the unused ColorXForm wipe in-place in order to implement 
//   their distinctive crossfade wipe.
//
int wipe_doColorXForm(int width, int height, int)
{
    uint8_t *cur_screen = wipe_scr;
    uint8_t *end_screen = wipe_scr_end;
    int   pix = width*height;
    int   i;
    bool changed = false;

    for(i = pix; i > 0; i--)
    {
        if(*cur_screen != *end_screen)
        {
            changed = true;
            *cur_screen = xlatab[(*cur_screen << 8) + *end_screen];
        }
        ++cur_screen;
        ++end_screen;
    }

    return !changed;
}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int wipe_exitColorXForm(int, int, int)
{
    return 0;
}


static int*	y;

int wipe_initMelt(int width, int height, int)
{
    int i, r;
    
    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, width*height);
    
    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((short*)wipe_scr_start, width/2, height);
    wipe_shittyColMajorXform((short*)wipe_scr_end, width/2, height);
    
    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = zmalloc<int *>(width*sizeof(int), PU_STATIC, 0);
    y[0] = -(M_Random()%16);
    for (i=1;i<width;i++)
    {
	r = (M_Random()%3) - 1;
	y[i] = y[i-1] + r;
	if (y[i] > 0) y[i] = 0;
	else if (y[i] == -16) y[i] = -15;
    }

    return 0;
}

int
wipe_doMelt
( int	width,
  int	height,
  int	ticks )
{
    int		i;
    int		j;
    int		dy;
    int		idx;
    
    short*	s;
    short*	d;
    bool	done = true;

    width/=2;

    while (ticks--)
    {
	for (i=0;i<width;i++)
	{
	    if (y[i]<0)
	    {
		y[i]++; done = false;
	    }
	    else if (y[i] < height)
	    {
		dy = (y[i] < 16) ? y[i]+1 : 8;
		if (y[i]+dy >= height) dy = height - y[i];
		s = &((short *)wipe_scr_end)[i*height+y[i]];
		d = &((short *)wipe_scr)[y[i]*width+i];
		idx = 0;
		for (j=dy;j;j--)
		{
		    d[idx] = *(s++);
		    idx += width;
		}
		y[i] += dy;
		s = &((short *)wipe_scr_start)[i*height];
		d = &((short *)wipe_scr)[y[i]*width+i];
		idx = 0;
		for (j=height-y[i];j;j--)
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

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int wipe_StartScreen(int, int, int, int)
{
    wipe_scr_start = zmalloc<uint8_t *>(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, nullptr);
    I_ReadScreen(wipe_scr_start);
    return 0;
}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int
wipe_EndScreen
( int	x,
  int	y_pos,
  int	width,
  int	height )
{
    wipe_scr_end = zmalloc<uint8_t *>(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, nullptr);
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(x, y_pos, width, height, wipe_scr_start); // restore start scr.
    return 0;
}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int
wipe_ScreenWipe
( int	wipeno,
  int	x,
  int	y_pos,
  int	width,
  int	height,
  int	ticks )
{
    int rc;
    static int (*wipes[])(int, int, int) =
    {
	wipe_initColorXForm, wipe_doColorXForm, wipe_exitColorXForm,
	wipe_initMelt, wipe_doMelt, wipe_exitMelt
    };

    ticks <<= crispy->hires;

    // initial stuff
    if (!go)
    {
	go = 1;
        // haleyjd 20110629 [STRIFE]: We *must* use a temp buffer here.
	wipe_scr = zmalloc<uint8_t *>(width * height, PU_STATIC, 0); // DEBUG
	//wipe_scr = I_VideoBuffer;
	(*wipes[wipeno*3])(width, height, ticks);
    }

    // do a piece of wipe-in
    V_MarkRect(0, 0, width, height);
    rc = (*wipes[wipeno*3+1])(width, height, ticks);

    // haleyjd 20110629 [STRIFE]: Copy temp buffer to the real screen.
    V_DrawBlock(x, y_pos, width, height, wipe_scr);

    // final stuff
    if (rc)
    {
	go = 0;
	(*wipes[wipeno*3+2])(width, height, ticks);
    }

    return !go;
}

