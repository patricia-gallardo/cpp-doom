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

#include <cctype>
#include <cstdlib>
#include <cstring>

#include "doomkeys.hpp"

#include "txt_inputbox.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_main.hpp"
#include "txt_utf8.hpp"
#include "memory.hpp"

extern txt_widget_class_t txt_inputbox_class;
extern txt_widget_class_t txt_int_inputbox_class;

static void SetBufferFromValue(txt_inputbox_t *inputbox) {
  if (inputbox->widget.widget_class == &txt_inputbox_class) {
    char **value = reinterpret_cast<char **>(inputbox->value);

    if (*value != nullptr) {
      TXT_StringCopy(inputbox->buffer, *value, inputbox->size);
    } else {
      TXT_StringCopy(inputbox->buffer, "", inputbox->buffer_len);
    }
  } else if (inputbox->widget.widget_class == &txt_int_inputbox_class) {
    int *value = reinterpret_cast<int *>(inputbox->value);
    TXT_snprintf(inputbox->buffer, inputbox->buffer_len, "%i", *value);
  }
}

static void StartEditing(txt_inputbox_t *inputbox) {
  // Integer input boxes start from an empty buffer:

  if (inputbox->widget.widget_class == &txt_int_inputbox_class) {
    TXT_StringCopy(inputbox->buffer, "", inputbox->buffer_len);
  } else {
    SetBufferFromValue(inputbox);
  }

  // Switch to text input mode so we get shifted input.
  TXT_SetInputMode(TXT_INPUT_TEXT);
  inputbox->editing = 1;
}

static void StopEditing(txt_inputbox_t *inputbox) {
  if (inputbox->editing) {
    // Switch back to normal input mode.
    TXT_SetInputMode(TXT_INPUT_NORMAL);
    inputbox->editing = 0;
  }
}

static void FinishEditing(txt_inputbox_t *inputbox) {
  if (!inputbox->editing) {
    return;
  }

  // Save the new value back to the variable.

  if (inputbox->widget.widget_class == &txt_inputbox_class) {
    char **value = reinterpret_cast<char **>(inputbox->value);
    free(*value);
    *value = strdup(inputbox->buffer);
  } else if (inputbox->widget.widget_class == &txt_int_inputbox_class) {
    int *pInt = static_cast<int *>(inputbox->value);
    *pInt     = std::atoi(inputbox->buffer);
  }

  TXT_EmitSignal(&inputbox->widget, "changed");

  StopEditing(inputbox);
}

static void TXT_InputBoxSizeCalc(void *uncast_inputbox) {
  auto *inputbox = reinterpret_cast<txt_inputbox_t *>(uncast_inputbox);
  ;

  // Enough space for the box + cursor

  inputbox->widget.w = inputbox->size + 1;
  inputbox->widget.h = 1;
}

static void TXT_InputBoxDrawer(void *uncast_inputbox) {
  auto *inputbox = reinterpret_cast<txt_inputbox_t *>(uncast_inputbox);
  ;

  int          focused = inputbox->widget.focused;
  unsigned int w       = inputbox->widget.w;

  // Select the background color based on whether we are currently
  // editing, and if not, whether the widget is focused.

  if (inputbox->editing && focused) {
    TXT_BGColor(TXT_COLOR_BLACK, 0);
  } else {
    TXT_SetWidgetBG(inputbox);
  }

  if (!inputbox->editing) {
    // If not editing, use the current value from inputbox->value.

    SetBufferFromValue(inputbox);
  }

  // If string size exceeds the widget's width, show only the end.

  unsigned int chars = 0;
  if (TXT_UTF8_Strlen(inputbox->buffer) > w - 1) {
    TXT_DrawCodePageString("\xae");
    TXT_DrawString(
        TXT_UTF8_SkipChars(inputbox->buffer,
                           TXT_UTF8_Strlen(inputbox->buffer) - w + 2));
    chars = w - 1;
  } else {
    TXT_DrawString(inputbox->buffer);
    chars = TXT_UTF8_Strlen(inputbox->buffer);
  }

  if (chars < w && inputbox->editing && focused) {
    TXT_BGColor(TXT_COLOR_BLACK, 1);
    TXT_DrawString("_");
    ++chars;
  }

  for (unsigned int i = chars; i < w; ++i) {
    TXT_DrawString(" ");
  }
}

static void TXT_InputBoxDestructor(void *uncast_inputbox) {
  auto *inputbox = reinterpret_cast<txt_inputbox_t *>(uncast_inputbox);
  ;

  StopEditing(inputbox);
  free(inputbox->buffer);
}

