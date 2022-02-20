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

#include "textscreen.hpp"
#include "doomtype.hpp"
#include "m_config.hpp"
#include "m_controls.hpp"
#include "m_misc.hpp"

#include "execute.hpp"
#include "txt_keyinput.hpp"

#include "mode.hpp"
#include "joystick.hpp"
#include "keyboard.hpp"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-keyboard"

int vanilla_keyboard_mapping = 1;

static int always_run = 0;

// Keys within these groups cannot have the same value.

static int *controls[] = {
    &g_m_controls_globals->key_left,
    &g_m_controls_globals->key_right,
    &g_m_controls_globals->key_up,
    &g_m_controls_globals->key_down,
    &g_m_controls_globals->key_alt_up,
    &g_m_controls_globals->key_alt_down,
    &g_m_controls_globals->key_reverse,
    &g_m_controls_globals->key_toggleautorun,
    &g_m_controls_globals->key_togglenovert,

    &g_m_controls_globals->key_strafeleft,
    &g_m_controls_globals->key_straferight,
    &g_m_controls_globals->key_fire,

    &g_m_controls_globals->key_alt_strafeleft,
    &g_m_controls_globals->key_alt_straferight,

    &g_m_controls_globals->key_use,
    &g_m_controls_globals->key_strafe,
    &g_m_controls_globals->key_speed,
    &g_m_controls_globals->key_jump,

    &g_m_controls_globals->key_flyup,
    &g_m_controls_globals->key_flydown,
    &g_m_controls_globals->key_flycenter,

    &g_m_controls_globals->key_lookup,
    &g_m_controls_globals->key_lookdown,
    &g_m_controls_globals->key_lookcenter,

    &g_m_controls_globals->key_invleft,
    &g_m_controls_globals->key_invright,
    &g_m_controls_globals->key_invquery,

    &g_m_controls_globals->key_invuse,
    &g_m_controls_globals->key_invpop,
    &g_m_controls_globals->key_mission,
    &g_m_controls_globals->key_invkey,

    &g_m_controls_globals->key_invhome,
    &g_m_controls_globals->key_invend,
    &g_m_controls_globals->key_invdrop,

    &g_m_controls_globals->key_useartifact,
    &g_m_controls_globals->key_pause,
    &g_m_controls_globals->key_usehealth,

    &g_m_controls_globals->key_weapon1,
    &g_m_controls_globals->key_weapon2,
    &g_m_controls_globals->key_weapon3,

    &g_m_controls_globals->key_weapon4,
    &g_m_controls_globals->key_weapon5,
    &g_m_controls_globals->key_weapon6,

    &g_m_controls_globals->key_weapon7,
    &g_m_controls_globals->key_weapon8,

    &g_m_controls_globals->key_arti_quartz,
    &g_m_controls_globals->key_arti_urn,
    &g_m_controls_globals->key_arti_bomb,

    &g_m_controls_globals->key_arti_tome,
    &g_m_controls_globals->key_arti_ring,
    &g_m_controls_globals->key_arti_chaosdevice,

    &g_m_controls_globals->key_arti_shadowsphere,
    &g_m_controls_globals->key_arti_wings,
    &g_m_controls_globals->key_arti_torch,

    &g_m_controls_globals->key_arti_all,
    &g_m_controls_globals->key_arti_health,
    &g_m_controls_globals->key_arti_poisonbag,

    &g_m_controls_globals->key_arti_blastradius,
    &g_m_controls_globals->key_arti_teleport,

    &g_m_controls_globals->key_arti_teleportother,
    &g_m_controls_globals->key_arti_egg,

    &g_m_controls_globals->key_arti_invulnerability,

    &g_m_controls_globals->key_prevweapon,
    &g_m_controls_globals->key_nextweapon,
    nullptr
};

static int *menu_nav[] = {
    &g_m_controls_globals->key_menu_activate,
    &g_m_controls_globals->key_menu_up,
    &g_m_controls_globals->key_menu_down,

    &g_m_controls_globals->key_menu_left,
    &g_m_controls_globals->key_menu_right,
    &g_m_controls_globals->key_menu_back,

    &g_m_controls_globals->key_menu_forward,
    &g_m_controls_globals->key_menu_del,
    nullptr
};

