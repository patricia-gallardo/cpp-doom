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
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_label.hpp"
#include "txt_main.hpp"
#include "txt_utf8.hpp"
#include "txt_window.hpp"

static void TXT_LabelSizeCalc(void *uncast_label)
{
    txt_label_t *label = reinterpret_cast<txt_label_t *>(uncast_label);

    label->widget.w = label->w;
    label->widget.h = label->h;
}

static void TXT_LabelDrawer(void *uncast_label)
{
    auto *label = reinterpret_cast<txt_label_t *>(uncast_label);
    unsigned int x, y;
    int origin_x, origin_y;
    unsigned int align_indent = 0;
    unsigned int w, sw;

    w = label->widget.w;

    if (label->bgcolor >= 0)
    {
        TXT_BGColor(label->bgcolor, 0);
    }
    if (label->fgcolor >= 0)
    {
        TXT_FGColor(label->fgcolor);
    }

    TXT_GetXY(&origin_x, &origin_y);

    for (y=0; y<label->h; ++y)
    {
        // Calculate the amount to indent this line due to the align
        // setting
        sw = TXT_UTF8_Strlen(label->lines[y]);
        switch (label->widget.align)
        {
            case TXT_HORIZ_LEFT:
                align_indent = 0;
                break;
            case TXT_HORIZ_CENTER:
                align_indent = (label->w - sw) / 2;
                break;
            case TXT_HORIZ_RIGHT:
                align_indent = label->w - sw;
                break;
        }

        // Draw this line

        TXT_GotoXY(origin_x, origin_y + y);

        // Gap at the start

        for (x=0; x<align_indent; ++x)
        {
            TXT_DrawString(" ");
        }

        // The string itself

        TXT_DrawString(label->lines[y]);
        x += sw;

        // Gap at the end

        for (; x<w; ++x)
        {
            TXT_DrawString(" ");
        }
    }
}

static void TXT_LabelDestructor(void *uncast_label)
{
    auto *label = reinterpret_cast<txt_label_t *>(uncast_label);

    free(label->label);
    free(label->lines);
}

txt_widget_class_t txt_label_class =
{
    TXT_NeverSelectable,
    TXT_LabelSizeCalc,
    TXT_LabelDrawer,
    nullptr,
    TXT_LabelDestructor,
    nullptr,
    nullptr,
};

void TXT_SetLabel(txt_label_t *label, const char *value)
{
    char *p;

    // Free back the old label

    free(label->label);
    free(label->lines);

    // Set the new value

    label->label = strdup(value);

    // Work out how many lines in this label

    label->h = 1;

    for (p = label->label; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            ++label->h;
        }
    }

    // Split into lines

    label->lines = static_cast<char **>(malloc(sizeof(char *) * label->h));
    label->lines[0] = label->label;
    unsigned int y = 1;

    for (p = label->label; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            label->lines[y] = p + 1;
            *p = '\0';
            ++y;
        }
    }

    label->w = 0;

    for (y=0; y<label->h; ++y)
    {
        unsigned int line_len;

        line_len = TXT_UTF8_Strlen(label->lines[y]);

        if (line_len > label->w)
            label->w = line_len;
    }
}

txt_label_t *TXT_NewLabel(const char *text)
{
    auto *label = create_struct<txt_label_t>();

    TXT_InitWidget(label, &txt_label_class);
    label->label = nullptr;
    label->lines = nullptr;

    // Default colors

    label->bgcolor = TXT_COLOR_BLACK;
    label->fgcolor = TXT_COLOR_BLACK;

    TXT_SetLabel(label, text);

    return label;
}

void TXT_SetFGColor(txt_label_t *label, txt_color_t color)
{
    label->fgcolor = color;
}

void TXT_SetBGColor(txt_label_t *label, txt_color_t color)
{
    label->bgcolor = color;
}

