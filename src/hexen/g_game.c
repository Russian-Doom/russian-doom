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



#include <string.h>
#include "m_random.h"
#include "h2def.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_misc.h"
#include "p_local.h"
#include "rd_keybinds.h"
#include "v_video.h"

#define AM_STARTKEY	9

// External functions

extern void R_InitSky(int map);
extern void P_PlayerNextArtifact(player_t * player);

// Functions

boolean G_CheckDemoStatus(void);
void G_ReadDemoTiccmd(ticcmd_t * cmd);
void G_WriteDemoTiccmd(ticcmd_t * cmd);

void G_DoReborn(int playernum);

void G_DoLoadLevel(void);
void G_DoInitNew(void);
void G_DoNewGame(void);
void G_DoPlayDemo(void);
void G_DoTeleportNewMap(void);
void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);
void G_DoSaveGame(void);
void G_DoSingleReborn(void);

void H2_PageTicker(void);
void H2_AdvanceDemo(void);

extern boolean mn_SuicideConsole;

gameaction_t gameaction;
gamestate_t gamestate;
skill_t gameskill;
//boolean         respawnmonsters;
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
pclass_t PlayerClass[MAXPLAYERS];

// Position indicator for cooperative net-play reborn
int RebornPosition;

int consoleplayer;              // player taking events and displaying
int displayplayer;              // view being displayed
int levelstarttic;              // gametic at level start

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

// TODO: Hexen uses 16-bit shorts for consistancy?
byte consistancy[MAXPLAYERS][BACKUPTICS];

int mouseSensitivity = 5;

int LeaveMap;
static int LeavePosition;

//#define MAXPLMOVE       0x32 // Old Heretic Max move

fixed_t MaxPlayerMove[NUMCLASSES] = { 0x3C, 0x32, 0x2D, 0x31 };
fixed_t forwardmove[NUMCLASSES][2] = {
    {0x1D, 0x3C},
    {0x19, 0x32},
    {0x16, 0x2E},
    {0x18, 0x31}
};

fixed_t sidemove[NUMCLASSES][2] = {
    {0x1B, 0x3B},
    {0x18, 0x28},
    {0x15, 0x25},
    {0x17, 0x27}
};

fixed_t angleturn[3] = { 640, 1280, 320 };      // + slow turn
// [JN] If true, activate maximum weapon bobbing
boolean max_bobbing;

static bound_key_t weapon_keys[] =
{
    bk_weapon_1,
    bk_weapon_2,
    bk_weapon_3,
    bk_weapon_4
};

static int next_weapon = 0;

#define SLOWTURNTICS    6

int turnheld;                   // for accelerative turning
int lookheld;

int mousex, mousey;             // mouse values are used once

int joyturn, joymove;         // joystick values are repeated
int joystrafemove;
int joyvlook;
int alwaysRun = 1;              // is always run enabled

int savegameslot;
char savedescription[32];

int inventoryTics;

// haleyjd: removed externdriver crap

static skill_t TempSkill;
static int TempEpisode;
static int TempMap;

boolean testcontrols = false;
int testcontrols_mousespeed;

//=============================================================================
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
boolean usearti = true;

