//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2000, 2005-2014 Simon Howard
// Copyright(C) 2019 Fabian Greffrath
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
//	[crispy] add support for SMMU swirling flats
//

// [crispy] adapted from smmu/r_ripple.c, by Simon Howard

#include "doomstat.h"
#include "tables.h"
#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_swirl.h"

// swirl factors determine the number of waves per flat width

// 1 cycle per 64 units
#define swirlfactor (8192/64)

// 1 cycle per 32 units (2 in 64)
#define swirlfactor2 (8192/32)

#define SEQUENCE 1024
#define FLATSIZE (64 * 64)

static int *offsets;
static int *offset;

extern int firstflat;

#define AMP 2
#define AMP2 2
#define SPEED 40

void R_InitDistortedFlats (void)
{
	if (!offsets)
	{
		int i;

		offsets = I_Realloc(NULL, SEQUENCE * FLATSIZE * sizeof(*offsets));
		offset = offsets;

		for (i = 0; i < SEQUENCE; i++)
		{
			int x, y;

			for (x = 0; x < 64; x++)
			{
				for (y = 0; y < 64; y++)
				{
					int x1, y1;
					int sinvalue, sinvalue2;

					sinvalue = (y * swirlfactor + i * SPEED * 5 + 900) & 8191;
					sinvalue2 = (x * swirlfactor2 + i * SPEED * 4 + 300) & 8191;
					x1 = x + 128
					   + ((finesine[sinvalue] * AMP) >> FRACBITS)
					   + ((finesine[sinvalue2] * AMP2) >> FRACBITS);

					sinvalue = (x * swirlfactor + i * SPEED * 3 + 700) & 8191;
					sinvalue2 = (y * swirlfactor2 + i * SPEED * 4 + 1200) & 8191;
					y1 = y + 128
					   + ((finesine[sinvalue] * AMP) >> FRACBITS)
					   + ((finesine[sinvalue2] * AMP2) >> FRACBITS);

					x1 &= 63;
					y1 &= 63;

					offset[(y << 6) + x] = (y1 << 6) + x1;
				}
			}

			offset += FLATSIZE;
		}
	}
}

char *R_DistortedFlat(int flatnum)
{
	static int swirltic = -1;
	static int swirlflat = -1;
	static char distortedflat[FLATSIZE];

	if (swirltic != leveltime)
	{
		offset = offsets + ((leveltime & (SEQUENCE - 1)) * FLATSIZE);

		swirltic = leveltime;
		swirlflat = -1;
	}

	if (swirlflat != flatnum)
	{
		char *normalflat;
		int i;

        // [JN] Use defined flat
		// normalflat = W_CacheLumpNum(flatnum, PU_STATIC);
        normalflat = W_CacheLumpNum(firstflat + flatnum, PU_LEVEL);

		for (i = 0; i < FLATSIZE; i++)
		{
			distortedflat[i] = normalflat[offset[i]];
		}

		Z_ChangeTag(normalflat, PU_CACHE);

		swirlflat = flatnum;
	}

	return distortedflat;
}

// =============================================================================
//
// [JN] Flow effect of swirling liquids.
//
// Following code emulates still liquid moving using a circle and elipse
// shapes, and scrolling effect in eight rays of Rose of Wind.
//
// Applicable only for vanilla maps, because of there is no one nice solution
// for every possible liquid, direction and sector size, and thus, there is
// no way to make it looking nice in every possible place in custom maps.
//
// =========================================================== (2022-04-07) ====

fixed_t FlowFactor_X;
fixed_t FlowFactor_Y;
fixed_t FlowDelta_X;
fixed_t FlowDelta_Y;
#define FLOW_SLOWEST  1
#define FLOW_SLOWER   2
#define FLOW_NORMAL   4
#define FLOW_FASTER   8
#define FLOW_FASTEST  12

// -----------------------------------------------------------------------------
// R_FlowPlane
// [JN] Modify floor texture offset deltas of flowing surfaces.
// -----------------------------------------------------------------------------

