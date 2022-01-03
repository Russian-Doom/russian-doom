//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 1993-2008 Raven Software
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2022 Julian Nechaevsky
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

#include "doomdef.h" 
#include "doomstat.h"

#include "z_zone.h"
#include "f_finale.h"
#include "m_misc.h"
#include "m_menu.h"
#include "i_system.h"

#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"

#include "d_main.h"

#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

// Needs access to LFB.
#include "v_video.h"
#include "w_wad.h"
#include "p_local.h" 
#include "s_sound.h"

// Data.
#include "sounds.h"

// SKY handling - still the wrong place.
#include "r_data.h"
#include "g_game.h"
#include "rd_lang.h"
#include "jn.h"


#define SAVESTRINGSIZE  24
#define MAX_JOY_BUTTONS 20


boolean	G_CheckDemoStatus (void);
void    G_ReadDemoTiccmd (ticcmd_t* cmd);
void    G_WriteDemoTiccmd (ticcmd_t* cmd);
void    G_PlayerReborn (int player);
void    G_InitNew (skill_t skill, int episode, int map);

void    G_DoReborn (int playernum);

void    G_DoLoadLevel (void);
void    G_DoNewGame (void);
void    G_DoLoadGame (void);
void    G_DoPlayDemo (void);
void    G_DoCompleted (void);
void    G_DoVictory (void);
void    G_DoWorldDone (void);
void    G_DoSaveGame (void);

gameaction_t    gameaction;
gamestate_t     gamestate;
skill_t         gameskill;
boolean         respawnmonsters;
int             gameepisode;
int             gamemap;

boolean         paused;
boolean         sendpause;      // send a pause event next tic
boolean         sendsave;       // send a save event next tic
boolean         usergame;       // ok to save / end game

boolean         timingdemo;     // if true, exit with report on completion
int             starttime;      // for comparative timing purposes

boolean         viewactive;

boolean         deathmatch;             // only if started as net death
boolean         netgame;                // only true if packets are broadcast
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];

int             consoleplayer;      // player taking events and displaying
int             displayplayer;      // view being displayed
int             gametic; 
int             levelstarttic;      // gametic at level start
int             totalleveltimes;    // [crispy] CPhipps - total time for all completed levels
int             totalkills, totalitems, totalsecret;    // for intermission


char            demoname[32];
boolean         demorecording;
boolean         demoplayback;
boolean         netdemo;
byte*           demobuffer;
byte*           demo_p;
byte*           demoend;
boolean         singledemo;     	// quit after playing a demo from cmdline

boolean         precache = true;    // if true, load all graphics at start

wbstartstruct_t wminfo;             // parms for world map / intermission

short		consistancy[MAXPLAYERS][BACKUPTICS];

byte*       savebuffer;
byte        saveshit[SAVEGAMESIZE];


//
// controls (have defaults)
//
int     key_right;
int     key_left;
int     key_up;
int     key_down; 
int     key_strafeleft;
int     key_straferight; 
int     key_fire;
int     key_use;
int     key_strafe;
int     key_speed; 
int     key_mouselook;
int     key_crosshair;

int     mousebfire; 
int     mousebstrafe; 
int     mousebforward; 

int     joybfire; 
int     joybstrafe; 
int     joybuse; 
int     joybspeed; 


// [JN] Russian Doom specific variables 

// Language swapping
int english_language;

// Rendering
int noflats;
int show_fps;
int show_diskicon;
int screen_wiping;
int screenshot_pcx;

// Display
int level_brightness;
int local_time;

// Messages
int messages_pickup_color;
int messages_secret_color = 3;
int messages_system_color;
int messages_chat_color = 1;

// Audio
int snd_samplerate;
int snd_monomode;
int snd_pitchshift;

// Automap
int automap_color;
int automap_antialias;
int automap_stats;
int automap_overlay = 0; // [JN] Not working, keep for better times.
int automap_rotate;
int automap_follow;
int automap_grid;

// Controls
int mlook;
int mouse_y_invert; // [crispy]
int novert;

// Gameplay: Graphical
int brightmaps;
int fake_contrast;
int translucency;
int improved_fuzz;
int colored_hud;
int colored_blood;
int swirling_liquids;
int invul_sky;
int linear_sky;
int flip_weapons;
int draw_shadowed_text;

// Gameplay: Audible
int play_exit_sfx;
int crushed_corpses_sfx;
int blazing_door_fix_sfx;
int noise_alert_sfx;

// Gameplay: Tactical
int secret_notification;
int negative_health;

