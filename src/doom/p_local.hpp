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
// DESCRIPTION:
//	Play functions, animation, global header.
//

#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.hpp"
#endif

constexpr auto TOCENTER   = -8;
constexpr auto AFLAG_JUMP = 0x80;
constexpr auto FLOATSPEED = (FRACUNIT * 4);

constexpr auto MAXHEALTH = 100;
constexpr auto VIEWHEIGHT = (41 * FRACUNIT);

// mapblocks are used to check movement
// against lines and things
constexpr auto MAPBLOCKUNITS = 128;
constexpr auto MAPBLOCKSIZE  = (MAPBLOCKUNITS * FRACUNIT);
constexpr auto MAPBLOCKSHIFT = (FRACBITS + 7);
[[maybe_unused]] constexpr auto MAPBMASK = (MAPBLOCKSIZE - 1);
constexpr auto MAPBTOFRAC = (MAPBLOCKSHIFT - FRACBITS);

// player radius for movement checking
constexpr auto PLAYERRADIUS = 16 * FRACUNIT;

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
constexpr auto MAXRADIUS = 32 * FRACUNIT;

constexpr auto GRAVITY = FRACUNIT;
constexpr auto MAXMOVE = (30 * FRACUNIT);

constexpr auto USERANGE     = (64 * FRACUNIT);
constexpr auto MELEERANGE   = (64 * FRACUNIT);
constexpr auto MISSILERANGE = (32 * 64 * FRACUNIT);

// follow a player exlusively for 3 seconds
constexpr auto BASETHRESHOLD = 100;

void P_InitThinkers();
void P_AddThinker(thinker_t * thinker);
void P_RemoveThinker(thinker_t * thinker);

//
// P_PSPR
//
void P_SetupPsprites(player_t * curplayer);
void P_MovePsprites(player_t * curplayer);
void P_DropWeapon(player_t * player);

//
// P_USER
//
constexpr auto MLOOKUNIT = 8;
#define PLAYER_SLOPE(a) ((((a)->lookdir / MLOOKUNIT) << FRACBITS) / 173)
void P_PlayerThink(player_t * player);

//
// P_MOBJ
//
#define ONFLOORZ   INT_MIN
#define ONCEILINGZ INT_MAX

// Time interval for item respawning.
constexpr auto ITEMQUESIZE = 128;

void P_RespawnSpecials();

mobj_t *
    P_SpawnMobj(fixed_t    x,
                fixed_t    y,
                fixed_t    z,
                mobjtype_t type);

void     P_RemoveMobj(mobj_t * th);
mobj_t * P_SubstNullMobj(mobj_t * th);
bool     P_SetMobjState(mobj_t * mobj, statenum_t state);
void     P_MobjThinker(mobj_t * mobj);
mobj_t * Crispy_PlayerSO(int p); // [crispy] weapon sound sources

void     P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void     P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t * target);
mobj_t * P_SpawnMissile(mobj_t * source, mobj_t * dest, mobjtype_t type);
void     P_SpawnPlayerMissile(mobj_t * source, mobjtype_t type);

void P_SpawnPuffSafe(fixed_t x, fixed_t y, fixed_t z, bool safe);

//
// P_ENEMY
//
void P_NoiseAlert(mobj_t * target, mobj_t * emmiter);

//
// P_MAPUTL
//
struct divline_t {
  fixed_t x;
  fixed_t y;
  fixed_t dx;
  fixed_t dy;
};

struct intercept_t {
  fixed_t frac; // along trace line
  bool    isaline;
  union {
    mobj_t * thing;
    line_t * line;
  } d;
};

// Extended MAXINTERCEPTS, to allow for intercepts overrun emulation.

constexpr auto                  MAXINTERCEPTS_ORIGINAL = 128;
[[maybe_unused]] constexpr auto MAXINTERCEPTS          = (MAXINTERCEPTS_ORIGINAL + 61);

using traverser_t = bool (*)(intercept_t *);

fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);
int     P_PointOnLineSide(fixed_t x, fixed_t y, line_t * line);
int     P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t * line);
void    P_MakeDivline(line_t * li, divline_t * dl);
fixed_t P_InterceptVector(divline_t * v2, divline_t * v1);
int     P_BoxOnLineSide(fixed_t * tmbox, line_t * ld);

void P_LineOpening(line_t * linedef_param);

bool P_BlockLinesIterator(int x, int y, bool (*func)(line_t *));
bool P_BlockThingsIterator(int x, int y, bool (*func)(mobj_t *));

constexpr auto PT_ADDLINES  = 1;
constexpr auto PT_ADDTHINGS = 2;
constexpr auto PT_EARLYOUT  = 4;

bool P_PathTraverse(fixed_t x1,
                    fixed_t y1,
                    fixed_t x2,
                    fixed_t y2,
                    int     flags,
                    bool (*trav)(intercept_t *));

