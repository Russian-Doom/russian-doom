//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//


#include <stdio.h>

#include "i_system.h"
#include "z_zone.h"
#include "m_misc.h"
#include "w_wad.h"
#include "doomdef.h"
#include "g_game.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"
#include "am_map.h"
#include "dutils.h"
#include "s_sound.h"
#include "v_video.h"
#include "doomstat.h"
#include "sounds.h"
#include "m_menu.h"
#include "v_trans.h"
#include "rd_lang.h"
#include "jn.h"


//
// STATUS BAR DATA
//


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS        1
#define STARTBONUSPALS      9
#define NUMREDPALS          8
#define NUMBONUSPALS        4

// Radiation suit, green shift.
#define RADIATIONPAL        13

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY  96

// For Responder
#define ST_TOGGLECHAT       KEY_ENTER

// Location of status bar
#define ST_X                0
#define ST_X2               104

#define ST_FX               143
#define ST_FY               169

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH     (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES     2
#define ST_NUMSPECIALFACES  3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES    2

// [JN] Additional faces:
// Atari Doom: +6 (exploded face)
//   PSX Doom: +1 (squished face)
#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES+7)

#define ST_TURNOFFSET       (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE+1)
// [JN] Atari Doom - exploded extra faces
#define ST_EXPLFACE0        (ST_DEADFACE+1)
#define ST_EXPLFACE1        (ST_EXPLFACE0+1)
#define ST_EXPLFACE2        (ST_EXPLFACE1+1)
#define ST_EXPLFACE3        (ST_EXPLFACE2+1)
#define ST_EXPLFACE4        (ST_EXPLFACE3+1)
#define ST_EXPLFACE5        (ST_EXPLFACE4+1)
// [JN] PSX Doom - squished extra face
#define ST_CRSHFACE0        (ST_EXPLFACE5+1)

#define ST_FACESX           143
#define ST_FACESY           168

#define ST_EVILGRINCOUNT    (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT        (1*TICRATE)
#define ST_OUCHCOUNT        (1*TICRATE)
#define ST_RAMPAGEDELAY     (2*TICRATE)

#define ST_MUCHPAIN         20


// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH        3	
#define ST_AMMOX            44
#define ST_AMMOY            171

// HEALTH number pos.
#define ST_HEALTHWIDTH      3	
#define ST_HEALTHX          90
#define ST_HEALTHY          171

// Weapon pos.
#define ST_ARMSX            111
#define ST_ARMSY            172
#define ST_ARMSBGX          104
#define ST_ARMSBGY          168
#define ST_ARMSXSPACE       12
#define ST_ARMSYSPACE       10

// Frags pos.
#define ST_FRAGSX           138
#define ST_FRAGSY           171	
#define ST_FRAGSWIDTH       2

// ARMOR number pos.
#define ST_ARMORWIDTH       3
#define ST_ARMORX           221
#define ST_ARMORY           171

// Key icon positions.
#define ST_KEY0WIDTH        8
#define ST_KEY0HEIGHT       5
#define ST_KEY0X            239
#define ST_KEY0Y            171
#define ST_KEY1WIDTH        ST_KEY0WIDTH
#define ST_KEY1X            239
#define ST_KEY1Y            181
#define ST_KEY2WIDTH        ST_KEY0WIDTH
#define ST_KEY2X            239
#define ST_KEY2Y            191

// Ammunition counter.
#define ST_AMMO0WIDTH       3
#define ST_AMMO0HEIGHT      6
#define ST_AMMO0X           288
#define ST_AMMO0Y           173
#define ST_AMMO1WIDTH       ST_AMMO0WIDTH
#define ST_AMMO1X           288
#define ST_AMMO1Y           179
#define ST_AMMO2WIDTH       ST_AMMO0WIDTH
#define ST_AMMO2X           288
#define ST_AMMO2Y           191
#define ST_AMMO3WIDTH       ST_AMMO0WIDTH
#define ST_AMMO3X           288
#define ST_AMMO3Y           185

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH    3
#define ST_MAXAMMO0HEIGHT   5
#define ST_MAXAMMO0X        314
#define ST_MAXAMMO0Y        173
#define ST_MAXAMMO1WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X        314
#define ST_MAXAMMO1Y        179
#define ST_MAXAMMO2WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X        314
#define ST_MAXAMMO2Y        191
#define ST_MAXAMMO3WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X        314
#define ST_MAXAMMO3Y        185

// pistol
#define ST_WEAPON0X         110 
#define ST_WEAPON0Y         172

// shotgun
#define ST_WEAPON1X         122 
#define ST_WEAPON1Y         172

// chain gun
#define ST_WEAPON2X         134 
#define ST_WEAPON2Y         172

// missile launcher
#define ST_WEAPON3X         110 
#define ST_WEAPON3Y         181

// plasma gun
#define ST_WEAPON4X         122 
#define ST_WEAPON4Y         181

 // bfg
#define ST_WEAPON5X         134
#define ST_WEAPON5Y         181


//Incoming messages window location
#define ST_MSGTEXTX         0
#define ST_MSGTEXTY         0
// Dimensions given in characters.
#define ST_MSGWIDTH         52
// Or shall I say, in lines?
#define ST_MSGHEIGHT        1


#define ST_MAPWIDTH	\
    (strlen(mapnames[(gameepisode-1)*9+(gamemap-1)]))

#define ST_MAPTITLEX \
    (SCREENWIDTH - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY        0
#define ST_MAPHEIGHT        1

int st_palette = 0;
int st_keyorskull[3];   // [crispy] blinking key or skull in the status bar

static int  lu_palette1, lu_palette2;   // [JN] lump numbers for both palettes
static int  veryfirsttime = 1;          // used to execute ST_Init() only once
static int  st_msgcounter = 0;          // used for making messages go away
// [Doom Retro] & [crispy] show SSG availability in the Shotgun slot of the arms widget
static int st_shotguns;
static int	st_fragscount;              // number of frags so far in deathmatch
static int	st_oldhealth = -1;          // used to use appopriately pained face
static int	st_facecount = 0;           // count until face changes
static int	st_faceindex = 0;           // current face index, used by w_faces
static int	keyboxes[3];                // holds key-type for each key box on bar
static int	st_randomnumber;            // a random number per tick

static unsigned int	st_clock;       // used for timing

static boolean  st_firsttime;       // ST_Start() has just been called
static boolean  st_statusbaron;     // whether left-side main status bar is active
static boolean  st_chat;            // whether status bar chat is active
static boolean  st_oldchat;         // value of st_chat before message popped up
static boolean  st_cursoron;        // whether chat window has the cursor on
static boolean  st_notdeathmatch;   // !deathmatch
static boolean  st_armson;          // !deathmatch && st_statusbaron
static boolean  st_fragson;         // !deathmatch
static boolean  st_neghealth;       // [JN] Negative player health
static boolean	st_stopped = true;
static boolean	oldweaponsowned[NUMWEAPONS];    // used for evil grin

static player_t *plyr;              // main player in game

static st_chatstateenum_t   st_chatstate;   // used when in chat 
static st_stateenum_t       st_gamestate;   // whether in automap or first-person

static patch_t  *sbar;                  // main bar 
static patch_t  *sbar_rus;              // [JN] Main bar (Russian)
static patch_t  *tallnum[10];           // 0-9, tall numbers
static patch_t  *tallpercent;           // tall % sign
static patch_t  *shortnum[10];          // 0-9, short, yellow (,different!) numbers
// jff 2/24/98 extend number of patches by three skull/card combos
static patch_t  *keys[NUMCARDS+3];      // 3 key-cards, 3 skulls
// [JN] Doubled array for GOD mode faces. Thanks Brad Harding for help!
static patch_t  *faces[ST_NUMFACES * 2];// face status patches
static patch_t  *faceback;              // face background
static patch_t  *armsbg;                // main bar right
static patch_t  *armsbg_rus;            // [JN] Main bar right (Russian)
static patch_t  *arms[6][2];            // weapon ownership patches

static st_number_t  w_ready;    // ready-weapon widget
static st_number_t  w_frags;    // in deathmatch only, summary of frags stats

static st_percent_t w_health;   // health widget
static st_percent_t w_health_neg; // [JN] Negative player health

static st_binicon_t     w_armsbg;       // arms background
static st_multicon_t    w_arms[6];      // weapon ownership widgets
static st_multicon_t    w_faces;        // face status widget
static st_multicon_t    w_keyboxes[3];  // keycard widgets
static st_percent_t     w_armor;        // armor widget
static st_number_t      w_ammo[4];      // ammo widgets
static st_number_t      w_maxammo[4];   // max ammo widgets

extern char *mapnames[];


enum
{
    hudcolor_ammo,
    hudcolor_health,
    hudcolor_frags,
    hudcolor_armor
} hudcolor_t;

// Massive bunches of cheat shit
//  to keep it from being easy to figure them out.
// Yeah, right...
unsigned char   cheat_mus_seq[] =
{
    0xb2, 0x26, 0xb6, 0xae, 0xea, 1, 0, 0, 0xff
};

unsigned char   cheat_choppers_seq[] =
{
    0xb2, 0x26, 0xe2, 0x32, 0xf6, 0x2a, 0x2a, 0xa6, 0x6a, 0xea, 0xff // id...
};

unsigned char   cheat_god_seq[] =
{
    0xb2, 0x26, 0x26, 0xaa, 0x26, 0xff  // iddqd
};

unsigned char   cheat_ammo_seq[] =
{
    0xb2, 0x26, 0xf2, 0x66, 0xa2, 0xff	// idkfa
};

unsigned char   cheat_ammonokey_seq[] =
{
    0xb2, 0x26, 0x66, 0xa2, 0xff	// idfa
};

// Smashing Pumpkins Into Samml Piles Of Putried Debris. 
unsigned char   cheat_noclip_seq[] =
{
    0xb2, 0x26, 0xea, 0x2a, 0xb2,	// idspispopd
    0xea, 0x2a, 0xf6, 0x2a, 0x26, 0xff
};

unsigned char   cheat_commercial_noclip_seq[] =
{
    0xb2, 0x26, 0xe2, 0x36, 0xb2, 0x2a, 0xff	// idclip
}; 

unsigned char   cheat_powerup_seq[7][10] =
{
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6e, 0xff }, // beholdv
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xea, 0xff }, // beholds
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xb2, 0xff }, // beholdi
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6a, 0xff }, // beholdr
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xa2, 0xff }, // beholda
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x36, 0xff }, // beholdl
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xff }        // behold
};

