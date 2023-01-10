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


#pragma once


extern void RD_DefineLanguageStrings ();


//
// AM_map.c
//

extern char *amstr_followon;
extern char *amstr_followoff;
extern char *amstr_gridon;
extern char *amstr_gridoff;
extern char *amstr_overlayon;
extern char *amstr_overlayoff;
extern char *amstr_rotateon;
extern char *amstr_rotateoff;
extern char *amstr_mark;
extern char *amstr_added;
extern char *amstr_cleared;
extern char *amstr_markscleared;

//
// CT_chat.c
//

extern char *txt_noplayers;

//
// G_game.c
//

extern char *txt_gamesaved;
extern char* txt_gameloaded;
extern char *txt_alwaysrun_on;
extern char *txt_alwaysrun_off;
extern char *txt_crosshair_on;
extern char *txt_crosshair_off;
extern char *txt_mlook_on;
extern char *txt_mlook_off;
extern char *txt_testcontrols;
extern char *txt_access_denied;

//
// MN_menu.c
//

extern char *txt_gammamsg;
extern char *txt_messages_on;
extern char *txt_messages_off;
extern char *txt_settings_reset;
extern char *txt_cant_start_in_netgame;
extern char *txt_cant_load_in_netgame;
extern char *txt_cant_end_in_netgame;

//
// P_acs.c
//

extern char *txt_unknown_script;
extern char *txt_you_need_the;

//
// P_inter.c
//

// Mana
extern char *txt_mana_1;
extern char *txt_mana_2;
extern char *txt_mana_both;

// Items
extern char *txt_itemhealth;
extern char *txt_armor1;
extern char *txt_armor2;
extern char *txt_armor3;
extern char *txt_armor4;

// Weapons
extern char *txt_weapon_f2;
extern char *txt_weapon_f3;
extern char *txt_weapon_f4;
extern char *txt_weapon_c2;
extern char *txt_weapon_c3;
extern char *txt_weapon_c4;
extern char *txt_weapon_m2;
extern char *txt_weapon_m3;
extern char *txt_weapon_m4;
extern char *txt_quietus_piece;
extern char *txt_wraithverge_piece;
extern char *txt_bloodscourge_piece;

//
// SB_bar.c
//

extern char *txt_cheatgodon;
extern char *txt_cheatgodoff;
extern char *txt_cheatnoclipon;
extern char *txt_cheatnoclipoff;
extern char *txt_cheatweapons;
extern char *txt_cheathealth;
extern char *txt_cheatkeys;
extern char *txt_cheatsoundon;
extern char *txt_cheatsoundoff;
extern char *txt_cheatartifacts3;
extern char *txt_cheatartifactsfail;
extern char *txt_cheatwarp;
extern char *txt_cheatscreenshot;
extern char *txt_cheatiddqd;
extern char *txt_cheatidkfa;
extern char *txt_cheatbadinput;
extern char *txt_cheatnomap;
extern char *txt_squeal;
extern char *txt_quicken_1;
extern char *txt_quicken_2;
extern char *txt_quicken_3;
extern char *txt_new_class;
extern char *txt_invalid_class;
extern char *txt_which_script;
extern char *txt_running_script;

// -----------------------------------------------------------------------------
// English language
// -----------------------------------------------------------------------------

//
// AM_map.c
//

#define AMSTR_FOLLOWON              "FOLLOW MODE ON"
#define AMSTR_FOLLOWOFF             "FOLLOW MODE OFF"
#define AMSTR_GRIDON                "GRID ON"
#define AMSTR_GRIDOFF               "GRID OFF"
#define AMSTR_OVERLAYON             "OVERLAY MODE ON"
#define AMSTR_OVERLAYOFF            "OVERLAY MODE OFF"
#define AMSTR_ROTATEON              "ROTATE MODE ON"
#define AMSTR_ROTATEOFF             "ROTATE MODE OFF"
#define AMSTR_MARK                  "MARK"
#define AMSTR_ADDED                 "ADDED"
#define AMSTR_CLEARED               "CLEARED"
#define AMSTR_MARKSCLEARED          "ALL MARKS CLEARED"

//
// CT_chat.c
//

