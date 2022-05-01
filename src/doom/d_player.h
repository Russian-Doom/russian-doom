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
//
//


#pragma once


// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "d_items.h"
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

#include "net_defs.h"
#include "rd_text.h"


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
    boolean	in; // whether the player is in game

    // Player stats, kills, collected items etc.
    int skills;
    int sextrakills;  // [JN] Resurrected monsters counter.
    int sitems;
    int ssecret;
    int stime; 
    int frags[4];
    int score; // current score on entry, modified on return
} wbplayerstruct_t;


typedef struct
{
    int epsd;   // episode # (0-2)

    // if true, splash the secret level
    boolean didsecret;

    // previous and next levels, origin 0
    int last;
    int next;	

    int maxkills;
    int maxitems;
    int maxsecret;
    int maxfrags;

    // the par time
    int partime;

    // index of this player in game
    int pnum;	

    wbplayerstruct_t plyr[MAXPLAYERS];

    // [crispy] CPhipps - total game time for completed levels so far
    int totaltimes;
} wbstartstruct_t;
