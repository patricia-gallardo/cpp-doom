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


#include "doomkeys.hpp"

#include "txt_strut.hpp"
#include "txt_io.hpp"
#include "txt_main.hpp"
#include "txt_window.hpp"
#include "memory.hpp"

static void TXT_StrutSizeCalc(TXT_UNCAST_ARG(strut))
{
    TXT_CAST_ARG(txt_strut_t, strut);

    // Minimum width is the string length + two spaces for padding

    strut->widget.w = strut->width;
    strut->widget.h = strut->height;
}

static void TXT_StrutDrawer(TXT_UNCAST_ARG(strut))
{
    // Nothing is drawn for a strut.
}

static void TXT_StrutDestructor(TXT_UNCAST_ARG(strut))
{
}

static int TXT_StrutKeyPress(TXT_UNCAST_ARG(strut), int key)
{
    return 0;
}

txt_widget_class_t txt_strut_class =
{
    TXT_NeverSelectable,
    TXT_StrutSizeCalc,
    TXT_StrutDrawer,
    TXT_StrutKeyPress,
    TXT_StrutDestructor,
    NULL,
    NULL,
};

txt_strut_t *TXT_NewStrut(int width, int height)
{
    auto *strut = create_struct<txt_strut_t>();

    TXT_InitWidget(strut, &txt_strut_class);
    strut->width = width;
    strut->height = height;

    return strut;
}

