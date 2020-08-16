//
// Copyright(C) 2020 Dasperal
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



#include "rushexen.h"
#include "i_swap.h"

#define US "YTJ;BLFYYFZ CNHJRF! 'NJ <FU!" // Õ≈ŒÊ»ƒ¿ÕÕ¿ﬂ —“–Œ ¿! ˝“Œ ¡¿√!

#define RECORD(a, c, v) {a, (int32_t) SDL_SwapLE32(c), (int32_t) SDL_SwapLE32(v)}

const char* Hexen_Map_01_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    "LDTHM PF,KJRBHJDFYF", // ƒ¬≈–‹ «¿·ÀŒ »–Œ¬¿Õ¿
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
    "GHBDTNCNDE.< CVTHNYSQ", // œ–»¬≈“—“¬”˛¡ —Ã≈–“Õ€…
    US,
    "UJNJD KB NS EVTHTNM?", // √Œ“Œ¬ À» “€ ”Ã≈–≈“‹,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "LDTHM JNRHSNF E CNHF;F KMLF", // ƒ¬≈–‹ Œ“ –€“¿ ” —“–¿Ê¿ À‹ƒ¿
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
    "CTQXFC 'NJN GENM PFRHSN", // —≈…◊¿— ˝“Œ“ œ”“‹ «¿ –€“
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
    "GJKJDBYF UJKJDJKJVRB HFPUFLFYF>>>", // œŒÀŒ¬»Õ¿ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>YF CTVB GJHNFKF[", // ﬁﬁﬁÕ¿ —≈Ã» œŒ–“¿À¿ı
    "NHTNM UJKJDJKJVRB HFPUFLFYF>>>", // “–≈“‹ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    "KTCNYBWF DJPLDBUYTNCZ YF CTVB GJHNFKF[", // À≈—“Õ»÷¿ ¬Œ«ƒ¬»√Õ≈“—ﬂ Õ¿ —≈Ã» œŒ–“¿À¿ı
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
    "NHTNM UJKJDJKJVRB HFPUFLFYF>>>", // “–≈“‹ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>YF CTVB GJHNFKF[", // ﬁﬁﬁÕ¿ —≈Ã» œŒ–“¿À¿ı
    "KTCNYBWF DJPLDBUYTNCZ YF CTVB GJHNFKF[", // À≈—“Õ»÷¿ ¬Œ«ƒ¬»√Õ≈“—ﬂ Õ¿ —≈Ã» œŒ–“¿À¿ı
    "JCNFKCZ TOT JLBY GTHTRK.XFNTKM>>>", // Œ—“¿À—ﬂ ≈Ÿ≈ Œƒ»Õ œ≈–≈ À˛◊¿“≈À‹ﬁﬁﬁ
    "RFVTYYFZ GHTUHFLF JNCNEGBN>>>", //  ¿Ã≈ÕÕ¿ﬂ œ–≈√–¿ƒ¿ Œ“—“”œ»“ﬁﬁﬁ
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
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ŒƒÕ¿ ÿ≈—“¿ﬂ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>D KTCE NTYTQ", // ﬁﬁﬁ¬ À≈—” “≈Õ≈…
    US,
    US,
    "LDTHM PF,KJRBHJDFYF BPYENHB", // ƒ¬≈–‹ «¿·ÀŒ »–Œ¬¿Õ¿ »«Õ”“–»
    "CKSITY PDER JNRHSDF.OTQCZ LDTHB", // —À€ÿ≈Õ «¬”  Œ“ –€¬¿˛Ÿ≈…—ﬂ ƒ¬≈–»
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
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ŒƒÕ¿ ÿ≈—“¿ﬂ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>D KTCE NTYTQ", // ﬁﬁﬁ¬ À≈—” “≈Õ≈…
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
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ŒƒÕ¿ ÿ≈—“¿ﬂ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>D KTCE NTYTQ", // ﬁﬁﬁ¬ À≈—” “≈Õ≈…
    US,
    US,
    US,
    US
};

