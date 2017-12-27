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
// Памятка по символам: docs_russian/charmap/raven_charmap.xlsx
//


// DStrings.h

//---------------------------------------------------------------------------
//
// P_inter.c
//
//---------------------------------------------------------------------------

// Keys

#define TXT_GOTBLUEKEY          "CBYBQ RK.X"    // СИНИЙ КЛЮЧ
#define TXT_GOTYELLOWKEY        ";TKNSQ RK.X"   // ЖЕЛТЫЙ КЛЮЧ
#define TXT_GOTGREENKEY         "PTKTYSQ RK.X"  // ЗЕЛЕНЫЙ КЛЮЧ

// Artifacts

#define TXT_ARTIHEALTH          "RDFHWTDSQ AKFRJY"      // КВАРЦЕВЫЙ ФЛАКОН
#define TXT_ARTIFLY             "RHSKMZ UYTDF"          // КРЫЛЬЯ ГНЕВА
#define TXT_ARTIINVULNERABILITY "RJKMWJ YTEZPDBVJCNB"   // КОЛЬЦО НЕУЯЗВИМОСТИ
#define TXT_ARTITOMEOFPOWER     "NJV VJUEOTCNDF"        // ТОМ МОГУЩЕСТВА
#define TXT_ARTIINVISIBILITY    "NTYTDFZ CATHF"         // ТЕНЕВАЯ СФЕРА
#define TXT_ARTIEGG             "VJHABQYJT ZQWJ"        // МОРФИЙНОЕ ЯЙЦО
#define TXT_ARTISUPERHEALTH     "VBCNBXTCRFZ EHYF"      // МИСТИЧЕСКАЯ УРНА
#define TXT_ARTITORCH           "AFRTK"                 // ФАКЕЛ
#define TXT_ARTIFIREBOMB        "XFCJDFZ ,JV,F LHTDYB]" // ЧАСОВАЯ БОМБА ДРЕВНИХ
#define TXT_ARTITELEPORT        "'V,KTVF [FJCF"         // ЭМБЛЕМА ХАОСА

// Items

#define TXT_ITEMHEALTH          "RHBCNFKMYSQ AKFRJY"    // КРИСТАЛЬНЫЙ ФЛАКОН
#define TXT_ITEMBAGOFHOLDING    "YJCBKMYSQ RJITKM"      // НОСИЛЬНЫЙ КОШЕЛЬ
#define TXT_ITEMSHIELD1         "CTHT,HZYSQ OBN"        // СЕРЕБРЯНЫЙ ЩИТ
#define TXT_ITEMSHIELD2         "PFXFHJDFYYSQ OBN"      // ЗАЧАРОВАННЫЙ ЩИТ
#define TXT_ITEMSUPERMAP        "CDBNJR RFHNS"          // СВИТОК КАРТЫ

// Ammo

#define TXT_AMMOGOLDWAND1       "RHBCNFKK LKZ 'KMABQCRJUJ ;TPKF"    // КРИСТАЛЛ ДЛЯ ЭЛЬФИЙСКОГО ЖЕЗЛА
#define TXT_AMMOGOLDWAND2       ";TJLF RHBCNFKKF"       // ЖЕОДА КРИСТАЛЛА
#define TXT_AMMOMACE1           "CATHS LKZ ,EKFDS"      // СФЕРЫ ДЛЯ БУЛАВЫ
#define TXT_AMMOMACE2           "UHELF CATH LKZ ,EKFDS" // ГРУДА СФЕР ДЛЯ БУЛАВЫ
#define TXT_AMMOCROSSBOW1       "'ABHYST CNHTKS"        // ЭФИРНЫЕ СТРЕЛЫ
#define TXT_AMMOCROSSBOW2       "RJKXFY 'ABHYS[ CNHTK"  // КОЛЧАН ЭФИРНЫХ СТРЕЛ
#define TXT_AMMOBLASTER1        "RJUNTDJQ IFH"          // КОГТЕВОЙ ШАР
#define TXT_AMMOBLASTER2        "'YTHUTNBXTCRBQ IFH"    // ЭНЕРГЕТИЧЕСКИЙ ШАР
#define TXT_AMMOSKULLROD1       "VKFLIFZ HEYF"          // МЛАДШАЯ РУНА
#define TXT_AMMOSKULLROD2       "CNFHIFZ HEYF"          // СТАРШАЯ РУНЫ
#define TXT_AMMOPHOENIXROD1     "GKFVTYYSQ IFH"         // ПЛАМЕННЫЙ ШАР
#define TXT_AMMOPHOENIXROD2     "BYATHYFKMYSQ IFH"      // ИНФЕРНАЛЬНЫЙ ШАР

