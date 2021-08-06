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
// 	The status bar widget code.
//


#ifndef __STLIB__
#define __STLIB__


#include "r_defs.h"  // We are referring to patches.


// =============================================================================
// Typedefs of widgets
// =============================================================================

// Number widget
typedef struct
{
    int       x;        // upper right-hand corner
    int       y;        //  of the number (right-justified)
    int       width;    // max # of digits in number
    int       oldnum;   // last number value
    int      *num;      // pointer to current value
    boolean  *on;       // pointer to boolean stating whether to update number
    patch_t **p;        // list of patches for 0-9
    int       data;     // user data
} st_number_t;


// Percent widget ("child" of number widget,
//  or, more precisely, contains a number widget.)
typedef struct
{
    st_number_t  n;     // number information
    patch_t     *p;     // percent sign graphic
} st_percent_t;


// Multiple Icon widget
typedef struct
{
    int       x;        // center-justified location of icons
    int       y;
    int       oldinum;  // last icon number
    int      *inum;     // pointer to current icon
    boolean  *on;       // pointer to boolean stating whether to update icon
    patch_t **p;        // list of icons
    int       data;     // user data
} st_multicon_t;


// Binary Icon widget
typedef struct
{
    int      x;         // center-justified location of icon
    int      y;
    boolean  oldval;    // last icon value
    boolean *val;       // pointer to current icon status
    boolean *on;        // pointer to boolean stating whether to update icon
    patch_t *p;	        // icon
    int      data;      // user data
} st_binicon_t;


// =============================================================================
// Widget creation, access, and update routines
// =============================================================================

// Initializes widget library.
// More precisely, initialize STMINUS,
//  everything else is done somewhere else.
//
void STlib_init(void);

// Number widget routines
void STlib_initNum (st_number_t *n, int x, int y, patch_t **pl, int *num, boolean *on, int width);
void STlib_updateNum (st_number_t *n, boolean refresh);

// Percent widget routines
void STlib_initPercent (st_percent_t *p, int x, int y, patch_t **pl, int *num, boolean *on, patch_t *percent);
void STlib_updatePercent (st_percent_t *per, int refresh);

// Multiple Icon widget routines
void STlib_initMultIcon (st_multicon_t *i, int x, int y, patch_t **il, int *inum, boolean *on);
void STlib_updateMultIcon (st_multicon_t *mi, boolean refresh);

// Binary Icon widget routines
void STlib_initBinIcon (st_binicon_t *b, int x, int y, patch_t *i, boolean *val, boolean *on);
void STlib_updateBinIcon (st_binicon_t *bi, boolean refresh);


#endif
