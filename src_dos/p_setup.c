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
//	Do all the WAD I/O, get map description,
//	set up initial state and misc. LUTs.
//

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "z_zone.h"
#include "m_misc.h"
#include "g_game.h"
#include "i_system.h"
#include "w_wad.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "jn.h"


void        P_SpawnMapThing (mapthing_t *mthing);
extern void P_SpawnBrainTargets ();


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int          numvertexes;
vertex_t    *vertexes;

int          numsegs;
seg_t       *segs;

int          numsectors;
sector_t    *sectors;

int          numsubsectors;
subsector_t *subsectors;

int          numnodes;
node_t      *nodes;

int          numlines;
line_t      *lines;

int          numsides;
side_t      *sides;


// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//

int        bmapwidth;       // Blockmap size.
int        bmapheight;      // size in mapblocks
short     *blockmap;        // int for larger maps
short     *blockmaplump;    // offsets in blockmap are from here
fixed_t    bmaporgx;        // origin of block map
fixed_t    bmaporgy;
mobj_t   **blocklinks;      // for thing chains


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte      *rejectmatrix;


// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS   10

mapthing_t  deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t *deathmatch_p;
mapthing_t  playerstarts[MAXPLAYERS];
boolean     playerstartsingame[MAXPLAYERS];


//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{
    int           i;
    byte         *data;
    vertex_t     *li;
    mapvertex_t  *ml;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);	

    // Load data into cache.
    data = W_CacheLumpNum (lump,PU_STATIC);

    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
        li->x = SHORT(ml->x)<<FRACBITS;
        li->y = SHORT(ml->y)<<FRACBITS;

        // [crispy] initialize pseudovertexes with actual vertex coordinates
        li->px = li->x;
        li->py = li->y;
        li->moved = false;
    }

    // Free buffer memory.
    Z_Free (data);
}


//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    int        i;
    int        linedef;
    int        side;
    byte      *data;
    seg_t     *li;
    line_t    *ldef;
    mapseg_t  *ml;

    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);	
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    ml = (mapseg_t *)data;
    li = segs;

    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
        li->v1 = &vertexes[SHORT(ml->v1)];
        li->v2 = &vertexes[SHORT(ml->v2)];

        li->angle = (SHORT(ml->angle))<<16;
        li->offset = (SHORT(ml->offset))<<16;
        linedef = SHORT(ml->linedef);
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;

        if (ldef-> flags & ML_TWOSIDED)
        {
            li->backsector = sides[ldef->sidenum[side^1]].sector;
        }
        else
        {
            li->backsector = 0;
        }
    }

    Z_Free (data);
}


// [crispy] fix long wall wobble
void P_SegLengths (void)
{
    int i;

    for (i = 0; i < numsegs; i++)
    {
        seg_t *const li = &segs[i];
        int64_t dx, dy;

        dx = li->v2->px - li->v1->px;
        dy = li->v2->py - li->v1->py;
        li->length = (uint32_t)(sqrt((double)dx*dx + (double)dy*dy)/2);

        // [crispy] re-calculate angle used for rendering
        viewx = li->v1->px;
        viewy = li->v1->py;
        li->r_angle = R_PointToAngleCrispy(li->v2->px, li->v2->py);
    }
}


//
// P_LoadSubsectors
//
void P_LoadSubsectors (int lump)
{
    int              i;
    byte            *data;
    subsector_t     *ss;
    mapsubsector_t  *ms;

    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);

    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;

    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
        ss->numlines = SHORT(ms->numsegs);
        ss->firstline = SHORT(ms->firstseg);
    }

    Z_Free (data);
}


//
// P_LoadSectors
//
void P_LoadSectors (int lump)
{
    byte*		data;
    int			i;
    mapsector_t*	ms;
    sector_t*		ss;
	
    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);	
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    ms = (mapsector_t *)data;
    ss = sectors;

    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
        ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
        ss->floorpic = R_FlatNumForName(ms->floorpic);
        ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
        ss->lightlevel = SHORT(ms->lightlevel);
        ss->special = SHORT(ms->special);
        ss->tag = SHORT(ms->tag);
        ss->thinglist = NULL;
        // [JN] Improved column clipping.
        ss->floor_xoffs = 0;
        ss->floor_yoffs = 0;
        ss->ceiling_xoffs = 0;
        ss->ceiling_yoffs = 0;
        // [crispy] WiggleFix: [kb] for R_FixWiggle()
        ss->cachedheight = 0;
    }

    Z_Free (data);
}


