//
// Copyright(C) 2013-2018 Brad Harding
// Copyright(C) 2018-2023 Julian Nechaevsky
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
//  [JN] Note: The idea and the code has beed taken from DOOM Retro.
//  Many thanks to Brad Harding for approval of using his work! 
//    2022-08-02 - partially rewritten to separated tables for 
//    faster lookup and faster applying/injection.
//


#include "doomstat.h"
#include "p_local.h"


#define VERTEXFIX_END  { -1, 0, 0, 0, 0, 0, 0, 0 }
#define LINEFIX_END    { -1, 0, 0, 0, 0, "", "", "", DEFAULT, DEFAULT, DEFAULT }
#define SECTORFIX_END  { -1, 0, 0, 0, "", "" },

#define FLOW_END       { -1, 0, 0, 0, DEFAULT }
#define FALL_END       { -1, 0, 0, 0, DEFAULT }

static const vertexfix_t vertexfix_dummy[] = { VERTEXFIX_END };
static const linefix_t   linefix_dummy[]   = { LINEFIX_END   };
static const sectorfix_t sectorfix_dummy[] = { SECTORFIX_END };

static const flow_t flow_dummy[] = { FLOW_END };
static const fall_t fall_dummy[] = { FLOW_END };

// [JN] Tables with map fixes.
const vertexfix_t *selected_vertexfix;
const linefix_t   *selected_linefix;
const sectorfix_t *selected_sectorfix;

// [JN] Tables with liquid flow/fall effect.
const flow_t *selected_flow;
const fall_t *selected_fall;


// =============================================================================
// THE ULTIMATE DOOM
// =============================================================================

// -----------------------------------------------------------------------------
// E1M1: HANGAR
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e1m1[] =
{
    { doom, 1, 1, 160, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    { doom, 1, 1, 161, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    { doom, 1, 1, 162, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    { doom, 1, 1, 163, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    { doom, 1, 1, 164, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    { doom, 1, 1, 165, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    { doom, 1, 1, 166, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    { doom, 1, 1, 167, 0, "", "", "NUKE24", DEFAULT, -8, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e1m1[] =
{
    { doom, 1, 1, 67, "", "FLAT1" },
    { doom, 1, 1, 71, "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e1m1[] =
{
    { doom, 1, 1,  0, 102 },
    { doom, 1, 1, 51, 101 },
    { doom, 1, 1, 53, 101 },
    { doom, 1, 1, 57, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E1M2: NUCLEAR PLANT
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e1m2[] =
{
    { doom, 1, 2, 134, 1, "STARTAN2", "", "", DEFAULT,      69, DEFAULT },
    { doom, 1, 2, 281, 0,         "", "", "",     -36, DEFAULT, DEFAULT },
    { doom, 1, 2, 573, 0,         "", "", "",      32, DEFAULT, DEFAULT },
    { doom, 1, 2, 574, 1, "COMPTALL", "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 1, 2, 575, 0,         "", "", "",     192, DEFAULT, DEFAULT },
    { doom, 1, 2, 689, 0,         "", "", "", DEFAULT,      55, DEFAULT },
    { doom, 1, 2, 694, 0,         "", "", "",      24, DEFAULT, DEFAULT },
    { doom, 1, 2, 696, 0,         "", "", "", DEFAULT,      72, DEFAULT },
    { doom, 1, 2, 698, 0,         "", "", "",      40, DEFAULT, DEFAULT },
    { doom, 1, 2, 986, 0,         "", "", "",      64,       8, DEFAULT },
    LINEFIX_END
};

static const flow_t flow_doom1_e1m2[] =
{
    { doom, 1, 2,  39, 102 },
    { doom, 1, 2, 141, 101 },
    { doom, 1, 2, 144, 101 },
    { doom, 1, 2, 163, 101 },
    { doom, 1, 2, 175, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E1M3: TOXIN REFINERY
// -----------------------------------------------------------------------------

static const vertexfix_t vertexfix_doom1_e1m3[] =
{
    { doom, 1, 3, 771, -328, -1920, -320, -1920 },
    VERTEXFIX_END
};

static const linefix_t linefix_doom1_e1m3[] =
{
    { doom, 1,  3,   75, 0,      "", "", "",       3, DEFAULT, DEFAULT },
    { doom, 1,  3,  334, 0,      "", "", "",       0, DEFAULT, DEFAULT },
    { doom, 1,  3,  434, 0, "STONE", "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 1,  3,  556, 0,      "", "", "",      72, DEFAULT, DEFAULT },
    { doom, 1,  3,  559, 0,      "", "", "",      56, DEFAULT, DEFAULT },
    { doom, 1,  3, 1014, 0,      "", "", "",     120, DEFAULT, DEFAULT },
    { doom, 1,  3, 1016, 0,      "", "", "",       8, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e1m3[] =
{
    { doom, 1, 3,  1, "",  "FLAT1" },
    { doom, 1, 3,  6, "",  "FLAT1" },
    { doom, 1, 3, 30, "", "FLAT20" },
    { doom, 1, 3, 42, "",  "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e1m3[] =
{
    { doom, 1, 3,  39, 101 },
    { doom, 1, 3,  46, 101 },
    { doom, 1, 3,  38, 101 },
    { doom, 1, 3,  58, 100 },
    { doom, 1, 3,  65, 101 },
    { doom, 1, 3,  66, 101 },
    { doom, 1, 3,  68, 101 },
    { doom, 1, 3, 153, 100 },
    { doom, 1, 3, 155, 100 },
    { doom, 1, 3, 156, 100 },
    { doom, 1, 3, 157, 100 },
    { doom, 1, 3, 158, 100 },
    { doom, 1, 3, 161, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E1M4: COMMAND CONTROL
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e1m4[] =
{
    { doom, 1, 4, 307, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 308, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 321, 0,        "", "", "STEP1", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 1, 4, 327, 0,        "", "", "STEP1", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 1, 4, 338, 0,        "", "", "STEP1", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 1, 4, 346, 0,        "", "", "STEP1", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 1, 4, 383, 0,        "", "",      "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 1, 4, 424, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 433, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 434, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 438, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 439, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 542, 0,        "", "",      "", DEFAULT,     -16,          DEFAULT },
    { doom, 1, 4, 543, 0,        "", "",      "", DEFAULT,     -16,          DEFAULT },
    { doom, 1, 4, 555, 0,        "", "",      "",      40, DEFAULT,          DEFAULT },
    { doom, 1, 4, 559, 0,        "", "",      "",      24, DEFAULT,          DEFAULT },
    { doom, 1, 4, 560, 0,        "", "",      "",       8, DEFAULT,          DEFAULT },
    { doom, 1, 4, 586, 0,        "", "",      "",      32, DEFAULT,          DEFAULT },
    { doom, 1, 4, 613, 0,        "", "",      "",      22, DEFAULT,          DEFAULT },
    { doom, 1, 4, 615, 0,        "", "",      "",      22, DEFAULT,          DEFAULT },
    { doom, 1, 4, 658, 0,        "", "",      "",      32, DEFAULT,          DEFAULT },
    { doom, 1, 4, 693, 1, "BROWN96", "",      "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 1, 4, 696, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 697, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 734, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 735, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 739, 0,        "", "",      "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 1, 4, 742, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 743, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 754, 0,        "", "",      "", DEFAULT,     -60,          DEFAULT },
    { doom, 1, 4, 757, 0,        "", "",      "", DEFAULT,     -60,          DEFAULT },
    { doom, 1, 4, 759, 0,        "", "",      "", DEFAULT,     -60,          DEFAULT },
    { doom, 1, 4, 760, 0,        "", "",      "", DEFAULT,     -60,          DEFAULT },
    { doom, 1, 4, 792, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 4, 793, 0,        "", "",      "", DEFAULT,       4,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e1m4[] =
{
    { doom, 1, 4, 56, "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e1m4[] =
{
    { doom, 1, 4,   5, 100 },
    { doom, 1, 4,   7, 100 },
    { doom, 1, 4,  42, 216 },
    { doom, 1, 4,  95, 212 },
    { doom, 1, 4,  98, 212 },
    { doom, 1, 4, 107, 100 },
    { doom, 1, 4, 110, 212 },
    { doom, 1, 4, 135, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E1M5: PHOBOS LAB
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e1m5[] =
{
    { doom, 1, 5,  37, 0, "", "", "",      -2, DEFAULT,          DEFAULT },
    { doom, 1, 5,  43, 0, "", "", "",      -2, DEFAULT,          DEFAULT },
    { doom, 1, 5,  45, 0, "", "", "",      44, DEFAULT,          DEFAULT },
    { doom, 1, 5,  51, 0, "", "", "",      28,     -16,          DEFAULT },
    { doom, 1, 5,  61, 0, "", "", "",       3, DEFAULT,          DEFAULT },
    { doom, 1, 5,  63, 0, "", "", "",       0, DEFAULT,          DEFAULT },
    { doom, 1, 5,  64, 0, "", "", "",      51, DEFAULT,          DEFAULT },
    { doom, 1, 5,  66, 0, "", "", "",      -3, DEFAULT,          DEFAULT },
    { doom, 1, 5,  67, 0, "", "", "",       0, DEFAULT,          DEFAULT },
    { doom, 1, 5,  68, 0, "", "", "",     -16, DEFAULT,          DEFAULT },
    { doom, 1, 5, 131, 0, "", "", "",     128, DEFAULT,          DEFAULT },
    { doom, 1, 5, 149, 0, "", "", "",      42,     -24,          DEFAULT },
    { doom, 1, 5, 150, 0, "", "", "",     -23,     -24,          DEFAULT },
    { doom, 1, 5, 244, 0, "", "", "",      -2, DEFAULT,          DEFAULT },
    { doom, 1, 5, 250, 0, "", "", "",      -2, DEFAULT,          DEFAULT },
    { doom, 1, 5, 265, 0, "", "", "", DEFAULT,       0, ML_DONTPEGBOTTOM },
    { doom, 1, 5, 270, 0, "", "", "", DEFAULT,       0, ML_DONTPEGBOTTOM },
    { doom, 1, 5, 759, 0, "", "", "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 5, 762, 0, "", "", "", DEFAULT,       4,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e1m5[] =
{
    { doom, 1, 5,   6, "", "FLAT19" },
    { doom, 1, 5,   8, "", "FLAT19" },
    { doom, 1, 5,  10, "", "FLAT19" },
    { doom, 1, 5, 114, "",  "FLAT1" },
    { doom, 1, 5, 117, "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e1m5[] =
{
    { doom, 1, 5, 19, 102 },
    { doom, 1, 5, 33, 232 },
    { doom, 1, 5, 59, 232 },
    { doom, 1, 5, 61, 232 },
    { doom, 1, 5, 72, 102 },
    { doom, 1, 5, 90, 102 },
    { doom, 1, 5, 91, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E1M6: CENTRAL PROCESSING
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e1m6[] =
{
    { doom, 1, 6,   30, 0, "",         "", "",       1, DEFAULT,          DEFAULT },
    { doom, 1, 6,   31, 0, "",         "", "",       2, DEFAULT,          DEFAULT },
    { doom, 1, 6,  147, 0, "", "SUPPORT2", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 1, 6,  148, 0, "", "SUPPORT2", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 1, 6,  631, 0, "",         "", "", DEFAULT,      -3,          DEFAULT },
    { doom, 1, 6,  634, 0, "",         "", "", DEFAULT,      -3,          DEFAULT },
    { doom, 1, 6,  636, 0, "",         "", "", DEFAULT,      -3,          DEFAULT },
    { doom, 1, 6,  663, 0, "",         "", "", DEFAULT,      -3,          DEFAULT },
    { doom, 1, 6,  877, 0, "",         "", "", DEFAULT,     -48,          DEFAULT },
    { doom, 1, 6,  889, 0, "",         "", "", DEFAULT,     -48,          DEFAULT },
    { doom, 1, 6,  894, 0, "",         "", "",       4, DEFAULT,          DEFAULT },
    { doom, 1, 6,  897, 0, "",         "", "",       4, DEFAULT,          DEFAULT },
    { doom, 1, 6,  903, 0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 1, 6,  909, 0, "",         "", "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 6,  912, 0, "",         "", "", DEFAULT,       4,          DEFAULT },
    { doom, 1, 6, 1005, 0, "",         "", "", DEFAULT,     -60,          DEFAULT },
    { doom, 1, 6, 1058, 0, "",         "", "", DEFAULT,     -56,          DEFAULT },
    { doom, 1, 6, 1059, 0, "",         "", "", DEFAULT,     -56,          DEFAULT },
    { doom, 1, 6, 1171, 0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e1m6[] =
{
    { doom, 1, 6, 32, "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e1m6[] =
{
    { doom, 1, 6,  34, 101 },
    { doom, 1, 6,  52, 100 },
    { doom, 1, 6,  57, 101 },
    { doom, 1, 6,  64, 100 },
    { doom, 1, 6,  81, 100 },
    { doom, 1, 6, 118, 101 },
    { doom, 1, 6, 119, 101 },
    { doom, 1, 6, 145, 100 },
    { doom, 1, 6, 146, 100 },
    { doom, 1, 6, 149, 101 },
    { doom, 1, 6, 150, 101 },
    { doom, 1, 6, 153, 101 },
    { doom, 1, 6, 178, 102 },
    { doom, 1, 6, 183, 102 },
    { doom, 1, 6, 184, 102 },
    { doom, 1, 6, 222, 100 },
    { doom, 1, 6, 224, 100 },
    { doom, 1, 6, 225, 100 },
    { doom, 1, 6, 228, 100 },
    { doom, 1, 6, 230, 100 },
    { doom, 1, 6, 231, 100 },
    { doom, 1, 6, 236, 100 },
    { doom, 1, 6, 234, 100 },
    { doom, 1, 6, 237, 100 },
    { doom, 1, 6, 239, 100 },
    { doom, 1, 6, 243, 100 },
    { doom, 1, 6, 245, 100 },
    { doom, 1, 6, 246, 100 },
    { doom, 1, 6, 247, 100 },
    { doom, 1, 6, 249, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E1M7: COMPUTER STATION
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e1m7[] =
{
    { doom, 1, 7,  61, 0,         "", "",         "",     112, DEFAULT, DEFAULT },
    { doom, 1, 7,  63, 0,         "", "",         "",      48, DEFAULT, DEFAULT },
    { doom, 1, 7,  64, 0,         "", "",         "",     112, DEFAULT, DEFAULT },
    { doom, 1, 7, 255, 0,         "", "",         "",       4, DEFAULT, DEFAULT },
    { doom, 1, 7, 450, 1, "COMPTALL", "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 1, 7, 689, 0,         "", "",         "",       4, DEFAULT, DEFAULT },
    { doom, 1, 7, 744, 1,         "", "", "TEKWALL1", DEFAULT, DEFAULT, DEFAULT },
    { doom, 1, 7, 745, 1,         "", "", "TEKWALL1", DEFAULT, DEFAULT, DEFAULT },
    { doom, 1, 7, 746, 1,         "", "", "TEKWALL1", DEFAULT, DEFAULT, DEFAULT },
    { doom, 1, 7, 747, 1,         "", "", "TEKWALL1", DEFAULT, DEFAULT, DEFAULT },
    { doom, 1, 7, 951, 0,         "", "",         "", DEFAULT,       8, DEFAULT },
    { doom, 1, 7, 952, 0,         "", "",         "", DEFAULT,       8, DEFAULT },
    { doom, 1, 7, 954, 0,         "", "",         "",       4, DEFAULT, DEFAULT },
    { doom, 1, 7, 957, 0,         "", "",         "",       4, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e1m7[] =
{
    { doom, 1, 7, 109, "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e1m7[] =
{
    { doom, 1, 7,   3, 102 },
    { doom, 1, 7,  74, 102 },
    { doom, 1, 7, 119, 102 },
    { doom, 1, 7, 120, 102 },
    { doom, 1, 7, 145, 102 },
    { doom, 1, 7, 150, 102 },
    { doom, 1, 7, 151, 102 },
    { doom, 1, 7, 152, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E1M8: PHOBOS ANOMALY
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e1m8[] =
{
    { doom, 1, 8,  44, 0, "", "", "", DEFAULT,     -12, DEFAULT },
    { doom, 1, 8,  45, 0, "", "", "", DEFAULT,     -12, DEFAULT },
    { doom, 1, 8,  74, 0, "", "", "",      23, DEFAULT, DEFAULT },
    { doom, 1, 8,  87, 0, "", "", "",       0,     120, DEFAULT },
    { doom, 1, 8,  88, 0, "", "", "",       0,     120, DEFAULT },
    { doom, 1, 8, 122, 0, "", "", "",     -16, DEFAULT, DEFAULT },
    { doom, 1, 8, 153, 0, "", "", "",      55, DEFAULT, DEFAULT },
    { doom, 1, 8, 154, 0, "", "", "",      47, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// E1M9: MILITARY BASE
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_doom1_e1m9[] =
{
    { doom, 1, 9, 44, "", "FLAT1" },
    { doom, 1, 9, 53, "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e1m9[] =
{
    { doom, 1, 9, 14, 102 },
    { doom, 1, 9, 17, 102 },
    { doom, 1, 9, 21, 102 },
    { doom, 1, 9, 37, 100 },
    { doom, 1, 9, 47, 102 },
    { doom, 1, 9, 48, 102 },
    { doom, 1, 9, 52, 102 },
    { doom, 1, 9, 62, 101 },
    { doom, 1, 9, 63, 101 },
    { doom, 1, 9, 66, 101 },
    { doom, 1, 9, 73, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M1: DEIMOS ANOMALY
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e2m1[] =
{
    { doom, 2, 1,  10, 0, "",       "", "",      23,     -32, DEFAULT },    
    { doom, 2, 1,  48, 0, "",       "", "", DEFAULT,      -4, DEFAULT },
    { doom, 2, 1,  94, 0, "",       "", "", DEFAULT,     -24, DEFAULT },
    { doom, 2, 1,  95, 0, "",       "", "", DEFAULT,     -24, DEFAULT },
    { doom, 2, 1, 113, 0, "",       "", "",      48,      56, DEFAULT },
    { doom, 2, 1, 114, 0, "",       "", "",     112,      56, DEFAULT },
    { doom, 2, 1, 115, 0, "",       "", "",     176,      56, DEFAULT },
    { doom, 2, 1, 116, 0, "",       "", "",      48, DEFAULT, DEFAULT },
    { doom, 2, 1, 174, 0, "",       "", "",      87, DEFAULT, DEFAULT },
    { doom, 2, 1, 182, 0, "",       "", "",      64, DEFAULT, DEFAULT },
    { doom, 2, 1, 239, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 240, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 242, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 243, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 244, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 245, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 246, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 247, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 250, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 251, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 252, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 253, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 254, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 255, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 256, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 257, 0, "",       "", "", DEFAULT,      64, DEFAULT },
    { doom, 2, 1, 321, 0, "",       "", "", DEFAULT,     -24, DEFAULT },
    { doom, 2, 1, 323, 0, "",       "", "", DEFAULT,     -24, DEFAULT },
    { doom, 2, 1, 324, 0, "",       "", "", DEFAULT,     -24, DEFAULT },
    { doom, 2, 1, 382, 0, "",       "", "", DEFAULT,       8, DEFAULT },
    { doom, 2, 1, 383, 0, "", "STONE2", "",      -8,     -88, DEFAULT },
    { doom, 2, 1, 385, 0, "", "STONE2", "",      64,     -88, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m1[] =
{
    { doom, 2, 1, 33, "", "FLAT20" },
    { doom, 2, 1, 87, "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m1[] =
{
    { doom, 2, 1, 79, 101 },
    { doom, 2, 1, 81, 101 },
    { doom, 2, 1, 82, 101 },
    { doom, 2, 1, 88, 101 },
    { doom, 2, 1, 90, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M2: CONTAINMENT AREA
// -----------------------------------------------------------------------------

static const vertexfix_t vertexfix_doom1_e2m2[] =
{
    { doom, 2, 2,  964, 2272, 1896, 2272, 1888 },
    { doom, 2, 2,  974, 2152, 1896, 2152, 1888 },
    { doom, 2, 2,  976, 2048, 1896, 2048, 1888 },
    { doom, 2, 2,  995, 1984, 1840, 1984, 1832 },
    { doom, 2, 2, 1344, 1312, 3992, 1312, 4000 },
    VERTEXFIX_END
};

static const linefix_t linefix_doom1_e2m2[] =
{
    { doom, 2, 2,   20, 0,         "",      "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 2, 2,   23, 0,         "",      "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 2, 2,   25, 0,         "",      "",         "",      80, DEFAULT,       DEFAULT },
    { doom, 2, 2,   26, 0,         "",      "",         "",     120, DEFAULT,       DEFAULT },
    { doom, 2, 2,   50, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   52, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   53, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   55, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   56, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   58, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   59, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   60, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   62, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,   64, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  125, 0,         "",      "",         "",     -32, DEFAULT,       DEFAULT },
    { doom, 2, 2,  412, 0,         "",      "",         "",      64, DEFAULT,       DEFAULT },
    { doom, 2, 2,  627, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  628, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  638, 0,         "",      "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 2, 2,  642, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  658, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  660, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  662, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  664, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  673, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  675, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  680, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  670, 0,         "",      "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 2, 2,  722, 1,         "",      "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 2, 2,  760, 0,         "",      "",  "GRAYBIG", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 2, 2,  947, 0,   "BROWN1",      "",         "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1251, 0,         "", "LITE3",         "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1253, 0,         "", "LITE3",         "",       9, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1255, 0,         "",      "",         "",      10, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1281, 0,         "",      "",         "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 2, 2, 1451, 0,         "",      "",         "",      88, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1494, 1, "ICKWALL2",      "",         "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1505, 0,         "",      "",         "", DEFAULT,      32,       DEFAULT },
    { doom, 2, 2, 1506, 0,         "",      "",         "", DEFAULT,      32,       DEFAULT },
    { doom, 2, 2, 1520, 0,         "",      "",         "",      96, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1521, 0,         "",      "",         "",      72, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1524, 0,         "",      "",         "",     104, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1527, 0,         "",      "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1528, 0,         "",      "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1529, 0,         "",      "",         "",      72, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1533, 0,         "",      "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1534, 0,         "",      "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 2, 2, 1596, 1,    "WOOD1",      "",         "", DEFAULT, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m2[] =
{
    { doom, 2, 2,  8, "",  "FLAT1" },
    { doom, 2, 2, 11, "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m2[] =
{
    { doom, 2, 2,   0, 202 },
    { doom, 2, 2,   1, 202 },
    { doom, 2, 2,  14, 202 },
    { doom, 2, 2,  15, 202 },
    { doom, 2, 2,  71, 101 },
    { doom, 2, 2,  72, 101 },
    { doom, 2, 2,  73, 101 },
    { doom, 2, 2,  74, 101 },
    { doom, 2, 2,  75, 101 },
    { doom, 2, 2,  76, 101 },
    { doom, 2, 2,  79, 101 },
    { doom, 2, 2,  80, 101 },
    { doom, 2, 2, 104, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M3: REFINERY
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e2m3[] =
{
    { doom, 2, 3,  68, 0, "",        "", "",      16, DEFAULT,          DEFAULT },
    { doom, 2, 3, 124, 0, "",        "", "",     176, DEFAULT,          DEFAULT },
    { doom, 2, 3, 128, 0, "",        "", "",     -48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 130, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 135, 0, "",        "", "",      64, DEFAULT,          DEFAULT },
    { doom, 2, 3, 136, 0, "",        "", "",      16, DEFAULT,          DEFAULT },
    { doom, 2, 3, 138, 0, "",        "", "",     -16, DEFAULT,          DEFAULT },
    { doom, 2, 3, 142, 0, "",        "", "",      64, DEFAULT,          DEFAULT },
    { doom, 2, 3, 196, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 199, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 204, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 207, 0, "",        "", "",     112, DEFAULT,          DEFAULT },
    { doom, 2, 3, 211, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 215, 0, "",        "", "",     -48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 219, 0, "",        "", "",     112, DEFAULT,          DEFAULT },
    { doom, 2, 3, 222, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 267, 0, "",        "", "", DEFAULT,      -8,          DEFAULT },
    { doom, 2, 3, 268, 0, "",        "", "", DEFAULT,      -8,          DEFAULT },
    { doom, 2, 3, 270, 0, "",        "", "", DEFAULT,      -8,          DEFAULT },
    { doom, 2, 3, 271, 0, "",        "", "", DEFAULT,      -8,          DEFAULT },
    { doom, 2, 3, 327, 0, "",        "", "", DEFAULT, DEFAULT,    ML_DONTPEGTOP },
    { doom, 2, 3, 344, 0, "",        "", "",     148,     -88,          DEFAULT },
    { doom, 2, 3, 350, 0, "",        "", "",      32, DEFAULT,          DEFAULT },
    { doom, 2, 3, 373, 0, "",        "", "",    -127, DEFAULT,          DEFAULT },
    { doom, 2, 3, 388, 0, "",        "", "",     136, DEFAULT,          DEFAULT },
    { doom, 2, 3, 396, 0, "",        "", "",     136, DEFAULT,          DEFAULT },
    { doom, 2, 3, 397, 0, "",        "", "",     136, DEFAULT,          DEFAULT },
    { doom, 2, 3, 400, 0, "",        "", "",     136, DEFAULT,          DEFAULT },
    { doom, 2, 3, 478, 0, "",        "", "", DEFAULT,      32,          DEFAULT },
    { doom, 2, 3, 479, 0, "",        "", "", DEFAULT,      32,          DEFAULT },
    { doom, 2, 3, 480, 0, "",        "", "", DEFAULT,      32,          DEFAULT },
    { doom, 2, 3, 481, 0, "",        "", "", DEFAULT,      32,          DEFAULT },
    { doom, 2, 3, 510, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 518, 0, "",        "", "", DEFAULT,      32,          DEFAULT },
    { doom, 2, 3, 563, 0, "",        "", "",      48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 567, 0, "",        "", "",      -8, DEFAULT,          DEFAULT },
    { doom, 2, 3, 568, 0, "",        "", "",     -48, DEFAULT,          DEFAULT },
    { doom, 2, 3, 578, 0, "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 2, 3, 662, 0, "",        "", "", DEFAULT, DEFAULT,    ML_DONTPEGTOP },
    { doom, 2, 3, 663, 0, "",        "", "",     144, DEFAULT,          DEFAULT },
    { doom, 2, 3, 713, 0, "",        "", "",      64, DEFAULT,          DEFAULT },
    { doom, 2, 3, 795, 0, "",        "", "", DEFAULT, DEFAULT,    ML_DONTPEGTOP },
    { doom, 2, 3, 844, 0, "",        "", "",       0, DEFAULT,          DEFAULT },
    { doom, 2, 3, 871, 0, "",        "", "", DEFAULT,      -8,          DEFAULT },
    { doom, 2, 3, 872, 0, "",        "", "", DEFAULT,      -8,          DEFAULT },
    { doom, 2, 3, 905, 1, "", "MIDBRN1", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 2, 3, 906, 1, "", "MIDBRN1", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 2, 3, 907, 1, "", "MIDBRN1", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 2, 3, 908, 1, "", "MIDBRN1", "", DEFAULT, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m3[] =
{
    { doom, 2, 3, 104, "", "FLAT5_5" },
    { doom, 2, 3, 111, "",   "FLAT1" },
    { doom, 2, 3, 112, "",   "FLAT1" },
    { doom, 2, 3, 115, "",  "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m3[] =
{
    { doom, 2, 3,  11, 101 },
    { doom, 2, 3,  15, 101 },
    { doom, 2, 3,  16, 101 },
    { doom, 2, 3,  34, 100 },
    { doom, 2, 3,  46, 101 },
    { doom, 2, 3,  54, 101 },
    { doom, 2, 3,  99, 101 },
    { doom, 2, 3, 116, 102 },
    { doom, 2, 3, 120, 102 },
    { doom, 2, 3, 122, 102 },
    { doom, 2, 3, 123, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M4: DEIMOS LAB
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e2m4[] =
{
    { doom, 2, 4,  257, 0,         "",         "",      "",     -32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  258, 0,         "",         "",      "",      32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  259, 0,         "",         "",      "",     -32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  303, 0,         "",         "",      "", DEFAULT,       8,                            DEFAULT },
    { doom, 2, 4,  304, 0,         "",         "",      "", DEFAULT,       8,                            DEFAULT },
    { doom, 2, 4,  329, 0,         "",         "",      "",       0, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  335, 0,         "",         "",      "",      35, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 2, 4,  346, 0,         "",         "",      "",      21, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  347, 0,         "",         "",      "",      21, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  350, 0,         "",         "",      "",      21, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  385, 0,         "",         "",      "",      38, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  367, 0,         "",         "",      "", DEFAULT,      24,                            DEFAULT },
    { doom, 2, 4,  368, 0,         "",         "",      "", DEFAULT,      24,                            DEFAULT },
    { doom, 2, 4,  369, 0,         "",         "",      "", DEFAULT,      24,                            DEFAULT },
    { doom, 2, 4,  374, 0,         "",         "",      "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 2, 4,  390, 0,         "",         "",      "",     -26, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  408, 0,         "",         "",      "",      21, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  409, 0,         "",         "",      "",      21, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  485, 0,         "",         "",      "",     -32,      24,                            DEFAULT },
    { doom, 2, 4,  498, 0,         "",         "",      "", DEFAULT,      24,                            DEFAULT },
    { doom, 2, 4,  551, 1,    "PIPE4",         "",      "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  559, 0,         "",         "",      "",     -32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  560, 0,         "",         "",      "",      32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  567, 0,         "",         "",      "",      53, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  568, 0,         "",         "",      "",      21, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  570, 0,         "",         "",      "",     -32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  571, 0,         "",         "",      "",     -32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  574, 0,         "",         "",      "",      21, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  644, 0,         "",         "",      "",     -32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  645, 0,         "",         "",      "",     -32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  650, 0,         "",         "",      "",      32, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  680, 1,         "",         "",      "", DEFAULT,      -8,                   ML_DONTPEGBOTTOM },
    { doom, 2, 4,  746, 0,         "",         "",      "",      64, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  748, 0,         "",         "",      "",     -40, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  751, 0,         "",         "",      "",     -48, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  834, 0,         "",         "",      "",     -85, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  836, 0,         "",         "",      "",      27, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  848, 0,         "",         "",      "",     -26, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  865, 1,         "",         "", "STEP5", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  866, 0,         "",         "",      "",     -64,      -8,                            DEFAULT },
    { doom, 2, 4,  876, 0,         "",         "",      "",      91,      -8,                            DEFAULT },
    { doom, 2, 4,  955, 1,         "", "MIDGRATE",      "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 2, 4,  978, 0,         "",         "",      "",     -64, DEFAULT,                            DEFAULT },
    { doom, 2, 4, 1031, 0,         "",         "",      "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom, 2, 4, 1043, 0,         "",         "",      "", DEFAULT,       0,                            DEFAULT },
    { doom, 2, 4, 1057, 1,         "",         "",      "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom, 2, 4, 1062, 0, "SLADWALL",         "",      "",      32, DEFAULT,                            DEFAULT },
    { doom, 2, 4, 1071, 0, "GRAYTALL",         "",      "", DEFAULT, DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m4[] =
{
    { doom, 2, 4,  51,       "", "FLAT5_3" },
    { doom, 2, 4, 151, "FLAT14",  "FLAT14" },
    { doom, 2, 4, 170,       "",  "FLAT20" },
    { doom, 2, 4, 173, "BLOOD3",   "FLAT1" },
    { doom, 2, 4, 177, "BLOOD3",   "FLAT1" },
    { doom, 2, 4, 202,       "",   "FLAT1" },
    { doom, 2, 4, 204,       "",   "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m4[] =
{
    { doom, 2, 4,  12, 101 },
    { doom, 2, 4,  35, 101 },
    { doom, 2, 4,  37, 100 },
    { doom, 2, 4,  51, 100 },
    { doom, 2, 4, 111, 102 },
    { doom, 2, 4, 156, 102 },
    { doom, 2, 4, 162, 101 },
    { doom, 2, 4, 163, 101 },
    { doom, 2, 4, 173, 102 },
    { doom, 2, 4, 174, 102 },
    { doom, 2, 4, 175, 102 },
    { doom, 2, 4, 177, 102 },
    { doom, 2, 4, 178, 102 },
    { doom, 2, 4, 179, 102 },
    { doom, 2, 4, 180, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M5: COMMAND CENTER
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e2m5[] =
{
    { doom, 2, 5,  370, 0,         "", "",      "",      48, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 2, 5,  371, 0,         "", "",      "",      44, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 2, 5,  380, 0,         "", "",      "",      45, DEFAULT,          DEFAULT },
    { doom, 2, 5,  381, 0,         "", "",      "",      29, DEFAULT,          DEFAULT },
    { doom, 2, 5,  590, 0,         "", "", "STEP1", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 2, 5,  590, 1,  "BROVINE", "",      "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 2, 5,  958, 0,         "", "",      "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 2, 5,  960, 0,         "", "",      "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 2, 5,  967, 0,         "", "",      "",      22, DEFAULT,          DEFAULT },
    { doom, 2, 5,  971, 0,         "", "",      "",      22, DEFAULT,          DEFAULT },
    { doom, 2, 5, 1027, 1, "COMPSPAN", "",      "", DEFAULT, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m5[] =
{
    { doom, 2, 5, 114,      "",   "FLAT1" },
    { doom, 2, 5, 119,      "",   "FLAT1" },
    { doom, 2, 5, 127,      "", "FLAT5_4" },
    { doom, 2, 5, 220,      "", "FLAT5_4" },
    { doom, 2, 5, 235, "GATE6",   "GATE6" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m5[] =
{
    { doom, 2, 5,  49, 222 },
    { doom, 2, 5,  63, 102 },
    { doom, 2, 5,  64, 102 },
    { doom, 2, 5,  72, 102 },
    { doom, 2, 5, 134, 102 },
    { doom, 2, 5, 135, 102 },
    { doom, 2, 5, 139, 102 },
    { doom, 2, 5, 162, 102 },
    { doom, 2, 5, 177, 102 },
    { doom, 2, 5, 212, 102 },
    { doom, 2, 5, 232, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M6: HALLS OF THE DAMNED
// -----------------------------------------------------------------------------

static const vertexfix_t vertexfix_doom1_e2m6[] =
{
    { doom, 2, 6, 646,   64, 2008,   66, 2006 },
    { doom, 2, 6, 647,  112, 1960,  111, 1961 },
    { doom, 2, 6, 648, -368, 1960, -367, 1961 },
    { doom, 2, 6, 649, -320, 2008, -322, 2006 },
    VERTEXFIX_END
};

static const linefix_t linefix_doom1_e2m6[] =
{
    { doom, 2, 6,    2, 0,         "", "",      "",     112,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,   26, 0,         "", "",      "",      16,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,   35, 0,         "", "",      "", DEFAULT,  DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom, 2, 6,   40, 0,         "", "",      "",      34,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,   42, 0,         "", "",      "",     -18,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,   44, 0,         "", "",      "",     -18,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,   45, 0,         "", "",      "",     160,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  460, 0,         "", "",      "",     -14,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  461, 0,         "", "",      "",      -8,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  462, 0,         "", "",      "",     -12,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  463, 0,         "", "",      "",     -14,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  463, 0,         "", "",      "",      -9,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  465, 0,         "", "",      "",      52,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  475, 0,         "", "",      "",      10,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  478, 0,         "", "",      "",      12,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  479, 0,         "", "",      "",       6,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  482, 0,         "", "",      "",       8,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  499, 0,         "", "",      "",       8,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  501, 0,         "", "",      "",      10,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  505, 0,         "", "",      "",       6,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  601, 0,         "", "",      "",       4,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  603, 0,         "", "",      "",       4,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  619, 0,         "", "",      "", DEFAULT,  DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 2, 6,  639, 0, "EXITSIGN", "",      "",      -8,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  640, 0, "EXITSIGN", "",      "",      -8,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  662, 0,         "", "",      "", DEFAULT,      -40, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 2, 6,  675, 0, "COMPBLUE", "",      "", DEFAULT,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  747, 0,         "", "",      "",       0,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  750, 0,         "", "",      "",       0,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  757, 0,         "", "",      "",      -1,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  756, 0,         "", "",      "",      -1,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  758, 0,         "", "",      "",      -1,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  779, 0,         "", "",      "",      -1,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  986, 1,         "", "", "STEP4", DEFAULT,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  987, 1,         "", "", "STEP4", DEFAULT,  DEFAULT,                            DEFAULT },
    { doom, 2, 6,  988, 1,         "", "", "STEP4", DEFAULT,  DEFAULT,                            DEFAULT },
    { doom, 2, 6, 1000, 0,         "", "",      "", DEFAULT,       -8,                            DEFAULT },
    { doom, 2, 6, 1005, 0,         "", "",      "", DEFAULT,       -8,                            DEFAULT },
    { doom, 2, 6, 1011, 0,         "", "",      "", DEFAULT,       -8,                            DEFAULT },
    { doom, 2, 6, 1012, 0,         "", "",      "", DEFAULT,       -8,                            DEFAULT },
    { doom, 2, 6, 1091, 1,    "STEP4", "",      "", DEFAULT,  DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m6[] =
{
    { doom, 2, 6,  20, "FLOOR5_4",       "" },
    { doom, 2, 6,  91,   "FLAT19",       "" },
    { doom, 2, 6,  92,   "FLAT19",       "" },
    { doom, 2, 6,  93,   "FLAT19",       "" },
    { doom, 2, 6, 161,         "",  "FLAT1" },
    { doom, 2, 6, 162,         "", "FLAT20" },
    { doom, 2, 6, 164,   "FLAT14",       "" },
    { doom, 2, 6, 165,         "", "FLAT20" },
    { doom, 2, 6, 170,         "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m6[] =
{
    { doom, 2, 6, 55, 102 },
    { doom, 2, 6, 56, 102 },
    { doom, 2, 6, 57, 102 },
    { doom, 2, 6, 59, 102 },
    { doom, 2, 6, 62, 102 },
    { doom, 2, 6, 63, 102 },
    { doom, 2, 6, 64, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M7: SPAWNING VATS
// -----------------------------------------------------------------------------

static const vertexfix_t vertexfix_doom1_e2m7[] =
{
    { doom, 2, 7, 255, 2880,  624, 2880,  629 },
    { doom, 2, 7, 378, 3824,  640, 3829,  640 },
    { doom, 2, 7, 379, 3840,  624, 3840,  629 },
    { doom, 2, 7, 381, 2896,  640, 2891,  640 },
    { doom, 2, 7, 393, 1944, 1024, 1952, 1024 },
    { doom, 2, 7, 495, 1944,  960, 1952,  960 },
    VERTEXFIX_END
};

static const linefix_t linefix_doom1_e2m7[] =
{
    { doom, 2, 7,   24, 0, "", "DOORSTOP",       "", DEFAULT,       4,                             DEFAULT },
    { doom, 2, 7,   30, 0, "", "DOORSTOP",       "", DEFAULT,       4,                             DEFAULT },
    { doom, 2, 7,   38, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,   40, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,   42, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,   45, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,   48, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  197, 0, "",         "",       "", DEFAULT, DEFAULT,                    ML_DONTPEGBOTTOM },
    { doom, 2, 7,  198, 0, "",         "",       "", DEFAULT,      24,                             DEFAULT },
    { doom, 2, 7,  205, 1, "",         "",       "",       0, DEFAULT,                             DEFAULT },
    { doom, 2, 7,  209, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  216, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  225, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  230, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  235, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  285, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  315, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  345, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  346, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  347, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  349, 0, "",         "",       "", DEFAULT,       1,                             DEFAULT },
    { doom, 2, 7,  355, 0, "",         "",       "", DEFAULT,      -8,                             DEFAULT },
    { doom, 2, 7,  357, 0, "",         "",       "", DEFAULT,      -8,                             DEFAULT },
    { doom, 2, 7,  359, 0, "",         "",       "", DEFAULT,      56,                             DEFAULT },
    { doom, 2, 7,  428, 0, "",         "",       "", DEFAULT, DEFAULT,                             DEFAULT },
    { doom, 2, 7,  467, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  468, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  469, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  470, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  484, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  485, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  486, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  491, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  492, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  493, 0, "",         "",       "", DEFAULT,       56,                            DEFAULT },
    { doom, 2, 7,  673, 0, "",         "",       "", DEFAULT,       36,                            DEFAULT },
    { doom, 2, 7,  674, 0, "",         "",       "", DEFAULT,       36,                            DEFAULT },
    { doom, 2, 7,  675, 0, "",         "",       "", DEFAULT,       36,                            DEFAULT },
    { doom, 2, 7,  676, 0, "",         "",       "", DEFAULT,       36,                            DEFAULT },
    { doom, 2, 7,  677, 0, "",         "",       "", DEFAULT,       36,                            DEFAULT },
    { doom, 2, 7,  678, 0, "",         "",       "", DEFAULT,       36,                            DEFAULT },
    { doom, 2, 7,  700, 0, "",         "",       "",     -56,  DEFAULT,                            DEFAULT },
    { doom, 2, 7,  701, 0, "",         "",       "",     -48,  DEFAULT,                            DEFAULT },
    { doom, 2, 7,  706, 0, "",         "",       "", DEFAULT,  DEFAULT,                      ML_DONTPEGTOP },
    { doom, 2, 7,  735, 0, "", "DOORSTOP",       "", DEFAULT,        4,                            DEFAULT },
    { doom, 2, 7,  738, 0, "", "DOORSTOP",       "", DEFAULT,        4,                            DEFAULT },
    { doom, 2, 7,  741, 0, "", "DOORSTOP",       "", DEFAULT,        4,                            DEFAULT },
    { doom, 2, 7,  744, 0, "", "DOORSTOP",       "", DEFAULT,        4,                            DEFAULT },
    { doom, 2, 7,  885, 0, "",         "",       "", DEFAULT,        7,                            DEFAULT },
    { doom, 2, 7,  904, 0, "",         "",       "",      88,  DEFAULT,                            DEFAULT },
    { doom, 2, 7,  962, 0, "",         "",       "", DEFAULT,  DEFAULT,                      ML_DONTPEGTOP },
    { doom, 2, 7, 1181, 0, "", "DOORSTOP",       "", DEFAULT,        8,                            DEFAULT },
    { doom, 2, 7, 1184, 0, "", "DOORSTOP",       "", DEFAULT,        8,                            DEFAULT },
    { doom, 2, 7, 1249, 0, "",         "",       "",     192,  DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom, 2, 7, 1250, 0, "",         "",       "", DEFAULT,  DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom, 2, 7, 1256, 0, "", "DOORSTOP",       "", DEFAULT,        8,                            DEFAULT },
    { doom, 2, 7, 1260, 0, "", "DOORSTOP",       "", DEFAULT,        8,                            DEFAULT },
    { doom, 2, 7, 1286, 0, "",         "", "SHAWN2", DEFAULT,  DEFAULT,                            DEFAULT },
    { doom, 2, 7, 1362, 0, "",         "",       "", DEFAULT,       40,                            DEFAULT },
    { doom, 2, 7, 1363, 0, "",         "",       "",      64,       40,                            DEFAULT },
    { doom, 2, 7, 1364, 0, "",         "",       "",    -209,       40,                            DEFAULT },
    { doom, 2, 7, 1365, 0, "",         "",       "",     -17,       40,                            DEFAULT },
    { doom, 2, 7, 1366, 0, "",         "",       "",    -182,       40,                            DEFAULT },
    { doom, 2, 7, 1367, 0, "",         "",       "",    -118,       40,                            DEFAULT },
    { doom, 2, 7, 1368, 0, "",         "",       "",     -27,       40,                            DEFAULT },
    { doom, 2, 7, 1369, 0, "",         "",       "",     -91,       40,                            DEFAULT },
    { doom, 2, 7, 1418, 0, "",         "",       "", DEFAULT,  DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 2, 7, 1619, 0, "",         "",       "",     -40,  DEFAULT,                            DEFAULT },
    { doom, 2, 7, 1621, 0, "",         "",       "",      64,  DEFAULT,                            DEFAULT },
    { doom, 2, 7, 1645, 0, "",         "",       "",      32,  DEFAULT,                            DEFAULT },
    { doom, 2, 7, 1662, 0, "",         "",       "",      32,  DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m7[] =
{
    { doom, 2, 7,  32,        "",  "FLAT19" },
    { doom, 2, 7,  33,        "",  "FLAT19" },
    { doom, 2, 7,  37,        "",  "FLAT19" },
    { doom, 2, 7,  38,        "", "FLAT5_4" },
    { doom, 2, 7,  57,        "",   "FLAT1" },
    { doom, 2, 7,  60,        "",  "FLAT20" },
    { doom, 2, 7,  77,        "", "CEIL3_4" },
    { doom, 2, 7, 251, "CRATOP2",        "" },
    { doom, 2, 7, 303,        "", "CEIL5_2" },
    { doom, 2, 7, 309,        "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m7[] =
{
    { doom, 2, 7,   0, 101 },
    { doom, 2, 7,  29, 101 },
    { doom, 2, 7,  30, 101 },
    { doom, 2, 7,  31, 101 },
    { doom, 2, 7,  47, 101 },
    { doom, 2, 7,  49, 100 },
    { doom, 2, 7,  53, 100 },
    { doom, 2, 7,  61, 100 },
    { doom, 2, 7,  62, 100 },
    { doom, 2, 7,  74, 100 },
    { doom, 2, 7,  75, 100 },
    { doom, 2, 7,  76, 100 },
    { doom, 2, 7, 155, 101 },
    { doom, 2, 7, 156, 101 },
    { doom, 2, 7, 279, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E2M9: FORTRESS OF MYSTERY
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e2m9[] =
{
    { doom, 2, 9,  68, 0,        "",         "", "",      15,   DEFAULT,          DEFAULT },
    { doom, 2, 9,  70, 0,        "",         "", "",     -24,   DEFAULT,          DEFAULT },
    { doom, 2, 9,  85, 0,        "",         "", "",     -24,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 110, 1,        "", "MIDGRATE", "",      32,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 115, 1,        "", "MIDGRATE", "",      32,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 121, 1, "GSTONE1",         "", "", DEFAULT,   DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 2, 9, 122, 0,        "",         "", "",     -32,       -88,          DEFAULT },
    { doom, 2, 9, 123, 1, "GSTONE1",         "", "", DEFAULT,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 124, 0,        "",         "", "",      88,       -88,          DEFAULT },
    { doom, 2, 9, 140, 1, "GSTONE1",         "", "", DEFAULT,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 158, 0,        "",         "", "",      64,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 159, 0,        "",         "", "",     112,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 160, 0,        "",         "", "",     -72,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 161, 0,        "",         "", "",     -40,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 163, 0,        "",         "", "",      56,   DEFAULT,          DEFAULT },
    { doom, 2, 9, 165, 0,        "",         "", "",     -24,   DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e2m9[] =
{
    { doom, 2, 9,  0, "SWATER1",      "" },
    { doom, 2, 9, 16, "GATE6",   "GATE6" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e2m9[] =
{
    { doom, 2, 9, 0, 100 },
    { doom, 2, 9, 1, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M1: HELL KEEP
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m1[] =
{
    { doom, 3, 1,  8, 0, "", "", "",  96, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 3, 1, 17, 0, "", "", "",  96,      48,          DEFAULT },
    { doom, 3, 1, 18, 0, "", "", "", 208,      48,          DEFAULT },
    LINEFIX_END
};

static const flow_t flow_doom1_e3m1[] =
{
    { doom, 3, 1,  0, 102 },
    { doom, 3, 1,  7, 102 },
    { doom, 3, 1, 14, 102 },
    { doom, 3, 1, 15, 102 },
    { doom, 3, 1, 16, 102 },
    { doom, 3, 1, 17, 102 },
    { doom, 3, 1, 18, 102 },
    { doom, 3, 1, 19, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M2: SLOUGH OF DESPAIR
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m2[] =
{
    { doom,  3, 2, 146, 1, "", "MIDVINE1",        "", DEFAULT, DEFAULT,          DEFAULT },
    { doom,  3, 2, 159, 0, "",         "",        "", DEFAULT, DEFAULT,      ML_DONTDRAW },
    { doom,  3, 2, 302, 0, "",         "",        "",      16, DEFAULT,          DEFAULT },
    { doom,  3, 2, 303, 0, "",         "",        "",      96, DEFAULT,          DEFAULT },
    { doom,  3, 2, 304, 0, "",         "",        "",      40, DEFAULT,          DEFAULT },
    { doom,  3, 2, 312, 0, "",         "",        "",      56, DEFAULT,          DEFAULT },
    { doom,  3, 2, 316, 0, "",         "",        "",     120, DEFAULT,          DEFAULT },
    { doom,  3, 2, 320, 0, "",         "",        "",      56, DEFAULT,          DEFAULT },
    { doom,  3, 2, 324, 0, "",         "",        "",     112, DEFAULT,          DEFAULT },
    { doom,  3, 2, 361, 0, "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom,  3, 2, 363, 0, "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom,  3, 2, 414, 0, "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom,  3, 2, 415, 0, "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom,  3, 2, 433, 1, "",         "", "ASHWALL", DEFAULT, DEFAULT,          DEFAULT },
    { doom,  3, 2, 434, 1, "",         "", "ASHWALL", DEFAULT, DEFAULT,          DEFAULT },
    { doom,  3, 2, 435, 1, "",         "", "ASHWALL", DEFAULT, DEFAULT,          DEFAULT },
    { doom,  3, 2, 436, 1, "",         "", "ASHWALL", DEFAULT, DEFAULT,          DEFAULT },
    { doom,  3, 2, 452, 0, "",         "",        "", DEFAULT,      -8,          DEFAULT },
    { doom,  3, 2, 454, 0, "",         "",        "", DEFAULT,      -8,          DEFAULT },
    { doom,  3, 2, 463, 0, "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom,  3, 2, 464, 0, "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e3m2[] =
{
    { doom, 3, 2, 26, "SWATER2",       "" },
    { doom, 3, 2, 40,        "", "FLAT19" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e3m2[] =
{
    { doom, 3, 2,  3, 100 },
    { doom, 3, 2, 12, 100 },
    { doom, 3, 2, 17, 210 },
    { doom, 3, 2, 20, 210 },
    { doom, 3, 2, 23, 100 },
    { doom, 3, 2, 26, 101 },
    { doom, 3, 2, 44, 101 },
    { doom, 3, 2, 66, 101 },
    { doom, 3, 2, 67, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M3: PANDEMONIUM
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m3[] =
{
    { doom, 3, 3,   31, 0,         "",         "", "",      24,  DEFAULT,     DEFAULT },
    { doom, 3, 3,   33, 0,         "",         "", "",      40,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  645, 0,         "",         "", "",       4,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  648, 0,         "",         "", "",       4,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  790, 0,         "",         "", "", DEFAULT,       -8,     DEFAULT },
    { doom, 3, 3,  791, 0,         "",         "", "", DEFAULT,       -8,     DEFAULT },
    { doom, 3, 3,  793, 0,         "",         "", "", DEFAULT,       -8,     DEFAULT },
    { doom, 3, 3,  797, 0,         "",         "", "", DEFAULT,       -8,     DEFAULT },
    { doom, 3, 3,  851, 0,         "",         "", "",      24,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  853, 0,         "",         "", "",      40,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  854, 1,         "", "MIDGRATE", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  855, 1,         "", "MIDGRATE", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  994, 1, "SLADWALL", "MIDGRATE", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  995, 1,         "", "MIDGRATE", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3,  996, 1,         "", "MIDGRATE", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3, 1055, 0,         "",   "NUKE24", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3, 1056, 0,         "",   "NUKE24", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3, 1057, 0,         "",   "NUKE24", "", DEFAULT,  DEFAULT,     DEFAULT },
    { doom, 3, 3, 1099, 0,         "", "BRNSMALC", "",     144,  DEFAULT,     DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e3m3[] =
{
    { doom, 3, 3, 172, "",  "FLAT1" },
    { doom, 3, 3, 176, "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e3m3[] =
{
    { doom, 3, 3,  11, 212 },
    { doom, 3, 3,  12, 212 },
    { doom, 3, 3,  13, 101 },
    { doom, 3, 3,  15, 101 },
    { doom, 3, 3,  51, 100 },
    { doom, 3, 3,  64, 101 },
    { doom, 3, 3,  70, 101 },
    { doom, 3, 3, 113, 102 },
    { doom, 3, 3, 145, 102 },
    { doom, 3, 3, 157, 101 },
    { doom, 3, 3, 161, 101 },
    { doom, 3, 3, 183, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M4: HOUSE OF PAIN
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m4[] =
{
    { doom, 3, 4,    4, 0,         "", "", "",      16,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,   16, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,   92, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,   93, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,   94, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,   95, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,  128, 0,         "", "", "",    -158,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  185, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  186, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  187, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  236, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  237, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  238, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  239, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  240, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  241, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  242, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  243, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  244, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4,  411, 0,         "", "", "",     155,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  412, 0,         "", "", "",      11,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  454, 0,         "", "", "",     171,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  458, 0,         "", "", "",      27,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  470, 0, "BIGDOOR2", "", "", DEFAULT,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  475, 0,         "", "", "",    -112,       -56,                            DEFAULT },
    { doom, 3, 4,  513, 0,         "", "", "",      13,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  524, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  528, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  529, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  562, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  565, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  572, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  574, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  581, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  582, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  589, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  590, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  606, 0,         "", "", "",      16,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  607, 0,         "", "", "",      56,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  608, 0,         "", "", "",     240,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  609, 0,         "", "", "",     112,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  610, 0,         "", "", "",      40,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  611, 0,         "", "", "",      64,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  623, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,  624, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,  632, 0,         "", "", "",     -72,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  648, 0,         "", "", "",       8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  649, 0,         "", "", "",     -80,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  650, 0,         "", "", "",    -168,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  651, 0,         "", "", "",     -24,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  652, 0,         "", "", "",    -112,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  657, 0,         "", "", "", DEFAULT,        -8,                            DEFAULT },
    { doom, 3, 4,  659, 0,         "", "", "",       0,        -8,                            DEFAULT },
    { doom, 3, 4,  660, 0,         "", "", "",       0,        -8,                            DEFAULT },
    { doom, 3, 4,  662, 0,         "", "", "", DEFAULT,        -8,                            DEFAULT },
    { doom, 3, 4,  665, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  666, 0,         "", "", "",      12,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  667, 0,         "", "", "", DEFAULT,        -8,                            DEFAULT },
    { doom, 3, 4,  668, 0,         "", "", "", DEFAULT,        -8,                            DEFAULT },
    { doom, 3, 4,  674, 0,         "", "", "",       4,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  675, 0,         "", "", "",       4,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  681, 0,         "", "", "",      96,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  682, 0,         "", "", "",      16,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  683, 0,         "", "", "",     120,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  684, 0,         "", "", "",     248,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  696, 0,         "", "", "",     -16,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  697, 0,         "", "", "",      72,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  768, 0,         "", "", "",      72,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  773, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,  774, 0,         "", "", "", DEFAULT,   DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 4,  776, 0,         "", "", "",     -23,         8,                            DEFAULT },
    { doom, 3, 4,  778, 0,         "", "", "",      23,         8,                            DEFAULT },
    { doom, 3, 4,  781, 0,         "", "", "",     -23,         8,                            DEFAULT },
    { doom, 3, 4,  783, 0,         "", "", "",      11,         8,                            DEFAULT },
    { doom, 3, 4,  791, 0,         "", "", "",      44,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  841, 0,         "", "", "",      64,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  859, 0,         "", "", "", DEFAULT,   DEFAULT,                      ML_DONTPEGTOP },
    { doom, 3, 4,  873, 0,         "", "", "",      96,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  876, 0,         "", "", "",     144,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  878, 0,         "", "", "",      -8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  882, 0,         "", "", "",      -8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  891, 0,         "", "", "",      -8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  898, 0,         "", "", "",     192,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  900, 0,         "", "", "",      -8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  907, 0,         "", "", "",     192,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  909, 0,         "", "", "",      -8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  944, 0,         "", "", "",      -8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  953, 0,         "", "", "",      64,   DEFAULT,                            DEFAULT },
    { doom, 3, 4,  955, 0,         "", "", "",      -8,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1015, 0,         "", "", "",       4,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1016, 0,         "", "", "",       4,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1019, 0,         "", "", "",       4,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1035, 0,         "", "", "",      32,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1043, 0,         "", "", "",       4,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1053, 0,         "", "", "",      64,        57,                            DEFAULT },
    { doom, 3, 4, 1055, 0,         "", "", "",      88,        57,                            DEFAULT },
    { doom, 3, 4, 1057, 0,         "", "", "",      48,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1058, 0,         "", "", "",      80,        57,                            DEFAULT },
    { doom, 3, 4, 1060, 0,         "", "", "",     104,        57,                            DEFAULT },
    { doom, 3, 4, 1061, 0,         "", "", "",      80,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1070, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4, 1071, 0,         "", "", "", DEFAULT,        72,                            DEFAULT },
    { doom, 3, 4, 1072, 0,         "", "", "",      68,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1073, 0,         "", "", "",     147,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1076, 0,         "", "", "",      72,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1077, 0,         "", "", "",     223,   DEFAULT,                            DEFAULT },
    { doom, 3, 4, 1082, 0,         "", "", "",     151,   DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e3m4[] =
{
    { doom, 3, 4,  62, "",   "FLAT20" },
    { doom, 3, 4,  69, "",  "CEIL3_3" },
    { doom, 3, 4, 109, "",    "FLAT1" },
    { doom, 3, 4, 118, "", "FLOOR7_2" },
    { doom, 3, 4, 125, "",    "FLAT5" },
    { doom, 3, 4, 178, "", "FLOOR7_2" },
    { doom, 3, 4, 185, "",    "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e3m4[] =
{
    { doom, 3, 4,  27, 101 },
    { doom, 3, 4,  37, 101 },
    { doom, 3, 4,  39, 101 },
    { doom, 3, 4,  43, 101 },
    { doom, 3, 4,  74, 101 },
    { doom, 3, 4,  99, 100 },
    { doom, 3, 4, 105, 102 },
    { doom, 3, 4, 139, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M5: UNHOLY CATHEDRAL
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m5[] =
{
    { doom, 3, 5,  294, 0, "", "",        "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom, 3, 5,  295, 0, "", "",        "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom, 3, 5,  325, 0, "", "",        "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom, 3, 5,  326, 0, "", "",        "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom, 3, 5,  332, 0, "", "",        "",     128, DEFAULT,                      ML_DONTPEGTOP },
    { doom, 3, 5,  557, 0, "", "",        "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 3, 5,  597, 0, "", "",        "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom, 3, 5,  598, 0, "", "",        "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
	{ doom, 3, 5,  665, 0, "", "",        "", DEFAULT,     -48,                      ML_DONTPEGTOP },
    { doom, 3, 5,  762, 0, "", "",        "",     168,      32,                            DEFAULT },
    { doom, 3, 5,  764, 0, "", "",        "",       8,      32,                            DEFAULT },
    { doom, 3, 5,  775, 0, "", "",        "",      40,       0,                            DEFAULT },
    { doom, 3, 5,  776, 0, "", "",        "",       8,      32,                            DEFAULT },
    { doom, 3, 5,  787, 0, "", "",        "",     168,      32,                            DEFAULT },
    { doom, 3, 5,  788, 0, "", "",        "",      40,       0,                            DEFAULT },
    { doom, 3, 5,  790, 0, "", "",        "",       8,      32,                            DEFAULT },
    { doom, 3, 5,  801, 0, "", "",        "",     168,      32,                            DEFAULT },
    { doom, 3, 5,  802, 0, "", "",        "",      40,       0,                            DEFAULT },
    { doom, 3, 5,  804, 0, "", "",        "",       8,      32,                            DEFAULT },
    { doom, 3, 5,  815, 0, "", "",        "",     168,      32,                            DEFAULT },
    { doom, 3, 5,  816, 0, "", "",        "",      40,       0,                            DEFAULT },
    { doom, 3, 5, 1056, 0, "", "",        "",       4, DEFAULT,                            DEFAULT },
    { doom, 3, 5, 1057, 0, "", "",        "",       4, DEFAULT,                            DEFAULT },
    { doom, 3, 5, 1128, 0, "", "",        "", DEFAULT,     -13,                            DEFAULT },
    { doom, 3, 5, 1130, 0, "", "",        "", DEFAULT,     -13,                            DEFAULT },
    { doom, 3, 5, 1228, 0, "", "",        "",      91,      32,                            DEFAULT },
    { doom, 3, 5, 1233, 0, "", "",        "",     -64,      32,                            DEFAULT },
    { doom, 3, 5, 1329, 1, "", "", "GSTONE1", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 3, 5, 1330, 1, "", "", "GSTONE1", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 3, 5, 1343, 0, "", "",        "",     160,    -208,                            DEFAULT },
    { doom, 3, 5, 1345, 0, "", "",        "",      -8,    -208,                            DEFAULT },
    { doom, 3, 5, 1349, 0, "", "",        "",     160,     -80,                            DEFAULT },
    { doom, 3, 5, 1351, 0, "", "",        "",      -8,     -80,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e3m5[] =
{
    { doom, 3, 5,  65, "", "CEIL3_5" },
    { doom, 3, 5, 150, "",   "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e3m5[] =
{
    { doom, 3, 5,   0, 100 },
    { doom, 3, 5,  43, 101 },
    { doom, 3, 5,  45, 101 },
    { doom, 3, 5, 235, 101 },
    { doom, 3, 5, 243, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M6: MT. EREBUS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m6[] =
{
    { doom, 3, 6,  39, 0, "",         "", "", DEFAULT,       48,          DEFAULT },
    { doom, 3, 6,  40, 0, "",         "", "", DEFAULT,       48,          DEFAULT },
    { doom, 3, 6, 255, 0, "",         "", "", DEFAULT,  DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 3, 6, 408, 1, "", "BRNSMAL2", "", DEFAULT,  DEFAULT,          DEFAULT },
    { doom, 3, 6, 410, 1, "", "BRNSMAL2", "", DEFAULT,  DEFAULT,          DEFAULT },
    { doom, 3, 6, 412, 1, "", "BRNSMAL1", "", DEFAULT,  DEFAULT,          DEFAULT },
    { doom, 3, 6, 414, 1, "", "BRNSMAL2", "", DEFAULT,  DEFAULT,          DEFAULT },
    { doom, 3, 6, 533, 0, "",         "", "", DEFAULT,        8,          DEFAULT },
    { doom, 3, 6, 534, 0, "",         "", "", DEFAULT,        8,          DEFAULT },
    { doom, 3, 6, 544, 0, "",         "", "", DEFAULT,       -9,          DEFAULT },
    { doom, 3, 6, 550, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 569, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 580, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 583, 0, "",         "", "", DEFAULT,       40,          DEFAULT },
    { doom, 3, 6, 587, 0, "",         "", "", DEFAULT,        8,          DEFAULT },
    { doom, 3, 6, 592, 0, "",         "", "", DEFAULT,        8,          DEFAULT },
    { doom, 3, 6, 593, 0, "",         "", "", DEFAULT,        8,          DEFAULT },
    { doom, 3, 6, 594, 0, "",         "", "", DEFAULT,        8,          DEFAULT },
    { doom, 3, 6, 619, 0, "",         "", "", DEFAULT,      -48,          DEFAULT },
    { doom, 3, 6, 623, 0, "",         "", "", DEFAULT,      -48,          DEFAULT },
    { doom, 3, 6, 645, 0, "",         "", "", DEFAULT,      -48,          DEFAULT },
    { doom, 3, 6, 649, 0, "",         "", "", DEFAULT,      -48,          DEFAULT },
    { doom, 3, 6, 662, 0, "",         "", "", DEFAULT,       24,          DEFAULT },
    { doom, 3, 6, 666, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 668, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 669, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 670, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 671, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    { doom, 3, 6, 672, 0, "",         "", "", DEFAULT,  DEFAULT,      ML_DONTDRAW },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e3m6[] =
{
    { doom, 3, 6, 29, "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e3m6[] =
{
    { doom, 3, 6,   4, 102 },
    { doom, 3, 6,  14, 102 },
    { doom, 3, 6,  15, 102 },
    { doom, 3, 6,  16, 102 },
    { doom, 3, 6,  19, 102 },
    { doom, 3, 6,  23, 102 },
    { doom, 3, 6,  24, 102 },
    { doom, 3, 6,  25, 101 },
    { doom, 3, 6,  26, 101 },
    { doom, 3, 6,  27, 101 },
    { doom, 3, 6,  28, 101 },
    { doom, 3, 6,  29, 101 },
    { doom, 3, 6,  32, 102 },
    { doom, 3, 6,  33, 102 },
    { doom, 3, 6,  35, 102 },
    { doom, 3, 6,  40, 102 },
    { doom, 3, 6,  83, 102 },
    { doom, 3, 6,  84, 102 },
    { doom, 3, 6,  85, 102 },
    { doom, 3, 6,  88, 101 },
    { doom, 3, 6,  90, 102 },
    { doom, 3, 6,  91, 102 },
    { doom, 3, 6,  93, 101 },
    { doom, 3, 6, 115, 102 },
    { doom, 3, 6, 123, 102 },
    { doom, 3, 6, 124, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M7: Limbo
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m7[] =
{
    { doom, 3, 7,  71, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7,  72, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7,  73, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7,  74, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7,  75, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7,  76, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7,  77, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7,  78, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 102, 0,        "", "",        "", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 126, 0,        "", "",        "",     -64,      32,       DEFAULT },
    { doom, 3, 7, 127, 0,        "", "",        "",       8,      32,       DEFAULT },
    { doom, 3, 7, 137, 0,        "", "",        "",     160,     -64,       DEFAULT },
    { doom, 3, 7, 141, 0,        "", "",        "",       0,     -64,       DEFAULT },
    { doom, 3, 7, 142, 0,        "", "",        "",      64,     -64,       DEFAULT },
    { doom, 3, 7, 143, 0,        "", "",        "",     192,     -64,       DEFAULT },
    { doom, 3, 7, 148, 0,        "", "",        "",      32,     -64, ML_DONTPEGTOP },
    { doom, 3, 7, 221, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 222, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 223, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 224, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 225, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 226, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 227, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 228, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 229, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 230, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 231, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 232, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 233, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 234, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 235, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 236, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 237, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 238, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 246, 0,        "", "",        "", DEFAULT,      51,       DEFAULT },
    { doom, 3, 7, 302, 0,        "", "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 3, 7, 312, 0,        "", "",        "", DEFAULT,     -56,       DEFAULT },
    { doom, 3, 7, 314, 0,        "", "",        "", DEFAULT,     -56,       DEFAULT },
    { doom, 3, 7, 317, 0,        "", "",        "", DEFAULT,     -56,       DEFAULT },
    { doom, 3, 7, 319, 0,        "", "",        "", DEFAULT,     -56,       DEFAULT },
    { doom, 3, 7, 339, 0,        "", "",        "",      96, DEFAULT,       DEFAULT },
    { doom, 3, 7, 341, 0,        "", "",        "",      32, DEFAULT, ML_DONTPEGTOP },
    { doom, 3, 7, 375, 0,        "", "",        "",      96, DEFAULT,       DEFAULT },
    { doom, 3, 7, 377, 0,        "", "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 3, 7, 443, 0,        "", "",        "",      32, DEFAULT, ML_DONTPEGTOP },
    { doom, 3, 7, 460, 0,        "", "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 3, 7, 548, 0,        "", "",        "",      -8,      11,       DEFAULT },
    { doom, 3, 7, 554, 0,        "", "",        "",     -22,      11,       DEFAULT },
    { doom, 3, 7, 563, 0,        "", "",        "",     -72,      11,       DEFAULT },
    { doom, 3, 7, 572, 0,        "", "",        "",      36,      11,       DEFAULT },
    { doom, 3, 7, 617, 0,        "", "",        "",      32, DEFAULT,       DEFAULT },
    { doom, 3, 7, 618, 0,        "", "",        "",      96, DEFAULT,       DEFAULT },
    { doom, 3, 7, 619, 0,        "", "",        "",     128, DEFAULT,       DEFAULT },
    { doom, 3, 7, 620, 0,        "", "",        "",     192, DEFAULT,       DEFAULT },
    { doom, 3, 7, 622, 0,        "", "",        "",      64, DEFAULT,       DEFAULT },
    { doom, 3, 7, 623, 0,        "", "",        "",      96, DEFAULT,       DEFAULT },
    { doom, 3, 7, 624, 0,        "", "",        "",     160, DEFAULT,       DEFAULT },
    { doom, 3, 7, 628, 0,        "", "",        "",      80, DEFAULT,       DEFAULT },
    { doom, 3, 7, 655, 0,        "", "",        "",      16, DEFAULT,       DEFAULT },
    { doom, 3, 7, 683, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 684, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 685, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 686, 0,        "", "", "GSTONE2", DEFAULT,     -24,       DEFAULT },
    { doom, 3, 7, 716, 0,        "", "",        "", DEFAULT,     -48,       DEFAULT },
    { doom, 3, 7, 717, 0,        "", "",        "", DEFAULT,     -48,       DEFAULT },
    { doom, 3, 7, 718, 0,        "", "",        "", DEFAULT,     -48,       DEFAULT },
    { doom, 3, 7, 845, 0,        "", "",        "", DEFAULT,     -32,       DEFAULT },
    { doom, 3, 7, 846, 0,        "", "",        "", DEFAULT,     -32,       DEFAULT },
    { doom, 3, 7, 899, 0,        "", "",        "", DEFAULT,     -32,       DEFAULT },
    { doom, 3, 7, 901, 1,        "", "",   "STEP2", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 3, 7, 971, 1, "SP_HOT1", "",        "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 3, 7, 973, 0,        "", "",        "", DEFAULT,      -7,       DEFAULT },
    { doom, 3, 7, 976, 0,        "", "",        "", DEFAULT,      -7,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e3m7[] =
{
    { doom, 3, 7, 42, "", "MFLR8_2" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e3m7[] =
{
    { doom, 3, 7,   0, 101 },
    { doom, 3, 7,   1, 101 },
    { doom, 3, 7,   2, 101 },
    { doom, 3, 7,  46, 101 },
    { doom, 3, 7,  47, 101 },
    { doom, 3, 7,  49, 102 },
    { doom, 3, 7,  57, 102 },
    { doom, 3, 7,  62, 101 },
    { doom, 3, 7,  64, 101 },
    { doom, 3, 7,  65, 101 },
    { doom, 3, 7,  67, 102 },
    { doom, 3, 7,  71, 102 },
    { doom, 3, 7,  73, 101 },
    { doom, 3, 7,  76, 101 },
    { doom, 3, 7,  79, 101 },
    { doom, 3, 7,  16, 102 },
    { doom, 3, 7,  81, 216 },
    { doom, 3, 7,  83, 226 },
    { doom, 3, 7,  85, 102 },
    { doom, 3, 7,  91, 201 },
    { doom, 3, 7,  92, 236 },
    { doom, 3, 7,  94, 206 },
    { doom, 3, 7, 100, 102 },
    { doom, 3, 7, 103, 101 },
    { doom, 3, 7, 104, 101 },
    { doom, 3, 7, 118, 102 },
    { doom, 3, 7, 121, 102 },
    { doom, 3, 7, 131, 102 },
    { doom, 3, 7, 135, 102 },
    { doom, 3, 7, 140, 101 },
    { doom, 3, 7, 142, 101 },
    { doom, 3, 7, 148, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E3M8: DIS
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_doom1_e3m8[] =
{
    { doom, 3, 8, 16, "", "FLOOR7_1" },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// E3M9: WARRENS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e3m9[] =
{
    { doom, 3, 9,   9, 0, "", "",       "", DEFAULT,      -8,          DEFAULT },
    { doom, 3, 9,  17, 0, "", "",       "",      96,      -8,          DEFAULT },
    { doom, 3, 9,  18, 0, "", "",       "",     208,      -8,          DEFAULT },
    { doom, 3, 9,  91, 1, "", "",  "STEP4", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 3, 9, 102, 1, "", "", "STONE3", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 3, 9, 205, 0, "", "",       "",     -16, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 3, 9, 206, 0, "", "",       "",     168, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 3, 9, 276, 0, "", "",       "", DEFAULT,      -8,          DEFAULT },
    { doom, 3, 9, 277, 0, "", "",       "", DEFAULT,      -8,          DEFAULT },
    { doom, 3, 9, 285, 0, "", "",       "",      72,      11,          DEFAULT },
    { doom, 3, 9, 286, 0, "", "",       "",     136,      11,          DEFAULT },
    { doom, 3, 9, 287, 0, "", "",       "",     200,      11,          DEFAULT },
    { doom, 3, 9, 449, 0, "", "",       "",     -24, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 3, 9, 450, 0, "", "",       "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e3m9[] =
{
    { doom, 3, 9, 27, "FLAT20", "" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e3m9[] =
{
    { doom, 3, 9,  0, 102 },
    { doom, 3, 9,  1, 102 },
    { doom, 3, 9,  2, 102 },
    { doom, 3, 9,  5, 102 },
    { doom, 3, 9,  6, 102 },
    { doom, 3, 9,  7, 102 },
    { doom, 3, 9,  8, 102 },
    { doom, 3, 9,  9, 102 },
    { doom, 3, 9, 11, 102 },
    { doom, 3, 9, 22, 102 },
    { doom, 3, 9, 30, 102 },
    { doom, 3, 9, 31, 102 },
    { doom, 3, 9, 38, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M1: HELL BENEATH
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m1[] =
{
    { doom, 4, 1,  58, 0,         "",         "",         "",      32, DEFAULT,       DEFAULT },
    { doom, 4, 1,  80, 0,         "",         "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 1,  81, 0,         "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 1,  83, 0,         "",         "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 1,  84, 0,         "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 1,  91, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1,  92, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1,  95, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1,  96, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 124, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 126, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 156, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 165, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 171, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 172, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 193, 0,         "",         "",         "",      -8, DEFAULT,       DEFAULT },
    { doom, 4, 1, 195, 0,         "",         "",         "",      -8, DEFAULT,       DEFAULT },
    { doom, 4, 1, 202, 0,         "",         "",         "",      64, DEFAULT,       DEFAULT },
    { doom, 4, 1, 203, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 214, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 215, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 227, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 230, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 232, 0,         "",         "",         "",     160, DEFAULT,       DEFAULT },
    { doom, 4, 1, 280, 0,         "",         "",         "",      32, DEFAULT,       DEFAULT },
    { doom, 4, 1, 282, 0,         "",         "",         "",      -7, DEFAULT,       DEFAULT },
    { doom, 4, 1, 283, 0,         "",         "",         "",     160, DEFAULT,       DEFAULT },
    { doom, 4, 1, 284, 0,         "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 1, 287, 0,         "",         "",         "",      32, DEFAULT,       DEFAULT },
    { doom, 4, 1, 290, 0,         "",         "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 1, 291, 0,         "",         "",         "",     184, DEFAULT,       DEFAULT },
    { doom, 4, 1, 293, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1, 294, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 296, 0,         "",         "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 1, 298, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1, 299, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 312, 0,         "",         "",         "",     -39, DEFAULT,       DEFAULT },
    { doom, 4, 1, 315, 0,         "",         "",         "",      72, DEFAULT,       DEFAULT },
    { doom, 4, 1, 317, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 318, 0,         "",         "",         "",     200, DEFAULT,       DEFAULT },
    { doom, 4, 1, 322, 1,         "",         "", "MARBFACE",      30,      14,       DEFAULT },
    { doom, 4, 1, 341, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 342, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 345, 0,         "",         "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 1, 346, 0,         "",         "",         "",     112, DEFAULT,       DEFAULT },
    { doom, 4, 1, 347, 0,         "",         "",         "",     176, DEFAULT,       DEFAULT },
    { doom, 4, 1, 358, 0,         "",         "",         "",     -16, DEFAULT,       DEFAULT },
    { doom, 4, 1, 359, 0,         "",         "",         "",     -80, DEFAULT,       DEFAULT },
    { doom, 4, 1, 360, 0,         "",    "METAL",         "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 1, 362, 0,         "",    "METAL",         "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 1, 363, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 368, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 370, 0,         "",         "",         "",      72, DEFAULT,       DEFAULT },
    { doom, 4, 1, 372, 0,         "",         "",         "",     152, DEFAULT,       DEFAULT },
    { doom, 4, 1, 375, 0,         "",         "",         "",    -120, DEFAULT,       DEFAULT },
    { doom, 4, 1, 381, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1, 382, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 429, 0,         "",         "",         "",     -16, DEFAULT,       DEFAULT },
    { doom, 4, 1, 430, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1, 434, 0,         "",         "",         "",     128, DEFAULT,       DEFAULT },
    { doom, 4, 1, 437, 0,         "",         "",         "",      40, DEFAULT,       DEFAULT },
    { doom, 4, 1, 440, 0,         "",         "",         "",      64, DEFAULT,       DEFAULT },
    { doom, 4, 1, 444, 0,         "",         "",         "",     128, DEFAULT,       DEFAULT },
    { doom, 4, 1, 446, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1, 447, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 449, 0,         "",         "",         "",      80, DEFAULT,       DEFAULT },
    { doom, 4, 1, 451, 0,         "",         "",         "",      24, DEFAULT,       DEFAULT },
    { doom, 4, 1, 452, 0,         "",         "",         "",      56, DEFAULT,       DEFAULT },
    { doom, 4, 1, 454, 0,         "", "SUPPORT3",         "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 1, 456, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 458, 0,         "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 461, 0, "EXITSIGN",         "",         "",      -8, DEFAULT,       DEFAULT },
    { doom, 4, 1, 463, 0, "EXITSIGN",         "",         "",      -8, DEFAULT,       DEFAULT },
    { doom, 4, 1, 466, 0,         "",         "",         "",      -4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 468, 0,         "",         "",         "",      -4, DEFAULT,       DEFAULT },
    { doom, 4, 1, 470, 0,  "GSTONE1",         "",         "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m1[] =
{
    { doom, 4, 1, 64, "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m1[] =
{
    { doom, 4, 1, 16, 101 },
    { doom, 4, 1, 17, 100 },
    { doom, 4, 1, 28, 101 },
    { doom, 4, 1, 31, 232 },
    { doom, 4, 1, 32, 232 },
    { doom, 4, 1, 33, 232 },
    { doom, 4, 1, 36, 101 },
    { doom, 4, 1, 37, 101 },
    { doom, 4, 1, 38, 101 },
    { doom, 4, 1, 39, 101 },
    { doom, 4, 1, 40, 101 },
    { doom, 4, 1, 41, 101 },
    { doom, 4, 1, 42, 101 },
    { doom, 4, 1, 43, 101 },
    { doom, 4, 1, 44, 101 },
    { doom, 4, 1, 71, 101 },
    { doom, 4, 1, 72, 101 },
    { doom, 4, 1, 73, 101 },
    { doom, 4, 1, 76, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M2: PERFECT HATRED
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m2[] =
{
    { doom, 4, 2, 138, 0,      "", "", "",      40, DEFAULT,          DEFAULT },
    { doom, 4, 2, 152, 0,      "", "", "",      57, DEFAULT,          DEFAULT },
    { doom, 4, 2, 165, 1, "WOOD5", "", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom, 4, 2, 198, 0,      "", "", "",      64, DEFAULT,          DEFAULT },
    { doom, 4, 2, 199, 0,      "", "", "",      64, DEFAULT,          DEFAULT },
    { doom, 4, 2, 219, 0,      "", "", "",       4, DEFAULT,          DEFAULT },
    { doom, 4, 2, 220, 0,      "", "", "",       4, DEFAULT,          DEFAULT },
    { doom, 4, 2, 221, 0,      "", "", "",       4, DEFAULT,          DEFAULT },
    { doom, 4, 2, 231, 0,      "", "", "",      64, DEFAULT,          DEFAULT },
    { doom, 4, 2, 236, 0,      "", "", "",       4, DEFAULT,          DEFAULT },
    { doom, 4, 2, 357, 0,      "", "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 4, 2, 372, 0,      "", "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom, 4, 2, 584, 0,      "", "", "", DEFAULT,      40,          DEFAULT },
    { doom, 4, 2, 585, 0,      "", "", "", DEFAULT,      40,          DEFAULT },
    { doom, 4, 2, 587, 0,      "", "", "", DEFAULT,      40,          DEFAULT },
    { doom, 4, 2, 588, 0,      "", "", "", DEFAULT,      40,          DEFAULT },
    { doom, 4, 2, 620, 0,      "", "", "",      16, DEFAULT,          DEFAULT },
    { doom, 4, 2, 621, 0,      "", "", "",      48, DEFAULT,          DEFAULT },
    { doom, 4, 2, 623, 0,      "", "", "",      48, DEFAULT,          DEFAULT },
    { doom, 4, 2, 630, 0,      "", "", "",      16, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m2[] =
{
    { doom, 4, 2, 14, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m2[] =
{
    { doom, 4, 2,   0, 102 },
    { doom, 4, 2,   3, 102 },
    { doom, 4, 2,   4, 102 },
    { doom, 4, 2,   8, 101 },
    { doom, 4, 2,  12, 102 },
    { doom, 4, 2,  14, 102 },
    { doom, 4, 2,  35, 101 },
    { doom, 4, 2,  34, 101 },
    { doom, 4, 2,  36, 101 },
    { doom, 4, 2,  37, 101 },
    { doom, 4, 2,  42, 101 },
    { doom, 4, 2,  43, 101 },
    { doom, 4, 2,  44, 101 },
    { doom, 4, 2,  45, 101 },
    { doom, 4, 2,  46, 101 },
    { doom, 4, 2,  47, 101 },
    { doom, 4, 2,  49, 102 },
    { doom, 4, 2,  51, 231 },
    { doom, 4, 2,  54, 231 },
    { doom, 4, 2,  56, 226 },
    { doom, 4, 2, 108, 226 },
    { doom, 4, 2, 112, 231 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M3: SEVER THE WICKED
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m3[] =
{
    { doom, 4, 3,  25, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3,  27, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3,  33, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3,  34, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3,  49, 0,  "", "",  "",      24, DEFAULT, DEFAULT },
    { doom, 4, 3,  50, 0,  "", "",  "",      24, DEFAULT, DEFAULT },
    { doom, 4, 3, 155, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 156, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 158, 0,  "", "",  "",      -8, DEFAULT, DEFAULT },
    { doom, 4, 3, 160, 0,  "", "",  "",      -8, DEFAULT, DEFAULT },
    { doom, 4, 3, 162, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 163, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 165, 0,  "", "",  "",      -8, DEFAULT, DEFAULT },
    { doom, 4, 3, 167, 0,  "", "",  "",      -8, DEFAULT, DEFAULT },
    { doom, 4, 3, 201, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 202, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 203, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 204, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 206, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 207, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 208, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 209, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 235, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 236, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 237, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 238, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 241, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 242, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 243, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 244, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 246, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 247, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 248, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 340, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 342, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 343, 0, "-", "",  "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 3, 344, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 346, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 347, 0, "-", "",  "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 3, 351, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 352, 0, "-", "",  "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 3, 353, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 355, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 356, 0, "-", "",  "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 3, 408, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 409, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 496, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 497, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 498, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 499, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 500, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 501, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 502, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 504, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 505, 0, "-", "",  "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 3, 508, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 509, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 510, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 511, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 512, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 513, 0,  "", "",  "",       4, DEFAULT, DEFAULT },
    { doom, 4, 3, 706, 0,  "", "",  "",     -16,     -88, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m3[] =
{
    { doom, 4, 3, 153, "", "FLOOR7_1" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m3[] =
{
    { doom, 4, 3,  12, 232 },
    { doom, 4, 3,  35, 232 },
    { doom, 4, 3,  39, 232 },
    { doom, 4, 3, 117, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M4: UNRULY EVIL
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m4[] =
{
    { doom, 4, 4,  15, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  15, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  56, 0,         "", "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 4, 4,  57, 0,         "", "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 4, 4,  86, 0,         "", "", "",      -4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  87, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  88, 0,         "", "", "",      -4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  89, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  90, 0,         "", "", "",      -4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  91, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  92, 0,         "", "", "",      -4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  93, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  94, 0,         "", "", "",      -4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  95, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  96, 0,         "", "", "",      -4, DEFAULT,                            DEFAULT },
    { doom, 4, 4,  97, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 204, 0,         "", "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom, 4, 4, 427, 1, "BROWNHUG", "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 428, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 429, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 430, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 431, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 432, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 433, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 434, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 435, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 436, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 437, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 438, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 439, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 440, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 441, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 442, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 443, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 444, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 445, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 446, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 447, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 448, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 449, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 450, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 451, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 452, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 453, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 454, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 455, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 456, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 457, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 458, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 459, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 558, 1, "BROWNHUG", "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 567, 0, "BROWNHUG", "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 572, 0, "BROWNHUG", "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 578, 0, "EXITSIGN", "", "",      -8, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 580, 0, "EXITSIGN", "", "",      -8, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 585, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 586, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 589, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 590, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 596, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 597, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 602, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 603, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 608, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 609, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 614, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 615, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 619, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 620, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 624, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom, 4, 4, 625, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m4[] =
{
    { doom, 4, 4, 62,        "", "FLAT1" },
    { doom, 4, 4, 75, "SWATER3",      "" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m4[] =
{
    { doom, 4, 4,   5, 101 },
    { doom, 4, 4,   6, 101 },
    { doom, 4, 4,   7, 101 },
    { doom, 4, 4,   9, 101 },
    { doom, 4, 4,  71, 101 },
    { doom, 4, 4,  75, 101 },
    { doom, 4, 4,  91, 102 },
    { doom, 4, 4,  98, 102 },
    { doom, 4, 4, 100, 102 },
    { doom, 4, 4, 119, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M5: THEY WILL REPENT
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m5[] =
{
    { doom, 4, 5,    0, 0,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,    5, 1,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,   19, 1,        "",         "", "BROWNHUG", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,   31, 0,        "",         "",         "",     -32,      16,       DEFAULT },
    { doom, 4, 5,   33, 0,        "",         "",         "",      64,      16,       DEFAULT },
    { doom, 4, 5,   35, 1,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  109, 1, "GSTONE1",         "",         "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 4, 5,  155, 1,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  182, 0,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  183, 0,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  184, 0,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  307, 0,        "",         "",         "",      96, DEFAULT,       DEFAULT },
    { doom, 4, 5,  332, 1,        "",         "",         "",      32, DEFAULT,       DEFAULT },
    { doom, 4, 5,  367, 0,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  407, 0,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  408, 0,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  465, 1,        "",         "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 5,  625, 1,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  673, 1,        "",         "",         "",     144,     -32,       DEFAULT },
    { doom, 4, 5,  711, 1,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  712, 0,        "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 5,  713, 0,        "",         "",         "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 5,  713, 1,        "",         "", "FIRELAVA", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 5,  714, 0,        "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 5,  777, 0,        "", "BROVINE2",         "",      39, DEFAULT,       DEFAULT },
    { doom, 4, 5,  779, 0,        "", "BROVINE2",         "",     -64, DEFAULT,       DEFAULT },
    { doom, 4, 5,  831, 0,        "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 5,  843, 0,        "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 5,  852, 0,        "",         "",         "",      48, DEFAULT,       DEFAULT },
    { doom, 4, 5,  974, 0,        "",         "",         "", DEFAULT,     -56,       DEFAULT },
    { doom, 4, 5,  975, 0,        "",         "",         "", DEFAULT,     -56,       DEFAULT },
    { doom, 4, 5,  976, 0,        "",         "",         "", DEFAULT,     -56,       DEFAULT },
    { doom, 4, 5,  977, 0,        "",         "",         "", DEFAULT,     -56,       DEFAULT },
    { doom, 4, 5,  978, 0,        "",         "",         "", DEFAULT,     -56,       DEFAULT },
    { doom, 4, 5,  985, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  986, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  987, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  988, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  989, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  990, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  991, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  992, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  993, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  994, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  995, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  996, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  997, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  998, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5,  999, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1000, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1001, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1002, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1003, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1004, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1005, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1006, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1007, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1008, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1009, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1010, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1011, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1012, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1013, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1014, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1015, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 5, 1016, 0,        "",         "",         "",       4, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m5[] =
{
    { doom, 4, 5, 134, "FLAT1", "" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m5[] =
{
    { doom, 4, 5,   4, 232 },
    { doom, 4, 5,   6, 222 },
    { doom, 4, 5,   7, 232 },
    { doom, 4, 5,  11, 232 },
    { doom, 4, 5,  22, 232 },
    { doom, 4, 5,  49, 102 },
    { doom, 4, 5,  56, 212 },
    { doom, 4, 5,  57, 102 },
    { doom, 4, 5,  61, 102 },
    { doom, 4, 5,  86, 232 },
    { doom, 4, 5,  87, 222 },
    { doom, 4, 5,  88, 222 },
    { doom, 4, 5,  97, 222 },
    { doom, 4, 5, 106, 102 },
    { doom, 4, 5, 110, 102 },
    { doom, 4, 5, 120, 206 },
    { doom, 4, 5, 124, 102 },
    { doom, 4, 5, 132, 101 },
    { doom, 4, 5, 137, 102 },
    { doom, 4, 5, 141, 211 },
    { doom, 4, 5, 154, 102 },
    { doom, 4, 5, 155, 202 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M6: Against Thee Wickedly
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m6[] =
{
    { doom, 4, 6,  373, 0,        "", "",      "",      64, DEFAULT, DEFAULT },
    { doom, 4, 6,  377, 0,        "", "",      "",      32, DEFAULT, DEFAULT },
    { doom, 4, 6,  378, 0,        "", "",      "",      32, DEFAULT, DEFAULT },
    { doom, 4, 6,  379, 0,        "", "",      "",      64, DEFAULT, DEFAULT },
    { doom, 4, 6,  391, 0,        "", "",      "",      64, DEFAULT, DEFAULT },
    { doom, 4, 6,  393, 0,        "", "",      "",      40, DEFAULT, DEFAULT },
    { doom, 4, 6,  431, 0,        "", "",      "",     -16, DEFAULT, DEFAULT },
    { doom, 4, 6,  432, 0,        "", "",      "",       8, DEFAULT, DEFAULT },
    { doom, 4, 6,  455, 0,        "", "",      "",     -32, DEFAULT, DEFAULT },
    { doom, 4, 6,  456, 0,        "", "",      "",      -8, DEFAULT, DEFAULT },
    { doom, 4, 6,  460, 0,        "", "",      "",      24, DEFAULT, DEFAULT },
    { doom, 4, 6,  465, 0,        "", "",      "",      24, DEFAULT, DEFAULT },
    { doom, 4, 6,  469, 0,        "", "",      "",     -32, DEFAULT, DEFAULT },
    { doom, 4, 6,  470, 0,        "", "",      "",      -8, DEFAULT, DEFAULT },
    { doom, 4, 6,  475, 1, "MARBLE1", "", "METAL", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 6,  476, 1, "MARBLE1", "", "METAL", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 6,  479, 1, "MARBLE1", "",      "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 6,  480, 1, "MARBLE1", "",      "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 6,  481, 1, "MARBLE1", "",      "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 6,  482, 1, "MARBLE1", "",      "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 6,  565, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  566, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  814, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  817, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  819, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  822, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  823, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  826, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  828, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  831, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  848, 0,        "", "",      "",       8, DEFAULT, DEFAULT },
    { doom, 4, 6,  850, 0,        "", "",      "",       8, DEFAULT, DEFAULT },
    { doom, 4, 6,  855, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  858, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  860, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  863, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  865, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  868, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  870, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  873, 0,        "", "",      "",       4, DEFAULT, DEFAULT },
    { doom, 4, 6,  887, 0,        "", "",      "",       8, DEFAULT, DEFAULT },
    { doom, 4, 6,  889, 0,        "", "",      "",       8, DEFAULT, DEFAULT },
    { doom, 4, 6,  979, 0,        "", "",      "",     120, DEFAULT, DEFAULT },
    { doom, 4, 6,  986, 0,        "", "",      "",     -16, DEFAULT, DEFAULT },
    { doom, 4, 6,  987, 0,        "", "",      "",      16, DEFAULT, DEFAULT },
    { doom, 4, 6, 1106, 0,        "", "",      "",      32, DEFAULT, DEFAULT },
    { doom, 4, 6, 1316, 0,        "", "",      "",      29, DEFAULT, DEFAULT },
    { doom, 4, 6, 1317, 0,        "", "",      "",      63, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const flow_t flow_doom1_e4m6[] =
{
    { doom, 4, 6,   0, 102 },
    { doom, 4, 6,   1, 102 },
    { doom, 4, 6,  10, 102 },
    { doom, 4, 6,  12, 102 },
    { doom, 4, 6,  13, 102 },
    { doom, 4, 6,  42, 102 },
    { doom, 4, 6,  63, 102 },
    { doom, 4, 6,  69, 102 },
    { doom, 4, 6, 105, 102 },
    { doom, 4, 6, 111, 102 },
    { doom, 4, 6, 133, 102 },
    { doom, 4, 6, 134, 102 },
    { doom, 4, 6, 135, 102 },
    { doom, 4, 6, 136, 102 },
    { doom, 4, 6, 139, 102 },
    { doom, 4, 6, 140, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M7: And Hell Followed
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m7[] =
{
    { doom, 4, 7, 325, 1, "",  "BRNBIGR",         "", DEFAULT,   DEFAULT, DEFAULT },
    { doom, 4, 7, 326, 1, "",  "BRNBIGC",         "", DEFAULT,   DEFAULT, DEFAULT },
    { doom, 4, 7, 327, 1, "",  "BRNBIGL",         "", DEFAULT,   DEFAULT, DEFAULT },
    { doom, 4, 7, 451, 1, "",  "BRNBIGR",         "", DEFAULT,   DEFAULT, DEFAULT },
    { doom, 4, 7, 452, 1, "",  "BRNBIGC",         "", DEFAULT,   DEFAULT, DEFAULT },
    { doom, 4, 7, 453, 1, "",  "BRNBIGL",         "", DEFAULT,   DEFAULT, DEFAULT },
    { doom, 4, 7, 475, 0, "",         "",         "", DEFAULT,       128, DEFAULT },
    { doom, 4, 7, 477, 0, "",         "", "NUKEDGE1", DEFAULT,       -12, DEFAULT },
    { doom, 4, 7, 478, 0, "",         "", "NUKEDGE1", DEFAULT,       -12, DEFAULT },
    { doom, 4, 7, 989, 0, "", "NUKEDGE1",         "", DEFAULT,   DEFAULT, DEFAULT },
    { doom, 4, 7, 991, 0, "", "NUKEDGE1",         "", DEFAULT,   DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m7[] =
{
    { doom, 4, 7, 186, "FLAT1",        "" },
    { doom, 4, 7, 212,      "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m7[] =
{
    { doom, 4, 7,  57, 232 },
    { doom, 4, 7,  59, 232 },
    { doom, 4, 7,  60, 232 },
    { doom, 4, 7,  61, 232 },
    { doom, 4, 7,  64, 232 },
    { doom, 4, 7,  84, 101 },
    { doom, 4, 7,  86, 101 },
    { doom, 4, 7,  90, 101 },
    { doom, 4, 7,  91, 101 },
    { doom, 4, 7, 107, 222 },
    { doom, 4, 7, 157, 222 },
    { doom, 4, 7, 158, 222 },
    { doom, 4, 7, 159, 222 },
    { doom, 4, 7, 160, 222 },
    { doom, 4, 7, 161, 232 },
    { doom, 4, 7, 166, 222 },
    { doom, 4, 7, 167, 232 },
    { doom, 4, 7, 168, 232 },
    { doom, 4, 7, 170, 232 },
    { doom, 4, 7, 171, 232 },
    { doom, 4, 7, 196, 232 },
    { doom, 4, 7, 235, 232 },
    { doom, 4, 7, 236, 232 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M8: UNTO THE CRUEL
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m8[] =
{
    { doom, 4, 8,   69, 0, "",        "", "",      26,     -64,       DEFAULT },
    { doom, 4, 8,  207, 0, "",        "", "",      77, DEFAULT,       DEFAULT },
    { doom, 4, 8,  213, 0, "",        "", "",      16, DEFAULT,       DEFAULT },
    { doom, 4, 8,  214, 0, "",        "", "",       8, DEFAULT,       DEFAULT },
    { doom, 4, 8,  220, 0, "",        "", "",      69, DEFAULT,       DEFAULT },
    { doom, 4, 8,  222, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  223, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  224, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  225, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  226, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  227, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  228, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  229, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  232, 0, "",        "", "",     -40, DEFAULT,       DEFAULT },
    { doom, 4, 8,  233, 0, "",        "", "",     -20, DEFAULT,       DEFAULT },
    { doom, 4, 8,  234, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  236, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  356, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  357, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  358, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  359, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  360, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  361, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  365, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  366, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  367, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  368, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  369, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  370, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  375, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  376, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  377, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  378, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  379, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  380, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  418, 0, "", "SP_HOT1", "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 8,  419, 0, "", "SP_HOT1", "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 8,  420, 0, "", "SP_HOT1", "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 8,  425, 0, "", "SP_HOT1", "", DEFAULT, DEFAULT,       DEFAULT },
    { doom, 4, 8,  513, 0, "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 4, 8,  570, 0, "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 4, 8,  576, 0, "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 4, 8,  579, 0, "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 4, 8,  596, 0, "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 4, 8,  604, 0, "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom, 4, 8,  626, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  627, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  628, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  629, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  630, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  631, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  638, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  639, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  640, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  641, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  642, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  643, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  647, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  648, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  649, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  650, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  651, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8,  652, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8, 1203, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    { doom, 4, 8, 1204, 0, "",        "", "",       4, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m8[] =
{
    { doom, 4, 8, 79, "", "CEIL5_2" },
    { doom, 4, 8, 96, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m8[] =
{
    { doom, 4, 8,  34, 102 },
    { doom, 4, 8,  40, 102 },
    { doom, 4, 8,  50, 102 },
    { doom, 4, 8, 121, 222 },
    { doom, 4, 8, 122, 222 },
    { doom, 4, 8, 124, 222 },
    { doom, 4, 8, 134, 216 },
    { doom, 4, 8, 135, 216 },
    { doom, 4, 8, 144, 222 },
    { doom, 4, 8, 146, 222 },
    { doom, 4, 8, 150, 222 },
    { doom, 4, 8, 153, 222 },
    { doom, 4, 8, 155, 222 },
    { doom, 4, 8, 156, 222 },
    { doom, 4, 8, 202, 216 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// E4M9: FEAR
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom1_e4m9[] =
{
    { doom, 4, 9,  343, 0, "", "METAL",        "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 9,  345, 0, "", "METAL",        "", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 9,  672, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9,  673, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9,  674, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9,  675, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9,  676, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9,  677, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9,  678, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9,  679, 0, "",      "",        "",       4, DEFAULT, DEFAULT },
    { doom, 4, 9, 1175, 0, "",      "", "CRATINY", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 9, 1176, 0, "",      "", "CRATINY", DEFAULT, DEFAULT, DEFAULT },
    { doom, 4, 9, 1195, 0, "",      "",        "",      16, DEFAULT, DEFAULT },
    { doom, 4, 9, 1583, 0, "",      "",        "", DEFAULT,    -120, DEFAULT },
    { doom, 4, 9, 1584, 0, "",      "",        "",      64,    -120, DEFAULT },
    { doom, 4, 9, 1585, 0, "",      "",        "", DEFAULT,    -120, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom1_e4m9[] =
{
    { doom, 4, 9, 100, "", "FLOOR7_1" },
    { doom, 4, 9, 238, "",   "FLAT20" },
    { doom, 4, 9, 240, "",   "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom1_e4m9[] =
{
    { doom, 4, 9,   9, 102 },
    { doom, 4, 9,  10, 102 },
    { doom, 4, 9, 123, 101 },
    { doom, 4, 9, 149, 101 },
    { doom, 4, 9, 187, 102 },
    { doom, 4, 9, 192, 101 },
    { doom, 4, 9, 201, 102 },
    { doom, 4, 9, 204, 102 },
    FLOW_END
};


// =============================================================================
// DOOM 2: HELL ON EARTH
// =============================================================================


// -----------------------------------------------------------------------------
// LEVEL 1: ENTRYWAY
// -----------------------------------------------------------------------------

static const vertexfix_t vertexfix_doom2_map01[] =
{
    { doom2, 1, 1, 288, 320, 1416, 320, 1408 },
    VERTEXFIX_END
};

static const linefix_t linefix_doom2_map01[] =
{
    { doom2, 1, 1,   4, 0, "",         "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 1,   6, 0, "",         "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 1, 121, 0, "",         "",        "",       6, DEFAULT, DEFAULT },
    { doom2, 1, 1, 125, 0, "",         "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 1, 141, 0, "",         "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 1, 143, 0, "",         "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 1, 175, 0, "",         "",        "",     117, DEFAULT, DEFAULT },
    { doom2, 1, 1, 169, 1, "", "BRNSMAL2",        "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 1, 334, 1, "", "MIDBARS3", "BRONZE1", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 1, 335, 1, "", "MIDBARS3", "BRONZE1", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 1, 369, 1, "", "MIDBARS3", "BRONZE1", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map01[] =
{
    { doom2, 1, 1, 29, "RROCK09",      "" },
    { doom2, 1, 1, 35,        "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map01[] =
{
    { doom2, 1, 1, 14, 236 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 2: UNDERHALLS
// -----------------------------------------------------------------------------

static const vertexfix_t vertexfix_doom2_map02[] =
{
    { doom2, 1, 2, 273, 736, 1216, 736, 1208 },
    VERTEXFIX_END
};

static const linefix_t linefix_doom2_map02[] =
{
    { doom2, 1, 2,   3, 0, "",         "",       "",     -24, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,   5, 0, "",         "",       "",     -24, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,   7, 0, "",         "",       "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,   9, 0, "",         "",       "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  10, 0, "",         "",       "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  12, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  13, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  17, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  19, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  20, 0, "",         "",       "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  27, 0, "",         "",       "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  28, 0, "",         "",       "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  29, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  37, 0, "",         "",       "",     -48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  38, 0, "",         "",       "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  39, 0, "",         "",       "",     -56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  40, 0, "",         "",       "",     -40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  63, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  77, 0, "",         "",       "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  78, 0, "",         "",       "",      48, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2,  79, 0, "",         "",       "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2,  80, 0, "",         "",       "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2,  83, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  87, 0, "",         "",       "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2,  91, 0, "",         "",       "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  92, 0, "",         "",       "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2,  93, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  94, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2,  95, 0, "",         "",       "", DEFAULT,     -80,                            DEFAULT },
    { doom2, 1, 2,  98, 0, "",         "",       "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 100, 0, "",         "",       "",     -24,     -80,                            DEFAULT },
    { doom2, 1, 2, 103, 0, "",         "",       "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 109, 0, "",         "",       "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 110, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 128, 0, "",         "",       "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 128, 0, "",         "",       "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 135, 0, "",         "",       "",      48, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2, 136, 0, "",         "",       "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2, 143, 0, "",         "",       "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 144, 0, "",         "",       "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 145, 0, "",         "",       "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 146, 0, "",         "",       "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 148, 0, "",         "",       "",     -40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 149, 0, "",         "",       "",     -56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 150, 0, "",         "",       "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 151, 0, "",         "",       "",     -48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 170, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 171, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 172, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 186, 0, "",         "",       "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 196, 0, "",         "",       "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 197, 0, "",         "",       "",     -56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 198, 0, "",         "",       "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 200, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 203, 0, "",         "",       "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 204, 0, "",         "",       "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 246, 0, "",         "",       "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 247, 0, "",         "",       "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 302, 0, "",         "",       "",      10, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 305, 0, "",         "",       "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2, 306, 0, "",         "",       "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 310, 0, "",         "",       "",      40, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2, 315, 0, "",         "",       "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2, 327, 0, "",         "", "STONE4", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 328, 0, "",         "", "STONE4", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 338, 0, "",         "", "STONE4", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 339, 0, "",         "", "STONE4", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 344, 0, "", "PIPEWAL2",       "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 345, 0, "", "PIPEWAL2",       "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 454, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 458, 0, "",         "",       "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 465, 0, "",         "",       "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 469, 0, "",         "",       "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 472, 0, "",         "",       "",     -48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 465, 0, "",         "",       "",     -40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 490, 0, "",         "",       "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 491, 0, "",         "",       "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 493, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 497, 0, "",         "",       "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 2, 499, 0, "",         "",       "", DEFAULT,     -48,                            DEFAULT },
    { doom2, 1, 2, 513, 0, "",         "",       "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 515, 0, "",         "",       "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 518, 0, "",         "",       "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 519, 0, "",         "",       "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 520, 0, "",         "",       "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 2, 521, 0, "",         "",       "",      48, DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map02[] =
{
    { doom2, 1, 2, 47, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map02[] =
{
    { doom2, 1, 2,  2, 101 },
    { doom2, 1, 2, 11, 101 },
    { doom2, 1, 2, 15, 101 },
    { doom2, 1, 2, 16, 101 },
    { doom2, 1, 2, 32, 101 },
    { doom2, 1, 2, 33, 101 },
    { doom2, 1, 2, 44, 201 },
    { doom2, 1, 2, 55, 231 },
    { doom2, 1, 2, 59, 100 },
    { doom2, 1, 2, 95, 211 },
    { doom2, 1, 2, 96, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 3: THE GANTLET
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map03[] =
{
    { doom2, 1, 3,   2, 0, "", "", "",       0, DEFAULT,       DEFAULT },
    { doom2, 1, 3,   6, 0, "", "", "",      -2, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  14, 0, "", "", "",       0, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  16, 0, "", "", "",      24, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  18, 0, "", "", "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  20, 0, "", "", "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  22, 0, "", "", "",      32, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  24, 0, "", "", "",      56, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  26, 0, "", "", "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  28, 0, "", "", "",      40, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  50, 0, "", "", "", DEFAULT,     -15,       DEFAULT },
    { doom2, 1, 3,  51, 0, "", "", "",      64,     -15,       DEFAULT },
    { doom2, 1, 3,  52, 0, "", "", "",      96,     -15,       DEFAULT },
    { doom2, 1, 3,  71, 0, "", "", "",      32, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  72, 0, "", "", "",     -36, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  73, 0, "", "", "",      -9, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  76, 0, "", "", "",      -3, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  77, 0, "", "", "",      -3, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  80, 0, "", "", "",      10, DEFAULT,       DEFAULT },
    { doom2, 1, 3,  81, 0, "", "", "",     -42, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 128, 0, "", "", "",     112,     -56,       DEFAULT },
    { doom2, 1, 3, 248, 0, "", "", "",     -96,     -56,       DEFAULT },
    { doom2, 1, 3, 169, 0, "", "", "",     -32, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 305, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { doom2, 1, 3, 358, 0, "", "", "",      20,     -10,       DEFAULT },
    { doom2, 1, 3, 369, 0, "", "", "",     -16, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 370, 0, "", "", "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 371, 0, "", "", "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 372, 0, "", "", "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 527, 0, "", "", "",     -16, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 577, 0, "", "", "",       5, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 578, 0, "", "", "",      93, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 579, 0, "", "", "",       5, DEFAULT,       DEFAULT },
    { doom2, 1, 3, 580, 0, "", "", "",      37, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map03[] =
{
    { doom2, 1, 3, 52,        "", "CRATOP2" },
    { doom2, 1, 3, 63, "SWATER4",        "" },
    { doom2, 1, 3, 67, "SWATER4",        "" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map03[] =
{
    { doom2, 1, 3,  15, 102 },
    { doom2, 1, 3,  23, 221 },
    { doom2, 1, 3,  30, 221 },
    { doom2, 1, 3,  31, 221 },
    { doom2, 1, 3,  32, 221 },
    { doom2, 1, 3,  33, 102 },
    { doom2, 1, 3,  34, 102 },
    { doom2, 1, 3,  35, 102 },
    { doom2, 1, 3,  58, 102 },
    { doom2, 1, 3,  59, 102 },
    { doom2, 1, 3,  60, 102 },
    { doom2, 1, 3,  61, 102 },
    { doom2, 1, 3,  63, 101 },
    { doom2, 1, 3,  67, 101 },
    { doom2, 1, 3,  83, 102 },
    { doom2, 1, 3,  89, 101 },
    { doom2, 1, 3,  97, 101 },
    { doom2, 1, 3, 102, 101 },
    { doom2, 1, 3, 108, 101 },
    { doom2, 1, 3, 117, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 4: THE FOCUS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map04[] =
{
    { doom2, 1, 4,   3, 1,         "", "", "STONE6", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4,   8, 0,         "", "",       "",      55, DEFAULT,          DEFAULT },
    { doom2, 1, 4,   9, 0,         "", "",       "",      51, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  29, 0,         "", "",       "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  31, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  36, 0,         "", "",       "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  39, 0,         "", "",       "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  40, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  41, 0,         "", "",       "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  42, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  43, 0,         "", "",       "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  44, 0,         "", "",       "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  45, 0,         "", "",       "",      56, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  46, 0,         "", "",       "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  47, 0,         "", "",       "",      56, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  52, 0,         "", "",       "",     -24, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  53, 0,         "", "",       "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  55, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  57, 0,         "", "",       "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  61, 0,         "", "",       "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  66, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4,  92, 0,         "", "",       "",      48,       8,          DEFAULT },
    { doom2, 1, 4,  94, 0,         "", "",       "",      -8,       8,          DEFAULT },
    { doom2, 1, 4,  96, 0,         "", "",       "",     112,       8,          DEFAULT },
    { doom2, 1, 4,  98, 0,         "", "",       "",      -8,       8,          DEFAULT },
    { doom2, 1, 4, 101, 0,         "", "",       "",      64,       8,          DEFAULT },
    { doom2, 1, 4, 103, 0,         "", "",       "",      -8,       8,          DEFAULT },
    { doom2, 1, 4, 104, 0,         "", "",       "",      16,       8,          DEFAULT },
    { doom2, 1, 4, 106, 0,         "", "",       "",      -8,       8,          DEFAULT },
    { doom2, 1, 4, 108, 0,    "STONE", "",       "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 109, 0,    "STONE", "",       "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 110, 0,    "STONE", "",       "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 111, 0,    "STONE", "",       "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 127, 0,    "STONE", "",       "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 128, 0,    "STONE", "",       "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 163, 0,         "", "",       "",      -1, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 187, 1,         "", "", "STONE6", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 200, 1,         "", "", "STONE6", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 201, 1,         "", "", "STONE6", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 220, 0,         "", "",       "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 259, 0,         "", "",       "",     -40, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 264, 0,         "", "",       "",     -16, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 283, 0,         "", "",       "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 287, 0,         "", "",       "", DEFAULT,      24,          DEFAULT },
    { doom2, 1, 4, 285, 0,         "", "",       "",       5, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 297, 0,         "", "",       "",      39, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 298, 0,         "", "",       "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 308, 0,         "", "",       "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 309, 0,         "", "",       "",      45, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 310, 0,         "", "",       "",      42, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 311, 0,         "", "",       "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 312, 0,         "", "",       "",     -12, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 314, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 315, 0,         "", "",       "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 317, 0,         "", "",       "",     -16, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 357, 0,         "", "",       "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 367, 0,         "", "",       "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 381, 0,         "", "",       "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 396, 0,         "", "",       "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 385, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 400, 0,         "", "",       "",      47, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 401, 0,         "", "",       "",     -24, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 471, 0,         "", "",       "",     -48, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 472, 0,         "", "",       "",     -16, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 473, 0,         "", "",       "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 456, 0,         "", "", "CRATE3", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 456, 1, "SUPPORT3", "",       "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 481, 0,         "", "",       "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 482, 0,         "", "",       "",      48, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 483, 0,         "", "",       "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 497, 0,         "", "",       "",      48, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 511, 0,         "", "",       "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 520, 0,         "", "",       "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 4, 524, 0,         "", "",       "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 4, 534, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 556, 0,         "", "",       "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 557, 0,         "", "",       "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 572, 0,         "", "",       "",     -40, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 573, 0,         "", "",       "",      -8, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 576, 0,         "", "",       "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 577, 0,         "", "",       "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 586, 0,         "", "",       "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 587, 0,         "", "",       "",      55, DEFAULT,          DEFAULT },
    { doom2, 1, 4, 589, 0,         "", "",       "",       8, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map04[] =
{
    { doom2, 1, 4,  6,        "", "FLAT5_4" },
    { doom2, 1, 4, 10, "SLIME15",        "" },
    { doom2, 1, 4, 35,        "", "FLAT5_4" },
    { doom2, 1, 4, 39, "CRATOP1",        "" },
    { doom2, 1, 4, 40, "CRATOP1",        "" },
    { doom2, 1, 4, 46, "CRATOP1",        "" },
    { doom2, 1, 4, 64,        "", "FLAT5_4" },
    { doom2, 1, 4, 77,        "", "FLAT5_4" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map04[] =
{
    { doom2, 1, 4, 24, 100 },
    { doom2, 1, 4, 25, 100 },
    { doom2, 1, 4, 26, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 5: THE WASTE TUNNELS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map05[] =
{
    { doom2, 1, 5,  31, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5,  32, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5,  39, 0,         "", "", "",     -28,     -16,                      ML_DONTPEGTOP },
    { doom2, 1, 5,  41, 0,         "", "", "",      41, DEFAULT,                            DEFAULT },
    { doom2, 1, 5,  43, 0,         "", "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 5,  44, 0,         "", "", "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 5,  46, 0,         "", "", "",      -8, DEFAULT,                            DEFAULT },
    { doom2, 1, 5,  47, 0,         "", "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 5,  50, 0,         "", "", "",     -23, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 233, 0,         "", "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 286, 0,         "", "", "",      -2, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 288, 0,         "", "", "",      18, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 290, 0,         "", "", "",      -2, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 292, 0,         "", "", "",      18, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 294, 0,         "", "", "",      -2, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 296, 0,         "", "", "",      18, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 298, 0,         "", "", "",      -2, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 300, 0,         "", "", "",      18, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 310, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 311, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 312, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 315, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 316, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 318, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 323, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 321, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 322, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 326, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 327, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 330, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 331, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 335, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 336, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 338, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 339, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 340, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 343, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 344, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 348, 0,         "", "", "",      -1, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 345, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 350, 0,         "", "", "",      17, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 355, 0,         "", "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 375, 0,         "", "", "",      25, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 376, 0,         "", "", "",      25, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 414, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 416, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 421, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 422, 0,         "", "", "",     -24, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 423, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 474, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 478, 0,         "", "", "",      16,       8,                            DEFAULT },
    { doom2, 1, 5, 479, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 480, 0,         "", "", "",      24,       8,                            DEFAULT },
    { doom2, 1, 5, 481, 0,         "", "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 5, 482, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 483, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 484, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 485, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 489, 1, "SUPPORT3", "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 524, 0,         "", "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 560, 1, "SUPPORT3", "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 541, 0,         "", "", "",      55,      16,                            DEFAULT },
    { doom2, 1, 5, 542, 0,         "", "", "",      56,      16,                            DEFAULT },
    { doom2, 1, 5, 543, 0,         "", "", "",       8,      16,                            DEFAULT },
    { doom2, 1, 5, 546, 0,         "", "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 556, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 557, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 558, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 559, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 575, 0,         "", "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 5, 601, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 602, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 604, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 608, 0,         "", "", "",      26, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 609, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 610, 0,         "", "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 5, 655, 0,         "", "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 657, 0,         "", "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 670, 0,         "", "", "",      -3, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 671, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 672, 0,         "", "", "",      51, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 711, 0,         "", "", "",      -3, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 799, 0,         "", "", "",     -40, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 801, 0,         "", "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 807, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 808, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 827, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 829, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 833, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 837, 0,         "", "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 850, 0,         "", "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 852, 0,         "", "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 853, 0,         "", "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 5, 854, 0,         "", "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 5, 869, 0,         "", "", "",      -8, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 871, 0,         "", "", "",      -8, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 913, 0,         "", "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 914, 0,         "", "", "",     -24, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 915, 0,         "", "", "",     -56, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 916, 0,         "", "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 917, 0,         "", "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 919, 0,         "", "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 920, 0,         "", "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 5, 946, 0,         "", "", "",      -8, DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map05[] =
{
    { doom2, 1, 5, 26,         "",  "CEIL5_2" },
    { doom2, 1, 5, 41,         "",  "CEIL5_2" },
    { doom2, 1, 5, 42, "FLOOR7_1", "FLOOR7_1" },
    { doom2, 1, 5, 95,  "CEIL5_2",         "" },
    { doom2, 1, 5, 97, "FLOOR7_1", "FLOOR7_1" },
    { doom2, 1, 5, 104,        "",  "CRATOP2" },
    { doom2, 1, 5, 107, "CEIL5_1",         "" },
    { doom2, 1, 5, 155,        "",    "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map05[] =
{
    { doom2, 1, 5,   5, 231 },
    { doom2, 1, 5,   6, 231 },
    { doom2, 1, 5,   7, 231 },
    { doom2, 1, 5,  10, 231 },
    { doom2, 1, 5,  11, 231 },
    { doom2, 1, 5,  12, 231 },
    { doom2, 1, 5,  20, 101 },
    { doom2, 1, 5,  22, 101 },
    { doom2, 1, 5,  63, 101 },
    { doom2, 1, 5,  65, 101 },
    { doom2, 1, 5,  66, 101 },
    { doom2, 1, 5, 141, 101 },
    { doom2, 1, 5, 143, 101 },
    { doom2, 1, 5, 145, 101 },
    { doom2, 1, 5, 146, 101 },
    { doom2, 1, 5, 147, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 6: THE CRUSHER
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map06[] =
{
    { doom2, 1, 6, 332, 0, "", "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 6, 339, 0, "", "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 6, 340, 0, "", "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 6, 836, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 837, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 839, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 840, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 882, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 883, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 886, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 887, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 890, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 891, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 895, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 896, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 899, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 900, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 924, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 925, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 929, 0, "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 6, 930, 0, "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 932, 0, "", "", "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 6, 947, 0, "", "", "",      56, DEFAULT, DEFAULT },
    { doom2, 1, 6, 949, 0, "", "", "",      -8, DEFAULT, DEFAULT },
    { doom2, 1, 6, 952, 0, "", "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 6, 954, 0, "", "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 6, 956, 0, "", "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 6, 960, 0, "", "", "", DEFAULT,     -18, DEFAULT },
    { doom2, 1, 6, 964, 0, "", "", "", DEFAULT,     -18, DEFAULT },
    { doom2, 1, 6, 969, 0, "", "", "", DEFAULT,     -18, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map06[] =
{
    { doom2, 1, 6, 15, "", "FLAT20" },
    { doom2, 1, 6, 25, "", "FLAT20" },
    { doom2, 1, 6, 47, "", "FLAT20" },
    { doom2, 1, 6, 86, "",  "FLAT1" },
    { doom2, 1, 6, 87, "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map06[] =
{
    { doom2, 1, 6,  30, 101 },
    { doom2, 1, 6,  31, 101 },
    { doom2, 1, 6,  50, 101 },
    { doom2, 1, 6,  74, 101 },
    { doom2, 1, 6, 101, 212 },
    { doom2, 1, 6, 168, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 7: DEAD SIMPLE
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map07[] =
{
    { doom2, 1, 7,  23, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  24, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  26, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  27, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  28, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  30, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  31, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  32, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  42, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7,  46, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7,  57, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7,  58, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7,  63, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  64, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  65, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  67, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  68, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  69, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  71, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  73, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  74, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  75, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  77, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  78, 0, "", "",        "", DEFAULT,      16, DEFAULT },
    { doom2, 1, 7,  83, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7,  84, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7, 104, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7, 105, 0, "", "",        "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 7, 160, 0, "", "",        "",      -4, DEFAULT, DEFAULT },
    { doom2, 1, 7, 161, 0, "", "",        "",      14, DEFAULT, DEFAULT },
    { doom2, 1, 7, 168, 1, "", "", "ZIMMER2", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// LEVEL 8: TRICKS AND TRAPS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map08[] =
{
    { doom2, 1, 8,  24, 0,         "",         "",      "",      36,     -72,          DEFAULT },
    { doom2, 1, 8,  29, 0,         "",         "",      "",       5,     -72,          DEFAULT },
    { doom2, 1, 8,  30, 0,         "",         "",      "",      36,     -72,          DEFAULT },
    { doom2, 1, 8,  35, 0,         "",         "",      "",     -12,     -72,          DEFAULT },
    { doom2, 1, 8,  39, 0,         "",         "",      "",      36,     -72,          DEFAULT },
    { doom2, 1, 8,  47, 0,         "",         "",      "",     -12,     -72,          DEFAULT },
    { doom2, 1, 8,  50, 0,         "",         "",      "",      -8,      24,          DEFAULT },
    { doom2, 1, 8,  55, 0,         "",         "",      "",      36,      24,          DEFAULT },
    { doom2, 1, 8,  68, 0,         "",         "",      "",      -4,     -72,          DEFAULT },
    { doom2, 1, 8,  75, 0,         "",         "",      "",      -4,     -72,          DEFAULT },
    { doom2, 1, 8,  79, 0,         "",         "",      "",      36,     -72,          DEFAULT },
    { doom2, 1, 8,  87, 0,         "",         "",      "",     -27,     -72,          DEFAULT },
    { doom2, 1, 8,  88, 0,         "",         "",      "",     -27,     -72,          DEFAULT },
    { doom2, 1, 8,  96, 0,         "",         "",      "",      23,     -72,          DEFAULT },
    { doom2, 1, 8, 101, 1, "TANROCK4",         "",      "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 183, 0,         "",         "",      "",     -13, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 196, 0,         "",         "",      "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 232, 0,         "",         "", "STEP2", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 237, 0,         "",         "",      "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 249, 1, "COMPWERD",         "",      "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 253, 0,         "",         "",      "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 265, 0,         "",         "",      "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 270, 0,         "",         "",      "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 8, 270, 1, "COMPTALL",         "",      "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 276, 0,         "",         "",      "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 8, 276, 1, "COMPTALL",         "",      "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 282, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 283, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 287, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 288, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 289, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 290, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 295, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 296, 0,         "",         "",      "", DEFAULT,     -48,    ML_DONTPEGTOP },
    { doom2, 1, 8, 353, 0,         "",         "",      "", DEFAULT,      46,          DEFAULT },
    { doom2, 1, 8, 542, 0,         "", "EXITDOOR",      "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 545, 0,         "",         "",      "",     136, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 547, 0,         "",         "",      "",    -102, DEFAULT,          DEFAULT },
    { doom2, 1, 8, 598, 0,         "",    "GRAY5",      "", DEFAULT, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map08[] =
{
    { doom2, 1, 8, 92, "", "CEIL3_6" },
    { doom2, 1, 8, 94, "", "CEIL3_6" },
    { doom2, 1, 8, 95, "", "CEIL3_6" },
    { doom2, 1, 8, 96, "", "CEIL3_6" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map08[] =
{
    { doom2, 1, 8,  8, 221 },
    { doom2, 1, 8,  9, 221 },
    { doom2, 1, 8, 10, 221 },
    { doom2, 1, 8, 11, 221 },
    { doom2, 1, 8, 14, 221 },
    { doom2, 1, 8, 38, 100 },
    { doom2, 1, 8, 40, 100 },
    { doom2, 1, 8, 41, 100 },
    { doom2, 1, 8, 54, 100 },
    { doom2, 1, 8, 88, 100 },
    { doom2, 1, 8, 89, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 9: THE PIT
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map09[] =
{
    { doom2, 1, 9,  25, 0, "", "",      "", DEFAULT,      56,          DEFAULT },
    { doom2, 1, 9,  27, 0, "", "",      "",     -24,      56,          DEFAULT },
    { doom2, 1, 9,  45, 0, "", "",      "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 9,  54, 0, "", "",      "",     112, DEFAULT,          DEFAULT },
    { doom2, 1, 9,  55, 0, "", "",      "",     192, DEFAULT,          DEFAULT },
    { doom2, 1, 9,  81, 0, "", "",      "",      56, DEFAULT,          DEFAULT },
    { doom2, 1, 9,  82, 0, "", "",      "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 9,  83, 0, "", "",      "",      56, DEFAULT,          DEFAULT },
    { doom2, 1, 9,  94, 0, "", "",      "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 110, 0, "", "",      "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 111, 0, "", "",      "",      56, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 112, 0, "", "",      "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 160, 0, "", "",      "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 9, 228, 0, "", "",      "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 292, 0, "", "", "STEP4", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 304, 0, "", "",      "",      53, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 332, 0, "", "",      "",       2, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 334, 0, "", "",      "",       2, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 336, 0, "", "",      "",      -3, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 341, 0, "", "",      "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 348, 0, "", "",      "",     -40, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 381, 0, "", "",      "",       2, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 524, 0, "", "",      "",     -15, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 553, 0, "", "",      "",     176, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 628, 1, "", "", "PIPE4", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 9, 631, 0, "", "",      "",       8, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map09[] =
{
    { doom2, 1, 9,  27, "",  "FLAT20" },
    { doom2, 1, 9,  60, "",  "FLAT20" },
    { doom2, 1, 9,  81, "", "CEIL5_2" },
    { doom2, 1, 9,  83, "", "CEIL5_2" },
    { doom2, 1, 9, 111, "", "CEIL5_2" },
    { doom2, 1, 9, 112, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map09[] =
{
    { doom2, 1, 9,  15, 101 },
    { doom2, 1, 9,  35, 100 },
    { doom2, 1, 9,  39, 101 },
    { doom2, 1, 9,  56, 101 },
    { doom2, 1, 9, 104, 100 },
    { doom2, 1, 9, 107, 100 },
    { doom2, 1, 9, 108, 100 },
    { doom2, 1, 9, 114, 101 },
    { doom2, 1, 9, 126, 101 },
    { doom2, 1, 9, 128, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 10: REFUELING BASE
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map10[] =
{
    { doom2, 1, 10, 190, 0, "",         "", "",                 32,       8, DEFAULT },
    { doom2, 1, 10, 195, 0, "",         "", "",                -24,       8, DEFAULT },
    { doom2, 1, 10, 208, 0, "",         "", "",                -24,      -8, DEFAULT },
    { doom2, 1, 10, 209, 0, "",         "", "",                -24,      -8, DEFAULT },
    { doom2, 1, 10, 408, 0, "",         "", "",               -224, DEFAULT, DEFAULT },
    { doom2, 1, 10, 409, 0, "",         "", "",                  0, DEFAULT, DEFAULT },
    { doom2, 1, 10, 410, 0, "",         "", "",                 30, DEFAULT, DEFAULT },
    { doom2, 1, 10, 411, 0, "",         "", "",                 31, DEFAULT, DEFAULT },
    { doom2, 1, 10, 412, 0, "",         "", "",                  7, DEFAULT, DEFAULT },
    { doom2, 1, 10, 493, 0, "",         "", "",            DEFAULT,      48, DEFAULT },
    { doom2, 1, 10, 494, 0, "",         "", "",            DEFAULT,      48, DEFAULT },
    { doom2, 1, 10, 495, 0, "", "DOORTRAK", "",            DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 10, 496, 0, "", "DOORTRAK", "",            DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 10, 497, 0, "",         "", "",            DEFAULT,      48, DEFAULT },
    { doom2, 1, 10, 498, 0, "",         "", "",            DEFAULT,      48, DEFAULT },
    { doom2, 1, 10, 625, 0, "",         "", "",                  4, DEFAULT, DEFAULT },
    { doom2, 1, 10, 638, 0, "",         "", "",            DEFAULT,      72, DEFAULT },
    { doom2, 1, 10, 668, 0, "",         "", "",                  4, DEFAULT, DEFAULT },
    { doom2, 1, 10, 669, 0, "",         "", "",                 62, DEFAULT, DEFAULT },
    { doom2, 1, 10, 670, 0, "",         "", "",                 55, DEFAULT, DEFAULT },
    { doom2, 1, 10, 671, 0, "",         "", "",                 15, DEFAULT, DEFAULT },
    { doom2, 1, 10, 672, 0, "",         "", "",                 58, DEFAULT, DEFAULT },
    { doom2, 1, 10, 673, 0, "",         "", "",                 61, DEFAULT, DEFAULT },
    { doom2, 1, 10, 676, 0, "",         "", "",                -36, DEFAULT, DEFAULT },
    { doom2, 1, 10, 677, 0, "",         "", "",                -32, DEFAULT, DEFAULT },
    { doom2, 1, 10, 678, 0, "",         "", "",                -11, DEFAULT, DEFAULT },
    { doom2, 1, 10, 679, 0, "",         "", "",                -48, DEFAULT, DEFAULT },
    { doom2, 1, 10, 782, 0, "",         "", "",                103, DEFAULT, DEFAULT },
    { doom2, 1, 10, 821, 0, "",         "", "",                192, DEFAULT, DEFAULT },
    { doom2, 1, 10, 822, 0, "",         "", "",               -128, DEFAULT, DEFAULT },
    { doom2, 1, 10, 823, 0, "",         "", "",               -192, DEFAULT, DEFAULT },
    { doom2, 1, 10, 898, 0, "", "BRNSMAL1", "",            DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 10, 899, 1, "",        "-", "",            DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 10, 900, 1, "",        "-", "",            DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 10, 901, 1, "",        "-", "",            DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 10, 902, 1, "",        "-", "",            DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 10, 946, 0, "",         "", "",                  8, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map10[] =
{
    { doom2, 1, 10,   6, "",  "FLAT20" },
    { doom2, 1, 10,  15, "",  "FLAT20" },
    { doom2, 1, 10,  37, "",  "FLAT20" },
    { doom2, 1, 10,  52, "",  "FLAT20" },
    { doom2, 1, 10,  75, "", "FLAT5_4" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map10[] =
{
    { doom2, 1, 10,  98, 100 },
    { doom2, 1, 10, 110, 100 },
    { doom2, 1, 10, 113, 100 },
    { doom2, 1, 10, 115, 100 },
    { doom2, 1, 10, 127, 101 },
    { doom2, 1, 10, 139, 101 },
    { doom2, 1, 10, 140, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 11: 'O' OF DESTRUCTION!
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map11[] =
{
    { doom2, 1, 11,  99, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 101, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 103, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 105, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 110, 0, "", "", "", -32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 136, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 203, 0, "", "", "",  -8, DEFAULT, DEFAULT },
    { doom2, 1, 11, 294, 0, "", "", "",  26, DEFAULT, DEFAULT },
    { doom2, 1, 11, 295, 0, "", "", "", -32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 298, 0, "", "", "",  26, DEFAULT, DEFAULT },
    { doom2, 1, 11, 299, 0, "", "", "", -32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 346, 0, "", "", "", 104, DEFAULT, DEFAULT },
    { doom2, 1, 11, 347, 0, "", "", "", 247, DEFAULT, DEFAULT },
    { doom2, 1, 11, 348, 0, "", "", "",  91, DEFAULT, DEFAULT },
    { doom2, 1, 11, 349, 0, "", "", "", 131, DEFAULT, DEFAULT },
    { doom2, 1, 11, 350, 0, "", "", "",  86, DEFAULT, DEFAULT },
    { doom2, 1, 11, 354, 0, "", "", "",  80, DEFAULT, DEFAULT },
    { doom2, 1, 11, 355, 0, "", "", "", 176, DEFAULT, DEFAULT },
    { doom2, 1, 11, 356, 0, "", "", "", 248, DEFAULT, DEFAULT },
    { doom2, 1, 11, 357, 0, "", "", "",  56, DEFAULT, DEFAULT },
    { doom2, 1, 11, 358, 0, "", "", "", 120, DEFAULT, DEFAULT },
    { doom2, 1, 11, 359, 0, "", "", "", 184, DEFAULT, DEFAULT },
    { doom2, 1, 11, 392, 0, "", "", "", -52, DEFAULT, DEFAULT },
    { doom2, 1, 11, 412, 0, "", "", "", -12, DEFAULT, DEFAULT },
    { doom2, 1, 11, 413, 0, "", "", "", -12, DEFAULT, DEFAULT },
    { doom2, 1, 11, 414, 0, "", "", "", -33,      24, DEFAULT },
    { doom2, 1, 11, 415, 0, "", "", "", -16,      24, DEFAULT },
    { doom2, 1, 11, 458, 0, "", "", "",  23,      24, DEFAULT },
    { doom2, 1, 11, 461, 0, "", "", "", -55, DEFAULT, DEFAULT },
    { doom2, 1, 11, 467, 0, "", "", "",   4, DEFAULT, DEFAULT },
    { doom2, 1, 11, 468, 0, "", "", "",  20, DEFAULT, DEFAULT },
    { doom2, 1, 11, 469, 0, "", "", "",  43, DEFAULT, DEFAULT },
    { doom2, 1, 11, 470, 0, "", "", "",   3, DEFAULT, DEFAULT },
    { doom2, 1, 11, 476, 0, "", "", "",  56, DEFAULT, DEFAULT },
    { doom2, 1, 11, 477, 0, "", "", "",  13, DEFAULT, DEFAULT },
    { doom2, 1, 11, 509, 0, "", "", "", -32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 515, 0, "", "", "",  56, DEFAULT, DEFAULT },
    { doom2, 1, 11, 537, 0, "", "", "",  -8, DEFAULT, DEFAULT },
    { doom2, 1, 11, 538, 0, "", "", "",  48, DEFAULT, DEFAULT },
    { doom2, 1, 11, 540, 0, "", "", "",   0, DEFAULT, DEFAULT },
    { doom2, 1, 11, 541, 0, "", "", "",  40, DEFAULT, DEFAULT },
    { doom2, 1, 11, 561, 0, "", "", "",  40, DEFAULT, DEFAULT },
    { doom2, 1, 11, 574, 0, "", "", "",  24, DEFAULT, DEFAULT },
    { doom2, 1, 11, 579, 0, "", "", "",   8, DEFAULT, DEFAULT },
    { doom2, 1, 11, 583, 0, "", "", "",  -8, DEFAULT, DEFAULT },
    { doom2, 1, 11, 586, 0, "", "", "",  48, DEFAULT, DEFAULT },
    { doom2, 1, 11, 587, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 617, 0, "", "", "",  12, DEFAULT, DEFAULT },
    { doom2, 1, 11, 628, 0, "", "", "",  56, DEFAULT, DEFAULT },
    { doom2, 1, 11, 629, 0, "", "", "",  30, DEFAULT, DEFAULT },
    { doom2, 1, 11, 630, 0, "", "", "",  22, DEFAULT, DEFAULT },
    { doom2, 1, 11, 631, 0, "", "", "",  22, DEFAULT, DEFAULT },
    { doom2, 1, 11, 632, 0, "", "", "",  22, DEFAULT, DEFAULT },
    { doom2, 1, 11, 667, 0, "", "", "", -24, DEFAULT, DEFAULT },
    { doom2, 1, 11, 671, 0, "", "", "", -48, DEFAULT, DEFAULT },
    { doom2, 1, 11, 680, 0, "", "", "", -32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 683, 0, "", "", "", -32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 688, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 694, 0, "", "", "",  48, DEFAULT, DEFAULT },
    { doom2, 1, 11, 699, 0, "", "", "",  -8, DEFAULT, DEFAULT },
    { doom2, 1, 11, 701, 0, "", "", "", -56, DEFAULT, DEFAULT },
    { doom2, 1, 11, 703, 0, "", "", "",   8, DEFAULT, DEFAULT },
    { doom2, 1, 11, 705, 0, "", "", "",  24, DEFAULT, DEFAULT },
    { doom2, 1, 11, 707, 0, "", "", "",  40, DEFAULT, DEFAULT },
    { doom2, 1, 11, 715, 0, "", "", "",  -4, DEFAULT, DEFAULT },
    { doom2, 1, 11, 716, 0, "", "", "",  -4, DEFAULT, DEFAULT },
    { doom2, 1, 11, 764, 0, "", "", "",  -3, DEFAULT, DEFAULT },
    { doom2, 1, 11, 766, 0, "", "", "",  -3, DEFAULT, DEFAULT },
    { doom2, 1, 11, 768, 0, "", "", "",  -3, DEFAULT, DEFAULT },
    { doom2, 1, 11, 770, 0, "", "", "",  -3, DEFAULT, DEFAULT },
    { doom2, 1, 11, 785, 0, "", "", "",  48, DEFAULT, DEFAULT },
    { doom2, 1, 11, 787, 0, "", "", "",  16, DEFAULT, DEFAULT },
    { doom2, 1, 11, 788, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 789, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 790, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 791, 0, "", "", "",  32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 795, 0, "", "", "",  20, DEFAULT, DEFAULT },
    { doom2, 1, 11, 796, 0, "", "", "",   7, DEFAULT, DEFAULT },
    { doom2, 1, 11, 811, 0, "", "", "",  53, DEFAULT, DEFAULT },
    { doom2, 1, 11, 812, 0, "", "", "",  29, DEFAULT, DEFAULT },
    { doom2, 1, 11, 822, 0, "", "", "", -32, DEFAULT, DEFAULT },
    { doom2, 1, 11, 840, 0, "", "", "",  22, DEFAULT, DEFAULT },
    { doom2, 1, 11, 842, 0, "", "", "",  20, DEFAULT, DEFAULT },
    { doom2, 1, 11, 844, 0, "", "", "",   7, DEFAULT, DEFAULT },
    { doom2, 1, 11, 845, 0, "", "", "",   7, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map11[] =
{
{ doom2, 1, 11, 90, "SLIME16", "" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map11[] =
{
    { doom2, 1, 11,   0, 102 },
    { doom2, 1, 11,   9, 102 },
    { doom2, 1, 11,  30, 101 },
    { doom2, 1, 11,  38, 102 },
    { doom2, 1, 11,  39, 102 },
    { doom2, 1, 11,  45, 102 },
    { doom2, 1, 11,  48, 102 },
    { doom2, 1, 11,  49, 102 },
    { doom2, 1, 11,  56, 102 },
    { doom2, 1, 11, 109, 232 },
    { doom2, 1, 11, 112, 232 },
    { doom2, 1, 11, 113, 232 },
    { doom2, 1, 11, 118, 232 },
    { doom2, 1, 11, 119, 232 },
    { doom2, 1, 11, 120, 232 },
    { doom2, 1, 11, 121, 232 },
    { doom2, 1, 11, 122, 232 },
    { doom2, 1, 11, 127, 232 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 12: THE FACTORY
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map12[] =
{
    { doom2, 1, 12,  41, 0, "",         "",    "", DEFAULT,      48,     DEFAULT },
    { doom2, 1, 12,  42, 0, "",         "",    "",      48,      48,     DEFAULT },
    { doom2, 1, 12,  43, 0, "",         "",    "",      24,      48,     DEFAULT },
    { doom2, 1, 12,  45, 0, "",         "",    "",      -8,      48,     DEFAULT },
    { doom2, 1, 12,  69, 0, "",         "",    "",      32, DEFAULT,     DEFAULT },
    { doom2, 1, 12,  70, 0, "",         "",    "", DEFAULT,       0,     DEFAULT },
    { doom2, 1, 12,  71, 0, "",         "",    "",      32,       0,     DEFAULT },
    { doom2, 1, 12,  73, 0, "",         "",    "", DEFAULT,      16,     DEFAULT },
    { doom2, 1, 12,  75, 0, "",         "",    "",      -1,      16,     DEFAULT },
    { doom2, 1, 12,  80, 0, "",         "",    "",     -40,      16,     DEFAULT },
    { doom2, 1, 12,  83, 0, "",         "",    "", DEFAULT,      16,     DEFAULT },
    { doom2, 1, 12,  85, 0, "",         "",    "",     -48,      16,     DEFAULT },
    { doom2, 1, 12,  89, 0, "",         "",    "", DEFAULT,      16,     DEFAULT },
    { doom2, 1, 12,  91, 0, "",         "",    "",      -1,      16,     DEFAULT },
    { doom2, 1, 12,  93, 0, "",         "",    "", DEFAULT,      16,     DEFAULT },
    { doom2, 1, 12,  95, 0, "",         "",    "",      -1,      16,     DEFAULT },
    { doom2, 1, 12,  96, 0, "",         "",    "", DEFAULT,      16,     DEFAULT },
    { doom2, 1, 12,  98, 0, "",         "",    "",     -48,      16,     DEFAULT },
    { doom2, 1, 12, 100, 0, "",         "",    "", DEFAULT,      16,     DEFAULT },
    { doom2, 1, 12, 102, 0, "",         "",    "",     -48,      16,     DEFAULT },
    { doom2, 1, 12, 112, 0, "",         "",    "", DEFAULT,      16,     DEFAULT },
    { doom2, 1, 12, 115, 0, "",         "",    "",     -16,      16,     DEFAULT },
    { doom2, 1, 12, 218, 0, "",         "",    "", DEFAULT,      32,     DEFAULT },
    { doom2, 1, 12, 223, 0, "",         "",    "", DEFAULT,      32,     DEFAULT },
    { doom2, 1, 12, 233, 0, "",         "",    "",       4, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 250, 0, "",         "",    "",      -4, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 251, 0, "",         "",    "",      -4, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 267, 0, "",         "",    "",      56, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 269, 0, "",   "PANEL9",    "", DEFAULT, DEFAULT, ML_DONTDRAW },
    { doom2, 1, 12, 296, 0, "",         "",    "",      80, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 326, 0, "",         "",    "",      24, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 327, 0, "",         "",    "", DEFAULT,     -16,     DEFAULT },
    { doom2, 1, 12, 328, 0, "",         "",    "", DEFAULT,     -48,     DEFAULT },
    { doom2, 1, 12, 344, 0, "",         "",    "",      56, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 348, 0, "",         "",    "",       4, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 350, 0, "",         "",    "",       4, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 458, 0, "",         "",    "",      -8, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 464, 0, "",         "",    "",      -8, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 471, 0, "",         "",    "",      -8, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 549, 0, "",         "",    "", DEFAULT, DEFAULT, ML_DONTDRAW },
    { doom2, 1, 12, 556, 0, "",         "",    "", DEFAULT, DEFAULT, ML_DONTDRAW },
    { doom2, 1, 12, 557, 0, "",         "",    "", DEFAULT, DEFAULT, ML_DONTDRAW },
    { doom2, 1, 12, 558, 0, "",         "",    "", DEFAULT, DEFAULT, ML_DONTDRAW },
    { doom2, 1, 12, 613, 0, "",         "",    "",       4, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 622, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 625, 0, "", "PANBORD1",    "",       8, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 630, 0, "", "PANBORD1",    "",       8, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 639, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 641, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 643, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 645, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 652, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 654, 0, "",         "",    "",      30, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 656, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 658, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 665, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 667, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 669, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 671, 0, "",         "",    "",      30, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 707, 0, "",         "",    "",       6, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 752, 0, "",         "",    "",       5, DEFAULT,     DEFAULT },
    { doom2, 1, 12, 754, 0, "",         "",    "",     -20, DEFAULT,     DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map12[] =
{
    { doom2, 1, 12,   8, "",  "CEIL5_2" },
    { doom2, 1, 12,  10, "",  "CEIL5_2" },
    { doom2, 1, 12,  18, "",  "CEIL5_2" },
    { doom2, 1, 12,  39, "",  "CEIL5_2" },
    { doom2, 1, 12,  42, "",  "CEIL5_2" },
    { doom2, 1, 12, 134, "", "FLOOR4_6" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map12[] =
{
    { doom2, 1, 12,  44, 100 },
    { doom2, 1, 12,  91, 100 },
    { doom2, 1, 12,  93, 100 },
    { doom2, 1, 12,  94, 100 },
    { doom2, 1, 12,  99, 100 },
    { doom2, 1, 12, 100, 100 },
    { doom2, 1, 12, 101, 100 },
    { doom2, 1, 12, 102, 100 },
    { doom2, 1, 12, 106, 100 },
    { doom2, 1, 12, 107, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 13: DOWNTOWN
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map13[] =
{
    { doom2, 1, 13,   40, 0,         "",         "", "", DEFAULT,       8,     DEFAULT },
    { doom2, 1, 13,   52, 0,         "",         "", "",      32, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  105, 0,         "",         "", "",      19, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  106, 0,         "",         "", "",       8, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  147, 0,         "",         "", "",     -24,     112,     DEFAULT },
    { doom2, 1, 13,  190, 0,         "",         "", "",     -48, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  191, 0,         "",         "", "",       0, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  192, 0,         "",         "", "",     -83, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  193, 0,         "",         "", "",     -35, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  195, 0,         "",         "", "",      58, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  196, 0,         "",         "", "",     103, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  197, 0,         "",         "", "",      23, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  268, 0,         "",         "", "", DEFAULT, DEFAULT, ML_DONTDRAW },
    { doom2, 1, 13,  320, 0,         "",         "", "", DEFAULT, DEFAULT, ML_DONTDRAW },
    { doom2, 1, 13,  474, 0,         "",         "", "",      -8,      48,     DEFAULT },
    { doom2, 1, 13,  484, 0,         "",         "", "",      56,      48,     DEFAULT },
    { doom2, 1, 13,  528, 0,         "",         "", "",     -40, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  529, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  553, 1,         "", "MIDBRONZ", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  546, 1,         "", "MIDBRONZ", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  547, 1,         "", "MIDBRONZ", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  548, 1,         "", "MIDBRONZ", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  574, 0,         "",         "", "", DEFAULT,      32,     DEFAULT },
    { doom2, 1, 13,  575, 0,         "",         "", "",     -24,      32,     DEFAULT },
    { doom2, 1, 13,  581, 0,         "",         "", "",     -32,      32,     DEFAULT },
    { doom2, 1, 13,  582, 0,         "",         "", "", DEFAULT,      32,     DEFAULT },
    { doom2, 1, 13,  584, 0,         "",         "", "",      56,      24,     DEFAULT },
    { doom2, 1, 13,  592, 0,         "",         "", "",      -8,      24,     DEFAULT },
    { doom2, 1, 13,  595, 0, "MARBGRAY",         "", "",      24, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  603, 0,         "",         "", "",      48, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  609, 0,         "",         "", "",      12, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  610, 0,         "",         "", "",       4,      -8,     DEFAULT },
    { doom2, 1, 13,  617, 0,         "",         "", "",      56, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  619, 0,         "",         "", "",       4,      -8,     DEFAULT },
    { doom2, 1, 13,  620, 0,         "",         "", "",      40, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  621, 0,         "",         "", "", DEFAULT,     -16,     DEFAULT },
    { doom2, 1, 13,  622, 1, "BROWNGRN",         "", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  624, 0,         "",         "", "",      -1,      -8,     DEFAULT },
    { doom2, 1, 13,  629, 0,         "",         "", "",      -7,      -8,     DEFAULT },
    { doom2, 1, 13,  640, 0,         "",         "", "",      64, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  644, 0,         "",         "", "",     -57, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  645, 0,         "",         "", "",      56, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  657, 0,         "",         "", "",     -32, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  659, 0,         "",         "", "",     -32, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  661, 0,         "",         "", "",      -6, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  754, 0,         "", "DOORTRAK", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  756, 0,         "",         "", "",      24, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  757, 0,         "",         "", "",       8, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  769, 0,         "",         "", "",      48, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  770, 0,         "",         "", "",       8, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  771, 0,         "",         "", "",      40, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  774, 0,         "",         "", "",      32, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  790, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  791, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  792, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  804, 0,         "",         "", "",      16, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  806, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  807, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  810, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  811, 1,         "", "MIDBARS3", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  821, 0,         "",         "", "",      16, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  831, 0,         "",         "", "",       8, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  842, 0,         "",         "", "",      -8,      40,     DEFAULT },
    { doom2, 1, 13,  846, 0,         "",         "", "",      64,      40,     DEFAULT },
    { doom2, 1, 13,  858, 0,         "",         "", "",       4, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  879, 1,         "", "MIDBRONZ", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  880, 1,         "", "MIDBRONZ", "", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1, 13,  946, 0,         "",         "", "", DEFAULT,      32,     DEFAULT },
    { doom2, 1, 13,  950, 0,         "",         "", "", DEFAULT,      32,     DEFAULT },
    { doom2, 1, 13,  952, 0,         "",         "", "", DEFAULT,     -32,     DEFAULT },
    { doom2, 1, 13,  968, 0,         "",         "", "",      60,     -24,     DEFAULT },
    { doom2, 1, 13,  970, 0,         "",         "", "",      -4,      40,     DEFAULT },
    { doom2, 1, 13,  980, 0,         "",         "", "", DEFAULT,       8,     DEFAULT },
    { doom2, 1, 13, 1005, 0,         "",         "", "", DEFAULT,       8,     DEFAULT },
    { doom2, 1, 13, 1054, 0,         "",         "", "",      32,     -24,     DEFAULT },
    { doom2, 1, 13, 1055, 0,         "",         "", "",     -32,     -24,     DEFAULT },
    { doom2, 1, 13, 1071, 0,         "",         "", "",     -32, DEFAULT,     DEFAULT },
    { doom2, 1, 13, 1072, 0,         "",         "", "",      24, DEFAULT,     DEFAULT },
    { doom2, 1, 13, 1084, 0,         "",         "", "",      16, DEFAULT,     DEFAULT },
    { doom2, 1, 13, 1096, 0,         "",         "", "",      16, DEFAULT,     DEFAULT },
    { doom2, 1, 13, 1102, 0,         "",         "", "",     -48,     -64,     DEFAULT },
    { doom2, 1, 13, 1103, 0,         "",         "", "",       0,      64,     DEFAULT },
    { doom2, 1, 13, 1104, 0,         "",         "", "",       0,      64,     DEFAULT },
    { doom2, 1, 13, 1105, 0,         "",         "", "",      48,     -64,     DEFAULT },
    { doom2, 1, 13, 1106, 0,         "",         "", "",      77,      64,     DEFAULT },
    { doom2, 1, 13, 1107, 0,         "",         "", "",     -24, DEFAULT,     DEFAULT },
    { doom2, 1, 13, 1108, 0,         "",         "", "",     -16, DEFAULT,     DEFAULT },
    { doom2, 1, 13, 1155, 0,         "",         "", "",      -8,    -120,     DEFAULT },
    { doom2, 1, 13, 1157, 0,         "",         "", "",       8,    -120,     DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map13[] =
{
    { doom2, 1, 13,  72, "FLAT5_4",        "" },
    { doom2, 1, 13, 151,        "", "CEIL5_1" },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// LEVEL 14: THE INMOST DENS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map14[] =
{
    { doom2, 1, 14,   48, 0,         "",         "", "",      16,      40,                            DEFAULT },
    { doom2, 1, 14,   49, 0,         "",         "", "",      32,      40,                            DEFAULT },
    { doom2, 1, 14,   50, 0,         "",         "", "",      16,      40,                            DEFAULT },
    { doom2, 1, 14,   51, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,   52, 0,         "",         "", "",      32,      40,                            DEFAULT },
    { doom2, 1, 14,   53, 0,         "",         "", "",      32,      40,                            DEFAULT },
    { doom2, 1, 14,   54, 0,         "",         "", "",      40,      16,                            DEFAULT },
    { doom2, 1, 14,   55, 0,         "",         "", "",      16,      40,                            DEFAULT },
    { doom2, 1, 14,   56, 0,         "",         "", "",      32,      40,                            DEFAULT },
    { doom2, 1, 14,   57, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,   58, 0,         "",         "", "",      16,      40,                            DEFAULT },
    { doom2, 1, 14,   59, 0,         "",         "", "",      16,      16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   60, 0,         "",         "", "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   61, 0,         "",         "", "",      16,      16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   62, 0,         "",         "", "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   63, 0,         "",         "", "",      48,      16,                            DEFAULT },
    { doom2, 1, 14,   64, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,   65, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,   66, 0,         "",         "", "",      40,      40,                            DEFAULT },
    { doom2, 1, 14,   67, 0,         "",         "", "",      24,      40,                            DEFAULT },
    { doom2, 1, 14,   69, 0,         "",         "", "",      40,      40,                            DEFAULT },
    { doom2, 1, 14,   70, 0,         "",         "", "",      24,      40,                            DEFAULT },
    { doom2, 1, 14,   72, 0,         "",         "", "",      40,      40,                            DEFAULT },
    { doom2, 1, 14,   73, 0,         "",         "", "",      24,      40,                            DEFAULT },
    { doom2, 1, 14,   75, 0,         "",         "", "",      40,      40,                            DEFAULT },
    { doom2, 1, 14,   76, 0,         "",         "", "",      24,      40,                            DEFAULT },
    { doom2, 1, 14,   79, 0,         "",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   81, 0,         "",         "", "",      24,      16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   83, 0,         "",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   85, 0,         "",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,   86, 0,         "",         "", "",      48,      16,                            DEFAULT },
    { doom2, 1, 14,   87, 0,         "",         "", "",     -24,      24,                            DEFAULT },
    { doom2, 1, 14,   95, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  109, 0,         "",         "", "", DEFAULT,      24,                            DEFAULT },
    { doom2, 1, 14,  110, 0,         "",         "", "",      33,      24,                            DEFAULT },
    { doom2, 1, 14,  114, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  121, 1,         "",         "", "",      16,      16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  122, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  123, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  124, 0,         "",         "", "",      16,      40,                            DEFAULT },
    { doom2, 1, 14,  126, 0,         "",         "", "",      32,      40,                            DEFAULT },
    { doom2, 1, 14,  129, 1,         "",         "", "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  131, 0,         "",         "", "", DEFAULT,      80,                            DEFAULT },
    { doom2, 1, 14,  133, 0,         "",         "", "",     -16,      80,                            DEFAULT },
    { doom2, 1, 14,  135, 0,         "",         "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  143, 0,         "",         "", "",      48,      16,                            DEFAULT },
    { doom2, 1, 14,  145, 0,         "",         "", "",      24,      40,                            DEFAULT },
    { doom2, 1, 14,  146, 0,         "",         "", "",      40,      40,                            DEFAULT },
    { doom2, 1, 14,  147, 0,         "",         "", "",      24,      16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  151, 0,         "",         "", "", DEFAULT,      16,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  153, 0,         "",         "", "", DEFAULT,      16,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  155, 0,         "",         "", "", DEFAULT,      16,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  157, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  158, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  159, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  160, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  161, 0,         "",         "", "",     -16,      16,                            DEFAULT },
    { doom2, 1, 14,  162, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  163, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  164, 0,         "",         "", "",     -16,      16,                            DEFAULT },
    { doom2, 1, 14,  165, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  166, 0,         "",         "", "",     -16,      16,                            DEFAULT },
    { doom2, 1, 14,  167, 0,         "",         "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  168, 0,         "",         "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  169, 0,         "",         "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  170, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  172, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  179, 0,         "",         "", "", DEFAULT,      16,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  181, 0,         "",         "", "", DEFAULT,      16,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  184, 0,         "",         "", "",     -16,      16,                            DEFAULT },
    { doom2, 1, 14,  185, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  186, 0,         "",         "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  187, 0,         "",         "", "",     -16,      16,                            DEFAULT },
    { doom2, 1, 14,  188, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  189, 0,         "",         "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  192, 0,         "",         "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  193, 0,  "BSTONE2",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  199, 0,         "",         "", "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  201, 0,         "",         "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  203, 0,         "",         "", "", DEFAULT,     -16,                            DEFAULT },
    { doom2, 1, 14,  204, 0,         "",         "", "",     -24,      16,                            DEFAULT },
    { doom2, 1, 14,  205, 0,         "",         "", "", DEFAULT,      16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  206, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  216, 0,         "",         "", "",      32,      56,                            DEFAULT },
    { doom2, 1, 14,  218, 0,         "",         "", "",      -8,      56,                            DEFAULT },
    { doom2, 1, 14,  220, 1,  "BSTONE2",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  223, 0,         "",         "", "", DEFAULT,     -16,                            DEFAULT },
    { doom2, 1, 14,  224, 0,         "",         "", "", DEFAULT,     -16,                            DEFAULT },
    { doom2, 1, 14,  294, 0,         "",         "", "",     -48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  296, 0,         "",         "", "",       8,      48,                            DEFAULT },
    { doom2, 1, 14,  297, 0,         "",         "", "",     -24,     -72,                            DEFAULT },
    { doom2, 1, 14,  298, 0,         "",         "", "",      40,     -88,                            DEFAULT },
    { doom2, 1, 14,  299, 0,         "",         "", "",      24,      48,                            DEFAULT },
    { doom2, 1, 14,  300, 0,         "",         "", "",     -32,     -88,                            DEFAULT },
    { doom2, 1, 14,  312, 0,         "",         "", "",     -56,     -48,                            DEFAULT },
    { doom2, 1, 14,  313, 0,         "",         "", "",      56,     -88,                            DEFAULT },
    { doom2, 1, 14,  321, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  323, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  325, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  326, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  327, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  328, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  329, 0,         "",         "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  350, 0,         "",         "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  351, 0,         "",         "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  353, 0,         "",         "", "",     -32,      16,                            DEFAULT },
    { doom2, 1, 14,  354, 0,         "",         "", "",     -48,      16,                            DEFAULT },
    { doom2, 1, 14,  355, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  356, 0,         "",         "", "",     -16,      16,                            DEFAULT },
    { doom2, 1, 14,  357, 0,         "",         "", "",     -32,      16,                            DEFAULT },
    { doom2, 1, 14,  358, 0,         "",         "", "",      16,     -16,                            DEFAULT },
    { doom2, 1, 14,  360, 0,         "",         "", "",      32,     -16,                            DEFAULT },
    { doom2, 1, 14,  362, 0,         "",         "", "",      48,     -16,                            DEFAULT },
    { doom2, 1, 14,  364, 0,         "",         "", "", DEFAULT,     -16,                            DEFAULT },
    { doom2, 1, 14,  366, 0,         "",         "", "",      16,     -16,                            DEFAULT },
    { doom2, 1, 14,  374, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  387, 0,         "",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  396, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  398, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  401, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  402, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  403, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  405, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  406, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  407, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  413, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  414, 0,         "",         "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  415, 0,         "",         "", "",      -8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  418, 0,         "",         "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  419, 0,         "",         "", "",      32,     -16,                            DEFAULT },
    { doom2, 1, 14,  422, 0,         "",         "", "",       6,     -16,                            DEFAULT },
    { doom2, 1, 14,  424, 0,         "",         "", "",      41, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  425, 0,         "",         "", "",       1, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  426, 0,         "",         "", "",      25,      16,                            DEFAULT },
    { doom2, 1, 14,  428, 0,         "",         "", "",      17,      16,                            DEFAULT },
    { doom2, 1, 14,  429, 0,  "BSTONE2",         "", "",      25, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  430, 0,  "BSTONE2",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  433, 0,         "",         "", "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  437, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  438, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  439, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  440, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  441, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  442, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  443, 0,         "",         "", "",      55, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  444, 0,         "",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  445, 0,         "",         "", "",      56, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  446, 0,         "",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  447, 0,         "",         "", "",      56, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  448, 0,         "",         "", "",      24, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  458, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  460, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  461, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  462, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  464, 0,         "",         "", "",      16, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  465, 0,         "",         "", "",      32, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  466, 0,         "",         "", "",      48, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  467, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  468, 0,         "",         "", "",      16, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  469, 0,         "",         "", "",      32, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  470, 0,         "",         "", "",      48, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  471, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  472, 0,         "",         "", "",      16, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  473, 0,         "",         "", "",      32, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  474, 0,         "",         "", "",      48, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  475, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  481, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  488, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  491, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  493, 0,         "",         "", "",      48, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  494, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  495, 0,         "",         "", "",      16, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  496, 0,         "",         "", "",      32, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  497, 0,         "",         "", "",      48, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  498, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  499, 0,         "",         "", "",      48, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  500, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  501, 0,         "",         "", "",      16, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  502, 0,         "",         "", "",      32, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  503, 0,         "",         "", "",      48, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  504, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  516, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  519, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  521, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  531, 0,  "BSTONE1",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  559, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  577, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  578, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  580, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  582, 0,         "",         "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { doom2, 1, 14,  602, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  603, 0,         "",         "", "",      40, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  604, 0,         "",         "", "",      56,      16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  607, 1,  "BSTONE2",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  608, 1,  "BSTONE2",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  609, 1,  "BSTONE2",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  610, 0,         "",         "", "",     -40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  611, 0,         "",         "", "",      -8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  612, 0,         "",         "", "",     -32,     -16, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  613, 0,         "",         "", "",     -56, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  614, 0,         "",         "", "",      -8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  615, 0,         "",         "", "",     -24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  617, 0,         "",         "", "",     -40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  618, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  619, 0,         "",         "", "",     -48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  622, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  623, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  624, 0,         "",         "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  627, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  628, 0,  "BSTONE2",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  629, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  631, 0,  "BSTONE2",         "", "",      40, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  632, 0,         "",         "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  661, 0,         "",         "", "",      16,      16,                            DEFAULT },
    { doom2, 1, 14,  663, 0,         "",         "", "", DEFAULT,     -16,                            DEFAULT },
    { doom2, 1, 14,  664, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  665, 0,         "",         "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  666, 0,         "",         "", "",      40,      16,                            DEFAULT },
    { doom2, 1, 14,  667, 0,         "",         "", "",      24,      16,                            DEFAULT },
    { doom2, 1, 14,  668, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  671, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  672, 0,         "",         "", "", DEFAULT,     -16,                            DEFAULT },
    { doom2, 1, 14,  676, 0,         "",         "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  677, 0,         "",         "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14,  679, 0,         "",         "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  680, 0,         "",         "", "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  681, 0,         "",         "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  686, 0,         "",         "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  684, 0,         "",         "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  693, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  694, 0,         "",         "", "",      16,      24,                            DEFAULT },
    { doom2, 1, 14,  696, 0,         "",         "", "",      24,      24,                            DEFAULT },
    { doom2, 1, 14,  697, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  698, 0,         "",         "", "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  699, 0,         "",         "", "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  703, 0,         "",         "", "",      16,      24,                            DEFAULT },
    { doom2, 1, 14,  704, 0,         "",         "", "",     -16,      24,                            DEFAULT },
    { doom2, 1, 14,  705, 0,         "",         "", "", DEFAULT,      24,                            DEFAULT },
    { doom2, 1, 14,  706, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  707, 0,         "",         "", "",      32,      24,                            DEFAULT },
    { doom2, 1, 14,  708, 0,         "",         "", "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  710, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  715, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  718, 0,         "",         "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  719, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  722, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  723, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  725, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  727, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  729, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  731, 0,         "",         "", "",     -16,      24,                            DEFAULT },
    { doom2, 1, 14,  732, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  733, 0,         "",         "", "",      32,      24,                            DEFAULT },
    { doom2, 1, 14,  734, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  735, 0,         "",         "", "",     -16,      24,                            DEFAULT },
    { doom2, 1, 14,  736, 0,         "",         "", "",      32,      24,                            DEFAULT },
    { doom2, 1, 14,  737, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  738, 0,         "",         "", "",     -16,      24,                            DEFAULT },
    { doom2, 1, 14,  739, 0,         "",         "", "",      32,      24,                            DEFAULT },
    { doom2, 1, 14,  740, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  741, 0,         "",         "", "",     -16,      24,                            DEFAULT },
    { doom2, 1, 14,  742, 0,         "",         "", "",      32,      24,                            DEFAULT },
    { doom2, 1, 14,  743, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  744, 0,         "",         "", "",     -16,      24,                            DEFAULT },
    { doom2, 1, 14,  745, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  746, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  747, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  748, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  749, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  751, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  753, 0,         "",         "", "",      32,      24,                            DEFAULT },
    { doom2, 1, 14,  755, 0,         "",         "", "",     -16,      24,                            DEFAULT },
    { doom2, 1, 14,  756, 0,         "",         "", "",      40, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  757, 0,         "",         "", "",      32,      24,                            DEFAULT },
    { doom2, 1, 14,  758, 0,         "",         "", "",      32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14,  759, 0,         "",         "", "",       0, DEFAULT,                      ML_DONTPEGTOP },
    { doom2, 1, 14,  786, 1, "TANROCK5",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  787, 1, "TANROCK5",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  788, 1, "TANROCK5",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  789, 1, "TANROCK5",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  791, 1, "TANROCK5",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14,  792, 1, "TANROCK5",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1023, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1025, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1026, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1027, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1028, 0,         "",         "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1029, 0,         "",         "", "",     -32,      56,                            DEFAULT },
    { doom2, 1, 14, 1030, 0,         "",         "", "",       8,      56,                            DEFAULT },
    { doom2, 1, 14, 1031, 0,         "",         "", "",     -32,      56,                            DEFAULT },
    { doom2, 1, 14, 1032, 0,         "",         "", "",       8,      56,                            DEFAULT },
    { doom2, 1, 14, 1033, 0,         "",         "", "",     -32,      56,                            DEFAULT },
    { doom2, 1, 14, 1034, 0,         "",         "", "",       8,      56,                            DEFAULT },
    { doom2, 1, 14, 1035, 0,         "",         "", "",     -32,      56,                            DEFAULT },
    { doom2, 1, 14, 1036, 0,         "",         "", "",       8,      56,                            DEFAULT },
    { doom2, 1, 14, 1037, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1038, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1039, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1040, 0,         "",         "", "",       8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1062, 0,         "", "DOORTRAK", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1063, 0,         "", "DOORTRAK", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1079, 0,         "",         "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1080, 0,         "",         "", "",      -8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1112, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1114, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1121, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1122, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1123, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1124, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1125, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1127, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1130, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1134, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1138, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1141, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1171, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1216, 0,         "",         "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1220, 0,         "",         "", "",      56, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1221, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1223, 0,         "",         "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1225, 0,         "",         "", "",     -56,     -48,                            DEFAULT },
    { doom2, 1, 14, 1226, 0,         "",         "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1227, 0,         "",         "", "",     -32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1259, 1, "BSTONE2",          "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1281, 0,        "",          "", "",      56,       0,                            DEFAULT },
    { doom2, 1, 14, 1282, 0,        "",          "", "",      24,       0,                            DEFAULT },
    { doom2, 1, 14, 1283, 0,        "",          "", "",      32,      72,                            DEFAULT },
    { doom2, 1, 14, 1284, 0,        "",          "", "",      40,      72,                            DEFAULT },
    { doom2, 1, 14, 1287, 0,        "",          "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1289, 0,        "",          "", "",      16,      56,                            DEFAULT },
    { doom2, 1, 14, 1291, 0,        "",          "", "",      40,      56,                            DEFAULT },
    { doom2, 1, 14, 1292, 0,        "",          "", "",      16, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1293, 0,        "",          "", "", DEFAULT,       8,                            DEFAULT },
    { doom2, 1, 14, 1295, 0,        "",          "", "",      24,       8,                            DEFAULT },
    { doom2, 1, 14, 1296, 0,        "",          "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1298, 0,        "",          "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1303, 0,        "",          "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1305, 1, "BSTONE1",          "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1306, 0,        "",          "", "",      48,       0,                            DEFAULT },
    { doom2, 1, 14, 1308, 0,        "",          "", "",      32,      32,                            DEFAULT },
    { doom2, 1, 14, 1309, 0,        "",          "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1310, 0,        "",          "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1311, 0,        "",          "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1312, 0,        "",          "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1313, 0,        "",          "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1314, 0,        "",          "", "",       4, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1365, 0,        "",          "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1492, 0,        "",          "", "",      32, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1493, 0,        "",          "", "",      16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1494, 0,        "",          "", "", DEFAULT,      -8,                            DEFAULT },
    { doom2, 1, 14, 1495, 0,        "",          "", "", DEFAULT,      -8,                            DEFAULT },
    { doom2, 1, 14, 1496, 0,        "",          "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1497, 0,        "",          "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1499, 0,        "",          "", "",     -16, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1501, 0,        "",          "", "",       8, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1503, 0,        "",          "", "", DEFAULT,      16,                            DEFAULT },
    { doom2, 1, 14, 1504, 0,        "",          "", "",       8,      16,                            DEFAULT },
    { doom2, 1, 14, 1505, 0,        "",          "", "",      -8,      16,                            DEFAULT },
    { doom2, 1, 14, 1507, 0,        "",          "", "",      48, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1513, 0,        "",          "", "",      16,     -40,                            DEFAULT },
    { doom2, 1, 14, 1523, 0,        "",          "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1524, 0,        "",          "", "",     -32, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { doom2, 1, 14, 1526, 0,        "",          "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1528, 0,        "",          "", "",      24, DEFAULT,                            DEFAULT },
    { doom2, 1, 14, 1531, 0,        "",          "", "",      16,     -32,                            DEFAULT },
    { doom2, 1, 14, 1532, 0,        "",          "", "",      16,     -32,                            DEFAULT },
    { doom2, 1, 14, 1561, 0,        "",          "", "",      -8, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map14[] =
{

    { doom2, 1, 14,  16, "", "CEIL5_2" },
    { doom2, 1, 14,  71, "", "CEIL5_2" },
    { doom2, 1, 14, 122, "", "CEIL5_2" },
    { doom2, 1, 14, 123, "", "CEIL5_2" },
    { doom2, 1, 14, 128, "", "CEIL5_2" },
    { doom2, 1, 14, 155, "", "CEIL5_2" },
    { doom2, 1, 14, 180, "", "CEIL5_2" },
    { doom2, 1, 14, 195, "", "CEIL5_2" },
    { doom2, 1, 14, 203, "", "CEIL5_2" },
    { doom2, 1, 14, 207, "", "CEIL5_2" },
    { doom2, 1, 14, 223, "", "CEIL5_2" },
    { doom2, 1, 14, 260, "", "CEIL5_2" },
    { doom2, 1, 14, 297, "", "CEIL5_2" },
    { doom2, 1, 14, 326, "", "RROCK12" },
    { doom2, 1, 14, 328, "", "CEIL5_2" },
    { doom2, 1, 14, 329, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map14[] =
{
    { doom2, 1, 14,   0, 102 },
    { doom2, 1, 14,   5, 102 },
    { doom2, 1, 14,  82, 102 },
    { doom2, 1, 14,  90, 102 },
    { doom2, 1, 14,  96, 102 },
    { doom2, 1, 14, 124, 102 },
    { doom2, 1, 14, 126, 102 },
    { doom2, 1, 14, 129, 102 },
    { doom2, 1, 14, 132, 102 },
    { doom2, 1, 14, 134, 102 },
    { doom2, 1, 14, 136, 102 },
    { doom2, 1, 14, 209, 102 },
    { doom2, 1, 14, 229, 102 },
    { doom2, 1, 14, 256, 102 },
    { doom2, 1, 14, 259, 102 },
    { doom2, 1, 14, 260, 102 },
    { doom2, 1, 14, 290, 102 },
    { doom2, 1, 14, 295, 102 },
    { doom2, 1, 14, 296, 102 },
    { doom2, 1, 14, 297, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 15: INDUSTRIAL ZONE
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map15[] =
{
    { doom2, 1, 15,   94, 1,  "BRICK1", "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 15,   95, 1,  "BRICK1", "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 15,  108, 0,        "", "", "",     -24, DEFAULT, DEFAULT },
    { doom2, 1, 15,  173, 0,        "", "", "",     -14, DEFAULT, DEFAULT },
    { doom2, 1, 15,  174, 0,        "", "", "",     -14, DEFAULT, DEFAULT },
    { doom2, 1, 15,  207, 0,        "", "", "",     -14, DEFAULT, DEFAULT },
    { doom2, 1, 15,  989, 1, "BRICK10", "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1051, 0,        "", "", "",      64, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1052, 0,        "", "", "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1062, 0,        "", "", "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1063, 0,        "", "", "",      64, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1064, 0,        "", "", "",      96, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1078, 0,        "", "", "",      10, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1154, 0,        "", "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1155, 0,        "", "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1160, 0,        "", "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1161, 0,        "", "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1162, 0,        "", "", "",      -8, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1607, 0,        "", "", "",     -24, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1609, 0,        "", "", "",     -24, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1659, 0,        "", "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1660, 0,        "", "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1661, 0,        "", "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1662, 0,        "", "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1663, 0,        "", "", "",      24, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1664, 0,        "", "", "",      24, DEFAULT, DEFAULT },
    { doom2, 1, 15, 1673, 0,        "", "", "",     -24, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map15[] =
{
    { doom2, 1, 15, 195,      "", "SLIME13" },
    { doom2, 1, 15, 197, "GATE5",   "GATE5" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map15[] =
{
    { doom2, 1, 15,  34, 100 },
    { doom2, 1, 15,  36, 100 },
    { doom2, 1, 15, 103, 101 },
    { doom2, 1, 15, 114, 234 },
    { doom2, 1, 15, 118, 100 },
    { doom2, 1, 15, 221, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 16: SUBURBS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map16[] =
{
    { doom2, 1, 16,   9, 0,         "", "",         "",     -40, DEFAULT, DEFAULT },
    { doom2, 1, 16,  22, 0,         "", "",         "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 16,  24, 0,         "", "",         "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 16,  25, 0,         "", "",         "",     -12, DEFAULT, DEFAULT },
    { doom2, 1, 16,  27, 0,         "", "",         "",      -8, DEFAULT, DEFAULT },
    { doom2, 1, 16, 159, 0,         "", "",         "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 16, 162, 1, "BROWNHUG", "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 16, 171, 0,         "", "",         "",     176, DEFAULT, DEFAULT },
    { doom2, 1, 16, 172, 0,         "", "",         "",     208, DEFAULT, DEFAULT },
    { doom2, 1, 16, 173, 0,         "", "",         "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 16, 176, 0,         "", "",         "",       7, DEFAULT, DEFAULT },
    { doom2, 1, 16, 192, 0,         "", "",         "",       5, DEFAULT, DEFAULT },
    { doom2, 1, 16, 205, 0,         "", "",         "",      40, DEFAULT, DEFAULT },
    { doom2, 1, 16, 239, 0,         "", "",         "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 16, 240, 0,         "", "",         "",     -56, DEFAULT, DEFAULT },
    { doom2, 1, 16, 241, 0,         "", "",         "",     -40, DEFAULT, DEFAULT },
    { doom2, 1, 16, 303, 0, "PANCASE2", "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 16, 304, 0, "PANCASE2", "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 16, 306, 0,         "", "",         "", DEFAULT,      17, DEFAULT },
    { doom2, 1, 16, 309, 0,         "", "",         "", DEFAULT,      17, DEFAULT },
    { doom2, 1, 16, 321, 0,         "", "",         "",     -18, DEFAULT, DEFAULT },
    { doom2, 1, 16, 322, 0,         "", "",         "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 16, 323, 0,         "", "",         "", DEFAULT,      32, DEFAULT },
    { doom2, 1, 16, 324, 0,         "", "",         "", DEFAULT,      32, DEFAULT },
    { doom2, 1, 16, 325, 0,         "", "",         "", DEFAULT,      32, DEFAULT },
    { doom2, 1, 16, 327, 0,         "", "",         "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 16, 328, 1,         "", "", "FIREBLU2", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 16, 600, 0,         "", "",         "", DEFAULT,       6, DEFAULT },
    { doom2, 1, 16, 605, 0,         "", "",         "", DEFAULT,       6, DEFAULT },
    { doom2, 1, 16, 610, 0,         "", "",         "",      43, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const flow_t flow_doom2_map16[] =
{
    { doom2, 1, 16, 16, 101 },
    { doom2, 1, 16, 67, 206 },
    { doom2, 1, 16, 68, 206 },
    { doom2, 1, 16, 78, 206 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 17: TENEMENTS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map17[] =
{
    { doom2, 1, 17, 182, 1,       "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 17, 250, 0,       "",         "", "",      56, DEFAULT, DEFAULT },
    { doom2, 1, 17, 310, 0,       "",         "", "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 17, 316, 1,       "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 17, 379, 1, "METAL2",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 17, 492, 0,       "",         "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 17, 493, 0,       "",         "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 17, 515, 0,       "",         "", "",     -45, DEFAULT, DEFAULT },
    { doom2, 1, 17, 516, 0,       "",         "", "",     -45, DEFAULT, DEFAULT },
    { doom2, 1, 17, 519, 0,       "",         "", "",      -7, DEFAULT, DEFAULT },
    { doom2, 1, 17, 520, 0,       "",         "", "",      -7, DEFAULT, DEFAULT },
    { doom2, 1, 17, 521, 0,       "",         "", "",     -26, DEFAULT, DEFAULT },
    { doom2, 1, 17, 522, 0,       "",         "", "",     -26, DEFAULT, DEFAULT },
    { doom2, 1, 17, 528, 0,       "",         "", "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 17, 529, 0,       "",         "", "",      59, DEFAULT, DEFAULT },
    { doom2, 1, 17, 530, 0,       "",         "", "",      59, DEFAULT, DEFAULT },
    { doom2, 1, 17, 531, 0,       "",         "", "",      22, DEFAULT, DEFAULT },
    { doom2, 1, 17, 533, 0,       "",         "", "",      24, DEFAULT, DEFAULT },
    { doom2, 1, 17, 534, 0,       "",         "", "",      17, DEFAULT, DEFAULT },
    { doom2, 1, 17, 535, 0,       "",         "", "",       1, DEFAULT, DEFAULT },
    { doom2, 1, 17, 536, 0,       "",         "", "",      58, DEFAULT, DEFAULT },
    { doom2, 1, 17, 538, 0,       "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 17, 539, 0,       "",         "", "",      42, DEFAULT, DEFAULT },
    { doom2, 1, 17, 540, 0,       "",         "", "",      58, DEFAULT, DEFAULT },
    { doom2, 1, 17, 541, 0,       "",         "", "",      28, DEFAULT, DEFAULT },
    { doom2, 1, 17, 661, 0,       "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 17, 669, 0,       "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 17, 725, 0,       "",         "", "",     -24, DEFAULT, DEFAULT },
    { doom2, 1, 17, 797, 0,       "",         "", "",      -5, DEFAULT, DEFAULT },
    { doom2, 1, 17, 798, 0,       "",         "", "",     -46, DEFAULT, DEFAULT },
    { doom2, 1, 17, 799, 0,       "",         "", "",     -14, DEFAULT, DEFAULT },
    { doom2, 1, 17, 800, 0,       "",         "", "",     -55, DEFAULT, DEFAULT },
    { doom2, 1, 17, 801, 0,       "",         "", "",     -23, DEFAULT, DEFAULT },
    { doom2, 1, 17, 803, 0,       "",         "", "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 17, 804, 0,       "",         "", "",     -37, DEFAULT, DEFAULT },
    { doom2, 1, 17, 816, 0,       "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 17, 864, 0,       "",         "", "",      56, DEFAULT, DEFAULT },
    { doom2, 1, 17, 865, 0,       "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 17, 866, 0,       "",         "", "",      48, DEFAULT, DEFAULT },
    { doom2, 1, 17, 895, 0,       "",         "", "",      19, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map17[] =
{
    { doom2, 1, 17,  47, "",  "CEIL5_2" },
    { doom2, 1, 17,  59, "", "FLOOR6_2" },
    { doom2, 1, 17,  70, "",  "CEIL5_1" },
    { doom2, 1, 17, 112, "",  "CEIL5_2" },
    { doom2, 1, 17, 147, "",  "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map17[] =
{
    { doom2, 1, 17,   0, 101 },
    { doom2, 1, 17,  25, 101 },
    { doom2, 1, 17,  27, 101 },
    { doom2, 1, 17,  37, 101 },
    { doom2, 1, 17,  38, 101 },
    { doom2, 1, 17,  39, 101 },
    { doom2, 1, 17,  40, 101 },
    { doom2, 1, 17,  84, 101 },
    { doom2, 1, 17,  91, 101 },
    { doom2, 1, 17,  98, 101 },
    { doom2, 1, 17, 136, 101 },
    { doom2, 1, 17, 137, 101 },
    { doom2, 1, 17, 138, 101 },
    { doom2, 1, 17, 163, 101 },
    { doom2, 1, 17, 173, 101 },
    { doom2, 1, 17, 191, 100 },
    { doom2, 1, 17, 171, 221 },
    { doom2, 1, 17, 176, 226 },
    { doom2, 1, 17, 204, 221 },
    { doom2, 1, 17, 206, 221 },
    { doom2, 1, 17, 229, 221 },
    { doom2, 1, 17, 241, 221 },
    { doom2, 1, 17, 255, 216 },
    { doom2, 1, 17, 256, 211 },
    { doom2, 1, 17, 257, 221 },
    { doom2, 1, 17, 218, 101 },
    { doom2, 1, 17, 222, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 18: THE COURTYARD
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map18[] =
{
    { doom2, 1, 18,   5, 0,         "", "SUPPORT3", "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 18,   8, 0,         "",         "", "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 18,   9, 0,         "",         "", "", DEFAULT,      80,          DEFAULT },
    { doom2, 1, 18,  17, 0,         "",         "", "", DEFAULT,     -64,          DEFAULT },
    { doom2, 1, 18,  18, 0,         "",         "", "", DEFAULT,     -64,          DEFAULT },
    { doom2, 1, 18,  22, 0,         "",         "", "",     -96, DEFAULT,          DEFAULT },
    { doom2, 1, 18,  26, 0,         "",         "", "", DEFAULT,      80,          DEFAULT },
    { doom2, 1, 18,  42, 0,         "",         "", "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 18,  69, 0,         "",         "", "",     128, DEFAULT,          DEFAULT },
    { doom2, 1, 18,  70, 0,         "",         "", "",       0,       0,          DEFAULT },
    { doom2, 1, 18,  71, 0,         "",         "", "",      80,       0,          DEFAULT },
    { doom2, 1, 18,  72, 0,         "",         "", "",     112,       0,          DEFAULT },
    { doom2, 1, 18,  73, 0,         "",         "", "",     168,       0,          DEFAULT },
    { doom2, 1, 18,  74, 0,         "",         "", "",     232,       0,          DEFAULT },
    { doom2, 1, 18,  75, 0,         "",         "", "",      32,       0,          DEFAULT },
    { doom2, 1, 18,  76, 0,         "",         "", "",      64,       0,          DEFAULT },
    { doom2, 1, 18,  85, 0,         "",         "", "",      96,      80,          DEFAULT },
    { doom2, 1, 18,  86, 0,         "",         "", "",      64,      80,          DEFAULT },
    { doom2, 1, 18,  91, 0,         "",         "", "",     -32,       0,          DEFAULT },
    { doom2, 1, 18, 103, 0,         "",         "", "",     128,      80,          DEFAULT },
    { doom2, 1, 18, 107, 0,         "",         "", "", DEFAULT,      80,          DEFAULT },
    { doom2, 1, 18, 167, 0,         "",         "", "",      45, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 224, 0,         "",         "", "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 230, 0,         "",         "", "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 235, 0,         "",         "", "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 240, 0,         "",         "", "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 314, 0,         "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 18, 326, 0,         "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 18, 352, 0,         "",         "", "",      32, DEFAULT,    ML_DONTPEGTOP },
    { doom2, 1, 18, 373, 0,         "",         "", "", DEFAULT,      64,          DEFAULT },
    { doom2, 1, 18, 378, 0,         "",         "", "", DEFAULT,      64,          DEFAULT },
    { doom2, 1, 18, 385, 0,         "",         "", "", DEFAULT,     -16,          DEFAULT },
    { doom2, 1, 18, 386, 0,         "",         "", "",      64, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 451, 0,         "", "DOORSTOP", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 459, 0,         "", "DOORSTOP", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 481, 0,         "",         "", "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 483, 0,         "",         "", "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 485, 0,         "",         "", "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 537, 0,         "",         "", "", DEFAULT,      16,          DEFAULT },
    { doom2, 1, 18, 538, 0,         "",         "", "", DEFAULT,      16,          DEFAULT },
    { doom2, 1, 18, 549, 0,         "",         "", "",     224,      80,          DEFAULT },
    { doom2, 1, 18, 558, 0,         "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 18, 569, 0,         "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 18, 574, 0, "GRAYVINE",         "", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 648, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 649, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 650, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 651, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 652, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 653, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 654, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 655, 0,         "",         "", "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 18, 718, 0,         "", "DOORTRAK", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 724, 0,         "", "DOORTRAK", "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 725, 0,         "",         "", "",      87, DEFAULT,          DEFAULT },
    { doom2, 1, 18, 742, 0,         "",         "", "",     192,      80,          DEFAULT },
    { doom2, 1, 18, 743, 0,         "",         "", "",       0,      80,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map18[] =
{
    { doom2, 1, 18,  4, "FLOOR5_1", "" },
    { doom2, 1, 18, 10, "FLOOR5_1", "" },
    { doom2, 1, 18, 32, "FLOOR5_1", "" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map18[] =
{
    { doom2, 1, 18, 45, 231 },
    { doom2, 1, 18, 49, 231 },
    { doom2, 1, 18, 57, 231 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 19: THE CITADEL
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map19[] =
{
    { doom2, 1, 19,    1, 0,         "",         "",         "", DEFAULT,       8,          DEFAULT },
    { doom2, 1, 19,    3, 0,         "",         "",         "",       0,       8,          DEFAULT },
    { doom2, 1, 19,  286, 1,         "",         "",   "STONE2", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  287, 1,         "",         "",   "STONE2", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  288, 1,         "",         "",   "STONE2", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  336, 0,         "",         "",         "", DEFAULT,       8,          DEFAULT },
    { doom2, 1, 19,  355, 1,   "STONE2",         "",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  455, 1,         "",         "", "FIRELAVA", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  495, 0,         "",         "",         "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 19,  510, 0,         "",         "",         "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 19,  529, 1,         "", "MIDGRATE",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  537, 0,         "", "DOORTRAK",         "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 19,  556, 0,         "", "DOORTRAK",         "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 19,  558, 0,         "",         "",         "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 19,  559, 0,         "",         "",         "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 19,  560, 0,         "",         "",         "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 19,  577, 1,         "", "BRNSMAL2",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  578, 1,         "", "BRNSMAL1",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  618, 1,         "", "BRNSMAL2",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  662, 0,         "",         "",         "", DEFAULT,      64,          DEFAULT },
    { doom2, 1, 19,  706, 0,         "",         "",         "",      56,      16,          DEFAULT },
    { doom2, 1, 19,  736, 0, "SLADWALL",         "",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  783, 0,         "",         "",         "",     -16,       0, ML_DONTPEGBOTTOM },
    { doom2, 1, 19,  788, 0,         "",         "",         "",     112,       0, ML_DONTPEGBOTTOM },
    { doom2, 1, 19,  845, 0,         "",         "",         "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  846, 0,         "",         "",         "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  847, 0,         "",         "",         "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  929, 0,         "",         "",         "",     120, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  930, 0,         "",         "",         "",     112, DEFAULT,          DEFAULT },
    { doom2, 1, 19,  931, 0,         "",         "",         "",     232, DEFAULT,          DEFAULT },
    { doom2, 1, 19, 1004, 0,         "",         "",         "",     197,      24,          DEFAULT },
    { doom2, 1, 19, 1028, 0,         "",         "",         "", DEFAULT,     -24, ML_DONTPEGBOTTOM },
    { doom2, 1, 19, 1029, 0,         "",         "",         "", DEFAULT,     -24, ML_DONTPEGBOTTOM },
    { doom2, 1, 19, 1059, 0,         "",         "",         "",      27,      72,          DEFAULT },
    { doom2, 1, 19, 1106, 0,         "",         "",         "",      61,      24,          DEFAULT },
    { doom2, 1, 19, 1107, 0,         "",         "",         "",     -11,      24,          DEFAULT },
    { doom2, 1, 19, 1111, 0,         "",         "",         "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 19, 1150, 1,         "",         "",         "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 19, 1152, 0,         "",         "",         "", DEFAULT,       8,          DEFAULT },
    { doom2, 1, 19, 1181, 1,  "MARBLE1",         "",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 19, 1229, 0,         "",         "",         "",     -32,     -60,          DEFAULT },
    { doom2, 1, 19, 1230, 0,         "",         "",         "",     -57, DEFAULT,          DEFAULT },
    { doom2, 1, 19, 1231, 0,         "",         "",         "",     -53, DEFAULT,          DEFAULT },
    { doom2, 1, 19, 1232, 0,         "",         "",         "",     -21, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map19[] =
{
    { doom2, 1, 19, 187, "FLAT5_4",       "" },
    { doom2, 1, 19, 193, "FLAT5_4",       "" },
    { doom2, 1, 19, 229,        "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map19[] =
{
    { doom2, 1, 19,   0, 221 },
    { doom2, 1, 19,  16, 100 },
    { doom2, 1, 19,  68, 102 },
    { doom2, 1, 19, 216, 100 },
    FLOW_END
};
    
// -----------------------------------------------------------------------------
// LEVEL 20: GOTCHA!
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map20[] =
{
    { doom2, 1, 20,   73, 1, "", "", "SPACEW4", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 20,   74, 1, "", "", "SPACEW4", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 20,   75, 1, "", "", "SPACEW4", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 20,   76, 1, "", "", "SPACEW4", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  133, 0, "", "",        "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  136, 0, "", "",        "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  138, 0, "", "",        "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  142, 0, "", "",        "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 20,  241, 0, "", "",        "",      -1, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  242, 0, "", "",        "",      31, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  244, 0, "", "",        "",     -39, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  245, 0, "", "",        "",     -23, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  247, 0, "", "",        "",     -39, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  248, 0, "", "",        "",     -23, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  250, 0, "", "",        "",     -47, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  251, 0, "", "",        "",     -31, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  264, 0, "", "",        "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 20,  265, 0, "", "",        "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 20,  293, 0, "", "",        "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  294, 0, "", "",        "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  294, 0, "", "",        "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  299, 0, "", "",        "",      32, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  304, 0, "", "",        "", DEFAULT,      -8,          DEFAULT },
    { doom2, 1, 20,  363, 0, "", "",        "",      29, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  364, 0, "", "",        "",      37, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  365, 0, "", "",        "",      60, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  366, 0, "", "",        "",      37, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  367, 0, "", "",        "",       5, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  392, 0, "", "",        "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  393, 0, "", "",        "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  394, 0, "", "",        "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  395, 0, "", "",        "",      60, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  396, 0, "", "",        "",      50, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  397, 0, "", "",        "",      58, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  398, 0, "", "",        "",       2, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  399, 0, "", "",        "",      56, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  400, 0, "", "",        "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  401, 0, "", "",        "",      58,      24,          DEFAULT },
    { doom2, 1, 20,  402, 0, "", "",        "",      42,      24,          DEFAULT },
    { doom2, 1, 20,  403, 0, "", "",        "",      21, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  404, 0, "", "",        "",      13, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  405, 0, "", "",        "",      21, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  406, 0, "", "",        "",      58, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  412, 0, "", "",        "",      25, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  413, 0, "", "",        "",      45, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  414, 0, "", "",        "",      19, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  415, 0, "", "",        "",      39, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  416, 0, "", "",        "",      15, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  417, 0, "", "",        "",      43, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  418, 0, "", "",        "",      51, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  419, 0, "", "",        "",      59, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  420, 0, "", "",        "",       5, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  421, 0, "", "",        "",       7, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  422, 0, "", "",        "",      51, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  423, 0, "", "",        "",      12, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  425, 0, "", "",        "",      71, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  426, 0, "", "",        "",      15, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  427, 0, "", "",        "",      15, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  428, 0, "", "",        "",      15, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  429, 0, "", "",        "",     143, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  431, 0, "", "",        "",      79, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  438, 0, "", "",        "",      16, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  452, 0, "", "",        "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  453, 1, "", "",   "ROCK1", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  463, 0, "", "",        "", DEFAULT,     102,          DEFAULT },
    { doom2, 1, 20,  522, 0, "", "",        "",     -16, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  530, 0, "", "",        "",      -1, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  531, 0, "", "",        "",       8, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  556, 0, "", "",        "",     -16, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  567, 0, "", "",        "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  570, 0, "", "",        "",      -8, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  590, 0, "", "",        "",       8,     -33,          DEFAULT },
    { doom2, 1, 20,  613, 0, "", "",        "",       8,     -33,          DEFAULT },
    { doom2, 1, 20,  614, 0, "", "",        "",       8,     -33,          DEFAULT },
    { doom2, 1, 20,  632, 0, "", "",        "",      -8, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  635, 0, "", "",        "",      56, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  638, 0, "", "",        "",     104, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  640, 0, "", "",        "",     118, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  641, 0, "", "",        "",      64, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  648, 0, "", "",        "",     108, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  722, 0, "", "",        "",     -32, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  725, 0, "", "",        "", DEFAULT,      16,          DEFAULT },
    { doom2, 1, 20,  726, 0, "", "",        "",       8,      16,          DEFAULT },
    { doom2, 1, 20,  727, 0, "", "",        "", DEFAULT,      16,          DEFAULT },
    { doom2, 1, 20,  732, 0, "", "",        "",      -4, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  733, 0, "", "",        "",      -4, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  762, 0, "", "",        "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  763, 0, "", "",        "",     -58,      16,          DEFAULT },
    { doom2, 1, 20,  764, 0, "", "",        "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 20,  998, 0, "", "",        "",       4, DEFAULT,          DEFAULT },
    { doom2, 1, 20, 1013, 0, "", "",        "",       4, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map20[] =
{
    { doom2, 1, 20,  17,        "", "RROCK09" },
    { doom2, 1, 20,  26, "RROCK13",        "" },
    { doom2, 1, 20,  62,        "", "CEIL5_2" },
    { doom2, 1, 20,  80,        "", "CEIL5_2" },
    { doom2, 1, 20,  83,        "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map20[] =
{
    { doom2, 1, 20,  0, 102 },
    { doom2, 1, 20,  2, 101 },
    { doom2, 1, 20, 19, 101 },
    { doom2, 1, 20, 20, 102 },
    { doom2, 1, 20, 26, 102 },
    { doom2, 1, 20, 49, 102 },
    { doom2, 1, 20, 53, 102 },
    { doom2, 1, 20, 84, 102 },
    { doom2, 1, 20, 85, 102 },
    { doom2, 1, 20, 97, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 21: NIRVANA
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map21[] =
{
    { doom2, 1, 21,  50, 0,        "", "", "",       0, DEFAULT, DEFAULT },
    { doom2, 1, 21,  57, 0,        "", "", "",      56, DEFAULT, DEFAULT },
    { doom2, 1, 21,  58, 0,        "", "", "",      31, DEFAULT, DEFAULT },
    { doom2, 1, 21,  59, 0,        "", "", "",      17, DEFAULT, DEFAULT },
    { doom2, 1, 21,  60, 0,        "", "", "",      31, DEFAULT, DEFAULT },
    { doom2, 1, 21,  61, 0,        "", "", "",      28, DEFAULT, DEFAULT },
    { doom2, 1, 21,  62, 0,        "", "", "",      33, DEFAULT, DEFAULT },
    { doom2, 1, 21,  65, 0,        "", "", "",       1, DEFAULT, DEFAULT },
    { doom2, 1, 21,  66, 0,        "", "", "",      39, DEFAULT, DEFAULT },
    { doom2, 1, 21,  67, 0,        "", "", "",      47, DEFAULT, DEFAULT },
    { doom2, 1, 21,  68, 0,        "", "", "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 21, 174, 0,        "", "", "", DEFAULT,     -40, DEFAULT },
    { doom2, 1, 21, 175, 0,        "", "", "", DEFAULT,     -40, DEFAULT },
    { doom2, 1, 21, 203, 0, "DOORRED", "", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map21[] =
{
    { doom2, 1, 21, 17, "SWATER2",        "" },
    { doom2, 1, 21, 22, "SWATER2",        "" },
    { doom2, 1, 21, 23, "SWATER2", "RROCK09" },
    { doom2, 1, 21, 24,        "", "RROCK11" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map21[] =
{
    { doom2, 1, 21, 13, 102 },
    { doom2, 1, 21, 17, 100 },
    { doom2, 1, 21, 22, 100 },
    { doom2, 1, 21, 23, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 22: THE CATACOMBS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map22[] =
{
    { doom2, 1, 22,  60, 0,      "", "",        "",      32, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  64, 0,      "", "",        "",      15, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  65, 0,      "", "",        "",      15, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  68, 0,      "", "",        "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  69, 0,      "", "",        "",      56, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  70, 0,      "", "",        "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  71, 0,      "", "",        "",      56, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  72, 0,      "", "",        "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  73, 0,      "", "",        "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  75, 0,      "", "",        "",      24, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  76, 0,      "", "",        "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  77, 0,      "", "",        "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  78, 0,      "", "",        "",      40, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  79, 0,      "", "",        "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  80, 0,      "", "",        "",      40, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  81, 0,      "", "",        "",      24, DEFAULT,       DEFAULT },
    { doom2, 1, 22,  89, 0,      "", "",        "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 158, 0,      "", "", "BROWN96", DEFAULT, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 162, 0,      "", "",        "",      32, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 192, 0,      "", "",        "", DEFAULT,      -7,       DEFAULT },
    { doom2, 1, 22, 197, 0,      "", "",        "", DEFAULT,      -7,       DEFAULT },
    { doom2, 1, 22, 226, 0,      "", "",        "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 227, 0,      "", "",        "",      40, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 228, 0,      "", "",        "",      56, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 229, 0,      "", "",        "",      16, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 230, 1,      "", "",        "", DEFAULT,       1, ML_DONTPEGTOP },
    { doom2, 1, 22, 230, 1,      "", "",        "", DEFAULT,       1, ML_DONTPEGTOP },
    { doom2, 1, 22, 231, 1,      "", "",        "",      16,       1, ML_DONTPEGTOP },
    { doom2, 1, 22, 232, 1,      "", "",        "",      40,       1, ML_DONTPEGTOP },
    { doom2, 1, 22, 233, 1,      "", "",        "",      56,       1, ML_DONTPEGTOP },
    { doom2, 1, 22, 234, 1,      "", "",        "",      80,       1, ML_DONTPEGTOP },
    { doom2, 1, 22, 312, 0,      "", "",        "",     -48, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 332, 0,      "", "",        "",     -16,     -32,       DEFAULT },
    { doom2, 1, 22, 334, 0,      "", "",        "", DEFAULT,     -32,       DEFAULT },
    { doom2, 1, 22, 407, 0,      "", "",        "",      24, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 409, 0,      "", "",        "",     -16, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 412, 0,      "", "",        "",      40, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 415, 0,      "", "",        "",     -40, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 442, 1, "METAL", "",        "", DEFAULT, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 443, 1, "METAL", "",        "", DEFAULT, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 464, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 465, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 467, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 468, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 480, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 489, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 505, 0,      "", "",        "",     -39, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 507, 0,      "", "",        "",     -58, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 508, 0,      "", "",        "",      23, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 509, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 510, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 511, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 512, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 513, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 514, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 515, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 516, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 517, 0,      "", "",        "",      23, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 518, 0,      "", "",        "",       6, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 519, 0,      "", "",        "",      25, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 521, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 522, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 523, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 524, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 525, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 526, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 527, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 528, 0,      "", "",        "",       4, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 533, 0,      "", "",        "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 535, 0,      "", "",        "",     -32, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 537, 0,      "", "",        "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 551, 0,      "", "",        "",      56, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 552, 0,      "", "",        "",      56, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 553, 0,      "", "",        "",      56, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 554, 0,      "", "",        "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 555, 0,      "", "",        "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 556, 0,      "", "",        "",       8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 597, 0,      "", "",        "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 566, 0,      "", "",        "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 568, 0,      "", "",        "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 569, 0,      "", "",        "",      48, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 583, 0,      "", "",        "",      -8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 586, 0,      "", "",        "",      32, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 588, 0,      "", "",        "",     -40, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 593, 0,      "", "",        "",      40, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 596, 0,      "", "",        "",      -8, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 617, 0,      "", "",        "",      40, DEFAULT,       DEFAULT },
    { doom2, 1, 22, 619, 0,      "", "",        "",     -16, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map22[] =
{
    { doom2, 1,  22,   8,        "",  "FLAT5_5" },
    { doom2, 1,  22,  17,        "",  "FLAT5_5" },
    { doom2, 1,  22,  18,        "",  "FLAT5_4" },
    { doom2, 1,  22,  23,        "", "FLOOR7_2" },
    { doom2, 1,  22,  27,        "",  "FLAT5_4" },
    { doom2, 1,  22,  28,        "",  "FLAT5_4" },
    { doom2, 1,  22,  30, "CEIL5_1",  "CEIL5_1" },
    { doom2, 1,  22,  31,        "",  "FLAT5_5" },
    { doom2, 1,  22,  43,        "",  "FLAT5_4" },
    { doom2, 1,  22,  45,        "",  "FLAT5_4" },
    { doom2, 1,  22,  51,        "", "FLOOR7_2" },
    { doom2, 1,  22,  56,        "",   "FLAT20" },
    { doom2, 1,  22,  58,        "",  "CEIL5_2" },
    { doom2, 1,  22,  81,        "",  "CEIL5_1" },
    { doom2, 1,  22, 114, "FLAT5_4",         "" },
    { doom2, 1,  22, 115, "FLAT5_4",         "" },
    { doom2, 1,  22, 116, "FLAT5_4",         "" },
    { doom2, 1,  22, 117, "FLAT5_4",         "" },
    { doom2, 1,  22, 118, "FLAT5_4",         "" },
    { doom2, 1,  22, 119,        "",  "CEIL5_2" },
    { doom2, 1,  22, 120,        "",  "RROCK03" },
    { doom2, 1,  22, 121, "FLAT5_4",         "" },
    { doom2, 1,  22, 124, "FLAT5_4",         "" },
    { doom2, 1,  22, 125, "CEIL5_2",         "" },
    { doom2, 1,  22, 126, "FLAT5_4",         "" },
    { doom2, 1,  22, 128, "FLAT5_4",         "" },
    { doom2, 1,  22, 130, "FLAT5_4",         "" },
    { doom2, 1,  22, 133, "FLAT5_4",         "" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map22[] =
{
    { doom2, 1, 22,   9, 101 },
    { doom2, 1, 22,  10, 101 },
    { doom2, 1, 22,  11, 101 },
    { doom2, 1, 22,  32, 101 },
    { doom2, 1, 22,  33, 101 },
    { doom2, 1, 22,  34, 101 },
    { doom2, 1, 22,  35, 101 },
    { doom2, 1, 22,  36, 101 },
    { doom2, 1, 22,  37, 101 },
    { doom2, 1, 22,  39, 101 },
    { doom2, 1, 22,  70, 101 },
    { doom2, 1, 22,  73, 100 },
    { doom2, 1, 22,  82, 101 },
    { doom2, 1, 22,  90, 101 },
    { doom2, 1, 22, 112, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 23: BARRELS O' FUN
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map23[] =
{
    { doom2, 1, 23,   8, 1, "",      "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 23, 110, 0, "", "METAL", "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 23, 112, 0, "", "METAL", "",       0, DEFAULT,          DEFAULT },
    { doom2, 1, 23, 382, 0, "",      "", "",      11,       4,          DEFAULT },
    { doom2, 1, 23, 383, 0, "",      "", "",      11,       4,          DEFAULT },
    { doom2, 1, 23, 384, 0, "",      "", "",      11,       4,          DEFAULT },
    { doom2, 1, 23, 385, 0, "",      "", "", DEFAULT,       4,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map23[] =
{
    { doom2, 1, 23, 50, "CEIL5_1", "CEIL5_1" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map23[] =
{
    { doom2, 1, 23, 0, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 24: THE CHASM
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map24[] =
{
    { doom2, 1,  24,  284, 0, "", "",        "",     -16, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  285, 0, "", "",        "",     -65, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  285, 0, "", "",        "",     -59, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  296, 0, "", "",        "",     -20, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  297, 0, "", "",        "",     -32, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  325, 0, "", "",        "",     -22, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  326, 0, "", "",        "",     -43, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  327, 0, "", "",        "",     -65, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  328, 0, "", "",        "",     -46, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  286, 0, "", "",        "",     -59, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  287, 0, "", "",        "",     -58, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  288, 0, "", "",        "",     -48, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  289, 0, "", "",        "",     -54, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  290, 0, "", "",        "",      61, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  292, 0, "", "",        "",     -11, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  293, 0, "", "",        "",     -50, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  294, 0, "", "",        "",     -42, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  295, 0, "", "",        "",     -24, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  324, 0, "", "",        "",     -12, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  330, 0, "", "",        "",     -13, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  331, 0, "", "",        "",     -37, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  687, 1, "", "", "SILVER2", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  688, 1, "", "", "SILVER2", DEFAULT, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  706, 0, "", "",        "",     -24, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  711, 0, "", "",        "", DEFAULT,     -72,     DEFAULT },
    { doom2, 1,  24,  712, 0, "", "",        "", DEFAULT,     -72,     DEFAULT },
    { doom2, 1,  24,  795, 0, "", "",        "", DEFAULT,     -72,     DEFAULT },
    { doom2, 1,  24,  813, 0, "", "",        "", DEFAULT,       8,     DEFAULT },
    { doom2, 1,  24,  814, 0, "", "",        "", DEFAULT,       8,     DEFAULT },
    { doom2, 1,  24,  815, 0, "", "",        "", DEFAULT,       8,     DEFAULT },
    { doom2, 1,  24,  816, 0, "", "",        "", DEFAULT,       8,     DEFAULT },
    { doom2, 1,  24,  817, 0, "", "",        "", DEFAULT,       8,     DEFAULT },
    { doom2, 1,  24,  818, 0, "", "",        "", DEFAULT,       8,     DEFAULT },
    { doom2, 1,  24,  929, 0, "", "",        "",    -200, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  930, 0, "", "",        "",     -72, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  931, 0, "", "",        "",     171, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  932, 0, "", "",        "",      83, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  983, 0, "", "",        "",      25, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  983, 1, "", "",        "",       5, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  985, 0, "", "",        "",      25, DEFAULT,     DEFAULT },
    { doom2, 1,  24,  985, 1, "", "",        "",     -20, DEFAULT,     DEFAULT },
    { doom2, 1,  24, 1038, 0, "", "",        "", DEFAULT, DEFAULT, ML_DONTDRAW },
    LINEFIX_END
};

static const flow_t flow_doom2_map24[] =
{
    { doom2, 1, 24,   0, 102 },
    { doom2, 1, 24,   2, 102 },
    { doom2, 1, 24,   9, 102 },
    { doom2, 1, 24,  13, 102 },
    { doom2, 1, 24,  25, 100 },
    { doom2, 1, 24,  32, 102 },
    { doom2, 1, 24,  42, 102 },
    { doom2, 1, 24,  43, 102 },
    { doom2, 1, 24,  54, 100 },
    { doom2, 1, 24,  55, 101 },
    { doom2, 1, 24,  63, 100 },
    { doom2, 1, 24, 126, 101 },
    { doom2, 1, 24, 127, 100 },
    FLOW_END
};

static const fall_t fall_doom2_map24[] =
{
    { doom2, 1, 24,  58, 102 },
    { doom2, 1, 24,  59, 102 },
    { doom2, 1, 24,  60, 102 },
    { doom2, 1, 24,  61, 102 },
    { doom2, 1, 24,  62, 102 },
    { doom2, 1, 24,  63, 102 },
    { doom2, 1, 24,  64, 102 },
    { doom2, 1, 24,  65, 102 },
    { doom2, 1, 24, 341, 102 },
    { doom2, 1, 24, 342, 102 },
    { doom2, 1, 24, 343, 102 },
    { doom2, 1, 24, 344, 102 },
    { doom2, 1, 24, 345, 102 },
    { doom2, 1, 24, 346, 102 },
    { doom2, 1, 24, 347, 102 },
    { doom2, 1, 24, 348, 102 },
    { doom2, 1, 24, 349, 102 },
    { doom2, 1, 24, 357, 101 },
    { doom2, 1, 24, 358, 101 },
    { doom2, 1, 24, 359, 101 },
    { doom2, 1, 24, 360, 101 },
    { doom2, 1, 24, 361, 101 },
    { doom2, 1, 24, 362, 101 },
    { doom2, 1, 24, 363, 101 },
    { doom2, 1, 24, 364, 101 },
    { doom2, 1, 24, 365, 101 },
    { doom2, 1, 24, 366, 101 },
    { doom2, 1, 24, 367, 101 },
    { doom2, 1, 24, 368, 101 },
    { doom2, 1, 24, 369, 101 },
    { doom2, 1, 24, 370, 101 },
    { doom2, 1, 24, 371, 101 },
    { doom2, 1, 24, 372, 101 },
    { doom2, 1, 24, 373, 101 },
    { doom2, 1, 24, 374, 101 },
    { doom2, 1, 24, 375, 101 },
    { doom2, 1, 24, 376, 101 },
    { doom2, 1, 24, 377, 101 },
    { doom2, 1, 24, 378, 101 },
    { doom2, 1, 24, 379, 101 },
    { doom2, 1, 24, 380, 101 },
    { doom2, 1, 24, 381, 101 },
    { doom2, 1, 24, 382, 101 },
    { doom2, 1, 24, 383, 101 },
    { doom2, 1, 24, 393, 101 },
    { doom2, 1, 24, 394, 101 },
    { doom2, 1, 24, 395, 101 },
    { doom2, 1, 24, 396, 101 },
    { doom2, 1, 24, 397, 101 },
    { doom2, 1, 24, 398, 101 },
    { doom2, 1, 24, 399, 101 },
    { doom2, 1, 24, 400, 101 },
    { doom2, 1, 24, 401, 101 },
    { doom2, 1, 24, 402, 101 },
    { doom2, 1, 24, 403, 101 },
    { doom2, 1, 24, 404, 101 },
    { doom2, 1, 24, 405, 101 },
    { doom2, 1, 24, 406, 101 },
    { doom2, 1, 24, 407, 101 },
    { doom2, 1, 24, 408, 101 },
    { doom2, 1, 24, 409, 101 },
    { doom2, 1, 24, 410, 101 },
    { doom2, 1, 24, 411, 101 },
    { doom2, 1, 24, 412, 101 },
    { doom2, 1, 24, 413, 101 },
    { doom2, 1, 24, 414, 101 },
    { doom2, 1, 24, 415, 101 },
    { doom2, 1, 24, 419, 101 },
    { doom2, 1, 24, 418, 101 },
    { doom2, 1, 24, 416, 101 },
    { doom2, 1, 24, 417, 101 },
    { doom2, 1, 24, 418, 101 },
    { doom2, 1, 24, 419, 101 },
    { doom2, 1, 24, 420, 101 },
    { doom2, 1, 24, 421, 101 },
    { doom2, 1, 24, 422, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 25: BLOODFALLS
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map25[] =
{
    { doom2, 1, 25, 158, 0,        "",         "", "",      -3, DEFAULT, DEFAULT },
    { doom2, 1, 25, 159, 0,        "",         "", "",      -3, DEFAULT, DEFAULT },
    { doom2, 1, 25, 161, 0,        "",         "", "",      -3, DEFAULT, DEFAULT },
    { doom2, 1, 25, 162, 0,        "",         "", "",      -3, DEFAULT, DEFAULT },
    { doom2, 1, 25, 163, 0,        "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 25, 164, 0,        "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 25, 165, 0,        "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 25, 166, 0,        "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 25, 173, 0,        "",         "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 25, 174, 0,        "",         "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 25, 175, 0,        "",         "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 25, 176, 0,        "",         "", "",       4, DEFAULT, DEFAULT },
    { doom2, 1, 25, 348, 1,        "", "MIDSPACE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 25, 349, 1,        "", "MIDSPACE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 25, 436, 1, "MARBLE1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 25, 618, 0,        "",         "", "",       8, DEFAULT, DEFAULT },
    { doom2, 1, 25, 689, 0,        "",         "", "",      48, DEFAULT, DEFAULT },
    { doom2, 1, 25, 690, 0,        "",         "", "",      48, DEFAULT, DEFAULT },
    { doom2, 1, 25, 693, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 694, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 695, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 696, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 697, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 698, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 699, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 701, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 702, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 703, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 705, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 707, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 708, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 710, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 711, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 712, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 713, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 714, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 717, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 718, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 719, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 720, 0,        "",         "", "",      16, DEFAULT, DEFAULT },
    { doom2, 1, 25, 721, 0,        "",         "", "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 25, 722, 0,        "",         "", "",     -16, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map25[] =
{
    { doom2, 1, 25,  94,        "", "CEIL5_2" },
    { doom2, 1, 25, 103,        "", "FLAT5_4" },
    { doom2, 1, 25, 125, "SWATER2",        "" },
    { doom2, 1, 25, 126, "SWATER2",        "" },
    { doom2, 1, 25, 127, "SWATER4",        "" },
    { doom2, 1, 25, 128, "SWATER4",        "" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map25[] =
{
    { doom2, 1,  25,   0, 202 },
    { doom2, 1,  25,  24, 212 },
    { doom2, 1,  25,  33, 212 },
    { doom2, 1,  25,  37, 212 },
    { doom2, 1,  25,  39, 100 },
    { doom2, 1,  25,  41, 212 },
    { doom2, 1,  25,  44, 100 },
    { doom2, 1,  25,  45, 202 },
    { doom2, 1,  25,  47, 202 },
    { doom2, 1,  25,  48, 222 },
    { doom2, 1,  25,  75, 217 },
    { doom2, 1,  25,  85, 227 },
    { doom2, 1,  25, 114, 102 },
    { doom2, 1,  25, 115, 102 },
    { doom2, 1,  25, 125, 100 },
    { doom2, 1,  25, 126, 100 },
    { doom2, 1,  25, 127, 100 },
    { doom2, 1,  25, 128, 100 },
    FLOW_END
};

static const fall_t fall_doom2_map25[] =
{
    { doom2, 1, 25, 367, 102 },
    { doom2, 1, 25, 372, 102 },
    { doom2, 1, 25, 379, 102 },
    { doom2, 1, 25, 389, 102 },
    { doom2, 1, 25, 394, 102 },
    { doom2, 1, 25, 452, 102 },
    { doom2, 1, 25, 472, 102 },
    { doom2, 1, 25, 473, 102 },
    { doom2, 1, 25, 474, 102 },
    { doom2, 1, 25, 475, 102 },
    { doom2, 1, 25, 476, 102 },
    { doom2, 1, 25, 477, 102 },
    { doom2, 1, 25, 478, 102 },
    { doom2, 1, 25, 479, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 26: THE ABANDONED MINES
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map26[] =
{
    { doom2, 1, 26, 761, 1, "METAL2", "", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map26[] =
{
    { doom2, 1,  26,  74,        "", "CEIL5_1" },
    { doom2, 1,  26,  79, "SWATER1",        "" },
    { doom2, 1,  26, 113,        "",  "FLAT19" },
    { doom2, 1,  26, 115,        "",  "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map26[] =
{
    { doom2, 1, 26,   0, 101 },
    { doom2, 1, 26,  10, 100 },
    { doom2, 1, 26,  22, 101 },
    { doom2, 1, 26,  25, 101 },
    { doom2, 1, 26,  39, 100 },
    { doom2, 1, 26,  42, 100 },
    { doom2, 1, 26,  72, 101 },
    { doom2, 1, 26,  79, 211 },
    { doom2, 1, 26,  92, 100 },
    { doom2, 1, 26, 104, 100 },
    { doom2, 1, 26, 107, 100 },
    { doom2, 1, 26, 108, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 27: MONSTER CONDO
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map27[] =
{
    { doom2, 1, 27,   4, 0,        "", "",         "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 27,  66, 0,        "", "",         "",      -5, DEFAULT,          DEFAULT },
    { doom2, 1, 27,  67, 0,        "", "",         "",      -5, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 375, 0,        "", "",         "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 376, 0,        "", "",         "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 385, 0,        "", "",         "",     -24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 386, 0,        "", "",         "",     -48, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 402, 0,        "", "",         "",     -40, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 404, 0,        "", "",         "",      48, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 416, 0,        "", "",         "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 417, 0,        "", "",         "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 426, 0,        "", "",         "",     -24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 427, 0,        "", "",         "",     -48, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 437, 0,        "", "",         "",     -40, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 439, 0,        "", "",         "",      48, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 448, 0,        "", "",         "",     -24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 449, 0,        "", "",         "",     -48, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 511, 0,        "", "",         "",     -24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 512, 0,        "", "",         "",     -48, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 540, 0,        "", "",         "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 547, 0,        "", "",         "",      40, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 548, 0,        "", "",         "",      24, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 578, 0,        "", "",         "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 27, 579, 0,        "", "",         "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { doom2, 1, 27, 582, 1, "ZIMMER3", "",         "", DEFAULT, DEFAULT,          DEFAULT },
    { doom2, 1, 27, 797, 0,        "", "",         "", DEFAULT, DEFAULT,    ML_DONTPEGTOP },
    { doom2, 1, 27, 810, 1,        "", "", "WOODVERT", DEFAULT, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map27[] =
{
    { doom2, 1, 27,  27, "",   "FLAT10" },
    { doom2, 1, 27,  80, "",  "FLAT5_2" },
    { doom2, 1, 27,  82, "", "FLOOR7_2" },
    { doom2, 1, 27,  87, "",   "FLAT20" },
    { doom2, 1, 27, 142, "",  "FLAT5_3" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map27[] =
{
    { doom2, 1, 27, 105, 101 },
    { doom2, 1, 27, 139, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 28: THE SPIRIT WORLD
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map28[] =
{
    { doom2, 1, 28,  38, 1,         "",         "",  "ZIMMER8", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28,  39, 1,         "",         "",  "ZIMMER8", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 103, 0, "ASHWALL6",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 104, 0, "ASHWALL6",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 105, 0, "ASHWALL6",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 106, 0, "ASHWALL6",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 107, 0, "ASHWALL6",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 161, 1,         "",         "",  "ZIMMER8", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 163, 0,         "",         "",         "",      32, DEFAULT, DEFAULT },
    { doom2, 1, 28, 164, 0,         "",         "",         "",      19, DEFAULT, DEFAULT },
    { doom2, 1, 28, 170, 1,         "",         "", "FIRELAVA", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 213, 1,         "",         "",  "ZIMMER8", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 214, 1,         "",         "",  "ZIMMER8", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 215, 1,         "",         "",  "ZIMMER8", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 221, 0,         "",         "",   "BFALL1", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 255, 0,         "",         "",         "",     -29, DEFAULT, DEFAULT },
    { doom2, 1, 28, 256, 0,         "",         "",         "",      29, DEFAULT, DEFAULT },
    { doom2, 1, 28, 256, 1,         "",         "",  "ZIMMER8", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 258, 0,         "",         "",         "",     -24, DEFAULT, DEFAULT },
    { doom2, 1, 28, 260, 0,         "",         "",         "",      29, DEFAULT, DEFAULT },
    { doom2, 1, 28, 264, 0,         "",         "",         "",      29, DEFAULT, DEFAULT },
    { doom2, 1, 28, 266, 0,         "",         "",         "",     -29, DEFAULT, DEFAULT },
    { doom2, 1, 28, 269, 0,         "",         "",         "",     -24, DEFAULT, DEFAULT },
    { doom2, 1, 28, 271, 0,         "",         "",         "",      29, DEFAULT, DEFAULT },
    { doom2, 1, 28, 287, 0,         "",         "",         "",      43, DEFAULT, DEFAULT },
    { doom2, 1, 28, 288, 0,         "",         "",         "",      63, DEFAULT, DEFAULT },
    { doom2, 1, 28, 289, 0,         "",         "",         "",      39, DEFAULT, DEFAULT },
    { doom2, 1, 28, 290, 0,         "",         "",         "",      54, DEFAULT, DEFAULT },
    { doom2, 1, 28, 357, 0,         "",         "",         "",      45, DEFAULT, DEFAULT },
    { doom2, 1, 28, 360, 0,         "",         "",         "",      62, DEFAULT, DEFAULT },
    { doom2, 1, 28, 361, 0,         "",         "",         "",      54, DEFAULT, DEFAULT },
    { doom2, 1, 28, 362, 0,         "",         "",         "",      47, DEFAULT, DEFAULT },
    { doom2, 1, 28, 363, 0,         "",         "",         "",      56, DEFAULT, DEFAULT },
    { doom2, 1, 28, 364, 0,         "",         "",         "",      28, DEFAULT, DEFAULT },
    { doom2, 1, 28, 365, 0,         "",         "",         "",       1, DEFAULT, DEFAULT },
    { doom2, 1, 28, 388, 1,         "",         "", "FIREBLU2", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 391, 0, "BIGDOOR5",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 531, 1,    "WOOD8",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 547, 1,    "WOOD8",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 548, 1,    "WOOD8",         "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 619, 0,         "",         "",         "",     -12, DEFAULT, DEFAULT },
    { doom2, 1, 28, 650, 1,         "", "MIDBARS1",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 651, 1,         "", "MIDBARS1",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 652, 1,         "", "MIDBARS1",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 28, 686, 0,         "",         "",         "",      64, DEFAULT, DEFAULT },
    { doom2, 1, 28, 687, 0,         "",         "",         "",     245, DEFAULT, DEFAULT },
    { doom2, 1, 28, 688, 0,         "",         "",         "",     204, DEFAULT, DEFAULT },
    { doom2, 1, 28, 689, 0,         "",         "",         "",      11, DEFAULT, DEFAULT },
    { doom2, 1, 28, 690, 0,         "",         "",         "",      52, DEFAULT, DEFAULT },
    { doom2, 1, 28, 692, 0,         "",         "",         "",     192, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const flow_t flow_doom2_map28[] =
{
    { doom2, 1, 28, 100, 211 },
    { doom2, 1, 28, 101, 211 },
    { doom2, 1, 28, 102, 211 },
    FLOW_END
};

static const fall_t fall_doom2_map28[] =
{
    { doom2, 1, 28, 124, 101 },
    { doom2, 1, 28, 125, 101 },
    { doom2, 1, 28, 126, 101 },
    { doom2, 1, 28, 127, 101 },
    { doom2, 1, 28, 128, 101 },
    { doom2, 1, 28, 201, 101 },
    { doom2, 1, 28, 202, 101 },
    { doom2, 1, 28, 203, 101 },
    { doom2, 1, 28, 204, 101 },
    { doom2, 1, 28, 205, 101 },
    { doom2, 1, 28, 206, 101 },
    { doom2, 1, 28, 221, 100 },
    { doom2, 1, 28, 283, 101 },
    { doom2, 1, 28, 284, 101 },
    { doom2, 1, 28, 285, 101 },
    { doom2, 1, 28, 286, 101 },
    { doom2, 1, 28, 287, 101 },
    { doom2, 1, 28, 289, 101 },
    { doom2, 1, 28, 290, 101 },
    { doom2, 1, 28, 291, 101 },
    { doom2, 1, 28, 292, 101 },
    { doom2, 1, 28, 293, 101 },
    { doom2, 1, 28, 315, 101 },
    { doom2, 1, 28, 318, 101 },
    { doom2, 1, 28, 323, 101 },
    { doom2, 1, 28, 324, 101 },
    { doom2, 1, 28, 325, 101 },
    { doom2, 1, 28, 326, 101 },
    { doom2, 1, 28, 327, 105 },
    { doom2, 1, 28, 351, 101 },
    { doom2, 1, 28, 435, 101 },
    { doom2, 1, 28, 436, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 29: THE LIVING END
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map29[] =
{
    { doom2, 1, 29,  6,   0,      "", "",         "",      48, DEFAULT, DEFAULT },
    { doom2, 1, 29,  105, 0,      "", "",         "",      56, DEFAULT, DEFAULT },
    { doom2, 1, 29,  106, 0,      "", "",         "",      40, DEFAULT, DEFAULT },
    { doom2, 1, 29,  373, 0,      "", "",         "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 29,  374, 0,      "", "",         "",     -32, DEFAULT, DEFAULT },
    { doom2, 1, 29,  370, 0,      "", "",         "",     -40, DEFAULT, DEFAULT },
    { doom2, 1, 29,  378, 0,      "", "",         "",      26, DEFAULT, DEFAULT },
    { doom2, 1, 29,  405, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  406, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  407, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  408, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  516, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  517, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  518, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  519, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  524, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  525, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  526, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  527, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29,  603, 1, "WOOD5", "",         "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1138, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1139, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1140, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1141, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1146, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1147, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1148, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 29, 1149, 1,      "", "", "SUPPORT3", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_doom2_map29[] =
{
    { doom2, 1, 29,   6, "CEIL5_2",        "" },
    { doom2, 1, 29, 204,        "", "FLAT5_1" },
    SECTORFIX_END
};

static const flow_t flow_doom2_map29[] =
{
    { doom2, 1, 29,   0, 102 },
    { doom2, 1, 29,  29, 102 },
    { doom2, 1, 29,  30, 102 },
    { doom2, 1, 29,  33, 100 },
    { doom2, 1, 29, 116, 102 },
    { doom2, 1, 29, 118, 102 },
    { doom2, 1, 29, 121, 102 },
    { doom2, 1, 29, 150, 100 },
    { doom2, 1, 29, 161, 100 },
    { doom2, 1, 29, 163, 100 },
    { doom2, 1, 29, 164, 100 },
    { doom2, 1, 29, 165, 100 },
    { doom2, 1, 29, 194, 100 },
    { doom2, 1, 29, 195, 100 },
    { doom2, 1, 29, 196, 100 },
    { doom2, 1, 29, 199, 100 },
    { doom2, 1, 29, 200, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 30: ICON OF SIN
// -----------------------------------------------------------------------------

static const vertexfix_t vertexfix_doom2_map30[] =
{
    { doom2, 1, 30, 50, 4224,  -64, 4224, 64 },
    { doom2, 1, 30, 51, 4224, -256, 4224, 64 },
    VERTEXFIX_END
};

static const linefix_t linefix_doom2_map30[] =
{
    { doom2, 1, 30, 38, 0, "", "", "",      32, DEFAULT,     DEFAULT },
    { doom2, 1, 30, 39, 0, "", "", "",      32, DEFAULT,     DEFAULT },
    { doom2, 1, 30, 48, 0, "", "", "", DEFAULT,      32,     DEFAULT },
    { doom2, 1, 30, 55, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTDRAW },
    LINEFIX_END
};

static const flow_t flow_doom2_map30[] =
{
    { doom2, 1, 30, 0, 102 },
    FLOW_END
};

static const fall_t fall_doom2_map30[] =
{
    { doom2, 1, 30,  7, 102 },
    { doom2, 1, 30,  8, 102 },
    { doom2, 1, 30,  9, 102 },
    { doom2, 1, 30, 10, 102 },
    { doom2, 1, 30, 11, 102 },
    { doom2, 1, 30, 26, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 31: WOLFENSTEIN
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map31[] =
{
    { doom2, 1, 31,  32, 1,        "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31,  34, 1,        "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31,  41, 1,        "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31,  43, 1,        "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31,  57, 1,        "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 137, 1,        "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 163, 1,        "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 210, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 218, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 226, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 234, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 243, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 251, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 259, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 266, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 274, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 282, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 316, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 330, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 338, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 364, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 409, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 431, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 452, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 459, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 548, 0,        "",         "", "",      -2, DEFAULT, DEFAULT },
    { doom2, 1, 31, 550, 0,        "",         "", "",      -2, DEFAULT, DEFAULT },
    { doom2, 1, 31, 551, 0,        "",         "", "",      -2, DEFAULT, DEFAULT },
    { doom2, 1, 31, 553, 0,        "",         "", "",      -2, DEFAULT, DEFAULT },
    { doom2, 1, 31, 569, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 594, 0, "ZDOORB1",         "", "", DEFAULT, DEFAULT, DEFAULT },
    { doom2, 1, 31, 666, 0,        "",         "", "",      25, DEFAULT, DEFAULT },
    { doom2, 1, 31, 668, 0,        "",         "", "",      25, DEFAULT, DEFAULT },
    { doom2, 1, 31, 670, 0,        "",         "", "",      25, DEFAULT, DEFAULT },
    { doom2, 1, 31, 672, 0,        "",         "", "",      25, DEFAULT, DEFAULT },
    LINEFIX_END
};

// -----------------------------------------------------------------------------
// LEVEL 32: GROSSE
// -----------------------------------------------------------------------------

static const linefix_t linefix_doom2_map32[] =
{
    { doom2, 1, 32, 30, 0, "", "", "", DEFAULT, 0, DEFAULT },
    { doom2, 1, 32, 31, 0, "", "", "", DEFAULT, 0, DEFAULT },
    LINEFIX_END
};


// =============================================================================
// FINAL DOOM: THE PLUTONIA EXPERIMENT
// =============================================================================


// -----------------------------------------------------------------------------
// LEVEL 1: CONGO
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map01[] =
{
    { pack_plut, 1, 1,  552, 0, "", "", "", 1, DEFAULT, DEFAULT },
    { pack_plut, 1, 1, 1110, 0, "", "", "", 0,       0, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map01[] =
{
    { pack_plut, 1, 1,  23,        "", "CEIL5_2" },
    { pack_plut, 1, 1,  24,        "", "CEIL5_2" },
    { pack_plut, 1, 1,  28, "CRATOP1",        "" },
    { pack_plut, 1, 1,  32,        "", "CEIL5_2" },
    { pack_plut, 1, 1,  34, "CRATOP1",        "" },
    { pack_plut, 1, 1,  40, "CEIL5_2",        "" },
    { pack_plut, 1, 1,  80,        "",  "FLAT20" },
    { pack_plut, 1, 1,  81,        "",  "FLAT20" },
    { pack_plut, 1, 1,  82,   "FLAT1",   "FLAT1" },
    { pack_plut, 1, 1,  94,        "", "CEIL5_2" },
    { pack_plut, 1, 1, 103,        "", "CEIL5_2" },
    { pack_plut, 1, 1, 117,        "",  "FLAT20" },
    { pack_plut, 1, 1, 124,        "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map01[] =
{
    { pack_plut, 1, 1,  8, 100 },
    { pack_plut, 1, 1, 56, 100 },
    { pack_plut, 1, 1, 72, 100 },
    { pack_plut, 1, 1, 83, 100 },
    { pack_plut, 1, 1, 85, 100 },
    { pack_plut, 1, 1, 86, 100 },
    { pack_plut, 1, 1, 87, 100 },
    { pack_plut, 1, 1, 95, 100 },
    { pack_plut, 1, 1, 98, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 2: WELL OF SOULS
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map02[] =
{
    { pack_plut, 1, 2,   44, 0, "", "", "",      -4, DEFAULT, DEFAULT },
    { pack_plut, 1, 2, 1173, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_plut, 1, 2, 1174, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_plut, 1, 2, 1176, 0, "", "", "",       4, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map02[] =
{
    { pack_plut, 1, 2, 16,        "",  "FLAT20" },
    { pack_plut, 1, 2, 78,        "", "CEIL5_2" },
    { pack_plut, 1, 2, 89, "CEIL5_1",        "" },
    { pack_plut, 1, 2, 93,        "",  "FLAT20" },
    { pack_plut, 1, 2, 96,  "FLAT23",        "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map02[] =
{
    { pack_plut, 1, 2,   0, 101 },
    { pack_plut, 1, 2,   5, 101 },
    { pack_plut, 1, 2,   6, 101 },
    { pack_plut, 1, 2,   7, 101 },
    { pack_plut, 1, 2,   9, 101 },
    { pack_plut, 1, 2,  12, 101 },
    { pack_plut, 1, 2,  14, 101 },
    { pack_plut, 1, 2,  17, 102 },
    { pack_plut, 1, 2,  18, 102 },
    { pack_plut, 1, 2,  20, 102 },
    { pack_plut, 1, 2,  21, 102 },
    { pack_plut, 1, 2,  33, 101 },
    { pack_plut, 1, 2,  34, 101 },
    { pack_plut, 1, 2,  52, 100 },
    { pack_plut, 1, 2,  55, 100 },
    { pack_plut, 1, 2,  56, 100 },
    { pack_plut, 1, 2, 120, 101 },
    { pack_plut, 1, 2, 121, 223 },
    { pack_plut, 1, 2, 122, 223 },
    { pack_plut, 1, 2, 123, 102 },
    { pack_plut, 1, 2, 124, 101 },
    { pack_plut, 1, 2, 125, 101 },
    FLOW_END
};

static const fall_t fall_plut_map02[] =
{
    { pack_plut, 1, 2,  21, 103 },
    { pack_plut, 1, 2,  30, 103 },
    { pack_plut, 1, 2,  31, 103 },
    { pack_plut, 1, 2,  32, 103 },
    { pack_plut, 1, 2,  47, 103 },
    { pack_plut, 1, 2,  58, 103 },
    { pack_plut, 1, 2,  73, 103 },
    { pack_plut, 1, 2, 144, 103 },
    { pack_plut, 1, 2, 145, 103 },
    { pack_plut, 1, 2, 146, 103 },
    { pack_plut, 1, 2, 147, 103 },
    { pack_plut, 1, 2, 148, 103 },
    { pack_plut, 1, 2, 149, 103 },
    { pack_plut, 1, 2, 173, 103 },
    { pack_plut, 1, 2, 174, 103 },
    { pack_plut, 1, 2, 175, 103 },
    { pack_plut, 1, 2, 176, 103 },
    { pack_plut, 1, 2, 177, 103 },
    { pack_plut, 1, 2, 178, 103 },
    { pack_plut, 1, 2, 179, 103 },
    { pack_plut, 1, 2, 180, 103 },
    { pack_plut, 1, 2, 181, 103 },
    { pack_plut, 1, 2, 182, 103 },
    { pack_plut, 1, 2, 183, 103 },
    { pack_plut, 1, 2, 736, 103 },
    { pack_plut, 1, 2, 737, 103 },
    { pack_plut, 1, 2, 738, 103 },
    { pack_plut, 1, 2, 739, 103 },
    { pack_plut, 1, 2, 740, 103 },
    { pack_plut, 1, 2, 741, 103 },
    { pack_plut, 1, 2, 742, 103 },
    { pack_plut, 1, 2, 743, 103 },
    { pack_plut, 1, 2, 744, 103 },
    { pack_plut, 1, 2, 745, 103 },
    { pack_plut, 1, 2, 746, 103 },
    { pack_plut, 1, 2, 747, 103 },
    { pack_plut, 1, 2, 748, 103 },
    { pack_plut, 1, 2, 749, 103 },
    { pack_plut, 1, 2, 750, 103 },
    { pack_plut, 1, 2, 752, 103 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 3: AZTEC
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map03[] =
{
    { pack_plut, 1, 3,  558, 1, "", "", "",      -6, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  560, 1, "", "", "",      -6, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  693, 1, "", "", "",      64, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  694, 1, "", "", "",     207, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  695, 1, "", "", "",      79, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  696, 1, "", "", "",     222, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  716, 1, "", "", "",      91, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  717, 1, "", "", "",     234, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  719, 1, "", "", "",     128, DEFAULT, DEFAULT },
    { pack_plut, 1, 3,  720, 1, "", "", "",      15, DEFAULT, DEFAULT },
    { pack_plut, 1, 3, 1173, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_plut, 1, 3, 1174, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_plut, 1, 3, 1176, 0, "", "", "",       4, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map03[] =
{
    { pack_plut, 1, 3,  35, "", "FLAT5_1" },
    { pack_plut, 1, 3, 120, "",  "FLAT23" },
    { pack_plut, 1, 3, 121, "",  "FLAT23" },
    { pack_plut, 1, 3, 128, "",   "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map03[] =
{
    { pack_plut, 1, 3,  18, 100 },
    { pack_plut, 1, 3,  22, 100 },
    { pack_plut, 1, 3,  23, 100 },
    { pack_plut, 1, 3,  24, 100 },
    { pack_plut, 1, 3,  37, 100 },
    { pack_plut, 1, 3,  47, 100 },
    { pack_plut, 1, 3,  48, 100 },
    { pack_plut, 1, 3,  49, 100 },
    { pack_plut, 1, 3,  74, 100 },
    { pack_plut, 1, 3,  76, 100 },
    { pack_plut, 1, 3,  77, 100 },
    { pack_plut, 1, 3,  78, 100 },
    { pack_plut, 1, 3,  91, 100 },
    { pack_plut, 1, 3,  93, 100 },
    { pack_plut, 1, 3, 104, 100 },
    { pack_plut, 1, 3, 107, 100 },
    { pack_plut, 1, 3, 108, 100 },
    { pack_plut, 1, 3, 109, 100 },
    { pack_plut, 1, 3, 123, 100 },
    { pack_plut, 1, 3, 132, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 4: CAGED
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map04[] =
{
    { pack_plut, 1, 4, 303, 1, "", "MIDBRONZ", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 4, 308, 1, "", "MIDBRONZ", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 4, 531, 0, "",         "", "", DEFAULT,      18, DEFAULT },
    { pack_plut, 1, 4, 532, 0, "",         "", "", DEFAULT,      18, DEFAULT },
    { pack_plut, 1, 4, 762, 1, "", "MIDBARS3", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 4, 763, 1, "", "MIDBARS3", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 4, 845, 0, "",         "", "",       4,      -2, DEFAULT },
    { pack_plut, 1, 4, 852, 0, "",         "", "",       4,      -2, DEFAULT },
    { pack_plut, 1, 4, 975, 0, "",         "", "",      87, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map04[] =
{
    { pack_plut, 1, 4,  35,        "",   "FLAT20" },
    { pack_plut, 1, 4,  63,        "",  "CRATOP2" },
    { pack_plut, 1, 4,  64,        "",  "CRATOP2" },
    { pack_plut, 1, 4, 103, "CEIL5_1",         "" },
    { pack_plut, 1, 4, 139,        "",   "FLAT20" },
    { pack_plut, 1, 4, 147,  "FLAT19",         "" },
    { pack_plut, 1, 4, 148,   "FLAT1",         "" },
    { pack_plut, 1, 4, 164,        "", "FLOOR7_1" },
    { pack_plut, 1, 4, 169,        "", "FLOOR7_1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map04[] =
{
    { pack_plut, 1, 4,   3, 100 },
    { pack_plut, 1, 4,   4, 100 },
    { pack_plut, 1, 4,   5, 100 },
    { pack_plut, 1, 4,  10, 100 },
    { pack_plut, 1, 4,  33, 220 },
    { pack_plut, 1, 4,  72, 100 },
    { pack_plut, 1, 4,  86, 100 },
    { pack_plut, 1, 4,  93, 100 },
    { pack_plut, 1, 4,  97, 100 },
    { pack_plut, 1, 4, 141, 100 },
    { pack_plut, 1, 4, 184, 100 },
    { pack_plut, 1, 4, 188, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 5: GHOST TOWN
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map05[] =
{
    { pack_plut, 1, 5, 411, 0, "A-BROCK2", "", "A-BROCK2", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map05[] =
{
    { pack_plut, 1, 5, 103, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map05[] =
{
    { pack_plut, 1, 5,  0, 100 },
    { pack_plut, 1, 5, 33, 101 },
    { pack_plut, 1, 5, 35, 100 },
    { pack_plut, 1, 5, 48, 101 },
    { pack_plut, 1, 5, 51, 101 },
    { pack_plut, 1, 5, 69, 101 },
    FLOW_END
};

static const fall_t fall_plut_map05[] =
{
    { pack_plut, 1, 5, 342, 103 },
    { pack_plut, 1, 5, 343, 103 },
    { pack_plut, 1, 5, 344, 103 },
    { pack_plut, 1, 5, 345, 103 },
    { pack_plut, 1, 5, 354, 103 },
    { pack_plut, 1, 5, 355, 103 },
    { pack_plut, 1, 5, 356, 103 },
    { pack_plut, 1, 5, 357, 103 },
    { pack_plut, 1, 5, 443, 103 },
    { pack_plut, 1, 5, 447, 103 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 6: BARON'S LAIR
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map06[] =
{
    { pack_plut, 1, 6, 1337, 1, "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 6, 1343, 1, "", "MIDGRATE", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 6,  236, 1, "",  "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 6,  239, 1, "",  "A-RAIL1", "",      -7, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map06[] =
{
    { pack_plut, 1, 6,  49,        "", "FLOOR7_1" },
    { pack_plut, 1, 6,  72,        "",   "FLAT20" },
    { pack_plut, 1, 6,  74,        "",   "FLAT20" },
    { pack_plut, 1, 6, 102, "CEIL5_2",         "" },
    { pack_plut, 1, 6, 118, "CEIL5_1",         "" },
    { pack_plut, 1, 6, 133,        "",   "FLAT20" },
    { pack_plut, 1, 6, 149,        "",   "FLAT23" },
    { pack_plut, 1, 6, 150,        "",   "FLAT23" },
    { pack_plut, 1, 6, 193, "SLIME15",         "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map06[] =
{
    { pack_plut, 1, 6,  88, 100 },
    { pack_plut, 1, 6, 119, 100 },
    { pack_plut, 1, 6, 120, 100 },
    { pack_plut, 1, 6, 121, 100 },
    { pack_plut, 1, 6, 122, 100 },
    { pack_plut, 1, 6, 130, 100 },
    { pack_plut, 1, 6, 161, 101 },
    { pack_plut, 1, 6, 162, 102 },
    { pack_plut, 1, 6, 163, 101 },
    FLOW_END
};

static const fall_t fall_plut_map06[] =
{
    { pack_plut, 1, 6, 1011, 102 },
    { pack_plut, 1, 6, 1012, 102 },
    { pack_plut, 1, 6, 1013, 102 },
    { pack_plut, 1, 6, 1014, 102 },
    { pack_plut, 1, 6, 1015, 102 },
    { pack_plut, 1, 6, 1021, 102 },
    { pack_plut, 1, 6, 1027, 102 },
    { pack_plut, 1, 6, 1033, 102 },
    { pack_plut, 1, 6, 1043, 102 },
    { pack_plut, 1, 6, 1044, 102 },
    { pack_plut, 1, 6, 1045, 102 },
    { pack_plut, 1, 6, 1046, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 7: CAUGHTYARD
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map07[] =
{
    { pack_plut, 1, 7, 50, 0, "", "", "", 6, DEFAULT, DEFAULT },
    { pack_plut, 1, 7, 51, 0, "", "", "", 6, DEFAULT, DEFAULT },
    { pack_plut, 1, 7, 79, 0, "", "", "", 6, DEFAULT, DEFAULT },
    { pack_plut, 1, 7, 82, 0, "", "", "", 6, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map07[] =
{
    { pack_plut, 1, 7, 31,        "", "FLOOR3_3" },
    { pack_plut, 1, 7, 33, "FLAT5_1",         "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map07[] =
{
    { pack_plut, 1, 7,  5, 102 },
    { pack_plut, 1, 7, 23, 100 },
    { pack_plut, 1, 7, 32, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 8: REALM
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map08[] =
{
    { pack_plut, 1, 8, 236, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 8, 239, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 8, 247, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 8, 249, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 8, 857, 0, "",        "", "",     -16, DEFAULT, ML_DONTPEGTOP },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map08[] =
{
    { pack_plut, 1, 8,   9,       "",  "RROCK09" },
    { pack_plut, 1, 8,  12,       "",  "CEIL5_2" },
    { pack_plut, 1, 8,  60,       "",  "CRATOP2" },
    { pack_plut, 1, 8,  98, "FLAT19",         "" },
    { pack_plut, 1, 8, 122,       "", "FLOOR7_1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map08[] =
{
    { pack_plut, 1, 8,   0, 102 },
    { pack_plut, 1, 8,   5, 102 },
    { pack_plut, 1, 8,   7, 102 },
    { pack_plut, 1, 8,   8, 102 },
    { pack_plut, 1, 8,  10, 102 },
    { pack_plut, 1, 8,  13, 102 },
    { pack_plut, 1, 8,  19, 224 },
    { pack_plut, 1, 8,  20, 223 },
    { pack_plut, 1, 8,  23, 102 },
    { pack_plut, 1, 8,  24, 223 },
    { pack_plut, 1, 8,  25, 101 },
    { pack_plut, 1, 8,  26, 101 },
    { pack_plut, 1, 8,  34, 101 },
    { pack_plut, 1, 8,  47, 101 },
    { pack_plut, 1, 8,  55, 101 },
    { pack_plut, 1, 8,  56, 101 },
    { pack_plut, 1, 8,  57, 101 },
    { pack_plut, 1, 8,  76, 101 },
    { pack_plut, 1, 8,  87, 101 },
    { pack_plut, 1, 8,  89, 101 },
    { pack_plut, 1, 8, 100, 101 },
    { pack_plut, 1, 8, 101, 101 },
    { pack_plut, 1, 8, 116, 100 },
    FLOW_END
};

static const fall_t fall_plut_map08[] =
{
    { pack_plut, 1, 8,   5, 103 },
    { pack_plut, 1, 8,  17, 103 },
    { pack_plut, 1, 8,  93, 104 },
    { pack_plut, 1, 8,  94, 103 },
    { pack_plut, 1, 8,  95, 104 },
    { pack_plut, 1, 8,  96, 104 },
    { pack_plut, 1, 8,  97, 104 },
    { pack_plut, 1, 8,  98, 104 },
    { pack_plut, 1, 8,  99, 104 },
    { pack_plut, 1, 8, 100, 104 },
    { pack_plut, 1, 8, 222, 103 },
    { pack_plut, 1, 8, 227, 103 },
    { pack_plut, 1, 8, 233, 103 },
    { pack_plut, 1, 8, 236, 103 },
    { pack_plut, 1, 8, 239, 103 },
    { pack_plut, 1, 8, 241, 103 },
    { pack_plut, 1, 8, 244, 103 },
    { pack_plut, 1, 8, 247, 103 },
    { pack_plut, 1, 8, 249, 103 },
    { pack_plut, 1, 8, 256, 103 },
    { pack_plut, 1, 8, 260, 103 },
    { pack_plut, 1, 8, 262, 103 },
    { pack_plut, 1, 8, 540, 103 },
    { pack_plut, 1, 8, 798, 103 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 9: ABATTOIRE
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map09[] =
{
    { pack_plut, 1, 9,  849, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_plut, 1, 9,  850, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_plut, 1, 9,  942, 0, "",         "", "",      18, DEFAULT,          DEFAULT },
    { pack_plut, 1, 9, 1179, 0, "", "DOORTRAK", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 9, 1181, 0, "", "DOORTRAK", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map09[] =
{
    { pack_plut, 1, 9,  52,  "SWATER2",         "" },
    { pack_plut, 1, 9,  56,  "SWATER2",         "" },
    { pack_plut, 1, 9,  74,  "CEIL5_1",         "" },
    { pack_plut, 1, 9,  75,  "CEIL5_1",         "" },
    { pack_plut, 1, 9,  78,         "",  "CEIL5_2" },
    { pack_plut, 1, 9,  80,  "CEIL5_1",         "" },
    { pack_plut, 1, 9,  83, "FLOOR3_3",         "" },
    { pack_plut, 1, 9,  99,         "",  "CEIL5_2" },
    { pack_plut, 1, 9, 103,         "", "FLOOR7_1" },
    { pack_plut, 1, 9, 118,         "",  "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map09[] =
{
    { pack_plut, 1, 9,   6, 100 },
    { pack_plut, 1, 9,   7, 100 },
    { pack_plut, 1, 9,   8, 100 },
    { pack_plut, 1, 9,   9, 100 },
    { pack_plut, 1, 9,  10, 100 },
    { pack_plut, 1, 9,  35, 100 },
    { pack_plut, 1, 9,  43, 100 },
    { pack_plut, 1, 9,  44, 100 },
    { pack_plut, 1, 9,  52, 200 },
    { pack_plut, 1, 9,  56, 200 },
    { pack_plut, 1, 9,  57, 101 },
    { pack_plut, 1, 9,  59, 101 },
    { pack_plut, 1, 9,  62, 100 },
    { pack_plut, 1, 9,  63, 100 },
    { pack_plut, 1, 9,  64, 100 },
    { pack_plut, 1, 9,  71, 101 },
    { pack_plut, 1, 9,  72, 101 },
    { pack_plut, 1, 9, 120, 100 },
    { pack_plut, 1, 9, 125, 100 },
    FLOW_END
};

static const fall_t fall_plut_map09[] =
{
    { pack_plut, 1, 9, 548, 101 },
    { pack_plut, 1, 9, 549, 101 },
    { pack_plut, 1, 9, 550, 101 },
    { pack_plut, 1, 9, 552, 101 },
    { pack_plut, 1, 9, 553, 105 },
    { pack_plut, 1, 9, 554, 101 },
    { pack_plut, 1, 9, 556, 101 },
    { pack_plut, 1, 9, 557, 101 },
    { pack_plut, 1, 9, 558, 105 },
    { pack_plut, 1, 9, 559, 101 },
    { pack_plut, 1, 9, 560, 101 },
    { pack_plut, 1, 9, 562, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 10: ONSLAUGHT
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map10[] =
{
    { pack_plut, 1, 10,  137, 0,        "",        "", "",       6,       4,       DEFAULT },
    { pack_plut, 1, 10,  138, 0,        "",        "", "",       6,       4,       DEFAULT },
    { pack_plut, 1, 10,  141, 0,        "",        "", "",       6,       4,       DEFAULT },
    { pack_plut, 1, 10,  142, 0,        "",        "", "",       6,       4,       DEFAULT },
    { pack_plut, 1, 10,  720, 0,        "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 10,  799, 0,        "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 10,  800, 0,        "",        "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 10,  988, 1,        "", "A-RAIL1", "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 10, 1010, 0, "GSTONE1",        "", "", DEFAULT, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map10[] =
{
    { pack_plut, 1, 10,   3,        "", "FLOOR7_1" },
    { pack_plut, 1, 10,  24, "CEIL5_2",         "" },
    { pack_plut, 1, 10,  35,  "FLAT19",         "" },
    { pack_plut, 1, 10,  38, "RROCK09",         "" },
    { pack_plut, 1, 10,  44,        "",  "FLAT5_5" },
    { pack_plut, 1, 10,  46,  "FLAT19",         "" },
    { pack_plut, 1, 10,  56, "CEIL5_2",         "" },
    { pack_plut, 1, 10,  64,        "",  "CEIL5_2" },
    { pack_plut, 1, 10,  65,        "",  "CEIL5_2" },
    { pack_plut, 1, 10,  66,        "",  "CEIL5_2" },
    { pack_plut, 1, 10,  67,        "",  "CEIL5_2" },
    { pack_plut, 1, 10,  68,        "",  "CEIL5_2" },
    { pack_plut, 1, 10,  74, "CEIL5_2",         "" },
    { pack_plut, 1, 10,  83,        "",  "CEIL5_2" },
    { pack_plut, 1, 10,  90,        "",  "CEIL5_2" },
    { pack_plut, 1, 10, 102,        "", "FLOOR5_4" },
    { pack_plut, 1, 10, 110, "CEIL5_2",         "" },
    { pack_plut, 1, 10, 112,        "", "FLOOR7_1" },
    { pack_plut, 1, 10, 117,        "",  "FLAT5_1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map10[] =
{
    { pack_plut, 1, 10,  12, 100 },
    { pack_plut, 1, 10,  93, 235 },
    { pack_plut, 1, 10,  94, 235 },
    { pack_plut, 1, 10, 123, 100 },
    { pack_plut, 1, 10, 124, 100 },
    { pack_plut, 1, 10, 125, 100 },
    { pack_plut, 1, 10, 126, 100 },
    { pack_plut, 1, 10, 132, 100 },
    { pack_plut, 1, 10, 134, 100 },
    { pack_plut, 1, 10, 135, 100 },
    { pack_plut, 1, 10, 136, 100 },
    { pack_plut, 1, 10, 137, 100 },
    { pack_plut, 1, 10, 144, 100 },
    { pack_plut, 1, 10, 147, 100 },
    { pack_plut, 1, 10, 152, 221 },
    { pack_plut, 1, 10, 154, 100 },
    { pack_plut, 1, 10, 155, 100 },
    { pack_plut, 1, 10, 157, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 11: HUNTED
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_plut_map11[] =
{
    { pack_plut, 1, 11,  7, "FLAT5_1",        "" },
    { pack_plut, 1, 11,  8, "FLAT5_1",        "" },
    { pack_plut, 1, 11, 52,        "", "FLAT5_5" },
    { pack_plut, 1, 11, 53,        "", "FLAT5_5" },
    { pack_plut, 1, 11, 54, "CEIL5_1",        "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map11[] =
{
    { pack_plut, 1, 11, 103, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 12: SPEED
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map12[] =
{
    { pack_plut, 1, 12,  690, 0, "",         "",      "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { pack_plut, 1, 12,  985, 0, "",    "WOOD5",      "",      64, DEFAULT,                            DEFAULT },
    { pack_plut, 1, 12,  988, 0, "",    "WOOD5",      "",     -32, DEFAULT,                            DEFAULT },
    { pack_plut, 1, 12,  996, 1, "",         "", "WOOD1",     -96, DEFAULT,                            DEFAULT },
    { pack_plut, 1, 12, 1018, 0, "",         "",      "", DEFAULT,       0, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { pack_plut, 1, 12, 1151, 0, "",    "WOOD5",      "",     -96,       0,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map12[] =
{
    { pack_plut, 1, 12,  14,        "", "CEIL5_2" },
    { pack_plut, 1, 12,  97, "CEIL5_2",        "" },
    { pack_plut, 1, 12, 109,        "", "CEIL5_2" },
    { pack_plut, 1, 12, 111, "CEIL5_2",        "" },
    { pack_plut, 1, 12, 125,        "", "CEIL5_2" },
    { pack_plut, 1, 12, 142,        "", "CEIL5_2" },
    { pack_plut, 1, 12, 155,        "", "CEIL5_2" },
    { pack_plut, 1, 12, 160,        "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map12[] =
{
    { pack_plut, 1, 12,  11, 100 },
    { pack_plut, 1, 12,  12, 100 },
    { pack_plut, 1, 12,  19, 100 },
    { pack_plut, 1, 12,  29, 101 },
    { pack_plut, 1, 12,  47, 100 },
    { pack_plut, 1, 12,  49, 232 },
    { pack_plut, 1, 12,  58, 100 },
    { pack_plut, 1, 12,  60, 100 },
    { pack_plut, 1, 12,  68, 101 },
    { pack_plut, 1, 12, 127, 100 },
    { pack_plut, 1, 12, 131, 101 },
    { pack_plut, 1, 12, 133, 101 },
    { pack_plut, 1, 12, 138, 101 },
    { pack_plut, 1, 12, 143, 101 },
    { pack_plut, 1, 12, 144, 100 },
    { pack_plut, 1, 12, 147, 101 },
    { pack_plut, 1, 12, 148, 101 },
    { pack_plut, 1, 12, 152, 235 },
    FLOW_END
};

static const fall_t fall_plut_map12[] =
{
    { pack_plut, 1, 12, 148, 102 },
    { pack_plut, 1, 12, 151, 102 },
    { pack_plut, 1, 12, 335, 102 },
    { pack_plut, 1, 12, 564, 102 },
    { pack_plut, 1, 12, 569, 102 },
    { pack_plut, 1, 12, 659, 102 },
    { pack_plut, 1, 12, 658, 102 },
    { pack_plut, 1, 12, 657, 102 },
    { pack_plut, 1, 12, 656, 102 },
    { pack_plut, 1, 12, 655, 102 },
    { pack_plut, 1, 12, 654, 102 },
    { pack_plut, 1, 12, 653, 102 },
    { pack_plut, 1, 12, 652, 102 },
    { pack_plut, 1, 12, 651, 102 },
    { pack_plut, 1, 12, 650, 102 },
    { pack_plut, 1, 12, 649, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 13: THE CRYPT
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map13[] =
{
    { pack_plut, 1, 13, 107, 1, "A-BROWN5", "", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 13, 119, 1, "A-BROWN5", "", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map13[] =
{
    { pack_plut, 1, 13,  33,         "", "CEIL5_2" },
    { pack_plut, 1, 13,  33,         "", "CEIL5_2" },
    { pack_plut, 1, 13,  34,         "", "CEIL5_2" },
    { pack_plut, 1, 13,  46,  "RROCK09",        "" },
    { pack_plut, 1, 13,  82, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  83, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  84, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  85, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  86, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  87, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  88, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  89, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  90, "FLOOR7_1",        "" },
    { pack_plut, 1, 13,  93, "FLAT5_5",         "" },
    { pack_plut, 1, 13, 114, "FLOOR3_3",        "" },
    { pack_plut, 1, 13, 149,  "CEIL5_2",        "" },
    { pack_plut, 1, 13, 169,   "FLAT19",        "" },
    { pack_plut, 1, 13, 170,         "", "RROCK09" },
    SECTORFIX_END
};

static const flow_t flow_plut_map13[] =
{
    { pack_plut, 1, 13,  73, 226 },
    { pack_plut, 1, 13,  74, 226 },
    { pack_plut, 1, 13,  76, 226 },
    { pack_plut, 1, 13,  77, 226 },
    { pack_plut, 1, 13,  78, 226 },
    { pack_plut, 1, 13, 102, 231 },
    { pack_plut, 1, 13, 106, 231 },
    { pack_plut, 1, 13, 138, 101 },
    { pack_plut, 1, 13, 142, 101 },
    { pack_plut, 1, 13, 158, 221 },
    { pack_plut, 1, 13, 158, 221 },
    { pack_plut, 1, 13, 159, 221 },
    { pack_plut, 1, 13, 162, 221 },
    { pack_plut, 1, 13, 164, 221 },
    { pack_plut, 1, 13, 165, 221 },
    FLOW_END
};

static const fall_t fall_plut_map13[] =
{
    { pack_plut, 1, 13,  549, 102 },
    { pack_plut, 1, 13,  993, 103 },
    { pack_plut, 1, 13,  994, 103 },
    { pack_plut, 1, 13,  995, 103 },
    { pack_plut, 1, 13,  996, 103 },
    { pack_plut, 1, 13,  997, 103 },
    { pack_plut, 1, 13,  998, 103 },
    { pack_plut, 1, 13,  999, 103 },
    { pack_plut, 1, 13, 1000, 103 },
    { pack_plut, 1, 13, 1001, 103 },
    { pack_plut, 1, 13, 1002, 103 },
    { pack_plut, 1, 13, 1003, 103 },
    { pack_plut, 1, 13, 1004, 103 },
    { pack_plut, 1, 13, 1005, 103 },
    { pack_plut, 1, 13, 1006, 103 },
    { pack_plut, 1, 13, 1007, 103 },
    { pack_plut, 1, 13, 1008, 103 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 14: GENESIS
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map14[] =
{
    { pack_plut, 1, 14,  770, 0, "",         "", "",       6, DEFAULT, DEFAULT },
    { pack_plut, 1, 14,  808, 1, "",         "", "",       6, DEFAULT, DEFAULT },
    { pack_plut, 1, 14, 1099, 1, "", "MIDBARS3", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 14, 1103, 1, "", "MIDBARS3", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 14, 1113, 1, "", "MIDBARS3", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map14[] =
{
    { pack_plut, 1, 14,  56, "", "CEIL5_2" },
    { pack_plut, 1, 14, 105, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map14[] =
{
    { pack_plut, 1, 14,   7, 213 },
    { pack_plut, 1, 14,  22, 213 },
    { pack_plut, 1, 14,  26, 213 },
    { pack_plut, 1, 14,  32, 213 },
    { pack_plut, 1, 14,  39, 213 },
    { pack_plut, 1, 14,  40, 213 },
    { pack_plut, 1, 14,  41, 213 },
    { pack_plut, 1, 14,  42, 213 },
    { pack_plut, 1, 14,  44, 213 },
    { pack_plut, 1, 14,  68, 213 },
    { pack_plut, 1, 14,  84, 213 },
    { pack_plut, 1, 14,  98, 213 },
    { pack_plut, 1, 14, 106, 213 },
    { pack_plut, 1, 14, 107, 213 },
    { pack_plut, 1, 14, 108, 213 },
    FLOW_END
};

static const fall_t fall_plut_map14[] =
{
    { pack_plut, 1, 14,  428, 102 },
    { pack_plut, 1, 14,  515, 102 },
    { pack_plut, 1, 14,  516, 102 },
    { pack_plut, 1, 14,  517, 102 },
    { pack_plut, 1, 14,  518, 102 },
    { pack_plut, 1, 14,  519, 102 },
    { pack_plut, 1, 14,  520, 102 },
    { pack_plut, 1, 14,  521, 102 },
    { pack_plut, 1, 14, 1101, 102 },
    { pack_plut, 1, 14, 1106, 102 },
    { pack_plut, 1, 14, 1111, 102 },
    { pack_plut, 1, 14, 1116, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 15: THE TWILIGHT
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map15[] =
{
    { pack_plut, 1, 15,   65, 0, "",         "", "",       0, DEFAULT, DEFAULT },
    { pack_plut, 1, 15,  407, 1, "",  "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 15,  874, 1, "", "BRNSMAL2", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 15, 1113, 0, "",         "", "",       0, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map15[] =
{
    { pack_plut, 1,  15,  55,        "", "CEIL5_2" },
    { pack_plut, 1,  15,  97, "CEIL5_2",        "" },
    { pack_plut, 1,  15, 139,        "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map15[] =
{
    { pack_plut, 1, 15,   0, 101 },
    { pack_plut, 1, 15,  11, 101 },
    { pack_plut, 1, 15,  28, 101 },
    { pack_plut, 1, 15,  48, 101 },
    { pack_plut, 1, 15,  57, 100 },
    { pack_plut, 1, 15, 101, 101 },
    { pack_plut, 1, 15, 102, 101 },
    { pack_plut, 1, 15, 123, 101 },
    { pack_plut, 1, 15, 126, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 16: THE OMEN
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map16[] =
{
    { pack_plut, 1, 16, 667, 1, "", "MIDGRATE", "", DEFAULT, DEFAULT,          DEFAULT },
    { pack_plut, 1, 16, 740, 0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 16, 751, 0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map16[] =
{
    { pack_plut, 1, 16,  70, "", "CEIL5_2" },
    { pack_plut, 1, 16,  78, "", "CEIL5_2" },
    { pack_plut, 1, 16,  94, "", "FLAT5_5" },
    { pack_plut, 1, 16,  97, "", "NUKAGE1" },
    { pack_plut, 1, 16, 104, "", "NUKAGE1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map16[] =
{
    { pack_plut, 1, 16,  0, 101 },
    { pack_plut, 1, 16, 11, 101 },
    { pack_plut, 1, 16, 12, 101 },
    { pack_plut, 1, 16, 13, 101 },
    { pack_plut, 1, 16, 32, 102 },
    { pack_plut, 1, 16, 33, 102 },
    { pack_plut, 1, 16, 34, 102 },
    { pack_plut, 1, 16, 51, 211 },
    { pack_plut, 1, 16, 52, 211 },
    { pack_plut, 1, 16, 53, 211 },
    { pack_plut, 1, 16, 54, 211 },
    { pack_plut, 1, 16, 55, 211 },
    { pack_plut, 1, 16, 56, 211 },
    { pack_plut, 1, 16, 57, 211 },
    { pack_plut, 1, 16, 58, 211 },
    { pack_plut, 1, 16, 71, 211 },
    { pack_plut, 1, 16, 72, 211 },
    { pack_plut, 1, 16, 75, 231 },
    { pack_plut, 1, 16, 95, 222 },
    { pack_plut, 1, 16, 96, 222 },
    FLOW_END
};

static const fall_t fall_plut_map16[] =
{
    { pack_plut, 1, 16, 330, 102 },
    { pack_plut, 1, 16, 334, 102 },
    { pack_plut, 1, 16, 378, 101 },
    { pack_plut, 1, 16, 379, 101 },
    { pack_plut, 1, 16, 380, 101 },
    { pack_plut, 1, 16, 381, 101 },
    { pack_plut, 1, 16, 382, 101 },
    { pack_plut, 1, 16, 383, 101 },
    { pack_plut, 1, 16, 384, 101 },
    { pack_plut, 1, 16, 385, 101 },
    { pack_plut, 1, 16, 386, 101 },
    { pack_plut, 1, 16, 387, 101 },
    { pack_plut, 1, 16, 388, 101 },
    { pack_plut, 1, 16, 413, 101 },
    { pack_plut, 1, 16, 414, 101 },
    { pack_plut, 1, 16, 415, 101 },
    { pack_plut, 1, 16, 416, 101 },
    { pack_plut, 1, 16, 417, 101 },
    { pack_plut, 1, 16, 418, 101 },
    { pack_plut, 1, 16, 419, 101 },
    { pack_plut, 1, 16, 420, 101 },
    { pack_plut, 1, 16, 421, 101 },
    { pack_plut, 1, 16, 422, 101 },
    { pack_plut, 1, 16, 423, 101 },
    { pack_plut, 1, 16, 618, 100 },
    { pack_plut, 1, 16, 619, 100 },
    { pack_plut, 1, 16, 620, 100 },
    { pack_plut, 1, 16, 621, 100 },
    { pack_plut, 1, 16, 622, 100 },
    { pack_plut, 1, 16, 623, 100 },
    { pack_plut, 1, 16, 624, 100 },
    { pack_plut, 1, 16, 625, 100 },
    { pack_plut, 1, 16, 626, 100 },
    { pack_plut, 1, 16, 629, 100 },
    { pack_plut, 1, 16, 630, 100 },
    { pack_plut, 1, 16, 631, 100 },
    { pack_plut, 1, 16, 632, 100 },
    { pack_plut, 1, 16, 633, 100 },
    { pack_plut, 1, 16, 634, 100 },
    { pack_plut, 1, 16, 635, 100 },
    { pack_plut, 1, 16, 636, 100 },
    { pack_plut, 1, 16, 637, 100 },
    { pack_plut, 1, 16, 643, 100 },
    { pack_plut, 1, 16, 640, 100 },
    { pack_plut, 1, 16, 641, 100 },
    { pack_plut, 1, 16, 642, 100 },
    { pack_plut, 1, 16, 644, 100 },
    { pack_plut, 1, 16, 645, 100 },
    { pack_plut, 1, 16, 646, 100 },
    { pack_plut, 1, 16, 649, 100 },
    { pack_plut, 1, 16, 650, 100 },
    { pack_plut, 1, 16, 651, 100 },
    { pack_plut, 1, 16, 652, 100 },
    { pack_plut, 1, 16, 653, 100 },
    { pack_plut, 1, 16, 656, 100 },
    { pack_plut, 1, 16, 657, 100 },
    { pack_plut, 1, 16, 658, 100 },
    { pack_plut, 1, 16, 659, 100 },
    { pack_plut, 1, 16, 660, 100 },
    { pack_plut, 1, 16, 663, 100 },
    { pack_plut, 1, 16, 664, 100 },
    { pack_plut, 1, 16, 665, 100 },
    { pack_plut, 1, 16, 668, 100 },
    { pack_plut, 1, 16, 669, 100 },
    { pack_plut, 1, 16, 670, 100 },
    { pack_plut, 1, 16, 671, 100 },
    { pack_plut, 1, 16, 672, 100 },
    { pack_plut, 1, 16, 680, 100 },
    { pack_plut, 1, 16, 681, 100 },
    { pack_plut, 1, 16, 682, 100 },
    { pack_plut, 1, 16, 683, 100 },
    { pack_plut, 1, 16, 684, 100 },
    { pack_plut, 1, 16, 686, 100 },
    { pack_plut, 1, 16, 688, 100 },
    { pack_plut, 1, 16, 909, 101 },
    { pack_plut, 1, 16, 911, 101 },
    { pack_plut, 1, 16, 912, 101 },
    { pack_plut, 1, 16, 913, 101 },
    { pack_plut, 1, 16, 914, 101 },
    { pack_plut, 1, 16, 915, 101 },
    { pack_plut, 1, 16, 916, 101 },
    { pack_plut, 1, 16, 930, 105 },
    { pack_plut, 1, 16, 932, 105 },
    { pack_plut, 1, 16, 934, 105 },
    { pack_plut, 1, 16, 937, 103 },
    { pack_plut, 1, 16, 938, 103 },
    { pack_plut, 1, 16, 939, 103 },
    { pack_plut, 1, 16, 940, 103 },
    { pack_plut, 1, 16, 941, 103 },
    { pack_plut, 1, 16, 942, 103 },
    { pack_plut, 1, 16, 943, 103 },
    { pack_plut, 1, 16, 944, 103 },
    { pack_plut, 1, 16, 945, 103 },
    { pack_plut, 1, 16, 946, 103 },
    { pack_plut, 1, 16, 947, 103 },
    { pack_plut, 1, 16, 948, 103 },
    { pack_plut, 1, 16, 949, 103 },
    { pack_plut, 1, 16, 950, 103 },
    { pack_plut, 1, 16, 951, 103 },
    { pack_plut, 1, 16, 952, 103 },
    { pack_plut, 1, 16, 953, 103 },
    { pack_plut, 1, 16, 954, 103 },
    { pack_plut, 1, 16, 955, 103 },
    { pack_plut, 1, 16, 956, 103 },
    { pack_plut, 1, 16, 957, 103 },
    { pack_plut, 1, 16, 958, 103 },
    { pack_plut, 1, 16, 959, 103 },
    { pack_plut, 1, 16, 960, 103 },
    { pack_plut, 1, 16, 961, 103 },
    { pack_plut, 1, 16, 962, 103 },
    { pack_plut, 1, 16, 963, 103 },
    { pack_plut, 1, 16, 964, 103 },
    { pack_plut, 1, 16, 965, 103 },
    { pack_plut, 1, 16, 966, 103 },
    { pack_plut, 1, 16, 967, 103 },
    { pack_plut, 1, 16, 968, 103 },
    { pack_plut, 1, 16, 969, 103 },
    { pack_plut, 1, 16, 970, 103 },
    { pack_plut, 1, 16, 971, 103 },
    { pack_plut, 1, 16, 972, 103 },
    { pack_plut, 1, 16, 973, 103 },
    { pack_plut, 1, 16, 974, 103 },
    { pack_plut, 1, 16, 975, 103 },
    { pack_plut, 1, 16, 976, 103 },
    { pack_plut, 1, 16, 977, 103 },
    { pack_plut, 1, 16, 978, 103 },
    { pack_plut, 1, 16, 979, 103 },
    { pack_plut, 1, 16, 980, 103 },
    { pack_plut, 1, 16, 981, 103 },
    { pack_plut, 1, 16, 982, 103 },
    { pack_plut, 1, 16, 983, 103 },
    { pack_plut, 1, 16, 984, 103 },
    { pack_plut, 1, 16, 985, 103 },
    { pack_plut, 1, 16, 986, 103 },
    { pack_plut, 1, 16, 987, 103 },
    { pack_plut, 1, 16, 988, 103 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 17: COMPOUND
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_plut_map17[] =
{
    { pack_plut, 1, 17,  79,        "",  "FLAT5_5" },
    { pack_plut, 1, 17,  83, "CEIL5_1",         "" },
    { pack_plut, 1, 17,  92,        "",  "CEIL5_2" },
    { pack_plut, 1, 17,  96,        "",   "FLAT20" },
    { pack_plut, 1, 17, 121,        "", "FLOOR7_1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map17[] =
{
    { pack_plut, 1, 17,   0, 211 },
    { pack_plut, 1, 17,  10, 211 },
    { pack_plut, 1, 17,  13, 211 },
    { pack_plut, 1, 17,  14, 211 },
    { pack_plut, 1, 17,  15, 211 },
    { pack_plut, 1, 17,  16, 211 },
    { pack_plut, 1, 17,  23, 211 },
    { pack_plut, 1, 17,  24, 211 },
    { pack_plut, 1, 17,  35, 211 },
    { pack_plut, 1, 17,  73, 211 },
    { pack_plut, 1, 17,  75, 100 },
    { pack_plut, 1, 17, 102, 101 },
    { pack_plut, 1, 17, 103, 101 },
    { pack_plut, 1, 17, 114, 211 },
    { pack_plut, 1, 17, 115, 211 },
    { pack_plut, 1, 17, 123, 211 },
    { pack_plut, 1, 17, 124, 211 },
    FLOW_END
};

static const fall_t fall_plut_map17[] =
{
    { pack_plut, 1, 17,  151, 101 },
    { pack_plut, 1, 17,  319, 101 },
    { pack_plut, 1, 17, 1177, 102 },
    { pack_plut, 1, 17, 1183, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 18: NEUROSPHERE
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_plut_map18[] =
{
    { pack_plut, 1, 18, 105,         "", "CEIL5_2" },
    { pack_plut, 1, 18, 106,   "FLAT20",        "" },
    { pack_plut, 1, 18, 110, "FLOOR7_1",        "" },
    { pack_plut, 1, 18, 139,         "", "CEIL5_2" },
    { pack_plut, 1, 18, 162,         "", "CEIL5_2" },
    { pack_plut, 1, 18, 164,   "FLAT20",        "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map18[] =
{
    { pack_plut, 1, 18,   0, 101 },
    { pack_plut, 1, 18,  22, 101 },
    { pack_plut, 1, 18,  27, 100 },
    { pack_plut, 1, 18,  86, 101 },
    { pack_plut, 1, 18, 115, 101 },
    { pack_plut, 1, 18, 129, 101 },
    { pack_plut, 1, 18, 130, 101 },
    { pack_plut, 1, 18, 133, 101 },
    { pack_plut, 1, 18, 135, 101 },
    { pack_plut, 1, 18, 169, 101 },
    { pack_plut, 1, 18, 170, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 19: NME
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map19[] =
{
    { pack_plut, 1, 19, 361,   1, "", "MIDBRONZ", "", DEFAULT, DEFAULT,          DEFAULT },
    { pack_plut, 1, 19, 366,   1, "", "MIDBRONZ", "", DEFAULT, DEFAULT,          DEFAULT },
    { pack_plut, 1, 19, 677,   0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 19, 678,   0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 19, 679,   0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 19, 824,   0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map19[] =
{
    { pack_plut, 1, 19,  21, "", "CEIL5_2" },
    { pack_plut, 1, 19, 125, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map19[] =
{
    { pack_plut, 1, 19,   0, 100 },
    { pack_plut, 1, 19,  50, 100 },
    { pack_plut, 1, 19,  54, 100 },
    { pack_plut, 1, 19,  55, 100 },
    { pack_plut, 1, 19,  56, 100 },
    { pack_plut, 1, 19,  57, 100 },
    { pack_plut, 1, 19,  58, 100 },
    { pack_plut, 1, 19,  59, 100 },
    { pack_plut, 1, 19,  87, 100 },
    { pack_plut, 1, 19, 112, 101 },
    { pack_plut, 1, 19, 114, 101 },
    FLOW_END
};

static const fall_t fall_plut_map19[] =
{
    { pack_plut, 1, 19, 532, 100 },
    { pack_plut, 1, 19, 533, 100 },
    { pack_plut, 1, 19, 534, 100 },
    { pack_plut, 1, 19, 535, 100 },
    { pack_plut, 1, 19, 536, 100 },
    { pack_plut, 1, 19, 537, 100 },
    { pack_plut, 1, 19, 538, 100 },
    { pack_plut, 1, 19, 539, 100 },
    { pack_plut, 1, 19, 540, 100 },
    { pack_plut, 1, 19, 541, 100 },
    { pack_plut, 1, 19, 542, 100 },
    { pack_plut, 1, 19, 543, 100 },
    { pack_plut, 1, 19, 544, 100 },
    { pack_plut, 1, 19, 545, 100 },
    { pack_plut, 1, 19, 546, 100 },
    { pack_plut, 1, 19, 547, 100 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 20: THE DEATH DOMAIN
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map20[] =
{
    { pack_plut, 1,  20,  179, 0, "",        "", "",       6, DEFAULT, DEFAULT },
    { pack_plut, 1,  20,  194, 0, "",        "", "",       6, DEFAULT, DEFAULT },
    { pack_plut, 1,  20,  267, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1,  20,  297, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1,  20,  315, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1,  20,  511, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1,  20,  666, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  670, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  671, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  672, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  678, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  679, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  680, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  681, 0, "",        "", "", DEFAULT,     -13, DEFAULT },
    { pack_plut, 1,  20,  869, 0, "",        "", "", DEFAULT,     -50, DEFAULT },
    { pack_plut, 1,  20,  876, 0, "",        "", "", DEFAULT,      65, DEFAULT },
    { pack_plut, 1,  20,  882, 0, "",        "", "", DEFAULT,      65, DEFAULT },
    { pack_plut, 1,  20,  904, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1,  20, 1110, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1,  20, 1115, 1, "", "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1,  20, 1146, 0, "",        "", "",     104, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map20[] =
{
    { pack_plut, 1, 20,  45,  "CEIL5_2",  "CEIL5_2" },
    { pack_plut, 1, 20,  49,         "",   "FLAT20" },
    { pack_plut, 1, 20,  59, "FLOOR7_2", "FLOOR7_2" },
    { pack_plut, 1, 20,  69,  "FLAT5_1",         "" },
    { pack_plut, 1, 20,  80,         "",  "FLAT5_4" },
    { pack_plut, 1, 20,  81,         "",  "CEIL5_2" },
    { pack_plut, 1, 20, 100, "FLOOR3_3",         "" },
    { pack_plut, 1, 20, 105,         "",   "FLAT20" },
    { pack_plut, 1, 20, 106,         "",   "FLAT20" },
    { pack_plut, 1, 20, 119,         "",  "RROCK09" },
    { pack_plut, 1, 20, 125,         "",  "CEIL5_2" },
    { pack_plut, 1, 20, 128, "FLOOR7_1",         "" },
    { pack_plut, 1, 20, 144,         "",   "FLAT20" },
    { pack_plut, 1, 20, 164,         "",  "FLAT5_4" },
    { pack_plut, 1, 20, 177, "FLOOR7_1",         "" },
    { pack_plut, 1, 20, 183,         "",  "CRATOP2" },
    { pack_plut, 1, 20, 189,  "CEIL5_2",         "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map20[] =
{
    {  pack_plut, 1, 20,   1, 100 },
    {  pack_plut, 1, 20,  12, 100 },
    {  pack_plut, 1, 20,  20, 100 },
    {  pack_plut, 1, 20,  22, 100 },
    {  pack_plut, 1, 20,  32, 100 },
    {  pack_plut, 1, 20,  33, 100 },
    {  pack_plut, 1, 20,  35, 100 },
    {  pack_plut, 1, 20,  36, 100 },
    {  pack_plut, 1, 20,  37, 100 },
    {  pack_plut, 1, 20,  40, 100 },
    {  pack_plut, 1, 20,  56, 100 },
    {  pack_plut, 1, 20,  57, 100 },
    {  pack_plut, 1, 20,  60, 100 },
    {  pack_plut, 1, 20,  61, 100 },
    {  pack_plut, 1, 20,  62, 100 },
    {  pack_plut, 1, 20,  63, 100 },
    {  pack_plut, 1, 20,  70, 100 },
    {  pack_plut, 1, 20,  71, 100 },
    {  pack_plut, 1, 20,  72, 100 },
    {  pack_plut, 1, 20,  73, 100 },
    {  pack_plut, 1, 20, 108, 100 },
    {  pack_plut, 1, 20, 109, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 21: SLAYER
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_plut_map21[] =
{
    { pack_plut, 1, 21, 46, "", "FLAT5_1" },
    { pack_plut, 1, 21, 47, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map21[] =
{
    {  pack_plut, 1, 21,  0, 101 },
    {  pack_plut, 1, 21,  1, 101 },
    {  pack_plut, 1, 21, 39, 101 },
    {  pack_plut, 1, 21, 48, 101 },
    {  pack_plut, 1, 21, 56, 100 },
    {  pack_plut, 1, 21, 57, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 22: IMPOSSIBLE MISSION
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map22[] =
{
    { pack_plut, 1, 22,  375, 1, "",  "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 22,  602, 0, "",         "", "",     -20,       1, DEFAULT },
    { pack_plut, 1, 22,  604, 0, "",         "", "",      13,       1, DEFAULT },
    { pack_plut, 1, 22,  763, 0, "",         "", "",      17,       1, DEFAULT },
    { pack_plut, 1, 22,  765, 0, "",         "", "",      17,       1, DEFAULT },
    { pack_plut, 1, 22, 1026, 1, "",         "", "",      45,      40, DEFAULT },
    { pack_plut, 1, 22, 1028, 1, "",         "", "",     -16,      40, DEFAULT },
    { pack_plut, 1, 22, 1033, 1, "",  "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 22, 1034, 1, "",  "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 22, 1035, 1, "",  "A-RAIL1", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 22, 1115, 1, "", "MIDBARS3", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 22, 1636, 0, "",         "", "",       6,       1, DEFAULT },
    { pack_plut, 1, 22, 1637, 0, "",         "", "",       2,       1, DEFAULT },
    { pack_plut, 1, 22, 1639, 0, "",         "", "",       2,       1, DEFAULT },
    { pack_plut, 1, 22, 1672, 0, "",         "", "",       7,       1, DEFAULT },
    { pack_plut, 1, 22, 1673, 0, "",         "", "",       7,       1, DEFAULT },
    { pack_plut, 1, 22, 1674, 0, "",         "", "",       7,       1, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map22[] =
{
    { pack_plut, 1, 22,   4,        "", "CEIL5_2" },
    { pack_plut, 1, 22,   5,        "", "CEIL5_1" },
    { pack_plut, 1, 22,  22,        "", "CEIL5_2" },
    { pack_plut, 1, 22,  34,        "", "CEIL5_1" },
    { pack_plut, 1, 22,  87,        "", "CEIL5_1" },
    { pack_plut, 1, 22,  89,        "", "CEIL5_1" },
    { pack_plut, 1, 22, 100, "RROCK09",        "" },    
    { pack_plut, 1, 22, 127,        "",  "FLAT20" },
    { pack_plut, 1, 22, 129,        "",  "FLAT20" },
    { pack_plut, 1, 22, 137, "CEIL1_1",        "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map22[] =
{
    { pack_plut, 1, 22,  10, 102 },
    { pack_plut, 1, 22,  24, 102 },
    { pack_plut, 1, 22,  30, 102 },
    { pack_plut, 1, 22,  33, 102 },
    { pack_plut, 1, 22,  34, 102 },
    { pack_plut, 1, 22,  39, 102 },
    { pack_plut, 1, 22,  40, 102 },
    { pack_plut, 1, 22,  41, 102 },
    { pack_plut, 1, 22,  42, 102 },
    { pack_plut, 1, 22,  65, 101 },
    { pack_plut, 1, 22,  85, 102 },
    { pack_plut, 1, 22,  86, 102 },
    { pack_plut, 1, 22,  87, 102 },
    { pack_plut, 1, 22,  88, 102 },
    { pack_plut, 1, 22,  89, 102 },
    { pack_plut, 1, 22,  91, 102 },
    { pack_plut, 1, 22,  92, 102 },
    { pack_plut, 1, 22,  93, 102 },
    { pack_plut, 1, 22,  94, 102 },
    { pack_plut, 1, 22, 110, 202 },
    { pack_plut, 1, 22, 111, 202 },
    { pack_plut, 1, 22, 115, 202 },
    { pack_plut, 1, 22, 116, 102 },
    { pack_plut, 1, 22, 118, 102 },
    { pack_plut, 1, 22, 119, 102 },
    { pack_plut, 1, 22, 120, 211 },
    { pack_plut, 1, 22, 121, 211 },
    { pack_plut, 1, 22, 122, 101 },
    { pack_plut, 1, 22, 124, 102 },
    { pack_plut, 1, 22, 125, 102 },
    { pack_plut, 1, 22, 131, 102 },
    { pack_plut, 1, 22, 132, 102 },
    { pack_plut, 1, 22, 139, 231 },
    { pack_plut, 1, 22, 140, 231 },
    { pack_plut, 1, 22, 141, 101 },
    { pack_plut, 1, 22, 157, 101 },
    { pack_plut, 1, 22, 174, 102 },
    FLOW_END
};

static const fall_t fall_plut_map22[] =
{
    { pack_plut, 1, 22,  301, 101 },
    { pack_plut, 1, 22,  302, 101 },
    { pack_plut, 1, 22,  784, 102 },
    { pack_plut, 1, 22,  832, 101 },
    { pack_plut, 1, 22,  866, 101 },
    { pack_plut, 1, 22,  867, 101 },
    { pack_plut, 1, 22,  868, 101 },
    { pack_plut, 1, 22,  869, 101 },
    { pack_plut, 1, 22,  870, 101 },
    { pack_plut, 1, 22,  871, 101 },
    { pack_plut, 1, 22,  872, 101 },
    { pack_plut, 1, 22,  895, 101 },
    { pack_plut, 1, 22,  896, 101 },
    { pack_plut, 1, 22,  911, 101 },
    { pack_plut, 1, 22,  927, 101 },
    { pack_plut, 1, 22,  933, 101 },
    { pack_plut, 1, 22,  959, 101 },
    { pack_plut, 1, 22,  970, 101 },
    { pack_plut, 1, 22,  977, 101 },
    { pack_plut, 1, 22,  983, 101 },
    { pack_plut, 1, 22,  989, 101 },
    { pack_plut, 1, 22,  992, 101 },
    { pack_plut, 1, 22, 1005, 101 },
    { pack_plut, 1, 22, 1166, 101 },
    { pack_plut, 1, 22, 1177, 101 },
    { pack_plut, 1, 22, 1180, 102 },
    { pack_plut, 1, 22, 1190, 101 },
    { pack_plut, 1, 22, 1191, 101 },
    { pack_plut, 1, 22, 1192, 101 },
    { pack_plut, 1, 22, 1193, 101 },
    { pack_plut, 1, 22, 1194, 101 },
    { pack_plut, 1, 22, 1198, 101 },
    { pack_plut, 1, 22, 1455, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 23: TOMBSTONE
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map23[] =
{
    { pack_plut, 1, 23, 1100, 0, "",         "", "", DEFAULT,      16, DEFAULT },
    { pack_plut, 1, 23, 1463, 1, "", "BRNSMALR", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 23, 1468, 1, "", "BRNSMALC", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 23, 1468, 1, "", "BRNSMALL", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map23[] =
{
    { pack_plut, 1, 23,  29,        "", "FLOOR7_1" },
    { pack_plut, 1, 23,  37,        "", "FLOOR7_1" },
    { pack_plut, 1, 23,  39,        "",  "RROCK14" },
    { pack_plut, 1, 23,  68, "FLAT5_1",         "" },
    { pack_plut, 1, 23,  69, "FLAT5_1",         "" },
    { pack_plut, 1, 23,  86,        "", "FLOOR7_1" },
    { pack_plut, 1, 23, 200,        "", "FLOOR7_1" },
    { pack_plut, 1, 23, 226,        "", "FLOOR7_1" },
    { pack_plut, 1, 23, 230,        "",  "CEIL5_2" },
    { pack_plut, 1, 23, 243,        "",  "FLAT5_2" },
    { pack_plut, 1, 23, 244,        "",  "FLAT5_1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map23[] =
{
    { pack_plut, 1, 23,   3, 101 },
    { pack_plut, 1, 23,   6, 101 },
    { pack_plut, 1, 23,  19, 101 },
    { pack_plut, 1, 23,  20, 101 },
    { pack_plut, 1, 23,  21, 101 },
    { pack_plut, 1, 23,  25, 102 },
    { pack_plut, 1, 23,  31, 102 },
    { pack_plut, 1, 23,  88, 226 },
    { pack_plut, 1, 23,  95, 226 },
    { pack_plut, 1, 23, 137, 101 },
    { pack_plut, 1, 23, 138, 101 },
    { pack_plut, 1, 23, 139, 101 },
    { pack_plut, 1, 23, 150, 101 },
    { pack_plut, 1, 23, 151, 101 },
    { pack_plut, 1, 23, 152, 101 },
    { pack_plut, 1, 23, 153, 101 },
    { pack_plut, 1, 23, 154, 101 },
    { pack_plut, 1, 23, 155, 101 },
    { pack_plut, 1, 23, 157, 101 },
    { pack_plut, 1, 23, 160, 101 },
    { pack_plut, 1, 23, 165, 100 },
    { pack_plut, 1, 23, 171, 231 },
    { pack_plut, 1, 23, 172, 231 },
    { pack_plut, 1, 23, 180, 100 },
    { pack_plut, 1, 23, 209, 101 },
    FLOW_END
};

static const fall_t fall_plut_map23[] =
{
    { pack_plut, 1, 23, 392, 102 },
    { pack_plut, 1, 23, 393, 102 },
    { pack_plut, 1, 23, 394, 102 },
    { pack_plut, 1, 23, 395, 102 },
    { pack_plut, 1, 23, 396, 102 },
    { pack_plut, 1, 23, 397, 102 },
    { pack_plut, 1, 23, 398, 102 },
    { pack_plut, 1, 23, 399, 102 },
    { pack_plut, 1, 23, 400, 102 },
    { pack_plut, 1, 23, 401, 102 },
    { pack_plut, 1, 23, 402, 102 },
    { pack_plut, 1, 23, 403, 102 },
    { pack_plut, 1, 23, 404, 102 },
    { pack_plut, 1, 23, 405, 102 },
    { pack_plut, 1, 23, 406, 102 },
    { pack_plut, 1, 23, 407, 102 },
    { pack_plut, 1, 23, 408, 102 },
    { pack_plut, 1, 23, 409, 102 },
    { pack_plut, 1, 23, 410, 102 },
    { pack_plut, 1, 23, 411, 102 },
    { pack_plut, 1, 23, 412, 102 },
    { pack_plut, 1, 23, 413, 102 },
    { pack_plut, 1, 23, 414, 102 },
    { pack_plut, 1, 23, 415, 102 },
    { pack_plut, 1, 23, 416, 102 },
    { pack_plut, 1, 23, 417, 102 },
    { pack_plut, 1, 23, 418, 102 },
    { pack_plut, 1, 23, 419, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 24: THE FINAL FRONTIER
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map24[] =
{
    { pack_plut, 1, 24, 828, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 24, 834, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 24, 835, 0, "", "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_plut, 1, 24, 990, 0, "", "", "",       3, DEFAULT,          DEFAULT },
    { pack_plut, 1, 24, 996, 0, "", "", "",       3, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map24[] =
{
    { pack_plut, 1, 24,  61, "",  "FLAT20" },
    { pack_plut, 1, 24, 123, "",  "FLAT20" },
    { pack_plut, 1, 24, 146, "", "RROCK09" },
    SECTORFIX_END
};

static const flow_t flow_plut_map24[] =
{
    { pack_plut, 1, 24,   2, 206 },
    { pack_plut, 1, 24,   5, 101 },
    { pack_plut, 1, 24,  16, 100 },
    { pack_plut, 1, 24,  27, 100 },
    { pack_plut, 1, 24,  29, 100 },
    { pack_plut, 1, 24,  56, 206 },
    { pack_plut, 1, 24,  62, 206 },
    { pack_plut, 1, 24,  95, 231 },
    { pack_plut, 1, 24,  96, 231 },
    { pack_plut, 1, 24,  97, 231 },
    { pack_plut, 1, 24,  98, 231 },
    { pack_plut, 1, 24,  99, 231 },
    { pack_plut, 1, 24, 100, 206 },
    { pack_plut, 1, 24, 107, 231 },
    { pack_plut, 1, 24, 110, 209 },
    { pack_plut, 1, 24, 110, 206 },
    FLOW_END
};

static const fall_t fall_plut_map24[] =
{
    { pack_plut, 1, 24, 670, 101 },
    { pack_plut, 1, 24, 672, 101 },
    { pack_plut, 1, 24, 673, 101 },
    { pack_plut, 1, 24, 674, 100 },
    { pack_plut, 1, 24, 675, 101 },
    { pack_plut, 1, 24, 676, 101 },
    { pack_plut, 1, 24, 677, 100 },
    { pack_plut, 1, 24, 678, 101 },
    { pack_plut, 1, 24, 679, 101 },
    { pack_plut, 1, 24, 680, 100 },
    { pack_plut, 1, 24, 681, 101 },
    { pack_plut, 1, 24, 682, 101 },
    { pack_plut, 1, 24, 683, 100 },
    { pack_plut, 1, 24, 684, 101 },
    { pack_plut, 1, 24, 685, 100 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 25: THE TEMPLE OF DARKNESS
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map25[] =
{
    { pack_plut, 1, 25,  142, 0,         "", "", "", DEFAULT,       4, DEFAULT },
    { pack_plut, 1, 25,  144, 0,         "", "", "", DEFAULT,       4, DEFAULT },
    { pack_plut, 1, 25,  952, 0,         "", "", "",       6,       1, DEFAULT },
    { pack_plut, 1, 25,  954, 1,         "", "", "",       6,       1, DEFAULT },
    { pack_plut, 1, 25, 1152, 0, "A-BROCK2", "", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 25, 1341, 0,         "", "", "",      -8,       1, DEFAULT },
    { pack_plut, 1, 25, 1342, 0,         "", "", "",       4,       1, DEFAULT },
    { pack_plut, 1, 25, 1343, 0,         "", "", "",       0,       1, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map25[] =
{
    { pack_plut, 1, 25,  47,        "",   "FLAT20" },
    { pack_plut, 1, 25,  48,        "",   "FLAT20" },
    { pack_plut, 1, 25,  49,        "",   "FLAT20" },
    { pack_plut, 1, 25,  51,        "",   "FLAT20" },
    { pack_plut, 1, 25,  52,        "", "FLOOR7_2" },
    { pack_plut, 1, 25,  69,        "",  "CRATOP2" },
    { pack_plut, 1, 25, 126, "CEIL5_2",         "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map25[] =
{
    { pack_plut, 1, 25,  68, 100 },
    { pack_plut, 1, 25,  69, 100 },
    { pack_plut, 1, 25,  81, 100 },
    { pack_plut, 1, 25,  84, 100 },
    { pack_plut, 1, 25,  85, 100 },
    { pack_plut, 1, 25,  86, 100 },
    { pack_plut, 1, 25,  91, 100 },
    { pack_plut, 1, 25,  92, 100 },
    { pack_plut, 1, 25,  93, 100 },
    { pack_plut, 1, 25,  95, 100 },
    { pack_plut, 1, 25,  96, 100 },
    { pack_plut, 1, 25, 107, 100 },
    { pack_plut, 1, 25, 108, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 26: BUNKER
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map26[] =
{
    { pack_plut, 1, 26, 389, 0, "", "DOORTRAK", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 26, 398, 0, "", "DOORTRAK", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map26[] =
{
    { pack_plut, 1, 26,  60,        "", "CRATOP2" },
    { pack_plut, 1, 26,  82,        "", "CEIL5_2" },
    { pack_plut, 1, 26,  98,        "", "CEIL5_2" },
    { pack_plut, 1, 26, 134,        "", "CRATOP2" },
    { pack_plut, 1, 26, 162,  "FLAT23",        "" },
    { pack_plut, 1, 26, 167,        "", "CEIL5_2" },
    { pack_plut, 1, 26, 182, "CEIL3_5",        "" },
    { pack_plut, 1, 26, 202, "CEIL3_5",        "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map26[] =
{
    { pack_plut, 1, 26,   0, 231 },
    { pack_plut, 1, 26,  12, 231 },
    { pack_plut, 1, 26,  13, 231 },
    { pack_plut, 1, 26,  21, 231 },
    { pack_plut, 1, 26,  22, 231 },
    { pack_plut, 1, 26,  23, 231 },
    { pack_plut, 1, 26,  24, 231 },
    { pack_plut, 1, 26,  25, 231 },
    { pack_plut, 1, 26,  26, 231 },
    { pack_plut, 1, 26,  27, 231 },
    { pack_plut, 1, 26,  28, 231 },
    { pack_plut, 1, 26,  29, 231 },
    { pack_plut, 1, 26,  30, 231 },
    { pack_plut, 1, 26,  31, 231 },
    { pack_plut, 1, 26,  33, 231 },
    { pack_plut, 1, 26,  34, 231 },
    { pack_plut, 1, 26,  35, 221 },
    { pack_plut, 1, 26,  36, 221 },
    { pack_plut, 1, 26,  37, 221 },
    { pack_plut, 1, 26,  38, 221 },
    { pack_plut, 1, 26,  40, 231 },
    { pack_plut, 1, 26,  53, 231 },
    { pack_plut, 1, 26,  54, 231 },
    { pack_plut, 1, 26,  87, 231 },
    { pack_plut, 1, 26,  91, 231 },
    { pack_plut, 1, 26, 105, 100 },
    { pack_plut, 1, 26, 112, 100 },
    { pack_plut, 1, 26, 131, 100 },
    { pack_plut, 1, 26, 156, 231 },
    { pack_plut, 1, 26, 175, 100 },
    FLOW_END
};

static const fall_t fall_plut_map26[] =
{
    { pack_plut, 1, 26, 204, 102 },
    { pack_plut, 1, 26, 205, 102 },
    { pack_plut, 1, 26, 206, 102 },
    { pack_plut, 1, 26, 207, 102 },
    { pack_plut, 1, 26, 208, 102 },
    { pack_plut, 1, 26, 273, 102 },
    { pack_plut, 1, 26, 274, 102 },
    { pack_plut, 1, 26, 275, 102 },
    { pack_plut, 1, 26, 276, 102 },
    { pack_plut, 1, 26, 277, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 27: ANTI-CHRIST
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_plut_map27[] =
{
    { pack_plut, 1, 27,   9,  "FLAT5_1",         "" },
    { pack_plut, 1, 27,  12, "FLOOR7_1",         "" },
    { pack_plut, 1, 27,  14,         "",  "FLAT5_1" },
    { pack_plut, 1, 27,  38,         "",  "FLAT5_1" },
    { pack_plut, 1, 27,  39,  "FLAT5_1",         "" },
    { pack_plut, 1, 27,  95,         "", "FLOOR7_1" },
    { pack_plut, 1, 27, 139,  "FLAT5_1",         "" },
    SECTORFIX_END
};

static const flow_t flow_plut_map27[] =
{
    { pack_plut, 1, 27,   0, 205 },
    { pack_plut, 1, 27,  35, 100 },
    { pack_plut, 1, 27,  57, 100 },
    { pack_plut, 1, 27, 155, 205 },
    { pack_plut, 1, 27, 159, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 28: THE SEWERS
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map28[] =
{
    { pack_plut, 1, 28,  179, 1,        "", "MIDBARS3",        "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28,  195, 1,        "", "BRNSMAL2",        "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28,  199, 1,        "", "BRNSMAL2",        "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28,  204, 1,        "", "BRNSMAL2",        "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28,  209, 1,        "", "BRNSMAL1",        "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28,  675, 0,        "",         "", "BRICK10", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28,  676, 0,        "",         "", "BRICK10", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28, 1232, 0,        "",         "",        "", DEFAULT,      -1,       DEFAULT },
    { pack_plut, 1, 28, 1233, 0,        "",         "",        "", DEFAULT,      -1,       DEFAULT },
    { pack_plut, 1, 28, 1234, 0,        "",         "",        "", DEFAULT,      -1,       DEFAULT },
    { pack_plut, 1, 28, 1235, 0,        "",         "",        "", DEFAULT,      -1,       DEFAULT },
    { pack_plut, 1, 28, 1711, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1712, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1713, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1714, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1715, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1716, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1717, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1718, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1719, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 1720, 0,        "",         "",        "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_plut, 1, 28, 2352, 1,        "", "MIDGRATE",        "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28, 2360, 1,        "", "MIDGRATE",        "", DEFAULT, DEFAULT,       DEFAULT },
    { pack_plut, 1, 28, 2496, 0, "BRICK10",         "",        "", DEFAULT, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map28[] =
{
    { pack_plut, 1, 28,  23,        "",  "FLAT5_1" },
    { pack_plut, 1, 28,  49,        "",  "CEIL5_2" },
    { pack_plut, 1, 28,  62,        "",   "FLAT20" },
    { pack_plut, 1, 28,  98,        "", "FLOOR7_2" },
    { pack_plut, 1, 28,  99,        "", "FLOOR7_2" },
    { pack_plut, 1, 28, 116,        "",  "FLAT5_1" },
    { pack_plut, 1, 28, 162,        "",   "FLAT20" },
    { pack_plut, 1, 28, 177,        "",   "FLAT20" },
    { pack_plut, 1, 28, 225,        "",   "FLAT20" },
    { pack_plut, 1, 28, 227, "CEIL5_1",         "" },
    { pack_plut, 1, 28, 245, "FLAT5_1",         "" },
    { pack_plut, 1, 28, 248,        "",   "FLAT20" },
    { pack_plut, 1, 28, 287,        "",  "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map28[] =
{
    { pack_plut, 1, 28,   0, 100 },
    { pack_plut, 1, 28,  11, 100 },
    { pack_plut, 1, 28,  13, 100 },
    { pack_plut, 1, 28,  31, 100 },
    { pack_plut, 1, 28,  32, 100 },
    { pack_plut, 1, 28,  33, 100 },
    { pack_plut, 1, 28,  36, 100 },
    { pack_plut, 1, 28,  53, 100 },
    { pack_plut, 1, 28,  58, 100 },
    { pack_plut, 1, 28,  59, 100 },
    { pack_plut, 1, 28,  60, 100 },
    { pack_plut, 1, 28,  65, 100 },
    { pack_plut, 1, 28,  66, 100 },
    { pack_plut, 1, 28,  67, 100 },
    { pack_plut, 1, 28,  78, 100 },
    { pack_plut, 1, 28,  81, 100 },
    { pack_plut, 1, 28,  82, 100 },
    { pack_plut, 1, 28,  84, 216 },
    { pack_plut, 1, 28,  89, 100 },
    { pack_plut, 1, 28,  91, 216 },
    { pack_plut, 1, 28,  92, 100 },
    { pack_plut, 1, 28,  93, 100 },
    { pack_plut, 1, 28,  97, 100 },
    { pack_plut, 1, 28, 145, 100 },
    { pack_plut, 1, 28, 146, 100 },
    { pack_plut, 1, 28, 150, 216 },
    { pack_plut, 1, 28, 151, 216 },
    { pack_plut, 1, 28, 157, 216 },
    { pack_plut, 1, 28, 165, 100 },
    { pack_plut, 1, 28, 178, 100 },
    { pack_plut, 1, 28, 179, 100 },
    { pack_plut, 1, 28, 180, 100 },
    { pack_plut, 1, 28, 181, 100 },
    { pack_plut, 1, 28, 182, 100 },
    { pack_plut, 1, 28, 183, 100 },
    { pack_plut, 1, 28, 184, 100 },
    { pack_plut, 1, 28, 185, 100 },
    { pack_plut, 1, 28, 186, 100 },
    { pack_plut, 1, 28, 198, 220 },
    { pack_plut, 1, 28, 199, 220 },
    { pack_plut, 1, 28, 214, 100 },
    { pack_plut, 1, 28, 215, 100 },
    { pack_plut, 1, 28, 239, 220 },
    { pack_plut, 1, 28, 241, 220 },
    { pack_plut, 1, 28, 282, 100 },
    FLOW_END
};

static const fall_t fall_plut_map28[] =
{
    { pack_plut, 1, 28, 603, 102 },
    { pack_plut, 1, 28, 606, 102 },
    { pack_plut, 1, 28, 608, 102 },
    { pack_plut, 1, 28, 609, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 29: ODYSSEY OF NOISES
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map29[] =
{
    { pack_plut, 1, 29, 2842, 0, "A-BROCK2", "", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map29[] =
{
    { pack_plut, 1, 29,   5,        "",  "CEIL5_2" },
    { pack_plut, 1, 29,  10,        "",  "FLAT5_1" },
    { pack_plut, 1, 29,  81,        "",  "CEIL5_2" },
    { pack_plut, 1, 29,  82,        "",  "CEIL5_2" },
    { pack_plut, 1, 29, 111,        "", "FLOOR7_1" },
    { pack_plut, 1, 29, 128,        "", "FLOOR7_1" },
    { pack_plut, 1, 29, 129,        "", "FLOOR7_1" },
    { pack_plut, 1, 29, 160, "CEIL5_2",         "" },
    { pack_plut, 1, 29, 161,        "", "FLOOR7_1" },
    { pack_plut, 1, 29, 233,        "",  "RROCK09" },
    { pack_plut, 1, 29, 234,        "", "FLOOR7_1" },
    { pack_plut, 1, 29, 236, "RROCK09",         "" },
    { pack_plut, 1, 29, 243,        "", "FLOOR7_1" },
    { pack_plut, 1, 29, 246,        "", "FLOOR7_1" },
    { pack_plut, 1, 29, 274,        "",  "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map29[] =
{
    { pack_plut, 1, 29,  31, 100 },
    { pack_plut, 1, 29,  32, 100 },
    { pack_plut, 1, 29,  33, 100 },
    { pack_plut, 1, 29,  34, 100 },
    { pack_plut, 1, 29,  35, 100 },
    { pack_plut, 1, 29,  36, 100 },
    { pack_plut, 1, 29,  37, 100 },
    { pack_plut, 1, 29,  38, 100 },
    { pack_plut, 1, 29,  41, 101 },
    { pack_plut, 1, 29,  42, 100 },
    { pack_plut, 1, 29,  43, 100 },
    { pack_plut, 1, 29,  57, 101 },
    { pack_plut, 1, 29, 120, 100 },
    { pack_plut, 1, 29, 272, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 30: THE GATEWAY OF HELL
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map30[] =
{
    { pack_plut, 1, 30, 730, 0, "", "", "ROCKRED1", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map30[] =
{
    { pack_plut, 1, 30,  35,        "", "RROCK09" },
    { pack_plut, 1, 30, 127, "CEIL5_1", "CEIL5_1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map30[] =
{
    { pack_plut, 1, 30,  0, 100 },
    { pack_plut, 1, 30,  2, 100 },
    { pack_plut, 1, 30,  3, 100 },
    { pack_plut, 1, 30,  7, 100 },
    { pack_plut, 1, 30, 23, 221 },
    { pack_plut, 1, 30, 70, 220 },
    { pack_plut, 1, 30, 71, 220 },
    { pack_plut, 1, 30, 72, 220 },
    { pack_plut, 1, 30, 73, 220 },
    { pack_plut, 1, 30, 74, 220 },
    { pack_plut, 1, 30, 75, 220 },
    { pack_plut, 1, 30, 76, 220 },
    { pack_plut, 1, 30, 83, 100 },
    FLOW_END
};

static const fall_t fall_plut_map30[] =
{
    { pack_plut, 1, 30,   7, 102 },
    { pack_plut, 1, 30,  94, 102 },
    { pack_plut, 1, 30, 141, 102 },
    { pack_plut, 1, 30, 146, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 31: CYBERDEN
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map31[] =
{
    { pack_plut, 1, 31, 929, 0, "", "", "", DEFAULT, 54, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map31[] =
{
    { pack_plut, 1, 31,  36,        "", "CEIL5_2" },
    { pack_plut, 1, 31,  47, "CEIL5_2",        "" },
    { pack_plut, 1, 31, 113,        "",  "FLAT23" },
    { pack_plut, 1, 31, 146,        "", "CRATOP2" },
    { pack_plut, 1, 31, 150, "CEIL5_2",        "" },
    { pack_plut, 1, 31, 152,        "", "CRATOP2" },
    SECTORFIX_END
};

static const flow_t flow_plut_map31[] =
{
    { pack_plut, 1, 31,  62, 201 },
    { pack_plut, 1, 31, 154, 100 },
    FLOW_END
};

static const fall_t fall_plut_map31[] =
{
    { pack_plut, 1, 31, 1073, 101 },
    { pack_plut, 1, 31, 1074, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 32: GO 2 IT
// -----------------------------------------------------------------------------

static const linefix_t linefix_plut_map32[] =
{
    { pack_plut, 1, 32, 569, 0, "SUPPORT3", "", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 32, 570, 0, "SUPPORT3", "", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 32, 571, 0, "SUPPORT3", "", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 32, 572, 0, "SUPPORT3", "", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_plut, 1, 32, 805, 0, "A-BRICK3", "", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_plut_map32[] =
{
    { pack_plut, 1, 32,  43, "", "FLOOR7_1" },
    { pack_plut, 1, 32,  46, "", "FLOOR7_1" },
    { pack_plut, 1, 32,  69, "",  "RROCK10" },
    { pack_plut, 1, 32,  71, "",  "CEIL5_2" },
    { pack_plut, 1, 32, 118, "", "FLOOR7_1" },
    { pack_plut, 1, 32, 148, "", "FLOOR7_1" },
    { pack_plut, 1, 32, 149, "", "FLOOR7_1" },
    { pack_plut, 1, 32, 150, "", "FLOOR7_1" },
    { pack_plut, 1, 32, 152, "", "FLOOR7_1" },
    SECTORFIX_END
};

static const flow_t flow_plut_map32[] =
{
    { pack_plut, 1, 32, 73, 101 },
    { pack_plut, 1, 32, 74, 100 },
    FLOW_END
};


// =============================================================================
// FINAL DOOM: TNT - EVILUTION
// =============================================================================


// -----------------------------------------------------------------------------
// LEVEL 1: SYSTEM CONTROL
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map01[] =
{
    { pack_tnt, 1, 1,  12, 0,         "", "", "", 24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1,  19, 0,         "", "", "",  4, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1,  20, 0,         "", "", "", 32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1,  47, 0,         "", "", "",  9, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 109, 0,         "", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 111, 0,         "", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 123, 0,         "", "", "", 48, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 124, 0,         "", "", "", 16, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 132, 0,         "", "", "", 48, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 133, 0,         "", "", "", 16, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 222, 0,         "", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 226, 0,         "", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 348, 0,         "", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 393, 0,         "", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 406, 0, "EXITSIGN", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 408, 0, "EXITSIGN", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 534, 0, "EXITSIGN", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 536, 0, "EXITSIGN", "", "", -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 541, 0,         "", "", "", 12, DEFAULT, DEFAULT },
    { pack_tnt, 1, 1, 582, 0,         "", "", "", -7, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const flow_t flow_tnt_map01[] =
{
    { pack_tnt, 1, 1, 5, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 2: HUMAN BBQ
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map02[] =
{
    { pack_tnt, 1, 2, 359, 0, "", "", "", DEFAULT,      56, DEFAULT },
    { pack_tnt, 1, 2, 360, 0, "", "", "", DEFAULT,      56, DEFAULT },
    { pack_tnt, 1, 2, 361, 0, "", "", "", DEFAULT,      56, DEFAULT },
    { pack_tnt, 1, 2, 362, 0, "", "", "", DEFAULT,      56, DEFAULT },
    { pack_tnt, 1, 2, 594, 0, "", "", "",     -12,      21, DEFAULT },
    { pack_tnt, 1, 2, 598, 0, "", "", "",       0,     -21, DEFAULT },
    { pack_tnt, 1, 2, 667, 0, "", "", "", DEFAULT,     112, DEFAULT },
    { pack_tnt, 1, 2, 709, 0, "", "", "",       1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 2, 710, 0, "", "", "",       1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 2, 711, 0, "", "", "",       1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 2, 713, 0, "", "", "",       1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 2, 715, 0, "", "", "",       1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 2, 718, 0, "", "", "",       1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 2, 720, 0, "", "", "",       1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 2, 723, 0, "", "", "",      -4,      56, DEFAULT },
    { pack_tnt, 1, 2, 726, 0, "", "", "",     -68,      56, DEFAULT },
    { pack_tnt, 1, 2, 727, 0, "", "", "",    -132,      56, DEFAULT },
    { pack_tnt, 1, 2, 730, 0, "", "", "",       1, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map02[] =
{
    { pack_tnt, 1, 2,  64, "",  "CEIL5_1" },
    { pack_tnt, 1, 2,  77, "", "FLOOR7_2" },
    { pack_tnt, 1, 2,  78, "",   "DEM1_6" },
    { pack_tnt, 1, 2,  82, "",  "SLIME16" },
    { pack_tnt, 1, 2, 116, "", "FLOOR7_2" },
    { pack_tnt, 1, 2, 119, "",  "CEIL5_2" },
    { pack_tnt, 1, 2, 152, "", "FLOOR7_1" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map02[] =
{
    { pack_tnt, 1, 2, 97, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 3: POWER CONTROL
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map03[] =
{
    { pack_tnt, 1, 3,  880, 0, "", "TEKGREN2", "", DEFAULT, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3,  951, 0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 3,  952, 0, "",         "", "", DEFAULT, DEFAULT, ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 3, 1047, 0, "",         "", "",       0, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1121, 0, "", "TEKGREN2", "", DEFAULT, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1208, 0, "",         "", "",       4, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1210, 0, "",         "", "",       4, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1291, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1293, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1297, 0, "", "TEKGREN2", "", DEFAULT, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1298, 0, "",         "", "",     -32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1299, 0, "",         "", "",      16, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1300, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1301, 0, "",         "", "",     -32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1302, 0, "",         "", "",      16, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1304, 0, "", "TEKGREN5", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1310, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1314, 0, "", "TEKGREN2", "", DEFAULT, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1315, 0, "",         "", "",      16, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1316, 0, "",         "", "",     -32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1318, 0, "",         "", "",     -32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1319, 0, "",         "", "",      16, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1320, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1321, 0, "", "TEKGREN5", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1327, 0, "",         "", "",      32, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1341, 0, "",         "", "",      16, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1348, 0, "",         "", "",      48, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1354, 0, "",         "", "",      48, DEFAULT,          DEFAULT },
    { pack_tnt, 1, 3, 1364, 0, "",         "", "",      16, DEFAULT,          DEFAULT },
    LINEFIX_END
};

static const flow_t flow_tnt_map03[] =
{
    { pack_tnt, 1, 3, 256, 100 },
    { pack_tnt, 1, 3, 260, 101 },
    { pack_tnt, 1, 3, 262, 101 },
    { pack_tnt, 1, 3, 329, 101 },
    { pack_tnt, 1, 3, 330, 101 },
    { pack_tnt, 1, 3, 333, 101 },
    { pack_tnt, 1, 3, 336, 101 },
    { pack_tnt, 1, 3, 338, 100 },
    { pack_tnt, 1, 3, 340, 101 },
    { pack_tnt, 1, 3, 341, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 4: WORMHOLE
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map04[] =
{
    { pack_tnt, 1, 4,    0, 0, "", "", "",      -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,   37, 0, "", "", "",      24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  164, 0, "", "", "", DEFAULT,       4, DEFAULT },
    { pack_tnt, 1, 4,  226, 0, "", "", "",      24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  228, 0, "", "", "",      24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  565, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  566, 0, "", "", "",       9, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  570, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  572, 0, "", "", "",       9, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  574, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  575, 0, "", "", "",       9, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  628, 0, "", "", "",      24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  629, 0, "", "", "",     -24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  637, 0, "", "", "",     -24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  638, 0, "", "", "",      24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  762, 0, "", "", "",      24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  786, 0, "", "", "",      24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  838, 0, "", "", "", DEFAULT,       4, DEFAULT },
    { pack_tnt, 1, 4,  860, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  877, 0, "", "", "",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  879, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  880, 0, "", "", "",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  881, 0, "", "", "",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  882, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  885, 0, "", "", "",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4,  898, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4, 1104, 0, "", "", "",       9, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4, 1105, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4, 1161, 0, "", "", "",       9, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4, 1162, 0, "", "", "",       0, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4, 1169, 0, "", "", "",       9, DEFAULT, DEFAULT },
    { pack_tnt, 1, 4, 1170, 0, "", "", "",       0, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map04[] =
{
    { pack_tnt, 1, 4,  86, "CEIL5_1",        "" },
    { pack_tnt, 1, 4,  87, "CEIL5_1",        "" },
    { pack_tnt, 1, 4,  88, "CEIL5_1",        "" },
    { pack_tnt, 1, 4, 208, "CEIL5_2",        "" },
    { pack_tnt, 1, 4, 209, "CEIL5_2",        "" },
    { pack_tnt, 1, 4, 210, "CEIL5_2",        "" },
    { pack_tnt, 1, 4, 216,        "", "CEIL5_2" },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// LEVEL 5: HANGER
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map05[] =
{
    { pack_tnt, 1, 5, 604, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 5, 606, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 5, 607, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 5, 644, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 5, 645, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 5, 647, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 5, 840, 0, "", "", "",      24, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map05[] =
{
    { pack_tnt, 1, 5,   2, "",  "FLAT20" },
    { pack_tnt, 1, 5,  79, "", "CEIL5_2" },
    { pack_tnt, 1, 5,  90, "",  "FLAT19" },
    { pack_tnt, 1, 5, 105, "",  "FLAT19" },
    { pack_tnt, 1, 5, 120, "",   "FLAT1" },
    { pack_tnt, 1, 5, 158, "", "CEIL5_2" },
    { pack_tnt, 1, 5, 168, "", "CEIL5_1" },
    { pack_tnt, 1, 5, 170, "", "CEIL5_2" },
    { pack_tnt, 1, 5, 209, "", "CEIL5_2" },
    SECTORFIX_END
};

// -----------------------------------------------------------------------------
// LEVEL 6: OPEN SEASON
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map06[] =
{
    { pack_tnt, 1, 6,    7, 0, "", "", "", DEFAULT,   4, DEFAULT },
    { pack_tnt, 1, 6,   26, 0, "", "", "", DEFAULT,   4, DEFAULT },
    { pack_tnt, 1, 6,  760, 0, "", "", "", DEFAULT,   2, DEFAULT },
    { pack_tnt, 1, 6,  761, 0, "", "", "", DEFAULT,   2, DEFAULT },
    { pack_tnt, 1, 6,  786, 0, "", "", "", DEFAULT, -32, DEFAULT },
    { pack_tnt, 1, 6,  790, 0, "", "", "", DEFAULT, -32, DEFAULT },
    { pack_tnt, 1, 6, 1322, 1, "", "", "", DEFAULT, -14, DEFAULT },
    { pack_tnt, 1, 6, 1327, 1, "", "", "", DEFAULT, -14, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map06[] =
{
    { pack_tnt, 1, 6, 121, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map06[] =
{
    { pack_tnt, 1, 6, 88, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 7: PRISON
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map07[] =
{
    { pack_tnt, 1, 7,  58,       "",  "FLAT1" },
    { pack_tnt, 1, 7, 106,       "", "FLAT20" },
    { pack_tnt, 1, 7, 121,       "", "FLAT20" },
    { pack_tnt, 1, 7, 153,       "", "FLAT19" },
    { pack_tnt, 1, 7, 169, "FLAT19",       "" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map07[] =
{
    { pack_tnt, 1, 7,  21, 231 },
    { pack_tnt, 1, 7,  62, 231 },
    { pack_tnt, 1, 7,  84, 231 },
    { pack_tnt, 1, 7,  85, 231 },
    { pack_tnt, 1, 7, 127, 231 },
    { pack_tnt, 1, 7, 129, 231 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 8: METAL
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map08[] =
{
    { pack_tnt, 1, 8,  28, "",   "FLAT1" },
    { pack_tnt, 1, 8,  74, "",  "FLAT19" },
    { pack_tnt, 1, 8, 304, "", "CEIL5_2" },
    { pack_tnt, 1, 8, 419, "",  "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map08[] =
{
    { pack_tnt, 1, 8,   0, 100 },
    { pack_tnt, 1, 8,   1, 100 },
    { pack_tnt, 1, 8,   2, 100 },
    { pack_tnt, 1, 8,   3, 100 },
    { pack_tnt, 1, 8,   4, 100 },
    { pack_tnt, 1, 8,   5, 100 },
    { pack_tnt, 1, 8,   6, 100 },
    { pack_tnt, 1, 8,   7, 100 },
    { pack_tnt, 1, 8,   8, 100 },
    { pack_tnt, 1, 8,   9, 100 },
    { pack_tnt, 1, 8,  19, 100 },
    { pack_tnt, 1, 8,  20, 100 },
    { pack_tnt, 1, 8,  21, 100 },
    { pack_tnt, 1, 8,  22, 100 },
    { pack_tnt, 1, 8,  23, 100 },
    { pack_tnt, 1, 8,  24, 100 },
    { pack_tnt, 1, 8,  25, 100 },
    { pack_tnt, 1, 8,  26, 101 },
    { pack_tnt, 1, 8, 109, 100 },
    { pack_tnt, 1, 8, 110, 100 },
    { pack_tnt, 1, 8, 131, 102 },
    { pack_tnt, 1, 8, 132, 102 },
    { pack_tnt, 1, 8, 133, 102 },
    { pack_tnt, 1, 8, 161, 102 },
    { pack_tnt, 1, 8, 162, 102 },
    { pack_tnt, 1, 8, 163, 102 },
    { pack_tnt, 1, 8, 164, 102 },
    { pack_tnt, 1, 8, 210, 100 },
    { pack_tnt, 1, 8, 211, 100 },
    { pack_tnt, 1, 8, 212, 100 },
    { pack_tnt, 1, 8, 214, 100 },
    { pack_tnt, 1, 8, 215, 100 },
    { pack_tnt, 1, 8, 216, 100 },
    { pack_tnt, 1, 8, 218, 100 },
    { pack_tnt, 1, 8, 219, 100 },
    { pack_tnt, 1, 8, 220, 100 },
    { pack_tnt, 1, 8, 222, 100 },
    { pack_tnt, 1, 8, 223, 100 },
    { pack_tnt, 1, 8, 224, 100 },
    { pack_tnt, 1, 8, 251, 100 },
    { pack_tnt, 1, 8, 252, 100 },
    { pack_tnt, 1, 8, 261, 100 },
    { pack_tnt, 1, 8, 262, 100 },
    { pack_tnt, 1, 8, 263, 100 },
    { pack_tnt, 1, 8, 264, 100 },
    { pack_tnt, 1, 8, 265, 100 },
    { pack_tnt, 1, 8, 267, 101 },
    { pack_tnt, 1, 8, 268, 101 },
    { pack_tnt, 1, 8, 269, 101 },
    { pack_tnt, 1, 8, 270, 101 },
    { pack_tnt, 1, 8, 271, 101 },
    { pack_tnt, 1, 8, 272, 101 },
    { pack_tnt, 1, 8, 273, 101 },
    { pack_tnt, 1, 8, 274, 101 },
    { pack_tnt, 1, 8, 275, 100 },
    { pack_tnt, 1, 8, 302, 100 },
    { pack_tnt, 1, 8, 303, 100 },
    { pack_tnt, 1, 8, 304, 100 },
    { pack_tnt, 1, 8, 305, 100 },
    { pack_tnt, 1, 8, 334, 102 },
    { pack_tnt, 1, 8, 335, 102 },
    { pack_tnt, 1, 8, 347, 100 },
    { pack_tnt, 1, 8, 348, 100 },
    { pack_tnt, 1, 8, 349, 100 },
    { pack_tnt, 1, 8, 350, 100 },
    { pack_tnt, 1, 8, 351, 100 },
    { pack_tnt, 1, 8, 352, 100 },
    { pack_tnt, 1, 8, 353, 100 },
    { pack_tnt, 1, 8, 354, 100 },
    { pack_tnt, 1, 8, 355, 100 },
    { pack_tnt, 1, 8, 356, 100 },
    { pack_tnt, 1, 8, 357, 100 },
    { pack_tnt, 1, 8, 358, 100 },
    { pack_tnt, 1, 8, 359, 100 },
    { pack_tnt, 1, 8, 360, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 9: STRONGHOLD
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map09[] =
{
    { pack_tnt, 1, 9,  623, 0, "", "", "", 32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 9, 1235, 0, "", "", "",  4, DEFAULT, DEFAULT },
    { pack_tnt, 1, 9, 1240, 0, "", "", "",  4, DEFAULT, DEFAULT },
    { pack_tnt, 1, 9, 1282, 0, "", "", "",  4, DEFAULT, DEFAULT },
    { pack_tnt, 1, 9, 2098, 0, "", "", "",  4, DEFAULT, DEFAULT },
    { pack_tnt, 1, 9, 2100, 0, "", "", "",  4, DEFAULT, DEFAULT },
    { pack_tnt, 1, 9, 2163, 0, "", "", "",  8, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map09[] =
{
    { pack_tnt, 1, 9,   2, "",  "FLAT20" },
    { pack_tnt, 1, 9,   8, "", "CEIL5_1" },
    { pack_tnt, 1, 9,  17, "", "CEIL5_2" },
    { pack_tnt, 1, 9,  20, "",  "FLAT20" },
    { pack_tnt, 1, 9,  21, "",  "FLAT20" },
    { pack_tnt, 1, 9,  29, "", "CEIL5_1" },
    { pack_tnt, 1, 9,  43, "", "CRATOP2" },
    { pack_tnt, 1, 9, 107, "", "CEIL5_2" },
    { pack_tnt, 1, 9, 152, "",  "FLAT20" },
    { pack_tnt, 1, 9, 182, "", "CRATOP2" },
    { pack_tnt, 1, 9, 201, "", "CEIL5_2" },
    { pack_tnt, 1, 9, 274, "",  "FLAT20" },
    { pack_tnt, 1, 9, 277, "", "CEIL5_2" },
    { pack_tnt, 1, 9, 364, "",  "FLAT23" },
    { pack_tnt, 1, 9, 366, "",  "FLAT23" },
    { pack_tnt, 1, 9, 368, "",  "FLAT23" },
    { pack_tnt, 1, 9, 369, "",  "FLAT23" },
    { pack_tnt, 1, 9, 370, "",  "FLAT23" },
    { pack_tnt, 1, 9, 412, "",  "FLAT20" },
    { pack_tnt, 1, 9, 419, "",  "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map09[] =
{
    { pack_tnt, 1, 9, 264, 102 },
    { pack_tnt, 1, 9, 270, 102 },
    { pack_tnt, 1, 9, 271, 102 },
    FLOW_END
};

static const fall_t fall_tnt_map09[] =
{
    { pack_tnt, 1, 9, 1370, 101 },
    { pack_tnt, 1, 9, 1372, 101 },
    { pack_tnt, 1, 9, 1373, 101 },
    { pack_tnt, 1, 9, 1409, 102 },
    { pack_tnt, 1, 9, 1411, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 10: REDEMPTION
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map10[] =
{
    { pack_tnt, 1, 10,    9, 0,         "",      "", "",       8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,   13, 0,         "",      "", "",       8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,   22, 0,         "", "GRAY1", "",     -12, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,   72, 0,         "",      "", "",      -8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,  125, 0,         "",      "", "",      -8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,  183, 0,         "",      "", "",      16, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,  188, 0,         "",      "", "",     -16, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,  189, 0,         "",      "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_tnt, 1, 10,  192, 0,         "",      "", "",      -8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,  195, 0,         "",      "", "",      24, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,  202, 0,         "",      "", "",      -8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10,  986, 0,         "",      "", "", DEFAULT, DEFAULT, ML_DONTPEGTOP },
    { pack_tnt, 1, 10, 1080, 0, "EXITSIGN",      "", "",      -8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10, 1082, 0, "EXITSIGN",      "", "",      -8, DEFAULT,       DEFAULT },
    { pack_tnt, 1, 10, 1083, 0,         "",      "", "",       1, DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map10[] =
{
    { pack_tnt, 1, 10,   2, "",   "FLAT20" },
    { pack_tnt, 1, 10,   9, "",   "FLAT20" },
    { pack_tnt, 1, 10,  25, "",   "FLAT20" },
    { pack_tnt, 1, 10,  50, "",   "FLAT19" },
    { pack_tnt, 1, 10,  57, "",   "FLAT20" },
    { pack_tnt, 1, 10, 174, "", "FLOOR7_2" },
    { pack_tnt, 1, 10, 175, "", "FLOOR7_2" },
    { pack_tnt, 1, 10, 231, "",   "FLAT20" },
    { pack_tnt, 1, 10, 254, "",   "FLAT20" },
    { pack_tnt, 1, 10, 257, "",    "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map10[] =
{
    { pack_tnt, 1, 10, 143, 102 },
    { pack_tnt, 1, 10, 172, 222 },
    { pack_tnt, 1, 10, 173, 222 },
    FLOW_END
};

static const fall_t fall_tnt_map10[] =
{
    { pack_tnt, 1, 10, 680, 102 },
    { pack_tnt, 1, 10, 681, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 11: STORAGE FACILITY
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map11[] =
{
    { pack_tnt, 1, 11,  573, 0, "", "EXITDOOR", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11,  885, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11,  887, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11,  893, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11,  900, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1194, 0, "",         "", "",      24,     -80,                            DEFAULT },
    { pack_tnt, 1, 11, 1195, 0, "",         "", "",     -18,     -80,                            DEFAULT },
    { pack_tnt, 1, 11, 1219, 0, "",         "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { pack_tnt, 1, 11, 1228, 0, "",         "", "", DEFAULT, DEFAULT,                      ML_DONTPEGTOP },
    { pack_tnt, 1, 11, 1247, 0, "",         "", "",      32,      48,                            DEFAULT },
    { pack_tnt, 1, 11, 1250, 0, "",         "", "",      -8,      48,                            DEFAULT },
    { pack_tnt, 1, 11, 1251, 0, "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { pack_tnt, 1, 11, 1600, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1601, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1604, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1606, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1607, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1609, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1611, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1613, 0, "",         "", "",       4, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 11, 1615, 0, "",         "", "",       0,     -80,                            DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map11[] =
{
    { pack_tnt, 1, 11,  24,        "", "CEIL5_2" },
    { pack_tnt, 1, 11,  70,        "", "CEIL5_2" },
    { pack_tnt, 1, 11,  83,        "",  "FLAT20" },
    { pack_tnt, 1, 11, 173,        "",  "FLAT20" },
    { pack_tnt, 1, 11, 179, "FLAT5_4",        "" },
    { pack_tnt, 1, 11, 181, "CRATOP2",        "" },
    { pack_tnt, 1, 11, 196, "FLAT5_4",        "" },
    { pack_tnt, 1, 11, 197, "FLAT5_4",        "" },
    { pack_tnt, 1, 11, 210,        "",  "FLAT20" },
    { pack_tnt, 1, 11, 221,        "",  "FLAT20" },
    { pack_tnt, 1, 11, 228,        "",  "FLAT20" },
    { pack_tnt, 1, 11, 242, "CRATOP2",        "" },
    { pack_tnt, 1, 11, 244,        "", "CEIL5_2" },
    { pack_tnt, 1, 11, 249, "FLAT5_4",        "" },
    { pack_tnt, 1, 11, 250, "FLAT5_4",        "" },
    { pack_tnt, 1, 11, 258,  "FLAT19",        "" },
    { pack_tnt, 1, 11, 261, "CRATOP1",        "" },
    { pack_tnt, 1, 11, 262, "CRATOP1",        "" },
    { pack_tnt, 1, 11, 263, "CRATOP1",        "" },
    { pack_tnt, 1, 11, 265, "CRATOP1",        "" },
    { pack_tnt, 1, 11, 268,        "", "CEIL5_2" },
    { pack_tnt, 1, 11, 272,        "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map11[] =
{
    { pack_tnt, 1, 11, 180, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 12: CRATER
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map12[] =
{
    { pack_tnt, 1, 12,    4, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,   10, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,   11, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,   15, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,  136, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,  138, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,  149, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,  157, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  160, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  169, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  172, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  175, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  178, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  163, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  166, 0, "", "", "",       3, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  196, 0, "", "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 12,  469, 0, "", "", "",     -14, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  470, 0, "", "", "",     -10, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  473, 0, "", "", "",     -57, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  474, 0, "", "", "",     -31, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  475, 0, "", "", "",     -22, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  476, 0, "", "", "",      -2, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  482, 0, "", "", "",     -51, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  484, 0, "", "", "",     -22, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  486, 0, "", "", "",     -30, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  487, 0, "", "", "",     -58, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  489, 0, "", "", "",      -2, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  491, 0, "", "", "",      51, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  497, 0, "", "", "",     -44, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12,  721, 0, "", "", "",      39, DEFAULT, DEFAULT },
    { pack_tnt, 1, 12, 1285, 0, "", "", "",       0,    -120, DEFAULT },
    { pack_tnt, 1, 12, 1416, 0, "", "", "",      12, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map12[] =
{
    { pack_tnt, 1, 12,  12,        "", "FLAT20" },
    { pack_tnt, 1, 12,  86,        "", "FLAT20" },
    { pack_tnt, 1, 12, 141, "CRATOP1",       "" },
    { pack_tnt, 1, 12, 142, "CRATOP1",       "" },
    { pack_tnt, 1, 12, 143,        "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map12[] =
{
    { pack_tnt, 1, 12,  14, 200 },
    { pack_tnt, 1, 12,  16, 200 },
    { pack_tnt, 1, 12,  17, 200 },
    { pack_tnt, 1, 12,  18, 200 },
    { pack_tnt, 1, 12,  19, 200 },
    { pack_tnt, 1, 12,  20, 200 },
    { pack_tnt, 1, 12,  21, 200 },
    { pack_tnt, 1, 12,  22, 200 },
    { pack_tnt, 1, 12,  23, 200 },
    { pack_tnt, 1, 12,  24, 200 },
    { pack_tnt, 1, 12,  25, 200 },
    { pack_tnt, 1, 12,  26, 200 },
    { pack_tnt, 1, 12,  49, 200 },
    { pack_tnt, 1, 12,  50, 200 },
    { pack_tnt, 1, 12,  51, 200 },
    { pack_tnt, 1, 12,  52, 200 },
    { pack_tnt, 1, 12,  53, 200 },
    { pack_tnt, 1, 12,  54, 200 },
    { pack_tnt, 1, 12,  55, 200 },
    { pack_tnt, 1, 12,  56, 200 },
    { pack_tnt, 1, 12,  57, 200 },
    { pack_tnt, 1, 12,  58, 200 },
    { pack_tnt, 1, 12,  59, 200 },
    { pack_tnt, 1, 12,  60, 200 },
    { pack_tnt, 1, 12,  61, 200 },
    { pack_tnt, 1, 12,  62, 200 },
    { pack_tnt, 1, 12,  63, 200 },
    { pack_tnt, 1, 12,  64, 200 },
    { pack_tnt, 1, 12,  65, 200 },
    { pack_tnt, 1, 12,  66, 200 },
    { pack_tnt, 1, 12,  67, 200 },
    { pack_tnt, 1, 12,  69, 200 },
    { pack_tnt, 1, 12,  70, 200 },
    { pack_tnt, 1, 12,  71, 200 },
    { pack_tnt, 1, 12,  72, 200 },
    { pack_tnt, 1, 12,  73, 200 },
    { pack_tnt, 1, 12,  74, 200 },
    { pack_tnt, 1, 12,  75, 200 },
    { pack_tnt, 1, 12,  76, 200 },
    { pack_tnt, 1, 12,  77, 200 },
    { pack_tnt, 1, 12,  78, 200 },
    { pack_tnt, 1, 12,  79, 200 },
    { pack_tnt, 1, 12,  80, 200 },
    { pack_tnt, 1, 12, 110, 200 },
    { pack_tnt, 1, 12, 111, 200 },
    { pack_tnt, 1, 12, 112, 200 },
    { pack_tnt, 1, 12, 113, 200 },
    { pack_tnt, 1, 12, 114, 200 },
    { pack_tnt, 1, 12, 115, 200 },
    { pack_tnt, 1, 12, 116, 200 },
    { pack_tnt, 1, 12, 117, 200 },
    { pack_tnt, 1, 12, 118, 200 },
    { pack_tnt, 1, 12, 119, 200 },
    { pack_tnt, 1, 12, 120, 200 },
    { pack_tnt, 1, 12, 121, 200 },
    { pack_tnt, 1, 12, 122, 200 },
    { pack_tnt, 1, 12, 123, 200 },
    { pack_tnt, 1, 12, 124, 200 },
    { pack_tnt, 1, 12, 125, 200 },
    { pack_tnt, 1, 12, 126, 200 },
    { pack_tnt, 1, 12, 127, 200 },
    { pack_tnt, 1, 12, 128, 200 },
    { pack_tnt, 1, 12, 129, 200 },
    { pack_tnt, 1, 12, 130, 200 },
    { pack_tnt, 1, 12, 131, 200 },
    { pack_tnt, 1, 12, 134, 200 },
    { pack_tnt, 1, 12, 135, 200 },
    { pack_tnt, 1, 12, 136, 200 },
    { pack_tnt, 1, 12, 137, 200 },
    { pack_tnt, 1, 12, 138, 200 },
    { pack_tnt, 1, 12, 139, 200 },
    { pack_tnt, 1, 12, 273, 102 },
    FLOW_END
};

static const fall_t fall_tnt_map12[] =
{
    { pack_tnt, 1, 12, 1555, 103 },
    { pack_tnt, 1, 12, 1556, 103 },
    { pack_tnt, 1, 12, 1557, 103 },
    { pack_tnt, 1, 12, 1558, 103 },
    { pack_tnt, 1, 12, 1559, 103 },
    { pack_tnt, 1, 12, 1560, 103 },
    { pack_tnt, 1, 12, 1561, 103 },
    { pack_tnt, 1, 12, 1562, 103 },
    { pack_tnt, 1, 12, 1563, 103 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 13: NUKAGE PROCESSING
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map13[] =
{
    { pack_tnt, 1, 13, 1178, 0, "",      "", "", DEFAULT,      88, DEFAULT },
    { pack_tnt, 1, 13, 1181, 0, "",      "", "", DEFAULT,      88, DEFAULT },
    { pack_tnt, 1, 13, 1294, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 13, 1296, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 13, 1297, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 13, 1299, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 13, 1555, 0, "",      "", "", DEFAULT,      32, DEFAULT },
    { pack_tnt, 1, 13, 1557, 0, "",      "", "", DEFAULT,      32, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map13[] =
{
    { pack_tnt, 1, 13, 191, "", "FLAT1" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map13[] =
{
    { pack_tnt, 1, 13,  58, 102 },
    { pack_tnt, 1, 13,  59, 102 },
    { pack_tnt, 1, 13,  60, 102 },
    { pack_tnt, 1, 13,  61, 102 },
    { pack_tnt, 1, 13, 157, 100 },
    { pack_tnt, 1, 13, 182, 102 },
    { pack_tnt, 1, 13, 206, 100 },
    { pack_tnt, 1, 13, 207, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 14: STEEL WORKS
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map14[] =
{
    { pack_tnt, 1, 14,   1,        "", "FLAT1" },
    { pack_tnt, 1, 14, 143,        "", "FLAT1" },
    { pack_tnt, 1, 14, 163, "CEIL5_1",      "" },
    { pack_tnt, 1, 14, 175, "FLAT5_1",      "" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map14[] =
{
    {   pack_tnt, 1, 14,   3, 100 },
    {   pack_tnt, 1, 14,   4, 100 },
    {   pack_tnt, 1, 14,   7, 100 },
    {   pack_tnt, 1, 14,  14, 100 },
    {   pack_tnt, 1, 14,  19, 100 },
    {   pack_tnt, 1, 14,  27, 100 },
    {   pack_tnt, 1, 14,  28, 100 },
    {   pack_tnt, 1, 14,  29, 100 },
    {   pack_tnt, 1, 14,  30, 100 },
    {   pack_tnt, 1, 14,  31, 100 },
    {   pack_tnt, 1, 14,  33, 100 },
    {   pack_tnt, 1, 14,  34, 100 },
    {   pack_tnt, 1, 14,  35, 100 },
    {   pack_tnt, 1, 14,  36, 100 },
    {   pack_tnt, 1, 14,  37, 100 },
    {   pack_tnt, 1, 14,  41, 100 },
    {   pack_tnt, 1, 14,  42, 100 },
    {   pack_tnt, 1, 14,  47, 100 },
    {   pack_tnt, 1, 14,  48, 100 },
    {   pack_tnt, 1, 14,  57, 100 },
    {   pack_tnt, 1, 14,  59, 100 },
    {   pack_tnt, 1, 14,  60, 100 },
    {   pack_tnt, 1, 14,  61, 100 },
    {   pack_tnt, 1, 14,  71, 100 },
    {   pack_tnt, 1, 14,  72, 100 },
    {   pack_tnt, 1, 14,  73, 100 },
    {   pack_tnt, 1, 14,  74, 100 },
    {   pack_tnt, 1, 14,  75, 100 },
    {   pack_tnt, 1, 14,  76, 100 },
    {   pack_tnt, 1, 14,  77, 100 },
    {   pack_tnt, 1, 14,  81, 100 },
    {   pack_tnt, 1, 14,  82, 100 },
    {   pack_tnt, 1, 14,  85, 100 },
    {   pack_tnt, 1, 14,  87, 100 },
    {   pack_tnt, 1, 14,  99, 100 },
    {   pack_tnt, 1, 14, 102, 100 },
    {   pack_tnt, 1, 14, 104, 100 },
    {   pack_tnt, 1, 14, 106, 100 },
    {   pack_tnt, 1, 14, 150, 100 },
    {   pack_tnt, 1, 14, 167, 100 },
    {   pack_tnt, 1, 14, 184, 221 },
    {   pack_tnt, 1, 14, 186, 221 },
    {   pack_tnt, 1, 14, 188, 221 },
    {   pack_tnt, 1, 14, 190, 221 },
    {   pack_tnt, 1, 14, 192, 201 },
    {   pack_tnt, 1, 14, 194, 201 },
    {   pack_tnt, 1, 14, 196, 201 },
    {   pack_tnt, 1, 14, 198, 201 },
    {   pack_tnt, 1, 14, 200, 100 },
    {   pack_tnt, 1, 14, 207, 100 },
    {   pack_tnt, 1, 14, 219, 100 },
    {   pack_tnt, 1, 14, 221, 100 },
    {   pack_tnt, 1, 14, 311, 100 },
    {   pack_tnt, 1, 14, 312, 100 },
    FLOW_END
};

static const fall_t fall_tnt_map14[] =
{
    { pack_tnt, 1, 14, 155, 101 },
    { pack_tnt, 1, 14, 161, 101 },
    { pack_tnt, 1, 14, 173, 101 },
    { pack_tnt, 1, 14, 175, 101 },
    { pack_tnt, 1, 14, 179, 101 },
    { pack_tnt, 1, 14, 188, 101 },
    { pack_tnt, 1, 14, 193, 101 },
    { pack_tnt, 1, 14, 198, 101 },
    { pack_tnt, 1, 14, 209, 101 },
    { pack_tnt, 1, 14, 212, 101 },
    { pack_tnt, 1, 14, 215, 101 },
    { pack_tnt, 1, 14, 218, 101 },
    { pack_tnt, 1, 14, 229, 101 },
    { pack_tnt, 1, 14, 232, 101 },
    { pack_tnt, 1, 14, 244, 101 },
    { pack_tnt, 1, 14, 247, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 15: DEAD ZONE
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map15[] =
{
    { pack_tnt, 1, 15,  380, 0,       "", "",         "",       6,       3, DEFAULT },
    { pack_tnt, 1, 15,  503, 0,       "", "",         "",     -24, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15,  908, 0, "METAL7", "",   "METAL6",      12, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15,  909, 0, "METAL7", "",   "METAL6",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15,  910, 0, "METAL7", "",   "METAL6",      12, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15,  911, 0, "METAL7", "",   "METAL6",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15,  912, 0, "METAL7", "",   "METAL6",      12, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15, 1015, 0, "METAL7", "",   "METAL6",      12, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15, 1019, 0, "METAL7", "",   "METAL6",      12, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15, 1020, 0, "METAL7", "",   "METAL6",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15, 1176, 0, "METAL7", "",   "METAL6",      32, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15, 1345, 1,       "", "", "GSTVINE1", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15, 1386, 1,       "", "", "GSTVINE1", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 15, 1388, 1,       "", "", "GSTVINE1", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map15[] =
{
    { pack_tnt, 1, 15, 170,        "",  "FLAT1" },
    { pack_tnt, 1, 15, 180,        "", "FLAT19" },
    { pack_tnt, 1, 15, 287, "CEIL5_2",       "" },
    { pack_tnt, 1, 15, 306,  "FLAT19",       "" },
    { pack_tnt, 1, 15, 307,  "FLAT19",       "" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map15[] =
{
    { pack_tnt, 1, 15, 125, 100 },
    { pack_tnt, 1, 15, 129, 100 },
    { pack_tnt, 1, 15, 130, 100 },
    { pack_tnt, 1, 15, 131, 100 },
    { pack_tnt, 1, 15, 132, 100 },
    { pack_tnt, 1, 15, 229, 101 },
    { pack_tnt, 1, 15, 292, 100 },
    { pack_tnt, 1, 15, 293, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 16: DEEPEST REACHES
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map16[] =
{
    { pack_tnt, 1, 16,   5, "",  "CEIL5_2" },
    { pack_tnt, 1, 16,  41, "",   "FLAT19" },
    { pack_tnt, 1, 16,  44, "",   "FLAT19" },
    { pack_tnt, 1, 16,  74, "",  "CEIL5_2" },
    { pack_tnt, 1, 16, 150, "",  "CEIL5_2" },
    { pack_tnt, 1, 16, 273, "", "FLOOR7_2" },
    { pack_tnt, 1, 16, 329, "",  "FLAT5_5" },
    { pack_tnt, 1, 16, 295, "",   "FLAT19" },
    { pack_tnt, 1, 16, 323, "", "FLOOR7_2" },
    { pack_tnt, 1, 16, 352, "",   "FLAT19" },
    { pack_tnt, 1, 16, 360, "",   "FLAT19" },
    { pack_tnt, 1, 16, 364, "",   "FLAT19" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map16[] =
{
    { pack_tnt, 1, 16,  51, 224 },
    { pack_tnt, 1, 16,  52, 224 },
    { pack_tnt, 1, 16,  59, 224 },
    { pack_tnt, 1, 16,  60, 224 },
    { pack_tnt, 1, 16,  61, 224 },
    { pack_tnt, 1, 16,  62, 224 },
    { pack_tnt, 1, 16,  63, 224 },
    { pack_tnt, 1, 16,  64, 224 },
    { pack_tnt, 1, 16,  65, 224 },
    { pack_tnt, 1, 16,  66, 224 },
    { pack_tnt, 1, 16,  67, 224 },
    { pack_tnt, 1, 16,  68, 224 },
    { pack_tnt, 1, 16,  71, 224 },
    { pack_tnt, 1, 16,  76, 224 },
    { pack_tnt, 1, 16,  77, 224 },
    { pack_tnt, 1, 16,  78, 224 },
    { pack_tnt, 1, 16,  79, 224 },
    { pack_tnt, 1, 16,  80, 224 },
    { pack_tnt, 1, 16,  81, 224 },
    { pack_tnt, 1, 16,  82, 224 },
    { pack_tnt, 1, 16,  83, 224 },
    { pack_tnt, 1, 16,  84, 224 },
    { pack_tnt, 1, 16,  85, 224 },
    { pack_tnt, 1, 16,  86, 224 },
    { pack_tnt, 1, 16,  87, 224 },
    { pack_tnt, 1, 16,  88, 224 },
    { pack_tnt, 1, 16,  89, 224 },
    { pack_tnt, 1, 16, 168, 224 },
    { pack_tnt, 1, 16, 178, 102 },
    { pack_tnt, 1, 16, 200, 102 },
    { pack_tnt, 1, 16, 202, 218 },
    { pack_tnt, 1, 16, 203, 218 },
    { pack_tnt, 1, 16, 204, 102 },
    { pack_tnt, 1, 16, 205, 213 },
    { pack_tnt, 1, 16, 207, 213 },
    { pack_tnt, 1, 16, 208, 213 },
    { pack_tnt, 1, 16, 209, 213 },
    { pack_tnt, 1, 16, 210, 223 },
    { pack_tnt, 1, 16, 230, 214 },
    { pack_tnt, 1, 16, 231, 214 },
    { pack_tnt, 1, 16, 232, 214 },
    { pack_tnt, 1, 16, 233, 214 },
    { pack_tnt, 1, 16, 234, 214 },
    { pack_tnt, 1, 16, 235, 214 },
    { pack_tnt, 1, 16, 236, 214 },
    { pack_tnt, 1, 16, 237, 214 },
    { pack_tnt, 1, 16, 238, 214 },
    { pack_tnt, 1, 16, 239, 219 },
    { pack_tnt, 1, 16, 240, 219 },
    { pack_tnt, 1, 16, 241, 214 },
    { pack_tnt, 1, 16, 242, 219 },
    { pack_tnt, 1, 16, 243, 219 },
    { pack_tnt, 1, 16, 244, 214 },
    { pack_tnt, 1, 16, 245, 214 },
    { pack_tnt, 1, 16, 246, 214 },
    { pack_tnt, 1, 16, 247, 214 },
    { pack_tnt, 1, 16, 248, 209 },
    { pack_tnt, 1, 16, 249, 209 },
    { pack_tnt, 1, 16, 250, 214 },
    { pack_tnt, 1, 16, 251, 212 },
    { pack_tnt, 1, 16, 252, 214 },
    { pack_tnt, 1, 16, 265, 212 },
    { pack_tnt, 1, 16, 266, 214 },
    { pack_tnt, 1, 16, 267, 212 },
    { pack_tnt, 1, 16, 271, 213 },
    { pack_tnt, 1, 16, 272, 213 },
    { pack_tnt, 1, 16, 288, 100 },
    { pack_tnt, 1, 16, 293, 202 },
    { pack_tnt, 1, 16, 322, 100 },
    { pack_tnt, 1, 16, 356, 212 },
    { pack_tnt, 1, 16, 358, 102 },
    { pack_tnt, 1, 16, 359, 102 },
    FLOW_END
};

static const fall_t fall_tnt_map16[] =
{
    { pack_tnt, 1, 16,  302, 103 },
    { pack_tnt, 1, 16,  527, 103 },
    { pack_tnt, 1, 16,  599, 105 },
    { pack_tnt, 1, 16,  600, 105 },
    { pack_tnt, 1, 16,  601, 105 },
    { pack_tnt, 1, 16,  603, 105 },
    { pack_tnt, 1, 16,  604, 105 },
    { pack_tnt, 1, 16,  607, 105 },
    { pack_tnt, 1, 16,  608, 105 },
    { pack_tnt, 1, 16,  667, 105 },
    { pack_tnt, 1, 16,  669, 100 },
    { pack_tnt, 1, 16,  670, 100 },
    { pack_tnt, 1, 16,  671, 100 },
    { pack_tnt, 1, 16,  672, 100 },
    { pack_tnt, 1, 16,  685, 100 },
    { pack_tnt, 1, 16,  687, 100 },
    { pack_tnt, 1, 16,  897, 100 },
    { pack_tnt, 1, 16,  898, 100 },
    { pack_tnt, 1, 16,  899, 100 },
    { pack_tnt, 1, 16,  900, 100 },
    { pack_tnt, 1, 16, 1238, 103 },
    { pack_tnt, 1, 16, 1247, 103 },
    { pack_tnt, 1, 16, 1248, 103 },
    { pack_tnt, 1, 16, 1364, 102 },
    { pack_tnt, 1, 16, 1373, 103 },
    { pack_tnt, 1, 16, 1407, 103 },
    { pack_tnt, 1, 16, 1408, 103 },
    { pack_tnt, 1, 16, 1409, 103 },
    { pack_tnt, 1, 16, 1412, 103 },
    { pack_tnt, 1, 16, 1413, 103 },
    { pack_tnt, 1, 16, 1416, 103 },
    { pack_tnt, 1, 16, 1417, 103 },
    { pack_tnt, 1, 16, 1418, 102 },
    { pack_tnt, 1, 16, 1439, 102 },
    { pack_tnt, 1, 16, 1443, 102 },
    { pack_tnt, 1, 16, 1444, 102 },
    { pack_tnt, 1, 16, 1447, 102 },
    { pack_tnt, 1, 16, 1456, 103 },
    { pack_tnt, 1, 16, 1457, 103 },
    { pack_tnt, 1, 16, 1458, 103 },
    { pack_tnt, 1, 16, 1459, 103 },
    { pack_tnt, 1, 16, 1468, 103 },
    { pack_tnt, 1, 16, 1525, 102 },
    { pack_tnt, 1, 16, 1528, 102 },
    { pack_tnt, 1, 16, 1532, 102 },
    { pack_tnt, 1, 16, 1540, 102 },
    { pack_tnt, 1, 16, 1546, 102 },
    { pack_tnt, 1, 16, 1549, 102 },
    { pack_tnt, 1, 16, 1553, 102 },
    { pack_tnt, 1, 16, 1558, 102 },
    { pack_tnt, 1, 16, 1561, 102 },
    { pack_tnt, 1, 16, 1564, 102 },
    { pack_tnt, 1, 16, 1567, 102 },
    { pack_tnt, 1, 16, 1572, 102 },
    { pack_tnt, 1, 16, 1575, 102 },
    { pack_tnt, 1, 16, 1578, 102 },
    { pack_tnt, 1, 16, 1581, 102 },
    { pack_tnt, 1, 16, 1584, 102 },
    { pack_tnt, 1, 16, 1587, 102 },
    { pack_tnt, 1, 16, 1590, 102 },
    { pack_tnt, 1, 16, 1593, 102 },
    { pack_tnt, 1, 16, 1596, 102 },
    { pack_tnt, 1, 16, 1599, 103 },
    { pack_tnt, 1, 16, 1602, 102 },
    { pack_tnt, 1, 16, 1696, 102 },
    { pack_tnt, 1, 16, 1706, 104 },
    { pack_tnt, 1, 16, 1825, 101 },
    { pack_tnt, 1, 16, 1896, 105 },
    { pack_tnt, 1, 16, 1897, 105 },
    { pack_tnt, 1, 16, 2060, 101 },
    { pack_tnt, 1, 16, 2073, 101 },
    { pack_tnt, 1, 16, 2128, 102 },
    { pack_tnt, 1, 16, 2129, 102 },
    { pack_tnt, 1, 16, 2130, 102 },
    { pack_tnt, 1, 16, 2131, 103 },
    { pack_tnt, 1, 16, 2132, 103 },
    { pack_tnt, 1, 16, 2133, 103 },
    { pack_tnt, 1, 16, 2134, 103 },
    { pack_tnt, 1, 16, 2135, 102 },
    { pack_tnt, 1, 16, 2136, 102 },
    { pack_tnt, 1, 16, 2137, 102 },
    { pack_tnt, 1, 16, 2138, 102 },
    { pack_tnt, 1, 16, 2144, 102 },
    { pack_tnt, 1, 16, 2145, 102 },
    { pack_tnt, 1, 16, 2146, 102 },
    { pack_tnt, 1, 16, 2147, 102 },
    { pack_tnt, 1, 16, 2148, 102 },
    { pack_tnt, 1, 16, 2149, 102 },
    { pack_tnt, 1, 16, 2150, 102 },
    { pack_tnt, 1, 16, 2151, 102 },
    { pack_tnt, 1, 16, 2152, 102 },
    { pack_tnt, 1, 16, 2153, 102 },
    { pack_tnt, 1, 16, 2154, 102 },
    { pack_tnt, 1, 16, 2155, 102 },
    { pack_tnt, 1, 16, 2156, 102 },
    { pack_tnt, 1, 16, 2157, 102 },
    { pack_tnt, 1, 16, 2158, 102 },
    { pack_tnt, 1, 16, 2159, 102 },
    { pack_tnt, 1, 16, 2160, 102 },
    { pack_tnt, 1, 16, 2161, 102 },
    { pack_tnt, 1, 16, 2162, 102 },
    { pack_tnt, 1, 16, 2163, 102 },
    { pack_tnt, 1, 16, 2164, 102 },
    { pack_tnt, 1, 16, 2165, 102 },
    { pack_tnt, 1, 16, 2166, 102 },
    { pack_tnt, 1, 16, 2167, 102 },
    { pack_tnt, 1, 16, 2307, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 17: PROCESSING AREA
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map17[] =
{
    { pack_tnt, 1, 17,  184, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  187, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  203, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  204, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  502, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  505, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  532, 0,       "", "DOORYEL", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  533, 0,       "", "DOORYEL", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  586, 0,       "",        "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 17,  587, 0,       "",        "", "", DEFAULT,      -1, DEFAULT },
    { pack_tnt, 1, 17,  899, 0, "STONE2",        "", "",     -41, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  937, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17,  939, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17, 1106, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 17, 1108, 0,       "",   "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map17[] =
{
    { pack_tnt, 1, 17, 120, "",  "FLAT20" },
    { pack_tnt, 1, 17, 127, "", "CEIL5_2" },
    { pack_tnt, 1, 17, 143, "",  "FLAT20" },
    { pack_tnt, 1, 17, 171, "",  "FLAT20" },
    { pack_tnt, 1, 17, 265, "",  "FLAT20" },
    { pack_tnt, 1, 17, 271, "",  "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map17[] =
{
    { pack_tnt, 1, 17,  31, 100 },
    { pack_tnt, 1, 17,  32, 100 },
    { pack_tnt, 1, 17,  33, 100 },
    { pack_tnt, 1, 17, 124, 101 },
    { pack_tnt, 1, 17, 129, 101 },
    { pack_tnt, 1, 17, 131, 101 },
    { pack_tnt, 1, 17, 164, 212 },
    { pack_tnt, 1, 17, 204, 212 },
    { pack_tnt, 1, 17, 263, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 18: MILL
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map18[] =
{
    { pack_tnt, 1, 18,  45,       "",  "FLAT20" },
    { pack_tnt, 1, 18,  64,       "", "CEIL5_1" },
    { pack_tnt, 1, 18, 112,       "",  "FLAT20" },
    { pack_tnt, 1, 18, 114,       "",  "FLAT20" },
    { pack_tnt, 1, 18, 131,       "",   "FLAT1" },
    { pack_tnt, 1, 18, 109,       "",  "FLAT20" },
    { pack_tnt, 1, 18, 134,       "", "CEIL5_2" },
    { pack_tnt, 1, 18, 146,       "", "CEIL5_2" },
    { pack_tnt, 1, 18, 179,       "",  "FLAT20" },
    { pack_tnt, 1, 18, 180,       "",  "FLAT20" },
    { pack_tnt, 1, 18, 212,       "", "CEIL5_2" },
    { pack_tnt, 1, 18, 241, "FLAT23",        "" },
    { pack_tnt, 1, 18, 327,       "",  "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map18[] =
{
    { pack_tnt, 1, 18, 330, 101 },
    { pack_tnt, 1, 18, 352, 101 },
    { pack_tnt, 1, 18, 353, 101 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 19: SHIPPING/RESPAWNING
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map19[] =
{
    { pack_tnt, 1, 19,  614,  0, "", "", "", DEFAULT,        -9,       DEFAULT },
    { pack_tnt, 1, 19,  615,  0, "", "", "", DEFAULT,        -9,       DEFAULT },
    { pack_tnt, 1, 19,  617,  0, "", "", "", DEFAULT,        -9,       DEFAULT },
    { pack_tnt, 1, 19,  618,  0, "", "", "", DEFAULT,        -9,       DEFAULT },
    { pack_tnt, 1, 19,  620,  0, "", "", "",      46,       -25,       DEFAULT },
    { pack_tnt, 1, 19,  623,  0, "", "", "",      48,       -25,       DEFAULT },
    { pack_tnt, 1, 19,  625,  0, "", "", "", DEFAULT,        -9,       DEFAULT },
    { pack_tnt, 1, 19,  626,  0, "", "", "",     -35,       -25,       DEFAULT },
    { pack_tnt, 1, 19,  628,  0, "", "", "", DEFAULT,   DEFAULT, ML_DONTPEGTOP },
    { pack_tnt, 1, 19,  629,  0, "", "", "",     -32,       -25,       DEFAULT },
    { pack_tnt, 1, 19,  630,  0, "", "", "", DEFAULT,        -9,       DEFAULT },
    { pack_tnt, 1, 19, 1700,  0, "", "", "",     201,   DEFAULT,       DEFAULT },
    { pack_tnt, 1, 19, 1760,  0, "", "", "", DEFAULT,   DEFAULT, ML_DONTPEGTOP },
    { pack_tnt, 1, 19, 1765,  0, "", "", "", DEFAULT,   DEFAULT, ML_DONTPEGTOP },
    { pack_tnt, 1, 19, 1839,  0, "", "", "",      25,   DEFAULT,       DEFAULT },
    { pack_tnt, 1, 19, 2125,  0, "", "", "",       8,   DEFAULT,       DEFAULT },
    { pack_tnt, 1, 19, 2207,  0, "", "", "",     -24,   DEFAULT,       DEFAULT },
    { pack_tnt, 1, 19, 2240,  0, "", "", "",      13,   DEFAULT,       DEFAULT },
    { pack_tnt, 1, 19, 2241,  0, "", "", "",      13,   DEFAULT,       DEFAULT },
    { pack_tnt, 1, 19, 2250,  0, "", "", "", DEFAULT,   DEFAULT, ML_DONTPEGTOP },
    { pack_tnt, 1, 19, 2362,  0, "", "", "",      26,   DEFAULT,       DEFAULT },
    { pack_tnt, 1, 19, 2377,  0, "", "", "",      26,   DEFAULT,       DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map19[] =
{
    { pack_tnt, 1, 19,  82, "CRATOP1",        "" },
    { pack_tnt, 1, 19,  84, "CRATOP1",        "" },
    { pack_tnt, 1, 19, 118,        "",  "FLAT20" },
    { pack_tnt, 1, 19, 212,        "",  "FLAT20" },
    { pack_tnt, 1, 19, 362, "CEIL5_1",        "" },
    { pack_tnt, 1, 19, 451,        "" , "FLAT20" },
    { pack_tnt, 1, 19, 446,        "" , "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map19[] =
{
    { pack_tnt, 1, 19, 121, 231 },
    { pack_tnt, 1, 19, 122, 231 },
    { pack_tnt, 1, 19, 124, 231 },
    { pack_tnt, 1, 19, 123, 231 },
    { pack_tnt, 1, 19, 452, 231 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 20: CENTRAL PROCESSING
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map20[] =
{
    { pack_tnt, 1, 20, 1426, 0, "", "", "",  36, DEFAULT, DEFAULT },
    { pack_tnt, 1, 20, 1510, 0, "", "", "",  16, DEFAULT, DEFAULT },
    { pack_tnt, 1, 20, 1511, 0, "", "", "",  48, DEFAULT, DEFAULT },
    { pack_tnt, 1, 20, 1512, 0, "", "", "",  16, DEFAULT, DEFAULT },
    { pack_tnt, 1, 20, 1522, 0, "", "", "", -13, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map20[] =
{
    { pack_tnt, 1, 20, 146, "", "FLOOR7_1" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map20[] =
{
    { pack_tnt, 1, 20,  69, 101 },
    { pack_tnt, 1, 20, 185, 100 },
    { pack_tnt, 1, 20, 103, 102 },
    { pack_tnt, 1, 20, 104, 102 },
    { pack_tnt, 1, 20, 105, 102 },
    { pack_tnt, 1, 20, 156, 100 },
    { pack_tnt, 1, 20, 158, 100 },
    { pack_tnt, 1, 20, 159, 211 },
    { pack_tnt, 1, 20, 160, 231 },
    { pack_tnt, 1, 20, 161, 226 },
    { pack_tnt, 1, 20, 162, 221 },
    { pack_tnt, 1, 20, 163, 211 },
    { pack_tnt, 1, 20, 168, 101 },
    { pack_tnt, 1, 20, 171, 101 },
    { pack_tnt, 1, 20, 195, 102 },
    { pack_tnt, 1, 20, 242, 102 },
    { pack_tnt, 1, 20, 243, 102 },
    { pack_tnt, 1, 20, 244, 100 },
    { pack_tnt, 1, 20, 245, 102 },
    { pack_tnt, 1, 20, 246, 102 },
    { pack_tnt, 1, 20, 247, 102 },
    { pack_tnt, 1, 20, 251, 102 },
    { pack_tnt, 1, 20, 280, 102 },
    { pack_tnt, 1, 20, 281, 101 },
    { pack_tnt, 1, 20, 285, 101 },
    { pack_tnt, 1, 20, 287, 100 },
    { pack_tnt, 1, 20, 288, 100 },
    { pack_tnt, 1, 20, 289, 100 },
    { pack_tnt, 1, 20, 290, 100 },
    { pack_tnt, 1, 20, 300, 100 },
    FLOW_END
};

static const fall_t fall_tnt_map20[] =
{
    { pack_tnt, 1, 20, 1254, 102 },
    { pack_tnt, 1, 20, 2159, 101 },
    { pack_tnt, 1, 20, 2160, 101 },
    { pack_tnt, 1, 20, 2163, 101 },
    { pack_tnt, 1, 20, 2164, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 21: ADMINISTRATION CENTER
// -----------------------------------------------------------------------------

static const flow_t flow_tnt_map21[] =
{
    { pack_tnt, 1, 21,   0, 102 },
    { pack_tnt, 1, 21,   2, 102 },
    { pack_tnt, 1, 21,  49, 100 },
    { pack_tnt, 1, 21,  58, 100 },
    { pack_tnt, 1, 21,  71, 100 },
    { pack_tnt, 1, 21, 151, 100 },
    { pack_tnt, 1, 21, 152, 100 },
    { pack_tnt, 1, 21, 175, 100 },
    { pack_tnt, 1, 21, 176, 100 },
    { pack_tnt, 1, 21, 177, 100 },
    { pack_tnt, 1, 21, 178, 100 },
    { pack_tnt, 1, 21, 179, 100 },
    { pack_tnt, 1, 21, 236, 231 },
    { pack_tnt, 1, 21, 249, 231 },
    { pack_tnt, 1, 21, 250, 231 },
    { pack_tnt, 1, 21, 251, 231 },
    { pack_tnt, 1, 21, 339, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 22: HABITAT
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map22[] =
{
    { pack_tnt, 1, 22, 889, 0, "", "", "", 8, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map22[] =
{
    { pack_tnt, 1, 22, 373, "", "FLOOR3_3" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map22[] =
{
    { pack_tnt, 1, 22,   0, 212 },
    { pack_tnt, 1, 22,   1, 212 },
    { pack_tnt, 1, 22,   2, 212 },
    { pack_tnt, 1, 22,   3, 101 },
    { pack_tnt, 1, 22,   4, 212 },
    { pack_tnt, 1, 22,   5, 101 },
    { pack_tnt, 1, 22,   6, 212 },
    { pack_tnt, 1, 22,   7, 101 },
    { pack_tnt, 1, 22,   8, 212 },
    { pack_tnt, 1, 22,   9, 212 },
    { pack_tnt, 1, 22,  10, 212 },
    { pack_tnt, 1, 22,  11, 212 },
    { pack_tnt, 1, 22,  12, 101 },
    { pack_tnt, 1, 22,  13, 212 },
    { pack_tnt, 1, 22,  14, 101 },
    { pack_tnt, 1, 22,  17, 101 },
    { pack_tnt, 1, 22,  18, 101 },
    { pack_tnt, 1, 22,  19, 101 },
    { pack_tnt, 1, 22,  20, 101 },
    { pack_tnt, 1, 22,  21, 212 },
    { pack_tnt, 1, 22,  22, 212 },
    { pack_tnt, 1, 22,  26, 212 },
    { pack_tnt, 1, 22,  27, 212 },
    { pack_tnt, 1, 22,  28, 212 },
    { pack_tnt, 1, 22,  29, 101 },
    { pack_tnt, 1, 22,  30, 101 },
    { pack_tnt, 1, 22,  44, 100 },
    { pack_tnt, 1, 22,  49, 100 },
    { pack_tnt, 1, 22,  50, 100 },
    { pack_tnt, 1, 22,  68, 100 },
    { pack_tnt, 1, 22,  72, 100 },
    { pack_tnt, 1, 22,  74, 100 },
    { pack_tnt, 1, 22,  75, 100 },
    { pack_tnt, 1, 22, 123, 100 },
    { pack_tnt, 1, 22, 134, 100 },
    { pack_tnt, 1, 22, 136, 100 },
    { pack_tnt, 1, 22, 137, 100 },
    { pack_tnt, 1, 22, 159, 100 },
    { pack_tnt, 1, 22, 160, 100 },
    { pack_tnt, 1, 22, 161, 100 },
    { pack_tnt, 1, 22, 163, 212 },
    { pack_tnt, 1, 22, 164, 212 },
    { pack_tnt, 1, 22, 165, 212 },
    { pack_tnt, 1, 22, 166, 212 },
    { pack_tnt, 1, 22, 177, 100 },
    { pack_tnt, 1, 22, 189, 100 },
    { pack_tnt, 1, 22, 190, 100 },
    { pack_tnt, 1, 22, 191, 100 },
    { pack_tnt, 1, 22, 192, 100 },
    { pack_tnt, 1, 22, 193, 100 },
    { pack_tnt, 1, 22, 269, 101 },
    { pack_tnt, 1, 22, 270, 101 },
    { pack_tnt, 1, 22, 271, 101 },
    { pack_tnt, 1, 22, 272, 101 },
    { pack_tnt, 1, 22, 305, 100 },
    { pack_tnt, 1, 22, 319, 101 },
    { pack_tnt, 1, 22, 320, 101 },
    { pack_tnt, 1, 22, 321, 101 },
    { pack_tnt, 1, 22, 327, 100 },
    { pack_tnt, 1, 22, 331, 221 },
    { pack_tnt, 1, 22, 332, 221 },
    { pack_tnt, 1, 22, 351, 220 },
    { pack_tnt, 1, 22, 352, 220 },
    { pack_tnt, 1, 22, 353, 220 },
    { pack_tnt, 1, 22, 355, 220 },
    { pack_tnt, 1, 22, 357, 220 },
    { pack_tnt, 1, 22, 359, 220 },
    { pack_tnt, 1, 22, 374, 100 },
    { pack_tnt, 1, 22, 376, 100 },
    { pack_tnt, 1, 22, 378, 100 },
    { pack_tnt, 1, 22, 384, 101 },
    FLOW_END
};

static const fall_t fall_tnt_map22[] =
{
    { pack_tnt, 1, 22,   77, 102 },
    { pack_tnt, 1, 22, 1539, 101 },
    { pack_tnt, 1, 22, 1727, 101 },
    { pack_tnt, 1, 22, 1728, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 23: LUNAR MINING PROJECT
// -----------------------------------------------------------------------------

static const flow_t flow_tnt_map23[] =
{
    { pack_tnt, 1, 23, 163, 100 },
    { pack_tnt, 1, 23, 164, 100 },
    { pack_tnt, 1, 23, 186, 216 },
    { pack_tnt, 1, 23, 187, 100 },
    { pack_tnt, 1, 23, 188, 216 },
    { pack_tnt, 1, 23, 195, 216 },
    { pack_tnt, 1, 23, 229, 101 },
    FLOW_END
};

static const fall_t fall_tnt_map23[] =
{
    { pack_tnt, 1, 23, 1184, 102 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 24: QUARRY
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map24[] =
{
    { pack_tnt, 1, 24, 19, "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map24[] =
{
    {   pack_tnt, 1, 24,   0, 100 },
    {   pack_tnt, 1, 24,   2, 100 },
    {   pack_tnt, 1, 24,  37, 100 },
    {   pack_tnt, 1, 24, 108, 201 },
    {   pack_tnt, 1, 24, 109, 201 },
    {   pack_tnt, 1, 24, 111, 226 },
    {   pack_tnt, 1, 24, 113, 100 },
    {   pack_tnt, 1, 24, 123, 100 },
    {   pack_tnt, 1, 24, 124, 231 },
    {   pack_tnt, 1, 24, 125, 100 },
    {   pack_tnt, 1, 24, 136, 100 },
    {   pack_tnt, 1, 24, 131, 231 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 25: BARON'S DEN
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map25[] =
{
    { pack_tnt, 1, 25,  633, 0, "",      "", "",      -2, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25,  635, 0, "",      "", "",      -1, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25,  695, 0, "",      "", "",      -2, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1012, 0, "",      "", "",      13, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1081, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1084, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1095, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1097, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1101, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1103, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1115, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1116, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1117, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1119, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1121, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1123, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1129, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1131, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1135, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1137, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1150, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1152, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1154, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1156, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1158, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1159, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1161, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1163, 0, "", "LITE5", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1342, 0, "",      "", "",      -8, DEFAULT, DEFAULT },
    { pack_tnt, 1, 25, 1442, 0, "",      "", "",       1, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map25[] =
{
    { pack_tnt, 1, 25,  39, "",  "CEIL5_2" },
    { pack_tnt, 1, 25,  98, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 118, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 126, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 144, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 169, "",  "FLAT5_5" },
    { pack_tnt, 1, 25, 193, "",  "RROCK09" },
    { pack_tnt, 1, 25, 237, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 273, "", "FLOOR7_2" },
    { pack_tnt, 1, 25, 276, "",  "FLAT5_5" },
    { pack_tnt, 1, 25, 277, "",  "FLAT5_5" },
    { pack_tnt, 1, 25, 278, "",  "FLAT5_5" },
    { pack_tnt, 1, 25, 279, "",  "FLAT5_5" },
    { pack_tnt, 1, 25, 292, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 304, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 306, "",  "CEIL5_2" },
    { pack_tnt, 1, 25, 319, "",  "RROCK09" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map25[] =
{
    { pack_tnt, 1, 25,  10, 100 },
    { pack_tnt, 1, 25,  31, 100 },
    { pack_tnt, 1, 25,  32, 100 },
    { pack_tnt, 1, 25,  33, 100 },
    { pack_tnt, 1, 25,  92, 100 },
    { pack_tnt, 1, 25, 114, 100 },
    { pack_tnt, 1, 25, 116, 100 },
    { pack_tnt, 1, 25, 130, 100 },
    { pack_tnt, 1, 25, 136, 102 },
    { pack_tnt, 1, 25, 138, 100 },
    { pack_tnt, 1, 25, 140, 100 },
    { pack_tnt, 1, 25, 155, 100 },
    { pack_tnt, 1, 25, 197, 100 },
    { pack_tnt, 1, 25, 198, 100 },
    { pack_tnt, 1, 25, 199, 100 },
    { pack_tnt, 1, 25, 201, 100 },
    { pack_tnt, 1, 25, 215, 100 },
    { pack_tnt, 1, 25, 218, 100 },
    { pack_tnt, 1, 25, 219, 100 },
    { pack_tnt, 1, 25, 220, 100 },
    { pack_tnt, 1, 25, 221, 100 },
    { pack_tnt, 1, 25, 222, 100 },
    { pack_tnt, 1, 25, 223, 100 },
    { pack_tnt, 1, 25, 230, 101 },
    { pack_tnt, 1, 25, 232, 101 },
    { pack_tnt, 1, 25, 235, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 26: BALLISTYX
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map26[] =
{
    { pack_tnt, 1, 26,    0, 0, "",         "", "",       5, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26,  518, 0, "",         "", "",       5, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26,  813, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26,  814, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26,  815, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26, 1297, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26, 1298, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26, 1299, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26, 1300, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26, 1301, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    { pack_tnt, 1, 26, 1302, 1, "", "TYIRONLG", "", DEFAULT, DEFAULT, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map26[] =
{
    { pack_tnt, 1, 26,  23, "",  "FLAT20" },
    { pack_tnt, 1, 26,  35, "", "FLAT5_1" },
    { pack_tnt, 1, 26,  44, "", "CEIL5_2" },
    { pack_tnt, 1, 26,  90, "", "FLAT5_1" },
    { pack_tnt, 1, 26, 234, "", "CEIL5_2" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map26[] =
{
    { pack_tnt, 1, 26,   9, 220 },
    { pack_tnt, 1, 26,  51, 100 },
    { pack_tnt, 1, 26,  52, 100 },
    { pack_tnt, 1, 26,  53, 100 },
    { pack_tnt, 1, 26,  54, 100 },
    { pack_tnt, 1, 26,  55, 100 },
    { pack_tnt, 1, 26,  57, 100 },
    { pack_tnt, 1, 26,  60, 100 },
    { pack_tnt, 1, 26,  61, 100 },
    { pack_tnt, 1, 26,  62, 100 },
    { pack_tnt, 1, 26,  63, 100 },
    { pack_tnt, 1, 26,  64, 100 },
    { pack_tnt, 1, 26,  65, 100 },
    { pack_tnt, 1, 26,  66, 100 },
    { pack_tnt, 1, 26, 188, 220 },
    { pack_tnt, 1, 26, 202, 220 },
    { pack_tnt, 1, 26, 203, 220 },
    { pack_tnt, 1, 26, 204, 220 },
    { pack_tnt, 1, 26, 219, 101 },
    { pack_tnt, 1, 26, 239, 100 },
    { pack_tnt, 1, 26, 240, 100 },
    { pack_tnt, 1, 26, 243, 100 },
    { pack_tnt, 1, 26, 258, 100 },
    { pack_tnt, 1, 26, 259, 100 },
    { pack_tnt, 1, 26, 262, 100 },
    FLOW_END
};

static const fall_t fall_tnt_map26[] =
{
    { pack_tnt, 1, 26, 1444,  101 },
    { pack_tnt, 1, 26, 1445,  101 },
    { pack_tnt, 1, 26, 1446,  101 },
    { pack_tnt, 1, 26, 1448,  101 },
    { pack_tnt, 1, 26, 1325,  101 },
    { pack_tnt, 1, 26, 1331,  101 },
    { pack_tnt, 1, 26, 1334,  101 },
    { pack_tnt, 1, 26, 1335,  101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 27: MOUNT PAIN
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map27[] =
{
    { pack_tnt, 1, 27, 1175, 0,         "", "BROWNGRN", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 27, 1177, 0,         "", "BROWNGRN", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 27, 1190, 0,         "",         "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { pack_tnt, 1, 27, 1244, 0,         "", "BROWNGRN", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 27, 1245, 0,         "", "BROWNGRN", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 27, 1604, 1, "ROCKRED1",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 27, 2002, 1, "ROCKRED1",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 27, 2007, 1, "ROCKRED1",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    { pack_tnt, 1, 27, 2008, 1, "ROCKRED1",         "", "", DEFAULT, DEFAULT,                            DEFAULT },
    LINEFIX_END
};

static const flow_t flow_tnt_map27[] =
{
    { pack_tnt, 1, 27,  44, 213 },
    { pack_tnt, 1, 27,  45, 213 },
    { pack_tnt, 1, 27,  46, 213 },
    { pack_tnt, 1, 27,  47, 213 },
    { pack_tnt, 1, 27,  62, 100 },
    { pack_tnt, 1, 27, 103, 101 },
    { pack_tnt, 1, 27, 105, 101 },
    { pack_tnt, 1, 27, 129, 101 },
    { pack_tnt, 1, 27, 130, 101 },
    { pack_tnt, 1, 27, 131, 101 },
    { pack_tnt, 1, 27, 168, 101 },
    { pack_tnt, 1, 27, 169, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 28: HECK
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map28[] =
{
    { pack_tnt, 1, 28, 86, "", "CEIL5_2" },
    { pack_tnt, 1, 28, 87, "", "CEIL5_2" },
    { pack_tnt, 1, 28, 89, "",  "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map28[] =
{
    { pack_tnt, 1, 28,   8, 100 },
    { pack_tnt, 1, 28,  21, 100 },
    { pack_tnt, 1, 28,  71, 100 },
    { pack_tnt, 1, 28,  73, 100 },
    { pack_tnt, 1, 28,  77, 100 },
    { pack_tnt, 1, 28,  78, 100 },
    { pack_tnt, 1, 28,  90, 100 },
    { pack_tnt, 1, 28,  98, 100 },
    { pack_tnt, 1, 28, 101, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 29: RIVER STYX
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map29[] =
{
    { pack_tnt, 1, 29, 442, 0, "", "", "", 136, 48, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map29[] =
{
    { pack_tnt, 1, 29, 129, "", "CEIL5_2" },
    { pack_tnt, 1, 29, 119, "", "FLAT5_1" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map29[] =
{
    { pack_tnt, 1, 29,   1, 100 },
    { pack_tnt, 1, 29,   2, 100 },
    { pack_tnt, 1, 29,   4, 100 },
    { pack_tnt, 1, 29,   5, 100 },
    { pack_tnt, 1, 29,   8, 100 },
    { pack_tnt, 1, 29,  38, 100 },
    { pack_tnt, 1, 29,  39, 100 },
    { pack_tnt, 1, 29,  51, 100 },
    { pack_tnt, 1, 29,  52, 100 },
    { pack_tnt, 1, 29,  53, 100 },
    { pack_tnt, 1, 29,  54, 100 },
    { pack_tnt, 1, 29,  93, 100 },
    { pack_tnt, 1, 29,  94, 206 },
    { pack_tnt, 1, 29,  97, 100 },
    { pack_tnt, 1, 29,  98, 100 },
    { pack_tnt, 1, 29,  99, 100 },
    { pack_tnt, 1, 29, 102, 100 },
    { pack_tnt, 1, 29, 103, 216 },
    { pack_tnt, 1, 29, 104, 216 },
    { pack_tnt, 1, 29, 105, 101 },
    { pack_tnt, 1, 29, 109, 230 },
    { pack_tnt, 1, 29, 111, 206 },
    { pack_tnt, 1, 29, 112, 206 },
    { pack_tnt, 1, 29, 113, 206 },
    { pack_tnt, 1, 29, 114, 201 },
    { pack_tnt, 1, 29, 115, 100 },
    { pack_tnt, 1, 29, 117, 100 },
    { pack_tnt, 1, 29, 132, 100 },
    { pack_tnt, 1, 29, 133, 101 },
    { pack_tnt, 1, 29, 147, 100 },
    { pack_tnt, 1, 29, 148, 231 },
    { pack_tnt, 1, 29, 149, 231 },
    { pack_tnt, 1, 29, 150, 231 },
    { pack_tnt, 1, 29, 151, 100 },
    { pack_tnt, 1, 29, 160, 100 },
    { pack_tnt, 1, 29, 161, 100 },
    { pack_tnt, 1, 29, 163, 100 },
    { pack_tnt, 1, 29, 201, 100 },
    { pack_tnt, 1, 29, 202, 100 },
    { pack_tnt, 1, 29, 208, 100 },
    { pack_tnt, 1, 29, 209, 100 },
    { pack_tnt, 1, 29, 235, 100 },
    { pack_tnt, 1, 29, 237, 100 },
    { pack_tnt, 1, 29, 468, 100 },
    { pack_tnt, 1, 29, 470, 100 },
    { pack_tnt, 1, 29, 471, 231 },
    { pack_tnt, 1, 29, 472, 231 },
    { pack_tnt, 1, 29, 473, 231 },
    FLOW_END
};

static const fall_t fall_tnt_map29[] =
{
    { pack_tnt, 1, 29,  452, 101 },
    { pack_tnt, 1, 29,  457, 101 },
    { pack_tnt, 1, 29,  480, 101 },
    { pack_tnt, 1, 29,  495, 102 },
    { pack_tnt, 1, 29,  514, 101 },
    { pack_tnt, 1, 29,  520, 101 },
    { pack_tnt, 1, 29,  528, 101 },
    { pack_tnt, 1, 29,  564, 101 },
    { pack_tnt, 1, 29,  576, 101 },
    { pack_tnt, 1, 29,  579, 101 },
    { pack_tnt, 1, 29,  582, 101 },
    { pack_tnt, 1, 29,  587, 101 },
    { pack_tnt, 1, 29,  596, 101 },
    { pack_tnt, 1, 29,  617, 102 },
    { pack_tnt, 1, 29,  618, 102 },
    { pack_tnt, 1, 29,  697, 101 },
    { pack_tnt, 1, 29,  707, 101 },
    { pack_tnt, 1, 29,  852, 101 },
    { pack_tnt, 1, 29,  856, 102 },
    { pack_tnt, 1, 29,  857, 101 },
    { pack_tnt, 1, 29,  859, 102 },
    { pack_tnt, 1, 29,  860, 102 },
    { pack_tnt, 1, 29,  863, 102 },
    { pack_tnt, 1, 29, 2483, 101 },
    { pack_tnt, 1, 29, 2486, 101 },
    { pack_tnt, 1, 29, 2489, 101 },
    FALL_END
};

// -----------------------------------------------------------------------------
// LEVEL 30: LAST CALL
// -----------------------------------------------------------------------------

static const sectorfix_t sectorfix_tnt_map30[] =
{
    { pack_tnt, 1, 30, 283, "FLAT23", "" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map30[] =
{
    { pack_tnt, 1, 30, 153, 232 },
    { pack_tnt, 1, 30, 186, 100 },
    { pack_tnt, 1, 30, 207, 100 },
    { pack_tnt, 1, 30, 277, 100 },
    { pack_tnt, 1, 30, 279, 100 },
    { pack_tnt, 1, 30, 280, 100 },
    { pack_tnt, 1, 30, 284, 100 },
    { pack_tnt, 1, 30, 285, 100 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 31: PHARAOH
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map31[] =
{
    { pack_tnt, 1, 31, 1190, 0, "", "", "", DEFAULT, DEFAULT, (ML_DONTPEGTOP | ML_DONTPEGBOTTOM) },
    { pack_tnt, 1, 31, 1251, 0, "", "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1252, 0, "", "", "", DEFAULT, DEFAULT,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1676, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1677, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1678, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1679, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1680, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1681, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1682, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    { pack_tnt, 1, 31, 1683, 0, "", "", "", DEFAULT,      64,                   ML_DONTPEGBOTTOM },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map31[] =
{
    { pack_tnt, 1, 31, 131, "", "FLOOR7_2" },
    { pack_tnt, 1, 31, 137, "",   "FLAT20" },
    { pack_tnt, 1, 31, 138, "",   "FLAT20" },
    { pack_tnt, 1, 31, 143, "",   "FLAT20" },
    { pack_tnt, 1, 31, 231, "",  "CEIL5_2" },
    { pack_tnt, 1, 31, 311, "",   "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map31[] =
{
    { pack_tnt, 1, 31, 109, 100 },
    { pack_tnt, 1, 31, 110, 100 },
    { pack_tnt, 1, 31, 166, 100 },
    { pack_tnt, 1, 31, 234, 100 },
    { pack_tnt, 1, 31, 312, 102 },
    { pack_tnt, 1, 31, 363, 102 },
    { pack_tnt, 1, 31, 364, 102 },
    { pack_tnt, 1, 31, 365, 102 },
    { pack_tnt, 1, 31, 371, 102 },
    { pack_tnt, 1, 31, 372, 102 },
    FLOW_END
};

// -----------------------------------------------------------------------------
// LEVEL 32: CARIBBEAN
// -----------------------------------------------------------------------------

static const linefix_t linefix_tnt_map32[] =
{
    { pack_tnt, 1, 32,  984, 0, "", "", "", DEFAULT, 16, DEFAULT },
    { pack_tnt, 1, 32, 1026, 0, "", "", "", DEFAULT, 16, DEFAULT },
    { pack_tnt, 1, 32, 1215, 0, "", "", "",       3,  8, DEFAULT },
    { pack_tnt, 1, 32, 1218, 0, "", "", "", DEFAULT, 56, DEFAULT },
    { pack_tnt, 1, 32, 1223, 0, "", "", "",      72, 56, DEFAULT },
    { pack_tnt, 1, 32, 1281, 0, "", "", "",      59,  8, DEFAULT },
    LINEFIX_END
};

static const sectorfix_t sectorfix_tnt_map32[] =
{
    { pack_tnt, 1, 32,  86, "FLOOR3_3",       "" },
    { pack_tnt, 1, 32,  81,  "RROCK09",       "" },
    { pack_tnt, 1, 32,  87,  "CEIL5_2",       "" },
    { pack_tnt, 1, 32,  94, "FLOOR7_1",       "" },
    { pack_tnt, 1, 32, 201,         "", "FLAT20" },
    { pack_tnt, 1, 32, 247,         "", "FLAT20" },
    SECTORFIX_END
};

static const flow_t flow_tnt_map32[] =
{
    { pack_tnt, 1, 32,   0, 223 },
    { pack_tnt, 1, 32,  17, 223 },
    { pack_tnt, 1, 32,  21, 223 },
    { pack_tnt, 1, 32,  22, 223 },
    { pack_tnt, 1, 32,  82, 223 },
    { pack_tnt, 1, 32, 108, 100 },
    { pack_tnt, 1, 32, 146, 100 },
    { pack_tnt, 1, 32, 147, 100 },
    { pack_tnt, 1, 32, 164, 100 },
    { pack_tnt, 1, 32, 165, 100 },
    { pack_tnt, 1, 32, 166, 100 },
    { pack_tnt, 1, 32, 167, 100 },
    { pack_tnt, 1, 32, 168, 100 },
    { pack_tnt, 1, 32, 169, 100 },
    { pack_tnt, 1, 32, 170, 100 },
    { pack_tnt, 1, 32, 171, 100 },
    { pack_tnt, 1, 32, 172, 100 },
    { pack_tnt, 1, 32, 173, 100 },
    { pack_tnt, 1, 32, 174, 100 },
    { pack_tnt, 1, 32, 175, 100 },
    { pack_tnt, 1, 32, 176, 100 },
    { pack_tnt, 1, 32, 177, 100 },
    { pack_tnt, 1, 32, 178, 100 },
    { pack_tnt, 1, 32, 179, 100 },
    { pack_tnt, 1, 32, 180, 100 },
    { pack_tnt, 1, 32, 181, 100 },
    { pack_tnt, 1, 32, 188, 100 },
    { pack_tnt, 1, 32, 190, 100 },
    { pack_tnt, 1, 32, 196, 100 },
    { pack_tnt, 1, 32, 197, 100 },
    { pack_tnt, 1, 32, 183, 223 },
    { pack_tnt, 1, 32, 237, 221 },
    { pack_tnt, 1, 32, 238, 221 },
    FLOW_END
};


// -----------------------------------------------------------------------------
// P_SetupFixes
// [JN] Sets appropriated fixes for selected map.
// -----------------------------------------------------------------------------

void P_SetupFixes (const int episode, const int map)
{
    // First, initialize all fixes with dummies.
    selected_vertexfix = vertexfix_dummy;
    selected_linefix   = linefix_dummy;
    selected_sectorfix = sectorfix_dummy;

    selected_flow = flow_dummy;
    selected_fall = fall_dummy;

    // Next, define which game will use which map fixes.
    if (logical_gamemission == doom)
    {
        if (gameepisode == 1)
        {
            if (gamemap == 1)
            {
                selected_linefix   = linefix_doom1_e1m1;
                selected_sectorfix = sectorfix_doom1_e1m1;
                selected_flow      = flow_doom1_e1m1;
            }
            else
            if (gamemap == 2)
            {
                selected_linefix   = linefix_doom1_e1m2;
                selected_flow      = flow_doom1_e1m2;
            }
            else
            if (gamemap == 3)
            {
                selected_vertexfix = vertexfix_doom1_e1m3;
                selected_linefix   = linefix_doom1_e1m3;
                selected_sectorfix = sectorfix_doom1_e1m3;
                selected_flow      = flow_doom1_e1m3;
            }
            else
            if (gamemap == 4)
            {
                selected_linefix   = linefix_doom1_e1m4;
                selected_sectorfix = sectorfix_doom1_e1m4;
                selected_flow      = flow_doom1_e1m4;
            }
            else
            if (gamemap == 5)
            {
                selected_linefix   = linefix_doom1_e1m5;
                selected_sectorfix = sectorfix_doom1_e1m5;
                selected_flow      = flow_doom1_e1m5;
            }
            else
            if (gamemap == 6)
            {
                selected_linefix   = linefix_doom1_e1m6;
                selected_sectorfix = sectorfix_doom1_e1m6;
                selected_flow      = flow_doom1_e1m6;
            }
            else
            if (gamemap == 7)
            {
                selected_linefix   = linefix_doom1_e1m7;
                selected_sectorfix = sectorfix_doom1_e1m7;
                selected_flow      = flow_doom1_e1m7;
            }
            else
            if (gamemap == 8)
            {
                selected_linefix   = linefix_doom1_e1m8;
            }
            else
            if (gamemap == 9)
            {
                selected_sectorfix = sectorfix_doom1_e1m9;
                selected_flow      = flow_doom1_e1m9;
            }
        }
        else
        if (gameepisode == 2)
        {
            if (gamemap == 1)
            {
                selected_linefix   = linefix_doom1_e2m1;
                selected_sectorfix = sectorfix_doom1_e2m1;
                selected_flow      = flow_doom1_e2m1;
            }
            else
            if (gamemap == 2)
            {
                selected_vertexfix = vertexfix_doom1_e2m2;
                selected_linefix   = linefix_doom1_e2m2;
                selected_sectorfix = sectorfix_doom1_e2m2;
                selected_flow      = flow_doom1_e2m2;
            }
            else
            if (gamemap == 3)
            {
                selected_linefix   = linefix_doom1_e2m3;
                selected_sectorfix = sectorfix_doom1_e2m3;
                selected_flow      = flow_doom1_e2m3;
            }
            else
            if (gamemap == 4)
            {
                selected_linefix   = linefix_doom1_e2m4;
                selected_sectorfix = sectorfix_doom1_e2m4;
                selected_flow      = flow_doom1_e2m4;
            }
            else
            if (gamemap == 5)
            {
                selected_linefix   = linefix_doom1_e2m5;
                selected_sectorfix = sectorfix_doom1_e2m5;
                selected_flow      = flow_doom1_e2m5;
            }
            else
            if (gamemap == 6)
            {
                selected_vertexfix = vertexfix_doom1_e2m6;
                selected_linefix   = linefix_doom1_e2m6;
                selected_sectorfix = sectorfix_doom1_e2m6;
                selected_flow      = flow_doom1_e2m6;
            }
            else
            if (gamemap == 7)
            {
                selected_vertexfix = vertexfix_doom1_e2m7;
                selected_linefix   = linefix_doom1_e2m7;
                selected_sectorfix = sectorfix_doom1_e2m7;
                selected_flow      = flow_doom1_e2m7;
            }
            else
            if (gamemap == 9)
            {
                selected_linefix   = linefix_doom1_e2m9;
                selected_sectorfix = sectorfix_doom1_e2m9;
                selected_flow      = flow_doom1_e2m9;
            }
        }
        else
        if (gameepisode == 3)
        {
            if (gamemap == 1)
            {
                selected_linefix   = linefix_doom1_e3m1;
                selected_flow      = flow_doom1_e3m1;
            }
            else
            if (gamemap == 2)
            {
                selected_linefix   = linefix_doom1_e3m2;
                selected_sectorfix = sectorfix_doom1_e3m2;
                selected_flow      = flow_doom1_e3m2;
            }
            else
            if (gamemap == 3)
            {
                selected_linefix   = linefix_doom1_e3m3;
                selected_sectorfix = sectorfix_doom1_e3m3;
                selected_flow      = flow_doom1_e3m3;
            }
            else
            if (gamemap == 4)
            {
                selected_linefix   = linefix_doom1_e3m4;
                selected_sectorfix = sectorfix_doom1_e3m4;
                selected_flow      = flow_doom1_e3m4;
            }
            else
            if (gamemap == 5)
            {
                selected_linefix   = linefix_doom1_e3m5;
                selected_sectorfix = sectorfix_doom1_e3m5;
                selected_flow      = flow_doom1_e3m5;
            }
            else
            if (gamemap == 6)
            {
                selected_linefix   = linefix_doom1_e3m6;
                selected_sectorfix = sectorfix_doom1_e3m6;
                selected_flow      = flow_doom1_e3m6;
            }
            else
            if (gamemap == 7)
            {
                selected_linefix   = linefix_doom1_e3m7;
                selected_sectorfix = sectorfix_doom1_e3m7;
                selected_flow      = flow_doom1_e3m7;
            }
            else
            if (gamemap == 8)
            {
                selected_sectorfix = sectorfix_doom1_e3m8;
            }
            else
            if (gamemap == 9)
            {
                selected_linefix   = linefix_doom1_e3m9;
                selected_sectorfix = sectorfix_doom1_e3m9;
                selected_flow      = flow_doom1_e3m9;
            }
        }
        else
        if (gameepisode == 4)
        {
            if (gamemap == 1)
            {
                selected_linefix   = linefix_doom1_e4m1;
                selected_sectorfix = sectorfix_doom1_e4m1;
                selected_flow      = flow_doom1_e4m1;
            }
            else
            if (gamemap == 2)
            {
                selected_linefix   = linefix_doom1_e4m2;
                selected_sectorfix = sectorfix_doom1_e4m2;
                selected_flow      = flow_doom1_e4m2;
            }
            else
            if (gamemap == 3)
            {
                selected_linefix   = linefix_doom1_e4m3;
                selected_sectorfix = sectorfix_doom1_e4m3;
                selected_flow      = flow_doom1_e4m3;
            }
            else
            if (gamemap == 4)
            {
                selected_linefix   = linefix_doom1_e4m4;
                selected_sectorfix = sectorfix_doom1_e4m4;
                selected_flow      = flow_doom1_e4m4;
            }
            else
            if (gamemap == 5)
            {
                selected_linefix   = linefix_doom1_e4m5;
                selected_sectorfix = sectorfix_doom1_e4m5;
                selected_flow      = flow_doom1_e4m5;
            }
            else
            if (gamemap == 6)
            {
                selected_linefix   = linefix_doom1_e4m6;
                selected_flow      = flow_doom1_e4m6;
            }
            else
            if (gamemap == 7)
            {
                selected_linefix   = linefix_doom1_e4m7;
                selected_sectorfix = sectorfix_doom1_e4m7;
                selected_flow      = flow_doom1_e4m7;
            }
            else
            if (gamemap == 8)
            {
                selected_linefix   = linefix_doom1_e4m8;
                selected_sectorfix = sectorfix_doom1_e4m8;
                selected_flow      = flow_doom1_e4m8;
            }
            else
            if (gamemap == 9)
            {
                selected_linefix   = linefix_doom1_e4m9;
                selected_sectorfix = sectorfix_doom1_e4m9;
                selected_flow      = flow_doom1_e4m9;
            }
        }
    }
    else
    if (logical_gamemission == doom2)
    {
        if (gamemap == 1)
        {
            selected_vertexfix = vertexfix_doom2_map01;
            selected_linefix   = linefix_doom2_map01;
            selected_sectorfix = sectorfix_doom2_map01;
            selected_flow      = flow_doom2_map01;
        }
        else
        if (gamemap == 2)
        {
            selected_vertexfix = vertexfix_doom2_map02;
            selected_linefix   = linefix_doom2_map02;
            selected_sectorfix = sectorfix_doom2_map02;
            selected_flow      = flow_doom2_map02;
        }
        else
        if (gamemap == 3)
        {
            selected_linefix   = linefix_doom2_map03;
            selected_sectorfix = sectorfix_doom2_map03;
            selected_flow      = flow_doom2_map03;
        }
        else
        if (gamemap == 4)
        {
            selected_linefix   = linefix_doom2_map04;
            selected_sectorfix = sectorfix_doom2_map04;
            selected_flow      = flow_doom2_map04;
        }
        else
        if (gamemap == 5)
        {
            selected_linefix   = linefix_doom2_map05;
            selected_sectorfix = sectorfix_doom2_map05;
            selected_flow      = flow_doom2_map05;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_doom2_map06;
            selected_sectorfix = sectorfix_doom2_map06;
            selected_flow      = flow_doom2_map06;
        }
        else
        if (gamemap == 7)
        {
            selected_linefix   = linefix_doom2_map07;
        }
        else
        if (gamemap == 8)
        {
            selected_linefix   = linefix_doom2_map08;
            selected_sectorfix = sectorfix_doom2_map08;
            selected_flow      = flow_doom2_map08;
        }
        else
        if (gamemap == 9)
        {
            selected_linefix   = linefix_doom2_map09;
            selected_sectorfix = sectorfix_doom2_map09;
            selected_flow      = flow_doom2_map09;
        }
        else
        if (gamemap == 10)
        {
            selected_linefix   = linefix_doom2_map10;
            selected_sectorfix = sectorfix_doom2_map10;
            selected_flow      = flow_doom2_map10;
        }
        else
        if (gamemap == 11)
        {
            selected_linefix   = linefix_doom2_map11;
            selected_sectorfix = sectorfix_doom2_map11;
            selected_flow      = flow_doom2_map11;
        }
        else
        if (gamemap == 12)
        {
            selected_linefix   = linefix_doom2_map12;
            selected_sectorfix = sectorfix_doom2_map12;
            selected_flow      = flow_doom2_map12;
        }
        else
        if (gamemap == 13)
        {
            selected_linefix   = linefix_doom2_map13;
            selected_sectorfix = sectorfix_doom2_map13;
        }
        else
        if (gamemap == 14)
        {
            selected_linefix   = linefix_doom2_map14;
            selected_sectorfix = sectorfix_doom2_map14;
            selected_flow      = flow_doom2_map14;
        }
        else
        if (gamemap == 15)
        {
            selected_linefix   = linefix_doom2_map15;
            selected_sectorfix = sectorfix_doom2_map15;
            selected_flow      = flow_doom2_map15;
        }
        else
        if (gamemap == 16)
        {
            selected_linefix   = linefix_doom2_map16;
            selected_flow      = flow_doom2_map16;
        }
        else
        if (gamemap == 17)
        {
            selected_linefix   = linefix_doom2_map17;
            selected_sectorfix = sectorfix_doom2_map17;
            selected_flow      = flow_doom2_map17;
        }
        else
        if (gamemap == 18)
        {
             selected_linefix   = linefix_doom2_map18;
             selected_sectorfix = sectorfix_doom2_map18;
             selected_flow      = flow_doom2_map18;
        }
        else
        if (gamemap == 19)
        {
            selected_linefix   = linefix_doom2_map19;
            selected_sectorfix = sectorfix_doom2_map19;
            selected_flow      = flow_doom2_map19;
        }
        else
        if (gamemap == 20)
        {
            selected_linefix   = linefix_doom2_map20;
            selected_sectorfix = sectorfix_doom2_map20;
            selected_flow      = flow_doom2_map20;
        }
        else
        if (gamemap == 21)
        {
             selected_linefix   = linefix_doom2_map21;
             selected_sectorfix = sectorfix_doom2_map21;
             selected_flow      = flow_doom2_map21;
        }
        else
        if (gamemap == 22)
        {
            selected_linefix   = linefix_doom2_map22;
            selected_sectorfix = sectorfix_doom2_map22;
            selected_flow      = flow_doom2_map22;
        }
        else
        if (gamemap == 23)
        {
            selected_linefix   = linefix_doom2_map23;
            selected_sectorfix = sectorfix_doom2_map23;
            selected_flow      = flow_doom2_map23;
        }
        else
        if (gamemap == 24)
        {
             selected_linefix   = linefix_doom2_map24;
             selected_flow      = flow_doom2_map24;
             selected_fall      = fall_doom2_map24;
        }
        else
        if (gamemap == 25)
        {
            selected_linefix   = linefix_doom2_map25;
            selected_sectorfix = sectorfix_doom2_map25;
            selected_flow      = flow_doom2_map25;
            selected_fall      = fall_doom2_map25;
        }
        else
        if (gamemap == 26)
        {
            selected_linefix   = linefix_doom2_map26;
            selected_sectorfix = sectorfix_doom2_map26;
            selected_flow      = flow_doom2_map26;
        }
        else
        if (gamemap == 27)
        {
            selected_linefix   = linefix_doom2_map27;
            selected_sectorfix = sectorfix_doom2_map27;
            selected_flow      = flow_doom2_map27;
        }
        else
        if (gamemap == 28)
        {
            selected_linefix   = linefix_doom2_map28;
            selected_flow      = flow_doom2_map28;
            selected_fall      = fall_doom2_map28;
        }
        else
        if (gamemap == 29)
        {
            selected_linefix   = linefix_doom2_map29;
            selected_sectorfix = sectorfix_doom2_map29;
            selected_flow      = flow_doom2_map29;
        }
        else
        if (gamemap == 30)
        {
            selected_vertexfix = vertexfix_doom2_map30;
            selected_linefix   = linefix_doom2_map30;
            selected_flow      = flow_doom2_map30;
            selected_fall      = fall_doom2_map30;
        }
        else
        if (gamemap == 31)
        {
            selected_linefix   = linefix_doom2_map31;
        }
        else
        if (gamemap == 32)
        {
            selected_linefix   = linefix_doom2_map32;
        }
    }
    else
    if (logical_gamemission == pack_plut)
    {
        if (gamemap == 1)
        {
            selected_linefix   = linefix_plut_map01;
            selected_sectorfix = sectorfix_plut_map01;
            selected_flow      = flow_plut_map01;
        }
        else
        if (gamemap == 2)
        {
            selected_linefix   = linefix_plut_map02;
            selected_sectorfix = sectorfix_plut_map02;
            selected_flow      = flow_plut_map02;
            selected_fall      = fall_plut_map02;
        }
        else
        if (gamemap == 3)
        {
            selected_linefix   = linefix_plut_map03;
            selected_sectorfix = sectorfix_plut_map03;
            selected_flow      = flow_plut_map03;
        }
        else
        if (gamemap == 4)
        {
            selected_linefix   = linefix_plut_map04;
            selected_sectorfix = sectorfix_plut_map04;
            selected_flow      = flow_plut_map04;
        }
        else
        if (gamemap == 5)
        {
            selected_linefix   = linefix_plut_map05;
            selected_sectorfix = sectorfix_plut_map05;
            selected_flow      = flow_plut_map05;
            selected_fall      = fall_plut_map05;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_plut_map06;
            selected_sectorfix = sectorfix_plut_map06;
            selected_flow      = flow_plut_map06;
            selected_fall      = fall_plut_map06;
        }
        else
        if (gamemap == 7)
        {
            selected_linefix   = linefix_plut_map07;
            selected_sectorfix = sectorfix_plut_map07;
            selected_flow      = flow_plut_map07;
        }
        else
        if (gamemap == 8)
        {
            selected_linefix   = linefix_plut_map08;
            selected_sectorfix = sectorfix_plut_map08;
            selected_flow      = flow_plut_map08;
            selected_fall      = fall_plut_map08;
        }
        else
        if (gamemap == 9)
        {
            selected_linefix   = linefix_plut_map09;
            selected_sectorfix = sectorfix_plut_map09;
            selected_flow      = flow_plut_map09;
            selected_fall      = fall_plut_map09;
        }
        else
        if (gamemap == 10)
        {
            selected_linefix   = linefix_plut_map10;
            selected_sectorfix = sectorfix_plut_map10;
            selected_flow      = flow_plut_map10;
        }
        else
        if (gamemap == 11)
        {
            selected_sectorfix = sectorfix_plut_map11;
            selected_flow      = flow_plut_map11;
        }
        else
        if (gamemap == 12)
        {
            selected_linefix   = linefix_plut_map12;
            selected_sectorfix = sectorfix_plut_map12;
            selected_flow      = flow_plut_map12;
            selected_fall      = fall_plut_map12;
        }
        else
        if (gamemap == 13)
        {
            selected_linefix   = linefix_plut_map13;
            selected_sectorfix = sectorfix_plut_map13;
            selected_flow      = flow_plut_map13;
            selected_fall      = fall_plut_map13;
        }
        else
        if (gamemap == 14)
        {
            selected_linefix   = linefix_plut_map14;
            selected_sectorfix = sectorfix_plut_map14;
            selected_flow      = flow_plut_map14;
            selected_fall      = fall_plut_map14;
        }
        else
        if (gamemap == 15)
        {
            selected_linefix   = linefix_plut_map15;
            selected_sectorfix = sectorfix_plut_map15;
            selected_flow      = flow_plut_map15;
        }
        else
        if (gamemap == 16)
        {
            selected_linefix   = linefix_plut_map16;
            selected_sectorfix = sectorfix_plut_map16;
            selected_flow      = flow_plut_map16;
            selected_fall      = fall_plut_map16;
        }
        else
        if (gamemap == 17)
        {
            selected_sectorfix = sectorfix_plut_map17;
            selected_flow      = flow_plut_map17;
            selected_fall      = fall_plut_map17;
        }
        else
        if (gamemap == 18)
        {
            selected_sectorfix = sectorfix_plut_map18;
            selected_flow      = flow_plut_map18;
        }
        else
        if (gamemap == 19)
        {
            selected_linefix   = linefix_plut_map19;
            selected_sectorfix = sectorfix_plut_map19;
            selected_flow      = flow_plut_map19;
            selected_fall      = fall_plut_map19;
        }
        else
        if (gamemap == 20)
        {
            selected_linefix   = linefix_plut_map20;
            selected_sectorfix = sectorfix_plut_map20;
            selected_flow      = flow_plut_map20;
        }
        else
        if (gamemap == 21)
        {
            selected_sectorfix = sectorfix_plut_map21;
            selected_flow      = flow_plut_map21;
        }
        else
        if (gamemap == 22)
        {
            selected_linefix   = linefix_plut_map22;
            selected_sectorfix = sectorfix_plut_map22;
            selected_flow      = flow_plut_map22;
            selected_fall      = fall_plut_map22;
        }
        else
        if (gamemap == 23)
        {
            selected_linefix   = linefix_plut_map23;
            selected_sectorfix = sectorfix_plut_map23;
            selected_flow      = flow_plut_map23;
            selected_fall      = fall_plut_map23;
        }
        else
        if (gamemap == 24)
        {
            selected_linefix   = linefix_plut_map24;
            selected_sectorfix = sectorfix_plut_map24;
            selected_flow      = flow_plut_map24;
            selected_fall      = fall_plut_map24;
        }
        else
        if (gamemap == 25)
        {
            selected_linefix   = linefix_plut_map25;
            selected_sectorfix = sectorfix_plut_map25;
            selected_flow      = flow_plut_map25;
        }
        else
        if (gamemap == 26)
        {
            selected_linefix   = linefix_plut_map26;
            selected_sectorfix = sectorfix_plut_map26;
            selected_flow      = flow_plut_map26;
            selected_fall      = fall_plut_map26;
        }
        else
        if (gamemap == 27)
        {
            selected_sectorfix = sectorfix_plut_map27;
            selected_flow      = flow_plut_map27;
        }
        else
        if (gamemap == 28)
        {
            selected_linefix   = linefix_plut_map28;
            selected_sectorfix = sectorfix_plut_map28;
            selected_flow      = flow_plut_map28;
            selected_fall      = fall_plut_map28;
        }
        else
        if (gamemap == 29)
        {
            selected_linefix   = linefix_plut_map29;
            selected_sectorfix = sectorfix_plut_map29;
            selected_flow      = flow_plut_map29;
        }
        else
        if (gamemap == 30)
        {
            selected_linefix   = linefix_plut_map30;
            selected_sectorfix = sectorfix_plut_map30;
            selected_flow      = flow_plut_map30;
            selected_fall      = fall_plut_map30;
        }
        else
        if (gamemap == 31)
        {
            selected_linefix   = linefix_plut_map31;
            selected_sectorfix = sectorfix_plut_map31;
            selected_flow      = flow_plut_map31;
            selected_fall      = fall_plut_map31;
        }
        else
        if (gamemap == 32)
        {
            selected_linefix   = linefix_plut_map32;
            selected_sectorfix = sectorfix_plut_map32;
            selected_flow      = flow_plut_map32;
        }
    }
    else
    if (logical_gamemission == pack_tnt)
    {
        if (gamemap == 1)
        {
            selected_linefix   = linefix_tnt_map01;
            selected_flow      = flow_tnt_map01;
        }
        else
        if (gamemap == 2)
        {
            selected_linefix   = linefix_tnt_map02;
            selected_sectorfix = sectorfix_tnt_map02;
            selected_flow      = flow_tnt_map02;
        }
        else
        if (gamemap == 3)
        {
            selected_linefix   = linefix_tnt_map03;
            selected_flow      = flow_tnt_map03;
        }
        else
        if (gamemap == 4)
        {
            selected_linefix   = linefix_tnt_map04;
            selected_sectorfix = sectorfix_tnt_map04;
        }
        else
        if (gamemap == 5)
        {
            selected_linefix   = linefix_tnt_map05;
            selected_sectorfix = sectorfix_tnt_map05;
        }
        else
        if (gamemap == 6)
        {
            selected_linefix   = linefix_tnt_map06;
            selected_sectorfix = sectorfix_tnt_map06;
            selected_flow      = flow_tnt_map06;
        }
        else
        if (gamemap == 7)
        {
            selected_sectorfix = sectorfix_tnt_map07;
            selected_flow      = flow_tnt_map07;
        }
        else
        if (gamemap == 8)
        {
            selected_sectorfix = sectorfix_tnt_map08;
            selected_flow      = flow_tnt_map08;
        }
        else
        if (gamemap == 9)
        {
            selected_linefix   = linefix_tnt_map09;
            selected_sectorfix = sectorfix_tnt_map09;
            selected_flow      = flow_tnt_map09;
            selected_fall      = fall_tnt_map09;
        }
        else
        if (gamemap == 10)
        {
            selected_linefix   = linefix_tnt_map10;
            selected_sectorfix = sectorfix_tnt_map10;
            selected_flow      = flow_tnt_map10;
            selected_fall      = fall_tnt_map10;
        }
        else
        if (gamemap == 11)
        {
            selected_linefix   = linefix_tnt_map11;
            selected_sectorfix = sectorfix_tnt_map11;
            selected_flow      = flow_tnt_map11;
        }
        else
        if (gamemap == 12)
        {
            selected_linefix   = linefix_tnt_map12;
            selected_sectorfix = sectorfix_tnt_map12;
            selected_flow      = flow_tnt_map12;
            selected_fall      = fall_tnt_map12;
        }
        else
        if (gamemap == 13)
        {
            selected_linefix   = linefix_tnt_map13;
            selected_sectorfix = sectorfix_tnt_map13;
            selected_flow      = flow_tnt_map13;
        }
        else
        if (gamemap == 14)
        {
            selected_sectorfix = sectorfix_tnt_map14;
            selected_flow      = flow_tnt_map14;
            selected_fall      = fall_tnt_map14;
        }
        else
        if (gamemap == 15)
        {
            selected_linefix   = linefix_tnt_map15;
            selected_sectorfix = sectorfix_tnt_map15;
            selected_flow      = flow_tnt_map15;
        }
        else
        if (gamemap == 16)
        {
            selected_sectorfix = sectorfix_tnt_map16;
            selected_flow      = flow_tnt_map16;
            selected_fall      = fall_tnt_map16;
        }
        else
        if (gamemap == 17)
        {
            selected_linefix   = linefix_tnt_map17;
            selected_sectorfix = sectorfix_tnt_map17;
            selected_flow      = flow_tnt_map17;
        }
        else
        if (gamemap == 18)
        {
            selected_sectorfix = sectorfix_tnt_map18;
            selected_flow      = flow_tnt_map18;
        }
        else
        if (gamemap == 19)
        {
            selected_linefix   = linefix_tnt_map19;
            selected_sectorfix = sectorfix_tnt_map19;
            selected_flow      = flow_tnt_map19;
        }
        else
        if (gamemap == 20)
        {
            selected_linefix   = linefix_tnt_map20;
            selected_sectorfix = sectorfix_tnt_map20;
            selected_flow      = flow_tnt_map20;
            selected_fall      = fall_tnt_map20;
        }
        else
        if (gamemap == 21)
        {
            selected_flow      = flow_tnt_map21;
        }
        else
        if (gamemap == 22)
        {
            selected_linefix   = linefix_tnt_map22;
            selected_sectorfix = sectorfix_tnt_map22;
            selected_flow      = flow_tnt_map22;
            selected_fall      = fall_tnt_map22;
        }
        else
        if (gamemap == 23)
        {
            selected_flow      = flow_tnt_map23;
            selected_fall      = fall_tnt_map23;
        }
        else
        if (gamemap == 24)
        {
            selected_sectorfix = sectorfix_tnt_map24;
            selected_flow      = flow_tnt_map24;
        }
        else
        if (gamemap == 25)
        {
            selected_linefix   = linefix_tnt_map25;
            selected_sectorfix = sectorfix_tnt_map25;
            selected_flow      = flow_tnt_map25;
        }
        else
        if (gamemap == 26)
        {
            selected_linefix   = linefix_tnt_map26;
            selected_sectorfix = sectorfix_tnt_map26;
            selected_flow      = flow_tnt_map26;
            selected_fall      = fall_tnt_map26;
        }
        else
        if (gamemap == 27)
        {
            selected_linefix   = linefix_tnt_map27;
            selected_flow      = flow_tnt_map27;
        }
        else
        if (gamemap == 28)
        {
            selected_sectorfix = sectorfix_tnt_map28;
            selected_flow      = flow_tnt_map28;
        }
        else
        if (gamemap == 29)
        {
            selected_linefix   = linefix_tnt_map29;
            selected_sectorfix = sectorfix_tnt_map29;
            selected_flow      = flow_tnt_map29;
            selected_fall      = fall_tnt_map29;
        }
        else
        if (gamemap == 30)
        {
            selected_sectorfix = sectorfix_tnt_map30;
            selected_flow      = flow_tnt_map30;
        }
        else
        if (gamemap == 31)
        {
            selected_linefix   = linefix_tnt_map31;
            selected_sectorfix = sectorfix_tnt_map31;
            selected_flow      = flow_tnt_map31;
        }
        else
        if (gamemap == 32)
        {
            selected_linefix   = linefix_tnt_map32;
            selected_sectorfix = sectorfix_tnt_map32;
            selected_flow      = flow_tnt_map32;
        }
    }
}
