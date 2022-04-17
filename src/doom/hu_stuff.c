//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:  Heads-up displays
//



#include <ctype.h>
#include <time.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "z_zone.h"
#include "deh_main.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"
#include "hu_stuff.h"
#include "hu_lib.h"
#include "m_misc.h"
#include "rd_keybinds.h"
#include "w_wad.h"
#include "s_sound.h"
#include "doomstat.h"
#include "p_local.h"
#include "st_stuff.h" // [JN] ST_HEIGHT
#include "v_trans.h"  // [JN] Crosshair coloring
#include "v_video.h"  // [JN] V_DrawPatch

// Data.
#include "rd_lang.h"
#include "sounds.h"

#include "jn.h"


//
// Locally used constants, shortcuts.
//

// DOOM 1 map names
#define HU_TITLE        (mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE_RUS    (mapnames_rus[(gameepisode-1)*9+gamemap-1])

// DOOM 2 map names
#define HU_TITLE2       (mapnames_commercial[gamemap-1])
#define HU_TITLE2_RUS   (mapnames_commercial_rus[gamemap-1])

// Plutonia map names
#define HU_TITLEP       (mapnames_commercial[gamemap-1 + 32])
#define HU_TITLEP_RUS   (mapnames_commercial_rus[gamemap-1 + 32])

// TNT map names
#define HU_TITLET       (mapnames_commercial[gamemap-1 + 64])
#define HU_TITLET_RUS   (mapnames_commercial_rus[gamemap-1 + 64])

// No Rest for the Living map names
#define HU_TITLEN       (mapnames_commercial[gamemap-1 + 96])
#define HU_TITLEN_RUS   (mapnames_commercial_rus[gamemap-1 + 96])

// Atari Jaguar map names
#define HU_TITLEJ       (mapnames_commercial[gamemap-1 + 105])
#define HU_TITLEJ_RUS   (mapnames_commercial_rus[gamemap-1 + 105])

#define HU_TITLE_CHEX   (mapnames_chex[(gameepisode-1)*9+gamemap-1])
#define HU_TITLEHEIGHT  1
#define HU_TITLEX       0
// [JN] Initially HU_TITLEY is 167. 
// Moved a bit higher for possible non-standard font compatibility,
// and for preventing text shadows being dropped on status bar.
#define HU_TITLEY       (165 - SHORT(hu_font[0]->height))
// [JN] Jaguar: draw level name slightly higher
#define HU_TITLEY_JAG   (158 - SHORT(hu_font[0]->height))

#define HU_INPUTTOGGLE  't'
#define HU_INPUTX       HU_MSGX
#define HU_INPUTY       (HU_MSGY + HU_MSGHEIGHT*(SHORT(hu_font[0]->height) +1))
#define HU_INPUTWIDTH   64
#define HU_INPUTHEIGHT  1

#define HU_COORDX       (ORIGWIDTH - 8 * hu_font['A'-HU_FONTSTART]->width)


char *chat_macros[10] =
{
    HUSTR_CHATMACRO0,
    HUSTR_CHATMACRO1,
    HUSTR_CHATMACRO2,
    HUSTR_CHATMACRO3,
    HUSTR_CHATMACRO4,
    HUSTR_CHATMACRO5,
    HUSTR_CHATMACRO6,
    HUSTR_CHATMACRO7,
    HUSTR_CHATMACRO8,
    HUSTR_CHATMACRO9
};

char* player_names[] =
{
    HUSTR_PLRGREEN,
    HUSTR_PLRINDIGO,
    HUSTR_PLRBROWN,
    HUSTR_PLRRED
};

char        chat_char; // remove later.
static      player_t* plr;
patch_t*    hu_font[HU_FONTSIZE];
patch_t*    hu_font_small_eng[HU_FONTSIZE]; // [JN] Small, unchangeable English font (FNTSE)
patch_t*    hu_font_small_rus[HU_FONTSIZE]; // [JN] Small, unchangeable Russian font (FNTSR)
patch_t*    hu_font_big_eng[HU_FONTSIZE2];  // [JN] Big, unchangeable English font (FNTBE)
patch_t*    hu_font_big_rus[HU_FONTSIZE2];  // [JN] Big, unchangeable Russian font (FNTBR)
patch_t*    hu_font_gray[HU_FONTSIZE_GRAY]; // [JN] Small gray STCFG font

boolean     chat_on;
static      hu_itext_t w_chat;

static boolean always_off = false;
static char	    chat_dest[MAXPLAYERS];
static hu_itext_t w_inputbuffer[MAXPLAYERS];

static char    *level_name;         // [JN] Level name
static boolean  message_on;         // [JN] Item pickup
static boolean  message_on_secret;  // [JN] Revealed secret
static boolean  message_on_system;  // [JN] System messages
static boolean  message_on_chat;    // [JN] Netgame chat
boolean         message_secret_keepvisible;  // [JN] Keep revealed secret message visible.
boolean         message_dontfuckwithme;
static boolean  message_nottobefuckedwith;

static hu_stext_t w_message;        // [JN] Item pickup
static hu_stext_t w_message_secret; // [JN] Revealed secret
static hu_stext_t w_message_system; // [JN] System messages
static hu_stext_t w_message_chat;   // [JN] Netgame chat
int message_counter;                // [JN] Un-static for fading

static boolean headsupactive = false;

extern int M_StringWidth(char *string);


//
// Builtin map names.
// The actual names can be found in DStrings.h.
//