// Gameplay: Physical
int over_under;
int torque;
int weapon_bobbing;
int ssg_blast_enemies;
int randomly_flipcorpses;
int floating_powerups;

// Gameplay: Crosshair
int crosshair_draw;
int crosshair_health;

// Gameplay: Gameplay
int extra_player_faces;
int unlimited_lost_souls;
int fast_quickload;
int no_internal_demos;


#define MAXPLMOVE       (forwardmove[1]) 
#define TURBOTHRESHOLD  0x32

fixed_t     forwardmove[2] = {0x19, 0x32};
fixed_t     sidemove[2]    = {0x18, 0x28};
fixed_t     angleturn[3]   = {640, 1280, 320};  // + slow turn

// [JN] If true, activate maximum weapon bobbing
boolean max_bobbing;

#define SLOWTURNTICS    6
#define NUMKEYS         256

boolean     gamekeydown[NUMKEYS];
int         turnheld;   // for accelerative turning
 
boolean     mousearray[4];
boolean*    mousebuttons = &mousearray[1];  // allow [-1]

// mouse values are used once
int     mousex;
int     mousey;
int     dclicktime;
int     dclickstate;
int     dclicks; 
int     dclicktime2;
int     dclickstate2;
int     dclicks2;

// joystick values are repeated
int         joyxmove;
int         joyymove;
boolean     joyarray[5];
boolean*    joybuttons = &joyarray[1];  // allow [-1]

int     savegameslot;
char    savedescription[32];
 
#define	BODYQUESIZE	32

mobj_t*     bodyque[BODYQUESIZE];
int         bodyqueslot;

void*       statcopy;       // for statistics driver

extern int isCyberPresent;  // is CyberMan present?
void I_ReadCyberCmd(ticcmd_t *cmd);

int G_CmdChecksum (ticcmd_t* cmd)
{
    int     i;
    int     sum = 0;

    for (i=0 ; i< sizeof(*cmd)/4 - 1 ; i++)
	sum += ((int *)cmd)[i];

    return sum;
}


