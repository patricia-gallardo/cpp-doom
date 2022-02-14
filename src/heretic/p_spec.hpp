//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

// P_spec.h

/*
===============================================================================

							P_SPEC

===============================================================================
*/

//
//      Animating textures and planes
//
typedef struct
{
    bool istexture;
    int picnum;
    int basepic;
    int numpics;
    int speed;
} anim_t;

//
//      source animation definition
//
struct animdef_t
{
    int istexture{};          // if false, it's a flat
    char endname[9]{};
    char startname[9]{};
    int speed{};
};

#define	MAXANIMS		32

extern anim_t anims[MAXANIMS], *lastanim;
extern int *TerrainTypes;

//
//      Animating line specials
//
#define	MAXLINEANIMS		64*256
extern short numlinespecials;
extern line_t *linespeciallist[MAXLINEANIMS];

//      Define values for map objects
#define	MO_TELEPORTMAN		14

// at game start
void P_InitPicAnims();
void P_InitTerrainTypes();
void P_InitLava();

// at map load
void P_SpawnSpecials();
void P_InitAmbientSound();
void P_AddAmbientSfx(int sequence);

// every tic
void P_UpdateSpecials();
void P_AmbientSound();

// when needed
bool P_UseSpecialLine(mobj_t * thing, line_t * line);
void P_ShootSpecialLine(mobj_t * thing, line_t * line);
void P_CrossSpecialLine(int linenum, int side, mobj_t * thing);

void P_PlayerInSpecialSector(player_t * player);

int twoSided(int sector, int line);
sector_t *getSector(int currentSector, int line, int side);
side_t *getSide(int currentSector, int line, int side);
fixed_t P_FindLowestFloorSurrounding(sector_t * sec);
fixed_t P_FindHighestFloorSurrounding(sector_t * sec);
fixed_t P_FindNextHighestFloor(sector_t * sec, int currentheight);
fixed_t P_FindLowestCeilingSurrounding(sector_t * sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t * sec);
int P_FindSectorFromLineTag(line_t * line, int start);
int P_FindMinSurroundingLight(sector_t * sector, int max);
sector_t *getNextSector(line_t * line, sector_t * sec);

//
//      SPECIAL
//
int EV_DoDonut(line_t * line);

/*
===============================================================================

							P_LIGHTS

===============================================================================
*/
typedef struct lightflash_s
{
    thinker_t thinker;
    sector_t *sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;
} lightflash_t;

typedef struct strobe_s
{
    thinker_t thinker;
    sector_t *sector;
    int count;
    int minlight;
    int maxlight;
    int darktime;
    int brighttime;
} strobe_t;

typedef struct glow_s
{
    thinker_t thinker;
    sector_t *sector;
    int minlight;
    int maxlight;
    int direction;
} glow_t;

#define GLOWSPEED		8
#define	STROBEBRIGHT	5
#define	FASTDARK		15
#define	SLOWDARK		35

void T_LightFlash(lightflash_t * flash);
void P_SpawnLightFlash(sector_t * sector);
void T_StrobeFlash(strobe_t * flash);
void P_SpawnStrobeFlash(sector_t * sector, int fastOrSlow, int inSync);
void EV_StartLightStrobing(line_t * line);
void EV_TurnTagLightsOff(line_t * line);
void EV_LightTurnOn(line_t * line, int bright);
void T_Glow(glow_t * g);
void P_SpawnGlowingLight(sector_t * sector);

/*
===============================================================================

							P_SWITCH

===============================================================================
*/
typedef struct
{
    char name1[9];
    char name2[9];
    short episode;
} switchlist_t;

enum bwhere_e
{
    top,
    middle,
    bottom
};

typedef struct
{
    line_t *line;
    bwhere_e where;
    int btexture;
    int btimer;
    void *soundorg;
} button_t;

#define	MAXSWITCHES	50      // max # of wall switches in a level
#define	MAXBUTTONS	16      // 4 players, 4 buttons each at once, max.
#define BUTTONTIME	35      // 1 second

extern button_t buttonlist[MAXBUTTONS];

void P_ChangeSwitchTexture(line_t * line, int useAgain);
void P_InitSwitchList();

/*
===============================================================================

							P_PLATS

===============================================================================
*/
enum plat_e
{
    up,
    down,
    waiting,
    in_stasis
};

enum plattype_e
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange
};

