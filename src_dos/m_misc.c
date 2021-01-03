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
// DESCRIPTION:
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "doomdef.h"
#include "z_zone.h"
#include "w_wad.h"
#include "i_system.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "doomstat.h"
#include "m_misc.h"
#include "r_main.h"
#include "s_sound.h"
#include "rd_lang.h"
#include "jn.h"


#ifndef O_BINARY
#define O_BINARY 0
#endif


int     myargc;
char  **myargv;

int     numdefaults;
char   *defaultfile;

int     rndindex = 0;
int     prndindex = 0;
int     crndindex = 0;

extern  byte scantokey[128];

//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
int M_CheckParm (char *check)
{
    int i;

    for (i = 1;i<myargc;i++)
    {
        if (!strcasecmp(check, myargv[i]))
        {
            return i;
        }
    }

    return 0;
}


// M_CheckParmWithArgs
// [JN] Same as above, but handles given arguments.
// Function from Chocolate Doom.
int M_CheckParmWithArgs(char *check, int num_args)
{
    int i;

    for (i = 1; i < myargc - num_args; i++)
    {
        if (!strcasecmp(check, myargv[i]))
        return i;
    }

    return 0;
}


// M_StrCaseStr
// [JN] Case-insensitive version of strstr().
// Function from Chocolate Doom.
char *M_StrCaseStr(char *haystack, char *needle)
{
    unsigned int haystack_len;
    unsigned int needle_len;
    unsigned int len;
    unsigned int i;

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);

    if (haystack_len < needle_len)
    {
        return NULL;
    }

    len = haystack_len - needle_len;

    for (i = 0; i <= len; ++i)
    {
        if (!strncasecmp(haystack + i, needle, needle_len))
        {
            return haystack + i;
        }
    }

    return NULL;
}


//
// M_Random
// Returns a 0-255 number
//
unsigned char rndtable[256] = {
      0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
     74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
     95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
     52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
     25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
     94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
     80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
     24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
    28,  139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
     71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
     17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
    120, 163, 236, 249
};


// Which one is deterministic?
int P_Random (void)
{
    prndindex = (prndindex+1)&0xff;
    return rndtable[prndindex];
}

int M_Random (void)
{
    rndindex = (rndindex+1)&0xff;
    return rndtable[rndindex];
}

// [crispy] our own private random function
int Crispy_Random (void)
{
    crndindex = (crndindex+1)&0xff;
    return rndtable[crndindex];
}

void M_ClearRandom (void)
{
    rndindex = prndindex = 0;
}

void M_ClearBox (fixed_t *box)
{
    box[BOXTOP] = box[BOXRIGHT] = MININT;
    box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
}

void M_AddToBox (fixed_t *box, fixed_t x, fixed_t y)
{
    if (x<box[BOXLEFT])
    {
        box[BOXLEFT] = x;
    }
    else if (x>box[BOXRIGHT])
    {
        box[BOXRIGHT] = x;
    }
    if (y<box[BOXBOTTOM])
    {
        box[BOXBOTTOM] = y;
    }
    else if (y>box[BOXTOP])
    {
        box[BOXTOP] = y;
    }
}

//
// M_WriteFile
//
boolean M_WriteFile (char const *name, void *source, int length)
{
    int handle;
    int count;

    handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

    if (handle == -1)
    {
        return false;
    }

    count = write (handle, source, length);
    close (handle);

    if (count < length)
    {
        return false;
    }

    return true;
}


