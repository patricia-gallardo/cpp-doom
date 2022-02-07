//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2018 Fabian Greffrath
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
//	[crispy] Crispness menu
//

#include "doomstat.hpp"
#include "p_local.hpp" // [crispy] thinkercap
#include "s_sound.hpp"
#include "r_defs.hpp" // [crispy] laserpatch
#include "r_sky.hpp"  // [crispy] R_InitSkyMap()

#include "m_crispy.hpp"

multiitem_t multiitem_bobfactor[NUM_BOBFACTORS] = {
    { BOBFACTOR_FULL, const_cast<char *>("full") },
    { BOBFACTOR_75, const_cast<char *>("75%") },
    { BOBFACTOR_OFF, const_cast<char *>("off") },
};

multiitem_t multiitem_brightmaps[NUM_BRIGHTMAPS] = {
    { BRIGHTMAPS_OFF, const_cast<char *>("none") },
    { BRIGHTMAPS_TEXTURES, const_cast<char *>("walls") },
    { BRIGHTMAPS_SPRITES, const_cast<char *>("items") },
    { BRIGHTMAPS_BOTH, const_cast<char *>("both") },
};

multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] = {
    { CENTERWEAPON_OFF, const_cast<char *>("off") },
    { CENTERWEAPON_CENTER, const_cast<char *>("centered") },
    { CENTERWEAPON_BOB, const_cast<char *>("bobbing") },
};

multiitem_t multiitem_coloredhud[NUM_COLOREDHUD] = {
    { COLOREDHUD_OFF, const_cast<char *>("off") },
    { COLOREDHUD_BAR, const_cast<char *>("status bar") },
    { COLOREDHUD_TEXT, const_cast<char *>("hud texts") },
    { COLOREDHUD_BOTH, const_cast<char *>("both") },
};

multiitem_t multiitem_crosshair[NUM_CROSSHAIRS] = {
    { CROSSHAIR_OFF, const_cast<char *>("off") },
    { CROSSHAIR_STATIC, const_cast<char *>("static") },
    { CROSSHAIR_PROJECTED, const_cast<char *>("projected") },
};

multiitem_t multiitem_crosshairtype[] = {
    { -1, const_cast<char *>("none") },
    { 0, const_cast<char *>("cross") },
    { 1, const_cast<char *>("chevron") },
    { 2, const_cast<char *>("dot") },
};

multiitem_t multiitem_freeaim[NUM_FREEAIMS] = {
    { FREEAIM_AUTO, const_cast<char *>("autoaim") },
    { FREEAIM_DIRECT, const_cast<char *>("direct") },
    { FREEAIM_BOTH, const_cast<char *>("both") },
};

multiitem_t multiitem_demotimer[NUM_DEMOTIMERS] = {
    { DEMOTIMER_OFF, const_cast<char *>("off") },
    { DEMOTIMER_RECORD, const_cast<char *>("recording") },
    { DEMOTIMER_PLAYBACK, const_cast<char *>("playback") },
    { DEMOTIMER_BOTH, const_cast<char *>("both") },
};

multiitem_t multiitem_demotimerdir[] = {
    { 0, const_cast<char *>("none") },
    { 1, const_cast<char *>("forward") },
    { 2, const_cast<char *>("backward") },
};

multiitem_t multiitem_freelook[NUM_FREELOOKS] = {
    { FREELOOK_OFF, const_cast<char *>("off") },
    { FREELOOK_SPRING, const_cast<char *>("spring") },
    { FREELOOK_LOCK, const_cast<char *>("lock") },
};

multiitem_t multiitem_jump[NUM_JUMPS] = {
    { JUMP_OFF, const_cast<char *>("off") },
    { JUMP_LOW, const_cast<char *>("low") },
    { JUMP_HIGH, const_cast<char *>("high") },
};

multiitem_t multiitem_secretmessage[NUM_SECRETMESSAGE] = {
    { SECRETMESSAGE_OFF, const_cast<char *>("off") },
    { SECRETMESSAGE_ON, const_cast<char *>("on") },
    { SECRETMESSAGE_COUNT, const_cast<char *>("count") },
};

multiitem_t multiitem_translucency[NUM_TRANSLUCENCY] = {
    { TRANSLUCENCY_OFF, const_cast<char *>("off") },
    { TRANSLUCENCY_MISSILE, const_cast<char *>("projectiles") },
    { TRANSLUCENCY_ITEM, const_cast<char *>("items") },
    { TRANSLUCENCY_BOTH, const_cast<char *>("both") },
};