char* mapnames[] = // DOOM shareware/registered/retail (Ultimate) names.
{
    HUSTR_E1M1,
    HUSTR_E1M2,
    HUSTR_E1M3,
    HUSTR_E1M4,
    HUSTR_E1M5,
    HUSTR_E1M6,
    HUSTR_E1M7,
    HUSTR_E1M8,
    HUSTR_E1M9,

    HUSTR_E2M1,
    HUSTR_E2M2,
    HUSTR_E2M3,
    HUSTR_E2M4,
    HUSTR_E2M5,
    HUSTR_E2M6,
    HUSTR_E2M7,
    HUSTR_E2M8,
    HUSTR_E2M9,

    HUSTR_E3M1,
    HUSTR_E3M2,
    HUSTR_E3M3,
    HUSTR_E3M4,
    HUSTR_E3M5,
    HUSTR_E3M6,
    HUSTR_E3M7,
    HUSTR_E3M8,
    HUSTR_E3M9,

    HUSTR_E4M1,
    HUSTR_E4M2,
    HUSTR_E4M3,
    HUSTR_E4M4,
    HUSTR_E4M5,
    HUSTR_E4M6,
    HUSTR_E4M7,
    HUSTR_E4M8,
    HUSTR_E4M9,

    // [crispy] Sigil
    HUSTR_E5M1,
    HUSTR_E5M2,
    HUSTR_E5M3,
    HUSTR_E5M4,
    HUSTR_E5M5,
    HUSTR_E5M6,
    HUSTR_E5M7,
    HUSTR_E5M8,
    HUSTR_E5M9,

    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL"
};

char* mapnames_rus[] =
{
    HUSTR_E1M1_RUS,
    HUSTR_E1M2_RUS,
    HUSTR_E1M3_RUS,
    HUSTR_E1M4_RUS,
    HUSTR_E1M5_RUS,
    HUSTR_E1M6_RUS,
    HUSTR_E1M7_RUS,
    HUSTR_E1M8_RUS,
    HUSTR_E1M9_RUS,

    HUSTR_E2M1_RUS,
    HUSTR_E2M2_RUS,
    HUSTR_E2M3_RUS,
    HUSTR_E2M4_RUS,
    HUSTR_E2M5_RUS,
    HUSTR_E2M6_RUS,
    HUSTR_E2M7_RUS,
    HUSTR_E2M8_RUS,
    HUSTR_E2M9_RUS,

    HUSTR_E3M1_RUS,
    HUSTR_E3M2_RUS,
    HUSTR_E3M3_RUS,
    HUSTR_E3M4_RUS,
    HUSTR_E3M5_RUS,
    HUSTR_E3M6_RUS,
    HUSTR_E3M7_RUS,
    HUSTR_E3M8_RUS,
    HUSTR_E3M9_RUS,

    HUSTR_E4M1_RUS,
    HUSTR_E4M2_RUS,
    HUSTR_E4M3_RUS,
    HUSTR_E4M4_RUS,
    HUSTR_E4M5_RUS,
    HUSTR_E4M6_RUS,
    HUSTR_E4M7_RUS,
    HUSTR_E4M8_RUS,
    HUSTR_E4M9_RUS,

    // [JN] Sigil
    HUSTR_E5M1_RUS,
    HUSTR_E5M2_RUS,
    HUSTR_E5M3_RUS,
    HUSTR_E5M4_RUS,
    HUSTR_E5M5_RUS,
    HUSTR_E5M6_RUS,
    HUSTR_E5M7_RUS,
    HUSTR_E5M8_RUS,
    HUSTR_E5M9_RUS,

    // [JN] "НОВЫЙ УРОВЕНЬ"
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM"
};


char* mapnames_chex[] = // Chex Quest names.
{
    HUSTR_E1M1,
    HUSTR_E1M2,
    HUSTR_E1M3,
    HUSTR_E1M4,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,

    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,

    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,

    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,
    HUSTR_E1M5,

    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL"
};


// List of names for levels in commercial IWADs
// (doom2.wad, plutonia.wad, tnt.wad).  These are stored in a
// single large array; WADs like pl2.wad have a MAP33, and rely on
// the layout in the Vanilla executable, where it is possible to
// overflow the end of one array into the next.