void P_UnsetThingPosition(mobj_t * thing);
void P_SetThingPosition(mobj_t * thing);

// fraggle: I have increased the size of this buffer.  In the original Doom,
// overrunning past this limit caused other bits of memory to be overwritten,
// affecting demo playback.  However, in doing so, the limit was still
// exceeded.  So we have to support more than 8 specials.
//
// We keep the original limit, to detect what variables in memory were
// overwritten (see SpechitOverrun())

constexpr auto MAXSPECIALCROSS          = 20;
constexpr auto MAXSPECIALCROSS_ORIGINAL = 8;

bool P_CheckPosition(mobj_t * thing, fixed_t x, fixed_t y);
bool P_TryMove(mobj_t * thing, fixed_t x, fixed_t y);
bool P_TeleportMove(mobj_t * thing, fixed_t x, fixed_t y);
void P_SlideMove(mobj_t * mo);
bool P_CheckSight(mobj_t * t1, mobj_t * t2);
void P_UseLines(player_t * player);

bool P_ChangeSector(sector_t * sector, bool crunch);

fixed_t
    P_AimLineAttack(mobj_t * t1,
                    angle_t  angle,
                    fixed_t  distance);

void P_LineAttack(mobj_t * t1,
                  angle_t  angle,
                  fixed_t  distance,
                  fixed_t  slope,
                  int      damage);

void P_RadiusAttack(mobj_t * spot,
                    mobj_t * source,
                    int      damage);

// [crispy] factor out map lump name and number finding into a separate function
extern int P_GetNumForMap(int episode, int map, bool critical_param);

// [crispy] blinking key or skull in the status bar
constexpr auto KEYBLINKMASK = 0x8;
constexpr auto KEYBLINKTICS = (7 * KEYBLINKMASK);

void P_TouchSpecialThing(mobj_t * special,
                         mobj_t * toucher);

void P_DamageMobj(mobj_t * target,
                  mobj_t * inflictor,
                  mobj_t * source,
                  int      damage);

struct p_local_t {
  //
  // P_TICK
  //

  // both the head and tail of the thinker list
  thinker_t thinkercap;

  mapthing_t itemrespawnque[ITEMQUESIZE] {};
  int        itemrespawntime[ITEMQUESIZE] {};
  int        iquehead {};
  int        iquetail {};

  intercept_t * intercept_p {}; // [crispy] remove INTERCEPTS limit

  //
  // P_LineOpening
  // Sets opentop and openbottom to the window
  // through a two sided line.
  // OPTIMIZE: keep this precalculated
  //
  fixed_t   opentop {};
  fixed_t   openbottom {};
  fixed_t   openrange {};
  fixed_t   lowfloor {};
  divline_t trace {};

  //
  // P_MAP
  //

  // If "floatok" true, move would be ok
  // if within "tmfloorz - tmceilingz".
  bool    floatok {};
  fixed_t tmfloorz {};
  fixed_t tmceilingz {};

  // keep track of the line that lowers the ceiling,
  // so missiles don't explode against sky hack walls
  line_t * ceilingline {};

  // keep track of special lines as they are hit,
  // but don't process them until the move is proven valid
  line_t * spechit[MAXSPECIALCROSS] {};
  int      numspechit {};

  //
  // P_LineAttack
  //
  mobj_t * linetarget {}; // who got hit (or nullptr)

  //
  // P_SETUP
  //

  // REJECT
  // For fast sight rejection.
  // Speeds up enemy AI by skipping detailed
  //  LineOf Sight calculation.
  // Without special effect, this could be
  //  used as a PVS lookup as well.
  //
  uint8_t * rejectmatrix {}; // for fast sight rejection

  // BLOCKMAP
  // Created from axis aligned bounding box
  // of the map, a rectangular array of
  // blocks of size ...
  // Used to speed up collision detection
  // by spatial subdivision in 2D.
  //
  // Blockmap size.
  int32_t * blockmaplump {}; // offsets in blockmap are from here // [crispy] BLOCKMAP limit
  int32_t * blockmap {};     // int for larger maps // [crispy] BLOCKMAP limit
  int       bmapwidth {};
  int       bmapheight {}; // size in mapblocks

  // origin of block map
  fixed_t   bmaporgx {};
  fixed_t   bmaporgy {};   // origin of block map
  mobj_t ** blocklinks {}; // for thing chains
  // [crispy] blinking key or skull in the status bar
  int st_keyorskull[3] {};
  //
  // P_INTER
  //

  // a weapon is found with two clip loads,
  // a big item has five clip loads
  int maxammo[NUMAMMO] {};
  int clipammo[NUMAMMO] {};
};

extern p_local_t * const g_p_local_globals;

//
// P_SPEC
//
#include "p_spec.hpp"

#endif // __P_LOCAL__
