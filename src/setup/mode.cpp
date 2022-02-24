//
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

#include <cstdlib>
#include <cstring>

#include "doomtype.hpp"

#include "config.h"
#include "textscreen.hpp"

#include "doomtype.hpp"
#include "d_mode.hpp"
#include "d_iwad.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_controls.hpp"
#include "m_misc.hpp"

#include "compatibility.hpp"
#include "display.hpp"
#include "joystick.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "multiplayer.hpp"
#include "sound.hpp"

#include "mode.hpp"

GameMission_t          gamemission;
static const iwad_t ** iwads;

struct mission_config_t {
  const char *  label;
  GameMission_t mission;
  int           mask;
  const char *  name;
  const char *  config_file;
  const char *  extra_config_file;
  const char *  executable;
};

// Default mission to fall back on, if no IWADs are found at all:

#define DEFAULT_MISSION (&mission_configs[0])

static mission_config_t mission_configs[] = {
  {"Doom",
   doom,
   IWAD_MASK_DOOM,
   "doom",
   "default.cfg",
   PROGRAM_PREFIX "doom.cfg",
   PROGRAM_PREFIX "doom"   },
  { "Heretic",
   heretic,
   IWAD_MASK_HERETIC,
   "heretic",
   "heretic.cfg",
   PROGRAM_PREFIX "heretic.cfg",
   PROGRAM_PREFIX "heretic"},
  { "Hexen",
   hexen,
   IWAD_MASK_HEXEN,
   "hexen",
   "hexen.cfg",
   PROGRAM_PREFIX "hexen.cfg",
   PROGRAM_PREFIX "hexen"  },
  { "Strife",
   strife,
   IWAD_MASK_STRIFE,
   "strife",
   "strife.cfg",
   PROGRAM_PREFIX "strife.cfg",
   PROGRAM_PREFIX "strife" }
};

static GameSelectCallback game_selected_callback;

// Miscellaneous variables that aren't used in setup.

static int          showMessages = 1;
static int          screenblocks = 10;
static int          detailLevel  = 0;
static char *       savedir      = nullptr;
static char *       executable   = nullptr;
static const char * game_title   = "Doom";
static char *       back_flat    = const_cast<char *>("F_PAVE01");
static int          comport      = 0;
static char *       nickname     = nullptr;

static void BindMiscVariables() {
  if (gamemission == doom) {
    M_BindIntVariable("detaillevel", &detailLevel);
    M_BindIntVariable("show_messages", &showMessages);
  }

  if (gamemission == hexen) {
    M_BindStringVariable("savedir", &savedir);
    M_BindIntVariable("messageson", &showMessages);

    // Hexen has a variable to control the savegame directory
    // that is used.

    savedir = M_GetSaveGameDir("hexen.wad");

    // On Windows, hexndata\ is the default.

    if (!strcmp(savedir, "")) {
      free(savedir);
      savedir = const_cast<char *>("hexndata" DIR_SEPARATOR_S);
    }
  }

  if (gamemission == strife) {
    // Strife has a different default value than the other games
    screenblocks = 10;

    M_BindStringVariable("back_flat", &back_flat);
    M_BindStringVariable("nickname", &nickname);

    M_BindIntVariable("screensize", &screenblocks);
    M_BindIntVariable("comport", &comport);
  } else {
    M_BindIntVariable("screenblocks", &screenblocks);
  }
}

//
// Initialise all configuration file bindings.
//

void InitBindings() {
  M_ApplyPlatformDefaults();

  // Keyboard, mouse, joystick controls

  M_BindBaseControls();
  M_BindWeaponControls();
  M_BindMapControls();
  M_BindMenuControls();

  if (gamemission == heretic || gamemission == hexen) {
    M_BindHereticControls();
  }

  if (gamemission == hexen) {
    M_BindHexenControls();
  }

  if (gamemission == strife) {
    M_BindStrifeControls();
  }

  // All other variables

  BindCompatibilityVariables();
  BindDisplayVariables();
  BindJoystickVariables();
  BindKeyboardVariables();
  BindMouseVariables();
  BindSoundVariables();
  BindMiscVariables();
  BindMultiplayerVariables();
}