void G_BuildTiccmd(ticcmd_t *cmd, int maketic)
{
    int i;
    boolean strafe;
    int speed, tspeed, lspeed;
    int forward, side;
    int look, arti;
    int flyheight;
    int pClass;
    extern boolean askforquit;

    extern boolean artiskip;

    // haleyjd: removed externdriver crap

    pClass = players[consoleplayer].class;
    memset(cmd, 0, sizeof(*cmd));

//      cmd->consistancy =
//              consistancy[consoleplayer][(maketic*ticdup)%BACKUPTICS];

    cmd->consistancy = consistancy[consoleplayer][maketic % BACKUPTICS];

//printf ("cons: %i\n",cmd->consistancy);

    strafe = BK_isKeyPressed(bk_strafe);

    // Allow joybspeed hack.

	// [crispy] when "always run" is active,
    // pressing the "run" key will result in walking

    speed = alwaysRun;
		
		// [JN] Модификатор мнопки бега
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
        turnheld = 0;

    if (turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

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
        lspeed = 1;             // 3;
    }
    else
    {
        lspeed = 2;             // 5;
    }
	
    // [crispy] toggle always run
    if (BK_isKeyPressed(bk_toggle_autorun))
    {
        alwaysRun ^= 1;

        // [JN] Added audible feedback
        if (english_language)
        {
            P_SetMessage(&players[consoleplayer], 
                         alwaysRun ?
                         TXT_ALWAYSRUN_ON : TXT_ALWAYSRUN_OFF,
                         false);
        }
        else
        {
            P_SetMessage(&players[consoleplayer],
                         alwaysRun ?
                         TXT_ALWAYSRUN_ON_RUS : TXT_ALWAYSRUN_OFF_RUS,
                         false);
        }
        S_StartSound(NULL, SFX_CHAT);

        BK_ReleaseKey(bk_toggle_autorun);
    }

    // [JN] Toggle crosshair
    if (BK_isKeyPressed(bk_toggle_crosshair))
    {
        // [JN] No toggling in -vanilla mode
        if (vanillaparm)
            return;

        if (!crosshair_draw)
        {
            crosshair_draw = true;
        }
        else
        {
            crosshair_draw = false;
        }

        if (english_language)
        {
            P_SetMessage(&players[consoleplayer], crosshair_draw ?
                         TXT_CROSSHAIR_ON : TXT_CROSSHAIR_OFF,
                         false);
        }
        else
        {
            P_SetMessage(&players[consoleplayer], crosshair_draw ?
                         TXT_CROSSHAIR_ON_RUS : TXT_CROSSHAIR_OFF_RUS,
                         false);
        }
        S_StartSound(NULL, SFX_CHAT);

        BK_ReleaseKey(bk_toggle_crosshair);
    }

    // [JN] Toggle level flipping.
    if (BK_isKeyPressed(bk_toggle_fliplvls))
    {
        flip_levels ^= 1;
        R_ExecuteSetViewSize();       // Redraw game screen
        S_StartSound(NULL, SFX_CHAT); // Play quiet sound

        BK_ReleaseKey(bk_toggle_fliplvls);
    }

//
// let movement keys cancel each other out
//
    if (strafe)
    {
        if (BK_isKeyPressed(bk_turn_right))
        {
            side += sidemove[pClass][speed];
        }
        if (BK_isKeyPressed(bk_turn_left))
        {
            side -= sidemove[pClass][speed];
        }
        if (joyturn != 0)
        {
            side += joyturn;
        }
        if(mousex != 0)
        {
            side += mousex*2;
        }
    }
    else
    {
        if (BK_isKeyPressed(bk_turn_right))
            cmd->angleturn -= angleturn[tspeed];
        if (BK_isKeyPressed(bk_turn_left))
            cmd->angleturn += angleturn[tspeed];
        if(joyturn != 0)
            cmd->angleturn -= joyturn;
        if(mousex != 0)
            cmd->angleturn -= mousex*0x8;
    }

    if (BK_isKeyPressed(bk_forward))
    {
        forward += forwardmove[pClass][speed];
    }
    if (BK_isKeyPressed(bk_backward))
    {
        forward -= forwardmove[pClass][speed];
    }
    if (joymove != 0)
    {
        forward += joymove;
    }
    if (BK_isKeyPressed(bk_strafe_right))
    {
        side += sidemove[pClass][speed];
    }
    if (BK_isKeyPressed(bk_strafe_left))
    {
        side -= sidemove[pClass][speed];
    }
    if (joystrafemove != 0)
    {
        side += joystrafemove;
    }

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
        flyheight = 5;          // note that the actual flyheight will be twice this
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
        if (BK_isKeyPressed(bk_speed) && artiskip)
        {
            if (players[consoleplayer].inventory[inv_ptr].type != arti_none)
            {                   // Skip an artifact
                BK_ReleaseKey(bk_inv_use_artifact);
                P_PlayerNextArtifact(&players[consoleplayer]);
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
                cmd->arti |=
                    players[consoleplayer].inventory[inv_ptr].
                    type & AFLAG_MASK;
                usearti = false;
            }
        }
    }
    if (BK_isKeyPressed(bk_jump))
    {
        cmd->arti |= AFLAG_JUMP;
    }
    if (mn_SuicideConsole)
    {
        cmd->arti |= AFLAG_SUICIDE;
        mn_SuicideConsole = false;
    }

    // Artifact hot keys
    // [JN] Extended to all of them.
    if (BK_isKeyPressed(bk_arti_all) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_all);     // Use one of each artifact
        cmd->arti = NUMARTIFACTS;
    }
    else if (BK_isKeyPressed(bk_arti_quartz) && !cmd->arti
         && (players[consoleplayer].mo->health < MAXHEALTH))
    {
        BK_ReleaseKey(bk_arti_quartz);
        cmd->arti = arti_health;
    }
    else if (BK_isKeyPressed(bk_arti_bomb) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_bomb);
        cmd->arti = arti_poisonbag;
    }
    else if (BK_isKeyPressed(bk_arti_blastradius) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_blastradius);
        cmd->arti = arti_blastradius;
    }
    else if (BK_isKeyPressed(bk_arti_chaosdevice) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_chaosdevice);
        cmd->arti = arti_teleport;
    }
    else if (BK_isKeyPressed(bk_arti_teleportother) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_teleportother);
        cmd->arti = arti_teleportother;
    }
    else if (BK_isKeyPressed(bk_arti_egg) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_egg);
        cmd->arti = arti_egg;
    }
    else if (BK_isKeyPressed(bk_arti_invulnerability) && !cmd->arti
             && !players[consoleplayer].powers[pw_invulnerability])
    {
        BK_ReleaseKey(bk_arti_invulnerability);
        cmd->arti = arti_invulnerability;
    }
    // [JN] The rest of Artifacts:
    else if (BK_isKeyPressed(bk_arti_urn) && !cmd->arti
         && (players[consoleplayer].mo->health < MAXHEALTH))
    {
        BK_ReleaseKey(bk_arti_urn);
        cmd->arti = arti_superhealth;
    }
    else if (BK_isKeyPressed(bk_arti_boostarmor) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_boostarmor);
        cmd->arti = arti_boostarmor;
    }
    else if (BK_isKeyPressed(bk_arti_boostmana) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_boostmana);
        cmd->arti = arti_boostmana;
    }
    else if (BK_isKeyPressed(bk_arti_summon) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_summon);
        cmd->arti = arti_summon;
    }
    else if (BK_isKeyPressed(bk_arti_wings) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_wings);
        cmd->arti = arti_fly;
    }
    else if (BK_isKeyPressed(bk_arti_speed) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_speed);
        cmd->arti = arti_speed;
    }
    else if (BK_isKeyPressed(bk_arti_torch) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_torch);
        cmd->arti = arti_torch;
    }
    else if (BK_isKeyPressed(bk_arti_healingradius) && !cmd->arti)
    {
        BK_ReleaseKey(bk_arti_healingradius);
        cmd->arti = arti_healingradius;
    }

