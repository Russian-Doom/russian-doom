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



// G_game.c

#include <stdlib.h>
#include <string.h>
#include "am_map.h"
#include "doomdef.h"
#include "deh_str.h"
#include "i_timer.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "rd_keybinds.h"
#include "s_sound.h"
#include "v_video.h"
#include "jn.h"

// Functions

boolean G_CheckDemoStatus(void);
void G_ReadDemoTiccmd(ticcmd_t * cmd);
void G_WriteDemoTiccmd(ticcmd_t * cmd);
void G_PlayerReborn(int player);

void G_DoReborn(int playernum);

void G_DoLoadLevel(void);
void G_DoNewGame(void);
void G_DoPlayDemo(void);
void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);
void G_DoSaveGame(void);

void D_PageTicker(void);
void D_AdvanceDemo(void);

struct
{
    int type;   // mobjtype_t
    int speed[2];
} MonsterMissileInfo[] = {
    { MT_IMPBALL, { 10, 20 } },
    { MT_MUMMYFX1, { 9, 18 } },
    { MT_KNIGHTAXE, { 9, 18 } },
    { MT_REDAXE, { 9, 18 } },
    { MT_BEASTBALL, { 12, 20 } },
    { MT_WIZFX1, { 18, 24 } },
    { MT_SNAKEPRO_A, { 14, 20 } },
    { MT_SNAKEPRO_B, { 14, 20 } },
    { MT_HEADFX1, { 13, 20 } },
    { MT_HEADFX3, { 10, 18 } },
    { MT_MNTRFX1, { 20, 26 } },
    { MT_MNTRFX2, { 14, 20 } },
    { MT_SRCRFX1, { 20, 28 } },
    { MT_SOR2FX1, { 20, 28 } },
    { -1, { -1, -1 } }                 // Terminator
};

gameaction_t gameaction;
gamestate_t gamestate;
skill_t gameskill;
boolean respawnmonsters;
int gameepisode;
int gamemap;
int prevmap;

boolean paused;
boolean sendpause;              // send a pause event next tic
boolean sendsave;               // send a save event next tic
boolean usergame;               // ok to save / end game

boolean timingdemo;             // if true, exit with report on completion
int starttime;                  // for comparative timing purposes

boolean viewactive;

boolean deathmatch;             // only if started as net death
boolean netgame;                // only true if packets are broadcast
boolean playeringame[MAXPLAYERS];
player_t players[MAXPLAYERS];

int consoleplayer;              // player taking events and displaying
int displayplayer;              // view being displayed
int levelstarttic;              // gametic at level start
int totalkills, totalitems, totalsecret;        // for intermission
int totalleveltimes;            // [crispy] CPhipps - total time for all completed levels
int totaltimes;

char *demoname;
boolean demorecording;
boolean longtics;               // specify high resolution turning in demos
boolean lowres_turn;
boolean shortticfix;            // calculate lowres turning like doom
boolean demoplayback;
boolean demoextend;
byte *demobuffer, *demo_p, *demoend;
boolean singledemo;             // quit after playing a demo from cmdline

boolean precache = true;        // if true, load all graphics at start

// TODO: Heretic uses 16-bit shorts for consistency?
byte consistancy[MAXPLAYERS][BACKUPTICS];
char *savegamedir;

boolean testcontrols = false;
int testcontrols_mousespeed;


//
// controls (have defaults)
//


#define MAXPLMOVE       0x32

fixed_t forwardmove[2] = { 0x19, 0x32 };
fixed_t sidemove[2] = { 0x18, 0x28 };
fixed_t angleturn[3] = { 640, 1280, 320 };      // + slow turn

// [JN] If true, activate maximum weapon bobbing
boolean max_bobbing;

static bound_key_t weapon_keys[] =
{
    bk_weapon_1,
    bk_weapon_2,
    bk_weapon_3,
    bk_weapon_4,
    bk_weapon_5,
    bk_weapon_6,
    bk_weapon_7
};

// Set to -1 or +1 to switch to the previous or next weapon.

static int next_weapon = 0;

// Used for prev/next weapon keys.

static const struct
{
    weapontype_t weapon;
    weapontype_t weapon_num;
} weapon_order_table[] = {
    { wp_staff,       wp_staff },
    { wp_gauntlets,   wp_staff },
    { wp_goldwand,    wp_goldwand },
    { wp_crossbow,    wp_crossbow },
    { wp_blaster,     wp_blaster },
    { wp_skullrod,    wp_skullrod },
    { wp_phoenixrod,  wp_phoenixrod },
    { wp_mace,        wp_mace },
    { wp_beak,        wp_beak },
};

#define SLOWTURNTICS    6

int turnheld;                   // for accelerative turning
int lookheld;

int mousex, mousey;             // mouse values are used once

#define MAX_JOY_BUTTONS 20

int joyturn, joymove;         // joystick values are repeated
int joystrafemove;
int joyvlook;
int alwaysRun = 1;              // is always run enabled

int savegameslot;
char savedescription[32];

int inventoryTics;

// haleyjd: removed WATCOMC

static boolean WeaponSelectable(weapontype_t weapon)
{
    if (weapon == wp_beak)
    {
        return false;
    }

    return players[consoleplayer].weaponowned[weapon];
}