#define TXT_NOPLAYERS               "THERE ARE NO OTHER PLAYERS IN THE GAME!"
#define HUSTR_CHATMACRO1            "I'm ready to kick butt!"
#define HUSTR_CHATMACRO2            "I'm OK."
#define HUSTR_CHATMACRO3            "I'm not looking too good!"
#define HUSTR_CHATMACRO4            "Help!"
#define HUSTR_CHATMACRO5            "You suck!"
#define HUSTR_CHATMACRO6            "Next time, scumbag..."
#define HUSTR_CHATMACRO7            "Come here!"
#define HUSTR_CHATMACRO8            "I'll take care of it."
#define HUSTR_CHATMACRO9            "Yes"
#define HUSTR_CHATMACRO0            "No"

//
// G_game.c
//

#define TXT_GAMESAVED               "GAME SAVED"
#define TXT_GAMELOADED              "GAME LOADED"
#define TXT_ALWAYSRUN_ON            "ALWAYS RUN ON"
#define TXT_ALWAYSRUN_OFF           "ALWAYS RUN OFF"
#define TXT_MLOOK_ON                "MOUSE LOOK ON"
#define TXT_MLOOK_OFF               "MOUSE LOOK OFF"
#define TXT_CROSSHAIR_ON            "CROSSHAIR ON"
#define TXT_CROSSHAIR_OFF           "CROSSHAIR OFF"
#define TXT_TESTCONTROLS            "PRESS ESCAPE TO QUIT"
#define TXT_ACCESS_DENIED           "ACCESS DENIED -- DEMO"

//
// MN_menu.c
//

#define TXT_GAMMA_LEVEL             "GAMMA CORRECTION LEVEL "
#define TXT_MESSAGES_ON             "MESSAGES ON"
#define TXT_MESSAGES_OFF            "MESSAGES OFF"
#define TXT_SETTINGS_RESET          "SETTINGS RESET"
#define TXT_CANT_START_IN_NETGAME   "YOU CAN'T START A NEW GAME IN NETPLAY!"
#define TXT_CANT_LOAD_IN_NETGAME    "YOU CAN'T LOAD A GAME IN NETPLAY!"
#define TXT_CANT_END_IN_NETGAME     "YOU CAN'T END A GAME IN NETPLAY!"

#define RD_ON                       "ON"
#define RD_OFF                      "OFF"

//
// P_acs.c
//

#define TXT_UNKNOWN_SCRIPT          "P[STARTACS ERROR: UNKNOWN SCRIPT"
#define TXT_YOU_NEED_THE            "YOU NEED THE"

//
// P_inter.c
//

// Mana
#define TXT_MANA_1                  "BLUE MANA"
#define TXT_MANA_2                  "GREEN MANA"
#define TXT_MANA_BOTH               "COMBINED MANA"

// Keys
#define	TXT_KEY_STEEL               "STEEL KEY"
#define	TXT_KEY_CAVE                "CAVE KEY"
#define	TXT_KEY_AXE                 "AXE KEY"
#define	TXT_KEY_FIRE                "FIRE KEY"
#define	TXT_KEY_EMERALD             "EMERALD KEY"
#define	TXT_KEY_DUNGEON             "DUNGEON KEY"
#define	TXT_KEY_SILVER              "SILVER KEY"
#define	TXT_KEY_RUSTED              "RUSTED KEY"
#define	TXT_KEY_HORN                "HORN KEY"
#define	TXT_KEY_SWAMP               "SWAMP KEY"
#define TXT_KEY_CASTLE              "CASTLE KEY"

// Artifacts
#define TXT_ARTIINVULNERABILITY     "ICON OF THE DEFENDER"
#define TXT_ARTIHEALTH              "QUARTZ FLASK"
#define TXT_ARTISUPERHEALTH         "MYSTIC URN"
#define TXT_ARTISUMMON              "DARK SERVANT"
#define TXT_ARTITORCH               "TORCH"
#define TXT_ARTIEGG                 "PORKALATOR"
#define TXT_ARTIFLY                 "WINGS OF WRATH"
#define TXT_ARTITELEPORT            "CHAOS DEVICE"
#define TXT_ARTIPOISONBAG           "FLECHETTE"
#define TXT_ARTITELEPORTOTHER       "BANISHMENT DEVICE"
#define TXT_ARTISPEED               "BOOTS OF SPEED"
#define TXT_ARTIBOOSTMANA           "KRATER OF MIGHT"
#define TXT_ARTIBOOSTARMOR          "DRAGONSKIN BRACERS"
#define TXT_ARTIBLASTRADIUS         "DISC OF REPULSION"
#define TXT_ARTIHEALINGRADIUS       "MYSTIC AMBIT INCANT"

