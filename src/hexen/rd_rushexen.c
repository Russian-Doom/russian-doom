//
// Copyright(C) 2020-2022 Dasperal
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



#include "i_swap.h"
#include "m_misc.h"
#include "rd_rushexen.h"
#include "w_wad.h"

#define US "YTJ;BLFYYFZ CNHJRF! 'NJ ,FU!" // НЕОжИДАННАЯ СТРОКА! эТО БАГ!

#define RECORD(a, c, v) {a, (int32_t) SDL_SwapLE32(c), (int32_t) SDL_SwapLE32(v)}
#define CMD_NOP 0 // 0 args
#define CMD_LSPEC3DIRECT 11 // 4 args
#define CMD_DELAYDIRECT 56 //1 args
#define LAST_EXTERNAL_CMD 101
#define CMD_TABLE_DELAY_DIRECT (LAST_EXTERNAL_CMD + 1) // 1 arg
#define CMD_PRINT_BOLD_ALWAYS_WITH_TABLE_DELAY_DIRECT (LAST_EXTERNAL_CMD + 2) // 2 args
#define CMD_PRINT_BOLD_RUSSIAN_DIRECT (LAST_EXTERNAL_CMD + 3) // 1 arg
#define CMD_PRINT_NUMBER_OR_PRINT_STRING_DIRECT (LAST_EXTERNAL_CMD + 4) // 1 arg
#define CMD_PRINT_STRING_DIRECT_OR_PRINT_NUMBER (LAST_EXTERNAL_CMD + 5) // 1 arg
#define CMD_PRINT_ALWAYS_WITH_TABLE_DELAY_DIRECT (LAST_EXTERNAL_CMD + 6) // 2 args
#define CMD_PRINT_RUSSIAN_DIRECT (LAST_EXTERNAL_CMD + 7) // 1 arg
#define CMD_PRINT_SCRIPTVAR_AND_STRING_ENGLISH_DIRECT (LAST_EXTERNAL_CMD + 8) // 2 arg
#define CMD_PRINT_MAPVAR_AND_STRING_ENGLISH_DIRECT (LAST_EXTERNAL_CMD + 9) // 2 arg
#define CMD_GT2EQ (LAST_EXTERNAL_CMD + 10) // 0 arg

const char* Hexen_Map_01_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    "LDTHM PF,KJRBHJDFYF", // ДВЕРЬ ЗАбЛОКИРОВАНА
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_02_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "GHBDTNCNDE.< CVTHNYSQ", // ПРИВЕТСТВУюБ СМЕРТНЫЙ
    US,
    "UJNJD KB NS EVTHTNM?", // ГОТОВ ЛИ ТЫ УМЕРЕТЬ,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "LDTHM JNRHSNF E CNHF;F KMLF", // ДВЕРЬ ОТКРЫТА У СТРАжА ЛЬДА
    US,
    US,
    US
};

const char* Hexen_Map_03_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "CTQXFC 'NJN GENM PFRHSN", // СЕЙЧАС эТОТ ПУТЬ ЗАКРЫТ
    US,
    US
};

const char* Hexen_Map_04_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "GJKJDBYF UJKJDJKJVRB HFPUFLFYF>>>", // ПОЛОВИНА ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>YF CTVB GJHNFKF[", // ЮЮЮНА СЕМИ ПОРТАЛАх
    "NHTNM UJKJDJKJVRB HFPUFLFYF>>>", // ТРЕТЬ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    "KTCNYBWF DJPLDBUYTNCZ YF CTVB GJHNFKF[", // ЛЕСТНИЦА ВОЗДВИГНЕТСЯ НА СЕМИ ПОРТАЛАх
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_05_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "NHTNM UJKJDJKJVRB HFPUFLFYF>>>", // ТРЕТЬ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>YF CTVB GJHNFKF[", // ЮЮЮНА СЕМИ ПОРТАЛАх
    "KTCNYBWF DJPLDBUYTNCZ YF CTVB GJHNFKF[", // ЛЕСТНИЦА ВОЗДВИГНЕТСЯ НА СЕМИ ПОРТАЛАх
    "JCNFKCZ TOT JLBY GTHTRK.XFNTKM>>>", // ОСТАЛСЯ ЕЩЕ ОДИН ПЕРЕКЛюЧАТЕЛЬЮЮЮ
    "RFVTYYFZ GHTUHFLF JNCNEGBN>>>", // КАМЕННАЯ ПРЕГРАДА ОТСТУПИТЮЮЮ
    US,
    US
};

