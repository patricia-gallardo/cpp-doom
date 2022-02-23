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
//	Game completion, final screen animation.
//

#include <array>
#include <cctype>

// Functions.
#include "deh_main.hpp"
#include "d_event.hpp"
#include "i_swap.hpp"
#include "z_zone.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"
#include "s_sound.hpp"

// Data.
#include "d_main.hpp"
#include "dstrings.hpp"
#include "sounds.hpp"

#include "doomstat.hpp"
#include "r_state.hpp"
#include "m_controls.hpp" // [crispy] key_*
#include "m_misc.hpp"     // [crispy] M_StringDuplicate()
#include "m_random.hpp"   // [crispy] Crispy_Random()
#include "event_function_decls.hpp"

enum finalestage_t
{
    F_STAGE_TEXT,
    F_STAGE_ARTSCREEN,
    F_STAGE_CAST,
};

// ?
//#include "doomstat.hpp"
//#include "r_local.hpp"
//#include "f_finale.hpp"

// Stage of animation:
finalestage_t finalestage;

unsigned int finalecount;

constexpr auto TEXTSPEED =3;
constexpr auto TEXTWAIT  =250;

typedef struct
{
    GameMission_t mission;
    int           episode, level;
    const char *  background;
    const char *  text;
} textscreen_t;

static textscreen_t textscreens[] = {
    { doom, 1, 8, "FLOOR4_8", E1TEXT },
    { doom, 2, 8, "SFLR6_1", E2TEXT },
    { doom, 3, 8, "MFLR8_4", E3TEXT },
    { doom, 4, 8, "MFLR8_3", E4TEXT },
    { doom, 5, 8, "FLOOR7_2", E5TEXT }, // [crispy] Sigil

    { doom2, 1, 6, "SLIME16", C1TEXT },
    { doom2, 1, 11, "RROCK14", C2TEXT },
    { doom2, 1, 20, "RROCK07", C3TEXT },
    { doom2, 1, 30, "RROCK17", C4TEXT },
    { doom2, 1, 15, "RROCK13", C5TEXT },
    { doom2, 1, 31, "RROCK19", C6TEXT },

    { pack_tnt, 1, 6, "SLIME16", T1TEXT },
    { pack_tnt, 1, 11, "RROCK14", T2TEXT },
    { pack_tnt, 1, 20, "RROCK07", T3TEXT },
    { pack_tnt, 1, 30, "RROCK17", T4TEXT },
    { pack_tnt, 1, 15, "RROCK13", T5TEXT },
    { pack_tnt, 1, 31, "RROCK19", T6TEXT },

    { pack_plut, 1, 6, "SLIME16", P1TEXT },
    { pack_plut, 1, 11, "RROCK14", P2TEXT },
    { pack_plut, 1, 20, "RROCK07", P3TEXT },
    { pack_plut, 1, 30, "RROCK17", P4TEXT },
    { pack_plut, 1, 15, "RROCK13", P5TEXT },
    { pack_plut, 1, 31, "RROCK19", P6TEXT },

    { pack_nerve, 1, 8, "SLIME16", N1TEXT },
    { pack_master, 1, 20, "SLIME16", M1TEXT },
};

const char * finaletext;
const char * finaleflat;
static char *finaletext_rw;

void    F_StartCast();
void    F_CastTicker();
bool F_CastResponder(event_t *ev);
void    F_CastDrawer();


