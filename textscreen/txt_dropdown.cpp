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
#include "memory.hpp"
#include "txt_button.hpp"
#include "txt_dropdown.hpp"
#include "txt_gui.hpp"
#include "txt_main.hpp"
#include "txt_utf8.hpp"
#include "txt_window.hpp"
#include <new>

typedef struct
{
    txt_window_t *window;
    txt_dropdown_list_t *list;
    int item;
} callback_data_t;

// Check if the selected value for a list is valid

static int ValidSelection(txt_dropdown_list_t *list)
{
    return *list->variable >= 0 && *list->variable < list->num_values;
}

// Calculate the Y position for the selector window

static int SelectorWindowY(txt_dropdown_list_t *list)
{
    int result = 0;

    if (ValidSelection(list))
    {
        result = list->widget.y - 1 - *list->variable;
    }
    else
    {
        result = list->widget.y - 1 - (list->num_values / 2);
    }

    // Keep dropdown inside the screen.

    if (result < 1)
    {
        result = 1;
    }
    else if (result + list->num_values > (TXT_SCREEN_H - 3))
    {
        result = TXT_SCREEN_H - list->num_values - 3;
    }

    return result;
}

// Called when a button in the selector window is pressed



static void ItemSelected(void *, void *uncast_callback_data)
{
    auto *callback_data = reinterpret_cast<callback_data_t *>(uncast_callback_data);

    // Set the variable

    *callback_data->list->variable = callback_data->item;

    TXT_EmitSignal(callback_data->list, "changed");

    // Close the window

    TXT_CloseWindow(callback_data->window);
}

// Free callback data when the window is closed
static void FreeCallbackData(void *, void *uncast_callback_data)
{
    auto *callback_data = reinterpret_cast<callback_data_t *>(uncast_callback_data);

    free(callback_data);
}

// Catch presses of escape and close the window.

static int SelectorWindowListener(txt_window_t *window, int key, void *)
{
    if (key == KEY_ESCAPE)
    {
        TXT_CloseWindow(window);
        return 1;
    }

    return 0;
}

static int SelectorMouseListener(txt_window_t *window, int x, int y, int,
                                 void *)
{
    auto *win = reinterpret_cast<txt_widget_t *>(window);
    int width = static_cast<int>(win->w);
    int height = static_cast<int>(win->h);
    if (x < win->x || x > win->x + width || y < win->y || y > win->y + height)
    {
        TXT_CloseWindow(window);
        return 1;
    }

    return 0;
}

// Open the dropdown list window to select an item

static void OpenSelectorWindow(txt_dropdown_list_t *list)
{
    // Open a simple window with no title bar or action buttons.

    txt_window_t *window = TXT_NewWindow(nullptr);

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, nullptr);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, nullptr);
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, nullptr);

    // Position the window so that the currently selected item appears
    // over the top of the list widget.

    TXT_SetWindowPosition(window, TXT_HORIZ_LEFT, TXT_VERT_TOP,
                          list->widget.x - 2, SelectorWindowY(list));

    // Add a button to the window for each option in the list.

    for (int i=0; i<list->num_values; ++i)
    {
        txt_button_t *button = TXT_NewButton(list->values[i]);

        TXT_AddWidget(window, button);

        // Callback struct

        auto *mem = malloc(sizeof(callback_data_t));
        auto *data = new (mem) callback_data_t{};
        data->list = list;
        data->window = window;
        data->item = i;
        
        // When the button is pressed, invoke the button press callback
       
        TXT_SignalConnect(button, "pressed", ItemSelected, data);
        
        // When the window is closed, free back the callback struct

        TXT_SignalConnect(window, "closed", FreeCallbackData, data);

        // Is this the currently-selected value?  If so, select the button
        // in the window as the default.
        
        if (i == *list->variable)
        {
            TXT_SelectWidget(window, button);
        }
    }

    // Catch presses of escape in this window and close it.

    TXT_SetKeyListener(window, SelectorWindowListener, nullptr);
    TXT_SetMouseListener(window, SelectorMouseListener, nullptr);
}

static int DropdownListWidth(txt_dropdown_list_t *list)
{
    // Find the maximum string width
 
    int result = 0;

    for (int i=0; i<list->num_values; ++i)
    {
        int w = static_cast<int>(TXT_UTF8_Strlen(list->values[i]));
        if (w > result) 
        {
            result = w;
        }
    }

    return result;
}

static void TXT_DropdownListSizeCalc(void *uncast_list)
{
    auto *list = reinterpret_cast<txt_dropdown_list_t *>(uncast_list);

    list->widget.w = static_cast<unsigned int>(DropdownListWidth(list));
    list->widget.h = 1;
}

static void TXT_DropdownListDrawer(void *uncast_list)
{
    auto *list = reinterpret_cast<txt_dropdown_list_t *>(uncast_list);
    const char *str = nullptr;

    // Set bg/fg text colors.

    TXT_SetWidgetBG(list);

    // Select a string to draw from the list, if the current value is
    // in range.  Otherwise fall back to a default.

    if (ValidSelection(list))
    {
        str = list->values[*list->variable];
    }
    else
    {
        str = "???";
    }

    // Draw the string and fill to the end with spaces

    TXT_DrawString(str);

    for (unsigned int i = TXT_UTF8_Strlen(str); i < list->widget.w; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_DropdownListDestructor(void *)
{
}

static int TXT_DropdownListKeyPress(void *uncast_list, int key)
{
    auto *list = reinterpret_cast<txt_dropdown_list_t *>(uncast_list);

    if (key == KEY_ENTER)
    {
        OpenSelectorWindow(list);
        return 1;
    }
    
    return 0;
}

static void TXT_DropdownListMousePress(void *uncast_list,
                                       int, int, int b)
{
    auto *list = reinterpret_cast<txt_dropdown_list_t *>(uncast_list);

    // Left mouse click does the same as selecting and pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_DropdownListKeyPress(list, KEY_ENTER);
    }
}

txt_widget_class_t txt_dropdown_list_class =
{
    TXT_AlwaysSelectable,
    TXT_DropdownListSizeCalc,
    TXT_DropdownListDrawer,
    TXT_DropdownListKeyPress,
    TXT_DropdownListDestructor,
    TXT_DropdownListMousePress,
    nullptr,
};

txt_dropdown_list_t *TXT_NewDropdownList(int *variable, const char **values,
                                         int num_values)
{
    auto *list = create_struct<txt_dropdown_list_t>();

    TXT_InitWidget(list, &txt_dropdown_list_class);
    list->variable = variable;
    list->values = values;
    list->num_values = num_values;

    return list;
}

