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

#define US "YTJ;BLFYYFZ CNHJRF! 'NJ <FU!" // менФхдюммюъ ярпнйю! Щрн аюц!

const char* Hexen_Map_01_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    "LDTHM PF,KJRBHJDFYF", // дбепэ гюАкнйхпнбюмю
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
    "GHBDTNCNDE.< CVTHNYSQ", // опхберярбсЧа ялепрмши
    US,
    "UJNJD KB NS EVTHTNM?", // цнрнб кх рш слеперэ,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "LDTHM JNRHSNF E CNHF;F KMLF", // дбепэ нрйпшрю с ярпюФю кэдю
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
    "CTQXFC 'NJN GENM PFRHSN", // яеивюя Щрнр осрэ гюйпшр
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
    "GJKJDBYF UJKJDJKJVRB HFPUFLFYF>>>", // онкнбхмю цнкнбнкнлйх пюгцюдюмюччч
    ">>>YF CTVB GJHNFKF[", // чччмю яелх онпрюкюУ
    "NHTNM UJKJDJKJVRB HFPUFLFYF>>>", // рперэ цнкнбнкнлйх пюгцюдюмюччч
    "KTCNYBWF DJPLDBUYTNCZ YF CTVB GJHNFKF[", // кеярмхжю бнгдбхцмеряъ мю яелх онпрюкюУ
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
    "NHTNM UJKJDJKJVRB HFPUFLFYF>>>", // рперэ цнкнбнкнлйх пюгцюдюмюччч
    ">>>YF CTVB GJHNFKF[", // чччмю яелх онпрюкюУ
    "KTCNYBWF DJPLDBUYTNCZ YF CTVB GJHNFKF[", // кеярмхжю бнгдбхцмеряъ мю яелх онпрюкюУ
    "JCNFKCZ TOT JLBY GTHTRK.XFNTKM>>>", // нярюкяъ еые ндхм оепейкЧвюрекэччч
    "RFVTYYFZ GHTUHFLF JNCNEGBN>>>", // йюлеммюъ опецпюдю нрярсохрччч
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
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ндмю ьеярюъ цнкнбнкнлйх пюгцюдюмюччч
    ">>>D KTCE NTYTQ", // чччб кеяс ремеи
    US,
    US,
    "LDTHM PF,KJRBHJDFYF BPYENHB", // дбепэ гюАкнйхпнбюмю хгмсрпх
    "CKSITY PDER JNRHSDF.OTQCZ LDTHB", // якшьем гбсй нрйпшбюЧыеияъ дбепх
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
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ндмю ьеярюъ цнкнбнкнлйх пюгцюдюмюччч
    ">>>D KTCE NTYTQ", // чччб кеяс ремеи
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
    "JLYF ITCNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ндмю ьеярюъ цнкнбнкнлйх пюгцюдюмюччч
    ">>>D KTCE NTYTQ", // чччб кеяс ремеи
    US,
    US,
    US,
    US
};

const char* Hexen_Map_11_StringTable[] = {
    "JCNFKJCM 'NNBYJD: ", // нярюкняэ Щррхмнбф 
    "CKBIRJV LJKUJ< UJNJDMCZ R CVTHNB!", // якхьйнл днкцна цнрнбэяъ й ялепрх!
    US,
    US,
    US,
    US,
    US,
    "LDTHM JNRHJTNCZ YF GJRBYENJQ PFCNFDT", // дбепэ нрйпнеряъ мю онйхмсрни гюярюбе
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
    "CTQXFC 'NF LDTHM PFRHSNF", // яеивюя Щрю дбепэ гюйпшрю
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
    "VJB GHBCKE;YBRB XE.N NDJ. RHJDM< XTKJDTR", // лнх опхяксФмхйх всЧр рбнЧ йпнбэа векнбей
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
    "GHJ[JL ,ELTN JNRHSN DJPKT DBCTKBWS", // опнУнд Асдер нрйпшр бнгке бхяекхжш
    US,
    "LDTHM PF,KJRBHJDFYF BPYENHB", // дбепэ гюАкнйхпнбюмю хгмсрпх
    US,
    US,
    US,
    US
};

