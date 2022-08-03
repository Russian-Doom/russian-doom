//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2013-2018 Brad Harding
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
// DESCRIPTION:
//	Play functions, animation, global header.
//


#pragma once

#include <SDL.h>
#include "r_local.h"


#define TOCENTER        -8
#define FLOATSPEED      (FRACUNIT*4)
#define MAXHEALTH       100
#define VIEWHEIGHT      (41*FRACUNIT)

// Mapblocks are used to check movement against lines and things.
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK        (MAPBLOCKSIZE-1)
#define MAPBTOFRAC      (MAPBLOCKSHIFT-FRACBITS)

// Player radius for movement checking.
#define PLAYERRADIUS    16*FRACUNIT

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger, but we do not have any moving sectors nearby.
#define MAXRADIUS       32*FRACUNIT

#define GRAVITY         FRACUNIT
#define MAXMOVE         (30*FRACUNIT)
#define USERANGE        (64*FRACUNIT)
#define MELEERANGE      (64*FRACUNIT)
#define MISSILERANGE    (32*64*FRACUNIT)

// Follow a player exlusively for 3 seconds.
#define	BASETHRESHOLD   100


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

const int EV_CeilingCrushStop(line_t *line);
const int EV_DoCeiling (line_t *line, ceiling_e type);
void P_ActivateInStasisCeiling(line_t *line);
void P_AddActiveCeiling (ceiling_t *c);
void P_RemoveActiveCeiling(const ceiling_t *c);
void T_MoveCeiling (ceiling_t *ceiling);

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

const int EV_DoDoor (const line_t *line, const vldoor_e type);
const int EV_DoLockedDoor (line_t *line, const vldoor_e type, const mobj_t *thing);
void EV_VerticalDoor (line_t *line, mobj_t *thing);
void P_SpawnDoorCloseIn30 (sector_t * sec);
void P_SpawnDoorRaiseIn5Mins (sector_t *sec, const int secnum);
void T_VerticalDoor (vldoor_t *door);

// -----------------------------------------------------------------------------
// P_ENEMY
// -----------------------------------------------------------------------------

void P_NoiseAlert (mobj_t *target, mobj_t *emmiter);

// -----------------------------------------------------------------------------
// P_FIX
// -----------------------------------------------------------------------------

#define DEFAULT     0x7FFF
#define REMOVE      0

typedef struct
{
    int mission;
    int epsiode;
    int map;
    int vertex;
    int oldx, oldy;
    int newx, newy;
} vertexfix_t;

typedef struct
{
    int    mission;
    int    epsiode;
    int    map;
    int    linedef;
    int    side;
    char  *toptexture;
    char  *middletexture;
    char  *bottomtexture;
    short  offset;
    short  rowoffset;
    int    flags;
    int    special;
    int    tag;
} linefix_t;

typedef struct
{
    int   mission;
    int   epsiode;
    int   map;
    int   sector;
    char *floorpic;
    char *ceilingpic;
    int   floorheight;
    int   ceilingheight;
    int   special;
    int   oldtag;
    int   newtag;
} sectorfix_t;

typedef struct
{
    int mission;
    int epsiode;
    int map;
    int thing;
    int type;
    int oldx, oldy;
    int newx, newy;
    int angle;
    int options;
} thingfix_t;

// -----------------------------------------------------------------------------
// P_FLOOR
// -----------------------------------------------------------------------------

#define FLOORSPEED  FRACUNIT

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

const int EV_BuildStairs (line_t *line, const stair_e type);
const int EV_DoFloor (line_t *line, const floor_e floortype);
const result_e T_MovePlane (sector_t *sector, const fixed_t speed, fixed_t dest,
                            const boolean crush, const int floorOrCeiling, const int direction);
void T_MoveFloor (floormove_t *floor);

// -----------------------------------------------------------------------------
// P_INTER
// -----------------------------------------------------------------------------

#define BONUSADD    6

extern int maxammo[NUMAMMO];
extern int clipammo[NUMAMMO];
extern boolean ultimatemsg;

