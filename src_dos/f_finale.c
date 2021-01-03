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
//	Game completion, final screen animation.
//

#include <ctype.h>


#include "i_system.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "sounds.h"
#include "doomstat.h"
#include "r_state.h"
#include "rd_lang.h"
#include "jn.h"

// ?
//#include "doomstat.h"
//#include "r_local.h"
//#include "f_finale.h"

// Stage of animation:
//  0 = text, 1 = art screen, 2 = character cast
int finalestage;

int finalecount;

#define TEXTSPEED   3
#define TEXTWAIT    250

// [JN] English texts
char*   e1text = E1TEXT;
char*   e2text = E2TEXT;
char*   e3text = E3TEXT;
char*   e4text = E4TEXT;
char*   e5text = E5TEXT; // [crispy] Sigil

char*   c1text = C1TEXT;
char*   c2text = C2TEXT;
char*   c3text = C3TEXT;
char*   c4text = C4TEXT;
char*   c5text = C5TEXT;
char*   c6text = C6TEXT;

char*   p1text = P1TEXT;
char*   p2text = P2TEXT;
char*   p3text = P3TEXT;
char*   p4text = P4TEXT;
char*   p5text = P5TEXT;
char*   p6text = P6TEXT;

char*   t1text = T1TEXT;
char*   t2text = T2TEXT;
char*   t3text = T3TEXT;
char*   t4text = T4TEXT;
char*   t5text = T5TEXT;
char*   t6text = T6TEXT;

// [JN] Russian texts
char*   e1text_rus = E1TEXT_RUS;
char*   e2text_rus = E2TEXT_RUS;
char*   e3text_rus = E3TEXT_RUS;
char*   e4text_rus = E4TEXT_RUS;
char*   e5text_rus = E5TEXT_RUS; // [JN] Sigil

char*   c1text_rus = C1TEXT_RUS;
char*   c2text_rus = C2TEXT_RUS;
char*   c3text_rus = C3TEXT_RUS;
char*   c4text_rus = C4TEXT_RUS;
char*   c5text_rus = C5TEXT_RUS;
char*   c6text_rus = C6TEXT_RUS;

char*   p1text_rus = P1TEXT_RUS;
char*   p2text_rus = P2TEXT_RUS;
char*   p3text_rus = P3TEXT_RUS;
char*   p4text_rus = P4TEXT_RUS;
char*   p5text_rus = P5TEXT_RUS;
char*   p6text_rus = P6TEXT_RUS;

char*   t1text_rus = T1TEXT_RUS;
char*   t2text_rus = T2TEXT_RUS;
char*   t3text_rus = T3TEXT_RUS;
char*   t4text_rus = T4TEXT_RUS;
char*   t5text_rus = T5TEXT_RUS;
char*   t6text_rus = T6TEXT_RUS;

char*   finaletext;
char*   finaleflat;

void    F_StartCast (void);
void    F_CastTicker (void);
boolean F_CastResponder (event_t *ev);
void    F_CastDrawer (void);