static int G_NextWeapon(int direction)
{
    weapontype_t weapon;
    int start_i, i;

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

/*
====================
=
= G_BuildTiccmd
=
= Builds a ticcmd from all of the available inputs or reads it from the
= demo buffer.
= If recording a demo, write it out
====================
*/

extern boolean inventory;
extern int curpos;
extern int inv_ptr;

boolean usearti = true;

void G_BuildTiccmd(ticcmd_t *cmd, int maketic)
{
    int i;
    boolean strafe;
    int speed, tspeed, lspeed;
    int forward, side;
    int look, arti;
    int flyheight;
    extern boolean askforquit;
    extern boolean noartiskip;

    // haleyjd: removed externdriver crap

    memset(cmd, 0, sizeof(*cmd));
    cmd->consistancy = consistancy[consoleplayer][maketic % BACKUPTICS];

    strafe = BK_isKeyPressed(bk_strafe);
		
	// [crispy] when "always run" is active,
    // pressing the "run" key will result in walking
    speed = alwaysRun;
	
	// [JN] Run button modifier
	speed ^= BK_isKeyPressed(bk_speed);

    // haleyjd: removed externdriver crap
    
    forward = side = look = arti = flyheight = 0;

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
        tspeed = 2;             // slow turn
    }
    else
    {
        tspeed = speed;
    }

    if (BK_isKeyPressed(bk_look_down) || BK_isKeyPressed(bk_look_up))
    {
        lookheld += ticdup;
    }
    else
    {
        lookheld = 0;
    }

    if (lookheld < SLOWTURNTICS)
    {
        lspeed = 1;
    }
    else
    {
        lspeed = 2;
    }

    // [crispy] toggle always run
    if (BK_isKeyPressed(bk_toggle_autorun))
    {
        alwaysRun ^= 1;

        // [JN] Added audible feedback
        P_SetMessage(&players[consoleplayer], alwaysRun ?
                     txt_alwaysrun_on : txt_alwaysrun_off, msg_system, false);
    
        S_StartSound(NULL, sfx_chat);

        BK_ReleaseKey(bk_toggle_autorun);
    }

    // [JN] Crosshair toggling. Disalloved in vanilla mode (no crosshair there).
    if (BK_isKeyPressed(bk_toggle_crosshair) && !vanillaparm)
    {
        crosshair_draw ^= 1;

        P_SetMessage(&players[consoleplayer], crosshair_draw ? 
                     txt_crosshair_on : txt_crosshair_off, msg_system, false);

        S_StartSound(NULL, sfx_chat);

        BK_ReleaseKey(bk_toggle_crosshair);
    }

    // [JN] Toggle level flipping.
    if (BK_isKeyPressed(bk_toggle_fliplvls))
    {
        flip_levels ^= 1;
        R_ExecuteSetViewSize();       // Redraw game screen
        S_StartSound(NULL, sfx_chat); // Play quiet sound

        BK_ReleaseKey(bk_toggle_fliplvls);
    }

//
// let movement keys cancel each other out
//
    if (strafe)
    {
        if (BK_isKeyPressed(bk_turn_right))
            side += sidemove[speed];
        if (BK_isKeyPressed(bk_turn_left))
            side -= sidemove[speed];
        if (joyturn != 0)
            side += joyturn;
        if(mousex != 0)
            side += mousex*2;
    }
    else
    {
        if (BK_isKeyPressed(bk_turn_right))
            cmd->angleturn -= angleturn[tspeed];
        if (BK_isKeyPressed(bk_turn_left))
            cmd->angleturn += angleturn[tspeed];
        if (joyturn != 0)
            cmd->angleturn -= joyturn;
        if(mousex != 0)
            cmd->angleturn -= mousex*0x8;
    }

    if (BK_isKeyPressed(bk_forward))
        forward += forwardmove[speed];
    if (BK_isKeyPressed(bk_backward))
        forward -= forwardmove[speed];
    if(joymove != 0)
        forward += joymove;
    if (BK_isKeyPressed(bk_strafe_right))
        side += sidemove[speed];
    if (BK_isKeyPressed(bk_strafe_left))
        side -= sidemove[speed];
    if(joystrafemove != 0)
        side += joystrafemove;

    // Look up/down/center keys
    if (BK_isKeyPressed(bk_look_up))
    {
        look = lspeed;
    }
    if (BK_isKeyPressed(bk_look_down))
    {
        look = -lspeed;
    }
    // haleyjd: removed externdriver crap
    if (BK_isKeyPressed(bk_look_center))
    {
        look = TOCENTER;
    }

    // haleyjd: removed externdriver crap
    
    // Fly up/down/drop keys
    if (BK_isKeyPressed(bk_fly_up))
    {
        flyheight = 5;  // note that the actual flyheight will be twice this
    }
    if (BK_isKeyPressed(bk_fly_down))
    {
        flyheight = -5;
    }
    if (BK_isKeyPressed(bk_fly_stop))
    {
        flyheight = TOCENTER;
        // haleyjd: removed externdriver crap
        look = TOCENTER;
    }

    // Use artifact key
    if (BK_isKeyPressed(bk_inv_use_artifact))
    {
        if (BK_isKeyPressed(bk_speed) && !noartiskip)
        {
            if (players[consoleplayer].inventory[inv_ptr].type != arti_none)
            {
                BK_ReleaseKey(bk_inv_use_artifact);
                cmd->arti = 0xff;       // skip artifact code
            }
        }
        else
        {
            if (inventory)
            {
                players[consoleplayer].readyArtifact =
                    players[consoleplayer].inventory[inv_ptr].type;
                inventory = false;
                cmd->arti = 0;
                usearti = false;
            }
            else if (usearti)
            {
                cmd->arti = players[consoleplayer].inventory[inv_ptr].type;
                usearti = false;
            }
        }
    }
    // [JN] Keys for Artifacts
    if (BK_isKeyPressed(bk_arti_quartz) && !cmd->arti
             && (players[consoleplayer].mo->health < MAXHEALTH))
    {
        BK_ReleaseKey(bk_arti_quartz);
        cmd->arti = arti_health;
    }
    else if (BK_isKeyPressed(bk_arti_urn) && !cmd->arti
             && (players[consoleplayer].mo->health < MAXHEALTH))
    {
        BK_ReleaseKey(bk_arti_urn);
        cmd->arti = arti_superhealth;
    }
    else if (BK_isKeyPressed(bk_arti_bomb) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_bomb);
        cmd->arti = arti_firebomb;
    }
    else if (BK_isKeyPressed(bk_arti_tome) && !cmd->arti
             && !players[consoleplayer].powers[pw_weaponlevel2])
    {
        BK_ReleaseKey(bk_arti_tome);
        cmd->arti = arti_tomeofpower;
    }
    else if (BK_isKeyPressed(bk_arti_invulnerability) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_invulnerability);
        cmd->arti = arti_invulnerability;
    }
    else if (BK_isKeyPressed(bk_arti_egg) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_egg);
        cmd->arti = arti_egg;
    }
    else if (BK_isKeyPressed(bk_arti_chaosdevice) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_chaosdevice);
        cmd->arti = arti_teleport;
    }
    else if (BK_isKeyPressed(bk_arti_shadowsphere) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_shadowsphere);
        cmd->arti = arti_invisibility;
    }
    else if (BK_isKeyPressed(bk_arti_wings) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_wings);
        cmd->arti = arti_fly;
    }
    else if (BK_isKeyPressed(bk_arti_torch) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_torch);
        cmd->arti = arti_torch;
    }

