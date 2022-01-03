//
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

// CT_chat.c
char* txt_msgsent;
char* txt_noplayers;

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
char* txt_cheatmus;
char* txt_cheatmus_e;
char* txt_cheatmus_m;
char* txt_cheatnomus;

// P_doors.c
char* txt_needbluekey;
char* txt_needgreenkey;
char* txt_needyellowkey;

// P_spec.c
char* txt_secret_found;

// G_game.c
char* txt_gamesaved;
char* txt_testcontrols;

// AM_map.c
char *LevelNames[] = {
    // EPISODE 1 - THE CITY OF THE DAMNED
    "E1M1:  THE DOCKS",
    "E1M2:  THE DUNGEONS",
    "E1M3:  THE GATEHOUSE",
    "E1M4:  THE GUARD TOWER",
    "E1M5:  THE CITADEL",
    "E1M6:  THE CATHEDRAL",
    "E1M7:  THE CRYPTS",
    "E1M8:  HELL'S MAW",
    "E1M9:  THE GRAVEYARD",
    // EPISODE 2 - HELL'S MAW
    "E2M1:  THE CRATER",
    "E2M2:  THE LAVA PITS",
    "E2M3:  THE RIVER OF FIRE",
    "E2M4:  THE ICE GROTTO",
    "E2M5:  THE CATACOMBS",
    "E2M6:  THE LABYRINTH",
    "E2M7:  THE GREAT HALL",
    "E2M8:  THE PORTALS OF CHAOS",
    "E2M9:  THE GLACIER",
    // EPISODE 3 - THE DOME OF D'SPARIL
    "E3M1:  THE STOREHOUSE",
    "E3M2:  THE CESSPOOL",
    "E3M3:  THE CONFLUENCE",
    "E3M4:  THE AZURE FORTRESS",
    "E3M5:  THE OPHIDIAN LAIR",
    "E3M6:  THE HALLS OF FEAR",
    "E3M7:  THE CHASM",
    "E3M8:  D'SPARIL'S KEEP",
    "E3M9:  THE AQUIFER",
    // EPISODE 4: THE OSSUARY
    "E4M1:  CATAFALQUE",
    "E4M2:  BLOCKHOUSE",
    "E4M3:  AMBULATORY",
    "E4M4:  SEPULCHER",
    "E4M5:  GREAT STAIR",
    "E4M6:  HALLS OF THE APOSTATE",
    "E4M7:  RAMPARTS OF PERDITION",
    "E4M8:  SHATTERED BRIDGE",
    "E4M9:  MAUSOLEUM",
    // EPISODE 5: THE STAGNANT DEMESNE
    "E5M1:  OCHRE CLIFFS",
    "E5M2:  RAPIDS",
    "E5M3:  QUAY",
    "E5M4:  COURTYARD",
    "E5M5:  HYDRATYR",
    "E5M6:  COLONNADE",
    "E5M7:  FOETID MANSE",
    "E5M8:  FIELD OF JUDGEMENT",
    "E5M9:  SKEIN OF D'SPARIL",
    // EPISODE 6: unnamed
    "E6M1:  ",
    "E6M2:  ",
    "E6M3:  ",
};

