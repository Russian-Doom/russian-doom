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
    {     doom2,       1,   1,     14,         "",      236 },

    {        -1,       0,   0,      0,         "",    32767 },
};
