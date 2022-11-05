//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


#pragma once

#include "r_local.h"


/*
================================================================================
=
= P_ENEMY
=
================================================================================
*/

typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
} dirtype_t;

void P_NoiseAlert (mobj_t *target, mobj_t *emmiter);
void P_InitMonsters (void);
void P_AddBossSpot (fixed_t x, fixed_t y, angle_t angle);
void P_Massacre (void);
void P_DSparilTeleport (mobj_t *actor);

/*
================================================================================
=
= P_FIX
=
================================================================================
*/

#define DEFAULT  0x7FFF
#define REMOVE   0

/*
typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     vertex;
    int     oldx, oldy;
    int     newx, newy;
} vertexfix_t;
*/

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     linedef;
    int     side;
    char    *toptexture;
    char    *middletexture;
    char    *bottomtexture;
    short   offset;
    short   rowoffset;
    int     flags;
    int     special;
    int     tag;
} linefix_t;

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     sector;
    char    *floorpic;
    char    *ceilingpic;
    int     floorheight;
    int     ceilingheight;
    int     special;
    int     oldtag;
    int     newtag;
} sectorfix_t;

/*
typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     thing;
    int     type;
    int     oldx, oldy;
    int     newx, newy;
    int     angle;
    int     options;
} thingfix_t;
*/

extern void P_SetupFixes (const int episode, const int map);

extern const linefix_t   *selected_linefix;
extern const sectorfix_t *selected_sectorfix;
/*
extern const vertexfix_t *selected_vertexfix;
extern const thingfix_t  *selected_thingfix;
*/


/* 
================================================================================
=
= P_TICK
=
================================================================================
*/

extern int leveltime;
extern int TimerGame;         // tic countdown for deathmatch
extern thinker_t thinkercap;  // both the head and tail of the thinker list

extern void P_AddThinker (thinker_t *thinker);
extern void P_InitThinkers (void);
extern void P_RemoveThinker (thinker_t *thinker);
extern void P_Ticker (void);

/* 
================================================================================
=
= P_PSPR
=
================================================================================
*/

#define USE_GWND_AMMO_1 1
#define USE_GWND_AMMO_2 1
#define USE_CBOW_AMMO_1 1
#define USE_CBOW_AMMO_2 1
#define USE_BLSR_AMMO_1 1
#define USE_BLSR_AMMO_2 5
#define USE_SKRD_AMMO_1 1
#define USE_SKRD_AMMO_2 5
#define USE_PHRD_AMMO_1 1
#define USE_PHRD_AMMO_2 1
#define USE_MACE_AMMO_1 1
#define USE_MACE_AMMO_2 5

extern weaponinfo_t wpnlev1info[NUMWEAPONS];
extern weaponinfo_t wpnlev2info[NUMWEAPONS];

extern void P_ActivateBeak (player_t *player);
extern void P_AddMaceSpot (const mapthing_t *mthing);
extern void P_CloseWeapons(void);
extern void P_DropWeapon (const player_t *player);
extern void P_MovePsprites (player_t *player);
extern void P_OpenWeapons (void);
extern void P_PostChickenWeapon (player_t *player, weapontype_t weapon);
extern void P_RepositionMace (mobj_t *mo);
extern void P_SetPsprite (const player_t *player, const int position, statenum_t stnum);
extern void P_SetupPsprites (player_t *player);
extern void P_UpdateBeak (player_t *player, pspdef_t *psp);

/* 
================================================================================
=
= P_USER
=
================================================================================
*/

#define PLAYERRADIUS     (16*FRACUNIT)
#define	MAXHEALTH        100
#define MAXCHICKENHEALTH 30
#define	VIEWHEIGHT       (41*FRACUNIT)
#define TOCENTER         -8
#define	GRAVITY          FRACUNIT

extern int P_GetPlayerNum(player_t * player);

extern boolean P_UseArtifact(player_t * player, artitype_t arti);

extern void P_PlayerNextArtifact (player_t *player);
extern void P_PlayerRemoveArtifact (player_t *player, int slot);
extern void P_PlayerThink (player_t *player);
extern void P_PlayerUseArtifact (player_t *player, artitype_t arti);
extern void P_Thrust (player_t *player, angle_t angle, fixed_t move);

/*
================================================================================
=
= P_MOBJ
=
================================================================================
*/

