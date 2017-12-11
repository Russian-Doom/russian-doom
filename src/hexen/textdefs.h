//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

// Russian Doom (C) 2016-2017 Julian Nechaevsky


//
//   Памятка по символам:
//   --------------------
//   Строчный шрифт:
//   { = Х (STCFN064)
//   } = Ъ (STCFN066)
//   : = Ж (FONTA26)
//   " = Э (FONTA02)
//   < = Б (FONTA28)
//   > = Ю (FONTA30)
//   ' = " (FONTA07)
//   & = ' (FONTA06)
//
//   Заглавный шрифт:
//   { = Х (STCFN065)
//   } = Ъ (STCFN067)
//   : = Ж (FONTB26)
//   " = Э (FONTB26)
//   < = Б (FONTB28)
//   > = Ю (FONTB30)


// MN_menu.c ---------------------------------------------------------------

#define PRESSKEY	"YF:VBNT K><E> RKFDBIE." 	// НАЖМИТЕ ЛЮБУЮ КЛАВИШУ.		press a key.
#define PRESSYN		"press y or n." 			// НАЖМИТЕ "Y" ИЛИ "N".			press y or n.
#define TXT_PAUSED	"GFEPF"						// ПАУЗА						PAUSED
#define QUITMSG		"DS LTQCNDBNTKMYJ {JNBNT DSQNB\nBP \"NJQ PFVTXFNTKMYJQ BUHS?"	// ВЫ ДЕЙСТВИТЕЛЬНО ХОТИТЕ ВЫЙТИ \n ИЗ ЭТОЙ ЗАМЕЧАТЕЛЬНОЙ ИГРЫ?		are you sure you want to\nquit this great game?
#define LOADNET		"YTDJPVJ:YJ PFUHEPBNMCZ D CTNTDJQ BUHT!\n\n"PRESSKEY			// НЕВОЗМОЖНО ЗАГРУЗИТЬСЯ В СЕТЕВОЙ ИГРЕ! 							you can't do load while in a net game!
#define QLOADNET	"YTDJPVJ:YJ PFUHEPBNMCZ D CTNTDJQ BUHT!\n\n"PRESSKEY			// НЕВОЗМОЖНО ЗАГРУЗИТЬСЯ В СЕТЕВОЙ ИГРЕ! 							you can't quickload during a netgame!
#define QSAVESPOT	"E DFC YT DS<HFY CKJN <SCNHJUJ CJ{HFYTYBZ!\n\n"PRESSKEY			// У ВАС НЕ ВЫБРАН СЛОТ БЫСТРОГО СОХРАНЕНИЯ!						you haven't picked a quicksave slot yet!
#define SAVEDEAD 	"YTDJPVJ:YJ CJ{HFYBNM BUHE, YT YFXFD TT!\n\n"PRESSKEY			// НЕВОЗМОЖНО СОХРАНИТЬ ИГРУ, НЕ НАЧАВ ЕЁ!							you can't save if you aren't playing!
#define QSPROMPT 	"GTHTPFGBCFNM <SCNHJT CJ{HFYTYBT\n\n'%s'?\n\n"PRESSYN			// ПЕРЕЗАПИСАТЬ БЫСТРОЕ СОХРАНЕНИЕ \n\n '%s'?						quicksave over your game named\n\n'%s'?
#define QLPROMPT	"do you want to quickload the game named"\
					"\n\n'%s'?\n\n"PRESSYN
#define NEWGAME		"you can't start a new game\n"\
					"while in a network game.\n\n"PRESSKEY
