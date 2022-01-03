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



//
// Памятка по символам: docs_russian/charmap/raven_charmap.xlsx
//

// -----------------------------------------------------------------------------
// English language
// -----------------------------------------------------------------------------

// Gamma-correction
#define TXT_GAMMA_0_50  "GAMMA CORRECTION LEVEL 0.50"
#define TXT_GAMMA_0_55  "GAMMA CORRECTION LEVEL 0.55"
#define TXT_GAMMA_0_60  "GAMMA CORRECTION LEVEL 0.60"
#define TXT_GAMMA_0_65  "GAMMA CORRECTION LEVEL 0.65"
#define TXT_GAMMA_0_70  "GAMMA CORRECTION LEVEL 0.70"
#define TXT_GAMMA_0_75  "GAMMA CORRECTION LEVEL 0.75"
#define TXT_GAMMA_0_80  "GAMMA CORRECTION LEVEL 0.80"
#define TXT_GAMMA_0_85  "GAMMA CORRECTION LEVEL 0.85"
#define TXT_GAMMA_0_90  "GAMMA CORRECTION LEVEL 0.90"
#define TXT_GAMMA_1_0   "GAMMA CORRECTION LEVEL 1.0"
#define TXT_GAMMA_1_125 "GAMMA CORRECTION LEVEL 1.125"
#define TXT_GAMMA_1_25  "GAMMA CORRECTION LEVEL 1.25"
#define TXT_GAMMA_1_375 "GAMMA CORRECTION LEVEL 1.375"
#define TXT_GAMMA_1_5   "GAMMA CORRECTION LEVEL 1.5"
#define TXT_GAMMA_1_625 "GAMMA CORRECTION LEVEL 1.625"
#define TXT_GAMMA_1_75  "GAMMA CORRECTION LEVEL 1.75"
#define TXT_GAMMA_1_875 "GAMMA CORRECTION LEVEL 1.875"
#define TXT_GAMMA_2_0   "GAMMA CORRECTION LEVEL 2.0"

#define	EMPTYSTRING     "EMPTY SLOT"

#define RD_ON   "ON"
#define RD_OFF  "OFF"


// P_inter.c ---------------------------------------------------------------

// Mana

#define TXT_MANA_1				"BLUE MANA"
#define TXT_MANA_2				"GREEN MANA"
#define TXT_MANA_BOTH			"COMBINED MANA"

// Keys

#define	TXT_KEY_STEEL		"STEEL KEY"
#define	TXT_KEY_CAVE		"CAVE KEY"
#define	TXT_KEY_AXE			"AXE KEY"
#define	TXT_KEY_FIRE		"FIRE KEY"
#define	TXT_KEY_EMERALD		"EMERALD KEY"
#define	TXT_KEY_DUNGEON		"DUNGEON KEY"
#define	TXT_KEY_SILVER		"SILVER KEY"
#define	TXT_KEY_RUSTED		"RUSTED KEY"
#define	TXT_KEY_HORN		"HORN KEY"
#define	TXT_KEY_SWAMP		"SWAMP KEY"
#define TXT_KEY_CASTLE		"CASTLE KEY"

// Artifacts

#define TXT_ARTIINVULNERABILITY		"ICON OF THE DEFENDER"
#define TXT_ARTIHEALTH				"QUARTZ FLASK"
#define TXT_ARTISUPERHEALTH			"MYSTIC URN"
#define TXT_ARTISUMMON				"DARK SERVANT"
#define TXT_ARTITORCH				"TORCH"
#define TXT_ARTIEGG					"PORKALATOR"
#define TXT_ARTIFLY					"WINGS OF WRATH"
#define TXT_ARTITELEPORT			"CHAOS DEVICE"
#define TXT_ARTIPOISONBAG			"FLECHETTE"
#define TXT_ARTITELEPORTOTHER		"BANISHMENT DEVICE"
#define TXT_ARTISPEED				"BOOTS OF SPEED"
#define TXT_ARTIBOOSTMANA			"KRATER OF MIGHT"
#define TXT_ARTIBOOSTARMOR			"DRAGONSKIN BRACERS"
#define TXT_ARTIBLASTRADIUS			"DISC OF REPULSION"
#define TXT_ARTIHEALINGRADIUS		"MYSTIC AMBIT INCANT"

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

