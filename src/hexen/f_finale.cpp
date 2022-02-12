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


// HEADER FILES ------------------------------------------------------------

#include "h2def.hpp"
#include "i_system.hpp"
#include "i_video.hpp"
#include "p_local.hpp"
#include "s_sound.hpp"
#include <cctype>
#include "v_video.hpp"
#include "i_swap.hpp"
#include "lump.hpp"
#include "memory.hpp"

// MACROS ------------------------------------------------------------------

#define	TEXTSPEED	3
#define	TEXTWAIT	250

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void TextWrite();
static void DrawPic();
static void InitializeFade(bool fadeIn);
static void DeInitializeFade();
static void FadePic();
static char *GetFinaleText(int sequence);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern bool automapactive;
extern bool viewactive;

// PUBLIC DATA DECLARATIONS ------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int FinaleStage;
static int FinaleCount;
static int FinaleEndCount;
static int FinaleLumpNum;
static int FontABaseLump;
static char *FinaleText;

static fixed_t *Palette;
static fixed_t *PaletteDelta;
uint8_t static *RealPalette;

// CODE --------------------------------------------------------------------

//===========================================================================
//
// F_StartFinale
//
//===========================================================================

void F_StartFinale()
{
    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;
    P_ClearMessage(&players[consoleplayer]);

    FinaleStage = 0;
    FinaleCount = 0;
    FinaleText = GetFinaleText(0);
    FinaleEndCount = 70;
    FinaleLumpNum = W_GetNumForName("FINALE1");
    FontABaseLump = W_GetNumForName("FONTA_S") + 1;
    InitializeFade(1);

//      S_ChangeMusic(mus_victor, true);
    S_StartSongName("hall", false);     // don't loop the song
}

//===========================================================================
//
// F_Responder
//
//===========================================================================

bool F_Responder(event_t *)
{
    return false;
}

//===========================================================================
//
// F_Ticker
//
//===========================================================================

void F_Ticker()
{
    FinaleCount++;
    if (FinaleStage < 5 && FinaleCount >= FinaleEndCount)
    {
        FinaleCount = 0;
        FinaleStage++;
        switch (FinaleStage)
        {
            case 1:            // Text 1
                FinaleEndCount = static_cast<int>(strlen(FinaleText) * TEXTSPEED + TEXTWAIT);
                break;
            case 2:            // Pic 2, Text 2
                FinaleText = GetFinaleText(1);
                FinaleEndCount = static_cast<int>(strlen(FinaleText) * TEXTSPEED + TEXTWAIT);
                FinaleLumpNum = W_GetNumForName("FINALE2");
                S_StartSongName("orb", false);
                break;
            case 3:            // Pic 2 -- Fade out
                FinaleEndCount = 70;
                DeInitializeFade();
                InitializeFade(0);
                break;
            case 4:            // Pic 3 -- Fade in
                FinaleLumpNum = W_GetNumForName("FINALE3");
                FinaleEndCount = 71;
                DeInitializeFade();
                InitializeFade(1);
                S_StartSongName("chess", true);
                break;
            case 5:            // Pic 3 , Text 3
                FinaleText = GetFinaleText(2);
                DeInitializeFade();
                break;
            default:
                break;
        }
        return;
    }
    if (FinaleStage == 0 || FinaleStage == 3 || FinaleStage == 4)
    {
        FadePic();
    }
}

//===========================================================================
//
// TextWrite
//
//===========================================================================

static void TextWrite()
{
    int count;
    char *ch;
    int c;
    int cx, cy;
    patch_t *w;

    V_CopyScaledBuffer(I_VideoBuffer, cache_lump_num<pixel_t *>(FinaleLumpNum, PU_CACHE),
           ORIGWIDTH * ORIGHEIGHT);
    if (FinaleStage == 5)
    {                           // Chess pic, draw the correct character graphic
        if (netgame)
        {
            V_DrawPatch(20, 0, cache_lump_name<patch_t *>("chessall", PU_CACHE));
        }
        else if (PlayerClass[consoleplayer])
        {
            V_DrawPatch(60, 0, cache_lump_num<patch_t *>(W_GetNumForName("chessc") + PlayerClass[consoleplayer] - 1, PU_CACHE));
        }
    }
    // Draw the actual text
    if (FinaleStage == 5)
    {
        cy = 135;
    }
    else
    {
        cy = 5;
    }
    cx = 20;
    ch = FinaleText;
    count = (FinaleCount - 10) / TEXTSPEED;
    if (count < 0)
    {
        count = 0;
    }
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
        {
            break;
        }
        if (c == '\n')
        {
            cx = 20;
            cy += 9;
            continue;
        }
        if (c < 32)
        {
            continue;
        }
        c = toupper(c);
        if (c == 32)
        {
            cx += 5;
            continue;
        }
        w = cache_lump_num<patch_t *>(FontABaseLump + c - 33, PU_CACHE);
        if (cx + SHORT(w->width) > SCREENWIDTH)
        {
            break;
        }
        V_DrawPatch(cx, cy, w);
        cx += SHORT(w->width);
    }
}

