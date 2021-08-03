//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
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
//	Game completion, final screen animation.
//


#include <stdio.h>
#include <ctype.h>

#include "deh_main.h"
#include "i_system.h"
#include "i_swap.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "d_main.h"
#include "rd_lang.h"
#include "sounds.h"
#include "doomstat.h"
#include "r_state.h"
#include "r_main.h"
#include "hu_stuff.h"
#include "crispy.h"
#include "jn.h"


#define TEXTSPEED   3
#define TEXTWAIT    250


typedef enum
{
    F_STAGE_TEXT,
    F_STAGE_ARTSCREEN,
    F_STAGE_CAST,
    F_STAGE_CAST_JAGUAR,
} finalestage_t;


// Stage of animation:
finalestage_t finalestage;

unsigned int finalecount;

// [JN] Was final wipe done?
boolean finale_wipe_done = false;

typedef struct
{
    GameMission_t mission;
    int episode, level;
    char *background;
    char *text;
} textscreen_t;

static textscreen_t textscreens[] =
{
    { doom,      1, 8,  "FLOOR4_8",  E1TEXT},
    { doom,      2, 8,  "SFLR6_1",   E2TEXT},
    { doom,      3, 8,  "MFLR8_4",   E3TEXT},
    { doom,      4, 8,  "MFLR8_3",   E4TEXT},
    { doom,      5, 8,  "FLOOR7_2",  E5TEXT}, // [crispy] Sigil

    { doom2,     1, 6,  "SLIME16",   C1TEXT},
    { doom2,     1, 11, "RROCK14",   C2TEXT},
    { doom2,     1, 20, "RROCK07",   C3TEXT},
    { doom2,     1, 30, "RROCK17",   C4TEXT},
    { doom2,     1, 15, "RROCK13",   C5TEXT},
    { doom2,     1, 31, "RROCK19",   C6TEXT},

    { pack_tnt,  1, 6,  "SLIME16",   T1TEXT},
    { pack_tnt,  1, 11, "RROCK14",   T2TEXT},
    { pack_tnt,  1, 20, "RROCK07",   T3TEXT},
    { pack_tnt,  1, 30, "RROCK17",   T4TEXT},
    { pack_tnt,  1, 15, "RROCK13",   T5TEXT},
    { pack_tnt,  1, 31, "RROCK19",   T6TEXT},

    { pack_plut, 1, 6,  "SLIME16",   P1TEXT},
    { pack_plut, 1, 11, "RROCK14",   P2TEXT},
    { pack_plut, 1, 20, "RROCK07",   P3TEXT},
    { pack_plut, 1, 30, "RROCK17",   P4TEXT},
    { pack_plut, 1, 15, "RROCK13",   P5TEXT},
    { pack_plut, 1, 31, "RROCK19",   P6TEXT},

    { pack_nerve, 1, 8, "SLIME16",   N1TEXT},

    { jaguar,    1, 23, "ROCKS",     J1TEXT},
};

static textscreen_t textscreens_rus[] =
{
    { doom,      1, 8,  "FLOOR4_8",  E1TEXT_RUS},
    { doom,      2, 8,  "SFLR6_1",   E2TEXT_RUS},
    { doom,      3, 8,  "MFLR8_4",   E3TEXT_RUS},
    { doom,      4, 8,  "MFLR8_3",   E4TEXT_RUS},
    { doom,      5, 8,  "FLOOR7_2",  E5TEXT_RUS}, // [JN] Sigil

    { doom2,     1, 6,  "SLIME16",   C1TEXT_RUS},
    { doom2,     1, 11, "RROCK14",   C2TEXT_RUS},
    { doom2,     1, 20, "RROCK07",   C3TEXT_RUS},
    { doom2,     1, 30, "RROCK17",   C4TEXT_RUS},
    { doom2,     1, 15, "RROCK13",   C5TEXT_RUS},
    { doom2,     1, 31, "RROCK19",   C6TEXT_RUS},

    { pack_tnt,  1, 6,  "SLIME16",   T1TEXT_RUS},
    { pack_tnt,  1, 11, "RROCK14",   T2TEXT_RUS},
    { pack_tnt,  1, 20, "RROCK07",   T3TEXT_RUS},
    { pack_tnt,  1, 30, "RROCK17",   T4TEXT_RUS},
    { pack_tnt,  1, 15, "RROCK13",   T5TEXT_RUS},
    { pack_tnt,  1, 31, "RROCK19",   T6TEXT_RUS},

    { pack_plut, 1, 6,  "SLIME16",   P1TEXT_RUS},
    { pack_plut, 1, 11, "RROCK14",   P2TEXT_RUS},
    { pack_plut, 1, 20, "RROCK07",   P3TEXT_RUS},
    { pack_plut, 1, 30, "RROCK17",   P4TEXT_RUS},
    { pack_plut, 1, 15, "RROCK13",   P5TEXT_RUS},
    { pack_plut, 1, 31, "RROCK19",   P6TEXT_RUS},

    { pack_nerve, 1, 8, "SLIME16",   N1TEXT_RUS},

    { jaguar,    1, 23, "ROCKS",     J1TEXT_RUS},
};


