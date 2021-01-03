//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 1993-2008 Raven Software
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
//  IBM DOS VGA graphics and key/mouse/joystick.
//


#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <graph.h>
#include "d_main.h"
#include "doomstat.h"
#include "doomdef.h"
#include "r_local.h"
#include "sounds.h"
#include "i_system.h"
#include "i_sound.h"
#include "g_game.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "dpmiapi.h"
#include "jn.h"


#define DPMI_INT                0x31

#define SC_INDEX                0x3C4
#define SC_RESET                0
#define SC_CLOCK                1
#define SC_MAPMASK              2
#define SC_CHARMAP              3
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3D4
#define CRTC_H_TOTAL            0
#define CRTC_H_DISPEND          1
#define CRTC_H_BLANK            2
#define CRTC_H_ENDBLANK         3
#define CRTC_H_RETRACE          4
#define CRTC_H_ENDRETRACE       5
#define CRTC_V_TOTAL            6
#define CRTC_OVERFLOW           7
#define CRTC_ROWSCAN            8
#define CRTC_MAXSCANLINE        9
#define CRTC_CURSORSTART        10
#define CRTC_CURSOREND          11
#define CRTC_STARTHIGH          12
#define CRTC_STARTLOW           13
#define CRTC_CURSORHIGH         14
#define CRTC_CURSORLOW          15
#define CRTC_V_RETRACE          16
#define CRTC_V_ENDRETRACE       17
#define CRTC_V_DISPEND          18
#define CRTC_OFFSET             19
#define CRTC_UNDERLINE          20
#define CRTC_V_BLANK            21
#define CRTC_V_ENDBLANK         22
#define CRTC_MODE               23
#define CRTC_LINECOMPARE        24

#define GC_INDEX                0x3CE
#define GC_SETRESET             0
#define GC_ENABLESETRESET       1
#define GC_COLORCOMPARE         2
#define GC_DATAROTATE           3
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS        6
#define GC_COLORDONTCARE        7
#define GC_BITMASK              8

#define ATR_INDEX               0x3c0
#define ATR_MODE                16
#define ATR_OVERSCAN            17
#define ATR_COLORPLANEENABLE    18
#define ATR_PELPAN              19
#define ATR_COLORSELECT         20

#define STATUS_REGISTER_1       0x3da

#define PEL_WRITE_ADR           0x3c8
#define PEL_READ_ADR            0x3c7
#define PEL_DATA                0x3c9
#define PEL_MASK                0x3c6

#define VBLCOUNTER              34000 // hardware tics to a frame

#define TIMERINT                8
#define KEYBOARDINT             9

#define CRTCOFF                 (_inbyte(STATUS_REGISTER_1)&1)
#define CLI                     _disable()
#define STI                     _enable()

#define _outbyte(x,y)           (outp(x,y))
#define _outhword(x,y)          (outpw(x,y))
#define _inbyte(x)              (inp(x))
#define _inhword(x)             (inpw(x))

#define KEY_LSHIFT              (0xfe)
#define KEY_INS                 (0x80+0x52)
#define KEY_DEL                 (0x80+0x53)
#define KEY_PGUP                (0x80+0x49)
#define KEY_PGDN                (0x80+0x51)
#define KEY_HOME                (0x80+0x47)
#define KEY_END                 (0x80+0x4f)

#define SC_UPARROW              0x48
#define SC_DOWNARROW            0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW           0x4d
#define SC_RSHIFT               0x36
#define SC_LSHIFT               0x2a

#define MOUSEB1                 1
#define MOUSEB2                 2
#define MOUSEB3                 4

#define KBDQUESIZE              32

#define REALSTACKSIZE           1024

#define DOOMCOM_ID              0x12345678l


void I_StartupNet(void);
void I_ShutdownNet(void);
void I_ReadExternDriver(void);
void I_ReadMouse(void);
void I_InitDiskFlash(void);
void DPMIInt(int i);
void I_WaitVBL(int vbls);
void I_StartupCyberMan(void);
void I_StartupSound(void);
void I_ShutdownSound(void);
void I_ShutdownTimer(void);
void (__interrupt __far *oldkeyboardisr) () = NULL;
void I_DivException(void);
void (__interrupt __far *oldtimerisr) ();
int  I_SetDivException(void);


int         ticcount;
int         kbdtail;
int         kbdhead;
int         olddb[2][4];
int         lastpress;
int         joyxl, joyxh, joyyl, joyyh;
int         basejoyx, basejoyy;
int         fps;

union       REGS regs;          // REGS stuff used for int calls
struct      SREGS segregs;

unsigned    joystickx;
unsigned    joysticky;
unsigned    realstackseg;