#define FLOOR_SOLID   0
#define FLOOR_WATER   1
#define FLOOR_LAVA    2
#define FLOOR_SLUDGE  3

#define ONFLOORZ      INT_MIN
#define ONCEILINGZ    INT_MAX
#define FLOATRANDZ    (INT_MAX-1)

#define	FLOATSPEED    (FRACUNIT*4)

extern mobjtype_t  PuffType;
extern mobj_t     *MissileMobj;

extern boolean P_CheckMissileSpawn (mobj_t *missile);
extern boolean P_SeekerMissile (mobj_t *actor, angle_t thresh, angle_t turnMax);
extern boolean P_SetMobjState (mobj_t *mobj, statenum_t state);
extern boolean P_SetMobjStateNF (mobj_t *mobj, statenum_t state);

extern int P_FaceMobj (mobj_t *source, mobj_t *target, angle_t *delta);
extern int P_GetThingFloorType (mobj_t *thing);
extern int P_HitFloor (mobj_t *thing);

extern mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, mobjtype_t type);
extern mobj_t *P_SpawnMissileAngle (mobj_t *source, mobjtype_t type, angle_t angle, fixed_t momz);
extern mobj_t *P_SpawnMobj (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
extern mobj_t *P_SpawnMobjSafe (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type, boolean safe);
extern mobj_t *P_SpawnPlayerMissile (mobj_t *source, mobjtype_t type);
extern mobj_t *P_SPMAngle (mobj_t *source, mobjtype_t type, angle_t angle);

extern void P_SpawnPuffSafe (fixed_t x, fixed_t y, fixed_t z, boolean safe);
extern void P_BlasterMobjThinker (mobj_t *mobj);
extern void P_BloodSplatter (fixed_t x, fixed_t y, fixed_t z, mobj_t *originator);
extern void P_MobjThinker (mobj_t *mobj);
extern void P_RemoveMobj (mobj_t *th);
extern void P_RipperBlood (mobj_t *mo);
extern void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage);
extern void P_SpawnMapThing (mapthing_t *mthing);
extern void P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z);
extern void P_ThrustMobj (mobj_t *mo, angle_t angle, fixed_t move);

/*
================================================================================
=
= P_MAPUTL
=
================================================================================
*/

#define FOOTCLIPSIZE    10*FRACUNIT

#define PT_ADDLINES     1
#define	PT_ADDTHINGS    2
#define	PT_EARLYOUT     4

#define	MAXINTERCEPTS   128

typedef struct
{
    fixed_t frac;  // along trace line
    boolean isaline;
    union
    {
        mobj_t *thing;
        line_t *line;
    } d;
} intercept_t;

extern intercept_t *intercepts, *intercept_p; // [crispy] remove INTERCEPTS limit

typedef struct
{
    fixed_t x, y, dx, dy;
} divline_t;

typedef boolean (*traverser_t) (intercept_t *in);

extern boolean P_BlockLinesIterator (int x, int y, boolean(*func) (line_t *));
extern boolean P_BlockThingsIterator (int x, int y, boolean(*func) (mobj_t *));
extern boolean P_PathTraverse (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                               int flags, boolean(*trav) (intercept_t *));

extern divline_t trace;

extern fixed_t lowfloor;
extern fixed_t opentop, openbottom, openrange;
extern fixed_t P_ApproxDistanceZ (fixed_t dx, fixed_t dy, fixed_t dz);
extern fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
extern fixed_t P_InterceptVector (divline_t *v2, divline_t *v1);

extern int P_BoxOnLineSide (const fixed_t *tmbox, const line_t *ld);
extern int P_PointOnDivlineSide (fixed_t x, fixed_t y, const divline_t *line);
extern int P_PointOnLineSide (fixed_t x, fixed_t y, const line_t *line);

extern void check_intercept (const int func); // [crispy] remove INTERCEPTS limit
extern void P_LineOpening (line_t *linedef);
extern void P_MakeDivline (line_t *li, divline_t *dl);
extern void P_SetThingPosition (mobj_t *thing);
extern void P_UnsetThingPosition (mobj_t *thing);

/*
================================================================================
=
= P_MAP
=
================================================================================
*/

#define	USERANGE (64*FRACUNIT)
#define	MELEERANGE (64*FRACUNIT)
#define	MISSILERANGE (32*64*FRACUNIT)

