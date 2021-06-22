//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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


#ifndef __RD_LANG__
#define __RD_LANG__


extern void RD_DefineLanguageStrings ();


//
// am_map.c
//

extern char *amstr_followon;
extern char *amstr_followoff;
extern char *amstr_gridon;
extern char *amstr_gridoff;
extern char *amstr_markedspot;
extern char *amstr_markscleared;

#define AMSTR_FOLLOWON      "Follow Mode ON"
#define AMSTR_FOLLOWOFF     "Follow Mode OFF"
#define AMSTR_GRIDON        "Grid ON"
#define AMSTR_GRIDOFF       "Grid OFF"
#define AMSTR_MARKEDSPOT    "Marked Spot"
#define AMSTR_MARKSCLEARED  "Last Mark Cleared" // [STRIFE]

#define AMSTR_FOLLOWON_RUS      "ht;bv cktljdfybz drk.xty"  // Режим следования включен
#define AMSTR_FOLLOWOFF_RUS     "ht;bv cktljdfybz dsrk.xty" // Режим следования выключен
#define AMSTR_GRIDON_RUS        "ctnrf drk.xtyf"            // Сетка включена
#define AMSTR_GRIDOFF_RUS       "ctnrf jnrk.xtyf"           // Сетка выключена
#define AMSTR_MARKEDSPOT_RUS    "jnvtnrf"                   // Отметка
#define AMSTR_MARKSCLEARED_RUS  "gjcktlyzz jnvtnrf jxbotyf" // Последняя отметка очищена

//
// d_main.c
//

extern char *d_devstr;
extern char *d_cdrom;

#define D_DEVSTR        "Development mode ON.\n"
#define D_CDROM         "CD-ROM Version: Accessing strife.cd\n"

#define D_DEVSTR_RUS    "Активирован режим разработчика.\n"
#define D_CDROM_RUS     "Дисковая версия.\n"

//
// g_game.c
//

extern char *ststr_alwrun_on;
extern char *ststr_alwrun_off;

#define STSTR_ALWRUNON  "Always run ON"
#define STSTR_ALWRUNOFF "Always run OFF"

#define STSTR_ALWRUNON_RUS  "gjcnjzyysq ,tu drk.xty"    // Постоянный бег включен
#define STSTR_ALWRUNOFF_RUS "gjcnjzyysq ,tu dsrk.xty"   // Постоянный бег выключен

//
// m_menu.c
//

extern char  gammamsg[18][41];
extern char  gammamsg_rus[18][41];
extern char *presskey;
extern char *pressyn;
extern char *quitmsg;
extern char *loadnet;
extern char *qloadnet;
extern char *qsavespot;
extern char *savedead;
extern char *qsprompt;
extern char *qlprompt;
extern char *netend;
extern char *endgame;
extern char *dosy;
extern char *leave;

// Improved (PALFIX) and standard (PLAYPAL) gamma correction
#define GAMMA_IMPROVED_OFF  "Improved gamma correction OFF"
#define GAMMA_IMPROVED_05   "Improved gamma correction level 0.5"
#define GAMMA_IMPROVED_1    "Improved gamma correction level 1.0"
#define GAMMA_IMPROVED_15   "Improved gamma correction level 1.5"
#define GAMMA_IMPROVED_2    "Improved gamma correction level 2.0"
#define GAMMA_IMPROVED_25   "Improved gamma correction level 2.5"
#define GAMMA_IMPROVED_3    "Improved gamma correction level 3.0"
#define GAMMA_IMPROVED_35   "Improved gamma correction level 3.5"
#define GAMMA_IMPROVED_4    "Improved gamma correction level 4.0"
#define GAMMA_ORIGINAL_OFF  "Standard gamma correction OFF"
#define GAMMA_ORIGINAL_05   "Standard gamma correction level 0.5"
#define GAMMA_ORIGINAL_1    "Standard gamma correction level 1.0"
#define GAMMA_ORIGINAL_15   "Standard gamma correction level 1.5"
#define GAMMA_ORIGINAL_2    "Standard gamma correction level 2.0"
#define GAMMA_ORIGINAL_25   "Standard gamma correction level 2.5"
#define GAMMA_ORIGINAL_3    "Standard gamma correction level 3.0"
#define GAMMA_ORIGINAL_35   "Standard gamma correction level 3.5"
#define GAMMA_ORIGINAL_4    "Standard gamma correction level 4.0"

