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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//


#include "i_swap.h" // [crispy] SHORT()
#include "z_zone.h"
#include "m_misc.h"
#include "w_wad.h"
#include "deh_main.h"
#include "deh_misc.h"
#include "g_game.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "p_local.h"
#include "am_map.h"
#include "m_menu.h"
#include "s_sound.h"
#include "v_video.h"
#include "doomstat.h"
#include "rd_lang.h"
#include "rd_text.h"
#include "v_trans.h"
#include "v_diskicon.h"
#include "jn.h"


// -----------------------------------------------------------------------------
// STATUS BAR DATA
// -----------------------------------------------------------------------------

// Palette indices. For damage/bonus red-/gold-shifts
#define STARTREDPALS        1
#define STARTBONUSPALS      9
#define NUMREDPALS          8
#define NUMBONUSPALS        4
// Radiation suit, green shift.
#define RADIATIONPAL        13
// [JN] Jaguar Doom: cyan invulnerability palette
#define INVULNERABILITYPAL  14

// Location of status bar
#define ST_X                0
#define ST_FX               143
#define ST_FY               169

// Number of status faces.
#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES     2
#define ST_NUMSPECIALFACES  3

#define ST_FACESTRIDE (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)
#define ST_NUMEXTRAFACES    2

// [JN] Additional faces:
// - Jaguar Doom: +6 (exploded face)
// - PSX Doom: +1 (squished face)
#define ST_NUMFACES (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES+7)

#define ST_TURNOFFSET       (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE+1)
// [JN] Jaguar Doom: exploded face.
#define ST_EXPLFACE0        (ST_DEADFACE+1)
#define ST_EXPLFACE1        (ST_EXPLFACE0+1)
#define ST_EXPLFACE2        (ST_EXPLFACE1+1)
#define ST_EXPLFACE3        (ST_EXPLFACE2+1)
#define ST_EXPLFACE4        (ST_EXPLFACE3+1)
#define ST_EXPLFACE5        (ST_EXPLFACE4+1)
// [JN] PSX Doom: squished face.
#define ST_CRSHFACE0        (ST_EXPLFACE5+1)

#define ST_FACESX           143
#define ST_FACESY           168

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN         20

// Location and size of statistics, justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump. Problem is, is the stuff rendered
// into a buffer, or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH        3	
#define ST_AMMOX            44
#define ST_AMMOY            171

// HEALTH number pos.
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

// [JN] Press Beta: player's life pos.
#define ST_LIFESX           177
#define ST_LIFESY           193
#define ST_LIFESWIDTH       1

// ARMOR number pos.
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

// Dimensions given in characters.
#define ST_MSGWIDTH         52


extern boolean sgl_loaded;
extern boolean old_godface;  // [JN] If false, we can use an extra GOD faces.

void ST_Stop (void);

// [JN] Jaguar Doom: prototypes
void ST_drawWidgetsJaguar (boolean refresh);
void ST_createWidgetsJaguar (void);


enum
{
    hudcolor_ammo,
    hudcolor_health,
    hudcolor_frags,
    hudcolor_armor,
    hudcolor_artifacts
} hudcolor_t;

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);


// graphics are drawn to a backing screen and blitted to the real screen
byte *st_backing_screen;

static char msg[ST_MSGWIDTH];

static player_t *plyr;          // main player in game

int        st_palette = 0;
static int lu_palette;                // lump number for PLAYPAL
static int st_msgcounter = 0;         // used for making messages go away

static st_chatstateenum_t st_chatstate; // used when in chat

static st_stateenum_t st_gamestate;     // whether in automap or first-person

static boolean st_firsttime;  // ST_Start() has just been called
static boolean st_statusbaron;    // whether left-side main status bar is active
static boolean st_chat;           // whether status bar chat is active
static boolean st_oldchat;        // value of st_chat before message popped up
static boolean st_cursoron;       // whether chat window has the cursor on
static boolean st_notdeathmatch;  // !deathmatch
static boolean st_armson;         // !deathmatch && st_statusbaron
static boolean st_fragson;        // !deathmatch
static boolean st_artifactson;    // [JN] only in Press Beta
static boolean st_stopped = true;

static patch_t *sbar, *sbar_rus;  // main bar left
static patch_t *tallnum[10];      // 0-9, tall numbers
static patch_t *tallpercent;      // tall % sign
static patch_t *shortnum[10];     // 0-9, short, yellow (,different!) numbers

// jff 2/24/98 extend number of patches by three skull/card combos
static patch_t *keys[NUMCARDS+3];  // 3 key-cards, 3 skulls 

// [JN] Doubled array for extra god mode faces, thanks Brad Harding for help!
static patch_t *faces[ST_NUMFACES * 2];  // face status patches
static patch_t *faceback;                // face background   
static patch_t *armsbg, *armsbg_rus;     // main bar right
static patch_t *arms[6][2];              // weapon ownership patches

static st_number_t w_ready, w_ready_wide;  // ready-weapon widget
static st_number_t w_frags, w_frags_wide;  // in deathmatch only, summary of frags stats

static st_number_t w_currentmap, w_currentmap_wide;  // [JN] Jaguar: current map widget
static st_number_t w_artifacts, w_artifacts_wide;    // [JN] Press Beta: artifacts widget
static st_number_t w_lifes;  // [JN] Press Beta: widget for player's lifes

static st_percent_t w_health, w_health_wide;  // health widget

static boolean      st_neghealth;           // [JN] Negative player health
static st_percent_t w_health_neg, w_health_neg_wide;
static st_percent_t w_armor, w_armor_wide;  // armor widget

static st_binicon_t     w_armsbg, w_armsbg_wide;    // arms background

// [DOOM Retro] & [crispy] show SSG availability in the Shotgun slot of the arms widget
static int st_shotguns;

static st_multicon_t w_faces;                            // face status widget
static st_multicon_t w_arms[6], w_arms_wide[6];  // weapon ownership widgets
static st_multicon_t w_keyboxes[3], w_keyboxes_wide[3];  // keycard widgets

static st_number_t w_ammo[4], w_ammo_wide[4];        // ammo widgets
static st_number_t w_maxammo[4], w_maxammo_wide[4];  // max ammo widgets

static int st_fragscount;      // number of frags so far in deathmatch
static int st_artifactscount;  // [JN] Press Beta: number of picked up artifacts
static int st_oldhealth = -1;  // used to use appopriately pained face
static boolean oldweaponsowned[NUMWEAPONS];  // used for evil grin

static int st_randomnumber;   // a random number per tick
static int st_facecount = 0;  // count until face changes
static int st_faceindex = 0;  // current face index, used by w_faces
static int keyboxes[3];       // holds key-type for each key box on bar

int st_keyorskull[3];  // [crispy] blinking key or skull in the status bar

// [JN] Different status height between common Doom and Jaguar Doom.
static int st_height;
static int st_y;

// [JN] status bar widget colors.
byte *sbar_color_high_set;
byte *sbar_color_normal_set;
byte *sbar_color_low_set;
byte *sbar_color_critical_set;
byte *sbar_color_armor_1_set;
byte *sbar_color_armor_2_set;
byte *sbar_color_armor_0_set;

cheatseq_t cheat_mus = CHEAT("idmus", 2);
cheatseq_t cheat_god = CHEAT("iddqd", 0);
cheatseq_t cheat_ammo = CHEAT("idkfa", 0);
cheatseq_t cheat_ammonokey = CHEAT("idfa", 0);
cheatseq_t cheat_keys = CHEAT("idka", 0);
cheatseq_t cheat_noclip = CHEAT("idspispopd", 0);
cheatseq_t cheat_commercial_noclip = CHEAT("idclip", 0);

cheatseq_t cheat_powerup[7] =
{
    CHEAT("idbeholdv", 0),
    CHEAT("idbeholds", 0),
    CHEAT("idbeholdi", 0),
    CHEAT("idbeholdr", 0),
    CHEAT("idbeholda", 0),
    CHEAT("idbeholdl", 0),
    CHEAT("idbehold",  0),
};

cheatseq_t cheat_choppers = CHEAT("idchoppers", 0);
cheatseq_t cheat_clev = CHEAT("idclev", 2);
cheatseq_t cheat_mypos = CHEAT("idmypos", 0);

// [crispy] pseudo cheats to eat up the first digit typed after a cheat expecting two parameters
cheatseq_t cheat_mus1 = CHEAT("idmus", 1);
cheatseq_t cheat_clev1 = CHEAT("idclev", 1);

// [crispy] new cheats
cheatseq_t cheat_massacre = CHEAT("tntem", 0);

// [JN] Press Beta cheat codes
cheatseq_t cheat_god_beta    = CHEAT("tst", 0); // iddqd
cheatseq_t cheat_ammo_beta   = CHEAT("amo", 0); // idkfa
cheatseq_t cheat_noclip_beta = CHEAT("nc", 0);  // idclip


// -----------------------------------------------------------------------------
// STATUS BAR CODE
// -----------------------------------------------------------------------------

