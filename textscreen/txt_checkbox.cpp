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

#include "txt_checkbox.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_main.hpp"
#include "txt_utf8.hpp"
#include "txt_window.hpp"
#include "memory.hpp"

static void TXT_CheckBoxSizeCalc(void *uncast_checkbox)
{
    auto *checkbox = reinterpret_cast<txt_checkbox_t *>(uncast_checkbox);;

    // Minimum width is the string length + right-side space for padding

    checkbox->widget.w = TXT_UTF8_Strlen(checkbox->label) + 5;
    checkbox->widget.h = 1;
}

static void TXT_CheckBoxDrawer(void *uncast_checkbox)
{
    auto    *checkbox = reinterpret_cast<txt_checkbox_t *>(uncast_checkbox);;
    txt_saved_colors_t colors;
    int i;
    int w;

    w = static_cast<int>(checkbox->widget.w);

    TXT_SaveColors(&colors);
    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_DrawString("(");

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    if ((*checkbox->variable != 0) ^ checkbox->inverted)
    {
        TXT_DrawCodePageString("\x07");
    }
    else
    {
        TXT_DrawString(" ");
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);

    TXT_DrawString(") ");

    TXT_RestoreColors(&colors);
    TXT_SetWidgetBG(checkbox);
    TXT_DrawString(checkbox->label);

    for (i = static_cast<int>(TXT_UTF8_Strlen(checkbox->label)); i < w-4; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_CheckBoxDestructor(void *uncast_checkbox)
{
    auto *checkbox = reinterpret_cast<txt_checkbox_t *>(uncast_checkbox);;

    free(checkbox->label);
}

static int TXT_CheckBoxKeyPress(void *uncast_checkbox, int key)
{
    auto *checkbox = reinterpret_cast<txt_checkbox_t *>(uncast_checkbox);;

    if (key == KEY_ENTER || key == ' ')
    {
        *checkbox->variable = !*checkbox->variable;
        TXT_EmitSignal(checkbox, "changed");
        return 1;
    }
    
    return 0;
}

static void TXT_CheckBoxMousePress(void *uncast_checkbox, int, int, int b)
{
    auto *checkbox = reinterpret_cast<txt_checkbox_t *>(uncast_checkbox);;

    if (b == TXT_MOUSE_LEFT)
    {
        // Equivalent to pressing enter

        TXT_CheckBoxKeyPress(checkbox, KEY_ENTER);
    }
}

txt_widget_class_t txt_checkbox_class =
{
    TXT_AlwaysSelectable,
    TXT_CheckBoxSizeCalc,
    TXT_CheckBoxDrawer,
    TXT_CheckBoxKeyPress,
    TXT_CheckBoxDestructor,
    TXT_CheckBoxMousePress,
    nullptr,
};

txt_checkbox_t *TXT_NewCheckBox(const char *label, int *variable)
{
    auto *checkbox = create_struct<txt_checkbox_t>();

    TXT_InitWidget(checkbox, &txt_checkbox_class);
    checkbox->label = strdup(label);
    checkbox->variable = variable;
    checkbox->inverted = 0;

    return checkbox;
}

txt_checkbox_t *TXT_NewInvertedCheckBox(const char *label, int *variable)
{
    txt_checkbox_t *result;

    result = TXT_NewCheckBox(label, variable);
    result->inverted = 1;

    return result;
}

