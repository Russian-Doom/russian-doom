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


#include "doomtype.h"
#include "jn.h"
#include "rd_lang.h"


// -----------------------------------------------------------------------------
// [JN] Chars* to be defined and used as a predefined text strings.
// -----------------------------------------------------------------------------

// Keys
char* txt_gotbluekey;
char* txt_gotyellowkey;
char* txt_gotgreenkey;

// Artifacts
char* txt_artihealth;
char* txt_artifly;
char* txt_artiinvulnerability;
char* txt_artitomeofpower;
char* txt_artiinvisibility;
char* txt_artiegg;
char* txt_artisuperhealth;
char* txt_artitorch;
char* txt_artifirebomb;
char* txt_artiteleport;

// Items
char* txt_itemhealth;
char* txt_itembagofholding;
char* txt_itemshield1;
char* txt_itemshield2;
char* txt_itemsupermap;

// Ammo
char* txt_ammogoldwand1;
char* txt_ammogoldwand2;
char* txt_ammomace1;
char* txt_ammomace2;
char* txt_ammocrossbow1;
char* txt_ammocrossbow2;
char* txt_ammoblaster1;
char* txt_ammoblaster2;
char* txt_ammoskullrod1;
char* txt_ammoskullrod2;
char* txt_ammophoenixrod1;
char* txt_ammophoenixrod2;

// Weapons
char* txt_wpnmace;
char* txt_wpncrossbow;
char* txt_wpnblaster;
char* txt_wpnskullrod;
char* txt_wpnphoenixrod;
char* txt_wpngauntlets;

// SB_bar.c
char* txt_cheatgodon;
char* txt_cheatgodoff;
char* txt_cheatnoclipon;
char* txt_cheatnoclipoff;
char* txt_cheatweapons;
char* txt_cheatflighton;
char* txt_cheatflightoff;
char* txt_cheatpoweron;
char* txt_cheatpoweroff;
char* txt_cheathealth;
char* txt_cheatkeys;
char* txt_cheatsoundon;
char* txt_cheatsoundoff;
char* txt_cheattickeron;
char* txt_cheattickeroff;
char* txt_cheatartifacts1;
char* txt_cheatartifacts2;
char* txt_cheatartifacts3;
char* txt_cheatartifactsfail;
char* txt_cheatwarp;
char* txt_cheatscreenshot;
char* txt_cheatchickenon;
char* txt_cheatchickenoff;
char* txt_cheatmassacre;
char* txt_cheatiddqd;
char* txt_cheatidkfa;

// P_doors.c
char* txt_needbluekey;
char* txt_needgreenkey;
char* txt_needyellowkey;

// G_game.c
char* txt_gamesaved;

// AM_map.c
char* amstr_followon;
char* amstr_followoff;
char* amstr_gridon;
char* amstr_gridoff;
char* amstr_overlayon;
char* amstr_overlayoff;
char* amstr_rotateon;
char* amstr_rotateoff;
char* amstr_markedspot;
char* amstr_markscleared;

// MN_menu.c
char* txt_gamma_improved_off;
char* txt_gamma_improved_05;
char* txt_gamma_improved_1;
char* txt_gamma_improved_15;
char* txt_gamma_improved_2;
char* txt_gamma_improved_25;
char* txt_gamma_improved_3;
char* txt_gamma_improved_35;
char* txt_gamma_improved_4;
char* txt_gamma_original_off;
char* txt_gamma_original_05;
char* txt_gamma_original_1;
char* txt_gamma_original_15;
char* txt_gamma_original_2;
char* txt_gamma_original_25;
char* txt_gamma_original_3;
char* txt_gamma_original_35;
char* txt_gamma_original_4;

// Port-specific strings
char* txt_alwaysrun_on;
char* txt_alwaysrun_off;
char* txt_crosshair_on;
char* txt_crosshair_off;
char* txt_mlook_on;
char* txt_mlook_off;

// -----------------------------------------------------------------------------
// [JN] Function for (pre-)defining strings for text translations.
// -----------------------------------------------------------------------------

