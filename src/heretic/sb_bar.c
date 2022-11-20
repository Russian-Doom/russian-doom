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
// SB_bar.c



#include "hr_local.h"
#include "deh_str.h"
#include "d_name.h"
#include "i_video.h"
#include "i_swap.h"
#include "i_timer.h"    // [JN] TICRATE
#include "m_cheat.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "v_trans.h"
#include "id_lang.h"
#include "jn.h"

// Types

typedef struct Cheat_s
{
    void (*func) (player_t * player, struct Cheat_s * cheat);
    cheatseq_t *seq;
} Cheat_t;

// Private Functions

static void DrawSoundInfo(void);
static void ShadeLine(int x, int y, int height, int shade);
static void ShadeChain(void);
static void DrINumber(signed int val, int x, int y);
static void DrBNumber(signed int val, int x, int y);
static void DrSmallNumber(int val, int x, int y, boolean draw_val1);
static void DrawCommonBar(void);
static void DrawMainBar(void);
static void DrawInventoryBar(void);
static void DrawFullScreenStuff(void);
static boolean HandleCheats(byte key);
static void CheatGodFunc(player_t * player, Cheat_t * cheat);
static void CheatNoClipFunc(player_t * player, Cheat_t * cheat);
static void CheatWeaponsFunc(player_t * player, Cheat_t * cheat);
static void CheatPowerFunc(player_t * player, Cheat_t * cheat);
static void CheatHealthFunc(player_t * player, Cheat_t * cheat);
static void CheatKeysFunc(player_t * player, Cheat_t * cheat);
static void CheatSoundFunc(player_t * player, Cheat_t * cheat);
static void CheatArtifact1Func(player_t * player, Cheat_t * cheat);
static void CheatArtifact2Func(player_t * player, Cheat_t * cheat);
static void CheatArtifact3Func(player_t * player, Cheat_t * cheat);
static void CheatWarpFunc(player_t * player, Cheat_t * cheat);
static void CheatChickenFunc(player_t * player, Cheat_t * cheat);
static void CheatMassacreFunc(player_t * player, Cheat_t * cheat);
static void CheatIDKFAFunc(player_t * player, Cheat_t * cheat);
static void CheatIDDQDFunc(player_t * player, Cheat_t * cheat);
static void CheatVERSIONFunc(player_t * player, Cheat_t * cheat);
static void CheatRAVMUSFunc(player_t * player, Cheat_t * cheat);

// [JN] Ammo widget prototypes.
static void DrSmallAmmoNumber (int val, int x, int y, boolean opaque);
static void SB_Draw_Ammo_Widget (void);

// Public Data

// graphics are drawn to a backing screen and blitted to the real screen
byte *st_backing_screen;

boolean DebugSound;             // debug flag for displaying sound info

boolean inventory;
int curpos;
int inv_ptr;
int ArtifactFlash;

// Private Data

static int HealthMarker;
static int ChainWiggle;
static player_t *CPlayer;
static int playpalette;

static void Crosshair_Colorize_inGame (void);

static patch_t *PatchLTFACE;
static patch_t *PatchRTFACE;
static patch_t *PatchBARBACK;
static patch_t *PatchCHAIN;
static patch_t *PatchSTATBAR;
static patch_t *PatchSTATBAR_RUS;
static patch_t *PatchLIFEGEM[4];
static patch_t *PatchLTFCTOP;
static patch_t *PatchRTFCTOP;
static patch_t *PatchSELECTBOX;
static patch_t *PatchINVLFGEM1;
static patch_t *PatchINVLFGEM2;
static patch_t *PatchINVRTGEM1;
static patch_t *PatchINVRTGEM2;
static patch_t *PatchINumbers[10];
static patch_t *PatchNEGATIVE;
static patch_t *PatchSmNumbers[10];
static patch_t *PatchBLACKSQ;
static patch_t *PatchINVBAR;
static patch_t *PatchARMCLEAR;
static patch_t *PatchCHAINBACK;

int FontBNumBase;
int spinbooklump;
int spinflylump;

// Toggle god mode
cheatseq_t CheatGodSeq = CHEAT("quicken", 0);

// Toggle no clipping mode
cheatseq_t CheatNoClipSeq = CHEAT("kitty", 0);

// Get all weapons and ammo
cheatseq_t CheatWeaponsSeq = CHEAT("rambo", 0);

// Toggle tome of power
cheatseq_t CheatPowerSeq = CHEAT("shazam", 0);

// Get full health
cheatseq_t CheatHealthSeq = CHEAT("ponce", 0);

// Get all keys
cheatseq_t CheatKeysSeq = CHEAT("skel", 0);

// Toggle sound debug info
cheatseq_t CheatSoundSeq = CHEAT("noise", 0);

// Get an artifact 1st stage (ask for type)
cheatseq_t CheatArtifact1Seq = CHEAT("gimme", 0);

// Get an artifact 2nd stage (ask for count)
cheatseq_t CheatArtifact2Seq = CHEAT("gimme", 1);

// Get an artifact final stage
cheatseq_t CheatArtifact3Seq = CHEAT("gimme", 2);

// Warp to new level
cheatseq_t CheatWarpSeq = CHEAT("engage", 2);

// Save a screenshot
cheatseq_t CheatChickenSeq = CHEAT("cockadoodledoo", 0);

// Kill all monsters
cheatseq_t CheatMassacreSeq = CHEAT("massacre", 0);

cheatseq_t CheatIDKFASeq = CHEAT("idkfa", 0);
cheatseq_t CheatIDDQDSeq = CHEAT("iddqd", 0);

// [JN] Чит-код отображения версии проекта
cheatseq_t CheatVERSIONSeq = CHEAT("version", 0);

// [JN] Music changing.
cheatseq_t CheatRAVMUSSeq = CHEAT("ravmus", 2);
cheatseq_t CheatIDMUSSeq = CHEAT("idmus", 2);

static Cheat_t Cheats[] = {
    {CheatGodFunc,       &CheatGodSeq},
    {CheatNoClipFunc,    &CheatNoClipSeq},
    {CheatWeaponsFunc,   &CheatWeaponsSeq},
    {CheatPowerFunc,     &CheatPowerSeq},
    {CheatHealthFunc,    &CheatHealthSeq},
    {CheatKeysFunc,      &CheatKeysSeq},
    {CheatSoundFunc,     &CheatSoundSeq},
    {CheatArtifact1Func, &CheatArtifact1Seq},
    {CheatArtifact2Func, &CheatArtifact2Seq},
    {CheatArtifact3Func, &CheatArtifact3Seq},
    {CheatWarpFunc,      &CheatWarpSeq},
    {CheatChickenFunc,   &CheatChickenSeq},
    {CheatMassacreFunc,  &CheatMassacreSeq},
    {CheatIDKFAFunc,     &CheatIDKFASeq},
    {CheatIDDQDFunc,     &CheatIDDQDSeq},
    {CheatVERSIONFunc,   &CheatVERSIONSeq},
    {CheatRAVMUSFunc,    &CheatRAVMUSSeq},
    {CheatRAVMUSFunc,    &CheatIDMUSSeq},
    {NULL,               NULL} 
};

//---------------------------------------------------------------------------
//
// PROC SB_Init
//
//---------------------------------------------------------------------------

void SB_Init(void)
{
    int i;
    int startLump;

    st_backing_screen = (byte *)Z_Malloc((screenwidth << hires) 
                                       * (40 << hires)
                                       * sizeof(*st_backing_screen), PU_STATIC, 0);

    PatchLTFACE = W_CacheLumpName(DEH_String("LTFACE"), PU_STATIC);
    PatchRTFACE = W_CacheLumpName(DEH_String("RTFACE"), PU_STATIC);
    PatchBARBACK = W_CacheLumpName(DEH_String("BARBACK"), PU_STATIC);
    PatchINVBAR = W_CacheLumpName(DEH_String("INVBAR"), PU_STATIC);
    PatchCHAIN = W_CacheLumpName(DEH_String("CHAIN"), PU_STATIC);
    if (deathmatch)
    {
        PatchSTATBAR = W_CacheLumpName(DEH_String("STATBAR"), PU_STATIC);
        PatchSTATBAR_RUS = W_CacheLumpName(DEH_String("RD_STBAR"), PU_STATIC);
    }
    else
    {
        PatchSTATBAR = W_CacheLumpName(DEH_String("LIFEBAR"), PU_STATIC);
        PatchSTATBAR_RUS = W_CacheLumpName(DEH_String("RD_LFBAR"), PU_STATIC);
    }

    // [JN] Life gem. Modified to array type for support of proper
    // coloring while toggling spy mode in multiplayer game.
    for (int i = 0 ; i < 4 ; i++)
    {
        PatchLIFEGEM[i] = W_CacheLumpNum(W_GetNumForName(DEH_String("LIFEGEM0"))
                                         + i, PU_STATIC);
    }
    PatchLTFCTOP = W_CacheLumpName(DEH_String("LTFCTOP"), PU_STATIC);
    PatchRTFCTOP = W_CacheLumpName(DEH_String("RTFCTOP"), PU_STATIC);
    PatchSELECTBOX = W_CacheLumpName(DEH_String("SELECTBOX"), PU_STATIC);
    PatchINVLFGEM1 = W_CacheLumpName(DEH_String("INVGEML1"), PU_STATIC);
    PatchINVLFGEM2 = W_CacheLumpName(DEH_String("INVGEML2"), PU_STATIC);
    PatchINVRTGEM1 = W_CacheLumpName(DEH_String("INVGEMR1"), PU_STATIC);
    PatchINVRTGEM2 = W_CacheLumpName(DEH_String("INVGEMR2"), PU_STATIC);
    PatchBLACKSQ = W_CacheLumpName(DEH_String("BLACKSQ"), PU_STATIC);
    PatchARMCLEAR = W_CacheLumpName(DEH_String("ARMCLEAR"), PU_STATIC);
    PatchCHAINBACK = W_CacheLumpName(DEH_String("CHAINBACK"), PU_STATIC);
    startLump = W_GetNumForName(DEH_String("IN0"));
    for (i = 0; i < 10; i++)
    {
        PatchINumbers[i] = W_CacheLumpNum(startLump + i, PU_STATIC);
    }
    PatchNEGATIVE = W_CacheLumpName(DEH_String("NEGNUM"), PU_STATIC);
    FontBNumBase = W_GetNumForName(DEH_String("FONTB16"));
    startLump = W_GetNumForName(DEH_String("SMALLIN0"));
    for (i = 0; i < 10; i++)
    {
        PatchSmNumbers[i] = W_CacheLumpNum(startLump + i, PU_STATIC);
    }
    playpalette = W_GetNumForName(DEH_String("PLAYPAL"));
    spinbooklump = W_GetNumForName(DEH_String("SPINBK0"));
    spinflylump = W_GetNumForName(DEH_String("SPFLY0"));
}

/*
================================================================================
=
= SB_PaletteFlash
=
= Sets the new palette based upon current values of player->damagecount
= and player->bonuscount.
=
================================================================================
*/

static void SB_PaletteFlash (void)
{
    static int  sb_palette = 0;
    int         palette;
    const byte *pal;

    CPlayer = &players[displayplayer];

    if (CPlayer->damagecount)
    {
        palette = (CPlayer->damagecount + 7) >> 3;
        if (palette >= NUMREDPALS)
        {
            palette = NUMREDPALS - 1;
        }
        palette += STARTREDPALS;
    }
    // [crispy] never show the yellow bonus palette for a dead player
    else if (CPlayer->bonuscount && CPlayer->health > 0)
    {
        // [JN] One extra palette for pickup flashing
        // https://doomwiki.org/wiki/PLAYPAL
        palette = (CPlayer->bonuscount + 7) >> 3;
        if (palette >= NUMBONUSPALS)
        {
            palette = NUMBONUSPALS;
        }
        palette += STARTBONUSPALS-1;
    }
    else
    {
        palette = 0;
    }
    if (palette != sb_palette)
    {
        sb_palette = palette;
        pal = (byte *) W_CacheLumpNum(playpalette, PU_CACHE) + palette * 768;
        I_SetPalette(pal);
    }
}

//---------------------------------------------------------------------------
//
// PROC SB_Ticker
//
//---------------------------------------------------------------------------

void SB_Ticker(void)
{
    int delta;
    int curHealth;

    curHealth = players[displayplayer].mo->health;

    if (leveltime & 1 && curHealth > 0)
    {
        ChainWiggle = P_Random() & 1;
    }
    if (curHealth < 0)
    {
        curHealth = 0;
    }
    if (curHealth < HealthMarker)
    {
        delta = (HealthMarker - curHealth) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > 8)
        {
            delta = 8;
        }
        HealthMarker -= delta;
    }
    else if (curHealth > HealthMarker)
    {
        delta = (curHealth - HealthMarker) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > 8)
        {
            delta = 8;
        }
        HealthMarker += delta;
    }

    // [JN] Do red-/gold-shifts from damage/items.
    SB_PaletteFlash();
}

