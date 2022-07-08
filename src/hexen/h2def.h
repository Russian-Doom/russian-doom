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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <values.h>

#include "st_start.h"
// haleyjd: removed WATCOMC

// ticcmd:

#include "d_ticcmd.h"

// events

#include "d_event.h"

// gamemode/mission

#include "d_mode.h"

// for fixed_t:

#include "m_fixed.h"

// angle definitions:

#include "tables.h"

#include "d_loop.h"
#include "net_defs.h"
#include "sounds.h"
#include "rd_menu_control.h"
#include "rd_text.h"
#include "jn.h"


#define HEXEN_VERSION 110
#define HEXEN_VERSION_TEXT "v1.1"

// if rangecheck is undefined, most parameter validation debugging code
// will not be compiled
#ifndef NORANGECHECKING
#define RANGECHECK
#endif

// Past distributions
//#ifndef VER_ID
//#define VER_ID "DVL"
//#endif
//#define HEXEN_VERSIONTEXT "ID V1.2"
//#define HEXEN_VERSIONTEXT "RETAIL STORE BETA"               // 9/26/95
//#define HEXEN_VERSIONTEXT "DVL BETA 10 05 95" // Used for GT for testing
//#define HEXEN_VERSIONTEXT "DVL BETA 10 07 95" // Just an update for Romero
//#define HEXEN_VERSIONTEXT "FINAL 1.0 (10 13 95)" // Just an update for Romero
//#ifdef RANGECHECK
//#define HEXEN_VERSIONTEXT "Version 1.1 +R "__DATE__" ("VER_ID")"
//#else
//#define HEXEN_VERSIONTEXT "Version 1.1 "__DATE__" ("VER_ID")"
//#endif
#define HEXEN_VERSIONTEXT ((gamemode == shareware) ? \
                           "DEMO 10 16 95" : \
                           "VERSION 1.1 MAR 22 1996 (BCP)")

#define HEXEN_VERSIONTEXT_RUS ((gamemode == shareware) ? \
                               "LTVJDTHCBZ (16>10>1995)" : \
                               "DTHCBZ 1>1 (22>03>1996)")                           

// all exterior data is defined here
#include "xddefs.h"

// all important printed strings
#include "rd_lang.h"

// header generated by multigen utility
#include "info.h"


/*
===============================================================================

						GLOBAL TYPES

===============================================================================
*/

//#define NUMARTIFCTS   28
#define MAXPLAYERS	8

#define	BT_ATTACK		1
#define	BT_USE			2
#define	BT_CHANGE		4       // if true, the next 3 bits hold weapon num
#define	BT_WEAPONMASK	(8+16+32)
#define	BT_WEAPONSHIFT	3

#define BT_SPECIAL		128     // game events, not really buttons
#define	BTS_SAVEMASK	(4+8+16)
#define	BTS_SAVESHIFT	2
#define	BT_SPECIALMASK	3
#define	BTS_PAUSE		1       // pause the game
#define	BTS_SAVEGAME	2       // save the game at each console
// savegame slot numbers occupy the second byte of buttons

// The top 3 bits of the artifact field in the ticcmd_t struct are used
//              as additional flags 
#define AFLAG_MASK			0x3F
#define AFLAG_SUICIDE		0x40
#define AFLAG_JUMP			0x80

typedef enum
{
    ga_nothing,
    ga_loadlevel,
    ga_initnew,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_playdemo,
    ga_completed,
    ga_leavemap,
    ga_singlereborn,
    ga_victory,
    ga_worlddone,
    ga_screenshot
} gameaction_t;

typedef enum
{
    wipe_0,
    wipe_1,
    wipe_2,
    wipe_3,
    wipe_4,
    NUMWIPES,
    wipe_random
} wipe_t;

/*
===============================================================================

							MAPOBJ DATA

===============================================================================
*/

// think_t is a function pointer to a routine to handle an actor
typedef void (*think_t) ();

typedef struct thinker_s
{
    struct thinker_s *prev, *next;
    think_t function;
} thinker_t;

struct player_s;

typedef union
{
    intptr_t i;
    struct mobj_s *m;
    struct player_s *p;
} specialval_t;

