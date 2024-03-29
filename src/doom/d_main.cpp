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
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//

#include <cctype>
#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include "cstring_view.hpp"
#include "deh_main.hpp"
#include "doomdef.hpp"
#include "doomstat.hpp"

#include "dstrings.hpp"
#include "sounds.hpp"

#include "d_iwad.hpp"

#include "s_sound.hpp"
#include "v_diskicon.hpp"
#include "v_video.hpp"
#include "w_main.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "f_finale.hpp"
#include "f_wipe.hpp"

#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_controls.hpp"
#include "m_menu.hpp"
#include "m_misc.hpp"
#include "p_saveg.hpp"

#include "i_endoom.hpp"
#include "i_input.hpp"
#include "i_joystick.hpp"
#include "i_system.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"

#include "g_game.hpp"

#include "am_map.hpp"
#include "hu_stuff.hpp"
#include "net_client.hpp"
#include "net_dedicated.hpp"
#include "net_query.hpp"
#include "st_stuff.hpp"
#include "wi_stuff.hpp"

#include "p_setup.hpp"
#include "r_local.hpp"
#include "statdump.hpp"

#include "d_main.hpp"
#include "lump.hpp"
#include "memory.hpp"

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
[[noreturn]] void D_DoomLoop();

// location of IWAD and WAD files

char * iwadfile;

extern bool inhelpscreens;

bool advancedemo;

// Store demo, do not accept any inputs
bool storedemo;

// If true, the main game loop has started.
bool main_loop_started = false;

[[maybe_unused]] char wadfile[1024]; // primary wad file
[[maybe_unused]] char mapdir[1024];  // directory of development maps

int show_endoom   = 0; // [crispy] disable
int show_diskicon = 1;

void D_ConnectNetGame();
void D_CheckNetGame();

//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents() {
  event_t * ev = nullptr;

  // IF STORE DEMO, DO NOT ACCEPT INPUT
  if (storedemo)
    return;

  while ((ev = D_PopEvent()) != nullptr) {
    if (M_Responder(ev))
      continue; // menu ate the event
    G_Responder(ev);
  }
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

extern bool setsizeneeded;
extern int  showMessages;
void        R_ExecuteSetViewSize();

bool D_Display() {
  static bool        viewactivestate    = false;
  static bool        menuactivestate    = false;
  static bool        inhelpscreensstate = false;
  static bool        fullscreen_local   = false;
  static gamestate_t oldgamestate { GS_FORCE_WIPE };
  static int         borderdrawcount;
  bool               wipe       = false;
  bool               redrawsbar = false;

  // change the view size if needed
  if (setsizeneeded) {
    R_ExecuteSetViewSize();
    oldgamestate    = GS_FORCE_WIPE; // force background redraw
    borderdrawcount = 3;
  }

  // save the current screen if about to wipe
  if (g_doomstat_globals->gamestate != g_doomstat_globals->wipegamestate) {
    wipe = true;
    wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
  } else
    wipe = false;

  if (g_doomstat_globals->gamestate == GS_LEVEL && gametic)
    HU_Erase();

  // do buffered drawing
  switch (g_doomstat_globals->gamestate) {
  case GS_LEVEL:
    if (!gametic)
      break;
    if (g_doomstat_globals->automapactive && !crispy->automapoverlay) {
      // [crispy] update automap while playing
      R_RenderPlayerView(&g_doomstat_globals->players[g_doomstat_globals->displayplayer]);
      AM_Drawer();
    }
    if (wipe || (g_r_state_globals->viewheight != SCREENHEIGHT && fullscreen_local))
      redrawsbar = true;
    if (inhelpscreensstate && !inhelpscreens)
      redrawsbar = true; // just put away the help screen
    ST_Drawer(g_r_state_globals->viewheight == SCREENHEIGHT, redrawsbar);
    fullscreen_local = g_r_state_globals->viewheight == SCREENHEIGHT;
    break;

  case GS_INTERMISSION:
    WI_Drawer();
    break;

  case GS_FINALE:
    F_Drawer();
    break;

  case GS_DEMOSCREEN:
    D_PageDrawer();
    break;

  case GS_FORCE_WIPE:
    // theoretically this just caused a background redraw (see above)
    break;
  }

  // draw the view directly
  if (g_doomstat_globals->gamestate == GS_LEVEL && (!g_doomstat_globals->automapactive || crispy->automapoverlay) && gametic) {
    R_RenderPlayerView(&g_doomstat_globals->players[g_doomstat_globals->displayplayer]);

    // [crispy] Crispy HUD
    if (screenblocks >= CRISPY_HUD)
      ST_Drawer(false, true);
  }

  // [crispy] in automap overlay mode,
  // the HUD is drawn on top of everything else
  if (g_doomstat_globals->gamestate == GS_LEVEL && gametic && !(g_doomstat_globals->automapactive && crispy->automapoverlay))
    HU_Drawer();

  // clean up border stuff
  if (g_doomstat_globals->gamestate != oldgamestate && g_doomstat_globals->gamestate != GS_LEVEL)
#ifndef CRISPY_TRUECOLOR
    I_SetPalette(cache_lump_name<uint8_t *>(DEH_String("PLAYPAL"), PU_CACHE));
#else
    I_SetPalette(0);
#endif

  // see if the border needs to be initially drawn
  if (g_doomstat_globals->gamestate == GS_LEVEL && oldgamestate != GS_LEVEL) {
    viewactivestate = false; // view was not active
    R_FillBackScreen();      // draw the pattern into the back screen
  }

  // see if the border needs to be updated to the screen
  if (g_doomstat_globals->gamestate == GS_LEVEL && (!g_doomstat_globals->automapactive || crispy->automapoverlay) && g_r_state_globals->scaledviewwidth != SCREENWIDTH) {
    if (g_doomstat_globals->menuactive || menuactivestate || !viewactivestate)
      borderdrawcount = 3;
    if (borderdrawcount) {
      R_DrawViewBorder(); // erase old menu stuff
      borderdrawcount--;
    }
  }

  if (g_doomstat_globals->testcontrols) {
    // Box showing current mouse speed

    V_DrawMouseSpeedBox(g_doomstat_globals->testcontrols_mousespeed);
  }

  menuactivestate    = g_doomstat_globals->menuactive;
  viewactivestate    = g_doomstat_globals->viewactive;
  inhelpscreensstate = inhelpscreens;
  oldgamestate = g_doomstat_globals->wipegamestate = g_doomstat_globals->gamestate;

  // [crispy] in automap overlay mode,
  // draw the automap and HUD on top of everything else
  if (g_doomstat_globals->automapactive && crispy->automapoverlay) {
    AM_Drawer();
    HU_Drawer();

    // [crispy] force redraw of status bar and border
    viewactivestate    = false;
    inhelpscreensstate = true;
  }

  // [crispy] draw neither pause pic nor menu when taking a clean screenshot
  if (crispy->cleanscreenshot) {
    return false;
  }

  // draw pause pic
  if (g_doomstat_globals->paused) {
    int y = 0;
    if (g_doomstat_globals->automapactive && !crispy->automapoverlay)
      y = 4;
    else
      y = (viewwindowy >> crispy->hires) + 4;
    V_DrawPatchDirect((viewwindowx >> crispy->hires) + ((g_r_state_globals->scaledviewwidth >> crispy->hires) - 68) / 2 - DELTAWIDTH, y, cache_lump_name<patch_t *>(DEH_String("M_PAUSE"), PU_CACHE));
  }

  // menus go directly to the screen
  M_Drawer();  // menu is drawn even on top of everything
  NetUpdate(); // send out any new accumulation

  return wipe;
}

void EnableLoadingDisk() // [crispy] un-static
{
  const char * disk_lump_name = nullptr;

  if (show_diskicon) {
    if (M_CheckParm("-cdrom") > 0) {
      disk_lump_name = DEH_String("STCDROM");
    } else {
      disk_lump_name = DEH_String("STDISK");
    }

    V_EnableLoadingDisk(disk_lump_name,
                        SCREENWIDTH - LOADING_DISK_W,
                        SCREENHEIGHT - LOADING_DISK_H);
  }
}

//
// Add configuration file variable bindings.
//