//
// F_StartFinale
//
void F_StartFinale (void)
{
    int     finalemusic;

    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;

    if(commercial)
    {
        if (plutonia)
        {
            switch (gamemap)
            {
                case 6:
                finaleflat = "SLIME16";
                finaletext = english_language ? p1text : p1text_rus;
                break;

                case 11:
                finaleflat = "RROCK14";
                finaletext = english_language ? p2text : p2text_rus;
                break;

                case 20:
                finaleflat = "RROCK07";
                finaletext = english_language ? p3text : p3text_rus;
                break;

                case 30:
                finaleflat = "RROCK17";
                finaletext = english_language ? p4text : p4text_rus;
                break;

                case 15:
                finaleflat = "RROCK13";
                finaletext = english_language ? p5text : p5text_rus;
                break;
                
                case 31:
                finaleflat = "RROCK19";
                finaletext = english_language ? p6text : p6text_rus;
                break;

                default:
                // Ouch.
                break;
            }
        }
        else if (tnt)
        {
            switch (gamemap)
            {
                case 6:
                finaleflat = "SLIME16";
                finaletext = english_language ? t1text : t1text_rus;
                break;

                case 11:
                finaleflat = "RROCK14";
                finaletext = english_language ? t2text : t2text_rus;
                break;

                case 20:
                finaleflat = "RROCK07";
                finaletext = english_language ? t3text : t3text_rus;
                break;

                case 30:
                finaleflat = "RROCK17";
                finaletext = english_language ? t4text : t4text_rus;
                break;

                case 15:
                finaleflat = "RROCK13";
                finaletext = english_language ? t5text : t5text_rus;
                break;

                case 31:
                finaleflat = "RROCK19";
                finaletext = english_language ? t6text : t6text_rus;
                break;

                default:
                // Ouch.
                break;
            }
        }
        else
        {
            // DOOM II and missions packs with E1, M34
            switch (gamemap)
            {
                case 6:
                finaleflat = "SLIME16";
                finaletext = english_language ? c1text : c1text_rus;
                break;

                case 11:
                finaleflat = "RROCK14";
                finaletext = english_language ? c2text : c2text_rus;
                break;

                case 20:
                finaleflat = "RROCK07";
                finaletext = english_language ? c3text : c3text_rus;
                break;

                case 30:
                finaleflat = "RROCK17";
                finaletext = english_language ? c4text : c4text_rus;
                break;

                case 15:
                finaleflat = "RROCK13";
                finaletext = english_language ? c5text : c5text_rus;
                break;

                case 31:
                finaleflat = "RROCK19";
                finaletext = english_language ? c6text : c6text_rus;
                break;

                default:
                // Ouch.
                break;
            }
        }
        finalemusic = mus_read_m;
    }
    else
    {
        // DOOM 1 - E1, E3 or E4, but each nine missions
        switch (gameepisode)
        {
            case 1:
            finaleflat = "FLOOR4_8";
            finaletext = english_language ? e1text : e1text_rus;
            break;

            case 2:
            finaleflat = "SFLR6_1";
            finaletext = english_language ? e2text : e2text_rus;
            break;

            case 3:
            finaleflat = "MFLR8_4";
            finaletext = english_language ? e3text : e3text_rus;
            break;

            case 4:
            finaleflat = "MFLR8_3";
            finaletext = english_language ? e4text : e4text_rus;
            break;

            // [JN] Sigil
            case 5:
            finaleflat = "FLOOR7_2";
            finaletext = english_language ? e5text : e5text_rus;
            break;

            default:
            // Ouch.
            break;
        }
        finalemusic = mus_victor;
    }

    S_ChangeMusic(finalemusic, true);
    finalestage = 0;
    finalecount = 0;
}


boolean F_Responder (event_t *event)
{
    if (finalestage == 2)
    return F_CastResponder (event);

    return false;
}


//
// F_Ticker
//
void F_Ticker (void)
{
    int	    i;

    // [JN] Make PAUSE working properly on text screen
    if (paused)
    {
        return;
    }

    // check for skipping
    if ((commercial) && (finalecount > 50))
    {
        // go on to the next level
        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            // [JN] Pressing PAUSE should not skip text screen
            if (players[i].cmd.buttons && !(players->cmd.buttons & BTS_PAUSE))
            {
                break;
            }
        }

        if (i < MAXPLAYERS)
        {	
            if (gamemap == 30)
            F_StartCast ();
            else
            gameaction = ga_worlddone;
        }
    }

    // advance animation
    finalecount++;

    if (finalestage == 2)
    {
        F_CastTicker ();
        return;
    }

    if (commercial)
    return;

    if (!finalestage && finalecount>strlen (finaletext)*TEXTSPEED + TEXTWAIT)
    {
        finalecount = 0;
        finalestage = 1;
        wipegamestate = -1;		// force a wipe

        if (gameepisode == 3)
        S_StartMusic (mus_bunny);
    }
}


//
// F_TextWrite
//
#include "hu_stuff.h"
extern patch_t *hu_font[HU_FONTSIZE];
extern patch_t *hu_font_small_rus[HU_FONTSIZE];