const char* Hexen_Map_08_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ОДНА ШЕСТАЯ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>D KTCE NTYTQ", // ЮЮЮВ ЛЕСУ ТЕНЕЙ
    US,
    US,
    "LDTHM PF,KJRBHJDFYF BPYENHB", // ДВЕРЬ ЗАбЛОКИРОВАНА ИЗНУТРИ
    "CKSITY PDER JNRHSDF.OTQCZ LDTHB", // СЛЫШЕН ЗВУК ОТКРЫВАюЩЕЙСЯ ДВЕРИ
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_09_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ОДНА ШЕСТАЯ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>D KTCE NTYTQ", // ЮЮЮВ ЛЕСУ ТЕНЕЙ
    US,
    US,
    US
};

const char* Hexen_Map_10_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ОДНА ШЕСТАЯ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>D KTCE NTYTQ", // ЮЮЮВ ЛЕСУ ТЕНЕЙ
    US,
    US,
    US,
    US
};

const char* Hexen_Map_11_StringTable[] = {
    "JCNFKJCM 'NNBYJD: ", // ОСТАЛОСЬ эТТИНОВЖ 
    "CKBIRJV LJKUJ< UJNJDMCZ R CVTHNB!", // СЛИШКОМ ДОЛГОБ ГОТОВЬСЯ К СМЕРТИ!
    US,
    US,
    US,
    US,
    US,
    "LDTHM JNRHJTNCZ YF GJRBYENJQ PFCNFDT", // ДВЕРЬ ОТКРОЕТСЯ НА ПОКИНУТОЙ ЗАСТАВЕ
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_12_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "CTQXFC 'NF LDTHM PFRHSNF", // СЕЙЧАС эТА ДВЕРЬ ЗАКРЫТА
    US,
    US,
    US
};

const char* Hexen_Map_13_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "VJB GHBCKE;YBRB XE.N NDJ. RHJDM< XTKJDTR", // МОИ ПРИСЛУжНИКИ ЧУюТ ТВОю КРОВЬБ ЧЕЛОВЕК
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_21_StringTable[] = {
    "GHJ[JL ,ELTN JNRHSN DJPKT DBCTKBWS", // ПРОхОД бУДЕТ ОТКРЫТ ВОЗЛЕ ВИСЕЛИЦЫ
    US,
    "LDTHM PF,KJRBHJDFYF BPYENHB", // ДВЕРЬ ЗАбЛОКИРОВАНА ИЗНУТРИ
    US,
    US,
    US,
    US
};

const char* Hexen_Map_22_StringTable[] = {
    US,
    US,
    US,
    "GKJOFLRF JGECNBKFCM D WTYNHFKMYJQ ,FIYT", // ПЛОЩАДКА ОПУСТИЛАСЬ В ЦЕНТРАЛЬНОЙ бАШНЕ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "NS CKBIRJV PFBUHFKCZ< CVTHNYSQ>>>", // ТЫ СЛИШКОМ ЗАИГРАЛСЯБ СМЕРТНЫЙЮЮЮ
    US,
    "GHBIKJ DHTVZ PFDTHIBNM NDJ. GFHNB.", // ПРИШЛО ВРЕМЯ ЗАВЕРШИТЬ ТВОю ПАРТИю
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_23_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "BP CTDTHYJUJ PFKF LJYJCBNCZ PDER>>>", // ИЗ СЕВЕРНОГО ЗАЛА ДОНОСИТСЯ ЗВУКЮЮЮ
    US,
    US,
    US,
    ">>>GJLYBVF.OTQCZ RFVTYYJQ RJKJYYS" // 14 : extra string : ЮЮЮПОДНИМАюЩЕЙСЯ КАМЕННОЙ КОЛОННЫ
};

const char* Hexen_Map_27_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "GHTRKJYBCM GHTLJ VYJQ<", // ПРЕКЛОНИСЬ ПРЕДО МНОЙБ
    US,
    "B< VJ;TN ,SNM< Z ,ELE VBKJCTHLTY", // ИБ МОжЕТ бЫТЬБ Я бУДУ МИЛОСЕРДЕН
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "F< VJ;TN ,SNM< B YTN" // 18 : extra string : АБ МОжЕТ бЫТЬБ И НЕТ
};

const char* Hexen_Map_28_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ОДНА ДЕВЯТАЯ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>D CTVBYFHBB THTCBFH[F", // ЮЮЮВ СЕМИНАРИИ ЕРЕСИАРхА
    US,
    US,
};