void D_BindVariables() {
  M_ApplyPlatformDefaults();

  I_BindInputVariables();
  I_BindVideoVariables();
  I_BindJoystickVariables();
  I_BindSoundVariables();

  M_BindBaseControls();
  M_BindWeaponControls();
  M_BindMapControls();
  M_BindMenuControls();
  M_BindChatControls(MAXPLAYERS);

  g_m_controls_globals->key_multi_msgplayer[0] = HUSTR_KEYGREEN;
  g_m_controls_globals->key_multi_msgplayer[1] = HUSTR_KEYINDIGO;
  g_m_controls_globals->key_multi_msgplayer[2] = HUSTR_KEYBROWN;
  g_m_controls_globals->key_multi_msgplayer[3] = HUSTR_KEYRED;

  NET_BindVariables();

  M_BindIntVariable("mouse_sensitivity", &g_doomstat_globals->mouseSensitivity);
  M_BindIntVariable("mouse_sensitivity_x2", &g_doomstat_globals->mouseSensitivity_x2); // [crispy]
  M_BindIntVariable("mouse_sensitivity_y", &g_doomstat_globals->mouseSensitivity_y);   // [crispy]
  M_BindIntVariable("sfx_volume", &g_doomstat_globals->sfxVolume);
  M_BindIntVariable("music_volume", &g_doomstat_globals->musicVolume);
  M_BindIntVariable("show_messages", &showMessages);
  M_BindIntVariable("screenblocks", &screenblocks);
  M_BindIntVariable("detaillevel", &detailLevel);
  M_BindIntVariable("snd_channels", &snd_channels);
  // [crispy] unconditionally disable savegame and demo limits
  //  M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
  //  M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
  M_BindIntVariable("show_endoom", &show_endoom);
  M_BindIntVariable("show_diskicon", &show_diskicon);

  // Multiplayer chat macros

  for (int i = 0; i < 10; ++i) {
    char buf[12];

    M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
    M_BindStringVariable(buf, &chat_macros[i]);
  }

  // [crispy] bind "crispness" config variables
  M_BindIntVariable("crispy_automapoverlay", &crispy->automapoverlay);
  M_BindIntVariable("crispy_automaprotate", &crispy->automaprotate);
  M_BindIntVariable("crispy_automapstats", &crispy->automapstats);
  M_BindIntVariable("crispy_bobfactor", &crispy->bobfactor);
  M_BindIntVariable("crispy_brightmaps", &crispy->brightmaps);
  M_BindIntVariable("crispy_centerweapon", &crispy->centerweapon);
  M_BindIntVariable("crispy_coloredblood", &crispy->coloredblood);
  M_BindIntVariable("crispy_coloredhud", &crispy->coloredhud);
  M_BindIntVariable("crispy_crosshair", &crispy->crosshair);
  M_BindIntVariable("crispy_crosshairhealth", &crispy->crosshairhealth);
  M_BindIntVariable("crispy_crosshairtarget", &crispy->crosshairtarget);
  M_BindIntVariable("crispy_crosshairtype", &crispy->crosshairtype);
  M_BindIntVariable("crispy_demobar", &crispy->demobar);
  M_BindIntVariable("crispy_demotimer", &crispy->demotimer);
  M_BindIntVariable("crispy_demotimerdir", &crispy->demotimerdir);
  M_BindIntVariable("crispy_extautomap", &crispy->extautomap);
  M_BindIntVariable("crispy_extsaveg", &crispy->extsaveg);
  M_BindIntVariable("crispy_flipcorpses", &crispy->flipcorpses);
  M_BindIntVariable("crispy_freeaim", &crispy->freeaim);
  M_BindIntVariable("crispy_freelook", &crispy->freelook);
  M_BindIntVariable("crispy_hires", &crispy->hires);
  M_BindIntVariable("crispy_jump", &crispy->jump);
  M_BindIntVariable("crispy_leveltime", &crispy->leveltime);
  M_BindIntVariable("crispy_mouselook", &crispy->mouselook);
  M_BindIntVariable("crispy_neghealth", &crispy->neghealth);
  M_BindIntVariable("crispy_overunder", &crispy->overunder);
  M_BindIntVariable("crispy_pitch", &crispy->pitch);
  M_BindIntVariable("crispy_playercoords", &crispy->playercoords);
  M_BindIntVariable("crispy_recoil", &crispy->recoil);
  M_BindIntVariable("crispy_secretmessage", &crispy->secretmessage);
  M_BindIntVariable("crispy_smoothlight", &crispy->smoothlight);
  M_BindIntVariable("crispy_smoothscaling", &crispy->smoothscaling);
  M_BindIntVariable("crispy_soundfix", &crispy->soundfix);
  M_BindIntVariable("crispy_soundfull", &crispy->soundfull);
  M_BindIntVariable("crispy_soundmono", &crispy->soundmono);
  M_BindIntVariable("crispy_translucency", &crispy->translucency);
#ifdef CRISPY_TRUECOLOR
  M_BindIntVariable("crispy_truecolor", &crispy->truecolor);
#endif
  M_BindIntVariable("crispy_uncapped", &crispy->uncapped);
  M_BindIntVariable("crispy_vsync", &crispy->vsync);
  M_BindIntVariable("crispy_weaponsquat", &crispy->weaponsquat);
  M_BindIntVariable("crispy_widescreen", &crispy->widescreen);
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

bool D_GrabMouseCallback() {
  // Drone players don't need mouse focus

  if (g_net_client_globals->drone)
    return false;

  // when menu is active or game is paused, release the mouse

  if (g_doomstat_globals->menuactive || g_doomstat_globals->paused)
    return false;

  // only grab mouse when playing levels (but not demos)

  return (g_doomstat_globals->gamestate == GS_LEVEL) && !g_doomstat_globals->demoplayback && !advancedemo;
}

//
//  D_RunFrame
//
void D_RunFrame() {
  int         nowtime   = 0;
  int         tics      = 0;
  static int  wipestart = 0;
  static bool wipe      = false;

  if (wipe) {
    do {
      nowtime = I_GetTime();
      tics    = nowtime - wipestart;
      I_Sleep(1);
    } while (tics <= 0);

    wipestart = nowtime;
    wipe      = !wipe_ScreenWipe(wipe_Melt, 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
    M_Drawer();       // menu is drawn even on top of wipes
    I_FinishUpdate(); // page flip or blit buffer
    return;
  }

  // frame syncronous IO operations
  I_StartFrame();

  TryRunTics(); // will run at least one tic

  S_UpdateSounds(g_doomstat_globals->players[g_doomstat_globals->consoleplayer].mo); // move positional sounds

  // Update display, next frame, with current state if no profiling is on
  if (g_i_video_globals->screenvisible && !g_doomstat_globals->nodrawers) {
    bool is_set = wipe = D_Display();
    if (is_set) {
      // start wipe on this frame
      wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

      wipestart = I_GetTime() - 1;
    } else {
      // normal update
      I_FinishUpdate(); // page flip or blit buffer
    }
  }

  // [crispy] post-rendering function pointer to apply config changes
  // that affect rendering and that are better applied after the current
  // frame has finished rendering
  if (crispy->post_rendering_hook && !wipe) {
    crispy->post_rendering_hook();
    crispy->post_rendering_hook = nullptr;
  }
}

//
//  D_DoomLoop
//
[[noreturn]] void D_DoomLoop() {
  if (g_doomstat_globals->gamevariant == bfgedition && (g_doomstat_globals->demorecording || (gameaction == ga_playdemo) || g_doomstat_globals->netgame)) {
    fmt::printf(" WARNING: You are playing using one of the Doom Classic\n"
                " IWAD files shipped with the Doom 3: BFG Edition. These are\n"
                " known to be incompatible with the regular IWAD files and\n"
                " may cause demos and network games to get out of sync.\n");
  }

  // [crispy] no need to write a demo header in demo continue mode
  if (g_doomstat_globals->demorecording && gameaction != ga_playdemo)
    G_BeginRecording();

  main_loop_started = true;

  I_SetWindowTitle(g_doomstat_globals->gamedescription);
  I_GraphicsCheckCommandLine();
  I_SetGrabMouseCallback(D_GrabMouseCallback);
  I_InitGraphics();
  // [crispy] re-init HUD widgets now just in case graphics were not initialized before
  if (crispy->widescreen && g_i_video_globals->aspect_ratio_correct) {
    extern void M_CrispyReinitHUDWidgets();
    M_CrispyReinitHUDWidgets();
  }
  EnableLoadingDisk();

  TryRunTics();

  V_RestoreBuffer();
  R_ExecuteSetViewSize();

  D_StartGameLoop();

  if (g_doomstat_globals->testcontrols) {
    g_doomstat_globals->wipegamestate = g_doomstat_globals->gamestate;
  }

  while (true) {
    D_RunFrame();
  }
}

//
//  DEMO LOOP
//
int          demosequence;
int          pagetic;
const char * pagename;

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker() {
  if (--pagetic < 0)
    D_AdvanceDemo();
}

//
// D_PageDrawer
//
void D_PageDrawer() {
  V_DrawPatchFullScreen(cache_lump_name<patch_t *>(pagename, PU_CACHE), crispy->fliplevels);
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo() {
  advancedemo = true;
}

//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
void D_DoAdvanceDemo() {
  g_doomstat_globals->players[g_doomstat_globals->consoleplayer].playerstate = PST_LIVE; // not reborn
  advancedemo                                                                = false;
  g_doomstat_globals->usergame                                               = false; // no save / end game here
  g_doomstat_globals->paused                                                 = false;
  gameaction                                                                 = ga_nothing;
  // [crispy] update the "singleplayer" variable
  CheckCrispySingleplayer(!g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback && !g_doomstat_globals->netgame);

  // The Ultimate Doom executable changed the demo sequence to add
  // a DEMO4 demo.  Final Doom was based on Ultimate, so also
  // includes this change; however, the Final Doom IWADs do not
  // include a DEMO4 lump, so the game bombs out with an error
  // when it reaches this point in the demo sequence.

  // However! There is an alternate version of Final Doom that
  // includes a fixed executable.

  // [crispy] get rid of this demo sequence breaking bug
  /*
  if (gameversion == exe_ultimate || gameversion == exe_final)
  */
  if (W_CheckNumForName(DEH_String("demo4")) >= 0)
    demosequence = (demosequence + 1) % 7;
  else
    demosequence = (demosequence + 1) % 6;

  switch (demosequence) {
  case 0:
    if (g_doomstat_globals->gamemode == commercial)
      pagetic = TICRATE * 11;
    else
      pagetic = 170;
    g_doomstat_globals->gamestate = GS_DEMOSCREEN;
    pagename                      = DEH_String("TITLEPIC");
    if (g_doomstat_globals->gamemode == commercial)
      S_StartMusic(mus_dm2ttl);
    else
      S_StartMusic(mus_intro);
    break;
  case 1:
    G_DeferedPlayDemo(DEH_String("demo1"));
    break;
  case 2:
    pagetic                       = 200;
    g_doomstat_globals->gamestate = GS_DEMOSCREEN;
    pagename                      = DEH_String("CREDIT");
    break;
  case 3:
    G_DeferedPlayDemo(DEH_String("demo2"));
    break;
  case 4:
    g_doomstat_globals->gamestate = GS_DEMOSCREEN;
    if (g_doomstat_globals->gamemode == commercial) {
      pagetic  = TICRATE * 11;
      pagename = DEH_String("TITLEPIC");
      S_StartMusic(mus_dm2ttl);
    } else {
      pagetic = 200;

      if (g_doomstat_globals->gameversion >= exe_ultimate)
        pagename = DEH_String("CREDIT");
      else
        pagename = DEH_String("HELP2");
    }
    break;
  case 5:
    G_DeferedPlayDemo(DEH_String("demo3"));
    break;
    // THE DEFINITIVE DOOM Special Edition demo
  case 6:
    G_DeferedPlayDemo(DEH_String("demo4"));
    break;
  }

  // The Doom 3: BFG Edition version of doom2.wad does not have a
  // TITLETPIC lump. Use INTERPIC instead as a workaround.
  if (g_doomstat_globals->gamevariant == bfgedition && iequals(pagename, "TITLEPIC")
      && W_CheckNumForName("titlepic") < 0) {
    // [crispy] use DMENUPIC instead of TITLEPIC, it's awesome
    pagename = DEH_String("DMENUPIC");
  }
}

//
// D_StartTitle
//
void D_StartTitle() {
  gameaction   = ga_nothing;
  demosequence = -1;
  D_AdvanceDemo();
}

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static const char * banners[] = {
  // doom2.wad
  "                         "
  "DOOM 2: Hell on Earth v%i.%i"
  "                           ",
  // doom2.wad v1.666
  "                         "
  "DOOM 2: Hell on Earth v%i.%i66"
  "                          ",
  // doom1.wad
  "                            "
  "DOOM Shareware Startup v%i.%i"
  "                           ",
  // doom.wad
  "                            "
  "DOOM Registered Startup v%i.%i"
  "                           ",
  // Registered DOOM uses this
  "                          "
  "DOOM System Startup v%i.%i"
  "                          ",
  // Doom v1.666
  "                          "
  "DOOM System Startup v%i.%i66"
  "                          "
  // doom.wad (Ultimate DOOM)
  "                         "
  "The Ultimate DOOM Startup v%i.%i"
  "                        ",
  // tnt.wad
  "                     "
  "DOOM 2: TNT - Evilution v%i.%i"
  "                           ",
  // plutonia.wad
  "                   "
  "DOOM 2: Plutonia Experiment v%i.%i"
  "                           ",
};

//
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
//

static const char * GetGameName(cstring_view gamename) {
  for (auto & banner : banners) {
    // Has the banner been replaced?

    const char * deh_sub = DEH_String(banner);

    if (deh_sub != banner) {
      // Has been replaced.
      // We need to expand via printf to include the Doom version number
      // We also need to cut off spaces to get the basic name

      const auto newgamename_size = strlen(deh_sub) + 10;
      auto *     newgamename      = zmalloc<char *>(newgamename_size, PU_STATIC, 0);
      int        version          = G_VanillaVersionCode();
      M_snprintf(newgamename, newgamename_size, deh_sub, version / 100, version % 100);

      while (newgamename[0] != '\0' && isspace(newgamename[0])) {
        std::memmove(newgamename, newgamename + 1, newgamename_size - 1);
      }

      while (newgamename[0] != '\0' && isspace(newgamename[strlen(newgamename) - 1])) {
        newgamename[strlen(newgamename) - 1] = '\0';
      }

      return newgamename;
    }
  }

  return gamename.c_str();
}

static void SetMissionForPackName(cstring_view pack_name) {
  static constexpr struct
  {
    const char *  name;
    GameMission_t mission;
  } packs[] = {
    {"doom2",     doom2    },
    { "tnt",      pack_tnt },
    { "plutonia", pack_plut},
  };

  for (const auto & pack : packs) {
    if (iequals(pack_name, pack.name)) {
      g_doomstat_globals->gamemission = pack.mission;
      return;
    }
  }

  fmt::printf("Valid mission packs are:\n");

  for (const auto & pack : packs) {
    fmt::printf("\t%s\n", pack.name);
  }

  I_Error("Unknown mission pack name: %s", pack_name.c_str());
}

//
// Find out what version of Doom is playing.
//

void D_IdentifyVersion() {
  // gamemission is set up by the D_FindIWAD function.  But if
  // we specify '-iwad', we have to identify using
  // IdentifyIWADByName.  However, if the iwad does not match
  // any known IWAD name, we may have a dilemma.  Try to
  // identify by its contents.

  if (g_doomstat_globals->gamemission == none) {
    for (unsigned int i = 0; i < numlumps; ++i) {
      if (!strncasecmp(lumpinfo[i]->name, "MAP01", 8)) {
        g_doomstat_globals->gamemission = doom2;
        break;
      } else if (!strncasecmp(lumpinfo[i]->name, "E1M1", 8)) {
        g_doomstat_globals->gamemission = doom;
        break;
      }
    }

    if (g_doomstat_globals->gamemission == none) {
      // Still no idea.  I don't think this is going to work.

      I_Error("Unknown or invalid IWAD file.");
    }
  }

  // Make sure gamemode is set up correctly

  if (logical_gamemission() == doom) {
    // Doom 1.  But which version?

    if (W_CheckNumForName("E4M1") > 0) {
      // Ultimate Doom

      g_doomstat_globals->gamemode = retail;
    } else if (W_CheckNumForName("E3M1") > 0) {
      g_doomstat_globals->gamemode = registered;
    } else {
      g_doomstat_globals->gamemode = shareware;
    }
  } else {
    // Doom 2 of some kind.
    g_doomstat_globals->gamemode = commercial;

    // We can manually override the gamemission that we got from the
    // IWAD detection code. This allows us to eg. play Plutonia 2
    // with Freedoom and get the right level names.

    //!
    // @category compat
    // @arg <pack>
    //
    // Explicitly specify a Doom II "mission pack" to run as, instead of
    // detecting it based on the filename. Valid values are: "doom2",
    // "tnt" and "plutonia".
    //
    int p = M_CheckParmWithArgs("-pack", 1);
    if (p > 0) {
      SetMissionForPackName(myargv[p + 1]);
    }
  }
}

// Set the gamedescription string

void D_SetGameDescription() {
  g_doomstat_globals->gamedescription = "Unknown";

  if (logical_gamemission() == doom) {
    // Doom 1.  But which version?

    if (g_doomstat_globals->gamevariant == freedoom) {
      g_doomstat_globals->gamedescription = GetGameName("Freedoom: Phase 1");
    } else if (g_doomstat_globals->gamemode == retail) {
      // Ultimate Doom

      g_doomstat_globals->gamedescription = GetGameName("The Ultimate DOOM");
    } else if (g_doomstat_globals->gamemode == registered) {
      g_doomstat_globals->gamedescription = GetGameName("DOOM Registered");
    } else if (g_doomstat_globals->gamemode == shareware) {
      g_doomstat_globals->gamedescription = GetGameName("DOOM Shareware");
    }
  } else {
    // Doom 2 of some kind.  But which mission?

    if (g_doomstat_globals->gamevariant == freedm) {
      g_doomstat_globals->gamedescription = GetGameName("FreeDM");
    } else if (g_doomstat_globals->gamevariant == freedoom) {
      g_doomstat_globals->gamedescription = GetGameName("Freedoom: Phase 2");
    } else if (logical_gamemission() == doom2) {
      g_doomstat_globals->gamedescription = GetGameName("DOOM 2: Hell on Earth");
    } else if (logical_gamemission() == pack_plut) {
      g_doomstat_globals->gamedescription = GetGameName("DOOM 2: Plutonia Experiment");
    } else if (logical_gamemission() == pack_tnt) {
      g_doomstat_globals->gamedescription = GetGameName("DOOM 2: TNT - Evilution");
    } else if (logical_gamemission() == pack_nerve) {
      g_doomstat_globals->gamedescription = GetGameName("DOOM 2: No Rest For The Living");
    } else if (logical_gamemission() == pack_master) {
      g_doomstat_globals->gamedescription = GetGameName("Master Levels for DOOM 2");
    }
  }
}

//      print title for every printed line
char title[128];

static bool D_AddFile(char * filename) {
  fmt::printf(" adding %s\n", filename);
  wad_file_t * handle = W_AddFile(filename);
  return handle != nullptr;
}

// Copyright message banners
// Some dehacked mods replace these.  These are only displayed if they are
// replaced by dehacked.

static const char * copyright_banners[] = {
  "===========================================================================\n"
  "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"
  "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"
  "        You will not receive technical support for modified games.\n"
  "                      press enter to continue\n"
  "===========================================================================\n",

  "===========================================================================\n"
  "                 Commercial product - do not distribute!\n"
  "         Please report software piracy to the SPA: 1-800-388-PIR8\n"
  "===========================================================================\n",

  "===========================================================================\n"
  "                                Shareware!\n"
  "===========================================================================\n"
};

// Prints a message only if it has been modified by dehacked.

void PrintDehackedBanners() {
  for (auto & copyright_banner : copyright_banners) {
    const char * deh_s = DEH_String(copyright_banner);

    if (deh_s != copyright_banner) {
      fmt::printf("%s", deh_s);

      // Make sure the modified banner always ends in a newline character.
      // If it doesn't, add a newline.  This fixes av.wad.

      if (deh_s[strlen(deh_s) - 1] != '\n') {
        fmt::printf("\n");
      }
    }
  }
}

static constexpr struct
{
  const char *  description;
  const char *  cmdline;
  GameVersion_t version;
} gameversions[] = {
  {"Doom 1.2",          "1.2",      exe_doom_1_2  },
  { "Doom 1.666",       "1.666",    exe_doom_1_666},
  { "Doom 1.7/1.7a",    "1.7",      exe_doom_1_7  },
  { "Doom 1.8",         "1.8",      exe_doom_1_8  },
  { "Doom 1.9",         "1.9",      exe_doom_1_9  },
  { "Hacx",             "hacx",     exe_hacx      },
  { "Ultimate Doom",    "ultimate", exe_ultimate  },
  { "Final Doom",       "final",    exe_final     },
  { "Final Doom (alt)", "final2",   exe_final2    },
  { "Chex Quest",       "chex",     exe_chex      },
  { nullptr,            nullptr,    exe_doom_1_2  },
};

// Initialize the game version

static void InitGameVersion() {
  //!
  // @arg <version>
  // @category compat
  //
  // Emulate a specific version of Doom.  Valid values are "1.2",
  // "1.666", "1.7", "1.8", "1.9", "ultimate", "final", "final2",
  // "hacx" and "chex".
  //

  int p = M_CheckParmWithArgs("-gameversion", 1);

  if (p) {
    int index = 0;
    for (index = 0; gameversions[index].description != nullptr; ++index) {
      if (!strcmp(myargv[p + 1], gameversions[index].cmdline)) {
        g_doomstat_globals->gameversion = gameversions[index].version;
        break;
      }
    }

    if (gameversions[index].description == nullptr) {
      fmt::printf("Supported game versions:\n");

      for (index = 0; gameversions[index].description != nullptr; ++index) {
        fmt::printf("\t%s (%s)\n", gameversions[index].cmdline, gameversions[index].description);
      }

      I_Error("Unknown game version '%s'", myargv[p + 1]);
    }
  } else {
    // Determine automatically

    if (g_doomstat_globals->gamemission == pack_chex) {
      // chex.exe - identified by iwad filename

      g_doomstat_globals->gameversion = exe_chex;
    } else if (g_doomstat_globals->gamemission == pack_hacx) {
      // hacx.exe: identified by iwad filename

      g_doomstat_globals->gameversion = exe_hacx;
    } else if (g_doomstat_globals->gamemode == shareware || g_doomstat_globals->gamemode == registered
               || (g_doomstat_globals->gamemode == commercial && g_doomstat_globals->gamemission == doom2)) {
      // original
      g_doomstat_globals->gameversion = exe_doom_1_9;

      // Detect version from demo lump
      for (int i = 1; i <= 3; ++i) {
        char demolumpname[6];
        M_snprintf(demolumpname, 6, "demo%i", i);
        if (W_CheckNumForName(demolumpname) > 0) {
          uint8_t * demolump    = cache_lump_name<uint8_t *>(demolumpname, PU_STATIC);
          int       demoversion = demolump[0];
          W_ReleaseLumpName(demolumpname);
          bool status = true;
          switch (demoversion) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
            g_doomstat_globals->gameversion = exe_doom_1_2;
            break;
          case 106:
            g_doomstat_globals->gameversion = exe_doom_1_666;
            break;
          case 107:
            g_doomstat_globals->gameversion = exe_doom_1_7;
            break;
          case 108:
            g_doomstat_globals->gameversion = exe_doom_1_8;
            break;
          case 109:
            g_doomstat_globals->gameversion = exe_doom_1_9;
            break;
          default:
            status = false;
            break;
          }
          if (status) {
            break;
          }
        }
      }
    } else if (g_doomstat_globals->gamemode == retail) {
      g_doomstat_globals->gameversion = exe_ultimate;
    } else if (g_doomstat_globals->gamemode == commercial) {
      // Final Doom: tnt or plutonia
      // Defaults to emulating the first Final Doom executable,
      // which has the crash in the demo loop; however, having
      // this as the default should mean that it plays back
      // most demos correctly.

      g_doomstat_globals->gameversion = exe_final;
    }
  }

  // Deathmatch 2.0 did not exist until Doom v1.4
  if (g_doomstat_globals->gameversion <= exe_doom_1_2 && g_doomstat_globals->deathmatch == 2) {
    g_doomstat_globals->deathmatch = 1;
  }

  // The original exe does not support retail - 4th episode not supported

  if (g_doomstat_globals->gameversion < exe_ultimate && g_doomstat_globals->gamemode == retail) {
    g_doomstat_globals->gamemode = registered;
  }

  // EXEs prior to the Final Doom exes do not support Final Doom.

  if (g_doomstat_globals->gameversion < exe_final && g_doomstat_globals->gamemode == commercial
      && (g_doomstat_globals->gamemission == pack_tnt || g_doomstat_globals->gamemission == pack_plut)) {
    g_doomstat_globals->gamemission = doom2;
  }
}

void PrintGameVersion() {
  for (int i = 0; gameversions[i].description != nullptr; ++i) {
    if (gameversions[i].version == g_doomstat_globals->gameversion) {
      fmt::printf("Emulating the behavior of the "
                  "'%s' executable.\n",
                  gameversions[i].description);
      break;
    }
  }
}

// Function called at exit to display the ENDOOM screen

static void D_Endoom() {
  // Don't show ENDOOM if we have it disabled, or we're running
  // in screensaver or control test mode. Only show it once the
  // game has actually started.

  if (!show_endoom || !main_loop_started
      || g_i_video_globals->screensaver_mode || M_CheckParm("-testcontrols") > 0) {
    return;
  }

  uint8_t * endoom = cache_lump_name<uint8_t *>(DEH_String("ENDOOM"), PU_STATIC);

  I_Endoom(endoom);
}

// Load dehacked patches needed for certain IWADs.
static void LoadIwadDeh() {
  // The Freedoom IWADs have DEHACKED lumps that must be loaded.
  if (g_doomstat_globals->gamevariant == freedoom || g_doomstat_globals->gamevariant == freedm) {
    // Old versions of Freedoom (before 2014-09) did not have technically
    // valid DEHACKED lumps, so ignore errors and just continue if this
    // is an old IWAD.
    DEH_LoadLumpByName("DEHACKED", false, true);
  }

  // If this is the HACX IWAD, we need to load the DEHACKED lump.
  if (g_doomstat_globals->gameversion == exe_hacx) {
    if (!DEH_LoadLumpByName("DEHACKED", true, false)) {
      I_Error("DEHACKED lump not found.  Please check that this is the "
              "Hacx v1.2 IWAD.");
    }
  }

  // Chex Quest needs a separate Dehacked patch which must be downloaded
  // and installed next to the IWAD.
  if (g_doomstat_globals->gameversion == exe_chex) {
    // Look for chex.deh in the same directory as the IWAD file.
    char * dirname  = M_DirName(iwadfile);
    char * chex_deh = M_StringJoin(dirname, DIR_SEPARATOR_S, "chex.deh");
    free(dirname);

    // If the dehacked patch isn't found, try searching the WAD
    // search path instead.  We might find it...
    if (!M_FileExists(chex_deh)) {
      free(chex_deh);
      chex_deh = D_FindWADByName("chex.deh");
    }

    // Still not found?
    if (chex_deh == nullptr) {
      I_Error("Unable to find Chex Quest dehacked file (chex.deh).\n"
              "The dehacked file is required in order to emulate\n"
              "chex.exe correctly.  It can be found in your nearest\n"
              "/idgames repository mirror at:\n\n"
              "   utils/exe_edit/patches/chexdeh.zip");
    }

    if (!DEH_LoadFile(chex_deh)) {
      I_Error("Failed to load chex.deh needed for emulating chex.exe.");
    }
  }
}

// [crispy] support loading SIGIL.WAD (and SIGIL_SHREDS.WAD) alongside DOOM.WAD
static void LoadSigilWad() {
  static constexpr struct {
    const char * name;
    const char   new_name[9];
  } sigil_lumps[] = {
    {"CREDIT",    "SIGCREDI"},
    { "HELP1",    "SIGHELP1"},
    { "TITLEPIC", "SIGTITLE"},
    { "DEHACKED", "SIG_DEH" },
    { "DEMO1",    "SIGDEMO1"},
    { "DEMO2",    "SIGDEMO2"},
    { "DEMO3",    "SIGDEMO3"},
    { "DEMO4",    "SIGDEMO4"},
    { "D_INTER",  "D_SIGINT"},
    { "D_INTRO",  "D_SIGTIT"},
  };

  const char * const texture_files[] = {
    "PNAMES",
    "TEXTURE1",
    "TEXTURE2",
  };

  // [crispy] don't load SIGIL.wad if another PWAD already provides E5M1
  auto e5m1 = W_CheckNumForName("E5M1");
  if (e5m1 != -1) {
    return;
  }

  // [crispy] don't load SIGIL.wad if SIGIL_COMPAT.wad is already loaded
  auto e3m1 = W_CheckNumForName("E3M1");
  if (e3m1 != -1 && !strncasecmp(W_WadNameForLump(lumpinfo[e3m1]), "SIGIL_COMPAT", 12)) {
    return;
  }

  // [crispy] don't load SIGIL.wad if another PWAD already modifies the texture files
  for (auto texture_file : texture_files) {
    int j = W_CheckNumForName(texture_file);

    if (j != -1 && !W_IsIWADLump(lumpinfo[j])) {
      return;
    }
  }

  if (g_doomstat_globals->gameversion == exe_ultimate) {
    const char * const sigil_wads[] = {
      "SIGIL_v1_21.wad",
      "SIGIL_v1_2.wad",
      "SIGIL.wad"
    };
    char * sigil_wad    = nullptr;
    char * dirname      = M_DirName(iwadfile);
    char * sigil_shreds = M_StringJoin(dirname, DIR_SEPARATOR_S, "SIGIL_SHREDS.wad");

    // [crispy] load SIGIL.WAD
    for (auto sigil : sigil_wads) {
      sigil_wad = M_StringJoin(dirname, DIR_SEPARATOR_S, sigil);

      if (M_FileExists(sigil_wad)) {
        break;
      }

      free(sigil_wad);
      sigil_wad = D_FindWADByName(sigil);

      if (sigil_wad) {
        break;
      }
    }
    free(dirname);

    if (sigil_wad == nullptr) {
      free(sigil_shreds);
      return;
    }

    fmt::printf(" [expansion]");
    D_AddFile(sigil_wad);
    free(sigil_wad);

    // [crispy] load SIGIL_SHREDS.WAD
    if (!M_FileExists(sigil_shreds)) {
      free(sigil_shreds);
      sigil_shreds = D_FindWADByName("SIGIL_SHREDS.wad");
    }

    if (sigil_shreds != nullptr) {
      fmt::printf(" [expansion]");
      D_AddFile(sigil_shreds);
      free(sigil_shreds);
    }

    // [crispy] rename intrusive SIGIL_SHREDS.wad music lumps out of the way
    for (const auto & sigil_lump : sigil_lumps) {
      // [crispy] skip non-music lumps
      if (strncasecmp(sigil_lump.name, "D_", 2) != 0) {
        continue;
      }

      int j = W_CheckNumForName(sigil_lump.name);

      if (j != -1 && !strncasecmp(W_WadNameForLump(lumpinfo[j]), "SIGIL_SHREDS", 12)) {
        std::memcpy(lumpinfo[j]->name, sigil_lump.new_name, 8);
      }
    }

    // [crispy] rename intrusive SIGIL.wad graphics, demos and music lumps out of the way
    for (const auto & sigil_lump : sigil_lumps) {
      int j = W_CheckNumForName(sigil_lump.name);

      if (j != -1 && !strncasecmp(W_WadNameForLump(lumpinfo[j]), "SIGIL", 5)) {
        std::memcpy(lumpinfo[j]->name, sigil_lump.new_name, 8);
      }
    }

    // [crispy] regenerate the hashtable
    W_GenerateHashTable();
  }
}

// [crispy] support loading NERVE.WAD alongside DOOM2.WAD
static void LoadNerveWad() {
  int num = 0, j = 0, k = 0;

  if (g_doomstat_globals->gamemission != doom2)
    return;

  if ((num = W_GetNumForName("map01")) != -1 && (j = W_GetNumForName("map09")) != -1 && iequals(W_WadNameForLump(lumpinfo[num]), "nerve.wad") && iequals(W_WadNameForLump(lumpinfo[j]), "nerve.wad")) {
    g_doomstat_globals->gamemission = pack_nerve;
    DEH_AddStringReplacement("TITLEPIC", "INTERPIC");
  } else
    // [crispy] The "New Game -> Which Expansion" menu is only shown if the
    // menu graphics lumps are available and (a) if they are from the IWAD
    // and that is the BFG Edition DOOM2.WAD or (b) if they are from a PWAD.
    if ((num = W_CheckNumForName("M_EPI1")) != -1 && (j = W_CheckNumForName("M_EPI2")) != -1 && (k = W_CheckNumForName("M_EPISOD")) != -1 && (g_doomstat_globals->gamevariant == bfgedition || (!W_IsIWADLump(lumpinfo[num]) && !W_IsIWADLump(lumpinfo[j]) && !W_IsIWADLump(lumpinfo[k])))) {
      if (strrchr(iwadfile, DIR_SEPARATOR) != nullptr) {
        char * dir                       = M_DirName(iwadfile);
        g_doomstat_globals->nervewadfile = M_StringJoin(dir, DIR_SEPARATOR_S, "nerve.wad");
        free(dir);
      } else {
        g_doomstat_globals->nervewadfile = M_StringDuplicate("nerve.wad");
      }

      if (!M_FileExists(g_doomstat_globals->nervewadfile)) {
        free(g_doomstat_globals->nervewadfile);
        g_doomstat_globals->nervewadfile = D_FindWADByName("nerve.wad");
      }

      if (g_doomstat_globals->nervewadfile == nullptr) {
        return;
      }

      fmt::printf(" [expansion]");
      D_AddFile(g_doomstat_globals->nervewadfile);

      // [crispy] rename level name patch lumps out of the way
      for (int i = 0; i < 9; i++) {
        char lumpname[9];

        M_snprintf(lumpname, 9, "CWILV%2.2d", i);
        lumpinfo[W_GetNumForName(lumpname)]->name[0] = 'N';
      }

      // [crispy] regenerate the hashtable
      W_GenerateHashTable();
    }
}

// [crispy] support loading MASTERLEVELS.WAD alongside DOOM2.WAD
static void LoadMasterlevelsWad() {
  int i = 0, j = 0;

  if (g_doomstat_globals->gamemission != doom2)
    return;

  if ((i = W_GetNumForName("map01")) != -1 && (j = W_GetNumForName("map21")) != -1 && iequals(W_WadNameForLump(lumpinfo[i]), "masterlevels.wad") && iequals(W_WadNameForLump(lumpinfo[j]), "masterlevels.wad")) {
    g_doomstat_globals->gamemission = pack_master;
  }
}

static void G_CheckDemoStatusAtExit() {
  G_CheckDemoStatus();
}

//
// D_DoomMain
//
void D_DoomMain() {
  char file[256];
  char demolumpname[9];

  I_AtExit(D_Endoom, false);

  // print banner

  I_PrintBanner(PACKAGE_STRING);

  fmt::printf("Z_Init: Init zone memory allocation daemon. \n");
  Z_Init();

  //!
  // @category net
  //
  // Start a dedicated server, routing packets but not participating
  // in the game itself.
  //

  if (M_CheckParm("-dedicated") > 0) {
    fmt::printf("Dedicated server mode.\n");
    NET_DedicatedServer();

    // Never returns
  }

  //!
  // @category net
  //
  // Query the Internet master server for a global list of active
  // servers.
  //

  if (M_CheckParm("-search")) {
    NET_MasterQuery();
    exit(0);
  }

  //!
  // @arg <address>
  // @category net
  //
  // Query the status of the server running on the given IP
  // address.
  //

  int p = M_CheckParmWithArgs("-query", 1);

  if (p) {
    NET_QueryAddress(myargv[p + 1]);
    exit(0);
  }

  //!
  // @category net
  //
  // Search the local LAN for running servers.
  //

  if (M_CheckParm("-localsearch")) {
    NET_LANQuery();
    exit(0);
  }

  //!
  // @category game
  // @vanilla
  //
  // Disable monsters.
  //

  g_doomstat_globals->nomonsters = M_CheckParm("-nomonsters");

  //!
  // @category game
  // @vanilla
  //
  // Monsters respawn after being killed.
  //

  g_doomstat_globals->respawnparm = M_CheckParm("-respawn");

  //!
  // @category game
  // @vanilla
  //
  // Monsters move faster.
  //

  g_doomstat_globals->fastparm = M_CheckParm("-fast");

  //!
  // @vanilla
  //
  // Developer mode. F1 saves a screenshot in the current working
  // directory.
  //

  g_doomstat_globals->devparm = M_CheckParm("-devparm");

  I_DisplayFPSDots(g_doomstat_globals->devparm);

  //!
  // @category net
  // @vanilla
  //
  // Start a deathmatch game.
  //

  if (M_CheckParm("-deathmatch"))
    g_doomstat_globals->deathmatch = 1;

  //!
  // @category net
  // @vanilla
  //
  // Start a deathmatch 2.0 game.  Weapons do not stay in place and
  // all items respawn after 30 seconds.
  //

  if (M_CheckParm("-altdeath"))
    g_doomstat_globals->deathmatch = 2;

  //!
  // @category net
  // @vanilla
  //
  // Start a deathmatch 3.0 game.  Weapons stay in place and
  // all items respawn after 30 seconds.
  //

  if (M_CheckParm("-dm3"))
    g_doomstat_globals->deathmatch = 3;

  if (g_doomstat_globals->devparm)
    fmt::printf(D_DEVSTR);

    // find which dir to use for config files

#ifdef _WIN32

  //!
  // @category obscure
  // @platform windows
  // @vanilla
  //
  // Save configuration data and savegames in c:\doomdata,
  // allowing play from CD.
  //

  if (M_ParmExists("-cdrom")) {
    fmt::printf(D_CDROM);

    M_SetConfigDir("c:\\doomdata\\");
  } else
#endif
  {
    // Auto-detect the configuration dir.

    M_SetConfigDir(nullptr);
  }

  //!
  // @category game
  // @arg <x>
  // @vanilla
  //
  // Turbo mode.  The player's speed is multiplied by x%.  If unspecified,
  // x defaults to 200.  Values are rounded up to 10 and down to 400.
  //

  int is_set = p = M_CheckParm("-turbo");
  if (is_set) {
    int        scale = 200;
    extern int forwardmove[2];
    extern int sidemove[2];

    if (p < myargc - 1)
      scale = std::atoi(myargv[p + 1]);
    if (scale < 10)
      scale = 10;
    if (scale > 400)
      scale = 400;
    fmt::printf("turbo scale: %i%%\n", scale);
    forwardmove[0] = forwardmove[0] * scale / 100;
    forwardmove[1] = forwardmove[1] * scale / 100;
    sidemove[0]    = sidemove[0] * scale / 100;
    sidemove[1]    = sidemove[1] * scale / 100;
  }

  // init subsystems
  fmt::printf("V_Init: allocate screens.\n");
  V_Init();

  // Load configuration files before initialising other subsystems.
  fmt::printf("M_LoadDefaults: Load system defaults.\n");
  M_SetConfigFilenames("default.cfg", PROGRAM_PREFIX "doom.cfg");
  D_BindVariables();
  M_LoadDefaults();

  // Save configuration at exit.
  I_AtExit(M_SaveDefaults, true); // [crispy] always save configuration at exit

  // Find main IWAD file and load it.
  iwadfile = D_FindIWAD(IWAD_MASK_DOOM, &g_doomstat_globals->gamemission);

  // None found?

  if (iwadfile == nullptr) {
    I_Error("Game mode indeterminate.  No IWAD file was found.  Try\n"
            "specifying one with the '-iwad' command line parameter.\n");
  }

  g_doomstat_globals->modifiedgame = false;

  fmt::printf("W_Init: Init WADfiles.\n");
  D_AddFile(iwadfile);
  size_t numiwadlumps = numlumps;

  W_CheckCorrectIWAD(doom);

  // Now that we've loaded the IWAD, we can figure out what gamemission
  // we're playing and which version of Vanilla Doom we need to emulate.
  D_IdentifyVersion();
  InitGameVersion();

  // Check which IWAD variant we are using.

  if (W_CheckNumForName("FREEDOOM") >= 0) {
    if (W_CheckNumForName("FREEDM") >= 0) {
      g_doomstat_globals->gamevariant = freedm;
    } else {
      g_doomstat_globals->gamevariant = freedoom;
    }
  } else if (W_CheckNumForName("DMENUPIC") >= 0) {
    g_doomstat_globals->gamevariant = bfgedition;
  }

  //!
  // @category mod
  //
  // Disable automatic loading of Dehacked patches for certain
  // IWAD files.
  //
  if (!M_ParmExists("-nodeh")) {
    // Some IWADs have dehacked patches that need to be loaded for
    // them to be played properly.
    LoadIwadDeh();
  }

  // Doom 3: BFG Edition includes modified versions of the classic
  // IWADs which can be identified by an additional DMENUPIC lump.
  // Furthermore, the M_GDHIGH lumps have been modified in a way that
  // makes them incompatible to Vanilla Doom and the modified version
  // of doom2.wad is missing the TITLEPIC lump.
  // We specifically check for DMENUPIC here, before PWADs have been
  // loaded which could probably include a lump of that name.

  if (g_doomstat_globals->gamevariant == bfgedition) {
    fmt::printf("BFG Edition: Using workarounds as needed.\n");

    // BFG Edition changes the names of the secret levels to
    // censor the Wolfenstein references. It also has an extra
    // secret level (MAP33). In Vanilla Doom (meaning the DOS
    // version), MAP33 overflows into the Plutonia level names
    // array, so HUSTR_33 is actually PHUSTR_1.
    DEH_AddStringReplacement(HUSTR_31, "level 31: idkfa");
    DEH_AddStringReplacement(HUSTR_32, "level 32: keen");
    DEH_AddStringReplacement(PHUSTR_1, "level 33: betray");

    // The BFG edition doesn't have the "low detail" menu option (fair
    // enough). But bizarrely, it reuses the M_GDHIGH patch as a label
    // for the options menu (says "Fullscreen:"). Why the perpetrators
    // couldn't just add a new graphic lump and had to reuse this one,
    // I don't know.
    //
    // The end result is that M_GDHIGH is too wide and causes the game
    // to crash. As a workaround to get a minimum level of support for
    // the BFG edition IWADs, use the "ON"/"OFF" graphics instead.
    DEH_AddStringReplacement("M_GDHIGH", "M_MSGON");
    DEH_AddStringReplacement("M_GDLOW", "M_MSGOFF");

    // The BFG edition's "Screen Size:" graphic has also been changed
    // to say "Gamepad:". Fortunately, it (along with the original
    // Doom IWADs) has an unused graphic that says "Display". So we
    // can swap this in instead, and it kind of makes sense.
    DEH_AddStringReplacement("M_SCRNSZ", "M_DISP");
  }

  //!
  // @category mod
  //
  // Disable auto-loading of .wad and .deh files.
  //
  if (!M_ParmExists("-noautoload") && g_doomstat_globals->gamemode != shareware) {
    char * autoload_dir = nullptr;

    // common auto-loaded files for all Doom flavors

    if (g_doomstat_globals->gamemission < pack_chex) {
      autoload_dir = M_GetAutoloadDir("doom-all");
      DEH_AutoLoadPatches(autoload_dir);
      W_AutoLoadWADs(autoload_dir);
      free(autoload_dir);
    }

    // auto-loaded files per IWAD

    autoload_dir = M_GetAutoloadDir(D_SaveGameIWADName(g_doomstat_globals->gamemission));
    DEH_AutoLoadPatches(autoload_dir);
    W_AutoLoadWADs(autoload_dir);
    free(autoload_dir);
  }

  // Load Dehacked patches specified on the command line with -deh.
  // Note that there's a very careful and deliberate ordering to how
  // Dehacked patches are loaded. The order we use is:
  //  1. IWAD dehacked patches.
  //  2. Command line dehacked patches specified with -deh.
  //  3. PWAD dehacked patches in DEHACKED lumps.
  DEH_ParseCommandLine();

  // Load PWAD files.
  g_doomstat_globals->modifiedgame = W_ParseCommandLine();

  //!
  // @arg <file>
  // @category mod
  //
  // [crispy] experimental feature: in conjunction with -merge <files>
  // merges PWADs into the main IWAD and writes the merged data into <file>
  //

  p = M_CheckParm("-mergedump");

  if (p) {
    p = M_CheckParmWithArgs("-mergedump", 1);

    if (p) {
      if (M_StringEndsWith(myargv[p + 1], ".wad")) {
        M_StringCopy(file, myargv[p + 1], sizeof(file));
      } else {
        DEH_snprintf(file, sizeof(file), "%s.wad", myargv[p + 1]);
      }

      int merged = W_MergeDump(file);
      I_Error("W_MergeDump: Merged %d lumps into file '%s'.", merged, file);
    } else {
      I_Error("W_MergeDump: The '-mergedump' parameter requires an argument.");
    }
  }

  //!
  // @arg <file>
  // @category mod
  //
  // [crispy] experimental feature: dump lump data into a new LMP file <file>
  //

  p = M_CheckParm("-lumpdump");

  if (p) {
    p = M_CheckParmWithArgs("-lumpdump", 1);

    if (p) {
      M_StringCopy(file, myargv[p + 1], sizeof(file));

      int dumped = W_LumpDump(file);

      if (dumped < 0) {
        I_Error("W_LumpDump: Failed to write lump '%s'.", file);
      } else {
        I_Error("W_LumpDump: Dumped lump into file '%s.lmp'.", file);
      }
    } else {
      I_Error("W_LumpDump: The '-lumpdump' parameter requires an argument.");
    }
  }

  // Debug:
  //    W_PrintDirectory();

  //!
  // @arg <demo>
  // @category demo
  // @vanilla
  //
  // Play back the demo named demo.lmp.
  //

  p = M_CheckParmWithArgs("-playdemo", 1);

  if (!p) {
    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp, determining the framerate
    // of the screen.
    //
    p = M_CheckParmWithArgs("-timedemo", 1);
  }

  if (!p) {
    // used for cicd integration/regression testing
    p = M_CheckParmWithArgs("-cicddemo", 1);
  }

  if (p) {
    char * uc_filename = strdup(myargv[p + 1]);
    M_ForceUppercase(uc_filename);

    // With Vanilla you have to specify the file without extension,
    // but make that optional.
    if (M_StringEndsWith(uc_filename, ".LMP")) {
      M_StringCopy(file, myargv[p + 1], sizeof(file));
    } else {
      DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p + 1]);
    }

    free(uc_filename);

    if (D_AddFile(file)) {
      M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name, sizeof(demolumpname));
    } else {
      // If file failed to load, still continue trying to play
      // the demo in the same way as Vanilla Doom.  This makes
      // tricks like "-playdemo demo1" possible.

      M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
    }

    fmt::printf("Playing demo %s.\n", file);
  }

  I_AtExit(G_CheckDemoStatusAtExit, true);

  // Generate the WAD hash table.  Speed things up a bit.
  W_GenerateHashTable();

  // [crispy] allow overriding of special-casing
  if (!M_ParmExists("-noautoload") && g_doomstat_globals->gamemode != shareware) {
    LoadMasterlevelsWad();
    LoadNerveWad();
    LoadSigilWad();
  }

  // Load DEHACKED lumps from WAD files - but only if we give the right
  // command line parameter.

  //!
  // @category mod
  //
  // Load Dehacked patches from DEHACKED lumps contained in one of the
  // loaded PWAD files.
  //
  // [crispy] load DEHACKED lumps by default, but allow overriding
  if (!M_ParmExists("-nodehlump") && !M_ParmExists("-nodeh")) {
    int loaded = 0;

    for (size_t i = numiwadlumps; i < numlumps; ++i) {
      if (!strncmp(lumpinfo[i]->name, "DEHACKED", 8)) {
        DEH_LoadLump(static_cast<int>(i), true, true); // [crispy] allow long, allow error
        loaded++;
      }
    }

    fmt::printf("  loaded %i DEHACKED lumps from PWAD files.\n", loaded);
  }

  // Set the gamedescription string. This is only possible now that
  // we've finished loading Dehacked patches.
  D_SetGameDescription();

  GameMission_t gamemission       = g_doomstat_globals->gamemission;
  const char *  iwadname          = D_SaveGameIWADName(gamemission);
  g_doomstat_globals->savegamedir = M_GetSaveGameDir(iwadname);

  // Check for -file in shareware
  if (g_doomstat_globals->modifiedgame && (g_doomstat_globals->gamevariant != freedoom)) {
    // These are the lumps that will be checked in IWAD,
    // if any one is not present, execution will be aborted.
    char name[23][9] = {
      "e2m1",
      "e2m2",
      "e2m3",
      "e2m4",
      "e2m5",
      "e2m6",
      "e2m7",
      "e2m8",
      "e2m9",
      "e3m1",
      "e3m3",
      "e3m3",
      "e3m4",
      "e3m5",
      "e3m6",
      "e3m7",
      "e3m8",
      "e3m9",
      "dphoof",
      "bfgga0",
      "heada1",
      "cybra1",
      "spida1d1"
    };

    if (g_doomstat_globals->gamemode == shareware)
      I_Error("%s", DEH_String("\nYou cannot -file with the shareware "
                         "version. Register!"));

    // Check for fake IWAD with right name,
    // but w/o all the lumps of the registered version.
    if (g_doomstat_globals->gamemode == registered)
      for (int i = 0; i < 23; i++)
        if (W_CheckNumForName(name[i]) < 0)
          I_Error("%s", DEH_String("\nThis is not the registered version."));
  }

