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

#include "txt_scrollpane.hpp"
#include "txt_gui.hpp"
#include "txt_main.hpp"
#include "txt_table.hpp"

#include "doomkeys.hpp"
#include "memory.hpp"

constexpr auto SCROLLBAR_VERTICAL   = (1 << 0);
constexpr auto SCROLLBAR_HORIZONTAL = (1 << 1);

static int FullWidth(txt_scrollpane_t * scrollpane) {
  if (scrollpane->child != nullptr) {
    return static_cast<int>(scrollpane->child->w);
  } else {
    return 0;
  }
}

static int FullHeight(txt_scrollpane_t * scrollpane) {
  if (scrollpane->child != nullptr) {
    return static_cast<int>(scrollpane->child->h);
  } else {
    return 0;
  }
}

// Calculate which scroll bars the pane needs.

static int NeedsScrollbars(txt_scrollpane_t * scrollpane) {
  int result = 0;

  if (FullWidth(scrollpane) > scrollpane->w) {
    result |= SCROLLBAR_HORIZONTAL;
  }
  if (FullHeight(scrollpane) > scrollpane->h) {
    result |= SCROLLBAR_VERTICAL;
  }

  return result;
}

// If a scrollbar isn't needed, the scroll position is reset.

static void SanityCheckScrollbars(txt_scrollpane_t * scrollpane) {
  int scrollbars = NeedsScrollbars(scrollpane);

  if ((scrollbars & SCROLLBAR_HORIZONTAL) == 0) {
    scrollpane->x = 0;
  }
  if ((scrollbars & SCROLLBAR_VERTICAL) == 0) {
    scrollpane->y = 0;
  }

  int max_x = FullWidth(scrollpane) - scrollpane->w;
  int max_y = FullHeight(scrollpane) - scrollpane->h;

  if (scrollpane->x < 0) {
    scrollpane->x = 0;
  } else if (scrollpane->x > max_x) {
    scrollpane->x = max_x;
  }

  if (scrollpane->y < 0) {
    scrollpane->y = 0;
  } else if (scrollpane->y > max_y) {
    scrollpane->y = max_y;
  }
}

static void TXT_ScrollPaneSizeCalc(void * uncast_scrollpane) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);

  if (scrollpane->child != nullptr) {
    TXT_CalcWidgetSize(scrollpane->child);
  }

  // Expand as necessary (to ensure that no scrollbars are needed)?

  if (scrollpane->expand_w) {
    scrollpane->w = FullWidth(scrollpane);
  }
  if (scrollpane->expand_h) {
    scrollpane->h = FullHeight(scrollpane);
  }

  scrollpane->widget.w = static_cast<unsigned int>(scrollpane->w);
  scrollpane->widget.h = static_cast<unsigned int>(scrollpane->h);

  // If we have scroll bars, we need to expand slightly to
  // accomodate them. Eg. if we have a vertical scrollbar, we
  // need to be an extra character wide.

  int scrollbars = NeedsScrollbars(scrollpane);

  if (scrollbars & SCROLLBAR_HORIZONTAL) {
    ++scrollpane->widget.h;
  }
  if (scrollbars & SCROLLBAR_VERTICAL) {
    ++scrollpane->widget.w;
  }

  if (scrollpane->child != nullptr) {
    if (static_cast<int>(scrollpane->child->w) < scrollpane->w) {
      scrollpane->child->w = static_cast<unsigned int>(scrollpane->w);
    }
    if (static_cast<int>(scrollpane->child->h) < scrollpane->h) {
      scrollpane->child->h = static_cast<unsigned int>(scrollpane->h);
    }
  }
}

static void TXT_ScrollPaneDrawer(void * uncast_scrollpane) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);

  // We set a clipping area of the scroll pane.

  int x1 = scrollpane->widget.x;
  int y1 = scrollpane->widget.y;
  int x2 = x1 + scrollpane->w;
  int y2 = y1 + scrollpane->h;

  int scrollbars = NeedsScrollbars(scrollpane);

  if (scrollbars & SCROLLBAR_HORIZONTAL) {
    TXT_DrawHorizScrollbar(x1,
                           y1 + scrollpane->h,
                           scrollpane->w,
                           scrollpane->x,
                           FullWidth(scrollpane) - scrollpane->w);
  }

  if (scrollbars & SCROLLBAR_VERTICAL) {
    TXT_DrawVertScrollbar(x1 + scrollpane->w,
                          y1,
                          scrollpane->h,
                          scrollpane->y,
                          FullHeight(scrollpane) - scrollpane->h);
  }

  TXT_PushClipArea(x1, x2, y1, y2);

  // Draw the child widget

  if (scrollpane->child != nullptr) {
    TXT_DrawWidget(scrollpane->child);
  }

  // Restore old clipping area.

  TXT_PopClipArea();
}