//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    int         i;
    int         j;
    int         k;
    byte       *data;
    node_t     *no;
    mapnode_t  *mn;

    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);

    mn = (mapnode_t *)data;
    no = nodes;

    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
        no->x = SHORT(mn->x)<<FRACBITS;
        no->y = SHORT(mn->y)<<FRACBITS;
        no->dx = SHORT(mn->dx)<<FRACBITS;
        no->dy = SHORT(mn->dy)<<FRACBITS;

        for (j=0 ; j<2 ; j++)
        {
            no->children[j] = SHORT(mn->children[j]);

            for (k=0 ; k<4 ; k++)
            {
                no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
            }
        }
    }

    Z_Free (data);
}


//
// P_LoadThings
//
void P_LoadThings (int lump)
{
    int	         i;
    int		     numthings;
    boolean	     spawn;
    byte        *data;
    mapthing_t  *mt;

    data = W_CacheLumpNum (lump,PU_STATIC);
    numthings = W_LumpLength (lump) / sizeof(mapthing_t);

    mt = (mapthing_t *)data;

    for (i=0 ; i<numthings ; i++, mt++)
    {
        spawn = true;

        // Do not spawn cool, new monsters if !commercial
        if (!commercial)
        {
            switch(mt->type)
            {
                case 68:	// Arachnotron
                case 64:	// Archvile
                case 88:	// Boss Brain
                case 89:	// Boss Shooter
                case 69:	// Hell Knight
                case 67:	// Mancubus
                case 71:	// Pain Elemental
                case 65:	// Former Human Commando
                case 66:	// Revenant
                case 84:	// Wolf SS
                spawn = false;
                break;
            }
        }

        if (spawn == false)
        {
            // [BH] Fix <https://doomwiki.org/wiki/Doom_II_monster_exclusion_bug>.
            continue;
        }

        // [crispy] minor fixes to prevent users from getting stuck in levels with mapping errors
        if (singleplayer)
        {
            // [crispy] spawn Former Human instead of Wolf SS in BFG Edition
            // if (gamevariant == bfgedition && mt->type == 84)
            // {
            //     mt->type = 3004;
            // }

            // [crispy] TNT MAP31 has a yellow key that is erroneously marked as multi-player only
            if (tnt && gamemap == 31 && mt->type == 6)
            {
                mt->options &= ~16;
            }
        }

        // Do spawn all other stuff. 
        mt->x = SHORT(mt->x);
        mt->y = SHORT(mt->y);
        mt->angle = SHORT(mt->angle);
        mt->type = SHORT(mt->type);
        mt->options = SHORT(mt->options);

        P_SpawnMapThing (mt);
    }
	
    if (!deathmatch)
        for (i = 0; i < MAXPLAYERS; i++)
            if (playeringame[i] && !playerstartsingame[i])
                I_Error(english_language ?
                "P_LoadThings: Player %d start missing" :
                "P_LoadThings: Отсутствует стартовая точка игрока №%d", i + 1);

    Z_Free (data);
}


