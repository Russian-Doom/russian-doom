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
//



#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "doomdef.h"
#include "doomstat.h"
#include "deh_main.h"
#include "deh_misc.h"
#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_input.h"
#include "p_saveg.h"
#include "d_main.h"
#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "v_video.h"
#include "w_wad.h"
#include "p_local.h" 
#include "s_sound.h"
#include "rd_keybinds.h"
#include "rd_lang.h"
#include "sounds.h"
#include "r_data.h"
#include "g_game.h"
#include "jn.h"

#define MAXPLMOVE       (forwardmove[1]) 
#define TURBOTHRESHOLD  0x32
#define SLOWTURNTICS    6
#define	BODYQUESIZE	32


void    G_ReadDemoTiccmd (ticcmd_t *cmd);
void    G_WriteDemoTiccmd (ticcmd_t *cmd);
void    G_PlayerReborn (int player);
void    G_DoReborn (int playernum);
void    G_DoLoadLevel (void);
void    G_DoNewGame (void);
void    G_DoPlayDemo (void);
void    G_DoCompleted (void);
void    G_DoVictory (void);
void    G_DoWorldDone (void);
void    G_DoSaveGame (void);
void    P_SpawnPlayer (mapthing_t *mthing); 

// Gamestate the last time G_Ticker was called.

gamestate_t     oldgamestate;
 
gameaction_t    gameaction;
gamestate_t     gamestate;
skill_t         gameskill;
boolean         respawnmonsters;
int             gameepisode;
int             gamemap;

// If non-zero, exit the level after this number of minutes.
int             timelimit;

boolean         paused;
boolean         sendpause;  // send a pause event next tic
boolean         sendsave;   // send a save event next tic
boolean         usergame;   // ok to save / end game

boolean         timingdemo; // if true, exit with report on completion
boolean         nodrawers;  // for comparative timing purposes
int             starttime;  // for comparative timing purposes
int             alwaysRun = 1;  // is always run enabled

boolean         viewactive;

int             deathmatch; // only if started as net death
boolean         netgame;    // only true if packets are broadcast
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];

boolean         turbodetected[MAXPLAYERS];

int             consoleplayer;      // player taking events and displaying 
int             displayplayer;      // view being displayed 
int             levelstarttic;      // gametic at level start 
int             totalkills, totalitems, totalsecret;    // for intermission
int             totalleveltimes;    // [crispy] CPhipps - total time for all completed levels
 
// [JN] Press Beta: value for player's artifacts and lifes.
// Note that these are "local" values, not placed into the player's structure -
// no need to do this, since Press Beta is single player only.
int             artifactcount;
int             lifecount;

char           *demoname;
boolean         demorecording;
boolean         longtics;           // cph's doom 1.91 longtics hack
boolean         lowres_turn;        // low resolution turning for longtics
boolean         demoplayback;
boolean         netdemo;
byte           *demobuffer;
byte           *demo_p;
byte           *demoend;
boolean         singledemo;         // quit after playing a demo from cmdline 
 
boolean         precache = true;    // if true, load all graphics at start 

boolean         testcontrols = false;   // Invoked by setup to test controls
int             testcontrols_mousespeed;

wbstartstruct_t wminfo;         // parms for world map / intermission 
 
byte            consistancy[MAXPLAYERS][BACKUPTICS]; 
 
fixed_t         forwardmove[2] = {0x19, 0x32}; 
fixed_t         sidemove[2] = {0x18, 0x28}; 
fixed_t         angleturn[3] = {640, 1280, 320};    // + slow turn 

// [JN] If true, activate maximum weapon bobbing
boolean max_bobbing;

boolean         secretexit; 
extern char    *pagename; 

static bound_key_t weapon_keys[] = {
    bk_weapon_1,
    bk_weapon_2,
    bk_weapon_3,
    bk_weapon_4,
    bk_weapon_5,
    bk_weapon_6,
    bk_weapon_7,
    bk_weapon_8
};

// Set to -1 or +1 to switch to the previous or next weapon.

static int next_weapon = 0;

// Used for prev/next weapon keys.

static const struct
{
    weapontype_t weapon;
    weapontype_t weapon_num;
} weapon_order_table[] = {
    { wp_fist,            wp_fist },
    { wp_chainsaw,        wp_fist },
    { wp_pistol,          wp_pistol },
    { wp_shotgun,         wp_shotgun },
    { wp_supershotgun,    wp_shotgun },
    { wp_chaingun,        wp_chaingun },
    { wp_missile,         wp_missile },
    { wp_plasma,          wp_plasma },
    { wp_bfg,             wp_bfg }
};

static int      turnheld;   // for accelerative turning

// mouse values are used once 
int             mousex;
int             mousey;

// joystick values are repeated 
static int      joyturn;
static int      joymove;
static int      joystrafemove;
static int      joyvlook;
 
static int      savegameslot; 
static char     savedescription[32]; 
 
mobj_t*         bodyque[BODYQUESIZE]; 
int             bodyqueslot; 

 
int G_CmdChecksum (ticcmd_t* cmd) 
{ 
    size_t  i;
    int     sum = 0; 

    for (i=0 ; i < sizeof(*cmd)/4 - 1 ; i++)
    {
        sum += ((int *)cmd)[i];
    }

    return sum; 
}

static boolean WeaponSelectable(weapontype_t weapon)
{
    // Can't select the super shotgun in Doom 1.
    // [JN] Also can't select in Jaguar Doom.

    if (weapon == wp_supershotgun && 
    (logical_gamemission == doom || gamemission == jaguar))
    {
        return false;
    }

    // These weapons aren't available in shareware.

    if ((weapon == wp_plasma || weapon == wp_bfg)
    && gamemission == doom && gamemode == shareware)
    {
        return false;
    }

    // Can't select a weapon if we don't own it.

    if (!players[consoleplayer].weaponowned[weapon])
    {
        return false;
    }

    // Can't select the fist if we have the chainsaw, unless
    // we also have the berserk pack.

    if (weapon == wp_fist
    && players[consoleplayer].weaponowned[wp_chainsaw]
    && !players[consoleplayer].powers[pw_strength])
    {
        return (singleplayer);  // [crispy] yes, we can
    }

    return true;
}


