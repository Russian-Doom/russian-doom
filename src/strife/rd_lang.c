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
    }
}