//
// F_StartFinale
//
void F_StartFinale()
{
    gameaction    = ga_nothing;
    g_doomstat_globals->gamestate     = GS_FINALE;
    g_doomstat_globals->viewactive    = false;
    g_doomstat_globals->automapactive = false;

    if (logical_gamemission == doom)
    {
        S_ChangeMusic(mus_victor, true);
    }
    else
    {
        S_ChangeMusic(mus_read_m, true);
    }

    // Find the right screen and set the text and background

    for (auto & textscreen : textscreens)
    {
        textscreen_t *screen = &textscreen;

        // Hack for Chex Quest

        if (g_doomstat_globals->gameversion == exe_chex && screen->mission == doom)
        {
            screen->level = 5;
        }

        // [crispy] Hack for Master Levels MAP21: Bad Dream
        if (g_doomstat_globals->gamemission == pack_master && screen->mission == pack_master && g_doomstat_globals->gamemap == 21)
        {
            screen->level = 21;
        }

        // [crispy] During demo recording/playback or network games
        // these two packs behave like any other ordinary PWAD
        if (!crispy->singleplayer && (g_doomstat_globals->gamemission == pack_nerve || g_doomstat_globals->gamemission == pack_master)
            && screen->mission == doom2)
        {
            screen->mission = g_doomstat_globals->gamemission;
        }

        if (logical_gamemission == screen->mission
            && (logical_gamemission != doom || g_doomstat_globals->gameepisode == screen->episode)
            && g_doomstat_globals->gamemap == screen->level)
        {
            finaletext = screen->text;
            finaleflat = screen->background;
        }
    }

    // Do dehacked substitutions of strings

    finaletext = DEH_String(finaletext);
    finaleflat = DEH_String(finaleflat);
    // [crispy] do the "char* vs. const char*" dance
    if (finaletext_rw)
    {
        free(finaletext_rw);
        finaletext_rw = nullptr;
    }
    finaletext_rw = M_StringDuplicate(finaletext);

    finalestage = F_STAGE_TEXT;
    finalecount = 0;
}


bool F_Responder(event_t *event)
{
    if (finalestage == F_STAGE_CAST)
        return F_CastResponder(event);

    return false;
}


//
// F_Ticker
//
void F_Ticker()
{
    size_t i = 0;

    // check for skipping
    if ((g_doomstat_globals->gamemode == commercial)
        && (finalecount > 50))
    {
        // go on to the next level
        for (i = 0; i < MAXPLAYERS; i++)
            if (g_doomstat_globals->players[i].cmd.buttons)
                break;

        if (i < MAXPLAYERS)
        {
            if (g_doomstat_globals->gamemission == pack_nerve && crispy->singleplayer && g_doomstat_globals->gamemap == 8)
                F_StartCast();
            else if (g_doomstat_globals->gamemission == pack_master && crispy->singleplayer && (g_doomstat_globals->gamemap == 20 || g_doomstat_globals->gamemap == 21))
                F_StartCast();
            else if (g_doomstat_globals->gamemap == 30)
                F_StartCast();
            else
                gameaction = ga_worlddone;
        }
    }

    // advance animation
    finalecount++;

    if (finalestage == F_STAGE_CAST)
    {
        F_CastTicker();
        return;
    }

    if (g_doomstat_globals->gamemode == commercial)
        return;

    if (finalestage == F_STAGE_TEXT
        && finalecount > strlen(finaletext) * TEXTSPEED + TEXTWAIT)
    {
        finalecount   = 0;
        finalestage   = F_STAGE_ARTSCREEN;
        g_doomstat_globals->wipegamestate = gamestate_t::GS_FORCE_WIPE; // force a wipe
        if (g_doomstat_globals->gameepisode == 3)
            S_StartMusic(mus_bunny);
    }
}


//
// F_TextWrite
//

#include "lump.hpp"
#include "hu_stuff.hpp"
extern patch_t *hu_font[HU_FONTSIZE];

// [crispy] add line breaks for lines exceeding screenwidth
static inline bool F_AddLineBreak(char *c)
{
    while (c-- > finaletext_rw)
    {
        if (*c == '\n')
        {
            return false;
        }
        else if (*c == ' ')
        {
            *c = '\n';
            return true;
        }
    }

    return false;
}