extern boolean floatok;  // if true, move would be ok if
extern boolean P_ChangeSector(sector_t * sector, boolean crunch);
extern boolean P_CheckPosition(mobj_t * thing, fixed_t x, fixed_t y);
extern boolean P_CheckSight(mobj_t * t1, mobj_t * t2);
extern boolean P_TeleportMove(mobj_t * thing, fixed_t x, fixed_t y);
extern boolean P_TestMobjLocation(mobj_t * mobj);
extern boolean P_TryMove(mobj_t * thing, fixed_t x, fixed_t y);
extern boolean PIT_ChangeSector(mobj_t * thing);
extern boolean PIT_RadiusAttack(mobj_t * thing);

extern fixed_t P_AimLineAttack(mobj_t * t1, angle_t angle, fixed_t distance);
extern fixed_t tmfloorz, tmceilingz;  // within tmfloorz - tmceilingz

extern int      numspechit;
extern line_t **spechit;  // [crispy] remove SPECHIT limit

extern line_t *ceilingline;

extern mobj_t *BlockingMobj;
extern mobj_t *linetarget;  // who got hit (or NULL)
extern mobj_t *P_CheckOnmobj(mobj_t * thing);

extern void P_FakeZMovement(mobj_t * mo);
extern void P_LineAttack(mobj_t * t1, angle_t angle, fixed_t distance, fixed_t slope, int damage);
extern void P_RadiusAttack(mobj_t * spot, mobj_t * source, int damage);
extern void P_SlideMove(mobj_t * mo);
extern void P_UseLines(player_t * player);

/*
================================================================================
=
= P_SAVEG
=
================================================================================
*/

#define SAVEGAMESIZE          0x30000
#define SAVESTRINGSIZE        24
#define SAVE_GAME_TERMINATOR  0x1d
#define	SAVEGAMENAME          "heretic-save-"

extern char *savegamedir;

extern thinker_t *P_IndexToThinker (uint32_t index);

extern uint32_t P_ThinkerToIndex (thinker_t* thinker);

// Support routines for saving games
extern byte SV_ReadByte (void);
extern char *SV_Filename (int slot);
extern uint16_t SV_ReadWord (void);
extern uint32_t SV_ReadLong (void);
extern void SV_Close (char *fileName);
extern void SV_Open (char *fileName);
extern void SV_OpenRead (char *fileName);
extern void SV_Read (void *buffer, int size);
extern void SV_Write (void *buffer, int size);
extern void SV_WriteByte (byte val);
extern void SV_WriteLong (unsigned int val);
extern void SV_WriteWord (unsigned short val);

extern void P_ArchiveAutomap (void);
extern void P_ArchivePlayers (void);
extern void P_ArchiveSpecials (void);
extern void P_ArchiveThinkers (void);
extern void P_ArchiveWorld (void);
extern void P_RestoreTargets (void);
extern void P_UnArchiveAutomap (void);
extern void P_UnArchivePlayers (void);
extern void P_UnArchiveSpecials (void);
extern void P_UnArchiveThinkers (void);
extern void P_UnArchiveWorld (void);

/*
================================================================================
=
= P_SETUP
=
================================================================================
*/

#define MAPBLOCKUNITS	128
#define	MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define	MAPBLOCKSHIFT	(FRACBITS+7)
#define	MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)
#define MAXRADIUS       (32*FRACUNIT)
#define	MAXMOVE         (30*FRACUNIT)

extern byte *rejectmatrix;          // for fast sight rejection

extern fixed_t bmaporgx, bmaporgy;  // origin of block map

extern int bmapwidth, bmapheight;   // in mapblocks

extern int32_t *blockmap;
extern int32_t *blockmaplump;       // offsets in blockmap are from here

extern mobj_t **blocklinks;         // for thing chains

extern void P_Init (void);
extern void P_SetupLevel (int episode, int map, int playermask, skill_t skill);

/*
================================================================================
=
= P_SPEC
=
================================================================================
*/

#define	MAXANIMS        32
#define	MAXLINEANIMS    64*256  // [JN] Multipled by 256
#define	MO_TELEPORTMAN  14

// Animating textures and planes
typedef struct
{
    boolean istexture;
    int picnum;
    int basepic;
    int numpics;
    int speed;
} anim_t;

// Source animation definition
typedef struct
{
    int  istexture;  // if false, it's a flat
    char endname[9];
    char startname[9];
    int  speed;
} animdef_t;