//---------------------------------------------------------------------------
//
// PROC DrINumber
//
// Draws a three digit number.
//
//---------------------------------------------------------------------------

static void DrINumber(signed int val, int x, int y)
{
    patch_t *patch;
    int oldval;

    oldval = val;
    if (val < 0)
    {
        if (val < -9)
        {
            // [JN] Negative health: Leave "LAME" sign for Deathmatch
            if (deathmatch)
            {
                V_DrawPatch(x + 1, y + 1, W_CacheLumpName(DEH_String("LAME"), PU_CACHE), NULL);
            }
            // [JN] Negative health: -10 and below routine
            else if (negative_health && !vanillaparm)
            {
                // [JN] Can't draw -100 and below
                if (val <= -99)
                val = -99;

                val = -val % 100;
                if (val < 9 || oldval < 99)
                {
                    patch = PatchINumbers[val / 10];
                    V_DrawPatch(x + 9, y, patch, NULL);
                }
                val = val % 10;
                patch = PatchINumbers[val];
                V_DrawPatch(x + 18, y, patch, NULL);

                V_DrawPatch(x + 1, y, PatchNEGATIVE, NULL);
            }
        }
        else
        {
            val = -val;
            V_DrawPatch(x + 18, y, PatchINumbers[val], NULL);
            V_DrawPatch(x + 9, y, PatchNEGATIVE, NULL);
        }
        return;
    }
    if (val > 99)
    {
        patch = PatchINumbers[val / 100];
        V_DrawPatch(x, y, patch, NULL);
    }
    val = val % 100;
    if (val > 9 || oldval > 99)
    {
        patch = PatchINumbers[val / 10];
        V_DrawPatch(x + 9, y, patch, NULL);
    }
    val = val % 10;
    patch = PatchINumbers[val];
    V_DrawPatch(x + 18, y, patch, NULL);

    // [JN] Reset color translation if colored hud activated
    if (sbar_colored)
    dp_translation = NULL;
}

//---------------------------------------------------------------------------
//
// PROC DrBNumber
//
// Draws a three digit number using FontB
//
//---------------------------------------------------------------------------

static void DrBNumber(signed int val, int x, int y)
{
    patch_t *patch;
    patch_t *patch_n;
    int xpos;
    int oldval;

    // [JN] Declare a "minus" symbol in the big green font
    patch_n = W_CacheLumpName(DEH_String("FONTB13"), PU_CACHE);

    oldval = val;
    xpos = x;
    if (val < 0)
    {
        val = -val; // [JN] Support for negative values
        
        if (-val <= -99) // [JN] Do not drop below -99. Looks confusing, eh?
        val = 99;

        // [JN] Draw minus symbol with respection of digits placement.
        // However, values below -10 requires some correction in "x" placement.
        V_DrawShadowedPatch(xpos + (val <= 9 ? 16 : 8) - SHORT(patch_n->width)
                                                       / 2, y-1, patch_n);
    }
    if (val > 99)
    {
        patch = W_CacheLumpNum(FontBNumBase + val / 100, PU_CACHE);
        V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
    }
    val = val % 100;
    xpos += 12;
    if (val > 9 || oldval > 99)
    {
        patch = W_CacheLumpNum(FontBNumBase + val / 10, PU_CACHE);
        V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
    }
    val = val % 10;
    xpos += 12;
    patch = W_CacheLumpNum(FontBNumBase + val, PU_CACHE);
    V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);

    // [JN] Reset color translation if colored hud activated
    if (sbar_colored)
    dp_translation = NULL;
}

//---------------------------------------------------------------------------
//
// PROC DrSmallNumber
//
// [JN] Extended to draw a small three digit number.
//
//---------------------------------------------------------------------------

static void DrSmallNumber(int val, int x, int y, boolean draw_val1)
{
    int oldval = val;
    patch_t *patch;

    if (val == 1 && !draw_val1)
    {
        return;
    }
    if (val > 99)
    {
        patch = PatchSmNumbers[val / 100];
        V_DrawPatch(x, y, patch, NULL);
    }
    val = val % 100;
    if (val > 9 || oldval > 99)
    {
        patch = PatchSmNumbers[val / 10];
        V_DrawPatch(x + 4, y, patch, NULL);
    }
    val = val % 10;
    patch = PatchSmNumbers[val];
    V_DrawPatch(x + 8, y, patch, NULL);
}

//---------------------------------------------------------------------------
//
// PROC ShadeLine
//
//---------------------------------------------------------------------------

static void ShadeLine(int x, int y, int height, int shade)
{
    byte *dest;
    byte *shades;

    x <<= hires;
    y <<= hires;
    height <<= hires;

    shades = colormaps + 9 * 256 + shade * 2 * 256;
    dest = I_VideoBuffer + y * screenwidth + x;
    while (height--)
    {
        if (hires)
           *(dest + 1) = *(shades + *dest);
        *(dest) = *(shades + *dest);
        dest += screenwidth;
    }
}

//---------------------------------------------------------------------------
//
// PROC ShadeChain
//
//---------------------------------------------------------------------------