char*   finaletext;
char*   finaleflat;

void    F_StartCast (void);
void    F_CastTicker (void);
void    F_CastDrawer (void);
boolean F_CastResponder (event_t *ev);

// [JN] Jaguar Doom prototypes
void F_TextWriteJaguar (void);
void F_CastTickerJaguar (void);
void F_CastPrintJaguar (char *text);
void F_CastDrawerJaguar (void);


//
// F_StartFinale
//
void F_StartFinale (void)
{
    size_t i;

    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;

    if (logical_gamemission == doom)
    {
        S_ChangeMusic(mus_victor, true);
    }
    else
    {
        S_ChangeMusic(mus_read_m, true);
    }

    // Find the right screen and set the text and background

    for (i = 0; i < arrlen(textscreens) ; ++i)
    {
        textscreen_t *screen = english_language ? 
                               &textscreens[i] :
                               &textscreens_rus[i];

        // Hack for Chex Quest

        if (gameversion == exe_chex && screen->mission == doom)
        {
            screen->level = 5;
        }

        if (logical_gamemission == screen->mission
        && (logical_gamemission != doom || gameepisode == screen->episode)
        && gamemap == screen->level)
        {
            finaletext = screen->text;
            finaleflat = screen->background;
        }
    }

    // Do dehacked substitutions of strings

    finaletext = DEH_String(finaletext);
    finaleflat = DEH_String(finaleflat);

    finalestage = F_STAGE_TEXT;
    finalecount = 0;
}


boolean F_Responder (event_t *event)
{
    if (finalestage == F_STAGE_CAST || finalestage == F_STAGE_CAST_JAGUAR)
    {
        return F_CastResponder (event);
    }

    return false;
}


