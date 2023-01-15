//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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



// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "i_system.h"
#include "i_video.h"
#include "p_local.h"
#include "s_sound.h"
#include <ctype.h>
#include "v_video.h"
#include "i_swap.h"
#include "m_misc.h"
#include "rd_rushexen.h"

// MACROS ------------------------------------------------------------------

#define	TEXTSPEED	3
#define	TEXTWAIT	250

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void TextWrite(void);
static void DrawPic(void);
static void InitializeFade(boolean fadeIn);
static void DeInitializeFade(void);
static void FadePic(void);
static char *GetFinaleText(int sequence);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean viewactive;

// PUBLIC DATA DECLARATIONS ------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int FinaleStage;
static int FinaleCount;
static int FinaleEndCount;
static int FinaleLumpNum;
static int FontABaseLump;
static int FontFBaseLump; // [JN] Small Russian font
static char *FinaleText;
static char *FinaleLumpWide;  // [JN] Wide screen background assets.

static fixed_t *Palette;
static fixed_t *PaletteDelta;
static byte *RealPalette;

// CODE --------------------------------------------------------------------

//===========================================================================
//
// F_StartFinale
//
//===========================================================================

void F_StartFinale(void)
{
    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;
    P_ClearMessage(&players[consoleplayer]);

    FinaleStage = 0;
    FinaleCount = 0;
    FinaleText = GetFinaleText(0);
    FinaleEndCount = 70;
    FinaleLumpNum = W_GetNumForName("FINALE1");
    FinaleLumpWide = "FINALE1";
    FontABaseLump = W_GetNumForName("FONTA_S") + 1;
    FontFBaseLump = W_GetNumForName("FONTF_S") + 1;
    InitializeFade(1);

//      S_ChangeMusic(mus_victor, true);
    S_StartSongName("hall", false);     // don't loop the song
}

//===========================================================================
//
// F_Responder
//
//===========================================================================

boolean F_Responder(event_t * event)
{
    return false;
}

//===========================================================================
//
// F_Ticker
//
//===========================================================================

void F_Ticker(void)
{
    // [JN] Make PAUSE working properly on text screen
    if (paused)
    {
        return;
    }

    FinaleCount++;
    if (FinaleStage < 5 && FinaleCount >= FinaleEndCount)
    {
        FinaleCount = 0;
        FinaleStage++;
        switch (FinaleStage)
        {
            case 1:            // Text 1
                FinaleEndCount = strlen(FinaleText) * TEXTSPEED + TEXTWAIT;
                break;
            case 2:            // Pic 2, Text 2
                FinaleText = GetFinaleText(1);
                FinaleEndCount = strlen(FinaleText) * TEXTSPEED + TEXTWAIT;
                FinaleLumpNum = W_GetNumForName("FINALE2");
                FinaleLumpWide = "FINALE2";
                S_StartSongName("orb", false);
                break;
            case 3:            // Pic 2 -- Fade out
                FinaleEndCount = 70;
                DeInitializeFade();
                InitializeFade(0);
                break;
            case 4:            // Pic 3 -- Fade in
                FinaleLumpNum = W_GetNumForName("FINALE3");
                FinaleLumpWide = "FINALE3";
                FinaleEndCount = 71;
                DeInitializeFade();
                InitializeFade(1);
                S_StartSongName("chess", true);
                break;
            case 5:            // Pic 3 , Text 3
                FinaleText = GetFinaleText(2);
                DeInitializeFade();
                break;
            default:
                break;
        }
        return;
    }
    if (FinaleStage == 0 || FinaleStage == 3 || FinaleStage == 4)
    {
        FadePic();
    }
}

/*
================================================================================
=
= ShowFinaleTime
=
= [JN] Show total game time on last CHESS screen.
=
================================================================================
*/

