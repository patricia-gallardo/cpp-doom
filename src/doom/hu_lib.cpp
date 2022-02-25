//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:  heads-up text and input code
//

#include <cctype>

#include "doomkeys.hpp"
#include "v_video.hpp"
#include "i_swap.hpp"
#include "hu_lib.hpp"
#include "r_local.hpp"
#include "r_draw.hpp"
#include "v_trans.hpp" // [crispy] colored HUlib_drawTextLine()
#include "doomstat.hpp"

extern bool automapactive; // in AM_map.c

[[maybe_unused]] void HUlib_init() {
}

void HUlib_clearTextLine(hu_textline_t * t) {
  t->len         = 0;
  t->l[0]        = 0;
  t->needsupdate = true;
}

void HUlib_initTextLine(hu_textline_t * t,
                        int             x,
                        int             y,
                        patch_t **      f,
                        int             sc) {
  t->x  = x;
  t->y  = y;
  t->f  = f;
  t->sc = sc;
  HUlib_clearTextLine(t);
}

bool HUlib_addCharToTextLine(hu_textline_t * t,
                             char            ch) {

  if (t->len == HU_MAXLINELENGTH)
    return false;
  else {
    t->l[t->len++] = ch;
    t->l[t->len]   = 0;
    t->needsupdate = 4;
    return true;
  }
}

bool HUlib_delCharFromTextLine(hu_textline_t * t) {

  if (!t->len)
    return false;
  else {
    t->l[--t->len] = 0;
    t->needsupdate = 4;
    return true;
  }
}

void HUlib_drawTextLine(hu_textline_t * l,
                        bool            drawcursor) {
  // draw the new stuff
  int x = l->x;
  int y = l->y; // [crispy] support line breaks
  for (int i = 0; i < l->len; i++) {
    unsigned char c = static_cast<unsigned char>(toupper(l->l[i]));
    // [crispy] support multi-colored text lines
    if (c == cr_esc) {
      if (l->l[i + 1] >= '0' && l->l[i + 1] <= '0' + static_cast<int>(cr_t::CRMAX) - 1) {
        i++;
        dp_translation = (crispy->coloredhud & COLOREDHUD_TEXT) ? cr_colors[static_cast<int>(l->l[i] - '0')] : nullptr;
      }
    } else
      // [crispy] support line breaks
      if (c == '\n') {
        x = l->x;
        y += SHORT(l->f[0]->height) + 1;
      } else if (c != ' '
                 && c >= l->sc
                 && c <= '_') {
        int w = SHORT(l->f[c - l->sc]->width);
        if (x + w > ORIGWIDTH + DELTAWIDTH)
          break;
        V_DrawPatchDirect(x, y, l->f[c - l->sc]);
        x += w;
      } else {
        x += 4;
        if (x >= ORIGWIDTH + DELTAWIDTH)
          break;
      }
  }

  // draw the cursor if requested
  if (drawcursor
      && x + SHORT(l->f['_' - l->sc]->width) <= ORIGWIDTH + DELTAWIDTH) {
    V_DrawPatchDirect(x, y, l->f['_' - l->sc]);
  }
}

// sorta called by HU_Erase and just better darn get things straight
void HUlib_eraseTextLine(hu_textline_t * l) {
  // Only erases when NOT in automap and the screen is reduced,
  // and the text must either need updating or refreshing
  // (because of a recent change back from the automap)

  if (!g_doomstat_globals->automapactive && viewwindowx && (l->needsupdate || crispy->cleanscreenshot || crispy->screenshotmsg == 4)) {
    int lh = (SHORT(l->f[0]->height) + 1) << crispy->hires;
    // [crispy] support line breaks
    int yoffset = 1;
    for (int y = 0; y < l->len; y++) {
      if (l->l[y] == '\n') {
        yoffset++;
      }
    }
    lh *= yoffset;
    int y;
    for (y = (l->y << crispy->hires), yoffset = y * SCREENWIDTH; y < (l->y << crispy->hires) + lh; y++, yoffset += SCREENWIDTH) {
      if (y < viewwindowy || y >= viewwindowy + g_r_state_globals->viewheight)
        R_VideoErase(static_cast<unsigned int>(yoffset), SCREENWIDTH); // erase entire line
      else {
        R_VideoErase(static_cast<unsigned int>(yoffset), viewwindowx); // erase left border
        R_VideoErase(static_cast<unsigned int>(yoffset + viewwindowx + g_r_state_globals->scaledviewwidth), viewwindowx);
        // erase right border
      }
    }
  }

  if (l->needsupdate) l->needsupdate--;
}