//
// buttons
//
    cmd->chatchar = CT_dequeueChatChar();

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
    // (Can't weapon cycle when the player is a chicken)

    if (gamestate == GS_LEVEL
     && players[consoleplayer].chickenTics == 0 && next_weapon != 0)
    {
        i = G_NextWeapon(next_weapon);
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= i << BT_WEAPONSHIFT;
    }
    else
    {
        for (i=0; i<arrlen(weapon_keys); ++i)
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

//
// mouse
//
    // No mouse movement in previous frame?
    if (mousex == 0)
    {
        testcontrols_mousespeed = 0;
    }

    // [JN] Mouselook: initials
    // TODO: make it safe for network game
    if (players[consoleplayer].playerstate == PST_LIVE && !netgame 
    && !demoplayback && !menuactive && !askforquit && !paused)
    {
        if (mlook || novert)
        {
            cmd->lookdir += mouse_y_invert ? -mousey : mousey;
            cmd->lookdir += joyvlook;
        }
        else if (!novert)
        {
            forward += mousey;
            forward += joyvlook;
        }
        
         if (players[consoleplayer].lookdir > LOOKDIRMAX * MLOOKUNIT)
             players[consoleplayer].lookdir = LOOKDIRMAX * MLOOKUNIT;
         else if (players[consoleplayer].lookdir < -LOOKDIRMIN * MLOOKUNIT)
             players[consoleplayer].lookdir = -LOOKDIRMIN * MLOOKUNIT;
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

        P_SetMessage(&players[consoleplayer], (mlook == true ?
                     txt_mlook_on : txt_mlook_off), msg_system, false);

        S_StartSound(NULL, sfx_chat);

        BK_ReleaseKey(bk_toggle_mlook);
    }

    mousex = mousey = joyturn = joyvlook = 0;

    // [JN] "false" must be set as initial and returning condition.
    max_bobbing = false; 

    if (forward > MAXPLMOVE)
    {
        forward = MAXPLMOVE;
        max_bobbing = true;
    }
    else if (forward < -MAXPLMOVE)
    {
        forward = -MAXPLMOVE;
        max_bobbing = false;
    }
    if (side > MAXPLMOVE)
    {
        side = MAXPLMOVE;
        max_bobbing = true;
    }
    else if (side < -MAXPLMOVE)
    {
        side = -MAXPLMOVE;
        max_bobbing = false; 
    }

    cmd->forwardmove += forward;
    cmd->sidemove += side;
    if (players[consoleplayer].playerstate == PST_LIVE)
    {
        if (look < 0)
        {
            look += 16;
        }
        cmd->lookfly = look;
    }
    if (flyheight < 0)
    {
        flyheight += 16;
    }
    cmd->lookfly |= flyheight << 4;

//
// special buttons
//
    if (sendpause)
    {
        sendpause = false;
        cmd->buttons = BT_SPECIAL | BTS_PAUSE;
    }

    if (sendsave)
    {
        sendsave = false;
        cmd->buttons =
            BT_SPECIAL | BTS_SAVEGAME | (savegameslot << BTS_SAVESHIFT);
    }

    if (flip_levels)
    {
        cmd->angleturn = -cmd->angleturn;
        cmd->sidemove = -cmd->sidemove;
    }

    if (lowres_turn)
    {
        if (shortticfix)
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
        else
        {
            // truncate angleturn to the nearest 256 boundary
            // for recording demos with single byte values for turn
            cmd->angleturn &= 0xff00;
        }
    }
}


/*
==============
=
= G_DoLoadLevel
=
==============
*/

void G_DoLoadLevel(void)
{
    int i;

    // [JN] Properly remove paused state and resume music playing.
    // Fixes a bug when pausing intermission screen causes locking up sound.
    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

    levelstarttic = gametic;    // for time calculation
    gamestate = GS_LEVEL;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i] && players[i].playerstate == PST_DEAD)
            players[i].playerstate = PST_REBORN;
        memset(players[i].frags, 0, sizeof(players[i].frags));
    }

    // [JN] Wand start
    if (singleplayer && !vanillaparm && pistol_start)
    {
        G_PlayerReborn(0);
    }

    P_SetupLevel(gameepisode, gamemap, 0, gameskill);
    displayplayer = consoleplayer;      // view the guy you are playing
    gameaction = ga_nothing;
    Z_CheckHeap();

//
// clear cmd building stuff
//

    BK_ReleaseAllKeys();
    joyvlook = joyturn = joymove = joystrafemove = 0;
    mousex = mousey = 0;
    sendpause = sendsave = paused = false;

    if (testcontrols)
    {
        P_SetMessage(&players[consoleplayer], txt_testcontrols, msg_system, false);
    }
}

/*
===============================================================================
=
= G_Responder
=
= get info needed to make ticcmd_ts for the players
=
===============================================================================
*/

boolean G_Responder(event_t * ev)
{
    player_t *plr;

    plr = &players[consoleplayer];
    if (BK_isKeyUp(ev, bk_inv_use_artifact))
    {   // flag to denote that it's okay to use an artifact
        if (!inventory)
        {
            plr->readyArtifact = plr->inventory[inv_ptr].type;
        }
        usearti = true;
    }

    // Check for spy mode player cycle
    if (gamestate == GS_LEVEL && BK_isKeyDown(ev, bk_spy) && !deathmatch)
    {                           // Cycle the display player
        do
        {
            displayplayer++;
            if (displayplayer == MAXPLAYERS)
            {
                displayplayer = 0;
            }
        }
        while (!playeringame[displayplayer]
               && displayplayer != consoleplayer);
        return (true);
    }

    if (gamestate == GS_LEVEL)
    {
        if (CT_Responder(ev))
        {                       // Chat ate the event
            return (true);
        }
        if (SB_Responder(ev))
        {                       // Status bar ate the event
            return (true);
        }
        if (AM_Responder(ev))
        {                       // Automap ate the event
            return (true);
        }
    }

    if (ev->type == ev_mouse_move)
    {
        testcontrols_mousespeed = abs(ev->data2);
    }

    if (BK_isKeyDown(ev, bk_weapon_prev))
    {
        next_weapon = -1;
    }
    else if (BK_isKeyDown(ev, bk_weapon_next))
    {
        next_weapon = 1;
    }

    if (BK_isKeyDown(ev, bk_inv_left))
    {
        inventoryTics = 5 * 35;
        if (!inventory)
        {
            inventory = true;
            return false;
        }
        inv_ptr--;
        if (inv_ptr < 0)
        {
            inv_ptr = 0;
        }
        else
        {
            curpos--;
            if (curpos < 0)
            {
                curpos = 0;
            }
        }
        return true;
    }
    if (BK_isKeyDown(ev, bk_inv_right))
    {
        inventoryTics = 5 * 35;
        if (!inventory)
        {
            inventory = true;
            return false;
        }
        inv_ptr++;
        if (inv_ptr >= plr->inventorySlotNum)
        {
            inv_ptr--;
            if (inv_ptr < 0)
                inv_ptr = 0;
        }
        else
        {
            curpos++;
            if (curpos > 6)
            {
                curpos = 6;
            }
        }
        return true;
    }
    if (BK_isKeyDown(ev, bk_pause) && !menuactive)
    {
        sendpause = true;
        return true;
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
            mousex = ev->data2 * (mouseSensitivity + 5) / 10;
            mousey = ev->data3 * (mouseSensitivity + 5) / 10;
            return true;      // eat events

        case ev_controller_move:
            joymove = ev->data1;
            joystrafemove = ev->data2;
            joyturn = ev->data3;
            joyvlook = ev->data4;
            return (true);      // eat events

        default:
            break;
    }
    return (false);
}

/*
===============================================================================
=
= G_Ticker
=
===============================================================================
*/

void G_Ticker(void)
{
    int i, buf;
    ticcmd_t *cmd = NULL;

//
// do player reborns if needed
//
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i] && players[i].playerstate == PST_REBORN)
            G_DoReborn(i);