#define MSGOFF		"CJJ<OTYBZ JNRK>XTYS"											// СООБЩЕНИЯ ОТКЛЮЧЕНЫ			Messages OFF
#define MSGON		"CJJ<OTYBZ DRK>XTYS"											// СООБЩЕНИЯ ВКЛЮЧЕНЫ 			Messages ON
#define NETEND		"YTDJPVJ:YJ PFRJYXBNM CTNTDE> BUHE!\n\n"PRESSKEY				// НЕВОЗМОЖНО ЗАКОНЧИТЬ СЕТЕВУЮ ИГРУ!	you can't end a netgame!
#define ENDGAME		"are you sure you want to end the game?\n\n"PRESSYN
#define DOSY		"(YF:VBNT ^E^ XNJ<S DSQNB.)"									// (НАЖМИТЕ "Y" ЧТОБЫ ВЫЙТИ.)	(press y to quit to dos.)

#define TXT_GAMMA_LEVEL_OFF      "UFVVF-RJHHTRWBZ JNRK>XTYF"        // ГАММА-КОРРЕКЦИЯ ОТКЛЮЧЕНА
#define TXT_GAMMA_LEVEL_0_25     "EHJDTYM UFVVF-RJHHTRWBB 0.25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 0.25
#define TXT_GAMMA_LEVEL_0_50     "EHJDTYM UFVVF-RJHHTRWBB 0.50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 0.50
#define TXT_GAMMA_LEVEL_0_75     "EHJDTYM UFVVF-RJHHTRWBB 0.75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 0.75
#define TXT_GAMMA_LEVEL_1_0      "EHJDTYM UFVVF-RJHHTRWBB 1.0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.0
#define TXT_GAMMA_LEVEL_1_25     "EHJDTYM UFVVF-RJHHTRWBB 1.25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.25
#define TXT_GAMMA_LEVEL_1_50     "EHJDTYM UFVVF-RJHHTRWBB 1.50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.50
#define TXT_GAMMA_LEVEL_1_75     "EHJDTYM UFVVF-RJHHTRWBB 1.75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.75
#define TXT_GAMMA_LEVEL_2_0      "EHJDTYM UFVVF-RJHHTRWBB 2.0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.0
#define TXT_GAMMA_LEVEL_2_25     "EHJDTYM UFVVF-RJHHTRWBB 2.25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.25
#define TXT_GAMMA_LEVEL_2_50     "EHJDTYM UFVVF-RJHHTRWBB 2.50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.50
#define TXT_GAMMA_LEVEL_2_75     "EHJDTYM UFVVF-RJHHTRWBB 2.75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.75
#define TXT_GAMMA_LEVEL_3_0      "EHJDTYM UFVVF-RJHHTRWBB 3.0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.0
#define TXT_GAMMA_LEVEL_3_25     "EHJDTYM UFVVF-RJHHTRWBB 3.25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.25
#define TXT_GAMMA_LEVEL_3_50     "EHJDTYM UFVVF-RJHHTRWBB 3.50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.50
#define TXT_GAMMA_LEVEL_3_75     "EHJDTYM UFVVF-RJHHTRWBB 3.75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.75
#define TXT_GAMMA_LEVEL_4_0      "EHJDTYM UFVVF-RJHHTRWBB 4.0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 4.0

#define	EMPTYSTRING             "GECNJQ CKJN"                       // ПУСТОЙ СЛОТ

// P_inter.c ---------------------------------------------------------------

// Mana

#define TXT_MANA_1				"CBYZZ VFYF"		   // СИНЯЯ МАНА			BLUE MANA
#define TXT_MANA_2				"PTKTYFZ VFYF"	       // ЗЕЛЕНАЯ МАНА          GREEN MANA 
#define TXT_MANA_BOTH			"RJV<BYBHJDFYYFZ VFYF" // КОМБИНИРОВАННАЯ МАНА  COMBINED MANA

// Keys