// Weapons

#define TXT_WPNMACE         "JUYTYYFZ ,EKFDF"       // ОГНЕННАЯ БУЛАВА
#define TXT_WPNCROSSBOW     "'ABHYSQ FH,FKTN"       // ЭФИРНЫЙ АРБАЛЕТ
#define TXT_WPNBLASTER      "RJUJNM LHFRJYF"        // КОГОТЬ ДРАКОНА
#define TXT_WPNSKULLROD     "GJCJ[ FLF"             // ПОСОХ АДА
#define TXT_WPNPHOENIXROD   ";TPK ATYBRCF"          // ЖЕЗЛ ФЕНИКСА
#define TXT_WPNGAUNTLETS    "GTHXFNRB YTRHJVFYNF"   // ПЕРЧАТКИ НЕКРОМАНТА

//---------------------------------------------------------------------------
//
// SB_bar.c
//
//---------------------------------------------------------------------------

#define TXT_CHEATGODON          "HT;BV ,JUF"                            // РЕЖИМ БОГА
#define TXT_CHEATGODOFF         "HT;BV ,JUF JNVTYTY"                    // РЕЖИМ БОГА ОТМЕНЕН
#define TXT_CHEATNOCLIPON       "GHJ[J;LTYBT XTHTP CNTYS"               // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ
#define TXT_CHEATNOCLIPOFF      "GHJ[J;LTYBT XTHTP CNTYS JNVTYTYJ"      // ПРОХОЖДЕНИЕ ЧЕРЕЗ СТЕНЫ ОТМЕНЕНО
#define TXT_CHEATWEAPONS        "GJKYJT DJJHE;TYBT"                     // ПОЛНОЕ ВООРУЖЕНИЕ
#define TXT_CHEATFLIGHTON       "HT;BV GJKTNF"                          // РЕЖИМ ПОЛЕТА
#define TXT_CHEATFLIGHTOFF      "HT;BV GJKTNF JNVTYTY"                  // РЕЖИМ ПОЛЕТА ОТМЕНЕН
#define TXT_CHEATPOWERON        "VJUEOTCNDJ JHE;BZ"                     // МОГУЩЕСТВО ОРУЖИЯ
#define TXT_CHEATPOWEROFF       "VJUEOTCNDJ JHE;BZ JNVTYTYJ"            // МОГУЩЕСТВО ОРУЖИЯ ОТМЕНЕНО
#define TXT_CHEATHEALTH         "PLJHJDMT DJCCNFYJDKTYJ"                // ЗДОРОВЬЕ ВОССТАНОВЛЕНО
#define TXT_CHEATKEYS           "DCT RK.XB"                             // ВСЕ КЛЮЧИ
#define TXT_CHEATSOUNDON        "JNKFLRF PDERF DRK.XTYF"                // ОТЛАДКА ЗВУКА ВКЛЮЧЕНА
#define TXT_CHEATSOUNDOFF       "JNKFLRF PDERF JNRK.XTYF"               // ОТЛАДКА ЗВУКА ОТКЛЮЧЕНА
#define TXT_CHEATTICKERON       "CXTNXBR RFLHJDJQ XFCNJNS DRK.XTY"      // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ВКЛЮЧЕН
#define TXT_CHEATTICKEROFF      "CXTNXBR RFLHJDJQ XFCNJNS JNRK.XTY"     // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ ОТКЛЮЧЕН
#define TXT_CHEATARTIFACTS1     "DS,THBNT FHNTAFRN ( F - J )"           // ВЫБЕРИТЕ АРТЕФАКТ ( А - O )
#define TXT_CHEATARTIFACTS2     "ERF;BNT RJKBXTCNDJ ( 1 - 9 )"          // УКАЖИТЕ КОЛИЧЕСТВО ( 1 - 9 )
#define TXT_CHEATARTIFACTS3     "GJKEXTYJ"                              // ПОЛУЧЕНО 
#define TXT_CHEATARTIFACTSFAIL  "YTRJHHTRNYSQ DDJL"                     // НЕКОРРЕНТНЫЙ ВВОД
#define TXT_CHEATWARP           "GTHTVTOTYBT YF EHJDTYM"                // ПЕРЕМЕЩЕНИЕ НА УРОВЕНЬ
#define TXT_CHEATSCREENSHOT     "CYBVJR 'RHFYF"                         // СНИМОК ЭКРАНА
#define TXT_CHEATCHICKENON      "GHTDHFOTYBT D WBGKTYRF"                // ПРЕВРАЩЕНИЕ В ЦИПЛЕНКА
#define TXT_CHEATCHICKENOFF     "GHTDHFOTYBT D WBGKTYRF JNVTYTYJ"       // ПРЕВРАЩЕНИЕ В ЦИПЛЕНКА ОТМЕНЕНО
#define TXT_CHEATMASSACRE       "DCT VJYCNHS EYBXNJ;TYS"                // ВСЕ МОНСТРЫ УНИЧТОЖЕНЫ
#define TXT_CHEATIDDQD          "GSNFTIMCZ C[BNHBNM? NFR EVHB ;T!"      // ПЫТАЕШЬСЯ СХИТРИТЬ? ТАК УМРИ ЖЕ!
#define TXT_CHEATIDKFA          ";EKBR< NS YT LJCNJBY CDJTUJ JHE;BZ"    // ЖУЛИК, ТЫ НЕ ДОСТОИН СВОЕГО ОРУЖИЯ