// [crispy] disable meaningless warning, we always use "-merge" anyway
#if 0
    if (W_CheckNumForName("SS_START") >= 0
     || W_CheckNumForName("FF_END") >= 0)
    {
        I_PrintDivider();
       fmt::printf(" WARNING: The loaded WAD file contains modified sprites or\n"
               " floor textures.  You may want to use the '-merge' command\n"
               " line option instead of '-file'.\n");
    }
#endif

  I_PrintStartupBanner(g_doomstat_globals->gamedescription);
  PrintDehackedBanners();

  fmt::printf("I_Init: Setting up machine state.\n");
  I_CheckIsScreensaver();
  I_InitTimer();
  I_InitJoystick();
  I_InitSound(true);
  I_InitMusic();

  // [crispy] check for SSG resources
  crispy->havessg =
      (g_doomstat_globals->gamemode == commercial || (W_CheckNumForName("sht2a0") != -1 &&         // [crispy] wielding/firing sprite sequence
                                                      I_GetSfxLumpNum(&S_sfx[sfx_dshtgn]) != -1 && // [crispy] firing sound
                                                      I_GetSfxLumpNum(&S_sfx[sfx_dbopn]) != -1 &&  // [crispy] opening sound
                                                      I_GetSfxLumpNum(&S_sfx[sfx_dbload]) != -1 && // [crispy] reloading sound
                                                      I_GetSfxLumpNum(&S_sfx[sfx_dbcls]) != -1     // [crispy] closing sound
                                                      ));

  // [crispy] check for presence of a 5th episode
  crispy->haved1e5 = (g_doomstat_globals->gameversion == exe_ultimate) && (W_CheckNumForName("m_epi5") != -1) && (W_CheckNumForName("e5m1") != -1) && (W_CheckNumForName("wilv40") != -1);

  // [crispy] check for presence of E1M10
  crispy->havee1m10 = (g_doomstat_globals->gamemode == retail) && (W_CheckNumForName("e1m10") != -1) && (W_CheckNumForName("sewers") != -1);

  // [crispy] check for presence of MAP33
  crispy->havemap33 = (g_doomstat_globals->gamemode == commercial) && (W_CheckNumForName("map33") != -1) && (W_CheckNumForName("cwilv32") != -1);

  // [crispy] change level name for MAP33 if not already changed
  if (crispy->havemap33 && !DEH_HasStringReplacement(PHUSTR_1)) {
    DEH_AddStringReplacement(PHUSTR_1, "level 33: betray");
  }

  fmt::printf("NET_Init: Init network subsystem.\n");
  NET_Init();

  // Initial netgame startup. Connect to server etc.
  D_ConnectNetGame();

  // get skill / episode / map from parms
  g_doomstat_globals->startskill   = sk_medium;
  g_doomstat_globals->startepisode = 1;
  g_doomstat_globals->startmap     = 1;
  g_doomstat_globals->autostart    = false;

  //!
  // @category game
  // @arg <skill>
  // @vanilla
  //
  // Set the game skill, 1-5 (1: easiest, 5: hardest).  A skill of
  // 0 disables all monsters.
  //

  p = M_CheckParmWithArgs("-skill", 1);

  if (p) {
    // todo does this need error handling?
    g_doomstat_globals->startskill = static_cast<skill_t>(myargv[p + 1][0] - '1');
    g_doomstat_globals->autostart  = true;
  }

  //!
  // @category game
  // @arg <n>
  // @vanilla
  //
  // Start playing on episode n (1-4)
  //

  p = M_CheckParmWithArgs("-episode", 1);

  if (p) {
    g_doomstat_globals->startepisode = myargv[p + 1][0] - '0';
    g_doomstat_globals->startmap     = 1;
    g_doomstat_globals->autostart    = true;
  }

  g_doomstat_globals->timelimit = 0;

  //!
  // @arg <n>
  // @category net
  // @vanilla
  //
  // For multiplayer games: exit each level after n minutes.
  //

  p = M_CheckParmWithArgs("-timer", 1);

  if (p) {
    g_doomstat_globals->timelimit = std::atoi(myargv[p + 1]);
  }

  //!
  // @category net
  // @vanilla
  //
  // Austin Virtual Gaming: end levels after 20 minutes.
  //

  p = M_CheckParm("-avg");

  if (p) {
    g_doomstat_globals->timelimit = 20;
  }

  //!
  // @category game
  // @arg [<x> <y> | <xy>]
  // @vanilla
  //
  // Start a game immediately, warping to ExMy (Doom 1) or MAPxy
  // (Doom 2)
  //

  p = M_CheckParmWithArgs("-warp", 1);

  if (p) {
    if (g_doomstat_globals->gamemode == commercial)
      g_doomstat_globals->startmap = std::atoi(myargv[p + 1]);
    else {
      g_doomstat_globals->startepisode = myargv[p + 1][0] - '0';

      // [crispy] only if second argument is not another option
      if (p + 2 < myargc && myargv[p + 2][0] != '-') {
        g_doomstat_globals->startmap = myargv[p + 2][0] - '0';
      } else {
        // [crispy] allow second digit without space in between for Doom 1
        g_doomstat_globals->startmap = myargv[p + 1][1] - '0';
      }
    }
    g_doomstat_globals->autostart = true;
    // [crispy] if used with -playdemo, fast-forward demo up to the desired map
    crispy->demowarp = g_doomstat_globals->startmap;
  }

  // Undocumented:
  // Invoked by setup to test the controls.

  p = M_CheckParm("-testcontrols");

  if (p > 0) {
    g_doomstat_globals->startepisode = 1;
    g_doomstat_globals->startmap     = 1;
    g_doomstat_globals->autostart    = true;
    g_doomstat_globals->testcontrols = true;
  }

  // [crispy] port level flipping feature over from Strawberry Doom
