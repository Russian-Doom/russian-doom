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


#include "i_swap.h"
#include "z_zone.h"
#include "m_misc.h"
#include "w_wad.h"
#include "deh_main.h"
#include "deh_misc.h"
#include "g_game.h"
#include "st_bar.h"
#include "p_local.h"
#include "am_map.h"
#include "m_menu.h"
#include "s_sound.h"
#include "v_video.h"
#include "doomstat.h"
#include "id_lang.h"
#include "rd_text.h"
#include "v_trans.h"
#include "v_diskicon.h"
#include "jn.h"


// Palette indices. For damage/bonus red-/gold-shifts
#define STARTREDPALS        1
#define STARTBONUSPALS      9
#define NUMREDPALS          8
#define NUMBONUSPALS        4
// Radiation suit, green shift.
#define RADIATIONPAL        13
// [JN] Jaguar Doom: cyan invulnerability palette
#define INVULNERABILITYPAL  14

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

// Amount of delay for certain face states.
#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

// Amount of damage for ouch face.
#define ST_MUCHPAIN 20

// Dimensions given in characters.
#define ST_MSGWIDTH 52


extern boolean sgl_loaded;
extern boolean old_godface;  // [JN] If false, we can use an extra GOD faces.

// [JN] Pointer to a function for using different status bars.
static void (*ST_DrawElementsFunc) (boolean wide);


enum
{
    hudcolor_ammo,
    hudcolor_health,
    hudcolor_frags,
    hudcolor_armor,
    hudcolor_artifacts
} hudcolor_t;

//
// Data
//

static player_t *plyr;          // Main player in game.

// [JN] Different status bar heights between original Doom and Jaguar Doom.
int          st_height;
static int   st_y;

int          st_palette = 0;
static int   lu_palette;        // Lump number for PLAYPAL.
static int   st_randomnumber;   // A random number per tick.
static int   st_facecount = 0;  // Count until face changes.
static int   st_faceindex = 1;  // Current face index.
static int   faceindex;         // [crispy] fix status bar face hysteresis
static int   st_oldhealth = -1; // Used to use appopriately pained face.
static int   st_fragscount;     // Number of frags so far in deathmatch.
static byte *st_backing_screen;

static char    msg[ST_MSGWIDTH];             // Message given by cheat codes.
static boolean oldweaponsowned[NUMWEAPONS];  // Used for evil grin.

// [JN] status bar widget colors.
byte *stbar_color_high_set;
byte *stbar_color_normal_set;
byte *stbar_color_low_set;
byte *stbar_color_critical_set;
byte *stbar_color_armor_1_set;
byte *stbar_color_armor_2_set;
byte *stbar_color_armor_0_set;

//
// Graphics
//

// [JN] Bezel pattern to fill status bar in widescreen mode.
static byte    *bezel_pattern;

// Big font
static int      FontBNumBase;
static patch_t *FontBNumbers[10];
static patch_t *FontBPercent;
static patch_t *FontBMinus;

// Small yellow font
static int      FontSNumBaseY;
static patch_t *FontSNumbersY[10];
// Small gray font
static int      FontSNumBaseG;
static patch_t *FontSNumbersG[10];

// Face backgrounds for different color players
static patch_t *faceback;

// Player faces
static patch_t *faces[ST_NUMFACES * 2];

// Key cards and Skulls
// [JN] jff 2/23/98 show both keys too
static patch_t *keys[NUMCARDS+3];

// [crispy] blinking key or skull in the status bar
int st_keyorskull[3];


// =============================================================================
//
// CHEAT CODES
//
// =============================================================================

cheatseq_t cheat_mus               = CHEAT("idmus", 2);
cheatseq_t cheat_god               = CHEAT("iddqd", 0);
cheatseq_t cheat_ammo              = CHEAT("idkfa", 0);
cheatseq_t cheat_ammonokey         = CHEAT("idfa", 0);
cheatseq_t cheat_keys              = CHEAT("idka", 0);
cheatseq_t cheat_noclip            = CHEAT("idspispopd", 0);
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
cheatseq_t cheat_clev     = CHEAT("idclev", 2);
cheatseq_t cheat_mypos    = CHEAT("idmypos", 0);

// [crispy] pseudo cheats to eat up the first digit typed after a cheat expecting two parameters
static cheatseq_t cheat_mus1  = CHEAT("idmus", 1);
static cheatseq_t cheat_clev1 = CHEAT("idclev", 1);

// [crispy] new cheats
static cheatseq_t cheat_massacre = CHEAT("tntem", 0);

// [JN] Press Beta cheat codes
static cheatseq_t cheat_god_beta    = CHEAT("tst", 0); // iddqd
static cheatseq_t cheat_ammo_beta   = CHEAT("amo", 0); // idkfa
static cheatseq_t cheat_noclip_beta = CHEAT("nc", 0);  // idclip

// -----------------------------------------------------------------------------
// ST_cheat_massacre
// [crispy] adapted from boom202s/M_CHEAT.C:467-498
// -----------------------------------------------------------------------------

static const int ST_cheat_massacre (void)
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

            if (mo->flags & MF_COUNTKILL || mo->flags & MF_COUNTEXTRAKILL
            ||  mo->type == MT_SKULL)
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

