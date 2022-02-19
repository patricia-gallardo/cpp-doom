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

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include "memory.hpp"
#include "txt_desktop.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_label.hpp"
#include "txt_main.hpp"
#include "txt_separator.hpp"
#include "txt_window.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

void TXT_SetWindowAction(txt_window_t *window,
                         txt_horiz_align_t position, void *uncast_action)
{
    auto *action = reinterpret_cast<txt_widget_t *>(uncast_action);

    if (window->actions[position] != nullptr)
    {
        TXT_DestroyWidget(window->actions[position]);
    }

    window->actions[position] = action;

    // Maintain parent pointer.

    if (action != nullptr)
    {
        action->parent = &window->table.widget;
    }
}

txt_window_t *TXT_NewWindow(const char *title)
{
    auto *win = create_struct<txt_window_t>();

    TXT_InitTable(&win->table, 1);

    if (title == nullptr)
    {
        win->title = nullptr;
    }
    else
    {
        win->title = strdup(title);
    }

    win->x = TXT_SCREEN_W / 2;
    win->y = TXT_SCREEN_H / 2;
    win->horiz_align = TXT_HORIZ_CENTER;
    win->vert_align = TXT_VERT_CENTER;
    win->key_listener = nullptr;
    win->mouse_listener = nullptr;
    win->help_url = nullptr;

    TXT_AddWidget(win, TXT_NewSeparator(nullptr));

    for (auto & action : win->actions)
    {
        action = nullptr;
    }

    TXT_AddDesktopWindow(win);

    // Default actions

    TXT_SetWindowAction(win, TXT_HORIZ_LEFT, TXT_NewWindowEscapeAction(win));
    TXT_SetWindowAction(win, TXT_HORIZ_RIGHT, TXT_NewWindowSelectAction(win));

    return win;
}

void TXT_CloseWindow(txt_window_t *window)
{
    TXT_EmitSignal(window, "closed");
    TXT_RemoveDesktopWindow(window);

    free(window->title);

    // Destroy all actions

    for (auto & action : window->actions)
    {
        if (action != nullptr)
        {
            TXT_DestroyWidget(action);
        }
    }

    // Destroy table and window

    TXT_DestroyWidget(window);
}

static void CalcWindowPosition(txt_window_t *window)
{
    switch (window->horiz_align)
    {
        case TXT_HORIZ_LEFT:
            window->window_x = window->x;
            break;
        case TXT_HORIZ_CENTER:
            window->window_x = static_cast<int>(static_cast<unsigned int>(window->x) - (window->window_w / 2));
            break;
        case TXT_HORIZ_RIGHT:
            window->window_x = static_cast<int>(static_cast<unsigned int>(window->x) - (window->window_w - 1));
            break;
    }

    switch (window->vert_align)
    {
        case TXT_VERT_TOP:
            window->window_y = window->y;
            break;
        case TXT_VERT_CENTER:
            window->window_y = static_cast<int>(static_cast<unsigned int>(window->y) - (window->window_h / 2));
            break;
        case TXT_VERT_BOTTOM:
            window->window_y = static_cast<int>(static_cast<unsigned int>(window->y) - (window->window_h - 1));
            break;
    }
}

static void LayoutActionArea(txt_window_t *window)
{
    txt_widget_t *widget = nullptr;

    // We need to calculate the available horizontal space for the center
    // action widget, so that we can center it within it.
    // To start with, we have the entire action area available.

    int space_available = static_cast<int>(window->window_w);
    int space_left_offset = 0;

    // Left action

    if (window->actions[TXT_HORIZ_LEFT] != nullptr)
    {
        widget = window->actions[TXT_HORIZ_LEFT];

        TXT_CalcWidgetSize(widget);

        widget->x = window->window_x + 1;
        widget->y = static_cast<int>(static_cast<unsigned int>(window->window_y) + window->window_h - widget->h - 1);

        // Adjust available space:
        space_available -= widget->w;
        space_left_offset += widget->w;

        TXT_LayoutWidget(widget);
    }

    // Draw the right action

    if (window->actions[TXT_HORIZ_RIGHT] != nullptr)
    {
        widget = window->actions[TXT_HORIZ_RIGHT];

        TXT_CalcWidgetSize(widget);

        widget->x = static_cast<int>(static_cast<unsigned int>(window->window_x) + window->window_w - 1 - widget->w);
        widget->y = static_cast<int>(static_cast<unsigned int>(window->window_y) + window->window_h - widget->h - 1);

        // Adjust available space:
        space_available -= widget->w;

        TXT_LayoutWidget(widget);
    }

    // Draw the center action

    if (window->actions[TXT_HORIZ_CENTER] != nullptr)
    {
        widget = window->actions[TXT_HORIZ_CENTER];

        TXT_CalcWidgetSize(widget);

        // The left and right widgets have left a space sandwiched between
        // them.  Center this widget within that space.
        widget->x = static_cast<int>(static_cast<unsigned int>(window->window_x + space_left_offset) + (static_cast<unsigned int>(space_available) - widget->w) / 2);
        widget->y = static_cast<int>(static_cast<unsigned int>(window->window_y) + window->window_h - widget->h - 1);

        TXT_LayoutWidget(widget);
    }
}