static int *shortcuts[] = {
    &g_m_controls_globals->key_menu_help,
    &g_m_controls_globals->key_menu_save,
    &g_m_controls_globals->key_menu_load,

    &g_m_controls_globals->key_menu_volume,
    &g_m_controls_globals->key_menu_detail,
    &g_m_controls_globals->key_menu_qsave,

    &g_m_controls_globals->key_menu_endgame,
    &g_m_controls_globals->key_menu_messages,
    &g_m_controls_globals->key_spy,

    &g_m_controls_globals->key_menu_qload,
    &g_m_controls_globals->key_menu_quit,
    &g_m_controls_globals->key_menu_gamma,

    &g_m_controls_globals->key_menu_nextlevel,
    &g_m_controls_globals->key_menu_reloadlevel,

    &g_m_controls_globals->key_menu_incscreen,
    &g_m_controls_globals->key_menu_decscreen,

    &g_m_controls_globals->key_menu_screenshot,
    &g_m_controls_globals->key_menu_cleanscreenshot,

    &g_m_controls_globals->key_message_refresh,
    &g_m_controls_globals->key_multi_msg,

    &g_m_controls_globals->key_multi_msgplayer[0],
    &g_m_controls_globals->key_multi_msgplayer[1],

    &g_m_controls_globals->key_multi_msgplayer[2],
    &g_m_controls_globals->key_multi_msgplayer[3]
};

static int *map_keys[] = {
    &g_m_controls_globals->key_map_north,
    &g_m_controls_globals->key_map_south,
    &g_m_controls_globals->key_map_east,

    &g_m_controls_globals->key_map_west,
    &g_m_controls_globals->key_map_zoomin,
    &g_m_controls_globals->key_map_zoomout,

    &g_m_controls_globals->key_map_toggle,
    &g_m_controls_globals->key_map_maxzoom,
    &g_m_controls_globals->key_map_follow,

    &g_m_controls_globals->key_map_grid,
    &g_m_controls_globals->key_map_mark,
    &g_m_controls_globals->key_map_clearmark,

    &g_m_controls_globals->key_map_overlay,
    &g_m_controls_globals->key_map_rotate,

    nullptr
};

static void UpdateJoybSpeed(void *, void *)
{
    if (always_run)
    {
        /*
         <Janizdreg> if you want to pick one for chocolate doom to use, 
                     pick 29, since that is the most universal one that 
                     also works with heretic, hexen and strife =P

         NB. This choice also works with original, ultimate and final exes.
        */

        g_m_controls_globals->joybspeed = 29;
    }
    else
    {
        g_m_controls_globals->joybspeed = 2;
    }
}

static int VarInGroup(int *variable, int **group)
{
    unsigned int i;

    for (i=0; group[i] != nullptr; ++i)
    {
        if (group[i] == variable)
        {
            return 1;
        }
    }

    return 0;
}

static void CheckKeyGroup(int *variable, int **group)
{
    unsigned int i;

    // Don't check unless the variable is in this group.

    if (!VarInGroup(variable, group))
    {
        return;
    }

    // If another variable has the same value as the new value, reset it.

    for (i=0; group[i] != nullptr; ++i)
    {
        if (*variable == *group[i] && group[i] != variable)
        {
            // A different key has the same value.  Clear the existing
            // value. This ensures that no two keys can have the same
            // value.

            *group[i] = 0;
        }
    }
}

// Callback invoked when a key control is set

static void KeySetCallback(void *, void *uncast_variable)
{
    auto *variable = reinterpret_cast<int *>(uncast_variable);

    CheckKeyGroup(variable, controls);
    CheckKeyGroup(variable, menu_nav);
    CheckKeyGroup(variable, shortcuts);
    CheckKeyGroup(variable, map_keys);
}

// Add a label and keyboard input to the specified table.