char* mapnames_commercial[] =
{
    // DOOM 2 map names.
    HUSTR_1,
    HUSTR_2,
    HUSTR_3,
    HUSTR_4,
    HUSTR_5,
    HUSTR_6,
    HUSTR_7,
    HUSTR_8,
    HUSTR_9,
    HUSTR_10,
    HUSTR_11,

    HUSTR_12,
    HUSTR_13,
    HUSTR_14,
    HUSTR_15,
    HUSTR_16,
    HUSTR_17,
    HUSTR_18,
    HUSTR_19,
    HUSTR_20,

    HUSTR_21,
    HUSTR_22,
    HUSTR_23,
    HUSTR_24,
    HUSTR_25,
    HUSTR_26,
    HUSTR_27,
    HUSTR_28,
    HUSTR_29,
    HUSTR_30,
    HUSTR_31,
    HUSTR_32,

    // Plutonia WAD map names.
    PHUSTR_1,
    PHUSTR_2,
    PHUSTR_3,
    PHUSTR_4,
    PHUSTR_5,
    PHUSTR_6,
    PHUSTR_7,
    PHUSTR_8,
    PHUSTR_9,
    PHUSTR_10,
    PHUSTR_11,

    PHUSTR_12,
    PHUSTR_13,
    PHUSTR_14,
    PHUSTR_15,
    PHUSTR_16,
    PHUSTR_17,
    PHUSTR_18,
    PHUSTR_19,
    PHUSTR_20,

    PHUSTR_21,
    PHUSTR_22,
    PHUSTR_23,
    PHUSTR_24,
    PHUSTR_25,
    PHUSTR_26,
    PHUSTR_27,
    PHUSTR_28,
    PHUSTR_29,
    PHUSTR_30,
    PHUSTR_31,
    PHUSTR_32,

    // TNT WAD map names.
    THUSTR_1,
    THUSTR_2,
    THUSTR_3,
    THUSTR_4,
    THUSTR_5,
    THUSTR_6,
    THUSTR_7,
    THUSTR_8,
    THUSTR_9,
    THUSTR_10,
    THUSTR_11,

    THUSTR_12,
    THUSTR_13,
    THUSTR_14,
    THUSTR_15,
    THUSTR_16,
    THUSTR_17,
    THUSTR_18,
    THUSTR_19,
    THUSTR_20,

    THUSTR_21,
    THUSTR_22,
    THUSTR_23,
    THUSTR_24,
    THUSTR_25,
    THUSTR_26,
    THUSTR_27,
    THUSTR_28,
    THUSTR_29,
    THUSTR_30,
    THUSTR_31,
    THUSTR_32,

    // Нет покоя для живых.
    NHUSTR_1,
    NHUSTR_2,
    NHUSTR_3,
    NHUSTR_4,
    NHUSTR_5,
    NHUSTR_6,
    NHUSTR_7,
    NHUSTR_8,
    NHUSTR_9,

    // [JN] Atari Jaguar
    JHUSTR_1,
    JHUSTR_2,
    JHUSTR_3,
    JHUSTR_4,
    JHUSTR_5,
    JHUSTR_6,
    JHUSTR_7,
    JHUSTR_8,
    JHUSTR_9,
    JHUSTR_10,
    JHUSTR_11,
    JHUSTR_12,
    JHUSTR_13,
    JHUSTR_14,
    JHUSTR_15,
    JHUSTR_16,
    JHUSTR_17,
    JHUSTR_18,
    JHUSTR_19,
    JHUSTR_20,
    JHUSTR_21,
    JHUSTR_22,
    JHUSTR_23,
    JHUSTR_24,
    JHUSTR_25,
    JHUSTR_26
};

char* mapnames_commercial_rus[] =
{
    // DOOM 2 map names.
    HUSTR_1_RUS,
    HUSTR_2_RUS,
    HUSTR_3_RUS,
    HUSTR_4_RUS,
    HUSTR_5_RUS,
    HUSTR_6_RUS,
    HUSTR_7_RUS,
    HUSTR_8_RUS,
    HUSTR_9_RUS,
    HUSTR_10_RUS,
    HUSTR_11_RUS,

    HUSTR_12_RUS,
    HUSTR_13_RUS,
    HUSTR_14_RUS,
    HUSTR_15_RUS,
    HUSTR_16_RUS,
    HUSTR_17_RUS,
    HUSTR_18_RUS,
    HUSTR_19_RUS,
    HUSTR_20_RUS,

    HUSTR_21_RUS,
    HUSTR_22_RUS,
    HUSTR_23_RUS,
    HUSTR_24_RUS,
    HUSTR_25_RUS,
    HUSTR_26_RUS,
    HUSTR_27_RUS,
    HUSTR_28_RUS,
    HUSTR_29_RUS,
    HUSTR_30_RUS,
    HUSTR_31_RUS,
    HUSTR_32_RUS,

    // Plutonia WAD map names.
    PHUSTR_1_RUS,
    PHUSTR_2_RUS,
    PHUSTR_3_RUS,
    PHUSTR_4_RUS,
    PHUSTR_5_RUS,
    PHUSTR_6_RUS,
    PHUSTR_7_RUS,
    PHUSTR_8_RUS,
    PHUSTR_9_RUS,
    PHUSTR_10_RUS,
    PHUSTR_11_RUS,

    PHUSTR_12_RUS,
    PHUSTR_13_RUS,
    PHUSTR_14_RUS,
    PHUSTR_15_RUS,
    PHUSTR_16_RUS,
    PHUSTR_17_RUS,
    PHUSTR_18_RUS,
    PHUSTR_19_RUS,
    PHUSTR_20_RUS,

    PHUSTR_21_RUS,
    PHUSTR_22_RUS,
    PHUSTR_23_RUS,
    PHUSTR_24_RUS,
    PHUSTR_25_RUS,
    PHUSTR_26_RUS,
    PHUSTR_27_RUS,
    PHUSTR_28_RUS,
    PHUSTR_29_RUS,
    PHUSTR_30_RUS,
    PHUSTR_31_RUS,
    PHUSTR_32_RUS,

    // TNT WAD map names.
    THUSTR_1_RUS,
    THUSTR_2_RUS,
    THUSTR_3_RUS,
    THUSTR_4_RUS,
    THUSTR_5_RUS,
    THUSTR_6_RUS,
    THUSTR_7_RUS,
    THUSTR_8_RUS,
    THUSTR_9_RUS,
    THUSTR_10_RUS,
    THUSTR_11_RUS,

    THUSTR_12_RUS,
    THUSTR_13_RUS,
    THUSTR_14_RUS,
    THUSTR_15_RUS,
    THUSTR_16_RUS,
    THUSTR_17_RUS,
    THUSTR_18_RUS,
    THUSTR_19_RUS,
    THUSTR_20_RUS,

    THUSTR_21_RUS,
    THUSTR_22_RUS,
    THUSTR_23_RUS,
    THUSTR_24_RUS,
    THUSTR_25_RUS,
    THUSTR_26_RUS,
    THUSTR_27_RUS,
    THUSTR_28_RUS,
    THUSTR_29_RUS,
    THUSTR_30_RUS,
    THUSTR_31_RUS,
    THUSTR_32_RUS,

    // Нет покоя для живых.
    NHUSTR_1_RUS,
    NHUSTR_2_RUS,
    NHUSTR_3_RUS,
    NHUSTR_4_RUS,
    NHUSTR_5_RUS,
    NHUSTR_6_RUS,
    NHUSTR_7_RUS,
    NHUSTR_8_RUS,
    NHUSTR_9_RUS,

    // [JN] Atari Jaguar
    JHUSTR_1_RUS,
    JHUSTR_2_RUS,
    JHUSTR_3_RUS,
    JHUSTR_4_RUS,
    JHUSTR_5_RUS,
    JHUSTR_6_RUS,
    JHUSTR_7_RUS,
    JHUSTR_8_RUS,
    JHUSTR_9_RUS,
    JHUSTR_10_RUS,
    JHUSTR_11_RUS,
    JHUSTR_12_RUS,
    JHUSTR_13_RUS,
    JHUSTR_14_RUS,
    JHUSTR_15_RUS,
    JHUSTR_16_RUS,
    JHUSTR_17_RUS,
    JHUSTR_18_RUS,
    JHUSTR_19_RUS,
    JHUSTR_20_RUS,
    JHUSTR_21_RUS,
    JHUSTR_22_RUS,
    JHUSTR_23_RUS,
    JHUSTR_24_RUS,
    JHUSTR_25_RUS,
    JHUSTR_26_RUS
};


