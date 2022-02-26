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
//
// DESCRIPTION:  the automap code
//

#include <array>
#include <cstdio>

#include <fmt/printf.h>

#include "deh_main.hpp"

#include "doomdef.hpp"
#include "p_local.hpp"
#include "st_stuff.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "i_timer.hpp"
#include "i_video.hpp"
#include "m_cheat.hpp"
#include "m_controls.hpp"
#include "m_misc.hpp"

// Needs access to LFB.
#include "v_video.hpp"

// State.
#include "doomstat.hpp"
#include "r_state.hpp"

// Data.
#include "dstrings.hpp"

#include "am_map.hpp"
#include "lump.hpp"
extern bool inhelpscreens; // [crispy]

// For use if I do walls with outsides/insides
constexpr auto REDS        = (256 - 5 * 16);
constexpr auto REDRANGE    = 16;
constexpr auto BLUES       = (256 - 4 * 16 + 8);
constexpr auto GREENS      = (7 * 16);
constexpr auto GREENRANGE  = 16;
constexpr auto GRAYS       = (6 * 16);
constexpr auto GRAYSRANGE  = 16;
constexpr auto BROWNS      = (4 * 16);
constexpr auto BROWNRANGE  = 16;
constexpr auto YELLOWS     = (256 - 32 + 7);
constexpr auto YELLOWRANGE = 1;
constexpr auto BLACK       = 0;
constexpr auto WHITE       = (256 - 47);

// Automap colors
constexpr auto                  BACKGROUND               = BLACK;
[[maybe_unused]] constexpr auto YOURCOLORS               = WHITE;
[[maybe_unused]] constexpr auto YOURRANGE                = 0;
constexpr auto                  WALLRANGE                = REDRANGE;
constexpr auto                  TSWALLCOLORS             = GRAYS;
[[maybe_unused]] constexpr auto TSWALLRANGE              = GRAYSRANGE;
[[maybe_unused]] constexpr auto FDWALLRANGE              = BROWNRANGE;
[[maybe_unused]] constexpr auto CDWALLRANGE              = YELLOWRANGE;
constexpr auto                  THINGCOLORS              = GREENS;
constexpr auto                  THINGRANGE               = GREENRANGE;
constexpr auto                  SECRETWALLCOLORS         = 252; // [crispy] purple
constexpr auto                  REVEALEDSECRETWALLCOLORS = 112; // [crispy] green
[[maybe_unused]] constexpr auto SECRETWALLRANGE          = WALLRANGE;
constexpr auto                  GRIDCOLORS               = (GRAYS + GRAYSRANGE / 2);
[[maybe_unused]] constexpr auto GRIDRANGE                = 0;
constexpr auto                  XHAIRCOLORS              = GRAYS;

#define CRISPY_HIGHLIGHT_REVEALED_SECRETS

// drawing stuff

constexpr auto AM_NUMMARKPOINTS = 10;

// scale on entry
constexpr auto INITSCALEMTOF = (.2 * FRACUNIT);
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
constexpr auto F_PANINC = 4;
// how much zoom-in per tic
// goes to 2x in 1 second
constexpr auto M_ZOOMIN = (static_cast<int>(1.02 * FRACUNIT));
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
constexpr auto M_ZOOMOUT = (static_cast<int>(FRACUNIT / 1.02));
// [crispy] zoom faster with the mouse wheel
constexpr auto M2_ZOOMIN  = (static_cast<int>(1.08 * FRACUNIT));
constexpr auto M2_ZOOMOUT = (static_cast<int>(FRACUNIT / 1.08));

// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = static_cast<fixed_t>(INITSCALEMTOF);

// translates between frame-buffer and map distances
// [crispy] fix int overflow that causes map and grid lines to disappear
template <typename T>
constexpr auto FTOM(T x) {
  return ((static_cast<int64_t>((x) << FRACBITS) * scale_ftom) >> FRACBITS);
}

template <typename T>
constexpr auto MTOF(T x) {
  return (((static_cast<int64_t>(x) * scale_mtof) >> FRACBITS) >> FRACBITS);
}

// location of window on screen
static int f_x;
static int f_y;

// size of window on screen
static int f_w;
static int f_h;

static int64_t m_x, m_y; // LL x,y where the window is on the map (map coords)

// translates between frame-buffer and map coordinates
template <typename T>
constexpr auto CXMTOF(T x) {
  return (f_x + MTOF((x)-m_x));
}

template <typename T>
constexpr auto CYMTOF(T y) {
  return (f_y + (f_h - MTOF((y)-m_y)));
}

// the following is crap
constexpr auto LINE_NEVERSEE = ML_DONTDRAW;

struct fpoint_t {
  int x, y;
};

struct fline_t {
  fpoint_t a, b;
};

struct mpoint_t {
  int64_t x, y;
};

struct mline_t {
  mpoint_t a, b;
};

struct islope_t {
  [[maybe_unused]] fixed_t slp;
  [[maybe_unused]] fixed_t islp;
};

enum keycolor_t
{
  no_key,
  red_key,
  yellow_key,
  blue_key
};

//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8 * PLAYERRADIUS) / 7)
mline_t player_arrow[] = {
  {{ -R + R / 8, 0 },      { R, 0 }              }, // -----
  { { R, 0 },              { R - R / 2, R / 4 }  }, // ----->
  { { R, 0 },              { R - R / 2, -R / 4 } },
  { { -R + R / 8, 0 },     { -R - R / 8, R / 4 } }, // >---->
  { { -R + R / 8, 0 },     { -R - R / 8, -R / 4 }},
  { { -R + 3 * R / 8, 0 }, { -R + R / 8, R / 4 } }, // >>--->
  { { -R + 3 * R / 8, 0 }, { -R + R / 8, -R / 4 }}
};
#undef R

#define R ((8 * PLAYERRADIUS) / 7)
mline_t cheat_player_arrow[] = {
  {{ -R + R / 8, 0 },                    { R, 0 }                           }, // -----
  { { R, 0 },                            { R - R / 2, R / 6 }               }, // ----->
  { { R, 0 },                            { R - R / 2, -R / 6 }              },
  { { -R + R / 8, 0 },                   { -R - R / 8, R / 6 }              }, // >----->
  { { -R + R / 8, 0 },                   { -R - R / 8, -R / 6 }             },
  { { -R + 3 * R / 8, 0 },               { -R + R / 8, R / 6 }              }, // >>----->
  { { -R + 3 * R / 8, 0 },               { -R + R / 8, -R / 6 }             },
  { { -R / 2, 0 },                       { -R / 2, -R / 6 }                 }, // >>-d--->
  { { -R / 2, -R / 6 },                  { -R / 2 + R / 6, -R / 6 }         },
  { { -R / 2 + R / 6, -R / 6 },          { -R / 2 + R / 6, R / 4 }          },
  { { -R / 6, 0 },                       { -R / 6, -R / 6 }                 }, // >>-dd-->
  { { -R / 6, -R / 6 },                  { 0, -R / 6 }                      },
  { { 0, -R / 6 },                       { 0, R / 4 }                       },
  { { R / 6, R / 4 },                    { R / 6, -R / 7 }                  }, // >>-ddt->
  { { R / 6, -R / 7 },                   { R / 6 + R / 32, -R / 7 - R / 32 }},
  { { R / 6 + R / 32, -R / 7 - R / 32 }, { R / 6 + R / 10, -R / 7 }         }
};
#undef R

