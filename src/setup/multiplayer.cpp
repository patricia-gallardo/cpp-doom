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
#include <cstring>

#include "doomtype.hpp"

#include "textscreen.hpp"

#include "d_iwad.hpp"
#include "m_config.hpp"
#include "m_misc.hpp"
#include "doom/d_englsh.hpp"
#include "m_controls.hpp"

#include "multiplayer.hpp"
#include "mode.hpp"
#include "execute.hpp"

#include "net_io.hpp"
#include "net_query.hpp"

#include "net_petname.hpp"

#define MULTI_START_HELP_URL "https://www.chocolate-doom.org/setup-multi-start"
#define MULTI_JOIN_HELP_URL "https://www.chocolate-doom.org/setup-multi-join"
#define MULTI_CONFIG_HELP_URL "https://www.chocolate-doom.org/setup-multi-config"
#define LEVEL_WARP_HELP_URL "https://www.chocolate-doom.org/setup-level-warp"

#define NUM_WADS 10
#define NUM_EXTRA_PARAMS 10

enum warptype_t
{
    WARP_ExMy,
    WARP_MAPxy,
};

// Fallback IWADs to use if no IWADs are detected.

static const iwad_t fallback_iwads[] = {
    { "doom.wad",     doom,     registered,  "Doom" },
    { "heretic.wad",  heretic,  retail,      "Heretic" },
    { "hexen.wad",    hexen,    commercial,  "Hexen" },
    { "strife1.wad",  strife,   commercial,  "Strife" },
};

// Array of IWADs found to be installed

static const iwad_t **found_iwads;
static const char **iwad_labels;

// Index of the currently selected IWAD

static int found_iwad_selected = -1;

// Filename to pass to '-iwad'.

static const char *iwadfile;

static const char *wad_extensions[] = { "wad", "lmp", "deh", nullptr };

static const char *doom_skills[] =
{
    "I'm too young to die.", "Hey, not too rough.", "Hurt me plenty.",
    "Ultra-Violence.", "NIGHTMARE!",
};

static const char *chex_skills[] =
{
    "Easy does it", "Not so sticky", "Gobs of goo", "Extreme ooze",
    "SUPER SLIMEY!"
};

static const char *heretic_skills[] =
{
    "Thou needeth a wet-nurse", "Yellowbellies-R-us", "Bringest them oneth",
    "Thou art a smite-meister", "Black plague possesses thee"
};

static const char *hexen_fighter_skills[] =
{
    "Squire", "Knight", "Warrior", "Berserker", "Titan"
};

static const char *hexen_cleric_skills[] =
{
    "Altar boy", "Acolyte", "Priest", "Cardinal", "Pope"
};

static const char *hexen_mage_skills[] =
{
    "Apprentice", "Enchanter", "Sorceror", "Warlock", "Archimage"
};

static const char *strife_skills[] =
{
    "Training", "Rookie", "Veteran", "Elite", "Bloodbath"
};

static const char *character_classes[] = { "Fighter", "Cleric", "Mage" };

static const char *gamemodes[] = { "Co-operative", "Deathmatch", "Deathmatch 2.0", "Deathmatch 3.0" };

static const char *strife_gamemodes[] =
{
    "Normal deathmatch",
    "Items respawn", // (altdeath)
};

static char *net_player_name;
static char *chat_macros[10];

static char *wads[NUM_WADS];
static char *extra_params[NUM_EXTRA_PARAMS];
static int character_class = 0;
static int skill = 2;
static int nomonsters = 0;
static int deathmatch = 0;
static int strife_altdeath = 0;
static int fast = 0;
static int respawn = 0;
static int udpport = 2342;
static int timer = 0;
static int privateserver = 0;

static txt_dropdown_list_t *skillbutton;
static txt_button_t *warpbutton;
static warptype_t warptype = WARP_MAPxy;
static int warpepisode = 1;
static int warpmap = 1;