static void Backspace(txt_inputbox_t *inputbox) {
  unsigned int len;
  char        *p;

  len = TXT_UTF8_Strlen(inputbox->buffer);

  if (len > 0) {
    p  = TXT_UTF8_SkipChars(inputbox->buffer, len - 1);
    *p = '\0';
  }
}

static void AddCharacter(txt_inputbox_t *inputbox, int key) {
  char *end, *p;

  if (TXT_UTF8_Strlen(inputbox->buffer) < inputbox->size) {
    // Add character to the buffer

    end = inputbox->buffer + strlen(inputbox->buffer);
    p   = TXT_EncodeUTF8(end, static_cast<unsigned int>(key));
    *p  = '\0';
  }
}

static int TXT_InputBoxKeyPress(void *uncast_inputbox, int key) {
  auto *inputbox = reinterpret_cast<txt_inputbox_t *>(uncast_inputbox);
  ;
  unsigned int c;

  if (!inputbox->editing) {
    if (key == KEY_ENTER) {
      StartEditing(inputbox);
      return 1;
    }

    // Backspace or delete erases the contents of the box.

    if ((key == KEY_DEL || key == KEY_BACKSPACE)
        && inputbox->widget.widget_class == &txt_inputbox_class) {
      char **value = reinterpret_cast<char **>(inputbox->value);
      free(*value);
      *value = strdup("");
    }

    return 0;
  }

  if (key == KEY_ENTER) {
    FinishEditing(inputbox);
  }

  if (key == KEY_ESCAPE) {
    StopEditing(inputbox);
  }

  if (key == KEY_BACKSPACE) {
    Backspace(inputbox);
  }

  c = TXT_KEY_TO_UNICODE(static_cast<unsigned int>(key));

  // Add character to the buffer, but only if it's a printable character
  // that we can represent on the screen.
  if (isprint(static_cast<int>(c))
      || (c >= 128 && TXT_UnicodeCharacter(c) >= 0)) {
    AddCharacter(inputbox, static_cast<int>(c));
  }

  return 1;
}

static void TXT_InputBoxMousePress(void *uncast_inputbox,
                                   int, int, int b) {
  auto *inputbox = reinterpret_cast<txt_inputbox_t *>(uncast_inputbox);
  ;

  if (b == TXT_MOUSE_LEFT) {
    // Make mouse clicks start editing the box

    if (!inputbox->editing) {
      // Send a simulated keypress to start editing

      TXT_WidgetKeyPress(inputbox, KEY_ENTER);
    }
  }
}

static void TXT_InputBoxFocused(void *uncast_inputbox, int focused) {
  auto *inputbox = reinterpret_cast<txt_inputbox_t *>(uncast_inputbox);
  ;

  // Stop editing when we lose focus.

  if (inputbox->editing && !focused) {
    FinishEditing(inputbox);
  }
}

txt_widget_class_t txt_inputbox_class = {
  TXT_AlwaysSelectable,
  TXT_InputBoxSizeCalc,
  TXT_InputBoxDrawer,
  TXT_InputBoxKeyPress,
  TXT_InputBoxDestructor,
  TXT_InputBoxMousePress,
  nullptr,
  TXT_InputBoxFocused,
};

txt_widget_class_t txt_int_inputbox_class = {
  TXT_AlwaysSelectable,
  TXT_InputBoxSizeCalc,
  TXT_InputBoxDrawer,
  TXT_InputBoxKeyPress,
  TXT_InputBoxDestructor,
  TXT_InputBoxMousePress,
  nullptr,
  TXT_InputBoxFocused,
};

static txt_inputbox_t *NewInputBox(txt_widget_class_t *widget_class,
                                   void *value, int size) {
  auto *inputbox = create_struct<txt_inputbox_t>();

  TXT_InitWidget(inputbox, widget_class);
  inputbox->value = value;
  inputbox->size  = static_cast<unsigned int>(size);
  // 'size' is the maximum number of characters that can be entered,
  // but for a UTF-8 string, each character can take up to four
  // characters.
  inputbox->buffer_len = static_cast<size_t>(size * 4 + 1);
  inputbox->buffer     = static_cast<char *>(malloc(inputbox->buffer_len));
  inputbox->editing    = 0;

  return inputbox;
}

txt_inputbox_t *TXT_NewInputBox(char **value, int size) {
  return NewInputBox(&txt_inputbox_class, value, size);
}

txt_inputbox_t *TXT_NewIntInputBox(int *value, int size) {
  return NewInputBox(&txt_int_inputbox_class, value, size);
}
