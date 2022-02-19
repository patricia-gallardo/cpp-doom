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

#include <cstdio>
#include <cstdlib>

#include "config.h"
#include "textscreen.hpp"

#include "execute.hpp"

#include "m_argv.hpp"
#include "m_config.hpp"
#include "m_controls.hpp"
#include "m_misc.hpp"
#include "z_zone.hpp"

#include "setup_icon.cpp"
#include "mode.hpp"

#include "compatibility.hpp"
#include "display.hpp"
#include "joystick.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "multiplayer.hpp"
#include "sound.hpp"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup"

static const int cheat_sequence[] =
{
    KEY_UPARROW, KEY_UPARROW, KEY_DOWNARROW, KEY_DOWNARROW,
    KEY_LEFTARROW, KEY_RIGHTARROW, KEY_LEFTARROW, KEY_RIGHTARROW,
    'b', 'a', KEY_ENTER, 0
};

static unsigned int cheat_sequence_index = 0;

// I think these are good "sensible" defaults:

static void SensibleDefaults()
{
    key_up = 'w';
    key_down = 's';
    key_strafeleft = 'a';
    key_straferight = 'd';
    key_jump = '/';
    key_lookup = KEY_PGUP;
    key_lookdown = KEY_PGDN;
    key_lookcenter = KEY_HOME;
    key_flyup = KEY_INS;
    key_flydown = KEY_DEL;
    key_flycenter = KEY_END;
    key_prevweapon = ',';
    key_nextweapon = '.';
    key_invleft = '[';
    key_invright = ']';
    key_message_refresh = '\'';
    key_mission = 'i';              // Strife keys
    key_invpop = 'o';
    key_invkey = 'p';
    key_multi_msgplayer[0] = 'g';
    key_multi_msgplayer[1] = 'h';
    key_multi_msgplayer[2] = 'j';
    key_multi_msgplayer[3] = 'k';
    key_multi_msgplayer[4] = 'v';
    key_multi_msgplayer[5] = 'b';
    key_multi_msgplayer[6] = 'n';
    key_multi_msgplayer[7] = 'm';
    mousebprevweapon = 4;           // Scroll wheel = weapon cycle
    mousebnextweapon = 3;
    snd_musicdevice = static_cast<snddevice_t>(3);
    joybspeed = 29;                 // Always run
    vanilla_savegame_limit = 0;
    vanilla_keyboard_mapping = 0;
    vanilla_demo_limit = 0;
    graphical_startup = 0;
    show_endoom = 0;
    dclick_use = 0;
    novert = 1;
    snd_dmxoption = const_cast<char *>("-opl3 -reverse");
    png_screenshots = 1;
}

static int MainMenuKeyPress(txt_window_t *, int key, void *)
{
    if (key == cheat_sequence[cheat_sequence_index])
    {
        ++cheat_sequence_index;

        if (cheat_sequence[cheat_sequence_index] == 0)
        {
            SensibleDefaults();
            cheat_sequence_index = 0;

            TXT_MessageBox(nullptr, "    \x01    ");

            return 1;
        }
    }
    else
    {
        cheat_sequence_index = 0;
    }

    return 0;
}

static void DoQuit(void *, void *dosave)
{
    if (dosave != nullptr)
    {
        M_SaveDefaults();
    }

    TXT_Shutdown();

    exit(0);
}

static void QuitConfirm(void *, void *)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_button_t *yes_button;
    txt_button_t *no_button;

    window = TXT_NewWindow(nullptr);

    TXT_AddWidgets(window, 
                   label = TXT_NewLabel("Exiting setup.\nSave settings?"),
                   TXT_NewStrut(24, 0),
                   yes_button = TXT_NewButton2("  Yes  ", DoQuit, reinterpret_cast<void *>(1)), // !NULL
                   no_button = TXT_NewButton2("  No   ", DoQuit, nullptr),
                   nullptr);

    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(yes_button, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(no_button, TXT_HORIZ_CENTER);

    // Only an "abort" button in the middle.
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, nullptr);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, nullptr);
}

static void LaunchDoom(void *, void *)
{
    execute_context_t *exec;
    
    // Save configuration first

    M_SaveDefaults();

    // Shut down textscreen GUI

    TXT_Shutdown();

    // Launch Doom

    exec = NewExecuteContext();
    PassThroughArguments(exec);
    ExecuteDoom(exec);

    exit(0);
}