unsigned char   cheat_clev_seq[] =
{
    0xb2, 0x26,  0xe2, 0x36, 0xa6, 0x6e, 1, 0, 0, 0xff  // idclev
};

// my position cheat
unsigned char   cheat_mypos_seq[] =
{
    0xb2, 0x26, 0xb6, 0xba, 0x2a, 0xf6, 0xea, 0xff  // idmypos
}; 

// [JN] idka
unsigned char   cheat_keys_seq[] =
{
    SCRAMBLE('i'),
    SCRAMBLE('d'),
    SCRAMBLE('k'),
    SCRAMBLE('a'),
    0xff
}; 

// [JN] version cheat
unsigned char	cheat_version_seq[] =
{
    SCRAMBLE('v'),
    SCRAMBLE('e'),
    SCRAMBLE('r'),
    SCRAMBLE('s'),
    SCRAMBLE('i'),
    SCRAMBLE('o'),
    SCRAMBLE('n'),
    0xff
}; 

// [JN] TNTEM (from Crispy Doom)
unsigned char   cheat_massacre_seq[] =
{
    SCRAMBLE('t'),
    SCRAMBLE('n'),
    SCRAMBLE('t'),
    SCRAMBLE('e'),
    SCRAMBLE('m'),
    0xff
}; 


// Now what?
cheatseq_t  cheat_mus = { cheat_mus_seq, 0 };
cheatseq_t  cheat_god = { cheat_god_seq, 0 };
cheatseq_t  cheat_ammo = { cheat_ammo_seq, 0 };
cheatseq_t  cheat_ammonokey = { cheat_ammonokey_seq, 0 };
cheatseq_t  cheat_noclip = { cheat_noclip_seq, 0 };
cheatseq_t  cheat_commercial_noclip = { cheat_commercial_noclip_seq, 0 };

cheatseq_t	cheat_powerup[7] =
{
    { cheat_powerup_seq[0], 0 },
    { cheat_powerup_seq[1], 0 },
    { cheat_powerup_seq[2], 0 },
    { cheat_powerup_seq[3], 0 },
    { cheat_powerup_seq[4], 0 },
    { cheat_powerup_seq[5], 0 },
    { cheat_powerup_seq[6], 0 }
};

cheatseq_t  cheat_choppers = { cheat_choppers_seq, 0 };
cheatseq_t  cheat_clev = { cheat_clev_seq, 0 };
cheatseq_t  cheat_mypos = { cheat_mypos_seq, 0 };
cheatseq_t  cheat_keys = { cheat_keys_seq, 0 };
cheatseq_t  cheat_version = { cheat_version_seq, 0 };
cheatseq_t  cheat_massacre = { cheat_massacre_seq, 0 };


//
// STATUS BAR CODE
//
void ST_Stop(void);