multiitem_t multiitem_sndchannels[4] = {
    { 8, const_cast<char *>("8") },
    { 16, const_cast<char *>("16") },
    { 32, const_cast<char *>("32") },
};

multiitem_t multiitem_widescreen[NUM_WIDESCREEN] = {
    { WIDESCREEN_OFF, const_cast<char *>("off") },
    { WIDESCREEN_WIDE, const_cast<char *>("on, wide HUD") },
    { WIDESCREEN_COMPACT, const_cast<char *>("on, compact HUD") },
};

multiitem_t multiitem_widgets[NUM_WIDGETS] = {
    { WIDGETS_OFF, const_cast<char *>("never") },
    { WIDGETS_AUTOMAP, const_cast<char *>("in Automap") },
    { WIDGETS_ALWAYS, const_cast<char *>("always") },
};

extern void AM_ReInit();
extern void EnableLoadingDisk();
extern void P_SegLengths(bool contrast_only);
extern void R_ExecuteSetViewSize();
extern void R_InitLightTables();
extern void I_ReInitGraphics(int reinit);
extern void ST_createWidgets();
extern void HU_Start();
extern void M_SizeDisplay(int choice);


void M_CrispyToggleAutomapstats(int)
{
    crispy->automapstats = (crispy->automapstats + 1) % NUM_WIDGETS;
}

void M_CrispyToggleBobfactor(int )
{
    crispy->bobfactor = (crispy->bobfactor + 1) % NUM_BOBFACTORS;
}

void M_CrispyToggleBrightmaps(int)
{
    crispy->brightmaps = (crispy->brightmaps + 1) % NUM_BRIGHTMAPS;
}

void M_CrispyToggleCenterweapon(int)
{
    crispy->centerweapon = (crispy->centerweapon + 1) % NUM_CENTERWEAPON;
}

void M_CrispyToggleColoredblood(int)
{
    thinker_t *th;

    if (gameversion == exe_chex)
    {
        return;
    }

    crispy->coloredblood = !crispy->coloredblood;

    // [crispy] switch NOBLOOD flag for Lost Souls
    for (th = thinkercap.next; th && th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == reinterpret_cast<actionf_p1>(P_MobjThinker))
        {
            mobj_t *mobj = reinterpret_cast<mobj_t *>(th);

            if (mobj->type == MT_SKULL)
            {
                if (crispy->coloredblood)
                {
                    mobj->flags |= MF_NOBLOOD;
                }
                else
                {
                    mobj->flags &= ~MF_NOBLOOD;
                }
            }
        }
    }
}

void M_CrispyToggleColoredhud(int)
{
    crispy->coloredhud = (crispy->coloredhud + 1) % NUM_COLOREDHUD;
}

void M_CrispyToggleCrosshair(int)
{
    crispy->crosshair = (crispy->crosshair + 1) % NUM_CROSSHAIRS;
}

void M_CrispyToggleCrosshairHealth(int)
{
    crispy->crosshairhealth = !crispy->crosshairhealth;
}

void M_CrispyToggleCrosshairTarget(int)
{
    crispy->crosshairtarget = !crispy->crosshairtarget;
}

void M_CrispyToggleCrosshairtype(int)
{
    if (!crispy->crosshair)
    {
        return;
    }

    crispy->crosshairtype = crispy->crosshairtype + 1;

    if (!laserpatch[crispy->crosshairtype].c)
    {
        crispy->crosshairtype = 0;
    }
}

void M_CrispyToggleDemoBar(int)
{
    crispy->demobar = !crispy->demobar;
}

void M_CrispyToggleDemoTimer(int)
{
    crispy->demotimer = (crispy->demotimer + 1) % NUM_DEMOTIMERS;
}

void M_CrispyToggleDemoTimerDir(int)
{
    if (!(crispy->demotimer & DEMOTIMER_PLAYBACK))
    {
        return;
    }

    crispy->demotimerdir = !crispy->demotimerdir;
}

void M_CrispyToggleExtAutomap(int)
{
    crispy->extautomap = !crispy->extautomap;
}

void M_CrispyToggleExtsaveg(int)
{
    crispy->extsaveg = !crispy->extsaveg;
}

void M_CrispyToggleFlipcorpses(int)
{
    if (gameversion == exe_chex)
    {
        return;
    }

    crispy->flipcorpses = !crispy->flipcorpses;
}

void M_CrispyToggleFreeaim(int)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->freeaim = (crispy->freeaim + 1) % NUM_FREEAIMS;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

