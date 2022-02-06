//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 Fabian Greffrath
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
//	Crispy Doom specific variables.
//


#ifndef __CRISPY_H__
#define __CRISPY_H__

#include "doomtype.hpp"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef BETWEEN
#define BETWEEN(l, u, x) (((l) > (x)) ? (l) : ((x) > (u)) ? (u) : (x))
#endif

typedef struct
{
    // [crispy] "crispness" config variables
    int automapoverlay;
    int automaprotate;
    int automapstats;
    int bobfactor;
    int brightmaps;
    int centerweapon;
    int coloredblood;
    int coloredhud;
    int crosshair;
    int crosshairhealth;
    int crosshairtarget;
    int crosshairtype;
    int demotimer;
    int demotimerdir;
    int demobar;
    int extautomap;
    int extsaveg;
    int flipcorpses;
    int freeaim;
    int freelook;
    int hires;
    int jump;
    int leveltime;
    int mouselook;
    int neghealth;
    int overunder;
    int pitch;
    int playercoords;
    int recoil;
    int secretmessage;
    int smoothlight;
    int smoothscaling;
    int soundfix;
    int soundfull;
    int soundmono;
    int translucency;
#if CRISPY_TRUECOLOR
    int truecolor;
#endif
    int uncapped;
    int vsync;
    int weaponsquat;
    int widescreen;

    // [crispy] in-game switches and variables
    int screenshotmsg;
    int cleanscreenshot;
    int demowarp;
    int fps;

    bool flashinghom;
    bool fliplevels;
    bool flipweapons;
    bool haved1e5;
    bool havee1m10;
    bool havemap33;
    bool havessg;
    bool singleplayer;
    bool stretchsky;

    const char *sdlversion;
    const char *platform;

    void (*post_rendering_hook)();
} crispy_t;

extern crispy_t *const crispy;
extern const crispy_t *critical;

extern void CheckCrispySingleplayer(bool singleplayer);

enum
{
    REINIT_FRAMEBUFFERS = 1,
    REINIT_RENDERER     = 2,
    REINIT_TEXTURES     = 4,
    REINIT_ASPECTRATIO  = 8,
};

enum
{
    BOBFACTOR_FULL,
    BOBFACTOR_75,
    BOBFACTOR_OFF,
    NUM_BOBFACTORS,
};

enum
{
    BRIGHTMAPS_OFF,
    BRIGHTMAPS_TEXTURES,
    BRIGHTMAPS_SPRITES,
    BRIGHTMAPS_BOTH,
    NUM_BRIGHTMAPS,
};

enum
{
    CENTERWEAPON_OFF,
    CENTERWEAPON_CENTER,
    CENTERWEAPON_BOB,
    NUM_CENTERWEAPON,
};

enum
{
    COLOREDHUD_OFF,
    COLOREDHUD_BAR,
    COLOREDHUD_TEXT,
    COLOREDHUD_BOTH,
    NUM_COLOREDHUD
};

enum
{
    CROSSHAIR_OFF,
    CROSSHAIR_STATIC,
    CROSSHAIR_PROJECTED,
    NUM_CROSSHAIRS,
    CROSSHAIR_INTERCEPT = 0x10
};

enum
{
    DEMOTIMER_OFF,
    DEMOTIMER_RECORD,
    DEMOTIMER_PLAYBACK,
    DEMOTIMER_BOTH,
    NUM_DEMOTIMERS
};

enum
{
    FREEAIM_AUTO,
    FREEAIM_DIRECT,
    FREEAIM_BOTH,
    NUM_FREEAIMS
};

enum
{
    FREELOOK_OFF,
    FREELOOK_SPRING,
    FREELOOK_LOCK,
    NUM_FREELOOKS
};

enum
{
    JUMP_OFF,
    JUMP_LOW,
    JUMP_HIGH,
    NUM_JUMPS
};

enum
{
    TRANSLUCENCY_OFF,
    TRANSLUCENCY_MISSILE,
    TRANSLUCENCY_ITEM,
    TRANSLUCENCY_BOTH,
    NUM_TRANSLUCENCY
};

enum
{
    SECRETMESSAGE_OFF,
    SECRETMESSAGE_ON,
    SECRETMESSAGE_COUNT,
    NUM_SECRETMESSAGE
};

enum
{
    WIDESCREEN_OFF,
    WIDESCREEN_WIDE,
    WIDESCREEN_COMPACT,
    NUM_WIDESCREEN
};

enum
{
    WIDGETS_OFF,
    WIDGETS_AUTOMAP,
    WIDGETS_ALWAYS,
    NUM_WIDGETS
};

#endif