void ST_refreshBackground(void)
{
    if (st_statusbaron && screenblocks < 11 || automapactive)
    {
        V_DrawPatch(ST_X, 0, BG, english_language ?
                                 sbar : sbar_rus);

        // [crispy] back up arms widget background
        if (!deathmatch)
        {
            V_DrawPatch(ST_ARMSBGX, 0, BG, english_language ?
                                           armsbg : armsbg_rus);
        }

        if (netgame)
        {
            V_DrawPatch(ST_FX, 0, BG, faceback);
        }

        V_CopyRect(ST_X, 0, BG, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y, FG);
    }
}


// [crispy] adapted from boom202s/M_CHEAT.C:467-498
static int ST_cheat_massacre()
{
    int killcount = 0;
    thinker_t *th;
    extern int numbraintargets;
    extern void A_PainDie(mobj_t *);

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *)th;

            if (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)
            {
                if (mo->health > 0)
                {
                    P_DamageMobj(mo, NULL, NULL, 10000);
                    killcount++;
                }
                if (mo->type == MT_PAIN)
                {
                    A_PainDie(mo);
                    P_SetMobjState(mo, S_PAIN_DIE6);
                }
            }
        }
    }

    // [crispy] disable brain spitters
    numbraintargets = -1;

    return killcount;
}


// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder (event_t *ev)
{
    int i;

    // Filter automap on/off.
    if (ev->type == ev_keyup && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
    {
        switch(ev->data1)
        {
            case AM_MSGENTERED:
            st_gamestate = AutomapState;
            st_firsttime = true;
            break;

            case AM_MSGEXITED:
            st_gamestate = FirstPersonState;
            break;
        }
    }

    // if a user keypress...
    else if (ev->type == ev_keydown)
    {
        if (!netgame && (gameskill != sk_nightmare /* && gameskill != sk_ultranm */))
        {
            // 'dqd' cheat for toggleable god mode
            if (cht_CheckCheat(&cheat_god, ev->data1))
            {
                // [JN] IDDQD does not affect on dead player
                if (!vanilla && plyr->playerstate == PST_DEAD)
                {
                    return false;
                }

                plyr->cheats ^= CF_GODMODE;

                if (plyr->cheats & CF_GODMODE)
                {
                    if (plyr->mo)
                    {
                        plyr->mo->health = 100;
                    }
                    plyr->health = 100;
                    plyr->message_system = ststr_dqdon;
                }
                else
                {
                    plyr->message_system = ststr_dqdoff;
                }
            }

            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheat(&cheat_ammonokey, ev->data1))
            {
                plyr->armorpoints = 200;
                plyr->armortype = 2;

                // [JN] Give all weapons properly.
                // Don't give SSG, Plasmagun and BFG in Shareware, leave their
                // indication on HUD as gray numbers. Give SSG only in Doom 2.
                plyr->weaponowned[wp_fist] = true;
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->weaponowned[wp_pistol] = true;
                plyr->weaponowned[wp_shotgun] = true;
                plyr->weaponowned[wp_supershotgun] = commercial ? true : false;
                plyr->weaponowned[wp_chaingun] = true;
                plyr->weaponowned[wp_missile] = true;
                plyr->weaponowned[wp_plasma] = shareware ? false : true;
                plyr->weaponowned[wp_bfg] = shareware ? false : true;

                // [JN] Checking: does player have a backpack?
                if (!plyr->backpack && !vanilla)
                {
                    for (i=0 ; i<NUMAMMO ; i++)
                    plyr->maxammo[i] *= 2;
                    plyr->backpack = true;
                }

                for (i=0 ; i < NUMAMMO ; i++)
                {
                    plyr->ammo[i] = plyr->maxammo[i];
                }

                plyr->message_system = ststr_faadded;
            }

            // 'kfa' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo, ev->data1))
            {
                plyr->armorpoints = 200;
                plyr->armortype = 2;

                // [JN] Give all weapons properly.
                // Don't give SSG, Plasmagun and BFG in Shareware, leave their
                // indication on HUD as gray numbers. Give SSG only in Doom 2.
                plyr->weaponowned[wp_fist] = true;
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->weaponowned[wp_pistol] = true;
                plyr->weaponowned[wp_shotgun] = true;
                plyr->weaponowned[wp_supershotgun] = commercial ? true : false;
                plyr->weaponowned[wp_chaingun] = true;
                plyr->weaponowned[wp_missile] = true;
                plyr->weaponowned[wp_plasma] = shareware ? false : true;
                plyr->weaponowned[wp_bfg] = shareware ? false : true;

                // [JN] Checking: does player have a backpack?
                if (!plyr->backpack && !vanilla)
                {
                    for (i=0 ; i<NUMAMMO ; i++)
                    plyr->maxammo[i] *= 2;
                    plyr->backpack = true;
                }

                for (i=0 ; i < NUMAMMO ; i++)
                {
                    plyr->ammo[i] = plyr->maxammo[i];
                }

                for (i=0 ; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                plyr->message_system = ststr_kfaadded;
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus, ev->data1))
            {
                int     musnum;
                char    buf[3];

                plyr->message_system = ststr_mus;
                cht_GetParam(&cheat_mus, buf);

                if (commercial)
                {
                    musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;

                    // [crispy] prevent crash with IDMUS00
                    if (((buf[0]-'0')*10 + buf[1]-'0') > 35 || musnum < mus_runnin)
                    {
                        plyr->message_system = ststr_nomus;
                    }
                    else
                    {
                        S_ChangeMusic(musnum, 1);
                    }
                }
                else
                {
                    musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');

                    // [crispy] prevent crash with IDMUS0x or IDMUSx0
                    if (((buf[0]-'1')*9 + buf[1]-'1') > 31
                    || buf[0] < '1' || buf[1] < '1')
                    {
                        plyr->message_system = ststr_nomus;
                    }
                    else
                    {
                        S_ChangeMusic(musnum, 1);
                    }
                }
            }

            // [JN] Both cheat codes IDCLIP / IDSPISPOPD works in all games
            else if(cht_CheckCheat(&cheat_noclip, ev->data1)
            ||      cht_CheckCheat(&cheat_commercial_noclip, ev->data1))
            {	
                plyr->cheats ^= CF_NOCLIP;

                plyr->message_system = plyr->cheats & CF_NOCLIP ?
                                       ststr_ncon : ststr_ncoff;
            }

            // 'behold?' power-up cheats
            for (i=0 ; i < 6 ; i++)
            {
                if (cht_CheckCheat(&cheat_powerup[i], ev->data1))
                {
                    if (!plyr->powers[i])
                    {
                        P_GivePower( plyr, i);
                        plyr->message_system = ststr_beholdx; // [JN] Deactivated
                    }
                    else if (i!=pw_strength)
                    {
                        plyr->powers[i] = 1;
                        plyr->message_system = ststr_beholdz; // [JN] Activated
                    }
                    else
                    {
                        plyr->powers[i] = 0;
                        plyr->message_system = ststr_beholdz; // [JN] Activated
                    }
                }
            }

            // 'behold' power-up menu
            if (cht_CheckCheat(&cheat_powerup[6], ev->data1))
            {
                plyr->message_system = ststr_behold;
            }

            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data1))
            {
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->powers[pw_invulnerability] = true;
                plyr->message_system = ststr_choppers;
            }

            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data1))
            {
                static char	buf[ST_MSGWIDTH];

                sprintf(buf, english_language ?
                             "x=%d, y=%d, ang=%d" :
                             "[=%d< e=%d< eujk=%d",
                             players[consoleplayer].mo->x >> FRACBITS,
                             players[consoleplayer].mo->y >> FRACBITS,
                             players[consoleplayer].mo->angle / ANG1);
                plyr->message_system = buf;
            }

            // [JN] 'ka' for keys
            else if (cht_CheckCheat(&cheat_keys, ev->data1))
            {
                for (i=0 ; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                plyr->message_system = ststr_kaadded;
            }

            // [crispy] implement Boom's "tntem" cheat
            else if (cht_CheckCheat(&cheat_massacre, ev->data1))
            {
                int killcount = ST_cheat_massacre();
                static char msg[ST_MSGWIDTH];

                snprintf(msg, sizeof(msg), "%s %d", ststr_massacre, killcount);
                plyr->message_system = msg;
            }

            // [JN] version cheat
            else if (cht_CheckCheat(&cheat_version, ev->data1))
            {
                plyr->message_system = ststr_version;
            }
        }

        // 'clev' change-level cheat
        if (cht_CheckCheat(&cheat_clev, ev->data1))
        {
            int     epsd;
            int     map;
            char    buf[3];

            cht_GetParam(&cheat_clev, buf);

            if (commercial)
            {
                epsd = 1;

                if (plutonia || tnt)
                {
                    epsd = 0;
                }

                map = (buf[0] - '0')*10 + buf[1] - '0';
            }
            else
            {
                epsd = buf[0] - '0';
                map = buf[1] - '0';
            }

            // Catch invalid maps.
            if ((!commercial && retail && epsd > 0 && epsd < 5 && map > 0 && map < 10)
            ||  (!commercial && retail && sigil && epsd > 0 && epsd < 6 && map > 0 && map < 10) // [JN] Sigil
            ||  (!commercial && epsd > 0 && epsd < 4 && map > 0 && map < 10)
            ||   (commercial && map > 0 && map <= 40))
            {
                // So be it.
                plyr->message_system = ststr_clev;
                G_DeferedInitNew(gameskill, epsd, map);
            }
        }
    }

    return false;
}


