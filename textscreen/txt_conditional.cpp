//
// Copyright(C) 2016 Simon Howard
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

#include "txt_conditional.hpp"
#include "txt_strut.hpp"
#include <new>

struct [[maybe_unused]] txt_conditional_s {
  txt_widget_t  widget;
  int          *var {};
  int           expected_value {};
  txt_widget_t *child {};
};

static int ConditionTrue(txt_conditional_t *conditional) {
  return *conditional->var == conditional->expected_value;
}

static int TXT_CondSelectable(void *uncast_conditional) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);
  return ConditionTrue(conditional)
         && TXT_SelectableWidget(conditional->child);
}

static void TXT_CondSizeCalc(void *uncast_conditional) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);

  if (!ConditionTrue(conditional)) {
    conditional->widget.w = 0;
    conditional->widget.h = 0;
  } else {
    TXT_CalcWidgetSize(conditional->child);
    conditional->widget.w = conditional->child->w;
    conditional->widget.h = conditional->child->h;
  }
}

static void TXT_CondLayout(void *uncast_conditional) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);

  if (ConditionTrue(conditional)) {
    conditional->child->x = conditional->widget.x;
    conditional->child->y = conditional->widget.y;
    TXT_LayoutWidget(conditional->child);
  }
}

static void TXT_CondDrawer(void *uncast_conditional) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);

  if (ConditionTrue(conditional)) {
    TXT_DrawWidget(conditional->child);
  }
}

static void TXT_CondDestructor(void *uncast_conditional) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);
  TXT_DestroyWidget(conditional->child);
}

static void TXT_CondFocused(void *uncast_conditional, int focused) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);

  if (ConditionTrue(conditional)) {
    TXT_SetWidgetFocus(conditional->child, focused);
  }
}

static int TXT_CondKeyPress(void *uncast_conditional, int key) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);

  if (ConditionTrue(conditional)) {
    return TXT_WidgetKeyPress(conditional->child, key);
  }

  return 0;
}

static void TXT_CondMousePress(void *uncast_conditional,
                               int x, int y, int b) {
  auto *conditional = reinterpret_cast<txt_conditional_t *>(uncast_conditional);

  if (ConditionTrue(conditional)) {
    TXT_WidgetMousePress(conditional->child, x, y, b);
  }
}

txt_widget_class_t txt_conditional_class = {
  TXT_CondSelectable,
  TXT_CondSizeCalc,
  TXT_CondDrawer,
  TXT_CondKeyPress,
  TXT_CondDestructor,
  TXT_CondMousePress,
  TXT_CondLayout,
  TXT_CondFocused,
};

txt_conditional_t *TXT_NewConditional(int *var, int expected_value, void *uncast_child) {
  auto *child = reinterpret_cast<txt_widget_t *>(uncast_child);
  ;

  auto *loc         = malloc(sizeof(txt_conditional_t));
  auto *conditional = new (loc) txt_conditional_t {};

  TXT_InitWidget(conditional, &txt_conditional_class);
  conditional->var            = var;
  conditional->expected_value = expected_value;
  conditional->child          = child;

  child->parent = &conditional->widget;

  return conditional;
}

// "Static" conditional that returns an empty strut if the given static
// value is false. Kind of like a conditional but we only evaluate it at
// creation time.
txt_widget_t *TXT_If(int conditional, void *uncast_child) {
  auto *child = reinterpret_cast<txt_widget_t *>(uncast_child);
  ;

  if (conditional) {
    return child;
  } else {
    TXT_DestroyWidget(child);
    txt_strut_t *nullwidget = TXT_NewStrut(0, 0);
    return &nullwidget->widget;
  }
}