extern int  usemouse, usejoystick;
extern      doomcom_t *doomcom;

byte        keyboardque[KBDQUESIZE];
byte        *pcscreen;
byte        *currentscreen;
byte        *destscreen;
byte        *destview;
byte        *I_AllocLow(int length);

boolean     grmode;
boolean     mousepresent;
boolean     novideo;            // if true, stay in text mode for debugging
boolean     joystickpresent;
boolean I_ReadJoystick (void)   // returns false if not connected
{
    return false;
}

typedef struct
{
    unsigned        edi, esi, ebp, reserved, ebx, edx, ecx, eax;
    unsigned short  flags, es, ds, fs, gs, ip, cs, sp, ss;
} dpmiregs_t;

extern      dpmiregs_t dpmiregs;
dpmiregs_t  dpmiregs;

byte scantokey[128] =
{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=',    KEY_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13 ,    KEY_RCTRL,'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	39 ,    '`',    KEY_LSHIFT,92,  'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '/',    KEY_RSHIFT,'*',
	KEY_RALT,' ',   0  ,    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,   // 3
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,0  ,    0  , KEY_HOME,
	KEY_UPARROW,KEY_PGUP,'-',KEY_LEFTARROW,'5',KEY_RIGHTARROW,'+',KEY_END, //4
	KEY_DOWNARROW,KEY_PGDN,KEY_INS,KEY_DEL,0,0,             0,              KEY_F11,
	KEY_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7
};

typedef struct
{
    int irq;
    ticcmd_t cmd;
} extapi_t;

extapi_t *extcontrol;

ticcmd_t emptycmd;

ticcmd_t* I_BaseTiccmd(void)
{
    if (!extcontrol)
    {
        return &emptycmd;
    }
    DPMIInt(extcontrol->irq);
    return &extcontrol->cmd;
}


//
// I_GetTime
// Returns time in 1/35th second tics.
//
int I_GetTime (void)
{
    return ticcount;
}


//
// I_ColorBorder
//
void I_ColorBorder (void)
{
    int i;

    I_WaitVBL(1);
    _outbyte(PEL_WRITE_ADR, 0);

    for (i = 0; i < 3; i++)
    {
        _outbyte(PEL_DATA, 63);
    }
}


//
// I_UnColorBorder
//
void I_UnColorBorder (void)
{
    int i;

    I_WaitVBL(1);
    _outbyte(PEL_WRITE_ADR, 0);

    for (i = 0; i < 3; i++)
    {
        _outbyte(PEL_DATA, 0);
    }
}


// -----------------------------------------------------------------------------
// User input
// -----------------------------------------------------------------------------

//
// I_WaitVBL
//
void I_WaitVBL (int vbls)
{
    int stat;

    if (novideo)
    {
        return;
    }

    while (vbls--)
    {
        do
        {
            stat = inp(STATUS_REGISTER_1);
            if (stat & 8)
            {
                break;
            }
        } while (1);
        do
        {
            stat = inp(STATUS_REGISTER_1);
            if ((stat & 8) == 0)
            {
                break;
            }
        } while (1);
    }
}


//
// I_SetPalette
// Palette source must use 8 bit RGB elements.
//
void I_SetPalette (byte *palette)
{
    int i;

    // [JN] Check for incorrect gamma levels at startup
    if (usegamma < 0)
        usegamma = 0;
    if (usegamma > 17)
        usegamma = 17;

	if(novideo)
	{
		return;
	}

	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);

	for(i = 0; i < 768; i++)
	{
		_outbyte(PEL_DATA, (gammatable[usegamma][*palette++])>>2);
	}
}


// -----------------------------------------------------------------------------
// Graphics mode
// -----------------------------------------------------------------------------

//
// I_UpdateBox
//
void I_UpdateBox (int x, int y, int w, int h)
{
    int      i, j, k, count;
    int      sp_x1, sp_x2;
    int      poffset;
    int      offset;
    int      pstep;
    int      step;
    byte    *dest, *source;

    if (x < 0 || y < 0 || w <= 0 || h <= 0
    ||  x + w > SCREENWIDTH || y + h > SCREENHEIGHT)
    {
        I_Error(english_language ?
                "Bad I_UpdateBox (%i, %i, %i, %i)" :
                "Ошибка I_UpdateBox (%i, %i, %i, %i)", x, y, w, h);
    }

    sp_x1 = x / 8;
    sp_x2 = (x + w) / 8;
    count = sp_x2 - sp_x1 + 1;
    offset = y * SCREENWIDTH + sp_x1 * 8;
    step = SCREENWIDTH - count * 8;
    poffset = offset / 4;
    pstep = step / 4;

    outp(SC_INDEX, SC_MAPMASK);

    for (i = 0 ; i < 4 ; i++)
    {
        outp(SC_INDEX + 1, 1 << i);
        source = &screens[0][offset + i];
        dest = destscreen + poffset;

        for (j = 0 ; j < h ; j++)
        {
            k = count;
            while (k--)
            {
                *(unsigned short *)dest = (unsigned short)
                                          (((*(source + 4)) << 8) + (*source));
                dest += 2;
                source += 8;
            }

            source += step;
            dest += pstep;
        }
    }
}