//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
void G_BuildTiccmd (ticcmd_t* cmd)
{
    int     i;
    boolean strafe;
    boolean bstrafe;
    int     speed;
    int     tspeed;
    int     forward;
    int     side;
    int     look;

    ticcmd_t*	base;

    base = I_BaseTiccmd (); // empty, or external driver
    memcpy (cmd,base,sizeof(*cmd));
	
    cmd->consistancy = consistancy[consoleplayer][maketic%BACKUPTICS];

    if (isCyberPresent)
    I_ReadCyberCmd(cmd);
 
    strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe] || joybuttons[joybstrafe]; 

    // [JN] Always Run hack
    // Taken from Chocolate Doom (src/doom/g_game.c)
    // fraggle: support the old "joyb_speed = 31" hack which
    // allowed an autorun effect

    speed = key_speed >= NUMKEYS || joybspeed >= MAX_JOY_BUTTONS || joybuttons[joybspeed];

    // [JN] "Run" button makes player walk in autorun mode,
    // and makes player run in walk mode.
    if ((joybspeed >= MAX_JOY_BUTTONS) && (gamekeydown[key_speed]))
        speed = false;
    else if ((joybspeed <= MAX_JOY_BUTTONS) && (gamekeydown[key_speed]))
        speed = true;

    forward = side = look = 0;

    // use two stage accelerative turning
    // on the keyboard and joystick
    if (joyxmove < 0 || joyxmove > 0 || gamekeydown[key_right] || gamekeydown[key_left])
    turnheld += ticdup;
    else
    turnheld = 0;

    if (turnheld < SLOWTURNTICS)
    tspeed = 2; // slow turn
    else
    tspeed = speed;

    // let movement keys cancel each other out
    if (strafe)
    {
        if (gamekeydown[key_right])
        side += sidemove[speed];

        if (gamekeydown[key_left])
        side -= sidemove[speed];

        if (joyxmove > 0)
	    side += sidemove[speed];

        if (joyxmove < 0)
        side -= sidemove[speed];
    }
    else
    {
        if (gamekeydown[key_right])
        cmd->angleturn -= angleturn[tspeed];

        if (gamekeydown[key_left])
        cmd->angleturn += angleturn[tspeed];

        if (joyxmove > 0) 
        cmd->angleturn -= angleturn[tspeed];

        if (joyxmove < 0)
        cmd->angleturn += angleturn[tspeed];
    }

    if (gamekeydown[key_up])
    forward += forwardmove[speed];

    if (gamekeydown[key_down])
    forward -= forwardmove[speed];

    if (joyymove < 0)
    forward += forwardmove[speed];

    if (joyymove > 0)
    forward -= forwardmove[speed];

    if (gamekeydown[key_straferight])
    side += sidemove[speed];

    if (gamekeydown[key_strafeleft])
    side -= sidemove[speed];

    // buttons
    cmd->chatchar = HU_dequeueChatChar();

    if (gamekeydown[key_fire] || mousebuttons[mousebfire] || joybuttons[joybfire])
    cmd->buttons |= BT_ATTACK;

    if (gamekeydown[key_use] || joybuttons[joybuse] )
    {
        cmd->buttons |= BT_USE;
        // clear double clicks if hit use button
        dclicks = 0;
    }

    // chainsaw overrides
    for (i=0 ; i<NUMWEAPONS-1 ; i++)
    if (gamekeydown['1'+i])
    {
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= i<<BT_WEAPONSHIFT;
        break;
    }

    // mouse
    if (mousebuttons[mousebforward])
    forward += forwardmove[speed];

    // forward double click
    if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1 ) 
    { 
        dclickstate = mousebuttons[mousebforward];
        if (dclickstate)
        dclicks++;
    if (dclicks == 2)
    {
        cmd->buttons |= BT_USE;
        dclicks = 0;
    }
	else
        dclicktime = 0;
    }
    else
    {
        dclicktime += ticdup;

        if (dclicktime > 20)
        {
            dclicks = 0;
            dclickstate = 0;
        }
    }

    // strafe double click
    bstrafe = mousebuttons[mousebstrafe] || joybuttons[joybstrafe]; 
    if (bstrafe != dclickstate2 && dclicktime2 > 1)
    {
        dclickstate2 = bstrafe;

        if (dclickstate2) 
        dclicks2++; 

        if (dclicks2 == 2)
        {
        cmd->buttons |= BT_USE;
        dclicks2 = 0;
        }
        else
        dclicktime2 = 0;
    }
    else
    {
        dclicktime2 += ticdup;

        if (dclicktime2 > 20)
        {
        dclicks2 = 0;
        dclickstate2 = 0;
        }
    } 

    if (strafe)
    side += mousex*2;
    else
    cmd->angleturn -= mousex*0x8;

    // [JN] Mouselook: toggling
    if (gamekeydown[key_mouselook])
    {
        mlook ^= 1;
        if (mlook)
        {
            look = TOCENTER;
        }
        players[consoleplayer].message_system = mlook ? 
                               ststr_mlook_on : ststr_mlook_off;
        S_StartSound(NULL, sfx_swtchn);
        gamekeydown[key_mouselook] = false;
    }
    
    // [JN] Mouselook: handling
    if (!demoplayback && players[consoleplayer].playerstate == PST_LIVE 
    &&  !paused && !menuactive)
    {
        if (mlook)
        {
            players[consoleplayer].lookdir += mouse_y_invert ? -mousey : mousey;
        }
        else if (!novert)
        {
            forward += mousey;
        }
        
        if (players[consoleplayer].lookdir > 90 * MLOOKUNIT)
            players[consoleplayer].lookdir = 90 * MLOOKUNIT;
        else
        if (players[consoleplayer].lookdir < -110 * MLOOKUNIT)
            players[consoleplayer].lookdir = -110 * MLOOKUNIT;
        
        if (look < 0)
        {
            look += 16;
        }
        cmd->lookfly = look;
    }

    mousex = mousey = 0;

    // [JN] Toggle crosshair
    if (gamekeydown[key_crosshair])
    {
        // [JN] No toggling in -vanilla mode
        if (vanilla)
        return;

        crosshair_draw ^= 1;
        players[consoleplayer].message_system = crosshair_draw ?
                               ststr_crosshair_on : ststr_crosshair_off;
        S_StartSound(NULL,sfx_swtchn);
        gamekeydown[key_crosshair] = false;
    }

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
}
 

//
// G_DoLoadLevel
//
extern  gamestate_t     wipegamestate; 

