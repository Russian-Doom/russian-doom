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
// DESCRIPTION:
//       Key definitions
//



#ifndef __DOOMKEYS__
#define __DOOMKEYS__

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW	(0x80+0x2e)
#define KEY_LEFTARROW	(0x80+0x2c)
#define KEY_UPARROW	    (0x80+0x2d)
#define KEY_DOWNARROW	(0x80+0x2f)
#define KEY_ESCAPE	27
#define KEY_ENTER	13
#define KEY_TAB		9
#define KEY_F1		(0x80+0x3b)
#define KEY_F2		(0x80+0x3c)
#define KEY_F3		(0x80+0x3d)
#define KEY_F4		(0x80+0x3e)
#define KEY_F5		(0x80+0x3f)
#define KEY_F6		(0x80+0x40)
#define KEY_F7		(0x80+0x41)
#define KEY_F8		(0x80+0x42)
#define KEY_F9		(0x80+0x43)
#define KEY_F10		(0x80+0x44)
#define KEY_F11		(0x80+0x57)
#define KEY_F12		(0x80+0x58)
#define KEY_F13     (0x80+0x5a)
#define KEY_F14     (0x80+0x5b)
#define KEY_F15     (0x80+0x5c)
#define KEY_F16     (0x80+0x5d)
#define KEY_F17     (0x80+0x5e)
#define KEY_F18     (0x80+0x5f)
#define KEY_F19     (0x80+0x61)
#define KEY_F20     (0x80+0x62)
#define KEY_F21     (0x80+0x63)
#define KEY_F22     (0x80+0x64)
#define KEY_F23     (0x80+0x65)
#define KEY_F24     (0x80+0x66)

#define KEY_BACKSPACE	0x7f
#define KEY_PAUSE	(0x80+0x7f)

#define KEY_EQUALS	0x3d
#define KEY_MINUS	0x2d

#define KEY_RSHIFT	(0x80+0x36)
#define KEY_RCTRL	(0x80+0x1d)
#define KEY_RALT	(0x80+0x38)

#define KEY_LSHIFT	(0x80+0x29)
#define KEY_LCTRL	(0x80+0x2a)
#define KEY_LALT	(0x80+0x2b)

#define KEY_APP (0x80+0x67)

// new keys:

#define KEY_CAPSLOCK    (0x80+0x3a)
#define KEY_NUMLOCK     (0x80+0x45)
#define KEY_SCRLCK      (0x80+0x46)
#define KEY_PRTSCR      (0x80+0x59)

#define KEY_HOME        (0x80+0x47)
#define KEY_END         (0x80+0x4f)
#define KEY_PGUP        (0x80+0x49)
#define KEY_PGDN        (0x80+0x51)
#define KEY_INS         (0x80+0x52)
#define KEY_DEL         (0x80+0x53)

#define KEYP_0          (0x80+0x37)
#define KEYP_1          (0x80+0x39)
#define KEYP_2          (0x80+0x48)
#define KEYP_3          (0x80+0x4a)
#define KEYP_4          (0x80+0x4b)
#define KEYP_5          (0x80+0x4c)
#define KEYP_6          (0x80+0x4d)
#define KEYP_7          (0x80+0x4e)
#define KEYP_8          (0x80+0x35)
#define KEYP_9          (0x80+0x34)

#define KEYP_DIVIDE     (0x80+0x33)
#define KEYP_PLUS       (0x80+0x32)
#define KEYP_MINUS      (0x80+0x31)
#define KEYP_MULTIPLY   (0x80+0x30)
#define KEYP_PERIOD     (0x80+0x28)
#define KEYP_EQUALS     (0x80+0x27)
#define KEYP_ENTER      (0x80+0x26)