const char* Hexen_Map_11_StringTable[] = {
    "JCNFKJCM 'NNBYJD: ", // Œ—“¿ÀŒ—‹ ˝““»ÕŒ¬∆ 
    "CKBIRJV LJKUJ< UJNJDMCZ R CVTHNB!", // —À»ÿ ŒÃ ƒŒÀ√Œ¡ √Œ“Œ¬‹—ﬂ   —Ã≈–“»!
    US,
    US,
    US,
    US,
    US,
    "LDTHM JNRHJTNCZ YF GJRBYENJQ PFCNFDT", // ƒ¬≈–‹ Œ“ –Œ≈“—ﬂ Õ¿ œŒ »Õ”“Œ… «¿—“¿¬≈
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
    "CTQXFC 'NF LDTHM PFRHSNF", // —≈…◊¿— ˝“¿ ƒ¬≈–‹ «¿ –€“¿
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
    "VJB GHBCKE;YBRB XE.N NDJ. RHJDM< XTKJDTR", // ÃŒ» œ–»—À”ÊÕ» » ◊”˛“ “¬Œ˛  –Œ¬‹¡ ◊≈ÀŒ¬≈ 
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
    "GHJ[JL ,ELTN JNRHSN DJPKT DBCTKBWS", // œ–ŒıŒƒ ·”ƒ≈“ Œ“ –€“ ¬Œ«À≈ ¬»—≈À»÷€
    US,
    "LDTHM PF,KJRBHJDFYF BPYENHB", // ƒ¬≈–‹ «¿·ÀŒ »–Œ¬¿Õ¿ »«Õ”“–»
    US,
    US,
    US,
    US
};

const char* Hexen_Map_22_StringTable[] = {
    US,
    US,
    US,
    "GKJOFLRF JGECNBKFCM D WTYNHFKMYJQ ,FIYT", // œÀŒŸ¿ƒ ¿ Œœ”—“»À¿—‹ ¬ ÷≈Õ“–¿À‹ÕŒ… ·¿ÿÕ≈
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
    "NS CKBIRJV PFBUHFKCZ< CVTHNYSQ>>>", // “€ —À»ÿ ŒÃ «¿»√–¿À—ﬂ¡ —Ã≈–“Õ€…ﬁﬁﬁ
    US,
    "GHBIKJ DHTVZ PFDTHIBNM NDJ. GFHNB.", // œ–»ÿÀŒ ¬–≈Ãﬂ «¿¬≈–ÿ»“‹ “¬Œ˛ œ¿–“»˛
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
    "BP CTDTHYJUJ PFKF LJYJCBNCZ PDER>>>", // »« —≈¬≈–ÕŒ√Œ «¿À¿ ƒŒÕŒ—»“—ﬂ «¬” ﬁﬁﬁ
    US,
    US,
    US,
    ">>>GJLYBVF.OTQCZ RFVTYYJQ RJKJYYS" // 14 : extra string : ﬁﬁﬁœŒƒÕ»Ã¿˛Ÿ≈…—ﬂ  ¿Ã≈ÕÕŒ…  ŒÀŒÕÕ€
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
    "GHTRKJYBCM GHTLJ VYJQ<", // œ–≈ ÀŒÕ»—‹ œ–≈ƒŒ ÃÕŒ…¡
    US,
    "B< VJ;TN ,SNM< Z ,ELE VBKJCTHLTY", // »¡ ÃŒÊ≈“ ·€“‹¡ ﬂ ·”ƒ” Ã»ÀŒ—≈–ƒ≈Õ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "F< VJ;TN ,SNM< B YTN" // 18 : extra string : ¿¡ ÃŒÊ≈“ ·€“‹¡ » Õ≈“
};