//
// F_Ticker
//
void F_Ticker (void)
{
    size_t i;

    //
    // [JN] If we are in single player mode, allow double skipping for 
    // intermission text. First skip printing all intermission text,
    // second is advancing to the next state.
    //
    if (singleplayer)
    {
        // [JN] Make PAUSE working properly on text screen
        if (paused)
        {
            return;
        }

        // [JN] Check for skipping. Allow double-press skiping, 
        // but don't skip immediately.
        if (finalecount > 10)
        {
            // go on to the next level
            for (i = 0 ; i < MAXPLAYERS ; i++)
            {
                // [JN] Don't allow to skip bunny screen,
                // and don't allow to skip by pressing "pause" button.
                if ((gameepisode == 3 && finalestage == F_STAGE_ARTSCREEN)
                || players[i].cmd.buttons == (BT_SPECIAL | BTS_PAUSE))
                continue;

                // [JN] Double-skip by pressing "attack" button.
                if (players[i].cmd.buttons & BT_ATTACK && !menuactive)
                {
                    if (!players[i].attackdown)
                    {
                        if (finalecount >= 5003)
                        break;
    
                        finalecount += 5000;
                        players[i].attackdown = true;
                    }
                    players[i].attackdown = true;
                }
                else
                {
                    players[i].attackdown = false;
                }
    
                // [JN] Double-skip by pressing "use" button.
                if (players[i].cmd.buttons & BT_USE && !menuactive)
                {
                    if (!players[i].usedown)
                    {
                        if (finalecount >= 5003)
                        break;
    
                        finalecount += 5000;
                        players[i].usedown = true;
                    }
                    players[i].usedown = true;
    
                }
                else
                {
                    players[i].usedown = false;
                }
            }
    
            if (i < MAXPLAYERS)
            {
                if (gamemode != commercial)
                {
                    
                    finalestage = F_STAGE_ARTSCREEN;
    
                    // [JN] Play wipe animation only once.
                    if (!finale_wipe_done)
                    {
                        wipegamestate = -1; // force a wipe
                        finale_wipe_done = true;
                    }
            
                    if (gameepisode == 3)
                    {
                        finalecount = 0;
                        S_StartMusic (mus_bunny);
                    }
                
                    return;
                }
    
                // [JN] No Rest for the Living
                if (gamemission == pack_nerve && gamemap == 8)
                {
                    F_StartCast ();
                }  
                
                // [JN] Jaguar Doom: after beating MAP23, don't go any farther
                else if (gamemission == jaguar && gamemap == 23)
                {
                    F_StartCast ();
                }
    
                else if (gamemap == 30)
                {
                    F_StartCast ();
                }
    
                else
                {
                    gameaction = ga_worlddone;
                }
            }
        }
    
        // advance animation
        finalecount++;
    
        if (finalestage == F_STAGE_CAST)
        {
            F_CastTicker ();
            return;
        }
        else if (finalestage == F_STAGE_CAST_JAGUAR)
        {
            F_CastTickerJaguar ();
            return;
        }
    }
    //
    // [JN] Standard Doom routine, safe for network game and demos.
    //        
    else
    {
        // check for skipping
        if ( (gamemode == commercial)
        && ( finalecount > 50) )
        {
            // go on to the next level
            for (i=0 ; i<MAXPLAYERS ; i++)
            {
                if (players[i].cmd.buttons)
                {
                    break;
                }
            }
    
            if (i < MAXPLAYERS)
            {
                // [JN] No Rest for the Living
                if (gamemission == pack_nerve && gamemap == 8)
                {
                    F_StartCast ();
                }  
                
                // [JN] Jaguar: after beating MAP23, don't go any farther
                else if (gamemission == jaguar && gamemap == 23)
                {
                    F_StartCast ();
                }
    
                else if (gamemap == 30)
                {
                    F_StartCast ();
                }
    
                else
                {
                    gameaction = ga_worlddone;
                }
            }
        }
    
        // advance animation
        finalecount++;
    
        if (finalestage == F_STAGE_CAST)
        {
            F_CastTicker ();
            return;
        }
        else if (finalestage == F_STAGE_CAST_JAGUAR)
        {
            F_CastTickerJaguar ();
            return;
        }
    
        if ( gamemode == commercial)
        return;
    
        if (finalestage == F_STAGE_TEXT
        && finalecount>strlen (finaletext)*TEXTSPEED + TEXTWAIT)
        {
            finalecount = 0;
            finalestage = F_STAGE_ARTSCREEN;
            wipegamestate = -1; // force a wipe
    
            if (gameepisode == 3)
            S_StartMusic (mus_bunny);
        }
    }
}


//
// F_TextWrite
//
void F_TextWrite (void)
{
    int    w;
    int    cx;
    int    cy;
    char  *ch;
    signed int count;

    // [JN] Erase the entire screen to a tiled background.
    V_FillFlat (finaleflat);

    V_MarkRect (0, 0, screenwidth, SCREENHEIGHT);

    if (gamemission == jaguar)
    {
        // Leaving MAP23, end game. Special background.
        if (gamemap == 23)
        {
            V_DrawPatchFullScreen (W_CacheLumpName (DEH_String("ENDPIC"), PU_CACHE), false);
        }
    }
    // [JN] Draw special background on entering Wolfenstein and Grosse levels
    else if (gamemission == doom2 && !vanillaparm)
    {
        // [JN] Leaving MAP15, entering MAP31: blue Wolfenstein 3D background.
        if (gamemap == 15)
        {
            V_DrawPatchFullScreen (W_CacheLumpName (DEH_String("WLFBACK1"), PU_CACHE), false);
        }

        // Leaving MAP31, entering MAP32: red Wolfenstein 3D background.
        if (gamemap == 31)
        {
            V_DrawPatchFullScreen (W_CacheLumpName (DEH_String("WLFBACK2"), PU_CACHE), false);
        }
    }

    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext;

    count = ((signed int) finalecount - 10) / TEXTSPEED;

    if (count < 0)
    {
        count = 0;
    }

    for ( ; count ; count-- )
    {
        int c = *ch++;

        if (!c)
        {
            break;
        }

        if (c == '\n')
        {
            cx = 10;
            cy += 11;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        if (english_language)
        w = SHORT (hu_font[c]->width);
        else
        w = SHORT (hu_font_small_rus[c]->width);

        if (cx+w > origwidth)
	    break;

        if (english_language)
        V_DrawShadowedPatchDoom(cx + wide_delta, cy, hu_font[c]);
        else
        V_DrawShadowedPatchDoom(cx + wide_delta, cy, hu_font_small_rus[c]);

        cx+=w;
    }
}


//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
    char        *name;
    mobjtype_t  type;
} castinfo_t;