#define PRESSKEY        "press a key."
#define PRESSYN         "press y or n."
#define QUITMSG         "are you sure you want to\nquit this great game?"
#define LOADNET         "you can't load while in a net game!\n\n"PRESSKEY
#define QLOADNET        "you can't quickload during a netgame!\n\n"PRESSKEY
#define QSAVESPOT       "you haven't picked a\nquicksave slot yet!\n\n"PRESSKEY
#define SAVEDEAD        "you're not playing a game\n\n"PRESSKEY
#define QSPROMPT        "quicksave over your game named\n\n'%s'?\n\n"PRESSYN
#define QLPROMPT        "do you want to quickload\n\n'%s'?\n\n"PRESSYN
#define NETEND          "you can't end a netgame!\n\n"PRESSKEY
#define ENDGAME         "are you sure you want\nto end the game?\n\n"PRESSYN
#define DOSY            "(press y to quit)" // haleyjd 09/11/10: [STRIFE] No "to dos." on this
#define LEAVE           "Do you really want to leave?\n\n"DOSY

// Улучшенная (PALFIX) и стандартная (PLAYPAL) гамма-коррекция 
#define GAMMA_IMPROVED_OFF_RUS  "ekexityyfz ufvvf-rjhhtrwbz"       // Улучшенная гамма-коррекция
#define GAMMA_IMPROVED_05_RUS   "ekexityyfz ufvvf-rjhhtrwbz 0>5"   // Улучшенная гамма-коррекция 0.5
#define GAMMA_IMPROVED_1_RUS    "ekexityyfz ufvvf-rjhhtrwbz 1>0"   // Улучшенная гамма-коррекция 1.0
#define GAMMA_IMPROVED_15_RUS   "ekexityyfz ufvvf-rjhhtrwbz 1>5"   // Улучшенная гамма-коррекция 1.5
#define GAMMA_IMPROVED_2_RUS    "ekexityyfz ufvvf-rjhhtrwbz 2>0"   // Улучшенная гамма-коррекция 2.0
#define GAMMA_IMPROVED_25_RUS   "ekexityyfz ufvvf-rjhhtrwbz 2>5"   // Улучшенная гамма-коррекция 2.5
#define GAMMA_IMPROVED_3_RUS    "ekexityyfz ufvvf-rjhhtrwbz 3>0"   // Улучшенная гамма-коррекция 3.0
#define GAMMA_IMPROVED_35_RUS   "ekexityyfz ufvvf-rjhhtrwbz 3>5"   // Улучшенная гамма-коррекция 3.5
#define GAMMA_IMPROVED_4_RUS    "ekexityyfz ufvvf-rjhhtrwbz 4>0"   // Улучшенная гамма-коррекция 4.0
#define GAMMA_ORIGINAL_OFF_RUS  "cnfylfhnyfz ufvvf-rjhhtrwbz"      // Стандартная гамма-коррекция
#define GAMMA_ORIGINAL_05_RUS   "cnfylfhnyfz ufvvf-rjhhtrwbz 0>5"  // Стандартная гамма-коррекция 0.5
#define GAMMA_ORIGINAL_1_RUS    "cnfylfhnyfz ufvvf-rjhhtrwbz 1>0"  // Стандартная гамма-коррекция 1.0
#define GAMMA_ORIGINAL_15_RUS   "cnfylfhnyfz ufvvf-rjhhtrwbz 1>5"  // Стандартная гамма-коррекция 1.5
#define GAMMA_ORIGINAL_2_RUS    "cnfylfhnyfz ufvvf-rjhhtrwbz 2>0"  // Стандартная гамма-коррекция 2.0
#define GAMMA_ORIGINAL_25_RUS   "cnfylfhnyfz ufvvf-rjhhtrwbz 2>5"  // Стандартная гамма-коррекция 2.5
#define GAMMA_ORIGINAL_3_RUS    "cnfylfhnyfz ufvvf-rjhhtrwbz 3>0"  // Стандартная гамма-коррекция 3.0
#define GAMMA_ORIGINAL_35_RUS   "cnfylfhnyfz ufvvf-rjhhtrwbz 3>5"  // Стандартная гамма-коррекция 3.5
#define GAMMA_ORIGINAL_4_RUS    "cnfylfhnyfz ufvvf-rjhhtrwbz 4>0"  // Стандартная гамма-коррекция 4.0