static void AddKeyControl(void *uncast_table, const char *name, int *var)
{
    auto     *table = reinterpret_cast<txt_table_t *>(uncast_table);
    txt_key_input_t *key_input;

    TXT_AddWidget(table, TXT_NewLabel(name));
    key_input = TXT_NewKeyInput(var);
    TXT_AddWidget(table, key_input);

    TXT_SignalConnect(key_input, "set", KeySetCallback, var);
}

static void AddSectionLabel(void *uncast_table, const char *title,
                            bool add_space)
{
    auto *table = reinterpret_cast<txt_table_t *>(uncast_table);
    char buf[64];

    if (add_space)
    {
        TXT_AddWidgets(table,
                       TXT_NewStrut(0, 1),
                       TXT_TABLE_EOL,
                       nullptr);
    }

    M_snprintf(buf, sizeof(buf), " - %s - ", title);

    TXT_AddWidgets(table,
                   TXT_NewLabel(buf),
                   TXT_TABLE_EOL,
                   nullptr);
}

static void ConfigExtraKeys(void *, void *)
{
    txt_window_t *window;
    txt_scrollpane_t *scrollpane;
    txt_table_t *table;
    bool extra_keys = gamemission == heretic
                      || gamemission == hexen
                      || gamemission == strife;

    window = TXT_NewWindow("Extra keyboard controls");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 21, 9);

    if (extra_keys || 1) // Crispy
    {
        // When we have extra controls, a scrollable pane must be used.

        scrollpane = TXT_NewScrollPane(0, 13, table);
        TXT_AddWidget(window, scrollpane);


        if (gamemission == doom)
        {
        AddSectionLabel(table, "View", false);

        AddKeyControl(table, "Look up [*]", &g_m_controls_globals->key_lookup);
        AddKeyControl(table, "Look down [*]", &g_m_controls_globals->key_lookdown);
        AddKeyControl(table, "Center view [*]", &g_m_controls_globals->key_lookcenter);

        AddSectionLabel(table, "Movement", false);
        AddKeyControl(table, "Move Forward (alt.)", &g_m_controls_globals->key_alt_up);
        AddKeyControl(table, "Move Backward (alt.)", &g_m_controls_globals->key_alt_down);
        AddKeyControl(table, "Strafe Left (alt.)", &g_m_controls_globals->key_alt_strafeleft);
        AddKeyControl(table, "Strafe Right (alt.)", &g_m_controls_globals->key_alt_straferight);
        AddKeyControl(table, "Toggle always run", &g_m_controls_globals->key_toggleautorun);
        AddKeyControl(table, "Toggle vert. mouse", &g_m_controls_globals->key_togglenovert);
        AddKeyControl(table, "Quick Reverse", &g_m_controls_globals->key_reverse);
        }
        else
        {
        AddSectionLabel(table, "View", false);

        AddKeyControl(table, "Look up", &g_m_controls_globals->key_lookup);
        AddKeyControl(table, "Look down", &g_m_controls_globals->key_lookdown);
        AddKeyControl(table, "Center view", &g_m_controls_globals->key_lookcenter);
        }

        if (gamemission == heretic || gamemission == hexen)
        {
            AddSectionLabel(table, "Flying", true);

            AddKeyControl(table, "Fly up", &g_m_controls_globals->key_flyup);
            AddKeyControl(table, "Fly down", &g_m_controls_globals->key_flydown);
            AddKeyControl(table, "Fly center", &g_m_controls_globals->key_flycenter);
        }

        if (gamemission != doom)
        {
        AddSectionLabel(table, "Inventory", true);

        AddKeyControl(table, "Inventory left", &g_m_controls_globals->key_invleft);
        AddKeyControl(table, "Inventory right", &g_m_controls_globals->key_invright);
        }

        if (gamemission == strife)
        {
            AddKeyControl(table, "Home", &g_m_controls_globals->key_invhome);
            AddKeyControl(table, "End", &g_m_controls_globals->key_invend);
            AddKeyControl(table, "Query", &g_m_controls_globals->key_invquery);
            AddKeyControl(table, "Drop", &g_m_controls_globals->key_invdrop);
            AddKeyControl(table, "Show weapons", &g_m_controls_globals->key_invpop);
            AddKeyControl(table, "Show mission", &g_m_controls_globals->key_mission);
            AddKeyControl(table, "Show keys", &g_m_controls_globals->key_invkey);
            AddKeyControl(table, "Use", &g_m_controls_globals->key_invuse);
            AddKeyControl(table, "Use health", &g_m_controls_globals->key_usehealth);
        }
        else
        if (gamemission == heretic || gamemission == hexen)
        {
            AddKeyControl(table, "Use artifact", &g_m_controls_globals->key_useartifact);
        }

        if (gamemission == heretic)
        {
            AddSectionLabel(table, "Artifacts", true);

            AddKeyControl(table, "Quartz Flask", &g_m_controls_globals->key_arti_quartz);
            AddKeyControl(table, "Mystic Urn", &g_m_controls_globals->key_arti_urn);
            AddKeyControl(table, "Timebomb", &g_m_controls_globals->key_arti_bomb);
            AddKeyControl(table, "Tome of Power", &g_m_controls_globals->key_arti_tome);
            AddKeyControl(table, "Ring of Invincibility ", &g_m_controls_globals->key_arti_ring);
            AddKeyControl(table, "Chaos Device", &g_m_controls_globals->key_arti_chaosdevice);
            AddKeyControl(table, "Shadowsphere", &g_m_controls_globals->key_arti_shadowsphere);
            AddKeyControl(table, "Wings of Wrath", &g_m_controls_globals->key_arti_wings);
            AddKeyControl(table, "Torch", &g_m_controls_globals->key_arti_torch);
        }

        if (gamemission == hexen)
        {
            AddSectionLabel(table, "Artifacts", true);

            AddKeyControl(table, "One of each", &g_m_controls_globals->key_arti_all);
            AddKeyControl(table, "Quartz Flask", &g_m_controls_globals->key_arti_health);
            AddKeyControl(table, "Flechette", &g_m_controls_globals->key_arti_poisonbag);
            AddKeyControl(table, "Disc of Repulsion", &g_m_controls_globals->key_arti_blastradius);
            AddKeyControl(table, "Chaos Device", &g_m_controls_globals->key_arti_teleport);
            AddKeyControl(table, "Banishment Device", &g_m_controls_globals->key_arti_teleportother);
            AddKeyControl(table, "Porkalator", &g_m_controls_globals->key_arti_egg);
            AddKeyControl(table, "Icon of the Defender",
                          &g_m_controls_globals->key_arti_invulnerability);
        }
    }
    else
    {
        TXT_AddWidget(window, table);
    }

    AddSectionLabel(table, "Weapons", extra_keys);

    AddKeyControl(table, "Weapon 1", &g_m_controls_globals->key_weapon1);
    AddKeyControl(table, "Weapon 2", &g_m_controls_globals->key_weapon2);
    AddKeyControl(table, "Weapon 3", &g_m_controls_globals->key_weapon3);
    AddKeyControl(table, "Weapon 4", &g_m_controls_globals->key_weapon4);
    AddKeyControl(table, "Weapon 5", &g_m_controls_globals->key_weapon5);
    AddKeyControl(table, "Weapon 6", &g_m_controls_globals->key_weapon6);
    AddKeyControl(table, "Weapon 7", &g_m_controls_globals->key_weapon7);
    AddKeyControl(table, "Weapon 8", &g_m_controls_globals->key_weapon8);
    AddKeyControl(table, "Previous weapon", &g_m_controls_globals->key_prevweapon);
    AddKeyControl(table, "Next weapon", &g_m_controls_globals->key_nextweapon);
}