void HU_Init(void)
{
    int     i;
    int     j;
    int     o, p;
    int     q, r;
    
    int     g;
    char    buffer[9];

    // load the heads-up font
    j = HU_FONTSTART;
    o = HU_FONTSTART;
    p = HU_FONTSTART;
    q = HU_FONTSTART2;
    r = HU_FONTSTART2;
    g = HU_FONTSTART_GRAY;

    // [JN] Standard STCFN font
    for (i=0;i<HU_FONTSIZE;i++)
    {
        DEH_snprintf(buffer, 9, "STCFN%.3d", j++);
        hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Small, unchangeable English font (FNTSE)
    for (i=0;i<HU_FONTSIZE;i++)
    {
        DEH_snprintf(buffer, 9, "FNTSE%.3d", o++);
        hu_font_small_eng[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Small, unchangeable Russian font (FNTSR)
    for (i=0;i<HU_FONTSIZE;i++)
    {
        DEH_snprintf(buffer, 9, "FNTSR%.3d", p++);
        hu_font_small_rus[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Big, unchangeable English font (FNTBE)
    for (i=0;i<HU_FONTSIZE2;i++)
    {
        DEH_snprintf(buffer, 9, "FNTBE%.3d", q++);
        hu_font_big_eng[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Big, unchangeable Russian font (FNTBR)
    for (i=0;i<HU_FONTSIZE2;i++)
    {
        DEH_snprintf(buffer, 9, "FNTBR%.3d", r++);
        hu_font_big_rus[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }    

    // [JN] Small gray STCFG font, used for local time widget and FPS counter
    for (i=0;i<HU_FONTSIZE_GRAY;i++)
    {
        DEH_snprintf(buffer, 9, "STCFG%.3d", g++);
        hu_font_gray[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
}


void HU_Stop(void) 
{
    headsupactive = false;
}


void HU_Start(void)
{
    int     i;
    char*   s;

    if (headsupactive)
    HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_on_secret = false;
    message_on_system = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    message_secret_keepvisible = false;
    chat_on = false;

    // create the message widget
    HUlib_initSText(&w_message, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, 
                    english_language ? hu_font : hu_font_small_rus,
                    HU_FONTSTART, &message_on);

    HUlib_initSText(&w_message_secret, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, 
                    english_language ? hu_font : hu_font_small_rus,
                    HU_FONTSTART, &message_on_secret);

    HUlib_initSText(&w_message_system, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, 
                    english_language ? hu_font : hu_font_small_rus,
                    HU_FONTSTART, &message_on_system);

    switch ( logical_gamemission )
    {
        case doom:
        s = english_language ? HU_TITLE : HU_TITLE_RUS;
        break;

        case doom2:
        s = english_language ? HU_TITLE2 : HU_TITLE2_RUS;
        break;

        case pack_plut:
        s = english_language ? HU_TITLEP : HU_TITLEP_RUS;
        break;

        case pack_tnt:
        s = english_language ? HU_TITLET : HU_TITLET_RUS;
        break;

        case pack_nerve:
        if (gamemap <= 9)
        s = english_language ? HU_TITLEN : HU_TITLEN_RUS;
        else
        s = english_language ? HU_TITLE2 : HU_TITLE2_RUS;
        break;

        case jaguar:
        if (gamemap <= 26)
        s = english_language ? HU_TITLEJ : HU_TITLEJ_RUS;
        else
        s = english_language ? HU_TITLE2 : HU_TITLE2_RUS;
        break;
        
        default:
        s = english_language ? "Unknown level" : "ytbpdtcnysq ehjdtym"; // [JN] "Неизвестный уровень"
        break;
    }

    if (logical_gamemission == doom && gameversion == exe_chex)
    {
        s = HU_TITLE_CHEX;
    }

    // dehacked substitution to get modified level name
    s = DEH_String(s);

    // [JN] Create the level name string.
    level_name = s;

    // create the chat widget
    HUlib_initIText(&w_chat, HU_INPUTX, HU_INPUTY, hu_font, HU_FONTSTART, &chat_on);

    // [JN] Separate netgame chat widget to use English only characters
    HUlib_initSText(&w_message_chat, HU_MSGX, HU_MSGY, HU_MSGHEIGHT, 
                        hu_font, HU_FONTSTART, &message_on_chat);

    // create the inputbuffer widgets
    for (i=0 ; i<MAXPLAYERS ; i++)
    HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);

    headsupactive = true;
}

// [crispy] print a bar indicating demo progress at the bottom of the screen
void HU_DemoProgressBar (void)
{
    const int i = screenwidth * defdemotics / deftotaldemotics;

    V_DrawHorizLine(0, SCREENHEIGHT - 2, i, 0); // [crispy] black
    V_DrawHorizLine(0, SCREENHEIGHT - 1, i, 4); // [crispy] white
}


void HU_Drawer(void)
{
    static char str[32];
    int time = leveltime / TICRATE;
    int totaltime = (totalleveltimes / TICRATE) + (leveltime / TICRATE);
    const int wide_4_3 = aspect_ratio >= 2 && screenblocks == 9 ? wide_delta : 0;
    const int map_y = gamemission == jaguar ? 151 : 159;
    plr = &players[consoleplayer];

    HUlib_drawSText(&w_message, msg_pickup);
    HUlib_drawSText(&w_message_secret, msg_secret);
    HUlib_drawSText(&w_message_system, msg_system);
    HUlib_drawSText(&w_message_chat, msg_chat);

    HUlib_drawIText(&w_chat);

    // [JN] Level name (in automap only).
    if (automapactive)
    {
        sprintf(str, "%s", level_name);

        if (english_language)
        {
            dp_translation = hud_stats_color ? cr[CR_YELLOW] : NULL;
            RD_M_DrawTextA(str, wide_4_3, map_y);
            dp_translation = NULL;
        }
        else
        {
            RD_M_DrawTextSmallRUS(str, wide_4_3, map_y,
                                  hud_stats_color ? CR_YELLOW : CR_NONE);
        }
    }

    // [JN] Level stats widgets.
    if (!vanillaparm)
    {
        if (((automapactive && automap_stats == 1) || automap_stats == 2))
        {
            // Kills:
            sprintf(str, plr->extrakillcount ? "%d+%d/%d" : "%d/%d",
                    plr->killcount,
                    plr->extrakillcount ? plr->extrakillcount : totalkills,
                    totalkills);

            english_language ? RD_M_DrawTextA("K:", wide_4_3, 9) :
                               RD_M_DrawTextSmallRUS("D:", wide_4_3, 9, CR_NONE);
            
            dp_translation = hud_stats_color == 0 ? NULL :
                             plr->killcount == 0 ? cr[CR_RED] :
                             plr->killcount < totalkills ? cr[CR_YELLOW] : cr[CR_GREEN];
            RD_M_DrawTextA(str, wide_4_3 + 16, 9);
            dp_translation = NULL;

            // Items:
            sprintf(str, "%d/%d", plr->itemcount, totalitems);

            english_language ? RD_M_DrawTextA("I:", wide_4_3, 17) :
                               RD_M_DrawTextSmallRUS("G:", wide_4_3, 17, CR_NONE);

            dp_translation = hud_stats_color == 0 ? NULL :
                             plr->itemcount == 0 ? cr[CR_RED] :
                             plr->itemcount < totalitems ? cr[CR_YELLOW] : cr[CR_GREEN];
            RD_M_DrawTextA(str, wide_4_3 + 16, 17);
            dp_translation = NULL;

            // Secret:
            sprintf(str, "%d/%d", plr->secretcount, totalsecret);

            english_language ? RD_M_DrawTextA("S:", wide_4_3, 25) :
                               RD_M_DrawTextSmallRUS("N:", wide_4_3, 25, CR_NONE);

            dp_translation = hud_stats_color == 0 ? NULL :
                             plr->secretcount == 0 ? cr[CR_RED] :
                             plr->secretcount < totalsecret ? cr[CR_YELLOW] : cr[CR_GREEN];
            RD_M_DrawTextA(str, wide_4_3 + 16, 25);
            dp_translation = NULL;
        }

        // Skill Level:
        if (((automapactive && automap_skill == 1) || automap_skill == 2))
        {
            sprintf(str, "%d", gameskill+1);

            english_language ? RD_M_DrawTextA("SKL:", wide_4_3, 33) :
                               RD_M_DrawTextSmallRUS("CK;:", wide_4_3, 33, CR_NONE);

            dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
            RD_M_DrawTextA(str, wide_4_3 + (english_language ? 31 : 36), 33);
            dp_translation = NULL;
        }

        // Level Time:
        if (((automapactive && automap_level_time == 1) || automap_level_time == 2))
        {
            sprintf(str, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);

            english_language ? RD_M_DrawTextA("LEVEL", wide_4_3, 49) :
                               RD_M_DrawTextSmallRUS("EHJDTYM", wide_4_3, 49, CR_NONE);

            dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
            RD_M_DrawTextA(str, wide_4_3, 57);
            dp_translation = NULL;
        }

        // Total Time:
        if (((automapactive && automap_total_time == 1) || automap_total_time == 2))
        {
            sprintf(str, "%02d:%02d:%02d", totaltime/3600, (totaltime%3600)/60, totaltime%60);

            english_language ? RD_M_DrawTextA("TOTAL", wide_4_3, 73) :
                               RD_M_DrawTextSmallRUS("J,OTT", wide_4_3, 73, CR_NONE);

            dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
            RD_M_DrawTextA(str, wide_4_3, 81);
            dp_translation = NULL;
        }

        // Player Coords:
        if (((automapactive && automap_coords == 1) || automap_coords == 2))
        {
            dp_translation = hud_stats_color == 0 ? NULL : cr[CR_GREEN];
            RD_M_DrawTextA("X:", wide_4_3, 97);
            RD_M_DrawTextA("Y:", wide_4_3, 105);
            RD_M_DrawTextA("Z:", wide_4_3, 113);
            RD_M_DrawTextA("ANG:", wide_4_3, 121);
            dp_translation = NULL;

            dp_translation = hud_stats_color == 0 ? NULL : cr[CR_WHITE];
            sprintf(str, "%d", plr->mo->x >> FRACBITS);
            RD_M_DrawTextA(str, wide_4_3 + 16, 97);
            sprintf(str, "%d", plr->mo->y >> FRACBITS);
            RD_M_DrawTextA(str, wide_4_3 + 16, 105);
            sprintf(str, "%d", plr->mo->z >> FRACBITS);
            RD_M_DrawTextA(str, wide_4_3 + 16, 113);
            sprintf(str, "%d", plr->mo->angle / ANG1);
            RD_M_DrawTextA(str, wide_4_3 + 32, 121);
            dp_translation = NULL;
        }

        // [JN] Draw crosshair. 
        // Thanks to Fabian Greffrath for ORIGWIDTH, ORIGHEIGHT and ST_HEIGHT values,
        // thanks to Zodomaniac for proper health values!
        if (crosshair_draw && !automapactive && !menuactive)
        {
            Crosshair_Draw();
        }

        // [crispy] demo timer widget
        if (demoplayback && (demotimer == 1 || demotimer == 3))
        {
            ST_DrawDemoTimer(demotimerdir ? (deftotaldemotics - defdemotics) : defdemotics);
        }
        else if (demorecording && (demotimer == 2 || demotimer == 3))
        {
            ST_DrawDemoTimer(leveltime);
        }

        // [crispy] demo progress bar
        if (demoplayback && demobar)
        {
            HU_DemoProgressBar();
        }
    }
}


void HU_Erase(void)
{
    HUlib_eraseSText(&w_message);
    HUlib_eraseSText(&w_message_secret);
    HUlib_eraseSText(&w_message_system);
    HUlib_eraseSText(&w_message_chat);
    HUlib_eraseIText(&w_chat);
}


void HU_Ticker(void)
{
    // tick down message counter if message is up
    if (message_counter && !--message_counter)
    {
        message_on = false;
        message_on_secret = false;
        message_on_system = false;
        message_on_chat = false;
        message_nottobefuckedwith = false;
        message_secret_keepvisible = false;
    }

    // [JN] Zero-out item pickup messages while 
    // revealed secret message is visible.
    if (message_secret_keepvisible)
    {
        plr->message = 0;
    }

    if (showMessages || message_dontfuckwithme)
    {
        // display message if necessary
        // [JN] Item pickup
        if (((plr->message && !message_nottobefuckedwith) || (plr->message && message_dontfuckwithme)) 
        && !message_secret_keepvisible)
        {
            HUlib_addMessageToSText(&w_message, 0, plr->message);
            message_nottobefuckedwith = 0;
            plr->message = 0;
            plr->message_secret = 0;
            plr->message_system = 0;
            plr->message_chat = 0;
            message_on = true;
            message_on_secret = false;
            message_on_system = false;
            message_on_chat = false;
            message_counter = messages_timeout * TICRATE;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }

        // [JN] Revealed secret
        if ((plr->message_secret && !message_nottobefuckedwith) || (plr->message_secret && message_dontfuckwithme))
        {
            HUlib_addMessageToSText(&w_message_secret, 0, plr->message_secret);
            plr->message = 0;
            plr->message_secret = 0;
            plr->message_system = 0;
            plr->message_chat = 0;
            message_on = false;
            message_on_secret = true;
            message_on_system = false;
            message_on_chat = false;
            message_counter = messages_timeout * TICRATE;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }

        // [JN] System messages
        if ((plr->message_system && !message_nottobefuckedwith) || (plr->message_system && message_dontfuckwithme))
        {
            HUlib_addMessageToSText(&w_message_system, 0, plr->message_system);
            plr->message = 0;
            plr->message_secret = 0;
            plr->message_system = 0;
            plr->message_chat = 0;
            message_on = false;
            message_on_secret = false;
            message_on_system = true;
            message_on_chat = false;
            message_counter = messages_timeout * TICRATE;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }

        // [JN] Netgame chat
        if ((plr->message_chat && !message_nottobefuckedwith) || (plr->message_chat && message_dontfuckwithme))
        {
            HUlib_addMessageToSText(&w_message_chat, 0, plr->message_chat);
            plr->message = 0;
            plr->message_secret = 0;
            plr->message_system = 0;
            plr->message_chat = 0;
            message_on = false;
            message_on_secret = false;
            message_on_system = false;
            message_on_chat = true;
            message_counter = messages_timeout * TICRATE;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }
    } // else message_on = false;

    // check for incoming chat characters
    if (netgame)
    {
        int  i;
        char c;

        for (i=0 ; i<MAXPLAYERS; i++)
        {
            if (!playeringame[i])
            continue;

            if (i != consoleplayer && (c = players[i].cmd.chatchar))
            {
                if (c <= HU_BROADCAST)
                {
                    chat_dest[i] = c;
                }
                else
                {
                    int rc = HUlib_keyInIText(&w_inputbuffer[i], c);

                    if (rc && c == KEY_ENTER)
                    {
                        if (w_inputbuffer[i].l.len && (chat_dest[i] == consoleplayer+1 || chat_dest[i] == HU_BROADCAST))
                        {
                            HUlib_addMessageToSText(&w_message_chat, DEH_String(player_names[i]), w_inputbuffer[i].l.l);

                            message_nottobefuckedwith = true;
                            message_on_chat = true;
                            message_counter = messages_timeout * TICRATE;

                            if ( gamemode == commercial )
                            S_StartSound(0, sfx_radio);
                            else
                            S_StartSound(0, sfx_tink);
                        }

                        HUlib_resetIText(&w_inputbuffer[i]);
                    }
                }

                players[i].cmd.chatchar = 0;
            }
        }
    }
}


#define QUEUESIZE 128

static char chatchars[QUEUESIZE];
static int  head = 0;
static int  tail = 0;


void HU_queueChatChar(char c)
{
    if (((head + 1) & (QUEUESIZE-1)) == tail)
    {
        plr->message = DEH_String(english_language ? HUSTR_MSGU : HUSTR_MSGU_RUS);
    }
    else
    {
        chatchars[head] = c;
        head = (head + 1) & (QUEUESIZE-1);
    }
}


char HU_dequeueChatChar(void)
{
    char c;

    if (head != tail)
    {
        c = chatchars[tail];
        tail = (tail + 1) & (QUEUESIZE-1);
    }
    else
    {
        c = 0;
    }

    return c;
}


static void StartChatInput(int dest)
{
    chat_on = true;
    HUlib_resetIText(&w_chat);
    HU_queueChatChar(HU_BROADCAST);

    I_StartTextInput(0, 8, screenwidth, 16);
}


static void StopChatInput(void)
{
    chat_on = false;
    I_StopTextInput();
}


boolean HU_Responder(event_t *ev)
{
    static char lastmessage[HU_MAXLINELENGTH+1];
    char*       macromessage;
    boolean     eatkey = false;

    static boolean  altdown = false;
    unsigned char   c;
    int             i;
    int             numplayers;

    numplayers = 0;
    for (i=0 ; i<MAXPLAYERS ; i++)
        numplayers += playeringame[i];

    if (ev->data1 == KEY_RSHIFT || ev->data1 == KEY_LSHIFT)
    {
        return false;
    }
    else if (ev->data1 == KEY_RALT || ev->data1 == KEY_LALT)
    {
        altdown = ev->type == ev_keydown;
        return false;
    }

    if (!chat_on)
    {
        if (netgame && BK_isKeyDown(ev, bk_multi_msg))
        {
            eatkey = true;
            StartChatInput(HU_BROADCAST);
        }
        else if (netgame && numplayers > 2)
        {
            for (i=0; i<MAXPLAYERS ; i++)
            {
                if (BK_isKeyDown(ev, bk_multi_msg_player_0 + i))
                {
                    if (playeringame[i] && i!=consoleplayer)
                    {
                        eatkey = true;
                        StartChatInput(i + 1);
                        break;
                    }
                    else if (i == consoleplayer)
                    {
                        static int num_nobrainers = 0;

                        num_nobrainers++;
                        if (num_nobrainers < 3)
                            plr->message_system = DEH_String(english_language ?
                                                      HUSTR_TALKTOSELF1 : HUSTR_TALKTOSELF1_RUS);
                        else if (num_nobrainers < 6)
                            plr->message_system = DEH_String(english_language ?
                                                      HUSTR_TALKTOSELF2 : HUSTR_TALKTOSELF2_RUS);
                        else if (num_nobrainers < 9)
                            plr->message_system = DEH_String(english_language ?
                                                      HUSTR_TALKTOSELF3 : HUSTR_TALKTOSELF3_RUS);
                        else if (num_nobrainers < 32)
                            plr->message_system = DEH_String(english_language ?
                                                      HUSTR_TALKTOSELF4 : HUSTR_TALKTOSELF4_RUS);
                        else
                            plr->message_system = DEH_String(english_language ?
                                                      HUSTR_TALKTOSELF5 : HUSTR_TALKTOSELF5_RUS);
                    }
                }
            }
        }
    }
    else
    {
        if (ev->type != ev_keydown)
            return false;

        // send a macro
        if (altdown)
        {
            c = ev->data1 - '0';
            if (c > 9)
                return false;
            // fprintf(stderr, "got here\n");
            macromessage = chat_macros[c];

            // kill last message with a '\n'
            HU_queueChatChar(KEY_ENTER); // DEBUG!!!

            // send the macro message
            while (*macromessage)
                HU_queueChatChar(*macromessage++);
            HU_queueChatChar(KEY_ENTER);

            // leave chat mode and notify that it was sent
            StopChatInput();
            M_StringCopy(lastmessage, chat_macros[c], sizeof(lastmessage));
            // [JN] Do not repeat typed message because of characters problem.
            // plr->message = lastmessage;
            eatkey = true;
        }
        else
        {
            c = ev->data3;

            eatkey = HUlib_keyInIText(&w_chat, c);
            if (eatkey)
            {
                // static unsigned char buf[20]; // DEBUG
                HU_queueChatChar(c);

                // M_snprintf(buf, sizeof(buf), "KEY: %d => %d", ev->data1, c);
                //        plr->message = buf;
            }
            if (c == KEY_ENTER || c == KEYP_ENTER)
            {
                StopChatInput();
                if (w_chat.l.len)
                {
                    M_StringCopy(lastmessage, w_chat.l.l, sizeof(lastmessage));
                    // [JN] Do not repeat typed message because of characters problem.
                    // plr->message = lastmessage;
                }
            }
            else if (c == KEY_ESCAPE)
            {
                StopChatInput();
            }
        }
    }

    return eatkey;
}

// =============================================================================
//
// [JN] Crosshair routines. Pre/re-defining, drawing, coloring.
//
// =============================================================================

patch_t      *CrosshairPatch;
byte         *CrosshairOpacity;
static void (*Crosshair_Draw_Func) (void);

// -----------------------------------------------------------------------------
// Crosshair_DefinePatch: which GFX patch will be used.
// -----------------------------------------------------------------------------

patch_t *Crosshair_DefinePatch (void)
{
    return CrosshairPatch =
        W_CacheLumpName(crosshair_shape == 1 ? "XHAIR_2" :
                        crosshair_shape == 2 ? "XHAIR_3" :
                        crosshair_shape == 3 ? "XHAIR_4" :
                        crosshair_shape == 4 ? "XHAIR_5" :
                        crosshair_shape == 5 ? "XHAIR_6" :
                        crosshair_shape == 6 ? "XHAIR_7" :
                                               "XHAIR_1", PU_CACHE);
}

// -----------------------------------------------------------------------------
// Crosshair_DefineOpacity: what amount of transparency will be used.
// -----------------------------------------------------------------------------

void Crosshair_DefineOpacity (void)
{
    CrosshairOpacity = crosshair_opacity == 0 ? transtable20 :
                       crosshair_opacity == 1 ? transtable30 :
                       crosshair_opacity == 2 ? transtable40 :
                       crosshair_opacity == 3 ? transtable50 :
                       crosshair_opacity == 4 ? transtable60 :
                       crosshair_opacity == 5 ? transtable70 :
                       crosshair_opacity == 6 ? transtable80 :
                       crosshair_opacity == 7 ? transtable90 :
                                                NULL;
}

// -----------------------------------------------------------------------------
// Crosshair_Colorize_inMenu: coloring routine for menu. Cycling through colors.
// -----------------------------------------------------------------------------

void Crosshair_Colorize_inMenu (void)
{
    if (crosshair_type == 1)
    {
        if (CrosshairShowcaseTimeout > 105)
        {
            CrosshairShowcaseTimeout = 105;
        }

        dp_translation = CrosshairShowcaseTimeout >= 70 ? cr[CR_RED]    :
                         CrosshairShowcaseTimeout >= 35 ? cr[CR_YELLOW] :
                                                          cr[CR_GREEN];
    }
    else if (crosshair_type == 2)
    {
        if (CrosshairShowcaseTimeout > 70)
        {
            CrosshairShowcaseTimeout = 70;
        }

        dp_translation = CrosshairShowcaseTimeout >= 35 ? cr[CR_RED] :
                                                          cr[CR_BLUE];
    }
    else if (crosshair_type == 3)
    {
        dp_translation = CrosshairShowcaseTimeout >= 105 ? cr[CR_RED]    :
                         CrosshairShowcaseTimeout >=  70 ? cr[CR_YELLOW] :
                         CrosshairShowcaseTimeout >=  35 ? cr[CR_GREEN]  :
                                                           cr[CR_BLUE];
    }
}

// -----------------------------------------------------------------------------
// Crosshair_Colorize_inGame: ingame coloring routine, actual colors/values.
// -----------------------------------------------------------------------------

static void Crosshair_Colorize_inGame (void)
{
    if (crosshair_type == 1)
    {
        dp_translation = plr->health >= 67 ? cr[CR_GREEN]  :
                         plr->health >= 34 ? cr[CR_YELLOW] :
                                             cr[CR_RED];
    }
    else if (crosshair_type == 2)
    {
        P_AimLineAttack(plr->mo, plr->mo->angle, MISSILERANGE);

        if (linetarget)
        {
            dp_translation = cr[CR_BLUE];
        }
    }
    else if (crosshair_type == 3)
    {
        dp_translation = plr->health >= 67 ? cr[CR_GREEN]  :
                         plr->health >= 34 ? cr[CR_YELLOW] :
                                             cr[CR_RED];

        P_AimLineAttack(plr->mo, plr->mo->angle, MISSILERANGE);

        if (linetarget)
        {
            dp_translation = cr[CR_BLUE];
        }
    }
}

// -----------------------------------------------------------------------------
// Crosshair_Draw_Scaled: drawing func for scaled (big) crosshair.
// -----------------------------------------------------------------------------

static void Crosshair_Draw_Scaled (void)
{
    V_DrawPatch(origwidth/2, screenblocks <= 10 ? 84 : 102,
                CrosshairPatch, CrosshairOpacity);
}

// -----------------------------------------------------------------------------
// Crosshair_Draw_Unscaled: drawing func for unscaled (small) crosshair.
// -----------------------------------------------------------------------------

static void Crosshair_Draw_Unscaled (void)
{
    V_DrawPatchUnscaled(screenwidth/2, screenblocks <= 10 ? 168 : 204,
                        CrosshairPatch, CrosshairOpacity);
}

// -----------------------------------------------------------------------------
// Crosshair_DefineDrawingFunc: predefinition of drawing func for later use.
// -----------------------------------------------------------------------------

void Crosshair_DefineDrawingFunc (void)
{
    Crosshair_Draw_Func = crosshair_scale ? Crosshair_Draw_Scaled :
                                            Crosshair_Draw_Unscaled;
}

// -----------------------------------------------------------------------------
// Crosshair_Draw: actual coloring and drawing.
// -----------------------------------------------------------------------------

void Crosshair_Draw (void)
{
    Crosshair_Colorize_inGame();
    Crosshair_Draw_Func();
    dp_translation = NULL;
}