static int G_NextWeapon(int direction)
{
    int start_i, i;
    weapontype_t weapon;

    // Find index in the table.

    if (players[consoleplayer].pendingweapon == wp_nochange)
    {
        weapon = players[consoleplayer].readyweapon;
    }
    else
    {
        weapon = players[consoleplayer].pendingweapon;
    }

    for (i=0; i<arrlen(weapon_order_table); ++i)
    {
        if (weapon_order_table[i].weapon == weapon)
        {
            break;
        }
    }

    // Switch weapon. Don't loop forever.
    start_i = i;
    do
    {
        i += direction;
        i = (i + arrlen(weapon_order_table)) % arrlen(weapon_order_table);
    } while (i != start_i && !WeaponSelectable(weapon_order_table[i].weapon));

    return weapon_order_table[i].weapon_num;
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer. 
// If recording a demo, write it out 
// 
void G_BuildTiccmd (ticcmd_t* cmd, int maketic) 
{ 
    int         i; 
    int         speed;
    int         tspeed; 
    int         forward;
    int         side;
    int         look;
    boolean     strafe;

    memset(cmd, 0, sizeof(ticcmd_t));

    cmd->consistancy = consistancy[consoleplayer][maketic%BACKUPTICS]; 

    strafe = BK_isKeyPressed(bk_strafe);
	
    // [crispy] when "always run" is active,
    // pressing the "run" key will result in walking
    speed = alwaysRun;
	
    // [JN] Speed key modifier
    speed ^= BK_isKeyPressed(bk_speed);
 
    forward = side = look = 0;

    // use two stage accelerative turning on the keyboard
    if(BK_isKeyPressed(bk_turn_right)
    || BK_isKeyPressed(bk_turn_left))
    {
        turnheld += ticdup;
    }
    else
    {
        turnheld = 0;
    }

    if (turnheld < SLOWTURNTICS)
    {
        tspeed = 2; // slow turn
    }
    else
    {
        tspeed = speed;
    }

    // [crispy] toggle always run
    if (BK_isKeyPressed(bk_toggle_autorun))
    {
        alwaysRun ^= 1;

        players[consoleplayer].message_system = alwaysRun ?  ststr_alwrun_on : ststr_alwrun_off;
        S_StartSound(NULL,sfx_swtchn);

        BK_ReleaseKey(bk_toggle_autorun);
    }

    // let movement keys cancel each other out
    if (strafe)
    {
        if (BK_isKeyPressed(bk_turn_right))
        {
            side += sidemove[speed];
        }
        if (BK_isKeyPressed(bk_turn_left))
        {
            side -= sidemove[speed];
        }
        if (joyturn != 0)
        {
            side += FixedMul(sidemove[speed], joyturn);
        }
        if(mousex != 0)
        {
            side += mousex*2;
        }
    }
    else 
    { 
        if (BK_isKeyPressed(bk_turn_right))
        {
            cmd->angleturn -= angleturn[tspeed];
        }
        if (BK_isKeyPressed(bk_turn_left))
        {
            cmd->angleturn += angleturn[tspeed];
        }
        if (joyturn != 0)
        {
            cmd->angleturn -= FixedMul(angleturn[1], joyturn);
        }
        if(mousex != 0)
        {
            cmd->angleturn -= mousex*0x8;
        }
    }

    if (BK_isKeyPressed(bk_forward))
    {
        forward += forwardmove[speed]; 
    }
    if (BK_isKeyPressed(bk_backward))
    {
        forward -= forwardmove[speed]; 
    }

    if (joymove != 0)
    {
        forward += FixedMul(forwardmove[speed], joymove);
    }

    if (BK_isKeyPressed(bk_strafe_left))
    {
        side -= sidemove[speed];
    }

    if (BK_isKeyPressed(bk_strafe_right))
    {
        side += sidemove[speed];
    }

    if (joystrafemove != 0)
    {
        side += FixedMul(sidemove[speed], joystrafemove);
    }

    // buttons
    cmd->chatchar = HU_dequeueChatChar(); 

    if (BK_isKeyPressed(bk_fire))
    {
        cmd->buttons |= BT_ATTACK;
    }

    if (BK_isKeyPressed(bk_use))
    {
        cmd->buttons |= BT_USE;
    }

    // If the previous or next weapon button is pressed, the
    // next_weapon variable is set to change weapons when
    // we generate a ticcmd.  Choose a new weapon.

    if (gamestate == GS_LEVEL && next_weapon != 0)
    {
        i = G_NextWeapon(next_weapon);
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= i << BT_WEAPONSHIFT;
    }
    else
    {
        // Check weapon keys.

        for (i = 0 ; i < arrlen(weapon_keys) ; ++i)
        {
            if (BK_isKeyPressed(weapon_keys[i]))
            {
                cmd->buttons |= BT_CHANGE;
                cmd->buttons |= i<<BT_WEAPONSHIFT;
                break;
            }
        }
    }

    next_weapon = 0;

    if (mousex == 0)
    {
        // No movement in the previous frame
        testcontrols_mousespeed = 0;
    }

    // [JN] Mouselook: toggling
    if (BK_isKeyPressed(bk_toggle_mlook))
    {
        if (!mlook)
        {
            mlook = true;
        }
        else
        {
            mlook = false;
            look = TOCENTER;
        }

        players[consoleplayer].message_system = mlook ? ststr_mlook_on : ststr_mlook_off;
        S_StartSound(NULL, sfx_swtchn);

        BK_ReleaseKey(bk_toggle_mlook);
    }

    // [JN] Mouselook: handling
    if (!demoplayback && players[consoleplayer].playerstate == PST_LIVE && !paused && !menuactive)
    {
        if (mlook)
        {
            cmd->lookdir += mouse_y_invert ? -mousey : mousey;
            cmd->lookdir += FixedMul(angleturn[2], joyvlook);
        }
        else if (!novert)
        {
            forward += mousey;
            forward += FixedMul(forwardmove[speed], joyvlook);
        }
        
        if (players[consoleplayer].lookdir > LOOKDIRMAX * MLOOKUNIT)
        {
            players[consoleplayer].lookdir = LOOKDIRMAX * MLOOKUNIT;
        }
        else if (players[consoleplayer].lookdir < -LOOKDIRMIN * MLOOKUNIT)
        {
            players[consoleplayer].lookdir = -LOOKDIRMIN * MLOOKUNIT;
        }
        
        if (look < 0)
        {
            look += 16;
        }

        cmd->lookfly = look;
    }

    mousex = mousey = joyturn = joyvlook = 0;

    // [JN] "false" must be set as initial and returning condition.
    max_bobbing = false;

    if (forward > MAXPLMOVE)
    {
        forward = MAXPLMOVE;
        if (novert)
        max_bobbing = true;
    }
    else if (forward < -MAXPLMOVE)
    {
        forward = -MAXPLMOVE;
        if (novert)
        max_bobbing = false;
    }
    if (side > MAXPLMOVE)
    {
        side = MAXPLMOVE;
        if (novert)
        max_bobbing = true;
    }
    else if (side < -MAXPLMOVE)
    {
        side = -MAXPLMOVE;
        if (novert)
        max_bobbing = false;
    }

    cmd->forwardmove += forward;
    cmd->sidemove += side;

    // [crispy] lookdir delta is stored in the lower 4 bits of the lookfly variable
    if (players[consoleplayer].playerstate == PST_LIVE)
    {
        if (look < 0)
        {
            look += 16;
        }
        cmd->lookfly = look;
    }

    // special buttons
    if (sendpause)
    {
        sendpause = false;
        cmd->buttons = BT_SPECIAL | BTS_PAUSE;
    }

    if (sendsave)
    {
        sendsave = false;
        cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT);
    }

    if (flip_levels)
    {
        cmd->angleturn = -cmd->angleturn;
        cmd->sidemove = -cmd->sidemove;
    }

    // low-res turning

    if (lowres_turn)
    {
        static signed short carry = 0;
        signed short desired_angleturn;

        desired_angleturn = cmd->angleturn + carry;

        // round angleturn to the nearest 256 unit boundary
        // for recording demos with single byte values for turn

        cmd->angleturn = (desired_angleturn + 128) & 0xff00;

        // Carry forward the error from the reduced resolution to the
        // next tic, so that successive small movements can accumulate.

        carry = desired_angleturn - cmd->angleturn;
    }
} 
 

//
// G_DoLoadLevel 
//
void G_DoLoadLevel (void) 
{ 
    int i; 

    // [JN] Properly remove paused state and resume music playing.
    // Fixes a bug when pausing intermission screen causes locking up sound.
    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

    // Set the sky map.
    // First thing, we have a dummy sky texture name,
    //  a flat. The data is in the WAD only because
    //  we look for an actual index, instead of simply
    //  setting one.

    skyflatnum = R_FlatNumForName(DEH_String(SKYFLATNAME));

    // The "Sky never changes in Doom II" bug was fixed in
    // the id Anthology version of doom2.exe for Final Doom.
    // [JN] Fixed "Sky never changes" bug
    if (gamemode == commercial)
    {
        char *skytexturename;

        // [JN] Jaguar Doom sky handling
        if (gamemission == jaguar)
        {
            if (gamemap < 9 || gamemap == 24)
            {
                skytexturename = "SKY1";
            }
            else if (gamemap < 17 || gamemap == 25)
            {
                skytexturename = "SKY2";
            }
            else
            {
                skytexturename = "SKY3";
            }
        }
        else if (gamemission == pack_nerve)
        {
            if (gamemap < 4 || gamemap == 9)
            {
                skytexturename = ("SKY1");
            }
            else
            {
                skytexturename = ("SKY3");
            }
        }
        else
        {
            if (gamemap < 12)
            {
                skytexturename = "SKY1";
            }
            else if (gamemap < 21)
            {
                skytexturename = "SKY2";
            }
            else
            {
                skytexturename = "SKY3";
            }
        }

        skytexturename = DEH_String(skytexturename);

        skytexture = R_TextureNumForName(skytexturename);
    }

    levelstarttic = gametic;    // for time calculation
    
    if (wipegamestate == GS_LEVEL) 
    {
        wipegamestate = -1;     // force a wipe 
    }

    gamestate = GS_LEVEL; 

    for (i=0 ; i < MAXPLAYERS ; i++) 
    { 
        turbodetected[i] = false;
        if (playeringame[i] && players[i].playerstate == PST_DEAD) 
        {
            players[i].playerstate = PST_REBORN; 
        }
        memset (players[i].frags,0,sizeof(players[i].frags)); 
    } 

    // [JN] Pistol start
    if (singleplayer && !vanillaparm && pistol_start)
    {
        G_PlayerReborn(0);
    }

    P_SetupLevel (gameepisode, gamemap, 0, gameskill);    
    displayplayer = consoleplayer;		// view the guy you are playing    
    gameaction = ga_nothing; 
    Z_CheckHeap ();

    // clear cmd building stuff

    BK_ReleaseAllKeys();
    joyvlook = joyturn = joymove = joystrafemove = 0;
    mousex = mousey = 0;
    sendpause = sendsave = paused = false;

    if (testcontrols)
    {
        players[consoleplayer].message_system = ststr_testctrls;
    }
}