//
// do things to change the game state
//
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
                // [JN] Reset looking direction if game is loaded w/o mouse look
                if (!mlook)
                    players[consoleplayer].lookdir = 0;
                // [JN] Reset counters for missing key fading effects.
                players[consoleplayer].yellowkeyTics = 0;
                players[consoleplayer].greenkeyTics = 0;
                players[consoleplayer].bluekeyTics = 0;
                // [Dasperal] Init inv_ptr
                for(i = 0; i < players[consoleplayer].inventorySlotNum; i++)
                {
                    if(players[consoleplayer].inventory[i].type == players[consoleplayer].readyArtifact)
                    {
                        inv_ptr = i;
                        break;
                    }
                }
                break;
            case ga_savegame:
                G_DoSaveGame();
                break;
            case ga_playdemo:
                G_DoPlayDemo();
                break;
            case ga_screenshot:  // [JN] Extended name from "HTIC%02i.%s"
                V_ScreenShot("screenshot-heretic-%02i.%s");
                S_StartSound(NULL, sfx_chat);   // [JN] Audio feedback
                gameaction = ga_nothing;
                break;
            case ga_completed:
                G_DoCompleted();
                break;
            case ga_worlddone:
                G_DoWorldDone();
                break;
            case ga_victory:
                F_StartFinale();
                break;
            default:
                break;
        }
    }


//
// get commands, check consistancy, and build new consistancy check
//
    buf = (gametic / ticdup) % BACKUPTICS;

    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i])
        {
            cmd = &players[i].cmd;

            memcpy(cmd, &netcmds[i], sizeof(ticcmd_t));

            if (demoplayback)
                G_ReadDemoTiccmd(cmd);
            if (demorecording)
                G_WriteDemoTiccmd(cmd);

            if (netgame && !(gametic % ticdup))
            {
                if (gametic > BACKUPTICS
                    && consistancy[i][buf] != cmd->consistancy)
                {
                    I_Error(english_language ?
                        "Consistency failure (%i should be %i)" :
                        "Нарушение последовательности (%i должно быть %i)",
                            cmd->consistancy, consistancy[i][buf]);
                }
                if (players[i].mo)
                    consistancy[i][buf] = players[i].mo->x;
                else
                    consistancy[i][buf] = rndindex;
            }
        }

//
// check for special buttons
//
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i])
        {
            if (players[i].cmd.buttons & BT_SPECIAL)
            {
                switch (players[i].cmd.buttons & BT_SPECIALMASK)
                {
                    case BTS_PAUSE:
                        paused ^= 1;
                        if (paused)
                        {
                            S_PauseSound();
                        }
                        else
                        {
                            S_ResumeSound();
                        }
                        break;

                    case BTS_SAVEGAME:
                        if (!savedescription[0])
                        {
                            if (netgame)
                            {
                                // [JN] Internal string, do not translate
                                M_StringCopy(savedescription, 
                                             DEH_String("NET GAME"),
                                             sizeof(savedescription));
                            }
                            else
                            {
                                // [JN] Internal string, do not translate
                                M_StringCopy(savedescription,
                                             DEH_String("SAVE GAME"),
                                             sizeof(savedescription));
                            }
                        }
                        savegameslot =
                            (players[i].cmd.
                             buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
                        gameaction = ga_savegame;
                        break;
                }
            }
        }
    // turn inventory off after a certain amount of time
    if (inventory && !(--inventoryTics))
    {
        players[consoleplayer].readyArtifact =
            players[consoleplayer].inventory[inv_ptr].type;
        inventory = false;
        cmd->arti = 0;
    }
//
// do main actions
//
//
// do main actions
//
    switch (gamestate)
    {
        case GS_LEVEL:
            P_Ticker();
            SB_Ticker();
            AM_Ticker();
            CT_Ticker();
            break;
        case GS_INTERMISSION:
            IN_Ticker();
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
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Things
==============================================================================
*/

/*
====================
=
= G_InitPlayer
=
= Called at the start
= Called by the game initialization functions
====================
*/

void G_InitPlayer(int player)
{
    // clear everything else to defaults
    G_PlayerReborn(player);
}


/*
====================
=
= G_PlayerFinishLevel
=
= Can when a player completes a level
====================
*/
extern int playerkeys;

void G_PlayerFinishLevel(int player)
{
    player_t *p;
    int i;

    p = &players[player];
    for (i = 0; i < p->inventorySlotNum; i++)
    {
        p->inventory[i].count = 1;
    }
    p->artifactCount = p->inventorySlotNum;

    if (!deathmatch)
    {
        for (i = 0; i < 16; i++)
        {
            P_PlayerUseArtifact(p, arti_fly);
        }
    }
    memset(p->powers, 0, sizeof(p->powers));
    memset(p->keys, 0, sizeof(p->keys));
    playerkeys = 0;

    if (p->chickenTics)
    {
        p->readyweapon = p->mo->special1.i;       // Restore weapon
        p->chickenTics = 0;
    }
    p->messageTics = 0;
    p->yellowkeyTics = 0;
    p->greenkeyTics = 0;
    p->bluekeyTics = 0;
    p->lookdir = 0;
    p->mo->flags &= ~MF_SHADOW; // Remove invisibility
    p->extralight = 0;          // Remove weapon flashes
    p->fixedcolormap = 0;       // Remove torch
    p->damagecount = 0;         // No palette changes
    p->bonuscount = 0;
    p->rain1 = NULL;
    p->rain2 = NULL;
    // [JN] Fix Raven's bug: finishing level 9 was not considered as "didsecret".
    if (gamemap == 9)
    {
        p->didsecret = true;
    }
}

/*
====================
=
= G_PlayerReborn
=
= Called after a player dies
= almost everything is cleared and initialized
====================
*/

void G_PlayerReborn(int player)
{
    player_t *p;
    int i;
    int frags[MAXPLAYERS];
    int killcount, itemcount, secretcount;
    boolean secret;

    secret = false;
    memcpy(frags, players[player].frags, sizeof(frags));
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;

    p = &players[player];
    if (p->didsecret)
    {
        secret = true;
    }
    memset(p, 0, sizeof(*p));

    memcpy(players[player].frags, frags, sizeof(players[player].frags));
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = wp_goldwand;
    p->weaponowned[wp_staff] = true;
    p->weaponowned[wp_goldwand] = true;
    p->messageTics = 0;
    p->yellowkeyTics = 0;
    p->greenkeyTics = 0;
    p->bluekeyTics = 0;
    p->lookdir = 0;
    p->ammo[am_goldwand] = 50;
    for (i = 0; i < NUMAMMO; i++)
    {
        p->maxammo[i] = maxammo[i];
    }
    if (gamemap == 9 || secret)
    {
        p->didsecret = true;
    }
    if (p == &players[consoleplayer])
    {
        inv_ptr = 0;            // reset the inventory pointer
        curpos = 0;
    }
}

/*
====================
=
= G_CheckSpot
=
= Returns false if the player cannot be respawned at the given mapthing_t spot
= because something is occupying it
====================
*/

void P_SpawnPlayer(mapthing_t * mthing);

boolean G_CheckSpot(int playernum, mapthing_t * mthing)
{
    fixed_t x, y;
    subsector_t *ss;
    unsigned an;
    mobj_t *mo;

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    players[playernum].mo->flags2 &= ~MF2_PASSMOBJ;
    if (!P_CheckPosition(players[playernum].mo, x, y))
    {
        players[playernum].mo->flags2 |= MF2_PASSMOBJ;
        return false;
    }
    players[playernum].mo->flags2 |= MF2_PASSMOBJ;

// spawn a teleport fog
    ss = R_PointInSubsector(x, y);
    an = ((unsigned) ANG45 * (mthing->angle / 45)) >> ANGLETOFINESHIFT;

    mo = P_SpawnMobj(x + 20 * finecosine[an], y + 20 * finesine[an],
                     ss->sector->floorheight + TELEFOGHEIGHT, MT_TFOG);

    if (players[consoleplayer].viewz != 1)
        S_StartSound(mo, sfx_telept);   // don't start sound on first frame

    return true;
}

/*
====================
=
= G_DeathMatchSpawnPlayer
=
= Spawns a player at one of the random death match spots
= called at level load and each death
====================
*/

void G_DeathMatchSpawnPlayer(int playernum)
{
    int i, j;
    int selections;

    selections = deathmatch_p - deathmatchstarts;
    if (selections < 4)
    {
        if (english_language)
        {
            I_Error("Only %i deathmatch spots, 4 required", selections);
        }
        else
        {
            I_Error ("Обнаружено %i стартовых точек для режима Дефтатч.\n"
                    "Минимальное необходимое количество: 4", selections);
        }
    }

    for (j = 0; j < 20; j++)
    {
        i = P_Random() % selections;
        if (G_CheckSpot(playernum, &deathmatchstarts[i]))
        {
            deathmatchstarts[i].type = playernum + 1;
            P_SpawnPlayer(&deathmatchstarts[i]);
            return;
        }
    }

// no good spot, so the player will probably get stuck
    P_SpawnPlayer(&playerstarts[playernum]);
}

/*
====================
=
= G_DoReborn
=
====================
*/

void G_DoReborn(int playernum)
{
    int i;

    // quit demo unless -demoextend
    if (!demoextend && G_CheckDemoStatus())
        return;

    if (!netgame)
    {
        gameaction = ga_loadlevel;  // reload the level from scratch
    }
    else
    {                               // respawn at the start
        players[playernum].mo->player = NULL;   // dissasociate the corpse

        // spawn at random spot if in death match
        if (deathmatch)
        {
            G_DeathMatchSpawnPlayer(playernum);
            return;
        }

        if (G_CheckSpot(playernum, &playerstarts[playernum]))
        {
            P_SpawnPlayer(&playerstarts[playernum]);
            return;
        }
        // try to spawn at one of the other players spots
        for (i = 0; i < MAXPLAYERS; i++)
            if (G_CheckSpot(playernum, &playerstarts[i]))
            {
                playerstarts[i].type = playernum + 1;   // fake as other player
                P_SpawnPlayer(&playerstarts[i]);
                playerstarts[i].type = i + 1;   // restore
                return;
            }
        // he's going to be inside something.  Too bad.
        P_SpawnPlayer(&playerstarts[playernum]);
    }
}


void G_ScreenShot(void)
{
    gameaction = ga_screenshot;
}


/*
====================
=
= G_DoCompleted
=
====================
*/

boolean secretexit;

void G_ExitLevel(void)
{
    secretexit = false;
    gameaction = ga_completed;
}

void G_SecretExitLevel(void)
{
    secretexit = true;
    gameaction = ga_completed;
}

void G_DoCompleted(void)
{
    int i;
    static int afterSecret[5] = { 7, 5, 5, 5, 4 };

    gameaction = ga_nothing;

    // quit demo unless -demoextend
    if (!demoextend && G_CheckDemoStatus())
    {
        return;
    }
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            G_PlayerFinishLevel(i);
        }
    }
    prevmap = gamemap;
    if (secretexit == true)
    {
        gamemap = 9;
    }
    else if (gamemap == 9)
    {                           // Finished secret level
        gamemap = afterSecret[gameepisode - 1];
    }
    // [crispy] display tally screen after ExM8
    // [JN] Do not display in vanilla game mode.
    else if (gamemap == 8 && vanillaparm)
    {
        gameaction = ga_victory;
        return;
    }
    else
    {
        gamemap++;
    }

    // [crispy] CPhipps - total time for all completed levels
    // cph - modified so that only whole seconds are added to the totalleveltimes
    // value; so our total is compatible with the "naive" total of just adding
    // the times in seconds shown for each level. Also means our total time
    // will agree with Compet-n.
    totaltimes = (totalleveltimes += (leveltime - leveltime % TICRATE));

    gamestate = GS_INTERMISSION;
    IN_Start();
}