const char* Hexen_Map_22_StringTable[] = {
    US,
    US,
    US,
    "GKJOFLRF JGECNBKFCM D WTYNHFKMYJQ ,FIYT", // окныюдйю носярхкюяэ б жемрпюкэмни Аюьме
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
    "NS CKBIRJV PFBUHFKCZ< CVTHNYSQ>>>", // рш якхьйнл гюхцпюкяъа ялепрмшиччч
    US,
    "GHBIKJ DHTVZ PFDTHIBNM NDJ. GFHNB.", // опхькн бпелъ гюбепьхрэ рбнЧ оюпрхЧ
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
    "BP CTDTHYJUJ PFKF LJYJCBNCZ PDER>>>", // хг яебепмнцн гюкю днмняхряъ гбсйччч
    US,
    US,
    US
    // TODO
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
    "GHTRKJYBCM GHTLJ VYJQ<", // опейкнмхяэ опедн лмниа
    US,
    "B< VJ;TN ,SNM< Z ,ELE VBKJCTHLTY", // ха лнФер Ашрэа ъ Асдс лхкняепдем
    US,
    US,
    US,
    US,
    US,
    US,
    US
    // TODO
};

const char* Hexen_Map_28_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ндмю дебърюъ цнкнбнкнлйх пюгцюдюмюччч
    ">>>D CTVBYFHBB THTCBFH[F", // чччб яелхмюпхх епеяхюпУю
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
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ндмю дебърюъ цнкнбнкнлйх пюгцюдюмюччч
    ">>>D CTVBYFHBB THTCBFH[F", // чччб яелхмюпхх епеяхюпУю
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_Map_34_StringTable[] = {
    US,
    "JLYF LTDZNFZ UJKJDJKJVRB HFPUFLFYF>>>", // ндмю дебърюъ цнкнбнкнлйх пюгцюдюмюччч
    ">>>D CTVBYFHBB THTCBFH[F", // чччб яелхмюпхх епеяхюпУю
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
    "DHFNF PFRHSKBCM>>>", // бпюрю гюйпшкхяэччч
    "CLTKFQ CDJQ DS,JH", // ядекюи ябни бшАнп
    US,
    "LDTHM PFGTHNF BPYENHB", // дбепэ гюоепрю хгмсрпх
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    "LJCNFNJXYJ KB NS CBKTY<", // днярюрнвмн кх рш яхкема
    US,
    "XNJ,S CHFPBNMCZ CJ CDJBVB YFCNFDYBRFVB?", // врнАш япюгхрэяъ ян ябнхлх мюярюбмхйюлх,
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
    "NS GJCVTK CHF;FNMCZ D RJVYFNT J;BLFYBZ?", // рш онялек япюФюрэяъ б йнлмюре нФхдюмхъ,
    "NFR EVHB ;T!" // рюй слпх Фе!
};