//
// M_ReadFile
//
int M_ReadFile (char const *name, byte **buffer)
{
    int     handle;
    int     count;
    int     length;
    byte   *buf;
    struct  stat fileinfo;

    handle = open (name, O_RDONLY | O_BINARY, 0666);

    if (handle == -1)
    {
	    I_Error (english_language ?
                 "Couldn't read file %s" :
                 "Невозможно прочитать файл %s", name);
    }
    if (fstat (handle,&fileinfo) == -1)
    {
	    I_Error (english_language ?
                 "Couldn't read file %s" :
                 "Невозможно прочитать файл %s", name);
    }

    length = fileinfo.st_size;
    buf = Z_Malloc (length, PU_STATIC, NULL);
    count = read (handle, buf, length);
    close (handle);

    if (count < length)
    {
	    I_Error (english_language ?
                 "Couldn't read file %s" :
                 "Невозможно прочитать файл %s", name);
    }

    *buffer = buf;
    return length;
}


//
// DEFAULTS
//
int         usemouse;
int         usejoystick;

extern int  key_right;
extern int  key_left;
extern int  key_up;
extern int  key_down;

extern int  key_strafeleft;
extern int  key_straferight;

extern int  key_fire;
extern int  key_use;
extern int  key_strafe;
extern int  key_speed;

extern int  key_mouselook;
extern int  key_crosshair;

extern int  mousebfire;
extern int  mousebstrafe;
extern int  mousebforward;

extern int  joybfire;
extern int  joybstrafe;
extern int  joybuse;
extern int  joybspeed;

extern int  viewwidth;
extern int  viewheight;

extern int  mouseSensitivity;
extern int  showMessages;

extern int  detailLevel;

extern int  showMessages;

// machine-independent sound params
extern int  numChannels;

extern int  sfxVolume;
extern int  musicVolume;
extern int  snd_SBport, snd_SBirq, snd_SBdma;
extern int  snd_Mport;

extern char*	chat_macros[];


typedef struct
{
    char  *name;
    int   *location;
    int    defaultvalue;
    int    scantranslate;       // PC scan code hack
    int    untranslated;        // lousy hack
} default_t;

#define SC_UPARROW              0x48
#define SC_DOWNARROW            0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW           0x4d
#define SC_RCTRL                0x1d
#define SC_RALT                 0x38
#define SC_RSHIFT               0x36
#define SC_SPACE                0x39
#define SC_COMMA                0x33
#define SC_PERIOD               0x34
#define SC_PAGEUP               0x49
#define SC_INSERT               0x52
#define SC_HOME                 0x47
#define SC_PAGEDOWN             0x51
#define SC_DELETE               0x53
#define SC_END                  0x4f
#define SC_ENTER                0x1c
#define SC_TILDE                0x29 // [JN] Mouselook keycode: `
#define SC_KEY_A                0x1e
#define SC_KEY_B                0x30
#define SC_KEY_C                0x2e
#define SC_KEY_D                0x20
#define SC_KEY_E                0x12
#define SC_KEY_F                0x21
#define SC_KEY_G                0x22
#define SC_KEY_H                0x23
#define SC_KEY_I                0x17
#define SC_KEY_J                0x24
#define SC_KEY_K                0x25
#define SC_KEY_L                0x26
#define SC_KEY_M                0x32
#define SC_KEY_N                0x31
#define SC_KEY_O                0x18
#define SC_KEY_P                0x19
#define SC_KEY_Q                0x10
#define SC_KEY_R                0x13
#define SC_KEY_S                0x1f
#define SC_KEY_T                0x14
#define SC_KEY_U                0x16
#define SC_KEY_V                0x2f
#define SC_KEY_W                0x11
#define SC_KEY_X                0x2d
#define SC_KEY_Y                0x15
#define SC_KEY_Z                0x2c
#define SC_BACKSPACE            0x0e

