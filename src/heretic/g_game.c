//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

// Russian Doom (C) 2016-2018 Julian Nechaevsky


// G_game.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "doomkeys.h"
#include "deh_str.h"
#include "i_input.h"
#include "i_timer.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "jn.h"

// Macros

#define AM_STARTKEY     9

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

int mouseSensitivity;

char demoname[32];
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


// [JN] Дополнительные параметры игры

// Графика
int brightmaps = 1;
int fake_contrast = 0;
int invul_sky = 1;
int draw_shadowed_text = 1;

// Звук
int snd_monomode = 0;

// Тактика
int automap_stats = 1;
int secret_notification = 1;

// Физика
int torque = 1;
int weapon_bobbing = 1;
int randomly_flipcorpses = 1;

// Геймплей
int flip_levels = 0;

// Прицел
int crosshair_draw = 0;
int crosshair_health = 0;
int crosshair_scale = 0;


//
// controls (have defaults)
//


#define MAXPLMOVE       0x32

fixed_t forwardmove[2] = { 0x19, 0x32 };
fixed_t sidemove[2] = { 0x18, 0x28 };
fixed_t angleturn[3] = { 640, 1280, 320 };      // + slow turn

static int *weapon_keys[] =
{
    &key_weapon1,
    &key_weapon2,
    &key_weapon3,
    &key_weapon4,
    &key_weapon5,
    &key_weapon6,
    &key_weapon7
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

#define NUMKEYS 256
boolean gamekeydown[NUMKEYS];
int turnheld;                   // for accelerative turning
int lookheld;


boolean mousearray[MAX_MOUSE_BUTTONS + 1];
boolean *mousebuttons = &mousearray[1];
        // allow [-1]
int mousex, mousey;             // mouse values are used once
int dclicktime, dclickstate, dclicks;
int dclicktime2, dclickstate2, dclicks2;

#define MAX_JOY_BUTTONS 20

int joyxmove, joyymove;         // joystick values are repeated
int joystrafemove;
boolean joyarray[MAX_JOY_BUTTONS + 1];
boolean *joybuttons = &joyarray[1];     // allow [-1]

int savegameslot;
char savedescription[32];

int inventoryTics;

// haleyjd: removed WATCOMC

//=============================================================================
// Not used - ripped out for Heretic
/*
int G_CmdChecksum(ticcmd_t *cmd)
{
	int     i;
	int sum;

	sum = 0;
	for(i = 0; i < sizeof(*cmd)/4-1; i++)
	{
		sum += ((int *)cmd)[i];
	}
	return(sum);
}
*/

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

// [JN] Небольшой хак, при котором в режиме Always Run и нажатии кнопки
// бега игрок переходит на шаг.

// [crispy] holding down the "Run" key may trigger special behavior,
// e.g. quick exit, clean screenshots, resurrection from savegames
boolean speedkeydown (void)
{
    return (key_speed < NUMKEYS && gamekeydown[key_speed]) ||
           (joybspeed < MAX_JOY_BUTTONS && joybuttons[joybspeed]);
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
    boolean strafe, bstrafe;
    int speed, tspeed, lspeed;
    int forward, side;
    int look, arti;
    int flyheight;
    static int  joybspeed_old = 2;
    extern boolean askforquit;
    extern boolean noartiskip;

    // haleyjd: removed externdriver crap

    memset(cmd, 0, sizeof(*cmd));
    //cmd->consistancy =
    //      consistancy[consoleplayer][(maketic*ticdup)%BACKUPTICS];
    cmd->consistancy = consistancy[consoleplayer][maketic % BACKUPTICS];

//printf ("cons: %i\n",cmd->consistancy);

    strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe]
        || joybuttons[joybstrafe];
		
	// [crispy] when "always run" is active,
    // pressing the "run" key will result in walking
    speed = key_speed >= NUMKEYS
         || joybspeed >= MAX_JOY_BUTTONS;
//       || gamekeydown[key_speed] 
//       || joybuttons[joybspeed];
	
	// [JN] Модификатор мнопки бега
	speed ^= speedkeydown();

    // haleyjd: removed externdriver crap
    
    forward = side = look = arti = flyheight = 0;

//
// use two stage accelerative turning on the keyboard and joystick
//
    if (joyxmove < 0 || joyxmove > 0
        || gamekeydown[key_right] || gamekeydown[key_left])
        turnheld += ticdup;
    else
        turnheld = 0;
    if (turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

    if (gamekeydown[key_lookdown] || gamekeydown[key_lookup])
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
    if (gamekeydown[key_toggleautorun])
    {
        if (joybspeed >= MAX_JOY_BUTTONS)
        {
            joybspeed = joybspeed_old;
        }
        else
        {
            joybspeed_old = joybspeed;
            joybspeed = 29;
        }

        // [JN] Added audible feedback
        P_SetMessage(&players[consoleplayer], (joybspeed >= MAX_JOY_BUTTONS) ? TXT_ALWAYSRUN_ON : TXT_ALWAYSRUN_OFF, false);
        S_StartSound(NULL, sfx_chat);

        gamekeydown[key_toggleautorun] = false;
    }

    // [JN] Toggle crosshair
    if (gamekeydown[key_togglecrosshair])
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

        P_SetMessage(&players[consoleplayer], crosshair_draw ? TXT_CROSSHAIR_ON : TXT_CROSSHAIR_OFF, false);
        S_StartSound(NULL, sfx_chat);

        gamekeydown[key_togglecrosshair] = false;
    }

//
// let movement keys cancel each other out
//
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
    if (gamekeydown[key_straferight] || mousebuttons[mousebstraferight]
     || joybuttons[joybstraferight] || joystrafemove > 0)
        side += sidemove[speed];
    if (gamekeydown[key_strafeleft] || mousebuttons[mousebstrafeleft]
     || joybuttons[joybstrafeleft] || joystrafemove < 0)
        side -= sidemove[speed];

    // Look up/down/center keys
    if (gamekeydown[key_lookup])
    {
        look = lspeed;
    }
    if (gamekeydown[key_lookdown])
    {
        look = -lspeed;
    }
    // haleyjd: removed externdriver crap
    if (gamekeydown[key_lookcenter])
    {
        look = TOCENTER;
    }

    // haleyjd: removed externdriver crap
    
    // Fly up/down/drop keys
    if (gamekeydown[key_flyup])
    {
        flyheight = 5;          // note that the actual flyheight will be twice this
    }
    if (gamekeydown[key_flydown])
    {
        flyheight = -5;
    }
    if (gamekeydown[key_flycenter])
    {
        flyheight = TOCENTER;
        // haleyjd: removed externdriver crap
        look = TOCENTER;
    }

    // Use artifact key
    if (gamekeydown[key_useartifact])
    {
        if (gamekeydown[key_speed] && !noartiskip)
        {
            if (players[consoleplayer].inventory[inv_ptr].type != arti_none)
            {
                gamekeydown[key_useartifact] = false;
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
    if (gamekeydown[127] && !cmd->arti
        && !players[consoleplayer].powers[pw_weaponlevel2])
    {
        gamekeydown[127] = false;
        cmd->arti = arti_tomeofpower;
    }

    // [JN] Keys for Artifacts
    if (gamekeydown[key_arti_quartz] && !cmd->arti && (players[consoleplayer].mo->health < MAXHEALTH))
    {
        gamekeydown[key_arti_quartz] = false;
        cmd->arti = arti_health;
    }
    else if (gamekeydown[key_arti_urn] && !cmd->arti && (players[consoleplayer].mo->health < MAXHEALTH))
    {
        gamekeydown[key_arti_urn] = false;
        cmd->arti = arti_superhealth;
    }
    else if (gamekeydown[key_arti_bomb] && !cmd->arti)
    {
        gamekeydown[key_arti_bomb] = false;
        cmd->arti = arti_firebomb;
    }
    else if (gamekeydown[key_arti_tome] && !cmd->arti)
    {
        gamekeydown[key_arti_tome] = false;
        cmd->arti = arti_tomeofpower;
    }
    else if (gamekeydown[key_arti_ring] && !cmd->arti)
    {
        gamekeydown[key_arti_ring] = false;
        cmd->arti = arti_invulnerability;
    }
    else if (gamekeydown[key_arti_morphegg] && !cmd->arti)
    {
        gamekeydown[key_arti_morphegg] = false;
        cmd->arti = arti_egg;
    }
    else if (gamekeydown[key_arti_chaosdevice] && !cmd->arti)
    {
        gamekeydown[key_arti_chaosdevice] = false;
        cmd->arti = arti_teleport;
    }
    else if (gamekeydown[key_arti_shadowsphere] && !cmd->arti)
    {
        gamekeydown[key_arti_shadowsphere] = false;
        cmd->arti = arti_invisibility;
    }
    else if (gamekeydown[key_arti_wings] && !cmd->arti)
    {
        gamekeydown[key_arti_wings] = false;
        cmd->arti = arti_fly;
    }
    else if (gamekeydown[key_arti_torch] && !cmd->arti)
    {
        gamekeydown[key_arti_torch] = false;
        cmd->arti = arti_torch;
    }

//
// buttons
//
    cmd->chatchar = CT_dequeueChatChar();

    if (gamekeydown[key_fire] || mousebuttons[mousebfire]
        || joybuttons[joybfire])
        cmd->buttons |= BT_ATTACK;

    if (gamekeydown[key_use] || joybuttons[joybuse] || mousebuttons[mousebuse])
    {
        cmd->buttons |= BT_USE;
        dclicks = 0;            // clear double clicks if hit use button
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
            int key = *weapon_keys[i];

            if (gamekeydown[key])
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
    if (mousebuttons[mousebforward])
    {
        forward += forwardmove[speed];
    }

    if (mousebuttons[mousebbackward])
    {
	forward -= forwardmove[speed];
    }

    // Double click to use can be disabled 
   
    if (dclick_use)
    {
	//
	// forward double click
	//
	if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1)
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

	//
	// strafe double click
	//

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
    }

    if (strafe)
    {
        side += mousex * 2;
    }
    else
    {
        cmd->angleturn -= mousex * 0x8;
    }

    // No mouse movement in previous frame?

    if (mousex == 0)
    {
        testcontrols_mousespeed = 0;
    }

    // [JN] Mouselook: initials
    // TODO: make it safe for network game
    if (mlook && !netgame && !demoplayback && players[consoleplayer].playerstate == PST_LIVE && !menuactive && !askforquit && !paused)
    {
        cmd->lookdir += mousey;
        
         if (players[consoleplayer].lookdir > LOOKDIRMAX * MLOOKUNIT)
             players[consoleplayer].lookdir = LOOKDIRMAX * MLOOKUNIT;
         else
         if (players[consoleplayer].lookdir < -LOOKDIRMIN * MLOOKUNIT)
             players[consoleplayer].lookdir = -LOOKDIRMIN * MLOOKUNIT;
    }

    // [JN] Mouselook: toggling
    if (gamekeydown[key_togglemlook])
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

        P_SetMessage(&players[consoleplayer], (mlook == true ? TXT_MLOOK_ON : TXT_MLOOK_OFF), false);
        S_StartSound(NULL, sfx_chat);

        gamekeydown[key_togglemlook] = false;
    }

    mousex = mousey = 0;

    // [JN] Toggle local time widget
    if (gamekeydown[key_toggletime])
    {
        if (!local_time)
        {
            local_time = true;
        }
        else
        {
            local_time = false;
        }
       
        gamekeydown[key_toggletime] = false;
    }    

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

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

    P_SetupLevel(gameepisode, gamemap, 0, gameskill);
    displayplayer = consoleplayer;      // view the guy you are playing
    gameaction = ga_nothing;
    Z_CheckHeap();

//
// clear cmd building stuff
//

    memset(gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = joystrafemove = 0;
    mousex = mousey = 0;
    sendpause = sendsave = paused = false;
    memset(mousearray, 0, sizeof(mousearray));
    memset(joyarray, 0, sizeof(joyarray));

    if (testcontrols)
    {
        P_SetMessage(&players[consoleplayer], "HT;BV GHJDTHRB EGHFDKTYBZ", false);	// РЕЖИМ ПРОВЕРКИ УПРАВЛЕНИЯ
    }
}

static void SetJoyButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_JOY_BUTTONS; ++i)
    {
        int button_on = (buttons_mask & (1 << i)) != 0;

        // Detect button press:

        if (!joybuttons[i] && button_on)
        {
            // Weapon cycling:

            if (i == joybprevweapon)
            {
                next_weapon = -1;
            }
            else if (i == joybnextweapon)
            {
                next_weapon = 1;
            }
        }

        joybuttons[i] = button_on;
    }
}

static void SetMouseButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_MOUSE_BUTTONS; ++i)
    {
        unsigned int button_on = (buttons_mask & (1 << i)) != 0;

        // Detect button press:

        if (!mousebuttons[i] && button_on)
        {
            if (i == mousebprevweapon)
            {
                next_weapon = -1;
            }
            else if (i == mousebnextweapon)
            {
                next_weapon = 1;
            }
        }

        mousebuttons[i] = button_on;
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
    if (ev->type == ev_keyup && ev->data1 == key_useartifact)
    {                           // flag to denote that it's okay to use an artifact
        if (!inventory)
        {
            plr->readyArtifact = plr->inventory[inv_ptr].type;
        }
        usearti = true;
    }

    // Check for spy mode player cycle
    if (gamestate == GS_LEVEL && ev->type == ev_keydown
        && ev->data1 == KEY_F12 && !deathmatch)
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

    if (ev->type == ev_mouse)
    {
        testcontrols_mousespeed = abs(ev->data2);
    }

    if (ev->type == ev_keydown && ev->data1 == key_prevweapon)
    {
        next_weapon = -1;
    }
    else if (ev->type == ev_keydown && ev->data1 == key_nextweapon)
    {
        next_weapon = 1;
    }

    switch (ev->type)
    {
        case ev_keydown:
            if (ev->data1 == key_invleft)
            {
                inventoryTics = 5 * 35;
                if (!inventory)
                {
                    inventory = true;
                    break;
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
                return (true);
            }
            if (ev->data1 == key_invright)
            {
                inventoryTics = 5 * 35;
                if (!inventory)
                {
                    inventory = true;
                    break;
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
                return (true);
            }
            if (ev->data1 == key_pause && !menuactive)
            {
                sendpause = true;
                return (true);
            }
            if (ev->data1 < NUMKEYS)
            {
                gamekeydown[ev->data1] = true;
            }
            return (true);      // eat key down events

        case ev_keyup:
            if (ev->data1 < NUMKEYS)
            {
                gamekeydown[ev->data1] = false;
            }
            return (false);     // always let key up events filter down

        case ev_mouse:
            SetMouseButtons(ev->data1);
            mousex = ev->data2 * (mouseSensitivity + 5) / 10;
            mousey = ev->data3 * (mouseSensitivity + 5) / 10;
            return (true);      // eat events

        case ev_joystick:
            SetJoyButtons(ev->data1);
            joyxmove = ev->data2;
            joyymove = ev->data3;
            joystrafemove = ev->data4;
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
                break;
            case ga_savegame:
                G_DoSaveGame();
                break;
            case ga_playdemo:
                G_DoPlayDemo();
                break;
            case ga_screenshot:
                V_ScreenShot("HTIC%02i.%s");
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
    //buf = gametic%BACKUPTICS;
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
                    I_Error("Нарушение последовательности (%i должно быть %i)",
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
                                M_StringCopy(savedescription,
                                             DEH_String("CTNTDFZ BUHF"),	// СЕТЕВАЯ ИГРА
                                             sizeof(savedescription));
                            }
                            else
                            {
                                M_StringCopy(savedescription,
                                             DEH_String("CJ[HFYBNM BUHE"),	// СОХРАНИТЬ ИГРУ
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

/*      // BIG HACK
	inv_ptr = 0;
	curpos = 0;
*/
    // END HACK
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
//      memset(p->inventory, 0, sizeof(p->inventory));
    if (p->chickenTics)
    {
        p->readyweapon = p->mo->special1.i;       // Restore weapon
        p->chickenTics = 0;
    }
    p->messageTics = 0;
    p->lookdir = 0;
    p->mo->flags &= ~MF_SHADOW; // Remove invisibility
    p->extralight = 0;          // Remove weapon flashes
    p->fixedcolormap = 0;       // Remove torch
    p->damagecount = 0;         // No palette changes
    p->bonuscount = 0;
    p->rain1 = NULL;
    p->rain2 = NULL;
    if (p == &players[consoleplayer])
    {
        SB_state = -1;          // refresh the status bar
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
        SB_state = -1;          // refresh the status bar
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
        I_Error ("Обнаружено %i стартовых точек для режима Дефтатч.\n"
                 "Минимальное необходимое количество: 4", selections); 

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
        gameaction = ga_loadlevel;      // reload the level from scratch
    else
    {                           // respawn at the start
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
    else if (gamemap == 8)
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
    {                           // Bad version
        return;
    }
    gameskill = SV_ReadByte();
    gameepisode = SV_ReadByte();
    gamemap = SV_ReadByte();
    for (i = 0; i < MAXPLAYERS; i++)
    {
        playeringame[i] = SV_ReadByte();
    }
    // Load a base level
    G_InitNew(gameskill, gameepisode, gamemap);

    // Create leveltime
    a = SV_ReadByte();
    b = SV_ReadByte();
    c = SV_ReadByte();
    leveltime = (a << 16) + (b << 8) + c;

    // De-archive all the modifications
    P_UnArchivePlayers();
    P_UnArchiveWorld();
    P_UnArchiveThinkers();
    P_UnArchiveSpecials();
    P_RestoreTargets();

    if (SV_ReadByte() != SAVE_GAME_TERMINATOR)
    {                           // Missing savegame termination marker
        I_Error("Некорректный файл сохранения");
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
    G_InitNew(d_skill, d_episode, d_map);
    gameaction = ga_nothing;
}

void G_InitNew(skill_t skill, int episode, int map)
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
    speed = skill == sk_nightmare;
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

        // Monster's missiles speed                     // Old value    What
        mobjinfo[MT_IMPBALL].speed = 13 * FRACUNIT;     // 10           Fire Gargolye fireball
        mobjinfo[MT_MUMMYFX1].speed = 12 * FRACUNIT;    // 9            Nitrogolem Ghost fireball
        mobjinfo[MT_KNIGHTAXE].speed = 12 * FRACUNIT;   // 9            Undead Warrior green axe
        mobjinfo[MT_REDAXE].speed = 12 * FRACUNIT;      // 9            Undead Warrior green axe
        mobjinfo[MT_BEASTBALL].speed = 15 * FRACUNIT;   // 12           Weredragon fireball
        mobjinfo[MT_SNAKEPRO_A].speed = 16 * FRACUNIT;  // 14           Ophidian fireball A
        mobjinfo[MT_SNAKEPRO_B].speed = 16 * FRACUNIT;  // 14           Ophidian fireball B
        mobjinfo[MT_WIZFX1].speed = 20 * FRACUNIT;      // 18           Disciple of D'Sparil fireball
        mobjinfo[MT_HEADFX1].speed = 16 * FRACUNIT;     // 13           Iron Lich missilie 1
        mobjinfo[MT_HEADFX2].speed = 12 * FRACUNIT;     // 8            Iron Lich missilie 2
        mobjinfo[MT_HEADFX3].speed = 13 * FRACUNIT;     // 10           Iron Lich missilie 3
        mobjinfo[MT_WHIRLWIND].speed = 14 * FRACUNIT;   // 10           Iron Lich whirlwind
        
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

        // Monster's missiles speed
        mobjinfo[MT_IMPBALL].speed = 10 * FRACUNIT;
        mobjinfo[MT_MUMMYFX1].speed = 9 * FRACUNIT;
        mobjinfo[MT_KNIGHTAXE].speed = 9 * FRACUNIT;
        mobjinfo[MT_REDAXE].speed = 9 * FRACUNIT;
        mobjinfo[MT_BEASTBALL].speed = 12 * FRACUNIT;
        mobjinfo[MT_SNAKEPRO_A].speed = 14 * FRACUNIT;
        mobjinfo[MT_SNAKEPRO_B].speed = 14 * FRACUNIT;
        mobjinfo[MT_WIZFX1].speed = 18 * FRACUNIT;
        mobjinfo[MT_HEADFX1].speed = 13 * FRACUNIT;
        mobjinfo[MT_HEADFX2].speed = 8 * FRACUNIT;
        mobjinfo[MT_HEADFX3].speed = 10 * FRACUNIT;
        mobjinfo[MT_WHIRLWIND].speed = 10 * FRACUNIT;
        
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
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;
    viewactive = true;
    BorderNeedRefresh = true;

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

    if (flip_levels)
    {
        cmd->sidemove *= (const signed char) -1;
        cmd->angleturn *= (const short) -1;
    }
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

    if (flip_levels)
    {
        cmd->sidemove *= (const signed char) -1;
        cmd->angleturn *= (const short) -1;
    }

    if (gamekeydown[key_demo_quit]) // press to end demo recording
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
    M_StringCopy(demoname, name, sizeof(demoname));
    M_StringConcat(demoname, ".lmp", sizeof(demoname));
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

    for (i = 0; i < MAXPLAYERS; i++)
    {
        playeringame[i] = (*demo_p++) != 0;
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
        I_Error ("Насчитано %i gametics в %i realtics.\n"
                 "Среднее значение FPS: %f.", gametic, realtics, fps);
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
        I_Error("Демозапись %s завершена",demoname); 
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
    for (i = 0; i < MAXPLAYERS; i++)
    {
        SV_WriteByte(playeringame[i]);
    }
    SV_WriteByte(leveltime >> 16);
    SV_WriteByte(leveltime >> 8);
    SV_WriteByte(leveltime);
    P_ArchivePlayers();
    P_ArchiveWorld();
    P_ArchiveThinkers();
    P_ArchiveSpecials();
    SV_Close(filename);

    gameaction = ga_nothing;
    savedescription[0] = 0;
    P_SetMessage(&players[consoleplayer], DEH_String(TXT_GAMESAVED), true);

    free(filename);
}