// Address to connect to when joining a game

static char *connect_address = nullptr;

static txt_window_t *query_window;
static int query_servers_found;

// Find an IWAD from its description

static const iwad_t *GetCurrentIWAD()
{
    return found_iwads[found_iwad_selected];
}

// Is the currently selected IWAD the Chex Quest chex.wad?

static bool IsChexQuest(const iwad_t *iwad)
{
    return !strcmp(iwad->name, "chex.wad");
}

static void AddWADs(execute_context_t *exec)
{
    int have_wads = 0;
    int i;

    for (i=0; i<NUM_WADS; ++i)
    {
        if (wads[i] != nullptr && strlen(wads[i]) > 0)
        {
            if (!have_wads)
            {
                AddCmdLineParameter(exec, "-file");
            }

            AddCmdLineParameter(exec, "\"%s\"", wads[i]);
        }
    }
}

static void AddExtraParameters(execute_context_t *exec)
{
    int i;

    for (i=0; i<NUM_EXTRA_PARAMS; ++i)
    {
        if (extra_params[i] != nullptr && strlen(extra_params[i]) > 0)
        {
            AddCmdLineParameter(exec, "%s", extra_params[i]);
        }
    }
}

static void AddIWADParameter(execute_context_t *exec)
{
    if (iwadfile != nullptr)
    {
        AddCmdLineParameter(exec, "-iwad %s", iwadfile);
    }
}

// Callback function invoked to launch the game.
// This is used when starting a server and also when starting a
// single player game via the "warp" menu.

static void StartGame(int multiplayer)
{
    execute_context_t *exec;

    exec = NewExecuteContext();

    // Extra parameters come first, before all others; this way,
    // they can override any of the options set in the dialog.

    AddExtraParameters(exec);

    AddIWADParameter(exec);
    AddCmdLineParameter(exec, "-skill %i", skill + 1);

    if (gamemission == hexen)
    {
        AddCmdLineParameter(exec, "-class %i", character_class);
    }

    if (nomonsters)
    {
        AddCmdLineParameter(exec, "-nomonsters");
    }

    if (fast)
    {
        AddCmdLineParameter(exec, "-fast");
    }

    if (respawn)
    {
        AddCmdLineParameter(exec, "-respawn");
    }

    if (warptype == WARP_ExMy)
    {
        // TODO: select IWAD based on warp type
        AddCmdLineParameter(exec, "-warp %i %i", warpepisode, warpmap);
    }
    else if (warptype == WARP_MAPxy)
    {
        AddCmdLineParameter(exec, "-warp %i", warpmap);
    }

    // Multiplayer-specific options:

    if (multiplayer)
    {
        AddCmdLineParameter(exec, "-server");
        AddCmdLineParameter(exec, "-port %i", udpport);

        if (deathmatch == 1)
        {
            AddCmdLineParameter(exec, "-deathmatch");
        }
        else if (deathmatch == 2 || strife_altdeath != 0)
        {
            AddCmdLineParameter(exec, "-altdeath");
        }
        else if (deathmatch == 3) // AX: this is a Crispy-specific change
        {
            AddCmdLineParameter(exec, "-dm3");
        }

        if (timer > 0)
        {
            AddCmdLineParameter(exec, "-timer %i", timer);
        }

        if (privateserver)
        {
            AddCmdLineParameter(exec, "-privateserver");
        }
    }

    AddWADs(exec);

    TXT_Shutdown();

    M_SaveDefaults();
    PassThroughArguments(exec);

    ExecuteDoom(exec);

    exit(0);
}

static void StartServerGame(void *, void *)
{
    StartGame(1);
}

static void StartSinglePlayerGame(void *, void *)
{
    StartGame(0);
}

