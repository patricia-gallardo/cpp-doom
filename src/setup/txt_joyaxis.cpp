//
// Copyright(C) 2014 Simon Howard
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

#include "SDL.h"

#include "joystick.hpp"
#include "i_joystick.hpp"
#include "i_system.hpp"
#include "m_controls.hpp"
#include "m_misc.hpp"

#include "textscreen.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_joyaxis.hpp"
#include "txt_utf8.hpp"

#define JOYSTICK_AXIS_WIDTH 20

static const char *CalibrationLabel(txt_joystick_axis_t *joystick_axis)
{
    switch (joystick_axis->config_stage)
    {
        case CONFIG_CENTER:
            return "Center the D-pad or joystick,\n"
                   "and press a button.";

        case CONFIG_STAGE1:
            if (joystick_axis->dir == JOYSTICK_AXIS_VERTICAL)
            {
                return "Push the D-pad or joystick up,\n"
                       "and press the button.";
            }
            else
            {
                return "Push the D-pad or joystick to the\n"
                       "left, and press the button.";
            }

        case CONFIG_STAGE2:
            if (joystick_axis->dir == JOYSTICK_AXIS_VERTICAL)
            {
                return "Push the D-pad or joystick down,\n"
                       "and press the button.";
            }
            else
            {
                return "Push the D-pad or joystick to the\n"
                       "right, and press the button.";
            }
    }

    return nullptr;
}

static void SetCalibrationLabel(txt_joystick_axis_t *joystick_axis)
{
    TXT_SetLabel(joystick_axis->config_label, CalibrationLabel(joystick_axis));
}

// Search all axes on joystick being configured; find a button that is
// pressed (other than the calibrate button). Returns the button number.

static int FindPressedAxisButton(txt_joystick_axis_t *joystick_axis)
{
    int i;

    for (i = 0; i < SDL_JoystickNumButtons(joystick_axis->joystick); ++i)
    {
        if (i == joystick_axis->config_button)
        {
            continue;
        }

        if (SDL_JoystickGetButton(joystick_axis->joystick, i))
        {
            return i;
        }
    }

    return -1;
}

// Look for a hat that isn't centered. Returns the encoded hat axis.

static int FindUncenteredHat(SDL_Joystick *joystick, int *axis_invert)
{
    int i, hatval;

    for (i = 0; i < SDL_JoystickNumHats(joystick); ++i)
    {
        hatval = SDL_JoystickGetHat(joystick, i);

        switch (hatval)
        {
            case SDL_HAT_LEFT:
            case SDL_HAT_RIGHT:
                *axis_invert = hatval != SDL_HAT_LEFT;
                return CREATE_HAT_AXIS(i, HAT_AXIS_HORIZONTAL);

            case SDL_HAT_UP:
            case SDL_HAT_DOWN:
                *axis_invert = hatval != SDL_HAT_UP;
                return CREATE_HAT_AXIS(i, HAT_AXIS_VERTICAL);

            // If the hat is centered, or is not pointing in a
            // definite direction, then ignore it. We don't accept
            // the hat being pointed to the upper-left for example,
            // because it's ambiguous.
            case SDL_HAT_CENTERED:
            default:
                break;
        }
    }

    // None found.
    return -1;
}

static bool CalibrateAxis(txt_joystick_axis_t *joystick_axis)
{
    int best_axis;
    int best_value;
    int best_invert;
    Sint16 axis_value;
    int i;

    // Check all axes to find which axis has the largest value.  We test
    // for one axis at a time, so eg. when we prompt to push the joystick 
    // left, whichever axis has the largest value is the left axis.

    best_axis = 0;
    best_value = 0;
    best_invert = 0;

    for (i = 0; i < SDL_JoystickNumAxes(joystick_axis->joystick); ++i)
    {
        axis_value = SDL_JoystickGetAxis(joystick_axis->joystick, i);

        if (joystick_axis->bad_axis[i])
        {
            continue;
        }

        if (std::abs(axis_value) > best_value)
        {
            best_value = std::abs(axis_value);
            best_invert = axis_value > 0;
            best_axis = i;
        }
    }

    // Did we find one axis that had a significant value?

    if (best_value > 32768 / 4)
    {
        // Save the best values we have found

        *joystick_axis->axis = best_axis;
        *joystick_axis->invert = best_invert;
        return true;
    }

    // Otherwise, maybe this is a "button axis", like the PS3 SIXAXIS
    // controller that exposes the D-pad as four individual buttons.
    // Search for a button.

    i = FindPressedAxisButton(joystick_axis);

    if (i >= 0)
    {
        *joystick_axis->axis = CREATE_BUTTON_AXIS(i, 0);
        *joystick_axis->invert = 0;
        return true;
    }

    // Maybe it's a D-pad that is presented as a hat. This sounds weird
    // but gamepads like this really do exist; an example is the
    // Nyko AIRFLO Ex.

    i = FindUncenteredHat(joystick_axis->joystick, joystick_axis->invert);

    if (i >= 0)
    {
        *joystick_axis->axis = i;
        return true;
    }

    // User pressed the button without pushing the joystick anywhere.
    return false;
}

