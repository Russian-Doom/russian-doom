//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2020 Julian Nechaevsky
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
// DESCRIPTION:
//	Printed strings for translation.
//	English language support (default).
//

// Памятка по символам:
// --------------------
// < = ,
// > = .
// $ = BFG
// # = y 
// @ = n
// ^ = "

#ifndef __D_ENGLSH__
#define __D_ENGLSH__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define D_DEVSTR	"Активирован режим разработчика.\n"
#define D_CDROM		"Дисковая версия: файлы конфигурации сохранятся в папке c:\\doomdata\n"

//
//	M_Menu.C
//
#define PRESSKEY    "(yf;vbnt k.,e. rkfdbie)"   // (Нажмите любую клавишу.)
#define PRESSYN     "(yf;vbnt ^#^ bkb ^@^)"     // (Нажмите "Y" или "N".)
#define DOSY        "(yf;vbnt ^#^ xnj,s dsqnb)" // (Нажмите "Y" чтобы выйти.)

#define QUITMSG     "ds ltqcndbntkmyj \[jnbnt dsqnb\nbp 'njq pfvtxfntkmyjq buhs?"            // Вы действительно хотите выйти \n из этой замечательной игры?
#define LOADNET     "ytdjpvj;yj pfuhepbnm cj[hfytyye.\nbuhe d ctntdjq buht!\n\n"PRESSKEY    // Невозможно загрузить сохраненую \n игру в сетевой игре!
#define QLOADNET    "ytdjpvj;yj pfuhepbnm ,scnhjt\ncj[hfytybt d ctntdjq buht!\n\n"PRESSKEY  // Невозможно загрузить быстрое \n сохранение в сетевой игре!
#define QSAVESPOT   "e dfc yt ds,hfy ckjn ,scnhjuj cj[hfytybz!\n\n"PRESSKEY                 // У Вас не выбран слот быстрого сохранения!
#define SAVEDEAD    "ytdjpvj;yj cj[hfybnm buhe< yt yfxfd tt!\n\n"PRESSKEY                   // Невозможно сохранить игру, не начав её!
#define QSPROMPT    "gthtpfgbcfnm ,scnhjt cj[hfytybt\n^%s^?\n\n"PRESSYN                     // quicksave over your game named\n\n'%s'?\n\n
#define QLPROMPT    "pfuhepbnm ,scnhjt cj[hfytybt\n^%s^?\n\n"PRESSYN                        // do you want to quickload the game named\n'%s'?\n\n

#define RD_ON       "drk"       // вкл
#define RD_OFF      "dsrk"      // выкл
#define RD_NEXT     "lfktt )"   // далее >
#define RD_PREV     "( yfpfl"   // < назад

// Внутриигровые настройки будут
// сброшены на стандартные значения.
//
// Уверены, что хотите продолжить?
#define RD_DEFAULTS                     \
"dyenhbbuhjdst yfcnhjqrb ,elen\n"       \
"c,hjitys yf cnfylfhnyst pyfxtybz>\n"   \
"\n"                                    \
"edthtys< xnj [jnbnt ghjljk;bnm?\n\n"   \
PRESSYN

// Невозможно начать новую игру
// при активной сетевой игре.
#define NEWGAME	\
"ytdjpvj;yj yfxfnm yjde. buhe\n"\
"ghb frnbdyjq ctntdjq buht>\n\n"PRESSKEY 

// Уверены? этот уровень сложности
// даже не близок к честному.
#define NIGHTMARE	\
"edthtys? 'njn ehjdtym ckj;yjcnb\n"\
"lf;t yt ,kbpjr r xtcnyjve>\n\n"PRESSYN 

// Монстры не воскрешаются, однако
// их скорость и диапазон урона,
// наносимого игроку, увеличены.
//
// Уверены, что сможете выжить?
#define ULTRANM	\
"vjycnhs yt djcrhtif.ncz< jlyfrj\n"\
"b[ crjhjcnm b lbfgfpjy ehjyf<\n"\
"yfyjcbvjuj buhjre< edtkbxtys>\n\n"\
"edthtys< xnj cvj;tnt ds;bnm?\n\n"PRESSYN

// Это демонстрационная версия игры.
// Вам необходимо приобрести всю трилогию.
#define SWSTRING	\
"'nj ltvjycnhfwbjyyfz dthcbz buhs>\n\n"\
"dfv ytj,[jlbvj ghbj,htcnb dc. nhbkjub.>\n\n"PRESSKEY 

#define MSGOFF  "cjj,otybz dsrk.xtys"                                   // Сообщения выключены
#define MSGON   "cjj,otybz drk.xtys"                                    // Сообщения включены
#define NETEND  "ytdjpvj;yj pfrjyxbnm ctntde. buhe!\n\n"PRESSKEY        // Невозможно закончить сетевую игру!
#define ENDGAME "ds ltqcndbntkmyj [jnbnt pfrjyxbnm buhe?\n\n"PRESSYN    // Вы действительно хотите закончить игру?

#define DETAILHI    "dscjrfz ltnfkbpfwbz"       // Высокая детализация
#define DETAILLO    "ybprfz ltnfkbpfwbz"        // Низкая детализация

// Improved (PALFIX) and standard (PLAYPAL) gamma-correction
#define GAMMA_IMPROVED_OFF  "ekexityyfz ufvvf-rjhhtrwbz jnrk.xtyf"     // Улучшенная гамма-коррекция отключена
#define GAMMA_IMPROVED_05   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 0>5"   // Уровень улучшенной гамма-коррекции 0.5
#define GAMMA_IMPROVED_1    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 1>0"   // Уровень улучшенной гамма-коррекции 1.0
#define GAMMA_IMPROVED_15   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 1>5"   // Уровень улучшенной гамма-коррекции 1.5
#define GAMMA_IMPROVED_2    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 2>0"   // Уровень улучшенной гамма-коррекции 2.0
#define GAMMA_IMPROVED_25   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 2>5"   // Уровень улучшенной гамма-коррекции 2.5
#define GAMMA_IMPROVED_3    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 3>0"   // Уровень улучшенной гамма-коррекции 3.0
#define GAMMA_IMPROVED_35   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 3>5"   // Уровень улучшенной гамма-коррекции 3.5
#define GAMMA_IMPROVED_4    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 4>0"   // Уровень улучшенной гамма-коррекции 4.0
#define GAMMA_ORIGINAL_OFF  "cnfylfhnyfz ufvvf-rjhhtrwbz jnrk.xtyf"    // Стандартная гамма-коррекция отключена
#define GAMMA_ORIGINAL_05   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 0>5"  // Уровень стандартной гамма-коррекции 0.5
#define GAMMA_ORIGINAL_1    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 1>0"  // Уровень стандартной гамма-коррекции 1.0
#define GAMMA_ORIGINAL_15   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 1>5"  // Уровень стандартной гамма-коррекции 1.5
#define GAMMA_ORIGINAL_2    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 2>0"  // Уровень стандартной гамма-коррекции 2.0
#define GAMMA_ORIGINAL_25   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 2>5"  // Уровень стандартной гамма-коррекции 2.5
#define GAMMA_ORIGINAL_3    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 3>0"  // Уровень стандартной гамма-коррекции 3.0
#define GAMMA_ORIGINAL_35   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 3>5"  // Уровень стандартной гамма-коррекции 3.5
#define GAMMA_ORIGINAL_4    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 4>0"  // Уровень стандартной гамма-коррекции 4.0

#define EMPTYSTRING "gecnjq ckjn"                  // Пустой слот
#define SECRETFOUND "j,yfhe;ty nfqybr!"            // Обнаружен тайник!
#define CHEATDENIED "dsgjkytybt rjvfyls pfghtotyj" // Выполнение команды запрещено

//
//	P_inter.C
//

#define GOTARMOR    "gjkextyf ,hjyz>"           // Получена броня.
#define GOTMEGA     "gjkextyf vtuf,hjyz!"       // Получена мегаброня!
#define GOTHTHBONUS "gjkexty ,jyec pljhjdmz>"   // Получен бонус здоровья.
#define GOTARMBONUS "gjkexty ,jyec ,hjyb>"      // Получен бонус брони.
#define GOTSTIM     "gjkexty cnbvekznjh>"       // Получен стимулятор.
#define GOTMEDIKIT  "gjkextyf fgntxrf>"         // Получена аптечка.
#define GOTSUPER    "cdth[pfhzl!"               // Сверхзаряд!
#define GOTMEDINEED "gjkextyf rhfqyt ytj,[jlbvfz fgntxrf!"  // Получена крайне необходимая аптечка!