#define	TXT_KEY_STEEL		"CNFKMYJQ RK>X"			// СТАЛЬНОЙ КЛЮЧ			STEEL KEY
#define	TXT_KEY_CAVE		"GTOTHYSQ RK>X"			// ПЕЩЕРНЫЙ КЛЮЧ            CAVE KEY
#define	TXT_KEY_AXE			"RK>X-NJGJH"			// КЛЮЧ-ТОПОР	            AXE KEY
#define	TXT_KEY_FIRE		"JUYTYYSQ RK>X"			// ОГНЕННЫЙ КЛЮЧ            FIRE KEY
#define	TXT_KEY_EMERALD		"BPEVHELYSQ RK>X"		// ИЗУМРУДНЫЙ КЛЮЧ          EMERALD KEY
#define	TXT_KEY_DUNGEON		"RK>X JN GJLPTVTKMZ"	// КЛЮЧ ОТ ПОДЗЕМЕЛЬЯ       DUNGEON KEY
#define	TXT_KEY_SILVER		"CTHT<HZYSQ RK>X"		// СЕРЕБРЯНЫЙ КЛЮЧ          SILVER KEY
#define	TXT_KEY_RUSTED		"H:FDSQ RK>X"			// РЖАВЫЙ КЛЮЧ              RUSTED KEY
#define	TXT_KEY_HORN		"HJUJDJQ RK>X"			// РОГОВОЙ КЛЮЧ             HORN KEY
#define	TXT_KEY_SWAMP		"<JKJNYSQ RK>X"			// БОЛОТНЫЙ КЛЮЧ            SWAMP KEY
#define TXT_KEY_CASTLE		"RK>X JN PFVRF"			// КЛЮЧ ОТ ЗАМКА            CASTLE KEY

// Artifacts

#define TXT_ARTIINVULNERABILITY		"CBVDJK PFOBNYBRF"			// СИМВОЛ ЗАЩИТНИКА				ICON OF THE DEFENDER
#define TXT_ARTIHEALTH				"RDFHWTDSQ AKFRJY"			// КВАРЦЕВЫЙ ФЛАКОН				QUARTZ FLASK
#define TXT_ARTISUPERHEALTH			"VBCNBXTCRFZ EHYF"			// МИСТИЧЕСКАЯ УРНА				MYSTIC URN
#define TXT_ARTISUMMON				"NTVYSQ CKEUF"				// ТЕМНЫЙ СЛУГА					DARK SERVANT
#define TXT_ARTITORCH				"AFRTK"						// ФАКЕЛ						TORCH
#define TXT_ARTIEGG					"CDBYJVJHATH"				// СВИНОМОРФЕР					PORKALATOR
#define TXT_ARTIFLY					"RHSKMZ UYTDF"				// КРЫЛЬЯ ГНЕВА					WINGS OF WRATH
#define TXT_ARTITELEPORT			"\"V<KTVF {FJCF"			// ЭМБЛЕМА ХАОСА				CHAOS DEVICE
#define TXT_ARTIPOISONBAG			"PTKMT"						// ЗЕЛЬЕ 						FLECHETTE
#define TXT_ARTITELEPORTOTHER		"\"V<KTVF BPUYFYBZ"			// ЭМБЛЕМА ИЗГНАНИЯ				BANISHMENT DEVICE
#define TXT_ARTISPEED				"CFGJUB-CRJHJ{JLS"			// САПОГИ-СКОРОХОДЫ				BOOTS OF SPEED
#define TXT_ARTIBOOSTMANA			"XFIF VJUEOTCNDF"			// ЧАША МОГУЩЕСТВА				KRATER OF MIGHT
#define TXT_ARTIBOOSTARMOR			"YFHEXB BP LHFRJYMTQ RJ:B"	// НАРУЧИ ИЗ ДРАКОНЬЕЙ КОЖИ		DRAGONSKIN BRACERS
#define TXT_ARTIBLASTRADIUS			"LBCR JNNJH:TYBZ"			// ДИСК ОТТОРЖЕНИЯ				DISC OF REPULSION
#define TXT_ARTIHEALINGRADIUS		"XFHS VFUBXTCRJUJ TLBYCNDF"	// ЧАРЫ МАГИЧЕСКОГО ЕДИНСТВА	MYSTIC AMBIT INCANT

// Puzzle artifacts