//
// I_UpdateNoBlit
//

void I_UpdateNoBlit (void)
{
    int realdr[4];
    int x, y, w, h;

    // Set current screen
    currentscreen = destscreen;

    // Update dirtybox size
    realdr[BOXTOP] = dirtybox[BOXTOP];
    if (realdr[BOXTOP] < olddb[0][BOXTOP])
    {
        realdr[BOXTOP] = olddb[0][BOXTOP];
    }
    if (realdr[BOXTOP] < olddb[1][BOXTOP])
    {
        realdr[BOXTOP] = olddb[1][BOXTOP];
    }

    realdr[BOXRIGHT] = dirtybox[BOXRIGHT];
    if (realdr[BOXRIGHT] < olddb[0][BOXRIGHT])
    {
        realdr[BOXRIGHT] = olddb[0][BOXRIGHT];
    }
    if (realdr[BOXRIGHT] < olddb[1][BOXRIGHT])
    {
        realdr[BOXRIGHT] = olddb[1][BOXRIGHT];
    }

    realdr[BOXBOTTOM] = dirtybox[BOXBOTTOM];
    if (realdr[BOXBOTTOM] > olddb[0][BOXBOTTOM])
    {
        realdr[BOXBOTTOM] = olddb[0][BOXBOTTOM];
    }
    if (realdr[BOXBOTTOM] > olddb[1][BOXBOTTOM])
    {
        realdr[BOXBOTTOM] = olddb[1][BOXBOTTOM];
    }

    realdr[BOXLEFT] = dirtybox[BOXLEFT];
    if (realdr[BOXLEFT] > olddb[0][BOXLEFT])
    {
        realdr[BOXLEFT] = olddb[0][BOXLEFT];
    }
    if (realdr[BOXLEFT] > olddb[1][BOXLEFT])
    {
        realdr[BOXLEFT] = olddb[1][BOXLEFT];
    }

    // Leave current box for next update
    memcpy(olddb[0], olddb[1], 16);
    memcpy(olddb[1], dirtybox, 16);

    // Update screen
    if (realdr[BOXBOTTOM] <= realdr[BOXTOP])
    {
        x = realdr[BOXLEFT];
        y = realdr[BOXBOTTOM];
        w = realdr[BOXRIGHT] - realdr[BOXLEFT] + 1;
        h = realdr[BOXTOP] - realdr[BOXBOTTOM] + 1;
        I_UpdateBox(x, y, w, h);
    }
    // Clear box
    M_ClearBox(dirtybox);
}


//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    int         tics;
    int         i;
    static int  lasttic;

    if (devparm)
    {
        i = ticcount;
        tics = i - lasttic;
        lasttic = i;

        if (tics > 20) tics = 20;
        outpw(SC_INDEX, 0x102);

        for (i = 0; i < tics; i++)
        {
            destscreen[(SCREENHEIGHT - 1)*SCREENWIDTH / 4 + i] = 0xff;
        }
        for (; i < 20; i++)
        {
            destscreen[(SCREENHEIGHT - 1)*SCREENWIDTH / 4 + i] = 0x0;
        }
    }
    outpw(CRTC_INDEX, ((int)destscreen & 0xff00) + 0xc);

    //Next plane
    destscreen += 0x4000;

    if (destscreen == (byte*)0xac000)
    {
        destscreen = (byte*)0xa0000;
    }

	// [JN] Real FPS counter.
    // Adapted from Doom 'MBF' for DOS, Maintenance release 2.04
    // https://www.vogons.org/viewtopic.php?f=24&t=40857
	{
        static int fps_counter, fps_starttime, fps_nextcalculation;
        int time = I_GetTime(); // I_GetTime_RealTime(); same result

        if (fps_counter==0)
        {    
            fps_starttime = I_GetTime();
        }

        fps_counter++;

        // store a value and/or draw when data is ok:
        if (fps_counter>(TICRATE+10)) 
        {
            // in case of a very fast system, this will limit the sampling
            if (fps_nextcalculation<time)
            {
                // minus 1!, exactly 35 FPS when measeraring for a longer time.
                fps=(double)((fps_counter-1)*TICRATE)/(time-fps_starttime);
                fps_nextcalculation=time+12; 
                if (fps>9999) fps=9999; // overflow
                fps_counter=0; // flush old data
            }
	   }
	}
}


