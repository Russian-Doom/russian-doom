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
// DESCRIPTION:
//	The status bar widget code.
//


#include <stdio.h>
#include <ctype.h>
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "z_zone.h"
#include "v_video.h"
#include "i_swap.h"
#include "i_system.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"
#include "m_menu.h"
#include "v_trans.h"
#include "jn.h"


// -----------------------------------------------------------------------------
// STlib_init
//
// Hack display negative frags. Loads and store the stminus lump.
// -----------------------------------------------------------------------------

static patch_t *sttminus;

void STlib_init(void)
{
    sttminus = (patch_t *) W_CacheLumpName(DEH_String("STTMINUS"), PU_STATIC);
}

// -----------------------------------------------------------------------------
// STlib_initNum
//
// Initializes an st_number_t widget.
//
// Passed the widget, its position, the patches for the digits, a pointer
// to the value displayed, a pointer to the on/off control, and the width.
// Returns nothing.
// -----------------------------------------------------------------------------

void STlib_initNum (st_number_t *n, int x, int y, patch_t **pl, int *num,
                    boolean *on, int width)
{
    n->x      = x;
    n->y      = y;
    n->oldnum = 0;
    n->width  = width;
    n->num    = num;
    n->on     = on;
    n->p      = pl;
}

// -----------------------------------------------------------------------------
// STlib_drawNum
//
// A fairly efficient way to draw a number 
//  based on differences from the old number.
// Note: worth the trouble?
// -----------------------------------------------------------------------------

