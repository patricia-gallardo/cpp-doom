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

#include <fmt/printf.h>

#include "memory.hpp"
#include "txt_gui.hpp"
#include "txt_io.hpp"
#include "txt_main.hpp"
#include "txt_utf8.hpp"

#include <new>

using txt_cliparea_t = struct txt_cliparea_s;

struct txt_cliparea_s {
  int              x1, x2;
  int              y1, y2;
  txt_cliparea_t * next;
};

// Array of border characters for drawing windows. The array looks like this:
//
// +-++
// | ||
// +-++
// +-++

static const int borders[4][4] = {
  {0xda,  0xc4, 0xc2, 0xbf},
  { 0xb3, ' ',  0xb3, 0xb3},
  { 0xc3, 0xc4, 0xc5, 0xb4},
  { 0xc0, 0xc4, 0xc1, 0xd9},
};

static txt_cliparea_t * cliparea = nullptr;

#define VALID_X(x) ((x) >= cliparea->x1 && (x) < cliparea->x2)
#define VALID_Y(y) ((y) >= cliparea->y1 && (y) < cliparea->y2)

[[maybe_unused]] void TXT_DrawDesktopBackground(cstring_view title) {
  unsigned char * screendata = TXT_GetScreenData();

  // Fill the screen with gradient characters

  unsigned char * p = screendata;

  for (int i = 0; i < TXT_SCREEN_W * TXT_SCREEN_H; ++i) {
    *p++ = 0xb1;
    *p++ = TXT_COLOR_GREY | (TXT_COLOR_BLUE << 4);
  }

  // Draw the top and bottom banners

  p = screendata;

  for (int i = 0; i < TXT_SCREEN_W; ++i) {
    *p++ = ' ';
    *p++ = TXT_COLOR_BLACK | (TXT_COLOR_GREY << 4);
  }

  p = screendata + (TXT_SCREEN_H - 1) * TXT_SCREEN_W * 2;

  for (int i = 0; i < TXT_SCREEN_W; ++i) {
    *p++ = ' ';
    *p++ = TXT_COLOR_BLACK | (TXT_COLOR_GREY << 4);
  }

  // Print the title

  TXT_GotoXY(0, 0);
  TXT_FGColor(TXT_COLOR_BLACK);
  TXT_BGColor(TXT_COLOR_GREY, 0);

  TXT_DrawString(" ");
  TXT_DrawString(title);
}

void TXT_DrawShadow(int x, int y, int w, int h) {
  unsigned char * screendata = TXT_GetScreenData();

  for (int y1 = y; y1 < y + h; ++y1) {
    unsigned char * p = screendata + (y1 * TXT_SCREEN_W + x) * 2;

    for (int x1 = x; x1 < x + w; ++x1) {
      if (VALID_X(x1) && VALID_Y(y1)) {
        p[1] = TXT_COLOR_DARK_GREY;
      }

      p += 2;
    }
  }
}

void TXT_DrawWindowFrame(cstring_view title, int x, int y, int w, int h) {
  txt_saved_colors_t colors {};
  TXT_SaveColors(&colors);
  TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);

  for (int y1 = y; y1 < y + h; ++y1) {
    // Select the appropriate row and column in the borders
    // array to pick the appropriate character to draw at
    // this location.
    //
    // Draw a horizontal line on the third line down, so we
    // draw a box around the title.
    int by = y1 == y                         ? 0 :
             y1 == y + 2 && title.c_str() != nullptr ? 2 :
             y1 == y + h - 1                 ? 3 :
                                               1;

    for (int x1 = x; x1 < x + w; ++x1) {
      int bx = x1 == x         ? 0 :
               x1 == x + w - 1 ? 3 :
                                 1;

      if (VALID_X(x1) && VALID_Y(y1)) {
        TXT_GotoXY(x1, y1);
        TXT_PutChar(borders[by][bx]);
      }
    }
  }

  // Draw the title

  if (title.c_str() != nullptr) {
    TXT_GotoXY(x + 1, y + 1);
    TXT_BGColor(TXT_COLOR_GREY, 0);
    TXT_FGColor(TXT_COLOR_BLUE);

    for (int x1 = 0; x1 < w - 2; ++x1) {
      TXT_DrawString(" ");
    }

    TXT_GotoXY(static_cast<int>(static_cast<unsigned int>(x) + (static_cast<unsigned int>(w) - TXT_UTF8_Strlen(title)) / 2), y + 1);
    TXT_DrawString(title);
  }

  // Draw the window's shadow.

  TXT_DrawShadow(x + 2, y + h, w, 1);
  TXT_DrawShadow(x + w, y + 1, 2, h);

  TXT_RestoreColors(&colors);
}

void TXT_DrawSeparator(int x, int y, int w) {
  unsigned char * data = TXT_GetScreenData();

  txt_saved_colors_t colors {};
  TXT_SaveColors(&colors);
  TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);

  if (!VALID_Y(y)) {
    return;
  }

  data += (y * TXT_SCREEN_W + x) * 2;

  for (int x1 = x; x1 < x + w; ++x1) {
    TXT_GotoXY(x1, y);
    int b = x1 == x         ? 0 :
            x1 == x + w - 1 ? 3 :
                              1;

    if (VALID_X(x1)) {
      // Read the current value from the screen
      // Check that it matches what the window should look like if
      // there is no separator, then apply the separator

      if (*data == borders[1][b]) {
        TXT_PutChar(borders[2][b]);
      }
    }

    data += 2;
  }

  TXT_RestoreColors(&colors);
}