//
// G_Responder  
// Get info needed to make ticcmd_ts for the players.
// 
boolean G_Responder (event_t *ev) 
{ 
    // allow spy mode changes even during the demo
    if (gamestate == GS_LEVEL && BK_isKeyDown(ev, bk_spy) && (singledemo || !deathmatch))
    {
        // spy mode 
        do 
        { 
            displayplayer++; 
            if (displayplayer == MAXPLAYERS) 
            {
                displayplayer = 0; 
            }
        } while (!playeringame[displayplayer] && displayplayer != consoleplayer); 
        return true; 
    }

    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && !singledemo && (demoplayback || gamestate == GS_DEMOSCREEN)) 
    { 
        if (ev->type == ev_keydown
        ||  ev->type == ev_mouse_keydown
        ||  ev->type == ev_controller_keydown)
        { 
            RD_Menu_ActivateMenu();
            return true; 
        } 
        return false; 
    } 

    if (gamestate == GS_LEVEL) 
    { 
        if (HU_Responder (ev)) 
            return true;	// chat ate the event
        if (ST_Responder (ev)) 
            return true;	// status window ate it
        if (AM_Responder (ev)) 
            return true;	// automap ate it
    } 

    if (gamestate == GS_FINALE) 
    { 
        if (F_Responder (ev)) 
            return true;	// finale ate the event
    } 

    if (testcontrols && ev->type == ev_mouse_move)
    {
        // If we are invoked by setup to test the controls, save the 
        // mouse speed so that we can display it on-screen.
        // Perform a low pass filter on this so that the thermometer 
        // appears to move smoothly.

        testcontrols_mousespeed = abs(ev->data2);
    }

    // If the next/previous weapon keys are pressed, set the next_weapon
    // variable to change weapons when the next ticcmd is generated.

    if (BK_isKeyDown(ev, bk_weapon_prev))
    {
        next_weapon = -1;
    }
    else if (BK_isKeyDown(ev, bk_weapon_next))
    {
        next_weapon = 1;
    }

    if (BK_isKeyDown(ev, bk_pause))
    {
        sendpause = true;
    }

    switch (ev->type)
    { 
        case ev_keydown:
        case ev_mouse_keydown:
        case ev_controller_keydown:
            BK_ProcessKey(ev);
            return true;    // eat key down events 

        case ev_keyup:
        case ev_mouse_keyup:
        case ev_controller_keyup:
            BK_ProcessKey(ev);
            return false;   // always let key up events filter down

        case ev_mouse_move:
            mousex = ev->data2*(mouseSensitivity+5)/10; 
            mousey = ev->data3*(mouseSensitivity+5)/10; 
            return true;    // eat events 

        case ev_controller_move:
            joymove = ev->data1;
            joystrafemove = ev->data2;
            joyturn = ev->data3;
            joyvlook = ev->data4;
            return true;    // eat events

        default: 
            break; 
    } 

    return false; 
}


//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void) 
{ 
    int        i;
    int        buf; 
    ticcmd_t  *cmd;

    // do player reborns if needed
    for (i = 0 ; i < MAXPLAYERS ; i++) 
        if (playeringame[i] && players[i].playerstate == PST_REBORN) 
        G_DoReborn (i);

    // do things to change the game state
    while (gameaction != ga_nothing) 
    { 
        switch (gameaction) 
        { 
            case ga_loadlevel:
            G_DoLoadLevel ();
            break;

            case ga_newgame:
            G_DoNewGame ();
            break;

            case ga_loadgame:
            G_DoLoadGame ();
            // [JN] Reset looking direction if game is loaded without mouse look
            if (!mlook)
            players[consoleplayer].lookdir = 0;
            break;

            case ga_savegame:
            G_DoSaveGame ();
            break;

            case ga_playdemo:
            G_DoPlayDemo ();
            break;

            case ga_completed:
            G_DoCompleted ();
            break;

            case ga_victory:
            F_StartFinale ();
            break;

            case ga_worlddone:
            G_DoWorldDone ();
            break; 

            case ga_screenshot: // [JN] Extended name from "DOOM%02i.%s"
            V_ScreenShot("screenshot-doom-%02i.%s");
            if (devparm)
            {
                players[consoleplayer].message_system = ststr_scrnsht;
            }
            S_StartSound(NULL,sfx_itemup); // [JN] Audible feedback
            gameaction = ga_nothing;
            break;

            case ga_nothing: 
            break; 
        } 
    }

    // get commands, check consistancy,
    // and build new consistancy check
    buf = (gametic/ticdup)%BACKUPTICS; 

    for (i = 0 ; i < MAXPLAYERS ; i++)
    {
        if (playeringame[i]) 
        { 
            cmd = &players[i].cmd; 

            memcpy(cmd, &netcmds[i], sizeof(ticcmd_t));

            if (demoplayback) 
            G_ReadDemoTiccmd (cmd); 
            if (demorecording) 
            G_WriteDemoTiccmd (cmd);
	    
            // check for turbo cheats

            // check ~ 4 seconds whether to display the turbo message. 
            // store if the turbo threshold was exceeded in any tics
            // over the past 4 seconds.  offset the checking period
            // for each player so messages are not displayed at the
            // same time.

            if (cmd->forwardmove > TURBOTHRESHOLD)
            {
                turbodetected[i] = true;
            }

            if ((gametic & 31) == 0 
            && ((gametic >> 5) % MAXPLAYERS) == i
            && turbodetected[i])
            {
                static char turbomessage[80];
                extern char *player_names[4];

                // [JN] Untranslated, since netgame chat using only English.
                M_snprintf(turbomessage, sizeof(turbomessage),
                           "%s is turbo!", player_names[i]);

                players[consoleplayer].message_chat = turbomessage;
                turbodetected[i] = false;
            }

            if (netgame && !netdemo && !(gametic%ticdup) ) 
            { 
                if (gametic > BACKUPTICS 
                && consistancy[i][buf] != cmd->consistancy) 
                { 
                    I_Error (english_language ?
                             "consistency failure (%i should be %i)" :
                             "Нарушение последовательности (%i должно быть %i)",
                    cmd->consistancy, consistancy[i][buf]); 
                } 
                if (players[i].mo)
                {
                    consistancy[i][buf] = players[i].mo->x; 
                }
                else 
                {
                    consistancy[i][buf] = rndindex; 
                }
            } 
        }
    }

    // check for special buttons
    for (i = 0 ; i < MAXPLAYERS ; i++)
    {
        if (playeringame[i]) 
        { 
            if (players[i].cmd.buttons & BT_SPECIAL) 
            { 
                switch (players[i].cmd.buttons & BT_SPECIALMASK) 
                { 
                    case BTS_PAUSE: 
                    paused ^= 1; 
                    if (paused) 
                    S_PauseSound (); 
                    else 
                    S_ResumeSound (); 
                    break; 
					 
                    case BTS_SAVEGAME: 
                    if (!savedescription[0]) 
                    {
                        M_StringCopy(savedescription, "NET GAME", sizeof(savedescription));
                    }

                savegameslot =  
                (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT; 
                gameaction = ga_savegame; 
                break; 
                } 
            } 
        }
    }

    // Have we just finished displaying an intermission screen?

    if (oldgamestate == GS_INTERMISSION && gamestate != GS_INTERMISSION)
    {
        WI_unloadData();
    }

    oldgamestate = gamestate;

    // do main actions
    switch (gamestate) 
    { 
        case GS_LEVEL: 
        P_Ticker (); 
        ST_Ticker (); 
        AM_Ticker (); 
        HU_Ticker ();            
        break; 

        case GS_INTERMISSION: 
        WI_Ticker (); 
        break; 

        case GS_FINALE: 
        F_Ticker (); 
        break; 

        case GS_DEMOSCREEN: 
        D_PageTicker (); 
        break;
    }        
} 


//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer 
// Called at the start.
// Called by the game initialization functions.
//
void G_InitPlayer (int player) 
{
    // clear everything else to defaults
    G_PlayerReborn (player); 
}


//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel (int player) 
{ 
    player_t *p = &players[player]; 

    memset (p->powers, 0, sizeof (p->powers)); 
    memset (p->cards, 0, sizeof (p->cards)); 
    memset (p->tryopen, 0, sizeof (p->tryopen)); // [crispy] blinking key or skull in the status bar
    p->mo->flags &= ~MF_SHADOW; // cancel invisibility 
    p->extralight = 0;          // cancel gun flashes 
    p->fixedcolormap = 0;       // cancel ir gogles 
    p->damagecount = 0;         // no palette changes 
    p->bonuscount = 0; 
    st_palette = 0;             // [JN] Also no inner palette changes
}


//
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized 
//
void G_PlayerReborn (int player) 
{ 
    int         i; 
    int         frags[MAXPLAYERS]; 
    int         killcount;
    int         itemcount;
    int         secretcount; 
    player_t   *p = &players[player];

    memcpy (frags,players[player].frags,sizeof(frags)); 
    killcount = players[player].killcount; 
    itemcount = players[player].itemcount; 
    secretcount = players[player].secretcount; 

    memset (p, 0, sizeof(*p)); 

    memcpy (players[player].frags, frags, sizeof(players[player].frags)); 
    players[player].killcount = killcount; 
    players[player].itemcount = itemcount; 
    players[player].secretcount = secretcount; 

    p->usedown = p->attackdown = true;  // don't do anything immediately 
    p->playerstate = PST_LIVE;       
    p->health = deh_initial_health;     // Use dehacked value
    p->health_neg = 0;                  // [JN] Reinitialize value
    p->readyweapon = p->pendingweapon = wp_pistol; 
    p->weaponowned[wp_fist] = true; 
    p->weaponowned[wp_pistol] = true; 
    p->ammo[am_clip] = deh_initial_bullets; 

    for (i=0 ; i<NUMAMMO ; i++)
    {
        p->maxammo[i] = maxammo[i]; 
    }

    artifactcount = 0;  // [JN] Press Beta: reset amount of artifacts
}

//
// G_CheckSpot  
// Returns false if the player cannot be respawned
// at the given mapthing_t spot  
// because something is occupying it 
//
boolean G_CheckSpot (int playernum, mapthing_t *mthing)
{ 
    int           i;
    fixed_t       x;
    fixed_t       y; 
    mobj_t       *mo; 
    subsector_t  *ss; 

    if (!players[playernum].mo)
    {
        // first spawn of level, before corpses
        for (i=0 ; i<playernum ; i++)
            if (players[i].mo->x == mthing->x << FRACBITS
            &&  players[i].mo->y == mthing->y << FRACBITS)
            return false;	
        return true;
    }

    x = mthing->x << FRACBITS; 
    y = mthing->y << FRACBITS; 

    if (!P_CheckPosition (players[playernum].mo, x, y))
    {
        return false;
    }

    // flush an old corpse if needed 
    if (bodyqueslot >= BODYQUESIZE) 
    P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]); 
    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
    bodyqueslot++; 

    // spawn a teleport fog
    ss = R_PointInSubsector (x,y);
    
    // The code in the released source looks like this:
    //
    //    an = ( ANG45 * (((unsigned int) mthing->angle)/45) )
    //         >> ANGLETOFINESHIFT;
    //    mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an]
    //                     , ss->sector->floorheight
    //                     , MT_TFOG);
    //
    // But 'an' can be a signed value in the DOS version. This means that
    // we get a negative index and the lookups into finecosine/finesine
    // end up dereferencing values in finetangent[].
    // A player spawning on a deathmatch start facing directly west spawns
    // "silently" with no spawn fog. Emulate this.
    //
    // This code is imported from PrBoom+.

    {
        fixed_t     xa, ya;
        signed int  an;

        // This calculation overflows in Vanilla Doom, but here we deliberately
        // avoid integer overflow as it is undefined behavior, so the value of
        // 'an' will always be positive.
        an = (ANG45 >> ANGLETOFINESHIFT) * ((signed int) mthing->angle / 45);

        // [JN] Unh-uh. Let MT_TFOG always to be spawned,
        // in any direction of Deathmatch spawning spot.
        xa = finecosine[an];
        ya = finesine[an];

        /*
        switch (an)
        {
            case 4096:              // -4096:
            xa = finetangent[2048]; // finecosine[-4096]
            ya = finetangent[0];    // finesine[-4096]
            break;

            case 5120:              // -3072:
            xa = finetangent[3072]; // finecosine[-3072]
            ya = finetangent[1024]; // finesine[-3072]
            break;

            case 6144:              // -2048:
            xa = finesine[0];       // finecosine[-2048]
            ya = finetangent[2048]; // finesine[-2048]
            break;
            
            case 7168:              // -1024:
            xa = finesine[1024];    // finecosine[-1024]
            ya = finetangent[3072]; // finesine[-1024]
            break;

            case 0:
            case 1024:
            case 2048:
            case 3072:
            xa = finecosine[an];
            ya = finesine[an];
            break;

            default:
            I_Error("G_CheckSpot: неопознанный угол %d\n", an);
            xa = ya = 0;
            break;
        }
        */
        mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya, ss->sector->floorheight, MT_TFOG);
    }

    if (players[consoleplayer].viewz != 1)
    {
        S_StartSound (mo, sfx_telept);	// don't start sound on first frame
    }

    return true; 
}


