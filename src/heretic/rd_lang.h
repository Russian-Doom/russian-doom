//
// Copyright(C) 1993-1996 Id Software, Inc.
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

//
// Памятка по символам:
// --------------------
// < = ,
// > = .
// $ = BFG
// # = y 
// @ = n
// ^ = "


#ifndef __RD_LANG__
#define __RD_LANG__


extern void RD_DefineLanguageStrings ();

// Keys
extern char* txt_gotbluekey;
extern char* txt_gotyellowkey;
extern char* txt_gotgreenkey;

// Artifacts
extern char* txt_artihealth;
extern char* txt_artifly;
extern char* txt_artiinvulnerability;
extern char* txt_artitomeofpower;
extern char* txt_artiinvisibility;
extern char* txt_artiegg;
extern char* txt_artisuperhealth;
extern char* txt_artitorch;
extern char* txt_artifirebomb;
extern char* txt_artiteleport;

// Items
extern char* txt_itemhealth;
extern char* txt_itembagofholding;
extern char* txt_itemshield1;
extern char* txt_itemshield2;
extern char* txt_itemsupermap;

// Ammo
extern char* txt_ammogoldwand1;
extern char* txt_ammogoldwand2;
extern char* txt_ammomace1;
extern char* txt_ammomace2;
extern char* txt_ammocrossbow1;
extern char* txt_ammocrossbow2;
extern char* txt_ammoblaster1;
extern char* txt_ammoblaster2;
extern char* txt_ammoskullrod1;
extern char* txt_ammoskullrod2;
extern char* txt_ammophoenixrod1;
extern char* txt_ammophoenixrod2;

// Weapons
extern char* txt_wpnmace;
extern char* txt_wpncrossbow;
extern char* txt_wpnblaster;
extern char* txt_wpnskullrod;
extern char* txt_wpnphoenixrod;
extern char* txt_wpngauntlets;

// CT_chat.c
extern char* txt_msgsent;
extern char* txt_noplayers;

// SB_bar.c
extern char* txt_cheatgodon;
extern char* txt_cheatgodoff;
extern char* txt_cheatnoclipon;
extern char* txt_cheatnoclipoff;
extern char* txt_cheatweapons;
extern char* txt_cheatflighton;
extern char* txt_cheatflightoff;
extern char* txt_cheatpoweron;
extern char* txt_cheatpoweroff;
extern char* txt_cheathealth;
extern char* txt_cheatkeys;
extern char* txt_cheatsoundon;
extern char* txt_cheatsoundoff;
extern char* txt_cheattickeron;
extern char* txt_cheattickeroff;
extern char* txt_cheatartifacts1;
extern char* txt_cheatartifacts2;
extern char* txt_cheatartifacts3;
extern char* txt_cheatartifactsfail;
extern char* txt_cheatwarp;
extern char* txt_cheatscreenshot;
extern char* txt_cheatchickenon;
extern char* txt_cheatchickenoff;
extern char* txt_cheatmassacre;
extern char* txt_cheatiddqd;
extern char* txt_cheatidkfa;
extern char* txt_cheatmus;
extern char* txt_cheatmus_e;
extern char* txt_cheatmus_m;
extern char* txt_cheatnomus;

// P_doors.c
extern char* txt_needbluekey;
extern char* txt_needgreenkey;
extern char* txt_needyellowkey;

// P_spec.c
extern char* txt_secret_found;

// G_game.c
extern char* txt_gamesaved;
extern char* txt_testcontrols;

// AM_map.c
extern char* LevelNames[];
extern char *LevelNames_Rus[];
extern char* amstr_followon;
extern char* amstr_followoff;
extern char* amstr_gridon;
extern char* amstr_gridoff;
extern char* amstr_overlayon;
extern char* amstr_overlayoff;
extern char* amstr_rotateon;
extern char* amstr_rotateoff;
extern char* amstr_mark;
extern char* amstr_added;
extern char* amstr_cleared;
extern char* amstr_markscleared;

// MN_menu.c
extern char* txt_gammamsg;
extern char* txt_detail_high;
extern char* txt_detail_low;
extern char* txt_messages_on;
extern char* txt_messages_off;
extern char* txt_settings_reset;
extern char* txt_cant_start_in_netgame;
extern char* txt_cant_load_in_netgame;
extern char* txt_registered_only;
extern char* txt_quicksaving;
extern char* txt_quickloading;
extern char* txt_choose_qsave;
extern char* txt_choose_qload;

// Port-specific strings
extern char* txt_alwaysrun_on;
extern char* txt_alwaysrun_off;
extern char* txt_crosshair_on;
extern char* txt_crosshair_off;
extern char* txt_mlook_on;
extern char* txt_mlook_off;


// -----------------------------------------------------------------------------
// English language
// -----------------------------------------------------------------------------

// Keys
#define TXT_GOTBLUEKEY      "BLUE KEY"
#define TXT_GOTYELLOWKEY    "YELLOW KEY"
#define TXT_GOTGREENKEY     "GREEN KEY"

// Artifacts
#define TXT_ARTIHEALTH          "QUARTZ FLASK"
#define TXT_ARTIFLY             "WINGS OF WRATH"
#define TXT_ARTIINVULNERABILITY "RING OF INVINCIBILITY"
#define TXT_ARTITOMEOFPOWER     "TOME OF POWER"
#define TXT_ARTIINVISIBILITY    "SHADOWSPHERE"
#define TXT_ARTIEGG             "MORPH OVUM"
#define TXT_ARTISUPERHEALTH     "MYSTIC URN"
#define TXT_ARTITORCH           "TORCH"
#define TXT_ARTIFIREBOMB        "TIME BOMB OF THE ANCIENTS"
#define TXT_ARTITELEPORT        "CHAOS DEVICE"

// Items
#define TXT_ITEMHEALTH          "CRYSTAL VIAL"
#define TXT_ITEMBAGOFHOLDING    "BAG OF HOLDING"
#define TXT_ITEMSHIELD1         "SILVER SHIELD"
#define TXT_ITEMSHIELD2         "ENCHANTED SHIELD"
#define TXT_ITEMSUPERMAP        "MAP SCROLL"

