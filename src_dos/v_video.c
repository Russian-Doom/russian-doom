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
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//


#include <conio.h>

#include "doomdef.h"
#include "doomdata.h"
#include "doomstat.h"

#include "i_system.h"
#include "m_misc.h"
#include "r_local.h"
#include "v_video.h"

#include "jn.h"


#define SC_INDEX            0x3C4
#define SC_RESET            0
#define SC_CLOCK            1
#define SC_MAPMASK          2
#define SC_CHARMAP          3
#define SC_MEMMODE          4

#define GC_INDEX            0x3CE
#define GC_SETRESET         0
#define GC_ENABLESETRESET   1
#define GC_COLORCOMPARE     2
#define GC_DATAROTATE       3
#define GC_READMAP          4
#define GC_MODE             5
#define GC_MISCELLANEOUS    6
#define GC_COLORDONTCARE    7
#define GC_BITMASK          8

int    usegamma;
int    dirtybox[4];

byte  *screens[5];	// Each screen is [SCREENWIDTH*SCREENHEIGHT]; 
byte  *dp_translation = NULL;

extern byte *tintmap;


//
// V_MarkRect 
// 
void V_MarkRect (int x, int y, int width, int height)
{ 
    M_AddToBox (dirtybox, x, y); 
    M_AddToBox (dirtybox, x+width-1, y+height-1); 
} 


//
// V_CopyRect 
// 
void
V_CopyRect
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn ) 
{ 
    byte *src;
    byte *dest; 

#ifdef RANGECHECK 
    if (srcx < 0
    ||  srcx+width > SCREENWIDTH
    ||  srcy < 0
    ||  srcy+height> SCREENHEIGHT 
    ||  destx < 0 || destx+width >SCREENWIDTH
    ||  desty < 0
    ||  desty+height > SCREENHEIGHT 
    || (unsigned)srcscrn > 4
    || (unsigned)destscrn> 4 )
    {
        I_Error (english_language ?
        "Bad V_CopyRect" :
        "Žè¨¡ª  V_CopyRect");
    }
#endif

    V_MarkRect (destx, desty, width, height); 

    src = screens[srcscrn]+SCREENWIDTH*srcy+srcx; 
    dest = screens[destscrn]+SCREENWIDTH*desty+destx; 

    for ( ; height > 0 ; height--) 
    { 
        memcpy (dest, src, width); 
        src += SCREENWIDTH; 
        dest += SCREENWIDTH; 
    } 
} 
 

//
// V_DrawPatch
// Masks a column based masked pic to the screen. 
//
void V_DrawPatch (int x, int y, int scrn, patch_t *patch)
{ 
    int        count;
    int        col;
    int        w;
    byte      *desttop;
    byte      *dest;
    byte      *source;
    byte      *sourcetrans;
    column_t  *column;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

#ifdef RANGECHECK 
    if (x < 0
    ||  x + SHORT(patch->width) >SCREENWIDTH
    ||  y < 0
    ||  y + SHORT(patch->height)>SCREENHEIGHT
    || (unsigned)scrn > 4)
    {
        // [JN] Disabled for preventing text mess appearing on the screen.
        /*
        fprintf( stderr, "Patch at %d,%d exceeds LFB\n", x,y );
        // No I_Error abort - what is up with TNT.WAD?
        fprintf( stderr, "V_DrawPatch: bad patch (ignored)\n");
        */
        return;
    }
#endif 

    if (!scrn)
    {
        V_MarkRect (x, y, SHORT(patch->width), SHORT(patch->height));
    }

    col = 0; 
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++)
    { 
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column 
        while (column->topdelta != 0xff ) 
        { 
            source = sourcetrans = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--) 
            { 
                if (dp_translation)
                {
                    sourcetrans = &dp_translation[*source++];
                }

                *dest = *sourcetrans++; 
                dest += SCREENWIDTH; 
            } 

            column = (column_t *)((byte *)column + column->length + 4 ); 
        }
    }
} 