default_t	defaults[] =
{
    // Language
    {"english_language",        &english_language,      1},

    // Rendering
    {"noflats",                 &noflats,               0},
    {"show_fps",                &show_fps,              0},
    {"show_diskicon",           &show_diskicon,         1},
    {"screen_wiping",           &screen_wiping,         1},

    // Display
    {"screenblocks",            &screenblocks,         10},
    {"usegamma",                &usegamma,              4},
    {"level_brightness",        &level_brightness,      0},
    {"detaillevel",             &detailLevel,           0},
    {"local_time",              &local_time,            0},

    // Messages
    {"show_messages",           &showMessages,          1},
    {"draw_shadowed_text",      &draw_shadowed_text,    1},
    {"messages_pickup_color",   &messages_pickup_color, 0},
    {"messages_secret_color",   &messages_secret_color, 3},
    {"messages_system_color",   &messages_system_color, 0},
    {"messages_chat_color",     &messages_chat_color,   1},

    // Automap
    {"automap_color",           &automap_color,         0},
    {"automap_antialias",       &automap_antialias,     1},
    {"automap_stats",           &automap_stats,         1},
//  {"automap_overlay",         &automap_overlay,       0}, 
    {"automap_rotate",          &automap_rotate,        0},
    {"automap_grid",            &automap_grid,          0},
    {"automap_follow",          &automap_follow,        1},

    // Sound
    {"sfx_volume",              &sfxVolume,             8},
    {"music_volume",            &musicVolume,           8},
    {"snd_channels",            &numChannels,          32},

    // Sound system
    {"snd_sfxdevice",           &snd_DesiredSfxDevice,  3},
    {"snd_musicdevice",         &snd_DesiredMusicDevice,3},
    {"snd_samplerate",          &snd_samplerate,    11025},
    {"snd_monomode",            &snd_monomode,          0},
    {"snd_pitchshift",          &snd_pitchshift,        0},
    
    {"snd_sbport",              &snd_SBport,        0x220},
    {"snd_sbirq",               &snd_SBirq,             5},
    {"snd_sbdma",               &snd_SBdma,             1},
    {"snd_mport",               &snd_Mport,         0x330},

    // Control bindings & cie.
    {"key_right",               &key_right,         SC_RIGHTARROW,  1},
    {"key_left",                &key_left,          SC_LEFTARROW,   1},
    {"key_up",                  &key_up,            SC_KEY_W,       1},
    {"key_down",                &key_down,          SC_KEY_S,       1},
    {"key_strafeleft",          &key_strafeleft,    SC_KEY_A,       1},
    {"key_straferight",         &key_straferight,   SC_KEY_D,       1},

    {"key_fire",                &key_fire,          SC_RCTRL,       1},
    {"key_use",                 &key_use,           SC_KEY_E,       1},
    {"key_strafe",              &key_strafe,        SC_RALT,        1},
    {"key_speed",               &key_speed,         SC_RSHIFT,      1},
    {"key_mouselook",           &key_mouselook,     SC_TILDE,       1},
    {"key_crosshair",           &key_crosshair,     SC_KEY_X,       1},

    {"use_mouse",               &usemouse,              1},
    {"mouseb_fire",             &mousebfire,            0},
    {"mouseb_strafe",           &mousebstrafe,          1},
    {"mouseb_forward",          &mousebforward,         2},
    {"use_joystick",            &usejoystick,           0},
    {"joyb_fire",               &joybfire,              0},
    {"joyb_strafe",             &joybstrafe,            1},
    {"joyb_use",                &joybuse,               3},

    // Controls
    {"joyb_speed",              &joybspeed,            29},
    {"mouse_sensitivity",       &mouseSensitivity,      5},
    {"mlook",                   &mlook,                 0},
    {"mouse_y_invert",          &mouse_y_invert,        0},
    {"novert",                  &novert,                1},

    // Gameplay: Graphical
    {"brightmaps",              &brightmaps,            1},
    {"fake_contrast",           &fake_contrast,         0},
    {"translucency",            &translucency,          1},
    {"improved_fuzz",           &improved_fuzz,         2},
    {"colored_hud",             &colored_hud,           0},
    {"colored_blood",           &colored_blood,         1},
    {"swirling_liquids",        &swirling_liquids,      1},
    {"invul_sky",               &invul_sky,             1},
    {"flip_weapons",            &flip_weapons,          0},

    // Gameplay: Audible
    {"play_exit_sfx",           &play_exit_sfx,         0},
    {"crushed_corpses_sfx",     &crushed_corpses_sfx,   1},
    {"blazing_door_fix_sfx",    &blazing_door_fix_sfx,  1},
    {"noise_alert_sfx",         &noise_alert_sfx,       0},

    // Gameplay: Tactical
    {"secret_notification",     &secret_notification,   1},
    {"negative_health",         &negative_health,       0},

    // Gameplay: Physical
    {"over_under",              &over_under,            0},
    {"torque",                  &torque,                1},
    {"weapon_bobbing",          &weapon_bobbing,        1},
    {"ssg_blast_enemies",       &ssg_blast_enemies,     1},
    {"randomly_flipcorpses",    &randomly_flipcorpses,  1},
    {"floating_powerups",       &floating_powerups,     0},

    // Gameplay: Crosshair
    {"crosshair_draw",          &crosshair_draw,        0},
    {"crosshair_health",        &crosshair_health,      1},

    // Gameplay: Gameplay
    {"extra_player_faces",      &extra_player_faces,    1},
    {"unlimited_lost_souls",    &unlimited_lost_souls,  1},
    {"fast_quickload",          &fast_quickload,        1},
    {"no_internal_demos",       &no_internal_demos,     0},

    // Chat macroses
    {"chatmacro0", (int *) &chat_macros[0], (int) HUSTR_CHATMACRO0 },
    {"chatmacro1", (int *) &chat_macros[1], (int) HUSTR_CHATMACRO1 },
    {"chatmacro2", (int *) &chat_macros[2], (int) HUSTR_CHATMACRO2 },
    {"chatmacro3", (int *) &chat_macros[3], (int) HUSTR_CHATMACRO3 },
    {"chatmacro4", (int *) &chat_macros[4], (int) HUSTR_CHATMACRO4 },
    {"chatmacro5", (int *) &chat_macros[5], (int) HUSTR_CHATMACRO5 },
    {"chatmacro6", (int *) &chat_macros[6], (int) HUSTR_CHATMACRO6 },
    {"chatmacro7", (int *) &chat_macros[7], (int) HUSTR_CHATMACRO7 },
    {"chatmacro8", (int *) &chat_macros[8], (int) HUSTR_CHATMACRO8 },
    {"chatmacro9", (int *) &chat_macros[9], (int) HUSTR_CHATMACRO9 },
};