// Set the name of the executable program to run the game:

static void SetExecutable(mission_config_t * config) {
  char * extension;

  free(executable);

#ifdef _WIN32
  extension = const_cast<char *>(".exe");
#else
  extension = const_cast<char *>("");
#endif

  executable = M_StringJoin(config->executable, extension, nullptr);
}

static void SetMission(mission_config_t * config) {
  iwads       = D_FindAllIWADs(config->mask);
  gamemission = config->mission;
  SetExecutable(config);
  game_title = config->label;
  M_SetConfigFilenames(config->config_file, config->extra_config_file);
}

static mission_config_t * GetMissionForName(char * name) {
  for (auto & mission_config : mission_configs) {
    if (!strcmp(mission_config.name, name)) {
      return &mission_config;
    }
  }

  return nullptr;
}

// Check the name of the executable.  If it contains one of the game
// names (eg. chocolate-hexen-setup.exe) then use that game.

static bool CheckExecutableName(GameSelectCallback callback) {
  const char * exe_name = M_GetExecutableName();

  for (auto & mission_config : mission_configs) {
    mission_config_t * config = &mission_config;

    if (strstr(exe_name, config->name) != nullptr) {
      SetMission(config);
      callback();
      return true;
    }
  }

  return false;
}

static void GameSelected(void *, void * uncast_config) {
  auto * config = reinterpret_cast<mission_config_t *>(uncast_config);

  SetMission(config);
  game_selected_callback();
}

static void OpenGameSelectDialog(GameSelectCallback callback) {
  mission_config_t * mission = nullptr;
  txt_window_t *     window  = TXT_NewWindow("Select game");

  TXT_AddWidget(window, TXT_NewLabel("Select a game to configure:\n"));
  int num_games = 0;

  // Add a button for each game.

  for (auto & mission_config : mission_configs) {
    // Do we have any IWADs for this game installed?
    // If so, add a button.

    const iwad_t ** iwads_local = D_FindAllIWADs(mission_config.mask);

    if (iwads_local[0] != nullptr) {
      mission = &mission_config;
      TXT_AddWidget(window, TXT_NewButton2(mission_config.label, GameSelected, &mission_config));
      ++num_games;
    }

    free(iwads_local);
  }

  TXT_AddWidget(window, TXT_NewStrut(0, 1));

  // No IWADs found at all?  Fall back to doom, then.

  if (num_games == 0) {
    TXT_CloseWindow(window);
    SetMission(DEFAULT_MISSION);
    callback();
    return;
  }

  // Only one game? Use that game, and don't bother with a dialog.

  if (num_games == 1) {
    TXT_CloseWindow(window);
    SetMission(mission);
    callback();
    return;
  }

  game_selected_callback = callback;
}

void SetupMission(GameSelectCallback callback) {
  mission_config_t * config;
  char *             mission_name;
  int                p;

  //!
  // @arg <game>
  //
  // Specify the game to configure the settings for.  Valid
  // values are 'doom', 'heretic', 'hexen' and 'strife'.
  //

  p = M_CheckParm("-game");

  if (p > 0) {
    mission_name = myargv[p + 1];

    config = GetMissionForName(mission_name);

    if (config == nullptr) {
      I_Error("Invalid parameter - '%s'", mission_name);
    }

    SetMission(config);
    callback();
  } else if (!CheckExecutableName(callback)) {
    OpenGameSelectDialog(callback);
  }
}

const char * GetExecutableName() {
  return executable;
}

const char * GetGameTitle() {
  return game_title;
}

const iwad_t ** GetIwads() {
  return iwads;
}
