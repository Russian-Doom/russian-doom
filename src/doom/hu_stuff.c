//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
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
#include "st_stuff.h" // [JN] ST_HEIGHT
#include "v_trans.h"  // [JN] Crosshair coloring
#include "v_video.h"  // [JN] V_DrawPatch

// Data.
#include "rd_lang.h"
#include "sounds.h"

#include "crispy.h"
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

static      hu_textline_t w_title;
boolean     chat_on;
static      hu_itext_t w_chat;

// [JN] Automap widgets
static hu_textline_t w_kills_title, w_kills;
static hu_textline_t w_items_title, w_items;
static hu_textline_t w_scrts_title, w_scrts;
static hu_textline_t w_skill_title, w_skill;
static hu_textline_t w_ltime_title, w_ltime;
static hu_textline_t w_ttime_title, w_ttime;
static hu_textline_t w_coord_x_title, w_coord_x;
static hu_textline_t w_coord_y_title, w_coord_y;
static hu_textline_t w_coord_z_title, w_coord_z;
static hu_textline_t w_ang_title, w_ang;
static int w_kills_title_lengh;
static int w_items_title_lengh;
static int w_scrts_title_lengh;
static int w_skill_title_lengh;
static int w_coord_x_title_lengh;
static int w_coord_y_title_lengh;
static int w_coord_z_title_lengh;
static int w_ang_title_lengh;
byte *hud_stats_color_set;
byte *hud_coords_color_set;
byte *hud_level_color_set;
byte *hud_values_color_set;

static boolean always_off = false;
static char	    chat_dest[MAXPLAYERS];
static hu_itext_t w_inputbuffer[MAXPLAYERS];

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

// -----------------------------------------------------------------------------
// HU_Init_Widgets
// [JN] Initialize HUD widget colors and lengths.
// -----------------------------------------------------------------------------