char *LevelNames_Rus[] = {
    // ЭПИЗОД 1 - ГОРОД ПРОКЛЯТЫХ
    "T1V1:  LJRB",                  // E1M1: ДОКИ
    "T1V2:  NTVYBWS",               // E1M2: ТЕМНИЦЫ
    "T1V3:  GHBDHFNYBWRFZ",         // E1M3: ПРИВРАТНИЦКАЯ
    "T1V4:  CNJHJ;TDFZ ,FIYZ",      // E1M4: СТОРОЖЕВАЯ БАШНЯ
    "T1V5:  WBNFLTKM",              // E1M5: ЦИТАДЕЛЬ
    "T1V6:  RFATLTHFKMYSQ CJ,JH",   // E1M6: КАФЕДРАЛЬНЫЙ СОБОР
    "T1V7:  CRKTGS",                // E1M7: СКЛЕПЫ
    "T1V8:  FLCRFZ ENHJ,F",         // E1M8: АДСКАЯ УТРОБА
    "T1V9:  RKFL,BOT",              // E1M9: КЛАДБИЩЕ
    // ЭПИЗОД 2 - АДСКАЯ УТРОБА
    "T2V1:  RHFNTH",                // E2M1: КРАТЕР
    "T2V2:  KFDJDST JXFUB",         // E2M2: ЛАВОВЫЕ ОЧАГИ
    "T2V3:  HTRF JUYZ",             // E2M3: РЕКА ОГНЯ
    "T2V4:  KTLZYJQ UHJN",          // E2M4: ЛЕДЯНОЙ ГРОТ
    "T2V5:  RFNFRJV,S",             // E2M5: КАТАКОМБЫ
    "T2V6:  KF,BHBYN",              // E2M6: ЛАБИРИНТ
    "T2V7:  ,JKMIJQ PFK",           // E2M7: БОЛЬШОЙ ЗАЛ
    "T2V8:  GJHNFKS [FJCF",         // E2M8: ПОРТАЛЫ ХАОСА
    "T2V9:  KTLYBR",                // E2M9: ЛЕДНИК 
    // ЭПИЗОД 3 - КУПОЛ Д'СПАРИЛА
    "T3V1:  RKFLJDFZ",              // E3M1: КЛАДОВАЯ
    "T3V2:  CNJXYSQ RJKJLTW",       // E3M2: СТОЧНЫЙ КОЛОДЕЦ
    "T3V3:  CKBZYBT",               // E3M3: СЛИЯНИЕ 
    "T3V4:  KFPEHYFZ RHTGJCNM",     // E3M4: ЛАЗУРНАЯ КРЕПОСТЬ
    "T3V5:  KJUJDJ JABLBFYJD",      // E3M5: ЛОГОВО ОФИДИАНОВ
    "T3V6:  PFKS CNHF[F",           // E3M6: ЗАЛЫ СТРАХА
    "T3V7:  GHJGFCNM",              // E3M7: ПРОПАСТЬ
    "T3V8:  RHTGJCNM L\"CGFHBKF",   // E3M8: КРЕПОСТЬ Д'СПАРИЛА
    "T3V9:  DJLJYJCYSQ CKJQ",       // E3M9: ВОДОНОСНЫЙ СЛОЙ
    // ЭПИЗОД 4: СКЛЕП
    "T4V1:  RFNFAFKR",              // E4M1: КАТАФАЛК
    "T4V2:  ERHSNBT",               // E4M2: УКРЫТИЕ
    "T4V3:  VJYFCNSHCRFZ UFKTHTZ",  // E4M3: МОНАСТЫРСКАЯ ГАЛЕРЕЯ
    "T4V4:  UHJ,YBWF",              // E4M4: ГРОБНИЦА
    "T4V5:  DTKBRFZ KTCNYBWF",      // E4M5: ВЕЛИКАЯ ЛЕСТНИЦА
    "T4V6:  PFKS JNCNEGYBRJD",      // E4M6: ЗАЛЫ ОТСТУПНИКОВ
    "T4V7:  NDTHLSYB GJUB,TKB",     // E4M7: ТВЕРДЫНИ ПОГИБЕЛИ
    "T4V8:  HFPHEITYYSQ VJCN",      // E4M8: РАЗРУШЕННЫЙ МОСТ
    "T4V9:  VFDPJKTQ",              // E4M9: МАВЗОЛЕЙ
    // ЭПИЗОД 5: ЗАСТОЙНЫЕ ВЛАДЕНИЯ  
    "T5V1:  J[HJDST ENTCS",         // E5M1: ОХРОВЫЕ УТЕСЫ
    "T5V2:  CNHTVYBYF",             // E5M2: СТРЕМНИНА
    "T5V3:  GHBXFK",                // E5M3: ПРИЧАЛ
    "T5V4:  DYENHTYYBQ LDJH",       // E5M4: ВНУТРЕННИЙ ДВОР
    "T5V5:  UBLHJNBH",              // E5M5: ГИДРОТИР
    "T5V6:  RJKJYYFLF",             // E5M6: КОЛОННАДА 
    "T5V7:  PKJDJYYSQ JCJ,YZR",     // E5M7: ЗЛОВОННЫЙ ОСОБНЯК
    "T5V8:  GJKT DSCITUJ CELF",     // E5M8: ПОЛЕ ВЫСШЕГО СУДА
    "T5V9:  GENFYBWF L\"CGFHBKF",   // E5M9: ПУТАНИЦА Д'СПАРИЛА
    // ЭПИЗОД 6: БЕЗ НАЗВАНИЯ
    "T6V1:  ",
    "T6V2:  ",
    "T6V3:  ",
};
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
char* amstr_mark;
char* amstr_added;
char* amstr_cleared;

