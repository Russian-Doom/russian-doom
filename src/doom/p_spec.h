//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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


#pragma once


//
// End-level timer (-TIMER option)
//
extern	boolean levelTimer;
extern	int	levelTimeCount;

// -----------------------------------------------------------------------------
// P_CEILNG
// -----------------------------------------------------------------------------

#define CEILSPEED       FRACUNIT
#define CEILWAIT        150
#define MAXCEILINGS     30

typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise
} ceiling_e;

typedef struct
{
    thinker_t  thinker;
    ceiling_e  type;
    sector_t  *sector;
    fixed_t    bottomheight;
    fixed_t    topheight;
    fixed_t    speed;
    boolean    crush;
    int        direction;  // 1 = up, 0 = waiting, -1 = down
    int        tag;        // ID
    int        olddirection;
} ceiling_t;

extern ceiling_t *activeceilings[MAXCEILINGS];

void T_MoveCeiling (ceiling_t* ceiling);
const int EV_DoCeiling (line_t *line, ceiling_e type);
void P_AddActiveCeiling (ceiling_t* c);
void P_RemoveActiveCeiling(ceiling_t *c);
void P_ActivateInStasisCeiling(line_t *line);
const int EV_CeilingCrushStop(line_t* line);

// -----------------------------------------------------------------------------
// P_DOORS
// -----------------------------------------------------------------------------

#define VDOORSPEED      FRACUNIT*2
#define VDOORWAIT       150

typedef enum
{
    vld_normal,
    vld_close30ThenOpen,
    vld_close,
    vld_open,
    vld_raiseIn5Mins,
    vld_blazeRaise,
    vld_blazeOpen,
    vld_blazeClose
} vldoor_e;

typedef struct
{
    thinker_t  thinker;
    vldoor_e   type;
    sector_t  *sector;
    fixed_t    topheight;
    fixed_t    speed;
    int        direction;     // 1 = up, 0 = waiting at top, -1 = down
    int        topwait;       // tics to wait at the top
    int        topcountdown;  // when it reaches 0, start going down
} vldoor_t;

void T_VerticalDoor (vldoor_t *door);
void EV_VerticalDoor (line_t *line, mobj_t *thing);
const int EV_DoDoor (const line_t *line, const vldoor_e type);
const int EV_DoLockedDoor (line_t *line, const vldoor_e type, const mobj_t *thing);
void P_SpawnDoorCloseIn30 (sector_t * sec);
void P_SpawnDoorRaiseIn5Mins (sector_t *sec, const int secnum);

// -----------------------------------------------------------------------------
// P_FLOOR
// -----------------------------------------------------------------------------

#define FLOORSPEED FRACUNIT

typedef enum
{
    lowerFloor,           // lower floor to highest surrounding floor
    lowerFloorToLowest,   // lower floor to lowest surrounding floor
    turboLower,           // lower floor to highest surrounding floor VERY FAST
    raiseFloor,           // raise floor to lowest surrounding CEILING
    raiseFloorToNearest,  // raise floor to next highest surrounding floor
    raiseToTexture,       // raise floor to shortest height texture around it
    lowerAndChange,       // lower floor to lowest surrounding floor and change floorpic
    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,
    raiseFloorTurbo,      // raise to next highest floor, turbo-speed
    donutRaise,
    raiseFloor512
} floor_e;

typedef enum
{
    build8,  // slowly build by 8
    turbo16	 // quickly build by 16
} stair_e;

typedef struct
{
    thinker_t  thinker;
    floor_e	   type;
    boolean	   crush;
    sector_t  *sector;
    int		   direction;
    int		   newspecial;
    short	   texture;
    fixed_t	   floordestheight;
    fixed_t	   speed;
} floormove_t;

typedef enum
{
    ok,
    crushed,
    pastdest
} result_e;

const result_e T_MovePlane (sector_t *sector, const fixed_t speed, fixed_t dest,
                            const boolean crush, const int floorOrCeiling, const int direction);
void T_MoveFloor(floormove_t *floor);
const int EV_DoFloor (line_t *line, const floor_e floortype);
const int EV_BuildStairs (line_t *line, const stair_e type);

// -----------------------------------------------------------------------------
// P_LIGHTS
// -----------------------------------------------------------------------------

#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35

typedef struct
{
    thinker_t  thinker;
    sector_t  *sector;
    int        count;
    int        maxlight;
    int        minlight;
} fireflicker_t;

typedef struct
{
    thinker_t  thinker;
    sector_t  *sector;
    int        count;
    int        maxlight;
    int        minlight;
    int        maxtime;
    int        mintime;
} lightflash_t;