#define TXT_ITEMHEALTH			"CRYSTAL VIAL"
#define TXT_ITEMBAGOFHOLDING	"BAG OF HOLDING"
#define TXT_ITEMSHIELD1			"SILVER SHIELD"
#define TXT_ITEMSHIELD2			"ENCHANTED SHIELD"
#define TXT_ITEMSUPERMAP		"MAP SCROLL"
#define TXT_ARMOR1				"MESH ARMOR"
#define TXT_ARMOR2				"FALCON SHIELD"
#define TXT_ARMOR3				"PLATINUM HELMET"
#define TXT_ARMOR4				"AMULET OF WARDING"

// Weapons

#define TXT_WEAPON_F2			"TIMON'S AXE"
#define TXT_WEAPON_F3			"HAMMER OF RETRIBUTION"
#define TXT_WEAPON_F4			"QUIETUS ASSEMBLED"
#define TXT_WEAPON_C2			"SERPENT STAFF"
#define TXT_WEAPON_C3			"FIRESTORM"
#define TXT_WEAPON_C4			"WRAITHVERGE ASSEMBLED"
#define TXT_WEAPON_M2			"FROST SHARDS"
#define TXT_WEAPON_M3			"ARC OF DEATH"
#define TXT_WEAPON_M4			"BLOODSCOURGE ASSEMBLED"
#define TXT_QUIETUS_PIECE		"SEGMENT OF QUIETUS"
#define TXT_WRAITHVERGE_PIECE	"SEGMENT OF WRAITHVERGE"
#define TXT_BLOODSCOURGE_PIECE	"SEGMENT OF BLOODSCOURGE"


// SB_bar.c ----------------------------------------------------------------

#define TXT_CHEATGODON			"GOD MODE ON"
#define TXT_CHEATGODOFF			"GOD MODE OFF"
#define TXT_CHEATNOCLIPON		"NO CLIPPING ON"
#define TXT_CHEATNOCLIPOFF		"NO CLIPPING OFF"
#define TXT_CHEATWEAPONS		"ALL WEAPONS"
#define TXT_CHEATHEALTH			"FULL HEALTH"
#define TXT_CHEATKEYS			"ALL KEYS"
#define TXT_CHEATSOUNDON		"SOUND DEBUG ON"
#define TXT_CHEATSOUNDOFF		"SOUND DEBUG OFF"
#define TXT_CHEATTICKERON		"TICKER ON"
#define TXT_CHEATTICKEROFF		"TICKER OFF"
#define TXT_CHEATARTIFACTS3		"ALL ARTIFACTS"
#define TXT_CHEATARTIFACTSFAIL	"BAD INPUT"
#define TXT_CHEATWARP			"LEVEL WARP"
#define TXT_CHEATSCREENSHOT		"SCREENSHOT"
#define TXT_CHEATIDDQD			"TRYING TO CHEAT, EH?  NOW YOU DIE!"
#define TXT_CHEATIDKFA			"CHEATER - YOU DON'T DESERVE WEAPONS"
#define TXT_CHEATBADINPUT		"BAD INPUT"
#define TXT_CHEATNOMAP			"CAN'T FIND MAP"


// G_game.c ----------------------------------------------------------------

#define TXT_GAMESAVED			"GAME SAVED"


// M_misc.c ----------------------------------------------------------------

#define HUSTR_CHATMACRO1 "I'm ready to kick butt!"
#define HUSTR_CHATMACRO2 "I'm OK."
#define HUSTR_CHATMACRO3 "I'm not looking too good!"
#define HUSTR_CHATMACRO4 "Help!"
#define HUSTR_CHATMACRO5 "You suck!"
#define HUSTR_CHATMACRO6 "Next time, scumbag..."
#define HUSTR_CHATMACRO7 "Come here!"
#define HUSTR_CHATMACRO8 "I'll take care of it."
#define HUSTR_CHATMACRO9 "Yes"
#define HUSTR_CHATMACRO0 "No"


// AM_map.c ----------------------------------------------------------------

#define AMSTR_FOLLOWON		"FOLLOW MODE ON"
#define AMSTR_FOLLOWOFF		"FOLLOW MODE OFF"