#define PRESSKEY_RUS    "(yf;vbnt k.,e. rkfdbie)"   // (Нажмите любую клавишу)
#define PRESSYN_RUS     "(yf;vbnt ^#^ bkb ^@^)"     // (Нажмите "Y" или "N")
#define QUITMSG_RUS     "ds ltqcndbntkmyj ]jnbnt dsqnb\nbp 'njq pfvtxfntkmyjq buhs?"    // Вы действительно хотите выйти \n из этой замечательной игры?
#define LOADNET_RUS     "ytdjpvj;yj pfuhepbnm cj[hfytyye.\nbuhe d ctntdjq buht!\n\n"PRESSKEY_RUS    // Невозможно загрузить сохраненую \n игру в сетевой игре!
#define QLOADNET_RUS    "ytdjpvj;yj pfuhepbnm ,scnhjt\ncj[hfytybt d ctntdjq buht!\n\n"PRESSKEY_RUS  // Невозможно загрузить быстрое \n сохранение в сетевой игре!
#define QSAVESPOT_RUS   "e dfc yt ds,hfy ckjn ,scnhjuj cj[hfytybz!\n\n"PRESSKEY_RUS         // У Вас не выбран слот быстрого сохранения!
#define SAVEDEAD_RUS    "ytdjpvj;yj cj[hfybnm buhe< yt yfxfd tt!\n\n"PRESSKEY_RUS           // Невозможно сохранить игру, не начав её!
#define QSPROMPT_RUS    "dsgjkybnm gthtpfgbcm\n,scnhjuj cj[hfytybz\n\n^%s^?\n\n"PRESSYN_RUS // Выполнить перезапись \n быстрого сохранения
#define QLPROMPT_RUS    "dsgjkybnm pfuhepre\n,scnhjuj cj[hfytybz\n\n^%s^?\n\n"PRESSYN_RUS   // Выполнить загрузку \n быстрого сохранения
#define NETEND_RUS      "ytdjpvj;yj pfrjyxbnm ctntde. buhe!\n\n"PRESSKEY_RUS                // Невозможно закончить сетевую игру!
#define ENDGAME_RUS     "ds ltqcndbntkmyj [jnbnt\npfrjyxbnm buhe?\n\n"PRESSKEY_RUS          // Вы действительно хотите \n закончить игру?
#define DOSY_RUS        "(yf;vbnt ^#^ xnj,s dsqnb)"                                         // (Нажмите "Y" чтобы выйти)
#define LEAVE_RUS       "ds ltqcndbntkmyj [jnbnt\ndsqnb bp buhs?\n\n"DOSY_RUS               // Вы действительно хотите \n выйти из игры?

//
// hu_stuff.c
//

#define EMPTYSTRING	    "---"