static txt_button_t *GetLaunchButton()
{
    const char *label;

    switch (gamemission)
    {
        case doom:
            label = "Save parameters and launch DOOM";
            break;
        case heretic:
            label = "Save parameters and launch Heretic";
            break;
        case hexen:
            label = "Save parameters and launch Hexen";
            break;
        case strife:
            label = "Save parameters and launch STRIFE!";
            break;
        default:
            label = "Save parameters and launch game";
            break;
    }

    return TXT_NewButton2(label, LaunchDoom, nullptr);
}

void MainMenu()
{
    txt_window_t *window;
    txt_window_action_t *quit_action;
    txt_window_action_t *warp_action;

    window = TXT_NewWindow("Main Menu");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
        TXT_NewButton2("Configure Display",
                       ConfigDisplay, nullptr),
        TXT_NewButton2("Configure Sound",
                       ConfigSound, nullptr),
        TXT_NewButton2("Configure Keyboard",
                       ConfigKeyboard, nullptr),
        TXT_NewButton2("Configure Mouse",
                       ConfigMouse, nullptr),
        TXT_NewButton2("Configure Gamepad/Joystick",
                       ConfigJoystick, nullptr),
        TXT_NewButton2(gamemission == doom ? "Crispness" : "Compatibility",
                       CompatibilitySettings, nullptr),
        GetLaunchButton(),
        TXT_NewStrut(0, 1),
        TXT_NewButton2("Start a Network Game",
                       StartMultiGame, nullptr),
        TXT_NewButton2("Join a Network Game",
                       JoinMultiGame, nullptr),
        TXT_NewButton2("Multiplayer Configuration",
                       MultiplayerConfig, nullptr),
        nullptr);

    quit_action = TXT_NewWindowAction(KEY_ESCAPE, "Quit");
    warp_action = TXT_NewWindowAction(KEY_F2, "Warp");
    TXT_SignalConnect(quit_action, "pressed", QuitConfirm, nullptr);
    TXT_SignalConnect(warp_action, "pressed",
                      WarpMenu, nullptr);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, quit_action);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, warp_action);

    TXT_SetKeyListener(window, MainMenuKeyPress, nullptr);
}

//
// Initialize all configuration variables, load config file, etc
//

static void InitConfig()
{
    M_SetConfigDir(nullptr);
    InitBindings();

    SetChatMacroDefaults();
    SetPlayerNameDefault();

    M_LoadDefaults();

    // Create and configure the music pack directory if it does not
    // already exist.
    M_SetMusicPackDir();
}

//
// Application icon
//

static void SetIcon()
{
    extern SDL_Window *TXT_SDLWindow;

    const unsigned int * const_ptr = setup_icon_data;
    auto * ptr = const_cast<unsigned int *>(const_ptr);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(reinterpret_cast<void*>(ptr), setup_icon_w,
                                       setup_icon_h, 32, setup_icon_w * 4,
                static_cast<Uint32>(0xff << 24), 0xff << 16,
                                       0xff << 8, 0xff << 0);
    SDL_SetWindowIcon(TXT_SDLWindow, surface);
    SDL_FreeSurface(surface);
}

static void SetWindowTitle()
{
    char *title;

    title = M_StringReplace(PACKAGE_NAME " Setup ver " PACKAGE_VERSION,
                            "Doom",
                            GetGameTitle());


    TXT_SetDesktopTitle(title);

    free(title);
}

// Initialize the textscreen library.

static void InitTextscreen()
{
    SetDisplayDriver();

    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialize GUI\n");
        exit(-1);
    }

    // Set Romero's "funky blue" color:
    // <https://doomwiki.org/wiki/Romero_Blue>
    TXT_SetColor(TXT_COLOR_BLUE, 0x04, 0x14, 0x40);

    // [crispy] Crispy colors for Crispy Setup
    TXT_SetColor(TXT_COLOR_BRIGHT_GREEN, 249, 227, 0);  // 0xF9, 0xE3, 0x00
    TXT_SetColor(TXT_COLOR_CYAN, 220, 153, 0);          // 0xDC, 0x99, 0x00
    TXT_SetColor(TXT_COLOR_BRIGHT_CYAN, 76, 160, 223);  // 0x4C, 0xA0, 0xDF

    SetIcon();
    SetWindowTitle();
}

// Restart the textscreen library.  Used when the video_driver variable
// is changed.

[[maybe_unused]] void RestartTextscreen()
{
    TXT_Shutdown();
    InitTextscreen();
}

// 
// Initialize and run the textscreen GUI.
//

static void RunGUI()
{
    InitTextscreen();

    TXT_GUIMainLoop();
}

static void MissionSet()
{
    SetWindowTitle();
    InitConfig();
    MainMenu();
}

void D_DoomMain()
{
    SetupMission(MissionSet);

    RunGUI();
}