void F_TextWrite()
{
    int        x, y, w;
    signed int count;
    char *     ch; // [crispy] un-const
    int        c;
    int        cx;
    int        cy;

    // erase the entire screen to a tiled background
    auto *src  = cache_lump_name<uint8_t *>(finaleflat, PU_CACHE);
    auto *dest = g_i_video_globals->I_VideoBuffer;

    for (y = 0; y < SCREENHEIGHT; y++)
    {
#ifndef CRISPY_TRUECOLOR
        for (x = 0; x < SCREENWIDTH / 64; x++)
        {
            std::memcpy(dest, src + ((y & 63) << 6), 64);
            dest += 64;
        }
        if (SCREENWIDTH & 63)
        {
            std::memcpy(dest, src + ((y & 63) << 6), SCREENWIDTH & 63);
            dest += (SCREENWIDTH & 63);
        }
#else
        for (x = 0; x < SCREENWIDTH; x++)
        {
            *dest++ = colormaps[src[((y & 63) << 6) + (x & 63)]];
        }
#endif
    }

    V_MarkRect(0, 0, SCREENWIDTH, SCREENHEIGHT);

    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext_rw;

    count = (static_cast<signed int>(finalecount) - 10) / TEXTSPEED;
    if (count < 0)
        count = 0;
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = 10;
            cy += 11;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c > HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        if (cx + w > ORIGWIDTH)
        {
            // [crispy] add line breaks for lines exceeding screenwidth
            if (F_AddLineBreak(ch))
            {
                continue;
            }
            else
                break;
        }
        // [cispy] prevent text from being drawn off-screen vertically
        if (cy + SHORT(hu_font[c]->height) > ORIGHEIGHT)
        {
            break;
        }
        V_DrawPatch(cx, cy, hu_font[c]);
        cx += w;
    }
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
    const char *name;
    mobjtype_t  type;
} castinfo_t;

castinfo_t castorder[] = {
    { CC_ZOMBIE, MT_POSSESSED },
    { CC_SHOTGUN, MT_SHOTGUY },
    { CC_HEAVY, MT_CHAINGUY },
    { CC_IMP, MT_TROOP },
    { CC_DEMON, MT_SERGEANT },
    { CC_LOST, MT_SKULL },
    { CC_CACO, MT_HEAD },
    { CC_HELL, MT_KNIGHT },
    { CC_BARON, MT_BRUISER },
    { CC_ARACH, MT_BABY },
    { CC_PAIN, MT_PAIN },
    { CC_REVEN, MT_UNDEAD },
    { CC_MANCU, MT_FATSO },
    { CC_ARCH, MT_VILE },
    { CC_SPIDER, MT_SPIDER },
    { CC_CYBER, MT_CYBORG },
    { CC_HERO, MT_PLAYER },

    { nullptr, {} }
};

int                castnum;
int                casttics;
state_t *          caststate;
bool            castdeath;
int                castframes;
int                castonmelee;
bool            castattacking;
static signed char castangle; // [crispy] turnable cast
static signed char castskip;  // [crispy] skippable cast
static bool     castflip;  // [crispy] flippable death sequence

// [crispy] randomize seestate and deathstate sounds in the cast
static int F_RandomizeSound(int sound)
{
    if (!crispy->soundfix)
        return sound;

    switch (sound)
    {
    // [crispy] actor->info->seesound, from p_enemy.c:A_Look()
    case sfx_posit1:
    case sfx_posit2:
    case sfx_posit3:
        return sfx_posit1 + Crispy_Random() % 3;
        break;

    case sfx_bgsit1:
    case sfx_bgsit2:
        return sfx_bgsit1 + Crispy_Random() % 2;
        break;

    // [crispy] actor->info->deathsound, from p_enemy.c:A_Scream()
    case sfx_podth1:
    case sfx_podth2:
    case sfx_podth3:
        return sfx_podth1 + Crispy_Random() % 3;
        break;

    case sfx_bgdth1:
    case sfx_bgdth2:
        return sfx_bgdth1 + Crispy_Random() % 2;
        break;

    default:
        return sound;
        break;
    }
}

typedef struct
{
    actionf_t     action;
    const int     sound;
    const bool early;
} actionsound_t;

