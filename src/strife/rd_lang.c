//
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


#include "doomstat.h"
#include "jn.h"
#include "rd_lang.h"


// -----------------------------------------------------------------------------
// [JN] Chars* to be defined and used as a predefined text strings.
// -----------------------------------------------------------------------------

// am_map.c

char *amstr_followon;
char *amstr_followoff;
char *amstr_gridon;
char *amstr_gridoff;
char *amstr_markedspot;
char *amstr_markscleared;

//
// d_main.c
//

char *d_devstr;
char *d_cdrom;

//
// g_game.c
//

char *ststr_alwrun_on;
char *ststr_alwrun_off;

//
// m_menu.c
//

char *presskey;
char *pressyn;
char *quitmsg;
char *loadnet;
char *qloadnet;
char *qsavespot;
char *savedead;
char *qsprompt;
char *qlprompt;
char *netend;
char *endgame;
char *dosy;
char *leave;

// Gamma-correction
char gammamsg[18][41] =
{
    GAMMA_IMPROVED_OFF,
    GAMMA_IMPROVED_05,
    GAMMA_IMPROVED_1,
    GAMMA_IMPROVED_15,
    GAMMA_IMPROVED_2,
    GAMMA_IMPROVED_25,
    GAMMA_IMPROVED_3,
    GAMMA_IMPROVED_35,
    GAMMA_IMPROVED_4,
    GAMMA_ORIGINAL_OFF,
    GAMMA_ORIGINAL_05,
    GAMMA_ORIGINAL_1,
    GAMMA_ORIGINAL_15,
    GAMMA_ORIGINAL_2,
    GAMMA_ORIGINAL_25,
    GAMMA_ORIGINAL_3,
    GAMMA_ORIGINAL_35,
    GAMMA_ORIGINAL_4
};

// Gamma-correction (Russian)
char gammamsg_rus[18][41] =
{
    GAMMA_IMPROVED_OFF_RUS,
    GAMMA_IMPROVED_05_RUS,
    GAMMA_IMPROVED_1_RUS,
    GAMMA_IMPROVED_15_RUS,
    GAMMA_IMPROVED_2_RUS,
    GAMMA_IMPROVED_25_RUS,
    GAMMA_IMPROVED_3_RUS,
    GAMMA_IMPROVED_35_RUS,
    GAMMA_IMPROVED_4_RUS,
    GAMMA_ORIGINAL_OFF_RUS,
    GAMMA_ORIGINAL_05_RUS,
    GAMMA_ORIGINAL_1_RUS,
    GAMMA_ORIGINAL_15_RUS,
    GAMMA_ORIGINAL_2_RUS,
    GAMMA_ORIGINAL_25_RUS,
    GAMMA_ORIGINAL_3_RUS,
    GAMMA_ORIGINAL_35_RUS,
    GAMMA_ORIGINAL_4_RUS
};

//
// st_stuff.c
//

char *ststr_mus;
char *ststr_nomus;
char *ststr_dqdon;
char *ststr_dqdoff;
char *ststr_faadded;
char *ststr_ncon;
char *ststr_ncoff;
char *ststr_behold;
char *ststr_beholdx;
char *ststr_clev;


// -----------------------------------------------------------------------------
// [JN] Function for (pre-)defining strings for text translations.
// -----------------------------------------------------------------------------

