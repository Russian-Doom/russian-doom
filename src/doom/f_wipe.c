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
//	Mission begin melt/wipe screen special effect.
//


#include <stdlib.h>     // [JN] rand()
#include "z_zone.h"
#include "i_video.h"
#include "v_video.h"
#include "f_wipe.h"
#include "deh_str.h"
#include "d_mode.h"     // [JN] Jaguar Doom: gamemission
#include "w_wad.h"      // [JN] Jaguar Doom: W_CacheLumpName
#include "doomstat.h"   // [JN] Jaguar Doom: screenblocks
#include "hu_stuff.h"
#include "st_stuff.h"   // [JN] Jaguar Doom: Status bar widgets
#include "jn.h"         // [JN] Jaguar Doom: english_language


// =============================================================================
// SCREEN WIPE PACKAGE
// =============================================================================

// when zero, stop the wipe
static boolean go = 0;

static byte *wipe_scr_start;
static byte *wipe_scr_end;
static byte *wipe_scr;

static int  *y;


// -----------------------------------------------------------------------------
// wipe_shittyColMajorXform
// -----------------------------------------------------------------------------

static void wipe_shittyColMajorXform (short *array, int width, int height)
{
    short *dest = (short*) Z_Malloc(width * height * sizeof(*dest), PU_STATIC, 0);

    for(int yy = 0 ; yy < height ; yy++)
        for(int xx = 0 ; xx < width ; xx++)
            dest[xx*height+yy] = array[yy*width+xx];

    memcpy(array, dest, width*height*sizeof(*dest));

    Z_Free(dest);
}

// -----------------------------------------------------------------------------
// wipe_initMelt
// -----------------------------------------------------------------------------

static int wipe_initMelt (int width, int height, int ticks)
{
    int i;

    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, width * height * sizeof(*wipe_scr));

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
        static int r;

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
                if ((gamemission == jaguar || screen_wiping == 2) && !vanillaparm)
                {
                    dy = 13; // [JN] almost identical to original wipe duration
                    y[i] += dy;

                    done = false;
                }
                // [JN] Standard wiping
                else
                {
                    dy = (y[i] < 16) ? y[i]+1 : 8;

                    if (y[i]+dy >= height)
                    {
                        dy = height - y[i];
                    }

                    s = &((short *)wipe_scr_end)[i*height+y[i]];
                    d = &((short *)wipe_scr)[y[i]*width+i];
                    idx = 0;

                    for (j = dy ; j ; j--)
                    {
                        d[idx] = *(s++);
                        idx += width;
                    }

                    y[i] += dy;
                    s = &((short *)wipe_scr_start)[i*height];
                    d = &((short *)wipe_scr)[y[i]*width+i];
                    idx = 0;

                    for (j = height-y[i] ; j ; j--)
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
    Z_Free(wipe_scr_start);
    Z_Free(wipe_scr_end);
    // [JN] Need to update classic HUD
    if ((gamemission == jaguar || screen_wiping == 2)
    && screenblocks <= 10 && gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
        HU_Drawer();
    }
    return 0;
}

// -----------------------------------------------------------------------------
// wipe_StartScreen
// -----------------------------------------------------------------------------

int wipe_StartScreen (int x, int y, int width, int height)
{
    wipe_scr_start = Z_Malloc(screenwidth * SCREENHEIGHT 
                   * sizeof(*wipe_scr_start), PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_start);
    return 0;
}

// -----------------------------------------------------------------------------
// wipe_EndScreen
// -----------------------------------------------------------------------------

int wipe_EndScreen (int x, int y, int width, int height)
{
    wipe_scr_end = Z_Malloc(screenwidth * SCREENHEIGHT
                 * sizeof(*wipe_scr_end), PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(x, y, width, height, wipe_scr_start); // restore start scr.
    return 0;
}

// -----------------------------------------------------------------------------
// wipe_ScreenWipe
// -----------------------------------------------------------------------------

int wipe_ScreenWipe (int x, int y, int width, int height, int ticks)
{
    int rc;

    ticks <<= hires;

    // initial stuff
    if (!go)
    {
        go = 1;
        wipe_scr = I_VideoBuffer;
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
    if ((gamemission == jaguar || screen_wiping == 2) && !vanillaparm)
    {
        V_DrawShadowedPatchDoom (wide_delta, 0, W_CacheLumpName(DEH_String
                                (english_language ? "M_LOADIN" : "RD_LDNG"),
                                 PU_CACHE));
    }
    
    return !go;
}