// haleyjd 08/31/10: [STRIFE] Strife map names
#define HUSTR_1         "AREA  1: sanctuary"
#define HUSTR_2         "AREA  2: town"
#define HUSTR_3         "AREA  3: front base"
#define HUSTR_4         "AREA  4: power station"
#define HUSTR_5         "AREA  5: prison"
#define HUSTR_6         "AREA  6: sewers"
#define HUSTR_7         "AREA  7: castle"
#define HUSTR_8         "AREA  8: Audience Chamber"
#define HUSTR_9         "AREA  9: Castle: Programmer's Keep"
#define HUSTR_10        "AREA 10: New Front Base"
#define HUSTR_11        "AREA 11: Borderlands"
#define HUSTR_12        "AREA 12: the temple of the oracle"
#define HUSTR_13        "AREA 13: Catacombs"
#define HUSTR_14        "AREA 14: mines"
#define HUSTR_15        "AREA 15: Fortress: Administration"
#define HUSTR_16        "AREA 16: Fortress: Bishop's Tower"
#define HUSTR_17        "AREA 17: Fortress: The Bailey"
#define HUSTR_18        "AREA 18: Fortress: Stores"
#define HUSTR_19        "AREA 19: Fortress: Security Complex"
#define HUSTR_20        "AREA 20: Factory: Receiving"
#define HUSTR_21        "AREA 21: Factory: Manufacturing"
#define HUSTR_22        "AREA 22: Factory: Forge"
#define HUSTR_23        "AREA 23: Order Commons"
#define HUSTR_24        "AREA 24: Factory: Conversion Chapel"
#define HUSTR_25        "AREA 25: Catacombs: Ruined Temple"
#define HUSTR_26        "AREA 26: proving grounds"
#define HUSTR_27        "AREA 27: The Lab"
#define HUSTR_28        "AREA 28: Alien Ship"
#define HUSTR_29        "AREA 29: Entity's Lair"
#define HUSTR_30        "AREA 30: Abandoned Front Base"
#define HUSTR_31        "AREA 31: Training Facility"
#define HUSTR_32        "AREA  1: Sanctuary"
#define HUSTR_33        "AREA  2: Town"
#define HUSTR_34        "AREA  3: Movement Base"

// haleyjd 20110219: [STRIFE] replaced all with Strife chat macros:
#define HUSTR_CHATMACRO1        "Fucker!"
#define HUSTR_CHATMACRO2        "--SPLAT-- Instant wall art."
#define HUSTR_CHATMACRO3        "That had to hurt!"
#define HUSTR_CHATMACRO4        "Smackings!"
#define HUSTR_CHATMACRO5        "Gib-O-Matic baby."
#define HUSTR_CHATMACRO6        "Burn!  Yah! Yah!"
#define HUSTR_CHATMACRO7        "Buh-Bye!"
#define HUSTR_CHATMACRO8        "Sizzle chest!"
#define HUSTR_CHATMACRO9        "That sucked!"
#define HUSTR_CHATMACRO0        "Mommy?"
#define HUSTR_TALKTOSELF1       "You mumble to yourself"
#define HUSTR_TALKTOSELF2       "Who's there?"
#define HUSTR_TALKTOSELF3       "You scare yourself"
#define HUSTR_TALKTOSELF4       "You start to rave"
#define HUSTR_TALKTOSELF5       "You've lost it..."