//
// M_SaveDefaults
//
void M_SaveDefaults (void)
{
    int    i;
    int    v;
    FILE  *f;

    f = fopen (defaultfile, "w");

    if (!f)
    {
        return; // can't write the file, but don't complain
    }

    for (i=0 ; i<numdefaults ; i++)
    {
        if (defaults[i].scantranslate)
        {
            defaults[i].location = &defaults[i].untranslated;
        }

        if (defaults[i].defaultvalue > -0xffffff
        &&  defaults[i].defaultvalue < 0xffffff)
        {
                v = *defaults[i].location;
                fprintf (f,"%s\t\t%i\n",defaults[i].name,v);
        }
        else
        {
            fprintf (f,"%s\t\t\"%s\"\n",defaults[i].name,
		     * (char **) (defaults[i].location));
        }
    }

    fclose (f);
}


//
// M_LoadDefaults
//


void M_LoadDefaults (void)
{
    int     i;
    int     len;
    FILE   *f;
    char    def[80];
    char    strparm[100];
    char   *newstring;
    int     parm;
    boolean isstring;

    // set everything to base values
    numdefaults = sizeof(defaults)/sizeof(defaults[0]);

    for (i=0 ; i<numdefaults ; i++)
    {
        *defaults[i].location = defaults[i].defaultvalue;
    }

    // check for a custom default file
    i = M_CheckParm ("-config");

    if (i && i<myargc-1)
    {
        defaultfile = myargv[i+1];
        printf (english_language ?
                "   default file: %s\n" :
                "   конфигурационный файл: %s\n", defaultfile);
    }
    else
    {
        defaultfile = basedefault;
    }

    // read the file in, overriding any set defaults
    f = fopen (defaultfile, "r");

    if (f)
    {
        while (!feof(f))
        {
            isstring = false;

            if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
            {
                if (strparm[0] == '"')
                {
                    // get a string default
                    isstring = true;
                    len = strlen(strparm);
                    newstring = (char *) malloc(len);
                    strparm[len-1] = 0;
                    strcpy(newstring, strparm+1);
                }
                else if (strparm[0] == '0' && strparm[1] == 'x')
                {
                    sscanf(strparm+2, "%x", &parm);
                }
                else
                {
                    sscanf(strparm, "%i", &parm);
                }

                for (i=0 ; i<numdefaults ; i++)
                if (!strcmp(def, defaults[i].name))
                {
                    if (!isstring)
                    {
                        *defaults[i].location = parm;
                    }
                    else
                    {
                        *defaults[i].location = (int) newstring;
                    }
                    break;
                }
            }
        }
        fclose (f);
    }

    for (i = 0; i < numdefaults; i++)
    {
        if (defaults[i].scantranslate)
        {
            parm = *defaults[i].location;
            defaults[i].untranslated = parm;
            *defaults[i].location = scantokey[parm];
        }
    }
}