// Ammo
#define TXT_AMMOGOLDWAND1       "WAND CRYSTAL"
#define TXT_AMMOGOLDWAND2       "CRYSTAL GEODE"
#define TXT_AMMOMACE1           "MACE SPHERES"
#define TXT_AMMOMACE2           "PILE OF MACE SPHERES"
#define TXT_AMMOCROSSBOW1       "ETHEREAL ARROWS"
#define TXT_AMMOCROSSBOW2       "QUIVER OF ETHEREAL ARROWS"
#define TXT_AMMOBLASTER1        "CLAW ORB"
#define TXT_AMMOBLASTER2        "ENERGY ORB"
#define TXT_AMMOSKULLROD1       "LESSER RUNES"
#define TXT_AMMOSKULLROD2       "GREATER RUNES"
#define TXT_AMMOPHOENIXROD1     "FLAME ORB"
#define TXT_AMMOPHOENIXROD2     "INFERNO ORB"

// Weapons
#define TXT_WPNMACE             "FIREMACE"
#define TXT_WPNCROSSBOW         "ETHEREAL CROSSBOW"
#define TXT_WPNBLASTER          "DRAGON CLAW"
#define TXT_WPNSKULLROD         "HELLSTAFF"
#define TXT_WPNPHOENIXROD       "PHOENIX ROD"
#define TXT_WPNGAUNTLETS        "GAUNTLETS OF THE NECROMANCER"

// CT_chat.c
#define TXT_MSGSENT             "-MESSAGE SENT-"
#define TXT_NOPLAYERS           "THERE ARE NO OTHER PLAYERS IN THE GAME!"

// SB_bar.c
#define TXT_CHEATGODON          "GOD MODE ON"
#define TXT_CHEATGODOFF         "GOD MODE OFF"
#define TXT_CHEATNOCLIPON       "NO CLIPPING ON"
#define TXT_CHEATNOCLIPOFF      "NO CLIPPING OFF"
#define TXT_CHEATWEAPONS        "ALL WEAPONS"
#define TXT_CHEATFLIGHTON       "FLIGHT ON"
#define TXT_CHEATFLIGHTOFF      "FLIGHT OFF"
#define TXT_CHEATPOWERON        "POWER ON"
#define TXT_CHEATPOWEROFF       "POWER OFF"
#define TXT_CHEATHEALTH         "FULL HEALTH"
#define TXT_CHEATKEYS           "ALL KEYS"
#define TXT_CHEATSOUNDON        "SOUND DEBUG ON"
#define TXT_CHEATSOUNDOFF       "SOUND DEBUG OFF"
#define TXT_CHEATTICKERON       "TICKER ON"
#define TXT_CHEATTICKEROFF      "TICKER OFF"
#define TXT_CHEATARTIFACTS1     "CHOOSE AN ARTIFACT ( A - J )"
#define TXT_CHEATARTIFACTS2     "HOW MANY ( 1 - 9 )"
#define TXT_CHEATARTIFACTS3     "YOU GOT IT"
#define TXT_CHEATARTIFACTSFAIL  "BAD INPUT"
#define TXT_CHEATWARP           "LEVEL WARP"
#define TXT_CHEATSCREENSHOT     "SCREENSHOT"
#define TXT_CHEATCHICKENON      "CHICKEN ON"
#define TXT_CHEATCHICKENOFF     "CHICKEN OFF"
#define TXT_CHEATMASSACRE       "MASSACRE"
#define TXT_CHEATIDDQD          "TRYING TO CHEAT, EH?  NOW YOU DIE!"
#define TXT_CHEATIDKFA          "CHEATER - YOU DON'T DESERVE WEAPONS"
#define TXT_CHEATMUS            "MUSIC CHANGED TO"
#define TXT_CHEATMUS_E          "E"
#define TXT_CHEATMUS_M          "M"
#define TXT_CHEATNOMUS          "IMPOSSIBLE SELECTION"

// P_doors.c
#define TXT_NEEDBLUEKEY         "YOU NEED A BLUE KEY TO OPEN THIS DOOR"
#define TXT_NEEDGREENKEY        "YOU NEED A GREEN KEY TO OPEN THIS DOOR"
#define TXT_NEEDYELLOWKEY       "YOU NEED A YELLOW KEY TO OPEN THIS DOOR"

// P_spec.c
#define TXT_SECRET_FOUND        "A SECRET IS REVEALED!"

// G_game.c
#define TXT_GAMESAVED           "GAME SAVED"
#define TXT_TESTCONTROLS        "PRESS ESCAPE TO QUIT"

// AM_map.c
#define AMSTR_FOLLOWON          "FOLLOW MODE ON"
#define AMSTR_FOLLOWOFF         "FOLLOW MODE OFF"
#define AMSTR_GRIDON            "GRID ON"
#define AMSTR_GRIDOFF           "GRID OFF"
#define AMSTR_OVERLAYON         "OVERLAY MODE ON"
#define AMSTR_OVERLAYOFF        "OVERLAY MODE OFF"
#define AMSTR_ROTATEON          "ROTATE MODE ON"
#define AMSTR_ROTATEOFF         "ROTATE MODE OFF"
#define AMSTR_MARKEDSPOT        "Marked Spot"
#define AMSTR_MARK              "Mark"
#define AMSTR_ADDED             "Added"
#define AMSTR_CLEARED           "Cleared"
#define AMSTR_MARKSCLEARED      "All Marks Cleared"

// MN_menu.c
#define TXT_GAMMA_LEVEL         "GAMMA CORRECTION LEVEL "
#define TXT_GAMMA_DETAIL_HIGH   "HIGH DETAIL"
#define TXT_GAMMA_DETAIL_LOW    "LOW DETAIL"
#define TXT_MESSAGES_ON         "MESSAGES ON"
#define TXT_MESSAGES_OFF        "MESSAGES OFF"
#define TXT_SETTINGS_RESET      "SETTINGS RESET"
#define TXT_CANT_START_IN_NETGAME "YOU CAN'T START A NEW GAME IN NETPLAY!"
#define TXT_CANT_LOAD_IN_NETGAME  "YOU CAN'T LOAD A GAME IN NETPLAY!"
#define TXT_REGISTERED_ONLY     "ONLY AVAILABLE IN THE REGISTERED VERSION"
#define TXT_QUICKSAVING         "QUICKSAVING...."
#define TXT_QUICKLOADING        "QUICKLOADING...."
#define TXT_CHOOSE_QSAVE        "CHOOSE A QUICKSAVE SLOT"
#define TXT_CHOOSE_QLOAD        "CHOOSE A QUICKLOAD SLOT"