static bool SetButtonAxisPositive(txt_joystick_axis_t *joystick_axis)
{
    int button;

    button = FindPressedAxisButton(joystick_axis);

    if (button >= 0)
    {
        *joystick_axis->axis |= CREATE_BUTTON_AXIS(0, button);
        return true;
    }

    return false;
}

static void IdentifyBadAxes(txt_joystick_axis_t *joystick_axis)
{
    int i, val;

    free(joystick_axis->bad_axis);

    joystick_axis->bad_axis
        = static_cast<bool *>(calloc(SDL_JoystickNumAxes(joystick_axis->joystick),
        sizeof(bool)));

    // Look for uncentered axes.

    for (i = 0; i < SDL_JoystickNumAxes(joystick_axis->joystick); ++i)
    {
        val = SDL_JoystickGetAxis(joystick_axis->joystick, i);

        joystick_axis->bad_axis[i] = std::abs(val) > (32768 / 5);

        if (joystick_axis->bad_axis[i])
        {
            printf("Ignoring uncentered joystick axis #%i: %i\n", i, val);
        }
    }
}

static int NextCalibrateStage(txt_joystick_axis_t *joystick_axis)
{
    switch (joystick_axis->config_stage)
    {
        case CONFIG_CENTER:
            return CONFIG_STAGE1;

        // After pushing to the left, there are two possibilities:
        // either it is a button axis, in which case we need to find
        // the other button, or we can just move on to the next axis.
        case CONFIG_STAGE1:
            if (IS_BUTTON_AXIS(*joystick_axis->axis))
            {
                return CONFIG_STAGE2;
            }
            else
            {
                return CONFIG_CENTER;
            }

        case CONFIG_STAGE2:
            return CONFIG_CENTER;
    }

    return -1;
}

static int EventCallback(SDL_Event *event, void *uncast_joystick_axis)
{
    auto *joystick_axis = reinterpret_cast<txt_joystick_axis_t *>(uncast_joystick_axis);
    bool advance;

    if (event->type != SDL_JOYBUTTONDOWN)
    {
        return 0;
    }

    // At this point, we have a button press.
    // In the first "center" stage, we're just trying to work out which
    // joystick is being configured and which button the user is pressing.
    if (joystick_axis->config_stage == CONFIG_CENTER)
    {
        joystick_axis->config_button = event->jbutton.button;
        IdentifyBadAxes(joystick_axis);

        // Advance to next stage.
        joystick_axis->config_stage = CONFIG_STAGE1;
        SetCalibrationLabel(joystick_axis);

        return 1;
    }

    // In subsequent stages, the user is asked to push in a specific
    // direction and press the button. They must push the same button
    // as they did before; this is necessary to support button axes.
    if (event->jbutton.which == SDL_JoystickInstanceID(joystick_axis->joystick)
     && event->jbutton.button == joystick_axis->config_button)
    {
        switch (joystick_axis->config_stage)
        {
            default:
            case CONFIG_STAGE1:
                advance = CalibrateAxis(joystick_axis);
                break;

            case CONFIG_STAGE2:
                advance = SetButtonAxisPositive(joystick_axis);
                break;
        }

        // Advance to the next calibration stage?

        if (advance)
        {
            joystick_axis->config_stage = static_cast<txt_joystick_axis_stage_t>(NextCalibrateStage(joystick_axis));
            SetCalibrationLabel(joystick_axis);

            // Finished?
            if (joystick_axis->config_stage == CONFIG_CENTER)
            {
                TXT_CloseWindow(joystick_axis->config_window);

                if (joystick_axis->callback != nullptr)
                {
                    joystick_axis->callback();
                }
            }

            return 1;
        }
    }

    return 0;
}

static void CalibrateWindowClosed(void *, void *uncast_joystick_axis)
{
    auto *joystick_axis = reinterpret_cast<txt_joystick_axis_t *>(uncast_joystick_axis);

    free(joystick_axis->bad_axis);
    joystick_axis->bad_axis = nullptr;

    SDL_JoystickClose(joystick_axis->joystick);
    SDL_JoystickEventState(SDL_DISABLE);
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    TXT_SDL_SetEventCallback(nullptr, nullptr);
}

