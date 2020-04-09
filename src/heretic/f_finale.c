//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julian Nechaevsky
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

extern int screenblocks;

extern boolean automapactive;
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

//      S_ChangeMusic(mus_victor, true);
    S_StartSong(mus_cptd, true);
}



boolean F_Responder(event_t * event)
{
    if (event->type != ev_keydown)
    {
        return false;
    }
    if (finalestage == 1 && gameepisode == 2)
    {                           // we're showing the water pic, make any key kick to demo mode
        finalestage++;
        /*
        memset((byte *) 0xa0000, 0, SCREENWIDTH * SCREENHEIGHT);
        memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);
        I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
        */
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
    // [JN] Make PAUSE working properly on text screen
    if (paused)
    {
        return;
    }

    finalecount++;
    if (!finalestage
        && finalecount > strlen(finaletext) * TEXTSPEED + TEXTWAIT)
    {
        finalecount = 0;
        if (!finalestage)
        {
            finalestage = 1;
        }

//              wipegamestate = -1;             // force a wipe
/*
		if (gameepisode == 3)
			S_StartMusic (mus_bunny);
*/
    }
}


/*
=======================
=
= F_TextWrite
=
=======================
*/

//#include "hu_stuff.h"
//extern        patch_t *hu_font[HU_FONTSIZE];

void F_TextWrite(void)
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

//      V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);

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

    // [JN] Wide screen: draw black borders in emulated 4:3 mode.
    if (widescreen && screenblocks == 9)
    {
        V_DrawBlackBorders();
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

    // [JN] Wide screen: draw black borders in emulated 4:3 mode.
    if (widescreen && screenblocks == 9)
    {
        V_DrawBlackBorders();
    }
}

void F_DrawPatchCol(int x, patch_t * patch, int col)
{
    column_t *column;
    byte *source, *dest, *desttop;
    int count;

    column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));
    desttop = I_VideoBuffer + x;

// step through the posts in a column

    while (column->topdelta != 0xff)
    {
        source = (byte *) column + 3;
        dest = desttop + column->topdelta * screenwidth;
        count = column->length;

        while (count--)
        {
            *dest = *source++;
            dest += screenwidth;
        }
        column = (column_t *) ((byte *) column + column->length + 4);
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
    // [JN] TODO - fix scrolling for new V_CopyScaledBuffer
    // byte *p1;
    byte *p2;
    // static int yval = 0;
    static int nextscroll = 0;

    if (finalecount < nextscroll)
    {
        return;
    }
    // p1 = W_CacheLumpName(DEH_String("FINAL1"), PU_LEVEL);
    p2 = W_CacheLumpName(DEH_String("FINAL2"), PU_LEVEL);

    // [JN] Only single static picture for now.
    V_CopyScaledBuffer(I_VideoBuffer, p2, SRCWIDTH * ORIGHEIGHT);

    /*
    if (finalecount < 70)
    {
        V_CopyScaledBuffer(I_VideoBuffer, p1, ORIGHEIGHT * ORIGWIDTH);
        nextscroll = finalecount;
        return;
    }
    if (yval < 64000)
    {
        V_CopyScaledBuffer(I_VideoBuffer, p2 + ORIGHEIGHT * ORIGWIDTH - yval, yval);
        V_CopyScaledBuffer(I_VideoBuffer + (yval << (2 * hires)), p1, ORIGHEIGHT * ORIGWIDTH - yval);
        yval += ORIGWIDTH;
        nextscroll = finalecount + 3;
    }
    else
    {                           //else, we'll just sit here and wait, for now
        V_CopyScaledBuffer(I_VideoBuffer, p2, ORIGWIDTH * ORIGHEIGHT);
    }
    */
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
                lumpname = DEH_String(usegamma <= 8 ?
                                      "E2PALFIX" :
                                      "E2PAL");
                palette = W_CacheLumpName(lumpname, PU_STATIC);
                I_SetPalette(palette);
                W_ReleaseLumpName(lumpname);
                V_DrawRawScreen(W_CacheLumpName(DEH_String("E2END"), PU_CACHE));
            }
            paused = false;
            menuactive = false;
            askforquit = false;

            break;
        case 2:
            if (underwawa)
            {
                lumpname = DEH_String(usegamma <= 8 ?
                                      "PALFIX" :
                                      "PLAYPAL");
                palette = W_CacheLumpName(lumpname, PU_STATIC);
                I_SetPalette(palette);
                W_ReleaseLumpName(lumpname);
                underwawa = false;
            }

            if (english_language)
            {
                V_DrawRawScreen(W_CacheLumpName(DEH_String("TITLE"), PU_CACHE));
            }
            else
            {
                if (gamemode == retail)
                V_DrawRawScreen(W_CacheLumpName(DEH_String("TITLE_RT"), PU_CACHE));
                else
                V_DrawRawScreen(W_CacheLumpName(DEH_String("TITLE"), PU_CACHE));
            }
            //D_StartTitle(); // go to intro/demo mode.
    }
}