void G_DoLoadLevel (void)
{
    int     i; 

    // [JN] Properly remove paused state and resume music playing.
    // Fixes a bug when pausing intermission screen causes locking up sound.
    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

    // DOOM determines the sky texture to be used
    // depending on the current episode, and the game version.

    // [JN] Fixed "Sky never changes in Doom II" bug
    if (commercial)
    {
        skytexture = R_TextureNumForName ("SKY3");

        if (gamemap < 12)
        skytexture = R_TextureNumForName ("SKY1");
        else if (gamemap < 21)
        skytexture = R_TextureNumForName ("SKY2");
    }

    levelstarttic = gametic;        // for time calculation

    if (wipegamestate == GS_LEVEL)
    wipegamestate = -1;             // force a wipe 

    gamestate = GS_LEVEL;

    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
        if (playeringame[i] && players[i].playerstate == PST_DEAD)
        players[i].playerstate = PST_REBORN;

        memset (players[i].frags,0,sizeof(players[i].frags));
    }

    P_SetupLevel (gameepisode, gamemap, 0, gameskill);
    displayplayer = consoleplayer;  // view the guy you are playing
    starttime = I_GetTime ();
    gameaction = ga_nothing;
    Z_CheckHeap ();

    // clear cmd building stuff
    memset (gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = 0;
    mousex = mousey = 0;
    sendpause = sendsave = paused = false;
    memset (mousebuttons, 0, sizeof(mousebuttons));
    memset (joybuttons, 0, sizeof(joybuttons));
}


//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//
boolean G_Responder (event_t* ev)
{
    // allow spy mode changes even during the demo
    if (gamestate == GS_LEVEL && ev->type == ev_keydown && ev->data1 == KEY_F12 && (singledemo || !deathmatch))
    {
        // spy mode 
        do 
        {
        displayplayer++;
        if (displayplayer == MAXPLAYERS)
        displayplayer = 0;
        } while (!playeringame[displayplayer] && displayplayer != consoleplayer);

        return true; 
    }

    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && !singledemo && (demoplayback || gamestate == GS_DEMOSCREEN))
    { 
        if (ev->type == ev_keydown || (ev->type == ev_mouse && ev->data1) || (ev->type == ev_joystick && ev->data1))
        {
            M_StartControlPanel ();
            return true;
        }

        return false;
    }

    if (gamestate == GS_LEVEL)
    {
        if (HU_Responder (ev))
        return true;    // chat ate the event
        if (ST_Responder (ev))
        return true;    // status window ate it 
        if (AM_Responder (ev)) 
        return true;    // automap ate it 
    }

    if (gamestate == GS_FINALE)
    {
        if (F_Responder (ev))
        return true;    // finale ate the event
    }

    switch (ev->type)
    {
        case ev_keydown:
        if (ev->data1 == KEY_PAUSE)
        {
            sendpause = true;
            return true;
        }

        if (ev->data1 <NUMKEYS)
        gamekeydown[ev->data1] = true;
        return true;    // eat key down events

        case ev_keyup:
        if (ev->data1 <NUMKEYS)
        gamekeydown[ev->data1] = false;
        return false;   // always let key up events filter down

        case ev_mouse:
        mousebuttons[0] = ev->data1 & 1;
        mousebuttons[1] = ev->data1 & 2;
        mousebuttons[2] = ev->data1 & 4;
        mousex = ev->data2*(mouseSensitivity+5)/10;
        mousey = ev->data3*(mouseSensitivity+5)/10;
        return true;    // eat events

        case ev_joystick: 
        joybuttons[0] = ev->data1 & 1;
        joybuttons[1] = ev->data1 & 2;
        joybuttons[2] = ev->data1 & 4;
        joybuttons[3] = ev->data1 & 8;
        joyxmove = ev->data2;
        joyymove = ev->data3;
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
    int         i;
    int         buf;
    ticcmd_t*   cmd;

    // do player reborns if needed
    for (i=0 ; i<MAXPLAYERS ; i++)
	if (playeringame[i] && players[i].playerstate == PST_REBORN)
    G_DoReborn(i);

    // do things to change the game state
    while (gameaction != ga_nothing)
    {
        switch (gameaction)
        {
            case ga_loadlevel:
            G_DoLoadLevel();
            break;

            case ga_newgame:
            G_DoNewGame();
            break;

            case ga_loadgame:
            G_DoLoadGame();
            break;

            case ga_savegame:
            G_DoSaveGame();
            break;

            case ga_playdemo:
            G_DoPlayDemo();
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

            case ga_screenshot:
            M_ScreenShot ();
            gameaction = ga_nothing;
            break;

            case ga_nothing:
            break;
        }
    }

    // get commands, check consistancy,
    // and build new consistancy check
    buf = (gametic/ticdup)%BACKUPTICS;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            cmd = &players[i].cmd;
            memcpy (cmd, &netcmds[i][buf], sizeof(ticcmd_t)); 

            if (demoplayback)
            G_ReadDemoTiccmd (cmd);
            if (demorecording)
            G_WriteDemoTiccmd (cmd);

            // check for turbo cheats
            if (cmd->forwardmove > TURBOTHRESHOLD && !(gametic&31) && ((gametic>>5)&3) == i)
            {
                static char turbomessage[80];
                extern char *player_names[4];
                // [JN] Only English left in netgame chat, do not translate:
                sprintf (turbomessage, "%s is turbo!",
                         player_names[i]);
                players[consoleplayer].message_chat = turbomessage;
            }

            if (netgame && !netdemo && !(gametic%ticdup))
            {
                if (gametic > BACKUPTICS && consistancy[i][buf] != cmd->consistancy)
                I_Error (english_language ?
                         "consistency failure (%i should be %i)" :
                         "Нарушение последовательности (%i должно быть %i)",
                         cmd->consistancy, consistancy[i][buf]); 

                if (players[i].mo)
                consistancy[i][buf] = players[i].mo->x;
                else
                consistancy[i][buf] = rndindex;
            }
        }
    }

    // check for special buttons
    for (i=0 ; i<MAXPLAYERS ; i++)
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
                    strcpy (savedescription, "NET GAME");
                    savegameslot = (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT;
                    gameaction = ga_savegame;
                    break; 
                }
            }
        }
    }

    // do main actions
    switch (gamestate) 
    { 
        case GS_LEVEL:
        P_Ticker();
        ST_Ticker();
        AM_Ticker();
        HU_Ticker();
        break;

        case GS_INTERMISSION:
        WI_Ticker();
        break;

        case GS_FINALE:
        F_Ticker();
        break;

        case GS_DEMOSCREEN:
        D_PageTicker();
        break;
    }
}