// Puzzle artifacts
#define TXT_ARTIPUZZSKULL			"YORICK'S SKULL"
#define TXT_ARTIPUZZGEMBIG			"HEART OF D'SPARIL"
#define TXT_ARTIPUZZGEMRED			"RUBY PLANET"
#define TXT_ARTIPUZZGEMGREEN1		"EMERALD PLANET"
#define TXT_ARTIPUZZGEMGREEN2		"EMERALD PLANET"
#define TXT_ARTIPUZZGEMBLUE1		"SAPPHIRE PLANET"
#define TXT_ARTIPUZZGEMBLUE2		"SAPPHIRE PLANET"
#define TXT_ARTIPUZZBOOK1			"DAEMON CODEX"
#define TXT_ARTIPUZZBOOK2			"LIBER OSCURA"
#define TXT_ARTIPUZZSKULL2			"FLAME MASK"
#define TXT_ARTIPUZZFWEAPON			"GLAIVE SEAL"
#define TXT_ARTIPUZZCWEAPON			"HOLY RELIC"
#define TXT_ARTIPUZZMWEAPON			"SIGIL OF THE MAGUS"
#define TXT_ARTIPUZZGEAR			"CLOCK GEAR"
#define TXT_USEPUZZLEFAILED			"YOU CANNOT USE THIS HERE"

// Items
#define TXT_ITEMHEALTH              "CRYSTAL VIAL"
#define TXT_ARMOR1                  "MESH ARMOR"
#define TXT_ARMOR2                  "FALCON SHIELD"
#define TXT_ARMOR3                  "PLATINUM HELMET"
#define TXT_ARMOR4                  "AMULET OF WARDING"

// Weapons
#define TXT_WEAPON_F2               "TIMON'S AXE"
#define TXT_WEAPON_F3               "HAMMER OF RETRIBUTION"
#define TXT_WEAPON_F4               "QUIETUS ASSEMBLED"
#define TXT_WEAPON_C2               "SERPENT STAFF"
#define TXT_WEAPON_C3               "FIRESTORM"
#define TXT_WEAPON_C4               "WRAITHVERGE ASSEMBLED"
#define TXT_WEAPON_M2               "FROST SHARDS"
#define TXT_WEAPON_M3               "ARC OF DEATH"
#define TXT_WEAPON_M4               "BLOODSCOURGE ASSEMBLED"
#define TXT_QUIETUS_PIECE           "SEGMENT OF QUIETUS"
#define TXT_WRAITHVERGE_PIECE       "SEGMENT OF WRAITHVERGE"
#define TXT_BLOODSCOURGE_PIECE      "SEGMENT OF BLOODSCOURGE"

//
// SB_bar.c
//

#define TXT_CHEATGODON              "GOD MODE ON"
#define TXT_CHEATGODOFF             "GOD MODE OFF"
#define TXT_CHEATNOCLIPON           "NO CLIPPING ON"
#define TXT_CHEATNOCLIPOFF          "NO CLIPPING OFF"
#define TXT_CHEATWEAPONS            "ALL WEAPONS"
#define TXT_CHEATHEALTH             "FULL HEALTH"
#define TXT_CHEATKEYS               "ALL KEYS"
#define TXT_CHEATSOUNDON            "SOUND DEBUG ON"
#define TXT_CHEATSOUNDOFF           "SOUND DEBUG OFF"
#define TXT_CHEATARTIFACTS3         "ALL ARTIFACTS"
#define TXT_CHEATARTIFACTSFAIL      "BAD INPUT"
#define TXT_CHEATWARP               "LEVEL WARP"
#define TXT_CHEATSCREENSHOT         "SCREENSHOT"
#define TXT_CHEATIDDQD              "TRYING TO CHEAT, EH?  NOW YOU DIE!"
#define TXT_CHEATIDKFA              "CHEATER - YOU DON'T DESERVE WEAPONS"
#define TXT_CHEATBADINPUT           "BAD INPUT"
#define TXT_CHEATNOMAP              "CAN'T FIND MAP"
#define TXT_SQUEAL                  "SQUEAL!!"
#define TXT_QUICKEN_1               "TRYING TO CHEAT?  THAT'S ONE...."
#define TXT_QUICKEN_2               "THAT'S TWO...."
#define TXT_QUICKEN_3               "THAT'S THREE!  TIME TO DIE."
#define TXT_NEW_CLASS               "ENTER NEW PLAYER CLASS (0 - 2)"
#define TXT_INVALID_CLASS           "INVALID PLAYER CLASS"
#define TXT_WHICH_SCRIPT            "RUN WHICH SCRIPT(01-99)?"
#define TXT_RUNNING_SCRIPT          "RUNNING SCRIPT"