//
// G_DeathMatchSpawnPlayer 
// Spawns a player at one of the random death match spots 
// called at level load and each death 
//
void G_DeathMatchSpawnPlayer (int playernum) 
{ 
    int i, j, selections;

    selections = deathmatch_p - deathmatchstarts;

    if (selections < 4)
    {
        I_Error (english_language ?
                 "Only %i deathmatch spots, 4 required" :
                 "Обнаружено %i стартовых точек для режима Дефтатч.\n Минимальное необходимое количество: 4",
                 selections);
    }
 
    for (j = 0 ; j < 20 ; j++) 
    { 
        i = P_Random() % selections;

        if (G_CheckSpot (playernum, &deathmatchstarts[i]))
        { 
            deathmatchstarts[i].type = playernum+1; 
            P_SpawnPlayer (&deathmatchstarts[i]); 
            return; 
        } 
    } 

    // no good spot, so the player will probably get stuck 
    P_SpawnPlayer (&playerstarts[playernum]); 
} 


//
// G_DoReborn 
// 
void G_DoReborn (int playernum) 
{ 
    int i; 

    if (!netgame)
    {
        // reload the level from scratch
        gameaction = ga_loadlevel;  
    }
    else 
    {
        // respawn at the start

        // first dissasociate the corpse 
        players[playernum].mo->player = NULL;   

        // spawn at random spot if in death match 
        if (deathmatch) 
        { 
            G_DeathMatchSpawnPlayer (playernum); 
            return; 
        } 

        if (G_CheckSpot (playernum, &playerstarts[playernum]) ) 
        { 
            P_SpawnPlayer (&playerstarts[playernum]); 
            return; 
        }

        // try to spawn at one of the other players spots 
        for (i = 0 ; i < MAXPLAYERS ; i++)
        {
            if (G_CheckSpot (playernum, &playerstarts[i]) ) 
            { 
                playerstarts[i].type = playernum+1; // fake as other player 
                P_SpawnPlayer (&playerstarts[i]);
                playerstarts[i].type = i+1;         // restore 
                return; 
            }	    
            // he's going to be inside something.  Too bad.
        }

        P_SpawnPlayer (&playerstarts[playernum]); 
    } 
} 


void G_ScreenShot (void) 
{ 
    gameaction = ga_screenshot; 
}


// DOOM Par Times
static const int pars[4][10] =
{ 
    {0}, 
    {0, 30, 75, 120,  90, 165, 180, 180, 30, 165},  // Episode 1
    {0, 90, 90,  90, 120,  90, 360, 240, 30, 170},  // Episode 2
    {0, 90, 45,  90, 150,  90,  90, 165, 30, 135}   // Episode 3
}; 

// DOOM II Par Times
static const int cpars[32] =
{
     30,  90, 120, 120,  90, 150, 120, 120, 270,  90,   //  1-10
    210, 150, 150, 150, 210, 150, 420, 150, 210, 150,   // 11-20
    240, 150, 180, 150, 150, 300, 330, 420, 300, 180,   // 21-30
    120, 30                                             // 31-32
};

// [crispy] Episode 4 par times from the BFG Edition
static const int e4pars[10] =
{
    0, 165, 255, 135, 150, 180, 390, 135, 360, 180
};

// [JN] Sigil par times
static const int e5pars[10] =
{
    0, 90, 150, 360, 420, 780, 420, 780, 300, 660
};

// [JN] Press Beta Par Times
static const int bpars[4][10] =
{ 
    {0}, 
    {0, 75},  // Уровень 1: Атомная электростанция
    {0, 90},  // Уровень 2: Нечестивый собор
    {0, 90}   // Уровень 3: Хранилище
}; 

// [crispy] No Rest For The Living par times from the BFG Edition
static const int npars[9] =
{
    75, 105, 120, 105, 210, 105, 165, 105, 135
};



//
// G_DoCompleted 
//


void G_ExitLevel (void) 
{ 
    secretexit = false; 
    gameaction = ga_completed; 
} 

// Here's for the german edition.
void G_SecretExitLevel (void) 
{
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    // [JN] No MAP31 in Jaguar Doom.
    if ((gamemode == commercial) && (W_CheckNumForName("map31") < 0) 
    && gamemission != jaguar)
    {
        secretexit = false;
    }
    else
    {
        secretexit = true;
    }

    gameaction = ga_completed; 
} 
 