//---------------------------------------------------------------------------
//
// P_doors.c
//
//---------------------------------------------------------------------------

#define TXT_NEEDBLUEKEY     "LKZ JNRHSNBZ YE;TY CBYBQ RK.X"     // ДЛЯ ОТКРЫТИЯ НУЖЕН СИНИЙ КЛЮЧ
#define TXT_NEEDGREENKEY    "LKZ JNRHSNBZ YE;TY PTKTYSQ RK.X"   // ДЛЯ ОТКРЫТИЯ НУЖЕН ЗЕЛЕНЫЙ КЛЮЧ
#define TXT_NEEDYELLOWKEY   "LKZ JNRHSNBZ YE;TY ;TKNSQ RK.X"    // ДЛЯ ОТКРЫТИЯ НУЖЕН ЖЕЛТЫЙ КЛЮЧ

//---------------------------------------------------------------------------
//
// G_game.c
//
//---------------------------------------------------------------------------

#define TXT_GAMESAVED   "BUHF CJ[HFYTYF"    // ИГРА СОХРАНЕНА

//---------------------------------------------------------------------------
//
// AM_map.c
//
//---------------------------------------------------------------------------

#define AMSTR_FOLLOWON      "HT;BV CKTLJDFYBZ DRK.XTY"      // РЕЖИМ СЛЕДОВАНИЯ ВКЛЮЧЕН
#define AMSTR_FOLLOWOFF     "HT;BV CKTLJDFYBZ JNRK.XTY"     // РЕЖИМ СЛЕДОВАНИЯ ОТКЛЮЧЕН

#define AMSTR_GRIDON        "CTNRF DRK.XTYF"                // СЕТКА ВКЛЮЧЕНА
#define AMSTR_GRIDOFF       "CTNRF DSRK.XTYF"               // СЕТКА ВЫКЛЮЧЕНА

#define AMSTR_MARKEDSPOT    "JNVTNRF"                       // ОТМЕТКА
#define AMSTR_MARKSCLEARED  "JNVTNRB JXBOTYS"               // ОТМЕТКИ ОЧИЩЕНЫ