static void ShowFinaleTime (void)
{
    int  hours, minutes, seconds;
    int  worldTimer = players[consoleplayer].worldTimer;
    char FinaleTime[32];

    worldTimer /= 35;
    hours = worldTimer / 3600;
    worldTimer -= hours * 3600;
    minutes = worldTimer / 60;
    worldTimer -= minutes * 60;
    seconds = worldTimer;

    if (english_language)
    {
        // Construct total time string for printing.
        M_snprintf(FinaleTime, sizeof(FinaleTime), 
                   "TOTAL TIME:  %1d : %.2d : %.2d", hours, minutes, seconds);

        // Draw centered string with fade in effect, colorize to dark gray.
        dp_translation = cr[CR_GRAY];
        RD_M_DrawTextAFade (FinaleTime, 160 - (RD_M_TextAWidth(FinaleTime) / 2) + wide_delta, 179, 
                            FinaleCount < 403 ? transtable10 :
                            FinaleCount < 406 ? transtable20 :
                            FinaleCount < 409 ? transtable30 :
                            FinaleCount < 412 ? transtable40 :
                            FinaleCount < 415 ? transtable50 :
                            FinaleCount < 418 ? transtable70 :
                            FinaleCount < 421 ? transtable80 :
                                                transtable90);
        dp_translation = NULL;
    }
    else
    {
        // Construct total time string for printing.
        M_snprintf(FinaleTime, sizeof(FinaleTime), 
                   "J,OTT DHTVZ:  %1d : %.2d : %.2d", hours, minutes, seconds);

        // Draw centered string with fade in effect, colorize to dark gray.
        dp_translation = cr[CR_GRAY];
        RD_M_DrawTextSmallRUSFade (FinaleTime, 160 - (RD_M_TextSmallRUSWidth(FinaleTime) / 2) + wide_delta, 179, 
                            FinaleCount < 403 ? transtable10 :
                            FinaleCount < 406 ? transtable20 :
                            FinaleCount < 409 ? transtable30 :
                            FinaleCount < 412 ? transtable40 :
                            FinaleCount < 415 ? transtable50 :
                            FinaleCount < 418 ? transtable70 :
                            FinaleCount < 421 ? transtable80 :
                                                transtable90);
        dp_translation = NULL;
    }
}

//===========================================================================
//
// TextWrite
//
//===========================================================================

static void TextWrite(void)
{
    int count;
    char *ch;
    int c;
    int cx, cy;
    patch_t *w;
    const patch_t *finale_gfx = W_CacheLumpName("FINALE1", PU_CACHE);

    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before drawing
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    // [JN] For checking of modified fullscreen graphics.
    if (finale_gfx->width == 560)
    {
        V_DrawPatchFullScreen(W_CacheLumpName(FinaleLumpWide, PU_CACHE), false);
    }
    else
    {
        V_CopyScaledBuffer(I_VideoBuffer, W_CacheLumpNum(FinaleLumpNum, PU_CACHE),
                           ORIGWIDTH * ORIGHEIGHT);
    }

    if (FinaleStage == 5)
    {                           // Chess pic, draw the correct character graphic
        if (netgame)
        {
            V_DrawPatch(20 + wide_delta, 0, W_CacheLumpName("chessall", PU_CACHE), NULL);
        }
        else if (PlayerClass[consoleplayer])
        {
            V_DrawPatch(60 + wide_delta, 0, W_CacheLumpNum(W_GetNumForName("chessc")
                                                           + PlayerClass[consoleplayer] -
                                                           1, PU_CACHE), NULL);
        }

        // [JN] Show total game time on last CHESS screen.
        if (FinaleCount >= 400 && !vanillaparm)
        {
            ShowFinaleTime();
        }
    }
    // Draw the actual text
    if (FinaleStage == 5)
    {
        cy = 135;
    }
    else
    {
        cy = 5;
    }
    cx = 20;
    ch = FinaleText;
    count = (FinaleCount - 10) / TEXTSPEED;
    if (count < 0)
    {
        count = 0;
    }
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
        {
            break;
        }
        if (c == '\n')
        {
            cx = 20;
            cy += 9;
            continue;
        }
        if (c < 32)
        {
            continue;
        }
        c = toupper(c);
        if (c == 32)
        {
            cx += 5;
            continue;
        }
        w = W_CacheLumpNum((english_language ? FontABaseLump : FontFBaseLump)
                                             + c - 33, PU_CACHE);
        if (cx + SHORT(w->width) > screenwidth)
        {
            break;
        }
        V_DrawShadowedPatchRaven(cx + wide_delta, cy, w);
        cx += SHORT(w->width);
    }
}

//===========================================================================
//
// InitializeFade
//
//===========================================================================

static void InitializeFade(boolean fadeIn)
{
    unsigned i;

    Palette = Z_Malloc(768 * sizeof(fixed_t), PU_STATIC, 0);
    PaletteDelta = Z_Malloc(768 * sizeof(fixed_t), PU_STATIC, 0);
    RealPalette = Z_Malloc(768 * sizeof(byte), PU_STATIC, 0);

    if (fadeIn)
    {
        memset(RealPalette, 0, 768 * sizeof(byte));
        for (i = 0; i < 768; i++)
        {
            Palette[i] = 0;
            PaletteDelta[i] = FixedDiv((*((byte *) W_CacheLumpName("PLAYPAL", 
                                                                   PU_CACHE) + i)) << FRACBITS, 
                                                                   70 * FRACUNIT);
            I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
        }
    }
    else
    {
        for (i = 0; i < 768; i++)
        {
            RealPalette[i] = *((byte *) W_CacheLumpName("PLAYPAL", PU_CACHE) + i);
            Palette[i] = RealPalette[i] << FRACBITS;
            PaletteDelta[i] = FixedDiv(Palette[i], -70 * FRACUNIT);
        }
    }
    I_SetPalette(RealPalette);
}