//============================================================================
//
// G_WorldDone
//
//============================================================================

void G_WorldDone(void)
{
    gameaction = ga_worlddone;
}

//============================================================================
//
// G_DoWorldDone
//
//============================================================================

void G_DoWorldDone(void)
{
    idmusnum = -1;  // [JN] jff 3/17/98 allow new level's music to be loaded
    gamestate = GS_LEVEL;
    G_DoLoadLevel();
    gameaction = ga_nothing;
    viewactive = true;
    AM_clearMarks();  // [JN] jff 4/12/98 clear any marks on the automap
}

//---------------------------------------------------------------------------
//
// PROC G_LoadGame
//
// Can be called by the startup code or the menu task.
//
//---------------------------------------------------------------------------

static char *savename = NULL;

void G_LoadGame(char *name)
{
    savename = M_StringDuplicate(name);
    gameaction = ga_loadgame;
}

//---------------------------------------------------------------------------
//
// PROC G_DoLoadGame
//
// Called by G_Ticker based on gameaction.
//
//---------------------------------------------------------------------------

#define VERSIONSIZE 16

void G_DoLoadGame(void)
{
    int i;
    int a, b, c;
    int d, e, f;
    char savestr[SAVESTRINGSIZE];
    char vcheck[VERSIONSIZE], readversion[VERSIONSIZE];

    gameaction = ga_nothing;

    SV_OpenRead(savename);

    free(savename);
    savename = NULL;

    // Skip the description field
    SV_Read(savestr, SAVESTRINGSIZE);

    memset(vcheck, 0, sizeof(vcheck));
    DEH_snprintf(vcheck, VERSIONSIZE, "version %i", HERETIC_VERSION);
    SV_Read(readversion, VERSIONSIZE);

    if (strncmp(readversion, vcheck, VERSIONSIZE) != 0)
    {   // Bad version
        return;
    }
    gameskill = SV_ReadByte();
    gameepisode = SV_ReadByte();
    gamemap = SV_ReadByte();
    // [JN] Read choosen by RAVMUS/IDMUS music from saved game.
    idmusnum = SV_ReadByte();
    // [JN] jff 3/18/98 account for unsigned byte
    if (idmusnum == 255)
    {
        idmusnum = -1;
    }
    for (i = 0; i < MAXPLAYERS; i++)
    {
        playeringame[i] = SV_ReadByte();
    }
    // Load a base level
    G_InitNew(gameskill, gameepisode, gamemap, 0);

    // Create leveltime
    a = SV_ReadByte();
    b = SV_ReadByte();
    c = SV_ReadByte();
    leveltime = (a << 16) + (b << 8) + c;

    // [JN] Get total level times
    d = SV_ReadByte();
    e = SV_ReadByte();
    f = SV_ReadByte();
    totalleveltimes = (d<<16) + (e<<8) + f;

    // De-archive all the modifications
    P_UnArchivePlayers();
    P_UnArchiveWorld();
    P_UnArchiveThinkers();
    P_UnArchiveSpecials();
    P_UnArchiveAutomap ();
    P_RestoreTargets();

    if (SV_ReadByte() != SAVE_GAME_TERMINATOR)
    {                           // Missing savegame termination marker
        I_Error(english_language ?
                "Bad savegame" :
                "Некорректный файл сохранения");
    }
}