#define HUSTR_1_RUS     "vtcnyjcnm  1: cdznbkbot"                    // [rus] МЕСТНОСТЬ 1: Святилище
#define HUSTR_2_RUS     "vtcnyjcnm  2: ujhjl"                        // [rus] МЕСТНОСТЬ 2: Город
#define HUSTR_3_RUS     "vtcnyjcnm  3: ,fpf cjghjnbdktybz"           // [rus] МЕСТНОСТЬ 3: База Сопротивления
#define HUSTR_4_RUS     "vtcnyjcnm  4: 'ktrnhjcnfywbz"               // [rus] МЕСТНОСТЬ 4: Электростанция
#define HUSTR_5_RUS     "vtcnyjcnm  5: n.hmvf"                       // [rus] МЕСТНОСТЬ 5: Тюрьма
#define HUSTR_6_RUS     "vtcnyjcnm  6: rfyfkbpfwbz"                  // [rus] МЕСТНОСТЬ 6: Канализация
#define HUSTR_7_RUS     "vtcnyjcnm  7: pfvjr"                        // [rus] МЕСТНОСТЬ 7: Замок
#define HUSTR_8_RUS     "vtcnyjcnm  8: ghbtvyfz"                     // [rus] МЕСТНОСТЬ 8: Приемная
#define HUSTR_9_RUS     "vtcnyjcnm  9: pfvjr (wbnfltkm ghjuhfvvbcnf)"// [rus] МЕСТНОСТЬ 9: Замок (Цитадель Программиста)
#define HUSTR_10_RUS    "vtcnyjcnm 10: yjdfz ,fpf cjgh>"            // [rus] МЕСТНОСТЬ 10: Новая база Сопр.
#define HUSTR_11_RUS    "vtcnyjcnm 11: gjuhfybxmt"                  // [rus] МЕСТНОСТЬ 11: Пограничье
#define HUSTR_12_RUS    "vtcnyjcnm 12: [hfv jhfrekf"                // [rus] МЕСТНОСТЬ 12: Храм Оракула
#define HUSTR_13_RUS    "vtcnyjcnm 13: rfnfrjv,s"                   // [rus] МЕСТНОСТЬ 13: Катакомбы
#define HUSTR_14_RUS    "vtcnyjcnm 14: if[ns"                       // [rus] МЕСТНОСТЬ 14: Шахты
#define HUSTR_15_RUS    "vtcnyjcnm 15: rhtgjcnm (flvbybcnhfwbz)"    // [rus] МЕСТНОСТЬ 15: Крепость (Администрация)
#define HUSTR_16_RUS    "vtcnyjcnm 16: rhtgjcnm (,fiyz tgbcrjgf)"   // [rus] МЕСТНОСТЬ 16: Крепость (Башня Епископа)
#define HUSTR_17_RUS    "vtcnyjcnm 17: rhtgjcnm (ldjh)"             // [rus] МЕСТНОСТЬ 17: Крепость (Двор)
#define HUSTR_18_RUS    "vtcnyjcnm 18: rhtgjcnm (crkfls)"           // [rus] МЕСТНОСТЬ 18: Крепость (Склады)
#define HUSTR_19_RUS    "vtcnyjcnm 19: rhtgjcnm (j[h> rjvgktrc)"    // [rus] МЕСТНОСТЬ 19: Крепость (Охр. комплекс)
#define HUSTR_20_RUS    "vtcnyjcnm 20: af,hbrf (ghbtv)" // [rus] МЕСТНОСТЬ 20: Фабрика (Прием)
#define HUSTR_21_RUS    "vtcnyjcnm 21: af,hbrf (j,hf,jnrf)" // [rus] МЕСТНОСТЬ 21: Фабрика (Обработка)
#define HUSTR_22_RUS    "vtcnyjcnm 22: af,hbrf (repyz)" // [rus] МЕСТНОСТЬ 22: Фабрика (Кузня)
#define HUSTR_23_RUS    "vtcnyjcnm 23: gjctktybt jhltyf" // [rus] МЕСТНОСТЬ 23: Поселение Ордена
#define HUSTR_24_RUS    "vtcnyjcnm 24: xfcjdyz j,hfotybz" // [rus] МЕСТНОСТЬ 24: Часовня обращения
#define HUSTR_25_RUS    "vtcnyjcnm 25: rfnfrjv,s (hebys [hfvf)" // [rus] МЕСТНОСТЬ 25: Катакомбы (Руины храма)
#define HUSTR_26_RUS    "vtcnyjcnm 26: bcgsnfntkmysq gjkbujy" // [rus] МЕСТНОСТЬ 26: Испытательный полигон
#define HUSTR_27_RUS    "vtcnyjcnm 27: kf,jhfnjhbz" // [rus] МЕСТНОСТЬ 27: Лаборатория
#define HUSTR_28_RUS    "vtcnyjcnm 28: rjhf,km ghbitkmwtd" // [rus] МЕСТНОСТЬ 28: Корабль пришельцев
#define HUSTR_29_RUS    "vtcnyjcnm 29: kjujdj ceoyjcnb" // [rus] МЕСТНОСТЬ 29: Логово Сущности
#define HUSTR_30_RUS    "vtcnyjcnm 30: pf,hjityyfz ,fpf cjgh>" // [rus] МЕСТНОСТЬ 30: Заброшенная база Сопр.
#define HUSTR_31_RUS    "vtcnyjcnm 31: nhtybhjdjxysq rjhgec" // [rus] МЕСТНОСТЬ 31: Тренировочный корпус
#define HUSTR_32_RUS    "vtcnyjcnm  1: cdznbkbot" // [rus] МЕСТНОСТЬ 1: Святилище
#define HUSTR_33_RUS    "vtcnyjcnm  2: ujhjl" // [rus] МЕСТНОСТЬ 2: Город
#define HUSTR_34_RUS    "vtcnyjcnm  3: ,fpf cjghjnbdktybz" // [rus] МЕСТНОСТЬ 3: База сопротивления


