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
// F_finale.c



#include <ctype.h>

#include "doomdef.h"
#include "deh_str.h"
#include "i_swap.h"
#include "i_video.h"
#include "s_sound.h"
#include "v_video.h"
#include "jn.h"


int finalestage;                // 0 = text, 1 = art screen
int finalecount;

#define TEXTSPEED       3
#define TEXTWAIT        250

char *finaletext;
char *finaleflat;

int FontABaseLump;
int FontFBaseLump;
// [JN] F_DemonScroll values, should be reinitialized 
// in F_StartFinale for repeated scrolling.
static int yval;
static int nextscroll;

static boolean reset_scroll_delay;
extern boolean viewactive;

extern void D_StartTitle(void);

/*
=======================
=
= F_StartFinale
=
=======================
*/

void F_StartFinale(void)
{
    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;
    players[consoleplayer].messageTics = 1;
    players[consoleplayer].message = NULL;

    switch (gameepisode)
    {
        case 1:
            finaleflat = DEH_String("FLOOR25");
            finaletext = DEH_String(english_language ? E1TEXT : E1TEXT_RUS);
            break;
        case 2:
            finaleflat = DEH_String("FLATHUH1");
            finaletext = DEH_String(english_language ? E2TEXT : E2TEXT_RUS);
            break;
        case 3:
            finaleflat = DEH_String("FLTWAWA2");
            finaletext = DEH_String(english_language ? E3TEXT : E3TEXT_RUS);
            break;
        case 4:
            finaleflat = DEH_String("FLOOR28");
            finaletext = DEH_String(english_language ? E4TEXT : E4TEXT_RUS);
            break;
        case 5:
            finaleflat = DEH_String("FLOOR08");
            finaletext = DEH_String(english_language ? E5TEXT : E5TEXT_RUS);
            break;
    }

    finalestage = 0;
    finalecount = 0;
    FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
    FontFBaseLump = W_GetNumForName(DEH_String("FONTF_S")) + 1;
    yval = 0;
    nextscroll = 0;
    reset_scroll_delay = false;

    S_StartSong(mus_cptd, true, false);
}



boolean F_Responder(event_t * event)
{
    if (event->type != ev_keydown)
    {
        return false;
    }
    if (finalestage == 1 && gameepisode == 2)
    {   // we're showing the water pic, make any key kick to demo mode
        finalestage++;

        return true;
    }
    return false;
}


/*
=======================
=
= F_Ticker
=
=======================
*/

void F_Ticker(void)
{
    // [JN] If we are in single player mode, allow double skipping for 
    // intermission text. First skip printing all intermission text,
    // second is advancing to the next state.
    if (singleplayer && !vanillaparm)
    {
        // [JN] Make PAUSE working properly on text screen.
        if (paused && finalestage == 0)
        {
            return;
        }

        // [JN] Check for skipping. Allow double-press skiping, but don't skip immediately.
        if (finalecount > 10)
        {
            // [JN] Don't allow to skip by pressing "pause" button.
            if (players[consoleplayer].cmd.buttons == (BT_SPECIAL | BTS_PAUSE))
            {
                return;
            }

            // [JN] Double-skip by pressing "attack" button.
            if (players[consoleplayer].cmd.buttons & BT_ATTACK && !menuactive && finalestage != 1)
            {
                if (!players[consoleplayer].attackdown)
                {
                    if (finalecount >= 5003)
                    {
                        finalestage = 1;
                    }
                    finalecount += 5000;
                    players[consoleplayer].attackdown = true;
                }
                players[consoleplayer].attackdown = true;
            }
            else
            {
                players[consoleplayer].attackdown = false;
            }
    
            // [JN] Double-skip by pressing "use" button.
            if (players[consoleplayer].cmd.buttons & BT_USE && !menuactive && finalestage != 1)
            {
                if (!players[consoleplayer].usedown)
                {
                    if (finalecount >= 5003 && !finalestage)
                    {
                        finalestage = 1;
                    }
                    finalecount += 5000;
                    players[consoleplayer].usedown = true;
                }
                players[consoleplayer].usedown = true;
            
            }
            else
            {
                players[consoleplayer].usedown = false;
            }
        }

        // [JN] Advance animation.
        finalecount++;
    }
    //
    // [JN] Standard Heretic routine, safe for network game and demos.
    //    
    else
    {
        finalecount++;

        if (!finalestage && finalecount > strlen(finaletext) * TEXTSPEED + TEXTWAIT)
        {
            finalecount = 0;
            if (!finalestage)
            {
                finalestage = 1;
            }
        }
    }
}


/*
=======================
=
= F_TextWrite
=
=======================
*/