int ST_calcPainOffset (void)
{
    int         health;
    static int  lastcalc;
    static int  oldhealth = -1;

    health = plyr->health > 100 ? 100 : plyr->health;

    if (health != oldhealth)
    {
        lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
        oldhealth = health;
    }

    return lastcalc;
}


//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
void ST_updateFaceWidget (void)
{
    int		    i;
    static int  lastattackdown = -1;
    static int  priority = 0;
    boolean	    doevilgrin;
    angle_t     badguyangle;
    angle_t     diffang;
    // [crispy] fix status bar face hysteresis
    int         painoffset;
    static int  faceindex;

    painoffset = ST_calcPainOffset();

    if (priority < 10)
    {
        // dead
        // [crispy] negative player health
        if (plyr->health <= 0)
        {
            priority = 9;
            painoffset = 0;
            faceindex = ST_DEADFACE;
            st_facecount = 1;
        }

        // [JN] Jaguar and PSX Doom - extra faces of exploded and squished player.
        // Proper checking for xdeath state has been taken from Crispy Doom,
        // thanks to Fabian Greffrath!
        if (extra_player_faces && !vanilla && plyr->health <= 0
        &&  plyr->mo->state - states >= mobjinfo[plyr->mo->type].xdeathstate)
        {
            priority = 9;
            painoffset = 0;

            // [JN] Sync with actual player state
            if (plyr->mo->state == &states[S_PLAY_XDIE1])
                faceindex = ST_EXPLFACE0;
            if (plyr->mo->state == &states[S_PLAY_XDIE2])
                faceindex = ST_EXPLFACE1;
            if (plyr->mo->state == &states[S_PLAY_XDIE3])
                faceindex = ST_EXPLFACE2;
            if (plyr->mo->state == &states[S_PLAY_XDIE4])
                faceindex = ST_EXPLFACE3;
            if (plyr->mo->state == &states[S_PLAY_XDIE5])
                faceindex = ST_EXPLFACE4;
            if ((plyr->mo->state == &states[S_PLAY_XDIE6])
            ||  (plyr->mo->state == &states[S_PLAY_XDIE7])
            ||  (plyr->mo->state == &states[S_PLAY_XDIE8])
            ||  (plyr->mo->state == &states[S_PLAY_XDIE9]))
                faceindex = ST_EXPLFACE5;

            if (plyr->mo->state == &states[S_GIBS])
                faceindex = ST_CRSHFACE0;
        }
    }

    if (priority < 9)
    {
        if (plyr->bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0 ; i < NUMWEAPONS ; i++)
            {
                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                    // [BH] no evil grin when invulnerable
                    // [JN] extra god faces have grin, use them in god mode
                    if (extra_player_faces && !vanilla)
                    {
                        doevilgrin = true;
                        oldweaponsowned[i] = plyr->weaponowned[i];
                    }
                }
            }

            if (doevilgrin) 
            {
                // evil grin if just picked up weapon
                priority = 8;
                st_facecount = ST_EVILGRINCOUNT;
                faceindex = ST_EVILGRINOFFSET;
            }
        }
    }

    if (priority < 8) 
    { 	    
        // being attacked
        if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
        {
            // [JN] Correct "Ouch face" formula (part 1)
            // Thanks Brad Harding!
            // Taken from Doom Retro (src/st_stuff.c)
            if (!vanilla)
            {
                // [BH] fix ouch-face when damage > 20
                if (st_oldhealth - plyr->health > ST_MUCHPAIN)
                {
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                    priority = 8;   // [BH] keep ouch-face visible
                }
                else
                {
                    angle_t badguyangle = R_PointToAngle2(plyr->mo->x, plyr->mo->y,
                                                          plyr->attacker->x,plyr->attacker->y);
                    angle_t diffang;

                    if (badguyangle > plyr->mo->angle)
                    {
                        // whether right or left
                        diffang = badguyangle - plyr->mo->angle;
                        i = (diffang > ANG180);
                    }
                    else
                    {
                        // whether left or right
                        diffang = plyr->mo->angle - badguyangle;
                        i = (diffang <= ANG180);
                    }   // confusing, ain't it?

                    st_facecount = ST_TURNCOUNT;

                    if (diffang < ANG45)
                    {
                        // head-on
                        faceindex = ST_RAMPAGEOFFSET;
                    }
                    else if (i)
                    {
                        // turn face right
                        faceindex = ST_TURNOFFSET;
                    }
                    else
                    {
                        // turn face left
                        faceindex = ST_TURNOFFSET+1;
                    }
                }
            }
            // [JN] Standard formula
            else
            {
                if (plyr->health - st_oldhealth > ST_MUCHPAIN)
                {
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                    priority = 7;
                }
                else
                {
                    badguyangle = R_PointToAngle2(plyr->mo->x,
                    plyr->mo->y,
                    plyr->attacker->x,
                    plyr->attacker->y);

                    if (badguyangle > plyr->mo->angle)
                    {
                        // whether right or left
                        diffang = badguyangle - plyr->mo->angle;
                        i = diffang > ANG180; 
                    }
                    else
                    {
                        // whether left or right
                        diffang = plyr->mo->angle - badguyangle;
                        i = diffang <= ANG180; 
                    }   // confusing, aint it?

                    st_facecount = ST_TURNCOUNT;

                    if (diffang < ANG45)
                    {
                        // head-on    
                        faceindex = ST_RAMPAGEOFFSET;
                    }
                    else if (i)
                    {
                        // turn face right
                        faceindex = ST_TURNOFFSET;
                    }
                    else
                    {
                        // turn face left
                        faceindex = ST_TURNOFFSET+1;
                    }
                }
            }
        }
    }

    // [JN] Correct "Ouch face" formula (part 2)
    // Thanks Brad Harding!
    // Taken from Doom Retro (src/st_stuff.c)
    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (plyr->damagecount)
        {
            if (extra_player_faces && !vanilla)
            {
                if (st_oldhealth - plyr->health > ST_MUCHPAIN)
                {
                    priority = 7;
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                }
                else
                {
                    priority = 6;
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_RAMPAGEOFFSET;
                }
            }
            else
            {
                if (plyr->health - st_oldhealth > ST_MUCHPAIN)
                {
                    priority = 7;
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                }
                else
                {
                    priority = 6;
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_RAMPAGEOFFSET;
                }
            }
        }
    }

    if (priority < 6)
    {
        // rapid firing
        if (plyr->attackdown)
        {
            // [BH] no rampage face when invulnerable
            // [JN] extra god faces have rampage, use them in god mode
            if (extra_player_faces && !vanilla)
            {
                if (lastattackdown==-1)
                {
                    lastattackdown = ST_RAMPAGEDELAY;
                }
                else if (!--lastattackdown)
                {
                    priority = 5;
                    faceindex = ST_RAMPAGEOFFSET;
                    st_facecount = 1;
                    lastattackdown = 1;
                }
            }
        }
        else
        {
            lastattackdown = -1;
        }
    }

    if (priority < 5)
    {
        // invulnerability
        if ((plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability])
        {
            priority = 4;
            faceindex = ST_GODFACE;

            if (!extra_player_faces || vanilla)
            {
                // [JN] Standard god mode face behaviour
                st_facecount = 1;
                painoffset = 0;
            }
            else
            {
                // [JN] Activate extra bloody god mode faces
                st_facecount = 0;
            }
        }
    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
        faceindex = st_randomnumber % 3;
        st_facecount = ST_STRAIGHTFACECOUNT;
        priority = 0;
    }

    st_facecount--;

    // [crispy] fix status bar face hysteresis
    st_faceindex = painoffset + faceindex;

    // [JN] Additional array of faces for GOD mode
    if (!vanilla && (plyr->powers[pw_invulnerability]) || (plyr->cheats & CF_GODMODE))
    {
        st_faceindex = painoffset + faceindex + ST_NUMFACES;
    }
}