static void M_CrispyToggleSkyHook()
{
    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

void M_CrispyToggleFreelook(int)
{
    crispy->freelook = (crispy->freelook + 1) % NUM_FREELOOKS;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleFullsounds(int)
{
    crispy->soundfull = !crispy->soundfull;

    // [crispy] weapon sound sources
    for (int i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            players[i].so = Crispy_PlayerSO(i);
        }
    }
}

static void M_CrispyToggleHiresHook()
{
    crispy->hires = !crispy->hires;

    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] re-draw bezel
    R_FillBackScreen();
    // [crispy] re-calculate disk icon coordinates
    EnableLoadingDisk();
    // [crispy] re-calculate automap coordinates
    AM_ReInit();
}

void M_CrispyToggleHires(int)
{
    crispy->post_rendering_hook = M_CrispyToggleHiresHook;
}

void M_CrispyToggleJumping(int)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->jump = (crispy->jump + 1) % NUM_JUMPS;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleLeveltime(int)
{
    crispy->leveltime = (crispy->leveltime + 1) % NUM_WIDGETS;
}

void M_CrispyToggleMouseLook(int)
{
    crispy->mouselook = !crispy->mouselook;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleNeghealth(int)
{
    crispy->neghealth = !crispy->neghealth;
}

void M_CrispyToggleOverunder(int)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->overunder = !crispy->overunder;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyTogglePitch(int)
{
    crispy->pitch = !crispy->pitch;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyTogglePlayerCoords(int)
{
    crispy->playercoords = (crispy->playercoords + 1) % (NUM_WIDGETS - 1); // [crispy] disable "always" setting
}

void M_CrispyToggleRecoil(int)
{
    if (!crispy->singleplayer)
    {
        return;
    }

    crispy->recoil = !crispy->recoil;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleSecretmessage(int)
{
    crispy->secretmessage = (crispy->secretmessage + 1) % NUM_SECRETMESSAGE;
}

void M_CrispyToggleSmoothScaling(int)
{
    crispy->smoothscaling = !crispy->smoothscaling;
}

static void M_CrispyToggleSmoothLightingHook()
{
    crispy->smoothlight = !crispy->smoothlight;

    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
    // [crispy] re-calculate fake contrast
    P_SegLengths(true);
}

void M_CrispyToggleSmoothLighting(int)
{
    crispy->post_rendering_hook = M_CrispyToggleSmoothLightingHook;
}

void M_CrispyToggleSndChannels(int)
{
    S_UpdateSndChannels();
}

void M_CrispyToggleSoundfixes(int)
{
    crispy->soundfix = !crispy->soundfix;
}

void M_CrispyToggleSoundMono(int)
{
    crispy->soundmono = !crispy->soundmono;

    S_UpdateStereoSeparation();
}

void M_CrispyToggleTranslucency(int)
{
    crispy->translucency = (crispy->translucency + 1) % NUM_TRANSLUCENCY;
}

void M_CrispyToggleUncapped(int)
{
    crispy->uncapped = !crispy->uncapped;
}

void M_CrispyToggleVsyncHook()
{
    crispy->vsync = !crispy->vsync;

    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

void M_CrispyToggleVsync(int)
{
    if (force_software_renderer)
    {
        return;
    }

    crispy->post_rendering_hook = M_CrispyToggleVsyncHook;
}

void M_CrispyToggleWeaponSquat(int)
{
    crispy->weaponsquat = !crispy->weaponsquat;
}

void M_CrispyReinitHUDWidgets()
{
    if (gamestate == GS_LEVEL && gamemap > 0)
    {
        // [crispy] re-arrange status bar widgets
        ST_createWidgets();
        // [crispy] re-arrange heads-up widgets
        HU_Start();
    }
}

static void M_CrispyToggleWidescreenHook()
{
    crispy->widescreen = (crispy->widescreen + 1) % NUM_WIDESCREEN;

    // [crispy] no need to re-init when switching from wide to compact
    if (crispy->widescreen == 1 || crispy->widescreen == 0)
    {
        // [crispy] re-initialize screenSize_min
        M_SizeDisplay(-1);
        // [crispy] re-initialize framebuffers, textures and renderer
        I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
        // [crispy] re-calculate framebuffer coordinates
        R_ExecuteSetViewSize();
        // [crispy] re-draw bezel
        R_FillBackScreen();
        // [crispy] re-calculate disk icon coordinates
        EnableLoadingDisk();
        // [crispy] re-calculate automap coordinates
        AM_ReInit();
    }

    M_CrispyReinitHUDWidgets();
}

void M_CrispyToggleWidescreen(int)
{
    crispy->post_rendering_hook = M_CrispyToggleWidescreenHook;
}