//
// I_InitGraphics
//
void I_InitGraphics (void)
{
    if (novideo)
    {
        return;
    }

    grmode = true;
    regs.w.ax = 0x13;
    int386(0x10, (union REGS *)&regs, &regs);
    pcscreen = currentscreen = (byte *)0xa0000;
    destscreen = (byte *)0xa4000;

    outp(SC_INDEX, SC_MEMMODE);
    outp(SC_INDEX + 1, (inp(SC_INDEX + 1)&~8) | 4);
    outp(GC_INDEX, GC_MODE);
    outp(GC_INDEX + 1, inp(GC_INDEX + 1)&~0x13);
    outp(GC_INDEX, GC_MISCELLANEOUS);
    outp(GC_INDEX + 1, inp(GC_INDEX + 1)&~2);
    outpw(SC_INDEX, 0xf02);
    memset(pcscreen, 0, 0x10000);
    outp(CRTC_INDEX, CRTC_UNDERLINE);
    outp(CRTC_INDEX + 1, inp(CRTC_INDEX + 1)&~0x40);
    outp(CRTC_INDEX, CRTC_MODE);
    outp(CRTC_INDEX + 1, inp(CRTC_INDEX + 1) | 0x40);
    outp(GC_INDEX, GC_READMAP);

    I_SetPalette(W_CacheLumpName(usegamma <= 8 ? 
                                 "PALFIX" : "PLAYPAL", PU_CACHE));
    I_InitDiskFlash();
}


//
// I_ShutdownGraphics
//
void I_ShutdownGraphics (void)
{
    if (*(byte *)0x449 == 0x13)     // don't reset mode if it didn't get set
    {
        regs.w.ax = 3;
        int386(0x10, &regs, &regs); // back to text mode
    }
}


//
// I_ReadScreen
// Reads the screen currently displayed into a linear buffer.
//
void I_ReadScreen (byte *scr)
{
    int i;
    int j;

    outp(GC_INDEX, GC_READMAP);

    for (i = 0 ; i < 4 ; i++)
    {
        outp(GC_INDEX+1, i);
        for (j = 0; j < SCREENWIDTH*SCREENHEIGHT/4; j++)
        {
            scr[i+j*4] = currentscreen[j];
        }
    }
}


//
// I_StartTic
//
// called by D_DoomLoop
// called before processing each tic in a frame
// can call D_PostEvent
// asyncronous interrupt functions should maintain private ques that are
// read by the syncronous functions to be converted into events
//
void I_StartTic (void)
{
    int      k;
    event_t  ev;

    I_ReadMouse();

    // keyboard events
    while (kbdtail < kbdhead)
    {
        k = keyboardque[kbdtail&(KBDQUESIZE - 1)];
        kbdtail++;

        // extended keyboard shift key bullshit
        if ((k & 0x7f) == SC_LSHIFT || (k & 0x7f) == SC_RSHIFT)
        {
            if (keyboardque[(kbdtail - 2)&(KBDQUESIZE - 1)] == 0xe0)
            {
                continue;
            }
            k &= 0x80;
            k |= SC_RSHIFT;
        }

        if (k == 0xe0)
        {
            continue;   // special / pause keys
        }
        if (keyboardque[(kbdtail - 2)&(KBDQUESIZE - 1)] == 0xe1)
        {
            continue;   // pause key bullshit
        }
        if (k == 0xc5 && keyboardque[(kbdtail - 2)&(KBDQUESIZE - 1)] == 0x9d)
        {
            ev.type = ev_keydown;
            ev.data1 = KEY_PAUSE;
            D_PostEvent(&ev);
            continue;
        }

        if (k & 0x80)
            ev.type = ev_keyup;
        else
            ev.type = ev_keydown;
        k &= 0x7f;
        switch (k)
        {
        case SC_UPARROW:
            ev.data1 = KEY_UPARROW;
            break;
        case SC_DOWNARROW:
            ev.data1 = KEY_DOWNARROW;
            break;
        case SC_LEFTARROW:
            ev.data1 = KEY_LEFTARROW;
            break;
        case SC_RIGHTARROW:
            ev.data1 = KEY_RIGHTARROW;
            break;
        default:
            ev.data1 = scantokey[k];
            break;
        }
        D_PostEvent(&ev);
    }
}

void I_ReadKeys (void)
{
    int k;

    while (1)
    {
        while (kbdtail < kbdhead)
        {
            k = keyboardque[kbdtail&(KBDQUESIZE - 1)];
            kbdtail++;
            printf("0x%x\n", k);
            if (k == 1)
            {
                I_Quit();
            }
        }
    }
}