// -----------------------------------------------------------------------------
// Русский язык
// -----------------------------------------------------------------------------

//
// AM_map.c
//

#define AMSTR_FOLLOWON_RUS      "HT;BV CKTLJDFYBZ DRK.XTY"  // РЕЖИМ СЛЕДОВАНИЯ ВКЛЮЧЕН
#define AMSTR_FOLLOWOFF_RUS     "HT;BV CKTLJDFYBZ JNRK.XTY" // РЕЖИМ СЛЕДОВАНИЯ ОТКЛЮЧЕН
#define AMSTR_GRIDON_RUS        "CTNRF DRK.XTYF"            // СЕТКА ВКЛЮЧЕНА
#define AMSTR_GRIDOFF_RUS       "CTNRF DSRK.XTYF"           // СЕТКА ВЫКЛЮЧЕНА
#define AMSTR_OVERLAYON_RUS     "HT;BV YFKJ;TYBZ DRK.XTY"   // РЕЖИМ НАЛОЖЕНИЯ ВКЛЮЧЕН
#define AMSTR_OVERLAYOFF_RUS    "HT;BV YFKJ;TYBZ DSRK.XTY"  // РЕЖИМ НАЛОЖЕНИЯ ВЫКЛЮЧЕН
#define AMSTR_ROTATEON_RUS      "HT;BV DHFOTYBZ DRK.XTY"    // РЕЖИМ ВРАЩЕНИЯ ВКЛЮЧЕН
#define AMSTR_ROTATEOFF_RUS     "HT;BV DHFOTYBZ DSRK.XTY"   // РЕЖИМ ВРАЩЕНИЯ ВЫКЛЮЧЕН
#define AMSTR_MARK_RUS          "JNVTNRF"                   // ОТМЕТКА
#define AMSTR_ADDED_RUS         "LJ,FDKTYF"                 // ДОБАВЛЕНА
#define AMSTR_CLEARED_RUS       "ELFKTYF"                   // УДАЛЕНА
#define AMSTR_MARKSCLEARED_RUS  "DCT JNVTNRB ELFKTYS"       // ВСЕ ОТМЕТКИ УДАЛЕНЫ

//
// CT_chat.c
//

#define TXT_NOPLAYERS_RUS       "D BUHT YTN LHEUB[ BUHJRJD!"  // В ИГРЕ НЕТ ДРУГИХ ИГРОКОВ!

//
// G_game.c
//

#define TXT_GAMESAVED_RUS       "BUHF CJ[HFYTYF"                 // ИГРА СОХРАНЕНА
#define TXT_GAMELOADED_RUS      "BUHF PFUHE;TYF"                 // ИГРА ЗАГРУЖЕНА
#define TXT_ALWAYSRUN_ON_RUS    "GJCNJZYYSQ ,TU DRK.XTY"         // ПОСТОЯННЫЙ БЕГ ВКЛЮЧЕН
#define TXT_ALWAYSRUN_OFF_RUS   "GJCNJZYYSQ ,TU DSRK.XTY"        // ПОСТОЯННЫЙ БЕГ ВЫКЛЮЧЕН
#define TXT_MLOOK_ON_RUS        "J,PJH VSIM. DRK.XTY"            // ОБЗОР МЫШЬЮ ВКЛЮЧЕН
#define TXT_MLOOK_OFF_RUS       "J,PJH VSIM. DSRK.XTY"           // ОБЗОР МЫШЬЮ ВЫКЛЮЧЕН
#define TXT_CROSSHAIR_ON_RUS    "GHBWTK DRK.XTY"                 // ПРИЦЕЛ ВКЛЮЧЕН
#define TXT_CROSSHAIR_OFF_RUS   "GHBWTK DSRK.XTY"                // ПРИЦЕЛ ВЫКЛЮЧЕН
#define TXT_TESTCONTROLS_RUS    "HT;BV GHJDTHRB EGHFDKTYBZ"      // РЕЖИМ ПРОВЕРКИ УПРАВЛЕНИЯ
#define TXT_ACCESS_DENIED_RUS   "LTVJDTHCBZ> GHJ[JL DJCGHTOTY>"  // ДЕМОВЕРСИЯ. ПРОХОД ВОСПРЕЩЕН.