#define TXT_ARTIPUZZSKULL			"XTHTG QJHBRF"			// ЧЕРЕП ЙОРИКА					YORICK'S SKULL
#define TXT_ARTIPUZZGEMBIG			"CTHLWT L&CGFHBKF"		// СЕРДЦЕ Д'СПАРИЛА				HEART OF D'SPARIL
#define TXT_ARTIPUZZGEMRED			"HE<BYJDFZ GKFYTNF"		// РУБИНОВАЯ ПЛАНЕТА			RUBY PLANET
#define TXT_ARTIPUZZGEMGREEN1		"BPEVHELYFZ GKFYTNF"	// ИЗУМРУДНАЯ ПЛАНЕТА			EMERALD PLANET
#define TXT_ARTIPUZZGEMGREEN2		"BPEVHELYFZ GKFYTNF"	// ИЗУМРУДНАЯ ПЛАНЕТА			EMERALD PLANET
#define TXT_ARTIPUZZGEMBLUE1		"CFGABHJDFZ GKFYTNF"	// САПФИРОВАЯ ПЛАНЕТА			SAPPHIRE PLANET
#define TXT_ARTIPUZZGEMBLUE2		"CFGABHJDFZ GKFYTNF"	// САПФИРОВАЯ ПЛАНЕТА			SAPPHIRE PLANET
#define TXT_ARTIPUZZBOOK1			"RJLTRC LTVJYF"			// КОДЕКС ДЕМОНА				DAEMON CODEX
#define TXT_ARTIPUZZBOOK2			"$"						// LIBER OSCURA (не переводить, спец. символ "$")
#define TXT_ARTIPUZZSKULL2			"VFCRF GKFVTYB"			// МАСКА ПЛАМЕНИ				FLAME MASK
#define TXT_ARTIPUZZFWEAPON			"GTXFNM DJBNTKZ"		// ПЕЧАТЬ ВОИТЕЛЯ				GLAIVE SEAL
#define TXT_ARTIPUZZCWEAPON			"CDZNFZ HTKBRDBZ"		// СВЯТАЯ РЕЛИКВИЯ				HOLY RELIC
#define TXT_ARTIPUZZMWEAPON			"CBVDJK VFUF"			// СИМВОЛ МАГА					SIGIL OF THE MAGUS
#define TXT_ARTIPUZZGEAR			"XFCJDFZ ITCNTHYZ"		// ЧАСОВАЯ ШЕСТЕРНЯ				CLOCK GEAR
#define TXT_USEPUZZLEFAILED			"PLTCM \"NJ YTDJPVJ:YJ BCGJKMPJDFNM"	// ЗДЕСЬ ЭТО НЕВОЗМОЖНО ИСПОЛЬЗОВАТЬ  	YOU CANNOT USE THIS HERE

// Items

#define TXT_ITEMHEALTH			"RHBCNFKMYSQ AKFRJY"	// КРИСТАЛЬНЫЙ ФЛАКОН		CRYSTAL VIAL
#define TXT_ITEMBAGOFHOLDING	"YJCBKMYFZ RJITKM"		// НОСИЛЬНЫЙ КОШЕЛЬ         BAG OF HOLDING
#define TXT_ITEMSHIELD1			"CTHT<HZYSQ OBN"		// СЕРЕБРЯНЫЙ ЩИТ           SILVER SHIELD
#define TXT_ITEMSHIELD2			"PFXFHJDFYYSQ OBN"		// ЗАЧАРОВАННЫЙ ЩИТ         ENCHANTED SHIELD
#define TXT_ITEMSUPERMAP		"CDBNJR RFHNS"			// СВИТОК КАРТЫ             MAP SCROLL
#define TXT_ARMOR1				"RJKMXEUF"				// КОЛЬЧУГА                 MESH ARMOR
#define TXT_ARMOR2				"CJRJKBYSQ OBN"			// СОКОЛИНЫЙ ЩИТ            FALCON SHIELD
#define TXT_ARMOR3				"GKFNBYJDSQ IKTV"		// ПЛАТИНОВЫЙ ШЛЕМ          PLATINUM HELMET
#define TXT_ARMOR4				"FVEKTN CNHF:F"			// АМУЛЕТ СТРАЖА            AMULET OF WARDING