void I_ColorBlack(int r, int g, int b)
{
    _outbyte(PEL_WRITE_ADR, 0);
    _outbyte(PEL_DATA, r);
    _outbyte(PEL_DATA, g);
    _outbyte(PEL_DATA, b);
}


// -----------------------------------------------------------------------------
// Timer interrupt
// -----------------------------------------------------------------------------

//
// I_TimerISR
//
int I_TimerISR (void)
{
    ticcount++;
    return 0;
}


// -----------------------------------------------------------------------------
// Keyboard
// -----------------------------------------------------------------------------

//
// I_KeyboardISR
//
void __interrupt I_KeyboardISR (void)
{
    // Get the scan code
    keyboardque[kbdhead&(KBDQUESIZE - 1)] = lastpress = _inbyte(0x60);
    kbdhead++;

    // acknowledge the interrupt
    _outbyte(0x20, 0x20);
}


//
// I_StartupKeyboard
//
void I_StartupKeyboard (void)
{
    oldkeyboardisr = _dos_getvect(KEYBOARDINT);
    _dos_setvect (0x8000 | KEYBOARDINT, I_KeyboardISR);
}


void I_ShutdownKeyboard (void)
{
    if (oldkeyboardisr)
    {
		_dos_setvect (KEYBOARDINT, oldkeyboardisr);
    }
	*(short *)0x41c = *(short *)0x41a;      // clear bios key buffer
}


// -----------------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------------

int I_ResetMouse (void)
{
    regs.w.ax = 0; // reset
    int386 (0x33, &regs, &regs);
    return regs.w.ax;
}


//
// StartupMouse
//
void I_StartupMouse (void)
{
    // General mouse detection
    mousepresent = 0;
    if (M_CheckParm("-nomouse") || !usemouse)
    {
        return;
    }

    if (I_ResetMouse() != 0xffff)
    {
        printf(english_language ?
               "Mouse: not present\n" :
               "Мышь отсутствует\n", 0);
        return;
    }
    printf(english_language ?
           "Mouse: detected\n" :
           "Мышь обнаружена\n", 0);

    mousepresent = 1;

    I_StartupCyberMan();
}


//
// ShutdownMouse
//
void I_ShutdownMouse (void)
{
    if (!mousepresent)
    {
        return;
    }

    I_ResetMouse();
}


//
// I_ReadMouse
//
void I_ReadMouse(void)
{
    event_t ev;

    // mouse events
    if (!mousepresent)
    {
        return;
    }

    ev.type = ev_mouse;

    memset(&dpmiregs, 0, sizeof(dpmiregs));
    dpmiregs.eax = 3;   // read buttons / position
    DPMIInt(0x33);
    ev.data1 = dpmiregs.ebx;

    dpmiregs.eax = 11;  // read counters
    DPMIInt(0x33);
    ev.data2 = (short)dpmiregs.ecx;
    ev.data3 = -(short)dpmiregs.edx;

    D_PostEvent(&ev);
}


// -----------------------------------------------------------------------------
// Joystick
// -----------------------------------------------------------------------------

boolean WaitJoyButton(void)
{
    int oldbuttons, buttons;

    oldbuttons = 0;
    do
    {
        I_WaitVBL(1);
        buttons = ((inp(0x201) >> 4) & 1) ^ 1;
        if (buttons != oldbuttons)
        {
            oldbuttons = buttons;
            continue;
        }

        if ((lastpress & 0x7f) == 1)
        {
            joystickpresent = false;
            return false;
        }
    } while (!buttons);

    do
    {
        I_WaitVBL(1);
        buttons = ((inp(0x201) >> 4) & 1) ^ 1;
        if (buttons != oldbuttons)
        {
            oldbuttons = buttons;
            continue;
        }

        if ((lastpress & 0x7f) == 1)
        {
            joystickpresent = false;
            return false;
        }
    } while (buttons);

    return true;
}