static const actionsound_t actionsounds[] = {
    { A_PosAttack, sfx_pistol, false },
    { A_SPosAttack, sfx_shotgn, false },
    { A_CPosAttack, sfx_shotgn, false },
    { A_CPosRefire, sfx_shotgn, false },
    { A_VileTarget, sfx_vilatk, true },
    { A_SkelWhoosh, sfx_skeswg, false },
    { A_SkelFist, sfx_skepch, false },
    { A_SkelMissile, sfx_skeatk, true },
    { A_FatAttack1, sfx_firsht, false },
    { A_FatAttack2, sfx_firsht, false },
    { A_FatAttack3, sfx_firsht, false },
    { A_HeadAttack, sfx_firsht, true },
    { A_BruisAttack, sfx_firsht, true },
    { A_TroopAttack, sfx_claw, false },
    { A_SargAttack, sfx_sgtatk, true },
    { A_SkullAttack, sfx_sklatk, false },
    { A_PainAttack, sfx_sklatk, true },
    { A_BspiAttack, sfx_plasma, false },
    { A_CyberAttack, sfx_rlaunc, false },
};

// [crispy] play attack sound based on state action function (instead of state number)
static int F_SoundForState(int st)
{
    return std::visit(overloaded {
                          [st](const null_hook &) { return (st == S_PLAY_ATK2) ? sfx_dshtgn : 0; },
                          [](const valid_hook &cast_action) {
                              const auto &next = states[caststate->nextstate].action;
                              for (const auto &action_sound : actionsounds)
                              {
                                  if (action_sound.action.index() != valid_hook_action_hook)
                                      continue;
                                  const auto &soundaction = std::get<valid_hook>(action_sound.action);

                                  if (!action_sound.early && is_valid(cast_action) == is_valid(soundaction))
                                  {
                                      return action_sound.sound;
                                  }

                                  if (next.index() != valid_hook_action_hook)
                                      continue;
                                  const auto &next_action = std::get<valid_hook>(next);

                                  if (action_sound.early && is_valid(next_action) == is_valid(soundaction))
                                  {
                                      return action_sound.sound;
                                  }
                              }
                              return 0;
                          },
                          [](const auto &) { return 0; },
                      },
        caststate->action);
}

//
// F_StartCast
//
void F_StartCast()
{
    g_doomstat_globals->wipegamestate = gamestate_t::GS_FORCE_WIPE; // force a screen wipe
    castnum       = 0;
    caststate     = &states[mobjinfo[castorder[castnum].type].seestate];
    casttics      = caststate->tics;
    castdeath     = false;
    finalestage   = F_STAGE_CAST;
    castframes    = 0;
    castonmelee   = 0;
    castattacking = false;
    S_ChangeMusic(mus_evil, true);
}