typedef struct mobj_s
{
    thinker_t thinker;          // thinker node

// info for drawing
    fixed_t x, y, z;
    struct mobj_s *snext, *sprev;       // links in sector (if needed)
    angle_t angle;
    spritenum_t sprite;         // used to find patch_t and flip value
    int frame;                  // might be ord with FF_FULLBRIGHT

// interaction info
    struct mobj_s *bnext, *bprev;       // links in blocks (if needed)
    struct subsector_s *subsector;
    fixed_t floorz, ceilingz;   // closest together of contacted secs
    fixed_t floorpic;           // contacted sec floorpic
    fixed_t radius, height;     // for movement checking
    fixed_t momx, momy, momz;   // momentums
    int validcount;             // if == validcount, already checked
    mobjtype_t type;
    mobjinfo_t *info;           // &mobjinfo[mobj->type]
    int tics;                   // state tic counter
    state_t *state;
    int damage;                 // For missiles
    int flags;
    int flags2;                 // Heretic flags
    int intflags;               // [JN] killough 9/15/98: internal flags
    specialval_t special1;      // Special info
    specialval_t special2;      // Special info
    int health;
    int movedir;                // 0-7
    int movecount;              // when 0, select a new dir
    struct mobj_s *target;      // thing being chased/attacked (or NULL)
    // also the originator for missiles
    int reactiontime;           // if non 0, don't attack yet
    // used by player to freeze a bit after
    // teleporting
    int threshold;              // if > 0, the target will be chased
    short gear;                 // [JN] killough 11/98: used in torque simulation
    int geartics;               // [JN] Duration of torque sumulation.
    // no matter what (even if shot)
    struct player_s *player;    // only valid if type == MT_PLAYER
    int lastlook;               // player number last looked for
    fixed_t floorclip;          // value to use for floor clipping
    int archiveNum;             // Identity during archive
    short tid;                  // thing identifier
    byte special;               // special
    byte args[5];               // special arguments
    // [AM] If true, ok to interpolate this tic.
    boolean interp;
    // [AM] Previous position of mobj before think.
    //      Used to interpolate between positions.
    fixed_t oldx;
    fixed_t oldy;
    fixed_t oldz;
    angle_t oldangle;
    fixed_t oldfloorclip;  // [JN] Interpolate floor clipping.
} mobj_t;

// [JN] killough 11/98:
// For torque simulation:

void P_ApplyTorque(mobj_t *mo); // killough 9/12/98 
#define OVERDRIVE 6
#define MAXGEAR (OVERDRIVE+16)

// each sector has a degenmobj_t in it's center for sound origin purposes
typedef struct
{
    thinker_t thinker;          // not used for anything
    fixed_t x, y, z;
} degenmobj_t;

//
// frame flags
//
#define	FF_FULLBRIGHT	0x8000  // flag in thing->frame
#define FF_FRAMEMASK	0x7fff

// --- mobj.flags ---

#define	MF_SPECIAL		1       // call P_SpecialThing when touched
#define	MF_SOLID		2
#define	MF_SHOOTABLE	4
#define	MF_NOSECTOR		8       // don't use the sector links
                                                                        // (invisible but touchable)
#define	MF_NOBLOCKMAP	16      // don't use the blocklinks
                                                                        // (inert but displayable)
#define	MF_AMBUSH		32
#define	MF_JUSTHIT		64      // try to attack right back
#define	MF_JUSTATTACKED	128     // take at least one step before attacking
#define	MF_SPAWNCEILING	256     // hang from ceiling instead of floor
#define	MF_NOGRAVITY	512     // don't apply gravity every tic

// movement flags
#define	MF_DROPOFF		0x400   // allow jumps from high places
#define	MF_PICKUP		0x800   // for players to pick up items
#define	MF_NOCLIP		0x1000  // player cheat
#define	MF_SLIDE		0x2000  // keep info about sliding along walls
#define	MF_FLOAT		0x4000  // allow moves to any height, no gravity
#define	MF_TELEPORT		0x8000  // don't cross lines or look at heights
#define MF_MISSILE		0x10000 // don't hit same species, explode on block