void F_TextWrite(void)
{
    int count;
    char *ch;
    int c;
    int cx, cy;
    patch_t *w;

//
// erase the entire screen to a tiled background
//
    // [JN] Use unified function.
    V_FillFlat(finaleflat);
//
// draw some of the text onto the screen
//
    cx = 20;
    cy = 5;
    ch = finaletext;

    count = (finalecount - 10) / TEXTSPEED;
    if (count < 0)
        count = 0;
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = 20;
            cy += 9;
            continue;
        }

        c = toupper(c);
        if (c < 33)
        {
            cx += 5;
            continue;
        }

        w = W_CacheLumpNum((FontABaseLump) + c - 33, PU_CACHE);

        if (cx + SHORT(w->width) > screenwidth)
            break;
        V_DrawShadowedPatchRaven(cx + wide_delta, cy, w);
        cx += SHORT(w->width);
    }
}

//---------------------------------------------------------------------------
//
// F_TextWriteRUS
//
//---------------------------------------------------------------------------

void F_TextWriteRUS(void)
{
    byte *src, *dest;
    int x, y;
    int count;
    char *ch;
    int c;
    int cx, cy;
    patch_t *w;

//
// erase the entire screen to a tiled background
//
    src = W_CacheLumpName(finaleflat, PU_CACHE);
    dest = I_VideoBuffer;
    for (y = 0; y < SCREENHEIGHT; y++)
    {
        for (x = 0; x < screenwidth / 64; x++)
        {
            memcpy(dest, src + ((y & 63) << 6), 64);
            dest += 64;
        }
        if (screenwidth & 63)
        {
            memcpy(dest, src + ((y & 63) << 6), screenwidth & 63);
            dest += (screenwidth & 63);
        }
    }

//
// draw some of the text onto the screen
//
    cx = 20;
    cy = 5;
    ch = finaletext;

    count = (finalecount - 10) / TEXTSPEED;
    if (count < 0)
        count = 0;
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
            break;
        if (c == '\n')
        {
            cx = 20;
            cy += 9;
            continue;
        }

        c = toupper(c);
        if (c < 33)
        {
            cx += 5;
            continue;
        }

        w = W_CacheLumpNum((FontFBaseLump) + c - 33, PU_CACHE);

        if (cx + SHORT(w->width) > screenwidth)
            break;
        V_DrawShadowedPatchRaven(cx + wide_delta, cy, w);
        cx += SHORT(w->width);
    }
}

/*
==================
=
= F_DemonScroll
=
==================
*/