// F_finale.c
#define E1TEXT  "with the destruction of the iron\n"\
                "liches and their minions, the last\n"\
                "of the undead are cleared from this\n"\
                "plane of existence.\n\n"\
                "those creatures had to come from\n"\
                "somewhere, though, and you have the\n"\
                "sneaky suspicion that the fiery\n"\
                "portal of hell's maw opens onto\n"\
                "their home dimension.\n\n"\
                "to make sure that more undead\n"\
                "(or even worse things) don't come\n"\
                "through, you'll have to seal hell's\n"\
                "maw from the other side. of course\n"\
                "this means you may get stuck in a\n"\
                "very unfriendly world, but no one\n"\
                "ever said being a Heretic was easy!"

#define E2TEXT  "the mighty maulotaurs have proved\n"\
                "to be no match for you, and as\n"\
                "their steaming corpses slide to the\n"\
                "ground you feel a sense of grim\n"\
                "satisfaction that they have been\n"\
                "destroyed.\n\n"\
                "the gateways which they guarded\n"\
                "have opened, revealing what you\n"\
                "hope is the way home. but as you\n"\
                "step through, mocking laughter\n"\
                "rings in your ears.\n\n"\
                "was some other force controlling\n"\
                "the maulotaurs? could there be even\n"\
                "more horrific beings through this\n"\
                "gate? the sweep of a crystal dome\n"\
                "overhead where the sky should be is\n"\
                "certainly not a good sign...."

#define E3TEXT  "the death of d'sparil has loosed\n"\
                "the magical bonds holding his\n"\
                "creatures on this plane, their\n"\
                "dying screams overwhelming his own\n"\
                "cries of agony.\n\n"\
                "your oath of vengeance fulfilled,\n"\
                "you enter the portal to your own\n"\
                "world, mere moments before the dome\n"\
                "shatters into a million pieces.\n\n"\
                "but if d'sparil's power is broken\n"\
                "forever, why don't you feel safe?\n"\
                "was it that last shout just before\n"\
                "his death, the one that sounded\n"\
                "like a curse? or a summoning? you\n"\
                "can't really be sure, but it might\n"\
                "just have been a scream.\n\n"\
                "then again, what about the other\n"\
                "serpent riders?"

#define E4TEXT  "you thought you would return to your\n"\
                "own world after d'sparil died, but\n"\
                "his final act banished you to his\n"\
                "own plane. here you entered the\n"\
                "shattered remnants of lands\n"\
                "conquered by d'sparil. you defeated\n"\
                "the last guardians of these lands,\n"\
                "but now you stand before the gates\n"\
                "to d'sparil's stronghold. until this\n"\
                "moment you had no doubts about your\n"\
                "ability to face anything you might\n"\
                "encounter, but beyond this portal\n"\
                "lies the very heart of the evil\n"\
                "which invaded your world. d'sparil\n"\
                "might be dead, but the pit where he\n"\
                "was spawned remains. now you must\n"\
                "enter that pit in the hopes of\n"\
                "finding a way out. and somewhere,\n"\
                "in the darkest corner of d'sparil's\n"\
                "demesne, his personal bodyguards\n"\
                "await your arrival ..."

#define E5TEXT  "as the final maulotaur bellows his\n"\
                "death-agony, you realize that you\n"\
                "have never come so close to your own\n"\
                "destruction. not even the fight with\n"\
                "d'sparil and his disciples had been\n"\
                "this desperate. grimly you stare at\n"\
                "the gates which open before you,\n"\
                "wondering if they lead home, or if\n"\
                "they open onto some undreamed-of\n"\
                "horror. you find yourself wondering\n"\
                "if you have the strength to go on,\n"\
                "if nothing but death and pain await\n"\
                "you. but what else can you do, if\n"\
                "the will to fight is gone? can you\n"\
                "force yourself to continue in the\n"\
                "face of such despair? do you have\n"\
                "the courage? you find, in the end,\n"\
                "that it is not within you to\n"\
                "surrender without a fight. eyes\n"\
                "wide, you go to meet your fate."

// Port-specific strings
#define TXT_ALWAYSRUN_ON    "ALWAYS RUN ON"
#define TXT_ALWAYSRUN_OFF   "ALWAYS RUN OFF"
#define TXT_MLOOK_ON        "MOUSE LOOK ON"
#define TXT_MLOOK_OFF       "MOUSE LOOK OFF"
#define TXT_CROSSHAIR_ON    "CROSSHAIR ON"
#define TXT_CROSSHAIR_OFF   "CROSSHAIR OFF"
#define RD_ON               "ON"
#define RD_OFF              "OFF"

// -----------------------------------------------------------------------------
// Русский язык
// -----------------------------------------------------------------------------

// Keys
#define TXT_GOTBLUEKEY_RUS      "CBYBQ RK.X"    // СИНИЙ КЛЮЧ
#define TXT_GOTYELLOWKEY_RUS    ";TKNSQ RK.X"   // ЖЕЛТЫЙ КЛЮЧ
#define TXT_GOTGREENKEY_RUS     "PTKTYSQ RK.X"  // ЗЕЛЕНЫЙ КЛЮЧ