static void UpdateWarpButton()
{
    char buf[10];

    if (warptype == WARP_ExMy)
    {
        M_snprintf(buf, sizeof(buf), "E%iM%i", warpepisode, warpmap);
    }
    else if (warptype == WARP_MAPxy)
    {
        M_snprintf(buf, sizeof(buf), "MAP%02i", warpmap);
    }

    TXT_SetButtonLabel(warpbutton, buf);
}

static void UpdateSkillButton()
{
    const iwad_t *iwad = GetCurrentIWAD();

    if (IsChexQuest(iwad))
    {
        skillbutton->values = chex_skills;
    }
    else switch(gamemission)
    {
        default:
        case doom:
            skillbutton->values = doom_skills;
            break;

        case heretic:
            skillbutton->values = heretic_skills;
            break;

        case hexen:
            if (character_class == 0)
            {
                skillbutton->values = hexen_fighter_skills;
            }
            else if (character_class == 1)
            {
                skillbutton->values = hexen_cleric_skills;
            }
            else
            {
                skillbutton->values = hexen_mage_skills;
            }
            break;

        case strife:
            skillbutton->values = strife_skills;
            break;
    }
}

static void SetExMyWarp(void *, void *val)
{
    int l = static_cast<int>(reinterpret_cast<intptr_t>(val));

    warpepisode = l / 10;
    warpmap = l % 10;

    UpdateWarpButton();
}

static void SetMAPxyWarp(void *, void *val)
{
    int l = static_cast<int>(reinterpret_cast<intptr_t>(val));

    warpmap = l;

    UpdateWarpButton();
}

static void CloseLevelSelectDialog(void *, void *uncast_window)
{
    auto *window = reinterpret_cast<txt_window_t *>(uncast_window);

    TXT_CloseWindow(window);
}

static void LevelSelectDialog(void *, void *)
{
    txt_window_t *window;
    txt_button_t *button;
    const iwad_t *iwad;
    char buf[10];
    int episodes;
    intptr_t x, y;
    intptr_t l;
    int i;

    window = TXT_NewWindow("Select level");
    iwad = GetCurrentIWAD();

    if (warptype == WARP_ExMy)
    {
        episodes = D_GetNumEpisodes(iwad->mission, iwad->mode);
        TXT_SetTableColumns(window, episodes);

        // ExMy levels

        for (y=1; y<10; ++y)
        {
            for (x=1; x<=episodes; ++x)
            {
                if (IsChexQuest(iwad) && (x > 1 || y > 5))
                {
                    continue;
                }

                if (!D_ValidEpisodeMap(iwad->mission, iwad->mode, static_cast<int>(x), static_cast<int>(y)))
                {
                    TXT_AddWidget(window, nullptr);
                    continue;
                }

                M_snprintf(buf, sizeof(buf),
                           " E%" PRIiPTR "M%" PRIiPTR " ", x, y);
                button = TXT_NewButton(buf);
                TXT_SignalConnect(button, "pressed",
                                  SetExMyWarp, reinterpret_cast<void *>(x * 10 + y));
                TXT_SignalConnect(button, "pressed",
                                  CloseLevelSelectDialog, window);
                TXT_AddWidget(window, button);

                if (warpepisode == x && warpmap == y)
                {
                    TXT_SelectWidget(window, button);
                }
            }
        }
    }
    else
    {
        TXT_SetTableColumns(window, 6);

        for (i=0; i<60; ++i)
        {
            x = i % 6;
            y = i / 6;

            l = x * 10 + y + 1;

            if (!D_ValidEpisodeMap(iwad->mission, iwad->mode, 1, static_cast<int>(l)))
            {
                TXT_AddWidget(window, nullptr);
                continue;
            }

            M_snprintf(buf, sizeof(buf), " MAP%02" PRIiPTR " ", l);
            button = TXT_NewButton(buf);
            TXT_SignalConnect(button, "pressed",
                              SetMAPxyWarp, reinterpret_cast<void *>(l));
            TXT_SignalConnect(button, "pressed",
                              CloseLevelSelectDialog, window);
            TXT_AddWidget(window, button);

            if (warpmap == l)
            {
                TXT_SelectWidget(window, button);
            }
        }
    }
}