boolean P_GivePower (player_t *player, const int power);
void P_DamageMobj (mobj_t *target, const mobj_t *inflictor, mobj_t *source, int damage);
void P_SetMessage (player_t *player, char *message, MessageType_t messageType, boolean ultmsg);
void P_TouchSpecialThing (const mobj_t *special, const mobj_t *toucher);

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

void EV_LightTurnOn (line_t *line, int bright);
void EV_StartLightStrobing (line_t *line);
void EV_TurnTagLightsOff (line_t *line);
void P_SpawnFireFlicker (sector_t *sector);
void P_SpawnGlowingLight (sector_t *sector);
void P_SpawnLightFlash (sector_t *sector);
void P_SpawnStrobeFlash (sector_t *sector, int fastOrSlow, int inSync);
void T_FireFlicker (fireflicker_t *flick);
void T_Glow (glow_t *g);
void T_LightFlash (lightflash_t *flash);
void T_StrobeFlash (strobe_t *flash);

// -----------------------------------------------------------------------------
// P_MAP
// -----------------------------------------------------------------------------

extern boolean  floatok;
extern fixed_t  tmfloorz;
extern fixed_t  tmceilingz;
extern line_t  *ceilingline;
extern line_t **spechit;
extern int      numspechit;
extern mobj_t  *BlockingMobj;
extern mobj_t  *linetarget;
extern int64_t  attackrange;

boolean P_ChangeSector (sector_t *sector, boolean crunch);
boolean PIT_ChangeSector (mobj_t *thing);
boolean PIT_RadiusAttack (mobj_t *thing);
boolean PTR_NoWayAudible (line_t *line);
const boolean P_CheckLineSide (mobj_t *actor, const fixed_t x, const fixed_t y);
const boolean P_CheckPosition (mobj_t *thing, const fixed_t x, const fixed_t y);
const boolean P_TeleportMove (mobj_t* thing, const fixed_t x, const fixed_t y);
const boolean P_TryMove (mobj_t* thing, const fixed_t x, const fixed_t y);
const fixed_t P_AimLineAttack (mobj_t *t1, angle_t angle, const fixed_t distance);
void P_ApplyTorque(mobj_t *mo);
void P_LineAttack (mobj_t *t1, angle_t angle, int64_t distance, const fixed_t slope, const int damage);
void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage);
void P_SlideMove (mobj_t *mo);
void P_UseLines (const player_t *player);

// -----------------------------------------------------------------------------
// P_MAPUTL
// -----------------------------------------------------------------------------

// Extended MAXINTERCEPTS, to allow for intercepts overrun emulation.
#define MAXINTERCEPTS_ORIGINAL  128*16 // [JN] Лимит шестнадцатикратно умножен
#define MAXINTERCEPTS           (MAXINTERCEPTS_ORIGINAL + 61)
#define PT_ADDLINES             1
#define PT_ADDTHINGS            2

typedef struct
{
    fixed_t	x;
    fixed_t	y;
    fixed_t	dx;
    fixed_t	dy;
} divline_t;

typedef struct
{
    fixed_t	frac;		// along trace line
    boolean	isaline;
    union {
	mobj_t *thing;
	line_t *line;
    }
    d;
} intercept_t;

typedef boolean (*traverser_t) (intercept_t *in);

extern intercept_t  intercepts[MAXINTERCEPTS];
extern intercept_t *intercept_p;
extern divline_t    trace;
extern fixed_t      opentop;
extern fixed_t      openbottom;
extern fixed_t      openrange;
extern fixed_t      lowfloor;

boolean P_PathTraverse (fixed_t x1, fixed_t y1, int64_t x2, int64_t y2, int flags, boolean (*trav)(intercept_t *));
const boolean P_BlockLinesIterator (const int x, const int y, boolean(*func)(line_t*));
const boolean P_BlockThingsIterator (const int x, const int y, boolean(*func)(mobj_t*));
const fixed_t P_ApproxDistanceZ (fixed_t dx, fixed_t dy, fixed_t dz);
const fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
const fixed_t P_InterceptVector (const divline_t* v2, const divline_t* v1);
const int P_BoxOnLineSide (const fixed_t *tmbox, const line_t *ld);
const int P_PointOnDivlineSide (fixed_t x, fixed_t y, const divline_t *line);
const int P_PointOnLineSide (const fixed_t x, const fixed_t y, const line_t *line);
void P_LineOpening (const line_t *linedef);
void P_MakeDivline (line_t* li, divline_t* dl);
void P_SetThingPosition (mobj_t* thing);
void P_UnsetThingPosition (const mobj_t* thing);