//
// F_CastTicker
//
void F_CastTicker()
{
    int st;
    int sfx;

    if (--casttics > 0)
        return; // not time to change state yet

    if (caststate->tics == -1 || caststate->nextstate == S_NULL || castskip) // [crispy] skippable cast
    {
        if (castskip)
        {
            castnum += castskip;
            castskip = 0;
        }
        else
            // switch from deathstate to next monster
            castnum++;
        castdeath = false;
        if (castorder[castnum].name == nullptr)
            castnum = 0;
        if (mobjinfo[castorder[castnum].type].seesound)
            S_StartSound(nullptr, F_RandomizeSound(mobjinfo[castorder[castnum].type].seesound));
        caststate  = &states[mobjinfo[castorder[castnum].type].seestate];
        castframes = 0;
        castangle  = 0;     // [crispy] turnable cast
        castflip   = false; // [crispy] flippable death sequence
    }
    else
    {
        // just advance to next state in animation
        // [crispy] fix Doomguy in casting sequence
        /*
	if (!castdeath && caststate == &states[S_PLAY_ATK1])
	    goto stopattack;	// Oh, gross hack!
	*/
        // [crispy] Allow A_RandomJump() in deaths in cast sequence
        action_hook needle = A_RandomJump;
        if (caststate->action == needle && Crispy_Random() < caststate->misc2)
        {
            st = caststate->misc1;
        }
        else
        {
            // [crispy] fix Doomguy in casting sequence
            if (!castdeath && caststate == &states[S_PLAY_ATK1])
                st = S_PLAY_ATK2;
            else if (!castdeath && caststate == &states[S_PLAY_ATK2])
                goto stopattack; // Oh, gross hack!
            else
                st = caststate->nextstate;
        }
        caststate = &states[st];
        castframes++;

        sfx = F_SoundForState(st);
        /*
	// sound hacks....
	switch (st)
	{
	  case S_PLAY_ATK2:	sfx = sfx_dshtgn; break; // [crispy] fix Doomguy in casting sequence
	  case S_POSS_ATK2:	sfx = sfx_pistol; break;
	  case S_SPOS_ATK2:	sfx = sfx_shotgn; break;
	  case S_VILE_ATK2:	sfx = sfx_vilatk; break;
	  case S_SKEL_FIST2:	sfx = sfx_skeswg; break;
	  case S_SKEL_FIST4:	sfx = sfx_skepch; break;
	  case S_SKEL_MISS2:	sfx = sfx_skeatk; break;
	  case S_FATT_ATK8:
	  case S_FATT_ATK5:
	  case S_FATT_ATK2:	sfx = sfx_firsht; break;
	  case S_CPOS_ATK2:
	  case S_CPOS_ATK3:
	  case S_CPOS_ATK4:	sfx = sfx_shotgn; break;
	  case S_TROO_ATK3:	sfx = sfx_claw; break;
	  case S_SARG_ATK2:	sfx = sfx_sgtatk; break;
	  case S_BOSS_ATK2:
	  case S_BOS2_ATK2:
	  case S_HEAD_ATK2:	sfx = sfx_firsht; break;
	  case S_SKULL_ATK2:	sfx = sfx_sklatk; break;
	  case S_SPID_ATK2:
	  case S_SPID_ATK3:	sfx = sfx_shotgn; break;
	  case S_BSPI_ATK2:	sfx = sfx_plasma; break;
	  case S_CYBER_ATK2:
	  case S_CYBER_ATK4:
	  case S_CYBER_ATK6:	sfx = sfx_rlaunc; break;
	  case S_PAIN_ATK3:	sfx = sfx_sklatk; break;
	  default: sfx = 0; break;
	}
		
*/
        if (sfx)
            S_StartSound(nullptr, sfx);
    }

    if (!castdeath && castframes == 12)
    {
        // go into attack frame
        castattacking = true;
        if (castonmelee)
            caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
        else
            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        castonmelee ^= 1;
        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
                caststate =
                    &states[mobjinfo[castorder[castnum].type].meleestate];
            else
                caststate =
                    &states[mobjinfo[castorder[castnum].type].missilestate];
        }
    }

    if (castattacking)
    {
        if (castframes == 24
            || caststate == &states[mobjinfo[castorder[castnum].type].seestate])
        {
        stopattack:
            castattacking = false;
            castframes    = 0;
            caststate     = &states[mobjinfo[castorder[castnum].type].seestate];
        }
    }

    casttics = caststate->tics;
    if (casttics == -1)
    {
        // [crispy] Allow A_RandomJump() in deaths in cast sequence
        action_hook needle = A_RandomJump;
        if (caststate->action == needle)
        {
            if (Crispy_Random() < caststate->misc2)
            {
                caststate = &states[caststate->misc1];
            }
            else
            {
                caststate = &states[caststate->nextstate];
            }

            casttics = caststate->tics;
        }

        if (casttics == -1)
        {
            casttics = 15;
        }
    }
}


//
// F_CastResponder
//

