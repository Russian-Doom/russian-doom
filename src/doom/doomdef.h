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
// DESCRIPTION:
//  Internally used data structures for virtually everything,
//   lots of other stuff.
//


#pragma once

#include "doomtype.h"
#include "i_timer.h"
#include "d_mode.h"
#include "tables.h"
#include "info.h"
#include "net_defs.h"
#include "rd_text.h"


// -----------------------------------------------------------------------------
// Global parameters/defines.
// -----------------------------------------------------------------------------

// DOOM version
#define DOOM_VERSION 109

// Version code for cph's longtics hack ("v1.91")
#define DOOM_191_VERSION 111

// If rangecheck is undefined,
// most parameter validation debugging code will not be compiled
#define RANGECHECK

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS 4

typedef enum
{
    ga_nothing,
    ga_loadlevel,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_playdemo,
    ga_completed,
    ga_victory,
    ga_worlddone,
    ga_screenshot
} gameaction_t;

//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//
// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
//

enum
{
    ML_LABEL,       // A separator, name, ExMx or MAPxx
    ML_THINGS,      // Monsters, items..
    ML_LINEDEFS,    // LineDefs, from editing
    ML_SIDEDEFS,    // SideDefs, from editing
    ML_VERTEXES,    // Vertices, edited and BSP splits generated
    ML_SEGS,        // LineSegs, from LineDefs split by BSP
    ML_SSECTORS,    // SubSectors, list of LineSegs
    ML_NODES,       // BSP nodes
    ML_SECTORS,     // Sectors, from editing
    ML_REJECT,      // LUT, sector-sector visibility	
    ML_BLOCKMAP     // LUT, motion clipping, walls/grid element
};

// A single Vertex.
typedef struct
{
    short x;
    short y;
} PACKEDATTR mapvertex_t;

// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef struct
{
    short textureoffset;
    short rowoffset;
    char  toptexture[8];
    char  bottomtexture[8];
    char  midtexture[8];
    // Front sector, towards viewer.
    short sector;
} PACKEDATTR mapsidedef_t;

// A LineDef, as used for editing, and as input
// to the BSP builder.
typedef struct
{
    unsigned short v1;
    unsigned short v2;
    unsigned short flags;
    short special;
    short tag;
    // sidenum[1] will be -1 if one sided
    unsigned short sidenum[2];
} PACKEDATTR maplinedef_t;

// [crispy] allow loading of Hexen-format maps
// taken from chocolate-doom/src/hexen/xddefs.h:63-75
typedef struct
{
    short v1;
    short v2;
    short flags;
    byte special;
    byte arg1;
    byte arg2;
    byte arg3;
    byte arg4;
    byte arg5;
    short sidenum[2];
} PACKEDATTR maplinedef_hexen_t;

//
// LineDef attributes.
//

// Solid, is an obstacle.
#define ML_BLOCKING 1

// Blocks monsters only.
#define ML_BLOCKMONSTERS 2

// Backside will not be present at all if not two sided.
#define ML_TWOSIDED 4

// If a texture is pegged, the texture will have
// the end exposed to air held constant at the
// top or bottom of the texture (stairs or pulled
// down things) and will move with a height change
// of one of the neighbor sectors.
// Unpegged textures allways have the first row of
// the texture at the top pixel of the line for both
// top and bottom textures (use next to windows).

// upper texture unpegged
#define ML_DONTPEGTOP 8

// lower texture unpegged
#define ML_DONTPEGBOTTOM 16

// In AutoMap: don't map as two sided: IT'S A SECRET!
#define ML_SECRET 32

// Sound rendering: don't let sound cross two of these.
#define ML_SOUNDBLOCK 64

// Don't draw on the automap at all.
#define ML_DONTDRAW 128

// Set if already seen, thus drawn in automap.
#define ML_MAPPED 256


// Sector definition, from editing.
typedef	struct
{
    short floorheight;
    short ceilingheight;
    char  floorpic[8];
    char  ceilingpic[8];
    short lightlevel;
    short special;
    short tag;
} PACKEDATTR mapsector_t;


// SubSector, as generated by BSP.
typedef struct
{
    unsigned short numsegs;
    // Index of first one, segs are stored sequentially.
    unsigned short firstseg;
} PACKEDATTR mapsubsector_t;

// [crispy] allow loading of maps with DeePBSP nodes
// taken from prboom-plus/src/doomdata.h:163-166
typedef struct
{
    unsigned short numsegs;
    int firstseg;
} PACKEDATTR mapsubsector_deepbsp_t;

// [crispy] allow loading of maps with ZDBSP nodes
// taken from prboom-plus/src/doomdata.h:168-170
typedef struct {
    unsigned int numsegs;
} PACKEDATTR mapsubsector_zdbsp_t;

// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
typedef struct
{
    unsigned short v1;
    unsigned short v2;
    short angle;		
    unsigned short linedef;
    short side;
    short offset;
} PACKEDATTR mapseg_t;

// [crispy] allow loading of maps with DeePBSP nodes
// taken from prboom-plus/src/doomdata.h:183-190
typedef struct
{
    int v1;
    int v2;
    unsigned short angle;
    unsigned short linedef;
    short side;
    unsigned short offset;
} PACKEDATTR mapseg_deepbsp_t;

// [crispy] allow loading of maps with ZDBSP nodes
// taken from prboom-plus/src/doomdata.h:192-196
typedef struct {
    unsigned int v1, v2;
    unsigned short linedef;
    unsigned char side;
} PACKEDATTR mapseg_zdbsp_t;

// BSP node structure.

// Indicate a leaf.
#define NF_SUBSECTOR    0x80000000
#define NO_INDEX        ((unsigned short)-1)

typedef struct
{
    // Partition line from (x,y) to x+dx,y+dy)
    short x;
    short y;
    short dx;
    short dy;

    // Bounding box for each child,
    // clip against view frustum.
    short bbox[2][4];

    // If NF_SUBSECTOR its a subsector,
    // else it's a node of another subtree.
    unsigned short children[2];
} PACKEDATTR mapnode_t;

// [crispy] allow loading of maps with DeePBSP nodes
// taken from prboom-plus/src/doomdata.h:216-225
typedef struct
{
    short x;
    short y;
    short dx;
    short dy;
    short bbox[2][4];
    int children[2];
} PACKEDATTR mapnode_deepbsp_t;

// [crispy] allow loading of maps with ZDBSP nodes
// taken from prboom-plus/src/doomdata.h:227-136
typedef struct {
    short x;
    short y;
    short dx;
    short dy;
    short bbox[2][4];
    int children[2];
} PACKEDATTR mapnode_zdbsp_t;

// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.
typedef struct
{
    short x;
    short y;
    short angle;
    short type;
    short options;
} PACKEDATTR mapthing_t;

// [crispy] allow loading of Hexen-format maps
// taken from chocolate-doom/src/hexen/xddefs.h:134-149
typedef struct
{
    short tid;
    short x;
    short y;
    short height;
    short angle;
    short type;
    short options;
    byte special;
    byte arg1;
    byte arg2;
    byte arg3;
    byte arg4;
    byte arg5;
} PACKEDATTR mapthing_hexen_t;

// -----------------------------------------------------------------------------
//
// PLAYER SPRITE DEFS
//
// -----------------------------------------------------------------------------

// Overlay psprites are scaled shapes drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
typedef enum
{
    ps_weapon,
    ps_flash,
    NUMPSPRITES

} psprnum_t;

typedef struct
{
    state_t *state;	// a NULL state means not active
    int      tics;
    fixed_t  sx;
    fixed_t  sy;

} pspdef_t;

// -----------------------------------------------------------------------------
// P_MOBJ
// -----------------------------------------------------------------------------

//
// Misc. mobj flags
//

typedef enum
{
    // Call P_SpecialThing when touched.
    MF_SPECIAL      = 1,
    // Blocks.
    MF_SOLID        = 2,
    // Can be hit.
    MF_SHOOTABLE    = 4,
    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR     = 8,
    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP   = 16,                    
    // Not to be activated by sound, deaf monster.
    MF_AMBUSH       = 32,
    // Will try to attack right back.
    MF_JUSTHIT      = 64,
    // Will take at least one step before attacking.
    MF_JUSTATTACKED = 128,
    // On level spawning (initial position),
    //  hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING = 256,
    // Don't apply gravity (every tic), that is, 
    // object will float, keeping current height or changing it actively.
    MF_NOGRAVITY    = 512,

    // Movement flags.

    // This allows jumps from high places.
    MF_DROPOFF      = 0x400,
    // For players, will pick up items.
    MF_PICKUP       = 0x800,
    // Player cheat. ???
    MF_NOCLIP       = 0x1000,
    // Player: keep info about sliding along walls.
    MF_SLIDE        = 0x2000,
    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT        = 0x4000,
    // Don't cross lines ??? or look at heights on teleport.
    MF_TELEPORT     = 0x8000,
    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE      = 0x10000,	
    // Dropped by a demon, not level spawned.
    // E.g. ammo clips dropped by dying former humans.
    MF_DROPPED      = 0x20000,
    // Use fuzzy draw (shadow demons or spectres),
    //  temporary player invisibility powerup.
    MF_SHADOW       = 0x40000,
    // Flag: don't bleed when shot (use puff),
    //  barrels and shootable furniture shall not bleed.
    MF_NOBLOOD      = 0x80000,
    // Don't stop moving halfway off a step,
    //  that is, have dead bodies slide down all the way.
    MF_CORPSE       = 0x100000,
    // Floating to a height for a move, ???
    //  don't auto float to target's height.
    MF_INFLOAT      = 0x200000,

    // On kill, count this enemy object
    //  towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL    = 0x400000,

    // On picking up, count this item object
    //  towards intermission item total.
    MF_COUNTITEM    = 0x800000,

    // Special handling: skull in flight.
    // Neither a cacodemon nor a missile.
    MF_SKULLFLY     = 0x1000000,

    // Don't spawn this object in death match mode (e.g. key cards).
    MF_NOTDMATCH    = 0x2000000,

    // Player sprites in multiplayer modes are modified
    //  using an internal color lookup table for re-indexing.
    // If 0x4 0x8 or 0xc,
    //  use a translation table for player colormaps
    MF_TRANSLATION  = 0xc000000,
    // Hmm ???.
    MF_TRANSSHIFT   = 26,

    // [crispy] Translucent sprite
    MF_TRANSLUCENT  = 0x80000000,

    // [JN] Resurrected monster is counted by extra counter.
    MF_COUNTEXTRAKILL = 0x160000000

} mobjflag_t;