#if 0
/*
==================
=
= F_BunnyScroll
=
==================
*/

void F_BunnyScroll(void)
{
    int scrolled, x;
    patch_t *p1, *p2;
    char name[10];
    int stage;
    static int laststage;

    p1 = W_CacheLumpName("PFUB2", PU_LEVEL);
    p2 = W_CacheLumpName("PFUB1", PU_LEVEL);

    V_MarkRect(0, 0, screenwidth, SCREENHEIGHT);

    scrolled = 320 - (finalecount - 230) / 2;
    if (scrolled > 320)
        scrolled = 320;
    if (scrolled < 0)
        scrolled = 0;

    for (x = 0; x < screenwidth; x++)
    {
        if (x + scrolled < 320)
            F_DrawPatchCol(x, p1, x + scrolled);
        else
            F_DrawPatchCol(x, p2, x + scrolled - 320);
    }

    if (finalecount < 1130)
        return;
    if (finalecount < 1180)
    {
        V_DrawPatch((screenwidth - 13 * 8) / 2, (SCREENHEIGHT - 8 * 8) / 2, 0,
                    W_CacheLumpName("END0", PU_CACHE));
        laststage = 0;
        return;
    }

    stage = (finalecount - 1180) / 5;
    if (stage > 6)
        stage = 6;
    if (stage > laststage)
    {
        S_StartSound(NULL, sfx_pistol);
        laststage = stage;
    }

    M_snprintf(name, sizeof(name), "END%i", stage);
    V_DrawPatch((screenwidth - 13 * 8) / 2, (SCREENHEIGHT - 8 * 8) / 2,
                W_CacheLumpName(name, PU_CACHE));
}
#endif

/*
=======================
=
= F_Drawer
=
=======================
*/

void F_Drawer(void)
{
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
                    V_DrawRawScreen(W_CacheLumpName(english_language ? 
                                                    "ORDER" : "ORDER_R", PU_CACHE));
                }
                else
                {
                    if (english_language)
                    V_DrawRawScreen(W_CacheLumpName("CREDIT", PU_CACHE));
                    else
                    V_DrawRawScreen(W_CacheLumpName(gamemode == retail ?
                                                    "CRED_RT" : "CRED_RG", PU_CACHE));
                }
                break;
            case 2:
                // [JN] No need to cleanup screen here, it is done in F_DrawUnderwater.
                // V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
                F_DrawUnderwater();
                break;
            case 3:
                // [JN] Clean up remainings of the wide screen
                V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
                F_DemonScroll();
                break;
            case 4:            // Just show credits screen for extended episodes
            case 5:
                V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
                if (english_language)
                V_DrawRawScreen(W_CacheLumpName("CREDIT", PU_CACHE));
                else
                V_DrawRawScreen(W_CacheLumpName(gamemode == retail ?
                                                "CRED_RT" : "CRED_RG", PU_CACHE));
                break;
        }
    }
}