void RD_DefineLanguageStrings (void)
{
    if (english_language)
    {
        // am_map.c
        amstr_followon     = AMSTR_FOLLOWON;
        amstr_followoff    = AMSTR_FOLLOWOFF;
        amstr_gridon       = AMSTR_GRIDON;
        amstr_gridoff      = AMSTR_GRIDOFF;
        amstr_markedspot   = AMSTR_MARKEDSPOT;
        amstr_markscleared = AMSTR_MARKSCLEARED;

        // d_main.c
        d_devstr = D_DEVSTR;
        d_cdrom  = D_CDROM;

        // g_game.c
        ststr_alwrun_on  = STSTR_ALWRUNON;
        ststr_alwrun_off = STSTR_ALWRUNOFF;

        // m_menu.c
        presskey  = PRESSKEY;
        pressyn   = PRESSYN;
        quitmsg   = QUITMSG;
        loadnet   = LOADNET;
        qloadnet  = QLOADNET;
        qsavespot = QSAVESPOT;
        savedead  = SAVEDEAD;
        qsprompt  = QSPROMPT;
        qlprompt  = QLPROMPT;
        netend    = NETEND;
        endgame   = ENDGAME;
        dosy      = DOSY;
        leave     = LEAVE;

        // st_stuff.c
        ststr_mus     = STSTR_MUS;
        ststr_nomus   = STSTR_NOMUS;
        ststr_dqdon   = STSTR_DQDON;
        ststr_dqdoff  = STSTR_DQDOFF;
        ststr_faadded = STSTR_FAADDED;
        ststr_ncon    = STSTR_NCON;
        ststr_ncoff   = STSTR_NCOFF;
        ststr_behold  = STSTR_BEHOLD;
        ststr_beholdx = STSTR_BEHOLDX;
        ststr_clev    = STSTR_CLEV;
        
        // Character names - info.c
        mobjinfo[MT_SHOPKEEPER_W].name = "Weapon_Smith";
        mobjinfo[MT_SHOPKEEPER_B].name = "Bar_Keep";
        mobjinfo[MT_SHOPKEEPER_A].name = "Armorer";
        mobjinfo[MT_SHOPKEEPER_M].name = "Medic";
        mobjinfo[MT_BEGGAR1].name = "Beggar";
        mobjinfo[MT_BEGGAR2].name = "Beggar";
        mobjinfo[MT_BEGGAR3].name = "Beggar";
        mobjinfo[MT_BEGGAR4].name = "Beggar";
        mobjinfo[MT_BEGGAR5].name = "Beggar";
        mobjinfo[MT_REBEL1].name = "Rebel";
        mobjinfo[MT_REBEL2].name = "Rebel";
        mobjinfo[MT_REBEL3].name = "Rebel";
        mobjinfo[MT_REBEL4].name = "Rebel";
        mobjinfo[MT_REBEL5].name = "Rebel";
        mobjinfo[MT_REBEL6].name = "Rebel";
        mobjinfo[MT_RLEADER].name = "MACIL";
        mobjinfo[MT_RLEADER2].name = "MACIL";
        mobjinfo[MT_GUARD1].name = "ACOLYTE";
        mobjinfo[MT_GUARD2].name = "ACOLYTE";
        mobjinfo[MT_GUARD3].name = "ACOLYTE";
        mobjinfo[MT_GUARD4].name = "ACOLYTE";
        mobjinfo[MT_GUARD5].name = "ACOLYTE";
        mobjinfo[MT_GUARD6].name = "ACOLYTE";
        mobjinfo[MT_GUARD7].name = "ACOLYTE";
        mobjinfo[MT_GUARD8].name = "ACOLYTE";
        mobjinfo[MT_SHADOWGUARD].name = "ACOLYTE";
        mobjinfo[MT_PGUARD].name = "TEMPLAR";
        mobjinfo[MT_ORACLE].name = "ORACLE";
        mobjinfo[MT_PRIEST].name = "PRIEST";
        mobjinfo[MT_INV_MED1].name = "Med_patch";
        mobjinfo[MT_INV_MED2].name = "Medical_kit";
        mobjinfo[MT_INV_MED3].name = "Surgery_Kit";
        mobjinfo[MT_DEGNINORE].name = "Degnin_Ore";
        mobjinfo[MT_INV_ARMOR2].name = "Metal_Armor";
        mobjinfo[MT_INV_ARMOR1].name = "Leather_Armor";
        mobjinfo[MT_KEY_BASE].name = "Base_Key";
        mobjinfo[MT_GOVSKEY].name = "Govs_Key";
        mobjinfo[MT_KEY_TRAVEL].name = "Passcard";
        mobjinfo[MT_KEY_ID_BLUE].name = "ID_Badge";
        mobjinfo[MT_PRISONKEY].name = "Prison_Key";
        mobjinfo[MT_KEY_HAND].name = "Severed_Hand";
        mobjinfo[MT_POWER1KEY].name = "Power1_Key";
        mobjinfo[MT_POWER2KEY].name = "Power2_Key";
        mobjinfo[MT_POWER3KEY].name = "Power3_Key";
        mobjinfo[MT_KEY_GOLD].name = "Gold_Key";
        mobjinfo[MT_KEY_ID_GOLD].name = "ID_Card";
        mobjinfo[MT_KEY_SILVER].name = "Silver_Key";
        mobjinfo[MT_KEY_ORACLE].name = "Oracle_Key";
        mobjinfo[MT_MILITARYID].name = "Military_ID";
        mobjinfo[MT_KEY_ORDER].name = "Order_Key";
        mobjinfo[MT_KEY_WAREHOUSE].name = "Warehouse_Key";
        mobjinfo[MT_KEY_BRASS].name = "Brass_Key";
        mobjinfo[MT_KEY_RED_CRYSTAL].name = "Red_Crystal_Key";
        mobjinfo[MT_KEY_BLUE_CRYSTAL].name = "Blue_Crystal_Key";
        mobjinfo[MT_KEY_CHAPEL].name = "Chapel_Key";
        mobjinfo[MT_CATACOMBKEY].name = "Catacomb_Key";
        mobjinfo[MT_SECURITYKEY].name = "Security_Key";
        mobjinfo[MT_KEY_CORE].name = "Core_Key";
        mobjinfo[MT_KEY_MAULER].name = "Mauler_Key";
        mobjinfo[MT_KEY_FACTORY].name = "Factory_Key";
        mobjinfo[MT_KEY_MINE].name = "MINE_KEY";
        mobjinfo[MT_NEWKEY5].name = "New_Key5";
        mobjinfo[MT_INV_SHADOWARMOR].name = "Shadow_armor";
        mobjinfo[MT_INV_SUIT].name = "Environmental_Suit";
        mobjinfo[MT_QUEST_UNIFORM].name = "Guard_Uniform";
        mobjinfo[MT_QUEST_GUARD_UNIFORM].name = "Officer's_Uniform";
        mobjinfo[MT_INV_SUPERMAP].name = "map";
        mobjinfo[MT_INV_RADAR].name = "scanner";
        mobjinfo[MT_BEACON].name = "Teleporter_Beacon";
        mobjinfo[MT_INV_TARGETER].name = "Targeter";
        mobjinfo[MT_MONY_1].name = "coin";
        mobjinfo[MT_MONY_10].name = "10_gold";
        mobjinfo[MT_MONY_25].name = "25_gold";
        mobjinfo[MT_MONY_50].name = "50_gold";
        mobjinfo[MT_MONY_300].name = "300_gold";
        mobjinfo[MT_TOKEN_RING].name = "ring";
        mobjinfo[MT_INV_CHALICE].name = "Offering_Chalice";
        mobjinfo[MT_TOKEN_EAR].name = "ear";
        mobjinfo[MT_INV_COMMUNICATOR].name = "Communicator";
        mobjinfo[MT_AGREN].name = "HE-Grenade_Rounds";
        mobjinfo[MT_APGREN].name = "Phosphorus-Grenade_Rounds";
        mobjinfo[MT_ACLIP].name = "clip_of_bullets";
        mobjinfo[MT_AAMMOBOX].name = "ammo";
        mobjinfo[MT_AMINI].name = "mini_missiles";
        mobjinfo[MT_AMINIBOX].name = "crate_of_missiles";
        mobjinfo[MT_ACELL].name = "energy_pod";
        mobjinfo[MT_APCELL].name = "energy_pack";
        mobjinfo[MT_APAROW].name = "poison_bolts";
        mobjinfo[MT_AAROW].name = "electric_bolts";
        mobjinfo[MT_INV_SATCHEL].name = "ammo_satchel";
        mobjinfo[MT_PULSE].name = "assault_gun";
        mobjinfo[MT_RIFLESTAND].name = "assault_gun";
        mobjinfo[MT_FLAMETHROWER].name = "flame_thrower";
        mobjinfo[MT_TOKEN_FLAME_THROWER_PARTS].name = "flame_thrower_parts";
        mobjinfo[MT_MISSILELAUNCHER].name = "mini_missile_launcher";
        mobjinfo[MT_BLASTER].name = "mauler";
        mobjinfo[MT_CROSSBOW].name = "crossbow";
        mobjinfo[MT_GRENADELAUNCHER].name = "Grenade_launcher";
        mobjinfo[MT_SIGIL_A].name = "SIGIL";
        mobjinfo[MT_SIGIL_B].name = "SIGIL";
        mobjinfo[MT_SIGIL_C].name = "SIGIL";
        mobjinfo[MT_SIGIL_D].name = "SIGIL";
        mobjinfo[MT_SIGIL_E].name = "SIGIL";
        mobjinfo[MT_COUPLING_BROKEN].name = "BROKEN_POWER_COUPLING";
        mobjinfo[MT_TOKEN_AMMO].name = "Ammo";
        mobjinfo[MT_TOKEN_HEALTH].name = "Health";
        mobjinfo[MT_TOKEN].name = "info";
        mobjinfo[MT_TOKEN_ALARM].name = "alarm";
        mobjinfo[MT_TOKEN_PRISON_PASS].name = "Prison_pass";
        mobjinfo[MT_TOKEN_REPORT].name = "report";
        mobjinfo[MT_TOKEN_TOUGHNESS].name = "Toughness";
        mobjinfo[MT_TOKEN_ACCURACY].name = "Accuracy";
        mobjinfo[MT_TOKEN_ORACLE_PASS].name = "Oracle_Pass";
        mobjinfo[MT_TOKEN_CRYSTAL].name = "You've_Blown_Up_the_Crystal";
        mobjinfo[MT_GATEQUEST].name = "You've_Blown_Up_the_Gates";
        mobjinfo[MT_TOKEN_BISHOP].name = "You_Killed_the_Bishop!";
        mobjinfo[MT_TOKEN_ORACLE].name = "You've_Killed_The_Oracle!";
        mobjinfo[MT_TOKEN_MACIL].name = "You_Killed_Macil!";
        mobjinfo[MT_TOKEN_LOREMASTER].name = "You've_Killed_The_Loremaster!";
        mobjinfo[MT_SECRQUEST].name = "You've_Blown_Up_the_Computer!";
    }
    else
    {
        // am_map.c
        amstr_followon     = AMSTR_FOLLOWON_RUS;
        amstr_followoff    = AMSTR_FOLLOWOFF_RUS;
        amstr_gridon       = AMSTR_GRIDON_RUS;
        amstr_gridoff      = AMSTR_GRIDOFF_RUS;
        amstr_markedspot   = AMSTR_MARKEDSPOT_RUS;
        amstr_markscleared = AMSTR_MARKSCLEARED_RUS;

        // d_main.c
        d_devstr = D_DEVSTR_RUS;
        d_cdrom  = D_CDROM_RUS;

        // g_game.c
        ststr_alwrun_on  = STSTR_ALWRUNON_RUS;
        ststr_alwrun_off = STSTR_ALWRUNOFF_RUS;

        // m_menu.c
        presskey = PRESSKEY_RUS;
        pressyn  = PRESSYN_RUS;
        quitmsg  = QUITMSG_RUS;
        loadnet   = LOADNET_RUS;
        qloadnet  = QLOADNET_RUS;
        qsavespot = QSAVESPOT_RUS;
        savedead  = SAVEDEAD_RUS;
        qsprompt  = QSPROMPT_RUS;
        qlprompt  = QLPROMPT_RUS;
        netend    = NETEND_RUS;
        endgame   = ENDGAME_RUS;
        dosy      = DOSY_RUS;
        leave     = LEAVE_RUS;

        // st_stuff.c
        ststr_mus     = STSTR_MUS_RUS;
        ststr_nomus   = STSTR_NOMUS_RUS;
        ststr_dqdon   = STSTR_DQDON_RUS;
        ststr_dqdoff  = STSTR_DQDOFF_RUS;
        ststr_faadded = STSTR_FAADDED_RUS;
        ststr_ncon    = STSTR_NCON_RUS;
        ststr_ncoff   = STSTR_NCOFF_RUS;
        ststr_behold  = STSTR_BEHOLD_RUS;
        ststr_beholdx = STSTR_BEHOLDX_RUS;
        ststr_clev    = STSTR_CLEV_RUS;

        // Character names - info.c
        mobjinfo[MT_SHOPKEEPER_W].name = "jhe;tqybr";           // [rus] Оружейник
        mobjinfo[MT_SHOPKEEPER_B].name = "nhfrnbhobr";          // [JN]  Трактирщик
        mobjinfo[MT_SHOPKEEPER_A].name = ",hjyybr";             // [rus] Бронник
        mobjinfo[MT_SHOPKEEPER_M].name = "vtlbr";               // [rus] Медик
        mobjinfo[MT_BEGGAR1].name = "ybobq";                    // [rus] Нищий
        mobjinfo[MT_BEGGAR2].name = "ybobq";                    // [rus] Нищий
        mobjinfo[MT_BEGGAR3].name = "ybobq";                    // [rus] Нищий
        mobjinfo[MT_BEGGAR4].name = "ybobq";                    // [rus] Нищий
        mobjinfo[MT_BEGGAR5].name = "ybobq";                    // [rus] Нищий
        mobjinfo[MT_REBEL1].name = "gjdcnfytw";                 // [rus] Повстанец
        mobjinfo[MT_REBEL2].name = "gjdcnfytw";                 // [rus] Повстанец
        mobjinfo[MT_REBEL3].name = "gjdcnfytw";                 // [rus] Повстанец
        mobjinfo[MT_REBEL4].name = "gjdcnfytw";                 // [rus] Повстанец
        mobjinfo[MT_REBEL5].name = "gjdcnfytw";                 // [rus] Повстанец
        mobjinfo[MT_REBEL6].name = "gjdcnfytw";                 // [rus] Повстанец
        mobjinfo[MT_RLEADER].name = "v'qcbk";                   // [rus] Мэйсил
        mobjinfo[MT_RLEADER2].name = "v'qcbk";                  // [rus] Мэйсил
        mobjinfo[MT_GUARD1].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_GUARD2].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_GUARD3].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_GUARD4].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_GUARD5].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_GUARD6].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_GUARD7].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_GUARD8].name = "cke;bntkm";                 // [rus] Служитель
        mobjinfo[MT_SHADOWGUARD].name = "cke;bntkm";            // [rus] Служитель
        mobjinfo[MT_PGUARD].name = "[hfvjdybr";                 // [rus] Храмовник
        mobjinfo[MT_ORACLE].name = "jhfrek";                    // [rus] Оракул
        mobjinfo[MT_PRIEST].name = "cdzotyybr";                 // [JN]  Священник
        mobjinfo[MT_INV_MED1].name = "vtlbwbycrbq_,byn";        // [rus] Медицинский бинт
        mobjinfo[MT_INV_MED2].name = "fgntxrf";                 // [rus] Аптечка
        mobjinfo[MT_INV_MED3].name = "vtlrjvgktrn";             // [rus] Медкомплект
        mobjinfo[MT_DEGNINORE].name = "ltuybycrfz_helf";        // [rus] Дегнинская руда
        mobjinfo[MT_INV_ARMOR2].name = "vtnfkkbxtcrfz_,hjyz";   // [rus] Металлическая броня
        mobjinfo[MT_INV_ARMOR1].name = "rj;fyfz_,hjyz";         // [rus] Кожаная броня
        mobjinfo[MT_KEY_BASE].name = "rk.x_jn_,fps";            // [rus] Ключ от базы
        mobjinfo[MT_GOVSKEY].name = "rk.x_ue,thyfnjhf";         // [rus] Ключ губернатора
        mobjinfo[MT_KEY_TRAVEL].name = "ghjgecr";               // [rus] Пропуск
        mobjinfo[MT_KEY_ID_BLUE].name = "kbxyfz_rfhnf";         // [rus] Личная карта
        mobjinfo[MT_PRISONKEY].name = "rk.x_jn_n.hmvs";         // [rus] Ключ от тюрьмы
        mobjinfo[MT_KEY_HAND].name = "jnjhdfyyfz_herf";         // [rus] Оторванная рука
        mobjinfo[MT_POWER1KEY].name = "rk.x_'ktrnhjcnfywbb_1";  // [rus] Ключ электростанции 1
        mobjinfo[MT_POWER2KEY].name = "rk.x_'ktrnhjcnfywbb_2";  // [rus] Ключ электростанции 2
        mobjinfo[MT_POWER3KEY].name = "rk.x_'ktrnhjcnfywbb_3";  // [rus] Ключ электростанции 3
        mobjinfo[MT_KEY_GOLD].name = "pjkjnjq_rk.x";            // [rus] Золотой ключ
        mobjinfo[MT_KEY_ID_GOLD].name = "eljcnjdthtybt";        // [rus] Удостоверение
        mobjinfo[MT_KEY_ORACLE].name = "ctht,hzysq_rk.x";       // [rus] Серебряный ключ
        mobjinfo[MT_KEY_SILVER].name = "rk.x_jhfrekf";          // [rus] Ключ Оракула
        mobjinfo[MT_MILITARYID].name = "djtyyjt_eljcnjdthtybt"; // [rus] Военное удостоверение
        mobjinfo[MT_KEY_ORDER].name = "rk.x_jhltyf";            // [rus] Ключ Ордена
        mobjinfo[MT_KEY_WAREHOUSE].name = "rk.x_jn_crkflf";     // [rus] Ключ от склада
        mobjinfo[MT_KEY_BRASS].name = "kfneyysq_rk.x";          // [rus] Латунный ключ
        mobjinfo[MT_KEY_RED_CRYSTAL].name = "rhfcysq_rk.x-rhbcnfkk";    // [rus] Красный ключ-кристалл
        mobjinfo[MT_KEY_BLUE_CRYSTAL].name = "cbybq_rk.x-rhbcnfkk";     // [rus] Синий ключ-кристалл
        mobjinfo[MT_KEY_CHAPEL].name = "rk.x_jn_xfcjdyb";       // [rus] Ключ от часовни
        mobjinfo[MT_CATACOMBKEY].name = "rk.x_jn_rfnfrjv,";     // [rus] Ключ от катакомб
        mobjinfo[MT_SECURITYKEY].name = "rk.x_j[hfys";          // [rus] Ключ охраны
        mobjinfo[MT_KEY_CORE].name = "rk.x_jn_htfrnjhf";        // [rus] Ключ от реактора
        mobjinfo[MT_KEY_MAULER].name = "rk.x_bcnzpfntkz";       // [rus] Ключ истязателя
        mobjinfo[MT_KEY_FACTORY].name = "rk.x_jn_af,hbrb";      // [rus] Ключ от фабрики
        mobjinfo[MT_KEY_MINE].name = "rk.x_jn_if[n";            // [rus] Ключ от шахт
        mobjinfo[MT_NEWKEY5].name = "yjdsq_rk.x5";              // [rus] Новый Ключ5
        mobjinfo[MT_INV_SHADOWARMOR].name = "ntytdfz_,hjyz";    // [rus] Теневая броня
        mobjinfo[MT_INV_SUIT].name = "pfobnysq_rjcn.v";         // [rus] Защитный костюм
        mobjinfo[MT_QUEST_UNIFORM].name = "eybajhvf_cnhf;ybrf"; // [rus] Униформа стражника
        mobjinfo[MT_QUEST_GUARD_UNIFORM].name = "eybajhvf_jabwthf";     // [rus] Униформа офицера
        mobjinfo[MT_INV_SUPERMAP].name = "rfhnf";               // [rus] Карта
        mobjinfo[MT_INV_RADAR].name = "crfyth";                 // [rus] Сканер
        mobjinfo[MT_BEACON].name = "ntktgjhnfwbjyysq_vfzr";     // [rus] Телепортационный маяк
        mobjinfo[MT_INV_TARGETER].name = "wtkterfpfntkm";       // [rus] Целеуказатель
        mobjinfo[MT_MONY_1].name = "vjytnf";                    // [rus] Монета
        mobjinfo[MT_MONY_10].name = "10_pjkjns[";               // [rus] 10 золотых
        mobjinfo[MT_MONY_25].name = "25_pjkjns[";               // [rus] 25 золотых
        mobjinfo[MT_MONY_50].name = "50_pjkjns[";               // [rus] 50 золотых
        mobjinfo[MT_MONY_300].name = "300_pjkjns[";             // [rus] 300 золотых
        mobjinfo[MT_TOKEN_RING].name = "rjkmwj";                // [rus] Кольцо
        mobjinfo[MT_INV_CHALICE].name = "xfif_lkz_gjlyjitybq";  // [rus] Чаша для подношений
        mobjinfo[MT_TOKEN_EAR].name = "e[j";                    // [rus] Ухо
        mobjinfo[MT_INV_COMMUNICATOR].name = "gthtlfnxbr";      // [rus] Передатчик
        mobjinfo[MT_AGREN].name = "cdzprf_uhfyfn";              // [rus] Связка гранат
        mobjinfo[MT_APGREN].name = "cdzprf_ajcajhys[_uhfyfn";   // [rus] Связка фосфорных гранат
        mobjinfo[MT_ACLIP].name = "j,jqvf";                     // [rus] Обойма
        mobjinfo[MT_AAMMOBOX].name = ",jtghbgfcs";              // [rus] Боеприпасы
        mobjinfo[MT_AMINI].name = "vbyb-hfrtns";                // [rus] Мини-ракеты
        mobjinfo[MT_AMINIBOX].name = "zobr_vbyb-hfrtn";         // [rus] Ящик мини-ракет
        mobjinfo[MT_ACELL].name = "'ythujzxtqrf";               // [rus] Энергоячейка
        mobjinfo[MT_APCELL].name = "'ythuj,fnfhtz";             // [rus] Энергобатарея
        mobjinfo[MT_APAROW].name = "jnhfdktyyst_,jkns";         // [rus] Отравленные болты
        mobjinfo[MT_AAROW].name = "'ktrnhbxtcrbt_,jkns";        // [rus] Электрические болты
        mobjinfo[MT_INV_SATCHEL].name = "hfytw_lkz_,jtghbgfcjd";// [rus] Ранец для боеприпасов
        mobjinfo[MT_PULSE].name = "inehvjdfz_dbynjdrf";         // [rus] Штурмовая винтовка
        mobjinfo[MT_RIFLESTAND].name = "inehvjdfz_dbynjdrf";    // [rus] Штурмовая винтовка
        mobjinfo[MT_FLAMETHROWER].name = "juytvtn";             // [rus] Огнемет
        mobjinfo[MT_TOKEN_FLAME_THROWER_PARTS].name = "ltnfkb_lkz_juytvtnf"; // [rus] Детали для огнемета
        mobjinfo[MT_MISSILELAUNCHER].name = "vbyb-hfrtnybwf";   // [rus] Мини-ракетница
        mobjinfo[MT_BLASTER].name = "bcnzpfntkm";               // [rus] Истязатель
        mobjinfo[MT_CROSSBOW].name = "fh,fktn";                 // [rus] Арбалет
        mobjinfo[MT_GRENADELAUNCHER].name = "uhfyfnjvtn";       // [rus] Гранатомет
        mobjinfo[MT_SIGIL_A].name = "cbubk";                    // [rus] СИГИЛ
        mobjinfo[MT_SIGIL_B].name = "cbubk";                    // [rus] СИГИЛ
        mobjinfo[MT_SIGIL_C].name = "cbubk";                    // [rus] СИГИЛ
        mobjinfo[MT_SIGIL_D].name = "cbubk";                    // [rus] СИГИЛ
        mobjinfo[MT_SIGIL_E].name = "cbubk";                    // [rus] СИГИЛ
        mobjinfo[MT_COUPLING_BROKEN].name = "gjdht;ltyyfz_veanf";       // [rus] Поврежденная муфта
        mobjinfo[MT_TOKEN_AMMO].name = ",jtghbgfcs";            // [rus] Боеприпасы
        mobjinfo[MT_TOKEN_HEALTH].name = "pljhjdmt";            // [rus] Здоровье
        mobjinfo[MT_TOKEN].name = "cdjlrf";                     // [rus] Сводка
        mobjinfo[MT_TOKEN_ALARM].name = "nhtdjuf";              // [rus] Тревога
        mobjinfo[MT_TOKEN_PRISON_PASS].name = "ghjgecr_d_n.hmve";       // [rus] Пропуск в тюрьму
        mobjinfo[MT_TOKEN_REPORT].name = "jnxtn";               // [rus] Отчет
        mobjinfo[MT_TOKEN_TOUGHNESS].name = ";bdextcnm";        // [rus] Живучесть
        mobjinfo[MT_TOKEN_ACCURACY].name = "vtnrjcnm";          // [rus] Меткость
        mobjinfo[MT_TOKEN_ORACLE_PASS].name = "ghjgecr_jhfrekf";        // [rus] Пропуск Оракула
        mobjinfo[MT_TOKEN_CRYSTAL].name = "rhbcnfkk_dpjhdfy";   // [rus] Кристалл взорван
        mobjinfo[MT_GATEQUEST].name = "djhjnf_dpjhdfys";        // [rus] Ворота взорваны
        mobjinfo[MT_TOKEN_BISHOP].name = "tgbcrjg_e,bn!";       // [rus] Епископ убит!
        mobjinfo[MT_TOKEN_ORACLE].name = "jhfrek_e,bn!";        // [rus] Оракул убит!
        mobjinfo[MT_TOKEN_MACIL].name = "v'qcbk_e,bn!";         // [rus] Мэйсил убит!
        mobjinfo[MT_TOKEN_LOREMASTER].name = "[hfybntkm_velhjcnb_e,bn!";     // [rus] Хранитель мудрости убит!
        mobjinfo[MT_SECRQUEST].name = "rjvgmperiodnth_dpjhdfy"; // [rus] Компьютер взорван
    }
}