void TXT_ConfigureJoystickAxis(txt_joystick_axis_t *joystick_axis,
                               int using_button,
                               txt_joystick_axis_callback_t callback)
{
    // Open the joystick first.
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
        return;
    }

    joystick_axis->joystick = SDL_JoystickOpen(joystick_index);
    if (joystick_axis->joystick == nullptr)
    {
        TXT_MessageBox(nullptr, "Please configure a controller first!");
        return;
    }

    SDL_JoystickEventState(SDL_ENABLE);

    // Build the prompt window.

    joystick_axis->config_window
        = TXT_NewWindow("Gamepad/Joystick calibration");
    TXT_AddWidgets(joystick_axis->config_window,
                   TXT_NewStrut(0, 1),
                   joystick_axis->config_label = TXT_NewLabel(""),
                   TXT_NewStrut(0, 1),
                   nullptr);

    TXT_SetWindowAction(joystick_axis->config_window, TXT_HORIZ_LEFT, nullptr);
    TXT_SetWindowAction(joystick_axis->config_window, TXT_HORIZ_CENTER,
                        TXT_NewWindowAbortAction(joystick_axis->config_window));
    TXT_SetWindowAction(joystick_axis->config_window, TXT_HORIZ_RIGHT, nullptr);
    TXT_SetWidgetAlign(joystick_axis->config_window, TXT_HORIZ_CENTER);

    if (using_button >= 0)
    {
        joystick_axis->config_stage = CONFIG_STAGE1;
        joystick_axis->config_button = using_button;
        IdentifyBadAxes(joystick_axis);
    }
    else
    {
        joystick_axis->config_stage = CONFIG_CENTER;
    }

    SetCalibrationLabel(joystick_axis);

    // Close the joystick and shut down joystick subsystem when the window
    // is closed.
    TXT_SignalConnect(joystick_axis->config_window, "closed",
                      CalibrateWindowClosed, joystick_axis);

    TXT_SDL_SetEventCallback(EventCallback, joystick_axis);

    // When successfully calibrated, invoke this callback:
    joystick_axis->callback = callback;
}

static void TXT_JoystickAxisSizeCalc(void *uncast_joystick_axis)
{
    auto *joystick_axis = reinterpret_cast<txt_joystick_axis_t *>(uncast_joystick_axis);

    // All joystickinputs are the same size.

    joystick_axis->widget.w = JOYSTICK_AXIS_WIDTH;
    joystick_axis->widget.h = 1;
}

static void TXT_JoystickAxisDrawer(void *uncast_joystick_axis)
{
    auto *joystick_axis = reinterpret_cast<txt_joystick_axis_t *>(uncast_joystick_axis);
    char buf[JOYSTICK_AXIS_WIDTH + 1];
    int i;

    if (*joystick_axis->axis < 0)
    {
        M_StringCopy(buf, "(none)", sizeof(buf));
    }
    else if (IS_BUTTON_AXIS(*joystick_axis->axis))
    {
        int neg, pos;

        neg = BUTTON_AXIS_NEG(*joystick_axis->axis);
        pos = BUTTON_AXIS_POS(*joystick_axis->axis);
        M_snprintf(buf, sizeof(buf), "BUTTONS #%i+#%i", neg, pos);
    }
    else if (IS_HAT_AXIS(*joystick_axis->axis))
    {
        int hat, dir;

        hat = HAT_AXIS_HAT(*joystick_axis->axis);
        dir = HAT_AXIS_DIRECTION(*joystick_axis->axis);

        M_snprintf(buf, sizeof(buf), "HAT #%i (%s)", hat,
                   dir == HAT_AXIS_HORIZONTAL ? "horizontal" : "vertical");
    }
    else
    {
        M_snprintf(buf, sizeof(buf), "AXIS #%i", *joystick_axis->axis);
    }

    TXT_SetWidgetBG(joystick_axis);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_DrawString(buf);

    for (i = TXT_UTF8_Strlen(buf); i < joystick_axis->widget.w; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_JoystickAxisDestructor(void *)
{
}

static int TXT_JoystickAxisKeyPress(void *uncast_joystick_axis, int key)
{
    auto *joystick_axis = reinterpret_cast<txt_joystick_axis_t *>(uncast_joystick_axis);

    if (key == KEY_ENTER)
    {
        TXT_ConfigureJoystickAxis(joystick_axis, -1, nullptr);
        return 1;
    }

    if (key == KEY_BACKSPACE || key == KEY_DEL)
    {
        *joystick_axis->axis = -1;
    }

    return 0;
}

static void TXT_JoystickAxisMousePress(void *uncast_widget, int, int, int b)
{
    auto *widget = reinterpret_cast<txt_joystick_axis_t *>(uncast_widget);

    // Clicking is like pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_JoystickAxisKeyPress(widget, KEY_ENTER);
    }
}

txt_widget_class_t txt_joystick_axis_class =
{
    TXT_AlwaysSelectable,
    TXT_JoystickAxisSizeCalc,
    TXT_JoystickAxisDrawer,
    TXT_JoystickAxisKeyPress,
    TXT_JoystickAxisDestructor,
    TXT_JoystickAxisMousePress,
    nullptr,
};

txt_joystick_axis_t *TXT_NewJoystickAxis(int *axis, int *invert,
                                         txt_joystick_axis_direction_t dir)
{
    txt_joystick_axis_t *joystick_axis;

    joystick_axis = static_cast<txt_joystick_axis_t *>(malloc(sizeof(txt_joystick_axis_t)));

    TXT_InitWidget(joystick_axis, &txt_joystick_axis_class);
    joystick_axis->axis = axis;
    joystick_axis->invert = invert;
    joystick_axis->dir = dir;
    joystick_axis->bad_axis = nullptr;

    return joystick_axis;
}

