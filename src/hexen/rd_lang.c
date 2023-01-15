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


#include "jn.h"
#include "rd_lang.h"


// -----------------------------------------------------------------------------
// [JN] Chars* to be defined and used as a predefined text strings.
// -----------------------------------------------------------------------------

//
// AM_map.c
//

char* amstr_followon;
char* amstr_followoff;
char* amstr_gridon;
char* amstr_gridoff;
char* amstr_overlayon;
char* amstr_overlayoff;
char* amstr_rotateon;
char* amstr_rotateoff;
char* amstr_mark;
char* amstr_added;
char* amstr_cleared;
char* amstr_markscleared;

//
// CT_chat.c
//

char* txt_noplayers;

//
// G_game.c
//

char* txt_gamesaved;
char* txt_gameloaded;
char* txt_alwaysrun_on;
char* txt_alwaysrun_off;
char* txt_crosshair_on;
char* txt_crosshair_off;
char* txt_mlook_on;
char* txt_mlook_off;
char* txt_testcontrols;
char* txt_access_denied;

//
// MN_menu.c
//

char* txt_gammamsg;
char* txt_messages_on;
char* txt_messages_off;
char* txt_settings_reset;
char* txt_cant_start_in_netgame;
char* txt_cant_load_in_netgame;
char* txt_cant_end_in_netgame;

//
// P_acs.c
//

char* txt_unknown_script;
char* txt_you_need_the;

//
// P_inter.c
//

// Mana
char* txt_mana_1;
char* txt_mana_2;
char* txt_mana_both;

// Items
char* txt_itemhealth;
char* txt_armor1;
char* txt_armor2;
char* txt_armor3;
char* txt_armor4;

// Weapons
char* txt_weapon_f2;
char* txt_weapon_f3;
char* txt_weapon_f4;
char* txt_weapon_c2;
char* txt_weapon_c3;
char* txt_weapon_c4;
char* txt_weapon_m2;
char* txt_weapon_m3;
char* txt_weapon_m4;
char* txt_quietus_piece;
char* txt_wraithverge_piece;
char* txt_bloodscourge_piece;

//
// SB_bar.c
//

char* txt_cheatgodon;
char* txt_cheatgodoff;
char* txt_cheatnoclipon;
char* txt_cheatnoclipoff;
char* txt_cheatweapons;
char* txt_cheathealth;
char* txt_cheatkeys;
char* txt_cheatsoundon;
char* txt_cheatsoundoff;
char* txt_cheatartifacts3;
char* txt_cheatartifactsfail;
char* txt_cheatwarp;
char* txt_cheatscreenshot;
char* txt_cheatiddqd;
char* txt_cheatidkfa;
char* txt_cheatbadinput;
char* txt_cheatnomap;
char* txt_squeal;
char* txt_quicken_1;
char* txt_quicken_2;
char* txt_quicken_3;
char* txt_new_class;
char* txt_invalid_class;
char* txt_which_script;
char* txt_running_script;

// -----------------------------------------------------------------------------
// [JN] Function for (pre-/re-)defining strings for text translations.
// -----------------------------------------------------------------------------