void ST_refreshBackground (void)
{
    if (screenblocks >= 11 && (!automapactive || automap_overlay))
    {
        return;
    }

    if (st_statusbaron)
    {
        V_UseBuffer(st_backing_screen);

        // [JN] Draw side screen borders in wide screen mode.
        if (aspect_ratio >= 2)
        {
            // [crispy] this is our own local copy of R_FillBackScreen() to
            // fill the entire background of st_backing_screen with the bezel pattern,
            // so it appears to the left and right of the status bar in widescreen mode
            if ((screenwidth >> hires) != ORIGWIDTH)
            {
                int x, y;
                byte *src;
                byte *dest;
                char *name = (gamemode == commercial) ? DEH_String("GRNROCK") : 
                                                        DEH_String("FLOOR7_2");
                const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;

                src = W_CacheLumpName(name, PU_CACHE);
                dest = st_backing_screen;
        
                // [JN] Variable HUD detail level.
                for (y = SCREENHEIGHT-(st_height << hires); y < SCREENHEIGHT; y++)
                {
                    for (x = 0; x < screenwidth; x++)
                    {
                        *dest++ = src[((( y >> shift_allowed) & 63) << 6) 
                                     + (( x >> shift_allowed) & 63)];
                    }
                }
        
                // [JN] Draw bezel bottom edge.
                if (scaledviewwidth == screenwidth)
                {
                    patch_t *patch = W_CacheLumpName(DEH_String("brdr_b"), PU_CACHE);
        
                    for (x = 0; x < screenwidth; x += 8)
                    {
                        if (hud_detaillevel)
                        {
                            V_DrawPatch(x, 0, patch);
                        }
                        else
                        {
                            V_DrawPatchUnscaled(x, 0, patch, NULL);
                        }
                    }
                }
            }
        }

        // [JN] Always draw status bar on the center of the screen.
        if (english_language)
        {
            V_DrawPatch(ST_X + (ORIGWIDTH - SHORT(sbar->width)) / 2 + wide_delta, 0, sbar);
        }
        else
        {
            V_DrawPatch(ST_X + (ORIGWIDTH - SHORT(sbar_rus->width)) / 2 + wide_delta, 0, sbar_rus);
        }

        // [crispy] back up arms widget background
        if (!deathmatch && gamemode != pressbeta)
        {
            V_DrawPatch(ST_ARMSBGX + wide_delta, 0, english_language ? armsbg : armsbg_rus);
        }

        if (netgame)
        {
            V_DrawPatch(ST_FX + wide_delta, 0, faceback);
        }

        V_RestoreBuffer();

        V_CopyRect(ST_X, 0, st_backing_screen, origwidth, st_height, ST_X, st_y);
    }
}

// -----------------------------------------------------------------------------
// ST_cheat_massacre
// [crispy] adapted from boom202s/M_CHEAT.C:467-498
// -----------------------------------------------------------------------------

static int ST_cheat_massacre (void)
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

// -----------------------------------------------------------------------------
// ST_Responder
// Respond to keyboard input events, intercept cheats.
// -----------------------------------------------------------------------------