#define	MF_ALTSHADOW	0x20000 // alternate translucent draw
#define	MF_SHADOW		0x40000 // use translucent draw (shadow demons / invis)
#define	MF_NOBLOOD		0x80000 // don't bleed when shot (use puff)
#define	MF_CORPSE		0x100000        // don't stop moving halfway off a step
#define	MF_INFLOAT		0x200000        // floating to a height for a move, don't
                                                                        // auto float to target's height

#define	MF_COUNTKILL	0x400000        // count towards intermission kill total
#define	MF_ICECORPSE	0x800000        // a frozen corpse (for blasting)

#define	MF_SKULLFLY		0x1000000       // skull in flight
#define	MF_NOTDMATCH	0x2000000       // don't spawn in death match (key cards)

//#define       MF_TRANSLATION  0xc000000       // if 0x4 0x8 or 0xc, use a translation
#define	MF_TRANSLATION	0x1c000000      // use a translation table (>>MF_TRANSHIFT)
#define	MF_TRANSSHIFT	26      // table for player colormaps

#define MF_EXTRATRANS	0x4000000 // [JN] Extra translucency

// --- mobj.flags2 ---

#define MF2_LOGRAV			0x00000001      // alternate gravity setting
#define MF2_WINDTHRUST		0x00000002      // gets pushed around by the wind
                                                                                // specials
#define MF2_FLOORBOUNCE		0x00000004      // bounces off the floor
#define MF2_BLASTED			0x00000008      // missile will pass through ghosts
#define MF2_FLY				0x00000010      // fly mode is active
#define MF2_FLOORCLIP		0x00000020      // if feet are allowed to be clipped
#define MF2_SPAWNFLOAT		0x00000040      // spawn random float z
#define MF2_NOTELEPORT		0x00000080      // does not teleport
#define MF2_RIP				0x00000100      // missile rips through solid
                                                                                // targets
#define MF2_PUSHABLE		0x00000200      // can be pushed by other moving
                                                                                // mobjs
#define MF2_SLIDE			0x00000400      // slides against walls
#define MF2_ONMOBJ			0x00000800      // mobj is resting on top of another
                                                                                // mobj
#define MF2_PASSMOBJ		0x00001000      // Enable z block checking.  If on,
                                                                                // this flag will allow the mobj to
                                                                                // pass over/under other mobjs.
#define MF2_CANNOTPUSH		0x00002000      // cannot push other pushable mobjs
#define MF2_DROPPED			0x00004000      // dropped by a demon
#define MF2_BOSS			0x00008000      // mobj is a major boss
#define MF2_FIREDAMAGE		0x00010000      // does fire damage
#define MF2_NODMGTHRUST		0x00020000      // does not thrust target when
                                                                                // damaging
#define MF2_TELESTOMP		0x00040000      // mobj can stomp another
#define MF2_FLOATBOB		0x00080000      // use float bobbing z movement
#define MF2_DONTDRAW		0x00100000      // don't generate a vissprite
#define MF2_IMPACT			0x00200000      // an MF_MISSILE mobj can activate
                                                                                // SPAC_IMPACT
#define MF2_PUSHWALL		0x00400000      // mobj can push walls
#define MF2_MCROSS			0x00800000      // can activate monster cross lines
#define MF2_PCROSS			0x01000000      // can activate projectile cross lines
#define MF2_CANTLEAVEFLOORPIC 0x02000000        // stay within a certain floor type
#define MF2_NONSHOOTABLE	0x04000000      // mobj is totally non-shootable,
                                                                                // but still considered solid
#define MF2_INVULNERABLE	0x08000000      // mobj is invulnerable
#define MF2_DORMANT			0x10000000      // thing is dormant
#define MF2_ICEDAMAGE		0x20000000      // does ice damage
#define MF2_SEEKERMISSILE	0x40000000      // is a seeker (for reflection)
#define MF2_REFLECTIVE		0x80000000      // reflects missiles

// [JN] killough 9/15/98: Same, but internal flags, not intended for .deh
// (some degree of opaqueness is good, to avoid compatibility woes)
enum {
    MIF_FALLING = 1,      // Object is falling
    MIF_ARMED = 2,        // Object is armed (for MF_TOUCHY objects)
    MIF_LINEDONE = 4,     // Object has activated W1 or S1 linedef via DEH frame
};

//=============================================================================

