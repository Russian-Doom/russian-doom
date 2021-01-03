//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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
#include "z_zone.h"
#include "hu_stuff.h"
#include "hu_lib.h"
#include "w_wad.h"
#include "s_sound.h"
#include "doomstat.h"
#include "v_video.h"    // [JN] V_DrawPatchDirect

// Data.
#include "sounds.h"

#include "r_main.h"
#include "v_trans.h"
#include "rd_lang.h"
#include "jn.h"

//
// Locally used constants, shortcuts.
//

// DOOM 1 map names
#define HU_TITLE        (mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE_RUS    (mapnames_rus[(gameepisode-1)*9+gamemap-1])

// DOOM 2 map names
#define HU_TITLE2       (mapnames2[gamemap-1])
#define HU_TITLE2_RUS   (mapnames2_rus[gamemap-1])

// Plutonia map names
#define HU_TITLEP       (mapnamesp[gamemap-1])
#define HU_TITLEP_RUS   (mapnamesp_rus[gamemap-1])

// TNT map names
#define HU_TITLET       (mapnamest[gamemap-1])
#define HU_TITLET_RUS   (mapnamest_rus[gamemap-1])

#define HU_TITLEHEIGHT  1
#define HU_TITLEX       0

// [JN] Previosly: 167. 
// Moved 2 px up, for prevent overlaying chars "ô" and "ñ" to HUD (also their shadows)
#define HU_TITLEY       (165 - SHORT(hu_font[0]->height))

#define HU_INPUTTOGGLE  't'
#define HU_INPUTX       HU_MSGX
#define HU_INPUTY       (HU_MSGY + HU_MSGHEIGHT*(SHORT(hu_font[0]->height) +1))
#define HU_INPUTWIDTH   64
#define HU_INPUTHEIGHT  1


char* chat_macros[] =
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


char            chat_char; // remove later.
static          player_t* plr;
patch_t*        hu_font[HU_FONTSIZE];
patch_t*        hu_font_small_eng[HU_FONTSIZE]; // [JN] Small, unchangeable English font (FNTSE)
patch_t*        hu_font_small_rus[HU_FONTSIZE]; // [JN] Small, unchangeable Russian font (FNTSR)
patch_t*        hu_font_big_eng[HU_FONTSIZE2];  // [JN] Big, unchangeable English font (FNTBE)
patch_t*        hu_font_big_rus[HU_FONTSIZE2];  // [JN] Big, unchangeable Russian font (FNTBR)

static          hu_textline_t w_title;
static          hu_textline_t w_kills;
static          hu_textline_t w_items;
static          hu_textline_t w_scrts;
static          hu_textline_t w_skill;
static          hu_textline_t w_ltime;
static          hu_textline_t w_fps;        // [JN] FPS widget
static          hu_textline_t w_loctime;    // [JN] Local time widget
boolean         chat_on;
static          hu_itext_t w_chat;
static boolean  always_off = false;
static char     chat_dest[MAXPLAYERS];
static          hu_itext_t w_inputbuffer[MAXPLAYERS];

static boolean  message_on;         // [JN] Item pickup
static boolean  message_on_secret;  // [JN] Revealed secret
static boolean  message_on_system;  // [JN] System messages
static boolean  message_on_chat;    // [JN] Netgame chat
boolean         message_dontfuckwithme;
static boolean  message_nottobefuckedwith;

static hu_stext_t   w_message;        // [JN] Item pickup
static hu_stext_t   w_message_secret; // [JN] Revealed secret
static hu_stext_t   w_message_system; // [JN] System messages
static hu_stext_t   w_message_chat;   // [JN] Netgame chat
static int          message_counter;

extern int      showMessages;

static boolean  headsupactive = false;

//
// Builtin map names.
// The actual names can be found in DStrings.h.
//

char* mapnames[] =	// DOOM shareware/registered/retail (Ultimate) names.
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

    // [JN] "ÕŒ¬€… ”–Œ¬≈Õ‹"
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