static void TXT_ScrollPaneDestructor(void * uncast_scrollpane) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);

  if (scrollpane->child != nullptr) {
    TXT_DestroyWidget(scrollpane->child);
  }
}

static void TXT_ScrollPaneFocused(void * uncast_scrollpane, int focused) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);

  // Whether the child is focused depends only on whether the scroll pane
  // itself is focused. Pass through focus to the child.

  if (scrollpane->child != nullptr) {
    TXT_SetWidgetFocus(scrollpane->child, focused);
  }
}

// Hack for tables - when browsing a table inside a scroll pane,
// automatically scroll the window to show the newly-selected
// item.

static void ShowSelectedWidget(txt_scrollpane_t * scrollpane) {
  txt_widget_t * selected = TXT_GetSelectedWidget(scrollpane->child);

  // Scroll up or down?

  if (selected->y <= scrollpane->widget.y) {
    scrollpane->y -= scrollpane->widget.y - selected->y;
  } else if (static_cast<signed>(static_cast<unsigned int>(selected->y) + selected->h) > static_cast<signed>(scrollpane->widget.y + scrollpane->h)) {
    scrollpane->y += (static_cast<unsigned int>(selected->y) + selected->h)
                     - (static_cast<unsigned int>(scrollpane->widget.y + scrollpane->h));
  }

  // Scroll left or right?

  if (selected->x <= scrollpane->widget.x) {
    scrollpane->x -= scrollpane->widget.x - selected->x;
  } else if (static_cast<signed>(static_cast<unsigned int>(selected->x) + selected->w) > static_cast<signed>(scrollpane->widget.x + scrollpane->w)) {
    scrollpane->x += (static_cast<unsigned int>(selected->x) + selected->w)
                     - (static_cast<unsigned int>(scrollpane->widget.x + scrollpane->w));
  }
}

// Another hack for tables - when scrolling in 'pages', the normal key press
// event does not provide children with enough information to know how far
// to move their selection to reach a new page. This function does so.
// Note that it *only* affects scrolling in pages, not with arrows!
// A side-effect of this, rather than 'pulling' the selection to fit within
// the new page, is that we will jump straight over ranges of unselectable
// items longer than a page, but that is also true of arrow-key scrolling.
// The other unfortunate effect of doing things this way is that page keys
// have no effect on tables _not_ in scrollpanes: not even home/end.

static int PageSelectedWidget(txt_scrollpane_t * scrollpane, int key) {
  int pagex = 0; // No page left/right yet, but some keyboards have them
  int pagey = 0;

  // Subtract one from the absolute page distance as this is slightly more
  // intuitive: a page down first jumps to the bottom of the current page,
  // then proceeds to scroll onwards.

  switch (key) {
  case KEY_PGUP:
    pagey = 1 - scrollpane->h;
    break;

  case KEY_PGDN:
    pagey = scrollpane->h - 1;
    break;

  default: // We shouldn't even be in this function
    return 0;
  }

  if (scrollpane->child->widget_class == &txt_table_class) {
    return TXT_PageTable(scrollpane->child, pagex, pagey);
  }

  return 0;
}

// Interpret arrow key presses as scroll commands

static int InterpretScrollKey(txt_scrollpane_t * scrollpane, int key) {
  int maxy = 0;

  switch (key) {
  case KEY_UPARROW:
    if (scrollpane->y > 0) {
      --scrollpane->y;
      return 1;
    }
    break;

  case KEY_DOWNARROW:
    if (scrollpane->y < FullHeight(scrollpane) - scrollpane->h) {
      ++scrollpane->y;
      return 1;
    }
    break;

  case KEY_LEFTARROW:
    if (scrollpane->x > 0) {
      --scrollpane->x;
      return 1;
    }
    break;

  case KEY_RIGHTARROW:
    if (scrollpane->x < FullWidth(scrollpane) - scrollpane->w) {
      ++scrollpane->x;
      return 1;
    }
    break;

  case KEY_PGUP:
    if (scrollpane->y > 0) {
      scrollpane->y -= scrollpane->h;
      if (scrollpane->y < 0) {
        scrollpane->y = 0;
      }
      return 1;
    }
    break;

  case KEY_PGDN:
    maxy = FullHeight(scrollpane) - scrollpane->h;
    if (scrollpane->y < maxy) {
      scrollpane->y += scrollpane->h;
      if (scrollpane->y > maxy) {
        scrollpane->y = maxy;
      }
      return 1;
    }
    break;

  default:
    break;
  }

  return 0;
}