static void IWADSelected(void *, void *)
{
    const iwad_t *iwad;

    // Find the iwad_t selected

    iwad = GetCurrentIWAD();

    // Update iwadfile

    iwadfile = iwad->name;
}

// Called when the IWAD button is changed, to update warptype.

static void UpdateWarpType(void *, void *)
{
    warptype_t new_warptype;
    const iwad_t *iwad;

    // Get the selected IWAD

    iwad = GetCurrentIWAD();

    // Find the new warp type

    if (D_IsEpisodeMap(iwad->mission))
    {
        new_warptype = WARP_ExMy;
    }
    else
    {
        new_warptype = WARP_MAPxy;
    }

    // Reset to E1M1 / MAP01 when the warp type is changed.

    if (new_warptype != warptype)
    {
        warpepisode = 1;
        warpmap = 1;
    }

    warptype = new_warptype;

    UpdateWarpButton();
    UpdateSkillButton();
}

// Get an IWAD list with a default fallback IWAD that is appropriate
// for the game we are configuring (matches gamemission global variable).

static const iwad_t **GetFallbackIwadList()
{
    static const iwad_t *fallback_iwad_list[2] = {};

    // Default to use if we don't find something better.

    fallback_iwad_list[0] = &fallback_iwads[0];
    fallback_iwad_list[1] = nullptr;

    for (const auto & fallback_iwad : fallback_iwads)
    {
        if (gamemission == fallback_iwad.mission)
        {
            fallback_iwad_list[0] = &fallback_iwad;
            break;
        }
    }

    return fallback_iwad_list;
}

static txt_widget_t *IWADSelector()
{
    // Find out what WADs are installed

    found_iwads = GetIwads();

    // Build a list of the descriptions for all installed IWADs

    int num_iwads = 0;

    for (int i=0; found_iwads[i] != nullptr; ++i)
    {
         ++num_iwads;
    }

    iwad_labels = static_cast<const char **>(malloc(sizeof(*iwad_labels) * static_cast<unsigned long>(num_iwads)));

    for (int i=0; i < num_iwads; ++i)
    {
        iwad_labels[i] = found_iwads[i]->description;
    }

    // If no IWADs are found, provide Doom 2 as an option, but
    // we're probably screwed.

    if (num_iwads == 0)
    {
        found_iwads = GetFallbackIwadList();
        num_iwads = 1;
    }

    // Build a dropdown list of IWADs
    txt_widget_t *result = nullptr;

    if (num_iwads < 2)
    {
        // We have only one IWAD.  Show as a label.

        result = reinterpret_cast<txt_widget_t *>(TXT_NewLabel(found_iwads[0]->description));
    }
    else
    {
        // Dropdown list allowing IWAD to be selected.

        txt_dropdown_list_t *dropdown = TXT_NewDropdownList(&found_iwad_selected,
                                       iwad_labels, num_iwads);

        TXT_SignalConnect(dropdown, "changed", IWADSelected, nullptr);

        result = reinterpret_cast<txt_widget_t *>(dropdown);
    }

    // The first time the dialog is opened, found_iwad_selected=-1,
    // so select the first IWAD in the list. Don't lose the setting
    // if we close and reopen the dialog.

    if (found_iwad_selected < 0 || found_iwad_selected >= num_iwads)
    {
        found_iwad_selected = 0;
    }

    IWADSelected(nullptr, nullptr);

    return result;
}

// Create the window action button to start the game.  This invokes
// a different callback depending on whether to start a multiplayer
// or single player game.

