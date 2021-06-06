//
// Copyright(C) 2013-2018 Brad Harding
// Copyright(C) 2021 Julian Nechaevsky
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
//	Verious level-specific fixes.
//
//  Note: The idea and the code has been taken from Doom Retro.
//  Many thanks to Brad Harding for approval of using his work! 
//


#include "p_fix.h"


vertexfix_t vertexfix[] =
{
    // episode, map, vertex,  oldx,  oldy,  newx,  newy
    // [JN] Nothing for now. An example:
    /*
    {        1,   3,    771,  -328, -1920,  -320, -1920 },
    */
    {        0,   0,      0,     0,     0,     0,     0 }
};

linefix_t linefix[] =
{
    // episode, map, linedef, side,  toptexture, middletexture, bottomtexture,  offset, rowoffset,   flags, special,     tag
    {        1,   5,    1844,    0,          "",            "",    "WATRWAL1", DEFAULT,   DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    {        1,   5,    1845,    0,          "",            "",    "WATRWAL1", DEFAULT,   DEFAULT, DEFAULT, DEFAULT, DEFAULT },

    {        0,   0,       0,    0,          "",            "",            "", DEFAULT,   DEFAULT, DEFAULT, DEFAULT, DEFAULT }
};

sectorfix_t sectorfix[] =
{
    // episode, map, sector,     floorpic, ceilingpic, floorheight, ceilingheight, special,   oldtag,    newtag
    {        1,   5,     49,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        1,   5,     63,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        1,   5,     54,   "WATSCRW1",         "",     DEFAULT,       DEFAULT,      38,  DEFAULT,   DEFAULT },
    {        1,   5,     57,   "WATSCRW1",         "",     DEFAULT,       DEFAULT,      38,  DEFAULT,   DEFAULT },    
    {        1,   5,     86,   "WATSCRE1",         "",     DEFAULT,       DEFAULT,      23,  DEFAULT,   DEFAULT },
    {        1,   5,     87,   "WATSCRE1",         "",     DEFAULT,       DEFAULT,      23,  DEFAULT,   DEFAULT },
    {        1,   5,     72,   "WATSCRX1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        1,   5,    295,   "WATSCRX1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     11,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     12,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     15,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     16,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     19,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     21,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     24,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     52,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     57,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     58,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     66,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,     67,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    126,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    128,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    129,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    131,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    132,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    137,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    139,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    157,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    162,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    173,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    176,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    182,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    185,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    189,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    191,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    192,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        2,   5,    195,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        3,   4,     54,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,      1,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,      2,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,      5,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,      6,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,      7,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,      8,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,      9,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     10,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     14,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     15,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     23,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     24,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     25,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     26,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     27,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     28,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     29,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     30,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     31,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     32,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     33,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     34,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     35,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     37,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     38,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     39,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     40,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     43,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     44,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     45,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     51,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     52,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     53,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     54,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     55,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     56,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     57,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     60,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     61,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     63,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     64,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     65,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     66,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     68,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     69,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     70,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     71,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     73,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     74,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     75,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     76,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     77,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     78,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     79,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     80,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     86,   "WATSCRS1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,     97,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    111,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    116,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    117,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    121,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    122,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    124,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    125,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    128,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    130,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    131,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    132,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    134,   "WATSCRN1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    135,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    136,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    137,   "WATSCRE1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    138,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    139,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    140,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    141,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    142,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    143,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    145,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    166,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
    {        5,   2,    167,   "WATSCRW1",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },

    {        0,   0,      0,           "",         "",     DEFAULT,       DEFAULT, DEFAULT,  DEFAULT,   DEFAULT },
};

thingfix_t thingfix[] =
{
    // episode, map, thing,  type,    oldx,    oldy,    newx,    newy,   angle, options
    // [JN] Nothing for now. An example:
    /*
    {        1,   6,    16,  3002,     800,    -704,     800,    -688, DEFAULT, DEFAULT },
    */
    {        0,   0,     0,     0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT }
};