//---------------------------------------------------------------------------
//
// MN_menu.c
//
//---------------------------------------------------------------------------

#define TXT_GAMMA_LEVEL_OFF      "UFVVF-RJHHTRWBZ JNRK.XTYF"        // ГАММА-КОРРЕКЦИЯ ОТКЛЮЧЕНА
#define TXT_GAMMA_LEVEL_0_25     "EHJDTYM UFVVF-RJHHTRWBB 0>25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 0.25
#define TXT_GAMMA_LEVEL_0_50     "EHJDTYM UFVVF-RJHHTRWBB 0>50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 0.50
#define TXT_GAMMA_LEVEL_0_75     "EHJDTYM UFVVF-RJHHTRWBB 0>75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 0.75
#define TXT_GAMMA_LEVEL_1_0      "EHJDTYM UFVVF-RJHHTRWBB 1>0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.0
#define TXT_GAMMA_LEVEL_1_25     "EHJDTYM UFVVF-RJHHTRWBB 1>25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.25
#define TXT_GAMMA_LEVEL_1_50     "EHJDTYM UFVVF-RJHHTRWBB 1>50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.50
#define TXT_GAMMA_LEVEL_1_75     "EHJDTYM UFVVF-RJHHTRWBB 1>75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 1.75
#define TXT_GAMMA_LEVEL_2_0      "EHJDTYM UFVVF-RJHHTRWBB 2>0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.0
#define TXT_GAMMA_LEVEL_2_25     "EHJDTYM UFVVF-RJHHTRWBB 2>25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.25
#define TXT_GAMMA_LEVEL_2_50     "EHJDTYM UFVVF-RJHHTRWBB 2>50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.50
#define TXT_GAMMA_LEVEL_2_75     "EHJDTYM UFVVF-RJHHTRWBB 2>75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 2.75
#define TXT_GAMMA_LEVEL_3_0      "EHJDTYM UFVVF-RJHHTRWBB 3>0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.0
#define TXT_GAMMA_LEVEL_3_25     "EHJDTYM UFVVF-RJHHTRWBB 3>25"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.25
#define TXT_GAMMA_LEVEL_3_50     "EHJDTYM UFVVF-RJHHTRWBB 3>50"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.50
#define TXT_GAMMA_LEVEL_3_75     "EHJDTYM UFVVF-RJHHTRWBB 3>75"     // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 3.75
#define TXT_GAMMA_LEVEL_4_0      "EHJDTYM UFVVF-RJHHTRWBB 4>0"      // УРОВЕНЬ ГАММА-КОРРЕКЦИИ 4.0

//---------------------------------------------------------------------------
//
// F_finale.c
//
//---------------------------------------------------------------------------

#define E1TEXT      "C EYBXNJ;TYBTV ;TKTPYS[ KBXTQ\n"\
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
/*
                    С уничтожением Железных Личей
                    и их приспешников, окрестные земли 
                    очистились от омерзительной нежити.

                    Эта нежить, проникшая в наш мир из
                    темного измерения, открыла Огненный
                    Портал. Он как отверстая пылающая 
                    пасть Ада ведет в его жуткую утробу.

                    Угроза исходит из Огненного Портала -
                    порождения Ада и черной магии могут
                    изринуться из него. Ваша цель - 
                    сойти в Ад и запечатать проклятый 
                    портал.

                    Это смертельно опасное деяние, и вы
                    рискуете навсегда увязнуть во Тьме. 
                    Но кто говорил, что путь истинного 
                    Еретика легок и прост?
*/

#define E2TEXT      "VJUEXBT VJKJNFDHS GJDTH;TYS>\n"\
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
/*
                    Могучие Молотавры повержены. 
                    Их дымящиеся трупы падают, 
                    разваливаясь на куски, к вашим
                    ногам, и мрачное удовлетворение 
                    наполняет вас.

                    Врата, что они охраняли, открылись. 
                    Вы шагнули в них, думая, что 
                    вернетесь в родной мир, но лишь 
                    громкий, насмешливый хохот был 
                    ответом на вашу надежду.

                    Чей это злобный хохот? Быть может 
                    это голос иных демонических сил, 
                    управляющих Молотаврами? Какие 
                    чудовищные создания ожидают вас
                    за этими вратами? Не голубое небо
                    родного мира над головой, а 
                    кристальный купол, - это плохой 
                    знак...
*/