// [JN] Port-specific strings

#define TXT_ALWAYSRUN_ON    "ALWAYS RUN ON"
#define TXT_ALWAYSRUN_OFF   "ALWAYS RUN OFF"

#define TXT_MLOOK_ON        "MOUSE LOOK ON"
#define TXT_MLOOK_OFF       "MOUSE LOOK OFF"

#define TXT_CROSSHAIR_ON    "CROSSHAIR ON"
#define TXT_CROSSHAIR_OFF   "CROSSHAIR OFF"


// -----------------------------------------------------------------------------
// Русский язык
// -----------------------------------------------------------------------------

// MN_menu.c ---------------------------------------------------------------

// Гамма-коррекция
#define TXT_GAMMA_RUS_0_50  "EHJDTYM UFVVF-RJHHTRWBB 0>50"
#define TXT_GAMMA_RUS_0_55  "EHJDTYM UFVVF-RJHHTRWBB 0>55"
#define TXT_GAMMA_RUS_0_60  "EHJDTYM UFVVF-RJHHTRWBB 0>60"
#define TXT_GAMMA_RUS_0_65  "EHJDTYM UFVVF-RJHHTRWBB 0>65"
#define TXT_GAMMA_RUS_0_70  "EHJDTYM UFVVF-RJHHTRWBB 0>70"
#define TXT_GAMMA_RUS_0_75  "EHJDTYM UFVVF-RJHHTRWBB 0>75"
#define TXT_GAMMA_RUS_0_80  "EHJDTYM UFVVF-RJHHTRWBB 0>80"
#define TXT_GAMMA_RUS_0_85  "EHJDTYM UFVVF-RJHHTRWBB 0>85"
#define TXT_GAMMA_RUS_0_90  "EHJDTYM UFVVF-RJHHTRWBB 0>90"
#define TXT_GAMMA_RUS_1_0   "EHJDTYM UFVVF-RJHHTRWBB 1>0"
#define TXT_GAMMA_RUS_1_125 "EHJDTYM UFVVF-RJHHTRWBB 1>125"
#define TXT_GAMMA_RUS_1_25  "EHJDTYM UFVVF-RJHHTRWBB 1>25"
#define TXT_GAMMA_RUS_1_375 "EHJDTYM UFVVF-RJHHTRWBB 1>375"
#define TXT_GAMMA_RUS_1_5   "EHJDTYM UFVVF-RJHHTRWBB 1>5"
#define TXT_GAMMA_RUS_1_625 "EHJDTYM UFVVF-RJHHTRWBB 1>625"
#define TXT_GAMMA_RUS_1_75  "EHJDTYM UFVVF-RJHHTRWBB 1>75"
#define TXT_GAMMA_RUS_1_875 "EHJDTYM UFVVF-RJHHTRWBB 1>875"
#define TXT_GAMMA_RUS_2_0   "EHJDTYM UFVVF-RJHHTRWBB 2>0"

#define	EMPTYSTRING_RUS     "GECNJQ CKJN"  // ПУСТОЙ СЛОТ

#define RD_ON_RUS   "DRK"   // ВКЛ
#define RD_OFF_RUS  "DSRK"  // ВЫКЛ

// P_inter.c ---------------------------------------------------------------

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