//===========================================================================
//
// InitializeFade
//
//===========================================================================

static void InitializeFade(bool fadeIn)
{
    unsigned i;

    Palette = zmalloc<fixed_t *>(768 * sizeof(fixed_t), PU_STATIC, 0);
    PaletteDelta = zmalloc<fixed_t *>(768 * sizeof(fixed_t), PU_STATIC, 0);
    RealPalette = zmalloc<uint8_t *>(768 * sizeof(uint8_t), PU_STATIC, 0);

    if (fadeIn)
    {
        memset(RealPalette, 0, 768 * sizeof(uint8_t));
        for (i = 0; i < 768; i++)
        {
            Palette[i] = 0;
            PaletteDelta[i] = FixedDiv((*(cache_lump_name<uint8_t *>("playpal",
                                                                   PU_CACHE) +
                                          i)) << FRACBITS, 70 * FRACUNIT);
        }
    }
    else
    {
        for (i = 0; i < 768; i++)
        {
            RealPalette[i] =
                *(cache_lump_name<uint8_t *>("playpal", PU_CACHE) + i);
            Palette[i] = RealPalette[i] << FRACBITS;
            PaletteDelta[i] = FixedDiv(Palette[i], -70 * FRACUNIT);
        }
    }
    I_SetPalette(RealPalette);
}

//===========================================================================
//
// DeInitializeFade
//
//===========================================================================

static void DeInitializeFade()
{
    Z_Free(Palette);
    Z_Free(PaletteDelta);
    Z_Free(RealPalette);
}

//===========================================================================
//
// FadePic
//
//===========================================================================

static void FadePic()
{
    unsigned i;

    for (i = 0; i < 768; i++)
    {
        Palette[i] += PaletteDelta[i];
        RealPalette[i] = static_cast<uint8_t>(Palette[i] >> FRACBITS);
    }
    I_SetPalette(RealPalette);
}

//===========================================================================
//
// DrawPic
//
//===========================================================================

static void DrawPic()
{
    V_CopyScaledBuffer(I_VideoBuffer, cache_lump_num<pixel_t *>(FinaleLumpNum, PU_CACHE),
           ORIGWIDTH * ORIGHEIGHT);
    if (FinaleStage == 4 || FinaleStage == 5)
    {                           // Chess pic, draw the correct character graphic
        if (netgame)
        {
            V_DrawPatch(20, 0, cache_lump_name<patch_t *>("chessall", PU_CACHE));
        }
        else if (PlayerClass[consoleplayer])
        {
            V_DrawPatch(60, 0, cache_lump_num<patch_t *>(W_GetNumForName("chessc") + PlayerClass[consoleplayer] - 1, PU_CACHE));
        }
    }
}

//===========================================================================
//
// F_Drawer
//
//===========================================================================

void F_Drawer()
{
    switch (FinaleStage)
    {
        case 0:                // Fade in initial finale screen
            DrawPic();
            break;
        case 1:
        case 2:
            TextWrite();
            break;
        case 3:                // Fade screen out
            DrawPic();
            break;
        case 4:                // Fade in chess screen
            DrawPic();
            break;
        case 5:
            TextWrite();
            break;
    }
    UpdateState |= I_FULLSCRN;
}

//==========================================================================
//
// GetFinaleText
//
//==========================================================================

static char *GetFinaleText(int sequence)
{
    const char *msgLumpName;
    int msgSize;
    int msgLump;
    static const char *winMsgLumpNames[] = {
        "win1msg",
        "win2msg",
        "win3msg"
    };

    msgLumpName = winMsgLumpNames[sequence];
    msgLump = W_GetNumForName(msgLumpName);
    msgSize = static_cast<int>(W_LumpLength(msgLump));
    if (msgSize >= MAX_INTRMSN_MESSAGE_SIZE)
    {
        I_Error("Finale message too long (%s)", msgLumpName);
    }
    W_ReadLump(msgLump, ClusterMessage);
    ClusterMessage[msgSize] = 0;        // Append terminator
    return ClusterMessage;
}