static void OtherKeysDialog(void *, void *)
{
    txt_window_t *window;
    txt_table_t *table;
    txt_scrollpane_t *scrollpane;

    window = TXT_NewWindow("Other keys");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 25, 9);

    AddSectionLabel(table, "Menu navigation", false);

    AddKeyControl(table, "Activate menu",         &g_m_controls_globals->key_menu_activate);
    AddKeyControl(table, "Move cursor up",        &g_m_controls_globals->key_menu_up);
    AddKeyControl(table, "Move cursor down",      &g_m_controls_globals->key_menu_down);
    AddKeyControl(table, "Move slider left",      &g_m_controls_globals->key_menu_left);
    AddKeyControl(table, "Move slider right",     &g_m_controls_globals->key_menu_right);
    AddKeyControl(table, "Go to previous menu",   &g_m_controls_globals->key_menu_back);
    AddKeyControl(table, "Activate menu item",    &g_m_controls_globals->key_menu_forward);
    AddKeyControl(table, "Confirm action",        &g_m_controls_globals->key_menu_confirm);
    AddKeyControl(table, "Cancel action",         &g_m_controls_globals->key_menu_abort);
    AddKeyControl(table, "Delete savegame",       &g_m_controls_globals->key_menu_del);

    AddSectionLabel(table, "Shortcut keys", true);

    AddKeyControl(table, "Pause game",            &g_m_controls_globals->key_pause);
    AddKeyControl(table, "Help screen",           &g_m_controls_globals->key_menu_help);
    AddKeyControl(table, "Save game",             &g_m_controls_globals->key_menu_save);
    AddKeyControl(table, "Load game",             &g_m_controls_globals->key_menu_load);
    AddKeyControl(table, "Sound volume",          &g_m_controls_globals->key_menu_volume);
    AddKeyControl(table, "Toggle detail",         &g_m_controls_globals->key_menu_detail);
    AddKeyControl(table, "Quick save",            &g_m_controls_globals->key_menu_qsave);
    AddKeyControl(table, "End game",              &g_m_controls_globals->key_menu_endgame);
    AddKeyControl(table, "Toggle messages",       &g_m_controls_globals->key_menu_messages);
    AddKeyControl(table, "Quick load",            &g_m_controls_globals->key_menu_qload);
    AddKeyControl(table, "Quit game",             &g_m_controls_globals->key_menu_quit);
    AddKeyControl(table, "Toggle gamma",          &g_m_controls_globals->key_menu_gamma);
    AddKeyControl(table, "Multiplayer spy",       &g_m_controls_globals->key_spy);
    AddKeyControl(table, "Go to next level",      &g_m_controls_globals->key_menu_nextlevel);
    AddKeyControl(table, "Restart level/demo",    &g_m_controls_globals->key_menu_reloadlevel);

    AddKeyControl(table, "Increase screen size",  &g_m_controls_globals->key_menu_incscreen);
    AddKeyControl(table, "Decrease screen size",  &g_m_controls_globals->key_menu_decscreen);
    AddKeyControl(table, "Save a screenshot",     &g_m_controls_globals->key_menu_screenshot);
    AddKeyControl(table, "Save a clean screenshot",&g_m_controls_globals->key_menu_cleanscreenshot);

    AddKeyControl(table, "Display last message",  &g_m_controls_globals->key_message_refresh);
    AddKeyControl(table, "Finish recording demo", &g_m_controls_globals->key_demo_quit);

    AddSectionLabel(table, "Map", true);
    AddKeyControl(table, "Toggle map",            &g_m_controls_globals->key_map_toggle);
    AddKeyControl(table, "Zoom in",               &g_m_controls_globals->key_map_zoomin);
    AddKeyControl(table, "Zoom out",              &g_m_controls_globals->key_map_zoomout);
    AddKeyControl(table, "Maximum zoom out",      &g_m_controls_globals->key_map_maxzoom);
    AddKeyControl(table, "Follow mode",           &g_m_controls_globals->key_map_follow);
    AddKeyControl(table, "Pan north",             &g_m_controls_globals->key_map_north);
    AddKeyControl(table, "Pan south",             &g_m_controls_globals->key_map_south);
    AddKeyControl(table, "Pan east",              &g_m_controls_globals->key_map_east);
    AddKeyControl(table, "Pan west",              &g_m_controls_globals->key_map_west);
    AddKeyControl(table, "Toggle grid",           &g_m_controls_globals->key_map_grid);
    AddKeyControl(table, "Mark location",         &g_m_controls_globals->key_map_mark);
    AddKeyControl(table, "Clear all marks",       &g_m_controls_globals->key_map_clearmark);
    AddKeyControl(table, "Overlay mode",          &g_m_controls_globals->key_map_overlay);
    AddKeyControl(table, "Rotate mode",           &g_m_controls_globals->key_map_rotate);

    AddSectionLabel(table, "Multiplayer", true);

    AddKeyControl(table, "Send message",          &g_m_controls_globals->key_multi_msg);
    AddKeyControl(table, "- to player 1",         &g_m_controls_globals->key_multi_msgplayer[0]);
    AddKeyControl(table, "- to player 2",         &g_m_controls_globals->key_multi_msgplayer[1]);
    AddKeyControl(table, "- to player 3",         &g_m_controls_globals->key_multi_msgplayer[2]);
    AddKeyControl(table, "- to player 4",         &g_m_controls_globals->key_multi_msgplayer[3]);

    if (gamemission == hexen || gamemission == strife)
    {
        AddKeyControl(table, "- to player 5",     &g_m_controls_globals->key_multi_msgplayer[4]);
        AddKeyControl(table, "- to player 6",     &g_m_controls_globals->key_multi_msgplayer[5]);
        AddKeyControl(table, "- to player 7",     &g_m_controls_globals->key_multi_msgplayer[6]);
        AddKeyControl(table, "- to player 8",     &g_m_controls_globals->key_multi_msgplayer[7]);
    }

    scrollpane = TXT_NewScrollPane(0, 13, table);

    TXT_AddWidget(window, scrollpane);
}