static txt_window_action_t *StartGameAction(int multiplayer)
{
    txt_window_action_t *action;
    TxtWidgetSignalFunc callback;

    action = TXT_NewWindowAction(KEY_F10, "Start");

    if (multiplayer)
    {
        callback = StartServerGame;
    }
    else
    {
        callback = StartSinglePlayerGame;
    }

    TXT_SignalConnect(action, "pressed", callback, nullptr);

    return action;
}

static void OpenWadsWindow(void *, void *)
{
    txt_window_t *window;
    int i;

    window = TXT_NewWindow("Add WADs");

    for (i=0; i<NUM_WADS; ++i)
    {
        TXT_AddWidget(window,
                      TXT_NewFileSelector(&wads[i], 60, "Select a WAD file",
                                          wad_extensions));
    }
}

static void OpenExtraParamsWindow(void *, void *)
{
    txt_window_t *window;
    int i;

    window = TXT_NewWindow("Extra command line parameters");

    for (i=0; i<NUM_EXTRA_PARAMS; ++i)
    {
        TXT_AddWidget(window, TXT_NewInputBox(&extra_params[i], 70));
    }
}

static txt_window_action_t *WadWindowAction()
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction('w', "Add WADs");
    TXT_SignalConnect(action, "pressed", OpenWadsWindow, nullptr);

    return action;
}

static txt_dropdown_list_t *GameTypeDropdown()
{
    switch (gamemission)
    {
        case doom:
        default:
            return TXT_NewDropdownList(&deathmatch, gamemodes, 4);

        // Heretic and Hexen don't support Deathmatch II:

        case heretic:
        case hexen:
            return TXT_NewDropdownList(&deathmatch, gamemodes, 2);

        // Strife supports both deathmatch modes, but doesn't support
        // multiplayer co-op. Use a different variable to indicate whether
        // to use altdeath or not.

        case strife:
            return TXT_NewDropdownList(&strife_altdeath, strife_gamemodes, 2);
    }
}

// "Start game" menu.  This is used for the start server window
// and the single player warp menu.  The parameters specify
// the window title and whether to display multiplayer options.

static void StartGameMenu(const char *window_title, int multiplayer)
{
    txt_window_t *window;
    txt_widget_t *iwad_selector;

    window = TXT_NewWindow(window_title);
    TXT_SetTableColumns(window, 2);
    TXT_SetColumnWidths(window, 12, 6);

    if (multiplayer)
    {
        TXT_SetWindowHelpURL(window, MULTI_START_HELP_URL);
    }
    else
    {
        TXT_SetWindowHelpURL(window, LEVEL_WARP_HELP_URL);
    }

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, StartGameAction(multiplayer));

    TXT_AddWidgets(window,
                   TXT_NewLabel("Game"),
                   iwad_selector = IWADSelector(),
                   nullptr);

    if (gamemission == hexen)
    {
        txt_dropdown_list_t *cc_dropdown;
        TXT_AddWidgets(window,
                       TXT_NewLabel("Character class "),
                       cc_dropdown = TXT_NewDropdownList(&character_class,
                                                         character_classes, 3),
                       nullptr);

        // Update skill level dropdown when the character class is changed:

        TXT_SignalConnect(cc_dropdown, "changed", UpdateWarpType, nullptr);
    }

    TXT_AddWidgets(window,
                   TXT_NewLabel("Skill"),
                   skillbutton = TXT_NewDropdownList(&skill, doom_skills, 5),
                   TXT_NewLabel("Level warp"),
                   warpbutton = TXT_NewButton2("?", LevelSelectDialog, nullptr),
                   nullptr);

    if (multiplayer)
    {
        TXT_AddWidgets(window,
               TXT_NewLabel("Game type"),
               GameTypeDropdown(),
               TXT_NewLabel("Time limit"),
               TXT_NewHorizBox(TXT_NewIntInputBox(&timer, 2),
                               TXT_NewLabel("minutes"),
                               nullptr),
               nullptr);
    }

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Monster options"),
                   TXT_NewInvertedCheckBox("Monsters enabled", &nomonsters),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Fast monsters", &fast),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Respawning monsters", &respawn),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   nullptr);

    if (multiplayer)
    {
        TXT_AddWidgets(window,
                       TXT_NewSeparator("Advanced"),
                       TXT_NewLabel("UDP port"),
                       TXT_NewIntInputBox(&udpport, 5),
                       TXT_NewInvertedCheckBox("Register with master server",
                                               &privateserver),
                       TXT_TABLE_OVERFLOW_RIGHT,
                       nullptr);
    }

    TXT_AddWidgets(window,
                   TXT_NewButton2("Add extra parameters...",
                                  OpenExtraParamsWindow, nullptr),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   nullptr);

    TXT_SignalConnect(iwad_selector, "changed", UpdateWarpType, nullptr);

    UpdateWarpType(nullptr, nullptr);
    UpdateWarpButton();
}