void ST_updateWidgets (void)
{
    int		    i;
    static int  largeammo = 1994; // means "n/a"

    // must redirect the pointer if the ready weapon has changed.
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
    {
        w_ready.num = &largeammo;
    }
    else
    {
        w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
    }

    w_ready.data = plyr->readyweapon;

    // update keycard multiple widgets
    for (i=0 ; i < 3 ; i++)
    {
        keyboxes[i] = plyr->cards[i] ? i : -1;

        //jff 2/24/98 select double key

        if (plyr->cards[i+3])
        {
            keyboxes[i] = (keyboxes[i]==-1) ? i+3 : i+6;
        }

        // [crispy] blinking key or skull in the status bar
        // [JN] blink in any HUD size, except full screen (no HUD) and vanilla
        if (plyr->tryopen[i] && !vanilla)
        {
            if (!(plyr->tryopen[i] & (2*KEYBLINKMASK-1)))
            {
                S_StartSound(NULL, sfx_itemup);
            }

            if (screenblocks < 14 && !(plyr->tryopen[i] & (KEYBLINKMASK-1)))
            {
                st_firsttime = true;
            }

            keyboxes[i] = (--plyr->tryopen[i] & KEYBLINKMASK) ? i + st_keyorskull[i] : -1;

            if (!plyr->tryopen[i])
            {
                w_keyboxes[i].oldinum = -1;
            }
        }
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    // used by the w_armsbg widget
    st_notdeathmatch = !deathmatch;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch; 

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 
    st_fragscount = 0;

    for (i=0 ; i < MAXPLAYERS ; i++)
    {
        if (i != consoleplayer)
        {
            st_fragscount += plyr->frags[i];
        }
        else
        {
            st_fragscount -= plyr->frags[i];
        }
    }

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
    {
        st_chat = st_oldchat;
    }
}


void ST_Ticker (void)
{
    st_clock++;
    st_randomnumber = M_Random();
    ST_updateWidgets();
    st_oldhealth = plyr->health;
}


void ST_doPaletteStuff (void)
{
    int     palette;
    int     cnt;
    int     bzc;
    byte   *pal;

    cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
        // slowly fade the berzerk out
        bzc = 12 - (plyr->powers[pw_strength]>>6);

        if (bzc > cnt)
        {
            cnt = bzc;
        }
    }

    if (cnt)
    {
        palette = (cnt+7)>>3;

        if (palette >= NUMREDPALS)
        {
            palette = NUMREDPALS-1;
        }

        palette += STARTREDPALS;
    }
    else if (plyr->bonuscount)
    {
        palette = (plyr->bonuscount+7)>>3;

        // [JN] Added one extra palette for more smoothing fade in/out
        // https://doomwiki.org/wiki/PLAYPAL
        if (palette >= NUMBONUSPALS)
        {
            palette = vanilla ? NUMBONUSPALS-1 : NUMBONUSPALS;
        }

        palette += vanilla ? STARTBONUSPALS : STARTBONUSPALS-1;
    }
    else if (plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet]&8)
    {
        palette = RADIATIONPAL;
    }
    else
    {
        palette = 0;
    }

    if (palette != st_palette)
    {
        st_palette = palette;
        pal = (byte *) W_CacheLumpNum (usegamma <= 8 ?
                                       lu_palette1 : lu_palette2,
                                       PU_CACHE)+palette*768;
        I_SetPalette (pal);
    }
}