//
// I_StartupJoystick
//
void I_StartupJoystick (void)
{
    int centerx, centery;

    joystickpresent = 0;

    if (M_CheckParm("-nojoy") || !usejoystick)
    {
        return;
    }

    if (!I_ReadJoystick())
    {
        joystickpresent = false;
        printf(english_language ?
               "Joystick: not found\n" :
               "Джойстик не обнаружен\n");
        return;
    }
    printf(english_language ?
           "Joystick: found\n" :
           "Джойстик обнаружен\n");
    joystickpresent = true;

    printf(english_language ?
    "CENTER the joystick and press button 1:" :
    "Отцентрируйте джойстик и нажмите кнопку 1:");
    if (!WaitJoyButton())
    {
        return;
    }
    I_ReadJoystick();
    centerx = joystickx;
    centery = joysticky;

    printf(english_language ?
    "\nPush the joystick to the UPPER LEFT corner and press button 1:" :
    "\nПереведите джойстик в верхнее левое положение и нажмите кнопку 1:");
    if (!WaitJoyButton())
    {
        return;
    }
    I_ReadJoystick();
    joyxl = (centerx + joystickx) / 2;
    joyyl = (centerx + joysticky) / 2;

    printf(english_language ?
    "\nPush the joystick to the LOWER RIGHT corner and press button 1:" :
    "\nПереведите джойстик в нижнее правое положение и нажмите кнопку 1:");
    if (!WaitJoyButton())
    {
        return;
    }
    I_ReadJoystick();
    joyxh = (centerx + joystickx) / 2;
    joyyh = (centery + joysticky) / 2;
    printf("\n");
}


//
// I_StartFrame
//
void I_StartFrame(void)
{
    event_t ev;

    // joystick events
    if (!joystickpresent)
    {
        return;
    }

    I_ReadJoystick();
    ev.type = ev_joystick;
    ev.data1 = ((inp(0x201) >> 4) & 15) ^ 15;

    if (joystickx < joyxl)
    {
        ev.data2 = -1;
    }
    else if (joystickx > joyxh)
    {
        ev.data2 = 1;
    }
    else
    {
        ev.data2 = 0;
    }
    if (joysticky < joyyl)
    {
        ev.data3 = -1;
    }
    else if (joysticky > joyyh)
    {
        ev.data3 = 1;
    }
    else
    {
        ev.data3 = 0;
    }

    D_PostEvent(&ev);
}


// -----------------------------------------------------------------------------
// DPMI stuff
// -----------------------------------------------------------------------------


//
// DPMIFarCall
//
void DPMIFarCall (void)
{
    segread(&segregs);
    regs.w.ax = 0x301;
    regs.w.bx = 0;
    regs.w.cx = 0;
    regs.x.edi = (unsigned)&dpmiregs;
    segregs.es = segregs.ds;
    int386x(DPMI_INT, &regs, &regs, &segregs);
}


//
// I_StartupDPMI
//
void I_StartupDPMI (void)
{
    extern char __begtext;
    extern char ___Argc;

    // allocate a decent stack for real mode ISRs
    realstackseg = (int)I_AllocLow (1024) >> 4;

    // lock the entire program down
    _dpmi_lockregion (&__begtext, &___Argc - &__begtext);
}


// -----------------------------------------------------------------------------
// Timer interrupt
// -----------------------------------------------------------------------------

//
// IO_TimerISR
//
void __interrupt __far IO_TimerISR (void)
{
    ticcount++;
    _outbyte(0x20, 0x20);   // Ack the interrupt
}


//
// IO_SetTimer0
// Sets system timer 0 to the specified speed
//
void IO_SetTimer0 (int speed)
{
    if (speed > 0 && speed < 150)
    {
        I_Error(english_language ?
                "INT_SetTimer0: %i is a bad value" :
                "INT_SetTimer0: %i является некорректным значением", speed);
    }

    _outbyte(0x43, 0x36);   // Change timer 0
    _outbyte(0x40, speed);
    _outbyte(0x40, speed >> 8);
}


//
// IO_StartupTimer
//
void IO_StartupTimer (void)
{
    oldtimerisr = _dos_getvect(TIMERINT);

    _dos_setvect(0x8000 | TIMERINT, IO_TimerISR);
    IO_SetTimer0(VBLCOUNTER);
}


void IO_ShutdownTimer (void)
{
    if (oldtimerisr)
    {
        IO_SetTimer0(0);    // back to 18.4 ips
        _dos_setvect(TIMERINT, oldtimerisr);
    }
}

//
// I_Init
// hook interrupts and set graphics mode
//
void I_Init (void)
{
    int p;

    novideo = M_CheckParm("novideo");
    p = M_CheckParm("-control");

    if (p)
    {
        extcontrol = (extapi_t*)atoi(myargv[p + 1]);
        printf(english_language ?
               "Using external control API\n" :
               "Используется внешний API управления\n");
    }

    printf("I_StartupDPMI\n");
    I_StartupDPMI();
    printf("I_StartupMouse\n");
    I_StartupMouse();
    printf("I_StartupJoystick\n");		
    I_StartupJoystick();
    printf("I_StartupKeyboard\n");
    I_StartupKeyboard();
    printf("I_StartupSound\n");
    I_StartupSound();
}


//
// I_Shutdown
// return to default system state
//
void I_Shutdown (void)
{
    I_ShutdownGraphics();
    I_ShutdownSound();
    I_ShutdownTimer();
    I_ShutdownMouse();
    I_ShutdownKeyboard();
}


