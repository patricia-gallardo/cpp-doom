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
// DESCRIPTION:  none
//	Implements special effects:
//	Texture animation, height or lighting changes
//	 according to adjacent sectors, respective
//	 utility functions, etc.
//


#ifndef __P_SPEC__
#define __P_SPEC__


//
// End-level timer (-TIMER option)
//
extern	boolean levelTimer;
extern	int	levelTimeCount;


//      Define values for map objects
#define MO_TELEPORTMAN          14


// at game start
void    P_InitPicAnims ();

// villsa [STRIFE]
using terraintype_e = enum
{
    FLOOR_WATER = 0,
    FLOOR_SLIME = 1,
    FLOOR_SOLID = 2,
    FLOOR_END   = -1
};

void P_InitTerrainTypes();                  // villsa [STRIFE]
terraintype_e P_GetTerrainType(mobj_t* mobj);   // villsa [STRIFE]

// at map load
void    P_SpawnSpecials ();

// every tic
void    P_UpdateSpecials ();

// when needed
boolean
P_UseSpecialLine
( mobj_t*	thing,
  line_t*	line,
  int		side );

void
P_ShootSpecialLine
( mobj_t*	thing,
  line_t*	line );

void
P_CrossSpecialLine
( int		linenum,
  int		side,
  mobj_t*	thing );

void    P_PlayerInSpecialSector (player_t* player);

int
twoSided
( int		sector,
  int		line );

sector_t*
getSector
( int		currentSector,
  int		line,
  int		side );

side_t*
getSide
( int		currentSector,
  int		line,
  int		side );

fixed_t P_FindLowestFloorSurrounding(sector_t* sec);
fixed_t P_FindHighestFloorSurrounding(sector_t* sec);

fixed_t
P_FindNextHighestFloor
( sector_t*	sec,
  int		currentheight );

fixed_t P_FindLowestCeilingSurrounding(sector_t* sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec);

int
P_FindSectorFromLineTag
( line_t*	line,
  int		start );

int
P_FindMinSurroundingLight
( sector_t*	sector,
  int		max );

sector_t*
getNextSector
( line_t*	line,
  sector_t*	sec );


//
// SPECIAL
//
int     EV_DoDonut(line_t* line);
boolean EV_ClearForceFields(line_t* line);  // villsa [STRIFE]


//
// P_LIGHTS
//
using fireflicker_t = struct
{
    thinker_t	thinker;
    sector_t*	sector;
    int		count;
    int		maxlight;
    int		minlight;

};



using lightflash_t = struct lightflash_s
{
    thinker_t	thinker;
    sector_t*	sector;
    int		count;
    int		maxlight;
    int		minlight;
    int		maxtime;
    int		mintime;

};



using strobe_t = struct strobe_s
{
    thinker_t	thinker;
    sector_t*	sector;
    int		count;
    int		minlight;
    int		maxlight;
    int		darktime;
    int		brighttime;

};




using glow_t = struct glow_s
{
    thinker_t	thinker;
    sector_t*	sector;
    int		minlight;
    int		maxlight;
    int		direction;

};


#define GLOWSPEED			8
#define STROBEBRIGHT		5
#define FASTDARK			15
#define SLOWDARK			35

void    P_SpawnFireFlicker (sector_t* sector);
void    T_LightFlash (lightflash_t* flash);
void    P_SpawnLightFlash (sector_t* sector);
void    T_StrobeFlash (strobe_t* flash);

void
P_SpawnStrobeFlash
( sector_t*	sector,
  int		fastOrSlow,
  int		inSync );

void    EV_StartLightStrobing(line_t* line);
void    EV_TurnTagLightsOff(line_t* line);

void
EV_LightTurnOn
( line_t*	line,
  int		bright );

void    T_Glow(glow_t* g);
void    P_SpawnGlowingLight(sector_t* sector);




//
// P_SWITCH
//
using switchlist_t = struct
{
    char	name1[9];
    char	name2[9];
    short	episode;
    int         sound;  // villsa [STRIFE]

};


using bwhere_e = enum
{
    top,
    middle,
    bottom

};


using button_t = struct
{
    line_t*	line;
    bwhere_e	where;
    int		btexture;
    int		btimer;
    degenmobj_t *soundorg;

};




 // max # of wall switches in a level