void G_DoCompleted (void) 
{ 
    int i; 
    extern int bex_pars[4][10], bex_cpars[32]; // [crispy] support [PARS] sections in BEX files

    gameaction = ga_nothing; 

    for (i = 0 ; i < MAXPLAYERS ; i++) 
        if (playeringame[i]) 
        G_PlayerFinishLevel (i);    // take away cards and stuff 

    if (automapactive)
    {
        AM_Stop ();
    }

    if (gamemode != commercial)
    {
        // Chex Quest ends after 5 levels, rather than 8.

        if (gameversion == exe_chex)
        {
            if (gamemap == 5)
            {
                gameaction = ga_victory;
                return;
            }
        }
        else
        {
            switch(gamemap)
            {
                // [crispy] display tally screen after ExM8
                /*
                case 8:
                gameaction = ga_victory;
                return;
                */

                case 9: 
                for (i=0 ; i<MAXPLAYERS ; i++) 
                    players[i].didsecret = true; 
                break;
            }
        }
    }

    wminfo.didsecret = players[consoleplayer].didsecret; 
    wminfo.epsd = gameepisode -1; 
    wminfo.last = gamemap -1;

    // wminfo.next is 0 biased, unlike gamemap
    if ( gamemission == pack_nerve )
    {
        if (secretexit)
            switch(gamemap)
            {
                case 4: wminfo.next = 8; break;
            }
        else
            switch(gamemap)
            {
                case 9: wminfo.next = 4; break;
                default: wminfo.next = gamemap;
            }
    }
    // [JN] Jaguar Doom: secret exit from MAP03 leading to MAP24
    else if (gamemission == jaguar)
    {
        if (secretexit)
	    switch(gamemap)
	    {
	      // Secret exit from Toxin Refinery (3) to Military Base (24-1)
	      case 3:  wminfo.next = 23;
          break;

	      // Secret exit from Command Controls (13) to Extra Map (25-1)
	      case 13:  wminfo.next = 24;
	      break;

	      // Secret exit from Mt. Erebus (21) to Warrens (26-1)
	      case 21:  wminfo.next = 25;
	      break;
	    }
        else
	    switch(gamemap)
	    {
	      // After Military Base (24) go to Command Control (4-1)
	      case 24: wminfo.next = 3;
	      break;

	      // After Extra Map (25) go to Halls of the Damned (14-1)
	      case 25: wminfo.next = 13;
	      break;

	      // After Warrens (26) go to Limbo (22-1)
	      case 26: wminfo.next = 21;
	      break;

	      default: wminfo.next = gamemap;
	    }
    }
    else if ( gamemode == commercial)
    {
        if (secretexit)
        {
            if (gamemap == 2 && havemap33)
            {
                wminfo.next = 32;
            }
            else
            {
                switch(gamemap)
                {
                    case 15: wminfo.next = 30; break;
                    case 31: wminfo.next = 31; break;
                }
            }
        }
        else
        {
            if (gamemap == 33 && havemap33)
            {
                wminfo.next = 2;
            }
            else
            {
                switch(gamemap)
                {
                    case 31:
                    case 32: wminfo.next = 15; break;
                    default: wminfo.next = gamemap;
                }
            }
        }
    }

    else
    {
        if (secretexit) 
        wminfo.next = 8; 	// go to secret level
    
        else if (gamemap == 9) 
        {
            // returning from secret level 
            switch (gameepisode) 
            { 
                case 1:
                wminfo.next = 3;
                break;

                case 2:
                wminfo.next = 5;
                break;

                case 3:
                wminfo.next = 6;
                break;

                case 4:
                wminfo.next = 2;
                break;

                case 5: // [crispy] Sigil
                wminfo.next = 6; 
                break; 
            }                
        }

        else 
        wminfo.next = gamemap;  // go to next level 
    }

    wminfo.maxkills = totalkills; 
    wminfo.maxitems = totalitems; 
    wminfo.maxsecret = totalsecret; 
    wminfo.maxfrags = 0; 

    // Set par time. Doom episode 4 doesn't have a par time, so this
    // overflows into the cpars array. It's necessary to emulate this
    // for statcheck regression testing.

    if (gamemode == commercial)
    {
        // map33 has no official time: initialize to zero
        if (gamemap == 33)
        {
            wminfo.partime = 0;
        }
        else
        // [crispy] support [PARS] sections in BEX files
        if (bex_cpars[gamemap-1])
        {
            wminfo.partime = TICRATE*bex_cpars[gamemap-1];
        }
        else
        {
            wminfo.partime = TICRATE*cpars[gamemap-1];
        }
    }
    else if (gameepisode < 4)
    {
        // [crispy] support [PARS] sections in BEX files
        if (bex_pars[gameepisode][gamemap])
        {
            wminfo.partime = TICRATE*bex_pars[gameepisode][gamemap];
        }
        else
        wminfo.partime = TICRATE*pars[gameepisode][gamemap];
    }
    else if (gameepisode == 4 && singleplayer)
    {
        wminfo.partime = TICRATE*e4pars[gamemap];
    }
    else if (gameepisode == 5)
    {
        // [JN] Sigil
        wminfo.partime = TICRATE*e5pars[gamemap];
    }
    else
    {
        wminfo.partime = TICRATE*cpars[gamemap];
    }

    // [JN] Почему это не работает как else if?
    if (gamemission == pack_nerve)
    wminfo.partime = TICRATE*npars[gamemap-1];

    // [JN] Press Beta par times
    if (gamemode == pressbeta)
    wminfo.partime = TICRATE*bpars[gameepisode][gamemap];

    wminfo.pnum = consoleplayer; 

    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
        wminfo.plyr[i].in = playeringame[i]; 
        wminfo.plyr[i].skills = players[i].killcount; 
        wminfo.plyr[i].sitems = players[i].itemcount; 
        wminfo.plyr[i].ssecret = players[i].secretcount; 
        wminfo.plyr[i].stime = leveltime; 
        memcpy (wminfo.plyr[i].frags, players[i].frags, sizeof(wminfo.plyr[i].frags)); 
    }

    // [crispy] CPhipps - total time for all completed levels
    // cph - modified so that only whole seconds are added to the totalleveltimes
    // value; so our total is compatible with the "naive" total of just adding
    // the times in seconds shown for each level. Also means our total time
    // will agree with Compet-n.
    wminfo.totaltimes = (totalleveltimes += (leveltime - leveltime % TICRATE));

    gamestate = GS_INTERMISSION; 
    viewactive = false; 
    automapactive = false; 

    WI_Start (&wminfo); 
} 


//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
    gameaction = ga_worlddone; 

    if (secretexit) 
    players[consoleplayer].didsecret = true; 

    if ( gamemission == pack_nerve )
    {
        switch (gamemap)
        {
            case 8:
            F_StartFinale ();
            break;
        }
    }
    else if (gamemission == jaguar)
    {
        switch (gamemap)
        {
            case 23:
            F_StartFinale ();
            break;
        }
    }
    // [JN] Не отображать межмиссионные тексты в FreeDM
    else if ( gamemode == commercial && gamevariant != freedm )
    {
        switch (gamemap)
        {
            case 15:
            case 31:
            if (!secretexit)
            break;

            case 6:
            case 11:
            case 20:
            case 30:
            F_StartFinale ();
            break;
        }
    }
    // [crispy] display tally screen after ExM8
    else if (gamemap == 8)
    {
        gameaction = ga_victory;
    }
} 

void G_DoWorldDone (void) 
{        
    idmusnum = -1;  // [JN] jff 3/17/98 allow new level's music to be loaded
    gamestate = GS_LEVEL; 
    gamemap = wminfo.next+1; 
    G_DoLoadLevel (); 
    gameaction = ga_nothing; 
    viewactive = true; 
    AM_clearMarks();  // [JN] jff 4/12/98 clear any marks on the automap
} 


//
// G_InitFromSavegame
// Can be called by the startup code or the menu task. 
//
extern boolean setsizeneeded;

char savename[256];

void G_LoadGame (char* name) 
{
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame; 
} 

void G_DoLoadGame (void) 
{ 
    int savedleveltime;

    gameaction = ga_nothing; 

    save_stream = fopen(savename, "rb");

    if (save_stream == NULL)
    {
        return;
    }

    savegame_error = false;

    if (!P_ReadSaveGameHeader())
    {
        fclose(save_stream);
        return;
    }

    savedleveltime = leveltime;

    // load a base level 
    G_InitNew (gameskill, gameepisode, gamemap); 
 
    leveltime = savedleveltime;

    // dearchive all the modifications
    P_UnArchivePlayers (); 
    P_UnArchiveWorld (); 
    P_UnArchiveThinkers (); 
    P_UnArchiveSpecials (); 
    P_UnArchiveAutomap ();
    P_RestoreTargets ();
 
    if (!P_ReadSaveGameEOF())
    I_Error (english_language ?
             "Bad savegame" :
             "Некорректный файл сохранения");

    fclose(save_stream);
    
    // [JN] Additional message after game load.
    if (!vanillaparm)
    {
        players[consoleplayer].message_system = DEH_String(ggloaded);
    }
    
    // draw the pattern into the back screen
    R_FillBackScreen ();   
} 


//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string 
//
void
G_SaveGame
( int   slot,
  char* description )
{
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
}