#define SCANCODE_TO_KEYS_ARRAY {                                            \
    0,   0,   0,   0,   'a',                                  /* 0-9 */     \
    'b', 'c', 'd', 'e', 'f',                                                \
    'g', 'h', 'i', 'j', 'k',                                  /* 10-19 */   \
    'l', 'm', 'n', 'o', 'p',                                                \
    'q', 'r', 's', 't', 'u',                                  /* 20-29 */   \
    'v', 'w', 'x', 'y', 'z',                                                \
    '1', '2', '3', '4', '5',                                  /* 30-39 */   \
    '6', '7', '8', '9', '0',                                                \
    KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, ' ',       /* 40-49 */   \
    KEY_MINUS, KEY_EQUALS, '[', ']', '\\',                                  \
    0,   ';', '\'', '`', ',',                                 /* 50-59 */   \
    '.', '/', KEY_CAPSLOCK, KEY_F1, KEY_F2,                                 \
    KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7,                   /* 60-69 */   \
    KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,                              \
    KEY_PRTSCR, KEY_SCRLCK, KEY_PAUSE, KEY_INS, KEY_HOME,     /* 70-79 */   \
    KEY_PGUP, KEY_DEL, KEY_END, KEY_PGDN, KEY_RIGHTARROW,                   \
    KEY_LEFTARROW, KEY_DOWNARROW, KEY_UPARROW,                /* 80-89 */   \
    KEY_NUMLOCK, KEYP_DIVIDE,                                               \
    KEYP_MULTIPLY, KEYP_MINUS, KEYP_PLUS, KEYP_ENTER, KEYP_1,               \
    KEYP_2, KEYP_3, KEYP_4, KEYP_5, KEYP_6,                   /* 90-99 */   \
    KEYP_7, KEYP_8, KEYP_9, KEYP_0, KEYP_PERIOD,                            \
    0, KEY_APP, 0, KEYP_EQUALS, KEY_F13, KEY_F14, KEY_F15,    /* 100-109 */ \
    KEY_F16, KEY_F17, KEY_F18,                                              \
    KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24,     /* 110-115 */ \
}

// Default names for keys, to use in English or as fallback.
#define KEY_NAMES_ARRAY {                                            \
    { KEY_BACKSPACE,  "BACKSP" },   { KEY_TAB,        "TAB" },       \
    { KEY_INS,        "INS" },      { KEY_DEL,        "DEL" },       \
    { KEY_PGUP,       "PGUP" },     { KEY_PGDN,       "PGDN" },      \
    { KEY_ENTER,      "ENTER" },    { KEY_ESCAPE,     "ESC" },       \
    { KEY_F1,         "F1" },       { KEY_F2,         "F2" },        \
    { KEY_F3,         "F3" },       { KEY_F4,         "F4" },        \
    { KEY_F5,         "F5" },       { KEY_F6,         "F6" },        \
    { KEY_F7,         "F7" },       { KEY_F8,         "F8" },        \
    { KEY_F9,         "F9" },       { KEY_F10,        "F10" },       \
    { KEY_F11,        "F11" },      { KEY_F12,        "F12" },       \
    { KEY_HOME,       "HOME" },     { KEY_END,        "END" },       \
    { KEY_MINUS,      "-" },        { KEY_EQUALS,     "=" },         \
    { KEY_NUMLOCK,    "NUMLCK" },   { KEY_SCRLCK,     "SCRLCK" },    \
    { KEY_PAUSE,      "PAUSE" },    { KEY_PRTSCR,     "PRTSC" },     \
    { KEY_UPARROW,    "UP" },       { KEY_DOWNARROW,  "DOWN" },      \
    { KEY_LEFTARROW,  "LEFT" },     { KEY_RIGHTARROW, "RIGHT" },     \
    { KEY_RALT,       "ALT" },      { KEY_LALT,       "ALT" },       \
    { KEY_RSHIFT,     "SHIFT" },    { KEY_LSHIFT,     "SHIFT" },     \
    { KEY_RCTRL,      "CTRL" },     { KEY_LCTRL,      "CTRL" },      \
    { KEYP_5,         "NUM5" },     { KEY_CAPSLOCK,   "CAPS" },      \
    { ' ',            "SPACE" },                                     \
    { 'a', "A" },   { 'b', "B" },   { 'c', "C" },   { 'd', "D" },    \
    { 'e', "E" },   { 'f', "F" },   { 'g', "G" },   { 'h', "H" },    \
    { 'i', "I" },   { 'j', "J" },   { 'k', "K" },   { 'l', "L" },    \
    { 'm', "M" },   { 'n', "N" },   { 'o', "O" },   { 'p', "P" },    \
    { 'q', "Q" },   { 'r', "R" },   { 's', "S" },   { 't', "T" },    \
    { 'u', "U" },   { 'v', "V" },   { 'w', "W" },   { 'x', "X" },    \
    { 'y', "Y" },   { 'z', "Z" },   { '0', "0" },   { '1', "1" },    \
    { '2', "2" },   { '3', "3" },   { '4', "4" },   { '5', "5" },    \
    { '6', "6" },   { '7', "7" },   { '8', "8" },   { '9', "9" },    \
    { '[', "[" },   { ']', "]" },   { ';', ";" },   { '`', "`" },    \
    { ',', "," },   { '.', "." },   { '/', "/" },   { '\\', "\\" },  \
    { '\'', "\'" },                                                  \
}

#endif // __DOOMKEYS__