#define GOTBLUECARD "gjkextyf cbyzz rk.x-rfhnf>"    // Получена синяя ключ-карта.
#define GOTYELWCARD "gjkextyf ;tknfz rk.x-rfhnf>"   // Получена желтая ключ-карта
#define GOTREDCARD  "gjkextyf rhfcyfz rk.x-rfhnf>"  // Получена красная ключ-карта.
#define GOTBLUESKUL "gjkexty cbybq rk.x-xthtg>"     // Получен синий ключ-череп.
#define GOTYELWSKUL "gjkexty ;tknsq rk.x-xthtg>"    // Получен желтый ключ-череп.
#define GOTREDSKULL "gjkexty rhfcysq rk.x-xthtg>"   // Получен красный ключ-череп.

#define GOTINVUL    "ytezpdbvjcnm!"                 // Неуязвимость!
#define GOTBERSERK  ",thcthr!"                      // Берсерк!
#define GOTINVIS    "xfcnbxyfz ytdblbvjcnm>"        // Частичная невидимость.
#define GOTSUIT	    "rjcn.v hflbfwbjyyjq pfobns>"   // Костюм радиационной защиты.
#define GOTMAP      "rjvgm.nthyfz rfhnf ehjdyz>"    // Компьютерная карта уровня.
#define GOTVISOR    "dbpjh ecbktybz jcdtotybz>"     // Визор усиления освещения.
#define GOTMSPHERE  "vtufcathf!"                    // Мегасфера!

#define GOTCLIP     "gjkextyf j,jqvf>"                          // Получена обойма.
#define GOTCLIPBOX  "gjkextyf rjhj,rf gfnhjyjd>"                // Получена коробка патронов.
#define GOTROCKET   "gjkextyf hfrtnf>"                          // Получена ракета.
#define GOTROCKET2  "gjkextyj ldt hfrtns>"                      // Получено две ракеты.
#define GOTROCKBOX  "gjkexty zobr hfrtn>"                       // Получен ящик ракет.
#define GOTCELL     "gjkextyf 'ythuj,fnfhtz>"                   // Получена энергобатарея.
#define GOTCELLBOX  "gjkexty 'ythujfrrevekznjh>"                // Получен энергоаккумулятор.
#define GOTSHELLS   "gjkextyj 4 gfnhjyf lkz lhj,jdbrf>"         // Получено 4 патрона для дробовика.
#define GOTSHELLS8  "gjkextyj 8 gfnhjyjd lkz lhj,jdbrf>"        // Получено 8 патронов для дробовика.
#define GOTSHELLBOX "gjkextyf rjhj,rf gfnhjyjd lkz lhj,jdbrf>"  // Получена коробка патронов для дробовика.
#define GOTBACKPACK "gjkexty h.rpfr c fvveybwbtq!"              // Получен рюкзак с амуницией!

#define GOTBFG9000  "gjkextyj $9000! j lf>"         // Получено BFG9000! О да.
#define GOTCHAINGUN "gjkexty gektvtn!"              // Получен пулемет!
#define GOTCHAINSAW ",typjgbkf! yfqlb ytvyjuj vzcf!"// Бензопила! Найди немного мяса!
#define GOTLAUNCHER "gjkextyf hfrtnybwf!"           // Получена ракетница!
#define GOTPLASMA   "gjkextyf gkfpvtyyfz geirf!"    // Получена плазменная пушка!
#define GOTSHOTGUN  "gjkexty lhj,jdbr!"             // Получен дробовик!
#define GOTSHOTGUN2 "gjkextyj ldecndjkmyjt he;mt!"  // Получено двуствольное ружье!

//
// P_Doors.C
//

#define PD_BLUEO    "lkz frnbdfwbb ye;ty cbybq rk.x>"   // Для активации нужен синий ключ.
#define PD_REDO     "lkz frnbdfwbb ye;ty rhfcysq rk.x>" // Для активации нужен красный ключ.
#define PD_YELLOWO  "lkz frnbdfwbb ye;ty ;tknsq rk.x>"  // Для активации нужен желтый ключ.
#define PD_BLUEK    "lkz jnrhsnbz ye;ty cbybq rk.x>"    // Для открытия нужен синий ключ.
#define PD_REDK     "lkz jnrhsnbz ye;ty rhfcysq rk.x>"  // Для открытия нужен красный ключ.
#define PD_YELLOWK  "lkz jnrhsnbz ye;ty ;tknsq rk.x>"   // Для открытия нужен жёлтый ключ.

//
//	G_game.C
//

#define GGSAVED     "buhf cj[hfytyf>"   // Игра сохранена.
#define GGLOADED    "buhf pfuhe;tyf>"   // Игра загружена.

//
//	HU_stuff.C
//

#define HUSTR_MSGU  "(cjj,otybt yt jnghfdktyj)"                 // (Сообщение не отправлено)

//
// DOOM 1
//

#define HUSTR_E1M1  "t1v1: fyufh"                               // E1M1: Ангар
#define HUSTR_E1M2  "t1v2: fnjvyfz 'ktrnhjcnfywbz"              // E1M2: Атомная электростанция
#define HUSTR_E1M3  "t1v3: pfdjl gj gththf,jnrt zlj[bvbrfnjd"   // E1M3: Завод по переработке ядохимикатов
#define HUSTR_E1M4  "t1v4: rjvfylysq geyrn"                     // E1M4: Командный пункт
#define HUSTR_E1M5  "t1v5: kf,jhfnjhbz yf aj,jct"               // E1M5: Лаборатория на Фобосе
#define HUSTR_E1M6  "t1v6: wtynhfkmysq geyrn j,hf,jnrb"         // E1M6: Центральный пункт обработки
#define HUSTR_E1M7  "t1v7: dsxbckbntkmysq wtynh"                // E1M7: Вычислительный центр
#define HUSTR_E1M8  "t1v8: fyjvfkbz yf aj,jct"                  // E1M8: Аномалия на Фобосе
#define HUSTR_E1M9  "t1v9: djtyyfz ,fpf"                        // E1M9: Военная база

#define HUSTR_E2M1  "t2v1: fyjvfkbz yf ltqvjct"                 // E2M1: Аномалия на Деймосе
#define HUSTR_E2M2  "t2v2: [hfybkbot"                           // E2M2: Хранилище
#define HUSTR_E2M3  "t2v3: jxbcnbntkmysq pfdjl"                 // E2M3: Очистительный завод
#define HUSTR_E2M4  "t2v4: kf,jhfnjhbz yf ltqvjct"              // E2M4: Лаборатория на Деймосе
#define HUSTR_E2M5  "t2v5: rjvfylysq wtynh"                     // E2M5: Командный центр
#define HUSTR_E2M6  "t2v6: pfks ghjrkzns["                      // E2M6: Залы проклятых
#define HUSTR_E2M7  "t2v7: ythtcnbkbot"                         // E2M7: Нерестилище
#define HUSTR_E2M8  "t2v8: dfdbkjycrfz ,fiyz"                   // E2M8: Вавилонская башня
#define HUSTR_E2M9  "t2v9: rhtgjcnm nfqy"                       // E2M9: Крепость тайн

#define HUSTR_E3M1  "t3v1: rhtgjcnm flf"                        // E3M1: Крепость Ада
#define HUSTR_E3M2  "t3v2: nhzcbyf jnxfzybz"                    // E3M2: Трясина отчаяния
#define HUSTR_E3M3  "t3v3: gfyltvjybq"                          // E3M3: Пандемоний
#define HUSTR_E3M4  "t3v4: ljv ,jkb"                            // E3M4: Дом боли
#define HUSTR_E3M5  "t3v5: ytxtcnbdsq cj,jh"                    // E3M5: Нечестивый собор
#define HUSTR_E3M6  "t3v6: ujhf 'ht,"                           // E3M6: Гора Эреб
#define HUSTR_E3M7  "t3v7: dhfnf d kbv,"                        // E3M7: Врата в Лимб
#define HUSTR_E3M8  "t3v8: lbn"                                 // E3M8: Дит
#define HUSTR_E3M9  "t3v9: rhjkbxbq cfl"                        // E3M9: Кроличий сад

#define HUSTR_E4M1  "t4v1: gjl fljv"                            // E4M1: Под Адом
#define HUSTR_E4M2  "t4v2: bcnbyyfz ytyfdbcnm"                  // E4M2: Истинная ненависть
#define HUSTR_E4M3  "t4v3: bcnzpfybt ytxtcnbds["                // E4M3: Истязание нечестивых
#define HUSTR_E4M4  "t4v4: ytelth;bvjt pkj"                     // E4M4: Неудержимое зло
#define HUSTR_E4M5  "t4v5: jyb gjrf.ncz"                        // E4M5: Они покаются
#define HUSTR_E4M6  "t4v6: ceghjnbd cb[ ytxtcnbds["             // E4M6: Супротив сих нечестивых
#define HUSTR_E4M7  "t4v7: b gjcktljdfk fl"                     // E4M7: И последовал Ад
#define HUSTR_E4M8  "t4v8: lj ,tccthltxbz"                      // E4M8: До бессердечия
#define HUSTR_E4M9  "t4v9: cnhf["                               // E4M9: Страх

