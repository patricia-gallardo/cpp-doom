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

#include <SDL_joystick.h>

#include "doomkeys.hpp"
#include "i_joystick.hpp"
#include "i_system.hpp"
#include "joystick.hpp"
#include "m_controls.hpp"
#include "m_misc.hpp"

#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_joybinput.hpp"
#include "txt_label.hpp"
#include "txt_sdl.hpp"
#include "txt_utf8.hpp"
#include "txt_window.hpp"

#define JOYSTICK_INPUT_WIDTH 10

extern int joystick_physical_buttons[NUM_VIRTUAL_BUTTONS];

// Joystick button variables.
// The ordering of this array is important. We will always try to map
// each variable to the virtual button with the same array index. For
// example: joybfire should always be 0, and then we change
// joystick_physical_buttons[0] to point to the physical joystick
// button that the user wants to use for firing. We do this so that
// the menus work (the game code is hard coded to interpret
// button #0 = select menu item, button #1 = go back to previous menu).
static int * all_joystick_buttons[NUM_VIRTUAL_BUTTONS] = {
  &g_m_controls_globals->joybfire,
  &g_m_controls_globals->joybuse,
  &g_m_controls_globals->joybstrafe,
  &g_m_controls_globals->joybspeed,
  &g_m_controls_globals->joybstrafeleft,
  &g_m_controls_globals->joybstraferight,
  &g_m_controls_globals->joybprevweapon,
  &g_m_controls_globals->joybnextweapon,
  &g_m_controls_globals->joybjump,
  &g_m_controls_globals->joybmenu,
  &g_m_controls_globals->joybautomap,
};

static int PhysicalForVirtualButton(int vbutton) {
  if (vbutton < NUM_VIRTUAL_BUTTONS) {
    return joystick_physical_buttons[vbutton];
  } else {
    return vbutton;
  }
}

// Get the virtual button number for the given variable, ie. the
// variable's index in all_joystick_buttons[NUM_VIRTUAL_BUTTONS].
static int VirtualButtonForVariable(int * variable) {
  for (size_t i = 0; i < std::size(all_joystick_buttons); ++i) {
    if (variable == all_joystick_buttons[i]) {
      return static_cast<int>(i);
    }
  }

  I_Error("Couldn't find virtual button");
  return -1;
}

// Rearrange joystick button configuration to be in "canonical" form:
// each joyb* variable should have a value equal to its index in
// all_joystick_buttons[NUM_VIRTUAL_BUTTONS] above.
static void CanonicalizeButtons() {
  int new_mapping[NUM_VIRTUAL_BUTTONS] = {};

  for (size_t i = 0; i < std::size(all_joystick_buttons); ++i) {
    int vbutton = *all_joystick_buttons[i];

    // Don't remap the speed key if it's bound to "always run".
    // Also preserve "unbound" variables.
    if ((all_joystick_buttons[i] == &g_m_controls_globals->joybspeed && vbutton >= 20)
        || vbutton < 0) {
      new_mapping[i] = static_cast<int>(i);
    } else {
      new_mapping[i]           = PhysicalForVirtualButton(vbutton);
      *all_joystick_buttons[i] = static_cast<int>(i);
    }
  }

  for (int i = 0; i < NUM_VIRTUAL_BUTTONS; ++i) {
    joystick_physical_buttons[i] = new_mapping[i];
  }
}

// Check all existing buttons and clear any using the specified physical
// button.
static void ClearVariablesUsingButton(int physbutton) {
  for (auto & all_joystick_button : all_joystick_buttons) {
    int vbutton = *all_joystick_button;

    if (vbutton >= 0 && physbutton == PhysicalForVirtualButton(vbutton)) {
      *all_joystick_button = -1;
    }
  }
}

// Called in response to SDL events when the prompt window is open:

static int EventCallback(SDL_Event * event, void * uncast_joystick_input) {
  txt_joystick_input_t * joystick_input = reinterpret_cast<txt_joystick_input_t *>(uncast_joystick_input);

  // Got the joystick button press?

  if (event->type == SDL_JOYBUTTONDOWN) {
    int vbutton, physbutton;

    // Before changing anything, remap button configuration into
    // canonical form, to avoid conflicts.
    CanonicalizeButtons();

    vbutton    = VirtualButtonForVariable(joystick_input->variable);
    physbutton = event->jbutton.button;

    if (joystick_input->check_conflicts) {
      ClearVariablesUsingButton(physbutton);
    }

    // Set mapping.
    *joystick_input->variable          = vbutton;
    joystick_physical_buttons[vbutton] = physbutton;

    TXT_CloseWindow(joystick_input->prompt_window);
    return 1;
  }

  return 0;
}