void RD_DefineLanguageStrings (void)
{
    if (english_language)
    {
        //
        // AM_map.c
        //

        amstr_followon   = AMSTR_FOLLOWON;
        amstr_followoff  = AMSTR_FOLLOWOFF;
        amstr_gridon     = AMSTR_GRIDON;
        amstr_gridoff    = AMSTR_GRIDOFF;
        amstr_overlayon  = AMSTR_OVERLAYON;
        amstr_overlayoff = AMSTR_OVERLAYOFF;
        amstr_rotateon   = AMSTR_ROTATEON;
        amstr_rotateoff  = AMSTR_ROTATEOFF;
        amstr_mark         = AMSTR_MARK;
        amstr_added        = AMSTR_ADDED;
        amstr_cleared      = AMSTR_CLEARED;
        amstr_markscleared = AMSTR_MARKSCLEARED;

        //
        // CT_chat.c
        //

        txt_noplayers = TXT_NOPLAYERS;

        //
        // G_game.c
        //

        txt_gamesaved     = TXT_GAMESAVED;
        txt_gameloaded    = TXT_GAMELOADED;
        txt_alwaysrun_on  = TXT_ALWAYSRUN_ON;
        txt_alwaysrun_off = TXT_ALWAYSRUN_OFF;
        txt_crosshair_on  = TXT_CROSSHAIR_ON;
        txt_crosshair_off = TXT_CROSSHAIR_OFF;
        txt_mlook_on      = TXT_MLOOK_ON;
        txt_mlook_off     = TXT_MLOOK_OFF;
        txt_testcontrols  = TXT_TESTCONTROLS;
        txt_access_denied = TXT_ACCESS_DENIED;

        //
        // MN_menu.c
        //

        txt_gammamsg              = TXT_GAMMA_LEVEL;
        txt_messages_on           = TXT_MESSAGES_ON;
        txt_messages_off          = TXT_MESSAGES_OFF;
        txt_settings_reset        = TXT_SETTINGS_RESET;
        txt_cant_start_in_netgame = TXT_CANT_START_IN_NETGAME;
        txt_cant_load_in_netgame  = TXT_CANT_LOAD_IN_NETGAME;
        txt_cant_end_in_netgame   = TXT_CANT_END_IN_NETGAME;

        //
        // P_acs.c
        //

        txt_unknown_script = TXT_UNKNOWN_SCRIPT;
        txt_you_need_the   = TXT_YOU_NEED_THE;

        //
        // P_inter.c
        //

        // Mana
        txt_mana_1    = TXT_MANA_1;
        txt_mana_2    = TXT_MANA_2;
        txt_mana_both = TXT_MANA_BOTH;

        // Items
        txt_itemhealth       = TXT_ITEMHEALTH;
        txt_armor1           = TXT_ARMOR1;
        txt_armor2           = TXT_ARMOR2;
        txt_armor3           = TXT_ARMOR3;
        txt_armor4           = TXT_ARMOR4;

        // Weapons
        txt_weapon_f2          = TXT_WEAPON_F2;
        txt_weapon_f3          = TXT_WEAPON_F3;
        txt_weapon_f4          = TXT_WEAPON_F4;
        txt_weapon_c2          = TXT_WEAPON_C2;
        txt_weapon_c3          = TXT_WEAPON_C3;
        txt_weapon_c4          = TXT_WEAPON_C4;
        txt_weapon_m2          = TXT_WEAPON_M2;
        txt_weapon_m3          = TXT_WEAPON_M3;
        txt_weapon_m4          = TXT_WEAPON_M4;
        txt_quietus_piece      = TXT_QUIETUS_PIECE;
        txt_wraithverge_piece  = TXT_WRAITHVERGE_PIECE;
        txt_bloodscourge_piece = TXT_BLOODSCOURGE_PIECE;

        //
        // SB_bar.c
        //

        txt_cheatgodon         = TXT_CHEATGODON;
        txt_cheatgodoff        = TXT_CHEATGODOFF;
        txt_cheatnoclipon      = TXT_CHEATNOCLIPON;
        txt_cheatnoclipoff     = TXT_CHEATNOCLIPOFF;
        txt_cheatweapons       = TXT_CHEATWEAPONS;
        txt_cheathealth        = TXT_CHEATHEALTH;
        txt_cheatkeys          = TXT_CHEATKEYS;
        txt_cheatsoundon       = TXT_CHEATSOUNDON;
        txt_cheatsoundoff      = TXT_CHEATSOUNDOFF;
        txt_cheatartifacts3    = TXT_CHEATARTIFACTS3;
        txt_cheatartifactsfail = TXT_CHEATARTIFACTSFAIL;
        txt_cheatwarp          = TXT_CHEATWARP;
        txt_cheatscreenshot    = TXT_CHEATSCREENSHOT;
        txt_cheatiddqd         = TXT_CHEATIDDQD;
        txt_cheatidkfa         = TXT_CHEATIDKFA;
        txt_cheatbadinput      = TXT_CHEATBADINPUT;
        txt_cheatnomap         = TXT_CHEATNOMAP;
        txt_squeal             = TXT_SQUEAL;
        txt_quicken_1          = TXT_QUICKEN_1;
        txt_quicken_2          = TXT_QUICKEN_2;
        txt_quicken_3          = TXT_QUICKEN_3;
        txt_new_class          = TXT_NEW_CLASS;
        txt_invalid_class      = TXT_INVALID_CLASS;
        txt_which_script       = TXT_WHICH_SCRIPT;
        txt_running_script     = TXT_RUNNING_SCRIPT;
    }
    else
    {
        //
        // AM_map.c
        //

        amstr_followon   = AMSTR_FOLLOWON_RUS;
        amstr_followoff  = AMSTR_FOLLOWOFF_RUS;
        amstr_gridon     = AMSTR_GRIDON_RUS;
        amstr_gridoff    = AMSTR_GRIDOFF_RUS;
        amstr_overlayon  = AMSTR_OVERLAYON_RUS;
        amstr_overlayoff = AMSTR_OVERLAYOFF_RUS;
        amstr_rotateon   = AMSTR_ROTATEON_RUS;
        amstr_rotateoff  = AMSTR_ROTATEOFF_RUS;
        amstr_mark         = AMSTR_MARK_RUS;
        amstr_added        = AMSTR_ADDED_RUS;
        amstr_cleared      = AMSTR_CLEARED_RUS;
        amstr_markscleared = AMSTR_MARKSCLEARED_RUS;

        //
        // CT_chat.c
        //

        txt_noplayers = TXT_NOPLAYERS_RUS;

        //
        // G_game.c
        //

        txt_gamesaved     = TXT_GAMESAVED_RUS;
        txt_gameloaded    = TXT_GAMELOADED_RUS;
        txt_alwaysrun_on  = TXT_ALWAYSRUN_ON_RUS;
        txt_alwaysrun_off = TXT_ALWAYSRUN_OFF_RUS;
        txt_crosshair_on  = TXT_CROSSHAIR_ON_RUS;
        txt_crosshair_off = TXT_CROSSHAIR_OFF_RUS;
        txt_mlook_on      = TXT_MLOOK_ON_RUS;
        txt_mlook_off     = TXT_MLOOK_OFF_RUS;
        txt_testcontrols  = TXT_TESTCONTROLS_RUS;
        txt_access_denied = TXT_ACCESS_DENIED_RUS;

        //
        // MN_menu.c
        //

        txt_gammamsg              = TXT_GAMMA_LEVEL_RUS;
        txt_messages_on           = TXT_MESSAGES_ON_RUS;
        txt_messages_off          = TXT_MESSAGES_OFF_RUS;
        txt_settings_reset        = TXT_SETTINGS_RESET_RUS;
        txt_cant_start_in_netgame = TXT_CANT_START_IN_NETGAME_RUS;
        txt_cant_load_in_netgame  = TXT_CANT_LOAD_IN_NETGAME_RUS;
        txt_cant_end_in_netgame   = TXT_CANT_END_IN_NETGAME_RUS;

        //
        // P_acs.c
        //

        txt_unknown_script = TXT_UNKNOWN_SCRIPT_RUS;
        txt_you_need_the   = TXT_YOU_NEED_THE_RUS;

        //
        // P_inter.c
        //

        // Mana
        txt_mana_1    = TXT_MANA_1_RUS;
        txt_mana_2    = TXT_MANA_2_RUS;
        txt_mana_both = TXT_MANA_BOTH_RUS;

        // Items
        txt_itemhealth = TXT_ITEMHEALTH_RUS;
        txt_armor1     = TXT_ARMOR1_RUS;
        txt_armor2     = TXT_ARMOR2_RUS;
        txt_armor3     = TXT_ARMOR3_RUS;
        txt_armor4     = TXT_ARMOR4_RUS;

        // Weapons
        txt_weapon_f2          = TXT_WEAPON_F2_RUS;
        txt_weapon_f3          = TXT_WEAPON_F3_RUS;
        txt_weapon_f4          = TXT_WEAPON_F4_RUS;
        txt_weapon_c2          = TXT_WEAPON_C2_RUS;
        txt_weapon_c3          = TXT_WEAPON_C3_RUS;
        txt_weapon_c4          = TXT_WEAPON_C4_RUS;
        txt_weapon_m2          = TXT_WEAPON_M2_RUS;
        txt_weapon_m3          = TXT_WEAPON_M3_RUS;
        txt_weapon_m4          = TXT_WEAPON_M4_RUS;
        txt_quietus_piece      = TXT_QUIETUS_PIECE_RUS;
        txt_wraithverge_piece  = TXT_WRAITHVERGE_PIECE_RUS;
        txt_bloodscourge_piece = TXT_BLOODSCOURGE_PIECE_RUS;

        //
        // SB_bar.c
        //

        txt_cheatgodon         = TXT_CHEATGODON_RUS;
        txt_cheatgodoff        = TXT_CHEATGODOFF_RUS;
        txt_cheatnoclipon      = TXT_CHEATNOCLIPON_RUS;
        txt_cheatnoclipoff     = TXT_CHEATNOCLIPOFF_RUS;
        txt_cheatweapons       = TXT_CHEATWEAPONS_RUS;
        txt_cheathealth        = TXT_CHEATHEALTH_RUS;
        txt_cheatkeys          = TXT_CHEATKEYS_RUS;
        txt_cheatsoundon       = TXT_CHEATSOUNDON_RUS;
        txt_cheatsoundoff      = TXT_CHEATSOUNDOFF_RUS;
        txt_cheatartifacts3    = TXT_CHEATARTIFACTS3_RUS;
        txt_cheatartifactsfail = TXT_CHEATARTIFACTSFAIL_RUS;
        txt_cheatwarp          = TXT_CHEATWARP_RUS;
        txt_cheatscreenshot    = TXT_CHEATSCREENSHOT_RUS;
        txt_cheatiddqd         = TXT_CHEATIDDQD_RUS;
        txt_cheatidkfa         = TXT_CHEATIDKFA_RUS;
        txt_cheatbadinput      = TXT_CHEATBADINPUT_RUS;
        txt_cheatnomap         = TXT_CHEATNOMAP_RUS;
        txt_squeal             = TXT_SQUEAL_RUS;
        txt_quicken_1          = TXT_QUICKEN_1_RUS;
        txt_quicken_2          = TXT_QUICKEN_2_RUS;
        txt_quicken_3          = TXT_QUICKEN_3_RUS;
        txt_new_class          = TXT_NEW_CLASS_RUS;
        txt_invalid_class      = TXT_INVALID_CLASS_RUS;
        txt_which_script       = TXT_WHICH_SCRIPT_RUS;
        txt_running_script     = TXT_RUNNING_SCRIPT_RUS;
    }
}