// Alternative to TXT_DrawString() where the argument is a "code page
// string" - characters are in native code page format and not UTF-8.
void TXT_DrawCodePageString(cstring_view s) {
  int x, y = 0;
  TXT_GetXY(&x, &y);

  if (VALID_Y(y)) {
    int          x1 = x;
    const char * p;
    for (p = s.c_str(); *p != '\0'; ++p) {
      if (VALID_X(x1)) {
        TXT_GotoXY(x1, y);
        TXT_PutChar(*p);
      }

      x1 += 1;
    }
  }

  int end_x = static_cast<int>(static_cast<unsigned long>(x) + s.size());
  TXT_GotoXY(end_x, y);
}

static void PutUnicodeChar(unsigned int c) {
  // Treat control characters specially.
  if (c == '\n' || c == '\b') {
    TXT_PutChar(static_cast<int>(c));
    return;
  }

  // Map Unicode character into the symbol used to represent it in this
  // code page. For unrepresentable characters, print a fallback instead.
  // Note that we use TXT_PutSymbol() here because we just want to do a
  // raw write into the screen buffer.
  int d = TXT_UnicodeCharacter(c);

  if (d >= 0) {
    TXT_PutSymbol(d);
  } else {
    TXT_PutSymbol('\xa8');
  }
}

void TXT_DrawString(cstring_view s) {
  int          x, y;
  int          x1;
  const char * p;
  unsigned int c;

  TXT_GetXY(&x, &y);

  if (VALID_Y(y)) {
    x1 = x;

    for (p = s.c_str(); *p != '\0';) {
      c = TXT_DecodeUTF8(&p);

      if (c == 0) {
        break;
      }

      if (VALID_X(x1)) {
        TXT_GotoXY(x1, y);
        PutUnicodeChar(c);
      }

      x1 += 1;
    }
  }

  TXT_GotoXY(static_cast<int>(static_cast<unsigned int>(x) + TXT_UTF8_Strlen(s)), y);
}

void TXT_DrawHorizScrollbar(int x, int y, int w, int cursor, int range) {
  if (!VALID_Y(y)) {
    return;
  }

  txt_saved_colors_t colors {};
  TXT_SaveColors(&colors);
  TXT_FGColor(TXT_COLOR_BLACK);
  TXT_BGColor(TXT_COLOR_GREY, 0);

  TXT_GotoXY(x, y);
  TXT_PutChar('\x1b');

  int cursor_x = x + 1;

  if (range > 0) {
    cursor_x += (cursor * (w - 3)) / range;
  }

  if (cursor_x > x + w - 2) {
    cursor_x = x + w - 2;
  }

  for (int x1 = x + 1; x1 < x + w - 1; ++x1) {
    if (VALID_X(x1)) {
      if (x1 == cursor_x) {
        TXT_PutChar('\xdb');
      } else {
        TXT_PutChar('\xb1');
      }
    }
  }

  TXT_PutChar('\x1a');
  TXT_RestoreColors(&colors);
}

void TXT_DrawVertScrollbar(int x, int y, int h, int cursor, int range) {
  if (!VALID_X(x)) {
    return;
  }

  txt_saved_colors_t colors {};
  TXT_SaveColors(&colors);
  TXT_FGColor(TXT_COLOR_BLACK);
  TXT_BGColor(TXT_COLOR_GREY, 0);

  TXT_GotoXY(x, y);
  TXT_PutChar('\x18');

  int cursor_y = y + 1;

  if (cursor_y > y + h - 2) {
    cursor_y = y + h - 2;
  }

  if (range > 0) {
    cursor_y += (cursor * (h - 3)) / range;
  }

  for (int y1 = y + 1; y1 < y + h - 1; ++y1) {
    if (VALID_Y(y1)) {
      TXT_GotoXY(x, y1);

      if (y1 == cursor_y) {
        TXT_PutChar('\xdb');
      } else {
        TXT_PutChar('\xb1');
      }
    }
  }

  TXT_GotoXY(x, y + h - 1);
  TXT_PutChar('\x19');
  TXT_RestoreColors(&colors);
}

void TXT_InitClipArea() {
  if (cliparea == nullptr) {
    auto * mem     = malloc(sizeof(txt_cliparea_t));
    cliparea       = new (mem) txt_cliparea_t {};
    cliparea->x1   = 0;
    cliparea->x2   = TXT_SCREEN_W;
    cliparea->y1   = 0;
    cliparea->y2   = TXT_SCREEN_H;
    cliparea->next = nullptr;
  }
}

void TXT_PushClipArea(int x1, int x2, int y1, int y2) {
  auto * newarea = create_struct<txt_cliparea_t>();

  // Set the new clip area to the intersection of the old
  // area and the new one.

  newarea->x1 = cliparea->x1;
  newarea->x2 = cliparea->x2;
  newarea->y1 = cliparea->y1;
  newarea->y2 = cliparea->y2;

  if (x1 > newarea->x1)
    newarea->x1 = x1;
  if (x2 < newarea->x2)
    newarea->x2 = x2;
  if (y1 > newarea->y1)
    newarea->y1 = y1;
  if (y2 < newarea->y2)
    newarea->y2 = y2;

#if 0
    fmt::printf("New scrollable area: %i,%i-%i,%i\n", x1, y1, x2, y2);
#endif

  // Hook into the list

  newarea->next = cliparea;
  cliparea      = newarea;
}

void TXT_PopClipArea() {
  // Never pop the last entry

  if (cliparea->next == nullptr)
    return;

  // Unlink the last entry and delete

  txt_cliparea_t * next_cliparea = cliparea->next;
  free(cliparea);
  cliparea = next_cliparea;
}