// ===== Player Class Types =====
typedef enum
{
    PCLASS_FIGHTER,
    PCLASS_CLERIC,
    PCLASS_MAGE,
    PCLASS_PIG,
    PCLASS_RANDOM,
    NUMCLASSES
} pclass_t;

typedef enum
{
    PST_LIVE,                   // playing
    PST_DEAD,                   // dead on the ground
    PST_REBORN                  // ready to restart
} playerstate_t;

// psprites are scaled shapes directly on the view screen
// coordinates are given for a 320*200 view screen
typedef enum
{
    ps_weapon,
    ps_flash,
    NUMPSPRITES
} psprnum_t;

typedef struct
{
    state_t *state;             // a NULL state means not active
    int tics;
    fixed_t sx, sy;
} pspdef_t;

/* Old Heretic key type
typedef enum
{
	key_yellow,
	key_green,
	key_blue,
	NUMKEYS
} keytype_t;
*/

typedef enum
{
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_A,
    KEY_B,
    NUMKEYS
} keytype_t;

typedef enum
{
    ARMOR_ARMOR,
    ARMOR_SHIELD,
    ARMOR_HELMET,
    ARMOR_AMULET,
    NUMARMOR
} armortype_t;

typedef enum
{
    WP_FIRST,
    WP_SECOND,
    WP_THIRD,
    WP_FOURTH,
    NUMWEAPONS,
    WP_NOCHANGE
} weapontype_t;

typedef enum
{
    MANA_1,
    MANA_2,
    NUMMANA,
    MANA_BOTH,
    MANA_NONE
} manatype_t;

#define MAX_MANA	200

#define WPIECE1		1
#define WPIECE2		2
#define WPIECE3		4

typedef struct
{
    manatype_t mana;
    int upstate;
    int downstate;
    int readystate;
    int atkstate;
    int holdatkstate;
    int flashstate;
} weaponinfo_t;

extern weaponinfo_t WeaponInfo[NUMWEAPONS][NUMCLASSES];

typedef enum
{
    arti_none,
    arti_invulnerability,
    arti_health,
    arti_superhealth,
    arti_healingradius,
    arti_summon,
    arti_torch,
    arti_egg,
    arti_fly,
    arti_blastradius,
    arti_poisonbag,
    arti_teleportother,
    arti_speed,
    arti_boostmana,
    arti_boostarmor,
    arti_teleport,
    // Puzzle artifacts
    arti_firstpuzzitem,
    arti_puzzskull = arti_firstpuzzitem,
    arti_puzzgembig,
    arti_puzzgemred,
    arti_puzzgemgreen1,
    arti_puzzgemgreen2,
    arti_puzzgemblue1,
    arti_puzzgemblue2,
    arti_puzzbook1,
    arti_puzzbook2,
    arti_puzzskull2,
    arti_puzzfweapon,
    arti_puzzcweapon,
    arti_puzzmweapon,
    arti_puzzgear1,
    arti_puzzgear2,
    arti_puzzgear3,
    arti_puzzgear4,
    NUMARTIFACTS
} artitype_t;

typedef enum
{
    pw_None,
    pw_invulnerability,
    pw_allmap,
    pw_infrared,
    pw_flight,
    pw_shield,
    pw_health2,
    pw_speed,
    pw_minotaur,
    NUMPOWERS
} powertype_t;

#define	INVULNTICS (30*35)
#define	INVISTICS (60*35)
#define	INFRATICS (120*35)
#define	IRONTICS (60*35)
#define WPNLEV2TICS (40*35)
#define FLIGHTTICS (60*35)
#define SPEEDTICS (45*35)
#define MORPHTICS (40*35)
#define MAULATORTICS (25*35)

#define MESSAGETICS (4*35)
#define BLINKTHRESHOLD (4*35)

#define NUMINVENTORYSLOTS	NUMARTIFACTS

typedef struct
{
    int type;
    int count;
} inventory_t;

/*
================
=
= player_t
=
================
*/