//===========================================================================
//
// DeInitializeFade
//
//===========================================================================

static void DeInitializeFade(void)
{
    Z_Free(Palette);
    Z_Free(PaletteDelta);
    Z_Free(RealPalette);
}

//===========================================================================
//
// FadePic
//
//===========================================================================

static void FadePic(void)
{
    unsigned i;

    for (i = 0; i < 768; i++)
    {
        Palette[i] += PaletteDelta[i];
        RealPalette[i] = Palette[i] >> FRACBITS;
    }
    I_SetPalette(RealPalette);
}

//===========================================================================
//
// DrawPic
//
//===========================================================================

static void DrawPic(void)
{
    const patch_t *finale_gfx = W_CacheLumpName("FINALE1", PU_CACHE);

    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before drawing
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    // [JN] For checking of modified fullscreen graphics.
    if (finale_gfx->width == 560)
    {
        V_DrawPatchFullScreen(W_CacheLumpName(FinaleLumpWide, PU_CACHE), false);
    }
    else
    {
        V_CopyScaledBuffer(I_VideoBuffer, W_CacheLumpNum(FinaleLumpNum, PU_CACHE),
                           ORIGWIDTH * ORIGHEIGHT);
    }

    if (FinaleStage == 4 || FinaleStage == 5)
    {                           // Chess pic, draw the correct character graphic
        if (netgame)
        {
            V_DrawPatch(20 + wide_delta, 0, W_CacheLumpName("chessall", PU_CACHE), NULL);
        }
        else if (PlayerClass[consoleplayer])
        {
            V_DrawPatch(60 + wide_delta, 0, W_CacheLumpNum(W_GetNumForName("chessc")
                                                           + PlayerClass[consoleplayer] -
                                                           1, PU_CACHE), NULL);
        }
    }
}

//===========================================================================
//
// F_Drawer
//
//===========================================================================

void F_Drawer(void)
{
    switch (FinaleStage)
    {
        case 0:                // Fade in initial finale screen
            DrawPic();
            break;
        case 1:
        case 2:
            TextWrite();
            break;
        case 3:                // Fade screen out
            DrawPic();
            break;
        case 4:                // Fade in chess screen
            DrawPic();
            break;
        case 5:
            TextWrite();
            break;
    }
    UpdateState |= I_FULLSCRN;
}

//==========================================================================
//
// GetFinaleText
//
//==========================================================================

static char *GetFinaleText(int sequence)
{
    char *msgLumpName;
    int msgSize;
    int msgLump;

    static char *winMsgLumpNames[] = {
        "win1msg",
        "win2msg",
        "win3msg"
    };

    static char *winMsgLumpNames_Rus[] = {
        "WIN1_BH",
        "WIN2_BH",
        "WIN3_BH"
    };

    static char *winMsgLumpNames_Rus_DK[] = {
        "WIN1_DK",
        "WIN2_DK",
        "WIN3_DK"
    };

    // [JN] Use standard WIN*MSG for English language
    if (english_language)
    {
        msgLumpName = winMsgLumpNames[sequence];
    }
    else
    {
        if (isDK)
        {
            // [JN] Use custom WIN*_DK for Russian language in Death Kings
            msgLumpName = winMsgLumpNames_Rus_DK[sequence];
        }
        else
        {
            // [JN] Use custom WIN*_BH for Russian language in Beyond Heretic
            msgLumpName = winMsgLumpNames_Rus[sequence];
        }
    }
    
    msgLump = W_GetNumForName(msgLumpName);
    msgSize = W_LumpLength(msgLump);
    if (msgSize >= MAX_INTRMSN_MESSAGE_SIZE)
    {
        I_Error(english_language ?
                "Finale message too long (%s)" :
                "Превышена допустимая длина текста (%s)",
                msgLumpName);
    }
    W_ReadLump(msgLump, ClusterMessage);
    ClusterMessage[msgSize] = 0;        // Append terminator
    return ClusterMessage;
}