/*
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
    player_t*   p;

    // set up the saved info         
    p = &players[player]; 

    // clear everything else to defaults 
    G_PlayerReborn (player); 
} 
*/


//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel (int player)
{
    extern int st_palette;
    player_t*   p;
    p = &players[player];

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
    player_t*   p;
    int         i;
    int         frags[MAXPLAYERS];
    int         killcount;
    int         itemcount;
    int         secretcount;

    memcpy (frags,players[player].frags,sizeof(frags));
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;

    p = &players[player];
    memset (p, 0, sizeof(*p));

    memcpy (players[player].frags, frags, sizeof(players[player].frags));
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->health_neg = 0; // [JN] Negative player health, reinitialize value
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = 50;

    for (i=0 ; i<NUMAMMO ; i++)
    p->maxammo[i] = maxammo[i];
}


//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//
void P_SpawnPlayer (mapthing_t* mthing);

boolean G_CheckSpot (int playernum, mapthing_t* mthing) 
{
    fixed_t         x;
    fixed_t         y;
    subsector_t*    ss;
    signed int      an; //
    fixed_t         xa; // [JN] For correct MT_FOG spawning, see below
    fixed_t         ya; //
    mobj_t*         mo;
    int             i;

    if (!players[playernum].mo)
    {
        // first spawn of level, before corpses
        for (i=0 ; i<playernum ; i++)
        if (players[i].mo->x == mthing->x << FRACBITS && players[i].mo->y == mthing->y << FRACBITS)
        return false;
        return true;
    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (!P_CheckPosition (players[playernum].mo, x, y))
    return false;

    // flush an old corpse if needed
    if (bodyqueslot >= BODYQUESIZE)
    P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]);

    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo;
    bodyqueslot++;

    // spawn a teleport fog
    // [JN] Now spawns in all possible directions of the spawn spots. Adapted from PrBoom+.
    ss = R_PointInSubsector (x,y);
    an = (ANG45 >> ANGLETOFINESHIFT) * ((signed int) mthing->angle / 45);

    xa = finecosine[an];
    ya = finesine[an];

    mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya, ss->sector->floorheight, MT_TFOG);

    if (players[consoleplayer].viewz != 1)
    S_StartSound (mo, sfx_telept);  // don't start sound on first frame 

    return true;
}