typedef struct player_s
{
    mobj_t *mo;
    playerstate_t playerstate;
    ticcmd_t cmd;

    pclass_t class;             // player class type

    fixed_t viewz;              // focal origin above r.z
    fixed_t viewheight;         // base height above floor for viewz
    fixed_t deltaviewheight;    // squat speed
    fixed_t bob;                // bounded/scaled total momentum

    int flyheight;
    int lookdir, oldlookdir;
    boolean centering;
    int health;                 // only used between levels, mo->health
    // is used during levels
    int armorpoints[NUMARMOR];

    inventory_t inventory[NUMINVENTORYSLOTS];
    artitype_t readyArtifact;
    int artifactCount;
    int inventorySlotNum;
    int powers[NUMPOWERS];
    int keys;
    int pieces;                 // Fourth Weapon pieces
    signed int frags[MAXPLAYERS];       // kills of other players
    weapontype_t readyweapon;
    weapontype_t pendingweapon; // wp_nochange if not changing
    boolean weaponowned[NUMWEAPONS];
    int mana[NUMMANA];
    int attackdown, usedown;    // true if button down last tic
    int cheats;                 // bit flags

    int refire;                 // refired shots are less accurate

    int killcount, itemcount, secretcount;      // for intermission
    char message[80];           // hint messages
    int messageTics;            // counter for showing messages
    // [JN] Colored message type
    MessageType_t messageType;
    short ultimateMessage;
    short yellowMessage;
    boolean engOnlyMessage;
    int damagecount, bonuscount;        // for screen flashing
    int poisoncount;            // screen flash for poison damage
    mobj_t *poisoner;           // NULL for non-player mobjs
    mobj_t *attacker;           // who did damage (NULL for floors)
    int extralight;             // so gun flashes light up areas
    int fixedcolormap;          // can be set to REDCOLORMAP, etc
    int colormap;               // 0-3 for which color to draw player
    pspdef_t psprites[NUMPSPRITES];     // view sprites (gun, etc)
    int morphTics;              // player is a pig if > 0
    unsigned int jumpTics;      // delay the next jump for a moment
    unsigned int worldTimer;    // total time the player's been playing
    // [AM] Previous position of viewz before think.
    //      Used to interpolate between camera positions.
    angle_t oldviewz;
} player_t;

#define CF_NOCLIP		1
#define	CF_GODMODE		2
#define	CF_NOMOMENTUM	4       // not really a cheat, just a debug aid

#define	SBARHEIGHT	(39 << hires)      // status bar height at bottom of screen

void NET_SendFrags(player_t * player);

/*
===============================================================================

					GLOBAL VARIABLES

===============================================================================
*/

#define TELEFOGHEIGHT (32*FRACUNIT)

extern GameMode_t gamemode;         // Always commercial

extern gameaction_t gameaction;

extern boolean paused;

extern boolean DevMaps;         // true = map development mode
extern char *DevMapsDir;        // development maps directory

extern boolean nomonsters;      // checkparm of -nomonsters

extern boolean respawnparm;     // checkparm of -respawn

extern boolean randomclass;     // checkparm of -randclass

extern boolean usergame;        // ok to save / end game

extern boolean ravpic;          // checkparm of -ravpic

extern boolean altpal;          // checkparm to use an alternate palette routine

extern boolean cdrom;           // true if cd-rom mode active ("-cdrom")

extern boolean deathmatch;      // only if started as net death

extern boolean netgame;         // only true if >1 player

extern boolean cmdfrag;         // true if a CMD_FRAG packet should be sent out every
                                                // kill
extern boolean automapactive;

extern boolean devparm;         // [JN] DEBUG: launched with -devparm

extern boolean playeringame[MAXPLAYERS];
extern pclass_t PlayerClass[MAXPLAYERS];

extern int consoleplayer;       // player taking events and displaying

extern int displayplayer;

extern int viewangleoffset;     // ANG90 = left side, ANG270 = right

extern player_t players[MAXPLAYERS];

extern boolean DebugSound;      // debug flag for displaying sound info

extern boolean demorecording;
extern boolean demoplayback;
extern boolean demoextend;      // allow demos to persist through exit/respawn
extern int maxzone;             // Maximum chunk allocated for zone heap

// Truncate angleturn in ticcmds to nearest 256.
// Used when recording Vanilla demos in netgames.
extern boolean lowres_turn;

extern int Sky1Texture;
extern int Sky2Texture;

extern gamestate_t gamestate;
extern skill_t gameskill;
//extern        boolean         respawnmonsters;
extern int gameepisode;
extern int gamemap;
extern int prevmap;
extern int levelstarttic;       // gametic at level start
extern int leveltime;           // tics in game play for par