void F_DemonScroll(void)
{
    byte *p1;
    byte *p2;

    if (!reset_scroll_delay)
    {
        // [JN] Reset finalecount to 0 after text screen has been skipped.
        finalecount = 0;
        reset_scroll_delay = true;
    }

    if (finalecount < nextscroll)
    {
        return;
    }
    p1 = W_CacheLumpName(DEH_String("FINAL1"), PU_LEVEL);
    p2 = W_CacheLumpName(DEH_String("FINAL2"), PU_LEVEL);

    if (screenwidth != ORIGWIDTH)
    {
        // [JN] Clean up remainings of the wide screen before drawing.
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    if (((patch_t*) p1)->width == 560)
    {
        // Scroll patches
        int x = ((screenwidth >> hires) - SHORT(((patch_t*) p1)->width)) / 2;
        if (finalecount < 70)
        {
            V_DrawPatchFullScreen((patch_t*) p1, false);
            nextscroll = finalecount;
            return;
        }
        if (yval < 200)
        {
            V_DrawPatch(x, -((patch_t*) p2)->height + yval, (patch_t*) p2);
            V_DrawPatch(x, yval, (patch_t*) p1);
            if (!paused)
            {
                yval++;
                nextscroll = finalecount + 3;
            }
            
        }
        else
        {   //else, we'll just sit here and wait, for now
            V_DrawPatchFullScreen((patch_t*) p2, false);
        }
    }
    else
    {
        // Scroll Raws
        if (finalecount < 70)
        {
            V_CopyScaledBuffer(I_VideoBuffer, p1, ORIGHEIGHT * ORIGWIDTH);
            nextscroll = finalecount;
            return;
        }
        if (yval < 64000)
        {
            V_CopyScaledBuffer(I_VideoBuffer,
                               p2 + ORIGHEIGHT * ORIGWIDTH - yval, yval);
            V_CopyScaledBuffer(I_VideoBuffer + screenwidth * ((yval / ORIGWIDTH) << hires),
                               p1, ORIGHEIGHT * ORIGWIDTH - yval);
            if (!paused)
            {
                yval += ORIGWIDTH;
                nextscroll = finalecount + 3;
            }
        }
        else
        {   //else, we'll just sit here and wait, for now
            V_CopyScaledBuffer(I_VideoBuffer, p2, ORIGWIDTH * ORIGHEIGHT);
        }
    }
}

/*
==================
=
= F_DrawUnderwater
=
==================
*/

void F_DrawUnderwater(void)
{
    static boolean underwawa = false;
    extern boolean askforquit;
    char *lumpname;
    byte *palette;
    patch_t *e2end = W_CacheLumpName("E2END", PU_CACHE);
    patch_t *title = W_CacheLumpName("TITLE", PU_CACHE);

    // The underwater screen has its own palette, which is rather annoying.
    // The palette doesn't correspond to the normal palette. Because of
    // this, we must regenerate the lookup tables used in the video scaling
    // code.

    switch (finalestage)
    {
        case 1:
            if (!underwawa)
            {
                underwawa = true;
                V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
                lumpname = DEH_String("E2PAL");
                palette = W_CacheLumpName(lumpname, PU_STATIC);
                I_SetPalette(palette);
                W_ReleaseLumpName(lumpname);
                
                if (e2end->width == 560)
                V_DrawPatchFullScreen(e2end, false);
                else
                V_DrawRawScreen(W_CacheLumpName(DEH_String("E2END"), PU_CACHE));
            }
            paused = false;
            menuactive = false;
            askforquit = false;
        break;
        case 2:
            if (underwawa)
            {
                lumpname = DEH_String("PLAYPAL");
                palette = W_CacheLumpName(lumpname, PU_STATIC);
                I_SetPalette(palette);
                W_ReleaseLumpName(lumpname);
                underwawa = false;
            }

            // [JN] Clean up remainings of the wide screen before 
            // drawing a TITLE screen.
            V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);

            if (english_language)
            {
                if (gamemode == retail && title->width == 560)
                V_DrawPatchFullScreen(W_CacheLumpName("TITLE", PU_CACHE), false);
                else if (gamemode == registered || gamemode == shareware)
                V_DrawPatchFullScreen(W_CacheLumpName("TITLEOLD", PU_CACHE), false);            
                else
                V_DrawRawScreen(W_CacheLumpName(DEH_String("TITLE"), PU_CACHE));
            }
            else
            {
                V_DrawPatchFullScreen(W_CacheLumpName(gamemode == retail ?
                                                      "TITLE_RT" : "TITLEOLD",
                                                      PU_CACHE), false);

            }
        break;
    }
}

/*
=======================
=
= F_Drawer
=
=======================
*/

void F_Drawer(void)
{
    const patch_t *credit =  W_CacheLumpName("CREDIT", PU_CACHE);

    UpdateState |= I_FULLSCRN;
    if (!finalestage)
    {
        if (english_language)
        F_TextWrite();
        else
        F_TextWriteRUS();
    }
    else
    {
        switch (gameepisode)
        {
            case 1:
                // [JN] Clean up remainings of the wide screen
                V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
                if (gamemode == shareware)
                {
                    V_DrawPatchFullScreen(W_CacheLumpName(english_language ? 
                                          "ORDER" : "ORDER_R", PU_CACHE), false);
                }
                else
                {
                    if (english_language)
                    {
                        if (credit->width == 560)
                        V_DrawPatchFullScreen(W_CacheLumpName("CREDIT", PU_CACHE), false);
                        else
                        V_DrawRawScreen(W_CacheLumpName("CREDIT", PU_CACHE));
                    }
                    else
                    {
                        V_DrawPatchFullScreen(W_CacheLumpName(gamemode == retail ?
                                              "CRED_RT" : "CRED_RG", PU_CACHE), false);
                    }
                }
                break;
            case 2:
                // [JN] No need to cleanup screen here, it is done in F_DrawUnderwater.
                // V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
                F_DrawUnderwater();
                break;
            case 3:
                // [JN] No need to cleanup screen here, it is done in F_DemonScroll.
                // V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
                F_DemonScroll();
                break;
            case 4:            // Just show credits screen for extended episodes
            case 5:
                V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
                if (english_language)
                {
                    if (credit->width == 560)
                    V_DrawPatchFullScreen(W_CacheLumpName("CREDIT", PU_CACHE), false);
                    else
                    V_DrawRawScreen(W_CacheLumpName("CREDIT", PU_CACHE));
                }
                else
                {
                    V_DrawPatchFullScreen(W_CacheLumpName(gamemode == retail ?
                                                    "CRED_RT" : "CRED_RG", PU_CACHE), false);
                }
                break;
        }
    }
}