void HU_Init_Widgets (void)
{
    // Set lengths:
    if (english_language)
    {
        w_kills_title_lengh = M_StringWidth("kills: ");
        w_items_title_lengh = M_StringWidth("items: ");
        w_scrts_title_lengh = M_StringWidth("secret: ");
        w_skill_title_lengh = M_StringWidth("skill: ");
        w_coord_x_title_lengh = M_StringWidth("x: ");
        w_coord_y_title_lengh = M_StringWidth("y: ");
        w_coord_z_title_lengh = M_StringWidth("z: ");
        w_ang_title_lengh = M_StringWidth("ang: ");
    }
    else
    {
        w_kills_title_lengh = M_StringWidth("dhfub: ");     // враги:
        w_items_title_lengh = M_StringWidth("ghtlvtns: ");  // предметы:
        w_scrts_title_lengh = M_StringWidth("nfqybrb: ");   // тайники:
        w_skill_title_lengh = M_StringWidth("ckj;yjcnm: "); // сложность:
        w_coord_x_title_lengh = M_StringWidth("[: ");       // x:
        w_coord_y_title_lengh = M_StringWidth("e: ");       // y:
        w_coord_z_title_lengh = M_StringWidth("x: ");       // z:
        w_ang_title_lengh = M_StringWidth("eujk: ");        // угол:
    }

    // Set colors:
    hud_stats_color_set = NULL;
    hud_coords_color_set = hud_widget_colors == 2 ? cr[CR_GREEN] : NULL;
    hud_level_color_set = hud_widget_colors == 2 && !vanillaparm ? cr[CR_YELLOW] : NULL;
    hud_values_color_set = hud_widget_colors == 0 ? NULL : cr[CR_WHITE];
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

    // create the map title widget
    HUlib_initTextLine(&w_title, HU_TITLEX, (gamemission == jaguar ?
                                             HU_TITLEY_JAG :
                                             HU_TITLEY),
                                             english_language ? hu_font : hu_font_small_rus, 
                                             HU_FONTSTART);

    HUlib_initTextLine(&w_kills_title,
		       HU_TITLEX, (HU_MSGY+1) + 1 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_kills,
		       HU_TITLEX + w_kills_title_lengh, (HU_MSGY+1) + 1 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_items_title,
		       HU_TITLEX, (HU_MSGY+1) + 2 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_items,
		       HU_TITLEX + w_items_title_lengh, (HU_MSGY+1) + 2 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_scrts_title,
		       HU_TITLEX, (HU_MSGY+1) + 3 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_scrts,
		       HU_TITLEX + w_scrts_title_lengh, (HU_MSGY+1) + 3 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_skill_title,
		       HU_TITLEX, (HU_MSGY+1) + 4 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_skill,
		       HU_TITLEX + w_skill_title_lengh, (HU_MSGY+1) + 4 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_ltime_title,
		       HU_TITLEX, (HU_MSGY+1) + 6 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_ltime,
		       HU_TITLEX, (HU_MSGY+1) + 7 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_ttime_title,
		       HU_TITLEX, (HU_MSGY+1) + 9 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_ttime,
		       HU_TITLEX, (HU_MSGY+1) + 10 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coord_x_title,
		       HU_TITLEX, (HU_MSGY+1) + 12 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coord_x,
		       HU_TITLEX + w_coord_x_title_lengh, (HU_MSGY+1) + 12 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coord_y_title,
		       HU_TITLEX, (HU_MSGY+1) + 13 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coord_y,
		       HU_TITLEX + w_coord_y_title_lengh, (HU_MSGY+1) + 13 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coord_z_title,
		       HU_TITLEX, (HU_MSGY+1) + 14 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coord_z,
		       HU_TITLEX + w_coord_z_title_lengh, (HU_MSGY+1) + 14 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_ang_title,
		       HU_TITLEX, (HU_MSGY+1) + 15 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_ang,
		       HU_TITLEX + w_ang_title_lengh, (HU_MSGY+1) + 15 * 8,
		       english_language ? hu_font : hu_font_small_rus,
		       HU_FONTSTART);

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

    while (*s)
    HUlib_addCharToTextLine(&w_title, *(s++));

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
    static char str[32], *s;
    int time = leveltime / TICRATE;
    int totaltime = (totalleveltimes / TICRATE) + (leveltime / TICRATE);
    int extrakills = players[consoleplayer].killcount - totalkills;

    HUlib_drawSText(&w_message, msg_pickup);
    HUlib_drawSText(&w_message_secret, msg_secret);
    HUlib_drawSText(&w_message_system, msg_system);
    HUlib_drawSText(&w_message_chat, msg_chat);

    HUlib_drawIText(&w_chat);

    // [JN] Level name (in automap only)
    if (automapactive)
    {
        HUlib_drawTextLine(&w_title, false, hud_level);
    }

    // [JN] Show level stats in automap (from Crispy Doom).
    if (((automapactive && automap_stats == 1) || automap_stats == 2) && !vanillaparm)
    {
        //
        // Kills
        //
        sprintf(str, english_language ? "Kills:" : "dhfub:");
        HUlib_clearTextLine(&w_kills_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_kills_title, *(s++));
        }
        HUlib_drawTextLine(&w_kills_title, false, hud_stats);

        // Counter
        if (players[consoleplayer].killcount <= totalkills)
        {
            sprintf(str, english_language ? "%d/%d" : "%d*%d",
                    players[consoleplayer].killcount, totalkills);
        }
        else
        {
            sprintf(str, english_language ? "%d/%d+%d" : "%d*%d+%d",
                    players[consoleplayer].killcount, totalkills, extrakills);
        }
        HUlib_clearTextLine(&w_kills);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_kills, *(s++));
        }
        HUlib_drawTextLine(&w_kills, false, hud_values);

        //
        // Items
        //
        sprintf(str, english_language ? "Items:" : "ghtlvtns:");
        HUlib_clearTextLine(&w_items_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_items_title, *(s++));
        }
        HUlib_drawTextLine(&w_items_title, false, hud_stats);

        // Counter
        sprintf(str, english_language ? "%d/%d" : "%d*%d",
                players[consoleplayer].itemcount, totalitems);
        HUlib_clearTextLine(&w_items);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_items, *(s++));
        }
        HUlib_drawTextLine(&w_items, false, hud_values);

        //
        // Secrets
        //
        sprintf(str, english_language ? "Secret:" : "nfqybrb:");
        HUlib_clearTextLine(&w_scrts_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_scrts_title, *(s++));
        }
        HUlib_drawTextLine(&w_scrts_title, false, hud_stats);

        // Counter
        sprintf(str, english_language ? "%d/%d" : "%d*%d",
                players[consoleplayer].secretcount, totalsecret);
        HUlib_clearTextLine(&w_scrts);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_scrts, *(s++));
        }
        HUlib_drawTextLine(&w_scrts, false, hud_values);

        //
        // Skill
        //
        sprintf(str, english_language ? "Skill:" : "ckj;yjcnm:");
        HUlib_clearTextLine(&w_skill_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_skill_title, *(s++));
        }
        HUlib_drawTextLine(&w_skill_title, false, hud_stats);

        // Indicator
        sprintf(str, "%d", gameskill+1);
        HUlib_clearTextLine(&w_skill);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_skill, *(s++));
        }
        HUlib_drawTextLine(&w_skill, false, hud_values);
    }

    //
    // Level time
    //
    if (((automapactive && automap_level_time == 1) || automap_level_time == 2) && !vanillaparm)
    {
        sprintf(str, english_language ? "Level" : "ehjdtym"); // Уровень
        HUlib_clearTextLine(&w_ltime_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_ltime_title, *(s++));
        }
        HUlib_drawTextLine(&w_ltime_title, false, hud_stats);

        // Indicator
        sprintf(str, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);
        HUlib_clearTextLine(&w_ltime);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_ltime, *(s++));
        }
        HUlib_drawTextLine(&w_ltime, false, hud_values);
    }

    //
    // Total time
    //
    if (((automapactive && automap_total_time == 1) || automap_total_time == 2) && !vanillaparm)
    {
        sprintf(str, english_language ? "Total" : "j,ott"); // Общее
        HUlib_clearTextLine(&w_ttime_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_ttime_title, *(s++));
        }
        HUlib_drawTextLine(&w_ttime_title, false, hud_stats);

        // Indicator
        sprintf(str, "%02d:%02d:%02d", totaltime/3600, (totaltime%3600)/60, totaltime%60);
        HUlib_clearTextLine(&w_ttime);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_ttime, *(s++));
        }
        HUlib_drawTextLine(&w_ttime, false, hud_values);
    }

    //
    // Player coords
    //
    if (((automapactive && automap_coords == 1) || automap_coords == 2) && !vanillaparm)
    {
        // X:
        sprintf(str, "X:");
        HUlib_clearTextLine(&w_coord_x_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_coord_x_title, *(s++));
        }
        HUlib_drawTextLine(&w_coord_x_title, false, hud_coords);

        // Indicator
        sprintf(str, "%d", players[consoleplayer].mo->x >> FRACBITS);
        HUlib_clearTextLine(&w_coord_x);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_coord_x, *(s++));
        }
        HUlib_drawTextLine(&w_coord_x, false, hud_values);

        // Y:
        sprintf(str, "Y:");
        HUlib_clearTextLine(&w_coord_y_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_coord_y_title, *(s++));
        }
        HUlib_drawTextLine(&w_coord_y_title, false, hud_coords);

        // Indicator
        sprintf(str, "%d", players[consoleplayer].mo->y >> FRACBITS);
        HUlib_clearTextLine(&w_coord_y);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_coord_y, *(s++));
        }
        HUlib_drawTextLine(&w_coord_y, false, hud_values);

        // Z:
        sprintf(str, "Z:");
        HUlib_clearTextLine(&w_coord_z_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_coord_z_title, *(s++));
        }
        HUlib_drawTextLine(&w_coord_z_title, false, hud_coords);

        // Indicator
        sprintf(str, "%d", players[consoleplayer].mo->z >> FRACBITS);
        HUlib_clearTextLine(&w_coord_z);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_coord_z, *(s++));
        }
        HUlib_drawTextLine(&w_coord_z, false, hud_values);

        // Angle
        sprintf(str, english_language ? "ANG:" : "EUJK:");
        HUlib_clearTextLine(&w_ang_title);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_ang_title, *(s++));
        }
        HUlib_drawTextLine(&w_ang_title, false, hud_coords);

        // Indicator
        sprintf(str, "%d", players[consoleplayer].mo->angle / ANG1);
        HUlib_clearTextLine(&w_ang);
        s = str;
        while (*s)
        {
            HUlib_addCharToTextLine(&w_ang, *(s++));
        }
        HUlib_drawTextLine(&w_ang, false, hud_values);
    }

    // [JN] Draw crosshair. 
    // Thanks to Fabian Greffrath for ORIGWIDTH, ORIGHEIGHT and ST_HEIGHT values,
    // thanks to Zodomaniac for proper health values!
    if (crosshair_draw && !automapactive && !vanillaparm)
    {
        static int missilerange = 32*64*FRACUNIT; // [JN] MISSILERANGE
        extern fixed_t  P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance);
        extern mobj_t  *linetarget; // who got hit (or NULL)

        if (crosshair_type == 1)
        {
            dp_translation = plr->health >= 67 ? cr[CR_GREEN] :
                             plr->health >= 34 ? cr[CR_YELLOW] :
                                                 cr[CR_RED];
        }
        else if (crosshair_type == 2)
        {
            P_AimLineAttack(plr->mo, plr->mo->angle, missilerange);

            if (linetarget)
            dp_translation = cr[CR_GRAY];
        }
        else if (crosshair_type == 3)
        {
            dp_translation = plr->health >= 67 ? cr[CR_GREEN] :
                             plr->health >= 34 ? cr[CR_YELLOW] :
                                                 cr[CR_RED];

            P_AimLineAttack(plr->mo, plr->mo->angle, missilerange);

            if (linetarget)
            dp_translation = cr[CR_GRAY];
        }

        if (crosshair_scale)
        {
            V_DrawPatch(origwidth/2, ((screenblocks <= 10) ?
                (ORIGHEIGHT-ST_HEIGHT+2)/2 : (ORIGHEIGHT+2)/2),
                    W_CacheLumpName(DEH_String("XHAIR_1S"), PU_CACHE));
        }
        else
        {
            V_DrawPatchUnscaled(screenwidth/2, ((screenblocks <= 10) ?
                (SCREENHEIGHT-ST_HEIGHT-26)/2 : (SCREENHEIGHT+4)/2),
                    W_CacheLumpName(DEH_String("XHAIR_1U"), PU_CACHE), NULL);
        }

        dp_translation = NULL;
    }

    // [crispy] demo timer widget
    if (demoplayback && (demotimer == 1 || demotimer == 3) && !vanillaparm)
    {
        ST_DrawDemoTimer(demotimerdir ? (deftotaldemotics - defdemotics) : defdemotics);
    }
    else
    if (demorecording && (demotimer == 2 || demotimer == 3) && !vanillaparm)
    {
        ST_DrawDemoTimer(leveltime);
    }

    // [crispy] demo progress bar
    if (demoplayback && demobar && !vanillaparm)
    {
        HU_DemoProgressBar();
    }
}


void HU_Erase(void)
{
    HUlib_eraseSText(&w_message);
    HUlib_eraseSText(&w_message_secret);
    HUlib_eraseSText(&w_message_system);
    HUlib_eraseSText(&w_message_chat);
    HUlib_eraseIText(&w_chat);
    HUlib_eraseTextLine(&w_title);
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