// Artifacts
#define TXT_ARTIHEALTH_RUS          "RDFHWTDSQ AKFRJY"      // КВАРЦЕВЫЙ ФЛАКОН
#define TXT_ARTIFLY_RUS             "RHSKMZ UYTDF"          // КРЫЛЬЯ ГНЕВА
#define TXT_ARTIINVULNERABILITY_RUS "RJKMWJ YTEZPDBVJCNB"   // КОЛЬЦО НЕУЯЗВИМОСТИ
#define TXT_ARTITOMEOFPOWER_RUS     "NJV VJUEOTCNDF"        // ТОМ МОГУЩЕСТВА
#define TXT_ARTIINVISIBILITY_RUS    "NTYTDFZ CATHF"         // ТЕНЕВАЯ СФЕРА
#define TXT_ARTIEGG_RUS             "ZQWJ GHTDHFOTYBQ"      // ЯЙЦО ПРЕВРАЩЕНИЙ
#define TXT_ARTISUPERHEALTH_RUS     "VBCNBXTCRFZ EHYF"      // МИСТИЧЕСКАЯ УРНА
#define TXT_ARTITORCH_RUS           "AFRTK"                 // ФАКЕЛ
#define TXT_ARTIFIREBOMB_RUS        "XFCJDFZ ,JV,F LHTDYB[" // ЧАСОВАЯ БОМБА ДРЕВНИХ
#define TXT_ARTITELEPORT_RUS        "'V,KTVF [FJCF"         // ЭМБЛЕМА ХАОСА

// Items
#define TXT_ITEMHEALTH_RUS          "RHBCNFKMYSQ AKFRJY"    // КРИСТАЛЬНЫЙ ФЛАКОН
#define TXT_ITEMBAGOFHOLDING_RUS    "YJCBKMYSQ RJITKM"      // НОСИЛЬНЫЙ КОШЕЛЬ
#define TXT_ITEMSHIELD1_RUS         "CTHT,HZYSQ OBN"        // СЕРЕБРЯНЫЙ ЩИТ
#define TXT_ITEMSHIELD2_RUS         "PFXFHJDFYYSQ OBN"      // ЗАЧАРОВАННЫЙ ЩИТ
#define TXT_ITEMSUPERMAP_RUS        "CDBNJR RFHNS"          // СВИТОК КАРТЫ

// Ammo
#define TXT_AMMOGOLDWAND1_RUS       "RHBCNFKK LKZ 'KMABQCRJUJ ;TPKF"    // КРИСТАЛЛ ДЛЯ ЭЛЬФИЙСКОГО ЖЕЗЛА
#define TXT_AMMOGOLDWAND2_RUS       ";TJLF RHBCNFKKF"       // ЖЕОДА КРИСТАЛЛА
#define TXT_AMMOMACE1_RUS           "CATHS LKZ ,EKFDS"      // СФЕРЫ ДЛЯ БУЛАВЫ
#define TXT_AMMOMACE2_RUS           "UHELF CATH LKZ ,EKFDS" // ГРУДА СФЕР ДЛЯ БУЛАВЫ
#define TXT_AMMOCROSSBOW1_RUS       "'ABHYST CNHTKS"        // ЭФИРНЫЕ СТРЕЛЫ
#define TXT_AMMOCROSSBOW2_RUS       "RJKXFY 'ABHYS[ CNHTK"  // КОЛЧАН ЭФИРНЫХ СТРЕЛ
#define TXT_AMMOBLASTER1_RUS        "RJUNTDJQ IFH"          // КОГТЕВОЙ ШАР
#define TXT_AMMOBLASTER2_RUS        "'YTHUTNBXTCRBQ IFH"    // ЭНЕРГЕТИЧЕСКИЙ ШАР
#define TXT_AMMOSKULLROD1_RUS       "VKFLIFZ HEYF"          // МЛАДШАЯ РУНА
#define TXT_AMMOSKULLROD2_RUS       "CNFHIFZ HEYF"          // СТАРШАЯ РУНЫ
#define TXT_AMMOPHOENIXROD1_RUS     "GKFVTYYSQ IFH"         // ПЛАМЕННЫЙ ШАР
#define TXT_AMMOPHOENIXROD2_RUS     "BYATHYFKMYSQ IFH"      // ИНФЕРНАЛЬНЫЙ ШАР

// Weapons
#define TXT_WPNMACE_RUS             "JUYTYYFZ ,EKFDF"       // ОГНЕННАЯ БУЛАВА
#define TXT_WPNCROSSBOW_RUS         "'ABHYSQ FH,FKTN"       // ЭФИРНЫЙ АРБАЛЕТ
#define TXT_WPNBLASTER_RUS          "RJUJNM LHFRJYF"        // КОГОТЬ ДРАКОНА
#define TXT_WPNSKULLROD_RUS         "GJCJ[ FLF"             // ПОСОХ АДА
#define TXT_WPNPHOENIXROD_RUS       ";TPK ATYBRCF"          // ЖЕЗЛ ФЕНИКСА
#define TXT_WPNGAUNTLETS_RUS        "GTHXFNRB YTRHJVFYNF"   // ПЕРЧАТКИ НЕКРОМАНТА

// CT_chat.c
#define TXT_MSGSENT_RUS             "-CJJ<OTYBT JNGHFDKTYJ-"     // -СООБЩЕНИЕ ОТПРАВЛЕНО-
#define TXT_NOPLAYERS_RUS           "D BUHT YTN LHEUB[ BUHJRJD!" // В ИГРЕ НЕТ ДРУГИХ ИГРОКОВ!