//
// buttons
//
    cmd->chatchar = CT_dequeueChatChar();

    if (BK_isKeyPressed(bk_fire))
        cmd->buttons |= BT_ATTACK;

    if (BK_isKeyPressed(bk_use))
    {
        cmd->buttons |= BT_USE;
    }

    // Weapon cycling. Switch to previous or next weapon.
    // (Disabled when player is a pig).
    if (gamestate == GS_LEVEL
     && players[consoleplayer].morphTics == 0 && next_weapon != 0)
    {
        int start_i;

        if (players[consoleplayer].pendingweapon == WP_NOCHANGE)
        {
            i = players[consoleplayer].readyweapon;
        }
        else
        {
            i = players[consoleplayer].pendingweapon;
        }

        // Don't loop forever.
        start_i = i;
        do {
            i = (i + next_weapon + NUMWEAPONS) % NUMWEAPONS;
        } while (i != start_i && !players[consoleplayer].weaponowned[i]);

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

        if (english_language)
        {
            P_SetMessage(&players[consoleplayer],
                         (mlook == true ? 
                         TXT_MLOOK_ON : TXT_MLOOK_OFF),
                         false);
        }
        else
        {
            P_SetMessage(&players[consoleplayer],
                         (mlook == true ? 
                         TXT_MLOOK_ON_RUS : TXT_MLOOK_OFF_RUS),
                         false);
        }
        S_StartSound(NULL, SFX_CHAT);

        BK_ReleaseKey(bk_toggle_mlook);
    }

    mousex = mousey = 0;

    // [JN] "false" must be set as initial and returning condition.
    max_bobbing = false; 

    if (forward > MaxPlayerMove[pClass])
    {
        forward = MaxPlayerMove[pClass];
        max_bobbing = true;
    }
    else if (forward < -MaxPlayerMove[pClass])
    {
        forward = -MaxPlayerMove[pClass];
        max_bobbing = false;
    }
    if (side > MaxPlayerMove[pClass])
    {
        side = MaxPlayerMove[pClass];
        max_bobbing = true;
    }
    else if (side < -MaxPlayerMove[pClass])
    {
        side = -MaxPlayerMove[pClass];
        max_bobbing = false;   
    }
    if (players[consoleplayer].powers[pw_speed]
        && !players[consoleplayer].morphTics)
    {                           // Adjust for a player with a speed artifact
        forward = (3 * forward) >> 1;
        side = (3 * side) >> 1;
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
    for (i = 0; i < maxplayers; i++)
    {
        if (playeringame[i] && players[i].playerstate == PST_DEAD)
            players[i].playerstate = PST_REBORN;
        memset(players[i].frags, 0, sizeof(players[i].frags));
    }

    SN_StopAllSequences();
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
        P_SetMessage(&players[consoleplayer], english_language ?
                                              "PRESS ESCAPE TO QUIT." :
                                              "HT;BV GHJDTHRB EGHFDKTYBZ", // РЕЖИМ ПРОВЕРКИ УПРАВЛЕНИЯ
                                              false);
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
    {                           // flag to denote that it's okay to use an artifact
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
            if (displayplayer == maxplayers)
            {
                displayplayer = 0;
            }
        }
        while (!playeringame[displayplayer]
               && displayplayer != consoleplayer);
        return (true);
    }

    if (CT_Responder(ev))
    {                           // Chat ate the event
        return (true);
    }
    if (gamestate == GS_LEVEL)
    {
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


//==========================================================================
//
// G_Ticker
//
//==========================================================================

void G_Ticker(void)
{
    int i, buf;
    ticcmd_t *cmd = NULL;

//
// do player reborns if needed
//
    for (i = 0; i < maxplayers; i++)
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
            case ga_initnew:
                G_DoInitNew();
                break;
            case ga_newgame:
                G_DoNewGame();
                break;
            case ga_loadgame:
                Draw_LoadIcon();
                G_DoLoadGame();
                // [JN] Reset looking direction if game is loaded without mouse look
                if (!mlook)
                players[consoleplayer].lookdir = 0;
                break;
            case ga_savegame:
                Draw_SaveIcon();
                G_DoSaveGame();
                break;
            case ga_singlereborn:
                G_DoSingleReborn();
                break;
            case ga_playdemo:
                G_DoPlayDemo();
                break;
            case ga_screenshot: // [JN] Extended name from "HEXEN%02i.%s"
                V_ScreenShot("screenshot-hexen-%02i.%s");
                S_StartSound(NULL, SFX_CHAT);   // [JN] Audio feedback
                // P_SetMessage(&players[consoleplayer], "CYBVJR 'RHFYF", false);	// СНИМОК ЭКРАНА | SCREEN SHOT
                gameaction = ga_nothing;
                break;
            case ga_leavemap:
                Draw_TeleportIcon();
                G_DoTeleportNewMap();
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
    //buf = gametic%BACKUPTICS;
    buf = (gametic / ticdup) % BACKUPTICS;

    for (i = 0; i < maxplayers; i++)
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
    for (i = 0; i < maxplayers; i++)
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
                                M_StringCopy(savedescription, "NET GAME",
                                             sizeof(savedescription));
                            }
                            else
                            {
                                M_StringCopy(savedescription, "SAVE GAME",
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
            H2_PageTicker();
            break;
    }
}


/*
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Things
==============================================================================
*/

//==========================================================================
//
// G_PlayerExitMap
//
// Called when the player leaves a map.
//
//==========================================================================

void G_PlayerExitMap(int playerNumber)
{
    int i;
    player_t *player;
    int flightPower;

    player = &players[playerNumber];

//      if(deathmatch)
//      {
//              // Strip all but one of each type of artifact
//              for(i = 0; i < player->inventorySlotNum; i++)
//              {
//                      player->inventory[i].count = 1;
//              }
//              player->artifactCount = player->inventorySlotNum;
//      }
//      else

    // Strip all current powers (retain flight)
    flightPower = player->powers[pw_flight];
    memset(player->powers, 0, sizeof(player->powers));
    player->powers[pw_flight] = flightPower;

    if (deathmatch)
    {
        player->powers[pw_flight] = 0;
    }
    else
    {
        if (P_GetMapCluster(gamemap) != P_GetMapCluster(LeaveMap))
        {                       // Entering new cluster
            // Strip all keys
            player->keys = 0;

            // Strip flight artifact
            for (i = 0; i < 25; i++)
            {
                player->powers[pw_flight] = 0;
                P_PlayerUseArtifact(player, arti_fly);
            }
            player->powers[pw_flight] = 0;
        }
    }

    if (player->morphTics)
    {
        player->readyweapon = player->mo->special1.i;     // Restore weapon
        player->morphTics = 0;
    }
    player->messageTics = 0;
    player->lookdir = 0;
    player->mo->flags &= ~MF_SHADOW;    // Remove invisibility
    player->extralight = 0;     // Remove weapon flashes
    player->fixedcolormap = 0;  // Remove torch
    player->damagecount = 0;    // No palette changes
    player->bonuscount = 0;
    player->poisoncount = 0;
    if (player == &players[consoleplayer])
    {
        SB_state = -1;          // refresh the status bar
        viewangleoffset = 0;
    }
}

//==========================================================================
//
// G_PlayerReborn
//
// Called after a player dies.  Almost everything is cleared and
// initialized.
//
//==========================================================================

void G_PlayerReborn(int player)
{
    player_t *p;
    int frags[MAXPLAYERS];
    int killcount, itemcount, secretcount;
    unsigned int worldTimer;

    memcpy(frags, players[player].frags, sizeof(frags));
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;
    worldTimer = players[player].worldTimer;

    p = &players[player];
    memset(p, 0, sizeof(*p));

    memcpy(players[player].frags, frags, sizeof(players[player].frags));
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;
    players[player].worldTimer = worldTimer;
    players[player].class = PlayerClass[player];

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = WP_FIRST;
    p->weaponowned[WP_FIRST] = true;
    p->messageTics = 0;
    p->lookdir = 0;
    localQuakeHappening[player] = false;
    if (p == &players[consoleplayer])
    {
        SB_state = -1;          // refresh the status bar
        inv_ptr = 0;            // reset the inventory pointer
        curpos = 0;
        viewangleoffset = 0;
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
        S_StartSound(mo, SFX_TELEPORT); // don't start sound on first frame

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

    // This check has been moved to p_setup.c:P_LoadThings()
    //if (selections < 8)
    //      I_Error ("Only %i deathmatch spots, 8 required", selections);

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
    P_SpawnPlayer(&playerstarts[0][playernum]);
}

//==========================================================================
//
// G_DoReborn
//
//==========================================================================

void G_DoReborn(int playernum)
{
    int i;
    boolean oldWeaponowned[NUMWEAPONS];
    int oldKeys;
    int oldPieces;
    boolean foundSpot;
    int bestWeapon;

    // quit demo unless -demoextend
    if (!demoextend && G_CheckDemoStatus())
    {
        return;
    }
    if (!netgame)
    {
        if (SV_RebornSlotAvailable())
        {                       // Use the reborn code if the slot is available
            gameaction = ga_singlereborn;
        }
        else
        {                       // Start a new game if there's no reborn info
            gameaction = ga_newgame;
        }
    }
    else
    {                           // Net-game
        players[playernum].mo->player = NULL;   // Dissassociate the corpse

        if (deathmatch)
        {                       // Spawn at random spot if in death match
            G_DeathMatchSpawnPlayer(playernum);
            return;
        }

        // Cooperative net-play, retain keys and weapons
        oldKeys = players[playernum].keys;
        oldPieces = players[playernum].pieces;
        for (i = 0; i < NUMWEAPONS; i++)
        {
            oldWeaponowned[i] = players[playernum].weaponowned[i];
        }

        foundSpot = false;
        if (G_CheckSpot(playernum, &playerstarts[RebornPosition][playernum]))
        {                       // Appropriate player start spot is open
            P_SpawnPlayer(&playerstarts[RebornPosition][playernum]);
            foundSpot = true;
        }
        else
        {
            // Try to spawn at one of the other player start spots
            for (i = 0; i < maxplayers; i++)
            {
                if (G_CheckSpot(playernum, &playerstarts[RebornPosition][i]))
                {               // Found an open start spot

                    // Fake as other player
                    playerstarts[RebornPosition][i].type = playernum + 1;
                    P_SpawnPlayer(&playerstarts[RebornPosition][i]);

                    // Restore proper player type
                    playerstarts[RebornPosition][i].type = i + 1;

                    foundSpot = true;
                    break;
                }
            }
        }

        if (foundSpot == false)
        {                       // Player's going to be inside something
            P_SpawnPlayer(&playerstarts[RebornPosition][playernum]);
        }

        // Restore keys and weapons
        players[playernum].keys = oldKeys;
        players[playernum].pieces = oldPieces;
        for (bestWeapon = 0, i = 0; i < NUMWEAPONS; i++)
        {
            if (oldWeaponowned[i])
            {
                bestWeapon = i;
                players[playernum].weaponowned[i] = true;
            }
        }
        players[playernum].mana[MANA_1] = 25;
        players[playernum].mana[MANA_2] = 25;
        if (bestWeapon)
        {                       // Bring up the best weapon
            players[playernum].pendingweapon = bestWeapon;
        }
    }
}

void G_ScreenShot(void)
{
    gameaction = ga_screenshot;
}

//==========================================================================
//
// G_StartNewInit
//
//==========================================================================

void G_StartNewInit(void)
{
    SV_InitBaseSlot();
    SV_ClearRebornSlot();
    P_ACSInitNewGame();
    // Default the player start spot group to 0
    RebornPosition = 0;
}

//==========================================================================
//
// G_StartNewGame
//
//==========================================================================

void G_StartNewGame(skill_t skill)
{
    int realMap;

    G_StartNewInit();
    realMap = P_TranslateMap(1);
    if (realMap == -1)
    {
        realMap = 1;
    }
    G_InitNew(TempSkill, 1, realMap);
}

//==========================================================================
//
// G_TeleportNewMap
//
// Only called by the warp cheat code.  Works just like normal map to map
// teleporting, but doesn't do any interlude stuff.
//
//==========================================================================

void G_TeleportNewMap(int map, int position)
{
    gameaction = ga_leavemap;
    LeaveMap = map;
    LeavePosition = position;
}

//==========================================================================
//
// G_DoTeleportNewMap
//
//==========================================================================

void G_DoTeleportNewMap(void)
{
    SV_MapTeleport(LeaveMap, LeavePosition);
    gamestate = GS_LEVEL;
    gameaction = ga_nothing;
    RebornPosition = LeavePosition;
}

/*
boolean secretexit;
void G_ExitLevel (void)
{
	secretexit = false;
	gameaction = ga_completed;
}
void G_SecretExitLevel (void)
{
	secretexit = true;
	gameaction = ga_completed;
}
*/

//==========================================================================
//
// G_Completed
//
// Starts intermission routine, which is used only during hub exits,
// and DeathMatch games.
//==========================================================================

void G_Completed(int map, int position)
{
    if (gamemode == shareware && map > 4)
    {
        P_SetMessage(&players[consoleplayer], english_language ?
                     "ACCESS DENIED -- DEMO" :
                     "LTVJDTHCBZ> GHJ[JL DJCGHTOTY>", // ДЕМОВЕРСИЯ. ПРОХОД ВОСПРЕЩЕН.
                     true);
        S_StartSound(NULL, SFX_CHAT);
        return;
    }

    gameaction = ga_completed;
    LeaveMap = map;
    LeavePosition = position;
}

void G_DoCompleted(void)
{
    int i;

    gameaction = ga_nothing;

    // quit demo unless -demoextend
    if (!demoextend && G_CheckDemoStatus())
    {
        return;
    }
    for (i = 0; i < maxplayers; i++)
    {
        if (playeringame[i])
        {
            G_PlayerExitMap(i);
        }
    }
    if (LeaveMap == -1 && LeavePosition == -1)
    {
        gameaction = ga_victory;
        return;
    }
    else
    {
        gamestate = GS_INTERMISSION;
        IN_Start();
    }

/*
	int i;
	static int afterSecret[3] = { 7, 5, 5 };

	gameaction = ga_nothing;
	if(G_CheckDemoStatus())
	{
		return;
	}
	for(i = 0; i < maxplayers; i++)
	{
		if(playeringame[i])
		{
			G_PlayerFinishLevel(i);
		}
	}
	prevmap = gamemap;
	if(secretexit == true)
	{
		gamemap = 9;
	}
	else if(gamemap == 9)
	{ // Finished secret level
		gamemap = afterSecret[gameepisode-1];
	}
	else if(gamemap == 8)
	{
		gameaction = ga_victory;
		return;
	}
	else
	{
		gamemap++;
	}
	gamestate = GS_INTERMISSION;
	IN_Start();
*/
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
    gamestate = GS_LEVEL;
    G_DoLoadLevel();
    gameaction = ga_nothing;
    viewactive = true;
}

//==========================================================================
//
// G_DoSingleReborn
//
// Called by G_Ticker based on gameaction.  Loads a game from the reborn
// save slot.
//
//==========================================================================

void G_DoSingleReborn(void)
{
    gameaction = ga_nothing;
    SV_LoadGame(SV_GetRebornSlot());
    SB_SetClassData();
}

//==========================================================================
//
// G_LoadGame
//
// Can be called by the startup code or the menu task.
//
//==========================================================================

static int GameLoadSlot;

void G_LoadGame(int slot)
{
    GameLoadSlot = slot;
    gameaction = ga_loadgame;
}

//==========================================================================
//
// G_DoLoadGame
//
// Called by G_Ticker based on gameaction.
//
//==========================================================================

void G_DoLoadGame(void)
{
    gameaction = ga_nothing;
    SV_LoadGame(GameLoadSlot);
    if (!netgame)
    {                           // Copy the base slot to the reborn slot
        SV_UpdateRebornSlot();
    }
    SB_SetClassData();
}

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
    SV_SaveGame(savegameslot, savedescription);
    gameaction = ga_nothing;
    savedescription[0] = 0;
    P_SetMessage(&players[consoleplayer], english_language ?
                                          TXT_GAMESAVED :
                                          TXT_GAMESAVED_RUS,
                                          true);
}

//==========================================================================
//
// G_DeferredNewGame
//
//==========================================================================

void G_DeferredNewGame(skill_t skill)
{
    TempSkill = skill;
    gameaction = ga_newgame;
}

//==========================================================================
//
// G_DoNewGame
//
//==========================================================================

void G_DoNewGame(void)
{
    G_StartNewGame(TempSkill);
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
    //Class
    PlayerClass[consoleplayer] = selective_class;
    p->class = selective_class;
    SB_SetClassData();
    SB_state = -1;

    demoplayback = false;
    netgame = false;
    deathmatch = false;
    playeringame[1] = playeringame[2] = playeringame[3] = 0;
    consoleplayer = 0;
    gameaction = ga_nothing;
    automapactive = false;

    // Close "Level select" menu
    RD_Menu_DeactivateMenu ();

    // Start a new game
    G_InitNew (selective_skill,selective_episode, P_TranslateMap(selective_map));

    // Health
    p->health = selective_health;
    p->mo->health = selective_health;

    // Armor
    p->armorpoints[ARMOR_ARMOR] = selective_armor_0 * 5 * FRACUNIT;
    p->armorpoints[ARMOR_SHIELD] = selective_armor_1 * 5 * FRACUNIT;
    p->armorpoints[ARMOR_HELMET] = selective_armor_2 * 5 * FRACUNIT;
    p->armorpoints[ARMOR_AMULET] = selective_armor_3 * 5 * FRACUNIT;

    // Weapons
    p->weaponowned[WP_FIRST]  = 1;
    p->weaponowned[WP_SECOND] = selective_wp_second;
    p->weaponowned[WP_THIRD]  = selective_wp_third;
    p->weaponowned[WP_FOURTH] = selective_wp_fourth;
    p->pieces |= selective_wp_piece_2;
    p->pieces |= selective_wp_piece_1 << 1;
    p->pieces |= selective_wp_piece_0 << 2;

    // Ammo
    p->mana[0] = selective_ammo_0; // blue mana
    p->mana[1] = selective_ammo_1; // green mana

    // Artifacts
    for (i = 0 ; i < selective_arti_0 ; i++) P_GiveArtifact(p, arti_health, NULL);          // Quartz Flask
    for (i = 0 ; i < selective_arti_1 ; i++) P_GiveArtifact(p, arti_superhealth, NULL);     // Mystic Urn
    for (i = 0 ; i < selective_arti_2 ; i++) P_GiveArtifact(p, arti_poisonbag, NULL);       // FLECHETTE
    for (i = 0 ; i < selective_arti_3 ; i++) P_GiveArtifact(p, arti_blastradius, NULL);     // DISC OF REPULSION
    for (i = 0 ; i < selective_arti_4 ; i++) P_GiveArtifact(p, arti_invulnerability, NULL); // ICON OF THE DEFENDER
    for (i = 0 ; i < selective_arti_5 ; i++) P_GiveArtifact(p, arti_egg, NULL);             // PORKALATOR
    for (i = 0 ; i < selective_arti_6 ; i++) P_GiveArtifact(p, arti_teleport, NULL);        // CHAOS DEVICE
    for (i = 0 ; i < selective_arti_7 ; i++) P_GiveArtifact(p, arti_teleportother, NULL);   // BANISHMENT DEVICE
    for (i = 0 ; i < selective_arti_8 ; i++) P_GiveArtifact(p, arti_fly, NULL);             // WINGS OF WRATH
    for (i = 0 ; i < selective_arti_9 ; i++) P_GiveArtifact(p, arti_torch, NULL);           // TORCH
    for (i = 0 ; i < selective_arti_10 ; i++) P_GiveArtifact(p, arti_boostmana, NULL);      // KRATER OF MIGHT
    for (i = 0 ; i < selective_arti_11 ; i++) P_GiveArtifact(p, arti_boostarmor, NULL);     // DRAGONSKIN BRACERS
    for (i = 0 ; i < selective_arti_12 ; i++) P_GiveArtifact(p, arti_summon, NULL);         // DARK SERVANT
    for (i = 0 ; i < selective_arti_13 ; i++) P_GiveArtifact(p, arti_speed, NULL);          // BOOTS OF SPEED
    for (i = 0 ; i < selective_arti_14 ; i++) P_GiveArtifact(p, arti_healingradius, NULL);  // MYSTIC AMBIT INCANT

    // Keys
    p->keys |= selective_key_0 << KEY_5;  // EMERALD KEY
    p->keys |= selective_key_1 << KEY_7;  // SILVER KEY
    p->keys |= selective_key_2 << KEY_4;  // FIRE KEY
    p->keys |= selective_key_3 << KEY_1;  // STEEL KEY
    p->keys |= selective_key_4 << KEY_9;  // HORN KEY
    p->keys |= selective_key_5 << KEY_2;  // CAVE KEY
    p->keys |= selective_key_6 << KEY_B;  // CASTLE KEY
    p->keys |= selective_key_7 << KEY_A;  // SWAMP KEY
    p->keys |= selective_key_8 << KEY_8;  // RUSTED KEY
    p->keys |= selective_key_9 << KEY_6;  // DUNGEON KEY
    p->keys |= selective_key_10 << KEY_3; // AXE KEY

    // Puzzles
    for (i = 0 ; i < selective_puzzle_0 ; i++) P_GiveArtifact(p, arti_puzzskull2, NULL);    // FLAME MASK
    for (i = 0 ; i < selective_puzzle_1 ; i++) P_GiveArtifact(p, arti_puzzgembig, NULL);    // HEART OF D'SPARIL
    for (i = 0 ; i < selective_puzzle_2 ; i++) P_GiveArtifact(p, arti_puzzgemred, NULL);    // RUBY PLANET
    for (i = 0 ; i < selective_puzzle_3 ; i++) P_GiveArtifact(p, arti_puzzgemgreen1, NULL); // EMERALD PLANET 1
    for (i = 0 ; i < selective_puzzle_4 ; i++) P_GiveArtifact(p, arti_puzzgemgreen2, NULL); // EMERALD PLANET 2
    for (i = 0 ; i < selective_puzzle_5 ; i++) P_GiveArtifact(p, arti_puzzgemblue1, NULL);  // SAPPHIRE PLANET 1
    for (i = 0 ; i < selective_puzzle_6 ; i++) P_GiveArtifact(p, arti_puzzgemblue2, NULL);  // SAPPHIRE PLANET 2
    for (i = 0 ; i < selective_puzzle_7 ; i++) P_GiveArtifact(p, arti_puzzgear1, NULL);     // CLOCK GEAR ((S)
    for (i = 0 ; i < selective_puzzle_8 ; i++) P_GiveArtifact(p, arti_puzzgear2, NULL);     // CLOCK GEAR ((B)
    for (i = 0 ; i < selective_puzzle_9 ; i++) P_GiveArtifact(p, arti_puzzgear3, NULL);     // CLOCK GEAR ((S B)
    for (i = 0 ; i < selective_puzzle_10 ; i++) P_GiveArtifact(p, arti_puzzgear4, NULL);    // CLOCK GEAR ((B S)
    for (i = 0 ; i < selective_puzzle_11 ; i++) P_GiveArtifact(p, arti_puzzbook1, NULL);    // DAEMON CODEX
    for (i = 0 ; i < selective_puzzle_12 ; i++) P_GiveArtifact(p, arti_puzzbook2, NULL);    // LIBER OSCURA
    for (i = 0 ; i < selective_puzzle_13 ; i++) P_GiveArtifact(p, arti_puzzskull, NULL);    // YORICK'S SKULL
    for (i = 0 ; i < selective_puzzle_14 ; i++) P_GiveArtifact(p, arti_puzzfweapon, NULL);  // GLAIVE SEAL
    for (i = 0 ; i < selective_puzzle_15 ; i++) P_GiveArtifact(p, arti_puzzcweapon, NULL);  // HOlY RELIC
    for (i = 0 ; i < selective_puzzle_16 ; i++) P_GiveArtifact(p, arti_puzzmweapon, NULL);  // SIGIL OF THE MAGUS
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

void G_DeferedInitNew(skill_t skill, int episode, int map)
{
    TempSkill = skill;
    TempEpisode = episode;
    TempMap = map;
    gameaction = ga_initnew;
}

void G_DoInitNew(void)
{
    SV_InitBaseSlot();
    G_InitNew(TempSkill, TempEpisode, TempMap);
    gameaction = ga_nothing;
}

void G_InitNew(skill_t skill, int episode, int map)
{
    int i;

    if (paused)
    {
        paused = false;
        S_ResumeSound();
    }
    if (skill < sk_baby)
    {
        skill = sk_baby;
    }
    if (skill > sk_ultranm)
    {
        skill = sk_ultranm;
    }
    if (map < 1)
    {
        map = 1;
    }
    if (map > 99)
    {
        map = 99;
    }
    M_ClearRandom();
    // Force players to be initialized upon first level load
    for (i = 0; i < maxplayers; i++)
    {
        players[i].playerstate = PST_REBORN;
        players[i].worldTimer = 0;
    }

    // Set up a bunch of globals
    if (!demoextend)
    {
        // This prevents map-loading from interrupting a demo.
        // demoextend is set back to false only if starting a new game or
        // loading a saved one from the menu, and only during playback.
        demorecording = false;
        demoplayback = false;
        usergame = true;            // will be set false if a demo
    }

    // [JN] Ultra-Nightmare definitions
    if (skill == sk_ultranm)
    {
        // Monster's speed                          // Old value   Who
        mobjinfo[MT_FIREDEMON].speed = 20;          // 13          Affrit
        
        // Monster's missiles speed                             // Old value    What
        mobjinfo[MT_FIREDEMON_FX6].speed = 13 * FRACUNIT;       // 10           Affrit fireball
        mobjinfo[MT_DEMON2FX1].speed = 18 * FRACUNIT;           // 15           Brown Chaos Serpent fireball
        mobjinfo[MT_BISH_FX].speed = 13 * FRACUNIT;             // 10           Dark Bishop fireball
        mobjinfo[MT_DEMONFX1].speed = 18 * FRACUNIT;            // 15           Green Chaos Serpent fireball
        mobjinfo[MT_WRAITHFX1].speed = 17 * FRACUNIT;           // 14           Reiver fireball
        mobjinfo[MT_CENTAUR_FX].speed = 23 * FRACUNIT;          // 20           Slaughtaur fireball
        mobjinfo[MT_SERPENTFX].speed = 18 * FRACUNIT;           // 15           Stalker fireball
        mobjinfo[MT_ICEGUY_FX].speed = 17 * FRACUNIT;           // 14           Wendige fireball 1
        mobjinfo[MT_ICEGUY_FX2].speed = 13 * FRACUNIT;          // 10           Wendige fireball 2
        
        // Monster's damage                         // Old value   What
        mobjinfo[MT_FIREDEMON_FX6].damage = 2;      // 1           Fire Gargolye fireball
        mobjinfo[MT_DEMON2FX1].damage = 6;          // 5           Brown Chaos Serpent fireball
        mobjinfo[MT_BISH_FX].damage = 2;            // 1           Dark Bishop fireball
        mobjinfo[MT_DEMONFX1].damage = 6;           // 5           Green Chaos Serpent fireball
        mobjinfo[MT_WRAITHFX1].damage = 6;          // 5           Reiver fireball
        mobjinfo[MT_CENTAUR_FX].damage = 5;         // 4           Slaughtaur fireball
        mobjinfo[MT_SERPENTFX].damage = 5;          // 4           Stalker fireball
    }
    // [JN] Fallback to standard values
    else
    {
        // Monster's speed
        mobjinfo[MT_FIREDEMON].speed = 13;
        
        // Monster's missiles speed
        mobjinfo[MT_FIREDEMON_FX6].speed = 10 * FRACUNIT;
        mobjinfo[MT_DEMON2FX1].speed = 15 * FRACUNIT;
        mobjinfo[MT_BISH_FX].speed = 10 * FRACUNIT;
        mobjinfo[MT_DEMONFX1].speed = 15 * FRACUNIT;
        mobjinfo[MT_WRAITHFX1].speed = 14 * FRACUNIT;
        mobjinfo[MT_CENTAUR_FX].speed = 20 * FRACUNIT;
        mobjinfo[MT_SERPENTFX].speed = 15 * FRACUNIT;
        mobjinfo[MT_ICEGUY_FX].speed = 14 * FRACUNIT;
        mobjinfo[MT_ICEGUY_FX2].speed = 10 * FRACUNIT;
        
        // Monster's damage
        mobjinfo[MT_FIREDEMON_FX6].damage = 1;
        mobjinfo[MT_DEMON2FX1].damage = 5;
        mobjinfo[MT_BISH_FX].damage = 1;
        mobjinfo[MT_DEMONFX1].damage = 5;
        mobjinfo[MT_WRAITHFX1].damage = 5;
        mobjinfo[MT_CENTAUR_FX].damage = 4;
        mobjinfo[MT_SERPENTFX].damage = 4;
    }

    paused = false;
    // [JN] Reset automap scale. Fixes:
    // https://doomwiki.org/wiki/Automap_scale_preserved_after_warps_in_Heretic_and_Hexen
    automapactive = false; 
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;
    BorderNeedRefresh = true;

    // Initialize the sky
    R_InitSky(map);

    // Give one null ticcmd_t
    //gametic = 0;
    //maketic = 1;
    //for (i=0 ; i<maxplayers ; i++)
    //      nettics[i] = 1; // one null event for this gametic
    //memset (localcmds,0,sizeof(localcmds));
    //memset (netcmds,0,sizeof(netcmds));

    G_DoLoadLevel();
}

/*
===============================================================================

							DEMO RECORDING

===============================================================================
*/

#define DEMOMARKER      0x80
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

    G_InitNew(skill, episode, map);
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
    // This aligns with vvHeretic demo usage. Hexen demo support has no
    // precedent here so consistency with another game is chosen:
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
    *demo_p++ = PlayerClass[0];

    for (i = 1; i < maxplayers; i++)
    {
        *demo_p++ = playeringame[i];
        *demo_p++ = PlayerClass[i];
    }

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

    // When recording we store some extra options inside the upper bits
    // of the player 1 present byte. However, this is a non-vanilla extension.
    // Note references to vvHeretic here; these are the extensions used by
    // vvHeretic, which we're just reusing for Hexen demos too. There is no
    // vvHexen.
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

    for (i = 0; i < maxplayers; i++)
    {
        playeringame[i] = (*demo_p++) != 0;
        PlayerClass[i] = *demo_p++;
    }

    // Initialize world info, etc.
    G_StartNewInit();

    precache = false;           // don't spend a lot of time in loadlevel
    G_InitNew(skill, episode, map);
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

    for (i = 0; i < maxplayers; i++)
    {
        playeringame[i] = (*demo_p++) != 0;
        PlayerClass[i] = *demo_p++;
    }

    G_InitNew(skill, episode, map);
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
        I_Error (english_language ?
                 "Timed %i gametics in %i realtics (%f fps)" :
                 "Насчитано %i gametics в %i realtics.\n Среднее значение FPS: %f.",
                 gametic, realtics, fps);
    }

    if (demoplayback)
    {
        if (singledemo)
            I_Quit();

        W_ReleaseLumpName(defdemoname);
        demoplayback = false;
        H2_AdvanceDemo();
        return true;
    }

    if (demorecording)
    {
        *demo_p++ = DEMOMARKER;
        M_WriteFile(demoname, demobuffer, demo_p - demobuffer);
        Z_Free(demobuffer);
        demorecording = false;
        I_Error(english_language?
                "Demo %s recorded" :
                "Демозапись %s завершена",
                demoname);
    }

    return false;
}