boolean ST_Responder (event_t *ev)
{
    int i;

    // Filter automap on/off.
    if (ev->type == ev_keyup && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
    {
        switch (ev->data1)
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
        if (!netgame && (gameskill != sk_nightmare) && gamemode != pressbeta)
        {
            // 'dqd' cheat for toggleable god mode
            if (cht_CheckCheat(&cheat_god, ev->data2))
            {
                // [crispy] dead players are first respawned at the current position
                mapthing_t mt = {0};
                if (plyr->playerstate == PST_DEAD)
                {
                    signed int an;
                    extern void P_SpawnPlayer (mapthing_t *mthing);

                    mt.x = plyr->mo->x >> FRACBITS;
                    mt.y = plyr->mo->y >> FRACBITS;
                    mt.angle = (plyr->mo->angle + ANG45/2)*(uint64_t)45/ANG45;
                    mt.type = consoleplayer + 1;
                    P_SpawnPlayer(&mt);
 
                    // [crispy] spawn a teleport fog
                    an = plyr->mo->angle >> ANGLETOFINESHIFT;
                    P_SpawnMobj(plyr->mo->x+20*finecosine[an],
                                plyr->mo->y+20*finesine[an],
                                plyr->mo->z, MT_TFOG);
                    S_StartSound(plyr->mo, sfx_telept);
                }

                plyr->cheats ^= CF_GODMODE;

                if (plyr->cheats & CF_GODMODE)
                {
                    // [JN] Jaguar Doom: no healing!
                    if (gamemission != jaguar)
                    {
                        if (plyr->mo)
                        {
                            plyr->mo->health = deh_god_mode_health;
                        }
                        plyr->health = deh_god_mode_health;
                    }
                    plyr->message_system = DEH_String(ststr_dqdon);
                }
                else
                {
                    plyr->message_system = DEH_String(ststr_dqdoff);
                }
            }

            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheat(&cheat_ammonokey, ev->data2))
            {
                plyr->armorpoints = deh_idfa_armor;
                plyr->armortype = deh_idfa_armor_class;

                // [JN] Give all weapons properly.
                // Don't give SSG, Plasmagun and BFG in Shareware, leave their
                // indication on HUD as gray numbers. Give SSG only in Doom 2.
                plyr->weaponowned[wp_fist] = true;
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->weaponowned[wp_pistol] = true;
                plyr->weaponowned[wp_shotgun] = true;
                if (gamemode == commercial && gamemission != jaguar)
                plyr->weaponowned[wp_supershotgun] = true;
                plyr->weaponowned[wp_chaingun] = true;
                plyr->weaponowned[wp_missile] = true;
                plyr->weaponowned[wp_plasma] = gamemode == shareware ? false : true;
                plyr->weaponowned[wp_bfg] = gamemode == shareware ? false : true;

                // [JN] Check if player have a backpack...
                if (!plyr->backpack && !vanillaparm)
                {
                    for (i = 0 ; i < NUMAMMO ; i++)
                    {
                        plyr->maxammo[i] *= 2;
                    }
                    plyr->backpack = true;
                }

                // [JN] ...and only then we replenish ammunition.
                for (i = 0; i < NUMAMMO ; i++)
                {
                    // [JN] Jaguar: happy 500 ammo for everything!
                    if (gamemission == jaguar)
                    {
                        plyr->ammo[i] = plyr->maxammo[i] = 500;
                    }
                    else
                    {
                        plyr->ammo[i] = plyr->maxammo[i];
                    }
                }

                plyr->message_system = DEH_String(ststr_faadded);
            }

            // 'kfa' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo, ev->data2))
            {
                plyr->armorpoints = deh_idkfa_armor;
                plyr->armortype = deh_idkfa_armor_class;

                // [JN] Give all weapons properly.
                // Don't give SSG, Plasmagun and BFG in Shareware, leave their
                // indication on HUD as gray numbers. Give SSG only in Doom 2.
                plyr->weaponowned[wp_fist] = true;
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->weaponowned[wp_pistol] = true;
                plyr->weaponowned[wp_shotgun] = true;
                if (gamemode == commercial && gamemission != jaguar)
                plyr->weaponowned[wp_supershotgun] = true;
                plyr->weaponowned[wp_chaingun] = true;
                plyr->weaponowned[wp_missile] = true;
                plyr->weaponowned[wp_plasma] = gamemode == shareware ? false : true;
                plyr->weaponowned[wp_bfg] = gamemode == shareware ? false : true;
	
                // [JN] Check if player have a backpack.
                if (!plyr->backpack && !vanillaparm)
                {
                    for (i = 0 ; i < NUMAMMO ; i++)
                    {
                        plyr->maxammo[i] *= 2;
                    }
                    plyr->backpack = true;
                }

                // [JN] ...and only then we replenish ammunition.
                for (i = 0 ; i < NUMAMMO ; i++)
                {
                    // [JN] Jaguar: happy 500 ammo for everything!
                    if (gamemission == jaguar)
                    plyr->ammo[i] = plyr->maxammo[i] = 500;
                    else
                    plyr->ammo[i] = plyr->maxammo[i];
                }

                for (i = 0 ; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                plyr->message_system = DEH_String(ststr_kfaadded);
            }

            // [JN] 'ka' чит для выдачи ключей
            else if (cht_CheckCheat(&cheat_keys, ev->data2))
            {
                for (i = 0; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                plyr->message_system = DEH_String(ststr_kaadded);
            }

            // [crispy] implement Boom's "tntem" cheat
            else if (cht_CheckCheat(&cheat_massacre, ev->data2))
            {
                int killcount = ST_cheat_massacre();

                M_snprintf(msg, sizeof(msg), "%s %d", ststr_massacre, killcount);
                plyr->message_system = msg;
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus, ev->data2))
            {
                char buf[3];
                int  musnum;

                plyr->message_system = DEH_String(ststr_mus);
                cht_GetParam(&cheat_mus, buf);

                // Note: The original v1.9 had a bug that tried to play back
                // the Doom II music regardless of gamemode.  This was fixed
                // in the Ultimate Doom executable so that it would work for
                // the Doom 1 music as well.

                // [JN] Fixed: using a proper IDMUS selection for shareware 
                // and registered game versions.
                if (gamemode == commercial/* || gameversion < exe_ultimate*/)
                {
                    musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;

                    // [crispy] prevent crash with IDMUS00
                    // [JN] Jaguar: do not try to play D_ROMER2 (map27) and higher
                    if (((((buf[0]-'0')*10 + buf[1]-'0') > 35 || musnum < mus_runnin) && gameversion >= exe_doom_1_8)
                    ||  ((((buf[0]-'0')*10 + buf[1]-'0') > 26 || musnum < mus_runnin) && gamemission == jaguar))
                        plyr->message_system = DEH_String(ststr_nomus);
                    else
                    {
                        S_ChangeMusic(musnum, 1);
                        // [JN] jff 3/17/98 remember idmus number for restore
                        idmusnum = musnum;
                        // [crispy] eat key press, i.e. don't change weapon upon music change
                        return true;
                    }
                }
                else
                {
                    musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');

                    // [crispy] prevent crash with IDMUS0x or IDMUSx0
                    // [JN] Sigil: allow to choose E5MX music, otherwise don't allow to choose E4MX music.
                    if ((((buf[0]-'1')*9 + buf[1]-'1') > (sgl_loaded ? 41 : 21) || buf[0] < '1' || buf[1] < '1'))
                    {
                        plyr->message_system = DEH_String(ststr_nomus);
                    }
                    else
                    {
                        S_ChangeMusic(musnum, 1);
                        // [JN] jff 3/17/98 remember idmus number for restore
                        idmusnum = musnum;
                        // [crispy] eat key press, i.e. don't change weapon upon music change
                        return true;
                    }
                }
            }

            // [crispy] eat up the first digit typed after a cheat expecting two parameters
            else if (cht_CheckCheat(&cheat_mus1, ev->data2))
            {
                char buf[2];
                cht_GetParam(&cheat_mus1, buf);
                return isdigit(buf[0]);
            }

            // [crispy] allow both idspispopd and idclip cheats in all gamemissions
            else if ( ( /* logical_gamemission == doom
            && */ cht_CheckCheat(&cheat_noclip, ev->data2))
            || ( /* logical_gamemission != doom
            && */ cht_CheckCheat(&cheat_commercial_noclip,ev->data2)))
            {	
                // Noclip cheat.
                // For Doom 1, use the idspipsopd cheat; for all others, use
                // idclip

                plyr->cheats ^= CF_NOCLIP;

                if (plyr->cheats & CF_NOCLIP)
                {
                    plyr->message_system = DEH_String(ststr_ncon);
                }
                else
                {
                    plyr->message_system = DEH_String(ststr_ncoff);
                }
            }

            // 'behold?' power-up cheats
            for (i = 0 ; i < 6 ; i++)
            {
                if (cht_CheckCheat(&cheat_powerup[i], ev->data2))
                {
                    // [JN] Atari Jaguar: no invisibility sphere and light visor
                    if (gamemission == jaguar && (i == pw_invisibility || i == pw_infrared))
                    {
                        return false;
                    }

                    if (!plyr->powers[i])
                    {
                        P_GivePower( plyr, i);
                        plyr->message_system = DEH_String(ststr_beholdx); // [JN] Активирован
                    }
                    else if (i!=pw_strength && i!=pw_allmap) // [crispy] disable full Automap
                    {
                        plyr->powers[i] = 1;
                        plyr->message_system = DEH_String(ststr_beholdz); // [JN] Деактивирован
                    }
                    else
                    {
                        plyr->powers[i] = 0;
                        plyr->message_system = DEH_String(ststr_beholdz); // [JN] Деактивирован
                    }
                }
            }

            // 'behold' power-up menu
            if (cht_CheckCheat(&cheat_powerup[6], ev->data2))
            {
                plyr->message_system = DEH_String(ststr_behold);
            }

            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data2))
            {
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->powers[pw_invulnerability] = true;
                plyr->message_system = DEH_String(ststr_choppers);
            }

            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data2))
            {
                static char buf[ST_MSGWIDTH];

                // [JN] Show human-readable coords
                M_snprintf(buf, sizeof(buf), english_language ?
                                "x=%d, y=%d, ang=%d" :
                                "[=%d< e=%d< eujk=%d",
                                players[consoleplayer].mo->x >> FRACBITS,
                                players[consoleplayer].mo->y >> FRACBITS,
                                players[consoleplayer].mo->angle / ANG1);
                plyr->message_system = buf;
            }
        }

        // 'clev' change-level cheat
        if (!netgame && cht_CheckCheat(&cheat_clev, ev->data2) && gamemode != pressbeta)
        {
            char    buf[3];
            int     epsd;
            int     map;

            cht_GetParam(&cheat_clev, buf);

            if (gamemode == commercial)
            {
                epsd = 1;
                map = (buf[0] - '0')*10 + buf[1] - '0';
            }
            else
            {
                epsd = buf[0] - '0';
                map = buf[1] - '0';

                // Chex.exe always warps to episode 1.
                if (gameversion == exe_chex)
                {
                    if (epsd > 1)
                    {
                        epsd = 1;
                    }
                    if (map > 5)
                    {
                        map = 5;
                    }
                }
            }

            // Catch invalid maps.
            if (gamemode != commercial)
            {
                if (epsd < 1)
                {
                    return false;
                }
                if (epsd > 4)
                {
                    // [crispy] Sigil
                    if (!(sgl_loaded && epsd == 5))
                    return false;
                }
                if (epsd == 4 && gameversion < exe_ultimate)
                {
                    return false;
                }
                if (map < 1)
                {
                    return false;
                }
                if (map > 9)
                {
                    return false;
                }
            }
            else
            {
                if (map < 1)
                {
                    return false;
                }
                if (map > 40)
                {
                    return false;
                }
                // [JN] В NRFTL не перемещаться на уровень 10 и выше
                if (map > 9 && gamemission == pack_nerve)
                {
                    return false;
                }
                // [JN] Jaguar: dont warp to map 27 and higher
                if (map > 26 && gamemission == jaguar)
                {
                    return false;
                }
            }

            // So be it.
            plyr->message_system = DEH_String(ststr_clev);
            G_DeferedInitNew(gameskill, epsd, map);
            // [crispy] eat key press, i.e. don't change weapon upon level change
            return true;
        }

        // [crispy] eat up the first digit typed after a cheat expecting two parameters
        else if (!netgame && !menuactive && cht_CheckCheat(&cheat_clev1, ev->data2))
        {
            char buf[2];
            cht_GetParam(&cheat_clev1, buf);
            return isdigit(buf[0]);
        }

        // [JN] Finally, Press Beta cheats
        if (gamemode == pressbeta)
        {
            // 'TST' - god mode
            if (cht_CheckCheat(&cheat_god_beta, ev->data2))
            {
                plyr->cheats ^= CF_GODMODE;

                if (plyr->cheats & CF_GODMODE)
                {
                    if (plyr->mo)
                    {
                        plyr->mo->health = 100;
                    }

                    plyr->health = deh_god_mode_health;
                    plyr->message_system = DEH_String(ststr_dqdon);
                }
                else
                {
                    plyr->message_system = DEH_String(ststr_dqdoff);
                }
            }

            // 'AMO' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo_beta, ev->data2))
            {
                plyr->armorpoints = deh_idkfa_armor;
                plyr->armortype = deh_idkfa_armor_class;

                for (i = 0 ; i < NUMWEAPONS ; i++)
                {
                    plyr->weaponowned[i] = true;
                }
                for (i = 0 ; i < NUMAMMO ; i++)
                {
                    plyr->ammo[i] = plyr->maxammo[i];
                }
                for (i = 0 ; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                plyr->message_system = DEH_String(ststr_kfaadded);
            }

            // 'NC' - noclipping mode
            else if (cht_CheckCheat(&cheat_noclip_beta, ev->data2))
            {	
                plyr->cheats ^= CF_NOCLIP;

                if (plyr->cheats & CF_NOCLIP)
                {
                    plyr->message_system = DEH_String(ststr_ncon);
                }
                else
                {
                    plyr->message_system = DEH_String(ststr_ncoff);
                }
            }
        }
    }

    return false;
}

// -----------------------------------------------------------------------------
// ST_calcPainOffset
// -----------------------------------------------------------------------------

static int ST_calcPainOffset (void)
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

// -----------------------------------------------------------------------------
// ST_updateFaceWidget
// This is a not-very-pretty routine which handles the face states and 
// their timing. The precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
// [crispy] fix status bar face hysteresis
// -----------------------------------------------------------------------------

static int faceindex;