#define TXT_ARTIINVULNERABILITY_RUS "CBVDJK PFOBNYBRF"          // СИМВОЛ ЗАЩИТНИКА
#define TXT_ARTIHEALTH_RUS          "RDFHWTDSQ AKFRJY"          // КВАРЦЕВЫЙ ФЛАКОН
#define TXT_ARTISUPERHEALTH_RUS     "VBCNBXTCRFZ EHYF"          // МИСТИЧЕСКАЯ УРНА
#define TXT_ARTISUMMON_RUS          "NTVYSQ CKEUF"              // ТЕМНЫЙ СЛУГА
#define TXT_ARTITORCH_RUS           "AFRTK"                     // ФАКЕЛ
#define TXT_ARTIEGG_RUS             "CDBYJVJHATH"               // СВИНОМОРФЕР
#define TXT_ARTIFLY_RUS             "RHSKMZ UYTDF"              // КРЫЛЬЯ ГНЕВА
#define TXT_ARTITELEPORT_RUS        "'V,KTVF [FJCF"             // ЭМБЛЕМА ХАОСА
#define TXT_ARTIPOISONBAG_RUS       "PTKMT"                     // ЗЕЛЬЕ
#define TXT_ARTITELEPORTOTHER_RUS   "'V,KTVF BPUYFYBZ"          // ЭМБЛЕМА ИЗГНАНИЯ
#define TXT_ARTISPEED_RUS           "CFGJUB-CRJHJ[JLS"          // САПОГИ-СКОРОХОДЫ
#define TXT_ARTIBOOSTMANA_RUS       "XFIF VJUEOTCNDF"           // ЧАША МОГУЩЕСТВА
#define TXT_ARTIBOOSTARMOR_RUS      "YFHEXB BP LHFRJYMTQ RJ;B"  // НАРУЧИ ИЗ ДРАКОНЬЕЙ КОЖИ
#define TXT_ARTIBLASTRADIUS_RUS     "LBCR JNNJH;TYBZ"           // ДИСК ОТТОРЖЕНИЯ
#define TXT_ARTIHEALINGRADIUS_RUS   "XFHS VFUBXTCRJUJ TLBYCNDF" // ЧАРЫ МАГИЧЕСКОГО ЕДИНСТВА

// Puzzle artifacts

#define TXT_ARTIPUZZSKULL_RUS       "XTHTG QJHBRF"			// ЧЕРЕП ЙОРИКА
#define TXT_ARTIPUZZGEMBIG_RUS      "CTHLWT L&CGFHBKF"		// СЕРДЦЕ Д'СПАРИЛА
#define TXT_ARTIPUZZGEMRED_RUS      "HE,BYJDFZ GKFYTNF"		// РУБИНОВАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMGREEN1_RUS   "BPEVHELYFZ GKFYTNF"	// ИЗУМРУДНАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMGREEN2_RUS   "BPEVHELYFZ GKFYTNF"	// ИЗУМРУДНАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMBLUE1_RUS    "CFGABHJDFZ GKFYTNF"	// САПФИРОВАЯ ПЛАНЕТА
#define TXT_ARTIPUZZGEMBLUE2_RUS    "CFGABHJDFZ GKFYTNF"	// САПФИРОВАЯ ПЛАНЕТА
#define TXT_ARTIPUZZBOOK1_RUS       "RJLTRC LTVJYF"			// КОДЕКС ДЕМОНА
#define TXT_ARTIPUZZBOOK2_RUS       "$"						// LIBER OSCURA (не переводить, спец. символ "$")
#define TXT_ARTIPUZZSKULL2_RUS      "VFCRF GKFVTYB"			// МАСКА ПЛАМЕНИ
#define TXT_ARTIPUZZFWEAPON_RUS     "GTXFNM DJBNTKZ"		// ПЕЧАТЬ ВОИТЕЛЯ
#define TXT_ARTIPUZZCWEAPON_RUS     "CDZNFZ HTKBRDBZ"		// СВЯТАЯ РЕЛИКВИЯ
#define TXT_ARTIPUZZMWEAPON_RUS     "CBVDJK VFUF"			// СИМВОЛ МАГА
#define TXT_ARTIPUZZGEAR_RUS        "XFCJDFZ ITCNTHYZ"		// ЧАСОВАЯ ШЕСТЕРНЯ
#define TXT_USEPUZZLEFAILED_RUS     "PLTCM 'NJ YTDJPVJ;YJ BCGJKMPJDFNM"	// ЗДЕСЬ ЭТО НЕВОЗМОЖНО ИСПОЛЬЗОВАТЬ

// Items