castinfo_t	castorder[] = {
    {CC_ZOMBIE,  MT_POSSESSED},
    {CC_SHOTGUN, MT_SHOTGUY},
    {CC_HEAVY,   MT_CHAINGUY},
    {CC_IMP,     MT_TROOP},
    {CC_DEMON,   MT_SERGEANT},
    {CC_LOST,    MT_SKULL},
    {CC_CACO,    MT_HEAD},
    {CC_HELL,    MT_KNIGHT},
    {CC_BARON,   MT_BRUISER},
    {CC_ARACH,   MT_BABY},
    {CC_PAIN,    MT_PAIN},
    {CC_REVEN,   MT_UNDEAD},
    {CC_MANCU,   MT_FATSO},
    {CC_ARCH,    MT_VILE},
    {CC_SPIDER,  MT_SPIDER},
    {CC_CYBER,   MT_CYBORG},
    {CC_HERO,    MT_PLAYER},

    {NULL,0}
};

castinfo_t	castorder_rus[] = {
    {CC_ZOMBIE_RUS,  MT_POSSESSED},
    {CC_SHOTGUN_RUS, MT_SHOTGUY},
    {CC_HEAVY_RUS,   MT_CHAINGUY},
    {CC_IMP_RUS,     MT_TROOP},
    {CC_DEMON_RUS,   MT_SERGEANT},
    {CC_LOST_RUS,    MT_SKULL},
    {CC_CACO_RUS,    MT_HEAD},
    {CC_HELL_RUS,    MT_KNIGHT},
    {CC_BARON_RUS,   MT_BRUISER},
    {CC_ARACH_RUS,   MT_BABY},
    {CC_PAIN_RUS,    MT_PAIN},
    {CC_REVEN_RUS,   MT_UNDEAD},
    {CC_MANCU_RUS,   MT_FATSO},
    {CC_ARCH_RUS,    MT_VILE},
    {CC_SPIDER_RUS,  MT_SPIDER},
    {CC_CYBER_RUS,   MT_CYBORG},
    {CC_HERO_RUS,    MT_PLAYER},

    {NULL,0}
};

// [JN] Jaguar: own casting order
castinfo_t	castorder_jaguar[] = {
    {CC_ZOMBIE,  MT_POSSESSED},
    {CC_SHOTGUN, MT_SHOTGUY},
    {CC_IMP,     MT_TROOP},
    {CC_DEMON,   MT_SERGEANT},
    {CC_LOST,    MT_SKULL},
    {CC_CACO,    MT_HEAD},
    {CC_BARON,   MT_BRUISER},
    {CC_HERO,    MT_PLAYER},
    {NULL,0}
};

castinfo_t	castorder_jaguar_russian[] = {
    {CC_ZOMBIE_RUS,  MT_POSSESSED},
    {CC_SHOTGUN_RUS, MT_SHOTGUY},
    {CC_IMP_RUS,     MT_TROOP},
    {CC_DEMON_RUS,   MT_SERGEANT},
    {CC_LOST_RUS,    MT_SKULL},
    {CC_CACO_RUS,    MT_HEAD},
    {CC_BARON_RUS,   MT_BRUISER},
    {CC_HERO_RUS,    MT_PLAYER},
    {NULL,0}
};


int         castnum;
int         casttics;
state_t*    caststate;
boolean     castdeath;
int         castframes;
int         castonmelee;
boolean     castattacking;


//
// F_StartCast
//
void F_StartCast (void)
{
    wipegamestate = -1; // force a screen wipe
    castnum = 0;

    // [JN] Jaguar Doom: own casting order.
    if (gamemission == jaguar)
    caststate = &states[mobjinfo[castorder_jaguar[castnum].type].seestate];
    else
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];

    casttics = caststate->tics;
    castdeath = false;

    finalestage = gamemission == jaguar ? F_STAGE_CAST_JAGUAR : F_STAGE_CAST;

    castframes = 0;
    castonmelee = 0;
    castattacking = false;
    S_ChangeMusic((gamemission == jaguar ? 
                   mus_adrian : // [JN] Jaguar Doom: Unreleased Doom MIDI: un20.mid
                   mus_evil), true);
}