void G_DoSaveGame (void) 
{ 
    char *savegame_file;
    char *temp_savegame_file;
    char *recovery_savegame_file;

    recovery_savegame_file = NULL;
    temp_savegame_file = P_TempSaveGameFile();
    savegame_file = P_SaveGameFile(savegameslot);

    // Open the savegame file for writing.  We write to a temporary file
    // and then rename it at the end if it was successfully written.
    // This prevents an existing savegame from being overwritten by
    // a corrupted one, or if a savegame buffer overrun occurs.
    save_stream = fopen(temp_savegame_file, "wb");

    if (save_stream == NULL)
    {
        // Failed to save the game, so we're going to have to abort. But
        // to be nice, save to somewhere else before we call I_Error().
        recovery_savegame_file = M_TempFile("recovery.dsg");
        save_stream = fopen(recovery_savegame_file, "wb");
        if (save_stream == NULL)
        {
            I_Error(english_language ?
                    "Failed to open either '%s' or '%s' to write savegame." :
                    "Невозможно открыть '%s' или '%s' для записи файла сохранения.",
                    temp_savegame_file, recovery_savegame_file);
        }
    }

    savegame_error = false;

    P_WriteSaveGameHeader(savedescription);

    P_ArchivePlayers ();
    P_ArchiveWorld ();
    P_ArchiveThinkers ();
    P_ArchiveSpecials ();
    P_ArchiveAutomap ();

    P_WriteSaveGameEOF();

    // Finish up, close the savegame file.

    fclose(save_stream);

    if (recovery_savegame_file != NULL)
    {
        // We failed to save to the normal location, but we wrote a
        // recovery file to the temp directory. Now we can bomb out
        // with an error.
        if (english_language)
        {
            I_Error("Failed to open savegame file '%s' for writing.\n"
                    "But your game has been saved to '%s' for recovery.",
                    temp_savegame_file, recovery_savegame_file);
        }
        else
        {
            I_Error("Невозможно открыть файл '%s' для совершения записи.\n"
                    "Сохранение было записано в '%s' для возможности восстановления.",
                    temp_savegame_file, recovery_savegame_file);
        }
    }

    // Now rename the temporary savegame file to the actual savegame
    // file, overwriting the old savegame if there was one there.

    remove(savegame_file);
    rename(temp_savegame_file, savegame_file);

    gameaction = ga_nothing;
    M_StringCopy(savedescription, "", sizeof(savedescription));

    players[consoleplayer].message_system = DEH_String(ggsaved);

    // draw the pattern into the back screen
    R_FillBackScreen ();
}
 

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set. 
//
skill_t d_skill; 
int     d_episode; 
int     d_map; 
 
void
G_DeferedInitNew
( skill_t   skill,
  int       episode,
  int       map) 
{
    d_skill = skill; 
    d_episode = episode; 
    d_map = map; 
    gameaction = ga_newgame; 
    flag667 = false;
} 


void G_DoNewGame (void) 
{
    idmusnum = -1;  // [JN] e6y: allow new level's music to be loaded
    demoplayback = false; 
    netdemo = false;
    netgame = false;
    deathmatch = false;
    playeringame[1] = playeringame[2] = playeringame[3] = 0;
    // [crispy] do not reset -respawn, -fast and -nomonsters parameters
    /*
    respawnparm = false;
    fastparm = false;
    nomonsters = false;
    */
    consoleplayer = 0;
    G_InitNew (d_skill, d_episode, d_map); 
    gameaction = ga_nothing; 
    flag667 = false;
} 


//
// G_DoSelectiveGame
// [JN] Start new game with given parameters in "Level select" menu
//
void G_DoSelectiveGame (int choice) 
{
    int i;
    player_t *plr = &players[consoleplayer];
    // [crispy] make sure "fast" parameters are really only applied once
    static boolean fast_applied;

    demoplayback = false; 
    netdemo = false;
    netgame = false;
    deathmatch = false;
    playeringame[1] = playeringame[2] = playeringame[3] = 0;
    consoleplayer = 0;
    gameaction = ga_nothing; 
    flag667 = false;

    // Close "Level select" menu
    RD_Menu_DeactivateMenu();

    G_InitNew (selective_skill,
               // Set appropriate episode
               gamemode == shareware  ? 1 : 
               gamemode == commercial ? 0 : 
               selective_episode,
               // Set appropriate map
               gamemode == pressbeta ? 1 :
               selective_map); 

    // Do not modify respawnparm parameter
    respawnmonsters = selective_respawn;

    // Do not modify fastparm parameter
    // [crispy] make sure "fast" parameters are really only applied once
    if ((selective_fast || gameskill == sk_nightmare || gameskill == sk_ultranm) && !fast_applied)
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
        // [crispy] Fix infinite loop caused by Demon speed bug
        if (states[i].tics > 1)
        {
            states[i].tics >>= 1;
        }

        mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;

        fast_applied = true;
    }
    else if (!selective_fast && gameskill != sk_nightmare && gameskill != sk_ultranm && fast_applied)
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
        states[i].tics <<= 1;
        mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
        fast_applied = false;
    }

    // Health
    plr->health = selective_health;
    plr->mo->health = selective_health;

    // Armor
    plr->armorpoints = selective_armor;
    // Armor type. Set to 0 if no armor given.
    plr->armortype = selective_armor == 0 ? 0 : selective_armortype;

    // Weapons
    plr->weaponowned[wp_chainsaw] = selective_wp_chainsaw;
    plr->weaponowned[wp_shotgun] = selective_wp_shotgun;
    // Super shotgun not available in Doom 1 and Jaguar
    plr->weaponowned[wp_supershotgun] = (logical_gamemission == doom || 
                                         gamemission == jaguar) ? 
                                         0 : selective_wp_supershotgun;
    plr->weaponowned[wp_chaingun] = selective_wp_chaingun;
    plr->weaponowned[wp_missile] = selective_wp_missile;
    // Plasma gun not available in shareware
    plr->weaponowned[wp_plasma] = gamemode == shareware ? 0 : selective_wp_plasma;
    // BFG9000 not available in shareware
    plr->weaponowned[wp_bfg] = gamemode == shareware ? 0 : selective_wp_bfg;

    // Backpack
    plr->backpack = selective_backpack;
    if (selective_backpack)
    {
        plr->maxammo[0] *= 2;
        plr->maxammo[1] *= 2;
        plr->maxammo[2] *= 2;
        plr->maxammo[3] *= 2;
    }

    // Ammo
    plr->ammo[0] = selective_ammo_0; // bullets
    plr->ammo[1] = selective_ammo_1; // shells
    plr->ammo[2] = selective_ammo_2; // cells
    plr->ammo[3] = selective_ammo_3; // rockets

    // Keys
    plr->cards[0] = selective_key_0; // blue keycard
    plr->cards[1] = selective_key_1; // yellow keycard
    plr->cards[2] = selective_key_2; // red keycard
    plr->cards[3] = selective_key_3; // blue skull key
    plr->cards[4] = selective_key_4; // yellow skull key
    plr->cards[5] = selective_key_5; // red skull key
} 