//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
void G_DeathMatchSpawnPlayer (int playernum)
{
    int     i,j;
    int     selections;

    selections = deathmatch_p - deathmatchstarts;
    if (selections < 4)
    {
        if (english_language)
        {
            I_Error ("Only %i deathmatch spots, 4 required", selections);
        }
        else
        {
            I_Error ("Обнаружено %i стартовых точек для режима Дефтатч.\n"
                     "Минимальное необходимое количество: 4", selections);
        }
    }

    for (j=0 ; j<20 ; j++) 
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
    int     i; 

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

        if (G_CheckSpot (playernum, &playerstarts[playernum]))
        {
            P_SpawnPlayer (&playerstarts[playernum]);
            return;
        }

        // try to spawn at one of the other players spots
        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (G_CheckSpot (playernum, &playerstarts[i]))
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
int pars[6][10] =
{
    {0}, 
    {0,30,75,120,90,165,180,180,30,165}, 
    {0,90,90,90,120,90,360,240,30,170}, 
    {0,90,45,90,150,90,90,165,30,135}, 
    {0,165,255,135,150,180,390,135,360,180}, // [crispy] episode 4 par times from the BFG Edition
    {0,90,150,360,420,780,420,780,300,660}   // [JN] Sigil par times
};

// DOOM II Par Times
int cpars[32] =
{
    30,90,120,120,90,150,120,120,270,90,        //  1-10
    210,150,150,150,210,150,420,150,210,150,    // 11-20
    240,150,180,150,150,300,330,420,300,180,    // 21-30
    120,30                                      // 31-32
};


//
// G_DoCompleted
//
boolean         secretexit;
extern char*    pagename;

void G_ExitLevel (void)
{
    secretexit = false;
    gameaction = ga_completed;
}

// Here's for the german edition.
void G_SecretExitLevel (void)
{
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    if ((commercial) && (W_GetNumForName("map31")<0))
    secretexit = false;
    else
    secretexit = true;
    gameaction = ga_completed;
}

void G_DoCompleted (void)
{ 
    int     i; 

    gameaction = ga_nothing; 

    for (i=0 ; i<MAXPLAYERS ; i++) 
    if (playeringame[i]) 
    G_PlayerFinishLevel(i);    // take away cards and stuff 

    if (automapactive)
    AM_Stop();

    if (!commercial)
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

    wminfo.didsecret = players[consoleplayer].didsecret;
    wminfo.epsd = gameepisode -1;
    wminfo.last = gamemap -1;

    // wminfo.next is 0 biased, unlike gamemap
    if (commercial)
    {
        if (secretexit)
        switch(gamemap)
        {
            case 15: wminfo.next = 30; break;
            case 31: wminfo.next = 31; break;
        }
        else
        switch(gamemap)
        {
            case 31:
            case 32: wminfo.next = 15; break;
            default: wminfo.next = gamemap;
        }
    }
    else
    {
        if (secretexit)
        wminfo.next = 8;    // go to secret level
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

    if (commercial)
    wminfo.partime = 35*cpars[gamemap-1];
    else
    wminfo.partime = 35*pars[gameepisode][gamemap];
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

    if (statcopy)
    memcpy (statcopy, &wminfo, sizeof(wminfo));

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

    if (commercial)
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
            F_StartFinale();
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
    gamestate = GS_LEVEL;
    gamemap = wminfo.next+1;
    G_DoLoadLevel();
    gameaction = ga_nothing;
    viewactive = true;
}


//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
extern boolean setsizeneeded;

void R_ExecuteSetViewSize (void);

char    savename[256];

void G_LoadGame (char* name) 
{
    strcpy (savename, name);
    gameaction = ga_loadgame;
}

#define VERSIONSIZE     16


void G_DoLoadGame (void) 
{ 
    int     length;
    int     i;
    int     a,b,c;
    int     d,e,f;
    char    vcheck[VERSIONSIZE];

    gameaction = ga_nothing;

    length = M_ReadFile (savename, &savebuffer);
    save_p = savebuffer + SAVESTRINGSIZE;

    // skip the description field
    memset (vcheck,0,sizeof(vcheck));
    sprintf (vcheck,"version %i",VERSION);
    if (strcmp ((char*)save_p, vcheck))
	return; // bad version 
    save_p += VERSIONSIZE;

    gameskill   = *save_p++;
    gameepisode = *save_p++;
    gamemap     = *save_p++;
    flag667     = *save_p++;
    for (i=0 ; i<MAXPLAYERS ; i++)
	playeringame[i] = *save_p++;

    // load a base level
    G_InitNew (gameskill, gameepisode, gamemap);

    // get the times
    a = *save_p++;
    b = *save_p++;
    c = *save_p++;
    leveltime = (a<<16) + (b<<8) + c;

    // [JN] Get total level times
    d = *save_p++;
    e = *save_p++;
    f = *save_p++;
    totalleveltimes = (d<<16) + (e<<8) + f;

    // dearchive all the modifications
    P_UnArchivePlayers();
    P_UnArchiveWorld();
    P_UnArchiveThinkers();
    P_UnArchiveSpecials();

    if (*save_p != 0x1d)
	I_Error (english_language ? 
             "Bad savegame" :
             "Некорректный файл сохраненной игры.");

    // done
    Z_Free (savebuffer);

    if (setsizeneeded)
    R_ExecuteSetViewSize();

    // [JN] Additional message for loaded game ("Игра загружена.")
    players[consoleplayer].message_system = ggloaded;

    // draw the pattern into the back screen
    if (screenblocks < 10) // [JN] Invoke only for appropriate screen sizes
    R_FillBackScreen ();
}


//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//
void
G_SaveGame (int slot, char* description)
{
    savegameslot = slot;
    strcpy (savedescription, description);
    sendsave = true;
}

void G_DoSaveGame (void)
{
    char    name[100];
    char    name2[VERSIONSIZE];
    char*   description;
    int     length;
    int     i;

    if (M_CheckParm("-cdrom"))
    sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.sav",savegameslot);
    else
    sprintf (name,SAVEGAMENAME"%d.sav",savegameslot);
    description = savedescription;

    // [JN] Perfect solution by Alexandre-Xavier Labonte-Lamoureux
    // Safely increases saveing buffer a lot, so now it is possible
    // to get Doom+ save gamelimit (x16) without any pain.
    save_p = savebuffer = saveshit;

    memcpy (save_p, description, SAVESTRINGSIZE);
    save_p += SAVESTRINGSIZE;
    memset (name2,0,sizeof(name2));
    sprintf (name2,"version %i",VERSION);
    memcpy (save_p, name2, VERSIONSIZE);
    save_p += VERSIONSIZE;

    *save_p++ = gameskill;
    *save_p++ = gameepisode;
    *save_p++ = gamemap;
    *save_p++ = flag667;
    for (i=0 ; i<MAXPLAYERS ; i++)
    *save_p++ = playeringame[i];
    *save_p++ = leveltime>>16;
    *save_p++ = leveltime>>8;
    *save_p++ = leveltime;
    *save_p++ = totalleveltimes>>16;
    *save_p++ = totalleveltimes>>8;
    *save_p++ = totalleveltimes;

    P_ArchivePlayers();
    P_ArchiveWorld();
    P_ArchiveThinkers();
    P_ArchiveSpecials();

    *save_p++ = 0x1d;   // consistancy marker

    length = save_p - savebuffer;
    if (length > SAVEGAMESIZE)
    {
        I_Error (english_language ?
                 "Savegame buffer overrun" :
                 "Ошибка переполнения буфера сохраненной игры.");
    }
    M_WriteFile (name, savebuffer, length);
    gameaction = ga_nothing;
    savedescription[0] = 0;

    players[consoleplayer].message_system = ggsaved;

    // draw the pattern into the back screen
    if (screenblocks < 10) // [JN] Invoke only for appropriate screen sizes
    R_FillBackScreen();
}


//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//
skill_t d_skill; 
int     d_episode; 
int     d_map; 

void G_DeferedInitNew (skill_t skill, int episode, int map)
{
    d_skill = skill;
    d_episode = episode;
    d_map = map;
    gameaction = ga_newgame;
    flag667 = false;
}


void G_DoNewGame (void)
{
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


// The sky texture to be used instead of the F_SKY1 dummy.
extern int  skytexture;


void G_InitNew (skill_t skill, int episode, int map)
{
    int     i;
    // [crispy] make sure "fast" parameters are really only applied once
    static boolean fast_applied;

    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

    // [JN] Increased from sk_nightmare to sk_ultranm
    if (skill > sk_ultranm)
    skill = sk_ultranm;

    if (episode == 0)
    episode = 4;

    if (episode > 1 && shareware)
    episode = 1;

    if (map < 1)
    map = 1;

    if ((map > 9) && (!commercial))
    map = 9; 

    M_ClearRandom ();

    if (skill == sk_nightmare || respawnparm)
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

    usergame = true;        // will be set false if a demo 
    paused = false;
    demoplayback = false;
    automapactive = false;
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;

    // [crispy] CPhipps - total time for all completed levels
    totalleveltimes = 0;

    viewactive = true;

    // set the sky map for the episode
    if (commercial)
    {
        skytexture = R_TextureNumForName ("SKY3");
        if (gamemap < 12)
        skytexture = R_TextureNumForName ("SKY1");
        else if (gamemap < 21)
        skytexture = R_TextureNumForName ("SKY2");
    }

    else
	switch (episode)
    {
        case 1: 
        skytexture = R_TextureNumForName ("SKY1"); 
        break;

        case 2:
        skytexture = R_TextureNumForName ("SKY2");
        break;

        case 3:
        skytexture = R_TextureNumForName ("SKY3");
        break;

        case 4: // Special Edition sky
        skytexture = R_TextureNumForName ("SKY4");
        break;

        case 5: // [crispy] Sigil
        skytexture = R_TextureNumForName ("SKY5_ZD");
        if (W_GetNumForName("SKY5_ZD") < 0)
        {
            skytexture = R_TextureNumForName ("SKY3");
        }
        break;
    }

    G_DoLoadLevel ();
}


//
// DEMO RECORDING
//
#define DEMOMARKER      0x80

void G_ReadDemoTiccmd (ticcmd_t* cmd) 
{
    if (*demo_p == DEMOMARKER)
    {
        // end of demo data stream
        G_CheckDemoStatus();
        return;
    }

    cmd->forwardmove = ((signed char)*demo_p++);
    cmd->sidemove = ((signed char)*demo_p++);
    cmd->angleturn = ((unsigned char)*demo_p++)<<8;
    cmd->buttons = (unsigned char)*demo_p++;
} 


void G_WriteDemoTiccmd (ticcmd_t* cmd)
{
    if (gamekeydown['q'])   // press q to end demo recording 
    G_CheckDemoStatus ();

    *demo_p++ = cmd->forwardmove;
    *demo_p++ = cmd->sidemove;
    *demo_p++ = (cmd->angleturn+128)>>8;
    *demo_p++ = cmd->buttons;
    demo_p -= 4;

    if (demo_p > demoend - 16)
    {
        // no more space 
        G_CheckDemoStatus (); 
        return; 
    }

    G_ReadDemoTiccmd (cmd); // make SURE it is exactly the same 
}
 

//
// G_RecordDemo
//
void G_RecordDemo (char* name) 
{
    int     i; 
    int     maxsize;

    usergame = false;
    strcpy (demoname, name);
    strcat (demoname, ".lmp");
    maxsize = 0x20000;
    i = M_CheckParm ("-maxdemo");

    if (i && i<myargc-1)
    maxsize = atoi(myargv[i+1])*1024;
    demobuffer = Z_Malloc (maxsize,PU_STATIC,NULL);
    demoend = demobuffer + maxsize;
    demorecording = true; 
}


void G_BeginRecording (void)
{
    int     i;

    demo_p = demobuffer;
	
    *demo_p++ = VERSION;
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

void G_DoPlayDemo (void)
{
    skill_t     skill;
    int         i, episode, map;

    gameaction = ga_nothing;
    demobuffer = demo_p = W_CacheLumpName (defdemoname, PU_STATIC);

    if ( *demo_p++ < 106)
    I_Error(english_language ? 
            "Demo is from a different game version!" :
            "Демозапись от другой версии игры!");

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
    if (playeringame[1])
    {
        netgame = true;
        netdemo = true;
    }

    // don't spend a lot of time in loadlevel
    precache = false;
    G_InitNew (skill, episode, map);
    precache = true;

    usergame = false;
    demoplayback = true;
}


//
// G_TimeDemo
//
void G_TimeDemo (char* name)
{
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
    int     endtime; 

    // [JN] -timedemo now able to show average FPS.
    // Adapted from Chocolate Doom.
    if (timingdemo) 
    { 
        float   fps;
        int     realtics;

        endtime = I_GetTime (); 
        realtics = endtime - starttime;
        fps = ((float) gametic * TICRATE) / realtics;
        
        // Prevent recursive calls
        timingdemo = false;
        demoplayback = false;        
        
        if (english_language)
        {
            I_Error ("timed %i gametics in %i realtics.\n"
                     "Average FPS: %f.", gametic, realtics, fps); 
        }
        else
        {
            I_Error ("Насчитано %i gametics в %i realtics.\n" 
                    "Среднее значение FPS: %f.", gametic, realtics, fps);
        }
    } 

    if (demoplayback) 
    { 
        if (singledemo) 
        I_Quit (); 

        Z_ChangeTag (demobuffer, PU_CACHE);
        demoplayback = false;
        netdemo = false;
        netgame = false;
        deathmatch = false;
        playeringame[1] = playeringame[2] = playeringame[3] = 0;
        respawnparm = false;
        fastparm = false;
        nomonsters = false;
        consoleplayer = 0;
        D_AdvanceDemo();
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
                 "Демозапись %s завершена.",
                 demoname);
    }

    return false;
}