/*
====================
=
= G_InitNew
=
= Can be called by the startup code or the menu task
= consoleplayer, displayplayer, playeringame[] should be set
====================
*/

skill_t d_skill;
int d_episode;
int d_map;

void G_DeferedInitNew(skill_t skill, int episode, int map)
{
    d_skill = skill;
    d_episode = episode;
    d_map = map;
    gameaction = ga_newgame;
}

void G_DoNewGame(void)
{
    idmusnum = -1;  // [JN] e6y: allow new level's music to be loaded
    G_InitNew(d_skill, d_episode, d_map, 0);
    gameaction = ga_nothing;
}

/*
================================================================================
=
= G_DoSelectiveGame
=
= [JN] Start new game with given parameters in "Level select" menu.
=
================================================================================
*/

void G_DoSelectiveGame(int option)
{
    int i;
    player_t *p = &players[consoleplayer];
    demoplayback = false; 
    netgame = false;
    deathmatch = false;
    playeringame[1] = playeringame[2] = playeringame[3] = 0;
    consoleplayer = 0;
    gameaction = ga_nothing; 
    automapactive = false;

    // Close "Level select" menu.
    RD_Menu_DeactivateMenu ();
    
    // Start a new game, activate fast monsters if appropriate.
    G_InitNew (selective_skill, selective_episode, selective_map, selective_fast);

    // Respawning monsters.
    respawnmonsters = selective_respawn;

    // Health.
    p->health = selective_health;
    p->mo->health = selective_health;

    // Armor.
    p->armorpoints = selective_armor;
    // Armor type (set to 0 if no armor given).
    p->armortype = selective_armor == 0 ? 0 : selective_armortype;

    // Weapons.
    p->weaponowned[wp_gauntlets]  = selective_wp_gauntlets;
    p->weaponowned[wp_crossbow]   = selective_wp_crossbow;
    p->weaponowned[wp_blaster]    = selective_wp_dragonclaw;
    p->weaponowned[wp_skullrod]   = gamemode == shareware ? 0 : selective_wp_hellstaff;
    p->weaponowned[wp_phoenixrod] = gamemode == shareware ? 0 : selective_wp_phoenixrod;
    p->weaponowned[wp_mace]       = gamemode == shareware ? 0 : selective_wp_firemace;

    // Backpack.
    p->backpack = selective_backpack;
    if (selective_backpack)
    {
        p->maxammo[0] *= 2;
        p->maxammo[1] *= 2;
        p->maxammo[2] *= 2;
        p->maxammo[3] *= 2;
    }

    // Ammo.
    p->ammo[0] = selective_ammo_0; // wand crystals
    p->ammo[1] = selective_ammo_1; // ethereal arrows
    p->ammo[2] = selective_ammo_2; // claw orbs
    p->ammo[3] = selective_ammo_3; // hellstaff runes
    p->ammo[4] = selective_ammo_4; // flame orbs
    p->ammo[5] = selective_ammo_5; // mace spheres

    // Keys.
    p->keys[0] = selective_key_0; // yellow key
    p->keys[1] = selective_key_1; // green key
    p->keys[2] = selective_key_2; // blue key

    // Artifacts.
    for (i = 0 ; i < selective_arti_0 ; i++) P_GiveArtifact(p, arti_health, NULL);
    for (i = 0 ; i < selective_arti_1 ; i++) P_GiveArtifact(p, arti_superhealth, NULL);
    for (i = 0 ; i < selective_arti_2 ; i++) P_GiveArtifact(p, arti_firebomb, NULL);
    for (i = 0 ; i < selective_arti_3 ; i++) P_GiveArtifact(p, arti_tomeofpower, NULL);
    for (i = 0 ; i < selective_arti_4 ; i++) P_GiveArtifact(p, arti_invulnerability, NULL);
    for (i = 0 ; i < selective_arti_5 ; i++) P_GiveArtifact(p, arti_egg, NULL);
    for (i = 0 ; i < selective_arti_6 ; i++) P_GiveArtifact(p, arti_teleport, NULL);
    for (i = 0 ; i < selective_arti_7 ; i++) P_GiveArtifact(p, arti_invisibility, NULL);
    for (i = 0 ; i < selective_arti_8 ; i++) P_GiveArtifact(p, arti_fly, NULL);
    for (i = 0 ; i < selective_arti_9 ; i++) P_GiveArtifact(p, arti_torch, NULL);
} 