// -----------------------------------------------------------------------------
// P_MOBJ
// -----------------------------------------------------------------------------

#define ONFLOORZ        INT_MIN
#define ONCEILINGZ      INT_MAX
#define ITEMQUESIZE     128  // Time interval for item respawning.
// [JN] killough 11/98 - for torque simulation:
#define OVERDRIVE       6
#define MAXGEAR         (OVERDRIVE+16)

extern const int P_FindDoomedNum (unsigned const type);
extern mapthing_t itemrespawnque[ITEMQUESIZE];
extern int itemrespawntime[ITEMQUESIZE];
extern int iquehead;
extern int iquetail;
extern void G_PlayerReborn (const int player);

const boolean P_SetMobjState (mobj_t *mobj, statenum_t state);
mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t *P_SpawnMobj (const fixed_t x, const fixed_t y, const fixed_t z, const mobjtype_t type);
mobj_t *P_SubstNullMobj (mobj_t *th);
void P_MobjThinker (mobj_t *mobj);
void P_RemoveMobj (const mobj_t *th);
void P_RespawnSpecials (void);
void P_SpawnBlood (const fixed_t x, const fixed_t y, fixed_t z, const int damage, mobj_t *target);
void P_SpawnMapThing (mapthing_t *mthing);
void P_SpawnPlayerMissile (mobj_t *source, const mobjtype_t type);
void P_SpawnPuff (const fixed_t x, const fixed_t y, const fixed_t z);
void P_SpawnPuffSafe (const fixed_t x, const fixed_t y, const fixed_t z, const boolean safe);

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

const int EV_DoPlat (line_t *line, plattype_e type, int amount);
void EV_StopPlat (const line_t *line);
void P_ActivateInStasis(const int tag);
void P_AddActivePlat (plat_t *plat);
void P_RemoveActivePlat (const plat_t *plat);
void T_PlatRaise(plat_t *plat);

// -----------------------------------------------------------------------------
// P_PSPR
// -----------------------------------------------------------------------------

// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
#define FF_FULLBRIGHT   0x8000	// flag in thing->frame
#define FF_FRAMEMASK    0x7fff

void P_DropWeapon (player_t *player);
void P_MovePsprites (player_t *player);
void P_SetupPsprites (player_t *player);

// -----------------------------------------------------------------------------
// P_SAVEG
// -----------------------------------------------------------------------------

#define SAVEGAME_EOF        0x1d
#define VERSIONSIZE         16
#define SAVESTRINGSIZE      24

extern FILE *save_stream;
extern boolean savegame_error;
extern void M_ConfirmDeleteGame (void);

boolean P_ReadSaveGameEOF (void);
boolean P_ReadSaveGameHeader (void);
char *P_SaveGameFile (int slot);
char *P_TempSaveGameFile (void);
thinker_t *P_IndexToThinker (uint32_t index);
const uint32_t P_ThinkerToIndex (const thinker_t *thinker);
void P_ArchiveAutomap (void);
void P_ArchivePlayers (void);
void P_ArchiveSpecials (void);
void P_ArchiveThinkers (void);
void P_ArchiveWorld (void);
void P_RestoreTargets (void);
void P_UnArchiveAutomap (void);
void P_UnArchivePlayers (void);
void P_UnArchiveSpecials (void);
void P_UnArchiveThinkers (void);
void P_UnArchiveWorld (void);
void P_WriteSaveGameEOF (void);
void P_WriteSaveGameHeader (char *description);

// -----------------------------------------------------------------------------
// P_SETUP
// -----------------------------------------------------------------------------

// [crispy] blinking key or skull in the status bar
#define KEYBLINKMASK 0x8
#define KEYBLINKTICS (7*KEYBLINKMASK)
extern int st_keyorskull[3];