// SB_bar.c
#define TXT_CHEATGODON_RUS          "HT;BV ,JUF"                            // РЕЖИМ БОГА
#define TXT_CHEATGODOFF_RUS         "HT;BV ,JUF JNVTYTY"                    // РЕЖИМ БОГА ОТМЕНЕН
#define TXT_CHEATNOCLIPON_RUS       "GHJ[J;LTYBT XTHTP CNTYS"               // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ
#define TXT_CHEATNOCLIPOFF_RUS      "GHJ[J;LTYBT XTHTP CNTYS JNVTYTYJ"      // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ ОТМЕНЕНО
#define TXT_CHEATWEAPONS_RUS        "GJKYJT DJJHE;TYBT"                     // ПОЛНОЕ ВООРУЖЕНИЕ
#define TXT_CHEATFLIGHTON_RUS       "HT;BV GJKTNF"                          // РЕЖИМ ПОЛЕТА
#define TXT_CHEATFLIGHTOFF_RUS      "HT;BV GJKTNF JNVTYTY"                  // РЕЖИМ ПОЛЕТА ОТМЕНЕН
#define TXT_CHEATPOWERON_RUS        "VJUEOTCNDJ JHE;BZ"                     // МОГУЩЕСТВО ОРУЖИЯ
#define TXT_CHEATPOWEROFF_RUS       "VJUEOTCNDJ JHE;BZ JNVTYTYJ"            // МОГУЩЕСТВО ОРУЖИЯ ОТМЕНЕНО
#define TXT_CHEATHEALTH_RUS         "PLJHJDMT DJCCNFYJDKTYJ"                // ЗДОРОВЬЕ ВОССТАНОВЛЕНО
#define TXT_CHEATKEYS_RUS           "DCT RK.XB"                             // ВСЕ КЛЮЧИ
#define TXT_CHEATSOUNDON_RUS        "JNKFLRF PDERF DRK.XTYF"                // ОТЛАДКА ЗВУКА ВКЛЮЧЕНА
#define TXT_CHEATSOUNDOFF_RUS       "JNKFLRF PDERF JNRK.XTYF"               // ОТЛАДКА ЗВУКА ОТКЛЮЧЕНА
#define TXT_CHEATTICKERON_RUS       "CXTNXBR RFLHJDJQ XFCNJNS DRK.XTY"      // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ВКЛЮЧЕН
#define TXT_CHEATTICKEROFF_RUS      "CXTNXBR RFLHJDJQ XFCNJNS JNRK.XTY"     // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ОТКЛЮЧЕН
#define TXT_CHEATARTIFACTS1_RUS     "DS,THBNT FHNTAFRN ( F - J )"           // ВЫБЕРИТЕ АРТЕФАКТ ( А - O )
#define TXT_CHEATARTIFACTS2_RUS     "ERF;BNT RJKBXTCNDJ ( 1 - 9 )"          // УКАЖИТЕ КОЛИЧЕСТВО ( 1 - 9 )
#define TXT_CHEATARTIFACTS3_RUS     "GJKEXTYJ"                              // ПОЛУЧЕНО 
#define TXT_CHEATARTIFACTSFAIL_RUS  "YTRJHHTRNYSQ DDJL"                     // НЕКОРРЕНТНЫЙ ВВОД
#define TXT_CHEATWARP_RUS           "GTHTVTOTYBT YF EHJDTYM"                // ПЕРЕМЕЩЕНИЕ НА УРОВЕНЬ
#define TXT_CHEATSCREENSHOT_RUS     "CYBVJR 'RHFYF"                         // СНИМОК ЭКРАНА
#define TXT_CHEATCHICKENON_RUS      "GHTDHFOTYBT D WSGKTYRF"                // ПРЕВРАЩЕНИЕ В ЦЫПЛЕНКА
#define TXT_CHEATCHICKENOFF_RUS     "GHTDHFOTYBT D WSGKTYRF JNVTYTYJ"       // ПРЕВРАЩЕНИЕ В ЦЫПЛЕНКА ОТМЕНЕНО
#define TXT_CHEATMASSACRE_RUS       "DCT VJYCNHS EYBXNJ;TYS"                // ВСЕ МОНСТРЫ УНИЧТОЖЕНЫ
#define TXT_CHEATIDDQD_RUS          "GSNFTIMCZ C[BNHBNM? NFR EVHB ;T!"      // ПЫТАЕШЬСЯ СХИТРИТЬ? ТАК УМРИ ЖЕ!
#define TXT_CHEATIDKFA_RUS          ";EKBR< NS YT LJCNJBY CDJTUJ JHE;BZ"    // ЖУЛИК, ТЫ НЕ ДОСТОИН СВОЕГО ОРУЖИЯ
#define TXT_CHEATMUS_RUS            "VEPSRF BPVTYTYF YF"                    // МУЗЫКА ИЗМЕНЕНА НА
#define TXT_CHEATMUS_E_RUS          "T"                                     // E
#define TXT_CHEATMUS_M_RUS          "V"                                     // M
#define TXT_CHEATNOMUS_RUS          "YTRJHHTRNYSQ DS,JH"                    // НЕКОРРЕКТНЫЙ ВЫБОР

// P_doors.c
#define TXT_NEEDBLUEKEY_RUS     "LKZ JNRHSNBZ YE;TY CBYBQ RK.X"     // ДЛЯ ОТКРЫТИЯ НУЖЕН СИНИЙ КЛЮЧ
#define TXT_NEEDGREENKEY_RUS    "LKZ JNRHSNBZ YE;TY PTKTYSQ RK.X"   // ДЛЯ ОТКРЫТИЯ НУЖЕН ЗЕЛЕНЫЙ КЛЮЧ
#define TXT_NEEDYELLOWKEY_RUS   "LKZ JNRHSNBZ YE;TY ;TKNSQ RK.X"    // ДЛЯ ОТКРЫТИЯ НУЖЕН ЖЕЛТЫЙ КЛЮЧ

// P_spec.c
#define TXT_SECRET_FOUND_RUS    "J,YFHE;TY NFQYBR!"                 // ОБНАРУЖЕН ТАЙНИК!

// G_game.c
#define TXT_GAMESAVED_RUS       "BUHF CJ[HFYTYF"    // ИГРА СОХРАНЕНА
#define TXT_TESTCONTROLS_RUS    "HT;BV GHJDTHRB EGHFDKTYBZ"     // РЕЖИМ ПРОВЕРКИ УПРАВЛЕНИЯ

