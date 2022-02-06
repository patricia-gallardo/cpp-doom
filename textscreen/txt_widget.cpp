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

#include "memory.hpp"
#include "txt_desktop.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_widget.hpp"
#include <vector>
#include <memory>

typedef struct
{
    char *signal_name;
    TxtWidgetSignalFunc func;
    void *user_data;
} txt_callback_t;

struct txt_callback_table_s
{
    std::vector<txt_callback_t> callbacks;

    ~txt_callback_table_s() {
      for (auto &callback : callbacks) {
        free(callback.signal_name);
      }
    }
};

std::shared_ptr<txt_callback_table_t> TXT_NewCallbackTable()
{
    return std::make_shared<txt_callback_table_t>();
}


void TXT_InitWidget(TXT_UNCAST_ARG(widget), txt_widget_class_t *widget_class)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->widget_class = widget_class;
    widget->callback_table = TXT_NewCallbackTable();
    widget->parent = nullptr;

    // Not focused until we hear otherwise.

    widget->focused = 0;

    // Visible by default.

    widget->visible = 1;

    // Align left by default

    widget->align = TXT_HORIZ_LEFT;
}

void TXT_SignalConnect(TXT_UNCAST_ARG(widget),
                       const char *signal_name,
                       TxtWidgetSignalFunc func, 
                       void *user_data)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    auto &table = widget->callback_table;

    // Add a new callback to the table


    auto &callback = table->callbacks.emplace_back();
    callback.signal_name = strdup(signal_name);
    callback.func = func;
    callback.user_data = user_data;
}

void TXT_EmitSignal(TXT_UNCAST_ARG(widget), const char *signal_name)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    auto table = widget->callback_table;

    // Don't destroy the table while we're searching through it
    // (one of the callbacks may destroy this window)


    // Search the table for all callbacks with this name and invoke
    // the functions.

    for (auto &callback : table->callbacks)
    {
        if (!strcmp(callback.signal_name, signal_name))
        {
            callback.func(widget, callback.user_data);
        }
    }
}

void TXT_CalcWidgetSize(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->widget_class->size_calc(widget);
}

void TXT_DrawWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);
    txt_saved_colors_t colors;

    // The drawing function might change the fg/bg colors,
    // so make sure we restore them after it's done.

    TXT_SaveColors(&colors);

    // For convenience...

    TXT_GotoXY(widget->x, widget->y);

    // Call drawer method

    widget->widget_class->drawer(widget);

    TXT_RestoreColors(&colors);
}

void TXT_DestroyWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->widget_class->destructor(widget);
    free(widget);
}

int TXT_WidgetKeyPress(TXT_UNCAST_ARG(widget), int key)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->key_press != nullptr)
    {
        return widget->widget_class->key_press(widget, key);
    }

    return 0;
}

void TXT_SetWidgetFocus(TXT_UNCAST_ARG(widget), int focused)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget == nullptr)
    {
        return;
    }

    if (widget->focused != focused)
    {
        widget->focused = focused;

        if (widget->widget_class->focus_change != nullptr)
        {
            widget->widget_class->focus_change(widget, focused);
        }
    }
}

void TXT_SetWidgetAlign(TXT_UNCAST_ARG(widget), txt_horiz_align_t horiz_align)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->align = horiz_align;
}

void TXT_WidgetMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->mouse_press != nullptr)
    {
        widget->widget_class->mouse_press(widget, x, y, b);
    }
}

void TXT_LayoutWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->layout != nullptr)
    {
        widget->widget_class->layout(widget);
    }
}

int TXT_AlwaysSelectable(TXT_UNCAST_ARG(widget))
{
    return 1;
}

int TXT_NeverSelectable(TXT_UNCAST_ARG(widget))
{
    return 0;
}

int TXT_SelectableWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->selectable != nullptr)
    {
        return widget->widget_class->selectable(widget);
    }
    else
    {
        return 0;
    }
}

int TXT_ContainsWidget(TXT_UNCAST_ARG(haystack), TXT_UNCAST_ARG(needle))
{
    TXT_CAST_ARG(txt_widget_t, haystack);
    TXT_CAST_ARG(txt_widget_t, needle);

    while (needle != nullptr)
    {
        if (needle == haystack)
        {
            return 1;
        }

        needle = needle->parent;
    }

    return 0;
}

int TXT_HoveringOverWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);
    txt_window_t *active_window;
    int x, y;

    // We can only be hovering over widgets in the active window.

    active_window = TXT_GetActiveWindow();

    if (active_window == NULL || !TXT_ContainsWidget(active_window, widget))
    {
        return 0;
    }

    // Is the mouse cursor within the bounds of the widget?

    TXT_GetMousePosition(&x, &y);

    return (x >= widget->x && x < widget->x + widget->w
         && y >= widget->y && y < widget->y + widget->h);
}

void TXT_SetWidgetBG(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->focused)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }
    else if (TXT_HoveringOverWidget(widget))
    {
        TXT_BGColor(TXT_HOVER_BACKGROUND, 0);
    }
    else
    {
        // Use normal window background.
    }
}

