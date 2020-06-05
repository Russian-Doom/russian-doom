//
// Copyright(C) 2016-2020 Julian Nechaevsky
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
// AM_map.C
//

char* amstr_followon;
char* amstr_followoff;
char* amstr_gridon;
char* amstr_gridoff;
char* amstr_markedspot;
char* amstr_markscleared;
char* amstr_overlayon;
char* amstr_overlayoff;
char* amstr_rotateon;
char* amstr_rotateoff;


//
// G_game.C
//

char* ggsaved;
char* ggloaded;


//
// M_Menu.C
//

char* msgoff;
char* msgon;
char* detailhi;
char* detaillo;
char* ststr_alwrun_on;
char* ststr_alwrun_off;
char* ststr_crosshair_on;
char* ststr_crosshair_off;
char* ststr_mlook_on;
char* ststr_mlook_off;
char* ststr_testctrls;
char* ststr_scrnsht;


//
// P_Doors.C
//

char* pd_blueo;
char* pd_redo;
char* pd_yellowo;
char* pd_bluek;
char* pd_redk;
char* pd_yellowk;


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


//
// P_Spec.C
//

char* secretfound;


//
// ST_stuff.C
//

char* ststr_mus;
char* ststr_nomus;
char* ststr_dqdon;
char* ststr_dqdoff;

char* ststr_kfaadded;
char* ststr_faadded;
char* ststr_kaadded;

char* ststr_ncon;
char* ststr_ncoff;

char* ststr_choppers;
char* ststr_clev;

char* ststr_behold;
char* ststr_beholdx;
char* ststr_beholdz;

// RD specific
char* ststr_massacre;
char* ststr_version;


// -----------------------------------------------------------------------------
// [JN] Function for (pre-)defining strings for text translations.
// -----------------------------------------------------------------------------