//
// DOOM2: Ад на Земле
//

#define HUSTR_1     "ehjdtym 1: gfhflysq d[jl"                  // Уровень 1: Парадный вход
#define HUSTR_2     "ehjdtym 2: gjlptvyst pfks"                 // Уровень 2: Подземные залы
#define HUSTR_3     "ehjdtym 3: dspjd ,hjity"                   // Уровень 3: Вызов брошен
#define HUSTR_4     "ehjdtym 4: chtljnjxbt"                     // Уровень 4: Средоточие
#define HUSTR_5     "ehjdtym 5: cnjxyst neyytkb"                // Уровень 5: Сточные туннели
#define HUSTR_6     "ehjdtym 6: ghtcc"                          // Уровень 6: Пресс
#define HUSTR_7     "ehjdtym 7: cvthntkmyj ghjcnjq"             // Уровень 7: Смертельно простой
#define HUSTR_8     "ehjdtym 8: ekjdrb b kjdeirb"               // Уровень 8: Уловки и ловушки
#define HUSTR_9     "ehjdtym 9: zvf"                            // Уровень 9: Яма
#define HUSTR_10    "ehjdtym 10: pfghfdjxyfz ,fpf"              // Уровень 10: Заправочная база
#define HUSTR_11    "ehjdtym 11: ^j^ hfpheitybz!"               // Уровень 11: "o" разрушения!

#define HUSTR_12    "ehjdtym 12: af,hbrf"                       // Уровень 12: Фабрика
#define HUSTR_13    "ehjdtym 13: ltkjdjq hfqjy"                 // Уровень 13: Деловой район
#define HUSTR_14    "ehjdtym 14: uke,jxfqibt kjujdbof"          // Уровень 14: Глубочайшие логовища
#define HUSTR_15    "ehjdtym 15: ghjvsiktyyfz pjyf"             // Уровень 15: Промышленная зона
#define HUSTR_16    "ehjdtym 16: ghbujhjl"                      // Уровень 16: Пригород 
#define HUSTR_17    "ehjdtym 17: dkfltybz"                      // Уровень 17: Владения 
#define HUSTR_18    "ehjdtym 18: dyenhtyybq ldjh"               // Уровень 18: Внутренний двор
#define HUSTR_19    "ehjdtym 19: wbnfltkm"                      // Уровень 19: Цитадель 
#define HUSTR_20    "ehjdtym 20: gjgfkcz!"                      // Уровень 20: Попался!

#define HUSTR_21    "ehjdtym 21: ybhdfyf"                       // Уровень 21: Нирвана 
#define HUSTR_22    "ehjdtym 22: rfnfrjv,s"                     // Уровень 22: Катакомбы 
#define HUSTR_23    "ehjdtym 23: wtkst ,jxrb dtctkmz"           // Уровень 23: Целые бочки веселья
#define HUSTR_24    "ehjdtym 24: ghjgfcnm"                      // Уровень 24: Пропасть 
#define HUSTR_25    "ehjdtym 25: rhjdjgfls"                     // Уровень 25: Кровопады
#define HUSTR_26    "ehjdtym 26: pf,hjityyst if[ns"             // Уровень 26: Заброшенные шахты
#define HUSTR_27    "ehjdtym 27: rjyljvbybev vjycnhjd"          // Уровень 27: Кондоминиум монстров
#define HUSTR_28    "ehjdtym 28: vbh le[jd"                     // Уровень 28: Мир Духов
#define HUSTR_29    "ehjdtym 29: rjytw dctuj ;bdjuj"            // Уровень 29: Конец всего живого
#define HUSTR_30    "ehjdtym 30: brjyf uht[f"                   // Уровень 30: Икона греха

#define HUSTR_31    "ehjdtym 31: djkmatyinfqy"                  // Уровень 31: Вольфенштайн
#define HUSTR_32    "ehjdtym 32: uhjcct"                        // Уровень 32: Гроссе

//
// DOOM2: Нет покоя для живых
//

#define NHUSTR_1	"ehjdtym 1: ,fpf yf ptvkt"      // Уровень 1: База на Земле
#define NHUSTR_2	"ehjdtym 2: kf,jhfnjhbb ,jkb"   // Уровень 2: Лаборатории боли
#define NHUSTR_3	"ehjdtym 3: rfymjy vthndtwjd"   // Уровень 3: Каньон мертвецов
#define NHUSTR_4	"ehjdtym 4: flcrfz ujhf"        // Уровень 4: Адская гора
#define NHUSTR_5	"ehjdtym 5: dbdbctrwbz"         // Уровень 5: Вивисекция
#define NHUSTR_6	"ehjdtym 6: rhjdm ghtbcgjlyzz"  // Уровень 6: Кровь преисподняя
#define NHUSTR_7	"ehjdtym 7: ,fyrtn e ,fhjyf"    // Уровень 7: Банкет у Барона
#define NHUSTR_8	"ehjdtym 8: uhj,ybwf pkj,s"     // Уровень 8: Гробница злобы
#define NHUSTR_9	"ehjdtym 9: itcndbt ltvjyjd"    // Уровень 9: Шествие демонов

//
// Мастер-Уровни для DOOM 2
//

#define MLSTR_1     "yfgfltybt"                // ATTACK.WAD   - Нападение
#define MLSTR_2     "xthyfz ,fiyz"             // BLACKTWR.WAD - Черная Башня
#define MLSTR_3     "rhtgjcnm d rhjdfdjv vjht" // BLOODSEA.WAD - Крепость в Кровавом море
#define MLSTR_4     "rfymjy"                   // CANYON.WAD   - Каньон
#define MLSTR_5     "gjvjcn"                   // CATWALK.WAD  - Помост
#define MLSTR_6     "rjv,byfn"                 // COMBINE.WAD  - Комбинат
#define MLSTR_7     "abcnekf"                  // FISTULA.WAD  - Фистула
#define MLSTR_8     "ufhybpjy"                 // GARRISON.WAD - Гарнизон
#define MLSTR_9     "uthbjy"                   // GERYON.WAD   - Герион
#define MLSTR_10    "gjvtcnmt ubufynf"         // MANOR.WAD    - Поместье Гиганта
#define MLSTR_11    "vfdpjktq vtabcnjatkz"     // MEPHISTO.WAD - Мавзолей Мефистофеля
#define MLSTR_12    "ghbujdjh vbyjcf"          // MINOS.WAD    - Приговор Миноса
#define MLSTR_13    "ytcc"                     // NESSUS.WAD   - Несс
#define MLSTR_14    "gfhfljrc"                 // PARADOX.WAD  - Парадокс
#define MLSTR_15    "gjlghjcnhfycndj"          // SUBSPACE.WAD - Подпространство
#define MLSTR_16    "gjlptvtkmt"               // SUBTERRA.WAD - Подземелье
#define MLSTR_17    "neh,jkban d ghtbcgjly.."  // TEETH.WAD    - Турболифт в Преисподнюю
#define MLSTR_18    "lehyjq cjy"               //              - Дурной сон
#define MLSTR_19    "pfcnhzdibq yf nbnfyt"     // TTRAP.WAD    - Застрявший на Титане
#define MLSTR_20    "dtxth"                    // VESPERAS.WAD - Вечер
#define MLSTR_21    "djkz dthubkbz"            // VIRGIL.WAD   - Воля Вергилия

//
// Final DOOM: Эксперимент "Плутония"
//

#define PHUSTR_1    "ehjdtym 1: rjyuj"                  // Уровень 1: Конго
#define PHUSTR_2    "ehjdtym 2: rjkjltw lei"            // Уровень 2: Колодец душ
#define PHUSTR_3    "ehjdtym 3: fwntr"                  // Уровень 3: Ацтек
#define PHUSTR_4    "ehjdtym 4: pfgthnsq d rktnrt"      // Уровень 4: Запертый в клетке
#define PHUSTR_5    "ehjdtym 5: ujhjl-ghbphfr"          // Уровень 5: Город-призрак
#define PHUSTR_6    "ehjdtym 6: kjujdj ,fhjyf"          // Уровень 6: Логово Барона
#define PHUSTR_7    "ehjdtym 7: ldjh-kjdeirf"           // Уровень 7: Двор-ловушка
#define PHUSTR_8    "ehjdtym 8: wfhcndj"                // Уровень 8: Царство
#define PHUSTR_9    "ehjdtym 9: f,,fncndj"              // Уровень 9: Аббатство
#define PHUSTR_10   "ehjdtym 10: yfnbcr"                // Уровень 10: Натиск
#define PHUSTR_11   "ehjdtym 11: ghtcktletvsq"          // Уровень 11: Преследуемый