// When the prompt window is closed, disable the event callback function;
// we are no longer interested in receiving notification of events.

static void PromptWindowClosed(void *, void * uncast_joystick) {
  auto * joystick = reinterpret_cast<SDL_Joystick *>(uncast_joystick);

  SDL_JoystickClose(joystick);
  TXT_SDL_SetEventCallback(nullptr, nullptr);
  SDL_JoystickEventState(SDL_DISABLE);
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

static void OpenErrorWindow() {
  TXT_MessageBox(nullptr, "Please configure a controller first!");
}

static void OpenPromptWindow(txt_joystick_input_t * joystick_input) {
  txt_window_t * window;
  SDL_Joystick * joystick;

  // Silently update when the shift button is held down.

  joystick_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

  if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
    return;
  }

  // Check the current joystick is valid

  joystick = SDL_JoystickOpen(joystick_index);

  if (joystick == nullptr) {
    OpenErrorWindow();
    return;
  }

  // Open the prompt window

  window = TXT_MessageBox(nullptr, "Press the new button on the controller...");

  TXT_SDL_SetEventCallback(EventCallback, joystick_input);
  TXT_SignalConnect(window, "closed", PromptWindowClosed, joystick);
  joystick_input->prompt_window = window;

  SDL_JoystickEventState(SDL_ENABLE);
}

static void TXT_JoystickInputSizeCalc(void * uncast_joystick_input) {
  auto * joystick_input = reinterpret_cast<txt_joystick_input_t *>(uncast_joystick_input);

  // All joystickinputs are the same size.

  joystick_input->widget.w = JOYSTICK_INPUT_WIDTH;
  joystick_input->widget.h = 1;
}

static void GetJoystickButtonDescription(int vbutton, char * buf, size_t buf_len) {
  M_snprintf(buf, buf_len, "BUTTON #%i", PhysicalForVirtualButton(vbutton) + 1);
}

static void TXT_JoystickInputDrawer(void * uncast_joystick_input) {
  auto * joystick_input = reinterpret_cast<txt_joystick_input_t *>(uncast_joystick_input);
  char   buf[20];

  if (*joystick_input->variable < 0) {
    M_StringCopy(buf, "(none)", sizeof(buf));
  } else {
    GetJoystickButtonDescription(*joystick_input->variable,
                                 buf,
                                 sizeof(buf));
  }

  TXT_SetWidgetBG(joystick_input);
  TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

  TXT_DrawString(buf);

  for (unsigned int i = TXT_UTF8_Strlen(buf); i < JOYSTICK_INPUT_WIDTH; ++i) {
    TXT_DrawString(" ");
  }
}

static void TXT_JoystickInputDestructor(void *) {
}

static int TXT_JoystickInputKeyPress(void * uncast_joystick_input, int key) {
  auto * joystick_input = reinterpret_cast<txt_joystick_input_t *>(uncast_joystick_input);

  if (key == KEY_ENTER) {
    // Open a window to prompt for the new joystick press

    OpenPromptWindow(joystick_input);

    return 1;
  }

  if (key == KEY_BACKSPACE || key == KEY_DEL) {
    *joystick_input->variable = -1;
  }

  return 0;
}

static void TXT_JoystickInputMousePress(void * uncast_widget,
                                        int,
                                        int,
                                        int b) {
  auto * widget = reinterpret_cast<txt_joystick_input_t *>(uncast_widget);

  // Clicking is like pressing enter

  if (b == TXT_MOUSE_LEFT) {
    TXT_JoystickInputKeyPress(widget, KEY_ENTER);
  }
}

txt_widget_class_t txt_joystick_input_class = {
  TXT_AlwaysSelectable,
  TXT_JoystickInputSizeCalc,
  TXT_JoystickInputDrawer,
  TXT_JoystickInputKeyPress,
  TXT_JoystickInputDestructor,
  TXT_JoystickInputMousePress,
  nullptr,
};

txt_joystick_input_t * TXT_NewJoystickInput(int * variable) {
  txt_joystick_input_t * joystick_input;

  joystick_input = static_cast<txt_joystick_input_t *>(malloc(sizeof(txt_joystick_input_t)));

  TXT_InitWidget(joystick_input, &txt_joystick_input_class);
  joystick_input->variable = variable;

  return joystick_input;
}