void STlib_drawNum (st_number_t *n, boolean refresh)
{
    int numdigits = n->width;
    int num = *n->num;
    int w = SHORT(n->p[0]->width);
    int h = SHORT(n->p[0]->height);
    int x;
    int neg;

    // [JN] CPhipps - compact some code, use num instead of *n->num
    if ((neg = (n->oldnum = num) < 0))
    {
        if (numdigits == 2 && num < -9)
        {
            num = -9;
        }
        else if (numdigits == 3 && num < -99)
        {
            num = -99;
        }
        num = -num;
    }

    // clear the area
    x = n->x - numdigits*w;

    if (n->y - (gamemission == jaguar ? ST_Y_JAG : ST_Y) < 0)
    {
        I_Error("drawNum: n->y - ST_Y < 0");
    }

    if (screenblocks < 11 || (automapactive && !automap_overlay))
    {
        V_CopyRect(x, n->y - (gamemission == jaguar ? ST_Y_JAG : ST_Y), 
                              st_backing_screen, w*numdigits, h, x, n->y);
    }

    // if non-number, do not draw it
    if (num == 1994)
    {
        return;
    }

    x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
    {
        V_DrawPatch(x - w, n->y, n->p[0]);
    }

    // draw the new number
    while (num && numdigits--)
    {
        x -= w;
        V_DrawPatch(x, n->y, n->p[num%10]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
    {
        V_DrawPatch(x - 8, n->y, sttminus);
    }
}

// -----------------------------------------------------------------------------
// STlib_updateNum
//
// Draws a number conditionally based on the widget's enable.
//
// Passed a number widget, the output color range, and a refresh flag.
// Returns nothing.
// -----------------------------------------------------------------------------

void STlib_updateNum (st_number_t *n, boolean refresh)
{
    if (*n->on)
    {
        STlib_drawNum(n, refresh);
    }
}

// -----------------------------------------------------------------------------
// STlib_initPercent
//
// Initialize a st_percent_t number with percent sign widget.
//
// Passed a st_percent_t widget, the position, the digit patches, a pointer
// to the number to display, a pointer to the enable flag, and patch
// for the percent sign.
// Returns nothing.
// -----------------------------------------------------------------------------

void STlib_initPercent (st_percent_t *p, int x, int y, patch_t **pl, int *num,
                        boolean *on, patch_t *percent)
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;
}

// -----------------------------------------------------------------------------
// STlib_updatePercent
// 
// Draws a number/percent conditionally based on the widget's enable.
// -----------------------------------------------------------------------------

void STlib_updatePercent (st_percent_t *per, int refresh)
{

    STlib_updateNum(&per->n, refresh); // [crispy] moved here

    if (sbar_colored == 1 && !vanillaparm)
    {
        dp_translation = cr[CR_WHITE];
    }

    // [JN] CPhipps - make %'s only be updated if number changed
    if (*per->n.on && (refresh || (per->n.oldnum != *per->n.num)))
    {
        V_DrawPatch(per->n.x, per->n.y, per->p);
    }

    dp_translation = NULL;
}

// -----------------------------------------------------------------------------
// STlib_initMultIcon
//
// Initialize a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys.
//
// Passed a st_multicon_t widget, the position, the graphic patches, a pointer
// to the numbers representing what to display, and pointer to the enable flag.
// Returns nothing.
// -----------------------------------------------------------------------------

void STlib_initMultIcon (st_multicon_t *i, int x, int y, patch_t **	il,
                         int *inum, boolean *on)
{
    i->x       = x;
    i->y       = y;
    i->oldinum = -1;
    i->inum    = inum;
    i->on      = on;
    i->p       = il;
}

// -----------------------------------------------------------------------------
// STlib_updateMultIcon
//
// Draw a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys. Displays each when the control
// numbers change or refresh is true.
//
// Passed a st_multicon_t widget, and a refresh flag.
// Returns nothing.
// -----------------------------------------------------------------------------

void STlib_updateMultIcon (st_multicon_t *mi, boolean refresh)
{
    int w, h, x, y;

    if (*mi->on && (mi->oldinum != *mi->inum || refresh))
    {
        if (mi->oldinum != -1)
        {
            x = mi->x - SHORT(mi->p[mi->oldinum]->leftoffset);
            y = mi->y - SHORT(mi->p[mi->oldinum]->topoffset);
            w = SHORT(mi->p[mi->oldinum]->width);
            h = SHORT(mi->p[mi->oldinum]->height);

            if (y - (gamemission == jaguar ? ST_Y_JAG : ST_Y) < 0)
            {
                I_Error("updateMultIcon: y - ST_Y < 0");
            }

            if (screenblocks < 11 || (automapactive && !automap_overlay))
            {
                V_CopyRect(x, y - (gamemission == jaguar ? ST_Y_JAG : ST_Y),
                           st_backing_screen, w, h, x, y);
            }
        }

        if (*mi->inum != -1)  // [JN] killough 2/16/98: redraw only if != -1
        {
            V_DrawPatch(mi->x, mi->y, mi->p[*mi->inum]);
        }

        mi->oldinum = *mi->inum;
    }
}

// -----------------------------------------------------------------------------
// STlib_initBinIcon
//
// Initialize a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons, that are present or not.
//
// Passed a st_binicon_t widget, the position, the digit patches, a pointer
// to the flags representing what is displayed, and pointer to the enable flag.
// Returns nothing.
// -----------------------------------------------------------------------------

void STlib_initBinIcon (st_binicon_t *b, int x, int y, patch_t *i, 
                        boolean *val, boolean *on)
{
    b->x      = x;
    b->y      = y;
    b->oldval = false;
    b->val    = val;
    b->on     = on;
    b->p      = i;
}

// -----------------------------------------------------------------------------
// STlib_updateBinIcon
//
// DInitialize a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons, that are present or not.
//
// Draw a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons that are present or not. Displays each
// when the control flag changes or refresh is true.
//
// Passed a st_binicon_t widget, and a refresh flag.
// Returns nothing.
// -----------------------------------------------------------------------------

void STlib_updateBinIcon (st_binicon_t *bi, boolean refresh)
{
    int x, y, w, h;

    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {
        x = bi->x - SHORT(bi->p->leftoffset);
        y = bi->y - SHORT(bi->p->topoffset);
        w = SHORT(bi->p->width);
        h = SHORT(bi->p->height);

        if (y - (gamemission == jaguar ? ST_Y_JAG : ST_Y) < 0)
        {
            I_Error("updateBinIcon: y - ST_Y < 0");
        }

        if (*bi->val)
        {
            V_DrawPatch(bi->x, bi->y, bi->p);
        }
        else if (screenblocks < 11 || (automapactive && !automap_overlay))
        {
            V_CopyRect(x, y - (gamemission == jaguar ? ST_Y_JAG : ST_Y),
                       st_backing_screen, w, h, x, y);
        }

        bi->oldval = *bi->val;
    }
}