// V_DrawShadowedPatch
// [JN] Masks a column based masked pic with shadow to the screen. 
void V_DrawShadowedPatch (int x, int y, int scrn, patch_t *patch)
{ 
    int        count, col, w;
    byte      *source, *sourcetrans;
    byte      *desttop, *dest;
    byte      *desttop2, *dest2;
    column_t  *column;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

#ifdef RANGECHECK 
    if (x < 0
    ||  x + SHORT(patch->width) >SCREENWIDTH
    ||  y < 0
    ||  y + SHORT(patch->height)>SCREENHEIGHT
    || (unsigned)scrn > 4)
    {
        // [JN] Disabled for preventing text mess appearing on the screen.
        return;
    }
#endif

    if (!scrn)
    {
        V_MarkRect (x, y, SHORT(patch->width), SHORT(patch->height));
    }

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;
    desttop2 = screens[scrn] + (y+1) * SCREENWIDTH + (x+1);

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++, desttop2++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff )
        {
            source = sourcetrans = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            dest2 = desttop2 + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                if (dp_translation)
                {
                    sourcetrans = &dp_translation[*source++];
                }

                if (draw_shadowed_text && !vanilla)
                {
                    *dest2 = tintmap[((*dest2)<<8)];
                    dest2 += SCREENWIDTH;
                }
                *dest = *sourcetrans++;
                dest += SCREENWIDTH;
            }

            column = (column_t *)((byte *)column + column->length + 4 );
        }
    }
}


//
// V_DrawPatchFlipped 
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//
void V_DrawPatchFlipped (int x, int y, int scrn, patch_t *patch)
{ 
    int        count;
    int        col;
    int        w;
    byte      *desttop;
    byte      *dest;
    byte      *source;
    byte      *sourcetrans;
    column_t  *column;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

#ifdef RANGECHECK
    if (x < 0
    ||  x + SHORT(patch->width) >SCREENWIDTH
    ||  y < 0
    ||  y + SHORT(patch->height)>SCREENHEIGHT
    || (unsigned)scrn > 4)
    {
        // [JN] Disabled for preventing text mess appearing on the screen,
        // make non-fatal.
        /*
        fprintf( stderr, "Patch origin %d,%d exceeds LFB\n", x,y );
        I_Error ("Žè¨¡ª  V_DrawPatch ¢ V_DrawPatchFlipped");
        */
        return;
    }
#endif 

    if (!scrn)
    {
        V_MarkRect (x, y, SHORT(patch->width), SHORT(patch->height));
    }

    col = 0;
    desttop = screens[scrn] + y * SCREENWIDTH + x;

    w = SHORT(patch->width); 

    for ( ; col<w ; x++, col++, desttop++)
    { 
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[w-1-col]));

        // step through the posts in a column 
        while (column->topdelta != 0xff ) 
        {
            source = sourcetrans = (byte *)column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length; 

            while (count--) 
            {
                if (dp_translation)
                {
                    sourcetrans = &dp_translation[*source++];
                }

                *dest = *sourcetrans++; 
                dest += SCREENWIDTH; 
            }

            column = (column_t *)((byte *)column + column->length + 4); 
        }
    }			 
} 