//
// F_CastTicker
//
void F_CastTicker (void)
{
    int st;
    int sfx;

    if (--casttics > 0)
    {
        // not time to change state yet
        return;
    }

    if (caststate->tics == -1 || caststate->nextstate == S_NULL)
    {
        // switch from deathstate to next monster
        castnum++;
        castdeath = false;

        if (castorder[castnum].name == NULL)
        castnum = 0;

        if (mobjinfo[castorder[castnum].type].seesound)
        S_StartSound (NULL, mobjinfo[castorder[castnum].type].seesound);

        caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        castframes = 0;
    }
    else
    {
        // [crispy] fix Doomguy in casting sequence
        if (!castdeath && caststate == &states[S_PLAY_ATK1])
        {
            st = S_PLAY_ATK2;
        }
        else if (!castdeath && caststate == &states[S_PLAY_ATK2])
        {
            goto stopattack;	// Oh, gross hack!
        }
        else
        {
            st = caststate->nextstate;
        }
        caststate = &states[st];
        castframes++;

        // sound hacks....
        switch (st)
        {
            case S_PLAY_ATK2:	sfx = sfx_dshtgn; break; // [crispy] fix Doomguy in casting sequence
            case S_POSS_ATK2:	sfx = sfx_pistol; break;
            case S_SPOS_ATK2:	sfx = sfx_shotgn; break;
            case S_VILE_ATK2:	sfx = sfx_vilatk; break;
            case S_SKEL_FIST2:	sfx = sfx_skeswg; break;
            case S_SKEL_FIST4:	sfx = sfx_skepch; break;
            case S_SKEL_MISS2:	sfx = sfx_skeatk; break;
            case S_FATT_ATK8:
            case S_FATT_ATK5:
            case S_FATT_ATK2:	sfx = sfx_firsht; break;
            case S_CPOS_ATK2:
            case S_CPOS_ATK3:
            case S_CPOS_ATK4:	sfx = sfx_shotgn; break;
            case S_TROO_ATK3:	sfx = sfx_claw;   break;
            case S_SARG_ATK2:	sfx = sfx_sgtatk; break;
            case S_BOSS_ATK2:
            case S_BOS2_ATK2:
            case S_HEAD_ATK2:	sfx = sfx_firsht; break;
            case S_SKULL_ATK2:	sfx = sfx_sklatk; break;
            case S_SPID_ATK2:
            case S_SPID_ATK3:	sfx = sfx_shotgn; break;
            case S_BSPI_ATK2:	sfx = sfx_plasma; break;
            case S_CYBER_ATK2:
            case S_CYBER_ATK4:
            case S_CYBER_ATK6:	sfx = sfx_rlaunc; break;
            case S_PAIN_ATK3:	sfx = sfx_sklatk; break;
            default: sfx = 0; break;
        }
		
        if (sfx)
        S_StartSound (NULL, sfx);
    }
	
    if (castframes == 12)
    {
        // go into attack frame
        castattacking = true;

        if (castonmelee)
        caststate=&states[mobjinfo[castorder[castnum].type].meleestate];
        else
        caststate=&states[mobjinfo[castorder[castnum].type].missilestate];

        castonmelee ^= 1;
        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
            caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
            else
            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        }
    }

    if (castattacking)
    {
        if (castframes == 24 ||	caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
        {
            stopattack:
            castattacking = false;
            castframes = 0;
            caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        }
    }

    casttics = caststate->tics;

    if (casttics == -1)
    {
        casttics = 15;
    }
}


//
// F_CastResponder
//

boolean F_CastResponder (event_t *ev)
{
    if (ev->type != ev_keydown)
    {
        return false;
    }

    if (castdeath)
    {
        // already in dying frames
        return true;
    }

    // go into death frame
    castdeath = true;

    // [JN] Jaguar Doom: own casting order. 
    if (gamemission == jaguar)
    caststate = &states[mobjinfo[castorder_jaguar[castnum].type].deathstate];
    else
    caststate = &states[mobjinfo[castorder[castnum].type].deathstate];

    casttics = caststate->tics;
    castframes = 0;
    castattacking = false;

    if (gamemission == jaguar)
    {
        if (mobjinfo[castorder_jaguar[castnum].type].deathsound)
        S_StartSound (NULL, mobjinfo[castorder_jaguar[castnum].type].deathsound);
    }
    else
    {
        if (mobjinfo[castorder[castnum].type].deathsound)
        S_StartSound (NULL, mobjinfo[castorder[castnum].type].deathsound);
    }

    return true;
}