extern ticcmd_t *netcmds;

#define MAXDEATHMATCHSTARTS 16
extern mapthing_t *deathmatch_p;
extern mapthing_t deathmatchstarts[MAXDEATHMATCHSTARTS];

// Position indicator for cooperative net-play reborn
extern int RebornPosition;

#define MAX_PLAYER_STARTS 8
extern mapthing_t playerstarts[MAX_PLAYER_STARTS][MAXPLAYERS];
extern int maxplayers;

extern int mouseSensitivity;

extern boolean precache;        // if true, load all graphics at level load

extern byte *screen;            // off screen work buffer, from V_video.c

extern boolean singledemo;      // quit after playing a demo from cmdline

extern int bodyqueslot;
extern skill_t startskill;
extern int startepisode;
extern int startmap;
extern boolean autostart;

extern int show_messages;

extern boolean testcontrols;
extern int testcontrols_mousespeed;

// [JN] Interpolation for weapon bobbing
extern boolean realframe;
extern boolean skippsprinterp;

/*
===============================================================================

					GLOBAL FUNCTIONS

===============================================================================
*/

#include "w_wad.h"
#include "z_zone.h"

//----------
//BASE LEVEL
//----------
void H2_Main(void);
// not a globally visible function, just included for source reference
// calls all startup code
// parses command line options
// if not overrided, calls N_AdvanceDemo

void H2_GameLoop(void);
// not a globally visible function, just included for source reference
// called by H2_Main, never exits
// manages timing and IO
// calls all ?_Responder, ?_Ticker, and ?_Drawer functions
// calls I_GetTime, I_StartFrame, and I_StartTic

//---------
//SYSTEM IO
//---------
byte *I_AllocLow(int length);
// allocates from low memory under dos, just mallocs under unix

// haleyjd: was WATCOMC, again preserved for historical interest as in Heretic
#if 0
extern boolean useexterndriver;

#define EBT_FIRE			1
#define EBT_OPENDOOR 		2
#define EBT_SPEED			4
#define EBT_STRAFE			8
#define EBT_MAP				0x10
#define EBT_INVENTORYLEFT 	0x20
#define EBT_INVENTORYRIGHT 	0x40
#define EBT_USEARTIFACT		0x80
#define EBT_FLYDROP			0x100
#define EBT_CENTERVIEW		0x200
#define EBT_PAUSE			0x400
#define EBT_WEAPONCYCLE		0x800
#define EBT_JUMP			0x1000

typedef struct
{
    short vector;               // Interrupt vector

    signed char moveForward;    // forward/backward (maxes at 50)
    signed char moveSideways;   // strafe (maxes at 24)
    short angleTurn;            // turning speed (640 [slow] 1280 [fast])
    short angleHead;            // head angle (+2080 [left] : 0 [center] : -2048 [right])
    signed char pitch;          // look up/down (-110 : +90)
    signed char flyDirection;   // flyheight (+1/-1)
    unsigned short buttons;     // EBT_* flags
} externdata_t;
#endif

//----
//GAME
//----

void G_DeathMatchSpawnPlayer(int playernum);

void G_InitNew(skill_t skill, int episode, int map);

void G_DeferedInitNew(skill_t skill, int episode, int map);
// can be called by the startup code or M_Responder
// a normal game starts at map 1, but a warp test can start elsewhere

void G_DeferredNewGame(skill_t skill);

void G_DoSelectiveGame(int direction);

void G_DeferedPlayDemo(char *demo);

void G_LoadGame(int slot);
// can be called by the startup code or M_Responder
// calls P_SetupLevel or W_EnterWorld
void G_DoLoadGame(void);

void G_SaveGame(int slot, char *description);
// called by M_Responder

void G_RecordDemo(skill_t skill, int numplayers, int episode, int map,
                  char *name);
// only called by startup code

void G_PlayDemo(char *name);
void G_TimeDemo(char *name);

void G_TeleportNewMap(int map, int position);

void G_Completed(int map, int position);
//void G_ExitLevel (void);
//void G_SecretExitLevel (void);

void G_StartNewGame(skill_t skill);
void G_StartNewInit(void);