void F_TextWrite (void)
{
    byte*   src;
    byte*   dest;

    int     x,y,w;
    int     count;
    char*   ch;
    int     c;
    int     cx;
    int     cy;

    // erase the entire screen to a tiled background
    src = W_CacheLumpName ( finaleflat , PU_CACHE);
    dest = screens[0];

    for (y=0 ; y<SCREENHEIGHT ; y++)
    {
        for (x=0 ; x<SCREENWIDTH/64 ; x++)
        {
            memcpy (dest, src+((y&63)<<6), 64);
            dest += 64;
        }

        // [JN] Watcom C - Unreachable code. No need to keep it, since
        // screen width never changes. Thanks AXDOOMER for explanation.
        /*
        if (SCREENWIDTH&63)
        {
            memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63);
            dest += (SCREENWIDTH&63);
        }
        */
    }

    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);

    // [JN] Draw special background on entering Wolfenstein and Grosse levels
    if ((commercial && !plutonia && !tnt) && !vanilla)
    {
        if (gamemap == 15)  // Leaving MAP15, entering MAP31
        V_DrawPatch (0,0,0,W_CacheLumpName("WLFBACK1",PU_CACHE));

        if (gamemap == 31)  // Leaving MAP31, entering MAP32
        V_DrawPatch (0,0,0,W_CacheLumpName("WLFBACK2",PU_CACHE));
    }

    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext;

    count = (finalecount - 10)/TEXTSPEED;
    if (count < 0)
    count = 0;

    for ( ; count ; count-- )
    {
        c = *ch++;
        if (!c)
        break;

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

        w = SHORT (english_language ? 
                   hu_font[c]->width : hu_font_small_rus[c]->width);

        if (cx+w > SCREENWIDTH)
        break;

        V_DrawShadowedPatch(cx, cy, 0, english_language ?
                                       hu_font[c] : hu_font_small_rus[c]);

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

int         castnum;
int         casttics;
state_t*    caststate;
boolean		castdeath;
int         castframes;
int         castonmelee;
boolean     castattacking;


//
// F_StartCast
//
extern gamestate_t wipegamestate;

void F_StartCast (void)
{
    wipegamestate = -1; // force a screen wipe
    castnum = 0;
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    casttics = caststate->tics;
    castdeath = false;
    finalestage = 2;	
    castframes = 0;
    castonmelee = 0;
    castattacking = false;
    S_ChangeMusic(mus_evil, true);
}


//
// F_CastTicker
//
void F_CastTicker (void)
{
    int    st;
    int    sfx;
	
    if (--casttics > 0)
    return; // not time to change state yet

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
        // just advance to next state in animation
        if (caststate == &states[S_PLAY_ATK1])
        goto stopattack;	// Oh, gross hack!

        st = caststate->nextstate;
        caststate = &states[st];
        castframes++;

        // sound hacks....
        switch (st)
        {
            case S_PLAY_ATK1:   sfx = sfx_dshtgn; break;
            case S_POSS_ATK2:   sfx = sfx_pistol; break;
            case S_SPOS_ATK2:   sfx = sfx_shotgn; break;
            case S_VILE_ATK2:   sfx = sfx_vilatk; break;
            case S_SKEL_FIST2:  sfx = sfx_skeswg; break;
            case S_SKEL_FIST4:  sfx = sfx_skepch; break;
            case S_SKEL_MISS2:  sfx = sfx_skeatk; break;
            case S_FATT_ATK8:
            case S_FATT_ATK5:
            case S_FATT_ATK2:   sfx = sfx_firsht; break;
            case S_CPOS_ATK2:
            case S_CPOS_ATK3:
            case S_CPOS_ATK4:   sfx = sfx_shotgn; break;
            case S_TROO_ATK3:   sfx = sfx_claw; break;
            case S_SARG_ATK2:   sfx = sfx_sgtatk; break;
            case S_BOSS_ATK2:
            case S_BOS2_ATK2:
            case S_HEAD_ATK2:   sfx = sfx_firsht; break;
            case S_SKULL_ATK2:  sfx = sfx_sklatk; break;
            case S_SPID_ATK2:
            case S_SPID_ATK3:   sfx = sfx_shotgn; break;
            case S_BSPI_ATK2:   sfx = sfx_plasma; break;
            case S_CYBER_ATK2:
            case S_CYBER_ATK4:
            case S_CYBER_ATK6:  sfx = sfx_rlaunc; break;
            case S_PAIN_ATK3:   sfx = sfx_sklatk; break;
            default:            sfx = 0; break;
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
    casttics = 15;
}


//
// F_CastResponder
//

boolean F_CastResponder (event_t* ev)
{
    if (ev->type != ev_keydown)
    return false;

    if (castdeath)
	return true; // already in dying frames

    // go into death frame
    castdeath = true;
    caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
    casttics = caststate->tics;
    castframes = 0;
    castattacking = false;

    if (mobjinfo[castorder[castnum].type].deathsound)
    S_StartSound (NULL, mobjinfo[castorder[castnum].type].deathsound);

    return true;
}

void F_CastPrint (char* text)
{
    char*   ch;
    int     c;
    int     cx;
    int     w;
    int     width;

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
        {
            w = SHORT (hu_font[c]->width);
        }
        else
        {
            w = SHORT (hu_font_small_rus[c]->width);
        }
        width += w;
    }

    // draw it
    cx = 160-width/2;
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

        w = SHORT (english_language ?
                   hu_font[c]->width : hu_font_small_rus[c]->width);

        V_DrawShadowedPatch(cx, 180, 0, english_language ?
                                        hu_font[c] : hu_font_small_rus[c]);

        cx+=w;
    }
}