void StartMultiGame(void *, void *)
{
    StartGameMenu("Start multiplayer game", 1);
}

void WarpMenu(void *, void *)
{
    StartGameMenu("Level Warp", 0);
}

static void DoJoinGame(void *, void *)
{
    if (connect_address == nullptr || strlen(connect_address) <= 0)
    {
        TXT_MessageBox(nullptr, "Please enter a server address\n"
                             "to connect to.");
        return;
    }

    execute_context_t *exec = NewExecuteContext();

    AddCmdLineParameter(exec, "-connect %s", connect_address);

    if (gamemission == hexen)
    {
        AddCmdLineParameter(exec, "-class %i", character_class);
    }

    // Extra parameters come first, so that they can be used to override
    // the other parameters.

    AddExtraParameters(exec);
    AddIWADParameter(exec);
    AddWADs(exec);

    TXT_Shutdown();

    M_SaveDefaults();

    PassThroughArguments(exec);

    ExecuteDoom(exec);

    exit(0);
}

static txt_window_action_t *JoinGameAction()
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_F10, "Connect");
    TXT_SignalConnect(action, "pressed", DoJoinGame, nullptr);

    return action;
}

static void SelectQueryAddress(void *uncast_button, void *uncast_querydata)
{
    auto *button    = reinterpret_cast<txt_button_t *>(uncast_button);
    auto *querydata = reinterpret_cast<net_querydata_t *>(uncast_querydata);
    int   i;

    if (querydata->server_state != 0)
    {
        TXT_MessageBox("Cannot connect to server",
                       "Gameplay is already in progress\n"
                       "on this server.");
        return;
    }

    // Set address to connect to:

    free(connect_address);
    connect_address = M_StringDuplicate(button->label);

    // Auto-choose IWAD if there is already a player connected.

    if (querydata->num_players > 0)
    {
        for (i = 0; found_iwads[i] != nullptr; ++i)
        {
            if (found_iwads[i]->mode == querydata->gamemode
             && found_iwads[i]->mission == querydata->gamemission)
            {
                found_iwad_selected = i;
                iwadfile = found_iwads[i]->name;
                break;
            }
        }

        if (found_iwads[i] == nullptr)
        {
            TXT_MessageBox(nullptr,
                           "The game on this server seems to be:\n"
                           "\n"
                           "   %s\n"
                           "\n"
                           "but the IWAD file %s is not found!\n"
                           "Without the required IWAD file, it may not be\n"
                           "possible to join this game.",
                D_SuggestGameName(static_cast<GameMission_t>(querydata->gamemission),
                    static_cast<GameMode_t>(querydata->gamemode)),
                D_SuggestIWADName(static_cast<GameMission_t>(querydata->gamemission),
                    static_cast<GameMode_t>(querydata->gamemode)));
        }
    }

    // Finished with search.

    TXT_CloseWindow(query_window);
}