void G_WorldDone(void);

void G_Ticker(void);
boolean G_Responder(event_t * ev);

void G_ScreenShot(void);

//-------
//SV_SAVE
//-------

#define HXS_VERSION_TEXT "HXS Ver 2.37"
#define HXS_VERSION_TEXT_LENGTH 16
#define HXS_DESCRIPTION_LENGTH 24

extern char *SavePath;

void SV_SaveGame(int slot, char *description);
void SV_SaveMap(boolean savePlayers);
void SV_LoadGame(int slot);
void SV_MapTeleport(int map, int position);
void SV_LoadMap(void);
void SV_ClearSaveSlot(int slot);
void SV_InitBaseSlot(void);
void SV_UpdateRebornSlot(void);
void SV_ClearRebornSlot(void);
boolean SV_RebornSlotAvailable(void);
int SV_GetRebornSlot(void);

//-----
//PLAY
//-----

void P_Ticker(void);
// called by C_Ticker
// can call G_PlayerExited
// carries out all thinking of monsters and players

void P_SetupLevel(int episode, int map, int playermask, skill_t skill);
// called by W_Ticker

void P_Init(void);
// called by startup code

int P_GetMapCluster(int map);
int P_TranslateMap(int map);
int P_GetMapCDTrack(int map);
int P_GetMapWarpTrans(int map);
int P_GetMapNextMap(int map);
int P_GetMapSky1Texture(int map);
int P_GetMapSky2Texture(int map);
char *P_GetMapName(int map);
fixed_t P_GetMapSky1ScrollDelta(int map);
fixed_t P_GetMapSky2ScrollDelta(int map);
boolean P_GetMapDoubleSky(int map);
boolean P_GetMapLightning(int map);
boolean P_GetMapFadeTable(int map);
char *P_GetMapSongLump(int map);
void P_PutMapSongLump(int map, char *lumpName);
int P_GetCDStartTrack(void);
int P_GetCDEnd1Track(void);
int P_GetCDEnd2Track(void);
int P_GetCDEnd3Track(void);
int P_GetCDIntermissionTrack(void);
int P_GetCDTitleTrack(void);

//-------
//REFRESH
//-------

extern boolean setsizeneeded;

extern boolean BorderNeedRefresh;
extern boolean BorderTopRefresh;

extern int UpdateState;
// define the different areas for the dirty map
#define I_NOUPDATE	0
#define I_FULLVIEW	1
#define I_STATBAR	2
#define I_MESSAGES	4
#define I_FULLSCRN	8

void R_RenderPlayerView(player_t * player);
// called by G_Drawer

void R_Init(void);
// called by startup code

void R_DrawViewBorder(void);
void R_DrawTopBorder(void);
// if the view size is not full screen, draws a border around it

void R_SetViewSize(int blocks, int detail);
// called by M_Responder

int R_FlatNumForName(char *name);

int R_TextureNumForName(char *name);
int R_CheckTextureNumForName(char *name);
// called by P_Ticker for switches and animations
// returns the texture number for the texture name


//----
//MISC
//----
extern int localQuakeHappening[MAXPLAYERS];

int M_DrawText(int x, int y, boolean direct, char *string);

//------------------------------
// SC_man.c
//------------------------------

void SC_Open(char *name);
void SC_OpenLump(char *name);
void SC_OpenFile(char *name);
void SC_Close(void);
boolean SC_GetString(void);
void SC_MustGetString(void);
void SC_MustGetStringName(char *name);
boolean SC_GetNumber(void);
void SC_MustGetNumber(void);
void SC_UnGet(void);
//boolean SC_Check(void);
boolean SC_Compare(char *text);
int SC_MatchString(char **strings);
int SC_MustMatchString(char **strings);
void SC_ScriptError(char *message);

extern char *sc_String;
extern int sc_Number;
extern int sc_Line;
extern boolean sc_End;
extern boolean sc_Crossed;
extern boolean sc_FileScripts;
extern char *sc_ScriptsDir;

//------------------------------
// SN_sonix.c
//------------------------------