#define R (FRACUNIT)
[[maybe_unused]] mline_t triangle_guy[] = {
  {{ static_cast<fixed_t>(-.867 * R), static_cast<fixed_t>(-.5 * R) }, { static_cast<fixed_t>(.867 * R), static_cast<fixed_t>(-.5 * R) } },
  { { static_cast<fixed_t>(.867 * R), static_cast<fixed_t>(-.5 * R) }, { static_cast<fixed_t>(0), static_cast<fixed_t>(R) }              },
  { { static_cast<fixed_t>(0), static_cast<fixed_t>(R) },              { static_cast<fixed_t>(-.867 * R), static_cast<fixed_t>(-.5 * R) }}
};
#undef R

#define R (FRACUNIT)
mline_t thintriangle_guy[] = {
  {{ static_cast<fixed_t>(-.5 * R), static_cast<fixed_t>(-.7 * R) }, { static_cast<fixed_t>(R), static_cast<fixed_t>(0) }            },
  { { static_cast<fixed_t>(R), static_cast<fixed_t>(0) },            { static_cast<fixed_t>(-.5 * R), static_cast<fixed_t>(.7 * R) } },
  { { static_cast<fixed_t>(-.5 * R), static_cast<fixed_t>(.7 * R) }, { static_cast<fixed_t>(-.5 * R), static_cast<fixed_t>(-.7 * R) }}
};
// [crispy] print keys as crosses
static mline_t cross_mark[] = {
  {{ -R, 0 },  { R, 0 }},
  { { 0, -R }, { 0, R }},
};
static mline_t square_mark[] = {
  {{ -R, 0 },  { 0, R } },
  { { 0, R },  { R, 0 } },
  { { R, 0 },  { 0, -R }},
  { { 0, -R }, { -R, 0 }},
};
#undef R

static int cheating = 0;
static int grid     = 0;

[[maybe_unused]] static int leveljuststarted = 1; // kluge until AM_LevelInit() is called

static int lightlev;                        // used for funky strobing effect
#define fb g_i_video_globals->I_VideoBuffer // [crispy] simplify
static int amclock;

static mpoint_t m_paninc;     // how far the window pans each tic (map coords)
static fixed_t  mtof_zoommul; // how far the window zooms in each tic (map coords)
static fixed_t  ftom_zoommul; // how far the window zooms in each tic (fb coords)

static int64_t m_x2, m_y2; // UR x,y where the window is on the map (map coords)

//
// width/height of window on map (map coords)
//
static int64_t m_w;
static int64_t m_h;

// based on level size
static fixed_t min_x;
static fixed_t min_y;
static fixed_t max_x;
static fixed_t max_y;

static fixed_t max_w; // max_x-min_x,
static fixed_t max_h; // max_y-min_y

// based on player size
[[maybe_unused]] static fixed_t min_w;
[[maybe_unused]] static fixed_t min_h;

static fixed_t min_scale_mtof; // used to tell when to stop zooming out
static fixed_t max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t old_m_w, old_m_h;
static int64_t old_m_x, old_m_y;

// old location used by the Follower routine
static mpoint_t f_oldloc;

static player_t * plr; // the player represented by an arrow

static patch_t * marknums[10];                 // numbers used for marking by the automap
static mpoint_t  markpoints[AM_NUMMARKPOINTS]; // where the points are
static int       markpointnum = 0;             // next point to be assigned

static int followplayer = 1; // specifies whether to follow the player around

cheatseq_t cheat_amap = CHEAT("iddt", 0);

static bool stopped = true;

// [crispy] automap rotate mode needs these early on
void            AM_rotate(int64_t * x, int64_t * y, angle_t a);
static void     AM_rotatePoint(mpoint_t * pt);
static mpoint_t mapcenter;
static angle_t  mapangle;

// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.

[[maybe_unused]] void AM_getIslope(mline_t *  ml,
                                   islope_t * is) {
  int dy = static_cast<int>(ml->a.y - ml->b.y);
  int dx = static_cast<int>(ml->b.x - ml->a.x);
  if (!dy)
    is->islp = (dx < 0 ? -std::numeric_limits<int32_t>::max() : std::numeric_limits<int32_t>::max());
  else
    is->islp = FixedDiv(dx, dy);
  if (!dx)
    is->slp = (dy < 0 ? -std::numeric_limits<int32_t>::max() : std::numeric_limits<int32_t>::max());
  else
    is->slp = FixedDiv(dy, dx);
}