void
G_InitNew
( skill_t   skill,
  int       episode,
  int       map )
{
    char    *skytexturename;
    int     i;
    // [crispy] make sure "fast" parameters are really only applied once
    static boolean fast_applied;

    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

    /*
    // Note: This commented-out block of code was added at some point
    // between the DOS version(s) and the Doom source release. It isn't
    // found in disassemblies of the DOS version and causes IDCLEV and
    // the -warp command line parameter to behave differently.
    // This is left here for posterity.

    // This was quite messy with SPECIAL and commented parts.
    // Supposedly hacks to make the latest edition work.
    // It might not work properly.
    if (episode < 1)
      episode = 1;

    if ( gamemode == retail )
    {
      if (episode > 4)
	episode = 4;
    }
    else if ( gamemode == shareware )
    {
      if (episode > 1)
	   episode = 1;	// only start episode 1 on shareware
    }
    else
    {
      if (episode > 3)
	episode = 3;
    }
    */

    // [JN] Increased from sk_nightmare to sk_ultranm
    if (skill > sk_ultranm)
    skill = sk_ultranm;

    if (gameversion >= exe_ultimate)
    {
        if (episode == 0)
        {
            episode = 4;
        }
    }
    else
    {
        if (episode < 1)
        {
            episode = 1;
        }
        if ((episode > 3 && !sgl_loaded) || (episode == 4 && sgl_loaded))
        {
            episode = 3;
        }
    }

    if (episode > 1 && gamemode == shareware)
    {
        episode = 1;
    }

    if (map < 1)
    map = 1;

    if ( (map > 9) && ( gamemode != commercial) )
    map = 9;

    M_ClearRandom ();

    if (skill == sk_nightmare || respawnparm )
    respawnmonsters = true;
    else
    respawnmonsters = false;

    // [crispy] make sure "fast" parameters are really only applied once
    if ((fastparm || skill == sk_nightmare || skill == sk_ultranm) && !fast_applied)
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
	    // [crispy] Fix infinite loop caused by Demon speed bug
	    if (states[i].tics > 1)
	    {
	    states[i].tics >>= 1;
	    }

        mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;

        fast_applied = true;
    }
    else if (!fastparm && skill != sk_nightmare && skill != sk_ultranm && fast_applied)
    {
        for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
        states[i].tics <<= 1;
        mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
        fast_applied = false;
    }
    
    // [JN] Ultra Nightmare definitions
    if (skill == sk_ultranm)
    {
        // Monster's speed                           Who (initial value)
        mobjinfo[MT_POSSESSED].speed = 12;           // Zombieman (8)
        mobjinfo[MT_SHOTGUY].speed = 12;             // Shotgun guy (8)
        mobjinfo[MT_CHAINGUY].speed = 12;            // Chaingunner (8)
        mobjinfo[MT_TROOP].speed = 13;               // Imp (8)
        mobjinfo[MT_SKULL].speed = 14;               // Lost Soul (8)
        mobjinfo[MT_HEAD].speed = 14;                // Cacodemon (8)
        mobjinfo[MT_KNIGHT].speed = 14;              // Hell Knight (8)
        mobjinfo[MT_BRUISER].speed = 14;             // Baron of Hell (8)
        mobjinfo[MT_BABY].speed = 17;                // Arachnotron (12)
        mobjinfo[MT_PAIN].speed = 14;                // Pain Elemental (8)
        mobjinfo[MT_UNDEAD].speed = 14;              // Revenant (10)
        mobjinfo[MT_FATSO].speed = 11;               // Mancubus (8)
        mobjinfo[MT_VILE].speed = 18;                // Archvile (15)
        mobjinfo[MT_SPIDER].speed = 17;              // Spider Mastermind (12)
        mobjinfo[MT_CYBORG].speed = 20;              // Cyberdemon (16)
        mobjinfo[MT_WOLFSS].speed = 13;              // Wolf SS (8)

        // Monster's missiles speed                  What (initial value)
        mobjinfo[MT_ARACHPLAZ].speed = 30*FRACUNIT;  // Arachnotron (25*FRACUNIT)
        mobjinfo[MT_FATSHOT].speed = 25*FRACUNIT;    // Mancubus (20*FRACUNIT)

        // Monster's damage                          What (initial value)
        mobjinfo[MT_TROOPSHOT].damage = 4;           // Imp (3)
        mobjinfo[MT_HEADSHOT].damage = 7;            // Cacodemon (5)
        mobjinfo[MT_BRUISERSHOT].damage = 10;        // Knight / Baron (8) 
        mobjinfo[MT_ARACHPLAZ].damage = 7;           // Arachnotron (5)
        mobjinfo[MT_TRACER].damage = 12;             // Revenant (10)
        mobjinfo[MT_FATSHOT].damage = 12;            // Mancubus (8)
    }
    // [JN] Fallback to standard values
    else
    {
        // Monster's speed                          Who
        mobjinfo[MT_POSSESSED].speed = 8;           // Zombieman
        mobjinfo[MT_SHOTGUY].speed = 8;             // Shotgun guy
        mobjinfo[MT_CHAINGUY].speed = 8;            // Chaingunner
        mobjinfo[MT_TROOP].speed = 8;               // Imp
        mobjinfo[MT_SKULL].speed = 8;               // Lost Soul
        mobjinfo[MT_HEAD].speed = 8;                // Cacodemon
        mobjinfo[MT_KNIGHT].speed = 8;              // Hell Knight
        mobjinfo[MT_BRUISER].speed = 8;             // Baron of Hell
        mobjinfo[MT_BABY].speed = 12;               // Arachnotron
        mobjinfo[MT_PAIN].speed = 8;                // Pain Elemental
        mobjinfo[MT_UNDEAD].speed = 10;             // Revenant
        mobjinfo[MT_FATSO].speed = 8;               // Mancubus
        mobjinfo[MT_VILE].speed = 15;               // Archvile
        mobjinfo[MT_SPIDER].speed = 12;             // Spider Mastermind
        mobjinfo[MT_CYBORG].speed = 16;             // Cyberdemon
        mobjinfo[MT_WOLFSS].speed = 8;              // Wolf SS

        // Monster's missiles speed                 What
        mobjinfo[MT_ARACHPLAZ].speed = 25*FRACUNIT; // Arachnotron plasma
        mobjinfo[MT_FATSHOT].speed = 20*FRACUNIT;   // Mancubus fireball

        // Monster's damage                         What
        mobjinfo[MT_TROOPSHOT].damage = 3;          // Imp fireball
        mobjinfo[MT_HEADSHOT].damage = 5;           // Cacodemon fireball
        mobjinfo[MT_BRUISERSHOT].damage = 8;        // Knight / Baron fireball 
        mobjinfo[MT_ARACHPLAZ].damage = 5;          // Arachnotron plasma
        mobjinfo[MT_TRACER].damage = 10;            // Revenant fireball
        mobjinfo[MT_FATSHOT].damage = 8;            // Mancubus fireball
    }

    // force players to be initialized upon first level load
    for (i=0 ; i<MAXPLAYERS ; i++)
    players[i].playerstate = PST_REBORN;

    usergame = true;                // will be set false if a demo
    paused = false;
    demoplayback = false;
    automapactive = false;
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;

    // [crispy] CPhipps - total time for all completed levels
    totalleveltimes = 0;
    defdemotics = 0;
    viewactive = true;

    // [JN] jff 4/16/98 force marks on automap cleared every new level start
    AM_clearMarks();

    artifactcount = 0;  // [JN] Press Beta: initialy 0 artifacts
    lifecount = 3;      // [JN] Press Beta: give player 3 lifes

    // Set the sky to use.
    //
    // Note: This IS broken, but it is how Vanilla Doom behaves.
    // See http://doomwiki.org/wiki/Sky_never_changes_in_Doom_II.
    //
    // Because we set the sky here at the start of a game, not at the
    // start of a level, the sky texture never changes unless we
    // restore from a saved game.  This was fixed before the Doom
    // source release, but this IS the way Vanilla DOS Doom behaves.

    if (gamemode == commercial)
    {
        if (gamemap < 12)
            skytexturename = "SKY1";
        else if (gamemap < 21)
            skytexturename = "SKY2";
        else
            skytexturename = "SKY3";
    }
    else
    {
        switch (gameepisode)
        {
            default:
            case 1:
            skytexturename = "SKY1";
            break;

            case 2:
            skytexturename = "SKY2";
            break;

            case 3:
            skytexturename = "SKY3";
            break;

            case 4: // Special Edition sky
            skytexturename = "SKY4";
            break;

            case 5: // [crispy] Sigil
            skytexturename = "SKY5_ZD";
            if (R_CheckTextureNumForName(DEH_String(skytexturename)) == -1)
            {
                skytexturename = "SKY3";
            }
            break;
        }
    }

    skytexturename = DEH_String(skytexturename);
    skytexture = R_TextureNumForName(skytexturename);

    G_DoLoadLevel ();
}


//
// DEMO RECORDING 
// 
#define DEMOMARKER  0x80

// [crispy] demo progress bar and timer widget
int defdemotics = 0, deftotaldemotics;

void G_ReadDemoTiccmd (ticcmd_t* cmd) 
{ 
    if (*demo_p == DEMOMARKER) 
    {
        // end of demo data stream 
        G_CheckDemoStatus (); 
        return; 
    }

    cmd->forwardmove = ((signed char)*demo_p++); 
    cmd->sidemove = ((signed char)*demo_p++); 

    // If this is a longtics demo, read back in higher resolution

    if (longtics)
    {
        cmd->angleturn = *demo_p++;
        cmd->angleturn |= (*demo_p++) << 8;
    }
    else
    {
        cmd->angleturn = ((unsigned char) *demo_p++)<<8; 
    }

    cmd->buttons = (unsigned char)*demo_p++; 

    // [crispy] increase demo tics counter
    // applies to both recording and playback,
    // because G_WriteDemoTiccmd() calls G_ReadDemoTiccmd() once
    defdemotics++;
} 

// Increase the size of the demo buffer to allow unlimited demos

static void IncreaseDemoBuffer(void)
{
    int     current_length;
    byte    *new_demobuffer;
    byte    *new_demop;
    int     new_length;

    // Find the current size

    current_length = demoend - demobuffer;
    
    // Generate a new buffer twice the size
    new_length = current_length * 2;
    
    new_demobuffer = Z_Malloc(new_length, PU_STATIC, 0);
    new_demop = new_demobuffer + (demo_p - demobuffer);

    // Copy over the old data

    memcpy(new_demobuffer, demobuffer, current_length);

    // Free the old buffer and point the demo pointers at the new buffer.

    Z_Free(demobuffer);

    demobuffer = new_demobuffer;
    demo_p = new_demop;
    demoend = demobuffer + new_length;
}