#define E3TEXT      "C GJUB,TKM. L\"CGFHBKF BCXTPKF VFUBZ<\n"\
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
/*
                    С погибелью Д'Спарила исчезла магия, 
                    сохранявшая жизнь порождениям Тьмы.
                    Стоны умирающих демонов заглушили 
                    вопль агонии самого Д'Спарила.

                    Вы исполнили свою клятву. Месть 
                    свершилась. И за секунду до 
                    разрушения хрустального купола, 
                    вы наконец-то, входите во врата, 
                    ведущие в родной мир.

                    Но и теперь, после гибели Д'Спарила,
                    душа ваша не спокойна, и плохие 
                    предчувствия одолевают ее. Не был 
                    ли проклятием его предсмертный 
                    крик? Или призывом темных сил?

                    И где таятся другие Змеиные 
                    Всадники?
*/

#define E4TEXT      "C UB,TKM. L\"CGFHBKF EVTHKF B DFIF\n"\
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
/*
                    С гибелью Д'Спарила умерла и ваша
                    надежда вернуться в родной мир. 
                    Своим последним предсмертным
                    проклятьем он отправил вас в те 
                    немногие, оставшиеся подвластными 
                    ему земли. Вы разбили последних 
                    хранителей этих земель и стоите
                    перед опустевшим замком Д'Спарила,
                    оплотом его темных сил. 

                    Само Сердце Зла растворило перед 
                    вами врата. И хотя Д'Спарил 
                    повержен, глубины, породившие его, 
                    стоят нерушимы.

                    Вы сойдете в эту преисподнюю, ибо 
                    единственный способ вернуться в 
                    родной мир -  отыскать вход в него 
                    в темных глубинах опустевшей вотчины 
                    Д'Спарила. Личные стражи мертвого 
                    господина уже дожидаются вас...
*/

#define E5TEXT      "YF,K.LFZ C VHFXYSV JNDHFOTYBTV\n"\
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
/*
                    Наблюдая с мрачным отвращением
                    предсмертную агонию последнего
                    сраженного Молотавра, вы только 
                    теперь понимаете, что смерть еще
                    никогда не была так близка. Даже
                    во время яростной битвы с самим
                    Д'Спарилом и его темными слугами.

                    С угрюмым отчаянием приближаетесь
                    вы к открытым порталам. Слабая 
                    надежда теплится в вашей душе, -
                    быть может за ними скрыта дорога
                    домой, а не бездны чужих миров?

                    Отчаяние наделяет вас мрачной 
                    решимостью. Ничто не способно 
                    остановить вас, одна только - смерть.
                    Не сдавайтесь без боя, взгляните в 
                    глаза своей судьбе. Знайте, если вы
                    упали на самое дно, есть лишь один 
                    путь - наверх.
*/

// [JN] Специфические сообщения порта

#define TXT_VERSION "DTHCBZ 2>4 (31>12>2017)"         // ВЕРСИЯ 2.4 (31.12.2017)

#define TXT_ALWAYSRUN_ON "GJCNJZYYSQ ,TU DRK.XTY"     // ПОСТОЯННЫЙ БЕГ ВКЛЮЧЕН
#define TXT_ALWAYSRUN_OFF "GJCNJZYYSQ ,TU DSRK.XTY"   // ПОСТОЯННЫЙ БЕГ ВЫКЛЮЧЕН

#define TXT_MLOOK_ON "J,PJH VSIM. DRK.XTY"            // ОБЗОР МЫШЬЮ ВКЛЮЧЕН
#define TXT_MLOOK_OFF "J,PJH VSIM. DSRK.XTY"          // ОБЗОР МЫШЬЮ ВЫКЛЮЧЕН

#define TXT_SECRET_FOUND "J,YFHE;TY NFQYBR!"          // ОБНАРУЖЕН ТАЙНИК!
