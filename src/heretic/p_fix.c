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
//  [JN] Note: The idea and the code has been taken from DOOM Retro.
//  Many thanks to Brad Harding for approval of using his work! 
//    2022-08-02 - partially rewritten to separated tables for 
//    faster lookup and faster applying/injection. Table contents:
//
// vertexfix:
// mission, episode, map, vertex, oldx, oldy, newx, newy
//
// linefix:
// mission, episode, map, linedef, side, toptexture, middletexture, bottomtexture, offset, rowoffset, flags, special, tag
//
// sectorfix: 
// mission, episode, map, sector, floorpic, ceilingpic, floorheight, ceilingheight, special, oldtag, newtag
//
// thingfix: 
// mission, episode, map, thing,  type, oldx, oldy, newx, newy, angle, options


#include "d_mode.h"
#include "doomdef.h"
#include "p_fix.h"


#define LINEFIX_END       { -1, 0, 0, 0, 0, "", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT }
#define SECTORFIX_END     { -1, 0, 0, 0, "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT }
// #define VERTEXFIX_END  { -1, 0, 0, 0, 0, 0, 0, 0 }
// #define THINGFIX_END   { -1, 0, 0, 0, 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT }

static const linefix_t   linefix_dummy[]   = { LINEFIX_END   };
static const sectorfix_t sectorfix_dummy[] = { SECTORFIX_END };
// static const vertexfix_t vertexfix_dummy[] = { VERTEXFIX_END };
// static const thingfix_t  thingfix_dummy[]  = { THINGFIX_END  };

// [JN] Tables with map fixes.
const linefix_t   *selected_linefix;
const sectorfix_t *selected_sectorfix;
// const vertexfix_t *selected_vertexfix;
// const thingfix_t  *selected_thingfix;



// -----------------------------------------------------------------------------
// E1M2:  THE DUNGEONS
// -----------------------------------------------------------------------------