// MN_menu.c
char* txt_gammamsg;
char* txt_detail_high;
char* txt_detail_low;
char* txt_messages_on;
char* txt_messages_off;
char* txt_settings_reset;
char* txt_cant_start_in_netgame;
char* txt_cant_load_in_netgame;
char* txt_registered_only;
char* txt_quicksaving;
char* txt_quickloading;
char* txt_choose_qsave;
char* txt_choose_qload;

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

        // CT_chat.c
        txt_msgsent   = TXT_MSGSENT;
        txt_noplayers = TXT_NOPLAYERS;

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
        txt_cheatmus           = TXT_CHEATMUS;
        txt_cheatmus_e         = TXT_CHEATMUS_E;
        txt_cheatmus_m         = TXT_CHEATMUS_M;
        txt_cheatnomus         = TXT_CHEATNOMUS;

        // P_doors.c
        txt_needbluekey        = TXT_NEEDBLUEKEY;
        txt_needgreenkey       = TXT_NEEDGREENKEY;
        txt_needyellowkey      = TXT_NEEDYELLOWKEY;

        // P_spec.c
        txt_secret_found       = TXT_SECRET_FOUND;

        // G_game.c
        txt_gamesaved          = TXT_GAMESAVED;
        txt_testcontrols       = TXT_TESTCONTROLS;

        // AM_map.c
        amstr_followon         = AMSTR_FOLLOWON;
        amstr_followoff        = AMSTR_FOLLOWOFF;
        amstr_gridon           = AMSTR_GRIDON;
        amstr_gridoff          = AMSTR_GRIDOFF;
        amstr_overlayon        = AMSTR_OVERLAYON;
        amstr_overlayoff       = AMSTR_OVERLAYOFF;
        amstr_rotateon         = AMSTR_ROTATEON;
        amstr_rotateoff        = AMSTR_ROTATEOFF;
        amstr_mark             = AMSTR_MARK;
        amstr_added            = AMSTR_ADDED;
        amstr_cleared          = AMSTR_CLEARED;
        amstr_markscleared     = AMSTR_MARKSCLEARED;

        // MN_menu.c
        txt_gammamsg           = TXT_GAMMA_LEVEL;
        txt_detail_high        = TXT_GAMMA_DETAIL_HIGH;
        txt_detail_low         = TXT_GAMMA_DETAIL_LOW;
        txt_messages_on        = TXT_MESSAGES_ON;
        txt_messages_off       = TXT_MESSAGES_OFF;
        txt_settings_reset     = TXT_SETTINGS_RESET;
        txt_cant_start_in_netgame = TXT_CANT_START_IN_NETGAME;
        txt_cant_load_in_netgame  = TXT_CANT_LOAD_IN_NETGAME;
        txt_registered_only    = TXT_REGISTERED_ONLY;
        txt_quicksaving        = TXT_QUICKSAVING;
        txt_quickloading       = TXT_QUICKLOADING;
        txt_choose_qsave       = TXT_CHOOSE_QSAVE;
        txt_choose_qload       = TXT_CHOOSE_QLOAD;

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

        // CT_chat.c
        txt_msgsent   = TXT_MSGSENT_RUS;
        txt_noplayers = TXT_NOPLAYERS_RUS;

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
        txt_cheatmus           = TXT_CHEATMUS_RUS;
        txt_cheatmus_e         = TXT_CHEATMUS_E_RUS;
        txt_cheatmus_m         = TXT_CHEATMUS_M_RUS;
        txt_cheatnomus         = TXT_CHEATNOMUS_RUS;

        // P_doors.c
        txt_needbluekey        = TXT_NEEDBLUEKEY_RUS;
        txt_needgreenkey       = TXT_NEEDGREENKEY_RUS;
        txt_needyellowkey      = TXT_NEEDYELLOWKEY_RUS;

        // P_spec.c
        txt_secret_found       = TXT_SECRET_FOUND_RUS;

        // G_game.c
        txt_gamesaved          = TXT_GAMESAVED_RUS;
        txt_testcontrols       = TXT_TESTCONTROLS_RUS;

        // AM_map.c
        amstr_followon         = AMSTR_FOLLOWON_RUS;
        amstr_followoff        = AMSTR_FOLLOWOFF_RUS;
        amstr_gridon           = AMSTR_GRIDON_RUS;
        amstr_gridoff          = AMSTR_GRIDOFF_RUS;
        amstr_overlayon        = AMSTR_OVERLAYON_RUS;
        amstr_overlayoff       = AMSTR_OVERLAYOFF_RUS;
        amstr_rotateon         = AMSTR_ROTATEON_RUS;
        amstr_rotateoff        = AMSTR_ROTATEOFF_RUS;
        amstr_mark             = AMSTR_MARK_RUS;
        amstr_added            = AMSTR_ADDED_RUS;
        amstr_cleared          = AMSTR_CLEARED_RUS;
        amstr_markscleared     = AMSTR_MARKSCLEARED_RUS;

        // MN_menu.c
        txt_gammamsg           = TXT_GAMMA_LEVEL_RUS;
        txt_detail_high        = TXT_GAMMA_DETAIL_HIGH_RUS;
        txt_detail_low         = TXT_GAMMA_DETAIL_LOW_RUS;
        txt_messages_on        = TXT_MESSAGES_ON_RUS;
        txt_messages_off       = TXT_MESSAGES_OFF_RUS;
        txt_settings_reset     = TXT_SETTINGS_RESET_RUS;
        txt_cant_start_in_netgame = TXT_CANT_START_IN_NETGAME_RUS;
        txt_cant_load_in_netgame  = TXT_CANT_LOAD_IN_NETGAME_RUS;
        txt_registered_only    = TXT_REGISTERED_ONLY_RUS;
        txt_quicksaving        = TXT_QUICKSAVING_RUS;
        txt_quickloading       = TXT_QUICKLOADING_RUS;
        txt_choose_qsave       = TXT_CHOOSE_QSAVE_RUS;
        txt_choose_qload       = TXT_CHOOSE_QLOAD_RUS;

        // Port-specific strings
        txt_alwaysrun_on       = TXT_ALWAYSRUN_ON_RUS;
        txt_alwaysrun_off      = TXT_ALWAYSRUN_OFF_RUS;
        txt_crosshair_on       = TXT_CROSSHAIR_ON_RUS;
        txt_crosshair_off      = TXT_CROSSHAIR_OFF_RUS;
        txt_mlook_on           = TXT_MLOOK_ON_RUS;
        txt_mlook_off          = TXT_MLOOK_OFF_RUS;
    }
}

