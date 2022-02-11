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

#include "crispy.hpp"
#include "textscreen.hpp"
#include "doomtype.hpp"
#include "m_config.hpp"
#include "m_controls.hpp"

#include "execute.hpp"
#include "txt_mouseinput.hpp"

#include "mode.hpp"
#include "mouse.hpp"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-mouse"

static int usemouse = 1;

static int mouseSensitivity = 5;
static int mouseSensitivity_x2 = 5; // [crispy]
static float mouse_acceleration = 2.0;
static int mouse_threshold = 10;
static int mouseSensitivity_y = 5; // [crispy]
static float mouse_acceleration_y = 1.0; // [crispy]
static int mouse_threshold_y = 0; // [crispy]
static int grabmouse = 1;

int novert = 1;

static int *all_mouse_buttons[] = {
    &mousebfire,
    &mousebstrafe,
    &mousebforward,
    &mousebstrafeleft,
    &mousebstraferight,
    &mousebbackward,
    &mousebuse,
    &mousebjump,
    &mousebprevweapon,
    &mousebnextweapon,
    &mousebmouselook, // [crispy]
    &mousebreverse // [crispy]
};

static void MouseSetCallback(void *, void *uncast_variable)
{
    int         *variable = reinterpret_cast<int *>(uncast_variable);

    // Check if the same mouse button is used for a different action
    // If so, set the other action(s) to -1 (unset)

    for (auto & all_mouse_button : all_mouse_buttons)
    {
        if (*all_mouse_button == *variable
         && all_mouse_button != variable)
        {
            *all_mouse_button = -1;
        }
    }
}

static void AddMouseControl(void *uncast_table, const char *label, int *var)
{
    auto              *table = reinterpret_cast<txt_table_t *>(uncast_table);
    txt_mouse_input_t *mouse_input;

    TXT_AddWidget(table, TXT_NewLabel(label));

    mouse_input = TXT_NewMouseInput(var);
    TXT_AddWidget(table, mouse_input);

    TXT_SignalConnect(mouse_input, "set", MouseSetCallback, var);
}

static void ConfigExtraButtons(void *, void *)
{
    txt_window_t *window;
    txt_table_t *buttons_table;

    window = TXT_NewWindow("Additional mouse buttons");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
                   buttons_table = TXT_NewTable(4),
                   nullptr);

    TXT_SetColumnWidths(buttons_table, 16, 11, 14, 10);

    AddMouseControl(buttons_table, "Move forward", &mousebforward);
    AddMouseControl(buttons_table, "Strafe left", &mousebstrafeleft);
    AddMouseControl(buttons_table, "Move backward", &mousebbackward);
    AddMouseControl(buttons_table, "Strafe right", &mousebstraferight);
    AddMouseControl(buttons_table, "Previous weapon", &mousebprevweapon);
    AddMouseControl(buttons_table, "Strafe on", &mousebstrafe);
    AddMouseControl(buttons_table, "Next weapon", &mousebnextweapon);

    if (gamemission == hexen || gamemission == strife)
    {
        AddMouseControl(buttons_table, "Jump", &mousebjump);
    }

    if (gamemission == doom) // [crispy]
    {
        AddMouseControl(buttons_table, "Quick Reverse", &mousebreverse);
        AddMouseControl(buttons_table, "Mouse Look [*]", &mousebmouselook);
        AddMouseControl(buttons_table, "Jump [*]", &mousebjump);
    }
}

void ConfigMouse(void *, void *)
{
    txt_window_t *window;

    window = TXT_NewWindow("Mouse configuration");

    TXT_SetTableColumns(window, 2);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    if (gamemission == doom) // [crispy]
    {
    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable mouse", &usemouse),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewInvertedCheckBox("Allow vertical mouse movement",
                                           &novert),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Grab mouse in windowed mode",
                                   &grabmouse),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Double click acts as \"use\"",
                                   &dclick_use),
                   TXT_TABLE_OVERFLOW_RIGHT,

                   TXT_NewSeparator("Mouse motion"),
                   TXT_NewLabel("Speed (h/turn)"),
                   TXT_NewSpinControl(&mouseSensitivity, 0, 255), // [crispy] extended range
                   TXT_NewLabel("Speed (h/strafe)"),
                   TXT_NewSpinControl(&mouseSensitivity_x2, 0, 255), // [crispy] extended range
                   TXT_NewLabel("Acceleration (h)"),
                   TXT_NewFloatSpinControl(&mouse_acceleration, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold (h)"),
                   TXT_NewSpinControl(&mouse_threshold, 0, 32),
                   TXT_NewLabel("Speed (v)"),
                   TXT_NewSpinControl(&mouseSensitivity_y, 0, 255), // [crispy] extended range
                   TXT_NewLabel("Acceleration (v)"),
                   TXT_NewFloatSpinControl(&mouse_acceleration_y, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold (v)"),
                   TXT_NewSpinControl(&mouse_threshold_y, 0, 32),

                   TXT_NewSeparator("Buttons"),
                   nullptr);
    }
    else
    {
    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable mouse", &usemouse),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewInvertedCheckBox("Allow vertical mouse movement", 
                                           &novert),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Grab mouse in windowed mode", 
                                   &grabmouse),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Double click acts as \"use\"",
                                   &dclick_use),
                   TXT_TABLE_OVERFLOW_RIGHT,

                   TXT_NewSeparator("Mouse motion"),
                   TXT_NewLabel("Speed"),
                   TXT_NewSpinControl(&mouseSensitivity, 1, 256),
                   TXT_NewLabel("Acceleration"),
                   TXT_NewFloatSpinControl(&mouse_acceleration, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold"),
                   TXT_NewSpinControl(&mouse_threshold, 0, 32),

                   TXT_NewSeparator("Buttons"),
                   nullptr);
    }

    AddMouseControl(window, "Fire/Attack", &mousebfire);
    AddMouseControl(window, "Use", &mousebuse);

    TXT_AddWidget(window,
                  TXT_NewButton2("More controls...", ConfigExtraButtons, nullptr));
}

void BindMouseVariables()
{
    M_BindIntVariable("use_mouse",               &usemouse);
    M_BindIntVariable("novert",                  &novert);
    M_BindIntVariable("grabmouse",               &grabmouse);
    M_BindIntVariable("mouse_sensitivity",       &mouseSensitivity);
    M_BindIntVariable("mouse_threshold",         &mouse_threshold);
    M_BindFloatVariable("mouse_acceleration",    &mouse_acceleration);
    if (gamemission == doom) // [crispy]
    {
    M_BindIntVariable("mouse_sensitivity_x2",    &mouseSensitivity_x2);
    M_BindIntVariable("mouse_sensitivity_y",     &mouseSensitivity_y);
    M_BindIntVariable("mouse_threshold_y",       &mouse_threshold_y);
    M_BindFloatVariable("mouse_acceleration_y",  &mouse_acceleration_y);
    M_BindIntVariable("crispy_mouselook",        &crispy->mouselook);
    }
}
