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

fixed_t FlowFactor_X, FlowFactor_X_old;
fixed_t FlowFactor_Y, FlowFactor_Y_old;
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
    //  mission, episode, map, sector, flow
    {      doom,       1,   1,      0,  102 },
    {      doom,       1,   1,     51,  101 },
    {      doom,       1,   1,     53,  101 },
    {      doom,       1,   1,     57,  101 },
    {      doom,       1,   2,     39,  102 },
    {      doom,       1,   2,    141,  101 },
    {      doom,       1,   2,    144,  101 },
    {      doom,       1,   2,    163,  101 },
    {      doom,       1,   2,    175,  100 },
    {      doom,       1,   3,     39,  101 },
    {      doom,       1,   3,     46,  101 },    
    {      doom,       1,   3,     38,  101 },
    {      doom,       1,   3,     58,  100 },
    {      doom,       1,   3,     65,  101 },
    {      doom,       1,   3,     66,  101 },
    {      doom,       1,   3,     68,  101 },
    {      doom,       1,   3,    153,  100 },
    {      doom,       1,   3,    155,  100 },
    {      doom,       1,   3,    156,  100 },
    {      doom,       1,   3,    157,  100 },
    {      doom,       1,   3,    158,  100 },
    {      doom,       1,   3,    161,  100 },
    {      doom,       1,   4,      5,  100 },
    {      doom,       1,   4,      7,  100 },
    {      doom,       1,   4,     42,  216 },
    {      doom,       1,   4,     95,  212 },
    {      doom,       1,   4,     98,  212 },
    {      doom,       1,   4,    107,  100 },
    {      doom,       1,   4,    110,  212 },
    {      doom,       1,   4,    135,  100 },
    {      doom,       1,   5,     19,  102 },
    {      doom,       1,   5,     33,  232 },
    {      doom,       1,   5,     59,  232 },
    {      doom,       1,   5,     61,  232 },
    {      doom,       1,   5,     72,  102 },
    {      doom,       1,   5,     90,  102 },
    {      doom,       1,   5,     91,  102 },
    {      doom,       1,   6,     34,  101 },
    {      doom,       1,   6,     52,  100 },
    {      doom,       1,   6,     57,  101 },
    {      doom,       1,   6,     64,  100 },
    {      doom,       1,   6,     81,  100 },
    {      doom,       1,   6,    118,  101 },
    {      doom,       1,   6,    119,  101 },
    {      doom,       1,   6,    145,  100 },
    {      doom,       1,   6,    146,  100 },
    {      doom,       1,   6,    149,  101 },
    {      doom,       1,   6,    150,  101 },
    {      doom,       1,   6,    153,  101 },
    {      doom,       1,   6,    178,  102 },
    {      doom,       1,   6,    183,  102 },
    {      doom,       1,   6,    184,  102 },
    {      doom,       1,   6,    222,  100 },
    {      doom,       1,   6,    224,  100 },
    {      doom,       1,   6,    225,  100 },
    {      doom,       1,   6,    228,  100 },
    {      doom,       1,   6,    230,  100 },
    {      doom,       1,   6,    231,  100 },
    {      doom,       1,   6,    236,  100 },
    {      doom,       1,   6,    234,  100 },
    {      doom,       1,   6,    237,  100 },
    {      doom,       1,   6,    239,  100 },
    {      doom,       1,   6,    243,  100 },
    {      doom,       1,   6,    245,  100 },
    {      doom,       1,   6,    246,  100 },
    {      doom,       1,   6,    247,  100 },
    {      doom,       1,   6,    249,  100 },
    {      doom,       1,   7,      3,  102 },
    {      doom,       1,   7,     74,  102 },
    {      doom,       1,   7,    119,  102 },
    {      doom,       1,   7,    120,  102 },
    {      doom,       1,   7,    145,  102 },
    {      doom,       1,   7,    150,  102 },
    {      doom,       1,   7,    151,  102 },
    {      doom,       1,   7,    152,  102 },
    {      doom,       1,   9,     14,  102 },
    {      doom,       1,   9,     17,  102 },
    {      doom,       1,   9,     21,  102 },
    {      doom,       1,   9,     37,  100 },
    {      doom,       1,   9,     47,  102 },
    {      doom,       1,   9,     48,  102 },
    {      doom,       1,   9,     52,  102 },
    {      doom,       1,   9,     62,  101 },
    {      doom,       1,   9,     63,  101 },
    {      doom,       1,   9,     66,  101 },
    {      doom,       1,   9,     73,  102 },
    {      doom,       2,   1,     79,  101 },
    {      doom,       2,   1,     81,  101 },
    {      doom,       2,   1,     82,  101 },
    {      doom,       2,   1,     88,  101 },
    {      doom,       2,   1,     90,  101 },
    {      doom,       2,   2,      0,  202 },
    {      doom,       2,   2,      1,  202 },
    {      doom,       2,   2,     14,  202 },
    {      doom,       2,   2,     15,  202 },
    {      doom,       2,   2,     71,  101 },
    {      doom,       2,   2,     72,  101 },
    {      doom,       2,   2,     73,  101 },
    {      doom,       2,   2,     74,  101 },
    {      doom,       2,   2,     75,  101 },
    {      doom,       2,   2,     76,  101 },
    {      doom,       2,   2,     79,  101 },
    {      doom,       2,   2,     80,  101 },
    {      doom,       2,   2,    104,  102 },
    {      doom,       2,   3,     11,  101 },
    {      doom,       2,   3,     15,  101 },
    {      doom,       2,   3,     16,  101 },
    {      doom,       2,   3,     34,  100 },
    {      doom,       2,   3,     46,  101 },
    {      doom,       2,   3,     54,  101 },
    {      doom,       2,   3,     99,  101 },
    {      doom,       2,   3,    116,  102 },
    {      doom,       2,   3,    120,  102 },
    {      doom,       2,   3,    122,  102 },
    {      doom,       2,   3,    123,  102 },
    {      doom,       2,   4,     12,  101 },
    {      doom,       2,   4,     35,  101 },
    {      doom,       2,   4,     37,  100 },
    {      doom,       2,   4,     51,  100 },
    {      doom,       2,   4,    111,  102 },
    {      doom,       2,   4,    156,  102 },
    {      doom,       2,   4,    162,  101 },
    {      doom,       2,   4,    163,  101 },
    {      doom,       2,   4,    173,  102 },
    {      doom,       2,   4,    174,  102 },
    {      doom,       2,   4,    175,  102 },
    {      doom,       2,   4,    177,  102 },
    {      doom,       2,   4,    178,  102 },
    {      doom,       2,   4,    179,  102 },
    {      doom,       2,   4,    180,  102 },
    {      doom,       2,   5,     49,  222 },
    {      doom,       2,   5,     63,  102 },
    {      doom,       2,   5,     64,  102 },
    {      doom,       2,   5,     72,  102 },
    {      doom,       2,   5,    134,  102 },
    {      doom,       2,   5,    135,  102 },
    {      doom,       2,   5,    139,  102 },
    {      doom,       2,   5,    162,  102 },
    {      doom,       2,   5,    177,  102 },
    {      doom,       2,   5,    212,  102 },
    {      doom,       2,   5,    232,  102 },
    {      doom,       2,   6,     55,  102 },
    {      doom,       2,   6,     56,  102 },
    {      doom,       2,   6,     57,  102 },
    {      doom,       2,   6,     59,  102 },
    {      doom,       2,   6,     62,  102 },
    {      doom,       2,   6,     63,  102 },
    {      doom,       2,   6,     64,  102 },
    {      doom,       2,   7,      0,  101 },
    {      doom,       2,   7,     29,  101 },
    {      doom,       2,   7,     30,  101 },
    {      doom,       2,   7,     31,  101 },
    {      doom,       2,   7,     47,  101 },
    {      doom,       2,   7,     49,  100 },
    {      doom,       2,   7,     53,  100 },
    {      doom,       2,   7,     61,  100 },
    {      doom,       2,   7,     62,  100 },
    {      doom,       2,   7,     74,  100 },
    {      doom,       2,   7,     75,  100 },
    {      doom,       2,   7,     76,  100 },
    {      doom,       2,   7,    155,  101 },
    {      doom,       2,   7,    156,  101 },
    {      doom,       2,   7,    279,  101 },
    {      doom,       2,   9,      0,  100 },
    {      doom,       2,   9,      1,  100 },
    {      doom,       3,   1,      0,  102 },
    {      doom,       3,   1,      7,  102 },
    {      doom,       3,   1,     14,  102 },
    {      doom,       3,   1,     15,  102 },
    {      doom,       3,   1,     16,  102 },
    {      doom,       3,   1,     17,  102 },
    {      doom,       3,   1,     18,  102 },
    {      doom,       3,   1,     19,  102 },
    {      doom,       3,   2,      3,  100 },
    {      doom,       3,   2,     12,  100 },
    {      doom,       3,   2,     17,  210 },
    {      doom,       3,   2,     20,  210 },
    {      doom,       3,   2,     23,  100 },
    {      doom,       3,   2,     26,  101 },
    {      doom,       3,   2,     44,  101 },
    {      doom,       3,   2,     66,  101 },
    {      doom,       3,   2,     67,  100 },
    {      doom,       3,   3,     11,  212 },
    {      doom,       3,   3,     12,  212 },
    {      doom,       3,   3,     13,  101 },
    {      doom,       3,   3,     15,  101 },
    {      doom,       3,   3,     51,  100 },
    {      doom,       3,   3,     64,  101 },
    {      doom,       3,   3,     70,  101 },
    {      doom,       3,   3,    113,  102 },
    {      doom,       3,   3,    145,  102 },
    {      doom,       3,   3,    157,  101 },
    {      doom,       3,   3,    161,  101 },
    {      doom,       3,   3,    183,  101 },
    {      doom,       3,   4,     27,  101 },
    {      doom,       3,   4,     37,  101 },
    {      doom,       3,   4,     39,  101 },
    {      doom,       3,   4,     43,  101 },
    {      doom,       3,   4,     74,  101 },
    {      doom,       3,   4,     99,  100 },
    {      doom,       3,   4,    105,  102 },
    {      doom,       3,   4,    139,  100 },
    {      doom,       3,   5,      0,  100 },
    {      doom,       3,   5,     43,  101 },
    {      doom,       3,   5,     45,  101 },
    {      doom,       3,   5,    235,  101 },
    {      doom,       3,   5,    243,  101 },
    {      doom,       3,   6,      4,  102 },
    {      doom,       3,   6,     14,  102 },
    {      doom,       3,   6,     15,  102 },
    {      doom,       3,   6,     16,  102 },
    {      doom,       3,   6,     19,  102 },
    {      doom,       3,   6,     23,  102 },
    {      doom,       3,   6,     24,  102 },
    {      doom,       3,   6,     25,  101 },
    {      doom,       3,   6,     26,  101 },
    {      doom,       3,   6,     27,  101 },
    {      doom,       3,   6,     28,  101 },
    {      doom,       3,   6,     29,  101 },
    {      doom,       3,   6,     32,  102 },
    {      doom,       3,   6,     33,  102 },
    {      doom,       3,   6,     35,  102 },
    {      doom,       3,   6,     40,  102 },
    {      doom,       3,   6,     83,  102 },
    {      doom,       3,   6,     84,  102 },
    {      doom,       3,   6,     85,  102 },
    {      doom,       3,   6,     88,  101 },
    {      doom,       3,   6,     90,  102 },
    {      doom,       3,   6,     91,  102 },
    {      doom,       3,   6,     93,  101 },
    {      doom,       3,   6,    115,  102 },
    {      doom,       3,   6,    123,  102 },
    {      doom,       3,   6,    124,  102 },
    {      doom,       3,   7,      0,  101 },
    {      doom,       3,   7,      1,  101 },
    {      doom,       3,   7,      2,  101 },
    {      doom,       3,   7,     46,  101 },
    {      doom,       3,   7,     47,  101 },
    {      doom,       3,   7,     49,  102 },
    {      doom,       3,   7,     57,  102 },
    {      doom,       3,   7,     62,  101 },
    {      doom,       3,   7,     64,  101 },
    {      doom,       3,   7,     65,  101 },
    {      doom,       3,   7,     67,  102 },
    {      doom,       3,   7,     71,  102 },
    {      doom,       3,   7,     73,  101 },
    {      doom,       3,   7,     76,  101 },
    {      doom,       3,   7,     79,  101 },
    {      doom,       3,   7,     16,  102 },
    {      doom,       3,   7,     81,  216 },
    {      doom,       3,   7,     83,  226 },
    {      doom,       3,   7,     85,  102 },
    {      doom,       3,   7,     91,  201 },
    {      doom,       3,   7,     92,  236 },
    {      doom,       3,   7,     94,  206 },
    {      doom,       3,   7,    100,  102 },
    {      doom,       3,   7,    103,  101 },
    {      doom,       3,   7,    104,  101 },
    {      doom,       3,   7,    118,  102 },
    {      doom,       3,   7,    121,  102 },
    {      doom,       3,   7,    131,  102 },
    {      doom,       3,   7,    135,  102 },
    {      doom,       3,   7,    140,  101 },
    {      doom,       3,   7,    142,  101 },
    {      doom,       3,   7,    148,  101 },
    {      doom,       3,   9,      0,  102 },
    {      doom,       3,   9,      1,  102 },
    {      doom,       3,   9,      2,  102 },
    {      doom,       3,   9,      5,  102 },
    {      doom,       3,   9,      6,  102 },
    {      doom,       3,   9,      7,  102 },
    {      doom,       3,   9,      8,  102 },
    {      doom,       3,   9,      9,  102 },
    {      doom,       3,   9,     11,  102 },
    {      doom,       3,   9,     22,  102 },
    {      doom,       3,   9,     30,  102 },
    {      doom,       3,   9,     31,  102 },
    {      doom,       3,   9,     38,  102 },
    {      doom,       4,   1,     16,  101 },
    {      doom,       4,   1,     17,  100 },
    {      doom,       4,   1,     28,  101 },
    {      doom,       4,   1,     31,  232 },
    {      doom,       4,   1,     32,  232 },
    {      doom,       4,   1,     33,  232 },
    {      doom,       4,   1,     36,  101 },
    {      doom,       4,   1,     37,  101 },
    {      doom,       4,   1,     38,  101 },
    {      doom,       4,   1,     39,  101 },
    {      doom,       4,   1,     40,  101 },
    {      doom,       4,   1,     41,  101 },
    {      doom,       4,   1,     42,  101 },
    {      doom,       4,   1,     43,  101 },
    {      doom,       4,   1,     44,  101 },
    {      doom,       4,   1,     71,  101 },
    {      doom,       4,   1,     72,  101 },
    {      doom,       4,   1,     73,  101 },
    {      doom,       4,   1,     76,  101 },
    {      doom,       4,   2,      0,  102 },
    {      doom,       4,   2,      3,  102 },
    {      doom,       4,   2,      4,  102 },
    {      doom,       4,   2,      8,  101 },
    {      doom,       4,   2,     12,  102 },
    {      doom,       4,   2,     14,  102 },
    {      doom,       4,   2,     35,  101 },
    {      doom,       4,   2,     34,  101 },
    {      doom,       4,   2,     36,  101 },
    {      doom,       4,   2,     37,  101 },
    {      doom,       4,   2,     42,  101 },
    {      doom,       4,   2,     43,  101 },
    {      doom,       4,   2,     44,  101 },
    {      doom,       4,   2,     45,  101 },
    {      doom,       4,   2,     46,  101 },
    {      doom,       4,   2,     47,  101 },
    {      doom,       4,   2,     49,  102 },
    {      doom,       4,   2,     51,  231 },
    {      doom,       4,   2,     54,  231 },
    {      doom,       4,   2,     56,  226 },
    {      doom,       4,   2,    108,  226 },
    {      doom,       4,   2,    112,  231 },
    {      doom,       4,   3,     12,  232 },
    {      doom,       4,   3,     35,  232 },
    {      doom,       4,   3,     39,  232 },
    {      doom,       4,   3,    117,  101 },
    {      doom,       4,   4,      5,  101 },
    {      doom,       4,   4,      6,  101 },
    {      doom,       4,   4,      7,  101 },
    {      doom,       4,   4,      9,  101 },
    {      doom,       4,   4,     71,  101 },
    {      doom,       4,   4,     75,  101 },
    {      doom,       4,   4,     91,  102 },
    {      doom,       4,   4,     98,  102 },
    {      doom,       4,   4,    100,  102 },
    {      doom,       4,   4,    119,  101 },
    {      doom,       4,   5,      4,  232 },
    {      doom,       4,   5,      6,  222 },
    {      doom,       4,   5,      7,  232 },
    {      doom,       4,   5,     11,  232 },
    {      doom,       4,   5,     22,  232 },
    {      doom,       4,   5,     49,  102 },
    {      doom,       4,   5,     56,  212 },
    {      doom,       4,   5,     57,  102 },
    {      doom,       4,   5,     61,  102 },
    {      doom,       4,   5,     86,  232 },
    {      doom,       4,   5,     87,  222 },
    {      doom,       4,   5,     88,  222 },
    {      doom,       4,   5,     97,  222 },
    {      doom,       4,   5,    106,  102 },
    {      doom,       4,   5,    110,  102 },
    {      doom,       4,   5,    120,  206 },
    {      doom,       4,   5,    124,  102 },
    {      doom,       4,   5,    132,  101 },
    {      doom,       4,   5,    137,  102 },
    {      doom,       4,   5,    141,  211 },
    {      doom,       4,   5,    154,  102 },
    {      doom,       4,   5,    155,  202 },
    {      doom,       4,   6,      0,  102 },
    {      doom,       4,   6,      1,  102 },
    {      doom,       4,   6,     10,  102 },
    {      doom,       4,   6,     12,  102 },
    {      doom,       4,   6,     13,  102 },
    {      doom,       4,   6,     42,  102 },
    {      doom,       4,   6,     63,  102 },
    {      doom,       4,   6,     69,  102 },
    {      doom,       4,   6,    105,  102 },
    {      doom,       4,   6,    111,  102 },
    {      doom,       4,   6,    133,  102 },
    {      doom,       4,   6,    134,  102 },
    {      doom,       4,   6,    135,  102 },
    {      doom,       4,   6,    136,  102 },
    {      doom,       4,   6,    139,  102 },
    {      doom,       4,   6,    140,  102 },
    {      doom,       4,   7,     57,  232 },
    {      doom,       4,   7,     59,  232 },
    {      doom,       4,   7,     60,  232 },
    {      doom,       4,   7,     61,  232 },
    {      doom,       4,   7,     64,  232 },
    {      doom,       4,   7,     84,  101 },
    {      doom,       4,   7,     86,  101 },
    {      doom,       4,   7,     90,  101 },
    {      doom,       4,   7,     91,  101 },
    {      doom,       4,   7,    107,  222 },
    {      doom,       4,   7,    157,  222 },
    {      doom,       4,   7,    158,  222 },
    {      doom,       4,   7,    159,  222 },
    {      doom,       4,   7,    160,  222 },
    {      doom,       4,   7,    161,  232 },
    {      doom,       4,   7,    166,  222 },
    {      doom,       4,   7,    167,  232 },
    {      doom,       4,   7,    168,  232 },
    {      doom,       4,   7,    170,  232 },
    {      doom,       4,   7,    171,  232 },
    {      doom,       4,   7,    196,  232 },
    {      doom,       4,   7,    235,  232 },
    {      doom,       4,   7,    236,  232 },
    {      doom,       4,   8,     34,  102 },
    {      doom,       4,   8,     40,  102 },
    {      doom,       4,   8,     50,  102 },
    {      doom,       4,   8,    121,  222 },
    {      doom,       4,   8,    122,  222 },
    {      doom,       4,   8,    124,  222 },
    {      doom,       4,   8,    134,  216 },
    {      doom,       4,   8,    135,  216 },
    {      doom,       4,   8,    144,  222 },
    {      doom,       4,   8,    146,  222 },
    {      doom,       4,   8,    150,  222 },
    {      doom,       4,   8,    153,  222 },
    {      doom,       4,   8,    155,  222 },
    {      doom,       4,   8,    156,  222 },
    {      doom,       4,   8,    202,  216 },
    {      doom,       4,   9,      9,  102 },
    {      doom,       4,   9,     10,  102 },
    {      doom,       4,   9,    123,  101 },
    {      doom,       4,   9,    149,  101 },
    {      doom,       4,   9,    187,  102 },
    {      doom,       4,   9,    192,  101 },
    {      doom,       4,   9,    201,  102 },
    {      doom,       4,   9,    204,  102 },

    {     doom2,       1,   1,     14,  236 },
    {     doom2,       1,   2,      2,  101 },
    {     doom2,       1,   2,     11,  101 },
    {     doom2,       1,   2,     15,  101 },
    {     doom2,       1,   2,     16,  101 },
    {     doom2,       1,   2,     32,  101 },
    {     doom2,       1,   2,     33,  101 },
    {     doom2,       1,   2,     44,  201 },
    {     doom2,       1,   2,     55,  231 },
    {     doom2,       1,   2,     59,  100 },
    {     doom2,       1,   2,     95,  211 },
    {     doom2,       1,   2,     96,  100 },
    {     doom2,       1,   3,     15,  102 },
    {     doom2,       1,   3,     23,  221 },
    {     doom2,       1,   3,     30,  221 },
    {     doom2,       1,   3,     31,  221 },
    {     doom2,       1,   3,     32,  221 },
    {     doom2,       1,   3,     33,  102 },
    {     doom2,       1,   3,     34,  102 },
    {     doom2,       1,   3,     35,  102 },
    {     doom2,       1,   3,     58,  102 },
    {     doom2,       1,   3,     59,  102 },
    {     doom2,       1,   3,     60,  102 },
    {     doom2,       1,   3,     61,  102 },
    {     doom2,       1,   3,     63,  101 },
    {     doom2,       1,   3,     67,  101 },
    {     doom2,       1,   3,     83,  102 },
    {     doom2,       1,   3,     89,  101 },
    {     doom2,       1,   3,     97,  101 },
    {     doom2,       1,   3,    102,  101 },
    {     doom2,       1,   3,    108,  101 },
    {     doom2,       1,   3,    117,  100 },
    {     doom2,       1,   4,     24,  100 },
    {     doom2,       1,   4,     25,  100 },
    {     doom2,       1,   4,     26,  100 },
    {     doom2,       1,   5,      5,  231 },
    {     doom2,       1,   5,      6,  231 },
    {     doom2,       1,   5,      7,  231 },
    {     doom2,       1,   5,     10,  231 },
    {     doom2,       1,   5,     11,  231 },
    {     doom2,       1,   5,     12,  231 },
    {     doom2,       1,   5,     20,  101 },
    {     doom2,       1,   5,     22,  101 },
    {     doom2,       1,   5,     63,  101 },
    {     doom2,       1,   5,     65,  101 },
    {     doom2,       1,   5,     66,  101 },
    {     doom2,       1,   5,    141,  101 },
    {     doom2,       1,   5,    143,  101 },
    {     doom2,       1,   5,    145,  101 },
    {     doom2,       1,   5,    146,  101 },
    {     doom2,       1,   5,    147,  101 },
    {     doom2,       1,   6,     30,  101 },
    {     doom2,       1,   6,     31,  101 },
    {     doom2,       1,   6,     50,  101 },
    {     doom2,       1,   6,     74,  101 },
    {     doom2,       1,   6,    101,  212 },
    {     doom2,       1,   6,    168,  100 },
    {     doom2,       1,   8,      8,  221 },
    {     doom2,       1,   8,      9,  221 },
    {     doom2,       1,   8,     10,  221 },
    {     doom2,       1,   8,     11,  221 },
    {     doom2,       1,   8,     14,  221 },
    {     doom2,       1,   8,     38,  100 },
    {     doom2,       1,   8,     40,  100 },
    {     doom2,       1,   8,     41,  100 },
    {     doom2,       1,   8,     54,  100 },
    {     doom2,       1,   8,     88,  100 },
    {     doom2,       1,   8,     89,  100 },
    {     doom2,       1,   9,     15,  101 },
    {     doom2,       1,   9,     35,  100 },
    {     doom2,       1,   9,     39,  101 },
    {     doom2,       1,   9,     56,  101 },
    {     doom2,       1,   9,    104,  100 },
    {     doom2,       1,   9,    107,  100 },
    {     doom2,       1,   9,    108,  100 },
    {     doom2,       1,   9,    114,  101 },
    {     doom2,       1,   9,    126,  101 },
    {     doom2,       1,   9,    128,  101 },
    {     doom2,       1,  10,     98,  100 },
    {     doom2,       1,  10,    110,  100 },
    {     doom2,       1,  10,    113,  100 },
    {     doom2,       1,  10,    115,  100 },
    {     doom2,       1,  10,    127,  101 },
    {     doom2,       1,  10,    139,  101 },
    {     doom2,       1,  10,    140,  101 },
    {     doom2,       1,  11,      0,  102 },
    {     doom2,       1,  11,      9,  102 },
    {     doom2,       1,  11,     30,  101 },
    {     doom2,       1,  11,     38,  102 },
    {     doom2,       1,  11,     39,  102 },
    {     doom2,       1,  11,     45,  102 },
    {     doom2,       1,  11,     48,  102 },
    {     doom2,       1,  11,     49,  102 },
    {     doom2,       1,  11,     56,  102 },
    {     doom2,       1,  11,    109,  232 },
    {     doom2,       1,  11,    112,  232 },
    {     doom2,       1,  11,    113,  232 },
    {     doom2,       1,  11,    118,  232 },
    {     doom2,       1,  11,    119,  232 },
    {     doom2,       1,  11,    120,  232 },
    {     doom2,       1,  11,    121,  232 },
    {     doom2,       1,  11,    122,  232 },
    {     doom2,       1,  11,    127,  232 },
    {     doom2,       1,  12,     44,  100 },
    {     doom2,       1,  12,     91,  100 },
    {     doom2,       1,  12,     93,  100 },
    {     doom2,       1,  12,     94,  100 },
    {     doom2,       1,  12,     99,  100 },
    {     doom2,       1,  12,    100,  100 },
    {     doom2,       1,  12,    101,  100 },
    {     doom2,       1,  12,    102,  100 },
    {     doom2,       1,  12,    106,  100 },
    {     doom2,       1,  12,    107,  100 },
    {     doom2,       1,  14,      0,  102 },
    {     doom2,       1,  14,      5,  102 },
    {     doom2,       1,  14,     82,  102 },
    {     doom2,       1,  14,     90,  102 },
    {     doom2,       1,  14,     96,  102 },
    {     doom2,       1,  14,    124,  102 },
    {     doom2,       1,  14,    126,  102 },
    {     doom2,       1,  14,    129,  102 },
    {     doom2,       1,  14,    132,  102 },
    {     doom2,       1,  14,    134,  102 },
    {     doom2,       1,  14,    136,  102 },
    {     doom2,       1,  14,    209,  102 },
    {     doom2,       1,  14,    229,  102 },
    {     doom2,       1,  14,    256,  102 },
    {     doom2,       1,  14,    259,  102 },
    {     doom2,       1,  14,    260,  102 },
    {     doom2,       1,  14,    290,  102 },
    {     doom2,       1,  14,    295,  102 },
    {     doom2,       1,  14,    296,  102 },
    {     doom2,       1,  14,    297,  102 },
    {     doom2,       1,  15,     34,  100 },
    {     doom2,       1,  15,     36,  100 },
    {     doom2,       1,  15,    103,  101 },
    {     doom2,       1,  15,    114,  234 },
    {     doom2,       1,  15,    118,  100 },
    {     doom2,       1,  15,    221,  100 },
    {     doom2,       1,  16,     16,  101 },
    {     doom2,       1,  16,     67,  206 },
    {     doom2,       1,  16,     68,  206 },
    {     doom2,       1,  16,     78,  206 },
    {     doom2,       1,  17,      0,  101 },
    {     doom2,       1,  17,     25,  101 },
    {     doom2,       1,  17,     27,  101 },
    {     doom2,       1,  17,     37,  101 },
    {     doom2,       1,  17,     38,  101 },
    {     doom2,       1,  17,     39,  101 },
    {     doom2,       1,  17,     40,  101 },
    {     doom2,       1,  17,     84,  101 },
    {     doom2,       1,  17,     91,  101 },
    {     doom2,       1,  17,     98,  101 },
    {     doom2,       1,  17,    136,  101 },
    {     doom2,       1,  17,    137,  101 },
    {     doom2,       1,  17,    138,  101 },
    {     doom2,       1,  17,    163,  101 },
    {     doom2,       1,  17,    173,  101 },
    {     doom2,       1,  17,    191,  100 },
    {     doom2,       1,  17,    171,  221 },
    {     doom2,       1,  17,    176,  226 },
    {     doom2,       1,  17,    204,  221 },
    {     doom2,       1,  17,    206,  221 },
    {     doom2,       1,  17,    229,  221 },
    {     doom2,       1,  17,    241,  221 },
    {     doom2,       1,  17,    255,  216 },
    {     doom2,       1,  17,    256,  211 },
    {     doom2,       1,  17,    257,  221 },
    {     doom2,       1,  17,    218,  101 },
    {     doom2,       1,  17,    222,  101 },
    {     doom2,       1,  18,     45,  231 },
    {     doom2,       1,  18,     49,  231 },
    {     doom2,       1,  18,     57,  231 },
    {     doom2,       1,  19,      0,  221 },
    {     doom2,       1,  19,     16,  100 },
    {     doom2,       1,  19,     68,  102 },
    {     doom2,       1,  19,    216,  100 },
    {     doom2,       1,  20,      0,  102 },
    {     doom2,       1,  20,      2,  101 },
    {     doom2,       1,  20,     19,  101 },
    {     doom2,       1,  20,     20,  102 },
    {     doom2,       1,  20,     26,  102 },
    {     doom2,       1,  20,     49,  102 },
    {     doom2,       1,  20,     53,  102 },
    {     doom2,       1,  20,     84,  102 },
    {     doom2,       1,  20,     85,  102 },
    {     doom2,       1,  20,     97,  102 },
    {     doom2,       1,  21,     13,  102 },
    {     doom2,       1,  21,     17,  100 },
    {     doom2,       1,  21,     22,  100 },
    {     doom2,       1,  21,     23,  100 },
    {     doom2,       1,  22,      9,  101 },
    {     doom2,       1,  22,     10,  101 },
    {     doom2,       1,  22,     11,  101 },
    {     doom2,       1,  22,     32,  101 },
    {     doom2,       1,  22,     33,  101 },
    {     doom2,       1,  22,     34,  101 },
    {     doom2,       1,  22,     35,  101 },
    {     doom2,       1,  22,     36,  101 },
    {     doom2,       1,  22,     37,  101 },
    {     doom2,       1,  22,     39,  101 },
    {     doom2,       1,  22,     70,  101 },
    {     doom2,       1,  22,     73,  100 },
    {     doom2,       1,  22,     82,  101 },
    {     doom2,       1,  22,     90,  101 },
    {     doom2,       1,  22,    112,  101 },
    {     doom2,       1,  23,      0,  101 },
    {     doom2,       1,  24,      0,  102 },
    {     doom2,       1,  24,      2,  102 },
    {     doom2,       1,  24,      9,  102 },
    {     doom2,       1,  24,     13,  102 },
    {     doom2,       1,  24,     25,  100 },
    {     doom2,       1,  24,     32,  102 },
    {     doom2,       1,  24,     42,  102 },
    {     doom2,       1,  24,     43,  102 },
    {     doom2,       1,  24,     54,  100 },
    {     doom2,       1,  24,     55,  101 },
    {     doom2,       1,  24,     63,  100 },
    {     doom2,       1,  24,    126,  101 },
    {     doom2,       1,  24,    127,  100 },
    {     doom2,       1,  25,      0,  202 },
    {     doom2,       1,  25,     24,  212 },
    {     doom2,       1,  25,     33,  212 },
    {     doom2,       1,  25,     37,  212 },
    {     doom2,       1,  25,     39,  100 },
    {     doom2,       1,  25,     41,  212 },
    {     doom2,       1,  25,     44,  100 },
    {     doom2,       1,  25,     45,  202 },
    {     doom2,       1,  25,     47,  202 },
    {     doom2,       1,  25,     48,  222 },
    {     doom2,       1,  25,     75,  217 },
    {     doom2,       1,  25,     85,  227 },
    {     doom2,       1,  25,    114,  102 },
    {     doom2,       1,  25,    115,  102 },
    {     doom2,       1,  25,    125,  100 },
    {     doom2,       1,  25,    126,  100 },
    {     doom2,       1,  25,    127,  100 },
    {     doom2,       1,  25,    128,  100 },
    {     doom2,       1,  26,      0,  101 },
    {     doom2,       1,  26,     10,  100 },
    {     doom2,       1,  26,     22,  101 },
    {     doom2,       1,  26,     25,  101 },
    {     doom2,       1,  26,     39,  100 },
    {     doom2,       1,  26,     42,  100 },
    {     doom2,       1,  26,     72,  101 },
    {     doom2,       1,  26,     79,  211 },
    {     doom2,       1,  26,     92,  100 },
    {     doom2,       1,  26,    104,  100 },
    {     doom2,       1,  26,    107,  100 },
    {     doom2,       1,  26,    108,  100 },
    {     doom2,       1,  27,    105,  101 },
    {     doom2,       1,  27,    139,  100 },
    {     doom2,       1,  28,    100,  211 },
    {     doom2,       1,  28,    101,  211 },
    {     doom2,       1,  28,    102,  211 },
    {     doom2,       1,  29,      0,  102 },
    {     doom2,       1,  29,     29,  102 },
    {     doom2,       1,  29,     30,  102 },
    {     doom2,       1,  29,     33,  100 },
    {     doom2,       1,  29,    116,  102 },
    {     doom2,       1,  29,    118,  102 },
    {     doom2,       1,  29,    121,  102 },
    {     doom2,       1,  29,    150,  100 },
    {     doom2,       1,  29,    161,  100 },
    {     doom2,       1,  29,    163,  100 },
    {     doom2,       1,  29,    164,  100 },
    {     doom2,       1,  29,    165,  100 },
    {     doom2,       1,  29,    194,  100 },
    {     doom2,       1,  29,    195,  100 },
    {     doom2,       1,  29,    196,  100 },
    {     doom2,       1,  29,    199,  100 },
    {     doom2,       1,  29,    200,  100 },

    {   pack_tnt,      1,   1,      5,  101 },
    {   pack_tnt,      1,   2,     97,  101 },
    {   pack_tnt,      1,   3,    256,  100 },
    {   pack_tnt,      1,   3,    260,  101 },
    {   pack_tnt,      1,   3,    262,  101 },
    {   pack_tnt,      1,   3,    329,  101 },
    {   pack_tnt,      1,   3,    330,  101 },
    {   pack_tnt,      1,   3,    333,  101 },
    {   pack_tnt,      1,   3,    336,  101 },
    {   pack_tnt,      1,   3,    338,  100 },
    {   pack_tnt,      1,   3,    340,  101 },
    {   pack_tnt,      1,   3,    341,  101 },
    {   pack_tnt,      1,   6,     88,  100 },
    {   pack_tnt,      1,   7,     21,  231 },
    {   pack_tnt,      1,   7,     62,  231 },
    {   pack_tnt,      1,   7,     84,  231 },
    {   pack_tnt,      1,   7,     85,  231 },
    {   pack_tnt,      1,   7,    127,  231 },
    {   pack_tnt,      1,   7,    129,  231 },
    {   pack_tnt,      1,   8,      0,  100 },
    {   pack_tnt,      1,   8,      1,  100 },
    {   pack_tnt,      1,   8,      2,  100 },
    {   pack_tnt,      1,   8,      3,  100 },
    {   pack_tnt,      1,   8,      4,  100 },
    {   pack_tnt,      1,   8,      5,  100 },
    {   pack_tnt,      1,   8,      6,  100 },
    {   pack_tnt,      1,   8,      7,  100 },
    {   pack_tnt,      1,   8,      8,  100 },
    {   pack_tnt,      1,   8,      9,  100 },
    {   pack_tnt,      1,   8,     19,  100 },
    {   pack_tnt,      1,   8,     20,  100 },
    {   pack_tnt,      1,   8,     21,  100 },
    {   pack_tnt,      1,   8,     22,  100 },
    {   pack_tnt,      1,   8,     23,  100 },
    {   pack_tnt,      1,   8,     24,  100 },
    {   pack_tnt,      1,   8,     25,  100 },
    {   pack_tnt,      1,   8,     26,  101 },
    {   pack_tnt,      1,   8,    109,  100 },
    {   pack_tnt,      1,   8,    110,  100 },
    {   pack_tnt,      1,   8,    131,  102 },
    {   pack_tnt,      1,   8,    132,  102 },
    {   pack_tnt,      1,   8,    133,  102 },
    {   pack_tnt,      1,   8,    161,  102 },
    {   pack_tnt,      1,   8,    162,  102 },
    {   pack_tnt,      1,   8,    163,  102 },
    {   pack_tnt,      1,   8,    164,  102 },
    {   pack_tnt,      1,   8,    210,  100 },
    {   pack_tnt,      1,   8,    211,  100 },
    {   pack_tnt,      1,   8,    212,  100 },
    {   pack_tnt,      1,   8,    214,  100 },
    {   pack_tnt,      1,   8,    215,  100 },
    {   pack_tnt,      1,   8,    216,  100 },
    {   pack_tnt,      1,   8,    218,  100 },
    {   pack_tnt,      1,   8,    219,  100 },
    {   pack_tnt,      1,   8,    220,  100 },
    {   pack_tnt,      1,   8,    222,  100 },
    {   pack_tnt,      1,   8,    223,  100 },
    {   pack_tnt,      1,   8,    224,  100 },
    {   pack_tnt,      1,   8,    251,  100 },
    {   pack_tnt,      1,   8,    252,  100 },
    {   pack_tnt,      1,   8,    261,  100 },
    {   pack_tnt,      1,   8,    262,  100 },
    {   pack_tnt,      1,   8,    263,  100 },
    {   pack_tnt,      1,   8,    264,  100 },
    {   pack_tnt,      1,   8,    265,  100 },
    {   pack_tnt,      1,   8,    267,  101 },
    {   pack_tnt,      1,   8,    268,  101 },
    {   pack_tnt,      1,   8,    269,  101 },
    {   pack_tnt,      1,   8,    270,  101 },
    {   pack_tnt,      1,   8,    271,  101 },
    {   pack_tnt,      1,   8,    272,  101 },
    {   pack_tnt,      1,   8,    273,  101 },
    {   pack_tnt,      1,   8,    274,  101 },
    {   pack_tnt,      1,   8,    275,  100 },
    {   pack_tnt,      1,   8,    302,  100 },
    {   pack_tnt,      1,   8,    303,  100 },
    {   pack_tnt,      1,   8,    304,  100 },
    {   pack_tnt,      1,   8,    305,  100 },
    {   pack_tnt,      1,   8,    334,  102 },
    {   pack_tnt,      1,   8,    335,  102 },
    {   pack_tnt,      1,   8,    347,  100 },
    {   pack_tnt,      1,   8,    348,  100 },
    {   pack_tnt,      1,   8,    349,  100 },
    {   pack_tnt,      1,   8,    350,  100 },
    {   pack_tnt,      1,   8,    351,  100 },
    {   pack_tnt,      1,   8,    352,  100 },
    {   pack_tnt,      1,   8,    353,  100 },
    {   pack_tnt,      1,   8,    354,  100 },
    {   pack_tnt,      1,   8,    355,  100 },
    {   pack_tnt,      1,   8,    356,  100 },
    {   pack_tnt,      1,   8,    357,  100 },
    {   pack_tnt,      1,   8,    358,  100 },
    {   pack_tnt,      1,   8,    359,  100 },
    {   pack_tnt,      1,   8,    360,  100 },
    {   pack_tnt,      1,   9,    264,  102 },
    {   pack_tnt,      1,   9,    270,  102 },
    {   pack_tnt,      1,   9,    271,  102 },
    {   pack_tnt,      1,  10,    143,  102 },
    {   pack_tnt,      1,  10,    172,  222 },
    {   pack_tnt,      1,  10,    173,  222 },
    {   pack_tnt,      1,  11,    180,  100 },
    {   pack_tnt,      1,  12,     14,  200 },
    {   pack_tnt,      1,  12,     16,  200 },
    {   pack_tnt,      1,  12,     17,  200 },
    {   pack_tnt,      1,  12,     18,  200 },
    {   pack_tnt,      1,  12,     19,  200 },
    {   pack_tnt,      1,  12,     20,  200 },
    {   pack_tnt,      1,  12,     21,  200 },
    {   pack_tnt,      1,  12,     22,  200 },
    {   pack_tnt,      1,  12,     23,  200 },
    {   pack_tnt,      1,  12,     24,  200 },
    {   pack_tnt,      1,  12,     25,  200 },
    {   pack_tnt,      1,  12,     26,  200 },
    {   pack_tnt,      1,  12,     49,  200 },
    {   pack_tnt,      1,  12,     50,  200 },
    {   pack_tnt,      1,  12,     51,  200 },
    {   pack_tnt,      1,  12,     52,  200 },
    {   pack_tnt,      1,  12,     53,  200 },
    {   pack_tnt,      1,  12,     54,  200 },
    {   pack_tnt,      1,  12,     55,  200 },
    {   pack_tnt,      1,  12,     56,  200 },
    {   pack_tnt,      1,  12,     57,  200 },
    {   pack_tnt,      1,  12,     58,  200 },
    {   pack_tnt,      1,  12,     59,  200 },
    {   pack_tnt,      1,  12,     60,  200 },
    {   pack_tnt,      1,  12,     61,  200 },
    {   pack_tnt,      1,  12,     62,  200 },
    {   pack_tnt,      1,  12,     63,  200 },
    {   pack_tnt,      1,  12,     64,  200 },
    {   pack_tnt,      1,  12,     65,  200 },
    {   pack_tnt,      1,  12,     66,  200 },
    {   pack_tnt,      1,  12,     67,  200 },
    {   pack_tnt,      1,  12,     69,  200 },
    {   pack_tnt,      1,  12,     70,  200 },
    {   pack_tnt,      1,  12,     71,  200 },
    {   pack_tnt,      1,  12,     72,  200 },
    {   pack_tnt,      1,  12,     73,  200 },
    {   pack_tnt,      1,  12,     74,  200 },
    {   pack_tnt,      1,  12,     75,  200 },
    {   pack_tnt,      1,  12,     76,  200 },
    {   pack_tnt,      1,  12,     77,  200 },
    {   pack_tnt,      1,  12,     78,  200 },
    {   pack_tnt,      1,  12,     79,  200 },
    {   pack_tnt,      1,  12,     80,  200 },
    {   pack_tnt,      1,  12,    110,  200 },
    {   pack_tnt,      1,  12,    111,  200 },
    {   pack_tnt,      1,  12,    112,  200 },
    {   pack_tnt,      1,  12,    113,  200 },
    {   pack_tnt,      1,  12,    114,  200 },
    {   pack_tnt,      1,  12,    115,  200 },
    {   pack_tnt,      1,  12,    116,  200 },
    {   pack_tnt,      1,  12,    117,  200 },
    {   pack_tnt,      1,  12,    118,  200 },
    {   pack_tnt,      1,  12,    119,  200 },
    {   pack_tnt,      1,  12,    120,  200 },
    {   pack_tnt,      1,  12,    121,  200 },
    {   pack_tnt,      1,  12,    122,  200 },
    {   pack_tnt,      1,  12,    123,  200 },
    {   pack_tnt,      1,  12,    124,  200 },
    {   pack_tnt,      1,  12,    125,  200 },
    {   pack_tnt,      1,  12,    126,  200 },
    {   pack_tnt,      1,  12,    127,  200 },
    {   pack_tnt,      1,  12,    128,  200 },
    {   pack_tnt,      1,  12,    129,  200 },
    {   pack_tnt,      1,  12,    130,  200 },
    {   pack_tnt,      1,  12,    131,  200 },
    {   pack_tnt,      1,  12,    134,  200 },
    {   pack_tnt,      1,  12,    135,  200 },
    {   pack_tnt,      1,  12,    136,  200 },
    {   pack_tnt,      1,  12,    137,  200 },
    {   pack_tnt,      1,  12,    138,  200 },
    {   pack_tnt,      1,  12,    139,  200 },
    {   pack_tnt,      1,  12,    273,  102 },
    {   pack_tnt,      1,  13,     58,  102 },
    {   pack_tnt,      1,  13,     59,  102 },
    {   pack_tnt,      1,  13,     60,  102 },
    {   pack_tnt,      1,  13,     61,  102 },
    {   pack_tnt,      1,  13,    157,  100 },
    {   pack_tnt,      1,  13,    182,  102 },
    {   pack_tnt,      1,  13,    206,  100 },
    {   pack_tnt,      1,  13,    207,  100 },
    {   pack_tnt,      1,  14,      3,  100 },
    {   pack_tnt,      1,  14,      4,  100 },
    {   pack_tnt,      1,  14,      7,  100 },
    {   pack_tnt,      1,  14,     14,  100 },
    {   pack_tnt,      1,  14,     19,  100 },
    {   pack_tnt,      1,  14,     27,  100 },
    {   pack_tnt,      1,  14,     28,  100 },
    {   pack_tnt,      1,  14,     29,  100 },
    {   pack_tnt,      1,  14,     30,  100 },
    {   pack_tnt,      1,  14,     31,  100 },
    {   pack_tnt,      1,  14,     33,  100 },
    {   pack_tnt,      1,  14,     34,  100 },
    {   pack_tnt,      1,  14,     35,  100 },
    {   pack_tnt,      1,  14,     36,  100 },
    {   pack_tnt,      1,  14,     37,  100 },
    {   pack_tnt,      1,  14,     41,  100 },
    {   pack_tnt,      1,  14,     42,  100 },
    {   pack_tnt,      1,  14,     47,  100 },
    {   pack_tnt,      1,  14,     48,  100 },
    {   pack_tnt,      1,  14,     57,  100 },
    {   pack_tnt,      1,  14,     59,  100 },
    {   pack_tnt,      1,  14,     60,  100 },
    {   pack_tnt,      1,  14,     61,  100 },
    {   pack_tnt,      1,  14,     71,  100 },
    {   pack_tnt,      1,  14,     72,  100 },
    {   pack_tnt,      1,  14,     73,  100 },
    {   pack_tnt,      1,  14,     74,  100 },
    {   pack_tnt,      1,  14,     75,  100 },
    {   pack_tnt,      1,  14,     76,  100 },
    {   pack_tnt,      1,  14,     77,  100 },
    {   pack_tnt,      1,  14,     81,  100 },
    {   pack_tnt,      1,  14,     82,  100 },
    {   pack_tnt,      1,  14,     85,  100 },
    {   pack_tnt,      1,  14,     87,  100 },
    {   pack_tnt,      1,  14,     99,  100 },
    {   pack_tnt,      1,  14,    102,  100 },
    {   pack_tnt,      1,  14,    104,  100 },
    {   pack_tnt,      1,  14,    106,  100 },
    {   pack_tnt,      1,  14,    150,  100 },
    {   pack_tnt,      1,  14,    167,  100 },
    {   pack_tnt,      1,  14,    184,  221 },
    {   pack_tnt,      1,  14,    186,  221 },
    {   pack_tnt,      1,  14,    188,  221 },
    {   pack_tnt,      1,  14,    190,  221 },
    {   pack_tnt,      1,  14,    192,  201 },
    {   pack_tnt,      1,  14,    194,  201 },
    {   pack_tnt,      1,  14,    196,  201 },
    {   pack_tnt,      1,  14,    198,  201 },
    {   pack_tnt,      1,  14,    200,  100 },
    {   pack_tnt,      1,  14,    207,  100 },
    {   pack_tnt,      1,  14,    219,  100 },
    {   pack_tnt,      1,  14,    221,  100 },
    {   pack_tnt,      1,  14,    311,  100 },
    {   pack_tnt,      1,  14,    312,  100 },
    {   pack_tnt,      1,  15,    125,  100 },
    {   pack_tnt,      1,  15,    129,  100 },
    {   pack_tnt,      1,  15,    130,  100 },
    {   pack_tnt,      1,  15,    131,  100 },
    {   pack_tnt,      1,  15,    132,  100 },
    {   pack_tnt,      1,  15,    229,  101 },
    {   pack_tnt,      1,  15,    292,  100 },
    {   pack_tnt,      1,  15,    293,  100 },
    {   pack_tnt,      1,  16,     51,  224 },
    {   pack_tnt,      1,  16,     52,  224 },
    {   pack_tnt,      1,  16,     59,  224 },
    {   pack_tnt,      1,  16,     60,  224 },
    {   pack_tnt,      1,  16,     61,  224 },
    {   pack_tnt,      1,  16,     62,  224 },
    {   pack_tnt,      1,  16,     63,  224 },
    {   pack_tnt,      1,  16,     64,  224 },
    {   pack_tnt,      1,  16,     65,  224 },
    {   pack_tnt,      1,  16,     66,  224 },
    {   pack_tnt,      1,  16,     67,  224 },
    {   pack_tnt,      1,  16,     68,  224 },
    {   pack_tnt,      1,  16,     71,  224 },
    {   pack_tnt,      1,  16,     76,  224 },
    {   pack_tnt,      1,  16,     77,  224 },
    {   pack_tnt,      1,  16,     78,  224 },
    {   pack_tnt,      1,  16,     79,  224 },
    {   pack_tnt,      1,  16,     80,  224 },
    {   pack_tnt,      1,  16,     81,  224 },
    {   pack_tnt,      1,  16,     82,  224 },
    {   pack_tnt,      1,  16,     83,  224 },
    {   pack_tnt,      1,  16,     84,  224 },
    {   pack_tnt,      1,  16,     85,  224 },
    {   pack_tnt,      1,  16,     86,  224 },
    {   pack_tnt,      1,  16,     87,  224 },
    {   pack_tnt,      1,  16,     88,  224 },
    {   pack_tnt,      1,  16,     89,  224 },
    {   pack_tnt,      1,  16,    168,  224 },
    {   pack_tnt,      1,  16,    178,  102 },
    {   pack_tnt,      1,  16,    200,  102 },
    {   pack_tnt,      1,  16,    202,  218 },
    {   pack_tnt,      1,  16,    203,  218 },
    {   pack_tnt,      1,  16,    204,  102 },
    {   pack_tnt,      1,  16,    205,  213 },
    {   pack_tnt,      1,  16,    207,  213 },
    {   pack_tnt,      1,  16,    208,  213 },
    {   pack_tnt,      1,  16,    209,  213 },
    {   pack_tnt,      1,  16,    210,  223 },
    {   pack_tnt,      1,  16,    230,  214 },
    {   pack_tnt,      1,  16,    231,  214 },
    {   pack_tnt,      1,  16,    232,  214 },
    {   pack_tnt,      1,  16,    233,  214 },
    {   pack_tnt,      1,  16,    234,  214 },
    {   pack_tnt,      1,  16,    235,  214 },
    {   pack_tnt,      1,  16,    236,  214 },
    {   pack_tnt,      1,  16,    237,  214 },
    {   pack_tnt,      1,  16,    238,  214 },
    {   pack_tnt,      1,  16,    239,  219 },
    {   pack_tnt,      1,  16,    240,  219 },
    {   pack_tnt,      1,  16,    241,  214 },
    {   pack_tnt,      1,  16,    242,  219 },
    {   pack_tnt,      1,  16,    243,  219 },
    {   pack_tnt,      1,  16,    244,  214 },
    {   pack_tnt,      1,  16,    245,  214 },
    {   pack_tnt,      1,  16,    246,  214 },
    {   pack_tnt,      1,  16,    247,  214 },
    {   pack_tnt,      1,  16,    248,  209 },
    {   pack_tnt,      1,  16,    249,  209 },
    {   pack_tnt,      1,  16,    250,  214 },
    {   pack_tnt,      1,  16,    251,  212 },
    {   pack_tnt,      1,  16,    252,  214 },
    {   pack_tnt,      1,  16,    265,  212 },
    {   pack_tnt,      1,  16,    266,  214 },
    {   pack_tnt,      1,  16,    267,  212 },
    {   pack_tnt,      1,  16,    271,  213 },
    {   pack_tnt,      1,  16,    272,  213 },
    {   pack_tnt,      1,  16,    288,  100 },
    {   pack_tnt,      1,  16,    293,  202 },
    {   pack_tnt,      1,  16,    322,  100 },
    {   pack_tnt,      1,  16,    356,  212 },
    {   pack_tnt,      1,  16,    358,  102 },
    {   pack_tnt,      1,  16,    359,  102 },
    {   pack_tnt,      1,  17,     31,  100 },
    {   pack_tnt,      1,  17,     32,  100 },
    {   pack_tnt,      1,  17,     33,  100 },
    {   pack_tnt,      1,  17,    124,  101 },
    {   pack_tnt,      1,  17,    129,  101 },
    {   pack_tnt,      1,  17,    131,  101 },
    {   pack_tnt,      1,  17,    164,  212 },
    {   pack_tnt,      1,  17,    204,  212 },
    {   pack_tnt,      1,  17,    263,  100 },
    {   pack_tnt,      1,  18,    330,  101 },
    {   pack_tnt,      1,  18,    352,  101 },
    {   pack_tnt,      1,  18,    353,  101 },
    {   pack_tnt,      1,  19,    121,  231 },
    {   pack_tnt,      1,  19,    122,  231 },
    {   pack_tnt,      1,  19,    124,  231 },
    {   pack_tnt,      1,  19,    123,  231 },
    {   pack_tnt,      1,  19,    452,  231 },
    {   pack_tnt,      1,  20,     69,  101 },
    {   pack_tnt,      1,  20,    185,  100 },
    {   pack_tnt,      1,  20,    103,  102 },
    {   pack_tnt,      1,  20,    104,  102 },
    {   pack_tnt,      1,  20,    105,  102 },
    {   pack_tnt,      1,  20,    156,  100 },
    {   pack_tnt,      1,  20,    158,  100 },
    {   pack_tnt,      1,  20,    159,  211 },
    {   pack_tnt,      1,  20,    160,  231 },
    {   pack_tnt,      1,  20,    161,  226 },
    {   pack_tnt,      1,  20,    162,  221 },
    {   pack_tnt,      1,  20,    163,  211 },
    {   pack_tnt,      1,  20,    168,  101 },
    {   pack_tnt,      1,  20,    171,  101 },
    {   pack_tnt,      1,  20,    195,  102 },
    {   pack_tnt,      1,  20,    242,  102 },
    {   pack_tnt,      1,  20,    243,  102 },
    {   pack_tnt,      1,  20,    244,  100 },
    {   pack_tnt,      1,  20,    245,  102 },
    {   pack_tnt,      1,  20,    246,  102 },
    {   pack_tnt,      1,  20,    247,  102 },
    {   pack_tnt,      1,  20,    251,  102 },
    {   pack_tnt,      1,  20,    280,  102 },
    {   pack_tnt,      1,  20,    281,  101 },
    {   pack_tnt,      1,  20,    285,  101 },
    {   pack_tnt,      1,  20,    287,  100 },
    {   pack_tnt,      1,  20,    288,  100 },
    {   pack_tnt,      1,  20,    289,  100 },
    {   pack_tnt,      1,  20,    290,  100 },
    {   pack_tnt,      1,  20,    300,  100 },
    {   pack_tnt,      1,  21,      0,  102 },
    {   pack_tnt,      1,  21,      2,  102 },
    {   pack_tnt,      1,  21,     49,  100 },
    {   pack_tnt,      1,  21,     58,  100 },
    {   pack_tnt,      1,  21,     71,  100 },
    {   pack_tnt,      1,  21,    151,  100 },
    {   pack_tnt,      1,  21,    152,  100 },
    {   pack_tnt,      1,  21,    175,  100 },
    {   pack_tnt,      1,  21,    176,  100 },
    {   pack_tnt,      1,  21,    177,  100 },
    {   pack_tnt,      1,  21,    178,  100 },
    {   pack_tnt,      1,  21,    179,  100 },
    {   pack_tnt,      1,  21,    236,  231 },
    {   pack_tnt,      1,  21,    249,  231 },
    {   pack_tnt,      1,  21,    250,  231 },
    {   pack_tnt,      1,  21,    251,  231 },
    {   pack_tnt,      1,  21,    339,  100 },
    {   pack_tnt,      1,  22,      0,  212 },
    {   pack_tnt,      1,  22,      1,  212 },
    {   pack_tnt,      1,  22,      2,  212 },
    {   pack_tnt,      1,  22,      3,  101 },
    {   pack_tnt,      1,  22,      4,  212 },
    {   pack_tnt,      1,  22,      5,  101 },
    {   pack_tnt,      1,  22,      6,  212 },
    {   pack_tnt,      1,  22,      7,  101 },
    {   pack_tnt,      1,  22,      8,  212 },
    {   pack_tnt,      1,  22,      9,  212 },
    {   pack_tnt,      1,  22,     10,  212 },
    {   pack_tnt,      1,  22,     11,  212 },
    {   pack_tnt,      1,  22,     12,  101 },
    {   pack_tnt,      1,  22,     13,  212 },
    {   pack_tnt,      1,  22,     14,  101 },
    {   pack_tnt,      1,  22,     17,  101 },
    {   pack_tnt,      1,  22,     18,  101 },
    {   pack_tnt,      1,  22,     19,  101 },
    {   pack_tnt,      1,  22,     20,  101 },
    {   pack_tnt,      1,  22,     21,  212 },
    {   pack_tnt,      1,  22,     22,  212 },
    {   pack_tnt,      1,  22,     26,  212 },
    {   pack_tnt,      1,  22,     27,  212 },
    {   pack_tnt,      1,  22,     28,  212 },
    {   pack_tnt,      1,  22,     29,  101 },
    {   pack_tnt,      1,  22,     30,  101 },
    {   pack_tnt,      1,  22,     44,  100 },
    {   pack_tnt,      1,  22,     49,  100 },
    {   pack_tnt,      1,  22,     50,  100 },
    {   pack_tnt,      1,  22,     68,  100 },
    {   pack_tnt,      1,  22,     72,  100 },
    {   pack_tnt,      1,  22,     74,  100 },
    {   pack_tnt,      1,  22,     75,  100 },
    {   pack_tnt,      1,  22,    123,  100 },
    {   pack_tnt,      1,  22,    134,  100 },
    {   pack_tnt,      1,  22,    136,  100 },
    {   pack_tnt,      1,  22,    137,  100 },
    {   pack_tnt,      1,  22,    159,  100 },
    {   pack_tnt,      1,  22,    160,  100 },
    {   pack_tnt,      1,  22,    161,  100 },
    {   pack_tnt,      1,  22,    163,  212 },
    {   pack_tnt,      1,  22,    164,  212 },
    {   pack_tnt,      1,  22,    165,  212 },
    {   pack_tnt,      1,  22,    166,  212 },
    {   pack_tnt,      1,  22,    177,  100 },
    {   pack_tnt,      1,  22,    189,  100 },
    {   pack_tnt,      1,  22,    190,  100 },
    {   pack_tnt,      1,  22,    191,  100 },
    {   pack_tnt,      1,  22,    192,  100 },
    {   pack_tnt,      1,  22,    193,  100 },
    {   pack_tnt,      1,  22,    269,  101 },
    {   pack_tnt,      1,  22,    270,  101 },
    {   pack_tnt,      1,  22,    271,  101 },
    {   pack_tnt,      1,  22,    272,  101 },
    {   pack_tnt,      1,  22,    305,  100 },
    {   pack_tnt,      1,  22,    319,  101 },
    {   pack_tnt,      1,  22,    320,  101 },
    {   pack_tnt,      1,  22,    321,  101 },
    {   pack_tnt,      1,  22,    327,  100 },
    {   pack_tnt,      1,  22,    331,  221 },
    {   pack_tnt,      1,  22,    332,  221 },
    {   pack_tnt,      1,  22,    351,  220 },
    {   pack_tnt,      1,  22,    352,  220 },
    {   pack_tnt,      1,  22,    353,  220 },
    {   pack_tnt,      1,  22,    355,  220 },
    {   pack_tnt,      1,  22,    357,  220 },
    {   pack_tnt,      1,  22,    359,  220 },
    {   pack_tnt,      1,  22,    374,  100 },
    {   pack_tnt,      1,  22,    376,  100 },
    {   pack_tnt,      1,  22,    378,  100 },
    {   pack_tnt,      1,  22,    384,  101 },
    {   pack_tnt,      1,  23,    163,  100 },
    {   pack_tnt,      1,  23,    164,  100 },
    {   pack_tnt,      1,  23,    186,  216 },
    {   pack_tnt,      1,  23,    187,  100 },
    {   pack_tnt,      1,  23,    188,  216 },
    {   pack_tnt,      1,  23,    195,  216 },
    {   pack_tnt,      1,  23,    229,  101 },
    {   pack_tnt,      1,  24,      0,  100 },
    {   pack_tnt,      1,  24,      2,  100 },
    {   pack_tnt,      1,  24,     37,  100 },
    {   pack_tnt,      1,  24,    108,  201 },
    {   pack_tnt,      1,  24,    109,  201 },
    {   pack_tnt,      1,  24,    111,  226 },
    {   pack_tnt,      1,  24,    113,  100 },
    {   pack_tnt,      1,  24,    123,  100 },
    {   pack_tnt,      1,  24,    124,  231 },
    {   pack_tnt,      1,  24,    125,  100 },
    {   pack_tnt,      1,  24,    136,  100 },
    {   pack_tnt,      1,  24,    131,  231 },
    {   pack_tnt,      1,  25,     10,  100 },
    {   pack_tnt,      1,  25,     31,  100 },
    {   pack_tnt,      1,  25,     32,  100 },
    {   pack_tnt,      1,  25,     33,  100 },
    {   pack_tnt,      1,  25,     92,  100 },
    {   pack_tnt,      1,  25,    114,  100 },
    {   pack_tnt,      1,  25,    116,  100 },
    {   pack_tnt,      1,  25,    130,  100 },
    {   pack_tnt,      1,  25,    136,  102 },
    {   pack_tnt,      1,  25,    138,  100 },
    {   pack_tnt,      1,  25,    140,  100 },
    {   pack_tnt,      1,  25,    155,  100 },
    {   pack_tnt,      1,  25,    197,  100 },
    {   pack_tnt,      1,  25,    198,  100 },
    {   pack_tnt,      1,  25,    199,  100 },
    {   pack_tnt,      1,  25,    201,  100 },
    {   pack_tnt,      1,  25,    215,  100 },
    {   pack_tnt,      1,  25,    218,  100 },
    {   pack_tnt,      1,  25,    219,  100 },
    {   pack_tnt,      1,  25,    220,  100 },
    {   pack_tnt,      1,  25,    221,  100 },
    {   pack_tnt,      1,  25,    222,  100 },
    {   pack_tnt,      1,  25,    223,  100 },
    {   pack_tnt,      1,  25,    230,  101 },
    {   pack_tnt,      1,  25,    232,  101 },
    {   pack_tnt,      1,  25,    235,  100 },
    {   pack_tnt,      1,  26,      9,  220 },
    {   pack_tnt,      1,  26,     51,  100 },
    {   pack_tnt,      1,  26,     52,  100 },
    {   pack_tnt,      1,  26,     53,  100 },
    {   pack_tnt,      1,  26,     54,  100 },
    {   pack_tnt,      1,  26,     55,  100 },
    {   pack_tnt,      1,  26,     57,  100 },
    {   pack_tnt,      1,  26,     60,  100 },
    {   pack_tnt,      1,  26,     61,  100 },
    {   pack_tnt,      1,  26,     62,  100 },
    {   pack_tnt,      1,  26,     63,  100 },
    {   pack_tnt,      1,  26,     64,  100 },
    {   pack_tnt,      1,  26,     65,  100 },
    {   pack_tnt,      1,  26,     66,  100 },
    {   pack_tnt,      1,  26,    188,  220 },
    {   pack_tnt,      1,  26,    202,  220 },
    {   pack_tnt,      1,  26,    203,  220 },
    {   pack_tnt,      1,  26,    204,  220 },
    {   pack_tnt,      1,  26,    219,  101 },
    {   pack_tnt,      1,  26,    239,  100 },
    {   pack_tnt,      1,  26,    240,  100 },
    {   pack_tnt,      1,  26,    243,  100 },
    {   pack_tnt,      1,  26,    258,  100 },
    {   pack_tnt,      1,  26,    259,  100 },
    {   pack_tnt,      1,  26,    262,  100 },
    {   pack_tnt,      1,  27,     44,  213 },
    {   pack_tnt,      1,  27,     45,  213 },
    {   pack_tnt,      1,  27,     46,  213 },
    {   pack_tnt,      1,  27,     47,  213 },
    {   pack_tnt,      1,  27,     62,  100 },
    {   pack_tnt,      1,  27,    103,  101 },
    {   pack_tnt,      1,  27,    105,  101 },
    {   pack_tnt,      1,  27,    129,  101 },
    {   pack_tnt,      1,  27,    130,  101 },
    {   pack_tnt,      1,  27,    131,  101 },
    {   pack_tnt,      1,  27,    168,  101 },
    {   pack_tnt,      1,  27,    169,  100 },
    {   pack_tnt,      1,  28,      8,  100 },
    {   pack_tnt,      1,  28,     21,  100 },
    {   pack_tnt,      1,  28,     71,  100 },
    {   pack_tnt,      1,  28,     73,  100 },
    {   pack_tnt,      1,  28,     77,  100 },
    {   pack_tnt,      1,  28,     78,  100 },
    {   pack_tnt,      1,  28,     90,  100 },
    {   pack_tnt,      1,  28,     98,  100 },
    {   pack_tnt,      1,  28,    101,  100 },
    {   pack_tnt,      1,  29,      1,  100 },
    {   pack_tnt,      1,  29,      2,  100 },
    {   pack_tnt,      1,  29,      4,  100 },
    {   pack_tnt,      1,  29,      5,  100 },
    {   pack_tnt,      1,  29,      8,  100 },
    {   pack_tnt,      1,  29,     38,  100 },
    {   pack_tnt,      1,  29,     39,  100 },
    {   pack_tnt,      1,  29,     51,  100 },
    {   pack_tnt,      1,  29,     52,  100 },
    {   pack_tnt,      1,  29,     53,  100 },
    {   pack_tnt,      1,  29,     54,  100 },
    {   pack_tnt,      1,  29,     93,  100 },
    {   pack_tnt,      1,  29,     94,  206 },
    {   pack_tnt,      1,  29,     97,  100 },
    {   pack_tnt,      1,  29,     98,  100 },
    {   pack_tnt,      1,  29,     99,  100 },
    {   pack_tnt,      1,  29,    102,  100 },
    {   pack_tnt,      1,  29,    103,  216 },
    {   pack_tnt,      1,  29,    104,  216 },
    {   pack_tnt,      1,  29,    105,  101 },
    {   pack_tnt,      1,  29,    109,  230 },
    {   pack_tnt,      1,  29,    111,  206 },
    {   pack_tnt,      1,  29,    112,  206 },
    {   pack_tnt,      1,  29,    113,  206 },
    {   pack_tnt,      1,  29,    114,  201 },
    {   pack_tnt,      1,  29,    115,  100 },
    {   pack_tnt,      1,  29,    117,  100 },
    {   pack_tnt,      1,  29,    132,  100 },
    {   pack_tnt,      1,  29,    133,  101 },
    {   pack_tnt,      1,  29,    147,  100 },
    {   pack_tnt,      1,  29,    148,  231 },
    {   pack_tnt,      1,  29,    149,  231 },
    {   pack_tnt,      1,  29,    150,  231 },
    {   pack_tnt,      1,  29,    151,  100 },
    {   pack_tnt,      1,  29,    160,  100 },
    {   pack_tnt,      1,  29,    161,  100 },
    {   pack_tnt,      1,  29,    163,  100 },
    {   pack_tnt,      1,  29,    201,  100 },
    {   pack_tnt,      1,  29,    202,  100 },
    {   pack_tnt,      1,  29,    208,  100 },
    {   pack_tnt,      1,  29,    209,  100 },
    {   pack_tnt,      1,  29,    235,  100 },
    {   pack_tnt,      1,  29,    237,  100 },
    {   pack_tnt,      1,  29,    468,  100 },
    {   pack_tnt,      1,  29,    470,  100 },
    {   pack_tnt,      1,  29,    471,  231 },
    {   pack_tnt,      1,  29,    472,  231 },
    {   pack_tnt,      1,  29,    473,  231 },
    {   pack_tnt,      1,  30,    153,  232 },
    {   pack_tnt,      1,  30,    186,  100 },
    {   pack_tnt,      1,  30,    207,  100 },
    {   pack_tnt,      1,  30,    277,  100 },
    {   pack_tnt,      1,  30,    279,  100 },
    {   pack_tnt,      1,  30,    280,  100 },
    {   pack_tnt,      1,  30,    284,  100 },
    {   pack_tnt,      1,  30,    285,  100 },
    {   pack_tnt,      1,  31,    109,  100 },
    {   pack_tnt,      1,  31,    110,  100 },
    {   pack_tnt,      1,  31,    166,  100 },
    {   pack_tnt,      1,  31,    234,  100 },
    {   pack_tnt,      1,  31,    312,  102 },
    {   pack_tnt,      1,  31,    363,  102 },
    {   pack_tnt,      1,  31,    364,  102 },
    {   pack_tnt,      1,  31,    365,  102 },
    {   pack_tnt,      1,  31,    371,  102 },
    {   pack_tnt,      1,  31,    372,  102 },
    {   pack_tnt,      1,  32,      0,  223 },
    {   pack_tnt,      1,  32,     17,  223 },
    {   pack_tnt,      1,  32,     21,  223 },
    {   pack_tnt,      1,  32,     22,  223 },
    {   pack_tnt,      1,  32,     82,  223 },
    {   pack_tnt,      1,  32,    108,  100 },
    {   pack_tnt,      1,  32,    146,  100 },
    {   pack_tnt,      1,  32,    147,  100 },
    {   pack_tnt,      1,  32,    164,  100 },
    {   pack_tnt,      1,  32,    165,  100 },
    {   pack_tnt,      1,  32,    166,  100 },
    {   pack_tnt,      1,  32,    167,  100 },
    {   pack_tnt,      1,  32,    168,  100 },
    {   pack_tnt,      1,  32,    169,  100 },
    {   pack_tnt,      1,  32,    170,  100 },
    {   pack_tnt,      1,  32,    171,  100 },
    {   pack_tnt,      1,  32,    172,  100 },
    {   pack_tnt,      1,  32,    173,  100 },
    {   pack_tnt,      1,  32,    174,  100 },
    {   pack_tnt,      1,  32,    175,  100 },
    {   pack_tnt,      1,  32,    176,  100 },
    {   pack_tnt,      1,  32,    177,  100 },
    {   pack_tnt,      1,  32,    178,  100 },
    {   pack_tnt,      1,  32,    179,  100 },
    {   pack_tnt,      1,  32,    180,  100 },
    {   pack_tnt,      1,  32,    181,  100 },
    {   pack_tnt,      1,  32,    188,  100 },
    {   pack_tnt,      1,  32,    190,  100 },
    {   pack_tnt,      1,  32,    196,  100 },
    {   pack_tnt,      1,  32,    197,  100 },
    {   pack_tnt,      1,  32,    183,  223 },
    {   pack_tnt,      1,  32,    237,  221 },
    {   pack_tnt,      1,  32,    238,  221 },

    {  pack_plut,      1,   1,      8,  100 },
    {  pack_plut,      1,   1,     56,  100 },
    {  pack_plut,      1,   1,     72,  100 },
    {  pack_plut,      1,   1,     83,  100 },
    {  pack_plut,      1,   1,     85,  100 },
    {  pack_plut,      1,   1,     86,  100 },
    {  pack_plut,      1,   1,     87,  100 },
    {  pack_plut,      1,   1,     95,  100 },
    {  pack_plut,      1,   1,     98,  100 },
    {  pack_plut,      1,   2,      0,  101 },
    {  pack_plut,      1,   2,      5,  101 },
    {  pack_plut,      1,   2,      6,  101 },
    {  pack_plut,      1,   2,      7,  101 },
    {  pack_plut,      1,   2,      9,  101 },
    {  pack_plut,      1,   2,     12,  101 },
    {  pack_plut,      1,   2,     14,  101 },
    {  pack_plut,      1,   2,     17,  102 },
    {  pack_plut,      1,   2,     18,  102 },
    {  pack_plut,      1,   2,     20,  102 },
    {  pack_plut,      1,   2,     21,  102 },
    {  pack_plut,      1,   2,     33,  101 },
    {  pack_plut,      1,   2,     34,  101 },
    {  pack_plut,      1,   2,     52,  100 },
    {  pack_plut,      1,   2,     55,  100 },
    {  pack_plut,      1,   2,     56,  100 },
    {  pack_plut,      1,   2,    120,  101 },
    {  pack_plut,      1,   2,    121,  223 },
    {  pack_plut,      1,   2,    122,  223 },
    {  pack_plut,      1,   2,    123,  102 },
    {  pack_plut,      1,   2,    124,  101 },
    {  pack_plut,      1,   2,    125,  101 },
    {  pack_plut,      1,   3,     18,  100 },
    {  pack_plut,      1,   3,     22,  100 },
    {  pack_plut,      1,   3,     23,  100 },
    {  pack_plut,      1,   3,     24,  100 },
    {  pack_plut,      1,   3,     37,  100 },
    {  pack_plut,      1,   3,     47,  100 },
    {  pack_plut,      1,   3,     48,  100 },
    {  pack_plut,      1,   3,     49,  100 },
    {  pack_plut,      1,   3,     74,  100 },
    {  pack_plut,      1,   3,     76,  100 },
    {  pack_plut,      1,   3,     77,  100 },
    {  pack_plut,      1,   3,     78,  100 },
    {  pack_plut,      1,   3,     91,  100 },
    {  pack_plut,      1,   3,     93,  100 },
    {  pack_plut,      1,   3,    104,  100 },
    {  pack_plut,      1,   3,    107,  100 },
    {  pack_plut,      1,   3,    108,  100 },
    {  pack_plut,      1,   3,    109,  100 },
    {  pack_plut,      1,   3,    123,  100 },
    {  pack_plut,      1,   3,    132,  100 },
    {  pack_plut,      1,   4,      3,  100 },
    {  pack_plut,      1,   4,      4,  100 },
    {  pack_plut,      1,   4,      5,  100 },
    {  pack_plut,      1,   4,     10,  100 },
    {  pack_plut,      1,   4,     33,  220 },
    {  pack_plut,      1,   4,     72,  100 },
    {  pack_plut,      1,   4,     86,  100 },
    {  pack_plut,      1,   4,     93,  100 },
    {  pack_plut,      1,   4,     97,  100 },
    {  pack_plut,      1,   4,    141,  100 },
    {  pack_plut,      1,   4,    184,  100 },
    {  pack_plut,      1,   4,    188,  100 },
    {  pack_plut,      1,   5,      0,  100 },
    {  pack_plut,      1,   5,     33,  101 },
    {  pack_plut,      1,   5,     35,  100 },
    {  pack_plut,      1,   5,     48,  101 },
    {  pack_plut,      1,   5,     51,  101 },
    {  pack_plut,      1,   5,     69,  101 },
    {  pack_plut,      1,   6,     88,  100 },
    {  pack_plut,      1,   6,    119,  100 },
    {  pack_plut,      1,   6,    120,  100 },
    {  pack_plut,      1,   6,    121,  100 },
    {  pack_plut,      1,   6,    122,  100 },
    {  pack_plut,      1,   6,    130,  100 },
    {  pack_plut,      1,   6,    161,  101 },
    {  pack_plut,      1,   6,    162,  102 },
    {  pack_plut,      1,   6,    163,  101 },
    
    {  pack_plut,      1,   7,      5,  102 },
    {  pack_plut,      1,   7,     23,  100 },
    {  pack_plut,      1,   7,     32,  102 },

    {        -1,       0,   0,      0,  32767 },
};