// [crispy] return ammo/health/armor widget color
static byte *ST_WidgetColor (int i)
{
    if (!colored_hud || vanilla)
    {
        return NULL;
    }

    switch (i)
    {
        case hudcolor_ammo:
        {
            if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
            {
                return NULL;
            }
            else
            {
                int ammo =  plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
                int fullammo = maxammo[weaponinfo[plyr->readyweapon].ammo];

                if (ammo < fullammo/4)
                {
                    return cr[CR_RED];
                }
                else if (ammo < fullammo/2)
                {
                    return cr[CR_GOLD];
                }
                else if (ammo <= fullammo)
                {
                    return cr[CR_GREEN];
                }
                else
                {
                    return cr[CR_BLUE2];
                }
            }
            break;
        }
        case hudcolor_health:
        {
            int health = plyr->health;

            // [crispy] Invulnerability powerup and God Mode cheat turn Health values gray
            // [JN] I'm using different health values, represented by crosshair,
            // and thus a little bit different logic.
            if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
            {
                return cr[CR_GRAY];
            }
            else if (health > 100)
            {
                return cr[CR_BLUE2];
            }
            else if (health >= 67)
            {
                return cr[CR_GREEN];
            }
            else if (health >= 34)
            {
                return cr[CR_GOLD];
            }
            else
            {
                return cr[CR_RED];
            }
            break;
        }
        case hudcolor_frags:
        {
            int frags = st_fragscount;

            if (frags < 0)
            {
                return cr[CR_RED];
            }
            else if (frags == 0)
            {
                return cr[CR_GOLD];
            }
            else
            {
                return cr[CR_GREEN];
            }
            break;
        }
        case hudcolor_armor:
        {
            // [crispy] Invulnerability powerup and God Mode cheat turn Armor values gray
            if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
            {
                return cr[CR_GRAY];
            }
            // [crispy] color by armor type
            else if (plyr->armortype >= 2)
            {
                return cr[CR_BLUE2];
            }
            else if (plyr->armortype == 1)
            {
                return cr[CR_GREEN];
            }
            else if (plyr->armortype == 0)
            {
                return cr[CR_RED];
            }
            break;
        }
    }

    return NULL;
}


void ST_drawWidgets (boolean refresh)
{
    int i;

    // [JN] No HUD, don't draw anything.
    if (screenblocks == 14 && !automapactive)
    return;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch;

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 

    // [JN] Negative player health
    st_neghealth = negative_health && plyr->health <= 0 && !vanilla;

    dp_translation = ST_WidgetColor(hudcolor_ammo);
    STlib_updateNum(&w_ready, refresh);
    dp_translation = NULL;

    // [crispy] draw "special widgets" in the Crispy HUD
    if ((screenblocks > 10 && screenblocks < 14) && !automapactive)
    {
        // [crispy] draw berserk pack instead of no ammo if appropriate
        if (plyr->readyweapon == wp_fist && plyr->powers[pw_strength])
        {
            static int  lump = -1;
            patch_t    *patch;

            if (lump == -1)
            {
                lump = W_GetNumForName("PSTRA0");

                if (lump == -1)
                {
                    lump = W_GetNumForName("MEDIA0");
                }
            }

            patch = W_CacheLumpNum(lump, PU_CACHE);

            // [crispy] (23,179) is the center of the Ammo widget
            V_DrawPatchDirect((23 - SHORT(patch->width)/2 + SHORT(patch->leftoffset)),
                              179 - SHORT(patch->height)/2 + SHORT(patch->topoffset), 0,
                              patch);
        }
    }

    for (i=0 ; i < 4 ; i++)
    {
        STlib_updateNum(&w_ammo[i], refresh);
        STlib_updateNum(&w_maxammo[i], refresh);
    }

    // [JN] Signed Crispy HUD: no STBAR backbround, with player's face/background
    if (screenblocks == 11 && !automapactive)
    {
        if (netgame)
        {
            // [JN] Account player's color in network game
            V_DrawPatch(ST_FX, 0, BG, faceback);
        }
        else
        {
            // [JN] Use only gray color in single player
            V_DrawPatchDirect(ST_FX, ST_FY, 0, W_CacheLumpName("STFB1", PU_CACHE));
        }
    }

    // [JN] Signed Crispy HUD: no STBAR backbround, without player's face/background
    if (screenblocks == 11 || screenblocks == 12)
    {
        // [JN] Don't draw these patches again in standard HUD while activated automap
        if (!automapactive)
        {
            // [JN] Don't draw ammo for fist and chainsaw
            if (plyr->readyweapon == wp_pistol
            ||  plyr->readyweapon == wp_shotgun
            ||  plyr->readyweapon == wp_supershotgun
            ||  plyr->readyweapon == wp_chaingun
            ||  plyr->readyweapon == wp_missile
            ||  plyr->readyweapon == wp_plasma
            ||  plyr->readyweapon == wp_bfg)
            {
                V_DrawPatchDirect(2, 191, 0, W_CacheLumpName(english_language ?
                                                             "STCHAMMO" :
                                                             "RDCHAMMO", PU_CACHE));
            }

            if (deathmatch)
            {
                // [JN] Frags
                V_DrawPatchDirect(108, 191, 0, W_CacheLumpName(english_language ?
                                                               "STCHFRGS" :
                                                               "RDCHFRGS", PU_CACHE));
            }
            else
            {
                // [JN] Arms
                V_DrawPatchDirect(108, 191, 0, W_CacheLumpName(english_language ?
                                                               "STCHARMS" :
                                                               "RDCHARMS", PU_CACHE));
            }

            // [JN] Health, armor, list of ammo
            V_DrawPatchDirect(52, 173, 0, W_CacheLumpName(english_language ?
                                                          "STCHNAMS" :
                                                          "RDCHNAMS", PU_CACHE));
        }

        // [JN] For prevention of yellow slashes "blinking",
        // they must be drawn above standard HUD while opened automap.
        if (automapactive)
        {
            V_DrawPatch(292, 173, 0, W_CacheLumpName("STYSSLSH", PU_CACHE));
        }
        else
        {
            V_DrawPatchDirect(292, 173, 0, W_CacheLumpName("STYSSLSH", PU_CACHE));
        }
    }

    // [JN] Traditional Crispy HUD
    if (screenblocks == 13)
    {
        // [JN] Only yellow slashes in Traditional HUD. The logics same as above.
        if (automapactive)
        {
            V_DrawPatch(292, 173, 0, W_CacheLumpName("STYSSLSH", PU_CACHE));
        }
        else
        {
            V_DrawPatchDirect(292, 173, 0, W_CacheLumpName("STYSSLSH", PU_CACHE));
        }
    }

    dp_translation = ST_WidgetColor(hudcolor_health);
    STlib_updatePercent(st_neghealth ? &w_health_neg : &w_health,
                        refresh || (screenblocks > 10 && screenblocks < 14));

    dp_translation = ST_WidgetColor(hudcolor_armor);
    STlib_updatePercent(&w_armor, refresh || (screenblocks > 10 && screenblocks < 14));
    dp_translation = NULL;

    if (screenblocks < 11 || automapactive)
    {
        STlib_updateBinIcon(&w_armsbg, refresh);
    }

    // [Doom Retro] & [crispy] show SSG availability in the Shotgun slot of the arms widget
    st_shotguns = plyr->weaponowned[wp_shotgun] | plyr->weaponowned[wp_supershotgun];

    for (i=0 ; i < 6 ; i++)
    {
        STlib_updateMultIcon(&w_arms[i], refresh || (screenblocks > 10 && screenblocks < 14));
    }

    if (screenblocks < 12 || automapactive)
    {
        STlib_updateMultIcon(&w_faces, refresh || screenblocks == 11);
    }

    for (i=0 ; i < 3 ; i++)
    {
        STlib_updateMultIcon(&w_keyboxes[i], refresh || (screenblocks > 10 && screenblocks < 14));
    }

    STlib_updateNum(&w_frags, refresh || (screenblocks > 10 && screenblocks < 14));
}