enum
{
    SEQ_PLATFORM,
    SEQ_PLATFORM_HEAVY,         // same script as a normal platform
    SEQ_PLATFORM_METAL,
    SEQ_PLATFORM_CREAK,         // same script as a normal platform
    SEQ_PLATFORM_SILENCE,
    SEQ_PLATFORM_LAVA,
    SEQ_PLATFORM_WATER,
    SEQ_PLATFORM_ICE,
    SEQ_PLATFORM_EARTH,
    SEQ_PLATFORM_METAL2,
    SEQ_DOOR_STONE,
    SEQ_DOOR_HEAVY,
    SEQ_DOOR_METAL,
    SEQ_DOOR_CREAK,
    SEQ_DOOR_SILENCE,
    SEQ_DOOR_LAVA,
    SEQ_DOOR_WATER,
    SEQ_DOOR_ICE,
    SEQ_DOOR_EARTH,
    SEQ_DOOR_METAL2,
    SEQ_ESOUND_WIND,
    SEQ_NUMSEQ
};

typedef enum
{
    SEQTYPE_STONE,
    SEQTYPE_HEAVY,
    SEQTYPE_METAL,
    SEQTYPE_CREAK,
    SEQTYPE_SILENCE,
    SEQTYPE_LAVA,
    SEQTYPE_WATER,
    SEQTYPE_ICE,
    SEQTYPE_EARTH,
    SEQTYPE_METAL2,
    SEQTYPE_NUMSEQ
} seqtype_t;

void SN_InitSequenceScript(void);
void SN_StartSequence(mobj_t * mobj, int sequence);
void SN_StartSequenceName(mobj_t * mobj, char *name);
void SN_StopSequence(mobj_t * mobj);
void SN_UpdateActiveSequences(void);
void SN_StopAllSequences(void);
int SN_GetSequenceOffset(int sequence, int *sequencePtr);
void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume,
                       int currentSoundID);


typedef struct seqnode_s seqnode_t;
struct seqnode_s
{
    int *sequencePtr;
    int sequence;
    mobj_t *mobj;
    int currentSoundID;
    int delayTics;
    int volume;
    int stopSound;
    seqnode_t *prev;
    seqnode_t *next;
};

extern int ActiveSequences;
extern seqnode_t *SequenceListHead;

//----------------------
// Interlude (IN_lude.c)
//----------------------

#define MAX_INTRMSN_MESSAGE_SIZE 1024

extern boolean intermission;
extern char ClusterMessage[MAX_INTRMSN_MESSAGE_SIZE];

void IN_Start(void);
void IN_Ticker(void);
void IN_Drawer(void);

//----------------------
// Chat mode (CT_chat.c)
//----------------------

void CT_Init(void);
void CT_Drawer(void);
boolean CT_Responder(event_t * ev);
void CT_Ticker(void);
char CT_dequeueChatChar(void);

extern boolean chatmodeon;

//--------------------
// Finale (F_finale.c)
//--------------------

void F_Drawer(void);
void F_Ticker(void);
void F_StartFinale(void);

//----------------------
// STATUS BAR (SB_bar.c)
//----------------------

extern int inv_ptr;
extern int curpos;
void SB_Init(void);
void SB_SetClassData(void);
boolean SB_Responder(event_t * event);
void SB_Ticker(void);
void SB_Drawer(void);
void Draw_TeleportIcon(void);
void Draw_SaveIcon(void);
void Draw_LoadIcon(void);

// [JN] Crosshair stuff.
extern patch_t *CrosshairPatch;
extern byte    *CrosshairOpacity;
extern int  CrosshairShowcaseTimeout;
extern void Crosshair_DefinePatch (void);
extern void Crosshair_DefineOpacity (void);
extern void Crosshair_DefineDrawingFunc (void);
extern void Crosshair_Colorize_inMenu (void);
extern void Crosshair_Draw (void);

//-----------------
// MENU (MN_menu.c)
//-----------------

void MN_Init(void);
boolean MN_Responder(event_t * event);
void MN_Ticker(void);
void MN_Drawer(void);
void MN_DrTextAYellow(char *text, int x, int y);
void MN_DrTextAYellowFade(char *text, int x, int y, byte *table);
void MN_DrTextSmallYellowRUS(char *text, int x, int y);
void MN_DrTextSmallYellowRUSFade(char *text, int x, int y, byte *table);

extern int messageson;