#define PHUSTR_12   "ehjdtym 12: crjhjcnm"              // Уровень 12: Скорость
#define PHUSTR_13   "ehjdtym 13: crktg"                 // Уровень 13: Склеп
#define PHUSTR_14   "ehjdtym 14: pfhj;ltybt"            // Уровень 14: Зарождение
#define PHUSTR_15   "ehjdtym 15: cevthrb"               // Уровень 15: Сумерки
#define PHUSTR_16   "ehjdtym 16: ghtlpyfvtyjdfybt"      // Уровень 16: Предзнаменование
#define PHUSTR_17   "ehjdtym 17: rjvgfeyl"              // Уровень 17: Компаунд
#define PHUSTR_18   "ehjdtym 18: ytqhjcathf"            // Уровень 18: Нейросфера
#define PHUSTR_19   "ehjdtym 19: d>h>f>u>"              // Уровень 19: В.Р.А.Г.
#define PHUSTR_20   "ehjdtym 20: j,bntkm cvthnb"        // Уровень 20: Обитель Смерти

#define PHUSTR_21   "ehjdtym 21: e,bqwf"                // Уровень 21: Убийца
#define PHUSTR_22   "ehjdtym 22: ytdsgjkybvjt pflfybt"  // Уровень 22: Невыполнимое задание
#define PHUSTR_23   "ehjdtym 23: yfluhj,bt"             // Уровень 23: Надгробие
#define PHUSTR_24   "ehjdtym 24: gjcktlybq he,t;"       // Уровень 24: Последний рубеж
#define PHUSTR_25   "ehjdtym 25: [hfv nmvs"             // Уровень 25: Храм тьмы
#define PHUSTR_26   "ehjdtym 26: ,eyrth"                // Уровень 26: Бункер
#define PHUSTR_27   "ehjdtym 27: fynb[hbcn"             // Уровень 27: Антихрист
#define PHUSTR_28   "ehjdtym 28: rfyfkbpfwbz"           // Уровень 28: Канализация
#define PHUSTR_29   "ehjdtym 29: jlbcctz ievjd"         // Уровень 29: Одиссея шумов
#define PHUSTR_30   "ehjdtym 30: dhfnf flf"             // Уровень 30: Врата Ада

#define PHUSTR_31   "ehjdtym 31: rb,thkjujdj"           // Уровень 31: Киберлогово
#define PHUSTR_32   "ehjdtym 32: blb r ctve"            // Уровень 32: Иди к сему

//
// Final DOOM: TNT - Дьяволюция
//

#define THUSTR_1    "ehjdtym 1: wtynh eghfdktybz cbcntvjq"      // Уровень 1: Центр управления системой
#define THUSTR_2    "ehjdtym 2: ,fh,tr. bp xtkjdtxbys"          // Уровень 2: Барбекю из человечины
#define THUSTR_3    "ehjdtym 3: wtynh eghfdktybz gbnfybtv"      // Уровень 3: Центр управления питанием
#define THUSTR_4    "ehjdtym 4: lshf dj dhtvtyb"                // Уровень 4: Дыра во времени
#define THUSTR_5    "ehjdtym 5: dbctkbwf"                       // Уровень 5: Виселица
#define THUSTR_6    "ehjdtym 6: jnrhsnsq ctpjy"                 // Уровень 6: Открытый сезон
#define THUSTR_7    "ehjdtym 7: n.hmvf"                         // Уровень 7: Тюрьма
#define THUSTR_8    "ehjdtym 8: vtnfkk"                         // Уровень 8: Металл
#define THUSTR_9    "ehjdtym 9: rhtgjcnm"                       // Уровень 9: Крепость
#define THUSTR_10   "ehjdtym 10: bcregktybt"                    // Уровень 10: Искупление
#define THUSTR_11   "ehjdtym 11: crkfl"                         // Уровень 11: Склад

#define THUSTR_12   "ehjdtym 12: rhfnth"                        // Уровень 12: Кратер
#define THUSTR_13   "ehjdtym 13: gththf,jnrf zlthys[ jn[jljd"   // Уровень 13: Переработка ядерных отходов
#define THUSTR_14   "ehjdtym 14: cnfktkbntqysq pfdjl"           // Уровень 14: Cталелитейный завод
#define THUSTR_15   "ehjdtym 15: vthndfz pjyf"                  // Уровень 15: Мертвая зона
#define THUSTR_16   "ehjdtym 16: uke,jxfqibt ljcnb;tybz"        // Уровень 16: Глубочайшие достижения
#define THUSTR_17   "ehjdtym 17: pjyf j,hf,jnrb"                // Уровень 17: Зона обработки
#define THUSTR_18   "ehjdtym 18: pfdjl"                         // Уровень 18: Завод
#define THUSTR_19   "ehjdtym 19: gjuheprf/jnghfdrf"             // Уровень 19: Погрузка/отправка
#define THUSTR_20   "ehjdtym 20: wtynhfkmysq geyrn j,hf,jnrb"   // Уровень 20: Центральный пункт обработки

#define THUSTR_21   "ehjdtym 21: flvbybcnhfnbdysq wtynh"        // Уровень 21: Административный центр
#define THUSTR_22   "ehjdtym 22: j,bnfkbot"                     // Уровень 22: Обиталище
#define THUSTR_23   "ehjdtym 23: keyysq ujhysq ghjtrn"          // Уровень 23: Лунный горный проект
#define THUSTR_24   "ehjdtym 24: rfhmth"                        // Уровень 24: Карьер
#define THUSTR_25   "ehjdtym 25: kjujdj ,fhjyf"                 // Уровень 25: Логово Барона
#define THUSTR_26   "ehjdtym 26: ,fkkbcnbrc"                    // Уровень 26: Баллистикс
#define THUSTR_27   "ehjdtym 27: ujhf ,jkm"                     // Уровень 27: Гора Боль
#define THUSTR_28   "ehjdtym 28: xthnjdobyf"                    // Уровень 28: Чертовщина
#define THUSTR_29   "ehjdtym 29: htrf cnbrc"                    // Уровень 29: Река Стикс
#define THUSTR_30   "ehjdtym 30: gjcktlybq dspjd"               // Уровень 30: Последний вызов

#define THUSTR_31   "ehjdtym 31: afhfjy"                        // Уровень 31: Фараон
#define THUSTR_32   "ehjdtym 32: rfhb,s"                        // Уровень 32: Карибы

#define HUSTR_CHATMACRO1    "ujnjd r ,j.!"                  // Готов к бою!
#define HUSTR_CHATMACRO2    "z d gjhzlrt>"                  // Я в порядке.
#define HUSTR_CHATMACRO3    "z hfyty!"                      // Я ранен!
#define HUSTR_CHATMACRO4    "yf gjvjom!"                    // На помощь!
#define HUSTR_CHATMACRO5    "ns - jncnjq!"                  // Ты - отстой!
#define HUSTR_CHATMACRO6    "d cktle.obq hfp< gjljyjr>>>"   // В следующий раз, подонок...
#define HUSTR_CHATMACRO7    "c.lf!"                         // Сюда!
#define HUSTR_CHATMACRO8    "z j, 'njv gjpf,jxecm>"         // Я об этом позабочусь.
#define HUSTR_CHATMACRO9    "lf."                           // Да.
#define HUSTR_CHATMACRO0    "ytn>"                          // Нет.

#define HUSTR_TALKTOSELF1   "ythfp,jhxbdjt ,jhvjnfybt>>>"   // Неразборчивое бормотание...
#define HUSTR_TALKTOSELF2   "rnj nfv?"                      // Кто там?
#define HUSTR_TALKTOSELF3   "xnj 'nj ,skj?"                 // Что это было?
#define HUSTR_TALKTOSELF4   "ds ,htlbnt>"                   // Вы бредите.
#define HUSTR_TALKTOSELF5   "rfrfz ljcflf!"                 // Какая досада!

#define HUSTR_MESSAGESENT   "(cjj,otybt jnghfdktyj)"        // (сообщение отправлено)

// The following should NOT be changed unless it seems
// just AWFULLY necessary
//
// [JN] Таковая необходимость у меня есть :)

#define HUSTR_PLRGREEN  "uhby: "    // Грин:
#define HUSTR_PLRINDIGO "bylbuj: "  // Индиго:
#define HUSTR_PLRBROWN  ",hfey: "   // Браун:
#define HUSTR_PLRRED    "htl: "     // Ред:

#define HUSTR_KEYGREEN  'g'
#define HUSTR_KEYINDIGO 'i'
#define HUSTR_KEYBROWN  'b'
#define HUSTR_KEYRED    'r'

//
//	AM_map.C
//

#define AMSTR_FOLLOWON  "ht;bv cktljdfybz drk.xty"  // Режим следования включен
#define AMSTR_FOLLOWOFF "ht;bv cktljdfybz dsrk.xty" // Режим следования выключен