//
// p_saveg.c
//

#define SAVEGAMENAME "strife-save-"


//
// st_stuff.c
//

extern char *ststr_mus;
extern char *ststr_nomus;
extern char *ststr_dqdon;
extern char *ststr_dqdoff;
extern char *ststr_faadded;
extern char *ststr_ncon;
extern char *ststr_ncoff;
extern char *ststr_behold;
extern char *ststr_beholdx;
extern char *ststr_clev;

#define STSTR_MUS           "Music Change"
#define STSTR_NOMUS         "IMPOSSIBLE SELECTION"
#define STSTR_DQDON         "You're Invincible!"   // [STRIFE]
#define STSTR_DQDOFF        "You're a looney!"     // [STRIFE]
#define STSTR_FAADDED       "Ammo Added"           // [STRIFE]
#define STSTR_NCON          "No Clipping Mode ON"
#define STSTR_NCOFF         "No Clipping Mode OFF"
#define STSTR_BEHOLD        "Bzrk, Inviso, Mask, Health, Pack, Stats"  // [STRIFE]
#define STSTR_BEHOLDX       "Power-up Toggled"
#define STSTR_CLEV          "Changing Level..."

#define STSTR_MUS_RUS       "cvtyf vepsrb"         // Смена музыки
#define STSTR_NOMUS_RUS     "ds,jh ytdjpvj;ty"     // Выбор невозможен
#define STSTR_DQDON_RUS     "ds ytezpdbvs!"        // Вы неуязвимы!
#define STSTR_DQDOFF_RUS    "ds ghtdjc[jlys!"      // Вы превосходны!
#define STSTR_FAADDED_RUS   "gjkyjt djjhe;tybt"    // Полное вооружение
#define STSTR_NCON_RUS      "ghj[j;ltybt xthtp cntys drk.xtyj"  // Прохождение через стены включено
#define STSTR_NCOFF_RUS     "ghj[j;ltybt xthtp cntys jnvtytyj"  // Прохождение через стены отменено
#define STSTR_BEHOLD_RUS    "b-,thcthr< I-ytdblbvjcnm< M-vfcrf< H-pljhjdmt< P-h.rpfr, S-lfyyst>"  // B-берсерк, I-невидимость, M-маска, H-здоровье, P-рюкзак, S-данные.
#define STSTR_BEHOLDX_RUS   "ghtlvtn gjkexty"      // Предмет получен
#define STSTR_CLEV_RUS      "cvtyf ehjdyz>>>"      // Смена уровня...

// English
#if defined (_WIN64)
#define STSRT_ARCH      " - (x64)"
#elif defined (_WIN32)
#define STSRT_ARCH      " - (x86)"
#else
#define STSRT_ARCH      ""
#endif

// // Русский
#if defined (_WIN64)
#define STSRT_ARCH_RUS      " - ([64)"      // x64
#elif defined (_WIN32)
#define STSRT_ARCH_RUS      " - ([86)"      // x86
#else
#define STSRT_ARCH_RUS      ""              // ?
#endif

#endif
