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

#include <stddef.h>
#include "h2def.h"
#include "m_random.h"
#include "i_system.h"
#include "p_local.h"
#include "r_swirl.h"
#include "s_sound.h"

// MACROS ------------------------------------------------------------------

#define ANIM_SCRIPT_NAME "ANIMDEFS"
#define ANIM_FLAT 0
#define ANIM_TEXTURE 1
#define SCI_FLAT "flat"
#define SCI_TEXTURE "texture"
#define SCI_PIC "pic"
#define SCI_TICS "tics"
#define SCI_RAND "rand"

#define LIGHTNING_SPECIAL 	198
#define LIGHTNING_SPECIAL2 	199
#define SKYCHANGE_SPECIAL 	200

// TYPES -------------------------------------------------------------------

typedef struct
{
    int index;
    int tics;
} frameDef_t;

typedef struct
{
    int type;
    int index;
    int tics;
    int currentFrameDef;
    int startFrameDef;
    int endFrameDef;
} animDef_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void P_LightningFlash(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern fixed_t Sky1ColumnOffset;
extern fixed_t Sky2ColumnOffset;
extern boolean DoubleSky;
// [JN] Smooth sky scrolling.
extern fixed_t Sky1SmoothScrollFactor;
extern fixed_t Sky2SmoothScrollFactor;
extern fixed_t Sky1SmoothScrollDelta;
extern fixed_t Sky2SmoothScrollDelta;
// [JN] Smooth plane scrolling.
extern fixed_t FlatScrollFactor_X, FlatScrollFactor_X_old;
extern fixed_t FlatScrollFactor_Y, FlatScrollFactor_Y_old;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

fixed_t Sky1ScrollDelta;
fixed_t Sky2ScrollDelta;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static animDef_t *AnimDefs = NULL;    // [JN] Remove MAX_ANIM_DEFS limit.
static int AnimDefsMax = 0;
static frameDef_t *FrameDefs = NULL;  // [JN] Remove MAX_FRAME_DEFS limit.
static int FrameDefsMax = 0;
static int AnimDefCount;
static boolean LevelHasLightning;
static int NextLightningFlash;
static int LightningFlash;
static int *LightningLightLevels;
// [JN] How far sky scrolling delta can go before reset.
static const fixed_t SkyScrollLimit = INT_MAX/2;
// [JN] How far flat scrolling factor can go before reset.
static const fixed_t FlatScrollLimit = FRACUNIT * 128;
// [JN] Swirling surfaces (same to flat names):
static int x_001, x_005, x_009;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// P_AnimateSurfaces
//
//==========================================================================

void P_AnimateSurfaces(void)
{
    int i;
    animDef_t *ad;
    line_t *line;

    // Animate flats and textures
    for (i = 0; i < AnimDefCount; i++)
    {
        ad = &AnimDefs[i];
        ad->tics--;
        if (ad->tics == 0)
        {
            if (ad->currentFrameDef == ad->endFrameDef)
            {
                ad->currentFrameDef = ad->startFrameDef;
            }
            else
            {
                ad->currentFrameDef++;
            }
            ad->tics = FrameDefs[ad->currentFrameDef].tics;
            if (ad->tics > 255)
            {                   // Random tics
                ad->tics = (ad->tics >> 16)
                    + P_Random() % ((ad->tics & 0xff00) >> 8);
            }
            if (ad->type == ANIM_FLAT)
            {
                // [JN] Add support for SMMU swirling flats.
                if (swirling_liquids && !vanillaparm
                && (ad->index == x_001
                ||  ad->index == x_005
                ||  ad->index == x_009))
                {
                    flattranslation[ad->index] = -1;
                }
                else
                {
                    flattranslation[ad->index] = FrameDefs[ad->currentFrameDef].index;
                }
            }
            else
            {                   // Texture
                texturetranslation[ad->index] =
                    FrameDefs[ad->currentFrameDef].index;
            }
        }
    }

    // Update scrolling textures
    for (i = 0; i < numlinespecials; i++)
    {
        line = linespeciallist[i];
        switch (line->special)
        {
            case 100:          // Scroll_Texture_Left
                sides[line->sidenum[0]].oldtextureoffset =
                sides[line->sidenum[0]].textureoffset;
                sides[line->sidenum[0]].textureoffset += line->arg1 << 10;
                break;
            case 101:          // Scroll_Texture_Right
                sides[line->sidenum[0]].oldtextureoffset =
                sides[line->sidenum[0]].textureoffset;
                sides[line->sidenum[0]].textureoffset -= line->arg1 << 10;
                break;
            case 102:          // Scroll_Texture_Up
                sides[line->sidenum[0]].oldrowoffset =
                sides[line->sidenum[0]].rowoffset;
                sides[line->sidenum[0]].rowoffset += line->arg1 << 10;
                break;
            case 103:          // Scroll_Texture_Down
                sides[line->sidenum[0]].oldrowoffset =
                sides[line->sidenum[0]].rowoffset;
                sides[line->sidenum[0]].rowoffset -= line->arg1 << 10;
                break;
        }
    }

    // Update sky column offsets
    Sky1ColumnOffset += Sky1ScrollDelta;
    Sky2ColumnOffset += Sky2ScrollDelta;

    // [JN] Update smoothed sky column offsets.
    Sky1SmoothScrollDelta += Sky1ScrollDelta > 0 ? (FRACUNIT*Sky1SmoothScrollFactor) + 16 : 0;
    Sky2SmoothScrollDelta += Sky2ScrollDelta > 0 ? (FRACUNIT*Sky2SmoothScrollFactor) + 16 : 0;

    // [JN] Once delta value reaches it's limit, reset it to zero.
    if (Sky1SmoothScrollDelta >= SkyScrollLimit)
    {
        Sky1SmoothScrollDelta = 0;
    }
    if (Sky2SmoothScrollDelta >= SkyScrollLimit)
    {
        Sky2SmoothScrollDelta = 0;
    }

    // [JN] Update smoothed plane textures offsets.
    FlatScrollFactor_X_old = FlatScrollFactor_X;
    FlatScrollFactor_Y_old = FlatScrollFactor_Y;
    FlatScrollFactor_X += FRACUNIT;
    FlatScrollFactor_Y += FRACUNIT;

    // [JN] Once factor value reaches it's limit, reset it to zero.
    if (FlatScrollFactor_X >= FlatScrollLimit)
    {
        FlatScrollFactor_X = 0;
    }
    if (FlatScrollFactor_Y >= FlatScrollLimit)
    {
        FlatScrollFactor_Y = 0;
    }

    if (LevelHasLightning)
    {
        if (!NextLightningFlash || LightningFlash)
        {
            P_LightningFlash();
        }
        else
        {
            NextLightningFlash--;
        }
    }
}

/*
================================================================================
=
= R_SmoothTextureScrolling
=
= [JN] Makes wall texture scrolling smoother. Based on implementation
= from Crispy Doom, added support for scrolling speed arguments.
=
================================================================================
*/

void R_SmoothTextureScrolling()
{
	fixed_t frac;

	if (uncapped_fps && leveltime > oldleveltime)
	{
		frac = fractionaltic;
	}
	else
	{
		frac = FRACUNIT;
	}

    for (int i = 0; i < numlinespecials; i++)
    {
        const line_t *line = linespeciallist[i];
        side_t *const side = &sides[line->sidenum[0]];
        const fixed_t frac = fractionaltic / 56 * line->arg1;

        switch (line->special)
        {
            case 100:          // Scroll_Texture_Right
                side->textureoffset = side->oldtextureoffset + frac;
                break;
            case 101:          // Scroll_Texture_Right
                side->textureoffset = side->oldtextureoffset - frac;
                break;
            case 102:          // Scroll_Texture_Up
                side->rowoffset = side->oldrowoffset + frac;
                break;
            case 103:          // Scroll_Texture_Down
                side->rowoffset = side->oldrowoffset - frac;
                break;
        }
    }

    FlatScrollFactor_X = FlatScrollFactor_X_old + frac;
    FlatScrollFactor_Y = FlatScrollFactor_Y_old + frac;
}

//==========================================================================
//
// P_LightningFlash
//
//==========================================================================

static void P_LightningFlash(void)
{
    int i;
    sector_t *tempSec;
    int *tempLight;
    boolean foundSec;
    int flashLight;

    if (LightningFlash)
    {
        LightningFlash--;
        if (LightningFlash)
        {
            tempLight = LightningLightLevels;
            tempSec = sectors;
            for (i = 0; i < numsectors; i++, tempSec++)
            {
                if (tempSec->ceilingpic == skyflatnum
                    || tempSec->special == LIGHTNING_SPECIAL
                    || tempSec->special == LIGHTNING_SPECIAL2)
                {
                    if (*tempLight < tempSec->lightlevel - 4)
                    {
                        tempSec->lightlevel -= 4;
                    }
                    tempLight++;
                }
            }
        }
        else
        {                       // remove the alternate lightning flash special
            tempLight = LightningLightLevels;
            tempSec = sectors;
            for (i = 0; i < numsectors; i++, tempSec++)
            {
                if (tempSec->ceilingpic == skyflatnum
                    || tempSec->special == LIGHTNING_SPECIAL
                    || tempSec->special == LIGHTNING_SPECIAL2)
                {
                    tempSec->lightlevel = *tempLight;
                    tempLight++;
                }
            }
            Sky1Texture = P_GetMapSky1Texture(gamemap);
        }
        return;
    }
    LightningFlash = (P_Random() & 7) + 8;
    flashLight = 200 + (P_Random() & 31);
    tempSec = sectors;
    tempLight = LightningLightLevels;
    foundSec = false;
    for (i = 0; i < numsectors; i++, tempSec++)
    {
        if (tempSec->ceilingpic == skyflatnum
            || tempSec->special == LIGHTNING_SPECIAL
            || tempSec->special == LIGHTNING_SPECIAL2)
        {
            *tempLight = tempSec->lightlevel;
            if (tempSec->special == LIGHTNING_SPECIAL)
            {
                tempSec->lightlevel += 64;
                if (tempSec->lightlevel > flashLight)
                {
                    tempSec->lightlevel = flashLight;
                }
            }
            else if (tempSec->special == LIGHTNING_SPECIAL2)
            {
                tempSec->lightlevel += 32;
                if (tempSec->lightlevel > flashLight)
                {
                    tempSec->lightlevel = flashLight;
                }
            }
            else
            {
                tempSec->lightlevel = flashLight;
            }
            if (tempSec->lightlevel < *tempLight)
            {
                tempSec->lightlevel = *tempLight;
            }
            tempLight++;
            foundSec = true;
        }
    }
    if (foundSec)
    {
        Sky1Texture = P_GetMapSky2Texture(gamemap);     // set alternate sky                
        S_StartSound(NULL, SFX_THUNDER_CRASH);
    }
    // Calculate the next lighting flash
    if (!NextLightningFlash)
    {
        if (P_Random() < 50)
        {                       // Immediate Quick flash
            NextLightningFlash = (P_Random() & 15) + 16;
        }
        else
        {
            if (P_Random() < 128 && !(leveltime & 32))
            {
                NextLightningFlash = ((P_Random() & 7) + 2) * 35;
            }
            else
            {
                NextLightningFlash = ((P_Random() & 15) + 5) * 35;
            }
        }
    }
}

//==========================================================================
//
// P_ForceLightning
//
//==========================================================================

void P_ForceLightning(void)
{
    NextLightningFlash = 0;
}

//==========================================================================
//
// P_InitLightning
//
//==========================================================================

void P_InitLightning(void)
{
    int i;
    int secCount;

    if (!P_GetMapLightning(gamemap))
    {
        LevelHasLightning = false;
        LightningFlash = 0;
        return;
    }
    LightningFlash = 0;
    secCount = 0;
    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].ceilingpic == skyflatnum
            || sectors[i].special == LIGHTNING_SPECIAL
            || sectors[i].special == LIGHTNING_SPECIAL2)
        {
            secCount++;
        }
    }
    if (secCount)
    {
        LevelHasLightning = true;
    }
    else
    {
        LevelHasLightning = false;
        return;
    }
    LightningLightLevels = (int *) Z_Malloc(secCount * sizeof(int), PU_LEVEL,
                                            NULL);
    NextLightningFlash = ((P_Random() & 15) + 5) * 35;  // don't flash at level start
}