extern byte     *rejectmatrix;  // for fast sight rejection
extern int32_t  *blockmaplump;  // offsets in blockmap are from here // [crispy] BLOCKMAP limit
extern int32_t  *blockmap;      // [crispy] BLOCKMAP limit
extern int       bmapwidth;
extern int       bmapheight;    // in mapblocks
extern fixed_t   bmaporgx;
extern fixed_t   bmaporgy;      // origin of block map
extern mobj_t  **blocklinks;    // for thing chains

extern const vertexfix_t *selected_vertexfix;
extern const linefix_t   *selected_linefix;
extern const sectorfix_t *selected_sectorfix;
extern const thingfix_t  *selected_thingfix;
extern const flow_t      *selected_flow;
extern const fall_t      *selected_fall;

void P_Init (void);
void P_SetupLevel (const int episode, const int map, const skill_t skill);
void P_SetupFixes (const int episode, const int map);

// -----------------------------------------------------------------------------
// P_SIGHT
// -----------------------------------------------------------------------------

const boolean P_CheckSight (const mobj_t *t1, const mobj_t *t2);

// -----------------------------------------------------------------------------
// P_SPEC
// -----------------------------------------------------------------------------

// End-level timer (-TIMER option)
extern boolean levelTimer;
extern int     levelTimeCount;

const fixed_t P_FindHighestCeilingSurrounding (const sector_t *sec);
const fixed_t P_FindHighestFloorSurrounding (const sector_t *sec);
const fixed_t P_FindLowestCeilingSurrounding (const sector_t *sec);
const fixed_t P_FindLowestFloorSurrounding (const sector_t *sec);
const fixed_t P_FindNextHighestFloor (const sector_t *sec, const int currentheight);
const int P_FindMinSurroundingLight (const sector_t *sector, const int max);
const int P_FindSectorFromLineTag (const line_t *line, const int start);
const int twoSided (const int sector, const int line);
int EV_DoDonut (line_t *line);
sector_t *getNextSector (const line_t *line, const sector_t *sec);
sector_t *getSector (const int currentSector, const int line, const int side);
side_t *getSide (const int currentSector, const int line, const int side);
void P_CrossSpecialLine (const int linenum, const int side, mobj_t *thing);
void P_CrossSpecialLinePtr (line_t *line, const int side, mobj_t *thing); // [crispy] more MBF code pointers
void P_InitPicAnims (void);
void P_PlayerInSpecialSector (player_t *player);
void P_ShootSpecialLine (const mobj_t *thing, line_t *line);
void P_SpawnSpecials (void);
void P_UpdateSpecials (void);
void R_InterpolateTextureOffsets (void);

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

extern button_t	*buttonlist;

const boolean P_UseSpecialLine (mobj_t *thing, line_t *line, const int side);
void P_ChangeSwitchTexture (line_t *line, int useAgain);
void P_InitSwitchList (void);

// -----------------------------------------------------------------------------
// P_TELEPT
// -----------------------------------------------------------------------------

#define MO_TELEPORTMAN  14  // Define values for map objects

const int EV_Teleport (const line_t *line, const int side, mobj_t *thing);

// -----------------------------------------------------------------------------
// P_TICK
// -----------------------------------------------------------------------------

// Both the head and tail of the thinker list.
extern thinker_t thinkercap;	

void P_AddThinker (thinker_t *thinker);
void P_InitThinkers (void);
void P_RemoveThinker (thinker_t *thinker);
void P_Ticker (void);

// -----------------------------------------------------------------------------
// P_USER
// -----------------------------------------------------------------------------

#define INVERSECOLORMAP     32  // Index of the special effects (INVUL inverse) map.
#define INFRAGREENCOLORMAP  1   // [JN] Infra green light amplification visor.
#define MAXBOB              1048576  // 16 pixels of bob (0x100000 in hexademical).
#define ANG5                (ANG90/18)
#define MLOOKUNIT           8

extern boolean max_bobbing;

void P_CalcHeight (player_t *player, boolean safe);
void P_PlayerThink (player_t *player);