#define AMSTR_GRIDON    "ctnrf drk.xtyf"            // Сетка включена
#define AMSTR_GRIDOFF   "ctnrf jnrk.xtyf"           // Сетка выключена

#define AMSTR_MARKEDSPOT    "jnvtnrf"               // Отметка
#define AMSTR_MARKSCLEARED  "jnvtnrb jxbotys"       // Отметки очищены

//
//	ST_stuff.C
//

#define STSTR_MUS       "cvtyf vepsrb"                              // Смена музыки
#define STSTR_NOMUS     "ds,jh ytdjpvj;ty"                          // Выбор невозможен
#define STSTR_DQDON     "ytezpdbvjcnm frnbdbhjdfyf"                 // Неуязвимость активирована
#define STSTR_DQDOFF    "ytezpdbvjcnm ltfrnbdbhjdfyf"               // Неуязвимость деактивирована

#define STSTR_KFAADDED  ",jtpfgfc gjgjkyty"                         // Боезапас пополнен
#define STSTR_FAADDED   ",jtpfgfc gjgjkyty (,tp rk.xtq)"            // Боезапас пополнен (без ключей)

#define STSTR_NCON      "ghj[j;ltybt xthtp cntys frnbdbhjdfyj"      // Прохождение через стены активировано
#define STSTR_NCOFF     "ghj[j;ltybt xthtp cntys ltfrnbdbhjdfyj"    // Прохождение через стены дереактивировано

#define STSTR_CHOPPERS  ">>> yt lehcndtyyj - u>v>"                  // ... не дурственно - г.м.
#define STSTR_CLEV      "cvtyf ehjdyz>>>"                           // Смена уровня...

#define STSTR_BEHOLD    "v=,tccv>< s=,thc>< i=ytd>< r=rjc>< a=rhn>< l=dbp>" // m=бессм., ы=берс., ш=нев., к=кос., ф=крт., д=виз.
#define STSTR_BEHOLDX   "ghtlvtn frnbdbhjdfy"                       // Предмет активирован
#define STSTR_BEHOLDZ   "ghtlvtn ltfrnbdbhjdfy"                     // Предмет деактивирован

#define STSTR_MASSACRE  "dct vjycnhs eybxnj;tys"                    // Все монстры уничтожены

//
//	F_Finale.C
//
#define E1TEXT \
"eybxnj;bd ,fhjyjd flf b pfxbcnbd keyye.\n"\
",fpe< ds ljk;ys ,skb gj,tlbnm< yt nfr kb?\n"\
"yt nfr kb? ult pfcke;tyyfz yfuhflf b\n"\
",bktn ljvjq? xnj 'nj pf xthnjdobyf?\n"\
"nfr yt ljk;yj ,skj dct pfrjyxbnmcz!\n"\
"\n" \
"'nj vtcnj gf[ytn rfr uybkjt vzcj< yj\n"\
"dsukzlbn rfr gjnthzyyfz ,fpf yf ltqvjct>\n"\
"gj[j;t< ds pfcnhzkb d ghb,ht;mt flf< b\n"\
"tlbycndtyysq genm ntgthm - ghjqnb tuj>\n"\
"\n"\
"xnj,s ghjljk;bnm gjuhe;tybt d buhe<\n"\
"ghjqlbnt 'gbpjl ^ghb,ht;mt flf^ b tuj\n"\
"pfvtxfntkmysq cbrdtk ^byathyj^!\n"

// Уничтожив Баронов Ада и зачистив лунную
// базу, Вы должны были победить, не так ли?
// Не так ли? Где заслуженная награда и
// билет домой? Что это за чертовщина?
// Так не должно было все закончиться!
// 
// Это место пахнет как гнилое мясо, но
// выглядит как потерянная база на Деймосе.
// Похоже, вы застряли в "Прибрежье Ада", и
// единственный путь теперь - пройти его.
//
// Чтобы продолжить погружение в игру,
// пройдите эпизод прибрежье ада и его
// замечательный сиквел "Инферно"!

#define E2TEXT \
"e dfc gjkexbkjcm! e;fcysq kjhl-\n"\
"rb,thltvjy< ghfdzobq gjnthzyyjq\n"\
"ltqvjcjdcrjq ,fpjq< ,sk gjdth;ty\n"\
"b ds njh;tcndetnt! yj>>> ult ds?\n"\
"gjlj,hfdibcm r rhf. cgenybrf< ds\n"\
"j,hfoftnt dpjh dybp< xnj,s\n"\
"edbltnm e;fcye. ghfdle>\n" \
"\n"\
"ltqvjc gksdtn yfl cfvbv fljv!\n"\
"ds ybrjulf yt cksifkb< xnj,s rnj-yb,elm\n"\
"c,tufk bp flf< yj ds pfcnfdbnt e,k.lrjd \n"\
"gj;fktnm j njv< xnj jyb ghjcksifkb j dfc>\n"\
"gjcgtiyj j,dzpfdibcm dthtdrjq< ds\n"\
"cgecrftntcm yf gjdth[yjcnm flf>\n"\
"\n" \
"Ntgthm yfcnegftn dhtvz gjcktlytq ukfds\n"\
"- ^byathyj^>"

// У вас получилось! Ужасный Лорд-
// Кибердемон, правящий потерянной 
// деймосовской базой, был повержен 
// и вы торжествуете! Но... Где вы? 
// Подобравшись к краю спутника, вы
// обращаете взор вниз, чтобы  
// увидеть ужасную правду. 
// 
// Деймос плывёт над самим Адом! 
// Вы никогда не слышали, чтобы кто-нибудь
// сбегал из Ада, но вы заставите ублюдков
// пожалеть о том, что они прослышали о вас.
// Поспешно обвязавшись верёвкой, Вы 
// спускаетесь на поверхность Ада.
//
// Теперь наступает время последней главы 
// - "Инферно". 

#define E3TEXT \
"jvthpbntkmysq gferjltvjy<\n"\
"herjdjlbdibq dnjh;tybtv yf keyyst\n"\
",fps b ghbytcibq cnjkm vyjuj \n"\
"cvthntq< ,sk jrjyxfntkmyj gjdth;ty>\n"\
"\n"\
"jnrhsdftncz ctrhtnyfz ldthm b ds\n"\
"d[jlbnt d ytt> ds ljrfpfkb< xnj\n"\
"ckbirjv rhens lkz flf< b gj'njve\n"\
"fl< yfrjytw< gjcnegftn cghfdtlkbdj -\n"\
"ds ds[jlbnt bp ldthb ghzvj yf\n"\
"ptktyst gjkz ptvkb> yfrjytw-nj ljv>\n" \
"\n"\
"ds cghfibdftnt ct,z: xnj ghjbc[jlbkj\n"\
"yf ptvkt< gjrf ds chf;fkbcm c \n"\
"dscdj,jlbdibvcz pkjv? [jhjij< xnj yb\n"\
"jlyj gjhj;ltybt flf yt cvjukj ghjqnb \n"\
"xthtp 'ne ldthm dvtcnt c dfvb>>>"

// Омерзительный паукодемон,
// руководивший вторжением на лунные
// базы и принесший столь много
// смертей, был окончательно повержен.
//
// Открывается секретная дверь и Вы
// входите в нее. Вы доказали, что
// слишком круты для ада, и поэтому
// Ад, наконец, поступает справедливо -
// Вы выходите из двери прямо на
// зеленые поля Земли. Наконец-то дом.
//
// Вы спрашиваете себя: что происходило
// на Земле, пока вы сражались с
// высвободившимся злом? Хорошо, что ни
// одно порождение Ада не смогло пройти
// через эту дверь вместе с Вами...

#define E4TEXT \
"gfer-ghtldjlbntkm< ljk;yj ,snm< gjckfk\n"\
"dgthtl cdjb ktubjys gjhj;ltybq flf\n"\
"gthtl dfibv gjcktlybv chf;tybtv c 'nbv\n"\
"e;fcysv jnhjlmtv> yj ds gjikb lj rjywf\n"\
"b chfpbkb dtxysv ghjrkznmtv b cnhflfybtv\n"\
"'ne jhle nfr< rfr gjcnegbk ,s yfcnjzobq\n"\
"uthjq gthtl kbwjv cvthntkmyjq jgfcyjcnb>\n"\
"\n"\
"rhjvt njuj< rnj-nj ljk;ty ,sk pfgkfnbnm\n"\
"pf nj< xnj ckexbkjcm c ltqpb< dfitq\n"\
"ljvfiytq rhjkmxb[jq>\n"\
"\n"\
"ntgthm ds dblbnt< crjkm vyjuj ,jkb b\n"\
"rhjdb dfv ghjhjxfn gjkxbof ltvjyjd<\n"\
"ytbcnde.ob[ d yfib[ ujhjlf[>\n"\
"\n"\
"cktle.ofz jcnfyjdrf - fl yf ptvkt!"