//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs (int lump)
{
    int            i;
    byte          *data;
    line_t        *ld;
    vertex_t      *v1;
    vertex_t      *v2;
    maplinedef_t  *mld;

    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);	
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    mld = (maplinedef_t *)data;
    ld = lines;

    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
        ld->flags = SHORT(mld->flags);
        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);
        v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
        v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;

        if (!ld->dx)
        {
            ld->slopetype = ST_VERTICAL;
        }
        else if (!ld->dy)
        {
            ld->slopetype = ST_HORIZONTAL;
        }
        else
        {
            if (FixedDiv (ld->dy , ld->dx) > 0)
            ld->slopetype = ST_POSITIVE;
            else
            ld->slopetype = ST_NEGATIVE;
        }

        if (v1->x < v2->x)
        {
            ld->bbox[BOXLEFT] = v1->x;
            ld->bbox[BOXRIGHT] = v2->x;
        }
        else
        {
            ld->bbox[BOXLEFT] = v2->x;
            ld->bbox[BOXRIGHT] = v1->x;
        }

        if (v1->y < v2->y)
        {
            ld->bbox[BOXBOTTOM] = v1->y;
            ld->bbox[BOXTOP] = v2->y;
        }
        else
        {
            ld->bbox[BOXBOTTOM] = v2->y;
            ld->bbox[BOXTOP] = v1->y;
        }

        // [crispy] calculate sound origin of line to be its midpoint
        ld->soundorg.x = ld->bbox[BOXLEFT] / 2 + ld->bbox[BOXRIGHT] / 2;
        ld->soundorg.y = ld->bbox[BOXTOP] / 2 + ld->bbox[BOXBOTTOM] / 2;

        ld->sidenum[0] = SHORT(mld->sidenum[0]);
        ld->sidenum[1] = SHORT(mld->sidenum[1]);

        if (ld->sidenum[0] != -1)
        {
            ld->frontsector = sides[ld->sidenum[0]].sector;
        }
        else
        {
            ld->frontsector = 0;
        }

        if (ld->sidenum[1] != -1)
        {
            ld->backsector = sides[ld->sidenum[1]].sector;
        }
        else
        {
            ld->backsector = 0;
        }
    }

    Z_Free (data);
}


//
// P_LoadSideDefs
//
void P_LoadSideDefs (int lump)
{
    int            i;
    byte          *data;
    side_t        *sd;
    mapsidedef_t  *msd;

    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);	
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    msd = (mapsidedef_t *)data;
    sd = sides;

    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
        sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);
        sd->sector = &sectors[SHORT(msd->sector)];
    }

    Z_Free (data);
}

// -----------------------------------------------------------------------------
// P_CreateBlockMap
// [crispy] taken from mbfsrc/P_SETUP.C:547-707, slightly adapted
// -----------------------------------------------------------------------------