void G_InitNew(skill_t skill, int episode, int map, int fast_monsters)
{
    int i;
    int speed;
    static char *skyLumpNames[5] = {
        "SKY1", "SKY2", "SKY3", "SKY1", "SKY3"
    };

    if (paused)
    {
        paused = false;
        S_ResumeSound();
    }
    if (skill < sk_baby)
        skill = sk_baby;
    if (skill > sk_ultranm)
        skill = sk_ultranm;
    if (episode < 1)
        episode = 1;
    // Up to 9 episodes for testing
    if (episode > 9)
        episode = 9;
    if (map < 1)
        map = 1;
    if (map > 9)
        map = 9;
    M_ClearRandom();
    if (respawnparm)
    {
        respawnmonsters = true;
    }
    else
    {
        respawnmonsters = false;
    }
    // Set monster missile speeds
    // [JN] Speed up for 5th and 6th skill levels
    speed = (skill == sk_nightmare || skill == sk_ultranm || fast_monsters);
    for (i = 0; MonsterMissileInfo[i].type != -1; i++)
    {
        mobjinfo[MonsterMissileInfo[i].type].speed
            = MonsterMissileInfo[i].speed[speed] << FRACBITS;
    }
    // Force players to be initialized upon first level load
    for (i = 0; i < MAXPLAYERS; i++)
    {
        players[i].playerstate = PST_REBORN;
        players[i].didsecret = false;
    }

    // [JN] Ultra-Nightmare definitions
    if (skill == sk_ultranm)
    {
        // Monster's speed                          // Old value   Who
        mobjinfo[MT_IMP].speed = 17;                // 10          Gargolye
        mobjinfo[MT_IMPLEADER].speed = 17;          // 10          Fire Gargolye
        mobjinfo[MT_MUMMY].speed = 21;              // 12          Golem
        mobjinfo[MT_MUMMYGHOST].speed = 21;         // 12          Golem Ghost
        mobjinfo[MT_MUMMYLEADER].speed = 21;        // 12          Nitrogolem
        mobjinfo[MT_MUMMYLEADERGHOST].speed = 21;   // 12          Nitrogolem Ghost
        mobjinfo[MT_CLINK].speed = 21;              // 14          Sabreclaw
        mobjinfo[MT_KNIGHT].speed = 19;             // 12          Undead Warrior
        mobjinfo[MT_KNIGHTGHOST].speed = 19;        // 12          Undead Warrior Ghost
        mobjinfo[MT_BEAST].speed = 21;              // 14          Weredragon
        mobjinfo[MT_SNAKE].speed = 17;              // 10          Ophidian
        mobjinfo[MT_WIZARD].speed = 19;             // 12          Disciple of D'Sparil
        mobjinfo[MT_HEAD].speed = 13;               // 6           Iron Lich
        mobjinfo[MT_MINOTAUR].speed = 21;           // 16          Maolotaur
        mobjinfo[MT_SORCERER1].speed = 21;          // 16          D'Sparil (riding)
        mobjinfo[MT_SORCERER2].speed = 20;          // 14          D'Sparil (walking)

        // Monster's damage                         // Old value   What
        mobjinfo[MT_IMPBALL].damage = 2;            // 1           Fire Gargolye fireball
        mobjinfo[MT_MUMMYFX1].damage = 6;           // 4           Nitrogolem Ghost fireball
        mobjinfo[MT_KNIGHTAXE].damage = 5;          // 3           Undead Warrior green axe
        mobjinfo[MT_REDAXE].damage = 8;             // 7           Undead Warrior red axe
        mobjinfo[MT_BEASTBALL].damage = 5;          // 4           Weredragon fireball
        mobjinfo[MT_SNAKEPRO_A].damage = 2;         // 1           Ophidian fireball A
        mobjinfo[MT_SNAKEPRO_B].damage = 4;         // 3           Ophidian fireball B
        mobjinfo[MT_WIZFX1].damage = 4;             // 3           Disciple of D'Sparil fireball
    }
    // [JN] Fallback to standard values
    else
    {
        // Monster's speed
        mobjinfo[MT_IMP].speed = 10;
        mobjinfo[MT_IMPLEADER].speed = 10;
        mobjinfo[MT_MUMMY].speed = 12;
        mobjinfo[MT_MUMMYGHOST].speed = 12;
        mobjinfo[MT_MUMMYLEADER].speed = 12;
        mobjinfo[MT_MUMMYLEADERGHOST].speed = 12;
        mobjinfo[MT_CLINK].speed = 14;
        mobjinfo[MT_KNIGHT].speed = 12;
        mobjinfo[MT_KNIGHTGHOST].speed = 12;
        mobjinfo[MT_BEAST].speed = 14;
        mobjinfo[MT_SNAKE].speed = 10;
        mobjinfo[MT_WIZARD].speed = 12;
        mobjinfo[MT_HEAD].speed = 6;
        mobjinfo[MT_MINOTAUR].speed = 16;
        mobjinfo[MT_SORCERER1].speed = 16;
        mobjinfo[MT_SORCERER2].speed = 14;

        // Monster's damage
        mobjinfo[MT_IMPBALL].damage = 1;
        mobjinfo[MT_MUMMYFX1].damage = 4;
        mobjinfo[MT_KNIGHTAXE].damage = 2;
        mobjinfo[MT_REDAXE].damage = 7;
        mobjinfo[MT_BEASTBALL].damage = 4;
        mobjinfo[MT_SNAKEPRO_A].damage = 1;
        mobjinfo[MT_SNAKEPRO_B].damage = 3;
        mobjinfo[MT_WIZFX1].damage = 3;
    }

    // Set up a bunch of globals
    usergame = true;            // will be set false if a demo
    paused = false;
    demorecording = false;
    demoplayback = false;
    // [JN] Reset automap scale. Fixes:
    // https://doomwiki.org/wiki/Automap_scale_preserved_after_warps_in_Heretic_and_Hexen
    automapactive = false; 
    // [JN] jff 4/16/98 force marks on automap cleared every new level start
    AM_clearMarks();
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;
    viewactive = true;
    BorderNeedRefresh = true;

    // [crispy] CPhipps - total time for all completed levels
    totalleveltimes = 0;

    // Set the sky map
    if (episode > 5)
    {
        skytexture = R_TextureNumForName(DEH_String("SKY1"));
    }
    else
    {
        skytexture = R_TextureNumForName(DEH_String(skyLumpNames[episode - 1]));
    }

//
// give one null ticcmd_t
//
#if 0
    gametic = 0;
    maketic = 1;
    for (i = 0; i < MAXPLAYERS; i++)
        nettics[i] = 1;         // one null event for this gametic
    memset(localcmds, 0, sizeof(localcmds));
    memset(netcmds, 0, sizeof(netcmds));
#endif
    G_DoLoadLevel();
}


/*
===============================================================================

							DEMO RECORDING

===============================================================================
*/

#define DEMOMARKER            0x80
#define DEMOHEADER_RESPAWN    0x20
#define DEMOHEADER_LONGTICS   0x10
#define DEMOHEADER_NOMONSTERS 0x02

void G_ReadDemoTiccmd(ticcmd_t * cmd)
{
    if (*demo_p == DEMOMARKER)
    {                           // end of demo data stream
        G_CheckDemoStatus();
        return;
    }
    cmd->forwardmove = ((signed char) *demo_p++);
    cmd->sidemove = ((signed char) *demo_p++);

    // If this is a longtics demo, read back in higher resolution

    if (longtics)
    {
        cmd->angleturn = *demo_p++;
        cmd->angleturn |= (*demo_p++) << 8;
    }
    else
    {
        cmd->angleturn = ((unsigned char) *demo_p++) << 8;
    }

    cmd->buttons = (unsigned char) *demo_p++;
    cmd->lookfly = (unsigned char) *demo_p++;
    cmd->arti = (unsigned char) *demo_p++;
}

// Increase the size of the demo buffer to allow unlimited demos