void G_WriteDemoTiccmd (ticcmd_t* cmd) 
{ 
    byte *demo_start;

    if (BK_isKeyPressed(bk_finish_demo)) // press q to end demo recording
    G_CheckDemoStatus (); 

    demo_start = demo_p;

    *demo_p++ = cmd->forwardmove; 
    *demo_p++ = cmd->sidemove; 

    // If this is a longtics demo, record in higher resolution
 
    if (longtics)
    {
        *demo_p++ = (cmd->angleturn & 0xff);
        *demo_p++ = (cmd->angleturn >> 8) & 0xff;
    }
    else
    {
        *demo_p++ = cmd->angleturn >> 8; 
    }

    *demo_p++ = cmd->buttons; 

    // reset demo pointer back
    demo_p = demo_start;

    // [JN] No limits for demo recording
    if (demo_p > demoend - 16)
    {
        IncreaseDemoBuffer();
    }
	
    G_ReadDemoTiccmd (cmd); // make SURE it is exactly the same 
}


//
// G_RecordDemo
//
void G_RecordDemo (char *name)
{
    size_t  demoname_size;
    int     i;
    int     maxsize;

    usergame = false;
    demoname_size = strlen(name) + 5;
    demoname = Z_Malloc(demoname_size, PU_STATIC, NULL);
    M_snprintf(demoname, demoname_size, "%s.lmp", name);
    maxsize = 0x20000;

    //!
    // @arg <size>
    // @category demo
    // @vanilla
    //
    // Specify the demo buffer size (KiB)
    //

    i = M_CheckParmWithArgs("-maxdemo", 1);
    if (i)
    maxsize = atoi(myargv[i+1])*1024;
    demobuffer = Z_Malloc (maxsize,PU_STATIC,NULL); 
    demoend = demobuffer + maxsize;

    demorecording = true; 
} 

// Get the demo version code appropriate for the version set in gameversion.
int G_VanillaVersionCode(void)
{
    switch (gameversion)
    {
        case exe_doom_1_2:
        I_Error(english_language ?
                "Doom 1.2 does not have a version code!" :
                "В Doom 1.2 отсутствует код версии!");

        case exe_doom_1_666:
        return 106;

        case exe_doom_1_7:
        return 107;
        
        case exe_doom_1_8:
        return 108;

        case exe_doom_1_9:
        default:  // All other versions are variants on v1.9:
        return 109;
    }
}

void G_BeginRecording (void) 
{ 
    int i;

    demo_p = demobuffer;

    //!
    // @category demo
    //
    // Record a high resolution "Doom 1.91" demo.
    //

    longtics = D_NonVanillaRecord(M_ParmExists("-longtics"), "Doom 1.91 demo format");

    // If not recording a longtics demo, record in low res
    lowres_turn = !longtics;

    if (longtics)
    {
        *demo_p++ = DOOM_191_VERSION;
    }
    else
    {
        *demo_p++ = G_VanillaVersionCode();
    }

    *demo_p++ = gameskill; 
    *demo_p++ = gameepisode; 
    *demo_p++ = gamemap; 
    *demo_p++ = deathmatch; 
    *demo_p++ = respawnparm;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;

    for (i=0 ; i<MAXPLAYERS ; i++) 
    *demo_p++ = playeringame[i]; 		 
} 


//
// G_PlayDemo 
//

char*   defdemoname; 

void G_DeferedPlayDemo (char* name) 
{ 
    defdemoname = name; 
    gameaction = ga_playdemo; 
} 

// Generate a string describing a demo version

// [JN] No longer used
/*
static char *DemoVersionDescription(int version)
{
    static char resultbuf[16];

    switch (version)
    {
        case 104:
        return "v1.4";

        case 105:
        return "v1.5";

        case 106:
        return "v1.6/v1.666";

        case 107:
        return "v1.7/v1.7a";

        case 108:
        return "v1.8";

        case 109:
        return "v1.9";

        case 111:
        return "v1.91 hack demo?";

        default:
        break;
    }

    // Unknown version.  Perhaps this is a pre-v1.4 IWAD?  If the version
    // byte is in the range 0-4 then it can be a v1.0-v1.2 demo.

    if (version >= 0 && version <= 4)
    {
        return "v1.0/v1.1/v1.2";
    }
    else
    {
        M_snprintf(resultbuf, sizeof(resultbuf), "%i.%i (неизвестная версия)", version / 100, version % 100);
        return resultbuf;
    }
}
*/

void G_DoPlayDemo (void)
{
    skill_t skill;
    int     i, lumpnum, episode, map;
    int     demoversion;
    int lumplength; // [crispy]

    lumpnum = W_GetNumForName(defdemoname);
    gameaction = ga_nothing;
    demobuffer = W_CacheLumpNum(lumpnum, PU_STATIC);
    demo_p = demobuffer;

    // [crispy] ignore empty demo lumps
    lumplength = W_LumpLength(lumpnum);
    if (lumplength < 0xd)
    {
        demoplayback = true;
        G_CheckDemoStatus();
        return;
    }

    demoversion = *demo_p++;

    longtics = false;

    // Longtics demos use the modified format that is generated by cph's
    // hacked "v1.91" doom exe. This is a non-vanilla extension.
    if (D_NonVanillaPlayback(demoversion == DOOM_191_VERSION, lumpnum, "Формат демозаписи Doom 1.91"))
    {
        longtics = true;
    }
    else if (demoversion != G_VanillaVersionCode())
    {
        // [JN] Don't bomb out on unsopported demo versions, just skip them
        return;

        /*
        char *message = "Демозапись от другой версии игры!\n"
                        "(версия %i, должна быть %i)\n"
                        "\n"
                        "*** Вам необходимо обновить вашу версию  Doom до v1.9. ***"
                        "\n"
                        "   Дополнительная информация: https://www.doomworld.com/classicdoom/info/patches.php"
                        "\n";

        I_Error(message, demoversion, G_VanillaVersionCode(), DemoVersionDescription(demoversion));
        */
    }

    skill = *demo_p++; 
    episode = *demo_p++; 
    map = *demo_p++; 
    deathmatch = *demo_p++;
    respawnparm = *demo_p++;
    fastparm = *demo_p++;
    nomonsters = *demo_p++;
    consoleplayer = *demo_p++;

    for (i=0 ; i<MAXPLAYERS ; i++) 
	playeringame[i] = *demo_p++; 

    if (playeringame[1] || M_CheckParm("-solo-net") > 0 || M_CheckParm("-netdemo") > 0)
    {
        netgame = true;
        netdemo = true;
    }

    // don't spend a lot of time in loadlevel 
    precache = false;
    G_InitNew (skill, episode, map); 
    precache = true; 
    starttime = I_GetTime (); 

    usergame = false; 
    demoplayback = true; 
    
    // [crispy] demo progress bar
    {
        int i, numplayersingame = 0;
        byte *demo_ptr = demo_p;

        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {
                numplayersingame++;
            }
        }

        deftotaldemotics = defdemotics = 0;

        while (*demo_ptr != DEMOMARKER && (demo_ptr - demobuffer) < lumplength)
        {
            demo_ptr += numplayersingame * (longtics ? 5 : 4);
            deftotaldemotics++;
        }
    }
} 


//
// G_TimeDemo 
//

void G_TimeDemo (char* name) 
{
    //!
    // @category video
    // @vanilla
    //
    // Disable rendering the screen entirely.
    //

    nodrawers = M_CheckParm ("-nodraw");

    timingdemo = true; 
    singletics = true; 

    defdemoname = name; 
    gameaction = ga_playdemo; 
} 


/* 
=================== 
= 
= G_CheckDemoStatus 
= 
= Called after a death or level completion to allow demos to be cleaned up 
= Returns true if a new demo loop action will take place 
=================== 
*/ 

boolean G_CheckDemoStatus (void) 
{ 
    int endtime; 

    if (timingdemo) 
    { 
        float fps;
        int   realtics;

        endtime = I_GetTime (); 
        realtics = endtime - starttime;
        fps = ((float) gametic * TICRATE) / realtics;

        // Prevent recursive calls
        timingdemo = false;
        demoplayback = false;

        if (english_language)
        {
            I_Error ("timed %i gametics in %i realtics (%f fps)",
                        gametic, realtics, fps);
        }
        else
        {
            I_Error ("Насчитано %i gametics в %i realtics.\n"
                    "Среднее значение FPS: %f.", gametic, realtics, fps);
        }
    } 

    if (demoplayback)
    { 
        W_ReleaseLumpName(defdemoname);
        demoplayback = false; 
        netdemo = false;
        netgame = false;
        deathmatch = false;
        playeringame[1] = playeringame[2] = playeringame[3] = 0;
        respawnparm = false;
        fastparm = false;
        nomonsters = false;
        consoleplayer = 0;

        if (singledemo) 
            I_Quit (); 
        else 
            D_AdvanceDemo (); 

        return true; 
    } 

    if (demorecording) 
    { 
        *demo_p++ = DEMOMARKER; 

        M_WriteFile (demoname, demobuffer, demo_p - demobuffer); 
        Z_Free (demobuffer); 
        demorecording = false; 
        I_Error (english_language ?
                 "Demo %s recorded" :
                 "Демозапись %s завершена",
                 demoname); 
    } 

    return false; 
}