static void P_CreateBlockMap(void)
{
    int i;
    fixed_t minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

    // First find limits of map

    for (i=0; i<numvertexes; i++)
    {
        if (vertexes[i].x >> FRACBITS < minx)
        {
            minx = vertexes[i].x >> FRACBITS;
        }
        else if (vertexes[i].x >> FRACBITS > maxx)
        {
            maxx = vertexes[i].x >> FRACBITS;
        }
        if (vertexes[i].y >> FRACBITS < miny)
        {
            miny = vertexes[i].y >> FRACBITS;
        }
        else if (vertexes[i].y >> FRACBITS > maxy)
        {
            maxy = vertexes[i].y >> FRACBITS;
        }
    }

    // Save blockmap parameters

    bmaporgx = minx << FRACBITS;
    bmaporgy = miny << FRACBITS;
    bmapwidth  = ((maxx-minx) >> MAPBTOFRAC) + 1;
    bmapheight = ((maxy-miny) >> MAPBTOFRAC) + 1;

    // Compute blockmap, which is stored as a 2d array of variable-sized lists.
    //
    // Pseudocode:
    //
    // For each linedef:
    //
    //   Map the starting and ending vertices to blocks.
    //
    //   Starting in the starting vertex's block, do:
    //
    //     Add linedef to current block's list, dynamically resizing it.
    //
    //     If current block is the same as the ending vertex's block, exit loop.
    //
    //     Move to an adjacent block by moving towards the ending block in
    //     either the x or y direction, to the block which contains the linedef.

    {
        typedef struct { int n, nalloc, *list; } bmap_t;  // blocklist structure
        unsigned tot = bmapwidth * bmapheight;            // size of blockmap
        bmap_t *bmap = calloc(sizeof *bmap, tot);         // array of blocklists
        int x, y, adx, ady, bend;

        for (i=0; i < numlines; i++)
        {
            int dx, dy, diff, b;

            // starting coordinates
            x = (lines[i].v1->x >> FRACBITS) - minx;
            y = (lines[i].v1->y >> FRACBITS) - miny;

            // x-y deltas
            adx = lines[i].dx >> FRACBITS, dx = adx < 0 ? -1 : 1;
            ady = lines[i].dy >> FRACBITS, dy = ady < 0 ? -1 : 1;

            // difference in preferring to move across y (>0) instead of x (<0)
            diff = !adx ? 1 : !ady ? -1 :
            (((x >> MAPBTOFRAC) << MAPBTOFRAC) +
            (dx > 0 ? MAPBLOCKUNITS-1 : 0) - x) * (ady = abs(ady)) * dx -
            (((y >> MAPBTOFRAC) << MAPBTOFRAC) +
            (dy > 0 ? MAPBLOCKUNITS-1 : 0) - y) * (adx = abs(adx)) * dy;

            // starting block, and pointer to its blocklist structure
            b = (y >> MAPBTOFRAC)*bmapwidth + (x >> MAPBTOFRAC);

            // ending block
            bend = (((lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC)
                 * bmapwidth + (((lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);

            // delta for pointer when moving across y
            dy *= bmapwidth;

            // deltas for diff inside the loop
            adx <<= MAPBTOFRAC;
            ady <<= MAPBTOFRAC;

            // Now we simply iterate block-by-block until we reach the end block.
            while ((unsigned) b < tot)    // failsafe -- should ALWAYS be true
            {
                // Increase size of allocated list if necessary
                if (bmap[b].n >= bmap[b].nalloc)
                    bmap[b].list = realloc(bmap[b].list,
                   (bmap[b].nalloc = bmap[b].nalloc ?
                    bmap[b].nalloc*2 : 8)*sizeof*bmap->list);

                // Add linedef to end of list
                bmap[b].list[bmap[b].n++] = i;

                // If we have reached the last block, exit
                if (b == bend)
                {
                    break;
                }

                // Move in either the x or y direction to the next block
                if (diff < 0)
                {
                    diff += ady, b += dx;
                }
                else
                {
                    diff -= adx, b += dy;
                }
            }
        }

        // Compute the total size of the blockmap.
        //
        // Compression of empty blocks is performed by reserving two offset words
        // at tot and tot+1.
        //
        // 4 words, unused if this routine is called, are reserved at the start.

        {
            int count = tot+6;  // we need at least 1 word per block, plus reserved's
        
            for (i = 0; i < tot; i++)
                if (bmap[i].n)
                    count += bmap[i].n + 2; // 1 header word + 1 trailer word + blocklist
        
            // Allocate blockmap lump with computed count
            blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);
        }

        // Now compress the blockmap.
        {
            int ndx = tot += 4;         // Advance index to start of linedef lists
            bmap_t *bp = bmap;          // Start of uncompressed blockmap

            blockmaplump[ndx++] = 0;    // Store an empty blockmap list at start
            blockmaplump[ndx++] = -1;   // (Used for compression)

            for (i = 4; i < tot; i++, bp++)
                if (bp->n)                                      // Non-empty blocklist
                {
                    blockmaplump[blockmaplump[i] = ndx++] = 0;  // Store index & header
                    do
                    blockmaplump[ndx++] = bp->list[--bp->n];    // Copy linedef list
                    while (bp->n);
                    blockmaplump[ndx++] = -1;                   // Store trailer
                    free(bp->list);                             // Free linedef list
                }
                else            // Empty blocklist: point to reserved empty blocklist
                blockmaplump[i] = tot;
        
            free(bmap);    // Free uncompressed blockmap
        }
    }

    // [crispy] copied over from P_LoadBlockMap()
    {
        int count = sizeof(*blocklinks) * bmapwidth * bmapheight;
        blocklinks = Z_Malloc(count, PU_LEVEL, 0);
        memset(blocklinks, 0, count);
        blockmap = blockmaplump+4;
    }
}

//
// P_LoadBlockMap
//
boolean P_LoadBlockMap (int lump)
{
    int i;
    int count;
    int lumplen;
    short *wadblockmaplump;

    // blockmap = blockmaplump+4;

    // [crispy] (re-)create BLOCKMAP if necessary
    if (M_CheckParm("-blockmap") ||
        lump >= numlumps ||
        (lumplen = W_LumpLength(lump)) < 8 ||
        (count = lumplen / 2) >= 0x10000)
    {
        return false;
    }

    // [crispy] remove BLOCKMAP limit
    // adapted from boom202s/P_SETUP.C:1025-1076
    wadblockmaplump = Z_Malloc(lumplen, PU_LEVEL, NULL);
    W_ReadLump(lump, wadblockmaplump);
    blockmaplump = W_CacheLumpNum (lump,PU_LEVEL);
    blockmap = blockmaplump + 4;

    blockmaplump[0] = SHORT(wadblockmaplump[0]);
    blockmaplump[1] = SHORT(wadblockmaplump[1]);
    blockmaplump[2] = (int32_t)(SHORT(wadblockmaplump[2])) & 0xffff;
    blockmaplump[3] = (int32_t)(SHORT(wadblockmaplump[3])) & 0xffff;

    // Swap all short integers to native byte ordering.

    for (i=4; i<count; i++)
    {
        short t = SHORT(wadblockmaplump[i]);
        blockmaplump[i] = (t == -1) ? -1l : (int32_t) t & 0xffff;
    }
    
    Z_Free(wadblockmaplump);

    // Read the header

    bmaporgx = blockmaplump[0]<<FRACBITS;
    bmaporgy = blockmaplump[1]<<FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];

    // Clear out mobj chains

    count = sizeof(*blocklinks)* bmapwidth*bmapheight;
    blocklinks = Z_Malloc (count,PU_LEVEL, 0);
    memset (blocklinks, 0, count);

    // [crispy] (re-)create BLOCKMAP if necessary
    return true;
}


//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    
    int           i;
    int           j;
    int           total;
    int           block;
    fixed_t       bbox[4];
    seg_t        *seg;
    line_t       *li;
    sector_t     *sector;
    subsector_t  *ss;
    line_t      **linebuffer;

    // look up sector number for each subsector
    ss = subsectors;

    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
        seg = &segs[ss->firstline];
        ss->sector = seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    total = 0;

    for (i=0 ; i<numlines ; i++, li++)
    {
        total++;
        li->frontsector->linecount++;

        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            total++;
        }
    }

    // build line tables for each sector	
    linebuffer = Z_Malloc (total*4, PU_LEVEL, 0);
    sector = sectors;

    for (i=0 ; i<numsectors ; i++, sector++)
    {
        M_ClearBox (bbox);
        sector->lines = linebuffer;
        li = lines;

        for (j=0 ; j<numlines ; j++, li++)
        {
            if (li->frontsector == sector || li->backsector == sector)
            {
                *linebuffer++ = li;
                M_AddToBox (bbox, li->v1->x, li->v1->y);
                M_AddToBox (bbox, li->v2->x, li->v2->y);
            }
        }

        if (linebuffer - sector->lines != sector->linecount)
        {
	        I_Error (english_language ?
                     "P_GroupLines: miscounted" :
                     "P_GroupLines: Ошибка в расчете");
        }

        // set the degenmobj_t to the middle of the bounding box
        sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
        sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;

        // adjust bounding box to map blocks
        block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapheight ? bmapheight-1 : block;
        sector->blockbox[BOXTOP]=block;

        block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXBOTTOM]=block;

        block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapwidth ? bmapwidth-1 : block;
        sector->blockbox[BOXRIGHT]=block;

        block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXLEFT]=block;
    }
}