//
// V_DrawPatchDirect
// Draws directly to the screen on the pc. 
//
void V_DrawPatchDirect (int x, int y, int scrn, patch_t *patch)
{
    int        count;
    int        col;
    int        w;
    byte      *desttop;
    byte      *dest;
    byte      *source;
    byte      *sourcetrans;
    column_t  *column;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

#ifdef RANGECHECK 
    if (x < 0
    ||  x + SHORT(patch->width) >SCREENWIDTH
    ||  y < 0
    ||  y + SHORT(patch->height)>SCREENHEIGHT
    || (unsigned)scrn > 4)
    {
        // [JN] Make non-fatal
        // I_Error ("Žè¨¡ª  V_DrawPatchDirect");
        return;
    }
#endif 

    desttop = destscreen + y*SCREENWIDTH/4 + (x>>2);

    w = SHORT(patch->width);

    for (col = 0 ; col<w ; col++)
    {
        outp (SC_INDEX+1,1<<(x&3));
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        { 
            source = sourcetrans = (byte *)column + 3;
            dest = desttop + column->topdelta*SCREENWIDTH/4;
            count = column->length;

            while (count--)
            {
                if (dp_translation)
                {
                    sourcetrans = &dp_translation[*source++];
                }

                *dest = *sourcetrans++;
                dest += SCREENWIDTH/4;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }

        if (((++x)&3) == 0) 
        {
            desttop++;	// go to next byte, not next plane
        }
    }
}


//
// V_DrawShadowDirect
// [JN] Masks a column based masked shadow to the screen.
//
void V_DrawShadowDirect (int x, int y, int scrn, patch_t *patch)
{
    int        count;
    int        col;
    int        w;
    byte      *desttop;
    byte      *dest;
    byte      *source;
    column_t  *column;

    if (!draw_shadowed_text || vanilla)
    {
        return;
    }

    y -= SHORT(patch->topoffset); 
    x -= SHORT(patch->leftoffset); 

#ifdef RANGECHECK 
    if (x < 0 
    ||  x + SHORT(patch->width) >SCREENWIDTH
    ||  y < 0
    ||  y + SHORT(patch->height)>SCREENHEIGHT
    || (unsigned)scrn > 4)
    {
        // [JN] Make non-fatal. 
        // I_Error ("Žè¨¡ª  V_DrawShadowDirect");
        return;
    }
#endif 

    desttop = destscreen + y*SCREENWIDTH/4 + (x>>2);

    w = SHORT(patch->width);

    for ( col = 0 ; col<w ; col++)
    {
        outpw (GC_INDEX,GC_READMAP+((x&3)<<8)); 
        outp (SC_INDEX+1,1<<(x&3));
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column 

        while (column->topdelta != 0xff ) 
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta*SCREENWIDTH/4;
            count = column->length;

            while (count--)
            {
                *dest = tintmap[((*dest)<<8)];
                dest += SCREENWIDTH/4;
            }

            column = (column_t *)((byte *)column + column->length + 4);
        }

        if (((++x)&3) == 0) 
        {
            desttop++;	// go to next byte, not next plane 
        }
    }
} 


//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//
void V_DrawBlock (int x, int y, int scrn, int width, int height, byte *src)
{ 
    byte *dest;

#ifdef RANGECHECK 
    if (x < 0
    ||  x + width >SCREENWIDTH
    ||  y < 0
    ||  y + height>SCREENHEIGHT
    ||  (unsigned)scrn > 4)
    {
        I_Error (english_language ?
                 "Bad V_DrawBlock" :
                 "Žè¨¡ª  V_DrawBlock");
    }
#endif 

    V_MarkRect (x, y, width, height);

    dest = screens[scrn] + y*SCREENWIDTH+x;

    while (height--)
    {
        memcpy (dest, src, width);
        src += width;
        dest += SCREENWIDTH;
    }
}


//
// V_GetBlock
// Gets a linear block of pixels from the view buffer.
//
void V_GetBlock (int x, int y, int scrn, int width, int height, byte *dest)
{
    byte *src;

#ifdef RANGECHECK 
    if (x < 0
    ||  x + width >SCREENWIDTH
    ||  y < 0
    ||  y + height>SCREENHEIGHT 
    || (unsigned)scrn>4 )
    {
        I_Error (english_language ?
                 "Bad V_GetBlock" :
                 "Žè¨¡ª  V_GetBlock");
    }
#endif

    src = screens[scrn] + y*SCREENWIDTH+x;

    while (height--)
    { 
        memcpy (dest, src, width);
        src += SCREENWIDTH;
        dest += width;
    }
}


//
// V_Init
// 
void V_Init (void) 
{ 
    int    i;
    byte  *base;

    // stick these in low dos memory on PCs
    base = I_AllocLow (SCREENWIDTH*SCREENHEIGHT*4);

    for (i=0 ; i<4 ; i++)
    {
        screens[i] = base + i*SCREENWIDTH*SCREENHEIGHT;
    }
}

