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

#include "doomkeys.hpp"
#include "m_misc.hpp"

#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_keyinput.hpp"
#include "txt_label.hpp"
#include "txt_utf8.hpp"
#include "txt_window.hpp"

constexpr auto KEY_INPUT_WIDTH = 8;

static int KeyPressCallback(txt_window_t * window, int key, void * uncast_key_input) {
  auto * key_input = reinterpret_cast<txt_key_input_t *>(uncast_key_input);

  if (key != KEY_ESCAPE) {
    // Got the key press. Save to the variable and close the window.

    *key_input->variable = key;

    if (key_input->check_conflicts) {
      TXT_EmitSignal(key_input, "set");
    }

    TXT_CloseWindow(window);

    // Return to normal input mode now that we have the key.
    TXT_SetInputMode(TXT_INPUT_NORMAL);

    return 1;
  } else {
    return 0;
  }
}

static void ReleaseGrab(void *, void *) {
  // SDL2-TODO: Needed?
  // SDL_WM_GrabInput(SDL_GRAB_OFF);
}

static void OpenPromptWindow(txt_key_input_t * key_input) {
  txt_window_t * window;

  // Silently update when the shift button is held down.

  key_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

  window = TXT_MessageBox(nullptr, "Press the new key...");

  TXT_SetKeyListener(window, KeyPressCallback, key_input);

  // Switch to raw input mode while we're grabbing the key.
  TXT_SetInputMode(TXT_INPUT_RAW);

  // Grab input while reading the key.  On Windows Mobile
  // handheld devices, the hardware keypresses are only
  // detected when input is grabbed.

  // SDL2-TODO: Needed?
  // SDL_WM_GrabInput(SDL_GRAB_ON);
  TXT_SignalConnect(window, "closed", ReleaseGrab, nullptr);
}

static void TXT_KeyInputSizeCalc(void * uncast_key_input) {
  auto * key_input = reinterpret_cast<txt_key_input_t *>(uncast_key_input);

  // All keyinputs are the same size.

  key_input->widget.w = KEY_INPUT_WIDTH;
  key_input->widget.h = 1;
}

static void TXT_KeyInputDrawer(void * uncast_key_input) {
  auto * key_input = reinterpret_cast<txt_key_input_t *>(uncast_key_input);
  char   buf[20];

  if (*key_input->variable == 0) {
    M_StringCopy(buf, "(none)", sizeof(buf));
  } else {
    TXT_GetKeyDescription(*key_input->variable, buf, sizeof(buf));
  }

  TXT_SetWidgetBG(key_input);
  TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

  TXT_DrawString(buf);

  for (unsigned int i = TXT_UTF8_Strlen(buf); i < KEY_INPUT_WIDTH; ++i) {
    TXT_DrawString(" ");
  }
}

static void TXT_KeyInputDestructor(void *) {
}

static int TXT_KeyInputKeyPress(void * uncast_key_input, int key) {
  auto * key_input = reinterpret_cast<txt_key_input_t *>(uncast_key_input);

  if (key == KEY_ENTER) {
    // Open a window to prompt for the new key press

    OpenPromptWindow(key_input);

    return 1;
  }

  if (key == KEY_BACKSPACE || key == KEY_DEL) {
    *key_input->variable = 0;
  }

  return 0;
}

static void TXT_KeyInputMousePress(void * uncast_widget, int, int, int b) {
  auto * widget = reinterpret_cast<txt_key_input_t *>(uncast_widget);

  // Clicking is like pressing enter

  if (b == TXT_MOUSE_LEFT) {
    TXT_KeyInputKeyPress(widget, KEY_ENTER);
  }
}

txt_widget_class_t txt_key_input_class = {
  TXT_AlwaysSelectable,
  TXT_KeyInputSizeCalc,
  TXT_KeyInputDrawer,
  TXT_KeyInputKeyPress,
  TXT_KeyInputDestructor,
  TXT_KeyInputMousePress,
  nullptr,
};

txt_key_input_t * TXT_NewKeyInput(int * variable) {
  txt_key_input_t * key_input;

  key_input = static_cast<txt_key_input_t *>(malloc(sizeof(txt_key_input_t)));

  TXT_InitWidget(key_input, &txt_key_input_class);
  key_input->variable = variable;

  return key_input;
}