const boolean ST_Responder (const event_t *ev)
{
    int i;

    // if a user keypress...
    if (ev->type == ev_keydown)
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
                    P_SetMessage(plyr, DEH_String(ststr_dqdon), msg_system, false);
                }
                else
                {
                    P_SetMessage(plyr, DEH_String(ststr_dqdoff), msg_system, false);
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

                P_SetMessage(plyr, DEH_String(ststr_faadded), msg_system, false);
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

                P_SetMessage(plyr, DEH_String(ststr_kfaadded), msg_system, false);
            }

            // [JN] 'ka' чит для выдачи ключей
            else if (cht_CheckCheat(&cheat_keys, ev->data2))
            {
                for (i = 0; i < NUMCARDS ; i++)
                {
                    plyr->cards[i] = true;
                }

                P_SetMessage(plyr, DEH_String(ststr_kaadded), msg_system, false);
            }

            // [crispy] implement Boom's "tntem" cheat
            else if (cht_CheckCheat(&cheat_massacre, ev->data2))
            {
                int killcount = ST_cheat_massacre();

                M_snprintf(msg, sizeof(msg), "%s %d", ststr_massacre, killcount);
                P_SetMessage(plyr, msg, msg_system, false);
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus, ev->data2))
            {
                char buf[3];
                int  musnum;

                P_SetMessage(plyr, DEH_String(ststr_mus), msg_system, false);
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
                        P_SetMessage(plyr, DEH_String(ststr_nomus), msg_system, false);
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
                        P_SetMessage(plyr, DEH_String(ststr_nomus), msg_system, false);
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

                P_SetMessage(plyr, DEH_String(plyr->cheats & CF_NOCLIP ?
                             ststr_ncon : ststr_ncoff), msg_system, false);
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
                        P_SetMessage(plyr, DEH_String(ststr_beholdx), msg_system, false);
                    }
                    else if (i!=pw_strength && i!=pw_allmap) // [crispy] disable full Automap
                    {
                        plyr->powers[i] = 1;
                        P_SetMessage(plyr, DEH_String(ststr_beholdz), msg_system, false);
                    }
                    else
                    {
                        plyr->powers[i] = 0;
                        P_SetMessage(plyr, DEH_String(ststr_beholdz), msg_system, false);
                    }
                }
            }

            // 'behold' power-up menu
            if (cht_CheckCheat(&cheat_powerup[6], ev->data2))
            {
                P_SetMessage(plyr, DEH_String(ststr_behold), msg_system, false);
            }

            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data2))
            {
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->powers[pw_invulnerability] = true;
                P_SetMessage(plyr, DEH_String(ststr_choppers), msg_system, false);
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
                P_SetMessage(plyr, buf, msg_system, true);
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
            P_SetMessage(plyr, DEH_String(ststr_clev), msg_system, false);
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
                    P_SetMessage(plyr, DEH_String(ststr_dqdon), msg_system, false);
                }
                else
                {
                    P_SetMessage(plyr, DEH_String(ststr_dqdoff), msg_system, false);
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

                P_SetMessage(plyr, DEH_String(ststr_kfaadded), msg_system, false);
            }

            // 'NC' - noclipping mode
            else if (cht_CheckCheat(&cheat_noclip_beta, ev->data2))
            {	
                plyr->cheats ^= CF_NOCLIP;

                P_SetMessage(plyr, DEH_String(plyr->cheats & CF_NOCLIP ?
                             ststr_ncon : ststr_ncoff), msg_system, false);
            }
        }
    }

    return false;
}


// =============================================================================
//
// STATUS BAR
//
// =============================================================================

// -----------------------------------------------------------------------------
// ST_DrawBackground
// [JN] Draws standard status bar background patch and fills side borders.
// -----------------------------------------------------------------------------

static void ST_DrawBackground (void)
{
    V_UseBuffer(st_backing_screen);
    
    // Draw side screen borders in wide screen mode.
    if (aspect_ratio >= 2
    && (screenblocks == 10 || (automapactive && !automap_overlay)))
    {
        // [crispy] this is our own local copy of R_FillBackScreen() to
        // fill the entire background of st_backing_screen with the bezel pattern,
        // so it appears to the left and right of the status bar in widescreen mode
        int x, y;
        byte *dest = st_backing_screen;
        patch_t *patch = W_CacheLumpName(DEH_String("brdr_b"), PU_CACHE);
        const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;

        // [JN] Variable HUD detail level.
        for (y = SCREENHEIGHT-(st_height << hires); y < SCREENHEIGHT; y++)
        {
            for (x = 0; x < screenwidth; x++)
            {
                *dest++ = bezel_pattern[((( y >> shift_allowed) & 63) << 6) 
                                       + (( x >> shift_allowed) & 63)];
            }
        }

        // [JN] Draw bezel bottom edge.
        for (x = 0; x < screenwidth; x += 8)
        {
            if (hud_detaillevel)
            {
                V_DrawPatch(x, 0, patch, NULL);
            }
            else
            {
                V_DrawPatchUnscaled(x, 0, patch, NULL);
            }
        }
    }

    // Always draw status bar on the center of the screen.
    V_DrawPatch((ORIGWIDTH - SHORT(stbar->width)) / 2 + wide_delta, 0, stbar, NULL);

    V_RestoreBuffer();
    V_CopyRect(0, 0, st_backing_screen, origwidth, st_height, 0, st_y);

    // Arms background.
    if (!deathmatch && gamemode != pressbeta)
    {
        V_DrawPatch(104 + wide_delta, 168, starms, NULL);
    }
}

// -----------------------------------------------------------------------------
// ST_calcPainOffset
// -----------------------------------------------------------------------------

static const int ST_calcPainOffset (void)
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

