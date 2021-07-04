//
// Copyright(C) 2021 Dasperal
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

#include "rd_text.h"

#include "d_name.h"
#include "i_swap.h"
#include "v_patch.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

Translation_CR_t messages_pickup_color_set;
Translation_CR_t messages_secret_color_set;
Translation_CR_t messages_system_color_set;
Translation_CR_t messages_chat_color_set;

void (*drawLetter)(int x, int y, patch_t *patch);

static lumpindex_t fontA;
static lumpindex_t fontB;
static lumpindex_t fontC;

static lumpindex_t smallEngFont;
static lumpindex_t bigEngFont;

static lumpindex_t smallRusFont;
static lumpindex_t bigRusFont;

void RD_M_InitFonts(char* FontA, char* FontB, char* FontC,
                    char* SmallEngFont, char* BigEngFont,
                    char* SmallRusFont, char* BigRusFont)
{
    switch (RD_GameType)
    {
        case gt_Doom:
            drawLetter = V_DrawShadowedPatchDoom;
            break;
        case gt_Heretic:
        case gt_Hexen:
            drawLetter = V_DrawShadowedPatchRaven;
            break;
        case gt_Strife:
            drawLetter = V_DrawShadowedPatchStrife;
            break;
        default:
            drawLetter = V_DrawShadowedPatch;
    }

    fontA = W_GetNumForName(FontA) + 1;
    fontB = W_GetNumForName(FontB) + 1;
    fontC = W_GetNumForName(FontC) + 1;

    smallEngFont = W_GetNumForName(SmallEngFont) + 1;
    bigEngFont = W_GetNumForName(BigEngFont) + 1;

    smallRusFont = W_GetNumForName(SmallRusFont) + 1;
    bigRusFont = W_GetNumForName(BigRusFont) + 1;
}

/** Draw text using replaceable English font A*/
void RD_M_DrawTextA(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(fontA + c - 33, PU_CACHE);
            drawLetter(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

/** Draw text using replaceable English font A with fading effect*/
void RD_M_DrawTextAFade(char *text, int x, int y, byte *table)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(fontA + c - 33, PU_CACHE);
            V_DrawFadePatch(x, y, p, table);
            x += SHORT(p->width) - 1;
        }
    }
}

/** Returns the pixel width of a string using replaceable English font A*/
int RD_M_TextAWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(fontA + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

/**Draw text using replaceable English font B*/
void RD_M_DrawTextB(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(fontB + c - 33, PU_CACHE);
            drawLetter(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

/** Returns the pixel width of a string using replaceable English font B*/
int RD_M_TextBWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(fontB + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

/** [JN] Draw small time digits using font C*/
void RD_M_DrawTextC(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33) // [JN] Means space symbol (" ").
            {
            x += 4;
            }
        else
        {
            p = W_CacheLumpNum(fontC + c - 33, PU_CACHE);
            V_DrawPatch(x, y, p);
            x += SHORT(p->width);
        }
    }
}

/** [JN] Draw text string with unreplaceable small English font*/
void RD_M_DrawTextSmallENG(char *text, int x, int y, Translation_CR_t translation)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(smallEngFont + c - 33, PU_CACHE);
            dp_translation = translation == CR_NONE ? NULL : cr[translation];
            drawLetter(x, y, p);
            dp_translation = NULL;
            x += SHORT(p->width) - 1;
        }
    }
}

/** [JN] Draw text string with unreplaceable big English font*/
void RD_M_DrawTextBigENG(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(bigEngFont + c - 33, PU_CACHE);
            drawLetter(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

/** [JN] Returns the pixel width of a string using unreplaceable big English font */
int RD_M_TextBigENGWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(bigEngFont + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

/** [JN] Draw text string with unreplaceable small Russian font*/
void RD_M_DrawTextSmallRUS(char *text, int x, int y, Translation_CR_t translation)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(smallRusFont + c - 33, PU_CACHE);
            dp_translation = translation == CR_NONE ? NULL : cr[translation];
            drawLetter(x, y, p);
            dp_translation = NULL;
            x += SHORT(p->width) - 1;
        }
    }
}

/** [JN] Draw text using unreplaceable small Russian font with fading effect*/
void RD_M_DrawTextSmallRUSFade(char *text, int x, int y, byte *table)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(smallRusFont + c - 33, PU_CACHE);
            V_DrawFadePatch(x, y, p, table);
            x += SHORT(p->width) - 1;
        }
    }
}

/** [JN] Returns the pixel width of a string using unreplaceable small Russian font*/
int RD_M_TextSmallRUSWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(smallRusFont + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

/** [JN] Draw text string with unreplaceable big Russian font*/
void RD_M_DrawTextBigRUS(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(bigRusFont + c - 33, PU_CACHE);
            drawLetter(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

/** [JN] Returns the pixel width of a string using unreplaceable big Russian font*/
int RD_M_TextBigRUSWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(bigRusFont + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}
