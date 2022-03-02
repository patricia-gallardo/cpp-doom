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
#include "r_defs.hpp"  // [crispy] laserpatch
#include "r_sky.hpp"   // [crispy] R_InitSkyMap()
#include "s_sound.hpp"

#include "m_crispy.hpp"

multiitem_t multiitem_bobfactor[NUM_BOBFACTORS] = {
  {BOBFACTOR_FULL, ("full")},
  { BOBFACTOR_75,  ("75%") },
  { BOBFACTOR_OFF, ("off") },
};

multiitem_t multiitem_brightmaps[NUM_BRIGHTMAPS] = {
  {BRIGHTMAPS_OFF,       ("none") },
  { BRIGHTMAPS_TEXTURES, ("walls")},
  { BRIGHTMAPS_SPRITES,  ("items")},
  { BRIGHTMAPS_BOTH,     ("both") },
};

multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] = {
  {CENTERWEAPON_OFF,     ("off")     },
  { CENTERWEAPON_CENTER, ("centered")},
  { CENTERWEAPON_BOB,    ("bobbing") },
};

multiitem_t multiitem_coloredhud[NUM_COLOREDHUD] = {
  {COLOREDHUD_OFF,   ("off")       },
  { COLOREDHUD_BAR,  ("status bar")},
  { COLOREDHUD_TEXT, ("hud texts") },
  { COLOREDHUD_BOTH, ("both")      },
};

multiitem_t multiitem_crosshair[NUM_CROSSHAIRS] = {
  {CROSSHAIR_OFF,        ("off")      },
  { CROSSHAIR_STATIC,    ("static")   },
  { CROSSHAIR_PROJECTED, ("projected")},
};

multiitem_t multiitem_crosshairtype[] = {
  {-1, ("none")   },
  { 0, ("cross")  },
  { 1, ("chevron")},
  { 2, ("dot")    },
};

multiitem_t multiitem_freeaim[NUM_FREEAIMS] = {
  {FREEAIM_AUTO,    ("autoaim")},
  { FREEAIM_DIRECT, ("direct") },
  { FREEAIM_BOTH,   ("both")   },
};

multiitem_t multiitem_demotimer[NUM_DEMOTIMERS] = {
  {DEMOTIMER_OFF,       ("off")      },
  { DEMOTIMER_RECORD,   ("recording")},
  { DEMOTIMER_PLAYBACK, ("playback") },
  { DEMOTIMER_BOTH,     ("both")     },
};

multiitem_t multiitem_demotimerdir[] = {
  {0,  ("none")    },
  { 1, ("forward") },
  { 2, ("backward")},
};

multiitem_t multiitem_freelook[NUM_FREELOOKS] = {
  {FREELOOK_OFF,     ("off")   },
  { FREELOOK_SPRING, ("spring")},
  { FREELOOK_LOCK,   ("lock")  },
};

multiitem_t multiitem_jump[NUM_JUMPS] = {
  {JUMP_OFF,   ("off") },
  { JUMP_LOW,  ("low") },
  { JUMP_HIGH, ("high")},
};

multiitem_t multiitem_secretmessage[NUM_SECRETMESSAGE] = {
  {SECRETMESSAGE_OFF,    ("off")  },
  { SECRETMESSAGE_ON,    ("on")   },
  { SECRETMESSAGE_COUNT, ("count")},
};

multiitem_t multiitem_translucency[NUM_TRANSLUCENCY] = {
  {TRANSLUCENCY_OFF,      ("off")        },
  { TRANSLUCENCY_MISSILE, ("projectiles")},
  { TRANSLUCENCY_ITEM,    ("items")      },
  { TRANSLUCENCY_BOTH,    ("both")       },
};

multiitem_t multiitem_sndchannels[4] = {
  {8,   ("8") },
  { 16, ("16")},
  { 32, ("32")},
};

multiitem_t multiitem_widescreen[NUM_WIDESCREEN] = {
  {WIDESCREEN_OFF,      ("off")            },
  { WIDESCREEN_WIDE,    ("on, wide HUD")   },
  { WIDESCREEN_COMPACT, ("on, compact HUD")},
};