// Weapons

#define TXT_WEAPON_F2			"NJGJH NBVJYF"					 // ТОПОР ТИМОНА					TIMON'S AXE
#define TXT_WEAPON_F3			"VJKJN DJPVTPLBZ"				 // МОЛОТ ВОЗМЕЗДИЯ                 HAMMER OF RETRIBUTION
#define TXT_WEAPON_F4			"GJCKTLYBQ LJDJL CJ<HFY DJTLBYJ" // ПОСЛЕДНИЙ ДОВОД СОБРАН ВОЕДИНО  QUIETUS ASSEMBLED
#define TXT_WEAPON_C2			"PVTBYSQ GJCJ{"					 // ЗМЕИНЫЙ ПОСОХ                   SERPENT STAFF
#define TXT_WEAPON_C3			"JUYTYYSQ INJHV"				 // ОГНЕННЫЙ ШТОРМ                  FIRESTORM
#define TXT_WEAPON_C4			":TPK LE{JD CJ<HFY DJTLBYJ"		 // ЖЕЗЛ ДУХОВ СОБРАН ВОЕДИНО       WRAITHVERGE ASSEMBLED
#define TXT_WEAPON_M2			"KTLZYST JCRJKRB"				 // ЛЕДЯНЫЕ ОСКОЛКИ					FROST SHARDS
#define TXT_WEAPON_M3			"LEUF CVTHNB"					 // ДУГА СМЕРТИ                     ARC OF DEATH
#define TXT_WEAPON_M4			"RHJDFDSQ <BX CJ<HFY DJTLBYJ"	 // КРОВАВЫЙ БИЧ СОБРАН ВОЕДИНО     BLOODSCOURGE ASSEMBLED
#define TXT_QUIETUS_PIECE		"XFCNM GJCKTLYTUJ LJDJLF"		 // ЧАСТЬ ПОСЛЕДНЕГО ДОВОДА         SEGMENT OF QUIETUS
#define TXT_WRAITHVERGE_PIECE	"XFCNM :TPKF LE{JD"				 // ЧАСТЬ ЖЕЗЛА ДУХОВ               SEGMENT OF WRAITHVERGE
#define TXT_BLOODSCOURGE_PIECE	"XFCNM RHJDFDJUJ <BXF"			 // ЧАСТЬ КРОВАВОГО БИЧА            SEGMENT OF BLOODSCOURGE

// SB_bar.c ----------------------------------------------------------------