const char* Hexen_Map_28_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ŒƒÕ¿ ƒ≈¬ﬂ“¿ﬂ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>D CTVBYFHBB THTCBFH[F", // ﬁﬁﬁ¬ —≈Ã»Õ¿–»» ≈–≈—»¿–ı¿
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
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ŒƒÕ¿ ƒ≈¬ﬂ“¿ﬂ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>D CTVBYFHBB THTCBFH[F", // ﬁﬁﬁ¬ —≈Ã»Õ¿–»» ≈–≈—»¿–ı¿
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_34_StringTable[] = {
    US,
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ŒƒÕ¿ ƒ≈¬ﬂ“¿ﬂ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿ﬁﬁﬁ
    ">>>D CTVBYFHBB THTCBFH[F", // ﬁﬁﬁ¬ —≈Ã»Õ¿–»» ≈–≈—»¿–ı¿
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
    "DHFNF PFRHSKBCM>>>", // ¬–¿“¿ «¿ –€À»—‹ﬁﬁﬁ
    "CLTKFQ CDJQ DS,JH", // —ƒ≈À¿… —¬Œ… ¬€·Œ–
    US,
    "LDTHM PFGTHNF BPYENHB", // ƒ¬≈–‹ «¿œ≈–“¿ »«Õ”“–»
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "LJCNFNJXYJ KB NS CBKTY<", // ƒŒ—“¿“Œ◊ÕŒ À» “€ —»À≈Õ¡
    US,
    "XNJ,S CHFPBNMCZ CJ CDJBVB YFCNFDYBRFVB?", // ◊“Œ·€ —–¿«»“‹—ﬂ —Œ —¬Œ»Ã» Õ¿—“¿¬Õ» ¿Ã»,
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
    "NS GJCVTK CHF;FNMCZ D RJVYFNT J;BLFYBZ?", // “€ œŒ—Ã≈À —–¿Ê¿“‹—ﬂ ¬  ŒÃÕ¿“≈ ŒÊ»ƒ¿Õ»ﬂ,
    "NFR EVHB ;T!" // “¿  ”Ã–» Ê≈!
};

const char* Hexen_DK_Map_41_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "GJNJR DJLS JCNFYJDKTY", // œŒ“Œ  ¬Œƒ€ Œ—“¿ÕŒ¬À≈Õ
    "GJNJR DJLS GHTUHF;LFTN GENM", // œŒ“Œ  ¬Œƒ€ œ–≈√–¿Êƒ¿≈“ œ”“‹
    "LDTHM JNRHSKFCM D XFCJDYT", // ƒ¬≈–‹ Œ“ –€À¿—‹ ¬ ◊¿—Œ¬Õ≈
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
    "RFR CNHFYYJ>>>", //  ¿  —“–¿ÕÕŒﬁﬁﬁ
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_44_StringTable[] = {
    US,
    "JCNFKJCM NHB XFCNB UJKJDJKJVRB", // Œ—“¿ÀŒ—‹ “–» ◊¿—“» √ŒÀŒ¬ŒÀŒÃ »
    "JCNFKJCM LDT XFCNB UJKJDJKJVRB", // Œ—“¿ÀŒ—‹ ƒ¬≈ ◊¿—“» √ŒÀŒ¬ŒÀŒÃ »
    "JCNFKFCM JLYF XFCNM UJKJDJKJVRB", // Œ—“¿À¿—‹ ŒƒÕ¿ ◊¿—“‹ √ŒÀŒ¬ŒÀŒÃ »
    "UJKJDJKJVRF HFPUFLFYF", // √ŒÀŒ¬ŒÀŒÃ ¿ –¿«√¿ƒ¿Õ¿
    US,
    "UJKJDJKJVRF YT HFPUFLFYF", // √ŒÀŒ¬ŒÀŒÃ ¿ Õ≈ –¿«√¿ƒ¿Õ¿
    US,
    "GJK CJDCTV GHJUYBK!", // œŒÀ —Œ¬—≈Ã œ–Œ√Õ»À!
    US,
    "JLYF NHTNM UJKJDJKJVRB HFPUFLFYF", // ŒƒÕ¿ “–≈“‹ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿
    "LDT NHTNB UJKJDJKJVRB HFPUFLFYS", // ƒ¬≈ “–≈“» √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ€
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_45_StringTable[] = {
    US,
    "YTGJLFKTRE HFPLFTNCZ PDER>>>", // Õ≈œŒƒ¿À≈ ” –¿«ƒ¿≈“—ﬂ «¬” ﬁﬁﬁ
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    ">>>LDB;EOTQCZ LTHTDZYYJQ GKJOFLRB" // 10 : extra string : ﬁﬁﬁƒ¬»Ê”Ÿ≈…—ﬂ ƒ≈–≈¬ﬂÕÕŒ… œÀŒŸ¿ƒ »
};

const char* Hexen_DK_Map_46_StringTable[] = {
    "UJNJDJ>>>", // √Œ“Œ¬Œﬁﬁﬁ
    "UJKJDJKJVRF YT HFPUFLFYF", // √ŒÀŒ¬ŒÀŒÃ ¿ Õ≈ –¿«√¿ƒ¿Õ¿
    "Z NT,Z GHTLEGHT;LF.>>>", // ﬂ “≈·ﬂ œ–≈ƒ”œ–≈Êƒ¿˛ﬁﬁﬁ
    "YT CKBIRJV KB NS EGHZVSQ?", // Õ≈ —À»ÿ ŒÃ À» “€ ”œ–ﬂÃ€…,
    "B YT CKBIRJV-NJ HFPEVYSQ!", // » Õ≈ —À»ÿ ŒÃ-“Œ –¿«”ÃÕ€…!
    US,
    US,
    US,
    "JLYF XTNDTHNFZ 'NJQ UJKJDJKJVRB HFPUFLFYF", // ŒƒÕ¿ ◊≈“¬≈–“¿ﬂ ˝“Œ… √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿
    "GKJ[JQ DS,JH>>>", // œÀŒıŒ… ¬€·Œ–ﬁﬁﬁ
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_47_StringTable[] = {
    US,
    US,
    "CBVDJKS YT CJDGFLF.N", // —»Ã¬ŒÀ€ Õ≈ —Œ¬œ¿ƒ¿˛“
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
    "C 'NJQ CNJHJYS LDTHM YT JNRHSNM", // — ˝“Œ… —“Œ–ŒÕ€ ƒ¬≈–‹ Õ≈ Œ“ –€“‹
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
    "LDTHM PF,KJRBHJDFYF CYFHE;B", // ƒ¬≈–‹ «¿·ÀŒ »–Œ¬¿Õ¿ —Õ¿–”Ê»
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
    "CDZNJNFNCNDJ!", // —¬ﬂ“Œ“¿“—“¬Œ!
    "NS JCRDTHYBK VJUBKE 'HBRF!!", // “€ Œ— ¬≈–Õ»À ÃŒ√»À” ˝–» ¿!!
    "B EVHTIM PF 'NJ CNHFIYJQ CVTHNM.!!!", // » ”Ã–≈ÿ‹ «¿ ˝“Œ —“–¿ÿÕŒ… —Ã≈–“‹˛!!!
    "JLYF NHTNM UJKJDJKJVRB HFPUFLFYF", // ŒƒÕ¿ “–≈“‹ √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ¿
    "LDT NHTNB UJKJDJKJVRB HFPUFLFYS", // ƒ¬≈ “–≈“» √ŒÀŒ¬ŒÀŒÃ » –¿«√¿ƒ¿Õ€
    "CRKTG JNRHSN", // — À≈œ Œ“ –€“
    "JGFCFQCZ UHJ,YBWS GFERF>>>", // Œœ¿—¿…—ﬂ √–Œ·Õ»÷€ œ¿” ¿ﬁﬁﬁ
    US,
    "CYFHE;B CKSITY PDER GJLYBVF.OTUJCZ RFVYZ", // —Õ¿–”Ê» —À€ÿ≈Õ «¬”  œŒƒÕ»Ã¿˛Ÿ≈√Œ—ﬂ  ¿ÃÕﬂ
    "&XEDCNDETIM KB NS CT,Z DTPEXBV?&", // ?◊”¬—“¬”≈ÿ‹ À» “€ —≈·ﬂ ¬≈«”◊»Ã,?
    "YTGHFDBKMYJT GHTLGJKJ;TYBT!", // Õ≈œ–¿¬»À‹ÕŒ≈ œ–≈ƒœŒÀŒÊ≈Õ»≈!
    "GHFDBKMYJT GHTLGJKJ;TYBT", // œ–¿¬»À‹ÕŒ≈ œ–≈ƒœŒÀŒÊ≈Õ»≈
    "&VJ;TIM YFGBCFNM PF VTYZ DCT CRHBGNS?&", // ?ÃŒÊ≈ÿ‹ Õ¿œ»—¿“‹ «¿ Ã≈Õﬂ ¬—≈ — –»œ“€,?
    "&YT NHJUFQ VJ. DRECYZIRE&", // ?Õ≈ “–Œ√¿… ÃŒ˛ ¬ ”—Õﬂÿ ”?
    "&JCNHTYMRJ ?!?!?!&", // ?Œ—“–≈Õ‹ Œ ,!,!,!?
    "&GJLFQ-RF VYT CF[FH< LTNRF&", // ?œŒƒ¿…- ¿ ÃÕ≈ —¿ı¿–¡ ƒ≈“ ¿?
    "&FUF-F-F-F>>>&", // ?¿√¿-¿-¿-¿ﬁﬁﬁ?
    "&ABKMV YF XFC?&", // ?‘»À‹Ã Õ¿ ◊¿—,?
    "&E VTYZ LF;T YTN CDJTUJ YFLUHJ,BZ (R>A>)&", // ?” Ã≈Õﬂ ƒ¿Ê≈ Õ≈“ —¬Œ≈√Œ Õ¿ƒ√–Œ·»ﬂ ( ﬁ‘ﬁ)?
    "LF YT GHJKMTNCZ RHJDM>>>", // ƒ¿ Õ≈ œ–ŒÀ‹≈“—ﬂ  –Œ¬‹ﬁﬁﬁ
    "B LF YT GJLYBVTNCZ HERF DJ UYTDT>>>", // » ƒ¿ Õ≈ œŒƒÕ»Ã≈“—ﬂ –” ¿ ¬Œ √Õ≈¬≈ﬁﬁﬁ
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
    "RNJ JCVTKBKCZ GJNHTDJ;BNM YFI GJRJQ?!", //  “Œ Œ—Ã≈À»À—ﬂ œŒ“–≈¬ŒÊ»“‹ Õ¿ÿ œŒ Œ…,!
    "GENM JNRHSN", // œ”“‹ Œ“ –€“
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_53_StringTable[] = {
    US,
    US,
    "JCNFKJCM GTHTRK.XFNTKTQ: ", // Œ—“¿ÀŒ—‹ œ≈–≈ À˛◊¿“≈À≈…∆
    "",
    "JCNFKCZ DCTUJ JLBY GTHTRK.XFNTKM", // Œ—“¿À—ﬂ ¬—≈√Œ Œƒ»Õ œ≈–≈ À˛◊¿“≈À‹
    US,
    "DS[JL JNRHSN", // ¬€ıŒƒ Œ“ –€“
    US,
    US,
    US
};

const char* Hexen_DK_Map_54_StringTable[] = {
    US,
    "C 'NJQ CNJHJYS LDTHM YT JNRHSNM", // — ˝“Œ… —“Œ–ŒÕ€ ƒ¬≈–‹ Õ≈ Œ“ –€“‹
    US,
    US,
    "GENM D WBNFLTKM JNRHSN>>>", // œ”“‹ ¬ ÷»“¿ƒ≈À‹ Œ“ –€“ﬁﬁﬁ
    ">>>JCVTKBIMCZ KB NS DJQNB?", // ﬁﬁﬁŒ—Ã≈À»ÿ‹—ﬂ À» “€ ¬Œ…“»,
    US,
    US,
    US,
    "LDTHM JNRHJTNCZ>>>", // ƒ¬≈–‹ Œ“ –Œ≈“—ﬂﬁﬁﬁ
    ">>>D XFCJDYT", // ﬁﬁﬁ¬ ◊¿—Œ¬Õ≈
    "VJCN DJPLDBUYTNCZ>>>", // ÃŒ—“ ¬Œ«ƒ¬»√Õ≈“—ﬂﬁﬁﬁ
    ">>>YF ,JQYT", // ﬁﬁﬁÕ¿ ·Œ…Õ≈
    "KTCNYBWF DJPLDBUYTNCZ>>>", // À≈—“Õ»÷¿ ¬Œ«ƒ¬»√Õ≈“—ﬂﬁﬁﬁ
    ">>>E NTVYJUJ CNHF;F", // ﬁﬁﬁ” “≈ÃÕŒ√Œ —“–¿Ê¿
    "ITCNTHYZ ECNFYJDKTYF", // ÿ≈—“≈–Õﬂ ”—“¿ÕŒ¬À≈Õ¿
    "ITCNTHYTQ ECNFYJDKTYJ: ", // ÿ≈—“≈–Õ≈… ”—“¿ÕŒ¬À≈ÕŒ∆ 
    "GHTUHFLF GJLYBVTNCZ>>>", // œ–≈√–¿ƒ¿ œŒƒÕ»Ã≈“—ﬂﬁﬁﬁ
    ">>>D RKJFRT", // ﬁﬁﬁ¬  ÀŒ¿ ≈
    US,
    US,
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
    "LDTHM PF,KJRBHJDFYF BPYENHB", // ƒ¬≈–‹ «¿·ÀŒ »–Œ¬¿Õ¿ »«Õ”“–»
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
    "NS GJCVTK JUHF,BNM VJUBKE>>>", // “€ œŒ—Ã≈À Œ√–¿·»“‹ ÃŒ√»À”ﬁﬁﬁ
    ">>>DTH[JDYJUJ GFKFXF?", // ﬁﬁﬁ¬≈–ıŒ¬ÕŒ√Œ œ¿À¿◊¿,
    "UJNJDMCZ R CVTHNB", // √Œ“Œ¬‹—ﬂ   —Ã≈–“»
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_59_StringTable[] = {
    US,
    "JCNFKJCM YFQNB GTHTRK.XFNTKTQ: ", // Œ—“¿ÀŒ—‹ Õ¿…“» œ≈–≈ À˛◊¿“≈À≈…∆ 
    "",
    "JCNFKCZ GJCKTLYBQ GTHTRK.XFNTKM", // Œ—“¿À—ﬂ œŒ—À≈ƒÕ»… œ≈–≈ À˛◊¿“≈À‹
    US,
    "GENM R ,FIYT JNRHSN", // œ”“‹   ·¿ÿÕ≈ Œ“ –€“
    US,
    US
};

const char* Hexen_DK_Map_60_StringTable[] = {
    US,
    US,
    US,
    "GJHNFK JNRHSN>>>", //œŒ–“¿À Œ“ –€“ﬁﬁﬁ
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