//
// I_Error
//
void I_Error (char *error, ...)
{
    va_list argptr;

    D_QuitNetGame();
    I_Shutdown();
    va_start(argptr, error);
    vprintf(error, argptr);
    va_end(argptr);
    printf("\n");
    exit(1);
}


//
// I_Quit
//
// Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
void I_Quit (void)
{
    byte *scr;

    if (demorecording)
    {
        G_CheckDemoStatus();
    }
    else
    {
        D_QuitNetGame();
    }
    M_SaveDefaults();
    if (english_language)
    {
        scr = (byte*)W_CacheLumpName("ENDOOM", PU_CACHE);
    }
    else
    {
        scr = (byte*)W_CacheLumpName(shareware ? "ENDOOMS" :
                                    registered ? "ENDOOMR" :
                                        retail ? "ENDOOMU" :
                                      plutonia ? "ENDOOMP" :
                                           tnt ? "ENDOOMT" :
                                                 "ENDOOMH" ,
                                                 PU_CACHE);
    }
    I_ShutdownGraphics();
    I_ShutdownSound();
    I_ShutdownTimer();
    I_ShutdownMouse();
    I_ShutdownKeyboard();
    memcpy((void *)0xb8000, scr, 80 * 25 * 2);
    regs.w.ax = 0x0200;
    regs.h.bh = 0;
    regs.h.dl = 0;
    regs.h.dh = 23;
    int386(0x10, (union REGS *)&regs, &regs); // Set text pos
    printf("\n");

    exit(0);
}


//
// I_ZoneBase
//
byte *I_ZoneBase (int *size)
{
    int    meminfo[32];
    int    heap;
    int    maxmem = 8388608; // [JN] Bytes, initial heap size to use (8 MB).
    int    p;
    byte  *ptr;

    memset(meminfo, 0, sizeof(meminfo));
    segread(&segregs);
    segregs.es = segregs.ds;
    regs.w.ax = 0x500; // get memory info
    regs.x.edi = (int)&meminfo;
    int386x(0x31, &regs, &regs, &segregs);

    heap = meminfo[0];
    printf(english_language ?
           "DPMI memory: %d MB" :
           "Память DPMI: %d Мбайт", heap >> 20);

    // [JN] Command line parameter to increase/decrease heap size
    // Thanks to Alexandre-Xavier Labonte-Lamoureux for the code!
    p = M_CheckParm ("-mb");
    if (p && p < myargc-1)
    {
        maxmem = 1024 * 1024 * atoi(myargv[p + 1]);

        // [JN] Crash-preventing conditions:

        // 1) Prevent lock-up if heap size is 0 or below.
        if (maxmem < 1)
        {
            maxmem = 1;
        }
        // 2) Prevent lock-up if heap size is above available memory.
        if (maxmem > heap)
        {
            if (english_language)
            {
                printf("\n\n\nZone memory can't be greater than available DPMI memory.\n\n");
                printf("DOOM aborted.\n");
            }
            else
            {
                printf("\n\n\nРазмер распределяемой памяти не может быть больше доступной памяти DPMI.\n\n");
                printf("Выполнение программы прервано.\n");
            }
            exit(1);
        }
    }
    
    do
    {
        heap -= 131072; // [JN] Bytes, equals 128 KB, leave alone.
        if (heap > maxmem)
        {
            heap = maxmem;
        }
        ptr = malloc(heap);
    } while (!ptr);

    printf(english_language ?
           ", %d MB allocated for zone.\n" :
           ", %d Мбайт обнаружено для распределения.\n", heap >> 20);
    if (heap < 1572864) // [JN] Bytes, equals 1.5 MB.
    {
        printf("\n");

        if (english_language)
        {
            printf("\n");
            printf("Insufficient memory! Please, free up more memory for DOOM.\n\n");
            printf("DOOM aborted.\n");
        }
        else
        {
            printf("\n");
            printf("Недостаточно оперативной памяти!\n\n");
            printf("Выполнение программы прервано.\n");
        }
        exit(1);
    }

    *size = heap;
    return ptr;
}


// -----------------------------------------------------------------------------
// Disk icon flashing
// -----------------------------------------------------------------------------

void I_InitDiskFlash (void)
{
    void *pic;
    byte *temp;

    if (M_CheckParm("-cdrom"))
    {
        pic = W_CacheLumpName("STCDDISK", PU_CACHE);
    }
    else
    {
        pic = W_CacheLumpName("STDISK", PU_CACHE);
    }

    temp = destscreen;
    destscreen = (byte *)0xac000;

    // [JN] Disk icon - make optional
    if (show_diskicon || vanilla)
    {
        V_DrawPatchDirect(SCREENWIDTH - 16, SCREENHEIGHT - 16, 0, pic);
    }
    destscreen = temp;
}