typedef struct
{
    thinker_t  thinker;
    sector_t  *sector;
    int        count;
    int        minlight;
    int        maxlight;
    int        darktime;
    int        brighttime;
} strobe_t;

typedef struct
{
    thinker_t  thinker;
    sector_t  *sector;
    int        minlight;
    int        maxlight;
    int        direction;
} glow_t;

void T_FireFlicker (fireflicker_t *flick);
void P_SpawnFireFlicker (sector_t *sector);
void T_LightFlash (lightflash_t *flash);
void P_SpawnLightFlash (sector_t *sector);
void T_StrobeFlash (strobe_t *flash);
void P_SpawnStrobeFlash (sector_t *sector, int fastOrSlow, int inSync);
void EV_StartLightStrobing (line_t *line);
void EV_TurnTagLightsOff (line_t *line);
void EV_LightTurnOn (line_t *line, int bright);
void T_Glow (glow_t *g);
void P_SpawnGlowingLight (sector_t *sector);

// -----------------------------------------------------------------------------
// P_SPEC
// -----------------------------------------------------------------------------

void P_InitPicAnims (void);
side_t *getSide (const int currentSector, const int line, const int side);
sector_t *getSector (const int currentSector, const int line, const int side);
const int twoSided (const int sector, const int line);
sector_t *getNextSector (const line_t *line, const sector_t *sec);
const fixed_t P_FindLowestFloorSurrounding (const sector_t *sec);
const fixed_t P_FindHighestFloorSurrounding (const sector_t *sec);
const fixed_t P_FindNextHighestFloor (const sector_t *sec, const int currentheight);
const fixed_t P_FindLowestCeilingSurrounding (const sector_t *sec);
const fixed_t P_FindHighestCeilingSurrounding (const sector_t *sec);
const int P_FindSectorFromLineTag (const line_t *line, const int start);
const int P_FindMinSurroundingLight (const sector_t *sector, const int max);
void P_CrossSpecialLine (const int linenum, const int side, mobj_t *thing);
// [crispy] more MBF code pointers
void P_CrossSpecialLinePtr (line_t *line, const int side, mobj_t *thing);
void P_ShootSpecialLine (const mobj_t *thing, line_t *line);
void P_PlayerInSpecialSector (player_t *player);
void P_UpdateSpecials (void);
void R_InterpolateTextureOffsets (void);
int EV_DoDonut (line_t *line);
void P_SpawnSpecials (void);

// -----------------------------------------------------------------------------
// P_SWITCH
// -----------------------------------------------------------------------------

#define MAXSWITCHES     50    // max # of wall switches in a level
#define MAXBUTTONS      16*2  // 4 players, 4 buttons each at once, max.
#define BUTTONTIME      35    // 1 second, in ticks. 

typedef struct
{
    char  name1[9];
    char  name2[9];
    short episode;
} switchlist_t;

typedef enum
{
    top,
    middle,
    bottom
} bwhere_e;

typedef struct
{
    line_t      *line;
    bwhere_e     where;
    int          btexture;
    int          btimer;
    degenmobj_t *soundorg;
} button_t;

extern button_t	buttonlist[MAXBUTTONS]; 

void P_InitSwitchList (void);
void P_ChangeSwitchTexture (line_t *line, int useAgain);
const boolean P_UseSpecialLine (mobj_t *thing, line_t *line, const int side);

// -----------------------------------------------------------------------------
// P_PLATS
// -----------------------------------------------------------------------------

#define PLATWAIT    3
#define PLATSPEED   FRACUNIT
#define MAXPLATS    7680  // [JN] 30 * 256 = 7680

typedef enum
{
    up,
    down,
    waiting,
    in_stasis
} plat_e;

typedef enum
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS
} plattype_e;

typedef struct
{
    thinker_t   thinker;
    sector_t   *sector;
    fixed_t     speed;
    fixed_t     low;
    fixed_t     high;
    int         wait;
    int         count;
    plat_e      status;
    plat_e      oldstatus;
    boolean     crush;
    int         tag;
    plattype_e  type;
} plat_t;

extern plat_t *activeplats[MAXPLATS];

void T_PlatRaise(plat_t *plat);
const int EV_DoPlat (line_t *line, plattype_e type, int amount);
void P_ActivateInStasis(const int tag);
void EV_StopPlat (const line_t *line);
void P_AddActivePlat (plat_t *plat);
void P_RemoveActivePlat (const plat_t *plat);

// -----------------------------------------------------------------------------
// P_TELEPT
// -----------------------------------------------------------------------------

#define MO_TELEPORTMAN  14  // Define values for map objects

const int EV_Teleport (const line_t *line, const int side, mobj_t *thing);