void ST_doRefresh (void)
{

    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);
}


void ST_diffDraw (void)
{
    // update all widgets
    ST_drawWidgets(false);
}


void ST_Drawer (boolean fullscreen, boolean refresh)
{
    st_statusbaron = (!fullscreen) || automapactive || screenblocks == 11 || screenblocks == 12;
    st_firsttime = st_firsttime || refresh;

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // [JN] Don't draw status bar while in help screens,
    // but let palette be changed properly.
    if (inhelpscreens)
    {
        return;
    }

    // If just after ST_Start(), refresh all
    if (st_firsttime)
    {
        ST_doRefresh();
    }
    // Otherwise, update as little as possible
    else
    {
        ST_diffDraw();
    }
}


void ST_loadGraphics (void)
{
    int     i;
    int     j;
    int     facenum;
    char    namebuf[9];

    // Load the numbers, tall and short
    for (i=0 ; i < 10 ; i++)
    {
        sprintf(namebuf, "STTNUM%d", i);
        tallnum[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);

        sprintf(namebuf, "STYSNUM%d", i);
        shortnum[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?
    tallpercent = (patch_t *) W_CacheLumpName("STTPRCNT", PU_STATIC);

    // key cards
    for (i=0;i<NUMCARDS+3;i++)  //jff 2/23/98 show both keys too
    {
        sprintf(namebuf, "STKEYS%d", i);
        keys[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
    }

    // arms background
    armsbg = (patch_t *) W_CacheLumpName("STARMS", PU_STATIC);
    armsbg_rus = (patch_t *) W_CacheLumpName("RDARMS", PU_STATIC);

    // arms ownership widgets
    for (i=0 ; i < 6 ; i++)
    {
        sprintf(namebuf, "STGNUM%d", i+2);

        // gray #
        arms[i][0] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);

        // yellow #
        arms[i][1] = shortnum[i+2]; 
    }

    // face backgrounds for different color players
    sprintf(namebuf, "STFB%d", consoleplayer);
    faceback = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);

    // status bar background bits
    sbar = (patch_t *) W_CacheLumpName("STBAR", PU_STATIC);
    sbar_rus = (patch_t *) W_CacheLumpName("RDSTBAR", PU_STATIC);

    // face states
    facenum = 0;
    for (i=0 ; i < ST_NUMPAINFACES ; i++)
    {
        for (j=0 ; j < ST_NUMSTRAIGHTFACES ; j++)
        {
            sprintf(namebuf, "STFST%d%d", i, j);
            faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        }

        sprintf(namebuf, "STFTR%d0", i);	// turn right
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "STFTL%d0", i);	// turn left
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "STFOUCH%d", i);	// ouch!
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "STFEVL%d", i);	// evil grin ;)
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "STFKILL%d", i);	// pissed off
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
    }

    faces[facenum++] = W_CacheLumpName("STFGOD0", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFDEAD0", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFEXPL0", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFEXPL1", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFEXPL2", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFEXPL3", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFEXPL4", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFEXPL5", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFCRSH0", PU_STATIC);

    // [JN] Doubled array of faced, needed for GOD mode
    for (i=0 ; i < ST_NUMPAINFACES ; i++)
    {
        for (j=0 ; j < ST_NUMSTRAIGHTFACES ; j++)
        {
            sprintf(namebuf, "GTFST%d%d", i, j);
            faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        }

        sprintf(namebuf, "GTFTR%d0", i);	// turn right
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "GTFTL%d0", i);	// turn left
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "GTFOUCH%d", i);	// ouch!
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "GTFEVL%d", i);	// evil grin ;)
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
        sprintf(namebuf, "GTFKILL%d", i);	// pissed off
        faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
    }

    faces[facenum++] = W_CacheLumpName("GTFGOD0", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFDEAD0", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFEXPL0", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFEXPL1", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFEXPL2", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFEXPL3", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFEXPL4", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFEXPL5", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("GTFCRSH0", PU_STATIC);
}


void ST_loadData (void)
{
    lu_palette1 = W_GetNumForName ("PALFIX");
    lu_palette2 = W_GetNumForName ("PLAYPAL");
    ST_loadGraphics();
}