bool F_CastResponder(event_t *ev)
{
    bool xdeath = false;

    if (ev->type != ev_keydown)
        return false;

    // [crispy] make monsters turnable in cast ...
    if (ev->data1 == g_m_controls_globals->key_left)
    {
        if (++castangle > 7)
            castangle = 0;
        return false;
    }
    else if (ev->data1 == g_m_controls_globals->key_right)
    {
        if (--castangle < 0)
            castangle = 7;
        return false;
    }
    else
        // [crispy] ... and allow to skip through them ..
        if (ev->data1 == g_m_controls_globals->key_strafeleft || ev->data1 == g_m_controls_globals->key_alt_strafeleft)
    {
        castskip = static_cast<signed char>(castnum ? -1 : std::size(castorder) - 2);
        return false;
    }
    else if (ev->data1 == g_m_controls_globals->key_straferight || ev->data1 == g_m_controls_globals->key_alt_straferight)
    {
        castskip = +1;
        return false;
    }
    // [crispy] ... and finally turn them into gibbs
    if (ev->data1 == g_m_controls_globals->key_speed)
        xdeath = true;

    if (castdeath)
        return true; // already in dying frames

    // go into death frame
    castdeath = true;
    if (xdeath && mobjinfo[castorder[castnum].type].xdeathstate)
        caststate = &states[mobjinfo[castorder[castnum].type].xdeathstate];
    else
        caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
    casttics = caststate->tics;
    // [crispy] Allow A_RandomJump() in deaths in cast sequence
    action_hook needle = A_RandomJump;
    if (casttics == -1 && caststate->action == needle)
    {
        if (Crispy_Random() < caststate->misc2)
        {
            caststate = &states[caststate->misc1];
        }
        else
        {
            caststate = &states[caststate->nextstate];
        }
        casttics = caststate->tics;
    }
    castframes    = 0;
    castattacking = false;
    if (xdeath && mobjinfo[castorder[castnum].type].xdeathstate)
        S_StartSound(nullptr, sfx_slop);
    else if (mobjinfo[castorder[castnum].type].deathsound)
        S_StartSound(nullptr, F_RandomizeSound(mobjinfo[castorder[castnum].type].deathsound));

    // [crispy] flippable death sequence
    castflip = crispy->flipcorpses && castdeath && (mobjinfo[castorder[castnum].type].flags & MF_FLIPPABLE) && (Crispy_Random() & 1);

    return true;
}


void F_CastPrint(const char *text)
{
    const char *ch;
    int         c;
    int         cx;
    int         w;
    int         width;

    // find width
    ch    = text;
    width = 0;

    while (ch)
    {
        c = *ch++;
        if (!c)
            break;
        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c > HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        width += w;
    }

    // draw it
    cx = ORIGWIDTH / 2 - width / 2;
    ch = text;
    while (ch)
    {
        c = *ch++;
        if (!c)
            break;
        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c > HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        V_DrawPatch(cx, 180, hu_font[c]);
        cx += w;
    }
}


//
// F_CastDrawer
//

void F_CastDrawer()
{
    spritedef_t *  sprdef;
    spriteframe_t *sprframe;
    int            lump;
    bool        flip;
    patch_t *      patch;

    // erase the entire screen to a background
    V_DrawPatchFullScreen(cache_lump_name<patch_t *>(DEH_String("BOSSBACK"), PU_CACHE), false);

    F_CastPrint(DEH_String(castorder[castnum].name));

    // draw the current frame in the middle of the screen
    sprdef = &g_r_state_globals->sprites[caststate->sprite];
    // [crispy] the TNT1 sprite is not supposed to be rendered anyway
    if (!sprdef->numframes && caststate->sprite == SPR_TNT1)
    {
        return;
    }
    sprframe = &sprdef->spriteframes[caststate->frame & FF_FRAMEMASK];
    lump     = sprframe->lump[castangle];                     // [crispy] turnable cast
    flip     = static_cast<bool>(sprframe->flip[castangle] ^ static_cast<uint8_t>(castflip)); // [crispy] turnable cast, flippable death sequence

    patch = cache_lump_num<patch_t *>(lump + g_r_state_globals->firstspritelump, PU_CACHE);
    if (flip)
        V_DrawPatchFlipped(ORIGWIDTH / 2, 170, patch);
    else
        V_DrawPatch(ORIGWIDTH / 2, 170, patch);
}


//
// F_DrawPatchCol
//
static fixed_t dxi, dy, dyi;

void F_DrawPatchCol(int x,
    patch_t *           patch,
    int                 col)
{
    column_t *column;
    uint8_t  *source;
    pixel_t * dest;
    pixel_t * desttop;
    int       count;

    column  = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + LONG(patch->columnofs[col >> FRACBITS]));
    desttop = g_i_video_globals->I_VideoBuffer + x + (DELTAWIDTH << crispy->hires);

    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
        int srccol = 0;
        source     = reinterpret_cast<uint8_t *>(column) + 3;
        dest       = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
        count      = (column->length * dy) >> FRACBITS;

        while (count--)
        {
            *dest = source[srccol >> FRACBITS];
            srccol += dyi;
            dest += SCREENWIDTH;
        }
        column = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(column) + column->length + 4);
    }
}