const char* Hexen_DK_Map_41_StringTable[] = {
    US,
    US,
    US,
    US,
    US,
    US,
    "GJNJR DJLS JCNFYJDKTY", // онрнй бндш нярюмнбкем
    "GJNJR DJLS GHTUHF;LFTN GENM", // онрнй бндш опецпюФдюер осрэ
    "LDTHM JNRHSKFCM D XFCJDYT", // дбепэ нрйпшкюяэ б вюянбме
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
    "RFR CNHFYYJ>>>", // йюй ярпюммнччч
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_44_StringTable[] = {
    US,
    "JCNFKJCM NHB XFCNB UJKJDJKJVRB", // нярюкняэ рпх вюярх цнкнбнкнлйх
    "JCNFKJCM LDT XFCNB UJKJDJKJVRB", // нярюкняэ дбе вюярх цнкнбнкнлйх
    "JCNFKFCM JLYF XFCNM UJKJDJKJVRB", // нярюкюяэ ндмю вюярэ цнкнбнкнлйх
    "UJKJDJKJVRF HFPUFLFYF", // цнкнбнкнлйю пюгцюдюмю
    US,
    "UJKJDJKJVRF YT HFPUFLFYF", // цнкнбнкнлйю ме пюгцюдюмю
    US,
    "GJK CJDCTV GHJUYBK!", // онк янбяел опнцмхк!
    US,
    "JLYF NHTNM UJKJDJKJVRB HFPUFLFYF", // ндмю рперэ цнкнбнкнлйх пюгцюдюмю
    "LDT NHTNB UJKJDJKJVRB HFPUFLFYS", // дбе рперх цнкнбнкнлйх пюгцюдюмш
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_45_StringTable[] = {
    US,
    "YTGJLFKTRE HFPLFTNCZ PDER>>>", // меондюкейс пюгдюеряъ гбсйччч
    US,
    US,
    US,
    US,
    US,
    US,
    US,
    US
    // TODO
};

const char* Hexen_DK_Map_46_StringTable[] = {
    "UJNJDJ>>>", // цнрнбнччч
    "UJKJDJKJVRF YT HFPUFLFYF", // цнкнбнкнлйю ме пюгцюдюмю
    "Z NT,Z GHTLEGHT;LF.>>>", // ъ реАъ опедсопеФдюЧччч
    "YT CKBIRJV KB NS EGHZVSQ?", // ме якхьйнл кх рш сопълши,
    "B YT CKBIRJV-NJ HFPEVYSQ!", // х ме якхьйнл-рн пюгслмши!
    US,
    US,
    US,
    "JLYF XTNDTHNFZ 'NJQ UJKJDJKJVRB HFPUFLFYF", // ндмю вербепрюъ Щрни цнкнбнкнлйх пюгцюдюмю
    "GKJ[JQ DS,JH>>>", // окнУни бшАнпччч
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_47_StringTable[] = {
    US,
    US,
    "CBVDJKS YT CJDGFLF.N", // яхлбнкш ме янбоюдюЧр
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
    "C 'NJQ CNJHJYS LDTHM YT JNRHSNM", // я Щрни ярнпнмш дбепэ ме нрйпшрэ
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
    "LDTHM PF,KJRBHJDFYF CYFHE;B", // дбепэ гюАкнйхпнбюмю ямюпсФх
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
    "CDZNJNFNCNDJ!", // ябърнрюрярбн!
    "NS JCRDTHYBK VJUBKE 'HBRF!!", // рш няйбепмхк лнцхкс Щпхйю!!
    "B EVHTIM PF 'NJ CNHFIYJQ CVTHNM.!!!", // х слпеьэ гю Щрн ярпюьмни ялепрэЧ!!!
    "JLYF NHTNM UJKJDJKJVRB HFPUFLFYF", // ндмю рперэ цнкнбнкнлйх пюгцюдюмю
    "LDT NHTNB UJKJDJKJVRB HFPUFLFYS", // дбе рперх цнкнбнкнлйх пюгцюдюмш
    "CRKTG JNRHSN", // яйкео нрйпшр
    "JGFCFQCZ UHJ,YBWS GFERF>>>", // ноюяюияъ цпнАмхжш оюсйюччч
    US,
    "CYFHE;B CKSITY PDER GJLYBVF.OTUJCZ RFVYZ", // ямюпсФх якшьем гбсй ондмхлюЧыецняъ йюлмъ
    "&XEDCNDETIM KB NS CT,Z DTPEXBV?&", // ?всбярбсеьэ кх рш яеАъ бегсвхл,?
    "YTGHFDBKMYJT GHTLGJKJ;TYBT!", // меопюбхкэмне опедонкнФемхе!
    "GHFDBKMYJT GHTLGJKJ;TYBT", // опюбхкэмне опедонкнФемхе
    "&VJ;TIM YFGBCFNM PF VTYZ DCT CRHBGNS?&", // ?лнФеьэ мюохяюрэ гю лемъ бяе яйпхорш,?
    "&YT NHJUFQ VJ. DRECYZIRE&", // ?ме рпнцюи лнЧ бйсямъьйс?
    "&JCNHTYMRJ ?!?!?!&", // ?нярпемэйн ,!,!,!?
    "&GJLFQ-RF VYT CF[FH< LTNRF&", // ?ондюи-йю лме яюУюпа дерйю?
    "&FUF-F-F-F>>>&", // ?юцю-ю-ю-юччч?
    "&ABKMV YF XFC?&", // ?тхкэл мю вюя,?
    "&E VTYZ LF;T YTN CDJTUJ YFLUHJ,BZ (R>A>)&", // ?с лемъ дюФе мер ябнецн мюдцпнАхъ (йчтч)?
    "LF YT GHJKMTNCZ RHJDM>>>", // дю ме опнкэеряъ йпнбэччч
    "B LF YT GJLYBVTNCZ HERF DJ UYTDT>>>", // х дю ме ондмхлеряъ псйю бн цмебеччч
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
    "RNJ JCVTKBKCZ GJNHTDJ;BNM YFI GJRJQ?!", // йрн нялекхкяъ онрпебнФхрэ мюь онйни,!
    "GENM JNRHSN", // осрэ нрйпшр
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_53_StringTable[] = {
    US,
    US,
    "JCNFKJCM GTHTRK.XFNTKTQ: ", // нярюкняэ оепейкЧвюрекеиф
    "",
    "JCNFKCZ DCTUJ JLBY GTHTRK.XFNTKM", // нярюкяъ бяецн ндхм оепейкЧвюрекэ
    "", //TODO
    "DS[JL JNRHSN", // бшУнд нрйпшр
    US,
    US,
    US
};

const char* Hexen_DK_Map_54_StringTable[] = {
    US,
    "C 'NJQ CNJHJYS LDTHM YT JNRHSNM", // я Щрни ярнпнмш дбепэ ме нрйпшрэ
    US,
    US,
    "GENM D WBNFLTKM JNRHSN>>>", // осрэ б жхрюдекэ нрйпшрччч
    ">>>JCVTKBIMCZ KB NS DJQNB?", // чччнялекхьэяъ кх рш бнирх,
    US,
    US,
    US,
    "LDTHM JNRHJTNCZ>>>", // дбепэ нрйпнеряъччч
    ">>>D XFCJDYT", // чччб вюянбме
    "VJCN DJPLDBUYTNCZ>>>", // лняр бнгдбхцмеряъччч
    ">>>YF ,JQYT", // чччмю Аниме
    "KTCNYBWF DJPLDBUYTNCZ>>>", // кеярмхжю бнгдбхцмеряъччч
    ">>>E NTVYJUJ CNHF;F", // чччс релмнцн ярпюФю
    "ITCNTHYZ ECNFYJDKTYF", // ьеярепмъ сярюмнбкемю
    "ITCNTHYTQ ECNFYJDKTYJ: ", // ьеярепмеи сярюмнбкемнф 
    "GHTUHFLF GJLYBVTNCZ>>>", // опецпюдю ондмхлеряъччч
    ">>>D RKJFRT", // чччб йкнюйе
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
    "LDTHM PF,KJRBHJDFYF BPYENHB", // дбепэ гюАкнйхпнбюмю хгмсрпх
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
    "NS GJCVTK JUHF,BNM VJUBKE>>>", // рш онялек нцпюАхрэ лнцхксччч
    ">>>DTH[JDYJUJ GFKFXF?", // чччбепУнбмнцн оюкювю,
    "UJNJDMCZ R CVTHNB", // цнрнбэяъ й ялепрх
    US,
    US,
    US,
    US,
    US
};

const char* Hexen_DK_Map_59_StringTable[] = {
    US,
    "JCNFKJCM YFQNB GTHTRK.XFNTKTQ: ", // нярюкняэ мюирх оепейкЧвюрекеиф 
    "",
    "JCNFKCZ GJCKTLYBQ GTHTRK.XFNTKM", // нярюкяъ онякедмхи оепейкЧвюрекэ
    "", //TODO
    "GENM R ,FIYT JNRHSN", // осрэ й Аюьме нрйпшр
    US,
    US
};

const char* Hexen_DK_Map_60_StringTable[] = {
    US,
    US,
    US,
    "GJHNFK JNRHSN>>>", //онпрюк нрйпшрччч
    US,
    US,
    US,
    US,
    US,
    US,
    US
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