void RD_DefineLanguageStrings (void)
{
    if (english_language)
    {
        //
        // AM_map.C
        //

        amstr_followon      = AMSTR_FOLLOWON;
        amstr_followoff     = AMSTR_FOLLOWOFF;
        amstr_gridon        = AMSTR_GRIDON;
        amstr_gridoff       = AMSTR_GRIDOFF;
        amstr_markedspot    = AMSTR_MARKEDSPOT;
        amstr_markscleared  = AMSTR_MARKSCLEARED;
        amstr_overlayon     = AMSTR_OVERLAYON;
        amstr_overlayoff    = AMSTR_OVERLAYOFF;
        amstr_rotateon      = AMSTR_ROTATEON;
        amstr_rotateoff     = AMSTR_ROTATEOFF;

        //
        // G_game.C
        //

        ggsaved         = GGSAVED;
        ggloaded        = GGLOADED;

        //
        // M_Menu.C
        //

        msgoff          = MSGOFF;
        msgon           = MSGON;
        detailhi        = DETAILHI;
        detaillo        = DETAILLO;
        ststr_alwrun_on = STSTR_ALWRUNON;
        ststr_alwrun_off= STSTR_ALWRUNOFF;
        ststr_crosshair_on = STSTR_CROSSHAIR_ON;
        ststr_crosshair_off = STSTR_CROSSHAIR_OFF;
        ststr_mlook_on  = STSTR_MLOOK_ON;
        ststr_mlook_off = STSTR_MLOOK_OFF;
        ststr_testctrls = STSTR_TESTCTRLS;
        ststr_scrnsht   = STSTR_SCRNSHT;

        //
        // P_Doors.C
        //

        pd_blueo    = PD_BLUEO;
        pd_redo     = PD_REDO;
        pd_yellowo  = PD_YELLOWO;
        pd_bluek    = PD_BLUEK;
        pd_redk     = PD_REDK;
        pd_yellowk  = PD_YELLOWK;

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

        //
        // P_Spec.C
        //

        secretfound     = SECRETFOUND;

        //
        // ST_stuff.C
        //

        ststr_mus       = STSTR_MUS;
        ststr_nomus     = STSTR_NOMUS;
        ststr_dqdon     = STSTR_DQDON;
        ststr_dqdoff    = STSTR_DQDOFF;

        ststr_kfaadded  = STSTR_KFAADDED;
        ststr_faadded   = STSTR_FAADDED;
        ststr_kaadded   = STSTR_KAADDED;

        ststr_ncon      = STSTR_NCON;
        ststr_ncoff     = STSTR_NCOFF;

        ststr_choppers  = STSTR_CHOPPERS;
        ststr_clev      = STSTR_CLEV;

        ststr_behold    = STSTR_BEHOLD;
        ststr_beholdx   = STSTR_BEHOLDX;
        ststr_beholdz   = STSTR_BEHOLDZ;

        // RD specific
        ststr_massacre  = STSTR_MASSACRE;
        ststr_version   = STSTR_VERSION;
    }
    else
    {
        //
        // AM_map.C
        //

        amstr_followon      = AMSTR_FOLLOWON_RUS;
        amstr_followoff     = AMSTR_FOLLOWOFF_RUS;
        amstr_gridon        = AMSTR_GRIDON_RUS;
        amstr_gridoff       = AMSTR_GRIDOFF_RUS;
        amstr_markedspot    = AMSTR_MARKEDSPOT_RUS;
        amstr_markscleared  = AMSTR_MARKSCLEARED_RUS;
        amstr_overlayon     = AMSTR_OVERLAYON_RUS;
        amstr_overlayoff    = AMSTR_OVERLAYOFF_RUS;
        amstr_rotateon      = AMSTR_ROTATEON_RUS;
        amstr_rotateoff     = AMSTR_ROTATEOFF_RUS;

        //
        // G_game.C
        //

        ggsaved         = GGSAVED_RUS;
        ggloaded        = GGLOADED_RUS;

        //
        // M_Menu.C
        //

        msgoff          = MSGOFF_RUS;
        msgon           = MSGON_RUS;
        detailhi        = DETAILHI_RUS;
        detaillo        = DETAILLO_RUS;
        ststr_alwrun_on = STSTR_ALWRUNON_RUS;
        ststr_alwrun_off= STSTR_ALWRUNOFF_RUS;
        ststr_crosshair_on = STSTR_CROSSHAIR_ON_RUS;
        ststr_crosshair_off = STSTR_CROSSHAIR_OFF_RUS;
        ststr_mlook_on  = STSTR_MLOOK_ON_RUS;
        ststr_mlook_off = STSTR_MLOOK_OFF_RUS;
        ststr_testctrls = STSTR_TESTCTRLS_RUS;
        ststr_scrnsht   = STSTR_SCRNSHT_RUS;

        //
        // P_Doors.C
        //

        pd_blueo    = PD_BLUEO_RUS;
        pd_redo     = PD_REDO_RUS;
        pd_yellowo  = PD_YELLOWO_RUS;
        pd_bluek    = PD_BLUEK_RUS;
        pd_redk     = PD_REDK_RUS;
        pd_yellowk  = PD_YELLOWK_RUS;

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

        //
        // P_Spec.C
        //

        secretfound     = SECRETFOUND_RUS;

        //
        // ST_stuff.C
        //

        ststr_mus       = STSTR_MUS_RUS;
        ststr_nomus     = STSTR_NOMUS_RUS;
        ststr_dqdon     = STSTR_DQDON_RUS;
        ststr_dqdoff    = STSTR_DQDOFF_RUS;

        ststr_kfaadded  = STSTR_KFAADDED_RUS;
        ststr_faadded   = STSTR_FAADDED_RUS;
        ststr_kaadded   = STSTR_KAADDED_RUS;

        ststr_ncon      = STSTR_NCON_RUS;
        ststr_ncoff     = STSTR_NCOFF_RUS;

        ststr_choppers  = STSTR_CHOPPERS_RUS;
        ststr_clev      = STSTR_CLEV_RUS;

        ststr_behold    = STSTR_BEHOLD_RUS;
        ststr_beholdx   = STSTR_BEHOLDX_RUS;
        ststr_beholdz   = STSTR_BEHOLDZ_RUS;

        // RD specific
        ststr_massacre  = STSTR_MASSACRE_RUS;
        ststr_version   = STSTR_VERSION_RUS;
    }
}