void ST_unloadGraphics (void)
{
    int i;

    // unload the numbers, tall and short
    for (i=0 ; i < 10 ; i++)
    {
        Z_ChangeTag(tallnum[i], PU_CACHE);
        Z_ChangeTag(shortnum[i], PU_CACHE);
    }

    // unload tall percent
    Z_ChangeTag(tallpercent, PU_CACHE); 

    // unload arms background
    Z_ChangeTag(armsbg, PU_CACHE); 
    Z_ChangeTag(armsbg_rus, PU_CACHE);

    // unload gray #'s
    for (i=0 ; i < 6 ; i++)
    {
        Z_ChangeTag(arms[i][0], PU_CACHE);
    }
    
    // unload the key cards
    for (i=0 ; i < NUMCARDS ; i++)
    {
        Z_ChangeTag(keys[i], PU_CACHE);
    }

    Z_ChangeTag(sbar, PU_CACHE);
    Z_ChangeTag(sbar_rus, PU_CACHE);
    Z_ChangeTag(faceback, PU_CACHE);

    for (i=0 ; i < ST_NUMFACES ; i++)
    {
        Z_ChangeTag(faces[i], PU_CACHE);
    }

    // Note: nobody ain't seen no unloading of stminus yet. Dude.
}


void ST_unloadData (void)
{
    ST_unloadGraphics();
}


void ST_initData (void)
{
    int i;

    st_firsttime = true;
    plyr = &players[consoleplayer];

    st_clock = 0;
    st_chatstate = StartChatState;
    st_gamestate = FirstPersonState;

    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_cursoron = false;

    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i=0 ; i < NUMWEAPONS ; i++)
    {
        oldweaponsowned[i] = plyr->weaponowned[i];
    }

    for (i=0 ; i < 3 ; i++)
    {
        keyboxes[i] = -1;
    }

    STlib_init();
}


void ST_createWidgets (void)
{
    int i;

    // ready weapon ammo
    STlib_initNum(&w_ready,
                  ST_AMMOX,
                  ST_AMMOY,
                  tallnum,
                  &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                  &st_statusbaron,
                  ST_AMMOWIDTH );

    // the last weapon type
    w_ready.data = plyr->readyweapon; 

    // health percentage
    STlib_initPercent(&w_health,
                      ST_HEALTHX,
                      ST_HEALTHY,
                      tallnum,
                      &plyr->health,
                      &st_statusbaron,
                      tallpercent);

    // Negative player health
    STlib_initPercent(&w_health_neg,
                      ST_HEALTHX,
                      ST_HEALTHY,
                      tallnum,
                      &plyr->health_neg,
                      &st_statusbaron,
                      tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg,
                      ST_ARMSBGX,
                      ST_ARMSBGY,
                      english_language ? armsbg : armsbg_rus,
                      &st_notdeathmatch,
                      &st_statusbaron);

    // weapons owned
    for(i=0 ; i < 6 ; i++)
    {
        STlib_initMultIcon(&w_arms[i],
                           ST_ARMSX+(i%3)*ST_ARMSXSPACE,
                           ST_ARMSY+(i/3)*ST_ARMSYSPACE,
                           arms[i], (int *) &plyr->weaponowned[i+1],
                           &st_armson);
    }

    // [Doom Retro] & [crispy] show SSG availability in the Shotgun slot of the arms widget
    w_arms[1].inum = &st_shotguns;

    // frags sum
    STlib_initNum(&w_frags,
                  ST_FRAGSX,
                  ST_FRAGSY,
                  tallnum,
                  &st_fragscount,
                  &st_fragson,
                  ST_FRAGSWIDTH);

    // faces
    STlib_initMultIcon(&w_faces,
                       ST_FACESX,
                       ST_FACESY,
                       faces,
                       &st_faceindex,
                       &st_statusbaron);

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor,
                      ST_ARMORX,
                      ST_ARMORY,
                      tallnum,
                      &plyr->armorpoints,
                      &st_statusbaron, tallpercent);

    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0],
                       ST_KEY0X,
                       ST_KEY0Y,
                       keys,
                       &keyboxes[0],
                       &st_statusbaron);
    
    STlib_initMultIcon(&w_keyboxes[1],
                       ST_KEY1X,
                       ST_KEY1Y,
                       keys,
                       &keyboxes[1],
                       &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[2],
                       ST_KEY2X,
                       ST_KEY2Y,
                       keys,
                       &keyboxes[2],
                       &st_statusbaron);

    // ammo count (all four kinds)
    STlib_initNum(&w_ammo[0],
                  ST_AMMO0X,
                  ST_AMMO0Y,
                  shortnum,
                  &plyr->ammo[0],
                  &st_statusbaron,
                  ST_AMMO0WIDTH);

    STlib_initNum(&w_ammo[1],
                  ST_AMMO1X,
                  ST_AMMO1Y,
                  shortnum,
                  &plyr->ammo[1],
                  &st_statusbaron,
                  ST_AMMO1WIDTH);

    STlib_initNum(&w_ammo[2],
                  ST_AMMO2X,
                  ST_AMMO2Y,
                  shortnum,
                  &plyr->ammo[2],
                  &st_statusbaron,
                  ST_AMMO2WIDTH);

    STlib_initNum(&w_ammo[3],
                  ST_AMMO3X,
                  ST_AMMO3Y,
                  shortnum,
                  &plyr->ammo[3],
                  &st_statusbaron,
                  ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_initNum(&w_maxammo[0],
                  ST_MAXAMMO0X,
                  ST_MAXAMMO0Y,
                  shortnum,
                  &plyr->maxammo[0],
                  &st_statusbaron,
                  ST_MAXAMMO0WIDTH);

    STlib_initNum(&w_maxammo[1],
                  ST_MAXAMMO1X,
                  ST_MAXAMMO1Y,
                  shortnum,
                  &plyr->maxammo[1],
                  &st_statusbaron,
                  ST_MAXAMMO1WIDTH);

    STlib_initNum(&w_maxammo[2],
                  ST_MAXAMMO2X,
                  ST_MAXAMMO2Y,
                  shortnum,
                  &plyr->maxammo[2],
                  &st_statusbaron,
                  ST_MAXAMMO2WIDTH);

    STlib_initNum(&w_maxammo[3],
                  ST_MAXAMMO3X,
                  ST_MAXAMMO3Y,
                  shortnum,
                  &plyr->maxammo[3],
                  &st_statusbaron,
                  ST_MAXAMMO3WIDTH);
}

void ST_Start (void)
{
    if (!st_stopped)
    {
        ST_Stop();
    }

    ST_initData();
    ST_createWidgets();
    st_stopped = false;
}


void ST_Stop (void)
{
    if (st_stopped)
    {
        return;
    }

    I_SetPalette (W_CacheLumpNum (usegamma <= 8 ? 
                                  lu_palette1 : lu_palette2, PU_CACHE));

    st_stopped = true;
}


void ST_Init (void)
{
    veryfirsttime = 0;
    ST_loadData();
    screens[4] = (byte *) Z_Malloc(ST_WIDTH*ST_HEIGHT, PU_STATIC, 0);
}