const char* Hexen_Map_30_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ОДНА ДЕВЯТАЯ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>D CTVBYFHBB THTCBFH[F", // ЮЮЮВ СЕМИНАРИИ ЕРЕСИАРхА
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_34_StringTable[] = {
    US,
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ОДНА ДЕВЯТАЯ ГОЛОВОЛОМКИ РАЗГАДАНАЮЮЮ
    ">>>D CTVBYFHBB THTCBFH[F", // ЮЮЮВ СЕМИНАРИИ ЕРЕСИАРхА
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_35_StringTable[] = {
    "DHFNF PFRHSKBCM>>>", // ВРАТА ЗАКРЫЛИСЬЮЮЮ
    "CLTKFQ CDJQ DS,JH", // СДЕЛАЙ СВОЙ ВЫбОР
    US,
    "LDTHM PFGTHNF BPYENHB", // ДВЕРЬ ЗАПЕРТА ИЗНУТРИ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "LJCNFNJXYJ KB NS CBKTY<", // ДОСТАТОЧНО ЛИ ТЫ СИЛЕНБ
    US,
    "XNJ,S CHFPBNMCZ CJ CDJBVB YFCNFDYBRFVB?", // ЧТОбЫ СРАЗИТЬСЯ СО СВОИМИ НАСТАВНИКАМИ,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_33_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "NS GJCVTK CHF;FNMCZ D RJVYFNT J;BLFYBZ?", // ТЫ ПОСМЕЛ СРАжАТЬСЯ В КОМНАТЕ ОжИДАНИЯ,
    "NFR EVHB ;T!" // ТАК УМРИ жЕ!
};

const char* Hexen_DK_Map_41_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "GJNJR DJLS JCNFYJDKTY", // ПОТОК ВОДЫ ОСТАНОВЛЕН
    "GJNJR DJLS GHTUHF;LFTN GENM", // ПОТОК ВОДЫ ПРЕГРАжДАЕТ ПУТЬ
    "LDTHM JNRHSKFCM D XFCJDYT", // ДВЕРЬ ОТКРЫЛАСЬ В ЧАСОВНЕ
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_42_StringTable[] = {
    US,
    US,
    US,
    US,
    "RFR CNHFYYJ>>>", // КАК СТРАННОЮЮЮ
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_44_StringTable[] = {
    US,
    "JCNFKJCM NHB XFCNB UJKJDJKJVRB", // ОСТАЛОСЬ ТРИ ЧАСТИ ГОЛОВОЛОМКИ
    "JCNFKJCM LDT XFCNB UJKJDJKJVRB", // ОСТАЛОСЬ ДВЕ ЧАСТИ ГОЛОВОЛОМКИ
    "JCNFKFCM JLYF XFCNM UJKJDJKJVRB", // ОСТАЛАСЬ ОДНА ЧАСТЬ ГОЛОВОЛОМКИ
    "UJKJDJKJVRF HFPUFLFYF", // ГОЛОВОЛОМКА РАЗГАДАНА
    US,
    "UJKJDJKJVRF YT HFPUFLFYF", // ГОЛОВОЛОМКА НЕ РАЗГАДАНА
    US,
    "GJK CJDCTV GHJUYBK!", // ПОЛ СОВСЕМ ПРОГНИЛ!
    US,
    "JLYF NHTNM UJKJDJKJVRB HFPUFLFYF", // ОДНА ТРЕТЬ ГОЛОВОЛОМКИ РАЗГАДАНА
    "LDT NHTNB UJKJDJKJVRB HFPUFLFYS", // ДВЕ ТРЕТИ ГОЛОВОЛОМКИ РАЗГАДАНЫ
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_45_StringTable[] = {
    US,
    "YTGJLFKTRE HFPLFTNCZ PDER>>>", // НЕПОДАЛЕКУ РАЗДАЕТСЯ ЗВУКЮЮЮ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    ">>>LDB;EOTQCZ LTHTDZYYJQ GKJOFLRB" // 10 : extra string : ЮЮЮДВИжУЩЕЙСЯ ДЕРЕВЯННОЙ ПЛОЩАДКИ
};

const char* Hexen_DK_Map_46_StringTable[] = {
    "UJNJDJ>>>", // ГОТОВОЮЮЮ
    "UJKJDJKJVRF YT HFPUFLFYF", // ГОЛОВОЛОМКА НЕ РАЗГАДАНА
    "Z NT,Z GHTLEGHT;LF.>>>", // Я ТЕбЯ ПРЕДУПРЕжДАюЮЮЮ
    "YT CKBIRJV KB NS EGHZVSQ?", // НЕ СЛИШКОМ ЛИ ТЫ УПРЯМЫЙ,
    "B YT CKBIRJV-NJ HFPEVYSQ!", // И НЕ СЛИШКОМ-ТО РАЗУМНЫЙ!
    US,
    US,
    US,
    "JLYF XTNDTHNFZ 'NJQ UJKJDJKJVRB HFPUFLFYF", // ОДНА ЧЕТВЕРТАЯ эТОЙ ГОЛОВОЛОМКИ РАЗГАДАНА
    "GKJ[JQ DS,JH>>>", // ПЛОхОЙ ВЫбОРЮЮЮ
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_47_StringTable[] = {
    US,
    US,
    "CBVDJKS YT CJDGFLF.N", // СИМВОЛЫ НЕ СОВПАДАюТ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_48_StringTable[] = {
    US,
    US,
    "C 'NJQ CNJHJYS LDTHM YT JNRHSNM", // С эТОЙ СТОРОНЫ ДВЕРЬ НЕ ОТКРЫТЬ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_50_StringTable[] = {
    US,
    "LDTHM PF,KJRBHJDFYF CYFHE;B", // ДВЕРЬ ЗАбЛОКИРОВАНА СНАРУжИ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_51_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    "CDZNJNFNCNDJ!", // СВЯТОТАТСТВО!
    "NS JCRDTHYBK VJUBKE 'HBRF!!", // ТЫ ОСКВЕРНИЛ МОГИЛУ эРИКА!!
    "B EVHTIM PF 'NJ CNHFIYJQ CVTHNM.!!!", // И УМРЕШЬ ЗА эТО СТРАШНОЙ СМЕРТЬю!!!
    "JLYF NHTNM UJKJDJKJVRB HFPUFLFYF", // ОДНА ТРЕТЬ ГОЛОВОЛОМКИ РАЗГАДАНА
    "LDT NHTNB UJKJDJKJVRB HFPUFLFYS", // ДВЕ ТРЕТИ ГОЛОВОЛОМКИ РАЗГАДАНЫ
    "CRKTG JNRHSN", // СКЛЕП ОТКРЫТ
    "JGFCFQCZ UHJ,YBWS GFERF>>>", // ОПАСАЙСЯ ГРОбНИЦЫ ПАУКАЮЮЮ
    US,
    "CYFHE;B CKSITY PDER GJLYBVF.OTUJCZ RFVYZ", // СНАРУжИ СЛЫШЕН ЗВУК ПОДНИМАюЩЕГОСЯ КАМНЯ
    "&XEDCNDETIM KB NS CT,Z DTPEXBV?&", // ?ЧУВСТВУЕШЬ ЛИ ТЫ СЕбЯ ВЕЗУЧИМ,?
    "YTGHFDBKMYJT GHTLGJKJ;TYBT!", // НЕПРАВИЛЬНОЕ ПРЕДПОЛОжЕНИЕ!
    "GHFDBKMYJT GHTLGJKJ;TYBT", // ПРАВИЛЬНОЕ ПРЕДПОЛОжЕНИЕ
    "&VJ;TIM YFGBCFNM PF VTYZ DCT CRHBGNS?&", // ?МОжЕШЬ НАПИСАТЬ ЗА МЕНЯ ВСЕ СКРИПТЫ,?
    "&YT NHJUFQ VJ. DRECYZIRE&", // ?НЕ ТРОГАЙ МОю ВКУСНЯШКУ?
    "&JCNHTYMRJ ?!?!?!&", // ?ОСТРЕНЬКО ,!,!,!?
    "&GJLFQ-RF VYT CF[FH< LTNRF&", // ?ПОДАЙ-КА МНЕ САхАРБ ДЕТКА?
    "&FUF-F-F-F>>>&", // ?АГА-А-А-АЮЮЮ?
    "&ABKMV YF XFC?&", // ?ФИЛЬМ НА ЧАС,?
    "&E VTYZ LF;T YTN CDJTUJ YFLUHJ,BZ (R>A>)&", // ?У МЕНЯ ДАжЕ НЕТ СВОЕГО НАДГРОбИЯ (КЮФЮ)?
    "LF YT GHJKMTNCZ RHJDM>>>", // ДА НЕ ПРОЛЬЕТСЯ КРОВЬЮЮЮ
    "B LF YT GJLYBVTNCZ HERF DJ UYTDT>>>", // И ДА НЕ ПОДНИМЕТСЯ РУКА ВО ГНЕВЕЮЮЮ
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_52_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "RNJ JCVTKBKCZ GJNHTDJ;BNM YFI GJRJQ?!", // КТО ОСМЕЛИЛСЯ ПОТРЕВОжИТЬ НАШ ПОКОЙ,!
    "GENM JNRHSN", // ПУТЬ ОТКРЫТ
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_53_StringTable[] = {
    US,
    US,
    "JCNFKJCM GTHTRK.XFNTKTQ: ", // ОСТАЛОСЬ ПЕРЕКЛюЧАТЕЛЕЙЖ
    "",
    "JCNFKCZ DCTUJ JLBY GTHTRK.XFNTKM", // ОСТАЛСЯ ВСЕГО ОДИН ПЕРЕКЛюЧАТЕЛЬ
    US,
    "DS[JL JNRHSN", // ВЫхОД ОТКРЫТ
    US,
    US,
    US
};

const char* Hexen_DK_Map_54_StringTable[] = {
    US,
    "C 'NJQ CNJHJYS LDTHM YT JNRHSNM", // С эТОЙ СТОРОНЫ ДВЕРЬ НЕ ОТКРЫТЬ
    US,
    US,
    "GENM D WBNFLTKM JNRHSN>>>", // ПУТЬ В ЦИТАДЕЛЬ ОТКРЫТЮЮЮ
    ">>>JCVTKBIMCZ KB NS DJQNB?", // ЮЮЮОСМЕЛИШЬСЯ ЛИ ТЫ ВОЙТИ,
    US,
    US,
    US,
    "LDTHM JNRHJTNCZ>>>", // ДВЕРЬ ОТКРОЕТСЯЮЮЮ
    ">>>D XFCJDYT", // ЮЮЮВ ЧАСОВНЕ
    "VJCN DJPLDBUYTNCZ>>>", // МОСТ ВОЗДВИГНЕТСЯЮЮЮ
    ">>>YF ,JQYT", // ЮЮЮНА бОЙНЕ
    "KTCNYBWF DJPLDBUYTNCZ>>>", // ЛЕСТНИЦА ВОЗДВИГНЕТСЯЮЮЮ
    ">>>E NTVYJUJ CNHF;F", // ЮЮЮУ ТЕМНОГО СТРАжА
    "ITCNTHYZ ECNFYJDKTYF", // ШЕСТЕРНЯ УСТАНОВЛЕНА
    "ITCNTHYTQ ECNFYJDKTYJ: ", // ШЕСТЕРНЕЙ УСТАНОВЛЕНОЖ 
    "GHTUHFLF GJLYBVTNCZ>>>", // ПРЕГРАДА ПОДНИМЕТСЯЮЮЮ
    ">>>D RKJFRT", // ЮЮЮВ КЛОАКЕ
    US,
    "GENM YFPFL JNRHSN", // ПУТЬ НАЗАД ОТКРЫТ
    US,
    US,
    US
};

const char* Hexen_DK_Map_55_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "LDTHM PF,KJRBHJDFYF BPYENHB", // ДВЕРЬ ЗАбЛОКИРОВАНА ИЗНУТРИ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_56_StringTable[] = {
    "NS GJCVTK JUHF,BNM VJUBKE>>>", // ТЫ ПОСМЕЛ ОГРАбИТЬ МОГИЛУЮЮЮ
    ">>>DTH[JDYJUJ GFKFXF?", // ЮЮЮВЕРхОВНОГО ПАЛАЧА,
    "UJNJDMCZ R CVTHNB", // ГОТОВЬСЯ К СМЕРТИ
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_59_StringTable[] = {
    US,
    "JCNFKJCM YFQNB GTHTRK.XFNTKTQ: ", // ОСТАЛОСЬ НАЙТИ ПЕРЕКЛюЧАТЕЛЕЙЖ 
    "",
    "JCNFKCZ GJCKTLYBQ GTHTRK.XFNTKM", // ОСТАЛСЯ ПОСЛЕДНИЙ ПЕРЕКЛюЧАТЕЛЬ
    US,
    "GENM R ,FIYT JNRHSN", // ПУТЬ К бАШНЕ ОТКРЫТ
    US,
    US
};

const char* Hexen_DK_Map_60_StringTable[] = {
    US,
    US,
    US,
    "GJHNFK JNRHSN>>>", //ПОРТАЛ ОТКРЫТЮЮЮ
    US,
    US,
    US,
    US,
    US,
    US,
    US
};

const int delayTable[][2] = {
    // eng    rus
    {   70,   100  }, // 0  Hexen_Map_02 : Korax lines GREETINGS, DIE
    {   96,   192  }, // 1  Hexen_Map_04, Hexen_Map_05, Hexen_Map_08, Hexen_Map_28, Hexen_Map_30 : Multi line prints
    {   0,    192  }, // 2  Hexen_Map_05 : Exploded print
    {  105,   120  }, // 3  Hexen_Map_13 : Korax line SERVANTS
    {  164,   140  }, // 4  Hexen_Map_22 : Korax line TOO LONG
    {   16,    20  }, // 5  Hexen_Map_22 : Korax line REMOVE
    {   0,    128  }, // 6  Hexen_Map_23, Hexen_DK_Map_45 : Exploded print
    {  196,   100  }, // 7  Hexen_Map_27 : Korax line WORSHIP
    {   0,    160  }, // 8  Hexen_Map_27 : Korax line MERCIFUL
    {  132,   120  }, // 9  Hexen_Map_27 : Korax line MAYBE NOT
    {   72,   100  }, // 10 Hexen_Map_35 : Korax line STRONG
    {  105,   140  }, // 11 Hexen_Map_35 : Korax line MASTERS
    {   64,   128  }, // 12 Hexen_DK_Map_33 : Multi line prints
    {   70,   128  }, // 13 Hexen_DK_Map_54 : Multi line prints
};

// [JN] Shutup GCC warning about missing braces around initializer.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"

const CMDInjectionRecord_t Hexen_Map_02_CMDInjectionTable[] = {
    RECORD(0xFC4, CMD_TABLE_DELAY_DIRECT, 0), //Korax: GREETINGS delay
    RECORD(0xFF4, CMD_TABLE_DELAY_DIRECT, 0), //Korax: READY delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_04_CMDInjectionTable[] = {
    RECORD(0x1450, CMD_TABLE_DELAY_DIRECT, 1), //ONE HALF delay
    RECORD(0x1588, CMD_TABLE_DELAY_DIRECT, 1), //ONE THIRD delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_05_CMDInjectionTable[] = {
    RECORD(0x1248, CMD_TABLE_DELAY_DIRECT, 1), //ONE THIRD delay
    RECORD(0x1374, CMD_TABLE_DELAY_DIRECT, 1), //ONE THIRD delay
    //Exploded print
    RECORD(0x1784, CMD_PRINT_BOLD_ALWAYS_WITH_TABLE_DELAY_DIRECT, 10), //STONES GRIND...
    RECORD(0x178C, 2, 0), // 0 will be owerwriten by next RECORD
    RECORD(0x1790, CMD_PRINT_BOLD_RUSSIAN_DIRECT, 7), // ...ON THE SEVEN PORTALS
    //End of exploded print
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_08_CMDInjectionTable[] = {
    RECORD(0x118, CMD_TABLE_DELAY_DIRECT, 1), //ONE SIXTH
    RECORD(0x280, CMD_TABLE_DELAY_DIRECT, 1), //ONE SIXTH
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_09_CMDInjectionTable[] = {
    RECORD(0x13C, CMD_TABLE_DELAY_DIRECT, 1), //ONE SIXTH
    RECORD(0x2C4, CMD_TABLE_DELAY_DIRECT, 1), //ONE SIXTH
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_10_CMDInjectionTable[] = {
    RECORD(0x460, CMD_TABLE_DELAY_DIRECT, 1), //ONE SIXTH
    RECORD(0x5C0, CMD_TABLE_DELAY_DIRECT, 1), //ONE SIXTH
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_11_CMDInjectionTable[] = {
    //Reordered print: "... ETTINS LEFT" in eng and "ETTINS LEFT: ..." in rus
    RECORD(0x64, CMD_PRINT_NUMBER_OR_PRINT_STRING_DIRECT, 0),
    RECORD(0x6C, CMD_PRINT_STRING_DIRECT_OR_PRINT_NUMBER, 0),
    //End of reordered print
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_13_CMDInjectionTable[] = {
    RECORD(0x16A0, CMD_TABLE_DELAY_DIRECT, 3), //Korax: SERVANTS delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_22_CMDInjectionTable[] = {
    RECORD(0x1710, CMD_TABLE_DELAY_DIRECT, 4), //Korax: TOO LONG delay
    RECORD(0x1740, CMD_TABLE_DELAY_DIRECT, 5), //Korax: REMOVE delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_23_CMDInjectionTable[] = {
    RECORD(0x4AC, 0, CMD_GT2EQ), // serpent bug fix
    RECORD(0x4BC, CMD_DELAYDIRECT, 1),
    //Move special call
    RECORD(0x8BC, CMD_LSPEC3DIRECT, 12),
    RECORD(0x8C4, 8, 32),
    //Exploded print
    RECORD(0x8CC, 0, CMD_PRINT_ALWAYS_WITH_TABLE_DELAY_DIRECT), // end of "move special call" and start of "exploded print"
    RECORD(0x8D4, 10, 6),
    RECORD(0x8DC, CMD_PRINT_RUSSIAN_DIRECT, 14),
    //End of exploded print
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_27_CMDInjectionTable[] = {
    RECORD(0xFA8, CMD_TABLE_DELAY_DIRECT, 7), //Korax : WORSHIP delay
    //Exploded print
    RECORD(0xFB0, CMD_PRINT_ALWAYS_WITH_TABLE_DELAY_DIRECT, 10),
    RECORD(0xFB8, 8, 0), // 0 will be owerwriten by next RECORD // Korax : MERCIFUL delay
    RECORD(0xFBC, CMD_PRINT_RUSSIAN_DIRECT, 18),
    //End of exploded print
    RECORD(0xFD8, CMD_TABLE_DELAY_DIRECT, 9), //Korax : MAYBE NOT delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_28_CMDInjectionTable[] = {
    RECORD(0x4F0, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    RECORD(0x670, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    RECORD(0x7B8, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_30_CMDInjectionTable[] = {
    RECORD(0x190, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    RECORD(0x53C, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    RECORD(0x698, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_34_CMDInjectionTable[] = {
    RECORD(0x94, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    RECORD(0x8C4, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    RECORD(0xA44, CMD_TABLE_DELAY_DIRECT, 1), //ONE NINTH delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Map_35_CMDInjectionTable[] = {
    RECORD(0x9D8, CMD_TABLE_DELAY_DIRECT, 10), //Korax: STRONG delay
    RECORD(0xA08, CMD_TABLE_DELAY_DIRECT, 11), //Korax: MASTERS delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Demo_Map_02_CMDInjectionTable[] = {
    RECORD(0xF80, CMD_TABLE_DELAY_DIRECT, 0), //Korax: GREETINGS delay
    RECORD(0xFB0, CMD_TABLE_DELAY_DIRECT, 0), //Korax: READY delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_Demo_Map_04_CMDInjectionTable[] = {
    RECORD(0x1440, CMD_TABLE_DELAY_DIRECT, 1), //ONE HALF delay
    RECORD(0x1578, CMD_TABLE_DELAY_DIRECT, 1), //ONE THIRD delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_DK_Map_33_CMDInjectionTable[] = {
    RECORD(0x28C, CMD_TABLE_DELAY_DIRECT, 12), //YOU DARE delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_DK_Map_45_CMDInjectionTable[] = {
    //Exploded print
    RECORD(0x300, CMD_PRINT_BOLD_ALWAYS_WITH_TABLE_DELAY_DIRECT, 1), //YOU HEAR...
    RECORD(0x308, 6, 0), // 0 will be owerwriten by next RECORD
    RECORD(0x30C, CMD_PRINT_BOLD_RUSSIAN_DIRECT, 10), // ...PLATFORM MOVING
    //End of exploded print
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_DK_Map_53_CMDInjectionTable[] = {
    //Shrinked print
    RECORD(0xF44, CMD_PRINT_SCRIPTVAR_AND_STRING_ENGLISH_DIRECT, 0), // SWITCH LEFT
    RECORD(0xF4C, 5, CMD_NOP),
    RECORD(0xF54, CMD_NOP, CMD_NOP),
    //End of shrinked print
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_DK_Map_54_CMDInjectionTable[] = {
    RECORD(0x638, CMD_TABLE_DELAY_DIRECT, 13), //A DOOR delay
    RECORD(0x778, CMD_TABLE_DELAY_DIRECT, 13), //A BRIDGE  delay
    RECORD(0x8B8, CMD_TABLE_DELAY_DIRECT, 13), // A STAIR delay
    //Reordered print: "... GEARS HAVE BEEN PLACED" in eng and "GEARS HAVE BEEN PLACED: ..." in rus
    RECORD(0xA00, CMD_PRINT_NUMBER_OR_PRINT_STRING_DIRECT, 16),
    RECORD(0xA08, CMD_PRINT_STRING_DIRECT_OR_PRINT_NUMBER, 16),
    //End of reordered print
    RECORD(0xAB0, CMD_TABLE_DELAY_DIRECT, 13), // A BARRICADE delay
    {0, 0, 0} //Important!
};

const CMDInjectionRecord_t Hexen_DK_Map_59_CMDInjectionTable[] = {
    //Shrinked print
    RECORD(0x330, CMD_PRINT_MAPVAR_AND_STRING_ENGLISH_DIRECT, 1), // SWITCH LEFT
    RECORD(0x338, 4, CMD_NOP),
    RECORD(0x340, CMD_NOP, CMD_NOP),
    //End of shrinked print
    {0, 0, 0} //Important!
};

#pragma GCC diagnostic pop 
// [JN] ENDOF #pragma GCC diagnostic ignored "-Wmissing-braces"

const char** stringTables[] = {
    NULL, //MAP00
    Hexen_Map_01_StringTable,
    Hexen_Map_02_StringTable,
    Hexen_Map_03_StringTable,
    Hexen_Map_04_StringTable,
    Hexen_Map_05_StringTable,
    NULL, //MAP06
    NULL, //MAP07
    Hexen_Map_08_StringTable,
    Hexen_Map_09_StringTable,
    Hexen_Map_10_StringTable,
    Hexen_Map_11_StringTable,
    Hexen_Map_12_StringTable,
    Hexen_Map_13_StringTable,
    NULL, //MAP14
    NULL, //MAP15
    NULL, //MAP16
    NULL, //MAP17
    NULL, //MAP18
    NULL, //MAP19
    NULL, //MAP20
    Hexen_Map_21_StringTable,
    Hexen_Map_22_StringTable,
    Hexen_Map_23_StringTable,
    NULL, //MAP24
    NULL, //MAP25
    NULL, //MAP26
    Hexen_Map_27_StringTable,
    Hexen_Map_28_StringTable,
    NULL, //MAP29
    Hexen_Map_30_StringTable,
    NULL, //MAP31
    NULL, //MAP32
    NULL, //MAP33
    Hexen_Map_34_StringTable,
    Hexen_Map_35_StringTable,
    NULL, //MAP36
    NULL, //MAP37
    NULL, //MAP38
    NULL, //MAP39
    NULL, //MAP40
    Hexen_DK_Map_41_StringTable,
    Hexen_DK_Map_42_StringTable,
    NULL, //MAP43
    Hexen_DK_Map_44_StringTable,
    Hexen_DK_Map_45_StringTable,
    Hexen_DK_Map_46_StringTable,
    Hexen_DK_Map_47_StringTable,
    Hexen_DK_Map_48_StringTable,
    NULL, //MAP49
    Hexen_DK_Map_50_StringTable,
    Hexen_DK_Map_51_StringTable,
    Hexen_DK_Map_52_StringTable,
    Hexen_DK_Map_53_StringTable,
    Hexen_DK_Map_54_StringTable,
    Hexen_DK_Map_55_StringTable,
    Hexen_DK_Map_56_StringTable,
    NULL, //MAP57
    NULL, //MAP58
    Hexen_DK_Map_59_StringTable,
    Hexen_DK_Map_60_StringTable
};

const CMDInjectionRecord_t* CMDInjectionTables[] = {
    NULL, // MAP00
    NULL, // MAP01
    Hexen_Map_02_CMDInjectionTable,
    NULL, // MAP03
    Hexen_Map_04_CMDInjectionTable,
    Hexen_Map_05_CMDInjectionTable,
    NULL, // MAP06
    NULL, // MAP07
    Hexen_Map_08_CMDInjectionTable,
    Hexen_Map_09_CMDInjectionTable,
    Hexen_Map_10_CMDInjectionTable,
    Hexen_Map_11_CMDInjectionTable,
    NULL, // MAP12
    Hexen_Map_13_CMDInjectionTable,
    NULL, // MAP14
    NULL, // MAP15
    NULL, // MAP16
    NULL, // MAP17
    NULL, // MAP18
    NULL, // MAP19
    NULL, // MAP20
    NULL, // MAP21
    Hexen_Map_22_CMDInjectionTable,
    Hexen_Map_23_CMDInjectionTable,
    NULL, // MAP24
    NULL, // MAP25
    NULL, // MAP26
    Hexen_Map_27_CMDInjectionTable,
    Hexen_Map_28_CMDInjectionTable,
    NULL, // MAP29
    Hexen_Map_30_CMDInjectionTable,
    NULL, // MAP31
    NULL, // MAP32
    NULL, // MAP33
    Hexen_Map_34_CMDInjectionTable,
    Hexen_Map_35_CMDInjectionTable,
    NULL, // MAP36
    NULL, // MAP37
    NULL, // MAP38
    NULL, // MAP39
    NULL, // MAP40
    NULL, // MAP41
    NULL, // MAP42
    NULL, // MAP43
    NULL, // MAP44
    Hexen_DK_Map_45_CMDInjectionTable,
    NULL, // MAP46
    NULL, // MAP47
    NULL, // MAP48
    NULL, // MAP49
    NULL, // MAP50
    NULL, // MAP51
    NULL, // MAP52
    Hexen_DK_Map_53_CMDInjectionTable,
    Hexen_DK_Map_54_CMDInjectionTable,
    NULL, // MAP55
    NULL, // MAP56
    NULL, // MAP57
    NULL, // MAP58
    Hexen_DK_Map_59_CMDInjectionTable,
    NULL  // MAP60
};

const char** GetRusStringTable(int map)
{
    if (isDK && map < 41) // DK DM maps
    {
        if (map == 33)
        {
            return Hexen_DK_Map_33_StringTable;
        }
        else
        {
            return NULL;
        }
    }
    return stringTables[map]; //Hexen original, DK story, Hexen Demo(String tables equal to Hexen original)
}

const CMDInjectionRecord_t* GetCMDInjectionTable(int map)
{
    if (isHexenDemo && map < 5) //Hexen Demo maps
    {
        if (map == 2)
        {
            return Hexen_Demo_Map_02_CMDInjectionTable;
        }
        else if(map == 4)
        {
            return Hexen_Demo_Map_04_CMDInjectionTable;
        }
        else
        {
            return NULL;
        }
    }
    if (isDK && map < 41) // DK DM maps
    {
        if (map == 33)
        {
            return Hexen_DK_Map_33_CMDInjectionTable;
        }
        else
        {
            return NULL;
        }
    }
    return CMDInjectionTables[map]; //Hexen original, DK story
}

boolean cantApplyACSInstrumentation(int map)
{
    char lumpname[9];
    int lumpCount;

    if(!hasUnknownPWads)
        return false;

    if(map > 60)
        return true;

    M_snprintf(lumpname, sizeof(lumpname), "MAP%02d", map);
    lumpCount = W_CheckMultipleLumps(lumpname);

    if(lumpCount == 1 || (isDK && map == 33 && lumpCount == 2) || (isDK && map > 33 && map < 41))
        return false;

    if(stringTables[map] == NULL && CMDInjectionTables[map] == NULL && !(isDK && map == 33))
        return false;

    return true;
}