//
// MN_menu.c
//

#define TXT_GAMMA_LEVEL_RUS             "EHJDTYM UFVVF-RJHHTRWBB "                      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ
#define TXT_MESSAGES_ON_RUS             "CJJ,OTYBZ DRK.XTYS"                            // СООБЩЕНИЯ ВКЛЮЧЕНЫ
#define TXT_MESSAGES_OFF_RUS            "CJJ,OTYBZ DSRK.XTYS"                           // СООБЩЕНИЯ ВЫКЛЮЧЕНЫ
#define TXT_SETTINGS_RESET_RUS          "YFCNHJQRB C,HJITYS"                            // НАСТРОЙКИ СБРОШЕНЫ
#define TXT_CANT_START_IN_NETGAME_RUS   "YTDJPVJ;YJ YFXFNM YJDE. BUHE D CTNTDJQ BUHT!"  // НЕВОЗМОЖНО НАЧАТЬ НОВУЮ ИГРУ В СЕТЕВОЙ ИГРЕ!
#define TXT_CANT_LOAD_IN_NETGAME_RUS    "YTDJPVJ;YJ PFUHEPBNMCZ D CTNTDJQ BUHT!"        // НЕВОЗМОЖНО ЗАГРУЗИТЬСЯ В СЕТЕВОЙ ИГРЕ!
#define TXT_CANT_END_IN_NETGAME_RUS     "YTDJPVJ;YJ PFRJYXBNM CTNTDE. BUHE!"            // НЕВОЗМОЖНО ЗАКОНЧИТЬ СЕТЕВУЮ ИГРУ!

#define RD_ON_RUS   "DRK"   // ВКЛ
#define RD_OFF_RUS  "DSRK"  // ВЫКЛ

//
// P_acs.c
//

#define TXT_UNKNOWN_SCRIPT_RUS  "JIB,RF DSGJKYTYBZ CRHBGNF"  // ОШИБКА ВЫПОЛНЕНИЯ СКРИПТА
#define TXT_YOU_NEED_THE_RUS    "LKZ JNRHSNBZ YE;TY"         // ДЛЯ ОТКРЫТИЯ НУЖЕН  

//
// P_inter.c
//

// Mana
#define TXT_MANA_1_RUS      "CBYZZ VFYF"            // СИНЯЯ МАНА
#define TXT_MANA_2_RUS      "PTKTYFZ VFYF"          // ЗЕЛЕНАЯ МАНА
#define TXT_MANA_BOTH_RUS   "RJV,BYBHJDFYYFZ VFYF"  // КОМБИНИРОВАННАЯ МАНА