void R_FlowPlane (int scroller)
{
    const fixed_t FlowAmplitude = (FINEANGLES / 160 * leveltime) & FINEMASK;

    switch (scroller)
    {
        // Circle rotation (low amplitude)
        case 100:
        {
            FlowDelta_X = finesine[FlowAmplitude] * 2;
            FlowDelta_Y = finecosine[FlowAmplitude] * 2;
            break;
        }

        // Elispe rotation (middle amplitude)
        case 101:
        {
            FlowDelta_X = finesine[FlowAmplitude] * 2;
            FlowDelta_Y = finecosine[FlowAmplitude] * 4;
            break;
        }

        // Elispe rotation (high amplitude)
        case 102:
        {
            FlowDelta_X = finesine[FlowAmplitude] * 4;
            FlowDelta_Y = finecosine[FlowAmplitude] * 8;
            break;
        }

        // Scroll NORTH
        case 200: case 201: case 202: case 203: case 204:
        {
            FlowDelta_Y = FlowFactor_Y;
            if (scroller == 200)
            {
                FlowDelta_Y *= FLOW_SLOWEST;
            }
            if (scroller == 201)
            {
                FlowDelta_Y *= FLOW_SLOWER;
            }
            if (scroller == 202)
            {
                FlowDelta_Y *= FLOW_NORMAL;
            }
            if (scroller == 203)
            {
                FlowDelta_Y *= FLOW_FASTER;
            }
            if (scroller == 204)
            {
                FlowDelta_Y *= FLOW_FASTEST;
            }
            break;
        }

        // Scroll NORTH-EAST
        case 205: case 206: case 207: case 208: case 209:
        {
            FlowDelta_X = -FlowFactor_X;
            FlowDelta_Y = FlowFactor_Y;
            if (scroller == 205)
            {
                FlowDelta_X *= FLOW_SLOWEST;
                FlowDelta_Y *= FLOW_SLOWEST;
            }
            if (scroller == 206)
            {
                FlowDelta_X *= FLOW_SLOWER;
                FlowDelta_Y *= FLOW_SLOWER;
            }
            if (scroller == 207)
            {
                FlowDelta_X *= FLOW_NORMAL;
                FlowDelta_Y *= FLOW_NORMAL;
            }
            if (scroller == 208)
            {
                FlowDelta_X *= FLOW_FASTER;
                FlowDelta_Y *= FLOW_FASTER;
            }
            if (scroller == 209)
            {
                FlowDelta_X *= FLOW_FASTEST;
                FlowDelta_Y *= FLOW_FASTEST;
            }
            break;
        }

        // Scroll EAST
        case 210: case 211: case 212: case 213: case 214:
        {
            FlowDelta_X = -FlowFactor_X;
            FlowDelta_Y = 0;
            if (scroller == 210)
            {
                FlowDelta_X *= FLOW_SLOWEST;
            }
            if (scroller == 211)
            {
                FlowDelta_X *= FLOW_SLOWER;
            }
            if (scroller == 212)
            {
                FlowDelta_X *= FLOW_NORMAL;
            }
            if (scroller == 213)
            {
                FlowDelta_X *= FLOW_FASTER;
            }
            if (scroller == 214)
            {
                FlowDelta_X *= FLOW_FASTEST;
            }
            break;
        }

        // Scroll SOUTH-EAST
        case 215: case 216: case 217: case 218: case 219:
        {
            FlowDelta_X = -FlowFactor_X;
            FlowDelta_Y = -FlowFactor_Y;
            if (scroller == 215)
            {
                FlowDelta_X *= FLOW_SLOWEST;
                FlowDelta_Y *= FLOW_SLOWEST;
            }
            if (scroller == 216)
            {
                FlowDelta_X *= FLOW_SLOWER;
                FlowDelta_Y *= FLOW_SLOWER;
            }
            if (scroller == 217)
            {
                FlowDelta_X *= FLOW_NORMAL;
                FlowDelta_Y *= FLOW_NORMAL;
            }
            if (scroller == 218)
            {
                FlowDelta_X *= FLOW_FASTER;
                FlowDelta_Y *= FLOW_FASTER;
            }
            if (scroller == 219)
            {
                FlowDelta_X *= FLOW_FASTEST;
                FlowDelta_Y *= FLOW_FASTEST;
            }
            break;
        }

        // Scroll SOUTH
        case 220: case 221: case 222: case 223: case 224:
        {
            FlowDelta_X = 0;
            FlowDelta_Y = -FlowFactor_Y;
            if (scroller == 220)
            {
                FlowDelta_Y *= FLOW_SLOWEST;
            }
            if (scroller == 221)
            {
                FlowDelta_Y *= FLOW_SLOWER;
            }
            if (scroller == 222)
            {
                FlowDelta_Y *= FLOW_NORMAL;
            }
            if (scroller == 223)
            {
                FlowDelta_Y *= FLOW_FASTER;
            }
            if (scroller == 224)
            {
                FlowDelta_Y *= FLOW_FASTEST;
            }
            break;
        }

        // Scroll SOUTH-WEST
        case 225: case 226: case 227: case 228: case 229:
        {
            FlowDelta_X = FlowFactor_X;
            FlowDelta_Y = -FlowFactor_Y;
            if (scroller == 225)
            {
                FlowDelta_X *= FLOW_SLOWEST;
                FlowDelta_Y *= FLOW_SLOWEST;
            }
            if (scroller == 226)
            {
                FlowDelta_X *= FLOW_SLOWER;
                FlowDelta_Y *= FLOW_SLOWER;
            }
            if (scroller == 227)
            {
                FlowDelta_X *= FLOW_NORMAL;
                FlowDelta_Y *= FLOW_NORMAL;
            }
            if (scroller == 228)
            {
                FlowDelta_X *= FLOW_FASTER;
                FlowDelta_Y *= FLOW_FASTER;
            }
            if (scroller == 229)
            {
                FlowDelta_X *= FLOW_FASTEST;
                FlowDelta_Y *= FLOW_FASTEST;
            }
            break;
        }

        // Scroll WEST
        case 230: case 231: case 232: case 233: case 234:
        {
            FlowDelta_X = FlowFactor_X;
            FlowDelta_Y = 0;
            if (scroller == 230)
            {
                FlowDelta_X *= FLOW_SLOWEST;
            }
            if (scroller == 231)
            {
                FlowDelta_X *= FLOW_SLOWER;
            }
            if (scroller == 232)
            {
                FlowDelta_X *= FLOW_NORMAL;
            }
            if (scroller == 233)
            {
                FlowDelta_X *= FLOW_FASTER;
            }
            if (scroller == 234)
            {
                FlowDelta_X *= FLOW_FASTEST;
            }
            break;
        }

        // Scroll NORTH-WEST
        case 235: case 236: case 237: case 238: case 239:
        {
            FlowDelta_X = FlowFactor_X;
            FlowDelta_Y = FlowFactor_Y;
            if (scroller == 235)
            {
                FlowDelta_X *= FLOW_SLOWEST;
                FlowDelta_Y *= FLOW_SLOWEST;
            }
            if (scroller == 236)
            {
                FlowDelta_X *= FLOW_SLOWER;
                FlowDelta_Y *= FLOW_SLOWER;
            }
            if (scroller == 237)
            {
                FlowDelta_X *= FLOW_NORMAL;
                FlowDelta_Y *= FLOW_NORMAL;
            }
            if (scroller == 238)
            {
                FlowDelta_X *= FLOW_FASTER;
                FlowDelta_Y *= FLOW_FASTER;
            }
            if (scroller == 239)
            {
                FlowDelta_X *= FLOW_FASTEST;
                FlowDelta_Y *= FLOW_FASTEST;
            }
            break;
        }

        default:
        {
            FlowDelta_X = 0;
            FlowDelta_Y = 0;
            break;
        }            
    }
}