static void ST_updateFaceWidget (void)
{
    int         i;
    static int  lastattackdown = -1;
    static int  priority = 0;
    boolean     doevilgrin;

    int         painoffset;

    painoffset = ST_calcPainOffset();

    if (priority < 10)
    {
        // dead
        // [crispy] theoretically allow for negative player health values
        if (plyr->health <= 0)
        {
            priority = 9;
            painoffset = 0;
            faceindex = ST_DEADFACE;
            st_facecount = 1;
        }
        // [JN] Jaguar and PSX Doom: extra exploded and squished player faces.
        // Proper checking for xdeath state has been taken from Crispy Doom, thanks Fabian Greffrath!
        // Don't use extra faces if possible custom faces has been loaded.
        if (!old_godface && extra_player_faces && !vanillaparm
        && plyr->health <= 0 && plyr->mo->state - states >= mobjinfo[plyr->mo->type].xdeathstate)
        {
            priority = 9;
            painoffset = 0;

            // [JN] Sync with actual player state:
            if (plyr->mo->state == &states[S_PLAY_XDIE1])
            {
                faceindex = ST_EXPLFACE0;
            }
            if (plyr->mo->state == &states[S_PLAY_XDIE2])
            {
                faceindex = ST_EXPLFACE1;
            }
            if (plyr->mo->state == &states[S_PLAY_XDIE3])
            {
                faceindex = ST_EXPLFACE2;
            }
            if (plyr->mo->state == &states[S_PLAY_XDIE4])
            {
                faceindex = ST_EXPLFACE3;
            }
            if (plyr->mo->state == &states[S_PLAY_XDIE5])
            {
                faceindex = ST_EXPLFACE4;
            }
            if (plyr->mo->state >= &states[S_PLAY_XDIE6])
            {
                faceindex = ST_EXPLFACE5;
            }

            if (plyr->mo->state == &states[S_GIBS])
            {
                faceindex = ST_CRSHFACE0;
            }
        }
    }

    if (priority < 9)
    {
        if (plyr->bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0;i<NUMWEAPONS;i++)
            {
                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                    // [BH] no evil grin when invulnerable
                    // [JN] extra god faces have grin, use them in god mode
                    if (extra_player_faces
                    || old_godface
                    || (!old_godface && !(plyr->cheats & CF_GODMODE) && !plyr->powers[pw_invulnerability]))
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
    { 	    // being attacked
        if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
        {
            angle_t badguyangle = R_PointToAngle2(plyr->mo->x, plyr->mo->y, plyr->attacker->x,plyr->attacker->y);
            angle_t diffang;

            // [JN] Исправление бага с отсутствующим Ouch Face.
            // По методу Brad Harding (DOOM Retro).

            // [JN] Корректная формула "Ouch face"
            if (!vanillaparm && gamemode != pressbeta)
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
                    }       // confusing, ain't it?

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

            // [JN] Традиционная формула
            else
            {
                if (plyr->health - st_oldhealth > ST_MUCHPAIN)
                {
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                    priority = 7; // [JN] Традиционный приоритет
                }
                else
                {
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
                    } // confusing, aint it?

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

    // [JN] Исправление бага с отсутствующим Ouch Face.
    // По методу Brad Harding (DOOM Retro).

    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (plyr->damagecount)
        {
            if (gamemode != pressbeta)
            {
            if (extra_player_faces && !vanillaparm)
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

            // [JN] Press Beta: OUCH face is appearing only after getting
            // hurt in special sectors (slime, blood, etc). Emulate this.
            else
            {
                priority = 7;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_RAMPAGEOFFSET;
                
                if (plyr->mo->subsector->sector->special)
                {
                    priority = 6;
                    faceindex = ST_OUCHOFFSET;
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
            if (extra_player_faces
            || old_godface
            || (!old_godface && !(plyr->cheats & CF_GODMODE) && !plyr->powers[pw_invulnerability]))
            {
                if (lastattackdown==-1)
                    lastattackdown = ST_RAMPAGEDELAY;
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
            if (!extra_player_faces || old_godface)
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
    st_faceindex = painoffset + faceindex;

    // [JN] При наличии бессмертия активируется дополнительный цикл.
    if (!old_godface && ((plyr->powers[pw_invulnerability]) || (plyr->cheats & CF_GODMODE)))
    {
        st_faceindex = painoffset + faceindex + ST_NUMFACES;
    }
}

// -----------------------------------------------------------------------------
// ST_updateWidgets
// -----------------------------------------------------------------------------

static void ST_updateWidgets (void)
{
    static int largeammo = 1994; // means "n/a"
    int        i;

    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
    {
        w_ready.num = &largeammo;
        w_ready_wide.num = &largeammo;
    }
    else
    {
        w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
        w_ready_wide.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
    }


    w_ready.data = plyr->readyweapon;
    w_ready_wide.data = plyr->readyweapon;

    // update keycard multiple widgets
    for (i = 0 ; i < 3 ; i++)
    {
        keyboxes[i] = plyr->cards[i] ? i : -1;

        //jff 2/24/98 select double key
        // [JN] Press Beta have a bug with missing skull keys on HUD.
        // To emulate this, following condition must be commented out:
        if (plyr->cards[i+3])
        {
            keyboxes[i] = (keyboxes[i]==-1) ? i+3 : i+6;
        }

        // [crispy] blinking key or skull in the status bar
        // [JN] blink in any HUD size, except full screen (no HUD) and vanilla
        if (plyr->tryopen[i] && !vanillaparm)
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
                w_keyboxes_wide[i].oldinum = -1;
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

    for (i = 0 ; i < MAXPLAYERS ; i++)
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

    // [JN] Jaguar: current map
    if (gamemission == jaguar)
    {
        w_currentmap.data = gamemap;
    }

    // [JN] Press Beta: artifacts counter routines
    if (gamemode == pressbeta)
    {   
        st_artifactson = !deathmatch && st_statusbaron; 
        st_artifactscount = 0;
        st_artifactscount += artifactcount;
    }

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
    {
        st_chat = st_oldchat;
    }
}

// -----------------------------------------------------------------------------
// ST_Ticker
// -----------------------------------------------------------------------------

void ST_Ticker (void)
{
    // [JN] Use real random number generator
    // instead of M_Random LUT for faces stide.
    st_randomnumber = rand();
    ST_updateWidgets();
    st_oldhealth = plyr->health;
}

// -----------------------------------------------------------------------------
// ST_doPaletteStuff
// -----------------------------------------------------------------------------

void ST_doPaletteStuff (void)
{
    int   cnt;
    int   palette;
    byte *pal;

    cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
        // slowly fade the berzerk out
        const int bzc = 12 - (plyr->powers[pw_strength]>>6);

        if (bzc > cnt)
        cnt = bzc;
    }

    if (cnt)
    {
        palette = (cnt+7)>>3;

        if (palette >= NUMREDPALS)
        palette = NUMREDPALS-1;

        palette += STARTREDPALS;
    }

    // [JN] Изменение палитры при получении бонусов
    else if (plyr->bonuscount)
    {
        palette = (plyr->bonuscount+7)>>3;
        
        // [JN] Дополнительный фрейм палитры для более плавного
        // появления/угасания жёлтого экрана при подборе предметов.
        // https://doomwiki.org/wiki/PLAYPAL
        // [JN] 11.09.2018 - не применять фикс в режиме -vanilla и для Atari Jaguar.

        if (palette >= NUMBONUSPALS)
        palette = (vanillaparm || gamemission == jaguar) ?
                   NUMBONUSPALS-1 : NUMBONUSPALS;

        palette += (vanillaparm || gamemission == jaguar) ?
                    STARTBONUSPALS : STARTBONUSPALS-1;
    }

    // [JN] Don't replace CYAN palette with GREEN palette in Atari Jaguar
    else if ( plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet]&8)
    {
        palette = RADIATIONPAL;
    }
    else
    {
        palette = 0;
    }
    
    // [JN] Use CYAN invulnerability palette in Atari Jaguar,
    // unbreakable by other palettes
    if (gamemission == jaguar
    && (plyr->powers[pw_invulnerability] > 4*32 || (plyr->powers[pw_invulnerability]&8)))
    {
        palette = INVULNERABILITYPAL;
    }

    // In Chex Quest, the player never sees red.  Instead, the
    // radiation suit palette is used to tint the screen green,
    // as though the player is being covered in goo by an
    // attacking flemoid.

    if (gameversion == exe_chex && palette >= STARTREDPALS && palette < STARTREDPALS + NUMREDPALS)
    {
        palette = RADIATIONPAL;
    }

    if (palette != st_palette)
    {
        st_palette = palette;
        pal = (byte *) W_CacheLumpNum ((lu_palette), PU_CACHE) + palette * 768;
        I_SetPalette (pal);
    }
}

// -----------------------------------------------------------------------------
// ST_WidgetColor
// [crispy] return ammo/health/armor widget color
// -----------------------------------------------------------------------------

static byte *ST_WidgetColor (int i)
{
    if (!sbar_colored || vanillaparm)
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
                    return sbar_color_critical_set;
                else if (ammo < fullammo/2)
                    return sbar_color_low_set;
                else if (ammo <= fullammo)
                    return sbar_color_normal_set;
                else
                    return sbar_color_high_set;
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
                return cr[CR_WHITE];
            else if (health > 100)
                return sbar_color_high_set;
            else if (health >= 67)
                return sbar_color_normal_set;
            else if (health >= 34)
                return sbar_color_low_set;            
            else
                return sbar_color_critical_set;
            break;
        }
        case hudcolor_frags:
        {
            int frags = st_fragscount;

            if (frags < 0)
                return sbar_color_critical_set;
            else if (frags == 0)
                return sbar_color_low_set;
            else
                return sbar_color_normal_set;

            break;
        }
        case hudcolor_armor:
        {
	    // [crispy] Invulnerability powerup and God Mode cheat turn Armor values gray
	    if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
                return cr[CR_WHITE];
	    // [crispy] color by armor type
	    else if (plyr->armortype >= 2)
                return sbar_color_armor_2_set;
	    else if (plyr->armortype == 1)
                return sbar_color_armor_1_set;
	    else if (plyr->armortype == 0)
                return sbar_color_armor_0_set;
            break;
        }
        case hudcolor_artifacts:
        {
            // [JN] Press Beta: colorize artifacts widget.
            // There is no internal counter for amount of collected artifacts,
            // so the values are hardcoded (same on intermission screen).
            // Also, there is only one map per each episode, which making
            // definition of 'gamemap' unnecessary.
            if ((gameepisode == 1 && st_artifactscount == 36)
            ||  (gameepisode == 2 && st_artifactscount == 29)
            ||  (gameepisode == 3 && st_artifactscount == 25))
            return sbar_color_normal_set;

            else if (st_artifactscount > 0)
            return sbar_color_low_set;

            else
            return sbar_color_critical_set;

            break;
        }
    }

    return NULL;
}


// -----------------------------------------------------------------------------
// ST_drawWidgets
// [JN] Slightly reformated to draw widgets from left to right, to support
// extra types and wider version of status bar.
// -----------------------------------------------------------------------------

void ST_drawWidgets (boolean refresh)
{
    int   i;
    const boolean wider_stbar = aspect_ratio >= 2 && screenblocks >= 14 && (!automapactive || automap_overlay);

    // [JN] Jaguar: use own widgets drawing function
    if (gamemission == jaguar)
    {
        ST_drawWidgetsJaguar(refresh);
        return;
    }

    st_armson = st_statusbaron && !deathmatch;       // used by w_arms[] widgets
    st_fragson = deathmatch && st_statusbaron;       // used by w_frags widget
    st_artifactson = !deathmatch && st_statusbaron;  // [JN] used by w_artifacts widget
    st_neghealth = negative_health && plyr->health <= 0 && !vanillaparm;
    // [DOOM Retro] & [crispy] show SSG availability in the Shotgun slot of the arms widget
    st_shotguns = plyr->weaponowned[wp_shotgun] | plyr->weaponowned[wp_supershotgun];

    // [JN] Labels ("ammo", "health", "arms", "armor", total ammo).
    if (screenblocks == 11 || screenblocks == 12 || screenblocks == 14 || screenblocks == 15)
    {
        if (!automapactive || automap_overlay) // [JN] Don't draw signs in automap
        {
            // [JN] Don't draw ammo for fist and chainsaw
            if (weaponinfo[plyr->readyweapon].ammo != am_noammo)
            {
                V_DrawPatch(2 + (wider_stbar ? 0 : wide_delta), 191, W_CacheLumpName
                           (DEH_String(english_language ? "STCHAMMO" : "RDCHAMMO"), PU_CACHE));
            }

            if (deathmatch)
            {
                // [JN] Frags
                V_DrawPatch(108 + (wider_stbar ? 0: wide_delta), 191, W_CacheLumpName
                           (DEH_String(english_language ? "STCHFRGS" : "RDCHFRGS"), PU_CACHE));
            }
            else
            {
                // [JN] Arms
                V_DrawPatch(108 + (wider_stbar ? 0 : wide_delta), 191, W_CacheLumpName
                           (DEH_String(english_language ? "STCHARMS" : "RDCHARMS"), PU_CACHE));
            }

            // [JN] Health
            V_DrawPatch(52 + (wider_stbar ? 0 : wide_delta), 173, W_CacheLumpName
                       (DEH_String(english_language ? "STCHHLTH" : "RDCHHLTH"), PU_CACHE));
                       
            // [JN] Armor, ammo
            V_DrawPatch(52 + (wider_stbar ? wide_delta*2 : wide_delta), 173, W_CacheLumpName
                       (DEH_String(english_language ? "STCHARAM" : "RDCHARAM"), PU_CACHE));
        }
    }

    // [JN] Yellow slashes
    if (screenblocks > 10 && screenblocks < 17 && (!automapactive || automap_overlay))
    {
        V_DrawPatch(292 + (wider_stbar ? wide_delta*2 : wide_delta), 173,
                    W_CacheLumpName(DEH_String("STYSSLSH"), PU_CACHE));
    }

    // Ammo amount for current weapon
    dp_translation = ST_WidgetColor(hudcolor_ammo);
    STlib_updateNum(wider_stbar ? &w_ready_wide : &w_ready, refresh);
    dp_translation = NULL;

    // [crispy] draw berserk pack instead of no ammo if appropriate
    if ((screenblocks >= 11 && screenblocks <= 16)
    && (!automapactive || automap_overlay))
    {
        if (plyr->readyweapon == wp_fist && plyr->powers[pw_strength])
        {
            static int lump = -1;
            patch_t *patch;

            if (lump == -1)
            {
                lump = W_CheckNumForName(DEH_String("PSTRA0"));

                if (lump == -1)
                {
                    lump = W_CheckNumForName(DEH_String("MEDIA0"));
                }
            }

            patch = W_CacheLumpNum(lump, PU_CACHE);

            // [crispy] (23,179) is the center of the Ammo widget
            V_DrawPatch((23 - SHORT(patch->width)/2 + SHORT(patch->leftoffset)) + (wider_stbar ? 0 : wide_delta),
                         179 - SHORT(patch->height)/2 + SHORT(patch->topoffset),
                         patch);
        }
    }

    // Health, negative health.
    dp_translation = ST_WidgetColor(hudcolor_health);
    if (wider_stbar)
    {
        STlib_updatePercent(st_neghealth ? &w_health_neg_wide : &w_health_wide, refresh || screenblocks <= 16);
    }
    else
    {
        STlib_updatePercent(st_neghealth ? &w_health_neg : &w_health, refresh || screenblocks <= 16);
    }
    dp_translation = NULL;

    // ARMS background. Do not update in Press Beta, ARTF is used there.
    if ((screenblocks < 11 || (automapactive && !automap_overlay)) && gamemode != pressbeta)
    {
        STlib_updateBinIcon(&w_armsbg, refresh);
    }

    // ARMS digits. Do not update in Press Beta, ARTF is used there.
    if (gamemode != pressbeta)
    {
        for (i = 0 ; i < 6 ; i++)
        {
            STlib_updateMultIcon(wider_stbar ? &w_arms_wide[i] : &w_arms[i],
                                 refresh || (screenblocks >= 11 &&  screenblocks <= 16));
        }
    }

    // Frags
    if (deathmatch)
    {
        STlib_updateNum(wider_stbar ? &w_frags_wide : &w_frags, 
                        refresh || (screenblocks >= 11 && screenblocks <= 16));
    }

    // [JN] Player face background
    if ((screenblocks == 11 || screenblocks == 14) && (!automapactive || automap_overlay))
    {
        // [JN] TODO - using different PU_CACHE backgrounds for netgame.
        /*
        if (netgame)
        {
            V_DrawPatch(ST_FX + wide_delta, ST_FY, faceback);
        }
        else
        */
        {
            V_DrawPatch(ST_FX + wide_delta, ST_FY, 
                        W_CacheLumpName(DEH_String("STPB1"), PU_CACHE));
        }
    }

    // Player faces
    if ((screenblocks < 12 || screenblocks == 14) || (automapactive && !automap_overlay))
    {
        STlib_updateMultIcon(&w_faces, refresh || screenblocks == 11 || screenblocks == 14);
    }

    // Armor
    dp_translation = ST_WidgetColor(hudcolor_armor);
    if (wider_stbar)
    {
        STlib_updatePercent(&w_armor_wide, refresh || screenblocks <= 16);
    }
    else
    {
        STlib_updatePercent(&w_armor, refresh || screenblocks <= 13);
    }
    dp_translation = NULL;

    // Key boxes.
    for (i = 0 ; i < 3 ; i++)
    {
        STlib_updateMultIcon(wider_stbar ? &w_keyboxes_wide[i] : &w_keyboxes[i],
                             refresh || (screenblocks >= 11 &&  screenblocks <= 16));
    }

    // Ammo totals: current ammo / max ammo
    for (i = 0 ; i < 4 ; i++)
    {
        STlib_updateNum(wider_stbar ? &w_ammo_wide[i] : &w_ammo[i], refresh);
        STlib_updateNum(wider_stbar ? &w_maxammo_wide[i] : &w_maxammo[i], refresh);
    }

    // Press Beta: some special routine. I need to draw Artifacts widet
    // while not in automap and Arms widget while in automap. Plus, background 
    // must be redrawn immediately. Also see AM_Stop at am_map.c.
    if (gamemode == pressbeta)
    {
        if (!automapactive || automap_overlay)
        {
            // [JN] Draw Artifacts widet
            dp_translation = ST_WidgetColor(hudcolor_artifacts);
            STlib_updateNum(wider_stbar ? &w_artifacts_wide : &w_artifacts, 
                            refresh || (screenblocks >= 11 &&  screenblocks <= 16));
            dp_translation = NULL;
        }
        else
        {
            // [JN] Draw Arms widet. Background (w_armsbg) and numbers (w_arms)
            STlib_updateBinIcon(wider_stbar ? &w_armsbg_wide : &w_armsbg,
                                refresh || (screenblocks >= 11 && screenblocks <= 16));

            for (i=0;i<6;i++)
            {
            STlib_updateMultIcon(wider_stbar ? &w_arms_wide[i] : &w_arms[i],
                                 refresh || (screenblocks >= 11 &&  screenblocks <= 16));
            }
        }

        // [JN] Amount of player lives.
        if (screenblocks <= 11 || screenblocks == 14 || (automapactive && !automap_overlay))
        {
            STlib_updateNum(&w_lifes, refresh);
        }
    }
}

// -----------------------------------------------------------------------------
// ST_doRefresh
// -----------------------------------------------------------------------------

void ST_doRefresh (void)
{
    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);
}

// -----------------------------------------------------------------------------
// ST_diffDraw
// -----------------------------------------------------------------------------

static void ST_diffDraw (void)
{
    // update all widgets
    ST_drawWidgets(false);
}

// -----------------------------------------------------------------------------
// ST_Drawer
// -----------------------------------------------------------------------------

void ST_Drawer (boolean fullscreen, boolean refresh)
{
    // [JN] Redraw whole status bar while in HELP screens.
    // Fixes a notable delay of HUD redraw after closing HELP screen.
    st_statusbaron = (!fullscreen) || (automapactive && !automap_overlay) || screenblocks == 11 || screenblocks == 12;
    st_firsttime = st_firsttime || refresh || inhelpscreens || (automapactive && !automap_overlay);

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // If just after ST_Start(), refresh all
    // [JN] Also do a full refresh if disk icon was drawn.
    if (st_firsttime || disk_drawn)
    {
        ST_doRefresh();
        disk_drawn = false;
    }
    // Otherwise, update as little as possible
    else
    {
        ST_diffDraw();
    }
}

// -----------------------------------------------------------------------------
// ST_loadUnloadGraphics
// Iterates through all graphics to be loaded or unloaded, along with
// the variable they use, invoking the specified callback function.
// -----------------------------------------------------------------------------

static void ST_loadUnloadGraphics (load_callback_t callback)
{
    int   i, j;
    int   facenum;
    char  namebuf[9];

    // Load the numbers, tall and short
    for (i = 0 ; i < 10 ; i++)
    {
        DEH_snprintf(namebuf, 9, "STTNUM%d", i);
        callback(namebuf, &tallnum[i]);

        DEH_snprintf(namebuf, 9, "STYSNUM%d", i);
        callback(namebuf, &shortnum[i]);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?

    callback(DEH_String("STTPRCNT"), &tallpercent);

    // key cards
    for (i = 0 ; i < NUMCARDS+3 ; i++)  //jff 2/23/98 show both keys too
    {
        DEH_snprintf(namebuf, 9, "STKEYS%d", i);
        callback(namebuf, &keys[i]);
    }

    // arms background
    callback(DEH_String("STARMS"), &armsbg);
    callback(DEH_String("RDARMS"), &armsbg_rus);

    // arms ownership widgets
    for (i = 0; i < 6; i++)
    {
        DEH_snprintf(namebuf, 9, "STGNUM%d", i+2);

        // gray #
        callback(namebuf, &arms[i][0]);

        // yellow #
        arms[i][1] = shortnum[i+2]; 
    }

    // face backgrounds for different color players
    DEH_snprintf(namebuf, 9, "STFB%d", consoleplayer);
    callback(namebuf, &faceback);

    // status bar background bits
    callback(DEH_String("STBAR"), &sbar);
    callback(DEH_String("RDSTBAR"), &sbar_rus);

    // face states
    facenum = 0;
    for (i = 0; i < ST_NUMPAINFACES; i++)
    {
        for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
        {
            DEH_snprintf(namebuf, 9, "STFST%d%d", i, j);
            callback(namebuf, &faces[facenum]);
            ++facenum;
        }

        DEH_snprintf(namebuf, 9, "STFTR%d0", i);	// turn right
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFTL%d0", i);	// turn left
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFOUCH%d", i);	// ouch!
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFEVL%d", i);	// evil grin ;)
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFKILL%d", i);	// pissed off
        callback(namebuf, &faces[facenum]);
        ++facenum;
    }

    callback(DEH_String("STFGOD0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFDEAD0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL1"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL2"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL3"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL4"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL5"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFCRSH0"), &faces[facenum]);
    ++facenum;

    // [JN] Удвоение массива спрайтов лиц, необходимое для бессмертия.
    if (!old_godface)
    {
        for (i = 0; i < ST_NUMPAINFACES; i++)
        {
            for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
            {
                M_snprintf(namebuf, 9, "STFST%i%iG", i, j);
                callback(namebuf, &faces[facenum++]);
            }

            M_snprintf(namebuf, 9, "STFTR%i0G", i);          // turn right
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFTL%i0G", i);          // turn left
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFOUC%iG", i);         // ouch!
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFEVL%iG", i);          // evil grin ;)
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFKIL%iG", i);         // pissed off
            callback(namebuf, &faces[facenum++]);
        }

        callback("STFGOD0G", &faces[facenum++]);
        callback("STFDEA0G", &faces[facenum++]);
        callback("STFEXP0G", &faces[facenum++]);
        callback("STFEXP1G", &faces[facenum++]);
        callback("STFEXP2G", &faces[facenum++]);
        callback("STFEXP3G", &faces[facenum++]);
        callback("STFEXP4G", &faces[facenum++]);
        callback("STFEXP5G", &faces[facenum++]);
        callback("STFCRS0G", &faces[facenum++]);
    }
}

// -----------------------------------------------------------------------------
// ST_loadCallback
// -----------------------------------------------------------------------------

static void ST_loadCallback (char *lumpname, patch_t **variable)
{
    *variable = W_CacheLumpName(lumpname, PU_STATIC);
}

// -----------------------------------------------------------------------------
// ST_loadGraphics
// -----------------------------------------------------------------------------

static void ST_loadGraphics (void)
{
    ST_loadUnloadGraphics(ST_loadCallback);
}

// -----------------------------------------------------------------------------
// ST_loadData
// -----------------------------------------------------------------------------

static void ST_loadData (void)
{
    lu_palette = W_GetNumForName (DEH_String("PLAYPAL"));

    ST_loadGraphics();
}

// -----------------------------------------------------------------------------
// ST_initData
// -----------------------------------------------------------------------------

static void ST_initData (void)
{
    int	    i;

    st_firsttime = true;
    plyr = &players[consoleplayer];

    st_chatstate = StartChatState;
    st_gamestate = FirstPersonState;

    // [JN] Initialize STBAR horizontal offset with zero for centering.
    sbar->leftoffset = 0;
    sbar_rus->leftoffset = 0;

    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_cursoron = false;

    faceindex = 0; // [crispy] fix status bar face hysteresis across level changes
    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i = 0 ; i < NUMWEAPONS ; i++)
    {
        oldweaponsowned[i] = plyr->weaponowned[i];
    }

    for (i = 0 ; i < 3 ; i++)
    {
        keyboxes[i] = -1;
    }

    STlib_init();
}

// -----------------------------------------------------------------------------
// ST_DrawDemoTimer
// [crispy] Demo Timer widget
// -----------------------------------------------------------------------------

void ST_DrawDemoTimer (const int time)
{
    const boolean wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9);
    const int mins = time / (60 * TICRATE);
    const float secs = (float)(time % (60 * TICRATE)) / TICRATE;
    char n[16];

    sprintf(n, "%02i:%05.02f", mins, secs);

    RD_M_DrawTextC(n, 278 + (wide_4_3 ? wide_delta : wide_delta*2), (viewwindowy >> hires) + 1);
}

// -----------------------------------------------------------------------------
// ST_createWidgets
// -----------------------------------------------------------------------------

void ST_createWidgets (void)
{
    int i;

    // ready weapon ammo
    STlib_initNum(&w_ready, ST_AMMOX + wide_delta, ST_AMMOY,
                  tallnum, &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                  &st_statusbaron, ST_AMMOWIDTH);

    STlib_initNum(&w_ready_wide, ST_AMMOX, ST_AMMOY,
                  tallnum, &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                  &st_statusbaron, ST_AMMOWIDTH );

    // the last weapon type
    w_ready.data = plyr->readyweapon; 
    w_ready_wide.data = plyr->readyweapon; 

    // health percentage
    STlib_initPercent(&w_health, ST_HEALTHX + wide_delta, ST_HEALTHY,
                      tallnum, &plyr->health, &st_statusbaron, tallpercent);

    STlib_initPercent(&w_health_wide, ST_HEALTHX, ST_HEALTHY,
                      tallnum, &plyr->health, &st_statusbaron, tallpercent);

    // [JN] Negative player health
    STlib_initPercent(&w_health_neg, ST_HEALTHX + wide_delta, ST_HEALTHY,
                      tallnum, &plyr->health_neg, &st_statusbaron, tallpercent);

    STlib_initPercent(&w_health_neg_wide, ST_HEALTHX, ST_HEALTHY,
                      tallnum, &plyr->health_neg, &st_statusbaron, tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg, ST_ARMSBGX + wide_delta, ST_ARMSBGY,
                      english_language ? armsbg : armsbg_rus,
                      &st_notdeathmatch, &st_statusbaron);

    STlib_initBinIcon(&w_armsbg_wide, ST_ARMSBGX, ST_ARMSBGY,
                      english_language ? armsbg : armsbg_rus,
                      &st_notdeathmatch, &st_statusbaron);

    // weapons owned
    for(i = 0 ; i < 6 ; i++)
    {
        STlib_initMultIcon(&w_arms[i], ST_ARMSX + wide_delta + (i%3)*ST_ARMSXSPACE, 
                           ST_ARMSY+(i/3)*ST_ARMSYSPACE, arms[i],
                           &plyr->weaponowned[i+1], &st_armson);

        STlib_initMultIcon(&w_arms_wide[i], ST_ARMSX + (i%3)*ST_ARMSXSPACE, 
                           ST_ARMSY+(i/3)*ST_ARMSYSPACE, arms[i],
                           &plyr->weaponowned[i+1], &st_armson);
    }

    // [DOOM Retro] & [crispy] show SSG availability in the Shotgun slot of the arms widget
    w_arms[1].inum = &st_shotguns;
    w_arms_wide[1].inum = &st_shotguns;

    // frags sum
    STlib_initNum(&w_frags, ST_FRAGSX + wide_delta, ST_FRAGSY,
                  tallnum, &st_fragscount, &st_fragson, ST_FRAGSWIDTH);
    STlib_initNum(&w_frags_wide, ST_FRAGSX, ST_FRAGSY,
                  tallnum, &st_fragscount, &st_fragson, ST_FRAGSWIDTH);

    // [JN] Press Beta artifacts sum
    STlib_initNum(&w_artifacts, ST_FRAGSX + wide_delta + 1, ST_FRAGSY,
                  tallnum, &st_artifactscount, &st_artifactson, ST_FRAGSWIDTH);
    STlib_initNum(&w_artifacts_wide, ST_FRAGSX, ST_FRAGSY,
                  tallnum, &st_artifactscount, &st_artifactson, ST_FRAGSWIDTH);


    // [JN] Press Beta: player's lifes sum
    STlib_initNum(&w_lifes, ST_LIFESX + wide_delta, ST_LIFESY,
                  shortnum, &lifecount, &st_statusbaron, ST_LIFESWIDTH);

    // faces
    STlib_initMultIcon(&w_faces, ST_FACESX + wide_delta, ST_FACESY,
                       faces, &st_faceindex, &st_statusbaron);

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor, ST_ARMORX + wide_delta, ST_ARMORY,
                      tallnum, &plyr->armorpoints, &st_statusbaron, tallpercent);

    STlib_initPercent(&w_armor_wide, ST_ARMORX + wide_delta*2, ST_ARMORY,
                      tallnum, &plyr->armorpoints, &st_statusbaron, tallpercent);

    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0], ST_KEY0X + wide_delta, ST_KEY0Y,
                       keys, &keyboxes[0], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes_wide[0], ST_KEY0X + wide_delta*2, ST_KEY0Y,
                       keys, &keyboxes[0], &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[1], ST_KEY1X + wide_delta, ST_KEY1Y,
                       keys, &keyboxes[1], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes_wide[1], ST_KEY1X + wide_delta*2, ST_KEY1Y,
                       keys, &keyboxes[1], &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[2], ST_KEY2X + wide_delta, ST_KEY2Y,
                       keys, &keyboxes[2], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes_wide[2], ST_KEY2X + wide_delta*2, ST_KEY2Y,
                       keys, &keyboxes[2], &st_statusbaron);

    // ammo count (all four kinds)
    STlib_initNum(&w_ammo[0], ST_AMMO0X + wide_delta, ST_AMMO0Y,
                  shortnum, &plyr->ammo[0], &st_statusbaron, ST_AMMO0WIDTH);
    STlib_initNum(&w_ammo_wide[0], ST_AMMO0X + wide_delta*2, ST_AMMO0Y,
                  shortnum, &plyr->ammo[0], &st_statusbaron, ST_AMMO0WIDTH);

    STlib_initNum(&w_ammo[1], ST_AMMO1X + wide_delta, ST_AMMO1Y,
                  shortnum, &plyr->ammo[1], &st_statusbaron, ST_AMMO1WIDTH);
    STlib_initNum(&w_ammo_wide[1], ST_AMMO1X + wide_delta*2, ST_AMMO1Y,
                  shortnum, &plyr->ammo[1], &st_statusbaron, ST_AMMO1WIDTH);

    STlib_initNum(&w_ammo[2], ST_AMMO2X + wide_delta, ST_AMMO2Y,
                  shortnum, &plyr->ammo[2], &st_statusbaron, ST_AMMO2WIDTH);
    STlib_initNum(&w_ammo_wide[2], ST_AMMO2X + wide_delta*2, ST_AMMO2Y,
                  shortnum, &plyr->ammo[2], &st_statusbaron, ST_AMMO2WIDTH);

    STlib_initNum(&w_ammo[3], ST_AMMO3X + wide_delta, ST_AMMO3Y,
                  shortnum, &plyr->ammo[3], &st_statusbaron, ST_AMMO3WIDTH);
    STlib_initNum(&w_ammo_wide[3], ST_AMMO3X + wide_delta*2, ST_AMMO3Y,
                  shortnum, &plyr->ammo[3], &st_statusbaron, ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_initNum(&w_maxammo[0], ST_MAXAMMO0X + wide_delta, ST_MAXAMMO0Y,
                  shortnum, &plyr->maxammo[0], &st_statusbaron, ST_MAXAMMO0WIDTH);
    STlib_initNum(&w_maxammo_wide[0], ST_MAXAMMO0X + wide_delta*2, ST_MAXAMMO0Y,
                  shortnum, &plyr->maxammo[0], &st_statusbaron, ST_MAXAMMO0WIDTH);

    STlib_initNum(&w_maxammo[1], ST_MAXAMMO1X + wide_delta, ST_MAXAMMO1Y,
                  shortnum, &plyr->maxammo[1], &st_statusbaron, ST_MAXAMMO1WIDTH);
    STlib_initNum(&w_maxammo_wide[1], ST_MAXAMMO1X + wide_delta*2, ST_MAXAMMO1Y,
                  shortnum, &plyr->maxammo[1], &st_statusbaron, ST_MAXAMMO1WIDTH);

    STlib_initNum(&w_maxammo[2], ST_MAXAMMO2X + wide_delta, ST_MAXAMMO2Y,
                  shortnum, &plyr->maxammo[2], &st_statusbaron, ST_MAXAMMO2WIDTH);
    STlib_initNum(&w_maxammo_wide[2], ST_MAXAMMO2X + wide_delta*2, ST_MAXAMMO2Y,
                  shortnum, &plyr->maxammo[2], &st_statusbaron, ST_MAXAMMO2WIDTH);

    STlib_initNum(&w_maxammo[3], ST_MAXAMMO3X + wide_delta, ST_MAXAMMO3Y,
                  shortnum, &plyr->maxammo[3], &st_statusbaron, ST_MAXAMMO3WIDTH);
    STlib_initNum(&w_maxammo_wide[3], ST_MAXAMMO3X + wide_delta*2, ST_MAXAMMO3Y,
                  shortnum, &plyr->maxammo[3], &st_statusbaron, ST_MAXAMMO3WIDTH);
}

// -----------------------------------------------------------------------------
// ST_Start
// -----------------------------------------------------------------------------

void ST_Start (void)
{
    if (!st_stopped)
    {
        ST_Stop();
    }

    ST_initData();

    if (gamemission == jaguar)
    {
        ST_createWidgetsJaguar();
    }
    else
    {
        ST_createWidgets();
    }

    st_stopped = false;
}

// -----------------------------------------------------------------------------
// ST_Stop
// -----------------------------------------------------------------------------

void ST_Stop (void)
{
    if (st_stopped)
    {
        return;
    }

    I_SetPalette (W_CacheLumpNum ((lu_palette), PU_CACHE));

    st_stopped = true;
}

// -----------------------------------------------------------------------------
// ST_Init
// -----------------------------------------------------------------------------

void ST_Init (void)
{
    ST_loadData();

    // [JN] Initialize status bar height.
    // In Jaguar Doom it's 40 pixels tall instead of 32.
    st_height = gamemission == jaguar ? ST_HEIGHT_JAG : ST_HEIGHT;
    st_y = gamemission == jaguar ? ST_Y_JAG : ST_Y;

    st_backing_screen = (byte *)Z_Malloc((screenwidth << hires) 
                                       * (st_height << hires)
                                       * sizeof(*st_backing_screen), PU_STATIC, 0);

    // [JN] Initialize status bar widget colors.
    M_RD_Define_SBarColorValue(&sbar_color_high_set, sbar_color_high);
    M_RD_Define_SBarColorValue(&sbar_color_normal_set, sbar_color_normal);
    M_RD_Define_SBarColorValue(&sbar_color_low_set, sbar_color_low);
    M_RD_Define_SBarColorValue(&sbar_color_critical_set, sbar_color_critical);
    M_RD_Define_SBarColorValue(&sbar_color_armor_1_set, sbar_color_armor_1);
    M_RD_Define_SBarColorValue(&sbar_color_armor_2_set, sbar_color_armor_2);
    M_RD_Define_SBarColorValue(&sbar_color_armor_0_set, sbar_color_armor_0);
}


// =============================================================================
//
// [JN] Jaguar Doom code
//
// =============================================================================

// -----------------------------------------------------------------------------
// [JN] ST_drawWidgetsJaguar
// Widget drawing routines for Jaguar, from left to right.
// -----------------------------------------------------------------------------

void ST_drawWidgetsJaguar (boolean refresh)
{
    int i;
    const boolean wider_stbar = aspect_ratio >= 2 && screenblocks >= 14 && (!automapactive || automap_overlay);
    st_armson = st_statusbaron;
    st_neghealth = negative_health && plyr->health <= 0 && !vanillaparm;

    // Labels ("AMMO", "HEALTH", "ARMOR", "ARMS", "AREA").
    // Don't draw signs in automap.
    if ((screenblocks == 11 || screenblocks == 12 || screenblocks == 14 || screenblocks == 15)
    && (!automapactive || automap_overlay))
    {
        // Don't draw ammo for fist and chainsaw
        if (plyr->readyweapon != wp_fist && plyr->readyweapon != wp_chainsaw)
        {
            V_DrawPatch((wider_stbar ? 0 : wide_delta), 0, W_CacheLumpName
                    (DEH_String(english_language ? "STCHAMMO" : "RDCHAMMO"), PU_CACHE));
        }

        // Ammo
        V_DrawPatch((wider_stbar ? 0 : wide_delta), 0, W_CacheLumpName
                   (DEH_String(english_language ? "STCHHLTH" : "RDCHHLTH"), PU_CACHE));

        // Armor, Arms, Area
        V_DrawPatch((wider_stbar ? wide_delta*2 : wide_delta), 0, W_CacheLumpName
                   (DEH_String(english_language ? "STCHARAM" : "RDCHARAM"), PU_CACHE));
    }

    // Current weapon ammo
    STlib_updateNum(wider_stbar ? &w_ready_wide : &w_ready, refresh);

    // [crispy] draw berserk pack instead of no ammo if appropriate
    if ((screenblocks >= 11 && screenblocks <= 16)
    && (!automapactive || automap_overlay))
    {
        if (plyr->readyweapon == wp_fist && plyr->powers[pw_strength])
        {
            static int lump;
            patch_t *patch;

            lump = W_CheckNumForName(DEH_String("PSTRA0"));
            patch = W_CacheLumpNum(lump, PU_CACHE);

            // [crispy] (23,179) is the center of the Ammo widget
            V_DrawPatch((23 - SHORT(patch->width)/2 + SHORT(patch->leftoffset))+ (wider_stbar ? 0 : wide_delta),
                         179 - SHORT(patch->height)/2 + SHORT(patch->topoffset),
                         patch);
        }
    }

    // Health
    if (wider_stbar)
    {
        STlib_updatePercent(st_neghealth ? &w_health_neg_wide : &w_health_wide,
                            refresh || screenblocks <= 16);
    }
    else
    {
        STlib_updatePercent(st_neghealth ? &w_health_neg : &w_health,
                            refresh || screenblocks <= 16);
    }

    // Keys
    for (i = 0 ; i < 3 ; i++)
    {
        STlib_updateMultIcon(wider_stbar ? &w_keyboxes_wide[i] : &w_keyboxes[i],
                             refresh || (screenblocks >= 11 &&  screenblocks <= 16));
    }

    // Player face background
    if ((screenblocks == 11 || screenblocks == 14) && (!automapactive || automap_overlay))
    {
        V_DrawPatch(0 + wide_delta, 0, W_CacheLumpName(DEH_String("STPBG"), PU_CACHE));
    }

    // Player faces
    if ((screenblocks < 12 || screenblocks == 14) || (automapactive && !automap_overlay))
    {
        STlib_updateMultIcon(&w_faces, refresh || screenblocks == 11 || screenblocks == 14);
    }

    // Armor
    STlib_updatePercent(wider_stbar ? &w_armor_wide : &w_armor,
                        refresh || screenblocks <= 16);

    // ARMS
    for (i = 0 ; i < 6 ; i++)
    {
        STlib_updateMultIcon(wider_stbar ? &w_arms_wide[i] : &w_arms[i],
                             refresh || (screenblocks >= 11 &&  screenblocks <= 16));
    }

    // Current map
    STlib_updateNum(wider_stbar ? &w_currentmap_wide : &w_currentmap, refresh);
}


// -----------------------------------------------------------------------------
// [JN] ST_createWidgetsJaguar
// Widget creating routines for Jaguar
// -----------------------------------------------------------------------------

void ST_createWidgetsJaguar (void)
{
    int i;

    // the last weapon type
    w_ready.data = plyr->readyweapon; 

    // Ammo
    STlib_initNum(&w_ready, 51 + wide_delta, 174, tallnum, 
                  &plyr->ammo[weaponinfo[plyr->readyweapon].ammo], &st_statusbaron, 3);

    STlib_initNum(&w_ready_wide, 51, 174, tallnum, 
                  &plyr->ammo[weaponinfo[plyr->readyweapon].ammo], &st_statusbaron, 3);

    // Health
    STlib_initPercent(&w_health, 104 + wide_delta, 174, tallnum,
                      &plyr->health, &st_statusbaron, tallpercent);
    STlib_initPercent(&w_health_wide, 104, 174, tallnum,
                      &plyr->health, &st_statusbaron, tallpercent);

    // Negative health
    STlib_initPercent(&w_health_neg, 104 + wide_delta, 174, tallnum,
                      &plyr->health_neg, &st_statusbaron, tallpercent);
    STlib_initPercent(&w_health_neg_wide, 104, 174, tallnum,
                      &plyr->health_neg, &st_statusbaron, tallpercent);

    // keyboxes 0-2
    // Blue
    STlib_initMultIcon(&w_keyboxes[0], 124 + wide_delta, 175, keys,
                       &keyboxes[0], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes_wide[0], 124, 175, keys,
                       &keyboxes[0], &st_statusbaron);

    // Yellow
    STlib_initMultIcon(&w_keyboxes[1], 124 + wide_delta, 187, keys,
                       &keyboxes[1], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes_wide[1], 124, 187, keys,
                       &keyboxes[1], &st_statusbaron);

    // Red
    STlib_initMultIcon(&w_keyboxes[2], 124 + wide_delta, 163, keys,
                       &keyboxes[2], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes_wide[2], 124, 163, keys,
                       &keyboxes[2], &st_statusbaron);

    // Faces
    STlib_initMultIcon(&w_faces, 143 + wide_delta, 166, faces,
                       &st_faceindex, &st_statusbaron);

    // Armor
    STlib_initPercent(&w_armor, 225 + wide_delta, 174, tallnum,
                      &plyr->armorpoints, &st_statusbaron, tallpercent);

    STlib_initPercent(&w_armor_wide, 225 + wide_delta*2, 174, tallnum,
                      &plyr->armorpoints, &st_statusbaron, tallpercent);

    // Weapons owned
    for(i = 0 ; i < 6 ; i++)
    {
        STlib_initMultIcon(&w_arms[i], wide_delta + 249+(i%3)*12, 175+(i/3)*10,
                           arms[i], &plyr->weaponowned[i+1], &st_armson);

        STlib_initMultIcon(&w_arms_wide[i], wide_delta*2 + 249+(i%3)*12, 175+(i/3)*10,
                           arms[i], &plyr->weaponowned[i+1], &st_armson);
    }

    // Area
    STlib_initNum(&w_currentmap, wide_delta + (gamemap >= 10 ? 317 : 309), 174,
                  tallnum, &gamemap, &st_statusbaron, gamemap >= 10 ? 2 : 1);

    STlib_initNum(&w_currentmap_wide, wide_delta*2 + (gamemap >= 10 ? 317 : 309), 174,
                  tallnum, &gamemap, &st_statusbaron, gamemap >= 10 ? 2 : 1);
}