// killough 9/15/98: Same, but internal flags, not intended for .deh
// (some degree of opaqueness is good, to avoid compatibility woes)
enum 
{
    MIF_FALLING = 1,      // Object is falling
    MIF_ARMED = 2,        // Object is armed (for MF_TOUCHY objects)
    MIF_LINEDONE = 4,     // Object has activated W1 or S1 linedef via DEH frame
};

//
// Map Object definition.
//

typedef struct mobj_s
{
    // List: thinker links.
    thinker_t   thinker;

    // Info for drawing: position.
    fixed_t     x;
    fixed_t     y;
    fixed_t     z;

    // More list: links in sector (if needed)
    struct mobj_s *snext;
    struct mobj_s *sprev;

    //More drawing info: to determine current sprite.
    angle_t     angle;  // orientation
    spritenum_t sprite; // used to find patch_t and flip value
    int         frame;  // might be ORed with FF_FULLBRIGHT

    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
    struct mobj_s      *bnext;
    struct mobj_s      *bprev;
    struct subsector_s *subsector;

    // The closest interval over all contacted Sectors.
    fixed_t floorz;
    fixed_t ceilingz;

    // killough 11/98: the lowest floor over all contacted Sectors.
    fixed_t dropoffz;

    // For movement checking.
    fixed_t radius;
    fixed_t height;	

    // Momentums, used to update position.
    fixed_t momx;
    fixed_t momy;
    fixed_t momz;

    // If == validcount, already checked.
    int validcount;

    mobjtype_t  type;
    mobjinfo_t *info;   // &mobjinfo[mobj->type]
    
    int       tics;     // state tic counter
    state_t  *state;
    int       flags;
    int       intflags; // killough 9/15/98: internal flags
    int       health;

    // Movement direction, movement generation (zig-zagging).
    int movedir;    // 0-7
    int movecount;  // when 0, select a new dir

    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
    struct mobj_s *target;

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    int reactiontime;   

    // If >0, the target will be chased
    // no matter what (even if shot)
    int threshold;

    short gear;     // killough 11/98: used in torque simulation
    int   geartics; // [JN] Duration of torque sumulation.

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    struct player_s *player;

    // Player number last looked for.
    int lastlook;	

    // For nightmare respawn.
    mapthing_t spawnpoint;	

    // Thing being chased/attacked for tracers.
    struct mobj_s *tracer;	

    // [AM] If true, ok to interpolate this tic.
    boolean interp;

    // [AM] Previous position of mobj before think.
    //      Used to interpolate between positions.
    fixed_t oldx;
    fixed_t oldy;
    fixed_t oldz;
    angle_t oldangle;

    // [JN] Light level of brightmap animation.
    int     brightmap_anim;

} mobj_t;


#define MTF_EASY    1
#define MTF_NORMAL  2
#define MTF_HARD    4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH  8

// [JN] "Muliplayer only" flag.
#define MTF_NETGAME 16


//
// Key cards.
//
typedef enum
{
    it_bluecard,
    it_yellowcard,
    it_redcard,
    it_blueskull,
    it_yellowskull,
    it_redskull,
    NUMCARDS
} card_t;


// The defined weapons,
//  including a marker indicating
//  user has not changed weapon.
typedef enum
{
    wp_fist,
    wp_pistol,
    wp_shotgun,
    wp_chaingun,
    wp_missile,
    wp_plasma,
    wp_bfg,
    wp_chainsaw,
    wp_supershotgun,
    NUMWEAPONS,

    // No pending weapon change.
    wp_nochange
} weapontype_t;