// Паук-предводитель, должно быть, послал 
// вперёд свои легионы порождений Ада перед
// Вашим последним сражением с этим 
// ужасным отродьем. Но Вы пошли до конца 
// и сразили вечным проклятьем и страданием 
// эту орду так, как поступил бы настоящий 
// герой перед лицом смертельной опасносности.
//
// Кроме того, кто-то должен был заплатить 
// за то, что случилось с Дэйзи, вашей
// домашней крольчихой. 
// 
// Теперь Вы видите, сколь много боли и
// крови Вам пророчат полчища демонов, 
// неиствующих в наших городах.
//
// Следующая остановка - Ад на земле!

// after level 6, put this:

#define C1TEXT \
"ds ghjikb duke,m gjhf;tyyjuj rjcvjgjhnf<\n" \
"yj xnj-nj pltcm ytghfdbkmyj> vjycnhs\n" \
"ghbytckb c cj,jq cj,cndtyye. htfkmyjcnm<\n" \
"b nt[yjkjubz rjcvjgjhnf \n" \
"nhfycajhvbhetncz jn b[ ghbcencndbz>\n" \
"\n"\
"dgthtlb ds dblbnt fdfygjcn flf>\n" \
"tckb dfv elfcncz ghj,hfnmcz xthtp ytuj<\n" \
"ds cvj;tnt ghjybryenm d yfctktyysq\n" \
"ltvjyfvb wtynh ,fps b yfqnb eghfdkz.obq\n" \
"dsrk.xfntkm< lth;fobq yfctktybt ptvkb\n" \
"d pfkj;ybrf[>"

// Вы прошли вглубь поражённого космопорта,
// но что-то здесь неправильно. Монстры
// принесли с собой собственную реальность,
// и технология космопорта 
// трансформируется от их присутствия.
//
// Впереди Вы видите аванпост Ада.
// Если Вам удастся пробраться через него,
// Вы сможете проникнуть в населённый 
// демонами центр базы и найти управляющий 
// выключатель, держащий население Земли 
// в заложниках.

// After level 11, put this:

#define C2TEXT \
"ds gj,tlbkb! dfif gj,tlf gjpdjkbkf\n" \
"xtkjdtxtcnde 'dfrebhjdfnmcz c ptvkb\n"\
"b cgfcnbcm jn rjivfhf> ntgthm ds -\n"\
"tlbycndtyysq xtkjdtr< jcnfdibqcz yf\n"\
"gkfytnt< b k.ljtls-venfyns< [boyst\n"\
"byjgkfytnzyt b pkst le[b - dfib\n"\
"tlbycndtyyst cjctlb> eljdktndjhtyysq\n"\
"ntv< xnj cgfckb cdjq dbl< ds cgjrjqyj\n"\
"lj;blftntcm ytvbyetvjq ub,tkb>\n"\
"\n"\
"yj dcrjht herjdjlcndj ptvkb gthtlftn\n"\
"cjj,otybt c jh,bns: ^ctycjhs j,yfhe;bkb\n"\
"bcnjxybr byjgkfytnyjuj dnjh;tybz> dfif\n"\
"pflfxf - kbrdblbhjdfnm tuj^> byjgkfytnyfz\n"\
",fpf yf[jlbncz d wtynht dfituj ujhjlf<\n"\
"ytlfktrj jn rjcvjgjhnf> vtlktyyj b\n"\
"vexbntkmyj ds djpdhfoftntcm d ,jq>\n"\

// Вы победили! Ваша победа позволила 
// человечеству эвакуироваться с Земли
// и спастись от кошмара. Теперь Вы - 
// единственный человек, оставшийся на 
// планете, и людоеды-мутанты, хищные 
// инопланетяне и злые духи - Ваши
// единственные соседи. Удовлетворённый 
// спасением своего вида, Вы спокойно
// дожидаетесь неминуемой гибели.
//
// Но вскоре руководство Земли передаёт
// сообщение с орбиты: "Сенсоры обнаружили
// источник инопланетного вторжения. Ваша
// задача - ликвидировать его". Инопланетная 
// база находится в центре Вашего города, 
// недалеко от космопорта. Медленно и 
// мучительно вы возрвращаетесь в бой.


// After level 20, put this:

#define C3TEXT \
"ds yf[jlbntcm d hfpkfuf.obvcz cthlwt\n"\
"ujhjlf< d jrhe;tybb nhegjd cdjb[ dhfujd>\n"\
"ds yt dblbnt ybrfrjuj cgjcj,f eybxnj;bnm\n"\
"gjhnfk yf 'njq cnjhjyt< b gj'njve< cnbcyed\n"\
"pe,s< ghj[jlbnt crdjpm ytuj>\n"\
"\n"\
"Ljk;ty ,snm cgjcj, pfrhsnm gjhnfk\n"\
"yf lheujq cnjhjyt> b rfrjt dfv ltkj lj\n"\
"njuj< xnj ghbltncz ghjqnb xthtp fl< xnj,s\n"\
"lj,hfnmcz lj 'njuj gjhnfkf?"

// Вы находитесь в разлагающемся сердце
// города, в окружении трупов своих врагов. 
// Вы не видите никакого способа уничтожить
// портал на этой стороне, и поэтому, стиснув 
// зубы, проходите сквозь него. 

// Должен быть способ закрыть портал 
// на другой стороне. И какое Вам дело до
// того, что придётся пройти через Ад, чтобы
// добраться до этого портала?



// After level 29, put this:

#define C4TEXT \
";enxfqibq kbr cfvjuj ,jkmijuj ltvjyf<\n"\
"rjnjhjuj ds rjulf-kb,j dbltkb< heibncz\n"\
"yf dfib[ ukfpf[ gjckt njuj< rfr ds\n"\
"yfrfxfkb hfrtnfvb tuj ytpfobotyysq\n"\
"vjpu> vjycnh eufcftn b ub,ytn<\n"\
"hfpheifz ,tcxbcktyyst vbkb gjdth[yjcnb\n"\
"flf>\n"\
"\n"\
"ds cltkfkb 'nj> dnjh;tyb. rjytw> ptvkz\n"\
"cgfctyf> fl gjdth;ty> ds cghfibdftnt\n"\
"ct,z: relf ntgthm gjckt cvthnb ,elen\n"\
"gjgflfnm gkj[bt k.lb? enthtd gjn\n"\
"cj k,f< ds yfxbyftnt ljkujt gentitcndbt\n"\
"j,hfnyj ljvjq> djccnfyjdktybt ptvkb\n"\
"ljk;yj ,snm ujhfplj ,jktt bynthtcysv\n"\
"pfyznbtv< yt;tkb tt hfpheitybt>"

// Жутчайший лик самого большого демона,
// которого Вы когда-либо видели, рушится
// на Ваших глазах после того, как Вы 
// накачали ракетами его незащищенный
// мозг. Монстр угасает и гибнет, 
// разрушая бесчисленные мили поверхности 
// Ада.
//
// Вы сделали это. Вторжению конец. Земля 
// спасена. Ад повержен. Вы спрашиваете 
// себя: куда теперь после смерти будут
// попадать плохие люди? Утерев пот 
// со лба, Вы начинаете долгое путешествие
// обратно домой. Восстановление Земли 
// должно быть гораздо более интересным
// занятием, нежели её разрушение. 


// Before level 31, put this:

#define C5TEXT \
"gjplhfdkztv< ds yfikb ctrhtnysq ehjdtym!\n"\
"gj[j;t< jy ,sk gjcnhjty k.lmvb< f yt\n"\
"ltvjyfvb> k.,jgsnyj< rnj ;t yfctkztn\n"\
"'njn eujkjr flf?\n"\

// Поздравляем, вы нашли секретный уровень! 
// Похоже, он был построен людьми, 
// а не демонами. Любопытно, кто же населяет
// этот уголок Ада?

// Before level 32, put this:

#define C6TEXT \
"gjplhfdkztv< ds yfikb cdth[ctrhtnysq\n"\
"ehjdtym! kexit ,s dfv cnhtvbntkmyj\n"\
"ghjhdfnmcz crdjpm ytuj!"

// Поздравляем, вы нашли сверхсекретный 
// уровень! Лучше бы вам стремительно
// прорваться сквозь него!

// after map 06	