char* mapnames2[] = // DOOM 2 map names.
{
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
    HUSTR_32
};

char* mapnames2_rus[] = // [JN] Russian DOOM 2 map names.
{
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
    HUSTR_32_RUS
};

char* mapnamesp[] = // Plutonia WAD map names.
{
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
    PHUSTR_32
};

char* mapnamesp_rus[] = // [JN] Russian Plutonia WAD map names.
{
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
    PHUSTR_32_RUS
};

char *mapnamest[] = // TNT WAD map names.
{
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
    THUSTR_32
};

char *mapnamest_rus[] = // TNT WAD map names.
{
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
    THUSTR_32_RUS
};

const char* shiftxform;

const char english_shiftxform[] =
{
    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31,
    ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};


void HU_Init(void)
{
    int	    i;
    int	    j;
    int     o, p;
    int     q, r;
    char    buffer[9];

    shiftxform = english_shiftxform;

    // load the heads-up font
    j = HU_FONTSTART;
    // [JN] Load system fonts
    o = HU_FONTSTART;
    p = HU_FONTSTART;
    q = HU_FONTSTART2;
    r = HU_FONTSTART2;

    // [JN] Standard STCFN font
    for (i=0;i<HU_FONTSIZE;i++)
    {
        sprintf(buffer, "STCFN%.3d", j++);
        hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Small, unchangeable English font (FNTSE)
    for (i=0;i<HU_FONTSIZE;i++)
    {
        sprintf(buffer, "FNTSE%.3d", o++);
        hu_font_small_eng[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Small, unchangeable Russian font (FNTSR)
    for (i=0;i<HU_FONTSIZE;i++)
    {
        sprintf(buffer, "FNTSR%.3d", p++);
        hu_font_small_rus[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Big, unchangeable English font (FNTBE)
    for (i=0;i<HU_FONTSIZE2;i++)
    {
        sprintf(buffer, "FNTBE%.3d", q++);
        hu_font_big_eng[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

    // [JN] Big, unchangeable Russian font (FNTBR)
    for (i=0;i<HU_FONTSIZE2;i++)
    {
        sprintf(buffer, "FNTBR%.3d", r++);
        hu_font_big_rus[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
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

    // [JN] Create the local time widget
    HUlib_initTextLine(&w_loctime,
                      (local_time == 1 ? 268 :
                       local_time == 2 ? 248 :
                       local_time == 3 ? 281 :
                                         261),
                       (HU_MSGY+1) + 1 * 8,
                       hu_font, HU_FONTSTART);

    // [JN] Create the FPS widget
    HUlib_initTextLine(&w_fps,
                        SCREENWIDTH-66, (HU_MSGY+1) + 2 * 8,
                        hu_font,
                        HU_FONTSTART);

    // create the map title widget
    if (!vanilla)
    HUlib_initTextLine(&w_title,HU_TITLEX, HU_TITLEY,
                       english_language ? hu_font : hu_font_small_rus,
                       HU_FONTSTART);
    else
    HUlib_initTextLine(&w_title,HU_TITLEX, HU_TITLEY+1,
                       english_language ? hu_font : hu_font_small_rus,
                       HU_FONTSTART);

    HUlib_initTextLine(&w_kills,
                        HU_TITLEX, (HU_MSGY+1) + 1 * 8,
                        english_language ? hu_font : hu_font_small_rus,
                        HU_FONTSTART);
    
    HUlib_initTextLine(&w_items,
                        HU_TITLEX, (HU_MSGY+1) + 2 * 8,
                        english_language ? hu_font : hu_font_small_rus,
                        HU_FONTSTART);
    
    HUlib_initTextLine(&w_scrts,
                        HU_TITLEX, (HU_MSGY+1) + 3 * 8,
                        english_language ? hu_font : hu_font_small_rus,
                        HU_FONTSTART);
    
    HUlib_initTextLine(&w_skill,
		                HU_TITLEX, (HU_MSGY+1) + 4 * 8,
		                english_language ? hu_font : hu_font_small_rus,
		                HU_FONTSTART);

    HUlib_initTextLine(&w_ltime,
                        HU_TITLEX, (HU_MSGY+1) + 6 * 8,
                        hu_font,
                        HU_FONTSTART);

    if (commercial)
    {
        if (english_language)
        {
            s = plutonia ? HU_TITLEP :
                     tnt ? HU_TITLET :
                           HU_TITLE2 ;
        }
        else
        {
            s = plutonia ? HU_TITLEP_RUS :
                     tnt ? HU_TITLET_RUS :
                           HU_TITLE2_RUS ;
        }
    }
    else
    {
        s = english_language ? HU_TITLE : HU_TITLE_RUS;
    }

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

void HU_Drawer(void)
{
    // [JN] Don't draw HUD stuff while in help screens.
    if (inhelpscreens)
    return;
    
    HUlib_drawSText(&w_message);
    HUlib_drawSText_Secret(&w_message_secret);
    HUlib_drawSText_System(&w_message_system);
    HUlib_drawSText_Chat(&w_message_chat);
    HUlib_drawIText(&w_chat);

    // [JN] Local time widget, DOS-friendly version.
    if (local_time && !vanilla)
    {
        time_t      rawtime;
        struct tm  *timeinfo;
        static char printtime[64], *t;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        // 12-hour (HH:MM designation)
        if (local_time == 1)
        {
            if (timeinfo->tm_hour < 12)
            {
                snprintf(printtime, sizeof(printtime), "AM %d:%02d",
                        timeinfo->tm_hour / 2, timeinfo->tm_min);
            }
            else
            {
                snprintf(printtime, sizeof(printtime), "PM %d:%02d",
                        timeinfo->tm_hour - 12, timeinfo->tm_min);
            }
        }

        // 12-hour (HH:MM:SS designation)
        if (local_time == 2)
        {
            if (timeinfo->tm_hour < 12)
            {
                snprintf(printtime, sizeof(printtime), "AM %d:%02d:%02d",
                        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
            }
            else
            {
                snprintf(printtime, sizeof(printtime), "PM %d:%02d:%02d",
                        timeinfo->tm_hour - 12, timeinfo->tm_min, timeinfo->tm_sec);
            }

        }

        // 24-hour (HH:MM)
        if (local_time == 3)
        {
            snprintf(printtime, sizeof(printtime), "%02d:%02d",
                    timeinfo->tm_hour, timeinfo->tm_min);
        }

        // 24-hour (HH:MM:SS)
        if (local_time == 4)
        {
            snprintf(printtime, sizeof(printtime), "%02d:%02d:%02d",
                    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        }

        HUlib_clearTextLine(&w_loctime);
        t = printtime;
        while (*t)
        {
            HUlib_addCharToTextLine(&w_loctime, *(t++));
        }
        HUlib_drawTextLineUncolored(&w_loctime);
    }

    if (show_fps && !vanilla)
    {
        static char str[32], *f;

        sprintf(str, "fps: %d", fps);
        HUlib_clearTextLine(&w_fps);
        f = str;
        while (*f)
        {
            HUlib_addCharToTextLine(&w_fps, *(f++));
        }
        HUlib_drawTextLineUncolored(&w_fps);
    }

    if (automapactive)
    {
        static char str[32], *s;
        int time = leveltime / TICRATE;
        int extrakills = players[consoleplayer].killcount - totalkills;

        HUlib_drawTextLineUncolored(&w_title);

        // [from-crispy] Show level stats in automap
        if (automap_stats && !vanilla)
        {
            // [JN] Show extra kills in case of having them
            if (players[consoleplayer].killcount <= totalkills)
            {
            sprintf(str, english_language ?
                         "Kills: %d/%d" : "dhfub: %d*%d",
                         players[consoleplayer].killcount, totalkills);
            }
            else
            {
            sprintf(str, english_language ?
                         "Kills: %d/%d+%d" : "dhfub: %d*%d+%d",
                         players[consoleplayer].killcount, totalkills, extrakills);                
            }
            HUlib_clearTextLine(&w_kills);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_kills, *(s++));
            HUlib_drawTextLineUncolored(&w_kills);

            sprintf(str, english_language ?
                         "Items: %d/%d" : "ghtlvtns: %d*%d",
                         players[consoleplayer].itemcount, totalitems);
            HUlib_clearTextLine(&w_items);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_items, *(s++));
            HUlib_drawTextLineUncolored(&w_items);

            sprintf(str, english_language ?
                         "Secret: %d/%d" : "nfqybrb: %d*%d",
                         players[consoleplayer].secretcount, totalsecret);
            HUlib_clearTextLine(&w_scrts);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_scrts, *(s++));
            HUlib_drawTextLineUncolored(&w_scrts);

            sprintf(str, english_language ?
                         "Skill: %d" : "ckj;yjcnm: %d",
                         gameskill+1);
            HUlib_clearTextLine(&w_skill);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_skill, *(s++));
            HUlib_drawTextLineUncolored(&w_skill);

            sprintf(str, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);
            HUlib_clearTextLine(&w_ltime);
            s = str;
            while (*s)
                HUlib_addCharToTextLine(&w_ltime, *(s++));
            HUlib_drawTextLineUncolored(&w_ltime);
        }
    }

    // [JN] Draw crosshair
    if (!vanilla && !automapactive && crosshair_draw)
    {
        if (crosshair_health)
        {
            if (plr->health >= 67)
            dp_translation = cr[CR_GREEN];
            else if (plr->health >= 34)
            dp_translation = cr[CR_GOLD];
            else
            dp_translation = cr[CR_RED];
            
        }

        V_DrawPatchDirect(SCREENWIDTH/2,
                ((screenblocks <= 10) ? (SCREENHEIGHT-32+4)/2 : (SCREENHEIGHT+4)/2), 0,
                W_CacheLumpName("XHAIRSR", PU_CACHE));

        if (crosshair_health)
            dp_translation = NULL;
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
    HUlib_eraseTextLine(&w_fps);
    HUlib_eraseTextLine(&w_loctime);
}

void HU_Ticker(void)
{
    int     i, rc;
    char    c;

    // tick down message counter if message is up
    if (message_counter && !--message_counter)
    {
        message_on = false;
        message_on_secret = false;
        message_on_system = false;
        message_on_chat = false;
        message_nottobefuckedwith = false;
    }

    if (showMessages || message_dontfuckwithme)
    {
        // display message if necessary
        // [JN] Item pickup
        if ((plr->message && !message_nottobefuckedwith) || (plr->message && message_dontfuckwithme))
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
            message_counter = HU_MSGTIMEOUT;
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
            message_counter = HU_MSGTIMEOUT;
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
            message_counter = HU_MSGTIMEOUT;
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
            message_counter = HU_MSGTIMEOUT;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }
    } // else message_on = false;

    // check for incoming chat characters
    if (netgame)
    {
        for (i=0 ; i<MAXPLAYERS; i++)
        {
            if (!playeringame[i])
            continue;

            if (i != consoleplayer && (c = players[i].cmd.chatchar))
            {
                if (c <= HU_BROADCAST)
                chat_dest[i] = c;
                else
                {
                    if (c >= 'a' && c <= 'z')
                    c = (char) shiftxform[(unsigned char) c];
                    rc = HUlib_keyInIText(&w_inputbuffer[i], c);
                    if (rc && c == KEY_ENTER)
                    {
                        if (w_inputbuffer[i].l.len && (chat_dest[i] == consoleplayer+1 || chat_dest[i] == HU_BROADCAST))
                        {
                            HUlib_addMessageToSText(&w_message_chat, player_names[i], w_inputbuffer[i].l.l);

                            message_nottobefuckedwith = true;
                            message_on_chat = true;
                            message_counter = HU_MSGTIMEOUT;
                            if ( commercial )
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

#define QUEUESIZE   128

static char chatchars[QUEUESIZE];
static int  head = 0;
static int  tail = 0;


void HU_queueChatChar(char c)
{
    if (((head + 1) & (QUEUESIZE-1)) == tail)
    {
        plr->message = HUSTR_MSGU;
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

boolean HU_Responder(event_t *ev)
{
    static char     lastmessage[HU_MAXLINELENGTH+1];
    char*           macromessage;
    boolean         eatkey = false;
    static boolean  shiftdown = false;
    static boolean  altdown = false;
    unsigned char   c;
    int             i;
    int             numplayers;

    static char destination_keys[MAXPLAYERS] =
    {
        HUSTR_KEYGREEN,
        HUSTR_KEYINDIGO,
        HUSTR_KEYBROWN,
        HUSTR_KEYRED
    };

    static int  num_nobrainers = 0;

    numplayers = 0;
    for (i=0 ; i<MAXPLAYERS ; i++)
    numplayers += playeringame[i];

    if (ev->data1 == KEY_RSHIFT)
    {
        shiftdown = ev->type == ev_keydown;
        return false;
    }
    else if (ev->data1 == KEY_RALT || ev->data1 == KEY_LALT)
    {
        altdown = ev->type == ev_keydown;
        return false;
    }

    if (ev->type != ev_keydown)
    return false;

    if (!chat_on)
    {
        if (ev->data1 == HU_MSGREFRESH)
        {
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            eatkey = true;
        }
        else if (netgame && ev->data1 == HU_INPUTTOGGLE)
        {
            eatkey = chat_on = true;
            HUlib_resetIText(&w_chat);
            HU_queueChatChar(HU_BROADCAST);
        }
        else if (netgame && numplayers > 2)
        {
            for (i=0; i<MAXPLAYERS ; i++)
            {
                if (ev->data1 == destination_keys[i])
                {
                    if (playeringame[i] && i!=consoleplayer)
                    {
                        eatkey = chat_on = true;
                        HUlib_resetIText(&w_chat);
                        HU_queueChatChar(i+1);
                        break;
                    }
                    else if (i == consoleplayer)
                    {
                        num_nobrainers++;
                        if (num_nobrainers < 3)
                        plr->message_system = HUSTR_TALKTOSELF1;
                        else if (num_nobrainers < 6)
                        plr->message_system = HUSTR_TALKTOSELF2;
                        else if (num_nobrainers < 9)
                        plr->message_system = HUSTR_TALKTOSELF3;
                        else if (num_nobrainers < 32)
                        plr->message_system = HUSTR_TALKTOSELF4;
                        else
                        plr->message_system = HUSTR_TALKTOSELF5;
                    }
                }
            }
        }
    }
    else
    {
        c = ev->data1;
        // send a macro
        if (altdown)
        {
            c = c - '0';
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
            chat_on = false;
            strcpy(lastmessage, chat_macros[c]);
            // [JN] Do not repeat typed message because of characters problem.
            // plr->message = lastmessage;
            eatkey = true;
        }
        else
        {
            if (shiftdown || (c >= 'a' && c <= 'z'))
            c = shiftxform[c];
            eatkey = HUlib_keyInIText(&w_chat, c);
            if (eatkey)
            {
                // static unsigned char buf[20]; // DEBUG
                HU_queueChatChar(c);
    
                // sprintf(buf, "KEY: %d => %d", ev->data1, c);
                //      plr->message = buf;
            }
            if (c == KEY_ENTER)
            {
                chat_on = false;
                if (w_chat.l.len)
                {
                    strcpy(lastmessage, w_chat.l.l);
                    // [JN] Do not repeat typed message because of characters problem.
                    // plr->message = lastmessage;
                }
            }
            else if (c == KEY_ESCAPE)
            chat_on = false;
        }
    }

    return eatkey;
}

