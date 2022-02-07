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

#ifndef TXT_WIDGET_H
#define TXT_WIDGET_H

/**
 * @file txt_widget.h
 *
 * Base "widget" GUI component class.
 */

#ifndef DOXYGEN

//#define TXT_UNCAST_ARG_NAME(name) uncast_ ## name
//#define TXT_UNCAST_ARG(name)   void * TXT_UNCAST_ARG_NAME(name)
//#define TXT_CAST_ARG(type, name)  type *name = (type *) uncast_ ## name

#else

#define TXT_UNCAST_ARG(name) txt_widget_t *name

#endif

#include <memory>

typedef enum
{
    TXT_VERT_TOP,
    TXT_VERT_CENTER,
    TXT_VERT_BOTTOM,
} txt_vert_align_t;

typedef enum
{
    TXT_HORIZ_LEFT,
    TXT_HORIZ_CENTER,
    TXT_HORIZ_RIGHT,
} txt_horiz_align_t;

/**
 * A GUI widget.
 *
 * A widget is an individual component of a GUI.  Various different widget
 * types exist.
 *
 * Widgets may emit signals.  The types of signal emitted by a widget
 * depend on the type of the widget.  It is possible to be notified
 * when a signal occurs using the @ref TXT_SignalConnect function.
 */

typedef struct txt_widget_s txt_widget_t;

typedef struct txt_widget_class_s txt_widget_class_t;
typedef struct txt_callback_table_s txt_callback_table_t;

typedef void (*TxtWidgetSizeCalc)(void *uncast_widget);
typedef void (*TxtWidgetDrawer)(void *uncast_widget);
typedef void (*TxtWidgetDestroy)(void *uncast_widget);
typedef int (*TxtWidgetKeyPress)(void *uncast_widget, int key);
typedef void (*TxtWidgetSignalFunc)(void *uncast_widget, void *user_data);
typedef void (*TxtMousePressFunc)(void *uncast_widget, int x, int y, int b);
typedef void (*TxtWidgetLayoutFunc)(void *uncast_widget);
typedef int (*TxtWidgetSelectableFunc)(void *uncast_widget);
typedef void (*TxtWidgetFocusFunc)(void *uncast_widget, int focused);

struct txt_widget_class_s
{
    TxtWidgetSelectableFunc selectable{};
    TxtWidgetSizeCalc size_calc{};
    TxtWidgetDrawer drawer{};
    TxtWidgetKeyPress key_press{};
    TxtWidgetDestroy destructor{};
    TxtMousePressFunc mouse_press{};
    TxtWidgetLayoutFunc layout{};
    TxtWidgetFocusFunc focus_change{};
};

struct txt_widget_s
{
    txt_widget_class_t *widget_class;
    std::shared_ptr<txt_callback_table_t> callback_table;
    int visible;
    txt_horiz_align_t align;
    int focused;

    // These are set automatically when the window is drawn and should
    // not be set manually.

    int x, y;
    unsigned int w, h;

    // Pointer up to parent widget that contains this widget.

    txt_widget_t *parent;
};

void TXT_InitWidget(void *uncast_widget, txt_widget_class_t *widget_class);
void TXT_CalcWidgetSize(void *uncast_widget);
void TXT_DrawWidget(void *uncast_widget);
void TXT_EmitSignal(void *uncast_widget, const char *signal_name);
int TXT_WidgetKeyPress(void *uncast_widget, int key);
void TXT_WidgetMousePress(void *uncast_widget, int x, int y, int b);
void TXT_DestroyWidget(void *uncast_widget);
void TXT_LayoutWidget(void *uncast_widget);
int TXT_AlwaysSelectable(void *uncast_widget);
int TXT_NeverSelectable(void *uncast_widget);
void TXT_SetWidgetFocus(void *uncast_widget, int focused);

/**
 * Set a callback function to be invoked when a signal occurs.
 *
 * @param widget       The widget to watch.
 * @param signal_name  The signal to watch.
 * @param func         The callback function to invoke.
 * @param user_data    User-specified pointer to pass to the callback function.
 */

void TXT_SignalConnect(void *uncast_widget, const char *signal_name,
                       TxtWidgetSignalFunc func, void *user_data);

/**
 * Set the policy for how a widget should be aligned within a table.
 * By default, widgets are aligned to the left of the column.
 *
 * @param widget       The widget.
 * @param horiz_align  The alignment to use.
 */

void TXT_SetWidgetAlign(void *uncast_widget, txt_horiz_align_t horiz_align);

/**
 * Query whether a widget is selectable with the cursor.
 *
 * @param widget       The widget.
 * @return             Non-zero if the widget is selectable.
 */

int TXT_SelectableWidget(void *uncast_widget);

/**
 * Query whether the mouse is hovering over the specified widget.
 *
 * @param widget       The widget.
 * @return             Non-zero if the mouse cursor is over the widget.
 */

int TXT_HoveringOverWidget(void *uncast_widget);

/**
 * Set the background to draw the specified widget, depending on
 * whether it is selected and the mouse is hovering over it.
 *
 * @param widget       The widget.
 */

void TXT_SetWidgetBG(void *uncast_widget);

/**
 * Query whether the specified widget is contained within another
 * widget.
 *
 * @param haystack     The widget that might contain needle.
 * @param needle       The widget being queried.
 */

int TXT_ContainsWidget(void *uncast_haystack, void *uncast_needle);

#endif /* #ifndef TXT_WIDGET_H */

