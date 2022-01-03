//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2022 Julian Nechaevsky
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
//	Mission begin melt/wipe screen special effect.
//


#include <stdlib.h>
#include "z_zone.h"
#include "i_system.h"
#include "v_video.h"
#include "doomdef.h"
#include "f_wipe.h"
#include "r_main.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "jn.h"


// =============================================================================
// SCREEN WIPE PACKAGE
// =============================================================================

// when zero, stop the wipe
static boolean  go = 0;

static byte *wipe_scr_start;
static byte *wipe_scr_end;
static byte *wipe_scr;

static int  *y;


// -----------------------------------------------------------------------------
// wipe_shittyColMajorXform
// -----------------------------------------------------------------------------

static void wipe_shittyColMajorXform (short *array, int width, int height)
{
    int     x;
    int     y;
    short  *dest = (short*) Z_Malloc(width * height * 2, PU_STATIC, 0);

    for(y = 0 ; y < height ; y++)
        for(x = 0 ; x < width ; x++)
            dest[x*height+y] = array[y*width+x];

    memcpy(array, dest, width*height*2);

    Z_Free(dest);
}

// -----------------------------------------------------------------------------
// wipe_initMelt
// -----------------------------------------------------------------------------

static int wipe_initMelt (int width, int height, int ticks)
{
    int i, r;

    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, width*height);

    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((short*)wipe_scr_start, width/2, height);
    wipe_shittyColMajorXform((short*)wipe_scr_end, width/2, height);

    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = (int *) Z_Malloc(width*sizeof(int), PU_STATIC, 0);
    // [JN] Use real random number generator instead of M_Random.
    y[0] = -(rand()%16);
    for (i=1;i<width;i++)
    {
        // [JN] Use real random number generator instead of M_Random.
        r = (rand()%3) - 1;
        y[i] = y[i-1] + r;
        if (y[i] > 0) y[i] = 0;
        else if (y[i] == -16) y[i] = -15;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// wipe_doMelt
// -----------------------------------------------------------------------------

static int wipe_doMelt (int width, int height, int ticks)
{
    int     i;
    int     j;
    int     dy;
    int     idx;

    short*  s;
    short*  d;
    boolean done = true;

    width/=2;

    // [JN] If wiping disabled, return immediately.
    if (screen_wiping == 0 && !vanilla)
    return done;

    while (ticks--)
    {
        for (i=0;i<width;i++)
        {
            if (y[i]<0)
            {
                y[i]++; done = false;
            }
            else if (y[i] < height)
            {
                // [JN] Loading delay emulation
                if (screen_wiping == 2 && !vanilla)
                {
                    dy = 24; // [JN] Slightly faster than original wiping
                    y[i] += dy;
                    idx = 0;

                    done = false;
                }
                else
                {
                    dy = (y[i] < 16) ? y[i]+1 : 8;
                    if (y[i]+dy >= height) dy = height - y[i];
                    s = &((short *)wipe_scr_end)[i*height+y[i]];
                    d = &((short *)wipe_scr)[y[i]*width+i];
                    idx = 0;

                    for (j=dy;j;j--)
                    {
                        d[idx] = *(s++);
                        idx += width;
                    }

                    y[i] += dy;
                    s = &((short *)wipe_scr_start)[i*height];
                    d = &((short *)wipe_scr)[y[i]*width+i];
                    idx = 0;

                    for (j=height-y[i];j;j--)
                    {
                        d[idx] = *(s++);
                        idx += width;
                    }
                    done = false;
                }
            }
        }
    }

    return done;
}

// -----------------------------------------------------------------------------
// wipe_exitMelt
// -----------------------------------------------------------------------------

static int wipe_exitMelt (int width, int height, int ticks)
{
    Z_Free(y);
    // [JN] Need to update classic HUD
    if ((screen_wiping == 0 || screen_wiping == 2) && !vanilla
    && screenblocks <= 10 && gamestate == GS_LEVEL)
    {
        HU_Drawer();
        ST_doRefresh();
        setsizeneeded = true;
    }
    return 0;
}

// -----------------------------------------------------------------------------
// wipe_StartScreen
// -----------------------------------------------------------------------------

int wipe_StartScreen (int x, int y, int width, int height)
{
    wipe_scr_start = screens[2];
    I_ReadScreen(wipe_scr_start);
    return 0;
}

// -----------------------------------------------------------------------------
// wipe_EndScreen
// -----------------------------------------------------------------------------

int wipe_EndScreen (int x, int y, int width, int height)
{
    wipe_scr_end = screens[3];
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(x, y, 0, width, height, wipe_scr_start); // restore start scr.
    return 0;
}

// -----------------------------------------------------------------------------
// wipe_ScreenWipe
// -----------------------------------------------------------------------------

int wipe_ScreenWipe (int x, int y, int width, int height, int ticks)
{
    int         rc;
    void V_MarkRect (int, int, int, int);

    // initial stuff
    if (!go)
    {
        go = 1;
        wipe_scr = screens[0];
        (*wipe_initMelt)(width, height, ticks);
    }

    // do a piece of wipe-in
    V_MarkRect(0, 0, width, height);
    rc = (*wipe_doMelt)(width, height, ticks);

    // final stuff
    if (rc)
    {
        go = 0;
        (*wipe_exitMelt)(width, height, ticks);
    }

    // [JN] Draw "Loading" picture
    if (screen_wiping == 2 && !vanilla)
    {
        V_DrawPatch (0, 0, 0, W_CacheLumpName(english_language ?
                              "M_LOADIN" : "RD_LDNG", PU_CACHE));
    }

    return !go;
}