#define P1TEXT  \
"ds cj pkjhflcndjv cvjnhbnt yf ujhzobq\n"\
"rfhrfc cnhf;f> c tuj cvthnm. ds dshdfkb\n"\
"ecrjhbntkm bp pkjdjyys[ rjuntq flf>\n"\
"hfcckf,bdibcm< ds jrblsdftnt dpukzljv\n"\
"gjvtotybt> ghjrkznmt! nen ljk;ty ,snm\n"\
"[jnz ,s jlby hf,jxbq ghjnjnbg< yj \n"\
"ltvjys< ljk;yj ,snm< pf,hfkb tuj c cj,jq>\n"\
"\n"\
"ds ljk;ys yfqnb ghjnjnbg< byfxt dct dfib\n"\
"ghjikst ecbkbz jrf;encz yfghfcysvb>\n"\
"ghjljk;fqnt ldbufnmcz< ghjljk;fqnt\n"\
"chf;fnmcz< ghjljk;fqnt e,bdfnm>\n"\
"b lf< ghjljk;fqnt ds;bdfnm>"

// Вы со злорадством смотрите на горящий 
// каркас Стража. С его смертью вы вырвали
// Ускоритель из зловонных когтей Ада.
// Расслабившись, Вы окидываете взглядом
// помещение. Проклятье! Тут должен быть
// хотя бы один рабочий прототип, но 
// демоны, должно быть, забрали его с собой.
//
// Вы должны найти прототип, иначе все Ваши
// прошлые усилия окажутся напрасными.
// Продолжайте двигаться, продолжайте
// сражаться, продолжайте убивать. И да,
// продолжайте выживать.

// after map 11

#define P2TEXT \
"lf;t cvthntkmysq kf,bhbyn fhxdfqkjd yt\n"\
"cvju jcnfyjdbnm dfc> ds lj,hfkbcm lj\n"\
"ghjnjnbgf ecrjhbntkz< rjnjhsq dcrjht\n"\
",sk eybxnj;ty hfp b yfdctulf>\n"\
"\n"\
"eybxnj;tybt - dfif cgtwbfkmyjcnm>"

// Даже смертельный лабиринт арчвайлов не
// смог остановить Вас. Вы добрались до
// прототипа Ускорителя, который вскоре
// был уничтожен раз и навсегда.
//
// Уничтожение - Ваша специальность.

// after map 20

#define P3TEXT \
"ds ghj,bkb b ghjrjkjnbkb cdjq genm d\n"\
"cthlwt lmzdjkmcrjuj ekmz> yfcnfkj dhtvz\n"\
"lkz vbccbb ^yfqnb b eybxnj;bnm^<\n"\
"j,]trnjv rjnjhjq cnfytn ghbdhfnybr<\n"\
"xmb ytxbcnbdst jnghscrb ybpdthuf.ncz\n"\
"yf ptvk.>\n"\
"\n"\
"lf< jy gkj[jq< yj ds pyftnt rnj tot [e;t!\n"\
"\n"\
"pkj,yj e[vskzzcm< ds ghjdthztnt cdjt\n"\
"cyfhz;tybt< b ujnjdbntcm ghtgjlfnm\n"\
"e,k.lre ytvyjuj flf dfituj\n"\
"cj,cndtyyjuj bpujnjdktybz!"

// Вы пробили и проколотили свой путь в
// сердце дьявольского улья. Настало время 
// для миссии "найти и уничтожить", 
// объектом которой станет Привратник, 
// чьи нечистивые отпрыски низвергаются 
// на Землю. 
//
// Да, он плохой. Но Вы знаете кто ещё хуже!
//
// Злобно ухмыляясь, Вы проверяете своё 
// снаряжение, и готовитесь преподать 
// ублюдку немного Ада Вашего 
// собственного изготовления!

// after map 30

#define P4TEXT \
"gjckt eybxnj;tybz kbrf ghbdhfnybrf\n"\
"yfxbyf.n ajhvbhjdfnmcz bydthnbhjdfyyst\n"\
"dhfnf< rjnjhst pfcfcsdf.n d ct,z\n"\
"gjcktlybt j,kjvrb ecrjhbntkz b\n"\
"ytcrjkmrb[ jcnfdib[cz ltvjyjd>\n"\
"\n"\
"ujnjdj> fl dthyekcz yf rheub cdjz<\n"\
"gjukfofz kbim uhtiys[ k.ltq< yt;tkb\n"\
"ghfdtlys[>\n"\
"\n"\
"yt pf,elmnt gjghjcbnm dyerjd gjkj;bnm\n"\
"hfrtnybwe d dfi uhj,> Tckb gjckt cvthnb\n"\
"ds gjgfltnt d fl< jyf gjyflj,bncz\n"\
"lkz yt,jkmijq gjcktlytq pfxbcnrb>>>"

// После уничтожения лика Привратника
// начинают формироваться инвертированные 
// Врата, которые затягивают в себя 
// последние обломки Ускорителя и 
// нескольких оставшихся демонов.
//
// Готово. Ад вернулся на круги своя,
// поглащая лишь грешных людей, нежели 
// праведных.
//
// Не забудьте попросить внуков положить
// ракетницу в Ваш гроб. Если после смерти
// Вы попадёте в Ад, она понадобится
// для небольшой последней зачистки...



// before map 31

#define P5TEXT \
"ds yfikb dnjhjq gj ckj;yjcnb ehjdtym<\n"\
"rjnjhsq e yfc tcnm> yflttvcz< ds\n"\
"cj[hfybkb buhe yf ghtlsleotv ehjdyt\n"\
"bkb tot hfymit> tckb ytn< ghbujnjdmntcm\n"\
"vyjuj evbhfnm>\n"\
"\n"\
"ehjdtym hfccxbnfy bcrk.xbntkmyj yf\n"\
"ghjatccbjyfkjd>"

// Вы нашли второй по сложности уровень,
// который у нас есть. Надеемся, Вы 
// сохранили игру на предыдущем уровне 
// или ещё раньше. Если нет, приготовьтесь 
// много умирать.
//
// Уровень раcсчитан исключительно на 
// профессионалов.

// before map 32

#define P6TEXT \
"cgjhbv< ds elbdkzkbcm< rfrjq ;t ehjdtym\n"\
"cfvsq ckj;ysq? ntgthm ds pyftnt>\n"\
"ybrnj yt ds,thtncz ;bdsv>"

// Спорим, вы удивлялись, какой же уровень
// САМЫЙ сложный? Теперь Вы знаете.
// Никто не выберется живым.

// after map 06	

#define T1TEXT \
"chf;fzcm< ds ds,hfkbcm bp pfhf;tyys[\n"\
"'rcgthtvtynfkmys[ kf,jhfnjhbq>\n"\
"gj[j;t< jfr hfcnhfy;bhbkf b b[< ytcvjnhz\n"\
"yf juhjvyst rjhgjhfnbdyst lj[jls> jyb\n"\
"lf;t yt gjpf,jnbkbcm j gjregrt cnhf[jdjr\n"\
"lkz cdjb[ cjnhelybrjd>>>\n"\
"\n"\
"dgthtlb hfcgjkj;ty djtyysq rjvgktrc<\n"\
"rbifobq dbhecfvb b njkmrj ;leobq\n"\
"rfr ,s duhspnmcz d dfie gkjnm> xnj ;<\n"\
"tckb gjdtptn< d rjvgktrct dct tot\n"\
"ljk;ys jcnfdfnmcz ,jtghbgfcs>"

// Сражаясь, Вы выбрались из заражённых
// эксперементальных лабораторий.
// Похоже, ОАК растранжирила и их, несмотря
// на огромные корпоративные доходы. Они
// даже не позаботились о покупке страховок
// для своих сотрудников...
//
// Впереди расположен военный комплекс, 
// кишащий вирусами и только ждущий 
// как бы вгрызться в Вашу плоть. Что ж,
// если повезёт, в комплексе всё ещё 
// должны оставаться боеприпасы.

// after map 11

#define T2TEXT \
"dgthtlb cksity vtnfkkbxtcrbq crht;tn\n"\
"nz;tks[ vt[fybpvjd> ds edthtys< xnj jyb\n"\
"yt infvge.n jxthtlye. gfhnb.\n"\
"lmzdjkmcrb[ jnhjlbq< yj lf;t tckb 'nj\n"\
"b nfr< ds r 'njve ujnjds>\n"\
"\n"\
"'nb pderb vjuen jpyfxfnm jxthtlyjq\n"\
"rhjdfdsq atcnbdfkm< yj ds-nj e;t pyftnt<\n"\
"rfrjuj 'nj joeofnm nsczxb ujkjdjhtpjd<\n"\
"cj,hfyys[ d jlyjv ,tpevyjv e,bqwt>\n"\
"\n"\
"ds nfr ghjcnj yt clflbntcm>"

// Впереди слышен металлический скрежет 
// тяжёлых механизмов. Вы уверены, что они
// не штампуют очередную партию 
// дьявольских отродий, но даже если
// и так, Вы к этому готовы. 
// Эти звуки могут означать очередной 
// кровавый фестиваль, но вы-то уже знаете, 
// какого это ощущать тысячи головорезов, 
// собранных в одном безумном убийце.
//
// Вы так просто не сдадитесь.