// =============================================================================
//
// [JN] Falling effect of liquid linedefs.
//
// Following table injects falling effect to linedefs with liquid textures.
//
// Nothing special about animation effect, just a five scroller types, where
// 100 is slowest and 104 is fastest. This effect must be injected as separate
// property, because engine allows only one effect per linedef, and this must
// be taken to account. Notable example: MAP25 of Doom 2 whith bloodfall pillar
// (sector 115) which is acting as a lift.
//
// Also, falling effect must be applied to both sides of linedef, since liquid
// texture can be present on both sides as well.
//
// =========================================================== (2022-04-12) ====

fixed_t FallFactor_100, FallFactor_100_old;
fixed_t FallFactor_101, FallFactor_101_old;
fixed_t FallFactor_102, FallFactor_102_old;
fixed_t FallFactor_103, FallFactor_103_old;
fixed_t FallFactor_104, FallFactor_104_old;

#define FALLDELTA_100  256
#define FALLDELTA_101  512
#define FALLDELTA_102  1024
#define FALLDELTA_103  2560
#define FALLDELTA_104  4096

// -----------------------------------------------------------------------------
// R_FlowPlane
// [JN] Modify linedef texture extra offsets of falling liquids.
// -----------------------------------------------------------------------------

void R_FallLinedef (void)
{
    // Slowest
    FallFactor_100_old = FallFactor_100;
    FallFactor_100 += FALLDELTA_100;

    // Slow
    FallFactor_101_old = FallFactor_101;
    FallFactor_101 += FALLDELTA_101;

    // Normal
    FallFactor_102_old = FallFactor_102;
    FallFactor_102 += FALLDELTA_102;

    // Fast
    FallFactor_103_old = FallFactor_103;
    FallFactor_103 += FALLDELTA_103;

    // Fastest
    FallFactor_104_old = FallFactor_104;
    FallFactor_104 += FALLDELTA_104;
}