// AM_map.c
#define AMSTR_FOLLOWON_RUS      "HT;BV CKTLJDFYBZ DRK.XTY"      // РЕЖИМ СЛЕДОВАНИЯ ВКЛЮЧЕН
#define AMSTR_FOLLOWOFF_RUS     "HT;BV CKTLJDFYBZ DSRK.XTY"     // РЕЖИМ СЛЕДОВАНИЯ ВЫКЛЮЧЕН
#define AMSTR_GRIDON_RUS        "CTNRF DRK.XTYF"                // СЕТКА ВКЛЮЧЕНА
#define AMSTR_GRIDOFF_RUS       "CTNRF DSRK.XTYF"               // СЕТКА ВЫКЛЮЧЕНА
#define AMSTR_OVERLAYON_RUS     "HT;BV YFKJ;TYBZ DRK.XTY"       // РЕЖИМ НАЛОЖЕНИЯ ВКЛЮЧЕН
#define AMSTR_OVERLAYOFF_RUS    "HT;BV YFKJ;TYBZ DSRK.XTY"      // РЕЖИМ НАЛОЖЕНИЯ ВЫКЛЮЧЕН
#define AMSTR_ROTATEON_RUS      "HT;BV DHFOTYBZ DRK.XTY"        // РЕЖИМ ВРАЩЕНИЯ ВКЛЮЧЕН
#define AMSTR_ROTATEOFF_RUS     "HT;BV DHFOTYBZ DSRK.XTY"       // РЕЖИМ ВРАЩЕНИЯ ВЫКЛЮЧЕН
#define AMSTR_MARK_RUS          "JNVTNRF"                       // ОТМЕТКА
#define AMSTR_ADDED_RUS         "LJ,FDKTYF"                     // ДОБАВЛЕНА
#define AMSTR_CLEARED_RUS       "ELFKTYF"                       // УДАЛЕНА
#define AMSTR_MARKSCLEARED_RUS  "DCT JNVTNRB ELFKTYS"           // ВСЕ ОТМЕТКИ УДАЛЕНЫ

// MN_menu.c
#define TXT_GAMMA_LEVEL_RUS         "EHJDTYM UFVVF-RJHHTRWBB "  // Уровень гамма-коррекции
#define TXT_GAMMA_DETAIL_HIGH_RUS   "DSCJRFZ LTNFKBPFWBZ"   // ВЫСОКАЯ ДЕТАЛИЗАЦИЯ
#define TXT_GAMMA_DETAIL_LOW_RUS    "YBPRFZ LTNFKBPFWBZ"    // НИЗКАЯ ДЕТАЛИЗАЦИЯ
#define TXT_MESSAGES_ON_RUS         "CJJ,OTYBZ DRK.XTYS"    // СООБЩЕНИЯ ВКЛЮЧЕНЫ
#define TXT_MESSAGES_OFF_RUS        "CJJ,OTYBZ DSRK.XTYS"   // СООБЩЕНИЯ ВЫКЛЮЧЕНЫ
#define TXT_SETTINGS_RESET_RUS      "YFCNHJQRB C,HJITYS"    // НАСТРОЙКИ СБРОШЕНЫ
#define TXT_CANT_START_IN_NETGAME_RUS  "YTDJPVJ;YJ YFXFNM YJDE. BUHE D CTNTDJQ BUHT!" // НЕВОЗМОЖНО НАЧАТЬ НОВУЮ ИГРУ В СЕТЕВОЙ ИГРЕ!
#define TXT_CANT_LOAD_IN_NETGAME_RUS   "YTDJPVJ;YJ PFUHEPBNMCZ D CTNTDJQ BUHT!"       // НЕВОЗМОЖНО ЗАГРУЗИТЬСЯ В СЕТЕВОЙ ИГРЕ!
#define TXT_REGISTERED_ONLY_RUS     "'GBPJL YTLJCNEGTY D LTVJDTHCBB"    // ЭПИЗОД НЕДОСТУПЕН В ДЕМОВЕРСИИ
#define TXT_QUICKSAVING_RUS         ",SCNHJT CJ[HFYTYBT>>>"             // БЫСТРОЕ СОХРАНЕНИЕ...
#define TXT_QUICKLOADING_RUS        ",SCNHFZ PFUHEPRF>>>"               // БЫСТРАЯ ЗАГРУЗКА...
#define TXT_CHOOSE_QSAVE_RUS        "DS,THBNT CKJN ,SCNHJUJ CJ[HFYTYBZ" // ВЫБЕРИТЕ СЛОТ БЫСТРОГО СОХРАНЕНИЯ
#define TXT_CHOOSE_QLOAD_RUS        "DS,THBNT CKJN ,SCNHJQ PFUHEPRB"    // ВЫБЕРИТЕ СЛОТ БЫСТРОЙ ЗАГРУЗКИ

// F_finale.c
#define E1TEXT_RUS  "C EYBXNJ;TYBTV ;TKTPYS[ KBXTQ\n"\
                    "B B[ GHBCGTIYBRJD< JRHTCNYST PTVKB\n"\
                    "JXBCNBKBCM JN JVTHPBNTKMYJQ YT;BNB>\n"\
                    "\n"\
                    "'NF YT;BNM< GHJYBRIFZ D YFI VBH BP\n"\
                    "NTVYJUJ BPVTHTYBZ< JNRHSKF JUYTYYSQ\n"\
                    "GJHNFK> JY RFR JNDTHCNFZ GSKF.OFZ\n"\
                    "GFCNM FLF DTLTN D TUJ ;ENRE. ENHJ,E>\n"\
                    "\n"\
                    "EUHJPF BC[JLBN BP JUYTYYJUJ GJHNFKF -\n"\
                    "GJHJ;LTYBZ FLF B XTHYJQ VFUBB VJUEN\n"\
                    "BPYEHBNMCZ BP YTUJ> DFIF WTKM -\n"\
                    "CJQNB D FL B PFGTXFNFNM GHJRKZNSQ\n"\
                    "GJHNFK>\n"\
                    "\n"\
                    "'NJ CVTHNTKMYJ JGFCYJT LTZYBT< B DS\n"\
                    "HBCRETNT YFDCTULF EDZPYENM DJ NMVT>\n"\
                    "YJ RNJ UJDJHBK< XNJ GENM BCNBYYJUJ\n"\
                    "THTNBRF KTUJR B GHJCN?"