typedef struct plat_s
{
    thinker_t thinker;
    sector_t *sector;
    fixed_t speed;
    fixed_t low;
    fixed_t high;
    int wait;
    int count;
    plat_e status;
    plat_e oldstatus;
    bool crush;
    int tag;
    plattype_e type;
} plat_t;

#define	PLATWAIT	3
#define	PLATSPEED	FRACUNIT
#define	MAXPLATS	30*256

extern plat_t *activeplats[MAXPLATS];

void T_PlatRaise(plat_t * plat);
int EV_DoPlat(line_t * line, plattype_e type, int amount);
void P_AddActivePlat(plat_t * plat);
void P_RemoveActivePlat(plat_t * plat);
void EV_StopPlat(line_t * line);
void P_ActivateInStasis(int tag);

/*
===============================================================================

							P_DOORS

===============================================================================
*/
enum vldoor_e
{
    vld_normal,
    vld_close30ThenOpen,
    vld_close,
    vld_open,
    vld_raiseIn5Mins
};

typedef struct vldoor_s
{
    thinker_t thinker;
    vldoor_e type;
    sector_t *sector;
    fixed_t topheight;
    fixed_t speed;
    int direction;              // 1 = up, 0 = waiting at top, -1 = down
    int topwait;                // tics to wait at the top
    // (keep in case a door going down is reset)
    int topcountdown;           // when it reaches 0, start going down
} vldoor_t;

#define	VDOORSPEED	FRACUNIT*2
#define	VDOORWAIT		150

void EV_VerticalDoor(line_t * line, mobj_t * thing);
int EV_DoDoor(line_t * line, vldoor_e type, fixed_t speed);
void T_VerticalDoor(vldoor_t * door);
void P_SpawnDoorCloseIn30(sector_t * sec);
void P_SpawnDoorRaiseIn5Mins(sector_t * sec, int secnum);

/*
===============================================================================

							P_CEILNG

===============================================================================
*/
enum ceiling_e
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise
};

typedef struct ceiling_s
{
    thinker_t thinker;
    ceiling_e type;
    sector_t *sector;
    fixed_t bottomheight, topheight;
    fixed_t speed;
    bool crush;
    int direction;              // 1 = up, 0 = waiting, -1 = down
    int tag;                    // ID
    int olddirection;
} ceiling_t;

#define	CEILSPEED		FRACUNIT
#define	CEILWAIT		150
#define MAXCEILINGS		30

extern ceiling_t *activeceilings[MAXCEILINGS];

int EV_DoCeiling(line_t * line, ceiling_e type);
void T_MoveCeiling(ceiling_t * ceiling);
void P_AddActiveCeiling(ceiling_t * c);
void P_RemoveActiveCeiling(ceiling_t * c);
int EV_CeilingCrushStop(line_t * line);
void P_ActivateInStasisCeiling(line_t * line);

/*
===============================================================================

							P_FLOOR

===============================================================================
*/
enum floor_e
{
    lowerFloor,                 // lower floor to highest surrounding floor
    lowerFloorToLowest,         // lower floor to lowest surrounding floor
    turboLower,                 // lower floor to highest surrounding floor VERY FAST
    raiseFloor,                 // raise floor to lowest surrounding CEILING
    raiseFloorToNearest,        // raise floor to next highest surrounding floor
    raiseToTexture,             // raise floor to shortest height texture around it
    lowerAndChange,             // lower floor to lowest surrounding floor and change
    // floorpic
    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,
    donutRaise,
    raiseBuildStep              // One step of a staircase
};

typedef struct floormove_s
{
    thinker_t thinker;
    floor_e type;
    bool crush;
    sector_t *sector;
    int direction;
    int newspecial;
    short texture;
    fixed_t floordestheight;
    fixed_t speed;
} floormove_t;

#define	FLOORSPEED	FRACUNIT

enum result_e
{
    ok,
    crushed,
    pastdest
};

result_e T_MovePlane(sector_t * sector, fixed_t speed,
                     fixed_t dest, bool crush, int floorOrCeiling,
                     int direction);

int EV_BuildStairs(line_t * line, fixed_t stepDelta);
int EV_DoFloor(line_t * line, floor_e floortype);
void T_MoveFloor(floormove_t * floor);

/*
===============================================================================

							P_TELEPT

===============================================================================
*/

bool P_Teleport(mobj_t * thing, fixed_t x, fixed_t y, angle_t angle);
bool EV_Teleport(line_t * line, int side, mobj_t * thing);