// -----------------------------------------------------------------------------
// [JN] Inject fall specials to vanilla maps.
// -----------------------------------------------------------------------------

fall_t fall[] =
{
    //  mission, episode, map, linedef, fall
    {     doom2,       1,  24,      58,  102 },
    {     doom2,       1,  24,      59,  102 },
    {     doom2,       1,  24,      60,  102 },
    {     doom2,       1,  24,      61,  102 },
    {     doom2,       1,  24,      62,  102 },
    {     doom2,       1,  24,      63,  102 },
    {     doom2,       1,  24,      64,  102 },
    {     doom2,       1,  24,      65,  102 },
    {     doom2,       1,  24,     341,  102 },
    {     doom2,       1,  24,     342,  102 },
    {     doom2,       1,  24,     343,  102 },
    {     doom2,       1,  24,     344,  102 },
    {     doom2,       1,  24,     345,  102 },
    {     doom2,       1,  24,     346,  102 },
    {     doom2,       1,  24,     347,  102 },
    {     doom2,       1,  24,     348,  102 },
    {     doom2,       1,  24,     349,  102 },
    {     doom2,       1,  24,     357,  101 },
    {     doom2,       1,  24,     358,  101 },
    {     doom2,       1,  24,     359,  101 },
    {     doom2,       1,  24,     360,  101 },
    {     doom2,       1,  24,     361,  101 },
    {     doom2,       1,  24,     362,  101 },
    {     doom2,       1,  24,     363,  101 },
    {     doom2,       1,  24,     364,  101 },
    {     doom2,       1,  24,     365,  101 },
    {     doom2,       1,  24,     366,  101 },
    {     doom2,       1,  24,     367,  101 },
    {     doom2,       1,  24,     368,  101 },
    {     doom2,       1,  24,     369,  101 },
    {     doom2,       1,  24,     370,  101 },
    {     doom2,       1,  24,     371,  101 },
    {     doom2,       1,  24,     372,  101 },
    {     doom2,       1,  24,     373,  101 },
    {     doom2,       1,  24,     374,  101 },
    {     doom2,       1,  24,     375,  101 },
    {     doom2,       1,  24,     376,  101 },
    {     doom2,       1,  24,     377,  101 },
    {     doom2,       1,  24,     378,  101 },
    {     doom2,       1,  24,     379,  101 },
    {     doom2,       1,  24,     380,  101 },
    {     doom2,       1,  24,     381,  101 },
    {     doom2,       1,  24,     382,  101 },
    {     doom2,       1,  24,     383,  101 },
    {     doom2,       1,  24,     393,  101 },
    {     doom2,       1,  24,     394,  101 },
    {     doom2,       1,  24,     395,  101 },
    {     doom2,       1,  24,     396,  101 },
    {     doom2,       1,  24,     397,  101 },
    {     doom2,       1,  24,     398,  101 },
    {     doom2,       1,  24,     399,  101 },
    {     doom2,       1,  24,     400,  101 },
    {     doom2,       1,  24,     401,  101 },
    {     doom2,       1,  24,     402,  101 },
    {     doom2,       1,  24,     403,  101 },
    {     doom2,       1,  24,     404,  101 },
    {     doom2,       1,  24,     405,  101 },
    {     doom2,       1,  24,     406,  101 },
    {     doom2,       1,  24,     407,  101 },
    {     doom2,       1,  24,     408,  101 },
    {     doom2,       1,  24,     409,  101 },
    {     doom2,       1,  24,     410,  101 },
    {     doom2,       1,  24,     411,  101 },
    {     doom2,       1,  24,     412,  101 },
    {     doom2,       1,  24,     413,  101 },
    {     doom2,       1,  24,     414,  101 },
    {     doom2,       1,  24,     415,  101 },
    {     doom2,       1,  24,     419,  101 },
    {     doom2,       1,  24,     418,  101 },
    {     doom2,       1,  24,     416,  101 },
    {     doom2,       1,  24,     417,  101 },
    {     doom2,       1,  24,     418,  101 },
    {     doom2,       1,  24,     419,  101 },
    {     doom2,       1,  24,     420,  101 },
    {     doom2,       1,  24,     421,  101 },
    {     doom2,       1,  24,     422,  101 },
    {     doom2,       1,  25,     367,  102 },
    {     doom2,       1,  25,     372,  102 },
    {     doom2,       1,  25,     379,  102 },
    {     doom2,       1,  25,     389,  102 },
    {     doom2,       1,  25,     394,  102 },
    {     doom2,       1,  25,     452,  102 },
    {     doom2,       1,  25,     472,  102 },
    {     doom2,       1,  25,     473,  102 },
    {     doom2,       1,  25,     474,  102 },
    {     doom2,       1,  25,     475,  102 },
    {     doom2,       1,  25,     476,  102 },
    {     doom2,       1,  25,     477,  102 },
    {     doom2,       1,  25,     478,  102 },
    {     doom2,       1,  25,     479,  102 },
    {     doom2,       1,  28,     124,  101 },
    {     doom2,       1,  28,     125,  101 },
    {     doom2,       1,  28,     126,  101 },
    {     doom2,       1,  28,     127,  101 },
    {     doom2,       1,  28,     128,  101 },
    {     doom2,       1,  28,     201,  101 },
    {     doom2,       1,  28,     202,  101 },
    {     doom2,       1,  28,     203,  101 },
    {     doom2,       1,  28,     204,  101 },
    {     doom2,       1,  28,     205,  101 },
    {     doom2,       1,  28,     206,  101 },
    {     doom2,       1,  28,     221,  100 },
    {     doom2,       1,  28,     283,  101 },
    {     doom2,       1,  28,     284,  101 },
    {     doom2,       1,  28,     285,  101 },
    {     doom2,       1,  28,     286,  101 },
    {     doom2,       1,  28,     287,  101 },
    {     doom2,       1,  28,     289,  101 },
    {     doom2,       1,  28,     290,  101 },
    {     doom2,       1,  28,     291,  101 },
    {     doom2,       1,  28,     292,  101 },
    {     doom2,       1,  28,     293,  101 },
    {     doom2,       1,  28,     315,  101 },
    {     doom2,       1,  28,     318,  101 },
    {     doom2,       1,  28,     323,  101 },
    {     doom2,       1,  28,     324,  101 },
    {     doom2,       1,  28,     325,  101 },
    {     doom2,       1,  28,     326,  101 },
    {     doom2,       1,  28,     327,  101 },
    {     doom2,       1,  28,     351,  101 },
    {     doom2,       1,  28,     435,  101 },
    {     doom2,       1,  28,     436,  101 },
    {     doom2,       1,  30,       7,  102 },
    {     doom2,       1,  30,       8,  102 },
    {     doom2,       1,  30,       9,  102 },
    {     doom2,       1,  30,      10,  102 },
    {     doom2,       1,  30,      11,  102 },
    {     doom2,       1,  30,      26,  102 },

    {  pack_tnt,       1,   9,    1370,  101 },
    {  pack_tnt,       1,   9,    1372,  101 },
    {  pack_tnt,       1,   9,    1373,  101 },
    {  pack_tnt,       1,   9,    1409,  102 },
    {  pack_tnt,       1,   9,    1411,  102 },
    {  pack_tnt,       1,  10,     680,  102 },
    {  pack_tnt,       1,  10,     681,  102 },
    {  pack_tnt,       1,  12,    1555,  103 },
    {  pack_tnt,       1,  12,    1556,  103 },
    {  pack_tnt,       1,  12,    1557,  103 },
    {  pack_tnt,       1,  12,    1558,  103 },
    {  pack_tnt,       1,  12,    1559,  103 },
    {  pack_tnt,       1,  12,    1560,  103 },
    {  pack_tnt,       1,  12,    1561,  103 },
    {  pack_tnt,       1,  12,    1562,  103 },
    {  pack_tnt,       1,  12,    1563,  103 },
    {  pack_tnt,       1,  14,     155,  101 },
    {  pack_tnt,       1,  14,     161,  101 },
    {  pack_tnt,       1,  14,     173,  101 },
    {  pack_tnt,       1,  14,     175,  101 },
    {  pack_tnt,       1,  14,     179,  101 },
    {  pack_tnt,       1,  14,     188,  101 },
    {  pack_tnt,       1,  14,     193,  101 },
    {  pack_tnt,       1,  14,     198,  101 },
    {  pack_tnt,       1,  14,     209,  101 },
    {  pack_tnt,       1,  14,     212,  101 },
    {  pack_tnt,       1,  14,     215,  101 },
    {  pack_tnt,       1,  14,     218,  101 },
    {  pack_tnt,       1,  14,     229,  101 },
    {  pack_tnt,       1,  14,     232,  101 },
    {  pack_tnt,       1,  14,     244,  101 },
    {  pack_tnt,       1,  14,     247,  101 },
    {  pack_tnt,       1,  16,     302,  103 },
    {  pack_tnt,       1,  16,     527,  103 },
    {  pack_tnt,       1,  16,     667,  100 },
    {  pack_tnt,       1,  16,     669,  100 },
    {  pack_tnt,       1,  16,     670,  100 },
    {  pack_tnt,       1,  16,     671,  100 },
    {  pack_tnt,       1,  16,     672,  100 },
    {  pack_tnt,       1,  16,     685,  100 },
    {  pack_tnt,       1,  16,     687,  100 },
    {  pack_tnt,       1,  16,    1238,  103 },
    {  pack_tnt,       1,  16,    1247,  103 },
    {  pack_tnt,       1,  16,    1248,  103 },
    {  pack_tnt,       1,  16,    1364,  102 },
    {  pack_tnt,       1,  16,    1373,  103 },
    {  pack_tnt,       1,  16,    1407,  103 },
    {  pack_tnt,       1,  16,    1408,  103 },
    {  pack_tnt,       1,  16,    1409,  103 },
    {  pack_tnt,       1,  16,    1412,  103 },
    {  pack_tnt,       1,  16,    1413,  103 },
    {  pack_tnt,       1,  16,    1416,  103 },
    {  pack_tnt,       1,  16,    1417,  103 },
    {  pack_tnt,       1,  16,    1418,  102 },
    {  pack_tnt,       1,  16,    1439,  102 },
    {  pack_tnt,       1,  16,    1443,  102 },
    {  pack_tnt,       1,  16,    1444,  102 },
    {  pack_tnt,       1,  16,    1447,  102 },
    {  pack_tnt,       1,  16,    1456,  103 },
    {  pack_tnt,       1,  16,    1457,  103 },
    {  pack_tnt,       1,  16,    1458,  103 },
    {  pack_tnt,       1,  16,    1459,  103 },
    {  pack_tnt,       1,  16,    1468,  103 },
    {  pack_tnt,       1,  16,    1525,  102 },
    {  pack_tnt,       1,  16,    1528,  102 },
    {  pack_tnt,       1,  16,    1532,  102 },
    {  pack_tnt,       1,  16,    1540,  102 },
    {  pack_tnt,       1,  16,    1546,  102 },
    {  pack_tnt,       1,  16,    1549,  102 },
    {  pack_tnt,       1,  16,    1553,  102 },
    {  pack_tnt,       1,  16,    1558,  102 },
    {  pack_tnt,       1,  16,    1561,  102 },
    {  pack_tnt,       1,  16,    1564,  102 },
    {  pack_tnt,       1,  16,    1567,  102 },
    {  pack_tnt,       1,  16,    1572,  102 },
    {  pack_tnt,       1,  16,    1575,  102 },
    {  pack_tnt,       1,  16,    1578,  102 },
    {  pack_tnt,       1,  16,    1581,  102 },
    {  pack_tnt,       1,  16,    1584,  102 },
    {  pack_tnt,       1,  16,    1587,  102 },
    {  pack_tnt,       1,  16,    1590,  102 },
    {  pack_tnt,       1,  16,    1593,  102 },
    {  pack_tnt,       1,  16,    1596,  102 },
    {  pack_tnt,       1,  16,    1599,  103 },
    {  pack_tnt,       1,  16,    1602,  102 },
    {  pack_tnt,       1,  16,    1696,  102 },
    {  pack_tnt,       1,  16,    1706,  104 },
    {  pack_tnt,       1,  16,    1825,  101 },
    {  pack_tnt,       1,  16,    2060,  101 },
    {  pack_tnt,       1,  16,    2073,  101 },
    {  pack_tnt,       1,  16,    2128,  102 },
    {  pack_tnt,       1,  16,    2129,  102 },
    {  pack_tnt,       1,  16,    2130,  102 },
    {  pack_tnt,       1,  16,    2131,  103 },
    {  pack_tnt,       1,  16,    2132,  103 },
    {  pack_tnt,       1,  16,    2133,  103 },
    {  pack_tnt,       1,  16,    2134,  103 },
    {  pack_tnt,       1,  16,    2135,  102 },
    {  pack_tnt,       1,  16,    2136,  102 },
    {  pack_tnt,       1,  16,    2137,  102 },
    {  pack_tnt,       1,  16,    2138,  102 },
    {  pack_tnt,       1,  16,    2144,  102 },
    {  pack_tnt,       1,  16,    2145,  102 },
    {  pack_tnt,       1,  16,    2146,  102 },
    {  pack_tnt,       1,  16,    2147,  102 },
    {  pack_tnt,       1,  16,    2148,  102 },
    {  pack_tnt,       1,  16,    2149,  102 },
    {  pack_tnt,       1,  16,    2150,  102 },
    {  pack_tnt,       1,  16,    2151,  102 },
    {  pack_tnt,       1,  16,    2152,  102 },
    {  pack_tnt,       1,  16,    2153,  102 },
    {  pack_tnt,       1,  16,    2154,  102 },
    {  pack_tnt,       1,  16,    2155,  102 },
    {  pack_tnt,       1,  16,    2156,  102 },
    {  pack_tnt,       1,  16,    2157,  102 },
    {  pack_tnt,       1,  16,    2158,  102 },
    {  pack_tnt,       1,  16,    2159,  102 },
    {  pack_tnt,       1,  16,    2160,  102 },
    {  pack_tnt,       1,  16,    2161,  102 },
    {  pack_tnt,       1,  16,    2162,  102 },
    {  pack_tnt,       1,  16,    2163,  102 },
    {  pack_tnt,       1,  16,    2164,  102 },
    {  pack_tnt,       1,  16,    2165,  102 },
    {  pack_tnt,       1,  16,    2166,  102 },
    {  pack_tnt,       1,  16,    2167,  102 },
    {  pack_tnt,       1,  16,    2307,  102 },
    {  pack_tnt,       1,  20,    1254,  102 },
    {  pack_tnt,       1,  20,    2159,  101 },
    {  pack_tnt,       1,  20,    2160,  101 },
    {  pack_tnt,       1,  20,    2163,  101 },
    {  pack_tnt,       1,  20,    2164,  101 },
    {  pack_tnt,       1,  22,      77,  102 },
    {  pack_tnt,       1,  22,    1539,  101 },
    {  pack_tnt,       1,  22,    1727,  101 },
    {  pack_tnt,       1,  22,    1728,  101 },
    {  pack_tnt,       1,  23,    1184,  102 },
    {  pack_tnt,       1,  26,    1444,  101 },
    {  pack_tnt,       1,  26,    1445,  101 },
    {  pack_tnt,       1,  26,    1446,  101 },
    {  pack_tnt,       1,  26,    1448,  101 },
    {  pack_tnt,       1,  26,    1325,  101 },
    {  pack_tnt,       1,  26,    1331,  101 },
    {  pack_tnt,       1,  26,    1334,  101 },
    {  pack_tnt,       1,  26,    1335,  101 },
    {  pack_tnt,       1,  29,     452,  101 },
    {  pack_tnt,       1,  29,     457,  101 },
    {  pack_tnt,       1,  29,     480,  101 },
    {  pack_tnt,       1,  29,     495,  102 },
    {  pack_tnt,       1,  29,     514,  101 },
    {  pack_tnt,       1,  29,     520,  101 },
    {  pack_tnt,       1,  29,     528,  101 },
    {  pack_tnt,       1,  29,     564,  101 },
    {  pack_tnt,       1,  29,     576,  101 },
    {  pack_tnt,       1,  29,     579,  101 },
    {  pack_tnt,       1,  29,     582,  101 },
    {  pack_tnt,       1,  29,     587,  101 },
    {  pack_tnt,       1,  29,     596,  101 },
    {  pack_tnt,       1,  29,     617,  102 },
    {  pack_tnt,       1,  29,     618,  102 },
    {  pack_tnt,       1,  29,     697,  101 },
    {  pack_tnt,       1,  29,     707,  101 },
    {  pack_tnt,       1,  29,     852,  101 },
    {  pack_tnt,       1,  29,     856,  102 },
    {  pack_tnt,       1,  29,     857,  101 },
    {  pack_tnt,       1,  29,     859,  102 },
    {  pack_tnt,       1,  29,     860,  102 },
    {  pack_tnt,       1,  29,     863,  102 },
    {  pack_tnt,       1,  29,    2483,  101 },
    {  pack_tnt,       1,  29,    2486,  101 },
    {  pack_tnt,       1,  29,    2489,  101 },

    {  pack_plut,      1,   2,      21,  103 },
    {  pack_plut,      1,   2,      30,  103 },
    {  pack_plut,      1,   2,      31,  103 },
    {  pack_plut,      1,   2,      32,  103 },
    {  pack_plut,      1,   2,      47,  103 },
    {  pack_plut,      1,   2,      58,  103 },
    {  pack_plut,      1,   2,      73,  103 },
    {  pack_plut,      1,   2,     144,  103 },
    {  pack_plut,      1,   2,     145,  103 },
    {  pack_plut,      1,   2,     146,  103 },
    {  pack_plut,      1,   2,     147,  103 },
    {  pack_plut,      1,   2,     148,  103 },
    {  pack_plut,      1,   2,     149,  103 },
    {  pack_plut,      1,   2,     173,  103 },
    {  pack_plut,      1,   2,     174,  103 },
    {  pack_plut,      1,   2,     175,  103 },
    {  pack_plut,      1,   2,     176,  103 },
    {  pack_plut,      1,   2,     177,  103 },
    {  pack_plut,      1,   2,     178,  103 },
    {  pack_plut,      1,   2,     179,  103 },
    {  pack_plut,      1,   2,     180,  103 },
    {  pack_plut,      1,   2,     181,  103 },
    {  pack_plut,      1,   2,     182,  103 },
    {  pack_plut,      1,   2,     183,  103 },
    {  pack_plut,      1,   2,     736,  103 },
    {  pack_plut,      1,   2,     737,  103 },
    {  pack_plut,      1,   2,     738,  103 },
    {  pack_plut,      1,   2,     739,  103 },
    {  pack_plut,      1,   2,     740,  103 },
    {  pack_plut,      1,   2,     741,  103 },
    {  pack_plut,      1,   2,     742,  103 },
    {  pack_plut,      1,   2,     743,  103 },
    {  pack_plut,      1,   2,     744,  103 },
    {  pack_plut,      1,   2,     745,  103 },
    {  pack_plut,      1,   2,     746,  103 },
    {  pack_plut,      1,   2,     747,  103 },
    {  pack_plut,      1,   2,     748,  103 },
    {  pack_plut,      1,   2,     749,  103 },
    {  pack_plut,      1,   2,     750,  103 },
    {  pack_plut,      1,   2,     752,  103 },
    {  pack_plut,      1,   5,     342,  103 },
    {  pack_plut,      1,   5,     343,  103 },
    {  pack_plut,      1,   5,     344,  103 },
    {  pack_plut,      1,   5,     345,  103 },
    {  pack_plut,      1,   5,     354,  103 },
    {  pack_plut,      1,   5,     355,  103 },
    {  pack_plut,      1,   5,     356,  103 },
    {  pack_plut,      1,   5,     357,  103 },
    {  pack_plut,      1,   5,     443,  103 },
    {  pack_plut,      1,   5,     447,  103 },
    {  pack_plut,      1,   6,    1011,  102 },
    {  pack_plut,      1,   6,    1012,  102 },
    {  pack_plut,      1,   6,    1013,  102 },
    {  pack_plut,      1,   6,    1014,  102 },
    {  pack_plut,      1,   6,    1015,  102 },
    {  pack_plut,      1,   6,    1021,  102 },
    {  pack_plut,      1,   6,    1027,  102 },
    {  pack_plut,      1,   6,    1033,  102 },
    {  pack_plut,      1,   6,    1043,  102 },
    {  pack_plut,      1,   6,    1044,  102 },
    {  pack_plut,      1,   6,    1045,  102 },
    {  pack_plut,      1,   6,    1046,  102 },
    

    {        -1,       0,   0,       0,  32767 },
};
