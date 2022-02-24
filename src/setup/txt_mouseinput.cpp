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

#include "txt_mouseinput.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_label.hpp"
#include "txt_utf8.hpp"
#include "txt_window.hpp"

// eg. "BUTTON #10"
#define MOUSE_INPUT_WIDTH 10

static int MousePressCallback(txt_window_t * window,
                              int,
                              int,
                              int    b,
                              void * uncast_mouse_input) {
  auto * mouse_input = reinterpret_cast<txt_mouse_input_t *>(uncast_mouse_input);

  // Got the mouse press.  Save to the variable and close the window.

  *mouse_input->variable = b - TXT_MOUSE_BASE;

  if (mouse_input->check_conflicts) {
    TXT_EmitSignal(mouse_input, "set");
  }

  TXT_CloseWindow(window);

  return 1;
}

static void OpenPromptWindow(txt_mouse_input_t * mouse_input) {
  txt_window_t * window;

  // Silently update when the shift key is held down.
  mouse_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

  window = TXT_MessageBox(nullptr, "Press the new mouse button...");

  TXT_SetMouseListener(window, MousePressCallback, mouse_input);
}

static void TXT_MouseInputSizeCalc(void * uncast_mouse_input) {
  auto * mouse_input = reinterpret_cast<txt_mouse_input_t *>(uncast_mouse_input);

  // All mouseinputs are the same size.

  mouse_input->widget.w = MOUSE_INPUT_WIDTH;
  mouse_input->widget.h = 1;
}

static void GetMouseButtonDescription(int button, char * buf, size_t buf_len) {
  switch (button) {
  case 0:
    M_StringCopy(buf, "LEFT", buf_len);
    break;
  case 1:
    M_StringCopy(buf, "RIGHT", buf_len);
    break;
  case 2:
    M_StringCopy(buf, "MID", buf_len);
    break;
  default:
    M_snprintf(buf, buf_len, "BUTTON #%i", button + 1);
    break;
  }
}

static void TXT_MouseInputDrawer(void * uncast_mouse_input) {
  auto * mouse_input = reinterpret_cast<txt_mouse_input_t *>(uncast_mouse_input);
  char   buf[20];

  if (*mouse_input->variable < 0) {
    M_StringCopy(buf, "(none)", sizeof(buf));
  } else {
    GetMouseButtonDescription(*mouse_input->variable, buf, sizeof(buf));
  }

  TXT_SetWidgetBG(mouse_input);
  TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

  TXT_DrawString(buf);

  for (unsigned int i = TXT_UTF8_Strlen(buf); i < MOUSE_INPUT_WIDTH; ++i) {
    TXT_DrawString(" ");
  }
}

static void TXT_MouseInputDestructor(void *) {
}

static int TXT_MouseInputKeyPress(void * uncast_mouse_input, int key) {
  auto * mouse_input = reinterpret_cast<txt_mouse_input_t *>(uncast_mouse_input);

  if (key == KEY_ENTER) {
    // Open a window to prompt for the new mouse press

    OpenPromptWindow(mouse_input);

    return 1;
  }

  if (key == KEY_BACKSPACE || key == KEY_DEL) {
    *mouse_input->variable = -1;
  }

  return 0;
}

static void TXT_MouseInputMousePress(void * uncast_widget, int, int, int b) {
  auto * widget = reinterpret_cast<txt_mouse_input_t *>(uncast_widget);

  // Clicking is like pressing enter

  if (b == TXT_MOUSE_LEFT) {
    TXT_MouseInputKeyPress(widget, KEY_ENTER);
  }
}

txt_widget_class_t txt_mouse_input_class = {
  TXT_AlwaysSelectable,
  TXT_MouseInputSizeCalc,
  TXT_MouseInputDrawer,
  TXT_MouseInputKeyPress,
  TXT_MouseInputDestructor,
  TXT_MouseInputMousePress,
  nullptr,
};

txt_mouse_input_t * TXT_NewMouseInput(int * variable) {
  txt_mouse_input_t * mouse_input;

  mouse_input = static_cast<txt_mouse_input_t *>(malloc(sizeof(txt_mouse_input_t)));

  TXT_InitWidget(mouse_input, &txt_mouse_input_class);
  mouse_input->variable = variable;

  return mouse_input;
}