static const linefix_t linefix_e1m2[] =
{
    { heretic, 1, 2,  477, 1, "MOSSRCK1", "", "", DEFAULT, DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 2,  478, 1, "MOSSRCK1", "", "", DEFAULT, DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 2,  479, 1, "MOSSRCK1", "", "",      96, DEFAULT, ML_DONTPEGTOP, DEFAULT, DEFAULT },
    { heretic, 1, 2, 1057, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT,       DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E1M3:  THE GATEHOUSE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e1m3[] =
{
    { heretic, 1, 3, 21, 1, "SQPEB1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 3, 22, 1, "SQPEB1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 3, 23, 1, "SQPEB1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 3, 24, 1, "SQPEB1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E1M4:  THE GUARD TOWER
// -----------------------------------------------------------------------------

static const linefix_t linefix_e1m4[] =
{
    { heretic, 1, 4,  368, 1, "CTYSTUC4", "",         "", DEFAULT,      -4, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4,  473, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4,  474, 1, "CTYSTUC4", "",         "", DEFAULT,      -4, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4,  490, 1,         "", "",  "GRSTNPB", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4,  722, 0,         "", "",   "WOODWL", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4,  911, 0,         "", "",   "WOODWL", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4,  913, 0,         "", "",   "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4,  914, 0,         "", "",   "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 4, 1296, 0,         "", "",   "WOODWL", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E1M5:  THE CITADEL
// -----------------------------------------------------------------------------

static const linefix_t linefix_e1m5[] =
{
    { heretic, 1, 5,  519, 0,        "", "",   "WOODWL", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 5, 1330, 0, "CSTLRCK", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 5, 1844, 0,        "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 5, 1845, 0,        "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_e1m5[] =
{
    { heretic, 1, 5, 54, "", "", DEFAULT, DEFAULT, 38, DEFAULT, DEFAULT },
    { heretic, 1, 5, 57, "", "", DEFAULT, DEFAULT, 38, DEFAULT, DEFAULT },    
    { heretic, 1, 5, 86, "", "", DEFAULT, DEFAULT, 23, DEFAULT, DEFAULT },
    { heretic, 1, 5, 87, "", "", DEFAULT, DEFAULT, 23, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E1M6:  THE CATHEDRAL
// -----------------------------------------------------------------------------

static const linefix_t linefix_e1m6[] =
{
    { heretic, 1, 6,  491, 1, "SPINE2", "",         "", DEFAULT,      -1, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 6,  492, 1, "SPINE2", "",         "", DEFAULT,      -1, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 6,  493, 1, "SPINE2", "",         "", DEFAULT,      -1, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 6,  494, 1, "SPINE2", "",         "", DEFAULT,      -1, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 6,  495, 1, "SPINE2", "",         "", DEFAULT,      -1, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 1, 6, 1207, 1,       "", "", "SNDBLCKS", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E1M7:  THE CRYPTS
// -----------------------------------------------------------------------------

static const linefix_t linefix_e1m7[] =
{
    { heretic, 1, 7, 547, 1, "", "", "SPINE2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E2M1:  THE CRATER
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_e2m1[] =
{
    { heretic, 2, 1, 23, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 1, 28, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 1, 30, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 1, 32, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E2M2:  THE LAVA PITS
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_e2m2[] =
{
    { heretic, 2, 2,  95, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 2, 103, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 2, 104, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 2, 106, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 2, 108, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 2, 108, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 2, 114, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 2, 115, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E2M3:  THE RIVER OF FIRE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e2m3[] =
{
    { heretic, 2, 3, 1029, 1, "", "", "LAVA1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_e2m3[] =
{
    { heretic, 2, 3,  16, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  19, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  26, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  29, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  53, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  61, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  63, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  80, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  82, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  85, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3,  87, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 3, 115, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E2M4:  THE ICE GROTTO
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_e2m4[] =
{
    { heretic, 2, 4, 33, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 4, 79, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E2M5:  THE CATACOMBS
// -----------------------------------------------------------------------------

static const linefix_t linefix_e2m5[] =
{
    { heretic, 2, 5,  486, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5,  492, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5,  501, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5,  507, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5,  513, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5,  523, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5,  528, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5,  557, 0, "GRSTNPB", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5, 1029, 1, "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_e2m5[] =
{
    { heretic, 2, 5, 0, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 5, 3, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E2M6:  THE LABYRINTH
// -----------------------------------------------------------------------------

static const linefix_t linefix_e2m6[] =
{
    { heretic, 2, 6, 238, 1, "TRISTON1", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 6, 726, 0, "TRISTON1", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 6, 737, 0, "TRISTON1", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 6, 862, 1,         "", "", "TRISTON2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E2M7:  THE GREAT HALL
// -----------------------------------------------------------------------------

static const linefix_t linefix_e2m7[] =
{
    { heretic, 2, 7, 493, 1,         "", "", "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 504, 1,         "", "", "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 505, 1,         "", "", "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 506, 1,         "", "", "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 507, 1,         "", "", "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 629, 1, "LOOSERCK", "",       "",      16, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 749, 0,         "", "",  "METL1",     -32, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_e2m7[] =
{
    { heretic, 2, 7, 105, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 110, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 111, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 112, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 113, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 114, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 115, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 116, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 117, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 118, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 119, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 120, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 2, 7, 122, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E3M1:  THE STOREHOUSE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e3m1[] =
{
    { heretic, 3, 1, 295, 0, "SQPEB1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E3M2:  THE CESSPOOL
// -----------------------------------------------------------------------------

static const linefix_t linefix_e3m2[] =
{
    { heretic, 3, 2, 521, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 522, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 523, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 524, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 525, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 526, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 527, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 528, 1, "TRISTON2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 2, 674, 1,         "", "", "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E3M4:  THE AZURE FORTRESS
// -----------------------------------------------------------------------------

static const linefix_t linefix_e3m4[] =
{
    { heretic, 3, 4, 36, 0, "SNDPLAIN", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 4, 36, 1,         "", "", "SNDPLAIN", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E3M6:  THE HALLS OF FEAR
// -----------------------------------------------------------------------------

static const linefix_t linefix_e3m6[] =
{
    { heretic, 3, 6, 343, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 6, 370, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E3M7:  THE CHASM
// -----------------------------------------------------------------------------

static const linefix_t linefix_e3m7[] =
{
    { heretic, 3, 7,  84, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7,  89, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 143, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 154, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 155, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 156, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 157, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 158, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 159, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 3, 7, 160, 0, "MOSSRCK1", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E3M9:  THE AQUIFER
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_e3m9[] =
{
    { heretic, 3, 9, 87, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E4M1:  CATAFALQUE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e4m1[] =
{
    { heretic, 4, 1, 514, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 520, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 526, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 527, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 528, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 529, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 530, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 532, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 533, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 534, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 535, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 564, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 565, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 566, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 567, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 568, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 569, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 570, 1, "SNDCHNKS", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 748, 1,  "CHAINSD", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 765, 0,  "SANDSQ2", "", "",       8,       2, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 766, 0,  "SANDSQ2", "", "",       8,       2, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 1, 767, 0,  "SANDSQ2", "", "", DEFAULT,       2, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E4M4:  SEPULCHER
// -----------------------------------------------------------------------------

static const linefix_t linefix_e4m4[] =
{
    { heretic, 4, 4, 524, 1, "", "",    "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 4, 534, 1, "", "",    "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 4, 544, 1, "", "",    "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 4, 554, 1, "", "",    "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 4, 805, 1, "", "",    "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 4, 808, 0, "", "", "SNDBLCKS", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 4, 963, 1, "", "",   "SQPEB1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E4M6:  HALLS OF THE APOSTATE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e4m6[] =
{
    { heretic, 4, 6, 948, 1, "", "", "GRSKULL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E4M7:  RAMPARTS OF PERDITION
// -----------------------------------------------------------------------------

static const linefix_t linefix_e4m7[] =
{
    { heretic, 4, 7,  721, 1,        "", "", "LOOSERCK", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 1274, 1, "CSTLRCK", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 1277, 1, "CSTLRCK", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 1278, 1, "CSTLRCK", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_e4m7[] =
{
    { heretic, 4, 7,  50, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7,  95, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7,  96, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7,  99, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 104, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 105, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 125, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 157, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 187, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 225, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 306, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 307, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 338, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 4, 7, 340, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E4M8:  SHATTERED BRIDGE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e4m8[] =
{
    { heretic, 4, 8, 142, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP, DEFAULT, DEFAULT },
    { heretic, 4, 8, 143, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP, DEFAULT, DEFAULT },
    { heretic, 4, 8, 384, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP, DEFAULT, DEFAULT },
    { heretic, 4, 8, 411, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E4M9:  MAUSOLEUM
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_e4m9[] =
{
    { heretic, 4, 9, 380, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E5M2:  RAPIDS
// -----------------------------------------------------------------------------

static const linefix_t linefix_e5m2[] =
{
    { heretic, 5, 2, 377, 0, "SQPEB2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E5M3:  QUAY
// -----------------------------------------------------------------------------

static const linefix_t linefix_e5m3[] =
{
    { heretic, 5, 3, 485, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 486, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 487, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 488, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 489, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 492, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 493, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 494, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 497, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 498, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 499, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 500, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 501, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 897, 1, "CSTLRCK", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_e5m3[] =
{
    { heretic, 5, 3, 176, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 3, 210,        "", "",      56,      56, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E5M4:  COURTYARD
// -----------------------------------------------------------------------------

static const linefix_t linefix_e5m4[] =
{
    { heretic, 5, 4, 1341, 0, "BLUEFRAG", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E5M5:  HYDRATYR
// -----------------------------------------------------------------------------

static const linefix_t linefix_e5m5[] =
{
    { heretic, 5, 5,  797, 1,       "", "", "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5,  805, 1,       "", "", "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 1533, 1, "SPINE2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 1534, 1, "SPINE2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 1535, 1, "SPINE2", "",      "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 1636, 1,       "", "", "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 1642, 1,       "", "", "METL2", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_e5m5[] =
{
    { heretic, 5, 5, 192, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 196, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 197, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 219, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 268, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 5, 272, "SMLAVA1", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E5M6:  COLONNADE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e5m6[] =
{
    { heretic, 5, 6,  680, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6,  681, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6,  682, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6,  683, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6,  684, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6,  685, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6, 1629, 1, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6, 1942, 1, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 6, 1945, 1, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E5M7:  FOETID MANSE
// -----------------------------------------------------------------------------

static const linefix_t linefix_e5m7[] =
{
    { heretic, 5, 7, 1109, 0, "CTYSTCI4", "",         "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1337, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1338, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1339, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1340, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1344, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1345, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1348, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 7, 1349, 1,         "", "", "WATRWAL1", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E5M9:  SKEIN OF D'SPARIL
// -----------------------------------------------------------------------------

static const linefix_t linefix_e5m9[] =
{
    { heretic, 5, 9,    5, 0,      "", "", "",     -32, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9,   18, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9,   35, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1362, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1363, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1364, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1365, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1366, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1367, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1368, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1369, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1370, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1371, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1372, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1373, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1374, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1375, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1376, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    { heretic, 5, 9, 1377, 0, "METL2", "", "", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};


// -----------------------------------------------------------------------------
// P_SetupFixes
// [JN] Sets appropriated fixes for selected map.
// -----------------------------------------------------------------------------

void P_SetupFixes (const int episode, const int map)
{
    // First, initialize all fixes with dummies.
    selected_linefix   = linefix_dummy;
    selected_sectorfix = sectorfix_dummy;
    // selected_vertexfix = vertexfix_dummy;
    // selected_thingfix  = thingfix_dummy;

    // Next, define which game will use which map fixes.
    if (gameepisode == 1)
    {
        if (gamemap == 2)
        {
            selected_linefix   = linefix_e1m2;
        }
        else
        if (gamemap == 3)
        {
            selected_linefix   = linefix_e1m3;
        }
        else
        if (gamemap == 4)
        {
            selected_linefix   = linefix_e1m4;
        }
        else
        if (gamemap == 5)
        {
            selected_linefix   = linefix_e1m5;
            selected_sectorfix = sectorfix_e1m5;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_e1m6;
        }
        else
        if (gamemap == 7)
        {
            selected_linefix   = linefix_e1m7;
        }
    }
    else
    if (gameepisode == 2)
    {
        if (gamemap == 1)
        {
            selected_sectorfix = sectorfix_e2m1;
        }
        else
        if (gamemap == 2)
        {
            selected_sectorfix = sectorfix_e2m2;
        }
        else
        if (gamemap == 3)
        {
            selected_linefix   = linefix_e2m3;
            selected_sectorfix = sectorfix_e2m3;
        }
        else
        if (gamemap == 4)
        {
            selected_sectorfix = sectorfix_e2m4;
        }
        else
        if (gamemap == 5)
        {
            selected_linefix   = linefix_e2m5;
            selected_sectorfix = sectorfix_e2m5;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_e2m6;
        }
        else
        if (gamemap == 7)
        {
            selected_linefix   = linefix_e2m7;
            selected_sectorfix = sectorfix_e2m7;
        }
    }
    else
    if (gameepisode == 3)
    {
        if (gamemap == 1)
        {
            selected_linefix   = linefix_e3m1;
        }
        else
        if (gamemap == 2)
        {
            selected_linefix   = linefix_e3m2;
        }
        else
        if (gamemap == 4)
        {
            selected_linefix   = linefix_e3m4;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_e3m6;
        }
        else
        if (gamemap == 7)
        {
            selected_linefix   = linefix_e3m7;
        }
        else
        if (gamemap == 9)
        {
            selected_sectorfix = sectorfix_e3m9;
        }
    }
    else
    if (gameepisode == 4)
    {
        if (gamemap == 1)
        {
            selected_linefix   = linefix_e4m1;
        }
        else
        if (gamemap == 4)
        {
            selected_linefix   = linefix_e4m4;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_e4m6;
        }
        else
        if (gamemap == 7)
        {
            selected_linefix   = linefix_e4m7;
            selected_sectorfix = sectorfix_e4m7;
        }
        else
        if (gamemap == 8)
        {
            selected_linefix   = linefix_e4m8;
        }
        else
        if (gamemap == 9)
        {
            selected_sectorfix = sectorfix_e4m9;
        }
    }
    else
    if (gameepisode == 5)
    {
        if (gamemap == 2)
        {
            selected_linefix   = linefix_e5m2;
        }
        else
        if (gamemap == 3)
        {
            selected_linefix   = linefix_e5m3;
            selected_sectorfix = sectorfix_e5m3;
        }
        else
        if (gamemap == 4)
        {
            selected_linefix   = linefix_e5m4;
        }
        else
        if (gamemap == 5)
        {
            selected_linefix   = linefix_e5m5;
            selected_sectorfix = sectorfix_e5m5;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_e5m6;
        }
        else
        if (gamemap == 7)
        {
            selected_linefix   = linefix_e5m7;
        }
        else
        if (gamemap == 9)
        {
            selected_linefix   = linefix_e5m9;
        }
    }
}