extern anim_t  anims[MAXANIMS], *lastanim;
extern int    *TerrainTypes;

// Animating line specials

extern line_t *linespeciallist[MAXLINEANIMS];
extern short   numlinespecials;

// Define values for map objects:

// At game start
extern void P_InitPicAnims (void);
extern void P_InitTerrainTypes (void);
extern void P_InitLava (void);

// At map load
extern void P_AddAmbientSfx (int sequence);
extern void P_InitAmbientSound (void);
extern void P_SpawnSpecials (void);

// Every tic
extern void P_AmbientSound (void);
extern void P_UpdateSpecials (void);

// When needed
extern boolean P_UseSpecialLine (mobj_t *thing, line_t *line);

extern fixed_t P_FindHighestCeilingSurrounding (sector_t *sec);
extern fixed_t P_FindHighestFloorSurrounding (sector_t *sec);
extern fixed_t P_FindLowestCeilingSurrounding (sector_t *sec);
extern fixed_t P_FindLowestFloorSurrounding (sector_t *sec);
extern fixed_t P_FindNextHighestFloor (sector_t *sec, int currentheight);

extern int EV_DoDonut (line_t *line);
extern int P_FindMinSurroundingLight (sector_t *sector, int max);
extern int P_FindSectorFromLineTag (line_t *line, int start);
extern int twoSided (int sector, int line);

extern sector_t *getNextSector (line_t *line, sector_t *sec);
extern sector_t *getSector (int currentSector, int line, int side);

extern side_t *getSide (int currentSector, int line, int side);

extern void P_CrossSpecialLine (int linenum, int side, mobj_t *thing);
extern void P_PlayerInSpecialSector (player_t *player);
extern void P_ShootSpecialLine (mobj_t *thing, line_t *line);

/*
================================================================================
=
= P_INTER
=
================================================================================
*/

#define BASETHRESHOLD 100  // follow a player exlusively for 3 seconds

extern boolean P_ChickenMorphPlayer(player_t *player);
extern boolean P_GiveAmmo(player_t *player, ammotype_t ammo, int count);
extern boolean P_GiveArtifact(player_t *player, artitype_t arti, mobj_t *mo);
extern boolean P_GiveBody(player_t *player, int num);
extern boolean P_GivePower(player_t *player, powertype_t power);

extern int clipammo[NUMAMMO];
extern int maxammo[NUMAMMO];

extern void P_DamageMobj (mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage);
extern void P_SetMessage (player_t *player, char *message, MessageType_t messageType, boolean ultmsg);
extern void P_TouchSpecialThing (mobj_t *special, mobj_t *toucher);

/*
================================================================================
=
= P_CEILNG
=
================================================================================
*/

#define	CEILSPEED    FRACUNIT
#define	CEILWAIT     150
#define MAXCEILINGS  30

typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise
} ceiling_e;

typedef struct
{
    thinker_t thinker;
    ceiling_e type;
    sector_t *sector;
    fixed_t bottomheight, topheight;
    fixed_t speed;
    boolean crush;
    int direction;      // 1 = up, 0 = waiting, -1 = down
    int tag;            // ID
    int olddirection;
} ceiling_t;

extern ceiling_t *activeceilings[MAXCEILINGS];

extern int EV_CeilingCrushStop (line_t *line);
extern int EV_DoCeiling (line_t *line, ceiling_e type);

extern void P_ActivateInStasisCeiling (line_t *line);
extern void P_AddActiveCeiling (ceiling_t *c);
extern void P_RemoveActiveCeiling (ceiling_t *c);
extern void T_MoveCeiling (ceiling_t *ceiling);

/*
================================================================================
=
= P_FLOOR
=
================================================================================
*/

#define	FLOORSPEED  FRACUNIT

typedef enum
{
    lowerFloor,                 // lower floor to highest surrounding floor
    lowerFloorToLowest,         // lower floor to lowest surrounding floor
    turboLower,                 // lower floor to highest surrounding floor VERY FAST
    raiseFloor,                 // raise floor to lowest surrounding CEILING
    raiseFloorToNearest,        // raise floor to next highest surrounding floor
    raiseToTexture,             // raise floor to shortest height texture around it
    lowerAndChange,             // lower floor to lowest surrounding floor and change
    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,
    donutRaise,
    raiseBuildStep              // One step of a staircase
} floor_e;