void HUlib_initSText(hu_stext_t * s,
                     int          x,
                     int          y,
                     int          h,
                     patch_t **   font,
                     int          startchar,
                     bool *       on) {
  s->h      = h;
  s->on     = on;
  s->laston = true;
  s->cl     = 0;
  for (int i = 0; i < h; i++)
    HUlib_initTextLine(&s->l[i],
                       x,
                       y - i * (SHORT(font[0]->height) + 1),
                       font,
                       startchar);
}

void HUlib_addLineToSText(hu_stext_t * s) {
  // add a clear line
  if (++s->cl == s->h)
    s->cl = 0;
  HUlib_clearTextLine(&s->l[s->cl]);

  // everything needs updating
  for (int i = 0; i < s->h; i++)
    s->l[i].needsupdate = 4;
}

void HUlib_addMessageToSText(hu_stext_t * s,
                             const char * prefix,
                             const char * msg) {
  HUlib_addLineToSText(s);
  if (prefix)
    while (*prefix)
      HUlib_addCharToTextLine(&s->l[s->cl], *(prefix++));

  while (*msg)
    HUlib_addCharToTextLine(&s->l[s->cl], *(msg++));
}

void HUlib_drawSText(hu_stext_t * s) {
  if (!*s->on)
    return; // if not on, don't draw

  // draw everything
  for (int i = 0; i < s->h; i++) {
    int idx = s->cl - i;
    if (idx < 0)
      idx += s->h; // handle queue of lines

    hu_textline_t * l = &s->l[idx];

    // need a decision made here on whether to skip the draw
    HUlib_drawTextLine(l, false); // no cursor, please
  }
}

void HUlib_eraseSText(hu_stext_t * s) {
  for (int i = 0; i < s->h; i++) {
    if (s->laston && !*s->on)
      s->l[i].needsupdate = 4;
    HUlib_eraseTextLine(&s->l[i]);
  }
  s->laston = *s->on;
}

void HUlib_initIText(hu_itext_t * it,
                     int          x,
                     int          y,
                     patch_t **   font,
                     int          startchar,
                     bool *       on) {
  it->lm     = 0; // default left margin is start of text
  it->on     = on;
  it->laston = true;
  HUlib_initTextLine(&it->l, x, y, font, startchar);
}

// The following deletion routines adhere to the left margin restriction
void HUlib_delCharFromIText(hu_itext_t * it) {
  if (it->l.len != it->lm)
    HUlib_delCharFromTextLine(&it->l);
}

[[maybe_unused]] void HUlib_eraseLineFromIText(hu_itext_t * it) {
  while (it->lm != it->l.len)
    HUlib_delCharFromTextLine(&it->l);
}

// Resets left margin as well
void HUlib_resetIText(hu_itext_t * it) {
  it->lm = 0;
  HUlib_clearTextLine(&it->l);
}

[[maybe_unused]] void HUlib_addPrefixToIText(hu_itext_t * it,
                                             char *       str) {
  while (*str)
    HUlib_addCharToTextLine(&it->l, *(str++));
  it->lm = it->l.len;
}

// wrapper function for handling general keyed input.
// returns true if it ate the key
bool HUlib_keyInIText(hu_itext_t *  it,
                      unsigned char ch) {
  ch = static_cast<unsigned char>(toupper(ch));

  if (ch >= ' ' && ch <= '_')
    HUlib_addCharToTextLine(&it->l, static_cast<char>(ch));
  else if (ch == KEY_BACKSPACE)
    HUlib_delCharFromIText(it);
  else if (ch != KEY_ENTER)
    return false; // did not eat key

  return true; // ate the key
}

void HUlib_drawIText(hu_itext_t * it) {

  hu_textline_t * l = &it->l;

  if (!*it->on)
    return;
  HUlib_drawTextLine(l, true); // draw the line w/ cursor
}

void HUlib_eraseIText(hu_itext_t * it) {
  if (it->laston && !*it->on)
    it->l.needsupdate = 4;
  HUlib_eraseTextLine(&it->l);
  it->laston = *it->on;
}