// draw disk icon
void I_BeginRead (void)
{
    int    y;
    byte  *src;
    byte  *dest;

    if (!grmode)
    {
        return;
    }

    // write through all planes
    outp(SC_INDEX, SC_MAPMASK);
    outp(SC_INDEX + 1, 15);
    // set write mode 1
    outp(GC_INDEX, GC_MODE);
    outp(GC_INDEX + 1, inp(GC_INDEX + 1) | 1);

    // copy to backup
    src = currentscreen + 184 * 80 + 304 / 4;
    dest = (byte *)0xac000 + 184 * 80 + 288 / 4;
    for (y = 0 ; y<16 ; y++)
    {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        src += 80;
        dest += 80;
    }

    // copy disk over
    dest = currentscreen + 184 * 80 + 304 / 4;
    src = (byte *)0xac000 + 184 * 80 + 304 / 4;
    for (y = 0 ; y<16 ; y++)
    {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        src += 80;
        dest += 80;
    }

    // set write mode 0
    outp(GC_INDEX, GC_MODE);
    outp(GC_INDEX + 1, inp(GC_INDEX + 1)&~1);
}


// erase disk icon
void I_EndRead (void)
{
    int   y;
    byte *src;
    byte *dest;    

    if (!grmode)
    {
        return;
    }

    // write through all planes
    outp(SC_INDEX, SC_MAPMASK);
    outp(SC_INDEX + 1, 15);
    // set write mode 1
    outp(GC_INDEX, GC_MODE);
    outp(GC_INDEX + 1, inp(GC_INDEX + 1) | 1);

    // copy disk over
    dest = currentscreen + 184 * 80 + 304 / 4;
    src = (byte *)0xac000 + 184 * 80 + 288 / 4;
    for (y = 0; y<16; y++)
    {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        src += 80;
        dest += 80;
    }

    // set write mode 0
    outp(GC_INDEX, GC_MODE);
    outp(GC_INDEX + 1, inp(GC_INDEX + 1)&~1);
}


//
// I_AllocLow
//
byte *I_AllocLow (int length)
{
    byte *mem;

    // DPMI call 100h allocates DOS memory
    segread(&segregs);
    regs.w.ax = 0x0100; // DPMI allocate DOS memory
    regs.w.bx = (length + 15) / 16;
    int386(DPMI_INT, &regs, &regs);
    if (regs.w.cflag != 0)
    {
        I_Error(english_language ?
                "I_AllocLow: DOS alloc of %i failed, %i free" :
                "I_AllocLow: невозможно обнаружить %i, свободно %i",
                length, regs.w.bx * 16);
    }

    mem = (void *)((regs.x.eax & 0xFFFF) << 4);

    memset(mem, 0, length);
    return mem;
}


// -----------------------------------------------------------------------------
// Networking
// -----------------------------------------------------------------------------



//
// I_InitNetwork
//
void I_InitNetwork (void)
{
    int i;

    i = M_CheckParm ("-net");

    if (!i)
    {
        // single player game
        doomcom = malloc(sizeof(*doomcom));
        if (!doomcom)
        {
            I_Error(english_language ?
                    "malloc() in I_InitNetwork() failed" :
                    "Ошибка malloc() в I_InitNetwork()");
        }
        memset(doomcom, 0, sizeof(*doomcom));
        netgame = false;
        doomcom->id = DOOMCOM_ID;
        doomcom->numplayers = doomcom->numnodes = 1;
        doomcom->deathmatch = false;
        doomcom->consoleplayer = 0;
        doomcom->ticdup = 1;
        doomcom->extratics = 0;
        return;
    }

    netgame = true;
    doomcom = (doomcom_t *)atoi(myargv[i + 1]);
    //DEBUG
    doomcom->skill = startskill;
    doomcom->episode = startepisode;
    doomcom->map = startmap;
    doomcom->deathmatch = deathmatch;
}


void I_NetCmd (void)
{
    if (!netgame)
    {
        I_Error(english_language ?
                "I_NetCmd when not in netgame" :
                "I_NetCmd не в сетевой игре");
    }
    DPMIInt(doomcom->intnum);
}


//
// DPMIInt
//
void DPMIInt (int i)
{
    dpmiregs.ss = realstackseg;
    dpmiregs.sp = REALSTACKSIZE - 4;

    segread(&segregs);
    regs.w.ax = 0x300;
    regs.w.bx = i;
    regs.w.cx = 0;
    regs.x.edi = (unsigned)&dpmiregs;
    segregs.es = segregs.ds;
    int386x(DPMI_INT, &regs, &regs, &segregs);
}