static void QueryResponseCallback(net_addr_t *addr, net_querydata_t *querydata, unsigned int ping_time, void *uncast_results_table)
{
    auto *results_table = reinterpret_cast<txt_table_t *>(uncast_results_table);
    char ping_time_str[16];
    char description[47];

    // When we connect we'll have to negotiate a common protocol that we
    // can agree upon between the client and server. If we can't then we
    // won't be able to connect, so it's pointless to include it in the
    // results list. If protocol==NET_PROTOCOL_UNKNOWN then this may be
    // an old, pre-3.0 Chocolate Doom server that doesn't support the new
    // protocol negotiation mechanism, or it may be an incompatible fork.
    if (querydata->protocol == NET_PROTOCOL_UNKNOWN)
    {
        return;
    }

    M_snprintf(ping_time_str, sizeof(ping_time_str), "%ims", ping_time);

    // Build description from server name field. Because there is limited
    // space, we only include the player count if there are already players
    // connected to the server.
    if (querydata->num_players > 0)
    {
        M_snprintf(description, sizeof(description), "(%d/%d) ",
                   querydata->num_players, querydata->max_players);
    }
    else
    {
        M_StringCopy(description, "", sizeof(description));
    }

    M_StringConcat(description, querydata->description, sizeof(description));

    TXT_AddWidgets(results_table,
                   TXT_NewLabel(ping_time_str),
                   TXT_NewButton2(NET_AddrToString(addr),
                                  SelectQueryAddress, querydata),
                   TXT_NewLabel(description),
                   nullptr);

    ++query_servers_found;
}

static void QueryPeriodicCallback(void *uncast_results_table)
{
    auto *results_table = reinterpret_cast<txt_table_t *>(uncast_results_table);

    if (!NET_Query_Poll(QueryResponseCallback, results_table))
    {
        TXT_SetPeriodicCallback(nullptr, nullptr, 0);

        if (query_servers_found == 0)
        {
            TXT_AddWidgets(results_table,
                TXT_TABLE_EMPTY,
                TXT_NewLabel("No compatible servers found."),
                nullptr
            );
        }
    }
}

static void QueryWindowClosed(void *, void *)
{
    TXT_SetPeriodicCallback(nullptr, nullptr, 0);
}

static void ServerQueryWindow(const char *title)
{
    txt_table_t *results_table;

    query_servers_found = 0;

    query_window = TXT_NewWindow(title);

    TXT_AddWidget(query_window,
                  TXT_NewScrollPane(70, 10,
                                    results_table = TXT_NewTable(3)));

    TXT_SetColumnWidths(results_table, 7, 22, 40);
    TXT_SetPeriodicCallback(QueryPeriodicCallback, results_table, 1);

    TXT_SignalConnect(query_window, "closed", QueryWindowClosed, nullptr);
}

static void FindInternetServer(void *, void *)
{
    NET_StartMasterQuery();
    ServerQueryWindow("Find Internet server");
}

static void FindLANServer(void *, void *)
{
    NET_StartLANQuery();
    ServerQueryWindow("Find LAN server");
}

void JoinMultiGame(void *, void *)
{
    txt_window_t *window;
    txt_inputbox_t *address_box;

    window = TXT_NewWindow("Join multiplayer game");
    TXT_SetTableColumns(window, 2);
    TXT_SetColumnWidths(window, 12, 12);

    TXT_SetWindowHelpURL(window, MULTI_JOIN_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewLabel("Game"),
                   IWADSelector(),
                   nullptr);

    if (gamemission == hexen)
    {
        TXT_AddWidgets(window,
                       TXT_NewLabel("Character class "),
                       TXT_NewDropdownList(&character_class,
                                           character_classes, 3),
                       nullptr);
    }

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Server"),
                   TXT_NewLabel("Connect to address: "),
                   address_box = TXT_NewInputBox(&connect_address, 30),

                   TXT_NewButton2("Find server on Internet...",
                                  FindInternetServer, nullptr),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewButton2("Find server on local network...",
                                  FindLANServer, nullptr),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewStrut(0, 1),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewButton2("Add extra parameters...",
                                  OpenExtraParamsWindow, nullptr),
                   nullptr);

    TXT_SelectWidget(window, address_box);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, JoinGameAction());
}