static void ST_updateFaceWidget (void)
{
    int         i;
    int         painoffset = ST_calcPainOffset();
    static int  lastattackdown = -1;
    static int  priority = 0;
    angle_t     badguyangle;
    angle_t     diffang;
    boolean     doevilgrin;

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
    {
        //if (gamemode != pressbeta)
        {
            if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
            {
                // being attacked
                priority = 7;

                // [crispy] show "Ouch Face" as intended
                if (st_oldhealth - plyr->health > ST_MUCHPAIN)
                {
                    // [crispy] raise "Ouch Face" priority
                    priority = 8;
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
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

    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (plyr->damagecount)
        {
            // [crispy] show "Ouch Face" as intended
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
        faceindex = st_randomnumber;
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
// ST_UpdateFragsCounter
// -----------------------------------------------------------------------------

static void ST_UpdateFragsCounter (void)
{
    st_fragscount = 0;

    for (int i = 0 ; i < MAXPLAYERS ; i++)
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
    
    // [JN] Prevent overflow, ST_DrawBigNumber can only draw three 
    // digit number, and status bar fits well only two digits number
    if (st_fragscount > 99)
        st_fragscount = 99;
    if (st_fragscount < -99)
        st_fragscount = -99;
}

// -----------------------------------------------------------------------------
// ST_BackgroundDrawer
// [JN] Consolidated function of drawing status bar background and elements.
// Called once (in fact) in ST_Drawer while wiping to draw beneath effect, 
// and constantly in ST_Ticker to make drawing independent from framerate.
// -----------------------------------------------------------------------------

void ST_BackgroundDrawer (void)
{
    if (screenblocks <= 10 || (automapactive && !automap_overlay))
    {
        ST_DrawBackground();
        ST_DrawElementsFunc(false);
    }
}

// -----------------------------------------------------------------------------
// ST_Ticker
// -----------------------------------------------------------------------------

void ST_Ticker (void)
{
    // [JN] Use real random number generator
    // instead of M_Random LUT for faces stide.
    st_randomnumber = rand() % 3;

    ST_updateFaceWidget();

    // [JN] Update blinking key or skull timer.
    for (int i = 0 ; i < 3 ; i++)
    {
        if (plyr->tryopen[i])
        {
            plyr->tryopen[i]--;
        }
    }

    if (deathmatch)
    {
        ST_UpdateFragsCounter();
    }
    
    st_oldhealth = plyr->health;

    // [JN] Once wipe effect done, draw standard 
    // status bar independently from framerate.
    if (wipegamestate == GS_LEVEL)
    {
        ST_BackgroundDrawer();
    }
}

// -----------------------------------------------------------------------------
// ST_DoPaletteStuff
// -----------------------------------------------------------------------------

static void ST_DoPaletteStuff (void)
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

static byte *ST_WidgetColor (const int i)
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
                    return stbar_color_critical_set;
                else if (ammo < fullammo/2)
                    return stbar_color_low_set;
                else if (ammo <= fullammo)
                    return stbar_color_normal_set;
                else
                    return stbar_color_high_set;
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
                return stbar_color_high_set;
            else if (health >= 67)
                return stbar_color_normal_set;
            else if (health >= 34)
                return stbar_color_low_set;            
            else
                return stbar_color_critical_set;
            break;
        }
        case hudcolor_frags:
        {
            int frags = st_fragscount;

            if (frags < 0)
                return stbar_color_critical_set;
            else if (frags == 0)
                return stbar_color_low_set;
            else
                return stbar_color_normal_set;

            break;
        }
        case hudcolor_armor:
        {
	    // [crispy] Invulnerability powerup and God Mode cheat turn Armor values gray
	    if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
                return cr[CR_WHITE];
	    // [crispy] color by armor type
	    else if (plyr->armortype >= 2)
                return stbar_color_armor_2_set;
	    else if (plyr->armortype == 1)
                return stbar_color_armor_1_set;
	    else if (plyr->armortype == 0)
                return stbar_color_armor_0_set;
            break;
        }
        case hudcolor_artifacts:
        {
            // [JN] Press Beta: colorize artifacts widget.
            // There is no internal counter for amount of collected artifacts,
            // so the values are hardcoded (same on intermission screen).
            // Also, there is only one map per each episode, which making
            // definition of 'gamemap' unnecessary.
            if ((gameepisode == 1 && artifactcount == 36)
            ||  (gameepisode == 2 && artifactcount == 29)
            ||  (gameepisode == 3 && artifactcount == 25))
            return stbar_color_normal_set;

            else if (artifactcount > 0)
            return stbar_color_low_set;

            else
            return stbar_color_critical_set;

            break;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------------
// ST_DrawBigNumber
// [JN] Draws a three digit big red number using STTNUM* graphics.
// -----------------------------------------------------------------------------

static void ST_DrawBigNumber (int val, const int x, const int y, byte *table)
{
    int oldval = val;
    int xpos = x;

    dp_translation = table;

    // [JN] Support for negative values.
    if (val < 0)
    {
        val = -val;
        
        if (-val <= -99)
        {
            val = 99;
        }

        // [JN] Draw minus symbol with respection of digits placement.
        // However, values below -10 requires some correction in "x" placement.
        V_DrawPatch(xpos + (val <= 9 ? 20 : 5) - 4, y, FontBMinus, NULL);
    }

    if (val > 99)
    {
        V_DrawPatch(xpos - 4, y, FontBNumbers[val / 100], NULL);
    }

    val = val % 100;
    xpos += 14;

    if (val > 9 || oldval > 99)
    {
        V_DrawPatch(xpos - 4, y, FontBNumbers[val / 10], NULL);
    }

    val = val % 10;
    xpos += 14;

    V_DrawPatch(xpos - 4, y, FontBNumbers[val], NULL);
    
    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// ST_DrawPercent
// [JN] Draws big red percent sign.
// -----------------------------------------------------------------------------

static void ST_DrawPercent (const int x, const int y, byte *table)
{
    dp_translation = table;
    V_DrawPatch(x, y, FontBPercent, NULL);
    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// ST_DrawSmallNumberY
// [JN] Draws a three digit yellow number using STYSNUM* graphics.
// -----------------------------------------------------------------------------

static void ST_DrawSmallNumberY (int val, const int x, const int y)
{
    int oldval = val;
    int xpos = x;

    if (val < 0)
    {
        val = 0;
    }

    if (val > 99)
    {
        V_DrawPatch(xpos - 4, y, FontSNumbersY[val / 100], NULL);
    }

    val = val % 100;
    xpos += 4;

    if (val > 9 || oldval > 99)
    {
        V_DrawPatch(xpos - 4, y, FontSNumbersY[val / 10], NULL);
    }

    val = val % 10;
    xpos += 4;

    V_DrawPatch(xpos - 4, y, FontSNumbersY[val], NULL);
}

// -----------------------------------------------------------------------------
// ST_DrawSmallNumberG
// [JN] Draws a one digit gray number using STGNUM* graphics.
// -----------------------------------------------------------------------------

static void ST_DrawSmallNumberG (int val, const int x, const int y)
{
    V_DrawPatch(x + 4, y, FontSNumbersG[val], NULL);
}

// -----------------------------------------------------------------------------
// ST_DrawElements
// [JN] Draw various digit values, faces and keys.
// Wide boolean stand for wider status bar.
// -----------------------------------------------------------------------------

static void ST_DrawElements (const boolean wide)
{
    int left_delta;
    int right_delta;
    
    if (wide)
    {
        left_delta = screenblocks <= 13 ? wide_delta : 0;
        right_delta = screenblocks <= 13 ? wide_delta : wide_delta*2;
    }
    else
    {
        left_delta = wide_delta;
        right_delta = wide_delta;
    }

    // Transparent signs
    if ((screenblocks == 11 || screenblocks == 12
    ||   screenblocks == 14 || screenblocks == 15)
    && (!automapactive || automap_overlay))
    
    {
        if (weaponinfo[plyr->readyweapon].ammo != am_noammo)
        {
            V_DrawPatch(left_delta + 2, 191, stchammo, NULL);
        }
        V_DrawPatch(left_delta + 52, 173, stchhlth, NULL);
        V_DrawPatch(left_delta + 108, 191, stcharms, NULL);
        V_DrawPatch(right_delta + 52, 173, stcharam, NULL);
        V_DrawPatch(right_delta + 292, 173, stysslsh, NULL);
    }

    // [crispy] draw berserk pack instead of no ammo if appropriate
    if (screenblocks > 10)
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
            V_DrawPatch(left_delta + 23 - SHORT(patch->width)/2 + SHORT(patch->leftoffset),
                        179 - SHORT(patch->height)/2 + SHORT(patch->topoffset),
                        patch, NULL);
        }
    }

    // Ammo amount for current weapon
    if (weaponinfo[plyr->readyweapon].ammo != am_noammo)
    {
        ST_DrawBigNumber(plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                         6 + left_delta, 171, ST_WidgetColor(hudcolor_ammo));
    }

    // Health, negative health
    ST_DrawBigNumber(negative_health ? plyr->mo->health : plyr->health, 
                     52 + left_delta, 171, ST_WidgetColor(hudcolor_health));
    ST_DrawPercent(90 + left_delta, 171, sbar_colored == 1 ? cr[CR_WHITE] :
                                         sbar_colored == 2 ?
                                         ST_WidgetColor(hudcolor_health) : NULL);

    // Frags or Arms
    if (deathmatch)
    {
        ST_DrawBigNumber(st_fragscount, 100 + left_delta, 171, ST_WidgetColor(hudcolor_frags));
    }
    else
    {
        // Weapons or artifacts owned
        if (gamemode != pressbeta)
        {
            ST_DrawSmallNumberY(2, 107 + left_delta, 172);
            // Shotgun or Super Shotgun
            plyr->weaponowned[2] || plyr->weaponowned[8] ? 
                                    ST_DrawSmallNumberY(3, 119 + left_delta, 172) :
                                    ST_DrawSmallNumberG(3, 119 + left_delta, 172) ;
            // Chaingun
            plyr->weaponowned[3] ? ST_DrawSmallNumberY(4, 131 + left_delta, 172) :
                                   ST_DrawSmallNumberG(4, 131 + left_delta, 172) ;
            // Rocket Launcher
            plyr->weaponowned[4] ? ST_DrawSmallNumberY(5, 107 + left_delta, 182) :
                                   ST_DrawSmallNumberG(5, 107 + left_delta, 182) ;
            // Plasma Gun
            plyr->weaponowned[5] ? ST_DrawSmallNumberY(6, 119 + left_delta, 182) :
                                   ST_DrawSmallNumberG(6, 119 + left_delta, 182) ;
            // BFG9000
            plyr->weaponowned[6] ? ST_DrawSmallNumberY(7, 131 + left_delta, 182) :
                                   ST_DrawSmallNumberG(7, 131 + left_delta, 182) ;
        }
        else
        {
            ST_DrawBigNumber(artifactcount, 100 + left_delta, 171, ST_WidgetColor(hudcolor_artifacts));
        }
    }

    // Player face background
    if ((screenblocks == 11 || screenblocks == 14) && (!automapactive || automap_overlay))
    {
        V_DrawPatch(143 + wide_delta, 168, faceback, NULL);        
    }

    // Player face
    if ((screenblocks <= 11 || screenblocks == 14) || (automapactive && !automap_overlay))
    {
        V_DrawPatch(143 + wide_delta, 168, faces[st_faceindex], NULL);
        // Amount of lifes
        if (gamemode == pressbeta)
        {
            ST_DrawSmallNumberY(lifecount, 168 + wide_delta, 193);
        }
    }

    // Armor
    ST_DrawBigNumber(plyr->armorpoints, 183 + right_delta, 171, ST_WidgetColor(hudcolor_armor));
    ST_DrawPercent(221 + right_delta, 171, sbar_colored == 1 ? cr[CR_WHITE] :
                                           sbar_colored == 2 ?
                                           ST_WidgetColor(hudcolor_armor) : NULL);

    // [crispy] blinking key or skull in the status bar
    for (int i = 0, y = 0 ; i < 3 ; i++, y += 10)
    {
        if (plyr->tryopen[i])
        {
            if (!(plyr->tryopen[i] & (2 * KEYBLINKMASK - 1)))
            {
                S_StartSound(NULL, sfx_itemup);
            }
            if (plyr->tryopen[i] & KEYBLINKMASK)
            {
                V_DrawPatch(239 + right_delta, 171 + y, keys[i + st_keyorskull[i]] , NULL);
            }
        }
    }

    // Keys
    if (plyr->cards[it_bluecard] && plyr->cards[it_blueskull])
    V_DrawPatch(239 + right_delta, 171, keys[6], NULL);
    else if (plyr->cards[it_bluecard])
    V_DrawPatch(239 + right_delta, 171, keys[0], NULL);
    else if (plyr->cards[it_blueskull])
    V_DrawPatch(239 + right_delta, 171, keys[3], NULL);

    if (plyr->cards[it_yellowcard] && plyr->cards[it_yellowskull])
    V_DrawPatch(239 + right_delta, 181, keys[7], NULL);
    else if (plyr->cards[it_yellowcard])
    V_DrawPatch(239 + right_delta, 181, keys[1], NULL);
    else if (plyr->cards[it_yellowskull])
    V_DrawPatch(239 + right_delta, 181, keys[4], NULL);

    if (plyr->cards[it_redcard] && plyr->cards[it_redskull])
    V_DrawPatch(239 + right_delta, 191, keys[8], NULL);
    else if (plyr->cards[it_redcard])
    V_DrawPatch(239 + right_delta, 191, keys[2], NULL);
    else if (plyr->cards[it_redskull])
    V_DrawPatch(239 + right_delta, 191, keys[5], NULL);

    // Ammo (current)
    ST_DrawSmallNumberY(plyr->ammo[0], 280 + right_delta, 173);
    ST_DrawSmallNumberY(plyr->ammo[1], 280 + right_delta, 179);
    ST_DrawSmallNumberY(plyr->ammo[3], 280 + right_delta, 185);
    ST_DrawSmallNumberY(plyr->ammo[2], 280 + right_delta, 191);
    
    // Ammo (max)
    ST_DrawSmallNumberY(plyr->maxammo[0], 306 + right_delta, 173);
    ST_DrawSmallNumberY(plyr->maxammo[1], 306 + right_delta, 179);
    ST_DrawSmallNumberY(plyr->maxammo[3], 306 + right_delta, 185);
    ST_DrawSmallNumberY(plyr->maxammo[2], 306 + right_delta, 191);
}

// -----------------------------------------------------------------------------
// ST_DrawElementsJaguar
// [JN] Draw various digit values, faces and keys, Jaguar Doom version.
// -----------------------------------------------------------------------------

static void ST_DrawElementsJaguar (const boolean wide)
{
    int left_delta;
    int right_delta;
    
    if (wide)
    {
        left_delta = screenblocks <= 13 ? wide_delta : 0;
        right_delta = screenblocks <= 13 ? wide_delta : wide_delta*2;
    }
    else
    {
        left_delta = wide_delta;
        right_delta = wide_delta;
    }

    // Transparent signs
    if (screenblocks == 11 || screenblocks == 12
    ||  screenblocks == 14 || screenblocks == 15)
    {
        if (weaponinfo[plyr->readyweapon].ammo != am_noammo)
        {
            V_DrawPatch(left_delta, 0, stchammo, NULL);
        }
        V_DrawPatch(left_delta, 0, stchhlth, NULL);
        V_DrawPatch(right_delta, 0, stcharam, NULL);
    }

    // Ammo amount for current weapon
    if (weaponinfo[plyr->readyweapon].ammo != am_noammo)
    {
        ST_DrawBigNumber(plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                         13 + left_delta, 174, NULL);
    }

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

            // [crispy] (28,179) is the center of the Ammo widget
            V_DrawPatch(left_delta + 28 - SHORT(patch->width)/2 + SHORT(patch->leftoffset),
                        179 - SHORT(patch->height)/2 + SHORT(patch->topoffset),
                         patch, NULL);
        }
    }

    // Health, negative health
    ST_DrawBigNumber(negative_health ? plyr->mo->health : plyr->health, 
                     66 + left_delta, 174, NULL);
    ST_DrawPercent(104 + left_delta, 174, NULL);

    // [crispy] blinking key or skull in the status bar
    for (int i = 0, y = 0 ; i < 3 ; i++, y += 12)
    {
        if (plyr->tryopen[i])
        {
            if (!(plyr->tryopen[i] & (2 * KEYBLINKMASK - 1)))
            {
                S_StartSound(NULL, sfx_itemup);
            }
            if (plyr->tryopen[i] & KEYBLINKMASK)
            {
                V_DrawPatch(124 + left_delta, 163 + y, keys[i + st_keyorskull[i]] , NULL);
            }
        }
    }

    // Keys (no combined versions)
    if (plyr->cards[it_bluecard])
    V_DrawPatch(124 + left_delta, 163, keys[0], NULL);
    else if (plyr->cards[it_blueskull])
    V_DrawPatch(124 + left_delta, 163, keys[3], NULL);  

    if (plyr->cards[it_yellowcard])
    V_DrawPatch(124 + left_delta, 175, keys[1], NULL);
    else if (plyr->cards[it_yellowskull])
    V_DrawPatch(124 + left_delta, 175, keys[4], NULL);

    if (plyr->cards[it_redcard])
    V_DrawPatch(124 + left_delta, 187, keys[2], NULL);
    else if (plyr->cards[it_redskull])
    V_DrawPatch(124 + left_delta, 187, keys[5], NULL);
  
      // Player face background
    if ((screenblocks == 11 || screenblocks == 14) && (!automapactive || automap_overlay))
    {
        V_DrawPatch(143 + wide_delta, 168, faceback, NULL);        
    }
  
    // Player face
    if ((screenblocks <= 11 || screenblocks == 14) || (automapactive && !automap_overlay))
    {
        V_DrawPatch(143 + wide_delta, 166, faces[st_faceindex], NULL);
    }

    // Armor
    ST_DrawBigNumber(plyr->armorpoints, 187 + right_delta, 174, NULL);
    ST_DrawPercent(225 + right_delta, 174, NULL);

    // Weapons or artifacts owned
    ST_DrawSmallNumberY(2, 245 + right_delta, 175);
    // Shotgun or Super Shotgun
    plyr->weaponowned[2] || plyr->weaponowned[8] ? 
                            ST_DrawSmallNumberY(3, 257 + right_delta, 175) :
                            ST_DrawSmallNumberG(3, 257 + right_delta, 175) ;
    // Chaingun
    plyr->weaponowned[3] ? ST_DrawSmallNumberY(4, 269 + right_delta, 175) :
                           ST_DrawSmallNumberG(4, 269 + right_delta, 175) ;
    // Rocket Launcher
    plyr->weaponowned[4] ? ST_DrawSmallNumberY(5, 245 + right_delta, 185) :
                           ST_DrawSmallNumberG(5, 245 + right_delta, 185) ;
    // Plasma Gun
    plyr->weaponowned[5] ? ST_DrawSmallNumberY(6, 257 + right_delta, 185) :
                           ST_DrawSmallNumberG(6, 257 + right_delta, 185) ;
    // BFG9000
    plyr->weaponowned[6] ? ST_DrawSmallNumberY(7, 269 + right_delta, 185) :
                           ST_DrawSmallNumberG(7, 269 + right_delta, 185) ;

    ST_DrawBigNumber(gamemap, 279 + right_delta, 174, NULL);
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
// ST_DrawDemoTimer
// [crispy] print a bar indicating demo progress at the bottom of the screen
// -----------------------------------------------------------------------------

void ST_DemoProgressBar (void)
{
    const int i = screenwidth * defdemotics / deftotaldemotics;

    V_DrawHorizLine(0, SCREENHEIGHT - 2, i, 0); // [crispy] black
    V_DrawHorizLine(0, SCREENHEIGHT - 1, i, 4); // [crispy] white
}

// -----------------------------------------------------------------------------
// [JN] ST_drawWidgets
// Various status bar widgets.
// -----------------------------------------------------------------------------

void ST_WidgetsDrawer (void)
{
    static char str[128];
    const int time = leveltime / TICRATE;
    const int totaltime = (totalleveltimes / TICRATE) + (leveltime / TICRATE);
    const int wide_4_3 = aspect_ratio >= 2 && screenblocks == 9 ? wide_delta : 0;
    const int net_y = netgame ? 8 : 0;  // [JN] Shift one line down for chat string.
    plyr = &players[consoleplayer];

    if (((automapactive && automap_stats == 1) || automap_stats == 2))
    {
        // Kills:
        sprintf(str, plyr->extrakillcount ? "%d+%d/%d" : "%d/%d",
                plyr->killcount,
                plyr->extrakillcount ? plyr->extrakillcount : totalkills,
                totalkills);

        english_language ? RD_M_DrawTextA("K:", wide_4_3, 9+net_y) :
                           RD_M_DrawTextSmallRUS("D:", wide_4_3, 9+net_y, CR_NONE);
        
        dp_translation = hud_stats_color == 0 ? NULL :
                         totalkills == 0 ? cr[CR_GREEN] :
                         plyr->killcount == 0 ? cr[CR_RED] :
                         plyr->killcount < totalkills ? cr[CR_YELLOW] : cr[CR_GREEN];
        RD_M_DrawTextA(str, wide_4_3 + 16, 9+net_y);
        dp_translation = NULL;

        // Items:
        sprintf(str, "%d/%d", plyr->itemcount, totalitems);

        english_language ? RD_M_DrawTextA("I:", wide_4_3, 17+net_y) :
                           RD_M_DrawTextSmallRUS("G:", wide_4_3, 17+net_y, CR_NONE);

        dp_translation = hud_stats_color == 0 ? NULL :
                         totalitems == 0 ? cr[CR_GREEN] :
                         plyr->itemcount == 0 ? cr[CR_RED] :
                         plyr->itemcount < totalitems ? cr[CR_YELLOW] : cr[CR_GREEN];
        RD_M_DrawTextA(str, wide_4_3 + 16, 17+net_y);
        dp_translation = NULL;

        // Secret:
        sprintf(str, "%d/%d", plyr->secretcount, totalsecret);

        english_language ? RD_M_DrawTextA("S:", wide_4_3, 25+net_y) :
                           RD_M_DrawTextSmallRUS("N:", wide_4_3, 25+net_y, CR_NONE);

        dp_translation = hud_stats_color == 0 ? NULL :
                         totalsecret == 0 ? cr[CR_GREEN] :
                         plyr->secretcount == 0 ? cr[CR_RED] :
                         plyr->secretcount < totalsecret ? cr[CR_YELLOW] : cr[CR_GREEN];
        RD_M_DrawTextA(str, wide_4_3 + 16, 25+net_y);
        dp_translation = NULL;
    }

    // Skill Level:
    if (((automapactive && automap_skill == 1) || automap_skill == 2))
    {
        sprintf(str, "%d", gameskill+1);

        english_language ? RD_M_DrawTextA("SKL:", wide_4_3, 33+net_y) :
                           RD_M_DrawTextSmallRUS("CK;:", wide_4_3, 33+net_y, CR_NONE);

        dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
        RD_M_DrawTextA(str, wide_4_3 + (english_language ? 31 : 36), 33+net_y);
        dp_translation = NULL;
    }

    // Level Time:
    if (((automapactive && automap_level_time == 1) || automap_level_time == 2))
    {
        sprintf(str, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);

        english_language ? RD_M_DrawTextA("LEVEL", wide_4_3, 49+net_y) :
                           RD_M_DrawTextSmallRUS("EHJDTYM", wide_4_3, 49+net_y, CR_NONE);

        dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
        RD_M_DrawTextA(str, wide_4_3, 57+net_y);
        dp_translation = NULL;
    }

    // Total Time:
    if (((automapactive && automap_total_time == 1) || automap_total_time == 2))
    {
        sprintf(str, "%02d:%02d:%02d", totaltime/3600, (totaltime%3600)/60, totaltime%60);

        english_language ? RD_M_DrawTextA("TOTAL", wide_4_3, 73+net_y) :
                           RD_M_DrawTextSmallRUS("J,OTT", wide_4_3, 73+net_y, CR_NONE);

        dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
        RD_M_DrawTextA(str, wide_4_3, 81+net_y);
        dp_translation = NULL;
    }

    // Player Coords:
    if (((automapactive && automap_coords == 1) || automap_coords == 2))
    {
        dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GREEN];
        RD_M_DrawTextA("X:", wide_4_3, 97+net_y);
        RD_M_DrawTextA("Y:", wide_4_3, 105+net_y);
        RD_M_DrawTextA("Z:", wide_4_3, 113+net_y);
        RD_M_DrawTextA("ANG:", wide_4_3, 121+net_y);
        dp_translation = NULL;

        dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
        sprintf(str, "%d", plyr->mo->x >> FRACBITS);
        RD_M_DrawTextA(str, wide_4_3 + 16, 97+net_y);
        sprintf(str, "%d", plyr->mo->y >> FRACBITS);
        RD_M_DrawTextA(str, wide_4_3 + 16, 105+net_y);
        sprintf(str, "%d", plyr->mo->z >> FRACBITS);
        RD_M_DrawTextA(str, wide_4_3 + 16, 113+net_y);
        sprintf(str, "%d", plyr->mo->angle / ANG1);
        RD_M_DrawTextA(str, wide_4_3 + 32, 121+net_y);
        dp_translation = NULL;
    }

    // [JN] Draw crosshair. 
    // Thanks to Fabian Greffrath for ORIGWIDTH, ORIGHEIGHT and ST_HEIGHT values,
    // thanks to Zodomaniac for proper health values!
    if (crosshair_draw && !automapactive && !menuactive)
    {
        Crosshair_Draw();
    }

    // [crispy] demo timer widget
    if (demoplayback && (demotimer == 1 || demotimer == 3))
    {
        ST_DrawDemoTimer(demotimerdir ? (deftotaldemotics - defdemotics) : defdemotics);
    }
    else if (demorecording && (demotimer == 2 || demotimer == 3))
    {
        ST_DrawDemoTimer(leveltime);
    }

    // [crispy] demo progress bar
    if (demoplayback && demobar)
    {
        ST_DemoProgressBar();
    }
}

// -----------------------------------------------------------------------------
// ST_MapNameDrawer
// Draws map name.
// -----------------------------------------------------------------------------

void ST_MapNameDrawer (void)
{
    static char str[128];
    const int map_y = gamemission == jaguar ? 151 : 159;
    const int wide_4_3 = aspect_ratio >= 2 && screenblocks == 9 ? wide_delta : 0;

    // Level name:
    if (automapactive)
    {
        extern char *level_name;

        sprintf(str, "%s", level_name);
        if (english_language)
        {
            dp_translation = (hud_stats_color && !vanillaparm) ? cr[CR_YELLOW] : NULL;
            RD_M_DrawTextA(str, wide_4_3, map_y);
            dp_translation = NULL;
        }
        else
        {
            RD_M_DrawTextSmallRUS(str, wide_4_3, map_y,
                                  (hud_stats_color && !vanillaparm) ? CR_YELLOW : CR_NONE);
        }
    }
}

// -----------------------------------------------------------------------------
// ST_Drawer
// [JN] Main drawing 
// -----------------------------------------------------------------------------

void ST_Drawer (void)
{
    // Do red-/gold-shifts from damage/items
    ST_DoPaletteStuff();

    // [JN] Refresh standard status bar beneath wipe effect.
    if (wipegamestate != GS_LEVEL)
    {
        ST_BackgroundDrawer();
    }

    // [JN] Draw full screen status bar.
    if (screenblocks > 10 && screenblocks < (aspect_ratio >= 2 ? 17 : 14)
    && (!automapactive || automap_overlay))
    {
        ST_DrawElementsFunc(screenblocks >= 14 ? true : false);
    }
}

// -----------------------------------------------------------------------------
// ST_LoadData
// -----------------------------------------------------------------------------

static void ST_LoadData (void)
{
    int   i, j;
    int   facenum = 0;
    char  namebuf[9];

    lu_palette = W_GetNumForName (DEH_String("PLAYPAL"));

    // [JN] Which background will be used to fill bezel pattern.
    bezel_pattern = W_CacheLumpName(DEH_String(gamemode == commercial ?
                                    "GRNROCK" : "FLOOR7_2"), PU_STATIC);

    // [JN] Initialize STBAR horizontal offset with zero for centering.
    stbar->leftoffset = 0;

    // Big font
    FontBNumBase = W_GetNumForName(DEH_String("STTNUM0"));
    for (i = 0; i < 10; i++)
    {
        FontBNumbers[i] = W_CacheLumpNum(FontBNumBase + i, PU_STATIC);
    }
    FontBPercent = W_CacheLumpName(DEH_String("STTPRCNT"), PU_STATIC);
    FontBMinus = W_CacheLumpName(DEH_String("STTMINUS"), PU_STATIC);

    // Small yellow font
    FontSNumBaseY = W_GetNumForName(DEH_String("STYSNUM0"));
    for (i = 0; i < 10; i++)
    {
        FontSNumbersY[i] = W_CacheLumpNum(FontSNumBaseY + i, PU_STATIC);
    }

    // Small gray font
    FontSNumBaseG = W_GetNumForName(DEH_String("STGNUM0"));
    for (i = 0; i < 10; i++)
    {
        FontSNumbersG[i] = W_CacheLumpNum(FontSNumBaseG + i, PU_STATIC);
    }

    // Face backgrounds for different color players
    if (netgame)
    {
        DEH_snprintf(namebuf, 9, "ID#STPB%d", consoleplayer);
        faceback = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
    }
    else
    {
        faceback = W_CacheLumpName(DEH_String("ID#STPB1"), PU_STATIC);
    }

    // Player faces
    for (i = 0; i < ST_NUMPAINFACES; i++)
    {
        for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
        {
            DEH_snprintf(namebuf, 9, "STFST%d%d", i, j);
            faces[facenum] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
            ++facenum;
        }

        DEH_snprintf(namebuf, 9, "STFTR%d0", i);   // turn right
        faces[facenum] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFTL%d0", i);   // turn left
        faces[facenum] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFOUCH%d", i);  // ouch!
        faces[facenum] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFEVL%d", i);   // evil grin ;)
        faces[facenum] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFKILL%d", i);  // pissed off
        faces[facenum] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
        ++facenum;
    }

    faces[facenum] = W_CacheLumpName(DEH_String("STFGOD0"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFDEAD0"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFEXPL0"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFEXPL1"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFEXPL2"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFEXPL3"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFEXPL4"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFEXPL5"), PU_STATIC);
    ++facenum;
    faces[facenum] = W_CacheLumpName(DEH_String("STFCRSH0"), PU_STATIC);
    ++facenum;

    // [JN] Doubled array for extra god mode faces, thanks Brad Harding for help!
    if (!old_godface)
    {
        for (i = 0; i < ST_NUMPAINFACES; i++)
        {
            for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
            {
                M_snprintf(namebuf, 9, "STFST%i%iG", i, j);
                faces[facenum++] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
            }

            M_snprintf(namebuf, 9, "STFTR%i0G", i);  // turn right
            faces[facenum++] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);

            M_snprintf(namebuf, 9, "STFTL%i0G", i);  // turn left
            faces[facenum++] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);

            M_snprintf(namebuf, 9, "STFOUC%iG", i);  // ouch!
            faces[facenum++] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);

            M_snprintf(namebuf, 9, "STFEVL%iG", i);  // evil grin ;)
            faces[facenum++] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);

            M_snprintf(namebuf, 9, "STFKIL%iG", i);  // pissed off
            faces[facenum++] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
        }
        
        faces[facenum++] = W_CacheLumpName(DEH_String("STFGOD0G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFDEA0G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFEXP0G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFEXP1G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFEXP2G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFEXP3G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFEXP4G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFEXP5G"), PU_STATIC);
        faces[facenum++] = W_CacheLumpName(DEH_String("STFCRS0G"), PU_STATIC);
    }

    // Key cards and Skulls
    for (i = 0 ; i < NUMCARDS+3 ; i++)  // [JN] jff 2/23/98 show both keys too
    {
        DEH_snprintf(namebuf, 9, "STKEYS%d", i);
        keys[i] = W_CacheLumpName(DEH_String(namebuf), PU_STATIC);
    }
}

// -----------------------------------------------------------------------------
// ST_Start
// -----------------------------------------------------------------------------

void ST_Start (void)
{
    I_SetPalette (W_CacheLumpNum ((lu_palette), PU_CACHE));
    plyr = &players[consoleplayer];

    faceindex = 1; // [crispy] fix status bar face hysteresis across level changes
    st_faceindex = 1;
    st_palette = -1;
    st_oldhealth = -1;
}

// -----------------------------------------------------------------------------
// ST_Init
// -----------------------------------------------------------------------------

void ST_Init (void)
{
    // [JN] Initialize status bar height.
    // In Jaguar Doom it's 40 pixels tall instead of 32.
    st_height = gamemission == jaguar ? ST_HEIGHT_JAG : ST_HEIGHT;
    st_y = gamemission == jaguar ? ST_Y_JAG : ST_Y;

    // [JN] Define screen border background.
    st_backing_screen = (byte *)Z_Malloc((screenwidth << hires) 
                      * (st_height << hires)
                      * sizeof(*st_backing_screen), PU_STATIC, 0);

    // Load graphics.
    ST_LoadData();

    // [JN] Jaguar Doom using defferent status bar values and arrangement.
    ST_DrawElementsFunc = gamemission == jaguar ? ST_DrawElementsJaguar :
                                                  ST_DrawElements;

    // [JN] Initialize status bar widget colors.
    M_RD_Define_SBarColorValue(&stbar_color_high_set, stbar_color_high);
    M_RD_Define_SBarColorValue(&stbar_color_normal_set, stbar_color_normal);
    M_RD_Define_SBarColorValue(&stbar_color_low_set, stbar_color_low);
    M_RD_Define_SBarColorValue(&stbar_color_critical_set, stbar_color_critical);
    M_RD_Define_SBarColorValue(&stbar_color_armor_1_set, stbar_color_armor_1);
    M_RD_Define_SBarColorValue(&stbar_color_armor_2_set, stbar_color_armor_2);
    M_RD_Define_SBarColorValue(&stbar_color_armor_0_set, stbar_color_armor_0);
}


// =============================================================================
//
// [JN] Crosshair routines. Pre/re-defining, drawing, coloring.
//
// =============================================================================

patch_t      *CrosshairPatch;
byte         *CrosshairOpacity;
static void (*Crosshair_Draw_Func) (void);

// -----------------------------------------------------------------------------
// Crosshair_DefinePatch: which GFX patch will be used.
// -----------------------------------------------------------------------------

void Crosshair_DefinePatch (void)
{
    CrosshairPatch = 
        W_CacheLumpName(crosshair_shape == 1 ? "XHAIR_2" :
                        crosshair_shape == 2 ? "XHAIR_3" :
                        crosshair_shape == 3 ? "XHAIR_4" :
                        crosshair_shape == 4 ? "XHAIR_5" :
                        crosshair_shape == 5 ? "XHAIR_6" :
                        crosshair_shape == 6 ? "XHAIR_7" :
                                               "XHAIR_1", PU_STATIC);
}

// -----------------------------------------------------------------------------
// Crosshair_DefineOpacity: what amount of transparency will be used.
// -----------------------------------------------------------------------------

void Crosshair_DefineOpacity (void)
{
    CrosshairOpacity = crosshair_opacity == 0 ? transtable20 :
                       crosshair_opacity == 1 ? transtable30 :
                       crosshair_opacity == 2 ? transtable40 :
                       crosshair_opacity == 3 ? transtable50 :
                       crosshair_opacity == 4 ? transtable60 :
                       crosshair_opacity == 5 ? transtable70 :
                       crosshair_opacity == 6 ? transtable80 :
                       crosshair_opacity == 7 ? transtable90 :
                                                NULL;
}

// -----------------------------------------------------------------------------
// Crosshair_Colorize_inMenu: coloring routine for menu. Cycling through colors.
// -----------------------------------------------------------------------------

void Crosshair_Colorize_inMenu (void)
{
    if (crosshair_type == 1)
    {
        if (CrosshairShowcaseTimeout > 105)
        {
            CrosshairShowcaseTimeout = 105;
        }

        dp_translation = CrosshairShowcaseTimeout >= 70 ? cr[CR_RED]    :
                         CrosshairShowcaseTimeout >= 35 ? cr[CR_YELLOW] :
                                                          cr[CR_GREEN];
    }
    else if (crosshair_type == 2)
    {
        if (CrosshairShowcaseTimeout > 70)
        {
            CrosshairShowcaseTimeout = 70;
        }

        dp_translation = CrosshairShowcaseTimeout >= 35 ? cr[CR_RED] :
                                                          cr[CR_BLUE];
    }
    else if (crosshair_type == 3)
    {
        dp_translation = CrosshairShowcaseTimeout >= 105 ? cr[CR_RED]    :
                         CrosshairShowcaseTimeout >=  70 ? cr[CR_YELLOW] :
                         CrosshairShowcaseTimeout >=  35 ? cr[CR_GREEN]  :
                                                           cr[CR_BLUE];
    }
}

// -----------------------------------------------------------------------------
// Crosshair_Colorize_inGame: ingame coloring routine, actual colors/values.
// -----------------------------------------------------------------------------

static void Crosshair_Colorize_inGame (void)
{
    if (crosshair_type == 1)
    {
        dp_translation = plyr->health >= 67 ? cr[CR_GREEN]  :
                         plyr->health >= 34 ? cr[CR_YELLOW] :
                                             cr[CR_RED];
    }
    else if (crosshair_type == 2)
    {
        P_AimLineAttack(plyr->mo, plyr->mo->angle, MISSILERANGE);

        if (linetarget)
        {
            dp_translation = cr[CR_BLUE];
        }
    }
    else if (crosshair_type == 3)
    {
        dp_translation = plyr->health >= 67 ? cr[CR_GREEN]  :
                         plyr->health >= 34 ? cr[CR_YELLOW] :
                                             cr[CR_RED];

        P_AimLineAttack(plyr->mo, plyr->mo->angle, MISSILERANGE);

        if (linetarget)
        {
            dp_translation = cr[CR_BLUE];
        }
    }
}

// -----------------------------------------------------------------------------
// Crosshair_Draw_Scaled: drawing func for scaled (big) crosshair.
// -----------------------------------------------------------------------------

static void Crosshair_Draw_Scaled (void)
{
    V_DrawPatch(origwidth/2, screenblocks <= 10 ? 84 : 102,
                CrosshairPatch, CrosshairOpacity);
}

// -----------------------------------------------------------------------------
// Crosshair_Draw_Unscaled: drawing func for unscaled (small) crosshair.
// -----------------------------------------------------------------------------

static void Crosshair_Draw_Unscaled (void)
{
    V_DrawPatchUnscaled(screenwidth/2, screenblocks <= 10 ? 168 : 204,
                        CrosshairPatch, CrosshairOpacity);
}

// -----------------------------------------------------------------------------
// Crosshair_DefineDrawingFunc: predefinition of drawing func for later use.
// -----------------------------------------------------------------------------

void Crosshair_DefineDrawingFunc (void)
{
    Crosshair_Draw_Func = crosshair_scale ? Crosshair_Draw_Scaled :
                                            Crosshair_Draw_Unscaled;
}

// -----------------------------------------------------------------------------
// Crosshair_Draw: actual coloring and drawing.
// -----------------------------------------------------------------------------

void Crosshair_Draw (void)
{
    Crosshair_Colorize_inGame();
    Crosshair_Draw_Func();
    dp_translation = NULL;
}