static void ShadeChain(void)
{
    int i;

    for (i = 0; i < 16; i++)
    {
        ShadeLine(277 + i + wide_delta, 190, 10, i / 2);
        ShadeLine(19 + i + wide_delta, 190, 10, 7 - (i / 2));
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawSoundInfo
//
// Displays sound debugging information.
//
//---------------------------------------------------------------------------

static void DrawSoundInfo(void)
{
    int i;
    SoundInfo_t s;
    ChanInfo_t *c;
    char text[32];
    int x;
    int y;
    int xPos[7] = { 1, 75, 112, 156, 200, 230, 260 };

    if (leveltime & 16)
    {
        if (english_language)
        {
            RD_M_DrawTextSmallENG(DEH_String("*** SOUND DEBUG INFO ***"), xPos[0]
                                                                          + wide_delta, 20, CR_NONE);
        }
        else
        {
            // *** ОТЛАДОЧНАЯ ИНФОРМАЦИЯ О ЗВУКЕ ***
            RD_M_DrawTextSmallRUS(DEH_String("*** JNKFLJXYFZ BYAJHVFWBZ J PDERT ***"), xPos[0]
                                                                                       + wide_delta, 20, CR_NONE);
        }
    }
    S_GetChannelInfo(&s);
    if (s.channelCount == 0)
    {
        return;
    }
    x = 0;
    RD_M_DrawTextA(DEH_String("NAME"), xPos[x++] + wide_delta, 30);
    RD_M_DrawTextA(DEH_String("MO.T"), xPos[x++] + wide_delta, 30);
    RD_M_DrawTextA(DEH_String("MO.X"), xPos[x++] + wide_delta, 30);
    RD_M_DrawTextA(DEH_String("MO.Y"), xPos[x++] + wide_delta, 30);
    RD_M_DrawTextA(DEH_String("ID"), xPos[x++] + wide_delta, 30);
    RD_M_DrawTextA(DEH_String("PRI"), xPos[x++] + wide_delta, 30);
    RD_M_DrawTextA(DEH_String("DIST"), xPos[x++] + wide_delta, 30);
    // [JN] "i < s.channelCount" replaced with "8".
    // Don't draw info for more than 8 channels.
    for (i = 0; i < 8; i++)
    {
        c = &s.chan[i];
        x = 0;
        y = 40 + i * 10;
        if (c->mo == NULL)
        {                       // Channel is unused
            RD_M_DrawTextA(DEH_String("------"), xPos[0] + wide_delta, y);
            continue;
        }
        M_snprintf(text, sizeof(text), "%s", c->name);
        M_ForceUppercase(text);
        RD_M_DrawTextA(text, xPos[x++] + wide_delta, y);
        M_snprintf(text, sizeof(text), "%d", c->mo->type);
        RD_M_DrawTextA(text, xPos[x++] + wide_delta, y);
        M_snprintf(text, sizeof(text), "%d", c->mo->x >> FRACBITS);
        RD_M_DrawTextA(text, xPos[x++] + wide_delta, y);
        M_snprintf(text, sizeof(text), "%d", c->mo->y >> FRACBITS);
        RD_M_DrawTextA(text, xPos[x++] + wide_delta, y);
        M_snprintf(text, sizeof(text), "%d", c->id);
        RD_M_DrawTextA(text, xPos[x++] + wide_delta, y);
        M_snprintf(text, sizeof(text), "%d", c->priority);
        RD_M_DrawTextA(text, xPos[x++] + wide_delta, y);
        M_snprintf(text, sizeof(text), "%d", c->distance);
        RD_M_DrawTextA(text, xPos[x++] + wide_delta, y);
    }
    UpdateState |= I_FULLSCRN;
    BorderNeedRefresh = true;
}

//---------------------------------------------------------------------------
//
// PROC SB_Drawer
//
//---------------------------------------------------------------------------

char patcharti[][10] = {
    {"ARTIBOX"},                // none
    {"ARTIINVU"},               // invulnerability
    {"ARTIINVS"},               // invisibility
    {"ARTIPTN2"},               // health
    {"ARTISPHL"},               // superhealth
    {"ARTIPWBK"},               // tomeofpower
    {"ARTITRCH"},               // torch
    {"ARTIFBMB"},               // firebomb
    {"ARTIEGGC"},               // egg
    {"ARTISOAR"},               // fly
    {"ARTIATLP"}                // teleport
};

char ammopic[][10] = {
    {"INAMGLD"},
    {"INAMBOW"},
    {"INAMBST"},
    {"INAMRAM"},
    {"INAMPNX"},
    {"INAMLOB"}
};

int SB_state = -1;
static int oldarti = 0;
static int oldartiCount = 0;
static int oldfrags = -9999;
static int oldammo = -1;
static int oldarmor = -1;
static int oldweapon = -1;
static int oldhealth = -1;
static int oldlife = -1;
static int oldkeys = -1;

int playerkeys = 0;



void SB_Drawer(void)
{
    int frame;
    int xval;
    const int xval_widget = demotimer || show_fps || local_time ? 50 : 0;
    static boolean hitCenterFrame;
    const int wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9 ? wide_delta : 0) + 2;
    // If automap active or map name is always shown, shift
    // widgets one line up to prevent drawing over map name.
    int map_active = automapactive ? 10 : 0;
    // Different languages have different string widths. 
    // Use a pointer to prevent extra conditions hitting.
    int (*StringWidth)(char *text) = english_language ? RD_M_TextAWidth : RD_M_TextSmallRUSWidth;

    CPlayer = &players[displayplayer];

    // [JN] Draw horns separatelly in non wide screen mode
    if (aspect_ratio < 2 && screenblocks <= 10 && automapactive && automap_overlay)
    {
        V_DrawPatch(0 + wide_delta, 148, PatchLTFCTOP, NULL);
        V_DrawPatch(290 + wide_delta, 148, PatchRTFCTOP, NULL);
    }

    // [JN] Level stats widgets.
    if (screenblocks <= 11 && !vanillaparm)
    {
        //
        // Placed at the bottom
        //
        if (stats_placement)
        {
            char str1[8], str2[16];  // kills
            char str3[8], str4[16];  // items
            char str5[8], str6[16];  // secret
            char str7[8], str8[16];  // skill

            if (((automapactive && automap_stats == 1) || automap_stats == 2))
            {
                // Kills:
                if (english_language)
                {
                    sprintf(str1, "K ");
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA(str1, wide_4_3, 136 - map_active);
                    dp_translation = NULL;

                    sprintf(str2, CPlayer->extrakillcount ? "%d+%d/%d " : "%d/%d ",
                                  CPlayer->killcount,
                                  CPlayer->extrakillcount ? CPlayer->extrakillcount : totalkills,
                                  totalkills);

                    dp_translation = hud_stats_color == 0 ? NULL :
                                     totalkills == 0 ? cr[CR_GREEN] :
                                     CPlayer->killcount == 0 ? cr[CR_RED] :
                                     CPlayer->killcount < totalkills ? cr[CR_YELLOW] : cr[CR_GREEN];

                    RD_M_DrawTextA(str2, wide_4_3 + StringWidth(str1), 136 - map_active);

                    dp_translation = NULL;
                }
                else
                {
                    sprintf(str1, "D ");
                    RD_M_DrawTextSmallRUS(str1, wide_4_3, 136 - map_active,
                                          hud_stats_color == 0 ? CR_NONE : CR_DARK);

                    sprintf(str2, CPlayer->extrakillcount ? "%d+%d/%d " : "%d/%d ",
                                  CPlayer->killcount,
                                  CPlayer->extrakillcount ? CPlayer->extrakillcount : totalkills,
                                  totalkills);

                    RD_M_DrawTextSmallRUS(str2, wide_4_3 + StringWidth(str1), 136 - map_active,
                                          hud_stats_color == 0 ? CR_NONE :
                                          totalkills == 0 ? CR_GREEN :
                                          CPlayer->killcount == 0 ? CR_RED :
                                          CPlayer->killcount < totalkills ? CR_YELLOW : CR_GREEN);
                }

                // Items:
                if (english_language)
                {
                    sprintf(str3, "I ");
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA(str3, wide_4_3 + StringWidth(str1)
                                                  + StringWidth(str2), 136 - map_active);
                    dp_translation = NULL;

                    sprintf(str4, "%d/%d ", CPlayer->itemcount, totalitems);

                    dp_translation = hud_stats_color == 0 ? NULL :
                                     totalitems == 0 ? cr[CR_GREEN] :
                                     CPlayer->itemcount == 0 ? cr[CR_RED] :
                                     CPlayer->itemcount < totalitems ? cr[CR_YELLOW] : cr[CR_GREEN];

                    RD_M_DrawTextA(str4, wide_4_3 + StringWidth(str1)
                                                  + StringWidth(str2)
                                                  + StringWidth(str3), 136 - map_active);

                    dp_translation = NULL;
                }
                else
                {
                    sprintf(str3, "G ");
                    RD_M_DrawTextSmallRUS(str3, wide_4_3 + StringWidth(str1)
                                                         + StringWidth(str2), 136 - map_active,
                                                           hud_stats_color == 0 ? CR_NONE : CR_DARK);

                    sprintf(str4, "%d/%d ", CPlayer->itemcount, totalitems);

                    RD_M_DrawTextSmallRUS(str4, wide_4_3 + StringWidth(str1)
                                                         + StringWidth(str2)
                                                         + StringWidth(str3), 136 - map_active,
                                                         hud_stats_color == 0 ? CR_NONE :
                                                         totalitems == 0 ? CR_GREEN :
                                                         CPlayer->itemcount == 0 ? CR_RED :
                                                         CPlayer->itemcount < totalitems ? CR_YELLOW : CR_GREEN);
                }

                // Secret:
                if (english_language)
                {
                    sprintf(str5, "S ");
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA(str5, wide_4_3 + StringWidth(str1)
                                                  + StringWidth(str2)
                                                  + StringWidth(str3)
                                                  + StringWidth(str4), 136 - map_active);
                    dp_translation = NULL;

                    sprintf(str6, "%d/%d ", CPlayer->secretcount, totalsecret);

                    dp_translation = hud_stats_color == 0 ? NULL :
                                     totalsecret == 0 ? cr[CR_GREEN] :
                                     CPlayer->secretcount == 0 ? cr[CR_RED] :
                                     CPlayer->secretcount < totalsecret ? cr[CR_YELLOW] : cr[CR_GREEN];

                    RD_M_DrawTextA(str6, wide_4_3 + StringWidth(str1)
                                                  + StringWidth(str2)
                                                  + StringWidth(str3)
                                                  + StringWidth(str4)
                                                  + StringWidth(str5), 136 - map_active);
                    dp_translation = NULL;
                }
                else
                {
                        sprintf(str5, "N ");
                        RD_M_DrawTextSmallRUS(str5, wide_4_3 + StringWidth(str1)
                                                             + StringWidth(str2)
                                                             + StringWidth(str3)
                                                             + StringWidth(str4), 136 - map_active,
                                                               hud_stats_color == 0 ? CR_NONE : CR_DARK);

                        sprintf(str6, "%d/%d ", CPlayer->secretcount, totalsecret);

                        RD_M_DrawTextSmallRUS(str6, wide_4_3 + StringWidth(str1)
                                                             + StringWidth(str2)
                                                             + StringWidth(str3)
                                                             + StringWidth(str4)
                                                             + StringWidth(str5), 136 - map_active,
                                                             hud_stats_color == 0 ? CR_NONE :
                                                             totalsecret == 0 ? CR_GREEN :
                                                             CPlayer->secretcount == 0 ? CR_RED :
                                                             CPlayer->secretcount < totalsecret ? CR_YELLOW : CR_GREEN);
                }

                // Skill Level:
                if (((automapactive && automap_skill == 1) || automap_skill == 2))
                {
                    if (english_language)
                    {
                        sprintf(str7, "SKL ");
                        dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                        RD_M_DrawTextA(str7, wide_4_3 + StringWidth(str1)
                                                      + StringWidth(str2)
                                                      + StringWidth(str3)
                                                      + StringWidth(str4)
                                                      + StringWidth(str5)
                                                      + StringWidth(str6), 136 - map_active);
                        dp_translation = NULL;
                    }
                    else
                    {
                        sprintf(str7, "CK; ");
                        RD_M_DrawTextSmallRUS(str7, wide_4_3 + StringWidth(str1)
                                                             + StringWidth(str2)
                                                             + StringWidth(str3)
                                                             + StringWidth(str4)
                                                             + StringWidth(str5)
                                                             + StringWidth(str6), 136 - map_active,
                                                               hud_stats_color == 0 ? CR_NONE : CR_DARK);
                    }

                    sprintf(str8, "%d", gameskill + 1);
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                    RD_M_DrawTextA(str8, wide_4_3 + StringWidth(str1)
                                                  + StringWidth(str2)
                                                  + StringWidth(str3)
                                                  + StringWidth(str4)
                                                  + StringWidth(str5)
                                                  + StringWidth(str6)
                                                  + StringWidth(str7), 136 - map_active);
                    dp_translation = NULL;
                }
            }

            // Level time:
            if (((automapactive && automap_level_time == 1) || automap_level_time == 2))
            {
                const int time = leveltime / TICRATE;
                const int x_shift = english_language ? 40 : 58;
                char str[16];

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA("LEVEL", wide_4_3, 126 - map_active);
                    dp_translation = NULL;
                }
                else
                {
                    RD_M_DrawTextSmallRUS("EHJDTYM", wide_4_3, 126 - map_active,
                                          hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                sprintf(str, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                RD_M_DrawTextA(str, wide_4_3 + x_shift, 126 - map_active);
                dp_translation = NULL;
            }

            // Total Time:
            if (((automapactive && automap_total_time == 1) || automap_total_time == 2))
            {
                const int totaltime = (totalleveltimes / TICRATE) + (leveltime / TICRATE);
                const int x_shift = english_language ? 40 : 58;
                char str1[16];
                char str2[16];

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    sprintf(str1, "TOTAL");
                    RD_M_DrawTextA(str1, wide_4_3, 116 - map_active);
                    dp_translation = NULL;
                }
                else
                {
                    sprintf(str1, "J,OTT");
                    RD_M_DrawTextSmallRUS(str1, wide_4_3, 116 - map_active,
                                          hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                sprintf(str2, "%02d:%02d:%02d", totaltime/3600, (totaltime%3600)/60, totaltime%60);

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                RD_M_DrawTextA(str2, wide_4_3 + x_shift, 116 - map_active);
                dp_translation = NULL;
            }

            // Player coords:
            if (((automapactive && automap_coords == 1) || automap_coords == 2))
            {
                char str[128];

                if (stats_placement)
                {
                    map_active += 20;
                }

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                RD_M_DrawTextA("X:", wide_4_3, 95 - map_active);
                RD_M_DrawTextA("Y:", wide_4_3, 104 - map_active);
                RD_M_DrawTextA("Z:", wide_4_3, 113 - map_active);
                RD_M_DrawTextA("ANG:", wide_4_3, 122 - map_active);
                dp_translation = NULL;

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                sprintf(str, "%d", CPlayer->mo->x >> FRACBITS);
                RD_M_DrawTextA(str, wide_4_3 + 16, 95 - map_active);
                sprintf(str, "%d", CPlayer->mo->y >> FRACBITS);
                RD_M_DrawTextA(str, wide_4_3 + 16, 104 - map_active);
                sprintf(str, "%d", CPlayer->mo->z >> FRACBITS);
                RD_M_DrawTextA(str, wide_4_3 + 16, 113 - map_active);
                sprintf(str, "%d", CPlayer->mo->angle / ANG1);
                RD_M_DrawTextA(str, wide_4_3 + 32, 122 - map_active);
                dp_translation = NULL;
            }
        }
        //
        // Placed at the top
        //
        else
        {
            char text[128];
            const int time = leveltime / TICRATE;
            const int totaltime = (totalleveltimes / TICRATE) + (leveltime / TICRATE);
            const int net_y = netgame ? 10 : 0;  // [JN] Shift one line down for chat string.

            if (((automapactive && automap_stats == 1) || automap_stats == 2))
            {
                // Kills:
                sprintf(text, CPlayer->extrakillcount ? "%d+%d/%d" : "%d/%d",
                        CPlayer->killcount,
                        CPlayer->extrakillcount ? CPlayer->extrakillcount : totalkills,
                        totalkills);

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA("K:", wide_4_3, 9 + net_y);
                    dp_translation = NULL;
                }
                else
                {
                    RD_M_DrawTextSmallRUS("D:", wide_4_3, 9 + net_y, hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                dp_translation = hud_stats_color == 0 ? NULL :
                                 totalkills == 0 ? cr[CR_GREEN] :
                                 CPlayer->killcount == 0 ? cr[CR_RED] :
                                 CPlayer->killcount < totalkills ? cr[CR_YELLOW] : cr[CR_GREEN];
                RD_M_DrawTextA(text, wide_4_3 + 16, 9 + net_y);
                dp_translation = NULL;

                // Items:
                sprintf(text, "%d/%d", CPlayer->itemcount, totalitems);

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA("I:", wide_4_3, 19 + net_y);
                    dp_translation = NULL;
                }
                else
                {
                    RD_M_DrawTextSmallRUS("G:", wide_4_3, 19 + net_y, hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                dp_translation = hud_stats_color == 0 ? NULL :
                                 totalitems == 0 ? cr[CR_GREEN] :
                                 CPlayer->itemcount == 0 ? cr[CR_RED] :
                                 CPlayer->itemcount < totalitems ? cr[CR_YELLOW] : cr[CR_GREEN];
                RD_M_DrawTextA(text, wide_4_3 + 16, 19 + net_y);
                dp_translation = NULL;

                // Secret:
                sprintf(text, "%d/%d", CPlayer->secretcount, totalsecret);

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA("S:", wide_4_3, 29 + net_y);
                    dp_translation = NULL;
                }
                else
                {
                    RD_M_DrawTextSmallRUS("N:", wide_4_3, 29 + net_y, hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                dp_translation = hud_stats_color == 0 ? NULL :
                                 totalsecret == 0 ? cr[CR_GREEN] :
                                 CPlayer->secretcount == 0 ? cr[CR_RED] :
                                 CPlayer->secretcount < totalsecret ? cr[CR_YELLOW] : cr[CR_GREEN];
                RD_M_DrawTextA(text, wide_4_3 + 16, 29 + net_y);
                dp_translation = NULL;
            }

            // Skill Level:
            if (((automapactive && automap_skill == 1) || automap_skill == 2))
            {
                sprintf(text, "%d", gameskill+1);

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA("SKL:", wide_4_3, 39 + net_y);
                    dp_translation = NULL;
                }
                else
                {
                    RD_M_DrawTextSmallRUS("CK;:", wide_4_3, 39 + net_y, hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                RD_M_DrawTextA(text, wide_4_3 + (english_language ? 31 : 36), 39 + net_y);
                dp_translation = NULL;
            }

            // Level Time:
            if ((automapactive && automap_level_time == 1) || automap_level_time == 2)
            {
                sprintf(text, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA("LEVEL", wide_4_3, 49);
                    dp_translation = NULL;
                }
                else
                {
                    RD_M_DrawTextSmallRUS("EHJDTYM", wide_4_3, 49, hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                RD_M_DrawTextA(text, wide_4_3, 59);
                dp_translation = NULL;
            }

            // Total Time:
            if ((automapactive && automap_total_time == 1) || automap_total_time == 2)
            {
                sprintf(text, "%02d:%02d:%02d", totaltime/3600, (totaltime%3600)/60, totaltime%60);

                if (english_language)
                {
                    dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                    RD_M_DrawTextA("TOTAL", wide_4_3, 69);
                    dp_translation = NULL;
                }
                else
                {
                    RD_M_DrawTextSmallRUS("J,OTT", wide_4_3, 69, hud_stats_color == 0 ? CR_NONE : CR_DARK);
                }

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                RD_M_DrawTextA(text, wide_4_3, 79);
                dp_translation = NULL;
            }

            // [JN] Player coords
            if ((automapactive && automap_coords == 1) || automap_coords == 2)
            {
                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_DARK];
                RD_M_DrawTextA("X:", wide_4_3, 89);
                RD_M_DrawTextA("Y:", wide_4_3, 99);
                RD_M_DrawTextA("Z:", wide_4_3, 109);
                RD_M_DrawTextA("ANG:", wide_4_3, 119);
                dp_translation = NULL;

                dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GRAY];
                M_snprintf(text, sizeof(text), "%d", CPlayer->mo->x >> FRACBITS);
                RD_M_DrawTextA(text, wide_4_3 + 16, 89);
                M_snprintf(text, sizeof(text), "%d", CPlayer->mo->y >> FRACBITS);
                RD_M_DrawTextA(text, wide_4_3 + 16, 99);
                M_snprintf(text, sizeof(text), "%d", CPlayer->mo->z >> FRACBITS);
                RD_M_DrawTextA(text, wide_4_3 + 16, 109);
                M_snprintf(text, sizeof(text), "%d", CPlayer->mo->angle / ANG1);
                RD_M_DrawTextA(text, wide_4_3 + 32, 119);
                dp_translation = NULL;
            }
        }
    }

    // Sound info debug stuff
    if (DebugSound == true)
    {
        DrawSoundInfo();
    }

    // [JN] Draw crosshair
    if (crosshair_draw && !automapactive && !vanillaparm)
    {
        Crosshair_Draw();
    }

    if ((screenblocks >= 11 && !automapactive) 
    ||  (screenblocks >= 11 && automapactive && automap_overlay))
    {
        if (screenblocks == 11) // [JN] Draw only in 11 screen size, 12 is clean full screen
        DrawFullScreenStuff();
    }
    else
    {
        // [JN] Draw side screen borders in wide screen mode.
        if (aspect_ratio >= 2)
        {
            V_UseBuffer(st_backing_screen);
            
            // [crispy] this is our own local copy of R_FillBackScreen() to
            // fill the entire background of st_backing_screen with the bezel pattern,
            // so it appears to the left and right of the status bar in widescreen mode
            if ((screenwidth >> hires) != ORIGWIDTH)
            {
                int x, y;
                byte *src;
                byte *dest;
                char *name = DEH_String(gamemode == shareware ? "FLOOR04" : "FLAT513");
                const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;
        
                src = W_CacheLumpName(name, PU_CACHE);
                dest = st_backing_screen;
        
                // [JN] Variable HUD detail level.
                for (y = SCREENHEIGHT-SBARHEIGHT; y < SCREENHEIGHT; y++)
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
                    patch_t *patch = W_CacheLumpName(DEH_String("BORDB"), PU_CACHE);
                
                    for (x = 0; x < screenwidth; x += 16)
                    {
                        V_DrawPatch(x, 0, patch, NULL);
                    }
                }
            }

            V_RestoreBuffer();

            V_CopyRect(0, 0, st_backing_screen, 
                       origwidth, 42, 0, 158);
        }

        if (SB_state == -1)
        {
            V_DrawPatch(0 + wide_delta, 158, PatchBARBACK, NULL);
            oldhealth = -1;
        }
        DrawCommonBar();
        if (!inventory)
        {
            if (SB_state != 0)
            {
                // Main interface
                V_DrawPatch(34 + wide_delta, 160, 
                            english_language ? PatchSTATBAR : PatchSTATBAR_RUS, NULL);
                oldarti = 0;
                oldammo = -1;
                oldarmor = -1;
                oldweapon = -1;
                oldfrags = -9999;       //can't use -1, 'cuz of negative frags
                oldlife = -1;
                oldkeys = -1;
            }
            DrawMainBar();
            SB_state = 0;
        }
        else
        {
            if (SB_state != 1)
            {
                V_DrawPatch(34 + wide_delta, 160, PatchINVBAR, NULL);
            }
            DrawInventoryBar();
            SB_state = 1;
        }
    }

    // [JN] Apply golden eyes to HUD gargoyles while Ring of Invincibility
    if ((screenblocks <= 10 || (automapactive && !automap_overlay))
    && (CPlayer->cheats & CF_GODMODE
    || (CPlayer->powers[pw_invulnerability] && !vanillaparm)))
    {
        V_DrawPatch(16 + wide_delta, 167,
                    W_CacheLumpName(DEH_String("GOD1"), PU_CACHE), NULL);
        V_DrawPatch(287 + wide_delta, 167,
                    W_CacheLumpName(DEH_String("GOD2"), PU_CACHE), NULL);
    }

    // Flight icons
    if (CPlayer->powers[pw_flight])
    {
        if (CPlayer->powers[pw_flight] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_flight] & 16))
        {
            frame = (leveltime / 3) & 15;
            if (CPlayer->mo->flags2 & MF2_FLY)
            {
                if (hitCenterFrame && (frame != 15 && frame != 0))
                {
                    V_DrawPatch(20 + (wide_4_3 ? wide_delta : 0), 17,
                                W_CacheLumpNum(spinflylump + 15, PU_CACHE), NULL);
                }
                else
                {
                    V_DrawPatch(20 + (wide_4_3 ? wide_delta : 0), 17,
                                W_CacheLumpNum(spinflylump + frame, PU_CACHE), NULL);
                    hitCenterFrame = false;
                }
            }
            else
            {
                if (!hitCenterFrame && (frame != 15 && frame != 0))
                {
                    V_DrawPatch(20 + (wide_4_3 ? wide_delta : 0), 17,
                                W_CacheLumpNum(spinflylump + frame, PU_CACHE), NULL);
                    hitCenterFrame = false;
                }
                else
                {
                    V_DrawPatch(20 + (wide_4_3 ? wide_delta : 0), 17,
                                W_CacheLumpNum(spinflylump + 15, PU_CACHE), NULL);
                    hitCenterFrame = true;
                }
            }
            // [JN] Draw artifact timer.
            if (show_artifacts_timer && !vanillaparm)
            {
                xval = CPlayer->powers[pw_flight] / TICRATE > 99 ? 0 :
                       CPlayer->powers[pw_flight] / TICRATE >  9 ? 2 : 4;

                if (show_artifacts_timer == 2)
                {
                    dp_translation = cr[CR_WHITE];
                }
                if (show_artifacts_timer == 3)
                {
                    if (CPlayer->powers[pw_flight] < FLIGHTTICS / 4)
                    dp_translation = cr[CR_RED];
                    else if (CPlayer->powers[pw_flight] > FLIGHTTICS / 2)
                    dp_translation = cr[CR_GREEN];
                }

                DrSmallNumber(CPlayer->powers[pw_flight] / TICRATE,
                              (14 - xval) + (wide_4_3 ? wide_delta : 0), 30, true);

                dp_translation = NULL;
            }
        }
        BorderNeedRefresh = true;
        UpdateState |= I_MESSAGES;
    }

    if (CPlayer->powers[pw_weaponlevel2] && !CPlayer->chickenTics)
    {
        if (CPlayer->powers[pw_weaponlevel2] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_weaponlevel2] & 16))
        {
            frame = (leveltime / 3) & 15;
            // [JN] Do not obstruct clock widget
            V_DrawPatch(300 + (wide_4_3 ? wide_delta : wide_delta*2) - xval_widget, 17,
                        W_CacheLumpNum(spinbooklump + frame, PU_CACHE), NULL);

            // [JN] Draw artifact timer.
            if (show_artifacts_timer && !vanillaparm)
            {
                xval = CPlayer->powers[pw_weaponlevel2] / TICRATE > 99 ? 0 :
                       CPlayer->powers[pw_weaponlevel2] / TICRATE >  9 ? 2 : 4;

                if (show_artifacts_timer == 2)
                {
                    dp_translation = cr[CR_WHITE];
                }
                if (show_artifacts_timer == 3)
                {
                    if (CPlayer->powers[pw_weaponlevel2] < WPNLEV2TICS / 4)
                    dp_translation = cr[CR_RED];
                    else if (CPlayer->powers[pw_weaponlevel2] > WPNLEV2TICS / 2)
                    dp_translation = cr[CR_GREEN];
                }

                DrSmallNumber(CPlayer->powers[pw_weaponlevel2] / TICRATE,
                              (294 - xval) + (wide_4_3 ? wide_delta : wide_delta*2) - xval_widget, 30, true);

                dp_translation = NULL;
            }
        }
        BorderNeedRefresh = true;
        UpdateState |= I_MESSAGES;
    }

    // [JN] Draw all active artifacts.
    if (show_all_artifacts && !vanillaparm)
    {
        if (CPlayer->powers[pw_invisibility] && !CPlayer->chickenTics)
        {
            if (CPlayer->powers[pw_invisibility] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_invisibility] & 16))
            {
                V_DrawPatch(40 + (wide_4_3 ? wide_delta : 0), 1,
                            W_CacheLumpName(DEH_String("ARTIINVS"), PU_CACHE), NULL);

                // [JN] Draw artifact timer.
                if (show_artifacts_timer && !vanillaparm)
                {
                    xval = CPlayer->powers[pw_invisibility] / TICRATE > 99 ? 0 :
                           CPlayer->powers[pw_invisibility] / TICRATE >  9 ? 2 : 4;

                    if (show_artifacts_timer == 2)
                    {
                        dp_translation = cr[CR_WHITE];
                    }
                    if (show_artifacts_timer == 3)
                    {
                        if (CPlayer->powers[pw_invisibility] < INVISTICS / 4)
                        dp_translation = cr[CR_RED];
                        else if (CPlayer->powers[pw_invisibility] > INVISTICS / 2)
                        dp_translation = cr[CR_GREEN];
                    }

                    DrSmallNumber(CPlayer->powers[pw_invisibility] / TICRATE,
                                (49 - xval) + (wide_4_3 ? wide_delta : 0), 30, true);

                    dp_translation = NULL;
                }
            }
            BorderNeedRefresh = true;
            UpdateState |= I_MESSAGES;
        }

        if (CPlayer->powers[pw_infrared] && !CPlayer->chickenTics)
        {
            if (CPlayer->powers[pw_infrared] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_infrared] & 16))
            {
                V_DrawPatch(74 + (wide_4_3 ? wide_delta : 0), 1,
                            W_CacheLumpName(DEH_String("ARTITRCH"), PU_CACHE), NULL);

                // [JN] Draw artifact timer.
                if (show_artifacts_timer && !vanillaparm)
                {
                    xval = CPlayer->powers[pw_infrared] / TICRATE > 99 ? 0 :
                           CPlayer->powers[pw_infrared] / TICRATE >  9 ? 2 : 4;

                    if (show_artifacts_timer == 2)
                    {
                        dp_translation = cr[CR_WHITE];
                    }
                    if (show_artifacts_timer == 3)
                    {
                        if (CPlayer->powers[pw_infrared] < INFRATICS / 4)
                        dp_translation = cr[CR_RED];
                        else if (CPlayer->powers[pw_infrared] > INFRATICS / 2)
                        dp_translation = cr[CR_GREEN];
                    }

                    DrSmallNumber(CPlayer->powers[pw_infrared] / TICRATE,
                                (83 - xval) + (wide_4_3 ? wide_delta : 0), 30, true);

                    dp_translation = NULL;
                }
            }
            BorderNeedRefresh = true;
            UpdateState |= I_MESSAGES;
        }

        if (CPlayer->powers[pw_invulnerability] && !CPlayer->chickenTics)
        {
            if (CPlayer->powers[pw_invulnerability] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_invulnerability] & 16))
            {
                V_DrawPatch(251 + (wide_4_3 ? wide_delta : wide_delta*2) - xval_widget, 1,
                            W_CacheLumpName(DEH_String("ARTIINVU"), PU_CACHE), NULL);

                // [JN] Draw artifact timer.
                if (show_artifacts_timer && !vanillaparm)
                {
                    xval = CPlayer->powers[pw_invulnerability] / TICRATE > 99 ? 0 :
                        CPlayer->powers[pw_invulnerability] / TICRATE >  9 ? 2 : 4;

                    if (show_artifacts_timer == 2)
                    {
                        dp_translation = cr[CR_WHITE];
                    }
                    if (show_artifacts_timer == 3)
                    {
                        if (CPlayer->powers[pw_invulnerability] < INVULNTICS / 4)
                        dp_translation = cr[CR_RED];
                        else if (CPlayer->powers[pw_invulnerability] > INVULNTICS / 2)
                        dp_translation = cr[CR_GREEN];
                    }

                    DrSmallNumber(CPlayer->powers[pw_invulnerability] / TICRATE,
                                (259 - xval) + (wide_4_3 ? wide_delta : wide_delta*2) - xval_widget, 30, true);

                    dp_translation = NULL;
                }
            }
            BorderNeedRefresh = true;
            UpdateState |= I_MESSAGES;
        }
    }

    // [crispy] demo timer widget
    if (demoplayback && (demotimer == 1 || demotimer == 3))
    {
        SB_DrawDemoTimer(demotimerdir ? (deftotaldemotics - defdemotics) : defdemotics);
    }
    else if (demorecording && (demotimer == 2 || demotimer == 3))
    {
        SB_DrawDemoTimer(leveltime);
    }

    // [JN] Draw ammo widget.
    if (ammo_widget && !vanillaparm)
    {
        SB_Draw_Ammo_Widget();
    }

    // [crispy] demo progress bar
    if (demoplayback && demobar)
    {
        SB_DemoProgressBar();
    }

    // [JN] Always update whole status bar.
    // TODO: remove bunch of other update conditions.
    SB_state = -1;
}