/*                   С уничтожением Железных Личей
**                   и их приспешников, окрестные земли 
**                   очистились от омерзительной нежити.
**
**                   Эта нежить, проникшая в наш мир из
**                   темного измерения, открыла Огненный
**                   Портал. Он как отверстая пылающая 
**                   пасть Ада ведет в его жуткую утробу.
**
**                   Угроза исходит из Огненного Портала -
**                   порождения Ада и черной магии могут
**                   изринуться из него. Ваша цель - 
**                   сойти в Ад и запечатать проклятый 
**                   портал.
**
**                   Это смертельно опасное деяние, и вы
**                   рискуете навсегда увязнуть во Тьме. 
**                   Но кто говорил, что путь истинного 
**                   Еретика легок и прост?
*/

#define E2TEXT_RUS  "VJUEXBT VJKJNFDHS GJDTH;TYS>\n"\
                    "B[ LSVZOBTCZ NHEGS GFLF.N<\n"\
                    "HFPDFKBDFZCM YF RECRB< R DFIBV\n"\
                    "YJUFV< B VHFXYJT ELJDKTNDJHTYBT\n"\
                    "YFGJKYZTN DFC>\n"\
                    "\n"\
                    "DHFNF< XNJ JYB J[HFYZKB< JNRHSKBCM>\n"\
                    "DS IFUYEKB D YB[< LEVFZ< XNJ\n"\
                    "DTHYTNTCM D HJLYJQ VBH< YJ KBIM\n"\
                    "UHJVRBQ< YFCVTIKBDSQ [J[JN ,SK\n"\
                    "JNDTNJV YF DFIE YFLT;LE>\n"\
                    "\n"\
                    "XTQ 'NJ PKJ,YSQ [J[JN? ,SNM VJ;TN\n"\
                    "'NJ UJKJC BYS[ LTVJYBXTCRB[ CBK<\n"\
                    "EGHFDKZ.OB[ VJKJNFDHFVB? RFRBT\n"\
                    "XELJDBOYST CJPLFYBZ J;BLF.N DFC\n"\
                    "PF 'NBVB DHFNFVB? YT UJKE,JT YT,J\n"\
                    "HJLYJUJ VBHF YFL UJKJDJQ< F\n"\
                    "RHBCNFKMYSQ REGJK< - 'NJ GKJ[JQ PYFR>>>\n"\
                    ""

/*                  Могучие Молотавры повержены. 
**                  Их дымящиеся трупы падают, 
**                  разваливаясь на куски, к вашим
**                  ногам, и мрачное удовлетворение 
**                  наполняет вас.
**
**                  Врата, что они охраняли, открылись. 
**                  Вы шагнули в них, думая, что 
**                  вернетесь в родной мир, но лишь 
**                  громкий, насмешливый хохот был 
**                  ответом на вашу надежду.
**
**                  Чей это злобный хохот? Быть может 
**                  это голос иных демонических сил, 
**                  управляющих Молотаврами? Какие 
**                  чудовищные создания ожидают вас
**                  за этими вратами? Не голубое небо
**                  родного мира над головой, а 
**                  кристальный купол, - это плохой 
**                  знак...
*/


#define E3TEXT_RUS  "C GJUB,TKM. L\"CGFHBKF BCXTPKF VFUBZ<\n"\
                    "CJ[HFYZDIFZ ;BPYM GJHJ;LTYBZV NMVS>\n"\
                    "CNJYS EVBHF.OB[ LTVJYJD PFUKEIBKB\n"\
                    "DJGKM FUJYBB CFVJUJ L\"CGFHBKF>\n"\
                    "\n"\
                    "DS BCGJKYBKB CDJ. RKZNDE> VTCNM\n"\
                    "CDTHIBKFCM> B PF CTREYLE LJ\n"\
                    "HFPHEITYBZ [HECNFKMYJUJ REGJKF<\n"\
                    "DS YFRJYTW-NJ< D[JLBNT DJ DHFNF<\n"\
                    "DTLEOBT D HJLYJQ VBH>\n"\
                    "\n"\
                    "YJ B NTGTHM< GJCKT UB,TKB L\"CGFHBKF<\n"\
                    "LEIF DFIF YT CGJRJQYF< B GKJ[BT\n"\
                    "GHTLXEDCNDBZ JLJKTDF.N TT> YT ,SK\n"\
                    "KB GHJRKZNBTV TUJ GHTLCVTHNYSQ\n"\
                    "RHBR? BKB GHBPSDJV NTVYS[ CBK?\n"\
                    "\n"\
                    "B ULT NFZNCZ JCNFKMYST PVTBYST\n"\
                    "DCFLYBRB?"

/*                  С погибелью Д'Спарила исчезла магия, 
**                  сохранявшая жизнь порождениям Тьмы.
**                  Стоны умирающих демонов заглушили 
**                  вопль агонии самого Д'Спарила.
**
**                  Вы исполнили свою клятву. Месть 
**                  свершилась. И за секунду до 
**                  разрушения хрустального купола, 
**                  вы наконец-то, входите во врата, 
**                  ведущие в родной мир.
**
**                  Но и теперь, после гибели Д'Спарила,
**                  душа ваша не спокойна, и плохие 
**                  предчувствия одолевают ее. Не был 
**                  ли проклятием его предсмертный 
**                  крик? Или призывом темных сил?
**
**                  И где таятся другие Змеиные 
**                  Всадники?
*/

#define E4TEXT_RUS  "C UB,TKM. L\"CGFHBKF EVTHKF B DFIF\n"\
                    "YFLT;LF DTHYENMCZ D HJLYJQ VBH>\n"\
                    "CDJBV GJCKTLYBV GHTLCVTHNYSV\n"\
                    "GHJRKZNBTV JY JNGHFDBK DFC D NT\n"\
                    "YTVYJUBT< JCNFDIBTCZ GJLDKFCNYSVB\n"\
                    "TVE PTVKB> DS HFP,BKB GJCKTLYB[\n"\
                    "CNHF;TQ 'NB[ PTVTKM B CNJBNT\n"\
                    "GTHTL JGECNTDIBV PFVRJV L\"CGFHBKF<\n"\
                    "JGKJNJV TUJ NTVYS[ CBK>\n"\
                    "\n"\
                    "CFVJ CTHLWT PKF HFCNDJHBKJ GTHTL\n"\
                    "DFVB DHFNF> B [JNZ L\"CGFHBK\n"\
                    "GJDTH;TY< UKE,BYS< GJHJLBDIBT TUJ<\n"\
                    "CNJZN YTHEIBVS>\n"\
                    "\n"\
                    "DS CJQLTNT D 'NE GHTBCGJLY..< B,J\n"\
                    "TLBYCNDTYYSQ CGJCJ, DTHYENMCZ D\n"\
                    "HJLYJQ VBH - JNSCRFNM D[JL D YTUJ\n"\
                    "D NTVYS[ UKE,BYF[ JGECNTDITQ DJNXBYS\n"\
                    "L\"CGFHBKF> KBXYST CNHF;B VTHNDJUJ\n"\
                    "UJCGJLBYF E;T LJ;BLF.NCZ DFC>>>"