#define MAXSWITCHES		80  // villsa [STRIFE] changed from 50 to 80

 // 4 players, 4 buttons each at once, max.
#define MAXBUTTONS		16

 // 1 second, in ticks. 
#define BUTTONTIME      35             

extern button_t	buttonlist[MAXBUTTONS]; 

void
P_ChangeSwitchTexture
( line_t*	line,
  int		useAgain );

void P_InitSwitchList();


//
// P_PLATS
//
using plat_e = enum
{
    up,
    down,
    waiting,
    in_stasis

};



using plattype_e = enum
{
    perpetualRaise,
    downWaitUpStay,
    slowDWUS,           // villsa [STRIFE]
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS,
    upWaitDownStay      // villsa [STRIFE]

};



using plat_t = struct plat_s
{
    thinker_t	thinker;
    sector_t*	sector;
    fixed_t	speed;
    fixed_t	low;
    fixed_t	high;
    int		wait;
    int		count;
    plat_e	status;
    plat_e	oldstatus;
    boolean	crush;
    int		tag;
    plattype_e	type;

};



#define PLATWAIT		3
#define PLATSPEED		FRACUNIT
#define MAXPLATS		30*256


extern plat_t*	activeplats[MAXPLATS];

void    T_PlatRaise(plat_t*	plat);

int
EV_DoPlat
( line_t*	line,
  plattype_e	type,
  int		amount );

void    P_AddActivePlat(plat_t* plat);
void    P_RemoveActivePlat(plat_t* plat);
void    EV_StopPlat(line_t* line);
void    P_ActivateInStasis(int tag);


//
// P_DOORS
//
using vldoor_e = enum
{
    vld_normal,
    vld_close30ThenOpen,
    vld_close,
    vld_open,
    vld_raiseIn5Mins,
    vld_blazeRaise,
    vld_blazeOpen,
    vld_blazeClose,
    vld_shopClose,          // villsa [STRIFE]
    vld_splitRaiseNearest,  // villsa [STRIFE]
    vld_splitOpen           // villsa [STRIFE]

};



using vldoor_t = struct vldoor_s
{
    thinker_t   thinker;
    vldoor_e    type;
    sector_t*   sector;
    fixed_t     topheight;
    fixed_t     speed;

    // 1 = up, 0 = waiting at top, -1 = down
    int         direction;

    // tics to wait at the top
    int         topwait;
    // (keep in case a door going down is reset)
    // when it reaches 0, start going down
    int         topcountdown;

    // villsa [STRIFE] new field - sound to play when opening
    int         opensound;

    // villsa [STRIFE] new field - sound to play when closing
    int         closesound;

};



#define VDOORSPEED		FRACUNIT*2
#define VDOORWAIT		150

void
EV_VerticalDoor
( line_t*	line,
  mobj_t*	thing );

int
EV_DoDoor
( line_t*	line,
  vldoor_e	type );

int
EV_DoLockedDoor
( line_t*	line,
  vldoor_e	type,
  mobj_t*	thing );

void    T_VerticalDoor (vldoor_t* door);
void    P_SpawnDoorCloseIn30 (sector_t* sec);

void
P_SpawnDoorRaiseIn5Mins
( sector_t*	sec,
  int		secnum );



// villsa [STRIFE] resurrected sliding doors
//
//      Sliding doors...
//
using sd_e = enum
{
    sd_opening,
    sd_waiting,
    sd_closing

};



using sdt_e = enum
{
    sdt_openOnly,
    sdt_closeOnly,
    sdt_openAndClose

};



// villsa [STRIFE] Rogue added a second line_t in the struct
// backsector is removed
using slidedoor_t = struct
{
    thinker_t   thinker;
    sdt_e       type;
    line_t*     line1;
    line_t*     line2;
    int         frame;
    int         whichDoorIndex;
    int         timer;
    sector_t*   frontsector;
    sd_e        status;

};

// villsa [STRIFE] no front/back frames
using slidename_t = struct
{
    char    frame1[9];
    char    frame2[9];
    char    frame3[9];
    char    frame4[9];
    char    frame5[9];
    char    frame6[9];
    char    frame7[9];
    char    frame8[9];

};

// villsa [STRIFE] no front/back frames
using slideframe_t = struct
{
    int frames[8];

};

// haleyjd 09/29/10: [STRIFE] Externalized for savegames
void T_SlidingDoor(slidedoor_t* door);