//
// F_BunnyScroll
//
void F_BunnyScroll()
{
    signed int scrolled;
    int        x;
    char       name[10];
    int        stage;
    static int laststage;

    dxi = (ORIGWIDTH << FRACBITS) / HIRESWIDTH;
    dy  = (SCREENHEIGHT << FRACBITS) / ORIGHEIGHT;
    dyi = (ORIGHEIGHT << FRACBITS) / SCREENHEIGHT;

    // [crispy] fill pillarboxes in widescreen mode
    if (SCREENWIDTH != HIRESWIDTH)
    {
        V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
    }

    auto *p1 = cache_lump_name<patch_t *>(DEH_String("PFUB2"), PU_LEVEL);
    auto *p2 = cache_lump_name<patch_t *>(DEH_String("PFUB1"), PU_LEVEL);

    V_MarkRect(0, 0, SCREENWIDTH, SCREENHEIGHT);

    scrolled = (ORIGWIDTH - (static_cast<signed int>(finalecount) - 230) / 2);
    if (scrolled > ORIGWIDTH)
        scrolled = ORIGWIDTH;
    if (scrolled < 0)
        scrolled = 0;
    scrolled <<= FRACBITS;

    for (x = 0; x < ORIGWIDTH << FRACBITS; x += dxi)
    {
        if (x + scrolled < ORIGWIDTH << FRACBITS)
            F_DrawPatchCol(x / dxi, p1, x + scrolled);
        else
            F_DrawPatchCol(x / dxi, p2, x + scrolled - (ORIGWIDTH << FRACBITS));
    }

    if (finalecount < 1130)
        return;
    if (finalecount < 1180)
    {
        V_DrawPatch((ORIGWIDTH - 13 * 8) / 2,
            (ORIGHEIGHT - 8 * 8) / 2,
            cache_lump_name<patch_t *>(DEH_String("END0"), PU_CACHE));
        laststage = 0;
        return;
    }

    stage = (finalecount - 1180) / 5;
    if (stage > 6)
        stage = 6;
    if (stage > laststage)
    {
        S_StartSound(nullptr, sfx_pistol);
        laststage = stage;
    }

    DEH_snprintf(name, 10, "END%i", stage);
    V_DrawPatch((ORIGWIDTH - 13 * 8) / 2,
        (ORIGHEIGHT - 8 * 8) / 2,
        cache_lump_name<patch_t *>(name, PU_CACHE));
}

static void F_ArtScreenDrawer()
{
    const char *lumpname;

    if (g_doomstat_globals->gameepisode == 3)
    {
        F_BunnyScroll();
    }
    else
    {
        switch (g_doomstat_globals->gameepisode)
        {
        case 1:
            if (g_doomstat_globals->gameversion >= exe_ultimate)
            {
                lumpname = "CREDIT";
            }
            else
            {
                lumpname = "HELP2";
            }
            break;
        case 2:
            lumpname = "VICTORY2";
            break;
        case 4:
            lumpname = "ENDPIC";
            break;
        // [crispy] Sigil
        case 5:
            lumpname = "SIGILEND";
            if (W_CheckNumForName(DEH_String(lumpname)) == -1)
            {
                return;
            }
            break;
        default:
            return;
        }

        lumpname = DEH_String(lumpname);

        V_DrawPatchFullScreen(cache_lump_name<patch_t *>(lumpname, PU_CACHE), false);
    }
}

//
// F_Drawer
//
void F_Drawer()
{
    switch (finalestage)
    {
    case F_STAGE_CAST:
        F_CastDrawer();
        break;
    case F_STAGE_TEXT:
        F_TextWrite();
        break;
    case F_STAGE_ARTSCREEN:
        F_ArtScreenDrawer();
        break;
    }
}