// Ammunition types defined.
typedef enum
{
    am_clip,    // Pistol / chaingun ammo.
    am_shell,   // Shotgun / double barreled shotgun.
    am_cell,    // Plasma rifle, BFG.
    am_misl,    // Missile launcher.
    NUMAMMO,
    am_noammo   // Unlimited for chainsaw / fist.	
} ammotype_t;


// Power up artifacts.
typedef enum
{
    pw_invulnerability,
    pw_strength,
    pw_invisibility,
    pw_ironfeet,
    pw_allmap,
    pw_infrared,
    NUMPOWERS
} powertype_t;


//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
typedef enum
{
    INVULNTICS  = (30 * TICRATE),
    INVISTICS   = (60 * TICRATE),
    INFRATICS   = (120 * TICRATE),
    IRONTICS    = (60 * TICRATE)
} powerduration_t;

// -----------------------------------------------------------------------------
//
//     D_PLAYER
//
// -----------------------------------------------------------------------------

//
// Player states.
//
typedef enum
{
    PST_LIVE, // Playing or camping.
    PST_DEAD, // Dead on the ground, view follows killer.
    PST_REBORN // Ready to restart/respawn???
} playerstate_t;


//
// Player internal flags, for cheats and debug.
//
typedef enum
{
    CF_NOCLIP		= 1, // No clipping, walk through barriers.
    CF_GODMODE		= 2, // No damage, no health loss.
    CF_NOMOMENTUM	= 4  // Not really a cheat, just a debug aid.
} cheat_t;

//
// Extended player object info: player_t
//

typedef struct player_s
{
    mobj_t        *mo;
    ticcmd_t       cmd;
    playerstate_t  playerstate;

    // Determine POV, including viewpoint bobbing during movement.
    fixed_t viewz;              // Focal origin above r.z
    fixed_t viewheight;         // Base height above floor for viewz.
    fixed_t deltaviewheight;    // Bob/squat speed.
    fixed_t bob;                // bounded/scaled total momentum.

    // This is only used between levels, mo->health is used during levels.
    int health;	
    int armorpoints;
    int armortype;	            // Armor type is 0-2.

    // Power ups. invinc and invis are tic counters.
    int     powers[NUMPOWERS];
    boolean cards[NUMCARDS];
    boolean tryopen[NUMCARDS];  // [crispy] blinking key or skull in the status bar
    boolean backpack;

    // Frags, kills of other players.
    int          frags[MAXPLAYERS];
    weapontype_t readyweapon;

    // Is wp_nochange if not changing.
    weapontype_t pendingweapon;

    int weaponowned[NUMWEAPONS];
    int ammo[NUMAMMO];
    int maxammo[NUMAMMO];

    // True if button down last tic.
    int attackdown;
    int usedown;

    // Bit flags, for cheats and debug.
    // See cheat_t, above.
    int cheats;		

    // Refired shots are less accurate.
    int refire;		

     // For intermission stats.
    int killcount;
    int extrakillcount;  // [JN] Resurrected monsters counter.
    int itemcount;
    int secretcount;

    // Hint messages.
    char *message;
    int   messageTics;
    MessageType_t messageType;

    // For screen flashing (red or bright).
    int damagecount;
    int bonuscount;

    // Who did damage (NULL for floors/ceilings).
    mobj_t *attacker;

    // So gun flashes light up areas.
    int extralight;

    // Current PLAYPAL, ??? can be set to REDCOLORMAP for pain, etc.
    int fixedcolormap;

    // Player skin colorshift, 0-3 for which color to draw player.
    int colormap;	

    // Overlay view sprites (gun, etc).
    pspdef_t psprites[NUMPSPRITES];

    // True if secret level has been done.
    boolean didsecret;	
    
    // [AM] Previous position of viewz before think.
    //      Used to interpolate between camera positions.
    angle_t		oldviewz;

    // [crispy] squat down weapon sprite a bit after hitting the ground
    fixed_t	psp_dy, psp_dy_max;

    int     lookdir, oldlookdir;
    boolean centering;

} player_t;


//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//

typedef struct
{
    boolean	in;         // whether the player is in game
    int skills;         // Player stats, kills, collected items etc:
    int sextrakills;    // [JN] Resurrected monsters counter.
    int sitems;
    int ssecret;
    int stime; 
    int frags[4];
    int score;          // current score on entry, modified on return
} wbplayerstruct_t;

typedef struct
{
    int epsd;           // episode # (0-2)
    boolean didsecret;  // if true, splash the secret level
    int last;           // previous and next levels, origin 0
    int next;	
    int maxkills;
    int maxitems;
    int maxsecret;
    int maxfrags;
    int partime;        // the par time
    int pnum;           // index of this player in game
    wbplayerstruct_t plyr[MAXPLAYERS];
    int totaltimes;     // [crispy] CPhipps - total game time for completed levels so far
} wbstartstruct_t;