//==========================================================================
//
// P_InitFTAnims
//
// Initialize flat and texture animation lists.
//
//==========================================================================

void P_InitFTAnims(void)
{
    int base;
    int mod;
    int fd;
    animDef_t *ad = AnimDefs;
    ptrdiff_t old;
    boolean ignore;
    boolean done;

    fd = 0;
    AnimDefCount = 0;
    SC_Open(ANIM_SCRIPT_NAME);
    while (SC_GetString())
    {
        // [JN] Remove MAX_ANIM_DEFS limit.
        if (AnimDefCount == AnimDefsMax)
        {
            old = ad - AnimDefs;
            AnimDefs = I_Realloc(AnimDefs, (AnimDefsMax = AnimDefsMax ?
                                 AnimDefsMax * 2 : 20) * sizeof(*AnimDefs));
            ad = AnimDefs + old;
        }
        if (SC_Compare(SCI_FLAT))
        {
            ad->type = ANIM_FLAT;
        }
        else if (SC_Compare(SCI_TEXTURE))
        {
            ad->type = ANIM_TEXTURE;
        }
        else
        {
            SC_ScriptError(NULL);
        }
        SC_MustGetString();     // Name
        ignore = false;
        if (ad->type == ANIM_FLAT)
        {
            if (W_CheckNumForName(sc_String) == -1)
            {
                ignore = true;
            }
            else
            {
                ad->index = R_FlatNumForName(sc_String);
            }
        }
        else
        {                       // Texture
            if (R_CheckTextureNumForName(sc_String) == -1)
            {
                ignore = true;
            }
            else
            {
                ad->index = R_TextureNumForName(sc_String);
            }
        }
        ad->startFrameDef = fd;
        done = false;
        while (done == false)
        {
            if (SC_GetString())
            {
                if (SC_Compare(SCI_PIC))
                {
                    // [JN] Remove MAX_FRAME_DEFS limit.
                    if (fd == FrameDefsMax)
                    {
                        FrameDefs = I_Realloc(FrameDefs, (FrameDefsMax = FrameDefsMax ?
                                              FrameDefsMax * 2 : 96) * sizeof(*FrameDefs));
                    }
                    SC_MustGetNumber();
                    if (ignore == false)
                    {
                        FrameDefs[fd].index = ad->index + sc_Number - 1;
                    }
                    SC_MustGetString();
                    if (SC_Compare(SCI_TICS))
                    {
                        SC_MustGetNumber();
                        if (ignore == false)
                        {
                            FrameDefs[fd].tics = sc_Number;
                            fd++;
                        }
                    }
                    else if (SC_Compare(SCI_RAND))
                    {
                        SC_MustGetNumber();
                        base = sc_Number;
                        SC_MustGetNumber();
                        if (ignore == false)
                        {
                            mod = sc_Number - base + 1;
                            FrameDefs[fd].tics = (base << 16) + (mod << 8);
                            fd++;
                        }
                    }
                    else
                    {
                        SC_ScriptError(NULL);
                    }
                }
                else
                {
                    SC_UnGet();
                    done = true;
                }
            }
            else
            {
                done = true;
            }
        }
        if ((ignore == false) && (fd - ad->startFrameDef < 2))
        {
            I_Error(english_language ?
                    "P_InitFTAnims: AnimDef has framecount < 2." :
                    "P_InitFTAnims: количество кадров AnimDef меньше 2х.");
        }
        if (ignore == false)
        {
            ad->endFrameDef = fd - 1;
            ad->currentFrameDef = ad->endFrameDef;
            ad->tics = 1;       // Force 1st game tic to animate
            AnimDefCount++;
            ad++;
        }
    }
    SC_Close();

    // [JN] Don't init in "-vanilla", since there is no swirling flats.
    if (!vanillaparm)
    {
        R_InitDistortedFlats();

        x_001 = R_FlatNumForName("x_001");
        x_005 = R_FlatNumForName("x_005");
        x_009 = R_FlatNumForName("x_009");
    }
}