void F_CastPrint (char *text)
{
    int    c;
    int    cx;
    int    w;
    int    width;
    char  *ch;

    // find width
    ch = text;
    width = 0;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        if (english_language)
        w = SHORT (hu_font[c]->width);
        else
        w = SHORT (hu_font_small_rus[c]->width);

        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = text;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        if (english_language)
        {
            w = SHORT (hu_font[c]->width);
            V_DrawShadowedPatchDoom(cx, 180, hu_font[c]);
        }
        else
        {
            w = SHORT (hu_font_small_rus[c]->width);
            V_DrawShadowedPatchDoom(cx, 180, hu_font_small_rus[c]);
        }

        cx+=w;
    }
}


//
// F_CastDrawer
//

void F_CastDrawer (void)
{
    int             lump;
    boolean         flip;
    patch_t        *patch;
    spritedef_t    *sprdef;
    spriteframe_t  *sprframe;

    // erase the entire screen to a background
    if (!english_language && logical_gamemission == pack_plut)
    V_DrawPatchFullScreen (W_CacheLumpName (DEH_String("BOSSBACP"), PU_CACHE), false);
    else
    V_DrawPatchFullScreen (W_CacheLumpName (DEH_String("BOSSBACK"), PU_CACHE), false);

    F_CastPrint (DEH_String(english_language ?
                            castorder[castnum].name :
                            castorder_rus[castnum].name));

    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    patch = W_CacheLumpNum (lump+firstspritelump, PU_CACHE);

    if (flip)
    V_DrawPatchFlipped(origwidth/2, 170, patch);
    else
    V_DrawPatch(origwidth/2, 170, patch);
}


//
// F_DrawPatchCol
//
void F_DrawPatchCol (int x, patch_t *patch, int col)
{
    int        count, f;
    byte      *source;
    byte      *dest;
    byte      *desttop;
    column_t  *column;

    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
    desttop = I_VideoBuffer + x;

    // step through the posts in a column
    while (column->topdelta != 0xff )
    {
        for (f = 0; f <= hires; f++)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta*(screenwidth << hires) + (x * hires) + f;
            count = column->length;
		
            while (count--)
            {
                if (hires)
                {
                    *dest = *source;
                    dest += screenwidth;
                }
                *dest = *source++;
                dest += screenwidth;
            }
        }
    column = (column_t *)(  (byte *)column + column->length + 4 );
    }
}


//
// F_BunnyScroll
//
void F_BunnyScroll (void)
{
    signed int          scrolled;
    int                 x;
    int                 initialShift1;
    int                 initialShift2;
    patch_t*            p1;
    patch_t*            p2;
    char                name[10];
    unsigned int        stage;
    static unsigned int laststage;
    extern boolean      pfub2_replaced;

    p1 = W_CacheLumpName (DEH_String("PFUB2"), PU_LEVEL);
    p2 = W_CacheLumpName (DEH_String("PFUB1"), PU_LEVEL);

    V_MarkRect (0, 0, screenwidth, SCREENHEIGHT);

    // [JN] If we are using standard 320x200 patch for PFUB2, 
    // then use original scrolling code.
    if (pfub2_replaced)
    {
        scrolled = (ORIGWIDTH - ((signed int) finalecount-230)/2);

        if (scrolled > ORIGWIDTH)
        scrolled = ORIGWIDTH;
        if (scrolled < 0)
        scrolled = 0;

        for ( x=0 ; x<ORIGWIDTH  ; x++)
        {
            if (x+scrolled < ORIGWIDTH)
            F_DrawPatchCol (x + wide_delta, p1, x+scrolled);
            else
            F_DrawPatchCol (x + wide_delta, p2, x+scrolled - ORIGWIDTH);
        }
    }
    else
    {
    //[Dasperal] Ненависть
    switch (origwidth) {
        default:
        case 320: // 4:3 and 5:4
            initialShift1 = 120;
            initialShift2 = 120;
            break;
        case 426: // 16:9
            initialShift1 = 28;
            initialShift2 = 106;
            break;
        case 384: // 16:10
            initialShift1 = 112;
            initialShift2 = 64;
            break;
        case 560: // 21:9
            initialShift1 = -240;
            initialShift2 = 240;
            break;
    }

    scrolled = (origwidth - ((signed int) finalecount - 230) / 2);

    if (scrolled > origwidth) scrolled = origwidth;
    //[Dasperal] Hack for 21:9 to escape the crash
    if (origwidth == 560)
    {
        if (scrolled < 240) scrolled = 240;
    }
    else
    {
        if (scrolled < 0) scrolled = 0;
    }

    for (x = 0; x < origwidth; x++) {
        if (x + scrolled < origwidth + initialShift2)
            F_DrawPatchCol(x, p1, x + scrolled + initialShift1);
        else
            F_DrawPatchCol(x, p2, x + scrolled - origwidth - initialShift2);
    }
    }

    if (finalecount < 1130) return;

    if (finalecount < 1180)
    {
        V_DrawShadowedPatchDoom(((ORIGWIDTH - 13 * 8) / 2) + wide_delta, (ORIGHEIGHT - 8 * 8) / 2, 
            W_CacheLumpName(DEH_String(english_language ? "END0" : "RD_END0"), PU_CACHE));

        laststage = 0;
        return;
    }

    stage = (finalecount-1180) / 5;

    if (stage > 6)
    {
        stage = 6;
    }

    if (stage > laststage)
    {
        S_StartSound (NULL, sfx_pistol);
        laststage = stage;
    }

    DEH_snprintf(name, 10, english_language ? "END%i" : "RD_END%i", stage);

    V_DrawShadowedPatchDoom(((ORIGWIDTH - 13 * 8) / 2) + wide_delta, (ORIGHEIGHT - 8 * 8) / 2, 
        W_CacheLumpName (name,PU_CACHE));
}