// -----------------------------------------------------------------------------
// [JN] Inject scroller specials to vanilla maps.
// -----------------------------------------------------------------------------

flow_t flow[] =
{
    //  mission, episode, map, sector,   floorpic, scroller
    {      doom,       1,   1,      0,         "",      102 },
    {      doom,       1,   1,     51,         "",      101 },
    {      doom,       1,   1,     53,         "",      101 },
    {      doom,       1,   1,     57,         "",      101 },
    {      doom,       1,   2,     39,         "",      102 },
    {      doom,       1,   2,    141,         "",      101 },
    {      doom,       1,   2,    144,         "",      101 },
    {      doom,       1,   2,    163,         "",      101 },
    {      doom,       1,   2,    175,         "",      100 },
    {      doom,       1,   3,     39,         "",      101 },
    {      doom,       1,   3,     46,         "",      101 },    
    {      doom,       1,   3,     38,         "",      101 },
    {      doom,       1,   3,     58,         "",      100 },
    {      doom,       1,   3,     65,         "",      101 },
    {      doom,       1,   3,     66,         "",      101 },
    {      doom,       1,   3,     68,         "",      101 },
    {      doom,       1,   3,    153,         "",      100 },
    {      doom,       1,   3,    155,         "",      100 },
    {      doom,       1,   3,    156,         "",      100 },
    {      doom,       1,   3,    157,         "",      100 },
    {      doom,       1,   3,    158,         "",      100 },
    {      doom,       1,   3,    161,         "",      100 },
    {      doom,       1,   4,      5,         "",      100 },
    {      doom,       1,   4,      7,         "",      100 },
    {      doom,       1,   4,     42,         "",      216 },
    {      doom,       1,   4,     95,         "",      212 },
    {      doom,       1,   4,     98,         "",      212 },
    {      doom,       1,   4,    107,         "",      100 },
    {      doom,       1,   4,    110,         "",      212 },
    {      doom,       1,   4,    135,         "",      100 },
    {      doom,       1,   5,     19,         "",      102 },
    {      doom,       1,   5,     33,         "",      232 },
    {      doom,       1,   5,     59,         "",      232 },
    {      doom,       1,   5,     61,         "",      232 },
    {      doom,       1,   5,     72,         "",      102 },
    {      doom,       1,   5,     90,         "",      102 },
    {      doom,       1,   5,     91,         "",      102 },
    {      doom,       1,   6,     34,         "",      101 },
    {      doom,       1,   6,     52,         "",      100 },
    {      doom,       1,   6,     57,         "",      101 },
    {      doom,       1,   6,     64,         "",      100 },
    {      doom,       1,   6,     81,         "",      100 },
    {      doom,       1,   6,    118,         "",      101 },
    {      doom,       1,   6,    119,         "",      101 },
    {      doom,       1,   6,    145,         "",      100 },
    {      doom,       1,   6,    146,         "",      100 },
    {      doom,       1,   6,    149,         "",      101 },
    {      doom,       1,   6,    150,         "",      101 },
    {      doom,       1,   6,    153,         "",      101 },
    {      doom,       1,   6,    178,         "",      102 },
    {      doom,       1,   6,    183,         "",      102 },
    {      doom,       1,   6,    184,         "",      102 },
    {      doom,       1,   6,    222,         "",      100 },
    {      doom,       1,   6,    224,         "",      100 },
    {      doom,       1,   6,    225,         "",      100 },
    {      doom,       1,   6,    228,         "",      100 },
    {      doom,       1,   6,    230,         "",      100 },
    {      doom,       1,   6,    231,         "",      100 },
    {      doom,       1,   6,    236,         "",      100 },
    {      doom,       1,   6,    234,         "",      100 },
    {      doom,       1,   6,    237,         "",      100 },
    {      doom,       1,   6,    239,         "",      100 },
    {      doom,       1,   6,    243,         "",      100 },
    {      doom,       1,   6,    245,         "",      100 },
    {      doom,       1,   6,    246,         "",      100 },
    {      doom,       1,   6,    247,         "",      100 },
    {      doom,       1,   6,    249,         "",      100 },
    {      doom,       1,   7,      3,         "",      102 },
    {      doom,       1,   7,     74,         "",      102 },
    {      doom,       1,   7,    119,         "",      102 },
    {      doom,       1,   7,    120,         "",      102 },
    {      doom,       1,   7,    145,         "",      102 },
    {      doom,       1,   7,    150,         "",      102 },
    {      doom,       1,   7,    151,         "",      102 },
    {      doom,       1,   7,    152,         "",      102 },
    {      doom,       1,   9,     14,         "",      102 },
    {      doom,       1,   9,     17,         "",      102 },
    {      doom,       1,   9,     21,         "",      102 },
    {      doom,       1,   9,     37,         "",      100 },
    {      doom,       1,   9,     47,         "",      102 },
    {      doom,       1,   9,     48,         "",      102 },
    {      doom,       1,   9,     52,         "",      102 },
    {      doom,       1,   9,     62,         "",      101 },
    {      doom,       1,   9,     63,         "",      101 },
    {      doom,       1,   9,     66,         "",      101 },
    {      doom,       1,   9,     73,         "",      102 },
    {      doom,       2,   1,     79,         "",      101 },
    {      doom,       2,   1,     81,         "",      101 },
    {      doom,       2,   1,     82,         "",      101 },
    {      doom,       2,   1,     88,         "",      101 },
    {      doom,       2,   1,     90,         "",      101 },
    {      doom,       2,   2,      0,         "",      202 },
    {      doom,       2,   2,      1,         "",      202 },
    {      doom,       2,   2,     14,         "",      202 },
    {      doom,       2,   2,     15,         "",      202 },
    {      doom,       2,   2,     71,         "",      101 },
    {      doom,       2,   2,     72,         "",      101 },
    {      doom,       2,   2,     73,         "",      101 },
    {      doom,       2,   2,     74,         "",      101 },
    {      doom,       2,   2,     75,         "",      101 },
    {      doom,       2,   2,     76,         "",      101 },
    {      doom,       2,   2,     79,         "",      101 },
    {      doom,       2,   2,     80,         "",      101 },
    {      doom,       2,   2,    104,         "",      102 },
    {      doom,       2,   3,     11,         "",      101 },
    {      doom,       2,   3,     15,         "",      101 },
    {      doom,       2,   3,     16,         "",      101 },
    {      doom,       2,   3,     34,         "",      100 },
    {      doom,       2,   3,     46,         "",      101 },
    {      doom,       2,   3,     54,         "",      101 },
    {      doom,       2,   3,     99,         "",      101 },
    {      doom,       2,   3,    116,         "",      102 },
    {      doom,       2,   3,    120,         "",      102 },
    {      doom,       2,   3,    122,         "",      102 },
    {      doom,       2,   3,    123,         "",      102 },
    {      doom,       2,   4,     12,         "",      101 },
    {      doom,       2,   4,     35,         "",      101 },
    {      doom,       2,   4,     37,         "",      100 },
    {      doom,       2,   4,     51,         "",      100 },
    {      doom,       2,   4,    111,         "",      102 },
    {      doom,       2,   4,    156,         "",      102 },
    {      doom,       2,   4,    162,         "",      101 },
    {      doom,       2,   4,    163,         "",      101 },
    {      doom,       2,   4,    173,         "",      102 },
    {      doom,       2,   4,    174,         "",      102 },
    {      doom,       2,   4,    175,         "",      102 },
    {      doom,       2,   4,    177,         "",      102 },
    {      doom,       2,   4,    178,         "",      102 },
    {      doom,       2,   4,    179,         "",      102 },
    {      doom,       2,   4,    180,         "",      102 },
    {      doom,       2,   5,     49,         "",      222 },
    {      doom,       2,   5,     63,         "",      102 },
    {      doom,       2,   5,     64,         "",      102 },
    {      doom,       2,   5,     72,         "",      102 },
    {      doom,       2,   5,    134,         "",      102 },
    {      doom,       2,   5,    135,         "",      102 },
    {      doom,       2,   5,    139,         "",      102 },
    {      doom,       2,   5,    162,         "",      102 },
    {      doom,       2,   5,    177,         "",      102 },
    {      doom,       2,   5,    212,         "",      102 },
    {      doom,       2,   5,    232,         "",      102 },
    {      doom,       2,   6,     55,         "",      102 },
    {      doom,       2,   6,     56,         "",      102 },
    {      doom,       2,   6,     57,         "",      102 },
    {      doom,       2,   6,     59,         "",      102 },
    {      doom,       2,   6,     62,         "",      102 },
    {      doom,       2,   6,     63,         "",      102 },
    {      doom,       2,   6,     64,         "",      102 },
    {      doom,       2,   7,      0,         "",      101 },
    {      doom,       2,   7,     29,         "",      101 },
    {      doom,       2,   7,     30,         "",      101 },
    {      doom,       2,   7,     31,         "",      101 },
    {      doom,       2,   7,     47,         "",      101 },
    {      doom,       2,   7,     49,         "",      100 },
    {      doom,       2,   7,     53,         "",      100 },
    {      doom,       2,   7,     61,         "",      100 },
    {      doom,       2,   7,     62,         "",      100 },
    {      doom,       2,   7,     74,         "",      100 },
    {      doom,       2,   7,     75,         "",      100 },
    {      doom,       2,   7,     76,         "",      100 },
    {      doom,       2,   7,    155,         "",      101 },
    {      doom,       2,   7,    156,         "",      101 },
    {      doom,       2,   7,    279,         "",      101 },
    {      doom,       2,   9,      0,         "",      100 },
    {      doom,       2,   9,      1,         "",      100 },
    {      doom,       3,   1,      0,         "",      102 },
    {      doom,       3,   1,      7,         "",      102 },
    {      doom,       3,   1,     14,         "",      102 },
    {      doom,       3,   1,     15,         "",      102 },
    {      doom,       3,   1,     16,         "",      102 },
    {      doom,       3,   1,     17,         "",      102 },
    {      doom,       3,   1,     18,         "",      102 },
    {      doom,       3,   1,     19,         "",      102 },
    {      doom,       3,   2,      3,         "",      100 },
    {      doom,       3,   2,     12,         "",      100 },
    {      doom,       3,   2,     17,         "",      210 },
    {      doom,       3,   2,     20,         "",      210 },
    {      doom,       3,   2,     23,         "",      100 },
    {      doom,       3,   2,     26,         "",      101 },
    {      doom,       3,   2,     44,         "",      101 },
    {      doom,       3,   2,     66,         "",      101 },
    {      doom,       3,   2,     67,         "",      100 },
    {      doom,       3,   3,     11,         "",      212 },
    {      doom,       3,   3,     12,         "",      212 },
    {      doom,       3,   3,     13,         "",      101 },
    {      doom,       3,   3,     15,         "",      101 },
    {      doom,       3,   3,     51,         "",      100 },
    {      doom,       3,   3,     64,         "",      101 },
    {      doom,       3,   3,     70,         "",      101 },
    {      doom,       3,   3,    113,         "",      102 },
    {      doom,       3,   3,    145,         "",      102 },
    {      doom,       3,   3,    157,         "",      101 },
    {      doom,       3,   3,    161,         "",      101 },
    {      doom,       3,   3,    183,         "",      101 },
    {      doom,       3,   4,     27,         "",      101 },
    {      doom,       3,   4,     37,         "",      101 },
    {      doom,       3,   4,     39,         "",      101 },
    {      doom,       3,   4,     43,         "",      101 },
    {      doom,       3,   4,     74,         "",      101 },
    {      doom,       3,   4,     99,         "",      100 },
    {      doom,       3,   4,    105,         "",      102 },
    {      doom,       3,   4,    139,         "",      100 },
    {      doom,       3,   5,      0,         "",      100 },
    {      doom,       3,   5,     43,         "",      101 },
    {      doom,       3,   5,     45,         "",      101 },
    {      doom,       3,   5,    235,         "",      101 },
    {      doom,       3,   5,    243,         "",      101 },
    {      doom,       3,   6,      4,         "",      102 },
    {      doom,       3,   6,     14,         "",      102 },
    {      doom,       3,   6,     15,         "",      102 },
    {      doom,       3,   6,     16,         "",      102 },
    {      doom,       3,   6,     19,         "",      102 },
    {      doom,       3,   6,     23,         "",      102 },
    {      doom,       3,   6,     24,         "",      102 },
    {      doom,       3,   6,     25,         "",      101 },
    {      doom,       3,   6,     26,         "",      101 },
    {      doom,       3,   6,     27,         "",      101 },
    {      doom,       3,   6,     28,         "",      101 },
    {      doom,       3,   6,     29,         "",      101 },
    {      doom,       3,   6,     32,         "",      102 },
    {      doom,       3,   6,     33,         "",      102 },
    {      doom,       3,   6,     35,         "",      102 },
    {      doom,       3,   6,     40,         "",      102 },
    {      doom,       3,   6,     83,         "",      102 },
    {      doom,       3,   6,     84,         "",      102 },
    {      doom,       3,   6,     85,         "",      102 },
    {      doom,       3,   6,     88,         "",      101 },
    {      doom,       3,   6,     90,         "",      102 },
    {      doom,       3,   6,     91,         "",      102 },
    {      doom,       3,   6,     93,         "",      101 },
    {      doom,       3,   6,    115,         "",      102 },
    {      doom,       3,   6,    123,         "",      102 },
    {      doom,       3,   6,    124,         "",      102 },
    {      doom,       3,   7,      0,         "",      101 },
    {      doom,       3,   7,      1,         "",      101 },
    {      doom,       3,   7,      2,         "",      101 },
    {      doom,       3,   7,     46,         "",      101 },
    {      doom,       3,   7,     47,         "",      101 },
    {      doom,       3,   7,     49,         "",      102 },
    {      doom,       3,   7,     57,         "",      102 },
    {      doom,       3,   7,     62,         "",      101 },
    {      doom,       3,   7,     64,         "",      101 },
    {      doom,       3,   7,     65,         "",      101 },
    {      doom,       3,   7,     67,         "",      102 },
    {      doom,       3,   7,     71,         "",      102 },
    {      doom,       3,   7,     73,         "",      101 },
    {      doom,       3,   7,     76,         "",      101 },
    {      doom,       3,   7,     79,         "",      101 },
    {      doom,       3,   7,     16,         "",      102 },
    {      doom,       3,   7,     81,         "",      216 },
    {      doom,       3,   7,     83,         "",      226 },
    {      doom,       3,   7,     85,         "",      102 },
    {      doom,       3,   7,     91,         "",      201 },
    {      doom,       3,   7,     92,         "",      236 },
    {      doom,       3,   7,     94,         "",      206 },
    {      doom,       3,   7,    100,         "",      102 },
    {      doom,       3,   7,    103,         "",      101 },
    {      doom,       3,   7,    104,         "",      101 },
    {      doom,       3,   7,    118,         "",      102 },
    {      doom,       3,   7,    121,         "",      102 },
    {      doom,       3,   7,    131,         "",      102 },
    {      doom,       3,   7,    135,         "",      102 },
    {      doom,       3,   7,    140,         "",      101 },
    {      doom,       3,   7,    142,         "",      101 },
    {      doom,       3,   7,    148,         "",      101 },
    {      doom,       3,   9,      0,         "",      102 },
    {      doom,       3,   9,      1,         "",      102 },
    {      doom,       3,   9,      2,         "",      102 },
    {      doom,       3,   9,      5,         "",      102 },
    {      doom,       3,   9,      6,         "",      102 },
    {      doom,       3,   9,      7,         "",      102 },
    {      doom,       3,   9,      8,         "",      102 },
    {      doom,       3,   9,      9,         "",      102 },
    {      doom,       3,   9,     11,         "",      102 },
    {      doom,       3,   9,     22,         "",      102 },
    {      doom,       3,   9,     30,         "",      102 },
    {      doom,       3,   9,     31,         "",      102 },
    {      doom,       3,   9,     38,         "",      102 },

    {     doom2,       1,   1,     14,         "",      236 },

    {        -1,       0,   0,      0,         "",    32767 },
};