static void DrawActionArea(txt_window_t *window)
{
    for (auto & action : window->actions)
    {
        if (action != nullptr)
        {
            TXT_DrawWidget(action);
        }
    }
}

static void CalcActionAreaSize(txt_window_t *window, 
                               unsigned int *w, unsigned int *h)
{
    *w = 0;
    *h = 0;

    // Calculate the width of all the action widgets and use this
    // to create an overall min. width of the action area

    for (auto widget : window->actions)
    {
        if (widget != nullptr)
        {
            TXT_CalcWidgetSize(widget);
            *w += widget->w;

            if (widget->h > *h)
            {
                *h = widget->h;
            }
        }
    }
}

// Sets size and position of all widgets in a window

void TXT_LayoutWindow(txt_window_t *window)
{
    auto *widgets = reinterpret_cast<txt_widget_t *>(window);

    // Calculate size of table
    
    TXT_CalcWidgetSize(window);

    // Widgets area: add one character of padding on each side
    unsigned int widgets_w = widgets->w + 2;

    // Calculate the size of the action area
    // Make window wide enough to action area

    unsigned int actionarea_w = 0;
    unsigned int actionarea_h = 0;
    CalcActionAreaSize(window, &actionarea_w, &actionarea_h);
    
    if (actionarea_w > widgets_w)
        widgets_w = actionarea_w;

    // Set the window size based on widgets_w
   
    window->window_w = widgets_w + 2;
    window->window_h = widgets->h + 1;

    // If the window has a title, add an extra two lines

    if (window->title != nullptr)
    {
        window->window_h += 2;
    }

    // If the window has an action area, add extra lines

    if (actionarea_h > 0)
    {
        window->window_h += actionarea_h + 1;
    }

    // Use the x,y position as the centerpoint and find the location to 
    // draw the window.

    CalcWindowPosition(window);

    // Set the table size and position

    widgets->w = widgets_w - 2;
    // widgets->h        (already set)
    widgets->x = window->window_x + 2;
    widgets->y = window->window_y;

    if (window->title != nullptr)
    {
        widgets->y += 2;
    }

    // Layout the table and action area

    LayoutActionArea(window);
    TXT_LayoutWidget(widgets);
}