//
// [crispy] remove slime trails
// mostly taken from Lee Killough's implementation in mbfsrc/P_SETUP.C:849-924,
// with the exception that not the actual vertex coordinates are modified,
// but pseudovertexes which are dummies that are *only* used in rendering,
// i.e. r_bsp.c:R_AddLine()
//
static void P_RemoveSlimeTrails(void)
{
    int i;

    for (i = 0 ; i < numsegs ; i++)
    {
        const line_t *l = segs[i].linedef;
        vertex_t *v = segs[i].v1;

        // [crispy] ignore exactly vertical or horizontal linedefs
        if (l->dx && l->dy)
        {
            do
            {
                // [crispy] vertex wasn't already moved
                if (!v->moved)
                {
                    v->moved = true;
                    // [crispy] ignore endpoints of linedefs
                    if (v != l->v1 && v != l->v2)
                    {
                        // [crispy] move the vertex towards the linedef
                        // by projecting it using the law of cosines
                        int64_t dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
                        int64_t dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
                        int64_t dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
                        int64_t s = dx2 + dy2;
                        int x0 = v->x, y0 = v->y, x1 = l->v1->x, y1 = l->v1->y;

                        // [crispy] MBF actually overrides v->x and v->y here
                        v->px = (fixed_t)((dx2 * x0 + dy2 * x1 + dxy * (y0 - y1)) / s);
                        v->py = (fixed_t)((dy2 * y0 + dx2 * y1 + dxy * (x0 - x1)) / s);
                    }
                }
                // [crispy] if v doesn't point to the second
                // vertex of the seg already, point it there
            } while ((v != segs[i].v2) && (v = segs[i].v2));
        }
    }
}