multiitem_t multiitem_widgets[NUM_WIDGETS] = {
  {WIDGETS_OFF,      ("never")     },
  { WIDGETS_AUTOMAP, ("in Automap")},
  { WIDGETS_ALWAYS,  ("always")    },
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

void M_CrispyToggleAutomapstats(int) {
  crispy->automapstats = (crispy->automapstats + 1) % NUM_WIDGETS;
}

void M_CrispyToggleBobfactor(int) {
  crispy->bobfactor = (crispy->bobfactor + 1) % NUM_BOBFACTORS;
}

void M_CrispyToggleBrightmaps(int) {
  crispy->brightmaps = (crispy->brightmaps + 1) % NUM_BRIGHTMAPS;
}

void M_CrispyToggleCenterweapon(int) {
  crispy->centerweapon = (crispy->centerweapon + 1) % NUM_CENTERWEAPON;
}

void M_CrispyToggleColoredblood(int) {
  thinker_t * th = nullptr;

  if (g_doomstat_globals->gameversion == exe_chex) {
    return;
  }

  crispy->coloredblood = !crispy->coloredblood;

  // [crispy] switch NOBLOOD flag for Lost Souls
  for (th = g_p_local_globals->thinkercap.next; th && th != &g_p_local_globals->thinkercap; th = th->next) {
    action_hook hook = P_MobjThinker;
    if (th->function == hook) {
      auto * mobj = reinterpret_cast<mobj_t *>(th);

      if (mobj->type == MT_SKULL) {
        if (crispy->coloredblood) {
          mobj->flags |= MF_NOBLOOD;
        } else {
          mobj->flags &= ~MF_NOBLOOD;
        }
      }
    }
  }
}

void M_CrispyToggleColoredhud(int) {
  crispy->coloredhud = (crispy->coloredhud + 1) % NUM_COLOREDHUD;
}

void M_CrispyToggleCrosshair(int) {
  crispy->crosshair = (crispy->crosshair + 1) % NUM_CROSSHAIRS;
}

void M_CrispyToggleCrosshairHealth(int) {
  crispy->crosshairhealth = !crispy->crosshairhealth;
}

void M_CrispyToggleCrosshairTarget(int) {
  crispy->crosshairtarget = !crispy->crosshairtarget;
}

void M_CrispyToggleCrosshairtype(int) {
  if (!crispy->crosshair) {
    return;
  }

  crispy->crosshairtype = crispy->crosshairtype + 1;

  if (!laserpatch[crispy->crosshairtype].c) {
    crispy->crosshairtype = 0;
  }
}

void M_CrispyToggleDemoBar(int) {
  crispy->demobar = !crispy->demobar;
}

void M_CrispyToggleDemoTimer(int) {
  crispy->demotimer = (crispy->demotimer + 1) % NUM_DEMOTIMERS;
}

void M_CrispyToggleDemoTimerDir(int) {
  if (!(crispy->demotimer & DEMOTIMER_PLAYBACK)) {
    return;
  }

  crispy->demotimerdir = !crispy->demotimerdir;
}

void M_CrispyToggleExtAutomap(int) {
  crispy->extautomap = !crispy->extautomap;
}

[[maybe_unused]] void M_CrispyToggleExtsaveg(int) {
  crispy->extsaveg = !crispy->extsaveg;
}

void M_CrispyToggleFlipcorpses(int) {
  if (g_doomstat_globals->gameversion == exe_chex) {
    return;
  }

  crispy->flipcorpses = !crispy->flipcorpses;
}

void M_CrispyToggleFreeaim(int) {
  if (!crispy->singleplayer) {
    return;
  }

  crispy->freeaim = (crispy->freeaim + 1) % NUM_FREEAIMS;

  // [crispy] update the "critical" struct
  CheckCrispySingleplayer(!g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback && !g_doomstat_globals->netgame);
}

static void M_CrispyToggleSkyHook() {
  g_doomstat_globals->players[g_doomstat_globals->consoleplayer].lookdir = 0;
  R_InitSkyMap();
}

void M_CrispyToggleFreelook(int) {
  crispy->freelook = (crispy->freelook + 1) % NUM_FREELOOKS;

  crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleFullsounds(int) {
  crispy->soundfull = !crispy->soundfull;

  // [crispy] weapon sound sources
  for (int i = 0; i < MAXPLAYERS; i++) {
    if (g_doomstat_globals->playeringame[i]) {
      g_doomstat_globals->players[i].so = Crispy_PlayerSO(i);
    }
  }
}

static void M_CrispyToggleHiresHook() {
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

void M_CrispyToggleHires(int) {
  crispy->post_rendering_hook = M_CrispyToggleHiresHook;
}

void M_CrispyToggleJumping(int) {
  if (!crispy->singleplayer) {
    return;
  }

  crispy->jump = (crispy->jump + 1) % NUM_JUMPS;

  // [crispy] update the "critical" struct
  CheckCrispySingleplayer(!g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback && !g_doomstat_globals->netgame);
}

void M_CrispyToggleLeveltime(int) {
  crispy->leveltime = (crispy->leveltime + 1) % NUM_WIDGETS;
}

void M_CrispyToggleMouseLook(int) {
  crispy->mouselook = !crispy->mouselook;

  crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleNeghealth(int) {
  crispy->neghealth = !crispy->neghealth;
}

void M_CrispyToggleOverunder(int) {
  if (!crispy->singleplayer) {
    return;
  }

  crispy->overunder = !crispy->overunder;

  // [crispy] update the "critical" struct
  CheckCrispySingleplayer(!g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback && !g_doomstat_globals->netgame);
}

void M_CrispyTogglePitch(int) {
  crispy->pitch = !crispy->pitch;

  crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyTogglePlayerCoords(int) {
  crispy->playercoords = (crispy->playercoords + 1) % (NUM_WIDGETS - 1); // [crispy] disable "always" setting
}

void M_CrispyToggleRecoil(int) {
  if (!crispy->singleplayer) {
    return;
  }

  crispy->recoil = !crispy->recoil;

  // [crispy] update the "critical" struct
  CheckCrispySingleplayer(!g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback && !g_doomstat_globals->netgame);
}

void M_CrispyToggleSecretmessage(int) {
  crispy->secretmessage = (crispy->secretmessage + 1) % NUM_SECRETMESSAGE;
}

void M_CrispyToggleSmoothScaling(int) {
  crispy->smoothscaling = !crispy->smoothscaling;
}

static void M_CrispyToggleSmoothLightingHook() {
  crispy->smoothlight = !crispy->smoothlight;

  // [crispy] re-calculate the zlight[][] array
  R_InitLightTables();
  // [crispy] re-calculate the scalelight[][] array
  R_ExecuteSetViewSize();
  // [crispy] re-calculate fake contrast
  P_SegLengths(true);
}

void M_CrispyToggleSmoothLighting(int) {
  crispy->post_rendering_hook = M_CrispyToggleSmoothLightingHook;
}

void M_CrispyToggleSndChannels(int) {
  S_UpdateSndChannels();
}

void M_CrispyToggleSoundfixes(int) {
  crispy->soundfix = !crispy->soundfix;
}

void M_CrispyToggleSoundMono(int) {
  crispy->soundmono = !crispy->soundmono;

  S_UpdateStereoSeparation();
}

void M_CrispyToggleTranslucency(int) {
  crispy->translucency = (crispy->translucency + 1) % NUM_TRANSLUCENCY;
}

void M_CrispyToggleUncapped(int) {
  crispy->uncapped = !crispy->uncapped;
}

void M_CrispyToggleVsyncHook() {
  crispy->vsync = !crispy->vsync;

  I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

void M_CrispyToggleVsync(int) {
  if (g_i_video_globals->force_software_renderer) {
    return;
  }

  crispy->post_rendering_hook = M_CrispyToggleVsyncHook;
}

void M_CrispyToggleWeaponSquat(int) {
  crispy->weaponsquat = !crispy->weaponsquat;
}

void M_CrispyReinitHUDWidgets() {
  if (g_doomstat_globals->gamestate == GS_LEVEL && g_doomstat_globals->gamemap > 0) {
    // [crispy] re-arrange status bar widgets
    ST_createWidgets();
    // [crispy] re-arrange heads-up widgets
    HU_Start();
  }
}

static void M_CrispyToggleWidescreenHook() {
  crispy->widescreen = (crispy->widescreen + 1) % NUM_WIDESCREEN;

  // [crispy] no need to re-init when switching from wide to compact
  if (crispy->widescreen == 1 || crispy->widescreen == 0) {
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

void M_CrispyToggleWidescreen(int) {
  crispy->post_rendering_hook = M_CrispyToggleWidescreenHook;
}