#define TXT_ITEMHEALTH_RUS          "RHBCNFKMYSQ AKFRJY"    // КРИСТАЛЬНЫЙ ФЛАКОН
#define TXT_ITEMBAGOFHOLDING_RUS    "YJCBKMYFZ RJITKM"      // НОСИЛЬНЫЙ КОШЕЛЬ
#define TXT_ITEMSHIELD1_RUS         "CTHT,HZYSQ OBN"        // СЕРЕБРЯНЫЙ ЩИТ
#define TXT_ITEMSHIELD2_RUS         "PFXFHJDFYYSQ OBN"      // ЗАЧАРОВАННЫЙ ЩИТ
#define TXT_ITEMSUPERMAP_RUS        "CDBNJR RFHNS"          // СВИТОК КАРТЫ
#define TXT_ARMOR1_RUS              "RJKMXEUF"              // КОЛЬЧУГА
#define TXT_ARMOR2_RUS              "CJRJKBYSQ OBN"         // СОКОЛИНЫЙ ЩИТ
#define TXT_ARMOR3_RUS              "GKFNBYJDSQ IKTV"       // ПЛАТИНОВЫЙ ШЛЕМ
#define TXT_ARMOR4_RUS              "FVEKTN CNHF;F"         // АМУЛЕТ СТРАЖА

// Weapons

#define TXT_WEAPON_F2_RUS           "NJGJH NBVJYF"                      // ТОПОР ТИМОНА
#define TXT_WEAPON_F3_RUS           "VJKJN DJPVTPLBZ"                   // МОЛОТ ВОЗМЕЗДИЯ
#define TXT_WEAPON_F4_RUS           "GJCKTLYBQ LJDJL CJ,HFY DJTLBYJ"    // ПОСЛЕДНИЙ ДОВОД СОБРАН ВОЕДИНО
#define TXT_WEAPON_C2_RUS           "PVTBYSQ GJCJ["                     // ЗМЕИНЫЙ ПОСОХ
#define TXT_WEAPON_C3_RUS           "JUYTYYSQ INJHV"                    // ОГНЕННЫЙ ШТОРМ
#define TXT_WEAPON_C4_RUS           ";TPK LE[JD CJ,HFY DJTLBYJ"         // ЖЕЗЛ ДУХОВ СОБРАН ВОЕДИНО
#define TXT_WEAPON_M2_RUS           "KTLZYST JCRJKRB"                   // ЛЕДЯНЫЕ ОСКОЛКИ
#define TXT_WEAPON_M3_RUS           "LEUF CVTHNB"                       // ДУГА СМЕРТИ
#define TXT_WEAPON_M4_RUS           "RHJDFDSQ ,BX CJ,HFY DJTLBYJ"       // КРОВАВЫЙ БИЧ СОБРАН ВОЕДИНО
#define TXT_QUIETUS_PIECE_RUS       "XFCNM GJCKTLYTUJ LJDJLF"           // ЧАСТЬ ПОСЛЕДНЕГО ДОВОДА
#define TXT_WRAITHVERGE_PIECE_RUS   "XFCNM ;TPKF LE[JD"                 // ЧАСТЬ ЖЕЗЛА ДУХОВ
#define TXT_BLOODSCOURGE_PIECE_RUS  "XFCNM RHJDFDJUJ ,BXF"              // ЧАСТЬ КРОВАВОГО БИЧА

// SB_bar.c ----------------------------------------------------------------