void SetChatMacroDefaults()
{
    int i;
    const char *const defaults[] =
    {
        HUSTR_CHATMACRO0,
        HUSTR_CHATMACRO1,
        HUSTR_CHATMACRO2,
        HUSTR_CHATMACRO3,
        HUSTR_CHATMACRO4,
        HUSTR_CHATMACRO5,
        HUSTR_CHATMACRO6,
        HUSTR_CHATMACRO7,
        HUSTR_CHATMACRO8,
        HUSTR_CHATMACRO9,
    };

    // If the chat macros have not been set, initialize with defaults.

    for (i=0; i<10; ++i)
    {
        if (chat_macros[i] == nullptr)
        {
            chat_macros[i] = M_StringDuplicate(defaults[i]);
        }
    }
}

void SetPlayerNameDefault()
{
    if (net_player_name == nullptr)
    {
        net_player_name = NET_GetRandomPetName();
    }
}


void MultiplayerConfig(void *, void *)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_table_t *table;
    char buf[10];
    int i;

    window = TXT_NewWindow("Multiplayer Configuration");
    TXT_SetWindowHelpURL(window, MULTI_CONFIG_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewStrut(0, 1),
                   TXT_NewHorizBox(TXT_NewLabel("Player name:  "),
                                   TXT_NewInputBox(&net_player_name, 25),
                                   nullptr),
                   TXT_NewStrut(0, 1),
                   TXT_NewSeparator("Chat macros"),
                   nullptr);

    table = TXT_NewTable(2);

    for (i=0; i<10; ++i)
    {
        M_snprintf(buf, sizeof(buf), "#%i ", i + 1);

        label = TXT_NewLabel(buf);
        TXT_SetFGColor(label, TXT_COLOR_BRIGHT_CYAN);

        TXT_AddWidgets(table,
                       label,
                       TXT_NewInputBox(&chat_macros[(i + 1) % 10], 40),
                       nullptr);
    }

    TXT_AddWidget(window, table);
}

void BindMultiplayerVariables()
{
    char buf[15];
    int i;

    M_BindStringVariable("player_name", &net_player_name);

    for (i=0; i<10; ++i)
    {
        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindStringVariable(buf, &chat_macros[i]);
    }

    switch (gamemission)
    {
        case doom:
            M_BindChatControls(4);
            g_m_controls_globals->key_multi_msgplayer[0] = 'g';
            g_m_controls_globals->key_multi_msgplayer[1] = 'i';
            g_m_controls_globals->key_multi_msgplayer[2] = 'b';
            g_m_controls_globals->key_multi_msgplayer[3] = 'r';
            break;

        case heretic:
            M_BindChatControls(4);
            g_m_controls_globals->key_multi_msgplayer[0] = 'g';
            g_m_controls_globals->key_multi_msgplayer[1] = 'y';
            g_m_controls_globals->key_multi_msgplayer[2] = 'r';
            g_m_controls_globals->key_multi_msgplayer[3] = 'b';
            break;

        case hexen:
            M_BindChatControls(8);
            g_m_controls_globals->key_multi_msgplayer[0] = 'b';
            g_m_controls_globals->key_multi_msgplayer[1] = 'r';
            g_m_controls_globals->key_multi_msgplayer[2] = 'y';
            g_m_controls_globals->key_multi_msgplayer[3] = 'g';
            g_m_controls_globals->key_multi_msgplayer[4] = 'j';
            g_m_controls_globals->key_multi_msgplayer[5] = 'w';
            g_m_controls_globals->key_multi_msgplayer[6] = 'h';
            g_m_controls_globals->key_multi_msgplayer[7] = 'p';
            break;

        default:
            break;
    }
}
