//
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


#include "doomstat.h"
#include "jn.h"
#include "rd_lang.h"


// -----------------------------------------------------------------------------
// [JN] Chars* to be defined and used as a predefined text strings.
// -----------------------------------------------------------------------------

//
// P_inter.C
//

// armor
char* gotarmor;
char* gotmega;

// bonus items
char* goththbonus;
char* gotarmbonus;
char* gotsceptre;
char* gotbible;
char* gotsuper;
char* gotmsphere;

// cards
char* gotbluecard;
char* gotyelwcard;
char* gotredcard;
char* gotblueskul;
char* gotyelwskul;
char* gotredskull;

// medikits, heals
char* gotstim;
char* gotmedineed;
char* gotmedikit;

// power ups
char* gotinvul;
char* gotberserk;
char* gotinvis;
char* gotsuit;
char* gotmap;
char* gotvisor;

// ammo
char* gotclip;
char* gotclipbox;
char* gotrocket;
char* gotrocket2;
char* gotrockbox;
char* gotcell;
char* gotcellbox;
char* gotshells;
char* gotshells8;
char* gotshellbox;
char* gotbackpack;

// weapons
char* gotbfg9000;
char* gotchaingun;
char* gotchainsaw;
char* gotlauncher;
char* gotplasma;
char* gotshotgun;
char* gotshotgun2;


// -----------------------------------------------------------------------------
// [JN] Function for (pre-)defining strings for text translations.
// -----------------------------------------------------------------------------

void RD_DefineLanguageStrings (void)
{
    if (english_language)
    {
        //
        // P_inter.C
        //

        // armor
        gotarmor    = GOTARMOR;
        gotmega     = GOTMEGA;

        // bonus items
        goththbonus = GOTHTHBONUS;
        gotarmbonus = GOTARMBONUS;
        gotsceptre  = GOTSCEPTRE;
        gotbible    = GOTBIBLE;
        gotsuper    = GOTSUPER;
        gotmsphere  = GOTMSPHERE;

        // cards
        gotbluecard = GOTBLUECARD;
        gotyelwcard = GOTYELWCARD;
        gotredcard  = GOTREDCARD;
        gotblueskul = GOTREDCARD;
        gotyelwskul = GOTYELWSKUL;
        gotredskull = GOTREDSKULL;

        // medikits, heals
        gotstim     = GOTSTIM;
        gotmedineed = GOTMEDINEED;
        gotmedikit  = GOTMEDIKIT;

        // power ups
        gotinvul    = GOTINVUL;
        gotberserk  = GOTBERSERK;
        gotinvis    = GOTINVIS;
        gotsuit     = GOTSUIT;
        gotmap      = GOTMAP;
        gotvisor    = GOTVISOR;

        // ammo
        gotclip     = GOTCLIP;
        gotclipbox  = GOTCLIPBOX;
        gotrocket   = GOTROCKET;
        gotrocket2  = GOTROCKET2;
        gotrockbox  = GOTROCKBOX;
        gotcell     = GOTCELL;
        gotcellbox  = GOTCELLBOX;
        gotshells   = GOTSHELLS;
        gotshells8  = GOTSHELLS8;
        gotshellbox = GOTSHELLBOX;
        gotbackpack = GOTBACKPACK;

        // weapons
        gotbfg9000  = GOTBFG9000;
        gotchaingun = GOTCHAINGUN;
        gotchainsaw = GOTCHAINSAW;
        gotlauncher = GOTLAUNCHER;
        gotplasma   = GOTPLASMA;
        gotshotgun  = GOTSHOTGUN;
        gotshotgun2 = GOTSHOTGUN2;
    }
    else
    {
        //
        // P_inter.C
        //

        // armor
        gotarmor    = GOTARMOR_RUS;
        gotmega     = GOTMEGA_RUS;

        // bonus items
        goththbonus = GOTHTHBONUS_RUS;
        gotarmbonus = GOTARMBONUS_RUS;
        gotsceptre  = GOTSCEPTRE_RUS;
        gotbible    = GOTBIBLE_RUS;
        gotsuper    = GOTSUPER_RUS;
        gotmsphere  = GOTMSPHERE_RUS;

        // cards
        gotbluecard = GOTBLUECARD_RUS;
        gotyelwcard = GOTYELWCARD_RUS;
        gotredcard  = GOTREDCARD_RUS;
        gotblueskul = GOTREDCARD_RUS;
        gotyelwskul = GOTYELWSKUL_RUS;
        gotredskull = GOTREDSKULL_RUS;

        // medikits, heals
        gotstim     = GOTSTIM_RUS;
        gotmedineed = GOTMEDINEED_RUS;
        gotmedikit  = GOTMEDIKIT_RUS;

        // power ups
        gotinvul    = GOTINVUL_RUS;
        gotberserk  = GOTBERSERK_RUS;
        gotinvis    = GOTINVIS_RUS;
        gotsuit     = GOTSUIT_RUS;
        gotmap      = GOTMAP_RUS;
        gotvisor    = GOTVISOR_RUS;

        // ammo
        gotclip     = GOTCLIP_RUS;
        gotclipbox  = GOTCLIPBOX_RUS;
        gotrocket   = GOTROCKET_RUS;
        gotrocket2  = GOTROCKET2_RUS;
        gotrockbox  = GOTROCKBOX_RUS;
        gotcell     = GOTCELL_RUS;
        gotcellbox  = GOTCELLBOX_RUS;
        gotshells   = GOTSHELLS_RUS;
        gotshells8  = GOTSHELLS8_RUS;
        gotshellbox = GOTSHELLBOX_RUS;
        gotbackpack = GOTBACKPACK_RUS;

        // weapons
        gotbfg9000  = GOTBFG9000_RUS;
        gotchaingun = GOTCHAINGUN_RUS;
        gotchainsaw = GOTCHAINSAW_RUS;
        gotlauncher = GOTLAUNCHER_RUS;
        gotplasma   = GOTPLASMA_RUS;
        gotshotgun  = GOTSHOTGUN_RUS;
        gotshotgun2 = GOTSHOTGUN2_RUS;
    }
}