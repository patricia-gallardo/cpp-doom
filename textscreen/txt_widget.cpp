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

#include "txt_desktop.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_widget.hpp"
#include <vector>
#include <memory>

struct txt_callback_t {
  char *              signal_name;
  TxtWidgetSignalFunc func;
  void *              user_data;
};

struct [[maybe_unused]] txt_callback_table_s {
  std::vector<txt_callback_t> callbacks;

  ~txt_callback_table_s() {
    for (auto & callback : callbacks) {
      free(callback.signal_name);
    }
  }
};

std::shared_ptr<txt_callback_table_t> TXT_NewCallbackTable() {
  return std::make_shared<txt_callback_table_t>();
}

void TXT_InitWidget(void * uncast_widget, txt_widget_class_t * widget_class) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  widget->widget_class   = widget_class;
  widget->callback_table = TXT_NewCallbackTable();
  widget->parent         = nullptr;

  // Not focused until we hear otherwise.

  widget->focused = 0;

  // Visible by default.

  widget->visible = 1;

  // Align left by default

  widget->align = TXT_HORIZ_LEFT;
}

void TXT_SignalConnect(void *              uncast_widget,
                       const char *        signal_name,
                       TxtWidgetSignalFunc func,
                       void *              user_data) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  auto & table = widget->callback_table;

  // Add a new callback to the table

  auto & callback      = table->callbacks.emplace_back();
  callback.signal_name = strdup(signal_name);
  callback.func        = func;
  callback.user_data   = user_data;
}

void TXT_EmitSignal(void * uncast_widget, const char * signal_name) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  auto table = widget->callback_table;

  // Don't destroy the table while we're searching through it
  // (one of the callbacks may destroy this window)

  // Search the table for all callbacks with this name and invoke
  // the functions.

  for (auto & callback : table->callbacks) {
    if (!strcmp(callback.signal_name, signal_name)) {
      callback.func(widget, callback.user_data);
    }
  }
}

void TXT_CalcWidgetSize(void * uncast_widget) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  widget->widget_class->size_calc(widget);
}

void TXT_DrawWidget(void * uncast_widget) {
  auto *             widget = reinterpret_cast<txt_widget_t *>(uncast_widget);
  txt_saved_colors_t colors {};

  // The drawing function might change the fg/bg colors,
  // so make sure we restore them after it's done.

  TXT_SaveColors(&colors);

  // For convenience...

  TXT_GotoXY(widget->x, widget->y);

  // Call drawer method

  widget->widget_class->drawer(widget);

  TXT_RestoreColors(&colors);
}

void TXT_DestroyWidget(void * uncast_widget) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  widget->widget_class->destructor(widget);
  free(widget);
}

int TXT_WidgetKeyPress(void * uncast_widget, int key) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  if (widget->widget_class->key_press != nullptr) {
    return widget->widget_class->key_press(widget, key);
  }

  return 0;
}

void TXT_SetWidgetFocus(void * uncast_widget, int focused) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  if (widget == nullptr) {
    return;
  }

  if (widget->focused != focused) {
    widget->focused = focused;

    if (widget->widget_class->focus_change != nullptr) {
      widget->widget_class->focus_change(widget, focused);
    }
  }
}

void TXT_SetWidgetAlign(void * uncast_widget, txt_horiz_align_t horiz_align) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  widget->align = horiz_align;
}

void TXT_WidgetMousePress(void * uncast_widget, int x, int y, int b) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  if (widget->widget_class->mouse_press != nullptr) {
    widget->widget_class->mouse_press(widget, x, y, b);
  }
}

void TXT_LayoutWidget(void * uncast_widget) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  if (widget->widget_class->layout != nullptr) {
    widget->widget_class->layout(widget);
  }
}

int TXT_AlwaysSelectable(void *) {
  return 1;
}

int TXT_NeverSelectable(void *) {
  return 0;
}

int TXT_SelectableWidget(void * uncast_widget) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  if (widget->widget_class->selectable != nullptr) {
    return widget->widget_class->selectable(widget);
  } else {
    return 0;
  }
}

int TXT_ContainsWidget(void * uncast_haystack, void * uncast_needle) {
  auto * haystack = reinterpret_cast<txt_widget_t *>(uncast_haystack);
  auto * needle   = reinterpret_cast<txt_widget_t *>(uncast_needle);

  while (needle != nullptr) {
    if (needle == haystack) {
      return 1;
    }

    needle = needle->parent;
  }

  return 0;
}

int TXT_HoveringOverWidget(void * uncast_widget) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  // We can only be hovering over widgets in the active window.

  txt_window_t * active_window = TXT_GetActiveWindow();

  if (active_window == nullptr || !TXT_ContainsWidget(active_window, widget)) {
    return 0;
  }

  // Is the mouse cursor within the bounds of the widget?

  int x = 0, y = 0;
  TXT_GetMousePosition(&x, &y);

  int width  = static_cast<int>(widget->w);
  int height = static_cast<int>(widget->h);
  return (x >= widget->x && x < widget->x + width && y >= widget->y && y < widget->y + height);
}

void TXT_SetWidgetBG(void * uncast_widget) {
  auto * widget = reinterpret_cast<txt_widget_t *>(uncast_widget);

  if (widget->focused) {
    TXT_BGColor(TXT_COLOR_GREY, 0);
  } else if (TXT_HoveringOverWidget(widget)) {
    TXT_BGColor(TXT_HOVER_BACKGROUND, 0);
  } else {
    // Use normal window background.
  }
}