typedef struct
{
    thinker_t thinker;
    floor_e type;
    boolean crush;
    sector_t *sector;
    int direction;
    int newspecial;
    short texture;
    fixed_t floordestheight;
    fixed_t speed;
} floormove_t;

typedef enum
{
    ok,
    crushed,
    pastdest
} result_e;

extern result_e T_MovePlane(sector_t *sector, fixed_t speed,
                            fixed_t dest, boolean crush, int floorOrCeiling,
                            int direction);

extern int EV_BuildStairs (line_t *line, fixed_t stepDelta);
extern int EV_DoFloor (line_t *line, floor_e floortype);

extern void T_MoveFloor (floormove_t *floor);

/*
================================================================================
=
= P_DOORS
=
================================================================================
*/

#define	VDOORSPEED  FRACUNIT*2
#define	VDOORWAIT   150

typedef enum
{
    vld_normal,
    vld_close30ThenOpen,
    vld_close,
    vld_open,
    vld_raiseIn5Mins
} vldoor_e;

typedef struct
{
    thinker_t thinker;
    vldoor_e type;
    sector_t *sector;
    fixed_t topheight;
    fixed_t speed;
    int direction;      // 1 = up, 0 = waiting at top, -1 = down
    int topwait;        // tics to wait at the top
    // (keep in case a door going down is reset)
    int topcountdown;   // when it reaches 0, start going down
} vldoor_t;

extern int EV_DoDoor (line_t *line, vldoor_e type, fixed_t speed);

extern void EV_VerticalDoor (line_t *line, mobj_t *thing);
extern void P_SpawnDoorCloseIn30 (sector_t *sec);
extern void P_SpawnDoorRaiseIn5Mins (sector_t *sec, int secnum);
extern void T_VerticalDoor (vldoor_t *door);

/*
================================================================================
=
= P_LIGHTS
=
================================================================================
*/

#define GLOWSPEED       8
#define	STROBEBRIGHT    5
#define	FASTDARK        15
#define	SLOWDARK        35

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;
} lightflash_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int count;
    int minlight;
    int maxlight;
    int darktime;
    int brighttime;
} strobe_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int minlight;
    int maxlight;
    int direction;
} glow_t;

extern void EV_LightTurnOn (line_t *line, int bright);
extern void EV_StartLightStrobing (line_t *line);
extern void EV_TurnTagLightsOff (line_t *line);
extern void P_SpawnGlowingLight (sector_t *sector);
extern void P_SpawnLightFlash (sector_t *sector);
extern void P_SpawnStrobeFlash (sector_t *sector, int fastOrSlow, int inSync);
extern void T_Glow (glow_t *g);
extern void T_LightFlash (lightflash_t *flash);
extern void T_StrobeFlash (strobe_t *flash);


/*
================================================================================
=
= P_PLATS
=
================================================================================
*/

#define	PLATWAIT    3
#define	PLATSPEED   FRACUNIT
#define	MAXPLATS    30*256  // [JN] Multipled by 256

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
    raiseToNearestAndChange
} plattype_e;

typedef struct
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
    boolean crush;
    int tag;
    plattype_e type;
} plat_t;

extern int EV_DoPlat (line_t *line, plattype_e type, int amount);

extern plat_t *activeplats[MAXPLATS];

extern void EV_StopPlat (line_t *line);
extern void P_ActivateInStasis (int tag);
extern void P_AddActivePlat (plat_t *plat);
extern void P_RemoveActivePlat (plat_t *plat);
extern void T_PlatRaise (plat_t *plat);

/*
================================================================================
=
= P_SWITCH
=
================================================================================
*/

#define	MAXSWITCHES	50      // max # of wall switches in a level
#define	MAXBUTTONS	16      // 4 players, 4 buttons each at once, max.
#define BUTTONTIME	35      // 1 second

typedef struct
{
    char name1[9];
    char name2[9];
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
    line_t *line;
    bwhere_e where;
    int btexture;
    int btimer;
    void *soundorg;
} button_t;

extern button_t buttonlist[MAXBUTTONS];

extern void P_ChangeSwitchTexture (line_t *line, int useAgain);
extern void P_InitSwitchList (void);

/*
================================================================================
=
= P_TELEPT
=
================================================================================
*/

extern boolean EV_Teleport (line_t *line, int side, mobj_t *thing);
extern boolean P_Teleport (mobj_t *thing, fixed_t x, fixed_t y, angle_t angle);