#define T3TEXT \
"jnrhsdf.ofzcz gthcgtrnbdf dsukzlbn\n"\
"xthnjdcrb pyfrjvjq b gf[ytn \n"\
"ckjdyj pf;fhtyyst 'rcrhbvtyns>\n"\
"'nj vtcnj yt yhfdbkjcm dfv hfymit<\n"\
"b ds xthnjdcrb edthtys< xnj yt\n"\
"gjyhfdbncz b ctqxfc> xtv ,jkmit ds\n"\
"hfpvsikztnt yfl 'nbv< ntv gtxfkmytt\n"\
"dct cnfyjdbncz>\n"\
"\n"\
"dpdtcbd cdjt jhe;bt< ds pkjdtot\n"\
"e[vskztntcm> yfcnfkj dhtvz dcthmtp\n"\
"yflhfnm rjt-rjve pflybwe>"

// Открывающаяся перспектива выглядит 
// чертовски знакомой и пахнет 
// словно зажаренные экскрименты. 
// Это место не нравилось вам раньше, 
// и Вы чертовски уверены, что не 
// понравится и сейчас. Чем больше Вы 
// размышляете над этим, тем печальнее 
// всё становится.
// 
// Взвесив своё оружие, Вы зловеще 
// ухмыляетесь. Настало время всерьез 
// надрать кое-кому задницу.

#define T4TEXT \
"dytpfgyj dct jrenfkjcm nbibyjq lj\n"\
"cfvjuj ujhbpjynf> fujybpbhe.ott flcrjt\n"\
"'[j cnb[kj< rjivfhyjt yt,j cnfkj dyjdm\n"\
"ujke,sv< nhegs vjycnhjd yfxfkb\n"\
"hfpkfufnmcz cj pkjdjyysv cvhfljv>\n"\
",j;t< yte;tkb ds b ghfdlf gj,tlbkb?\n"\
"\n"\
"gjckt rjhjnrjuj ptvktnhzctybz< bp\n"\
"hfpuhjvktyyjuj xthtgf bphsufntkz\n"\
"ltvjyjd yfxbyftn ghj,bdfnmcz zhrjt\n"\
"cbytt cdtxtybt>"

// Внезапно всё окуталось тишиной до
// самого горизона. Агонизирующее Адское
// эхо стихло, кошмарное небо стало вновь
// голубым, трупы монстров начали 
// разлагаться со зловонным смрадом.
// Боже, неужели Вы и правда победили?
//
// После короткого землетрясения, из
// разгромленного черепа Изрыгателя 
// Демонов начинает пробиваться яркое
// синее свечение.


#define T5TEXT \
"b xnj ntgthm? dct dsukzlbn cjdthityyj\n"\
"gj-lheujve< ckjdyj gjrjb dkflsrb\n"\
"nenfy[jvjyf>\n"\
"\n"\
"xnj ;< xnj ,s yt j;blfkj dfc dyenhb<\n"\
"[e;t e;t ,snm yt vj;tn< yt nfr kb?\n"\
"bkb vj;tn kexit yt nhtdj;bnm\n"\
"cgzob[ ,jujd>>>"

// И что теперь? Всё выглядит совершенно
// по-другому, словно покои владыки
// Тутанхамона.
//
// Что ж, что бы не ожидало Вас внутри,
// хуже уже быть не может, не так ли?
// Или может лучше не тревожить 
// спящих богов...

#define T6TEXT \
"ghbikj dhtvz lkz jngecrf> tq-,jue<\n"\
"djhjif ytlhf flf< ds njkmrj j ytv\n"\
"b vtxnfkb! Gecnm rnj-yb,elm lheujq\n"\
"ntgthm ,jhtncz c gjkxbofvb ltvjyjd>\n"\
"\n"\
"dgthtlb hfcgjkj;ty nb[bq ujhjljr c\n"\
"ytcgtiyj ntreotq djljq< ghbxelkbdsvb\n"\
"ljvbrfvb< b< dthjznyj< yt yfctktyysq\n"\
"flcrbv jnhjlmtv>\n"\
"\n"\
"gjrbyed nhfycgjhnyjt chtlcndj<\n"\
"ds cksibnt njgjn ;tktpyjuj rjgsnf\n"\
"rb,thltvjyf>"

// Пришло время для отпуска. Ей-богу, 
// вороша недра Ада, Вы только о нем
// и мечтали! Пусть кто-нибудь другой 
// теперь борется с полчищами демонов.
//
// Впереди располагается тихий городок с
// неспешно текущей водой, причудливыми
// домиками, и, вероятно, не населенный
// адским отродьем.
//
// Покинув транспортное средство,
// Вы слышите топот железного копыта 
// кибердемона.


/******************************************** 

[JN] NRFTL text. Not used in DOS version. 

#define N1TEXT \
"ytghbznyjcnb< rfpfkjcm< yfcnbukb d cfvsq\n"\
"ytgjl[jlzobq vjvtyn>>>\n"\
"ytcvjnhz yf hfpuhjvyjt gjhf;tybt cbk\n"\
"flf< j,tpevtdibq rb,thltvjy htibk< xnj\n"\
"cvj;tn bpvtybnm [jl cj,snbq b dyjdm\n"\
"gjdthuyenm ptvk. d ltvjybxtcrbq [fjc>\n"\
"\n" \
"b vexbntkmyfz cvthnm cnfkf gkfnjq pf 'ne\n"\
"hjrjde. jib,re> cntys tuj j,bntkb tot\n"\
"ljkuj ,elen gjvybnm ghtlcvthnye. fujyb.\n"\
"'njuj gjhj;ltybz gjnecnjhjyyb[ cbk>\n"\
"b gecnm tuj uyb.obt jcnfyrb cnfyen\n" \
"dtxysv yfgjvbyfybtv lkz nt[< rnj dyjdm\n" \
"jcvtkbncz yfheibnm gjrjq yfituj vbhf>\n" \
"\n" \
"'nf vbccbz pfdthityf>"

// Неприятности, казалось, настигли в самый 
// неподходящий момент...
// Несмотря на разгромное поражение сил 
// Ада, обезумевший Кибердемон решил, что 
// сможет изменить ход событий и вновь 
// повергнуть Землю в демонический хаос.
// 
// И мучительная смерть стала платой за эту 
// роковую ошибку. Стены его обители ещё 
// долго будут помнить предсмертную агонию 
// этого порождения потусторонних сил.
// И пусть его гниющие останки станут  
// вечным напоминанием для тех, кто вновь
// осмелится нарушить покой нашего мира.
// 
// Эта миссия завершена.


[JN] The Master Levels. Not used.

#define M1TEXT \
"CONGRATULATIONS YOU HAVE FINISHED... \n\n"\
"THE MASTER LEVELS\n"

********************************************/

//
// F_FINALE.C
//

#define CC_ZOMBIE   "pjv,b"                 // Зомби
#define CC_SHOTGUN  "pjv,b-cth;fyn"         // Зомби-сержант
#define CC_HEAVY    "gektvtnxbr"            // Пулемётчик
#define CC_IMP      ",tc"                   // Бес
#define CC_DEMON    "ltvjy"                 // Демон
#define CC_LOST     "gjnthzyyfz leif"       // Потерянная душа
#define CC_CACO     "rfrjltvjy"             // Какодемон
#define CC_HELL     "hswfhm flf"            // Рыцарь ада
#define CC_BARON    ",fhjy flf"             // Барон ада
#define CC_ARACH    "fhf[yjnhjy"            // Арахнотрон
#define CC_PAIN     "'ktvtynfkm ,jkb"       // Элементаль боли
#define CC_REVEN    "htdtyfyn"              // Ревенант
#define CC_MANCU    "vfyre,ec"              // Манкубус
#define CC_ARCH     "fhxdfqk"               // Арчвайл
#define CC_SPIDER   "gfer-ghtldjlbntkm"     // Паук-предводитель
#define CC_CYBER    "rb,thltvjy"            // Кибердемон
#define CC_HERO     "yfi uthjq"             // Наш Герой


// Port-specific strings
#define STSTR_VERSION   "dthcbz 1>7 (2019-08-04)"                   // Версия 1.7 (2019-08-04)

#define STSTR_KAADDED   "rk.xb gjkextys"                            // Ключи получены

#define STSTR_MLOOK_ON  "j,pjh vsim. drk.xty"                       // Обзор мышью включен
#define STSTR_MLOOK_OFF "j,pjh vsim. dsrk.xty"                      // Обзор мышью выключен

#define STSTR_CROSSHAIR_ON  "ghbwtk drk.xty"                        // Прицел включен
#define STSTR_CROSSHAIR_OFF "ghbwtk dsrk.xty"                       // Прицел выключен


#endif