#define TXT_CHEATGODON_RUS          "HT;BV ,JUF"                            // РЕЖИМ БОГА
#define TXT_CHEATGODOFF_RUS         "HT;BV ,JUF JNVTYTY"                    // РЕЖИМ БОГА ОТМЕНЕН
#define TXT_CHEATNOCLIPON_RUS       "GHJ[J;LTYBT XTHTP CNTYS"               // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ
#define TXT_CHEATNOCLIPOFF_RUS      "GHJ[J;LTYBT XTHTP CNTYS JNVTYTYJ"      // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ ОТМЕНЕНО
#define TXT_CHEATWEAPONS_RUS        "GJKYJT DJJHE;TYBT"                     // ПОЛНОЕ ВООРУЖЕНИЕ
#define TXT_CHEATHEALTH_RUS         "PLJHJDMT DJCCNFYJDKTYJ"                // ЗДОРОВЬЕ ВОССТАНОВЛЕНО
#define TXT_CHEATKEYS_RUS           "DCT RK.XB"                             // ВСЕ КЛЮЧИ
#define TXT_CHEATSOUNDON_RUS        "JNKFLRF PDERF DRK.XTYF"                // ОТЛАДКА ЗВУКА ВКЛЮЧЕНА
#define TXT_CHEATSOUNDOFF_RUS       "JNKFLRF PDERF JNRK.XTYF"               // ОТЛАДКА ЗВУКА ОТКЛЮЧЕНА
#define TXT_CHEATTICKERON_RUS       "CXTNXBR RFLHJDJQ XFCNJNS DRK.XTY"      // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ВКЛЮЧЕН
#define TXT_CHEATTICKEROFF_RUS      "CXTNXBR RFLHJDJQ XFCNJNS JNRK.XTY"     // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ОТКЛЮЧЕН
#define TXT_CHEATARTIFACTS3_RUS     "DCT FHNTAFRNS"                         // ВСЕ АРТЕФАКТЫ
#define TXT_CHEATARTIFACTSFAIL_RUS  "YTRJHHTRNYSQ DDJL"                     // НЕКОРРЕНТНЫЙ ВВОД
#define TXT_CHEATWARP_RUS           "GTHTVTOTYBT YF EHJDTYM"                // ПЕРЕМЕЩЕНИЕ НА УРОВЕНЬ
#define TXT_CHEATSCREENSHOT_RUS     "CYBVJR 'RHFYF"                         // СНИМОК ЭКРАНА
#define TXT_CHEATIDDQD_RUS          "GSNFTIMCZ C[BNHBNM? NFR EVHB ;T!"      // ПЫТАЕШЬСЯ СХИТРИТЬ? ТАК УМРИ ЖЕ!
#define TXT_CHEATIDKFA_RUS          ";EKBR< NS YT LJCNJBY CDJTUJ JHE;BZ"    // ЖУЛИК, ТЫ НЕ ДОСТОИН СВОЕГО ОРУЖИЯ
#define TXT_CHEATBADINPUT_RUS       "YTRJHHTRNYSQ DDJL"                     // НЕКОРРЕНТНЫЙ ВВОД
#define TXT_CHEATNOMAP_RUS          "EHJDTYM YT YFQLTY"                     // УРОВЕНЬ НЕ НАЙДЕН

// G_game.c ----------------------------------------------------------------

#define TXT_GAMESAVED_RUS           "BUHF CJ[HFYTYF"    // ИГРА СОХРАНЕНА

// AM_map.c ----------------------------------------------------------------

#define AMSTR_FOLLOWON_RUS          "HT;BV CKTLJDFYBZ DRK.XTY"  // РЕЖИМ СЛЕДОВАНИЯ ВКЛЮЧЕН
#define AMSTR_FOLLOWOFF_RUS         "HT;BV CKTLJDFYBZ JNRK.XTY" // РЕЖИМ СЛЕДОВАНИЯ ОТКЛЮЧЕН

// [JN] Специфические сообщения порта

#define TXT_ALWAYSRUN_ON_RUS        "GJCNJZYYSQ ,TU DRK.XTY"    // ПОСТОЯННЫЙ БЕГ ВКЛЮЧЕН
#define TXT_ALWAYSRUN_OFF_RUS       "GJCNJZYYSQ ,TU DSRK.XTY"   // ПОСТОЯННЫЙ БЕГ ВЫКЛЮЧЕН

#define TXT_MLOOK_ON_RUS            "J,PJH VSIM. DRK.XTY"       // ОБЗОР МЫШЬЮ ВКЛЮЧЕН
#define TXT_MLOOK_OFF_RUS           "J,PJH VSIM. DSRK.XTY"      // ОБЗОР МЫШЬЮ ВЫКЛЮЧЕН

#define TXT_CROSSHAIR_ON_RUS        "GHBWTK DRK.XTY"            // ПРИЦЕЛ ВКЛЮЧЕН
#define TXT_CROSSHAIR_OFF_RUS       "GHBWTK DSRK.XTY"           // ПРИЦЕЛ ВЫКЛЮЧЕН

#if defined (_WIN64)
#define TXT_ARCH      " - (X64)"
#elif defined (_WIN32)
#define TXT_ARCH      " - (X86)"
#else
#define TXT_ARCH      ""
#endif

#if defined (_WIN64)
#define TXT_ARCH_RUS      " - ([64)"        // x64
#elif defined (_WIN32)
#define TXT_ARCH_RUS      " - ([86)"        // x86
#else
#define TXT_ARCH_RUS      ""                // ?  
#endif