void TXT_DrawWindow(txt_window_t *window)
{
    TXT_LayoutWindow(window);

    if (window->table.widget.focused)
    {
        TXT_BGColor(TXT_ACTIVE_WINDOW_BACKGROUND, 0);
    }
    else
    {
        TXT_BGColor(TXT_INACTIVE_WINDOW_BACKGROUND, 0);
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    // Draw the window

    TXT_DrawWindowFrame(window->title, 
                        window->window_x, window->window_y, static_cast<int>(window->window_w), static_cast<int>(window->window_h));

    // Draw all widgets

    TXT_DrawWidget(window);

    // Draw an action area, if we have one

    auto *widgets = reinterpret_cast<txt_widget_t *>(window);

    if (static_cast<unsigned int>(widgets->y) + widgets->h < static_cast<unsigned int>(window->window_y) + window->window_h - 1)
    {
        // Separator for action area

        TXT_DrawSeparator(window->window_x, static_cast<int>(static_cast<unsigned int>(widgets->y) + widgets->h), static_cast<int>(window->window_w));

        // Action area at the window bottom

        DrawActionArea(window);
    }
}

void TXT_SetWindowPosition(txt_window_t *window,
                           txt_horiz_align_t horiz_align,
                           txt_vert_align_t vert_align,
                           int x, int y)
{
    window->vert_align = vert_align;
    window->horiz_align = horiz_align;
    window->x = x;
    window->y = y;
}

static int MouseButtonPress(txt_window_t *window, int b)
{
    // Lay out the window, set positions and sizes of all widgets

    TXT_LayoutWindow(window);

    // Get the current mouse position

    int x = 0, y = 0;
    TXT_GetMousePosition(&x, &y);

    // Try the mouse button listener
    // This happens whether it is in the window range or not

    if (window->mouse_listener != nullptr)
    {
        // Mouse listener can eat button presses

        if (window->mouse_listener(window, x, y, b, 
                                   window->mouse_listener_data))
        {
            return 1;
        }
    }

    // Is it within the table range?

    auto *widgets = reinterpret_cast<txt_widget_t *>(window);

    if (x >= widgets->x && x < static_cast<signed>(static_cast<unsigned int>(widgets->x) + widgets->w)
     && y >= widgets->y && y < static_cast<signed>(static_cast<unsigned int>(widgets->y) + widgets->h))
    {
        TXT_WidgetMousePress(window, x, y, b);
        return 1;
    }

    // Was one of the action area buttons pressed?

    for (auto widget : window->actions)
    {
        if (widget != nullptr
         && x >= widget->x && x < static_cast<signed>(static_cast<unsigned int>(widget->x) + widget->w)
         && y >= widget->y && y < static_cast<signed>(static_cast<unsigned int>(widget->y) + widget->h))
        {
            // Main table temporarily loses focus when action area button
            // is clicked. This way, any active input boxes that depend
            // on having focus will save their values before the
            // action is performed.

            int was_focused = window->table.widget.focused;
            TXT_SetWidgetFocus(window, 0);
            TXT_SetWidgetFocus(window, was_focused);

            // Pass through mouse press.

            TXT_WidgetMousePress(widget, x, y, b);
            return 1;
        }
    }

    return 0;
}

int TXT_WindowKeyPress(txt_window_t *window, int c)
{
    // Is this a mouse button ?

    if (c >= TXT_MOUSE_BASE && c < TXT_MOUSE_BASE + TXT_MAX_MOUSE_BUTTONS)
    {
        return MouseButtonPress(window, c);
    }

    // Try the window key spy

    if (window->key_listener != nullptr)
    {
        // key listener can eat keys

        if (window->key_listener(window, c, window->key_listener_data))
        {
            return 1;
        }
    }

    // Send to the currently selected widget:

    if (TXT_WidgetKeyPress(window, c))
    {
        return 1;
    }

    // Try all of the action buttons

    for (auto & action : window->actions)
    {
        if (action != nullptr
         && TXT_WidgetKeyPress(action, c))
        {
            return 1;
        }
    }

    return 0;
}

void TXT_SetKeyListener(txt_window_t *window, TxtWindowKeyPress key_listener, 
                        void *user_data)
{
    window->key_listener = key_listener;
    window->key_listener_data = user_data;
}

void TXT_SetMouseListener(txt_window_t *window, 
                          TxtWindowMousePress mouse_listener,
                          void *user_data)
{
    window->mouse_listener = mouse_listener;
    window->mouse_listener_data = user_data;
}

void TXT_SetWindowFocus(txt_window_t *window, int focused)
{
    TXT_SetWidgetFocus(window, focused);
}

void TXT_SetWindowHelpURL(txt_window_t *window, const char *help_url)
{
    window->help_url = help_url;
}

#ifdef _WIN32

void TXT_OpenURL(const char *url)
{
    ShellExecute(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

#else

void TXT_OpenURL(const char *url)
{
    size_t cmd_len = strlen(url) + 30;
    char *cmd = static_cast<char *>(malloc(cmd_len));

#if defined(__MACOSX__)
    TXT_snprintf(cmd, cmd_len, "open \"%s\"", url);
#else
    // The Unix situation sucks as usual, but the closest thing to a
    // standard that exists is the xdg-utils package.
    if (system("xdg-open --version 2>/dev/null") != 0)
    {
        fprintf(stderr,
                "xdg-utils is not installed. Can't open this URL:\n%s\n", url);
        free(cmd);
        return;
    }

    TXT_snprintf(cmd, cmd_len, "xdg-open \"%s\"", url);
#endif

    int retval = system(cmd);
    free(cmd);
    if (retval != 0)
    {
        fprintf(stderr, "TXT_OpenURL: error executing '%s'; return code %d\n",
            cmd, retval);
    }
}

#endif /* #ifndef _WIN32 */

void TXT_OpenWindowHelpURL(txt_window_t *window)
{
    if (window->help_url != nullptr)
    {
        TXT_OpenURL(window->help_url);
    }
}

txt_window_t *TXT_MessageBox(const char *title, const char *message, ...)
{
    char buf[256];
    va_list args;

    va_start(args, message);
    TXT_vsnprintf(buf, sizeof(buf), message, args);
    va_end(args);

    txt_window_t *window = TXT_NewWindow(title);
    TXT_AddWidget(window, TXT_NewLabel(buf));

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, nullptr);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowEscapeAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, nullptr);

    return window;
}