//
// SCREEN SHOTS
//

typedef struct
{
    char            manufacturer;
    char            version;
    char            encoding;
    char            bits_per_pixel;

    unsigned short  xmin;
    unsigned short  ymin;
    unsigned short  xmax;
    unsigned short  ymax;

    unsigned short  hres;
    unsigned short  vres;

    unsigned char   palette[48];

    char            reserved;
    char            color_planes;
    unsigned short  bytes_per_line;
    unsigned short  palette_type;

    char            filler[58];
    unsigned char   data;		// unbounded
} pcx_t;


//
// WritePCXfile
//
void WritePCXfile (char *filename, byte *data, int width, int height, byte *palette)
{
    int     i;
    int     length;
    pcx_t  *pcx;
    byte   *pack;

    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;   // PCX id
    pcx->version = 5;           // 256 color
    pcx->encoding = 1;          // uncompressed
    pcx->bits_per_pixel = 8;    // 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;      // chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);   // not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;

    for (i=0 ; i<width*height ; i++)
    {
        if ((*data & 0xc0) != 0xc0)
        {
            *pack++ = *data++;
        }
        else
        {
            *pack++ = 0xc1;
            *pack++ = *data++;
        }
    }

    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
    *pack++ = *palette++;

    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}


//
// M_ScreenShot
//
void M_ScreenShot (void)
{
    int    i;
    byte  *linear;
    char   lbmname[12];

    // munge planar buffer to linear
    linear = screens[2];
    I_ReadScreen (linear);

    // find a file name to save it to
    strcpy(lbmname,"DOOM00.pcx");

    for (i=0 ; i<=99 ; i++)
    {
        lbmname[4] = i/10 + '0';
        lbmname[5] = i%10 + '0';
        if (access(lbmname,0) == -1)
        {
            break;	// file doesn't exist
        }
    }

    // [JN] Do not crash if limit is reached.
    // Instead, play "oof" sound and print warning.
    if (i==100)
    {
        S_StartSound(NULL, sfx_oof);
        players[consoleplayer].message_system = english_language ?
                               "unable to write a screenshot" :
                               "ytdjpvj;yj cj[hfybnm crhbyijn";
        return;
    }

    // save the pcx file
    WritePCXfile (lbmname, linear, SCREENWIDTH, SCREENHEIGHT,
                  W_CacheLumpName (usegamma <= 8 ? "PALFIX" : "PLAYPAL",PU_CACHE));

    // [JN] Play sound instead of "screenshot" message.
    S_StartSound(NULL, sfx_itemup);
}