//
// F_CastDrawer
//
void V_DrawPatchFlipped (int x, int y, int scrn, patch_t *patch);

void F_CastDrawer (void)
{
    spritedef_t*    sprdef;
    spriteframe_t*  sprframe;
    int             lump;
    boolean         flip;
    patch_t*        patch;

    // erase the entire screen to a background
    // [JN] Plutonia using translated final screen.
    V_DrawPatch (0,0,0, W_CacheLumpName (!english_language && plutonia ?
                                         "BOSSBACP" : "BOSSBACK", PU_CACHE));

    F_CastPrint (english_language ? 
                 castorder[castnum].name : castorder_rus[castnum].name);

    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    patch = W_CacheLumpNum (lump+firstspritelump, PU_CACHE);
    if (flip)
    V_DrawPatchFlipped (160,170,0,patch);
    else
    V_DrawPatch (160,170,0,patch);
}


//
// F_DrawPatchCol
//
void
F_DrawPatchCol (int x, patch_t* patch, int col)
{
    column_t*   column;
    byte*       source;
    byte*       dest;
    byte*       desttop;
    int     count;

    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
    desttop = screens[0]+x;

    // step through the posts in a column
    while (column->topdelta != 0xff )
    {
        source = (byte *)column + 3;
        dest = desttop + column->topdelta*SCREENWIDTH;
        count = column->length;
		
        while (count--)
        {
            *dest = *source++;
            dest += SCREENWIDTH;
        }

    column = (column_t *)(  (byte *)column + column->length + 4 );
    }
}


//
// F_BunnyScroll
//
void F_BunnyScroll (void)
{
    int         scrolled;
    int         x;
    patch_t*    p1;
    patch_t*    p2;
    char        name[10];
    int         stage;
    static int  laststage;

    p1 = W_CacheLumpName ("PFUB2", PU_LEVEL);
    p2 = W_CacheLumpName ("PFUB1", PU_LEVEL);

    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);

    scrolled = 320 - (finalecount-230)/2;
    if (scrolled > 320)
    scrolled = 320;
    if (scrolled < 0)
    scrolled = 0;

    for ( x=0 ; x<SCREENWIDTH ; x++)
    {
        if (x+scrolled < 320)
        F_DrawPatchCol (x, p1, x+scrolled);
        else
        F_DrawPatchCol (x, p2, x+scrolled - 320);		
    }

    if (finalecount < 1130)
    return;

    if (finalecount < 1180)
    {
        V_DrawShadowedPatch((SCREENWIDTH-13*8)/2,
                            (SCREENHEIGHT-8*8)/2,0,
                             W_CacheLumpName (english_language ?
                                              "END0" : "RD_END0", PU_CACHE));

        laststage = 0;
        return;
    }

    stage = (finalecount-1180) / 5;

    if (stage > 6)
    stage = 6;

    if (stage > laststage)
    {
        S_StartSound (NULL, sfx_pistol);
        laststage = stage;
    }

    sprintf (name, english_language ? "END%i" : "RD_END%i",stage);

    V_DrawShadowedPatch((SCREENWIDTH-13*8)/2,
                        (SCREENHEIGHT-8*8)/2,0,
                         W_CacheLumpName (name,PU_CACHE));
}


//
// F_Drawer
//
void F_Drawer (void)
{
    if (finalestage == 2)
    {
        F_CastDrawer();
        return;
    }

    if (!finalestage)
    F_TextWrite ();
    else
    {
        switch (gameepisode)
        {
            case 1:
            if (english_language)
            {
                V_DrawPatch(0,0,0,W_CacheLumpName("CREDIT",PU_CACHE));
            }
            else
            {
                V_DrawPatch(0,0,0,W_CacheLumpName
                           (shareware || registered ? 
                            "CREDITS" : "CREDITU", PU_CACHE));
            }
            break;

            case 2:
            V_DrawPatch(0,0,0,
            W_CacheLumpName("VICTORY2",PU_CACHE));
            break;

            case 3:
            F_BunnyScroll ();
            break;

            case 4:
            V_DrawPatch (0,0,0,
            W_CacheLumpName("ENDPIC",PU_CACHE));
            break;

            // [JN] Sigil
            case 5:
            V_DrawPatch (0,0,0,
            W_CacheLumpName("SIGILEND",PU_CACHE));
            break;
        }
    }
}

