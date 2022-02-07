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

#include "txt_separator.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_main.hpp"
#include "txt_utf8.hpp"
#include "txt_window.hpp"
#include "memory.hpp"

static void TXT_SeparatorSizeCalc(void *uncast_separator)
{
    txt_separator_t *separator = (txt_separator_t *)uncast_separator;

    if (separator->label != nullptr)
    {
        // Minimum width is the string length + two spaces for padding

        separator->widget.w = TXT_UTF8_Strlen(separator->label) + 2;
    }
    else
    {
        separator->widget.w = 0;
    }

    separator->widget.h = 1;
}

static void TXT_SeparatorDrawer(void *uncast_separator)
{
    txt_separator_t *separator = (txt_separator_t *)uncast_separator;
    int x, y;
    int w;

    w = separator->widget.w;

    TXT_GetXY(&x, &y);

    // Draw separator.  Go back one character and draw two extra
    // to overlap the window borders.

    TXT_DrawSeparator(x-2, y, w + 4);

    if (separator->label != nullptr)
    {
        TXT_GotoXY(x, y);

        TXT_FGColor(TXT_COLOR_BRIGHT_GREEN);
        TXT_DrawString(" ");
        TXT_DrawString(separator->label);
        TXT_DrawString(" ");
    }
}

static void TXT_SeparatorDestructor(void *uncast_separator)
{
    txt_separator_t *separator = (txt_separator_t *)uncast_separator;

    free(separator->label);
}

void TXT_SetSeparatorLabel(txt_separator_t *separator, const char *label)
{
    free(separator->label);

    if (label != nullptr)
    {
        separator->label = strdup(label);
    }
    else
    {
        separator->label = nullptr;
    }
}

txt_widget_class_t txt_separator_class =
{
    TXT_NeverSelectable,
    TXT_SeparatorSizeCalc,
    TXT_SeparatorDrawer,
    nullptr,
    TXT_SeparatorDestructor,
    nullptr,
    nullptr,
};

txt_separator_t *TXT_NewSeparator(const char *label)
{
    auto *separator = create_struct<txt_separator_t>();

    TXT_InitWidget(separator, &txt_separator_class);

    separator->label = nullptr;
    TXT_SetSeparatorLabel(separator, label);

    return separator;
}