/*                  С гибелью Д'Спарила умерла и ваша
**                  надежда вернуться в родной мир. 
**                  Своим последним предсмертным
**                  проклятьем он отправил вас в те 
**                  немногие, оставшиеся подвластными 
**                  ему земли. Вы разбили последних 
**                  хранителей этих земель и стоите
**                  перед опустевшим замком Д'Спарила,
**                  оплотом его темных сил. 
**
**                  Само Сердце Зла растворило перед 
**                  вами врата. И хотя Д'Спарил 
**                  повержен, глубины, породившие его, 
**                  стоят нерушимы.
**
**                  Вы сойдете в эту преисподнюю, ибо 
**                  единственный способ вернуться в 
**                  родной мир -  отыскать вход в него 
**                  в темных глубинах опустевшей вотчины 
**                  Д'Спарила. Личные стражи мертвого 
**                  господина уже дожидаются вас...
*/

#define E5TEXT_RUS  "YF,K.LFZ C VHFXYSV JNDHFOTYBTV\n"\
                    "GHTLCVTHNYE. FUJYB. GJCKTLYTUJ\n"\
                    "CHF;TYYJUJ VJKJNFDHF< DS NJKMRJ\n"\
                    "NTGTHM GJYBVFTNT< XNJ CVTHNM TOT\n"\
                    "YBRJULF YT ,SKF NFR ,KBPRF> LF;T\n"\
                    "DJ DHTVZ ZHJCNYJQ ,BNDS C CFVBV\n"\
                    "L\"CGFHBKJV B TUJ NTVYSVB CKEUFVB>\n"\
                    "\n"\
                    "C EUH.VSV JNXFZYBTV GHB,KB;FTNTCM\n"\
                    "DS R JNRHSNSV GJHNFKFV> CKF,FZ\n"\
                    "YFLT;LF NTGKBNCZ D DFITQ LEIT< -\n"\
                    ",SNM VJ;TN PF YBVB CRHSNF LJHJUF\n"\
                    "LJVJQ< F YT ,TPLYS XE;B[ VBHJD?\n"\
                    "\n"\
                    "JNXFZYBT YFLTKZTN DFC VHFXYJQ\n"\
                    "HTIBVJCNM.> YBXNJ YT CGJCJ,YJ\n"\
                    "JCNFYJDBNM DFC< JLYF NJKMRJ CVTHNM>\n"\
                    "YT CLFDFQNTCM ,TP ,JZ< DPUKZYBNT D\n"\
                    "UKFPF CDJTQ CELM,T> PYFQNT< TCKB DS\n"\
                    "EGFKB YF CFVJT LYJ< TCNM KBIM JLBY\n"\
                    "GENM - YFDTH[>"

/*                  Наблюдая с мрачным отвращением
**                  предсмертную агонию последнего
**                  сраженного Молотавра, вы только 
**                  теперь понимаете, что смерть еще
**                  никогда не была так близка. Даже
**                  во время яростной битвы с самим
**                  Д'Спарилом и его темными слугами.
**
**                  С угрюмым отчаянием приближаетесь
**                  вы к открытым порталам. Слабая 
**                  надежда теплится в вашей душе, -
**                  быть может за ними скрыта дорога
**                  домой, а не бездны чужих миров?
**
**                  Отчаяние наделяет вас мрачной 
**                  решимостью. Ничто не способно 
**                  остановить вас, одна только - смерть.
**                  Не сдавайтесь без боя, взгляните в 
**                  глаза своей судьбе. Знайте, если вы
**                  упали на самое дно, есть лишь один 
**                  путь - наверх.
*/

// Специфические сообщения порта
#define TXT_ALWAYSRUN_ON_RUS    "GJCNJZYYSQ ,TU DRK.XTY"    // ПОСТОЯННЫЙ БЕГ ВКЛЮЧЕН
#define TXT_ALWAYSRUN_OFF_RUS   "GJCNJZYYSQ ,TU DSRK.XTY"   // ПОСТОЯННЫЙ БЕГ ВЫКЛЮЧЕН
#define TXT_MLOOK_ON_RUS        "J,PJH VSIM. DRK.XTY"       // ОБЗОР МЫШЬЮ ВКЛЮЧЕН
#define TXT_MLOOK_OFF_RUS       "J,PJH VSIM. DSRK.XTY"      // ОБЗОР МЫШЬЮ ВЫКЛЮЧЕН
#define TXT_CROSSHAIR_ON_RUS    "GHBWTK DRK.XTY"            // ПРИЦЕЛ ВКЛЮЧЕН
#define TXT_CROSSHAIR_OFF_RUS   "GHBWTK DSRK.XTY"           // ПРИЦЕЛ ВЫКЛЮЧЕН
#define RD_ON_RUS               "DRK"                       // ВКЛ
#define RD_OFF_RUS              "DSRK"                      // ВЫКЛ

// English
#if defined (_WIN64)
#define TXT_ARCH      " - (X64)"
#elif defined (_WIN32)
#define TXT_ARCH      " - (X86)"
#else
#define TXT_ARCH      ""
#endif

// Русский
#if defined (_WIN64)
#define TXT_ARCH_RUS      " - ([64)"        // x64
#elif defined (_WIN32)
#define TXT_ARCH_RUS      " - ([86)"        // x86
#else
#define TXT_ARCH_RUS      ""                // ?  
#endif

#endif