static void F_ArtScreenDrawer(void)
{
    if (gameepisode == 3)
    {
        // [JN] Clean up wide screen remainings before drawing.
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
        F_BunnyScroll();
    }
    else
    {
        static char *lumpname;

        switch (gameepisode)
        {
            case 1:
            {
                if (gameversion >= exe_ultimate)
                {
                    lumpname = english_language ? "CREDIT" : "CREDITU";
                }
                else
                {
                    lumpname = english_language ? "HELP2" : "HELP2R";
                }
                break;
            }
            case 2:
            {
                lumpname = "VICTORY2";
                break;
            }
            case 4:
            {
                lumpname = "ENDPIC";
                break;
            }
            // [crispy] Sigil
            case 5:
            {
                lumpname = "SIGILEND";
                if (W_CheckNumForName(DEH_String(lumpname)) == -1)
                {
                    return;
                }
                break;
            }
            default:
            {
                return;
            }
        }

        lumpname = DEH_String(lumpname);

        V_DrawPatchFullScreen(W_CacheLumpName(lumpname, PU_CACHE), false);
    }
}


//
// F_Drawer
//
void F_Drawer (void)
{
    switch (finalestage)
    {
        case F_STAGE_CAST:
        F_CastDrawer();
        break;

        case F_STAGE_CAST_JAGUAR:
        F_CastDrawerJaguar();
        break;

        case F_STAGE_TEXT:
        // [JN] Jaguar Doom: using own font writing function.
        gamemission == jaguar ? F_TextWriteJaguar() : F_TextWrite();
        break;

        case F_STAGE_ARTSCREEN:
        F_ArtScreenDrawer();
        break;
    }
}


// =============================================================================
//
// [JN] Jaguar Doom code
//
// =============================================================================

//
// [JN] F_TextWriteJaguar
//
void F_TextWriteJaguar (void)
{
    int         w, cx, cy;
    signed int  count;
    char       *ch;

    // Leaving MAP23, end game. Special background.
    V_DrawPatchFullScreen (W_CacheLumpName (DEH_String("ENDPIC"), PU_CACHE), false);

    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext;

    count = ((signed int) finalecount - 10) / TEXTSPEED;

    if (count < 0)
	count = 0;

    for ( ; count ; count-- )
    {
        int c = *ch++;

        if (!c)
        break;

        if (c == '\n')
        {
            cx = 10;
            cy += 14;
            continue;
        }

        c = c - HU_FONTSTART2;
        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 7;
            continue;
        }

        if (english_language)
        w = SHORT (hu_font_big_eng[c]->width);
        else
        w = SHORT (hu_font_big_rus[c]->width);

        if (cx+w > origwidth)
	    break;

        if (english_language)
        V_DrawShadowedPatchDoom(cx + wide_delta, cy, hu_font_big_eng[c]);
        else
        V_DrawShadowedPatchDoom(cx + wide_delta, cy, hu_font_big_rus[c]);

        cx+=w;
    }
}