static int TXT_ScrollPaneKeyPress(void * uncast_scrollpane, int key) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);
  int    result     = 0;

  if (scrollpane->child != nullptr) {
    result = TXT_WidgetKeyPress(scrollpane->child, key);

    // Gross hack - if we're scrolling in a menu with the keyboard,
    // automatically move the scroll pane to show the new
    // selected item.

    if ((key == KEY_UPARROW || key == KEY_DOWNARROW
         || key == KEY_LEFTARROW || key == KEY_RIGHTARROW
         || key == KEY_PGUP || key == KEY_PGDN
         || key == KEY_TAB)
        && scrollpane->child->widget_class == &txt_table_class) {
      if (PageSelectedWidget(scrollpane, key)) {
        result = 1;
      }

      ShowSelectedWidget(scrollpane);
    }

    // If the child widget didn't use the keypress, we can see
    // if it can be interpreted as a scrolling command.

    if (result == 0) {
      result = InterpretScrollKey(scrollpane, key);
    }
  }

  return result;
}

static void TXT_ScrollPaneMousePress(void * uncast_scrollpane,
                                     int    x,
                                     int    y,
                                     int    b) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);

  int scrollbars = NeedsScrollbars(scrollpane);

  if (b == TXT_MOUSE_SCROLLUP) {
    if (scrollbars & SCROLLBAR_VERTICAL) {
      scrollpane->y -= 3;
    } else if (scrollbars & SCROLLBAR_HORIZONTAL) {
      scrollpane->x -= 3;
    }

    return;
  } else if (b == TXT_MOUSE_SCROLLDOWN) {
    if (scrollbars & SCROLLBAR_VERTICAL) {
      scrollpane->y += 3;
    } else if (scrollbars & SCROLLBAR_HORIZONTAL) {
      scrollpane->x += 3;
    }

    return;
  }

  int rel_x = x - scrollpane->widget.x;
  int rel_y = y - scrollpane->widget.y;

  // Click on the horizontal scrollbar?
  if ((scrollbars & SCROLLBAR_HORIZONTAL) && rel_y == scrollpane->h) {
    if (rel_x == 0) {
      --scrollpane->x;
    } else if (rel_x == scrollpane->w - 1) {
      ++scrollpane->x;
    } else {
      int range   = FullWidth(scrollpane) - scrollpane->w;
      int bar_max = scrollpane->w - 3;

      scrollpane->x = ((rel_x - 1) * range + bar_max - 1) / bar_max;
    }

    return;
  }

  // Click on the vertical scrollbar?
  if ((scrollbars & SCROLLBAR_VERTICAL) && rel_x == scrollpane->w) {
    if (rel_y == 0) {
      --scrollpane->y;
    } else if (rel_y == scrollpane->h - 1) {
      ++scrollpane->y;
    } else {
      int range   = FullHeight(scrollpane) - scrollpane->h;
      int bar_max = scrollpane->h - 3;

      scrollpane->y = ((rel_y - 1) * range + bar_max - 1) / bar_max;
    }

    return;
  }

  if (scrollpane->child != nullptr) {
    TXT_WidgetMousePress(scrollpane->child, x, y, b);
  }
}

static void TXT_ScrollPaneLayout(void * uncast_scrollpane) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);

  SanityCheckScrollbars(scrollpane);

  // The child widget takes the same position as the scroll pane
  // itself, but is offset by the scroll position.

  if (scrollpane->child != nullptr) {
    scrollpane->child->x = scrollpane->widget.x - scrollpane->x;
    scrollpane->child->y = scrollpane->widget.y - scrollpane->y;

    TXT_LayoutWidget(scrollpane->child);
  }
}

static int TXT_ScrollPaneSelectable(void * uncast_scrollpane) {
  auto * scrollpane = reinterpret_cast<txt_scrollpane_t *>(uncast_scrollpane);

  // If scroll bars are displayed, the scroll pane must be selectable
  // so that we can use the arrow keys to scroll around.

  if (NeedsScrollbars(scrollpane)) {
    return 1;
  }

  // Otherwise, whether this is selectable depends on the child widget.

  return TXT_SelectableWidget(scrollpane->child);
}

txt_widget_class_t txt_scrollpane_class = {
  TXT_ScrollPaneSelectable,
  TXT_ScrollPaneSizeCalc,
  TXT_ScrollPaneDrawer,
  TXT_ScrollPaneKeyPress,
  TXT_ScrollPaneDestructor,
  TXT_ScrollPaneMousePress,
  TXT_ScrollPaneLayout,
  TXT_ScrollPaneFocused,
};

txt_scrollpane_t * TXT_NewScrollPane(int w, int h, void * uncast_target) {
  auto * target     = reinterpret_cast<txt_widget_t *>(uncast_target);
  auto * scrollpane = create_struct<txt_scrollpane_t>();

  TXT_InitWidget(scrollpane, &txt_scrollpane_class);
  scrollpane->w        = w;
  scrollpane->h        = h;
  scrollpane->x        = 0;
  scrollpane->y        = 0;
  scrollpane->child    = target;
  scrollpane->expand_w = w <= 0;
  scrollpane->expand_h = h <= 0;

  // Set parent pointer for inner widget.

  target->parent = &scrollpane->widget;

  return scrollpane;
}