void RD_DefineLanguageStrings (void)
{
    if (english_language)
    {
        // Keys
        txt_gotbluekey   = TXT_GOTBLUEKEY;
        txt_gotyellowkey = TXT_GOTYELLOWKEY;
        txt_gotgreenkey  = TXT_GOTGREENKEY;

        // Artifacts
        txt_artihealth          = TXT_ARTIHEALTH;
        txt_artifly             = TXT_ARTIFLY;
        txt_artiinvulnerability = TXT_ARTIINVULNERABILITY;
        txt_artitomeofpower     = TXT_ARTITOMEOFPOWER;
        txt_artiinvisibility    = TXT_ARTIINVISIBILITY;
        txt_artiegg             = TXT_ARTIEGG;
        txt_artisuperhealth     = TXT_ARTISUPERHEALTH;
        txt_artitorch           = TXT_ARTITORCH;
        txt_artifirebomb        = TXT_ARTIFIREBOMB;
        txt_artiteleport        = TXT_ARTITELEPORT;

        // Items
        txt_itemhealth       = TXT_ITEMHEALTH;
        txt_itembagofholding = TXT_ITEMBAGOFHOLDING;
        txt_itemshield1      = TXT_ITEMSHIELD1;
        txt_itemshield2      = TXT_ITEMSHIELD2;
        txt_itemsupermap     = TXT_ITEMSUPERMAP;

        // Ammo
        txt_ammogoldwand1   = TXT_AMMOGOLDWAND1;
        txt_ammogoldwand2   = TXT_AMMOGOLDWAND2;
        txt_ammomace1       = TXT_AMMOMACE1;
        txt_ammomace2       = TXT_AMMOMACE2;
        txt_ammocrossbow1   = TXT_AMMOCROSSBOW1;
        txt_ammocrossbow2   = TXT_AMMOCROSSBOW2;
        txt_ammoblaster1    = TXT_AMMOBLASTER1;
        txt_ammoblaster2    = TXT_AMMOBLASTER2;
        txt_ammoskullrod1   = TXT_AMMOSKULLROD1;
        txt_ammoskullrod2   = TXT_AMMOSKULLROD2;
        txt_ammophoenixrod1 = TXT_AMMOPHOENIXROD1;
        txt_ammophoenixrod2 = TXT_AMMOPHOENIXROD2;

        // Weapons
        txt_wpnmace       = TXT_WPNMACE;
        txt_wpncrossbow   = TXT_WPNCROSSBOW;
        txt_wpnblaster    = TXT_WPNBLASTER;
        txt_wpnskullrod   = TXT_WPNSKULLROD;
        txt_wpnphoenixrod = TXT_WPNPHOENIXROD;
        txt_wpngauntlets  = TXT_WPNGAUNTLETS;

        // SB_bar.c
        txt_cheatgodon         = TXT_CHEATGODON;
        txt_cheatgodoff        = TXT_CHEATGODOFF;
        txt_cheatnoclipon      = TXT_CHEATNOCLIPON;
        txt_cheatnoclipoff     = TXT_CHEATNOCLIPOFF;
        txt_cheatweapons       = TXT_CHEATWEAPONS;
        txt_cheatflighton      = TXT_CHEATFLIGHTON;
        txt_cheatflightoff     = TXT_CHEATFLIGHTOFF;
        txt_cheatpoweron       = TXT_CHEATPOWERON;
        txt_cheatpoweroff      = TXT_CHEATPOWEROFF;
        txt_cheathealth        = TXT_CHEATHEALTH;
        txt_cheatkeys          = TXT_CHEATKEYS;
        txt_cheatsoundon       = TXT_CHEATSOUNDON;
        txt_cheatsoundoff      = TXT_CHEATSOUNDOFF;
        txt_cheattickeron      = TXT_CHEATTICKERON;
        txt_cheattickeroff     = TXT_CHEATTICKEROFF;
        txt_cheatartifacts1    = TXT_CHEATARTIFACTS1;
        txt_cheatartifacts2    = TXT_CHEATARTIFACTS2;
        txt_cheatartifacts3    = TXT_CHEATARTIFACTS3;
        txt_cheatartifactsfail = TXT_CHEATARTIFACTSFAIL;
        txt_cheatwarp          = TXT_CHEATWARP;
        txt_cheatscreenshot    = TXT_CHEATSCREENSHOT;
        txt_cheatchickenon     = TXT_CHEATCHICKENON;
        txt_cheatchickenoff    = TXT_CHEATCHICKENOFF;
        txt_cheatmassacre      = TXT_CHEATMASSACRE;
        txt_cheatiddqd         = TXT_CHEATIDDQD;
        txt_cheatidkfa         = TXT_CHEATIDKFA;

        // P_doors.c
        txt_needbluekey        = TXT_NEEDBLUEKEY;
        txt_needgreenkey       = TXT_NEEDGREENKEY;
        txt_needyellowkey      = TXT_NEEDYELLOWKEY;

        // G_game.c
        txt_gamesaved          = TXT_GAMESAVED;

        // AM_map.c
        amstr_followon         = AMSTR_FOLLOWON;
        amstr_followoff        = AMSTR_FOLLOWOFF;
        amstr_gridon           = AMSTR_GRIDON;
        amstr_gridoff          = AMSTR_GRIDOFF;
        amstr_overlayon        = AMSTR_OVERLAYON;
        amstr_overlayoff       = AMSTR_OVERLAYOFF;
        amstr_rotateon         = AMSTR_ROTATEON;
        amstr_rotateoff        = AMSTR_ROTATEOFF;
        amstr_markedspot       = AMSTR_MARKEDSPOT;
        amstr_markscleared     = AMSTR_MARKSCLEARED;

        // MN_menu.c
        txt_gamma_improved_off = TXT_GAMMA_IMPROVED_OFF;
        txt_gamma_improved_05  = TXT_GAMMA_IMPROVED_05;
        txt_gamma_improved_1   = TXT_GAMMA_IMPROVED_1;
        txt_gamma_improved_15  = TXT_GAMMA_IMPROVED_15;
        txt_gamma_improved_2   = TXT_GAMMA_IMPROVED_2;
        txt_gamma_improved_25  = TXT_GAMMA_IMPROVED_25;
        txt_gamma_improved_3   = TXT_GAMMA_IMPROVED_3;
        txt_gamma_improved_35  = TXT_GAMMA_IMPROVED_35;
        txt_gamma_improved_4   = TXT_GAMMA_IMPROVED_4;
        txt_gamma_original_off = TXT_GAMMA_ORIGINAL_OFF;
        txt_gamma_original_05  = TXT_GAMMA_ORIGINAL_05;
        txt_gamma_original_1   = TXT_GAMMA_ORIGINAL_1;
        txt_gamma_original_15  = TXT_GAMMA_ORIGINAL_15;
        txt_gamma_original_2   = TXT_GAMMA_ORIGINAL_2;
        txt_gamma_original_25  = TXT_GAMMA_ORIGINAL_25;
        txt_gamma_original_3   = TXT_GAMMA_ORIGINAL_3;
        txt_gamma_original_35  = TXT_GAMMA_ORIGINAL_35;
        txt_gamma_original_4   = TXT_GAMMA_ORIGINAL_4;

        // Port-specific strings
        txt_alwaysrun_on       = TXT_ALWAYSRUN_ON;
        txt_alwaysrun_off      = TXT_ALWAYSRUN_OFF;
        txt_crosshair_on       = TXT_CROSSHAIR_ON;
        txt_crosshair_off      = TXT_CROSSHAIR_OFF;
        txt_mlook_on           = TXT_MLOOK_ON;
        txt_mlook_off          = TXT_MLOOK_OFF;
    }
    else
    {
        // Keys
        txt_gotbluekey   = TXT_GOTBLUEKEY_RUS;
        txt_gotyellowkey = TXT_GOTYELLOWKEY_RUS;
        txt_gotgreenkey  = TXT_GOTGREENKEY_RUS;

        // Artifacts
        txt_artihealth          = TXT_ARTIHEALTH_RUS;
        txt_artifly             = TXT_ARTIFLY_RUS;
        txt_artiinvulnerability = TXT_ARTIINVULNERABILITY_RUS;
        txt_artitomeofpower     = TXT_ARTITOMEOFPOWER_RUS;
        txt_artiinvisibility    = TXT_ARTIINVISIBILITY_RUS;
        txt_artiegg             = TXT_ARTIEGG_RUS;
        txt_artisuperhealth     = TXT_ARTISUPERHEALTH_RUS;
        txt_artitorch           = TXT_ARTITORCH_RUS;
        txt_artifirebomb        = TXT_ARTIFIREBOMB_RUS;
        txt_artiteleport        = TXT_ARTITELEPORT_RUS;

        // Items
        txt_itemhealth       = TXT_ITEMHEALTH_RUS;
        txt_itembagofholding = TXT_ITEMBAGOFHOLDING_RUS;
        txt_itemshield1      = TXT_ITEMSHIELD1_RUS;
        txt_itemshield2      = TXT_ITEMSHIELD2_RUS;
        txt_itemsupermap     = TXT_ITEMSUPERMAP_RUS;

        // Ammo
        txt_ammogoldwand1   = TXT_AMMOGOLDWAND1_RUS;
        txt_ammogoldwand2   = TXT_AMMOGOLDWAND2_RUS;
        txt_ammomace1       = TXT_AMMOMACE1_RUS;
        txt_ammomace2       = TXT_AMMOMACE2_RUS;
        txt_ammocrossbow1   = TXT_AMMOCROSSBOW1_RUS;
        txt_ammocrossbow2   = TXT_AMMOCROSSBOW2_RUS;
        txt_ammoblaster1    = TXT_AMMOBLASTER1_RUS;
        txt_ammoblaster2    = TXT_AMMOBLASTER2_RUS;
        txt_ammoskullrod1   = TXT_AMMOSKULLROD1_RUS;
        txt_ammoskullrod2   = TXT_AMMOSKULLROD2_RUS;
        txt_ammophoenixrod1 = TXT_AMMOPHOENIXROD1_RUS;
        txt_ammophoenixrod2 = TXT_AMMOPHOENIXROD2_RUS;

        // Weapons
        txt_wpnmace       = TXT_WPNMACE_RUS;
        txt_wpncrossbow   = TXT_WPNCROSSBOW_RUS;
        txt_wpnblaster    = TXT_WPNBLASTER_RUS;
        txt_wpnskullrod   = TXT_WPNSKULLROD_RUS;
        txt_wpnphoenixrod = TXT_WPNPHOENIXROD_RUS;
        txt_wpngauntlets  = TXT_WPNGAUNTLETS_RUS;

        // SB_bar.c
        txt_cheatgodon         = TXT_CHEATGODON_RUS;
        txt_cheatgodoff        = TXT_CHEATGODOFF_RUS;
        txt_cheatnoclipon      = TXT_CHEATNOCLIPON_RUS;
        txt_cheatnoclipoff     = TXT_CHEATNOCLIPOFF_RUS;
        txt_cheatweapons       = TXT_CHEATWEAPONS_RUS;
        txt_cheatflighton      = TXT_CHEATFLIGHTON_RUS;
        txt_cheatflightoff     = TXT_CHEATFLIGHTOFF_RUS;
        txt_cheatpoweron       = TXT_CHEATPOWERON_RUS;
        txt_cheatpoweroff      = TXT_CHEATPOWEROFF_RUS;
        txt_cheathealth        = TXT_CHEATHEALTH_RUS;
        txt_cheatkeys          = TXT_CHEATKEYS_RUS;
        txt_cheatsoundon       = TXT_CHEATSOUNDON_RUS;
        txt_cheatsoundoff      = TXT_CHEATSOUNDOFF_RUS;
        txt_cheattickeron      = TXT_CHEATTICKERON_RUS;
        txt_cheattickeroff     = TXT_CHEATTICKEROFF_RUS;
        txt_cheatartifacts1    = TXT_CHEATARTIFACTS1_RUS;
        txt_cheatartifacts2    = TXT_CHEATARTIFACTS2_RUS;
        txt_cheatartifacts3    = TXT_CHEATARTIFACTS3_RUS;
        txt_cheatartifactsfail = TXT_CHEATARTIFACTSFAIL_RUS;
        txt_cheatwarp          = TXT_CHEATWARP_RUS;
        txt_cheatscreenshot    = TXT_CHEATSCREENSHOT_RUS;
        txt_cheatchickenon     = TXT_CHEATCHICKENON_RUS;
        txt_cheatchickenoff    = TXT_CHEATCHICKENOFF_RUS;
        txt_cheatmassacre      = TXT_CHEATMASSACRE_RUS;
        txt_cheatiddqd         = TXT_CHEATIDDQD_RUS;
        txt_cheatidkfa         = TXT_CHEATIDKFA_RUS;

        // P_doors.c
        txt_needbluekey        = TXT_NEEDBLUEKEY_RUS;
        txt_needgreenkey       = TXT_NEEDGREENKEY_RUS;
        txt_needyellowkey      = TXT_NEEDYELLOWKEY_RUS;

        // G_game.c
        txt_gamesaved          = TXT_GAMESAVED_RUS;

        // AM_map.c
        amstr_followon         = AMSTR_FOLLOWON_RUS;
        amstr_followoff        = AMSTR_FOLLOWOFF_RUS;
        amstr_gridon           = AMSTR_GRIDON_RUS;
        amstr_gridoff          = AMSTR_GRIDOFF_RUS;
        amstr_overlayon        = AMSTR_OVERLAYON_RUS;
        amstr_overlayoff       = AMSTR_OVERLAYOFF_RUS;
        amstr_rotateon         = AMSTR_ROTATEON_RUS;
        amstr_rotateoff        = AMSTR_ROTATEOFF_RUS;
        amstr_markedspot       = AMSTR_MARKEDSPOT_RUS;
        amstr_markscleared     = AMSTR_MARKSCLEARED_RUS;

        // MN_menu.c
        txt_gamma_improved_off = TXT_GAMMA_IMPROVED_OFF_RUS;
        txt_gamma_improved_05  = TXT_GAMMA_IMPROVED_05_RUS;
        txt_gamma_improved_1   = TXT_GAMMA_IMPROVED_1_RUS;
        txt_gamma_improved_15  = TXT_GAMMA_IMPROVED_15_RUS;
        txt_gamma_improved_2   = TXT_GAMMA_IMPROVED_2_RUS;
        txt_gamma_improved_25  = TXT_GAMMA_IMPROVED_25_RUS;
        txt_gamma_improved_3   = TXT_GAMMA_IMPROVED_3_RUS;
        txt_gamma_improved_35  = TXT_GAMMA_IMPROVED_35_RUS;
        txt_gamma_improved_4   = TXT_GAMMA_IMPROVED_4_RUS;
        txt_gamma_original_off = TXT_GAMMA_ORIGINAL_OFF_RUS;
        txt_gamma_original_05  = TXT_GAMMA_ORIGINAL_05_RUS;
        txt_gamma_original_1   = TXT_GAMMA_ORIGINAL_1_RUS;
        txt_gamma_original_15  = TXT_GAMMA_ORIGINAL_15_RUS;
        txt_gamma_original_2   = TXT_GAMMA_ORIGINAL_2_RUS;
        txt_gamma_original_25  = TXT_GAMMA_ORIGINAL_25_RUS;
        txt_gamma_original_3   = TXT_GAMMA_ORIGINAL_3_RUS;
        txt_gamma_original_35  = TXT_GAMMA_ORIGINAL_35_RUS;
        txt_gamma_original_4   = TXT_GAMMA_ORIGINAL_4_RUS;

        // Port-specific strings
        txt_alwaysrun_on       = TXT_ALWAYSRUN_ON_RUS;
        txt_alwaysrun_off      = TXT_ALWAYSRUN_OFF_RUS;
        txt_crosshair_on       = TXT_CROSSHAIR_ON_RUS;
        txt_crosshair_off      = TXT_CROSSHAIR_OFF_RUS;
        txt_mlook_on           = TXT_MLOOK_ON_RUS;
        txt_mlook_off          = TXT_MLOOK_OFF_RUS;
    }
}