//
// [JN] F_CastTickerJaguar
//
void F_CastTickerJaguar (void)
{
    int st;
    int sfx;

    if (--casttics > 0)
    return;			// not time to change state yet

    if (caststate->tics == -1 || caststate->nextstate == S_NULL)
    {
        // switch from deathstate to next monster
        castnum++;
        castdeath = false;

        if (castorder_jaguar[castnum].name == NULL)
        castnum = 0;

        if (mobjinfo[castorder_jaguar[castnum].type].seesound)
        S_StartSound (NULL, mobjinfo[castorder_jaguar[castnum].type].seesound);

        caststate = &states[mobjinfo[castorder_jaguar[castnum].type].seestate];
        castframes = 0;
    }
    else
    {
        // just advance to next state in animation
        if (caststate == &states[S_PLAY_ATK1])
        goto stopattack;	// Oh, gross hack!

        st = caststate->nextstate;
        caststate = &states[st];
        castframes++;

        // sound hacks....
        switch (st)
        {
            case S_PLAY_ATK1:	sfx = sfx_dshtgn; break;
            case S_POSS_ATK2:	sfx = sfx_pistol; break;
            case S_SPOS_ATK2:	sfx = sfx_shotgn; break;

            case S_TROO_ATK3:	sfx = sfx_claw; break;
            case S_SARG_ATK2:	sfx = sfx_sgtatk; break;
            case S_BOSS_ATK2:
            case S_HEAD_ATK2:	sfx = sfx_firsht; break;
            case S_SKULL_ATK2:	sfx = sfx_sklatk; break;

            default: sfx = 0; break;
        }
		
        if (sfx)
        S_StartSound (NULL, sfx);
    }
	
    if (castframes == 12)
    {
        // go into attack frame
        castattacking = true;

        if (castonmelee)
        caststate=&states[mobjinfo[castorder_jaguar[castnum].type].meleestate];
        else
        caststate=&states[mobjinfo[castorder_jaguar[castnum].type].missilestate];

        castonmelee ^= 1;
        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
            caststate = &states[mobjinfo[castorder_jaguar[castnum].type].meleestate];
            else
            caststate = &states[mobjinfo[castorder_jaguar[castnum].type].missilestate];
        }
    }

    if (castattacking)
    {
        if (castframes == 24 ||	caststate == &states[mobjinfo[castorder_jaguar[castnum].type].seestate] )
        {
            stopattack:
            castattacking = false;
            castframes = 0;
            caststate = &states[mobjinfo[castorder_jaguar[castnum].type].seestate];
        }
    }

    casttics = caststate->tics;

    if (casttics == -1)
    casttics = 15;
}


//
// [JN] F_CastPrintJaguar
//
void F_CastPrintJaguar (char *text)
{
    int	     c, cx, w, width;
    char    *ch;

    // find width
    ch = text;
    width = 0;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = tolower(c) - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 10;
            continue;
        }

        if (english_language)
        w = SHORT (hu_font_big_eng[c]->width);
        else
        w = SHORT (hu_font_big_rus[c]->width);
        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = text;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = tolower(c) - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 10;
            continue;
        }

        if (english_language)
        {
            w = SHORT (hu_font_big_eng[c]->width);
            V_DrawShadowedPatchDoom(cx, 15, hu_font_big_eng[c]);
        }
        else
        {
            w = SHORT (hu_font_big_rus[c]->width);
            V_DrawShadowedPatchDoom(cx, 15, hu_font_big_rus[c]);
        }

        cx+=w;
    }
}


//
// [JN] F_CastDrawerJaguar
//
void F_CastDrawerJaguar (void)
{
    spritedef_t     *sprdef;
    spriteframe_t   *sprframe;
    int              lump;
    patch_t         *patch;

    // erase the entire screen to a background
    V_DrawPatchFullScreen (W_CacheLumpName (DEH_String("VICPIC"), PU_CACHE), false);

    F_CastPrintJaguar (DEH_String(english_language ?
                                  castorder_jaguar[castnum].name :
                                  castorder_jaguar_russian[castnum].name));
    
    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];
    patch = W_CacheLumpNum (lump+firstspritelump, PU_CACHE);

    V_DrawPatchFinale(80 + (wide_delta/2), 90, patch);
}