void ConfigKeyboard(void *, void *)
{
    txt_window_t *window;
    txt_checkbox_t *run_control;

    always_run = g_m_controls_globals->joybspeed >= 20;

    window = TXT_NewWindow("Keyboard configuration");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    // The window is on a 5-column grid layout that looks like:
    // Label | Control | | Label | Control
    // There is a small gap between the two conceptual "columns" of
    // controls, just for spacing.
    TXT_SetTableColumns(window, 5);
    TXT_SetColumnWidths(window, 15, 8, 2, 15, 8);

    TXT_AddWidget(window, TXT_NewSeparator("Movement"));
    AddKeyControl(window, "Move Forward", &g_m_controls_globals->key_up);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Strafe Left", &g_m_controls_globals->key_strafeleft);

    AddKeyControl(window, "Move Backward", &g_m_controls_globals->key_down);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Strafe Right", &g_m_controls_globals->key_straferight);

    AddKeyControl(window, "Turn Left", &g_m_controls_globals->key_left);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Speed On", &g_m_controls_globals->key_speed);

    AddKeyControl(window, "Turn Right", &g_m_controls_globals->key_right);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Strafe On", &g_m_controls_globals->key_strafe);

    if (gamemission == hexen || gamemission == strife)
    {
        AddKeyControl(window, "Jump", &g_m_controls_globals->key_jump);
    }
    else
    if (gamemission == doom) // Crispy
    {
        AddKeyControl(window, "Jump [*]", &g_m_controls_globals->key_jump);
    }

    TXT_AddWidget(window, TXT_NewSeparator("Action"));
    AddKeyControl(window, "Fire/Attack", &g_m_controls_globals->key_fire);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Use", &g_m_controls_globals->key_use);

    TXT_AddWidgets(window,
                   TXT_NewButton2("More controls...", ConfigExtraKeys, nullptr),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_EMPTY,
                   TXT_NewButton2("Other keys...", OtherKeysDialog, nullptr),
                   TXT_TABLE_OVERFLOW_RIGHT,

                   TXT_NewSeparator("Misc."),
                   run_control = TXT_NewCheckBox("Always run", &always_run),
                   TXT_TABLE_EOL,
                   TXT_NewInvertedCheckBox("Use native keyboard mapping",
                                           &vanilla_keyboard_mapping),
                   TXT_TABLE_EOL,
                   nullptr);

    TXT_SignalConnect(run_control, "changed", UpdateJoybSpeed, nullptr);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
}

void BindKeyboardVariables()
{
    M_BindIntVariable("vanilla_keyboard_mapping", &vanilla_keyboard_mapping);
}