//---------------------------------------------------------------------------
//
// PROC DrawCommonBar
//
//---------------------------------------------------------------------------

void DrawCommonBar(void)
{
    int chainY;
    int healthPos;

    V_DrawPatch(0 + wide_delta, 148, PatchLTFCTOP, NULL);
    V_DrawPatch(290 + wide_delta, 148, PatchRTFCTOP, NULL);

    if (oldhealth != HealthMarker)
    {
        oldhealth = HealthMarker;
        healthPos = HealthMarker;
        if (healthPos < 0)
        {
            healthPos = 0;
        }
        if (healthPos > 100)
        {
            healthPos = 100;
        }
        healthPos = (healthPos * 256) / 100;
        chainY =
            (HealthMarker == CPlayer->mo->health) ? 191 : 191 + ChainWiggle;
        V_DrawPatch(0 + wide_delta, 190, PatchCHAINBACK, NULL);
        V_DrawPatch(2 + (healthPos % 17) + wide_delta, chainY, PatchCHAIN, NULL);

        // [JN] Health gem:
        {
            // Set appropriate gem patch. "2" means LIFEGEM2 patch, 
            // suitable for proper color red-to-x translation.
            int gem_patch;
            
            if (netgame)
            {
                // In netgame, gem can be colored by player's health
                // or represent current player color in spy mode.
                gem_patch = sbar_colored_gem ? 2 : displayplayer;
            }
            else
            {
                // In singleplayer, gem is always red patch.
                gem_patch = 2;
            }


            if (sbar_colored_gem && !vanillaparm)
            {
                if (CPlayer->cheats & CF_GODMODE
                ||  CPlayer->powers[pw_invulnerability])
                {
                    dp_translation = cr[CR_RED2WHITE_HERETIC];
                }
                else
                if (CPlayer->mo->health <= 0)
                {
                    dp_translation = cr[CR_RED2BLACK_HERETIC];
                }
                else
                if (CPlayer->mo->health >= 67)
                {
                    dp_translation = sbar_colored_gem == 1 ? cr[CR_RED2GREEN_HERETIC]
                                                           : cr[CR_RED2MIDGREEN_HERETIC];
                }
                else
                if (CPlayer->mo->health >= 34)
                {
                    dp_translation = sbar_colored_gem == 1 ? cr[CR_RED2YELLOW_HERETIC]
                                                           : cr[CR_RED2GOLD_HERETIC];
                }
                else
                {
                    dp_translation = sbar_colored_gem == 2 ? cr[CR_RED2DARKRED_HERETIC]
                                                           : NULL;
                }
            }
            
            V_DrawPatch(17 + healthPos + wide_delta, chainY, PatchLIFEGEM[gem_patch], NULL);
            dp_translation = NULL;
        }
        V_DrawPatch(0 + wide_delta, 190, PatchLTFACE, NULL);
        V_DrawPatch(276 + wide_delta, 190, PatchRTFACE, NULL);
        ShadeChain();
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawMainBar
//
//---------------------------------------------------------------------------

void DrawMainBar(void)
{
    int i;
    int temp;

    // Ready artifact
    if (ArtifactFlash)
    {
        V_DrawPatch(180 + wide_delta, 161, PatchBLACKSQ, NULL);

        temp = W_GetNumForName(DEH_String("useartia")) + ArtifactFlash - 1;

        V_DrawPatch(182 + wide_delta, 161, W_CacheLumpNum(temp, PU_CACHE), NULL);
        ArtifactFlash--;
        oldarti = -1;   // so that the correct artifact fills in after the flash
        UpdateState |= I_STATBAR;
    }
    else if (oldarti != CPlayer->readyArtifact
             || oldartiCount != CPlayer->inventory[inv_ptr].count)
    {
        V_DrawPatch(180 + wide_delta, 161, PatchBLACKSQ, NULL);
        if (CPlayer->readyArtifact > 0)
        {
            V_DrawPatch(179 + wide_delta, 160,
                        W_CacheLumpName(DEH_String(patcharti[CPlayer->readyArtifact]),
                                        PU_CACHE), NULL);
            DrSmallNumber(CPlayer->inventory[inv_ptr].count, 197 + wide_delta, 182, false);
        }
        oldarti = CPlayer->readyArtifact;
        oldartiCount = CPlayer->inventory[inv_ptr].count;
        UpdateState |= I_STATBAR;
    }

    // Frags
    if (deathmatch)
    {
        temp = 0;
        for (i = 0; i < MAXPLAYERS; i++)
        {
            temp += CPlayer->frags[i];
        }

        // [JN] Always update frags value, needed for colored HUD
        // if (temp != oldfrags)
        {
            V_DrawPatch(57, 171, PatchARMCLEAR, NULL);

            // [JN] Colored HUD: Frags
            if (sbar_colored && !vanillaparm)
            {
                if (temp < 0)
                dp_translation = cr[CR_RED];
                else if (temp == 0)
                dp_translation = cr[CR_YELLOW];
                else
                dp_translation = cr[CR_GREEN];
            }
            DrINumber(temp, 61 + wide_delta, 170);
            oldfrags = temp;
            UpdateState |= I_STATBAR;
        }
    }
    else
    {
        temp = HealthMarker;
        if (temp < 0)
        {
            temp = 0;
        }
        // [JN] Negative health: use actual value
        else if (negative_health && !vanillaparm)
        {
            temp = CPlayer->mo->health;
        }
        else if (temp > 100)
        {
            temp = 100;
        }

        // [JN] Always update health value, needed for colored HUD
        // if (oldlife != temp)
        {
            oldlife = temp;
            V_DrawPatch(57 + wide_delta, 171, PatchARMCLEAR, NULL);

            // [JN] Colored HUD: Health
            if (sbar_colored && !vanillaparm)
            {
                if ((CPlayer->cheats & CF_GODMODE) || CPlayer->powers[pw_invulnerability])
                dp_translation = cr[CR_BRIGHTYELLOW];
                else if (CPlayer->mo->health >= 67)
                dp_translation = cr[CR_GREEN];
                else if (CPlayer->mo->health >= 34)
                dp_translation = cr[CR_YELLOW];
                else
                dp_translation = cr[CR_RED];
            }

            DrINumber(temp, 61 + wide_delta, 170);
            UpdateState |= I_STATBAR;
        }
    }

    // Keys
    if (oldkeys != playerkeys)
    {
        // [JN] Reset color translation if colored hud activated
        if (sbar_colored)
        dp_translation = NULL;
        
        if (CPlayer->keys[key_yellow])
        {
            V_DrawPatch(153 + wide_delta, 164,
                        W_CacheLumpName(DEH_String("ykeyicon"), PU_CACHE), NULL);
        }
        else
        {
            if (CPlayer->yellowkeyTics && !vanillaparm)
            {
                V_DrawFadePatch(153 + wide_delta, 164, W_CacheLumpName(DEH_String("ykeyicon"), PU_CACHE), 
                                CPlayer->yellowkeyTics > 13 ? transtable90 :
                                CPlayer->yellowkeyTics > 11 ? transtable80 :
                                CPlayer->yellowkeyTics >  9 ? transtable70 :
                                CPlayer->yellowkeyTics >  7 ? transtable60 :
                                CPlayer->yellowkeyTics >  5 ? transtable50 :
                                CPlayer->yellowkeyTics >  4 ? transtable40 :
                                CPlayer->yellowkeyTics >  3 ? transtable30 :
                                CPlayer->yellowkeyTics >  2 ? transtable20 :
                                                              transtable10);
            }
        }
        if (CPlayer->keys[key_green])
        {
            V_DrawPatch(153 + wide_delta, 172,
                        W_CacheLumpName(DEH_String("gkeyicon"), PU_CACHE), NULL);
        }
        else
        {
            if (CPlayer->greenkeyTics && !vanillaparm)
            {
                V_DrawFadePatch(153 + wide_delta, 172, W_CacheLumpName(DEH_String("gkeyicon"), PU_CACHE), 
                                CPlayer->greenkeyTics > 13 ? transtable90 :
                                CPlayer->greenkeyTics > 11 ? transtable80 :
                                CPlayer->greenkeyTics >  9 ? transtable70 :
                                CPlayer->greenkeyTics >  7 ? transtable60 :
                                CPlayer->greenkeyTics >  5 ? transtable50 :
                                CPlayer->greenkeyTics >  4 ? transtable40 :
                                CPlayer->greenkeyTics >  3 ? transtable30 :
                                CPlayer->greenkeyTics >  2 ? transtable20 :
                                                             transtable10);
            }
        }
        if (CPlayer->keys[key_blue])
        {
            V_DrawPatch(153 + wide_delta, 180,
                        W_CacheLumpName(DEH_String("bkeyicon"), PU_CACHE), NULL);
        }
        else
        {
            if (CPlayer->bluekeyTics && !vanillaparm)
            {
                V_DrawFadePatch(153 + wide_delta, 180, W_CacheLumpName(DEH_String("bkeyicon"), PU_CACHE), 
                                CPlayer->bluekeyTics > 13 ? transtable90 :
                                CPlayer->bluekeyTics > 11 ? transtable80 :
                                CPlayer->bluekeyTics >  9 ? transtable70 :
                                CPlayer->bluekeyTics >  7 ? transtable60 :
                                CPlayer->bluekeyTics >  5 ? transtable50 :
                                CPlayer->bluekeyTics >  4 ? transtable40 :
                                CPlayer->bluekeyTics >  3 ? transtable30 :
                                CPlayer->bluekeyTics >  2 ? transtable20 :
                                                            transtable10);
            }
        }
        oldkeys = playerkeys;
        UpdateState |= I_STATBAR;
    }
    // Ammo
    temp = CPlayer->ammo[wpnlev1info[CPlayer->readyweapon].ammo];
    if (oldammo != temp || oldweapon != CPlayer->readyweapon)
    {
        V_DrawPatch(108 + wide_delta, 161, PatchBLACKSQ, NULL);
        if (temp && CPlayer->readyweapon > 0 && CPlayer->readyweapon < 7)
        {
            // [JN] Colored HUD: Ammo
            if (sbar_colored && !vanillaparm)
            {
                int ammo =  CPlayer->ammo[wpnlev1info[CPlayer->readyweapon].ammo];
                int fullammo = CPlayer->maxammo[wpnlev1info[CPlayer->readyweapon].ammo];
                
                if (ammo < fullammo/4)
                dp_translation = cr[CR_RED];
                else if (ammo < fullammo/2)
                dp_translation = cr[CR_YELLOW];
                else
                dp_translation = cr[CR_GREEN]; 
            }
            
            DrINumber(temp, 109 + wide_delta, 162);
            V_DrawPatch(111 + wide_delta, 172,
                        W_CacheLumpName(DEH_String(ammopic[CPlayer->readyweapon - 1]),
                                        PU_CACHE), NULL);
        }
        oldammo = temp;
        oldweapon = CPlayer->readyweapon;
        UpdateState |= I_STATBAR;
    }

    // Armor
    // [JN] Always update armor value, needed for colored HUD
    // if (oldarmor != CPlayer->armorpoints)
    {
        V_DrawPatch(224 + wide_delta, 171, PatchARMCLEAR, NULL);

        // [JN] Colored HUD: Armor
        if (sbar_colored && !vanillaparm)
        {
            if (CPlayer->cheats & CF_GODMODE || CPlayer->powers[pw_invulnerability])
            dp_translation = cr[CR_BRIGHTYELLOW];
            else if (CPlayer->armortype >= 2)
            dp_translation = cr[CR_YELLOW];
            else if (CPlayer->armortype == 1)
            dp_translation = cr[CR_WHITE];
            else
            dp_translation = cr[CR_RED];
        }

        DrINumber(CPlayer->armorpoints, 228 + wide_delta, 170);
        oldarmor = CPlayer->armorpoints;
        UpdateState |= I_STATBAR;
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawInventoryBar
//
//---------------------------------------------------------------------------

void DrawInventoryBar(void)
{
    char *patch;
    int i;
    int x;

    x = inv_ptr - curpos;
    UpdateState |= I_STATBAR;
    V_DrawPatch(34 + wide_delta, 160, PatchINVBAR, NULL);
    for (i = 0; i < 7; i++)
    {
        //V_DrawPatch(50+i*31, 160, W_CacheLumpName("ARTIBOX", PU_CACHE));
        if (CPlayer->inventorySlotNum > x + i
            && CPlayer->inventory[x + i].type != arti_none)
        {
            patch = DEH_String(patcharti[CPlayer->inventory[x + i].type]);

            V_DrawPatch(50 + i * 31 + wide_delta, 160,
                        W_CacheLumpName(patch, PU_CACHE), NULL);
            DrSmallNumber(CPlayer->inventory[x + i].count, 65 + i * 31
                                                              + wide_delta, 182, false);
        }
    }
    V_DrawPatch(50 + curpos * 31 + wide_delta, 189, PatchSELECTBOX, NULL);

    if (x > 0)
    {
        V_DrawPatch(38 + wide_delta, 159, !(leveltime & 4) ? PatchINVLFGEM1 :
                    PatchINVLFGEM2, NULL);
    }
    if (CPlayer->inventorySlotNum - x > 7)
    {
        V_DrawPatch(269 + wide_delta, 159, !(leveltime & 4) ?
                    PatchINVRTGEM1 : PatchINVRTGEM2, NULL);
    }
}

void DrawFullScreenStuff(void)
{
    char *patch;
    int i;
    int x;
    int temp;
    // [JN] Definition of full screen ammo
    int fs_ammo = CPlayer->ammo[wpnlev1info[CPlayer->readyweapon].ammo];

    UpdateState |= I_FULLSCRN;

    if (CPlayer->mo->health > 0)
    {
        // [JN] Colored HUD: Health
        if (sbar_colored && !vanillaparm)
        {
            if (CPlayer->cheats & CF_GODMODE || CPlayer->powers[pw_invulnerability])
            dp_translation = cr[CR_FLAME];
            else if (CPlayer->mo->health >= 67)
            dp_translation = cr[CR_GREEN];
            else if (CPlayer->mo->health >= 34)
            dp_translation = cr[CR_BRIGHTYELLOW];
            else
            dp_translation = cr[CR_RED];
        }

        DrBNumber(CPlayer->mo->health, 5, 176);
    }
    // [JN] Negative and zero health: can't drop below -99, drawing, colorizing
    else
    {
        if (sbar_colored && !vanillaparm)
        dp_translation = cr[CR_RED];

        if (CPlayer->mo->health <= -99)
        CPlayer->mo->health = -99;

        if (negative_health && !vanillaparm)
        DrBNumber(CPlayer->mo->health, 5, 176);
        else
        DrBNumber(0, 5, 176);
    }

    // [JN] Always draw ammo in full screen HUD
    if (fs_ammo && CPlayer->readyweapon > 0 && CPlayer->readyweapon < 7)
    {
        // [JN] Colored HUD: Ammo
        if (sbar_colored && !vanillaparm)
        {
            int ammo =  CPlayer->ammo[wpnlev1info[CPlayer->readyweapon].ammo];
            int fullammo = CPlayer->maxammo[wpnlev1info[CPlayer->readyweapon].ammo];

            if (ammo < fullammo/4)
            dp_translation = cr[CR_RED];
            else if (ammo < fullammo/2)
            dp_translation = cr[CR_BRIGHTYELLOW];
            else
            dp_translation = cr[CR_GREEN];
        }

        DrBNumber(fs_ammo, 274 + (wide_delta * 2), 176);
    }

    if (deathmatch)
    {
        temp = 0;

        // [JN] Do not draw frag counter below opened inventory, 
        // because it looks aesthetically bad.
        if (!inventory)
        {
            for (i = 0; i < MAXPLAYERS; i++)
            {
                if (playeringame[i])
                {
                    temp += CPlayer->frags[i];
                }
            }

            // [JN] Colored HUD: Frags
            if (sbar_colored && !vanillaparm)
            {
                if (temp < 0)
                dp_translation = cr[CR_RED];
                else if (temp == 0)
                dp_translation = cr[CR_BRIGHTYELLOW];
                else
                dp_translation = cr[CR_GREEN];
            }

            DrBNumber(temp, 173 + (wide_delta * 2), 176);
        }
        
        // [JN] Always draw keys in Deathmatch, but only if player alive,
        // and not while opened inventory. Just as visual reminder.
        if (CPlayer->mo->health > 0 && !inventory)
        {
            V_DrawShadowedPatch(219 + (wide_delta * 2), 174,
                                W_CacheLumpName(DEH_String("ykeyicon"), PU_CACHE));
            V_DrawShadowedPatch(219 + (wide_delta * 2), 182,
                                W_CacheLumpName(DEH_String("gkeyicon"), PU_CACHE));
            V_DrawShadowedPatch(219 + (wide_delta * 2), 190,
                                W_CacheLumpName(DEH_String("bkeyicon"), PU_CACHE));
        }
    }
    if (!inventory)
    {
        // [JN] Draw health vial, representing player's health
        if (CPlayer->mo->health == 100)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT00", PU_CACHE));
        else if (CPlayer->mo->health >= 90)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT01", PU_CACHE));
        else if (CPlayer->mo->health >= 80)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT02", PU_CACHE));
        else if (CPlayer->mo->health >= 70)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT03", PU_CACHE));
        else if (CPlayer->mo->health >= 60)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT04", PU_CACHE));
        else if (CPlayer->mo->health >= 50)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT05", PU_CACHE));
        else if (CPlayer->mo->health >= 40)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT06", PU_CACHE));
        else if (CPlayer->mo->health >= 30)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT07", PU_CACHE));
        else if (CPlayer->mo->health >= 20)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT08", PU_CACHE));
        else if (CPlayer->mo->health >= 10)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT09", PU_CACHE));
        else if (CPlayer->mo->health >= 1)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT10", PU_CACHE));
        else if (CPlayer->mo->health <= 0)
        V_DrawShadowedPatch(46, 177, W_CacheLumpName("HUDPNT11", PU_CACHE));
        
        // [JN] Draw keys
        if (!deathmatch)
        {
            if (CPlayer->keys[key_yellow])
            {
                V_DrawShadowedPatch(219 + (wide_delta * 2), 174,
                                    W_CacheLumpName(DEH_String("ykeyicon"), PU_CACHE));
            }
            else
            {
                if (CPlayer->yellowkeyTics && !vanillaparm)
                {
                    V_DrawFadePatch(219 + (wide_delta * 2), 174, 
                                    W_CacheLumpName(DEH_String("ykeyicon"), PU_CACHE), 
                                    CPlayer->yellowkeyTics > 13 ? transtable90 :
                                    CPlayer->yellowkeyTics > 11 ? transtable80 :
                                    CPlayer->yellowkeyTics >  9 ? transtable70 :
                                    CPlayer->yellowkeyTics >  7 ? transtable60 :
                                    CPlayer->yellowkeyTics >  5 ? transtable50 :
                                    CPlayer->yellowkeyTics >  4 ? transtable40 :
                                    CPlayer->yellowkeyTics >  3 ? transtable30 :
                                    CPlayer->yellowkeyTics >  2 ? transtable20 :
                                                                  transtable10);
                }
            }
            if (CPlayer->keys[key_green])
            {
                V_DrawShadowedPatch(219 + (wide_delta * 2), 182,
                                    W_CacheLumpName(DEH_String("gkeyicon"), PU_CACHE));
            }
            else
            {
                if (CPlayer->greenkeyTics && !vanillaparm)
                {
                    V_DrawFadePatch(219 + (wide_delta * 2), 182,
                                    W_CacheLumpName(DEH_String("gkeyicon"), PU_CACHE), 
                                    CPlayer->greenkeyTics > 13 ? transtable90 :
                                    CPlayer->greenkeyTics > 11 ? transtable80 :
                                    CPlayer->greenkeyTics >  9 ? transtable70 :
                                    CPlayer->greenkeyTics >  7 ? transtable60 :
                                    CPlayer->greenkeyTics >  5 ? transtable50 :
                                    CPlayer->greenkeyTics >  4 ? transtable40 :
                                    CPlayer->greenkeyTics >  3 ? transtable30 :
                                    CPlayer->greenkeyTics >  2 ? transtable20 :
                                                                 transtable10);
                }
            }
            if (CPlayer->keys[key_blue])
            {
                V_DrawShadowedPatch(219 + (wide_delta * 2), 190,
                                    W_CacheLumpName(DEH_String("bkeyicon"), PU_CACHE));
            }
            else
            {
                if (CPlayer->bluekeyTics && !vanillaparm)
                {
                    V_DrawFadePatch(219 + (wide_delta * 2), 190,
                                    W_CacheLumpName(DEH_String("bkeyicon"), PU_CACHE), 
                                    CPlayer->bluekeyTics > 13 ? transtable90 :
                                    CPlayer->bluekeyTics > 11 ? transtable80 :
                                    CPlayer->bluekeyTics >  9 ? transtable70 :
                                    CPlayer->bluekeyTics >  7 ? transtable60 :
                                    CPlayer->bluekeyTics >  5 ? transtable50 :
                                    CPlayer->bluekeyTics >  4 ? transtable40 :
                                    CPlayer->bluekeyTics >  3 ? transtable30 :
                                    CPlayer->bluekeyTics >  2 ? transtable20 :
                                                                transtable10);
                }
            }
        }

        if (CPlayer->readyArtifact > 0)
        {
            patch = DEH_String(patcharti[CPlayer->readyArtifact]);
            
            // [JN] Draw Artifacts
            V_DrawShadowedPatch(238 + (wide_delta * 2), 170,
                                W_CacheLumpName(patch, PU_CACHE));
            DrSmallNumber(CPlayer->inventory[inv_ptr].count, 255 + (wide_delta * 2), 191, false);
        }

        if (CPlayer->armorpoints > 0)
        {
            // [JN] Player have Silver Shield
            if (CPlayer->armortype == 1)
                V_DrawShadowedPatch(110, 214, W_CacheLumpName("SHLDA0", PU_CACHE));
            // [JN] Player have Enchanted Shield
            else if (CPlayer->armortype == 2)
                V_DrawShadowedPatch(108, 216, W_CacheLumpName("SHD2A0", PU_CACHE));

            // [JN] Colored HUD: Armor
            if (sbar_colored && !vanillaparm)
            {
                if (CPlayer->cheats & CF_GODMODE || CPlayer->powers[pw_invulnerability])
                dp_translation = cr[CR_FLAME];
                else if (CPlayer->armortype >= 2)
                dp_translation = cr[CR_BRIGHTYELLOW];
                else if (CPlayer->armortype == 1)
                dp_translation = cr[CR_WHITE];
                else
                dp_translation = cr[CR_RED];
            }

            // [JN] Draw ammount of armor
            DrBNumber(CPlayer->armorpoints, 57, 176);
        }
    }
    else
    {
        x = inv_ptr - curpos;
        for (i = 0; i < 7; i++)
        {
            V_DrawTLPatch(50 + i * 31 + wide_delta, 168,
                          W_CacheLumpName(DEH_String("ARTIBOX"), PU_CACHE));
            if (CPlayer->inventorySlotNum > x + i
                && CPlayer->inventory[x + i].type != arti_none)
            {
                patch = DEH_String(patcharti[CPlayer->inventory[x + i].type]);
                V_DrawPatch(50 + i * 31 + wide_delta, 168,
                            W_CacheLumpName(patch, PU_CACHE), NULL);
                DrSmallNumber(CPlayer->inventory[x + i].count, 65 + i * 31 
                                                                  + wide_delta, 190, false);
            }
        }
        V_DrawPatch(50 + curpos * 31 + wide_delta, 197, PatchSELECTBOX, NULL);

        if (x > 0)
        {
            V_DrawPatch(38 + wide_delta, 167, !(leveltime & 4) ? PatchINVLFGEM1 :
                        PatchINVLFGEM2, NULL);
        }
        if (CPlayer->inventorySlotNum - x > 7)
        {
            V_DrawPatch(269 + wide_delta, 167, !(leveltime & 4) ?
                        PatchINVRTGEM1 : PatchINVRTGEM2, NULL);
        }
    }
}

//--------------------------------------------------------------------------
//
// FUNC SB_Responder
//
//--------------------------------------------------------------------------

boolean SB_Responder(event_t * event)
{
    if (event->type == ev_keydown)
    {
        if (HandleCheats(event->data2))
        {                       // Need to eat the key
            return (true);
        }
    }
    return (false);
}

//--------------------------------------------------------------------------
//
// FUNC HandleCheats
//
// Returns true if the caller should eat the key.
//
//--------------------------------------------------------------------------

static boolean HandleCheats(byte key)
{
    int i;
    boolean eat;

    /* [crispy] check for nightmare/netgame per cheat, to allow "harmless" cheats
    if (netgame || gameskill == sk_nightmare)
    {                         // Can't cheat in a net-game, or in nightmare mode
        return (false);
    }
    */
    if (players[consoleplayer].health <= 0)
    {                         // Dead players can't cheat
        return (false);
    }
    eat = false;
    for (i = 0; Cheats[i].func != NULL; i++)
    {
        if (cht_CheckCheat(Cheats[i].seq, key))
        {
            Cheats[i].func(&players[consoleplayer], &Cheats[i]);
            S_StartSound(NULL, sfx_dorcls);
        }
    }
    return (eat);
}

#define NIGHTMARE_NETGAME_CHECK if(netgame||gameskill==sk_nightmare){return;}
#define NETGAME_CHECK if(netgame){return;}

//--------------------------------------------------------------------------
//
// CHEAT FUNCTIONS
//
//--------------------------------------------------------------------------

static void CheatGodFunc(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    player->cheats ^= CF_GODMODE;
    P_SetMessage(player, DEH_String(player->cheats & CF_GODMODE ?
                 txt_cheatgodon : txt_cheatgodoff), msg_system, false);
}

static void CheatNoClipFunc(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    player->cheats ^= CF_NOCLIP;
    if (player->cheats & CF_NOCLIP)
    {
        player->mo->flags |= MF_NOCLIP;
        P_SetMessage(player, DEH_String(txt_cheatnoclipon), msg_system, false);
    }
    else
    {
        player->mo->flags &= ~MF_NOCLIP;
        P_SetMessage(player, DEH_String(txt_cheatnoclipoff), msg_system, false);
    }
}

static void CheatWeaponsFunc(player_t * player, Cheat_t * cheat)
{
    int i;
    //extern boolean *WeaponInShareware;

    NIGHTMARE_NETGAME_CHECK;
    player->armorpoints = 200;
    player->armortype = 2;
    if (!player->backpack)
    {
        for (i = 0; i < NUMAMMO; i++)
        {
            player->maxammo[i] *= 2;
        }
        player->backpack = true;
    }
    for (i = 0; i < NUMWEAPONS - 1; i++)
    {
        player->weaponowned[i] = true;
    }
    if (gamemode == shareware)
    {
        player->weaponowned[wp_skullrod] = false;
        player->weaponowned[wp_phoenixrod] = false;
        player->weaponowned[wp_mace] = false;
    }
    for (i = 0; i < NUMAMMO; i++)
    {
        player->ammo[i] = player->maxammo[i];
    }
    P_SetMessage(player, DEH_String(txt_cheatweapons), msg_system, false);
}

static void CheatPowerFunc(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    if (player->powers[pw_weaponlevel2])
    {
        player->powers[pw_weaponlevel2] = 0;
        P_SetMessage(player, DEH_String(txt_cheatpoweroff), msg_system, false);
    }
    else
    {
        P_UseArtifact(player, arti_tomeofpower);
        P_SetMessage(player, DEH_String(txt_cheatpoweron), msg_system, false);
    }
}

static void CheatHealthFunc(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    if (player->chickenTics)
    {
        player->health = player->mo->health = MAXCHICKENHEALTH;
    }
    else
    {
        player->health = player->mo->health = MAXHEALTH;
    }
    P_SetMessage(player, DEH_String(txt_cheathealth), msg_system, false);
}

static void CheatKeysFunc(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    player->keys[key_yellow] = true;
    player->keys[key_green] = true;
    player->keys[key_blue] = true;
    playerkeys = 7;             // Key refresh flags
    P_SetMessage(player, DEH_String(txt_cheatkeys), msg_system, false);
}

static void CheatSoundFunc(player_t * player, Cheat_t * cheat)
{
    NETGAME_CHECK;
    DebugSound = !DebugSound;
    P_SetMessage(player, DEH_String(DebugSound ?
                 txt_cheatsoundon : txt_cheatsoundoff), msg_system, false);
}

static void CheatArtifact1Func(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    P_SetMessage(player, DEH_String(txt_cheatartifacts1), msg_system, false);
}

static void CheatArtifact2Func(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    P_SetMessage(player, DEH_String(txt_cheatartifacts2), msg_system, false);
}

static void CheatArtifact3Func(player_t * player, Cheat_t * cheat)
{
    char args[2];
    int i;
    int j;
    int type;
    int count;

    NIGHTMARE_NETGAME_CHECK;
    cht_GetParam(cheat->seq, args);
    type = args[0] - 'a' + 1;
    count = args[1] - '0';
    if (type == 26 && count == 0)
    {                           // All artifacts
        for (i = arti_none + 1; i < NUMARTIFACTS; i++)
        {
            if (gamemode == shareware 
             && (i == arti_superhealth || i == arti_teleport))
            {
                continue;
            }
            for (j = 0; j < 16; j++)
            {
                P_GiveArtifact(player, i, NULL);
            }
        }
        P_SetMessage(player, DEH_String(txt_cheatartifacts3), msg_system, false);
    }
    else if (type > arti_none && type < NUMARTIFACTS
             && count > 0 && count < 10)
    {
        if (gamemode == shareware
         && (type == arti_superhealth || type == arti_teleport))
        {
            P_SetMessage(player, DEH_String(txt_cheatartifactsfail), msg_system, false);
            return;
        }
        for (i = 0; i < count; i++)
        {
            P_GiveArtifact(player, type, NULL);
        }
        P_SetMessage(player, DEH_String(txt_cheatartifacts3), msg_system, false);
    }
    else
    {                           // Bad input
        P_SetMessage(player, DEH_String(txt_cheatartifactsfail), msg_system, false);
    }
}

static void CheatWarpFunc(player_t * player, Cheat_t * cheat)
{
    char args[2];
    int episode;
    int map;

    NETGAME_CHECK;
    cht_GetParam(cheat->seq, args);

    episode = args[0] - '0';
    map = args[1] - '0';
    if (D_ValidEpisodeMap(heretic, gamemode, episode, map))
    {
        G_DeferedInitNew(gameskill, episode, map);
    }
}

static void CheatChickenFunc(player_t * player, Cheat_t * cheat)
{
    extern boolean P_UndoPlayerChicken(player_t * player);

    NIGHTMARE_NETGAME_CHECK;
    if (player->chickenTics)
    {
        if (P_UndoPlayerChicken(player))
        {
            P_SetMessage(player, DEH_String(txt_cheatchickenoff), msg_system, false);
        }
    }
    else if (P_ChickenMorphPlayer(player))
    {
        P_SetMessage(player, DEH_String(txt_cheatchickenon), msg_system, false);
    }
}

static void CheatMassacreFunc(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    P_Massacre();
    P_SetMessage(player, DEH_String(txt_cheatmassacre), msg_system, false);
}

static void CheatIDKFAFunc(player_t * player, Cheat_t * cheat)
{
    int i;

    NIGHTMARE_NETGAME_CHECK;
    if (player->chickenTics)
    {
        return;
    }
    for (i = 1; i < 8; i++)
    {
        player->weaponowned[i] = false;
    }
    player->pendingweapon = wp_staff;
    P_SetMessage(player, DEH_String(txt_cheatidkfa), msg_system, true);
}

static void CheatIDDQDFunc(player_t * player, Cheat_t * cheat)
{
    NIGHTMARE_NETGAME_CHECK;
    P_DamageMobj(player->mo, NULL, player->mo, 10000);
    P_SetMessage(player, DEH_String(txt_cheatiddqd), msg_system, true);
}

static void CheatVERSIONFunc(player_t * player, Cheat_t * cheat)
{
    static char msg[38];

    char* version = english_language
                  ? RD_Project_Version
                  : M_StringReplace(RD_Project_Version, ".", ">");
    M_snprintf(msg, sizeof(msg), "%s %s",
               english_language ? "Version" : "dthcbz", version);

    P_SetMessage(player, msg, msg_system, true);
}

/*
================================================================================
=
= CheatRAVMUSFunc
=
= [JN] Allow music changing. Working as both RAVMUSxx and IDMUSxx.
=
================================================================================
*/

static char msg[32];

static void CheatRAVMUSFunc (player_t *player, Cheat_t *cheat)
{
    char args[2];
    int  musnum;

    cht_GetParam(cheat->seq, args);
    musnum = mus_e1m1 + (args[0]-'1')*9 + (args[1]-'1');

    // [JN] Prevent crash with RAVMUS0x or RAVMUSx0 and don't
    // allow to play unexisting music depending on game version:
    // Retail:     E1M1 ... E6M3
    // Registered: E1M1 ... E3M9
    // Shareware:  E1M1 ... E1M9
    if ((((args[0]-'1')*9 + args[1]-'1') >
    (gamemode == retail ? 47 : gamemode == registered ? 26 : 8)    
    || args[0] < '1' || args[1] < '1'))
    {
        P_SetMessage(player, DEH_String(txt_cheatnomus), msg_system, true);
    }
    else
    {
        S_StartSong(musnum, true, false);
        
        // [JN] jff 3/17/98 remember idmus number for restore
        idmusnum = musnum;

        // [JN] Consolidated string: 
        // Music changed to "ExMx" | Музыка изменена на `ExMx`
        M_snprintf(msg, sizeof(msg), "%s \"%s%c%s%c\"",
                   txt_cheatmus, txt_cheatmus_e, args[0], txt_cheatmus_m, args[1]);
        P_SetMessage(player, msg, msg_system, true);
    }
}

/*
================================================================================
=
= [JN] Crosshair routines. Defining, drawing, coloring.
=
================================================================================
*/

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

        dp_translation = CrosshairShowcaseTimeout >= 70 ? cr[CR_RED]  :
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
                                                          cr[CR_AZURE];
    }
    else if (crosshair_type == 3)
    {
        dp_translation = CrosshairShowcaseTimeout >= 105 ? cr[CR_RED]   :
                         CrosshairShowcaseTimeout >=  70 ? cr[CR_YELLOW]  :
                         CrosshairShowcaseTimeout >=  35 ? cr[CR_GREEN] :
                                                           cr[CR_AZURE];
    }
    else
    {
        dp_translation = cr[CR_RED];
    }
}

// -----------------------------------------------------------------------------
// Crosshair_Colorize_inGame: ingame coloring routine, actual colors/values.
// -----------------------------------------------------------------------------

static void Crosshair_Colorize_inGame (void)
{
    if (crosshair_type == 1)
    {
        dp_translation = CPlayer->health >= 67 ? cr[CR_GREEN] :
                         CPlayer->health >= 34 ? cr[CR_YELLOW]  :
                                                 cr[CR_RED];
    }
    else if (crosshair_type == 2)
    {
        dp_translation = cr[CR_RED];

        P_AimLineAttack(CPlayer->mo, CPlayer->mo->angle, MISSILERANGE);

        if (linetarget)
        {
            dp_translation = cr[CR_AZURE];
        }
    }
    else if (crosshair_type == 3)
    {
        dp_translation = CPlayer->health >= 67 ? cr[CR_GREEN] :
                         CPlayer->health >= 34 ? cr[CR_YELLOW]  :
                                                 cr[CR_RED];

        P_AimLineAttack(CPlayer->mo, CPlayer->mo->angle, MISSILERANGE);

        if (linetarget)
        {
            dp_translation = cr[CR_AZURE];
        }
    }
    else
    {
        dp_translation = cr[CR_RED];
    }
}

// -----------------------------------------------------------------------------
// Crosshair_Draw_Scaled: drawing func for scaled (big) crosshair.
// -----------------------------------------------------------------------------

static void Crosshair_Draw_Scaled (void)
{
    V_DrawPatch(origwidth/2, screenblocks <= 10 ? 81 : 102,
                CrosshairPatch, CrosshairOpacity);
}

// -----------------------------------------------------------------------------
// Crosshair_Draw_Unscaled: drawing func for unscaled (small) crosshair.
// -----------------------------------------------------------------------------

static void Crosshair_Draw_Unscaled (void)
{
    V_DrawPatchUnscaled(screenwidth/2, screenblocks <= 10 ? 162 : 204,
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

/*
================================================================================
=
= [JN] Ammo widget. Drawing and coloring.
=
================================================================================
*/

static void DrSmallAmmoNumber (int val, int x, int y, boolean opaque)
{
    int oldval = val;
    patch_t *patch;

    if (val > 999)
    {
        val = 999;  // [JN] Overflow guard.
    }
    if (val > 99)
    {
        patch = PatchINumbers[val / 100];
        V_DrawPatchUnscaled(x << hires, y << hires, patch, opaque ? NULL : transtable60);
    }
    val = val % 100;
    if (val > 9 || oldval > 99)
    {
        patch = PatchINumbers[val / 10];
        V_DrawPatchUnscaled((x + 5) << hires, y << hires, patch, opaque ? NULL : transtable60);
    }
    val = val % 10;
    patch = PatchINumbers[val];
    V_DrawPatchUnscaled((x + 10) << hires, y << hires, patch, opaque ? NULL : transtable60);
}

static void SB_Draw_Ammo_Widget (void)
{
    const int wide_4_3   = aspect_ratio >= 2 && screenblocks == 9 ? wide_delta * 2 : 0;
    const int xpos_pic   = (((ammo_widget == 1 ? 288 : 267) + wide_delta * 2) << hires) - wide_4_3;
    const int xpos_slash = (((ammo_widget == 1 ? 288 : 296) + wide_delta * 2) << hires) - wide_4_3;
    const int xpos_qty1  = ((ammo_widget == 1 ? 302 : 281) + wide_delta * 2) - (wide_4_3 / 2);
    const int xpos_qty2  = (302 + wide_delta * 2) - (wide_4_3 / 2);
    const int ammo1 = CPlayer->ammo[am_goldwand], fullammo1 = CPlayer->maxammo[am_goldwand];
    const int ammo2 = CPlayer->ammo[am_crossbow], fullammo2 = CPlayer->maxammo[am_crossbow];
    const int ammo3 = CPlayer->ammo[am_blaster], fullammo3 = CPlayer->maxammo[am_blaster];
    const int ammo4 = CPlayer->ammo[am_skullrod], fullammo4 = CPlayer->maxammo[am_skullrod];
    const int ammo5 = CPlayer->ammo[am_phoenixrod], fullammo5 = CPlayer->maxammo[am_phoenixrod];
    const int ammo6 = CPlayer->ammo[am_mace], fullammo6 = CPlayer->maxammo[am_mace];

    // Ammo GFX patches
    //if (!(CPlayer->weaponowned[wp_goldwand])) dp_translation = cr[CR_MONOCHROME];
    V_DrawPatchUnscaled(xpos_pic, 99 << hires, W_CacheLumpName(DEH_String("INAMGLD"), PU_CACHE),
                       (CPlayer->readyweapon == wp_goldwand || (automapactive && !automap_overlay)) ? NULL : transtable60);
    dp_translation = NULL;

    if (!(CPlayer->weaponowned[wp_crossbow])) dp_translation = cr[CR_MONOCHROME];
    V_DrawPatchUnscaled(xpos_pic, 106 << hires, W_CacheLumpName(DEH_String("INAMBOW"), PU_CACHE),
                       (CPlayer->readyweapon == wp_crossbow || (automapactive && !automap_overlay)) ? NULL : transtable60);
    dp_translation = NULL;

    if (!(CPlayer->weaponowned[wp_blaster])) dp_translation = cr[CR_MONOCHROME];
    V_DrawPatchUnscaled(xpos_pic, 113 << hires, W_CacheLumpName(DEH_String("INAMBST"), PU_CACHE),
                       (CPlayer->readyweapon == wp_blaster || (automapactive && !automap_overlay)) ? NULL : transtable60);
    dp_translation = NULL;

    // Following weapons not available in Shareware
    if (gamemode != shareware)
    {
        if (!(CPlayer->weaponowned[wp_skullrod])) dp_translation = cr[CR_MONOCHROME];
        V_DrawPatchUnscaled(xpos_pic, 120 << hires, W_CacheLumpName(DEH_String("INAMRAM"), PU_CACHE),
                           (CPlayer->readyweapon == wp_skullrod || (automapactive && !automap_overlay)) ? NULL : transtable60);
        dp_translation = NULL;

        if (!(CPlayer->weaponowned[wp_phoenixrod])) dp_translation = cr[CR_MONOCHROME];
        V_DrawPatchUnscaled(xpos_pic, 127 << hires, W_CacheLumpName(DEH_String("INAMPNX"), PU_CACHE),
                           (CPlayer->readyweapon == wp_phoenixrod || (automapactive && !automap_overlay)) ? NULL : transtable60);
        dp_translation = NULL;

        if (!(CPlayer->weaponowned[wp_mace])) dp_translation = cr[CR_MONOCHROME];
        V_DrawPatchUnscaled(xpos_pic, 134 << hires, W_CacheLumpName(DEH_String("INAMLOB"), PU_CACHE),
                           (CPlayer->readyweapon == wp_mace || (automapactive && !automap_overlay)) ? NULL : transtable60);
        dp_translation = NULL;
    }

    // Wand ammo
    if (ammo_widget_colored)
    dp_translation = ammo1 < fullammo1 / 4 ? cr[CR_RED] :
                     ammo1 < fullammo1 / 2 ? cr[CR_BRIGHTYELLOW] : cr[CR_GREEN];
    DrSmallAmmoNumber(ammo1, xpos_qty1, 100,
                     (CPlayer->readyweapon == wp_goldwand || (automapactive && !automap_overlay)) ? true : false);
    if (ammo_widget == 2)
    {
        V_DrawPatchUnscaled(xpos_slash, 100 << hires, W_CacheLumpName(DEH_String("SLASHNUM"), PU_CACHE),
                           (CPlayer->readyweapon == wp_goldwand || (automapactive && !automap_overlay)) ? NULL : transtable60);
        DrSmallAmmoNumber(fullammo1, xpos_qty2, 100,
                         (CPlayer->readyweapon == wp_goldwand || (automapactive && !automap_overlay)) ? true : false);
    }
    dp_translation = NULL;

    // Crossbow ammo
    if (ammo_widget_colored)
    dp_translation = !(CPlayer->weaponowned[wp_crossbow]) ? cr[CR_WHITE] :
                     ammo2 < fullammo2 / 4 ? cr[CR_RED] :
                     ammo2 < fullammo2 / 2 ? cr[CR_BRIGHTYELLOW] : cr[CR_GREEN];
    DrSmallAmmoNumber(ammo2, xpos_qty1, 107,
                     (CPlayer->readyweapon == wp_crossbow || (automapactive && !automap_overlay)) ? true : false);
    if (ammo_widget == 2)
    {
        V_DrawPatchUnscaled(xpos_slash, 107 << hires, W_CacheLumpName(DEH_String("SLASHNUM"), PU_CACHE),
                           (CPlayer->readyweapon == wp_crossbow || (automapactive && !automap_overlay)) ? NULL : transtable60);
        DrSmallAmmoNumber(fullammo2, xpos_qty2, 107,
                         (CPlayer->readyweapon == wp_crossbow || (automapactive && !automap_overlay)) ? true : false);
    }
    dp_translation = NULL;

    // Dragon Claw ammo
    if (ammo_widget_colored)
    dp_translation = !(CPlayer->weaponowned[wp_blaster]) ? cr[CR_WHITE] :
                     ammo3 < fullammo3 / 4 ? cr[CR_RED] :
                     ammo3 < fullammo3 / 2 ? cr[CR_BRIGHTYELLOW] : cr[CR_GREEN];
    DrSmallAmmoNumber(ammo3, xpos_qty1, 114,
                     (CPlayer->readyweapon == wp_blaster || (automapactive && !automap_overlay)) ? true : false);
    if (ammo_widget == 2)
    {
        V_DrawPatchUnscaled(xpos_slash, 114 << hires, W_CacheLumpName(DEH_String("SLASHNUM"), PU_CACHE),
                           (CPlayer->readyweapon == wp_blaster || (automapactive && !automap_overlay)) ? NULL : transtable60);
        DrSmallAmmoNumber(fullammo3, xpos_qty2, 114,
                         (CPlayer->readyweapon == wp_blaster || (automapactive && !automap_overlay)) ? true : false);
    }
    dp_translation = NULL;

    // Following weapons not available in Shareware
    if (gamemode != shareware)
    {
        // Hellstaff ammo
        if (ammo_widget_colored)
        dp_translation = !(CPlayer->weaponowned[wp_skullrod]) ? cr[CR_WHITE] :
                         ammo4 < fullammo4 / 4 ? cr[CR_RED] :
                         ammo4 < fullammo4 / 2 ? cr[CR_BRIGHTYELLOW] : cr[CR_GREEN];
        DrSmallAmmoNumber(ammo4, xpos_qty1, 121,
                         (CPlayer->readyweapon == wp_skullrod || (automapactive && !automap_overlay)) ? true : false);
        if (ammo_widget == 2)
        {
            V_DrawPatchUnscaled(xpos_slash, 121 << hires, W_CacheLumpName(DEH_String("SLASHNUM"), PU_CACHE),
                               (CPlayer->readyweapon == wp_skullrod || (automapactive && !automap_overlay)) ? NULL : transtable60);
            DrSmallAmmoNumber(fullammo4, xpos_qty2, 121,
                             (CPlayer->readyweapon == wp_skullrod || (automapactive && !automap_overlay)) ? true : false);
        }
        dp_translation = NULL;

        // Phoenix Rod ammo
        if (ammo_widget_colored)
        dp_translation = !(CPlayer->weaponowned[wp_phoenixrod]) ? cr[CR_WHITE] :
                         ammo5 < fullammo5 / 4 ? cr[CR_RED] :
                         ammo5 < fullammo5 / 2 ? cr[CR_BRIGHTYELLOW] : cr[CR_GREEN];
        DrSmallAmmoNumber(ammo5, xpos_qty1, 128,
                         (CPlayer->readyweapon == wp_phoenixrod || (automapactive && !automap_overlay)) ? true : false);
        if (ammo_widget == 2)
        {
            V_DrawPatchUnscaled(xpos_slash, 128 << hires, W_CacheLumpName(DEH_String("SLASHNUM"), PU_CACHE),
                               (CPlayer->readyweapon == wp_phoenixrod || (automapactive && !automap_overlay)) ? NULL : transtable60);
            DrSmallAmmoNumber(fullammo5, xpos_qty2, 128,
                              (CPlayer->readyweapon == wp_phoenixrod || (automapactive && !automap_overlay)) ? true : false);
        }
        dp_translation = NULL;

        // Firemace ammo
        if (ammo_widget_colored)
        dp_translation = !(CPlayer->weaponowned[wp_mace]) ? cr[CR_WHITE] :
                         ammo6 < fullammo6 / 4 ? cr[CR_RED] :
                         ammo6 < fullammo6 / 2 ? cr[CR_BRIGHTYELLOW] : cr[CR_GREEN];
        DrSmallAmmoNumber(ammo6, xpos_qty1, 135,
                         (CPlayer->readyweapon == wp_mace || (automapactive && !automap_overlay)) ? true : false);
        if (ammo_widget == 2)
        {
            V_DrawPatchUnscaled(xpos_slash, 135 << hires, W_CacheLumpName(DEH_String("SLASHNUM"), PU_CACHE),
                               (CPlayer->readyweapon == wp_mace || (automapactive && !automap_overlay)) ? NULL : transtable60);
            DrSmallAmmoNumber(fullammo6, xpos_qty2, 135,
                             (CPlayer->readyweapon == wp_mace || (automapactive && !automap_overlay)) ? true : false);
        }
        dp_translation = NULL;
    }
}

/*
================================================================================
=
= [crispy] Demo Timer widget
=
================================================================================
*/

void SB_DrawDemoTimer (const int time)
{
    const boolean wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9);
    const int hours = time / (3600 * TICRATE);
    const int mins = time / (60 * TICRATE) % 60;
    const float secs = (float)(time % (60 * TICRATE)) / TICRATE;
    char n[16];
    int x = 273;

    if (hours)
    {
        M_snprintf(n, sizeof(n), "%02i:%02i:%05.02f", hours, mins, secs);
    }
    else
    {
        M_snprintf(n, sizeof(n), "%02i:%05.02f", mins, secs);
        x += 12;
    }

    RD_M_DrawTextC(n, x + (wide_4_3 ? wide_delta : wide_delta*2), 11);
}

/*
================================================================================
=
= [crispy] print a bar indicating demo progress at the bottom of the screen
=
================================================================================
*/

void SB_DemoProgressBar (void)
{
    int i, x;

    // [JN] Clamp bar in emulated 4:3 display of wide screen mode.
    if (aspect_ratio >= 2 && screenblocks == 9)
    {
        x = wide_delta * 2;
        i = SCREENWIDTH * defdemotics / deftotaldemotics;
    }
    else
    {
        x = 0;
        i = screenwidth * defdemotics / deftotaldemotics;
    }

    V_DrawHorizLine(x, SCREENHEIGHT - 2, i, 0);   // [crispy] black
    V_DrawHorizLine(x, SCREENHEIGHT - 1, i, 255); // [crispy] white
}