#ifdef ENABLE_APRIL_1ST_JOKE
  {
    time_t      curtime = time(nullptr);
    struct tm * curtm   = localtime(&curtime);

    if (curtm && curtm->tm_mon == 3 && curtm->tm_mday == 1)
      crispy->fliplevels = true;
  }
#endif

  p = M_CheckParm("-fliplevels");

  if (p > 0) {
    crispy->fliplevels  = !crispy->fliplevels;
    crispy->flipweapons = !crispy->flipweapons;
  }

  p = M_CheckParm("-flipweapons");

  if (p > 0) {
    crispy->flipweapons = !crispy->flipweapons;
  }

  // Check for load game parameter
  // We do this here and save the slot number, so that the network code
  // can override it or send the load slot to other players.

  //!
  // @category game
  // @arg <s>
  // @vanilla
  //
  // Load the game in slot s.
  //

  p = M_CheckParmWithArgs("-loadgame", 1);

  if (p) {
    g_doomstat_globals->startloadgame = std::atoi(myargv[p + 1]);
  } else {
    // Not loading a game
    g_doomstat_globals->startloadgame = -1;
  }

  fmt::printf("M_Init: Init miscellaneous info.\n");
  M_Init();

  fmt::printf("R_Init: Init DOOM refresh daemon - ");
  R_Init();

  fmt::printf("\nP_Init: Init Playloop state.\n");
  P_Init();

  fmt::printf("S_Init: Setting up sound.\n");
  S_Init(g_doomstat_globals->sfxVolume * 8, g_doomstat_globals->musicVolume * 8);

  fmt::printf("D_CheckNetGame: Checking network game status.\n");
  D_CheckNetGame();

  PrintGameVersion();

  fmt::printf("HU_Init: Setting up heads up display.\n");
  HU_Init();

  fmt::printf("ST_Init: Init status bar.\n");
  ST_Init();

  // If Doom II without a MAP01 lump, this is a store demo.
  // Moved this here so that MAP01 isn't constantly looked up
  // in the main loop.

  if (g_doomstat_globals->gamemode == commercial && W_CheckNumForName("map01") < 0)
    storedemo = true;

  if (M_CheckParmWithArgs("-statdump", 1)) {
    I_AtExit(StatDump, true);
    fmt::printf("External statistics registered.\n");
  }

  //!
  // @arg <x>
  // @category demo
  // @vanilla
  //
  // Record a demo named x.lmp.
  //

  p = M_CheckParmWithArgs("-record", 1);

  if (p) {
    G_RecordDemo(myargv[p + 1]);
    g_doomstat_globals->autostart = true;
  }

  p = M_CheckParmWithArgs("-playdemo", 1);
  if (p) {
    g_doomstat_globals->singledemo = true; // quit after one demo
    G_DeferedPlayDemo(demolumpname);
    D_DoomLoop(); // never returns
  }

  p = M_CheckParmWithArgs("-cicddemo", 1);
  if (p) {
      g_doomstat_globals->singledemo = true; // quit after one demo
      G_CiCdDemo(demolumpname);
      D_DoomLoop(); // never returns
  }

  crispy->demowarp = 0; // [crispy] we don't play a demo, so don't skip maps

  p = M_CheckParmWithArgs("-timedemo", 1);
  if (p) {
    G_TimeDemo(demolumpname);
    D_DoomLoop(); // never returns
  }

  if (g_doomstat_globals->startloadgame >= 0) {
    M_StringCopy(file, P_SaveGameFile(g_doomstat_globals->startloadgame), sizeof(file));
    G_LoadGame(file);
  }

  if (gameaction != ga_loadgame) {
    if (g_doomstat_globals->autostart || g_doomstat_globals->netgame)
      G_InitNew(g_doomstat_globals->startskill, g_doomstat_globals->startepisode, g_doomstat_globals->startmap);
    else
      D_StartTitle(); // start up intro loop
  }

  D_DoomLoop(); // never returns
}