// how many frames of animation
#define SNUMFRAMES      8       // villsa [STRIFE] changed from 4 to 8

#define SDOORWAIT       TICRATE*3
#define SWAITTICS       4

// how many diff. types of anims
#define MAXSLIDEDOORS	8       // villsa [STRIFE] changed from 5 to 8                     

void P_InitSlidingDoorFrames();
void EV_SlidingDoor(line_t* line, mobj_t* thing);
int EV_RemoteSlidingDoor(line_t* line, mobj_t* thing);



//
// P_CEILNG
//
using ceiling_e = enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise

};



using ceiling_t = struct ceiling_s
{
    thinker_t	thinker;
    ceiling_e	type;
    sector_t*	sector;
    fixed_t	bottomheight;
    fixed_t	topheight;
    fixed_t	speed;
    boolean	crush;

    // 1 = up, 0 = waiting, -1 = down
    int		direction;

    // ID
    int		tag;
    int		olddirection;

};





#define CEILSPEED		FRACUNIT
#define CEILWAIT		150
#define MAXCEILINGS		30

extern ceiling_t*	activeceilings[MAXCEILINGS];

int
EV_DoCeiling
( line_t*	line,
  ceiling_e	type );

void    T_MoveCeiling (ceiling_t* ceiling);
void    P_AddActiveCeiling(ceiling_t* c);
void    P_RemoveActiveCeiling(ceiling_t* c);
int	EV_CeilingCrushStop(line_t* line);
void    P_ActivateInStasisCeiling(line_t* line);


//
// P_FLOOR
//
using floor_e = enum
{
    // lower floor to highest surrounding floor
    lowerFloor,

    // lower floor to lowest surrounding floor
    lowerFloorToLowest,

    // lower floor to highest surrounding floor VERY FAST
    turboLower,

    // raise floor to lowest surrounding CEILING
    raiseFloor,

    // raise floor to next highest surrounding floor
    raiseFloorToNearest,

    // raise floor to shortest height texture around it
    raiseToTexture,

    // lower floor to lowest surrounding floor
    //  and change floorpic
    lowerAndChange,

    raiseFloor64,          // [STRIFE] changed from 24 to 64
    raiseFloor24AndChange,
    raiseFloorCrush,

     // raise to next highest floor, turbo-speed
    raiseFloorTurbo,
    donutRaise,
    raiseFloor512,

    // [STRIFE] New floor type - used for the coolant reactor pit
    raiseFloor512AndChange

};




using stair_e = enum
{
    build8,     // slowly build by 8
    turbo16,    // quickly build by 16
    buildDown16 // haleyjd 09/24/10: [STRIFE] new stair type
};



using floormove_t = struct floormove_s
{
    thinker_t	thinker;
    floor_e	type;
    boolean	crush;
    sector_t*	sector;
    int		direction;
    int		newspecial;
    short	texture;
    fixed_t	floordestheight;
    fixed_t	speed;

};



#define FLOORSPEED		FRACUNIT

using result_e = enum
{
    ok,
    crushed,
    pastdest

};

result_e
T_MovePlane
( sector_t*	sector,
  fixed_t	speed,
  fixed_t	dest,
  boolean	crush,
  int		floorOrCeiling,
  int		direction );

int
EV_BuildStairs
( line_t*	line,
  stair_e	type );

int
EV_DoFloor
( line_t*	line,
  floor_e	floortype );

void T_MoveFloor( floormove_t* floor);

//
// P_TELEPT
//

// [STRIFE] Teleportation flags - teleflags
// Not to be conflated with telefrags, though they be tangentially related ;)
using teleflags_e = enum teleflags
{
    TF_NOSRCSND = 0x01,
    TF_NODSTSND = 0x02,
    TF_NODSTFOG = 0x10,
    TF_NOSRCFOG = 0x20,

    TF_NORMAL      = 0,
    TF_DSTSILENCE  = (TF_NODSTSND|TF_NODSTFOG),    // 0x12 (18) (Not used)
    TF_SRCSILENCE  = (TF_NOSRCSND|TF_NOSRCFOG),    // 0x21 (33)
    TF_FULLSILENCE = (TF_SRCSILENCE|TF_DSTSILENCE) // 0x33 (51)

};

int
EV_Teleport
( line_t*       line,
  int           side,
  mobj_t*       thing,
  teleflags_e   flags);

#endif