#define TXT_CHEATGODON			"HT:BV <JUF"						// РЕЖИМ БОГА							GOD MODE ON
#define TXT_CHEATGODOFF			"HT:BV <JUF JNVTYTY"				// РЕЖИМ БОГА ОТМЕНЕН					GOD MODE OFF
#define TXT_CHEATNOCLIPON		"GHJ{J:LTYBT XTHTP CNTYS"			// ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ				NO CLIPPING ON
#define TXT_CHEATNOCLIPOFF		"GHJ{J:LTYBT XTHTP CNTYS JNVTYTYJ"	// ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ ОТМЕНЕНО		NO CLIPPING OFF
#define TXT_CHEATWEAPONS		"GJKYJT DJJHE:TYBT"					// ПОЛНОЕ ВООРУЖЕНИЕ					ALL WEAPONS
#define TXT_CHEATHEALTH			"PLJHJDMT DJCCNFYJDKTYJ"			// ЗДОРОВЬЕ ВОССТАНОВЛЕНО				FULL HEALTH
#define TXT_CHEATKEYS			"DCT RK>XB"							// ВСЕ КЛЮЧИ							ALL KEYS
#define TXT_CHEATSOUNDON		"JNKFLRF PDERF DRK>XTYF"			// ОТЛАДКА ЗВУКА ВКЛЮЧЕНА				SOUND DEBUG ON
#define TXT_CHEATSOUNDOFF		"JNKFLRF PDERF JNRK>XTYF"			// ОТЛАДКА ЗВУКА ОТКЛЮЧЕНА				SOUND DEBUG OFF
#define TXT_CHEATTICKERON		"CXTNXBR RFLHJDJQ XFCNJNS DRK>XTY"	// СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ВКЛЮЧЕН		TICKER ON
#define TXT_CHEATTICKEROFF		"CXTNXBR RFLHJDJQ XFCNJNS JNRK>XTY"	// СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ОТКЛЮЧЕН	TICKER OFF
#define TXT_CHEATARTIFACTS3		"DCT FHNTAFRNS"						// ВСЕ АРТЕФАКТЫ						ALL ARTIFACTS
#define TXT_CHEATARTIFACTSFAIL	"YTRJHHTRNYSQ DDJL"					// НЕКОРРЕНТНЫЙ ВВОД					BAD INPUT
#define TXT_CHEATWARP			"GTHTVTOTYBT YF EHJDTYM"			// ПЕРЕМЕЩЕНИЕ НА УРОВЕНЬ				LEVEL WARP
#define TXT_CHEATSCREENSHOT		"CYBVJR \"RHFYF"					// СНИМОК ЭКРАНА						SCREENSHOT
#define TXT_CHEATIDDQD			"GSNFTIMCZ C{BNHBNM? NFR EVHB :T!"	// ПЫТАЕШЬСЯ СХИТРИТЬ? ТАК УМРИ ЖЕ!		TRYING TO CHEAT, EH?  NOW YOU DIE!
#define TXT_CHEATIDKFA			":EKBR, NS YT LJCNJBY CDJTUJ JHE:BZ"// ЖУЛИК, ТЫ НЕ ДОСТОИН СВОЕГО ОРУЖИЯ	CHEATER - YOU DON'T DESERVE WEAPONS
#define TXT_CHEATBADINPUT		"YTRJHHTRNYSQ DDJL"					// НЕКОРРЕНТНЫЙ ВВОД					BAD INPUT
#define TXT_CHEATNOMAP			"EHJDTYM YT YFQLTY"					// УРОВЕНЬ НЕ НАЙДЕН					CAN'T FIND MAP

// G_game.c ----------------------------------------------------------------

#define TXT_GAMESAVED			"BUHF CJ{HFYTYF"	// ИГРА СОХРАНЕНА						GAME SAVED

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

#define AMSTR_FOLLOWON		"HT:BV CKTLJDFYBZ DRK>XTY"		// РЕЖИМ СЛЕДОВАНИЯ ВКЛЮЧЕН			FOLLOW MODE ON
#define AMSTR_FOLLOWOFF		"HT:BV CKTLJDFYBZ JNRK>XTY"		// РЕЖИМ СЛЕДОВАНИЯ ОТКЛЮЧЕН		FOLLOW MODE OFF

// [JN] Специфические сообщения порта

#define TXT_VERSION "DTHCBZ 2.4 (D HFPHF<JNRT)"       // ВЕРСИЯ 2.4 (В РАЗРАБОТКЕ)

#define TXT_ALWAYSRUN_ON "GJCNJZYYSQ <TU DRK>XTY"     // ПОСТОЯННЫЙ БЕГ ВКЛЮЧЕН
#define TXT_ALWAYSRUN_OFF "GJCNJZYYSQ <TU DSRK>XTY"   // ПОСТОЯННЫЙ БЕГ ВЫКЛЮЧЕН

#define TXT_MLOOK_ON "J<PJH VSIM> DRK>XTY"            // ОБЗОР МЫШЬЮ ВКЛЮЧЕН
#define TXT_MLOOK_OFF "J<PJH VSIM> DSRK>XTY"          // ОБЗОР МЫШЬЮ ВЫКЛЮЧЕН