//
//
//
void AM_activateNewScale() {
  m_x += m_w / 2;
  m_y += m_h / 2;
  m_w = FTOM(f_w);
  m_h = FTOM(f_h);
  m_x -= m_w / 2;
  m_y -= m_h / 2;
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

//
//
//
void AM_saveScaleAndLoc() {
  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;
}

//
//
//
void AM_restoreScaleAndLoc() {

  m_w = old_m_w;
  m_h = old_m_h;
  if (!followplayer) {
    m_x = old_m_x;
    m_y = old_m_y;
  } else {
    m_x = plr->mo->x - m_w / 2;
    m_y = plr->mo->y - m_h / 2;
  }
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;

  // Change the scaling multipliers
  scale_mtof = FixedDiv(f_w << FRACBITS, static_cast<fixed_t>(m_w));
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// adds a marker at the current location
//
void AM_addMark() {
  // [crispy] keep the map static in overlay mode
  // if not following the player
  if (!(!followplayer && crispy->automapoverlay)) {
    markpoints[markpointnum].x = m_x + m_w / 2;
    markpoints[markpointnum].y = m_y + m_h / 2;
  } else {
    markpoints[markpointnum].x = plr->mo->x;
    markpoints[markpointnum].y = plr->mo->y;
  }
  markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;
}

//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
void AM_findMinMaxBoundaries() {
  min_x = min_y = std::numeric_limits<int32_t>::max();
  max_x = max_y = -std::numeric_limits<int32_t>::max();

  for (int i = 0; i < g_r_state_globals->numvertexes; i++) {
    if (g_r_state_globals->vertexes[i].x < min_x)
      min_x = g_r_state_globals->vertexes[i].x;
    else if (g_r_state_globals->vertexes[i].x > max_x)
      max_x = g_r_state_globals->vertexes[i].x;

    if (g_r_state_globals->vertexes[i].y < min_y)
      min_y = g_r_state_globals->vertexes[i].y;
    else if (g_r_state_globals->vertexes[i].y > max_y)
      max_y = g_r_state_globals->vertexes[i].y;
  }

  // [crispy] cope with huge level dimensions which span the entire INT range
  max_w = max_x / 2 - min_x / 2;
  max_h = max_y / 2 - min_y / 2;

  min_w = 2 * PLAYERRADIUS; // const? never changed?
  min_h = 2 * PLAYERRADIUS;

  fixed_t a = FixedDiv(f_w << FRACBITS, max_w);
  fixed_t b = FixedDiv(f_h << FRACBITS, max_h);

  min_scale_mtof = a < b ? a / 2 : b / 2;
  max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * PLAYERRADIUS);
}

//
//
//
void AM_changeWindowLoc() {
  if (m_paninc.x || m_paninc.y) {
    followplayer = 0;
    f_oldloc.x   = std::numeric_limits<int32_t>::max();
  }

  int64_t incx = m_paninc.x;
  int64_t incy = m_paninc.y;
  if (crispy->automaprotate) {
    // Subtracting from 0u avoids compiler warnings
    AM_rotate(&incx, &incy, 0u - mapangle);
  }
  m_x += incx;
  m_y += incy;

  if (m_x + m_w / 2 > max_x)
    m_x = max_x - m_w / 2;
  else if (m_x + m_w / 2 < min_x)
    m_x = min_x - m_w / 2;

  if (m_y + m_h / 2 > max_y)
    m_y = max_y - m_h / 2;
  else if (m_y + m_h / 2 < min_y)
    m_y = min_y - m_h / 2;

  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;

  // [crispy] reset after moving with the mouse
  if (f_oldloc.y == std::numeric_limits<int32_t>::max()) {
    m_paninc.x = 0;
    m_paninc.y = 0;
  }
}

//
//
//
void AM_initVariables() {
  static event_t st_notify = { ev_keyup, AM_MSGENTERED, 0, 0 };

  g_doomstat_globals->automapactive = true;
  //  fb = I_VideoBuffer; // [crispy] simplify

  f_oldloc.x = std::numeric_limits<int32_t>::max();
  amclock    = 0;
  lightlev   = 0;

  m_paninc.x = m_paninc.y = 0;
  ftom_zoommul            = FRACUNIT;
  mtof_zoommul            = FRACUNIT;

  m_w = FTOM(f_w);
  m_h = FTOM(f_h);

  // find player to center on initially
  if (g_doomstat_globals->playeringame[g_doomstat_globals->consoleplayer]) {
    plr = &g_doomstat_globals->players[g_doomstat_globals->consoleplayer];
  } else {
    plr = &g_doomstat_globals->players[0];

    for (int pnum = 0; pnum < MAXPLAYERS; pnum++) {
      if (g_doomstat_globals->playeringame[pnum]) {
        plr = &g_doomstat_globals->players[pnum];
        break;
      }
    }
  }

  m_x = plr->mo->x - m_w / 2;
  m_y = plr->mo->y - m_h / 2;
  AM_changeWindowLoc();

  // for saving & restoring
  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;

  // inform the status bar of the change
  ST_Responder(&st_notify);
}

//
//
//
void AM_loadPics() {
  char namebuf[9];

  for (int i = 0; i < 10; i++) {
    DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
    marknums[i] = cache_lump_name<patch_t *>(namebuf, PU_STATIC);
  }
}

void AM_unloadPics() {
  char namebuf[9];

  for (int i = 0; i < 10; i++) {
    DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
    W_ReleaseLumpName(namebuf);
  }
}

void AM_clearMarks() {
  for (auto & markpoint : markpoints)
    markpoint.x = -1; // means empty
  markpointnum = 0;
}

//
// should be called at the start of every level
// right now, i figure it out myself
//
void AM_LevelInit() {
  leveljuststarted = 0;

  f_x = f_y = 0;
  f_w       = SCREENWIDTH;
  f_h       = SCREENHEIGHT;
  // [crispy] automap without status bar in widescreen mode
  if (!crispy->widescreen) {
    f_h -= (ST_HEIGHT << crispy->hires);
  }

  AM_clearMarks();

  AM_findMinMaxBoundaries();
  // [crispy] initialize zoomlevel on all maps so that a 4096 units
  // square map would just fit in (MAP01 is 3376x3648 units)
  fixed_t a  = FixedDiv(f_w, (max_w >> FRACBITS < 2048) ? 2 * (max_w >> FRACBITS) : 4096);
  fixed_t b  = FixedDiv(f_h, (max_h >> FRACBITS < 2048) ? 2 * (max_h >> FRACBITS) : 4096);
  scale_mtof = FixedDiv(a < b ? a : b, static_cast<int>(0.7 * FRACUNIT));
  if (scale_mtof > max_scale_mtof)
    scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

void AM_ReInit() {
  f_w = SCREENWIDTH;
  f_h = SCREENHEIGHT;
  // [crispy] automap without status bar in widescreen mode
  if (!crispy->widescreen) {
    f_h -= (ST_HEIGHT << crispy->hires);
  }

  AM_findMinMaxBoundaries();

  scale_mtof = crispy->hires ? scale_mtof * 2 : scale_mtof / 2;
  if (scale_mtof > max_scale_mtof)
    scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
//
//
void AM_Stop() {
  static event_t st_notify = { {}, ev_keyup, AM_MSGEXITED, 0 };

  AM_unloadPics();
  g_doomstat_globals->automapactive = false;
  ST_Responder(&st_notify);
  stopped = true;
}

//
//
//
// [crispy] moved here for extended savegames
static int lastlevel = -1, lastepisode = -1;
void       AM_Start() {
  if (!stopped) AM_Stop();
  stopped = false;
  if (lastlevel != g_doomstat_globals->gamemap || lastepisode != g_doomstat_globals->gameepisode) {
    AM_LevelInit();
    lastlevel   = g_doomstat_globals->gamemap;
    lastepisode = g_doomstat_globals->gameepisode;
  }
  // [crispy] reset IDDT cheat when re-starting map during demo recording
  else if (g_doomstat_globals->demorecording) {
    cheating = 0;
  }
  AM_initVariables();
  AM_loadPics();
}

//
// set the window scale to the maximum size
//
void AM_minOutWindowScale() {
  scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

//
// set the window scale to the minimum size
//
void AM_maxOutWindowScale() {
  scale_mtof = max_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

//
// Handle events (user inputs) in automap mode
//
bool AM_Responder(event_t * ev) {

  static int  bigstate = 0;
  static char buffer[20];

  bool rc = false;

  if (ev->type == ev_joystick && g_m_controls_globals->joybautomap >= 0
      && (ev->data1 & (1 << g_m_controls_globals->joybautomap)) != 0) {
    g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 5);

    if (!g_doomstat_globals->automapactive) {
      AM_Start();
      g_doomstat_globals->viewactive = false;
    } else {
      bigstate                       = 0;
      g_doomstat_globals->viewactive = true;
      AM_Stop();
    }

    return true;
  }

  if (!g_doomstat_globals->automapactive) {
    if (ev->type == ev_keydown && ev->data1 == g_m_controls_globals->key_map_toggle) {
      AM_Start();
      g_doomstat_globals->viewactive = false;
      rc                             = true;
    }
  }
  // [crispy] zoom and move Automap with the mouse (wheel)
  else if (ev->type == ev_mouse && !crispy->automapoverlay && !g_doomstat_globals->menuactive && !inhelpscreens) {
    if (g_m_controls_globals->mousebprevweapon >= 0 && ev->data1 & (1 << g_m_controls_globals->mousebprevweapon)) {
      mtof_zoommul = M2_ZOOMOUT;
      ftom_zoommul = M2_ZOOMIN;
      rc           = true;
    } else if (g_m_controls_globals->mousebnextweapon >= 0 && ev->data1 & (1 << g_m_controls_globals->mousebnextweapon)) {
      mtof_zoommul = M2_ZOOMIN;
      ftom_zoommul = M2_ZOOMOUT;
      rc           = true;
    } else if (!followplayer && (ev->data2 || ev->data3)) {
      // [crispy] mouse sensitivity for strafe
      m_paninc.x = FTOM(ev->data2 * (g_doomstat_globals->mouseSensitivity_x2 + 5) / 80);
      m_paninc.y = FTOM(ev->data3 * (g_doomstat_globals->mouseSensitivity_x2 + 5) / 80);
      f_oldloc.y = std::numeric_limits<int32_t>::max();
      rc         = true;
    }
  } else if (ev->type == ev_keydown) {
    rc      = true;
    int key = ev->data1;

    if (key == g_m_controls_globals->key_map_east) // pan right
    {
      // [crispy] keep the map static in overlay mode
      // if not following the player
      if (!followplayer && !crispy->automapoverlay)
        m_paninc.x = crispy->fliplevels ? -FTOM(F_PANINC) : FTOM(F_PANINC);
      else
        rc = false;
    } else if (key == g_m_controls_globals->key_map_west) // pan left
    {
      if (!followplayer && !crispy->automapoverlay)
        m_paninc.x = crispy->fliplevels ? FTOM(F_PANINC) : -FTOM(F_PANINC);
      else
        rc = false;
    } else if (key == g_m_controls_globals->key_map_north) // pan up
    {
      if (!followplayer && !crispy->automapoverlay)
        m_paninc.y = FTOM(F_PANINC);
      else
        rc = false;
    } else if (key == g_m_controls_globals->key_map_south) // pan down
    {
      if (!followplayer && !crispy->automapoverlay)
        m_paninc.y = -FTOM(F_PANINC);
      else
        rc = false;
    } else if (key == g_m_controls_globals->key_map_zoomout) // zoom out
    {
      mtof_zoommul = M_ZOOMOUT;
      ftom_zoommul = M_ZOOMIN;
    } else if (key == g_m_controls_globals->key_map_zoomin) // zoom in
    {
      mtof_zoommul = M_ZOOMIN;
      ftom_zoommul = M_ZOOMOUT;
    } else if (key == g_m_controls_globals->key_map_toggle) {
      bigstate                       = 0;
      g_doomstat_globals->viewactive = true;
      AM_Stop();
    } else if (key == g_m_controls_globals->key_map_maxzoom) {
      bigstate = !bigstate;
      if (bigstate) {
        AM_saveScaleAndLoc();
        AM_minOutWindowScale();
      } else
        AM_restoreScaleAndLoc();
    } else if (key == g_m_controls_globals->key_map_follow) {
      followplayer = !followplayer;
      f_oldloc.x   = std::numeric_limits<int32_t>::max();
      if (followplayer)
        plr->message = DEH_String(AMSTR_FOLLOWON);
      else
        plr->message = DEH_String(AMSTR_FOLLOWOFF);
    } else if (key == g_m_controls_globals->key_map_grid) {
      grid = !grid;
      if (grid)
        plr->message = DEH_String(AMSTR_GRIDON);
      else
        plr->message = DEH_String(AMSTR_GRIDOFF);
    } else if (key == g_m_controls_globals->key_map_mark) {
      M_snprintf(buffer, sizeof(buffer), "%s %d", DEH_String(AMSTR_MARKEDSPOT), markpointnum);
      plr->message = buffer;
      AM_addMark();
    } else if (key == g_m_controls_globals->key_map_clearmark) {
      AM_clearMarks();
      plr->message = DEH_String(AMSTR_MARKSCLEARED);
    } else if (key == g_m_controls_globals->key_map_overlay) {
      // [crispy] force redraw status bar
      inhelpscreens = true;

      crispy->automapoverlay = !crispy->automapoverlay;
      if (crispy->automapoverlay)
        plr->message = DEH_String(AMSTR_OVERLAYON);
      else
        plr->message = DEH_String(AMSTR_OVERLAYOFF);
    } else if (key == g_m_controls_globals->key_map_rotate) {
      crispy->automaprotate = !crispy->automaprotate;
      if (crispy->automaprotate)
        plr->message = DEH_String(AMSTR_ROTATEON);
      else
        plr->message = DEH_String(AMSTR_ROTATEOFF);
    } else {
      rc = false;
    }

    if ((!g_doomstat_globals->deathmatch || g_doomstat_globals->gameversion <= exe_doom_1_8)
        && cht_CheckCheat(&cheat_amap, static_cast<char>(ev->data2))) {
      rc       = false;
      cheating = (cheating + 1) % 3;
    }
  } else if (ev->type == ev_keyup) {
    rc      = false;
    int key = ev->data1;

    if (key == g_m_controls_globals->key_map_east) {
      if (!followplayer) m_paninc.x = 0;
    } else if (key == g_m_controls_globals->key_map_west) {
      if (!followplayer) m_paninc.x = 0;
    } else if (key == g_m_controls_globals->key_map_north) {
      if (!followplayer) m_paninc.y = 0;
    } else if (key == g_m_controls_globals->key_map_south) {
      if (!followplayer) m_paninc.y = 0;
    } else if (key == g_m_controls_globals->key_map_zoomout || key == g_m_controls_globals->key_map_zoomin) {
      mtof_zoommul = FRACUNIT;
      ftom_zoommul = FRACUNIT;
    }
  }

  return rc;
}

//
// Zooming
//
void AM_changeWindowScale() {

  // Change the scaling multipliers
  scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

  // [crispy] reset after zooming with the mouse wheel
  if (ftom_zoommul == M2_ZOOMIN || ftom_zoommul == M2_ZOOMOUT) {
    mtof_zoommul = FRACUNIT;
    ftom_zoommul = FRACUNIT;
  }

  if (scale_mtof < min_scale_mtof)
    AM_minOutWindowScale();
  else if (scale_mtof > max_scale_mtof)
    AM_maxOutWindowScale();
  else
    AM_activateNewScale();
}

//
//
//
void AM_doFollowPlayer() {

  if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y) {
    m_x        = FTOM(MTOF(plr->mo->x)) - m_w / 2;
    m_y        = FTOM(MTOF(plr->mo->y)) - m_h / 2;
    m_x2       = m_x + m_w;
    m_y2       = m_y + m_h;
    f_oldloc.x = plr->mo->x;
    f_oldloc.y = plr->mo->y;

    //  m_x = FTOM(MTOF(plr->mo->x - m_w/2));
    //  m_y = FTOM(MTOF(plr->mo->y - m_h/2));
    //  m_x = plr->mo->x - m_w/2;
    //  m_y = plr->mo->y - m_h/2;
  }
}

//
//
//
[[maybe_unused]] void AM_updateLightLev() {
  static int nexttic = 0;
  // static int litelevels[] = { 0, 3, 5, 6, 6, 7, 7, 7 };
  static int    litelevels[]  = { 0, 4, 7, 10, 12, 14, 15, 15 };
  static size_t litelevelscnt = 0;

  // Change light level
  if (amclock > nexttic) {
    lightlev = litelevels[litelevelscnt++];
    if (litelevelscnt == std::size(litelevels)) litelevelscnt = 0;
    nexttic = amclock + 6 - (amclock % 6);
  }
}

//
// Updates on Game Tick
//
void AM_Ticker() {

  if (!g_doomstat_globals->automapactive)
    return;

  amclock++;

  if (followplayer)
    AM_doFollowPlayer();

  // Change the zoom if necessary
  if (ftom_zoommul != FRACUNIT)
    AM_changeWindowScale();

  // Change x,y location
  if (m_paninc.x || m_paninc.y)
    AM_changeWindowLoc();

  // Update light level
  // AM_updateLightLev();

  // [crispy] required for AM_rotatePoint()
  if (crispy->automaprotate) {
    mapcenter.x = m_x + m_w / 2;
    mapcenter.y = m_y + m_h / 2;
    // [crispy] keep the map static in overlay mode
    // if not following the player
    if (!(!followplayer && crispy->automapoverlay))
      mapangle = ANG90 - g_r_state_globals->viewangle;
  }
}

//
// Clear automap frame buffer.
//
void AM_clearFB(int color) {
  std::memset(g_i_video_globals->I_VideoBuffer, color, static_cast<unsigned long>(f_w * f_h) * sizeof(*g_i_video_globals->I_VideoBuffer));
}

//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
//
bool AM_clipMline(mline_t * ml,
                  fline_t * fl) {
  enum
  {
    LEFT   = 1,
    RIGHT  = 2,
    BOTTOM = 4,
    TOP    = 8
  };

  int outcode1 = 0;
  int outcode2 = 0;
  int outside  = 0;

  fpoint_t tmp;
  int      dx = 0;
  int      dy = 0;

#define DOOUTCODE(oc, mx, my) \
  (oc) = 0;                   \
  if ((my) < 0)               \
    (oc) |= TOP;              \
  else if ((my) >= f_h)       \
    (oc) |= BOTTOM;           \
  if ((mx) < 0)               \
    (oc) |= LEFT;             \
  else if ((mx) >= f_w)       \
    (oc) |= RIGHT;

  // do trivial rejects and outcodes
  if (ml->a.y > m_y2)
    outcode1 = TOP;
  else if (ml->a.y < m_y)
    outcode1 = BOTTOM;

  if (ml->b.y > m_y2)
    outcode2 = TOP;
  else if (ml->b.y < m_y)
    outcode2 = BOTTOM;

  if (outcode1 & outcode2)
    return false; // trivially outside

  if (ml->a.x < m_x)
    outcode1 |= LEFT;
  else if (ml->a.x > m_x2)
    outcode1 |= RIGHT;

  if (ml->b.x < m_x)
    outcode2 |= LEFT;
  else if (ml->b.x > m_x2)
    outcode2 |= RIGHT;

  if (outcode1 & outcode2)
    return false; // trivially outside

  // transform to frame-buffer coordinates.
  fl->a.x = static_cast<int>(CXMTOF(ml->a.x));
  fl->a.y = static_cast<int>(CYMTOF(ml->a.y));
  fl->b.x = static_cast<int>(CXMTOF(ml->b.x));
  fl->b.y = static_cast<int>(CYMTOF(ml->b.y));

  DOOUTCODE(outcode1, fl->a.x, fl->a.y);
  DOOUTCODE(outcode2, fl->b.x, fl->b.y);

  if (outcode1 & outcode2)
    return false;

  while (outcode1 | outcode2) {
    // may be partially inside box
    // find an outside point
    if (outcode1)
      outside = outcode1;
    else
      outside = outcode2;

    // clip to each side
    if (outside & TOP) {
      dy    = fl->a.y - fl->b.y;
      dx    = fl->b.x - fl->a.x;
      tmp.x = fl->a.x + (dx * (fl->a.y)) / dy;
      tmp.y = 0;
    } else if (outside & BOTTOM) {
      dy    = fl->a.y - fl->b.y;
      dx    = fl->b.x - fl->a.x;
      tmp.x = fl->a.x + (dx * (fl->a.y - f_h)) / dy;
      tmp.y = f_h - 1;
    } else if (outside & RIGHT) {
      dy    = fl->b.y - fl->a.y;
      dx    = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (dy * (f_w - 1 - fl->a.x)) / dx;
      tmp.x = f_w - 1;
    } else if (outside & LEFT) {
      dy    = fl->b.y - fl->a.y;
      dx    = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (dy * (-fl->a.x)) / dx;
      tmp.x = 0;
    } else {
      tmp.x = 0;
      tmp.y = 0;
    }

    if (outside == outcode1) {
      fl->a = tmp;
      DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    } else {
      fl->b = tmp;
      DOOUTCODE(outcode2, fl->b.x, fl->b.y);
    }

    if (outcode1 & outcode2)
      return false; // trivially outside
  }

  return true;
}
#undef DOOUTCODE

//
// Classic Bresenham w/ whatever optimizations needed for speed
//
void AM_drawFline(fline_t * fl,
                  int       color) {
  int x  = 0;
  int y  = 0;
  int dx = 0;
  int dy = 0;
  int sx = 0;
  int sy = 0;
  int ax = 0;
  int ay = 0;
  int d  = 0;

  static int fuck = 0;

  // For debugging only
  if (fl->a.x < 0 || fl->a.x >= f_w
      || fl->a.y < 0 || fl->a.y >= f_h
      || fl->b.x < 0 || fl->b.x >= f_w
      || fl->b.y < 0 || fl->b.y >= f_h) {
    fmt::fprintf(stderr, "fuck %d \r", fuck++);
    return;
  }

#ifndef CRISPY_TRUECOLOR
  auto PUTDOT = [](auto xx, auto yy, auto cc) { fb[(yy)*f_w + (g_r_state_globals->flipscreenwidth[xx])] = (cc); };
#else
#define PUTDOT(xx, yy, cc) fb[(yy)*f_w + (flipscreenwidth[xx])] = (colormaps[(cc)])
#endif

  dx = fl->b.x - fl->a.x;
  ax = 2 * (dx < 0 ? -dx : dx);
  sx = dx < 0 ? -1 : 1;

  dy = fl->b.y - fl->a.y;
  ay = 2 * (dy < 0 ? -dy : dy);
  sy = dy < 0 ? -1 : 1;

  x = fl->a.x;
  y = fl->a.y;

  if (ax > ay) {
    d = ay - ax / 2;
    while (true) {
      PUTDOT(x, y, static_cast<pixel_t>(color));
      if (x == fl->b.x) return;
      if (d >= 0) {
        y += sy;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  } else {
    d = ax - ay / 2;
    while (true) {
      PUTDOT(x, y, static_cast<pixel_t>(color));
      if (y == fl->b.y) return;
      if (d >= 0) {
        x += sx;
        d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
}

//
// Clip lines, draw visible part sof lines.
//
void AM_drawMline(mline_t * ml,
                  int       color) {
  static fline_t fl;

  if (AM_clipMline(ml, &fl))
    AM_drawFline(&fl, color); // draws it on frame buffer using fb coords
}

//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
void AM_drawGrid(int color) {
  // Figure out start of vertical gridlines
  int64_t start = m_x;
  if (crispy->automaprotate) {
    start -= m_h / 2;
  }
  // [crispy] fix losing grid lines near the automap boundary
  if ((start - g_p_local_globals->bmaporgx) % (MAPBLOCKUNITS << FRACBITS))
    start += // (MAPBLOCKUNITS<<FRACBITS)
        -((start - g_p_local_globals->bmaporgx) % (MAPBLOCKUNITS << FRACBITS));
  int64_t end = m_x + m_w;
  if (crispy->automaprotate) {
    end += m_h / 2;
  }

  // draw vertical gridlines
  for (int64_t x = start; x < end; x += (MAPBLOCKUNITS << FRACBITS)) {
    mline_t ml;
    ml.a.x = x;
    ml.b.x = x;
    // [crispy] moved here
    ml.a.y = m_y;
    ml.b.y = m_y + m_h;
    if (crispy->automaprotate) {
      ml.a.y -= m_w / 2;
      ml.b.y += m_w / 2;
      AM_rotatePoint(&ml.a);
      AM_rotatePoint(&ml.b);
    }
    AM_drawMline(&ml, color);
  }

  // Figure out start of horizontal gridlines
  start = m_y;
  if (crispy->automaprotate) {
    start -= m_w / 2;
  }
  // [crispy] fix losing grid lines near the automap boundary
  if ((start - g_p_local_globals->bmaporgy) % (MAPBLOCKUNITS << FRACBITS))
    start += // (MAPBLOCKUNITS<<FRACBITS)
        -((start - g_p_local_globals->bmaporgy) % (MAPBLOCKUNITS << FRACBITS));
  end = m_y + m_h;
  if (crispy->automaprotate) {
    end += m_w / 2;
  }

  // draw horizontal gridlines
  for (int64_t y = start; y < end; y += (MAPBLOCKUNITS << FRACBITS)) {
    mline_t ml;
    ml.a.y = y;
    ml.b.y = y;
    // [crispy] moved here
    ml.a.x = m_x;
    ml.b.x = m_x + m_w;
    if (crispy->automaprotate) {
      ml.a.x -= m_h / 2;
      ml.b.x += m_h / 2;
      AM_rotatePoint(&ml.a);
      AM_rotatePoint(&ml.b);
    }
    AM_drawMline(&ml, color);
  }
}

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//

// [crispy] keyed linedefs (PR, P1, SR, S1)
static keycolor_t AM_DoorColor(int type) {
  if (crispy->extautomap) {
    switch (type) {
    case 26:
    case 32:
    case 99:
    case 133:
      return blue_key;
    case 27:
    case 34:
    case 136:
    case 137:
      return yellow_key;
    case 28:
    case 33:
    case 134:
    case 135:
      return red_key;
    }
  }
  return no_key;
}

void AM_drawWalls() {
  static mline_t l;

  for (int i = 0; i < g_r_state_globals->numlines; i++) {
    l.a.x = g_r_state_globals->lines[i].v1->x;
    l.a.y = g_r_state_globals->lines[i].v1->y;
    l.b.x = g_r_state_globals->lines[i].v2->x;
    l.b.y = g_r_state_globals->lines[i].v2->y;
    if (crispy->automaprotate) {
      AM_rotatePoint(&l.a);
      AM_rotatePoint(&l.b);
    }
    if (cheating || (g_r_state_globals->lines[i].flags & ML_MAPPED)) {
      if ((g_r_state_globals->lines[i].flags & LINE_NEVERSEE) && !cheating)
        continue;
      {
        // [crispy] draw keyed doors in their respective colors
        // (no Boom multiple keys)
        keycolor_t amd = no_key;
        if (!(g_r_state_globals->lines[i].flags & ML_SECRET) && (amd = AM_DoorColor(g_r_state_globals->lines[i].special)) > no_key) {
          switch (amd) {
          case blue_key:
            AM_drawMline(&l, BLUES);
            continue;
          case yellow_key:
            AM_drawMline(&l, YELLOWS);
            continue;
          case red_key:
            AM_drawMline(&l, REDS);
            continue;
          default:
            // [crispy] it should be impossible to reach here
            break;
          }
        }
      }
      // [crispy] draw exit lines in white (no Boom exit lines 197, 198)
      // NB: Choco does not have this at all, Boom/PrBoom+ have this disabled by default
      if (crispy->extautomap && (g_r_state_globals->lines[i].special == 11 || g_r_state_globals->lines[i].special == 51 || g_r_state_globals->lines[i].special == 52 || g_r_state_globals->lines[i].special == 124)) {
        AM_drawMline(&l, WHITE);
        continue;
      }

      auto WALLCOLORS   = []() { return (crispy->extautomap ? 23 : REDS); };     // [crispy] red-brown
      auto FDWALLCOLORS = []() { return (crispy->extautomap ? 55 : BROWNS); };   // [crispy] lt brown
      auto CDWALLCOLORS = []() { return (crispy->extautomap ? 215 : YELLOWS); }; // [crispy] orange

      if (!g_r_state_globals->lines[i].backsector) {
        // [crispy] draw 1S secret sector boundaries in purple
        if (crispy->extautomap && cheating && (g_r_state_globals->lines[i].frontsector->special == 9))
          AM_drawMline(&l, SECRETWALLCOLORS);
#if defined CRISPY_HIGHLIGHT_REVEALED_SECRETS
        // [crispy] draw revealed secret sector boundaries in green
        else if (crispy->extautomap && crispy->secretmessage && (g_r_state_globals->lines[i].frontsector->oldspecial == 9))
          AM_drawMline(&l, REVEALEDSECRETWALLCOLORS);
#endif
        else
          AM_drawMline(&l, WALLCOLORS() + lightlev);
      } else {
        // [crispy] draw teleporters in green
        // and also WR teleporters 97 if they are not secret
        // (no monsters-only teleporters 125, 126; no Boom teleporters)
        if (g_r_state_globals->lines[i].special == 39 || (crispy->extautomap && !(g_r_state_globals->lines[i].flags & ML_SECRET) && g_r_state_globals->lines[i].special == 97)) { // teleporters
          AM_drawMline(&l, crispy->extautomap ? (GREENS + GREENRANGE / 2) : (WALLCOLORS() + WALLRANGE / 2));
        } else if (g_r_state_globals->lines[i].flags & ML_SECRET) // secret door
        {
          AM_drawMline(&l, WALLCOLORS() + lightlev);
        }
#if defined CRISPY_HIGHLIGHT_REVEALED_SECRETS
        // [crispy] draw revealed secret sector boundaries in green
        else if (crispy->extautomap && crispy->secretmessage && (g_r_state_globals->lines[i].backsector->oldspecial == 9 || g_r_state_globals->lines[i].frontsector->oldspecial == 9)) {
          AM_drawMline(&l, REVEALEDSECRETWALLCOLORS);
        }
#endif
        // [crispy] draw 2S secret sector boundaries in purple
        else if (crispy->extautomap && cheating && (g_r_state_globals->lines[i].backsector->special == 9 || g_r_state_globals->lines[i].frontsector->special == 9)) {
          AM_drawMline(&l, SECRETWALLCOLORS);
        } else if (g_r_state_globals->lines[i].backsector->floorheight
                   != g_r_state_globals->lines[i].frontsector->floorheight) {
          AM_drawMline(&l, FDWALLCOLORS() + lightlev); // floor level change
        } else if (g_r_state_globals->lines[i].backsector->ceilingheight
                   != g_r_state_globals->lines[i].frontsector->ceilingheight) {
          AM_drawMline(&l, CDWALLCOLORS() + lightlev); // ceiling level change
        } else if (cheating) {
          AM_drawMline(&l, TSWALLCOLORS + lightlev);
        }
      }
    } else if (plr->powers[pw_allmap]) {
      if (!(g_r_state_globals->lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, GRAYS + 3);
    }
  }
}

//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
void AM_rotate(int64_t * x,
               int64_t * y,
               angle_t   a) {
  int64_t tmpx =
      FixedMul(static_cast<fixed_t>(*x), finecosine[a >> ANGLETOFINESHIFT])
      - FixedMul(static_cast<fixed_t>(*y), finesine[a >> ANGLETOFINESHIFT]);

  *y =
      FixedMul(static_cast<fixed_t>(*x), finesine[a >> ANGLETOFINESHIFT])
      + FixedMul(static_cast<fixed_t>(*y), finecosine[a >> ANGLETOFINESHIFT]);

  *x = tmpx;
}

// [crispy] rotate point around map center
// adapted from prboom-plus/src/am_map.c:898-920
static void AM_rotatePoint(mpoint_t * pt) {
  pt->x -= mapcenter.x;
  pt->y -= mapcenter.y;

  int64_t tmpx = static_cast<int64_t>(FixedMul(static_cast<fixed_t>(pt->x), finecosine[mapangle >> ANGLETOFINESHIFT]))
                 - static_cast<int64_t>(FixedMul(static_cast<fixed_t>(pt->y), finesine[mapangle >> ANGLETOFINESHIFT]))
                 + mapcenter.x;

  pt->y = static_cast<int64_t>(FixedMul(static_cast<fixed_t>(pt->x), finesine[mapangle >> ANGLETOFINESHIFT]))
          + static_cast<int64_t>(FixedMul(static_cast<fixed_t>(pt->y), finecosine[mapangle >> ANGLETOFINESHIFT]))
          + mapcenter.y;

  pt->x = tmpx;
}

void AM_drawLineCharacter(mline_t * lineguy,
                          int       lineguylines,
                          fixed_t   scale,
                          angle_t   angle,
                          int       color,
                          fixed_t   x,
                          fixed_t   y) {
  mline_t l;

  if (crispy->automaprotate) {
    angle += mapangle;
  }

  for (int i = 0; i < lineguylines; i++) {
    l.a.x = lineguy[i].a.x;
    l.a.y = lineguy[i].a.y;

    if (scale) {
      l.a.x = FixedMul(scale, static_cast<fixed_t>(l.a.x));
      l.a.y = FixedMul(scale, static_cast<fixed_t>(l.a.y));
    }

    if (angle)
      AM_rotate(&l.a.x, &l.a.y, angle);

    l.a.x += x;
    l.a.y += y;

    l.b.x = lineguy[i].b.x;
    l.b.y = lineguy[i].b.y;

    if (scale) {
      l.b.x = FixedMul(scale, static_cast<fixed_t>(l.b.x));
      l.b.y = FixedMul(scale, static_cast<fixed_t>(l.b.y));
    }

    if (angle)
      AM_rotate(&l.b.x, &l.b.y, angle);

    l.b.x += x;
    l.b.y += y;

    AM_drawMline(&l, color);
  }
}

void AM_drawPlayers() {
  static int their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
  int        their_color    = -1;
  mpoint_t   pt;

  if (!g_doomstat_globals->netgame) {
    pt.x = plr->mo->x;
    pt.y = plr->mo->y;
    if (crispy->automaprotate) {
      AM_rotatePoint(&pt);
    }

    if (cheating)
      AM_drawLineCharacter(cheat_player_arrow, static_cast<int>(std::size(cheat_player_arrow)), 0, plr->mo->angle, WHITE, static_cast<fixed_t>(pt.x), static_cast<fixed_t>(pt.y));
    else
      AM_drawLineCharacter(player_arrow, static_cast<int>(std::size(player_arrow)), 0, plr->mo->angle, WHITE, static_cast<fixed_t>(pt.x), static_cast<fixed_t>(pt.y));
    return;
  }

  for (int i = 0; i < MAXPLAYERS; i++) {
    their_color++;
    player_t * p = &g_doomstat_globals->players[i];

    if ((g_doomstat_globals->deathmatch && !g_doomstat_globals->singledemo) && p != plr)
      continue;

    if (!g_doomstat_globals->playeringame[i])
      continue;

    int color = 0;
    if (p->powers[pw_invisibility])
      color = 246; // *close* to black
    else
      color = their_colors[their_color];

    pt.x = p->mo->x;
    pt.y = p->mo->y;
    if (crispy->automaprotate) {
      AM_rotatePoint(&pt);
    }

    AM_drawLineCharacter(player_arrow, static_cast<int>(std::size(player_arrow)), 0, p->mo->angle, color, static_cast<fixed_t>(pt.x), static_cast<fixed_t>(pt.y));
  }
}

void AM_drawThings(int colors, int) {
  for (int i = 0; i < g_r_state_globals->numsectors; i++) {
    mobj_t * t = g_r_state_globals->sectors[i].thinglist;
    while (t) {
      // [crispy] do not draw an extra triangle for the player
      if (t == plr->mo) {
        t = t->snext;
        continue;
      }

      mpoint_t pt;
      pt.x = t->x;
      pt.y = t->y;
      if (crispy->automaprotate) {
        AM_rotatePoint(&pt);
      }

      if (crispy->extautomap) {
        // [crispy] skull keys and key cards
        keycolor_t key = no_key;
        switch (t->info->doomednum) {
        case 38:
        case 13:
          key = red_key;
          break;
        case 39:
        case 6:
          key = yellow_key;
          break;
        case 40:
        case 5:
          key = blue_key;
          break;
        default:
          key = no_key;
          break;
        }

        // [crispy] draw keys as crosses in their respective colors
        if (key > no_key) {
          AM_drawLineCharacter(cross_mark, static_cast<int>(std::size(cross_mark)), 16 << FRACBITS, t->angle, (key == red_key) ? REDS : (key == yellow_key) ? YELLOWS :
                                                                                                                                    (key == blue_key)       ? BLUES :
                                                                                                                                                              colors + lightlev,
                               static_cast<fixed_t>(pt.x),
                               static_cast<fixed_t>(pt.y));
        } else
          // [crispy] draw blood splats and puffs as small squares
          if (t->type == MT_BLOOD || t->type == MT_PUFF) {
            AM_drawLineCharacter(square_mark, static_cast<int>(std::size(square_mark)), t->radius >> 2, t->angle, (t->type == MT_BLOOD) ? REDS : GRAYS, static_cast<fixed_t>(pt.x), static_cast<fixed_t>(pt.y));
          } else {
            AM_drawLineCharacter(thintriangle_guy, static_cast<int>(std::size(thintriangle_guy)),
                                 // [crispy] triangle size represents actual thing size
                                 t->radius,
                                 t->angle,
                                 // [crispy] show countable kills in red ...
                                 ((t->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL) ? REDS :
                                                                                             // [crispy] ... show Lost Souls and missiles in orange ...
                                     (t->flags & (MF_FLOAT | MF_MISSILE)) ? 216 :
                                                                            // [crispy] ... show other shootable items in dark gold ...
                                     (t->flags & MF_SHOOTABLE) ? 164 :
                                                                 // [crispy] ... corpses in gray ...
                                     (t->flags & MF_CORPSE) ? GRAYS :
                                                              // [crispy] ... and countable items in yellow
                                     (t->flags & MF_COUNTITEM) ? YELLOWS :
                                                                 colors + lightlev,
                                 static_cast<fixed_t>(pt.x),
                                 static_cast<fixed_t>(pt.y));
          }
      } else {
        AM_drawLineCharacter(thintriangle_guy, static_cast<int>(std::size(thintriangle_guy)), 16 << FRACBITS, t->angle, colors + lightlev, t->x, t->y);
      }
      t = t->snext;
    }
  }
}

void AM_drawMarks() {
  for (int i = 0; i < AM_NUMMARKPOINTS; i++) {
    if (markpoints[i].x != -1) {
      //      w = SHORT(marknums[i]->width);
      //      h = SHORT(marknums[i]->height);
      int w = 5; // because something's wrong with the wad, i guess
      int h = 6; // because something's wrong with the wad, i guess
      // [crispy] center marks around player
      mpoint_t pt;
      pt.x = markpoints[i].x;
      pt.y = markpoints[i].y;
      if (crispy->automaprotate) {
        AM_rotatePoint(&pt);
      }
      int fx = (g_r_state_globals->flipscreenwidth[CXMTOF(pt.x)] >> crispy->hires) - 1 - DELTAWIDTH;
      int fy = static_cast<int>((CYMTOF(pt.y) >> crispy->hires) - 2);
      if (fx >= f_x && fx <= (f_w >> crispy->hires) - w && fy >= f_y && fy <= (f_h >> crispy->hires) - h)
        V_DrawPatch(fx, fy, marknums[i]);
    }
  }
}

void AM_drawCrosshair(int color) {
  // [crispy] draw an actual crosshair
  if (!followplayer) {
    static fline_t h, v;

    if (!h.a.x) {
      h.a.x = h.b.x = v.a.x = v.b.x = f_x + f_w / 2;
      h.a.y = h.b.y = v.a.y = v.b.y = f_y + f_h / 2;
      h.a.x -= 2;
      h.b.x += 2;
      v.a.y -= 2;
      v.b.y += 2;
    }

    AM_drawFline(&h, color);
    AM_drawFline(&v, color);
  }
  // [crispy] do not draw the useless dot on the player arrow
  /*
  else
#ifndef CRISPY_TRUECOLOR
  fb[(f_w*(f_h+1))/2] = color; // single point for now
#else
  fb[(f_w*(f_h+1))/2] = colormaps[color]; // single point for now
#endif
*/
}

void AM_Drawer() {
  if (!g_doomstat_globals->automapactive) return;

  if (!crispy->automapoverlay)
    AM_clearFB(BACKGROUND);
  if (grid)
    AM_drawGrid(GRIDCOLORS);
  AM_drawWalls();
  AM_drawPlayers();
  if (cheating == 2)
    AM_drawThings(THINGCOLORS, THINGRANGE);
  AM_drawCrosshair(XHAIRCOLORS);

  AM_drawMarks();

  V_MarkRect(f_x, f_y, f_w, f_h);
}

// [crispy] extended savegames
void AM_GetMarkPoints(int * n, long * p) {
  *n = markpointnum;
  *p = -1L;

  // [crispy] prevent saving markpoints from previous map
  if (lastlevel == g_doomstat_globals->gamemap && lastepisode == g_doomstat_globals->gameepisode) {
    for (auto & markpoint : markpoints) {
      *p++ = static_cast<long>(markpoint.x);
      *p++ = (markpoint.x == -1) ? 0L : static_cast<long>(markpoint.y);
    }
  }
}

void AM_SetMarkPoints(int n, long * p) {
  AM_LevelInit();
  lastlevel   = g_doomstat_globals->gamemap;
  lastepisode = g_doomstat_globals->gameepisode;

  markpointnum = n;

  for (auto & markpoint : markpoints) {
    markpoint.x = static_cast<int64_t>(*p++);
    markpoint.y = static_cast<int64_t>(*p++);
  }
}
