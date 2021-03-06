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

#pragma once

/**
 * @file txt_label.h
 *
 * Text label widget.
 */

/**
 * Label widget.
 *
 * A label widget does nothing except show a text label.
 */

using txt_label_t = struct txt_label_s;

#include "txt_main.hpp"
#include "txt_widget.hpp"

struct txt_label_s {
  txt_widget_t widget;
  char *       label {};
  char **      lines {};
  unsigned int w {}, h {};
  txt_color_t  fgcolor;
  txt_color_t  bgcolor;
};

/**
 * Create a new label widget.
 *
 * @param label         String to display in the widget (UTF-8 format).
 * @return              Pointer to the new label widget.
 */

txt_label_t * TXT_NewLabel(cstring_view label);

/**
 * Set the string displayed in a label widget.
 *
 * @param label         The widget.
 * @param value         The string to display (UTF-8 format).
 */

void TXT_SetLabel(txt_label_t * label, cstring_view value);

/**
 * Set the background color of a label widget.
 *
 * @param label         The widget.
 * @param color         The background color to use.
 */

[[maybe_unused]] void TXT_SetBGColor(txt_label_t * label, txt_color_t color);

/**
 * Set the foreground color of a label widget.
 *
 * @param label         The widget.
 * @param color         The foreground color to use.
 */

void TXT_SetFGColor(txt_label_t * label, txt_color_t color);
