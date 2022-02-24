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

#include "doomkeys.hpp"

#include "txt_button.hpp"
#include "txt_gui.hpp"
#include "txt_main.hpp"
#include "txt_utf8.hpp"
#include "memory.hpp"

static void TXT_ButtonSizeCalc(void *uncast_button) {
  auto *button = reinterpret_cast<txt_button_t *>(uncast_button);

  button->widget.w = TXT_UTF8_Strlen(button->label);
  button->widget.h = 1;
}

static void TXT_ButtonDrawer(void *uncast_button) {
  auto *button = reinterpret_cast<txt_button_t *>(uncast_button);

  TXT_SetWidgetBG(button);

  TXT_DrawString(button->label);

  for (unsigned int i = TXT_UTF8_Strlen(button->label); i < button->widget.w; ++i) {
    TXT_DrawString(" ");
  }
}

static void TXT_ButtonDestructor(void *uncast_button) {
  auto *button = reinterpret_cast<txt_button_t *>(uncast_button);

  free(button->label);
}

static int TXT_ButtonKeyPress(void *uncast_button, int key) {
  auto *button = reinterpret_cast<txt_button_t *>(uncast_button);

  if (key == KEY_ENTER) {
    TXT_EmitSignal(button, "pressed");
    return 1;
  }

  return 0;
}

static void TXT_ButtonMousePress(void *uncast_button, int, int, int b) {
  auto *button = reinterpret_cast<txt_button_t *>(uncast_button);

  if (b == TXT_MOUSE_LEFT) {
    // Equivalent to pressing enter

    TXT_ButtonKeyPress(button, KEY_ENTER);
  }
}

txt_widget_class_t txt_button_class = {
  TXT_AlwaysSelectable,
  TXT_ButtonSizeCalc,
  TXT_ButtonDrawer,
  TXT_ButtonKeyPress,
  TXT_ButtonDestructor,
  TXT_ButtonMousePress,
  nullptr,
};

void TXT_SetButtonLabel(txt_button_t *button, const char *label) {
  free(button->label);
  button->label = strdup(label);
}

txt_button_t *TXT_NewButton(const char *label) {
  auto *button = create_struct<txt_button_t>();

  TXT_InitWidget(button, &txt_button_class);
  button->label = strdup(label);

  return button;
}

// Button with a callback set automatically

txt_button_t *TXT_NewButton2(const char *label, TxtWidgetSignalFunc func, void *user_data) {
  txt_button_t *button = TXT_NewButton(label);

  TXT_SignalConnect(button, "pressed", func, user_data);

  return button;
}