// Keys
#define	TXT_KEY_STEEL_RUS   "CNFKMYJQ RK.X"         // СТАЛЬНОЙ КЛЮЧ
#define	TXT_KEY_CAVE_RUS    "GTOTHYSQ RK.X"         // ПЕЩЕРНЫЙ КЛЮЧ
#define	TXT_KEY_AXE_RUS     "RK.X-NJGJH"            // КЛЮЧ-ТОПОР
#define	TXT_KEY_FIRE_RUS    "JUYTYYSQ RK.X"         // ОГНЕННЫЙ КЛЮЧ
#define	TXT_KEY_EMERALD_RUS "BPEVHELYSQ RK.X"       // ИЗУМРУДНЫЙ КЛЮЧ
#define	TXT_KEY_DUNGEON_RUS "RK.X JN GJLPTVTKMZ"    // КЛЮЧ ОТ ПОДЗЕМЕЛЬЯ
#define	TXT_KEY_SILVER_RUS  "CTHT,HZYSQ RK.X"       // СЕРЕБРЯНЫЙ КЛЮЧ
#define	TXT_KEY_RUSTED_RUS  "H;FDSQ RK.X"           // РЖАВЫЙ КЛЮЧ
#define	TXT_KEY_HORN_RUS    "HJUJDJQ RK.X"          // РОГОВОЙ КЛЮЧ
#define	TXT_KEY_SWAMP_RUS   ",JKJNYSQ RK.X"         // БОЛОТНЫЙ КЛЮЧ
#define TXT_KEY_CASTLE_RUS  "RK.X JN PFVRF"         // КЛЮЧ ОТ ЗАМКА

// Artifacts
#define TXT_ARTIINVULNERABILITY_RUS "CBVDJK PFOBNYBRF"           // СИМВОЛ ЗАЩИТНИКА
#define TXT_ARTIHEALTH_RUS          "RDFHWTDSQ AKFRJY"           // КВАРЦЕВЫЙ ФЛАКОН
#define TXT_ARTISUPERHEALTH_RUS     "VBCNBXTCRFZ EHYF"           // МИСТИЧЕСКАЯ УРНА
#define TXT_ARTISUMMON_RUS          "NTVYSQ CKEUF"               // ТЕМНЫЙ СЛУГА
#define TXT_ARTITORCH_RUS           "AFRTK"                      // ФАКЕЛ
#define TXT_ARTIEGG_RUS             "CDBYJVJHATH"                // СВИНОМОРФЕР
#define TXT_ARTIFLY_RUS             "RHSKMZ UYTDF"               // КРЫЛЬЯ ГНЕВА
#define TXT_ARTITELEPORT_RUS        "'V,KTVF [FJCF"              // ЭМБЛЕМА ХАОСА
#define TXT_ARTIPOISONBAG_RUS       "PTKMT"                      // ЗЕЛЬЕ
#define TXT_ARTITELEPORTOTHER_RUS   "'V,KTVF BPUYFYBZ"           // ЭМБЛЕМА ИЗГНАНИЯ
#define TXT_ARTISPEED_RUS           "CFGJUB-CRJHJ[JLS"           // САПОГИ-СКОРОХОДЫ
#define TXT_ARTIBOOSTMANA_RUS       "XFIF VJUEOTCNDF"            // ЧАША МОГУЩЕСТВА
#define TXT_ARTIBOOSTARMOR_RUS      "YFHEXB BP LHFRJYMTQ RJ;B"   // НАРУЧИ ИЗ ДРАКОНЬЕЙ КОЖИ
#define TXT_ARTIBLASTRADIUS_RUS     "LBCR JNNJH;TYBZ"            // ДИСК ОТТОРЖЕНИЯ
#define TXT_ARTIHEALINGRADIUS_RUS   "XFHS VFUBXTCRJUJ TLBYCNDF"  // ЧАРЫ МАГИЧЕСКОГО ЕДИНСТВА