static void IncreaseDemoBuffer(void)
{
    int current_length;
    byte *new_demobuffer;
    byte *new_demop;
    int new_length;

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

void G_WriteDemoTiccmd(ticcmd_t * cmd)
{
    byte *demo_start;

    if (BK_isKeyPressed(bk_finish_demo)) // press to end demo recording
        G_CheckDemoStatus();

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
    *demo_p++ = cmd->lookfly;
    *demo_p++ = cmd->arti;

    // reset demo pointer back
    demo_p = demo_start;

    // [JN] No limits for demo recording
    if (demo_p > demoend - 16)
    {
        IncreaseDemoBuffer();
    }

    G_ReadDemoTiccmd(cmd);      // make SURE it is exactly the same
}



/*
===================
=
= G_RecordDemo
=
===================
*/

void G_RecordDemo(skill_t skill, int numplayers, int episode, int map,
                  char *name)
{
    size_t demoname_size;
    int i;
    int maxsize;

    //!
    // @category demo
    //
    // Record or playback a demo with high resolution turning.
    //

    longtics = D_NonVanillaRecord(M_ParmExists("-longtics"),
                                  "vvHeretic longtics demo");

    // If not recording a longtics demo, record in low res

    lowres_turn = !longtics;

    //!
    // @category demo
    //
    // Smooth out low resolution turning when recording a demo.
    //

    shortticfix = M_ParmExists("-shortticfix");

    G_InitNew(skill, episode, map, 0);
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
        maxsize = atoi(myargv[i + 1]) * 1024;
    demobuffer = Z_Malloc(maxsize, PU_STATIC, NULL);
    demoend = demobuffer + maxsize;

    demo_p = demobuffer;
    *demo_p++ = skill;
    *demo_p++ = episode;
    *demo_p++ = map;

    // Write special parameter bits onto player one byte.
    // This aligns with vvHeretic demo usage:
    //   0x20 = -respawn
    //   0x10 = -longtics
    //   0x02 = -nomonsters

    *demo_p = 1; // assume player one exists
    if (D_NonVanillaRecord(respawnparm, "vvHeretic -respawn header flag"))
    {
        *demo_p |= DEMOHEADER_RESPAWN;
    }
    if (longtics)
    {
        *demo_p |= DEMOHEADER_LONGTICS;
    }
    if (D_NonVanillaRecord(nomonsters, "vvHeretic -nomonsters header flag"))
    {
        *demo_p |= DEMOHEADER_NOMONSTERS;
    }
    demo_p++;

    for (i = 1; i < MAXPLAYERS; i++)
        *demo_p++ = playeringame[i];

    demorecording = true;
}


/*
===================
=
= G_PlayDemo
=
===================
*/

char *defdemoname;

void G_DeferedPlayDemo(char *name)
{
    defdemoname = name;
    gameaction = ga_playdemo;
}

void G_DoPlayDemo(void)
{
    skill_t skill;
    int i, lumpnum, episode, map;

    gameaction = ga_nothing;
    lumpnum = W_GetNumForName(defdemoname);
    demobuffer = W_CacheLumpNum(lumpnum, PU_STATIC);
    demo_p = demobuffer;
    skill = *demo_p++;
    episode = *demo_p++;
    map = *demo_p++;

    // vvHeretic allows extra options to be stored in the upper bits of
    // the player 1 present byte. However, this is a non-vanilla extension.
    if (D_NonVanillaPlayback((*demo_p & DEMOHEADER_LONGTICS) != 0,
                             lumpnum, "vvHeretic longtics demo"))
    {
        longtics = true;
    }
    if (D_NonVanillaPlayback((*demo_p & DEMOHEADER_RESPAWN) != 0,
                             lumpnum, "vvHeretic -respawn header flag"))
    {
        respawnparm = true;
    }
    if (D_NonVanillaPlayback((*demo_p & DEMOHEADER_NOMONSTERS) != 0,
                             lumpnum, "vvHeretic -nomonsters header flag"))
    {
        nomonsters = true;
    }

    for (i = 0; i < MAXPLAYERS; i++)
        playeringame[i] = (*demo_p++) != 0;

    precache = false;           // don't spend a lot of time in loadlevel
    G_InitNew(skill, episode, map, 0);
    precache = true;
    usergame = false;
    demoplayback = true;
}


/*
===================
=
= G_TimeDemo
=
===================
*/

void G_TimeDemo(char *name)
{
    skill_t skill;
    int episode, map, i;

    demobuffer = demo_p = W_CacheLumpName(name, PU_STATIC);
    skill = *demo_p++;
    episode = *demo_p++;
    map = *demo_p++;

    // Read special parameter bits: see G_RecordDemo() for details.
    longtics = (*demo_p & DEMOHEADER_LONGTICS) != 0;

    // don't overwrite arguments from the command line
    respawnparm |= (*demo_p & DEMOHEADER_RESPAWN) != 0;
    nomonsters  |= (*demo_p & DEMOHEADER_NOMONSTERS) != 0;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        playeringame[i] = (*demo_p++) != 0;
    }

    G_InitNew(skill, episode, map, 0);
    starttime = I_GetTime();

    usergame = false;
    demoplayback = true;
    timingdemo = true;
    singletics = true;
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

boolean G_CheckDemoStatus(void)
{
    int endtime, realtics;

    if (timingdemo)
    {
        float fps;
        endtime = I_GetTime();
        realtics = endtime - starttime;
        fps = ((float) gametic * TICRATE) / realtics;
        
        if (english_language)
        {
            I_Error("timed %i gametics in %i realtics (%f fps)",
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
        if (singledemo)
            I_Quit();

        W_ReleaseLumpName(defdemoname);
        demoplayback = false;
        D_AdvanceDemo();
        return true;
    }

    if (demorecording)
    {
        *demo_p++ = DEMOMARKER;
        M_WriteFile(demoname, demobuffer, demo_p - demobuffer);
        Z_Free(demobuffer);
        demorecording = false;
        I_Error(english_language ?
                "Demo %s recorded" :
                "Демозапись %s завершена",demoname); 
    }

    return false;
}

/**************************************************************************/
/**************************************************************************/

//==========================================================================
//
// G_SaveGame
//
// Called by the menu task.  <description> is a 24 byte text string.
//
//==========================================================================

void G_SaveGame(int slot, char *description)
{
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
}

//==========================================================================
//
// G_DoSaveGame
//
// Called by G_Ticker based on gameaction.
//
//==========================================================================

void G_DoSaveGame(void)
{
    int i;
    char *filename;
    char verString[VERSIONSIZE];
    char *description;

    filename = SV_Filename(savegameslot);

    description = savedescription;

    SV_Open(filename);
    SV_Write(description, SAVESTRINGSIZE);
    memset(verString, 0, sizeof(verString));
    DEH_snprintf(verString, VERSIONSIZE, "version %i", HERETIC_VERSION);
    SV_Write(verString, VERSIONSIZE);
    SV_WriteByte(gameskill);
    SV_WriteByte(gameepisode);
    SV_WriteByte(gamemap);
    // [JN] Write choosen by RAVMUS/IDMUS music into saved game.
    SV_WriteByte(idmusnum);
    for (i = 0; i < MAXPLAYERS; i++)
    {
        SV_WriteByte(playeringame[i]);
    }
    SV_WriteByte(leveltime >> 16);
    SV_WriteByte(leveltime >> 8);
    SV_WriteByte(leveltime);
    // [JN] Write total level times
    SV_WriteByte(totalleveltimes >> 16);
    SV_WriteByte(totalleveltimes >> 8);
    SV_WriteByte(totalleveltimes);
    P_ArchivePlayers();
    P_ArchiveWorld();
    P_ArchiveThinkers();
    P_ArchiveSpecials();
    P_ArchiveAutomap ();
    SV_Close(filename);

    gameaction = ga_nothing;
    savedescription[0] = 0;
    P_SetMessage(&players[consoleplayer], DEH_String(txt_gamesaved), msg_system, true);

    free(filename);
}