//
// P_SetupLevel
//
void P_SetupLevel (int episode, int map, int playermask, skill_t skill)
{
    int   i;
    int   lumpnum;
    char  lumpname[9];
    boolean crispy_validblockmap;
	
    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        players[i].killcount = players[i].secretcount  = players[i].itemcount = 0;
    }

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1; 

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start ();			

    Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);

    P_InitThinkers ();

    // if working with a devlopment map, reload it
    W_Reload ();			

    // find map name
    if (commercial)
    {
        if (map<10)
        {
            sprintf (lumpname,"map0%i", map);
        }
        else
        {
            sprintf (lumpname,"map%i", map);
        }
    }
    else
    {
        lumpname[0] = 'E';
        lumpname[1] = '0' + episode;
        lumpname[2] = 'M';
        lumpname[3] = '0' + map;
        lumpname[4] = 0;
    }

    lumpnum = W_GetNumForName (lumpname);

    leveltime = 0;

    // note: most of this ordering is important	
    crispy_validblockmap = P_LoadBlockMap (lumpnum+ML_BLOCKMAP); // [crispy] (re-)create BLOCKMAP if necessary
    P_LoadVertexes (lumpnum+ML_VERTEXES);
    P_LoadSectors (lumpnum+ML_SECTORS);
    P_LoadSideDefs (lumpnum+ML_SIDEDEFS);
    P_LoadLineDefs (lumpnum+ML_LINEDEFS);

    // [crispy] (re-)create BLOCKMAP if necessary
    if (!crispy_validblockmap)
    P_CreateBlockMap();

    P_LoadSubsectors (lumpnum+ML_SSECTORS);
    P_LoadNodes (lumpnum+ML_NODES);
    P_LoadSegs (lumpnum+ML_SEGS);
    rejectmatrix = W_CacheLumpNum (lumpnum+ML_REJECT,PU_LEVEL);
    P_GroupLines ();
    // [crispy] remove slime trails
    P_RemoveSlimeTrails();
    // [crispy] fix long wall wobble
    P_SegLengths();

    // [crispy] blinking key or skull in the status bar
    memset(st_keyorskull, 0, sizeof(st_keyorskull));

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    P_LoadThings (lumpnum+ML_THINGS);

    // if deathmatch, randomly spawn the active players
    if (deathmatch)
    {
        for (i=0 ; i<MAXPLAYERS ; i++)
            if (playeringame[i])
            {
                players[i].mo = NULL;
                G_DeathMatchSpawnPlayer (i);
            }
    }

    // [JN] killough 3/26/98: Spawn icon landings:
    if (commercial)
    {
        P_SpawnBrainTargets();
    }

    // clear special respawning que
    iquehead = iquetail = 0;		

    // set up world state
    P_SpawnSpecials ();
	
    // preload graphics
    if (precache)
    {
        R_PrecacheLevel ();
    }
}


//
// P_Init
//
void P_Init (void)
{
    P_InitSwitchList ();
    P_InitPicAnims ();
    R_InitSprites (sprnames);
}