// Puzzle artifacts
#define TXT_ARTIPUZZSKULL_RUS       "XTHTG QJHBRF"                       // ЧЕРЕП ЙОРИКА
#define TXT_ARTIPUZZGEMBIG_RUS      "CTHLWT L&CGFHBKF"                   // СЕРДЦЕ Д'СПАРИЛА
#define TXT_ARTIPUZZGEMRED_RUS      "HE,BYJDFZ GKFYTNF"                  // РУБИНОВАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMGREEN1_RUS   "BPEVHELYFZ GKFYTNF"                 // ИЗУМРУДНАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMGREEN2_RUS   "BPEVHELYFZ GKFYTNF"                 // ИЗУМРУДНАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMBLUE1_RUS    "CFGABHJDFZ GKFYTNF"                 // САПФИРОВАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMBLUE2_RUS    "CFGABHJDFZ GKFYTNF"                 // САПФИРОВАЯ ПЛАНЕТА
#define TXT_ARTIPUZZBOOK1_RUS       "RJLTRC LTVJYF"                      // КОДЕКС ДЕМОНА
#define TXT_ARTIPUZZBOOK2_RUS       "$"                                  // LIBER OSCURA (не переводить, спец. символ "$")
#define TXT_ARTIPUZZSKULL2_RUS      "VFCRF GKFVTYB"                      // МАСКА ПЛАМЕНИ
#define TXT_ARTIPUZZFWEAPON_RUS     "GTXFNM DJBNTKZ"                     // ПЕЧАТЬ ВОИТЕЛЯ
#define TXT_ARTIPUZZCWEAPON_RUS     "CDZNFZ HTKBRDBZ"                    // СВЯТАЯ РЕЛИКВИЯ
#define TXT_ARTIPUZZMWEAPON_RUS     "CBVDJK VFUF"                        // СИМВОЛ МАГА
#define TXT_ARTIPUZZGEAR_RUS        "XFCJDFZ ITCNTHYZ"                   // ЧАСОВАЯ ШЕСТЕРНЯ
#define TXT_USEPUZZLEFAILED_RUS     "PLTCM 'NJ YTDJPVJ;YJ BCGJKMPJDFNM"  // ЗДЕСЬ ЭТО НЕВОЗМОЖНО ИСПОЛЬЗОВАТЬ

// Items
#define TXT_ITEMHEALTH_RUS  "RHBCNFKMYSQ AKFRJY"  // КРИСТАЛЬНЫЙ ФЛАКОН
#define TXT_ARMOR1_RUS      "RJKMXEUF"            // КОЛЬЧУГА
#define TXT_ARMOR2_RUS      "CJRJKBYSQ OBN"       // СОКОЛИНЫЙ ЩИТ
#define TXT_ARMOR3_RUS      "GKFNBYJDSQ IKTV"     // ПЛАТИНОВЫЙ ШЛЕМ
#define TXT_ARMOR4_RUS      "FVEKTN CNHF;F"       // АМУЛЕТ СТРАЖА

// Weapons

#define TXT_WEAPON_F2_RUS           "NJGJH NBVJYF"                    // ТОПОР ТИМОНА
#define TXT_WEAPON_F3_RUS           "VJKJN DJPVTPLBZ"                 // МОЛОТ ВОЗМЕЗДИЯ
#define TXT_WEAPON_F4_RUS           "GJCKTLYBQ LJDJL CJ,HFY DJTLBYJ"  // ПОСЛЕДНИЙ ДОВОД СОБРАН ВОЕДИНО
#define TXT_WEAPON_C2_RUS           "PVTBYSQ GJCJ["                   // ЗМЕИНЫЙ ПОСОХ
#define TXT_WEAPON_C3_RUS           "JUYTYYSQ INJHV"                  // ОГНЕННЫЙ ШТОРМ
#define TXT_WEAPON_C4_RUS           ";TPK LE[JD CJ,HFY DJTLBYJ"       // ЖЕЗЛ ДУХОВ СОБРАН ВОЕДИНО
#define TXT_WEAPON_M2_RUS           "KTLZYST JCRJKRB"                 // ЛЕДЯНЫЕ ОСКОЛКИ
#define TXT_WEAPON_M3_RUS           "LEUF CVTHNB"                     // ДУГА СМЕРТИ
#define TXT_WEAPON_M4_RUS           "RHJDFDSQ ,BX CJ,HFY DJTLBYJ"     // КРОВАВЫЙ БИЧ СОБРАН ВОЕДИНО
#define TXT_QUIETUS_PIECE_RUS       "XFCNM GJCKTLYTUJ LJDJLF"         // ЧАСТЬ ПОСЛЕДНЕГО ДОВОДА
#define TXT_WRAITHVERGE_PIECE_RUS   "XFCNM ;TPKF LE[JD"               // ЧАСТЬ ЖЕЗЛА ДУХОВ
#define TXT_BLOODSCOURGE_PIECE_RUS  "XFCNM RHJDFDJUJ ,BXF"            // ЧАСТЬ КРОВАВОГО БИЧА

//
// SB_bar.c
//

#define TXT_CHEATGODON_RUS          "HT;BV ,JUF"                              // РЕЖИМ БОГА
#define TXT_CHEATGODOFF_RUS         "HT;BV ,JUF JNVTYTY"                      // РЕЖИМ БОГА ОТМЕНЕН
#define TXT_CHEATNOCLIPON_RUS       "GHJ[J;LTYBT XTHTP CNTYS"                 // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ
#define TXT_CHEATNOCLIPOFF_RUS      "GHJ[J;LTYBT XTHTP CNTYS JNVTYTYJ"        // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ ОТМЕНЕНО
#define TXT_CHEATWEAPONS_RUS        "GJKYJT DJJHE;TYBT"                       // ПОЛНОЕ ВООРУЖЕНИЕ
#define TXT_CHEATHEALTH_RUS         "PLJHJDMT DJCCNFYJDKTYJ"                  // ЗДОРОВЬЕ ВОССТАНОВЛЕНО
#define TXT_CHEATKEYS_RUS           "DCT RK.XB"                               // ВСЕ КЛЮЧИ
#define TXT_CHEATSOUNDON_RUS        "JNKFLRF PDERF DRK.XTYF"                  // ОТЛАДКА ЗВУКА ВКЛЮЧЕНА
#define TXT_CHEATSOUNDOFF_RUS       "JNKFLRF PDERF JNRK.XTYF"                 // ОТЛАДКА ЗВУКА ОТКЛЮЧЕНА
#define TXT_CHEATARTIFACTS3_RUS     "DCT FHNTAFRNS"                           // ВСЕ АРТЕФАКТЫ
#define TXT_CHEATARTIFACTSFAIL_RUS  "YTRJHHTRNYSQ DDJL"                       // НЕКОРРЕНТНЫЙ ВВОД
#define TXT_CHEATWARP_RUS           "GTHTVTOTYBT YF EHJDTYM"                  // ПЕРЕМЕЩЕНИЕ НА УРОВЕНЬ
#define TXT_CHEATSCREENSHOT_RUS     "CYBVJR 'RHFYF"                           // СНИМОК ЭКРАНА
#define TXT_CHEATIDDQD_RUS          "GSNFTIMCZ C[BNHBNM? NFR EVHB ;T!"        // ПЫТАЕШЬСЯ СХИТРИТЬ? ТАК УМРИ ЖЕ!
#define TXT_CHEATIDKFA_RUS          ";EKBR< NS YT LJCNJBY CDJTUJ JHE;BZ"      // ЖУЛИК, ТЫ НЕ ДОСТОИН СВОЕГО ОРУЖИЯ
#define TXT_CHEATBADINPUT_RUS       "YTRJHHTRNYSQ DDJL"                       // НЕКОРРЕНТНЫЙ ВВОД
#define TXT_CHEATNOMAP_RUS          "EHJDTYM YT YFQLTY"                       // УРОВЕНЬ НЕ НАЙДЕН
#define TXT_SQUEAL_RUS              "[H.-[H.!"                                // ХРЮ-ХРЮ!
#define TXT_QUICKEN_1_RUS           "GSNFTIMCZ C[BNHBNM? 'NJ HFP>>>"          // ПЫТАЕШЬСЯ СХИТРИТЬ? ЭТО РАЗ...
#define TXT_QUICKEN_2_RUS           "'NJ LDF>>>"                              // ЭТО ДВА...
#define TXT_QUICKEN_3_RUS           "'NJ NHB> DHTVZ EVBHFNM!"                 // ЭТО ТРИ. ВРЕМЯ УМИРАТЬ!
#define TXT_NEW_CLASS_RUS           "DS,THBNT YJDSQ RKFCC GTHCJYF;F (0 - 2)"  // ВЫБЕРИТЕ НОВЫЙ КЛАСС ПЕРСОНАЖА (0 - 2)
#define TXT_INVALID_CLASS_RUS       "ERFPFY YTRJHHTRNYSQ RKFCC"               // УКАЗАН НЕКОРРЕКТНЫЙ КЛАСС
#define TXT_WHICH_SCRIPT_RUS        "YJVTH DSGJKYZTVJUJ CRHBGNF (01-99)?"     // НОМЕР ВЫПОЛНЯЕМОГО СКРИПТА (01-99)?
#define TXT_RUNNING_SCRIPT_RUS      "DSGJKYTYBT CRHBGNF"                      // ВЫПОЛНЕНИЕ СКРИПТА
