//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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
// P_main.c



#include <math.h>
#include <stdlib.h>

#include "d_mode.h"
#include "doomdef.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "p_fix.h"
#include "p_local.h"
#include "s_sound.h"
#include "jn.h"

// [crispy] support maps with compressed ZDBSP nodes
#include "config.h"
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif


void P_SpawnMapThing(mapthing_t * mthing);

int numvertexes;
vertex_t *vertexes;

int numsegs;
seg_t *segs;

int numsectors;
sector_t *sectors;

int numsubsectors;
subsector_t *subsectors;

int numnodes;
node_t *nodes;

int numlines;
line_t *lines;

int numsides;
side_t *sides;

static int totallines;

// [JN] Is applying map-specific fixes allowed?
boolean canmodify;
GameMission_t gamemission;

int32_t *blockmaplump;          // offsets in blockmap are from here // [crispy] BLOCKMAP limit
int32_t *blockmap;              // [crispy] BLOCKMAP limit
int      bmapwidth, bmapheight; // in mapblocks
fixed_t  bmaporgx, bmaporgy;    // origin of block map
mobj_t **blocklinks;            // for thing chains

byte *rejectmatrix;             // for fast sight rejection

mapthing_t deathmatchstarts[10], *deathmatch_p;
mapthing_t playerstarts[MAXPLAYERS];

typedef enum
{
    DOOMBSP = 0x000,
    DEEPBSP = 0x001,
    ZDBSPX  = 0x002,
    ZDBSPZ  = 0x004,
    HEXEN   = 0x100,
} mapformat_t;

/*
================================================================================
=
= GetOffset
=
= [crispy] recalculate seg offsets
= adapted from prboom-plus/src/p_setup.c:474-482
=
================================================================================
*/

fixed_t GetOffset (vertex_t *v1, vertex_t *v2)
{
    fixed_t dx, dy;
    fixed_t r;

    dx = (v1->x - v2->x)>>FRACBITS;
    dy = (v1->y - v2->y)>>FRACBITS;
    r = (fixed_t)(sqrt(dx*dx + dy*dy))<<FRACBITS;

    return r;
}

/*
================================================================================
=
= P_LoadVertexes
=
================================================================================
*/

void P_LoadVertexes (int lump)
{
    int           i;
    byte         *data;
    mapvertex_t  *ml;
    vertex_t     *li;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);	

    // Load data into cache.
    data = W_CacheLumpNum (lump, PU_STATIC);
	
    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
        li->x = SHORT(ml->x)<<FRACBITS;
        li->y = SHORT(ml->y)<<FRACBITS;

        // [JN] Apply any map-specific fixes.
        if (canmodify && fix_map_errors)
        {
            for (int j = 0; vertexfix[j].mission != 0; j++)
            {
                if (gameepisode == vertexfix[j].epsiode
                && gamemap == vertexfix[j].map
                && i == vertexfix[j].vertex
                && vertexes[i].x == SHORT(vertexfix[j].oldx) << FRACBITS
                && vertexes[i].y == SHORT(vertexfix[j].oldy) << FRACBITS)
                {
                    vertexes[i].x = SHORT(vertexfix[j].newx) << FRACBITS;
                    vertexes[i].y = SHORT(vertexfix[j].newy) << FRACBITS;

                    break;
                }
            }
        }

        // [crispy] initialize pseudovertexes with actual vertex coordinates
        li->px = li->x;
        li->py = li->y;
        li->moved = false;
    }

    // Free buffer memory.
    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= GetSectorAtNullAddress
=
================================================================================
*/

sector_t *GetSectorAtNullAddress (void)
{
    static boolean null_sector_is_initialized = false;
    static sector_t null_sector;

    if (!null_sector_is_initialized)
    {
        memset(&null_sector, 0, sizeof(null_sector));
        I_GetMemoryValue(0, &null_sector.floorheight, 4);
        I_GetMemoryValue(4, &null_sector.ceilingheight, 4);
        null_sector_is_initialized = true;
    }

    return &null_sector;
}

/*
================================================================================
=
= P_LoadSegs
=
================================================================================
*/

void P_LoadSegs(int lump)
{
    int        i;
    int        linedef;
    int        side;
    int        sidenum;
    byte      *data;
    mapseg_t  *ml;
    seg_t     *li;
    line_t    *ldef;

    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);	
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    ml = (mapseg_t *)data;
    li = segs;

    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
        li->v1 = &vertexes[(unsigned short)SHORT(ml->v1)]; // [crispy] extended nodes
        li->v2 = &vertexes[(unsigned short)SHORT(ml->v2)]; // [crispy] extended nodes
        li->angle = (SHORT(ml->angle))<<FRACBITS;
        linedef = (unsigned short)SHORT(ml->linedef); // [crispy] extended nodes
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: check for wrong indexes
        if ((unsigned)ldef->sidenum[side] >= (unsigned)numsides)
        {
            I_Error(english_language ?
                    "P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d" :
                    "P_LoadSegs: линия %d для сегмента %d указывает на несуществующую сторону %d",
                    linedef, i, (unsigned)ldef->sidenum[side]);
        }

        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;
        li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

        if (ldef-> flags & ML_TWOSIDED)
        {
            sidenum = ldef->sidenum[side ^ 1];

            // If the sidenum is out of range, this may be a "glass hack"
            // impassible window.  Point at side #0 (this may not be
            // the correct Vanilla behavior; however, it seems to work for
            // OTTAWAU.WAD, which is the one place I've seen this trick
            // used).

            if (sidenum < 0 || sidenum >= numsides)
            {
                // [crispy] linedef has two-sided flag set, but no valid second sidedef;
                // but since it has a midtexture, it is supposed to be rendered just
                // like a regular one-sided linedef
                if (li->sidedef->midtexture)
                {
                    li->backsector = 0;
                    fprintf(stderr, english_language ? 
                            "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n" :
                            "P_LoadSegs: линия %d помечена как двусторонняя, но вторая сторона отсутствует\n", i);
                }
                else
                {
                    li->backsector = GetSectorAtNullAddress();
                }
            }
            else
            {
                li->backsector = sides[sidenum].sector;
            }
        }
        else
        {
            li->backsector = 0;
        }

        // [JN] Apply any map-specific fixes.
        if (canmodify && fix_map_errors)
        {
            for (int j = 0; linefix[j].mission != 0; j++)
            {
                if (gameepisode == linefix[j].epsiode
                && gamemap == linefix[j].map
                && linedef == linefix[j].linedef
                && side == linefix[j].side)
                {
                    if (*linefix[j].toptexture)
                    {
                        li->sidedef->toptexture = R_TextureNumForName(linefix[j].toptexture);
                    }

                    if (*linefix[j].middletexture)
                    {
                        li->sidedef->midtexture = R_TextureNumForName(linefix[j].middletexture);
                    }

                    if (*linefix[j].bottomtexture)
                    {
                        li->sidedef->bottomtexture = R_TextureNumForName(linefix[j].bottomtexture);
                    }

                    if (linefix[j].offset != DEFAULT)
                    {
                        li->offset = SHORT(linefix[j].offset) << FRACBITS;
                        li->sidedef->textureoffset = 0;
                    }

                    if (linefix[j].rowoffset != DEFAULT)
                    {
                        li->sidedef->rowoffset = SHORT(linefix[j].rowoffset) << FRACBITS;
                    }

                    if (linefix[j].flags != DEFAULT)
                    {
                        if (li->linedef->flags & linefix[j].flags)
                            li->linedef->flags &= ~linefix[j].flags;
                        else
                            li->linedef->flags |= linefix[j].flags;
                    }
                    if (linefix[j].special != DEFAULT)
                    {
                        li->linedef->special = linefix[j].special;
                    }

                    if (linefix[j].tag != DEFAULT)
                    {
                        li->linedef->tag = linefix[j].tag;
                    }

                    break;
                }
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadSegs_DeePBSP
=
= [crispy] support maps with DeePBSP nodes
= adapted from prboom-plus/src/p_setup.c:633-752
=
================================================================================
*/

static void P_LoadSegs_DeePBSP (int lump)
{
    int i;
    mapseg_deepbsp_t *data;

    numsegs = W_LumpLength(lump) / sizeof(mapseg_deepbsp_t);
    segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
    data = (mapseg_deepbsp_t *)W_CacheLumpNum(lump, PU_STATIC);

    for (i = 0; i < numsegs; i++)
    {
        seg_t *li = segs + i;
        mapseg_deepbsp_t *ml = data + i;
        int side, linedef;
        line_t *ldef;

        li->v1 = &vertexes[ml->v1];
        li->v2 = &vertexes[ml->v2];
        li->angle = (SHORT(ml->angle))<<16;

        linedef = (unsigned short)SHORT(ml->linedef);
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;
        li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

        if (ldef->flags & ML_TWOSIDED)
        {
            int sidenum = ldef->sidenum[side ^ 1];
        
            if (sidenum < 0 || sidenum >= numsides)
            {
                if (li->sidedef->midtexture)
                {
                    li->backsector = 0;
                    fprintf(stderr, english_language ?
                            "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n" :
                            "P_LoadSegs: линия %d помечена как двусторонняя, но вторая сторона отсутствует\n", i);
                }
                else
                {
                    li->backsector = GetSectorAtNullAddress();
                }
            }
            else
            {
                li->backsector = sides[sidenum].sector;
            }
        }
        else
        {
            li->backsector = 0;
        }
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_SegLengths
=
= [crispy] fix long wall wobble
=
================================================================================
*/

void P_SegLengths (void)
{
    int      i;

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

/*
================================================================================
=
= P_LoadSubsectors
=
================================================================================
*/

void P_LoadSubsectors (int lump)
{
    int              i;
    byte            *data;
    mapsubsector_t  *ms;
    subsector_t     *ss;

    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);

    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;

    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
        ss->numlines = (unsigned short)SHORT(ms->numsegs);
        ss->firstline = (unsigned short)SHORT(ms->firstseg);
    }
	
    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadSubsectors_DeePBSP
=
= [crispy] support maps with DeePBSP nodes
= adapted from prboom-plus/src/p_setup.c:843-863
=
================================================================================
*/

static void P_LoadSubsectors_DeePBSP (int lump)
{
    int i;
    mapsubsector_deepbsp_t *data;

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_deepbsp_t);
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
    data = (mapsubsector_deepbsp_t *)W_CacheLumpNum(lump, PU_STATIC);

    for (i = 0; i < numsubsectors; i++)
    {
        subsectors[i].numlines = (int)data[i].numsegs;
        subsectors[i].firstline = (int)data[i].firstseg;
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadSectors
=
================================================================================
*/

void P_LoadSectors (int lump)
{
    int          i;
    byte        *data;
    mapsector_t *ms;
    sector_t    *ss;

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

        if (!detailLevel)
        {
           // [crispy] WiggleFix: [kb] for R_FixWiggle()
            ss->cachedheight = 0;
        }

        // [AM] Sector interpolation.  Even if we're
        //      not running uncapped, the renderer still
        //      uses this data.
        ss->oldfloorheight = ss->floorheight;
        ss->interpfloorheight = ss->floorheight;
        ss->oldceilingheight = ss->ceilingheight;
        ss->interpceilingheight = ss->ceilingheight;
        // [crispy] inhibit sector interpolation during the 0th gametic
        ss->oldgametic = -1;

        // [JN] Apply any map-specific fixes.
        if (canmodify && fix_map_errors)
        {
            for (int j = 0; sectorfix[j].mission != 0; j++)
            {
                if (gameepisode == sectorfix[j].epsiode
                && gamemap == sectorfix[j].map
                && i == sectorfix[j].sector)
                {
                    if (*sectorfix[j].floorpic)
                    {
                        ss->floorpic = R_FlatNumForName(sectorfix[j].floorpic);
                    }
    
                    if (*sectorfix[j].ceilingpic)
                    {
                        ss->ceilingpic = R_FlatNumForName(sectorfix[j].ceilingpic);
                    }
    
                    if (sectorfix[j].floorheight != DEFAULT)
                    {
                        ss->floorheight = SHORT(sectorfix[j].floorheight) << FRACBITS;
                    }
    
                    if (sectorfix[j].ceilingheight != DEFAULT)
                    {
                        ss->ceilingheight = SHORT(sectorfix[j].ceilingheight) << FRACBITS;
                    }
    
                    if (sectorfix[j].special != DEFAULT)
                    {
                        ss->special = SHORT(sectorfix[j].special);
                    }
    
                    if (sectorfix[j].newtag != DEFAULT && (sectorfix[j].oldtag == DEFAULT
                        || sectorfix[j].oldtag == ss->tag))
                    {
                        ss->tag = SHORT(sectorfix[j].newtag) << FRACBITS;
                    }
    
                    break;
                }
            }
        }

    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadNodes
=
================================================================================
*/

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
            no->children[j] = (unsigned short)SHORT(mn->children[j]);

            // [crispy] support for extended nodes
            if (no->children[j] == 0xFFFF)
            {
                no->children[j] = -1;
            }
            else if (no->children[j] & 0x8000)
            {
                no->children[j] &= ~0x8000;

                if (no->children[j] >= numsubsectors)
                    no->children[j] = 0;

                no->children[j] |= NF_SUBSECTOR;
            }

            for (k=0 ; k<4 ; k++)
            {
                no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadNodes_DeePBSP
=
= [crispy] support maps with DeePBSP nodes
= adapted from prboom-plus/src/p_setup.c:995-1038
=
================================================================================
*/

static void P_LoadNodes_DeePBSP (int lump)
{
    int         i;
    const byte *data;

    numnodes = (W_LumpLength (lump) - 8) / sizeof(mapnode_deepbsp_t);
    nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);
    data = W_CacheLumpNum (lump, PU_STATIC);

    // [crispy] warn about missing nodes
    if (!data || !numnodes)
    {
        if (numsubsectors == 1)
        {
            fprintf(stderr, english_language ?
                    "P_LoadNodes: No nodes in map, but only one subsector\n" :
                    "P_LoadNodes: у уровня отсутствуют ноды, доступен только один подсектор\n");
        }
        else
        {
            I_Error(english_language ? 
                    "P_LoadNodes: No nodes in map!" :
                    "P_LoadNodes: у уровня отсутсвуют ноды!");
        }
    }

    // skip header
    data += 8;

    for (i = 0; i < numnodes; i++)
    {
        node_t *no = nodes + i;
        mapnode_deepbsp_t *mn = (mapnode_deepbsp_t *) data + i;
        int j;

        no->x = SHORT(mn->x)<<FRACBITS;
        no->y = SHORT(mn->y)<<FRACBITS;
        no->dx = SHORT(mn->dx)<<FRACBITS;
        no->dy = SHORT(mn->dy)<<FRACBITS;

        for (j = 0; j < 2; j++)
        {
            int k;
            no->children[j] = (unsigned int)(mn->children[j]);

            for (k = 0; k < 4; k++)
            no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadNodes_ZDBSP
=
= [crispy] support maps with compressed or uncompressed ZDBSP nodes
= adapted from prboom-plus/src/p_setup.c:1040-1331
= heavily modified, condensed and simplyfied
= - removed most paranoid checks, brought in line with Vanilla P_LoadNodes()
= - removed const type punning pointers
= - inlined P_LoadZSegs()
= - added support for compressed ZDBSP nodes
= - added support for flipped levels
=
================================================================================
*/

static void P_LoadNodes_ZDBSP (int lump, boolean compressed)
{
    byte *data;
    unsigned int i;
#ifdef HAVE_LIBZ
    byte *output;
#endif

    unsigned int orgVerts, newVerts;
    unsigned int numSubs, currSeg;
    unsigned int numSegs;
    unsigned int numNodes;
    vertex_t *newvertarray = NULL;

    data = W_CacheLumpNum(lump, PU_LEVEL);

    // 0. Uncompress nodes lump (or simply skip header)

    if (compressed)
    {
#ifdef HAVE_LIBZ
        const int len =  W_LumpLength(lump);
        int outlen, err;
        z_stream *zstream;

        // first estimate for compression rate:
        // output buffer size == 2.5 * input size
        outlen = 2.5 * len;
        output = Z_Malloc(outlen, PU_STATIC, 0);

        // initialize stream state for decompression
        zstream = malloc(sizeof(*zstream));
        memset(zstream, 0, sizeof(*zstream));
        zstream->next_in = data + 4;
        zstream->avail_in = len - 4;
        zstream->next_out = output;
        zstream->avail_out = outlen;

        if (inflateInit(zstream) != Z_OK)
            I_Error(english_language ?
                    "P_LoadNodes: Error during ZDBSP nodes decompression initialization!" :
                    "P_LoadNodes: ошибка при инициализации распаковки нодов ZDBSP!");

        // resize if output buffer runs full
        while ((err = inflate(zstream, Z_SYNC_FLUSH)) == Z_OK)
        {
            int outlen_old = outlen;
            outlen = 2 * outlen_old;
            output = I_Realloc(output, outlen);
            zstream->next_out = output + outlen_old;
            zstream->avail_out = outlen - outlen_old;
        }

        if (err != Z_STREAM_END)
            I_Error(english_language ?
                    "P_LoadNodes: Error during ZDBSP nodes decompression!" :
                    "P_LoadNodes: ошибка при распаковке нодов ZDBSP!");

        fprintf(stderr, english_language ?
                "P_LoadNodes: ZDBSP nodes compression ratio %.3f\n" :
                "P_LoadNodes: степень сжатия нодов ZDBSP: %.3f\n",
                (float)zstream->total_out/zstream->total_in);

        data = output;
        len = zstream->total_out;

        if (inflateEnd(zstream) != Z_OK)
            I_Error(english_language ?
                    "P_LoadNodes: Error during ZDBSP nodes decompression shut-down!" :
                    "P_LoadNodes: ошибка при завершении распаковки нодов ZDBSP!");

        // release the original data lump
        W_ReleaseLumpNum(lump);
        free(zstream);
#else
        I_Error(english_language ?
                "P_LoadNodes: Compressed ZDBSP nodes are not supported!" :
                "P_LoadNodes: сжатые ноды формата ZDBSP не поддерживаются!");
#endif
    }
    else
    {
        // skip header
        data += 4;
    }

    // 1. Load new vertices added during node building

    orgVerts = *((unsigned int*)data);
    data += sizeof(orgVerts);

    newVerts = *((unsigned int*)data);
    data += sizeof(newVerts);

    if (orgVerts + newVerts == (unsigned int)numvertexes)
    {
        newvertarray = vertexes;
    }
    else
    {
        newvertarray = Z_Malloc((orgVerts + newVerts) * sizeof(vertex_t), PU_LEVEL, 0);
        memset(newvertarray, 0, (orgVerts + newVerts) * sizeof(vertex_t));
        memcpy(newvertarray, vertexes, orgVerts * sizeof(vertex_t));
    }

    for (i = 0; i < newVerts; i++)
    {
        newvertarray[i + orgVerts].px =
        newvertarray[i + orgVerts].x = *((unsigned int*)data);
        data += sizeof(newvertarray[0].x);
        
        newvertarray[i + orgVerts].py =
        newvertarray[i + orgVerts].y = *((unsigned int*)data);
        data += sizeof(newvertarray[0].y);
    }

    if (vertexes != newvertarray)
    {
        for (i = 0; i < (unsigned int)numlines; i++)
        {
            lines[i].v1 = lines[i].v1 - vertexes + newvertarray;
            lines[i].v2 = lines[i].v2 - vertexes + newvertarray;
        }

        Z_Free(vertexes);
        vertexes = newvertarray;
        numvertexes = orgVerts + newVerts;
    }

    // 2. Load subsectors

    numSubs = *((unsigned int*)data);
    data += sizeof(numSubs);

    if (numSubs < 1)
	I_Error(english_language ?
            "P_LoadNodes: No subsectors in map!" :
            "P_LoadNodes: на уровне отсутствуют подсекторы!");

    numsubsectors = numSubs;
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);

    for (i = currSeg = 0; i < numsubsectors; i++)
    {
        mapsubsector_zdbsp_t *mseg = (mapsubsector_zdbsp_t*) data + i;

        subsectors[i].firstline = currSeg;
        subsectors[i].numlines = mseg->numsegs;
        currSeg += mseg->numsegs;
    }

    data += numsubsectors * sizeof(mapsubsector_zdbsp_t);

    // 3. Load segs

    numSegs = *((unsigned int*)data);
    data += sizeof(numSegs);

    // The number of stored segs should match the number of segs used by subsectors
    if (numSegs != currSeg)
    {
        I_Error(english_language ?
                "P_LoadNodes: Incorrect number of segs in ZDBSP nodes!" :
                "P_LoadNodes: некорректное количество сегментов в нодах ZDBSP!");
    }

    numsegs = numSegs;
    segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);

    for (i = 0; i < numsegs; i++)
    {
        line_t *ldef;
        unsigned int linedef;
        unsigned char side;
        seg_t *li = segs + i;
        mapseg_zdbsp_t *ml = (mapseg_zdbsp_t *) data + i;

        li->v1 = &vertexes[ml->v1];
        li->v2 = &vertexes[ml->v2];

        linedef = (unsigned short)SHORT(ml->linedef);
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = ml->side;
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;

        // seg angle and offset are not included
        li->angle = R_PointToAngle2(segs[i].v1->x, segs[i].v1->y, segs[i].v2->x, segs[i].v2->y);
        li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

        if (ldef->flags & ML_TWOSIDED)
        {
            int sidenum = ldef->sidenum[side ^ 1];
        
            if (sidenum < 0 || sidenum >= numsides)
            {
                if (li->sidedef->midtexture)
                {
                    li->backsector = 0;
                    fprintf(stderr, english_language ?
                            "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n" :
                            "P_LoadSegs: линия %d помечена как двусторонняя, но вторая сторона отсутствует\n", i);
                }
                else
                {
                    li->backsector = GetSectorAtNullAddress();
                }
            }
            else
            {
                li->backsector = sides[sidenum].sector;
            }
        }
        else
        {
            li->backsector = 0;
        }
    }

    data += numsegs * sizeof(mapseg_zdbsp_t);

    // 4. Load nodes

    numNodes = *((unsigned int*)data);
    data += sizeof(numNodes);

    numnodes = numNodes;
    nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);

    for (i = 0; i < numnodes; i++)
    {
        int j, k;
        node_t *no = nodes + i;
        mapnode_zdbsp_t *mn = (mapnode_zdbsp_t *) data + i;

        no->x = SHORT(mn->x)<<FRACBITS;
        no->y = SHORT(mn->y)<<FRACBITS;
        no->dx = SHORT(mn->dx)<<FRACBITS;
        no->dy = SHORT(mn->dy)<<FRACBITS;

        for (j = 0; j < 2; j++)
        {
            no->children[j] = (unsigned int)(mn->children[j]);

            for (k = 0; k < 4; k++)
            no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);

#ifdef HAVE_LIBZ
    if (compressed)
    {
        Z_Free(output);
    }
#endif
}

/*
================================================================================
=
= P_LoadThings
=
================================================================================
*/

void P_LoadThings (int lump)
{
    int    i;
    int    numthings;
    byte  *data;
    mapthing_t  spawnthing;
    mapthing_t *mt;

    data = W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_t);
    mt = (mapthing_t *) data;

    for (i = 0; i < numthings; i++, mt++)
    {
        spawnthing.x = SHORT(mt->x);
        spawnthing.y = SHORT(mt->y);
        spawnthing.angle = SHORT(mt->angle);
        spawnthing.type = SHORT(mt->type);
        spawnthing.options = SHORT(mt->options);

        // [JN] Apply any map-specific fixes.
        if (canmodify && fix_map_errors)
        {
            for (int j = 0; thingfix[j].mission != 0; j++)
            {
                if (gameepisode == thingfix[j].epsiode
                && gamemap == thingfix[j].map
                && i == thingfix[j].thing
                && spawnthing.type == thingfix[j].type
                && spawnthing.x == SHORT(thingfix[j].oldx)
                && spawnthing.y == SHORT(thingfix[j].oldy))
                {
                    // if (thingfix[j].newx == REMOVE && thingfix[j].newy == REMOVE)
                    //     spawn = false;
                    // else
                    {
                        spawnthing.x = SHORT(thingfix[j].newx);
                        spawnthing.y = SHORT(thingfix[j].newy);
                    }
        
                    if (thingfix[j].angle != DEFAULT)
                    {
                        spawnthing.angle = SHORT(thingfix[j].angle);
                    }
        
                    if (thingfix[j].options != DEFAULT)
                    {
                        spawnthing.options = thingfix[j].options;
                    }
        
                    break;
                }
            }
        }

        P_SpawnMapThing(&spawnthing);
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadThings_Hexen
=
= [crispy] allow loading of Hexen-format maps
= adapted from chocolate-doom/src/hexen/p_setup.c:348-400
=
================================================================================
*/

static void P_LoadThings_Hexen (int lump)
{
    int              i;
    int              numthings;
    byte            *data;
    mapthing_t       spawnthing;
    mapthing_hexen_t *mt;

    data = W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_hexen_t);
    mt = (mapthing_hexen_t *) data;

    for (i = 0; i < numthings; i++, mt++)
    {
        spawnthing.x = SHORT(mt->x);
        spawnthing.y = SHORT(mt->y);
        spawnthing.angle = SHORT(mt->angle);
        spawnthing.type = SHORT(mt->type);
        spawnthing.options = SHORT(mt->options);

        P_SpawnMapThing(&spawnthing);
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadLineDefs
=
= Also counts secret lines for intermissions
=
================================================================================
*/

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
        ld->flags = (unsigned short)SHORT(mld->flags);
        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);

        v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)]; // [crispy] extended nodes
        v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)]; // [crispy] extended nodes

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
            {
                ld->slopetype = ST_POSITIVE;
            }
            else
            {
                ld->slopetype = ST_NEGATIVE;
            }
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

        // [crispy] substitute dummy sidedef for missing right side
        if (ld->sidenum[0] == NO_INDEX)
        {
            ld->sidenum[0] = 0;
            fprintf(stderr, english_language ?
                    "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n" :
                    "P_LoadSegs: линия %d помечена как двусторонняя, но вторая сторона отсутствует\n", i);
        }

        if (ld->sidenum[0] != NO_INDEX) // [crispy] extended nodes
        {
            ld->frontsector = sides[ld->sidenum[0]].sector;
        }
        else
        {
            ld->frontsector = 0;
        }

        if (ld->sidenum[1] != NO_INDEX) // [crispy] extended nodes
        {
            ld->backsector = sides[ld->sidenum[1]].sector;
        }
        else
        {
            ld->backsector = 0;
        }
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadLineDefs_Hexen
=
= [crispy] allow loading of Hexen-format maps
= adapted from chocolate-doom/src/hexen/p_setup.c:410-490
=
================================================================================
*/

static void P_LoadLineDefs_Hexen (int lump)
{
    int        i;
    int        warn; // [crispy] warn about unknown linedef types
    byte      *data;
    line_t    *ld;
    vertex_t  *v1, *v2;
    maplinedef_hexen_t *mld;

    numlines = W_LumpLength(lump) / sizeof(maplinedef_hexen_t);
    lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
    memset(lines, 0, numlines * sizeof(line_t));
    data = W_CacheLumpNum(lump, PU_STATIC);
    mld = (maplinedef_hexen_t *) data;
    ld = lines;
    warn = 0; // [crispy] warn about unknown linedef types

    for (i = 0; i < numlines; i++, mld++, ld++)
    {
        ld->flags = (unsigned short)SHORT(mld->flags);
        ld->special = mld->special;

        // [crispy] warn about unknown linedef types
        if ((unsigned short) ld->special > 141)
        {
            fprintf(stderr, english_language ?
                    "P_LoadLineDefs: Unknown special %d at line %d\n" :
                    "P_LoadLineDefs: неизвестная специфика %d у линии %d\n",
                    ld->special, i);
            warn++;
        }

        v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)];
        v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)];

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
            if (FixedDiv(ld->dy, ld->dx) > 0)
            {
                ld->slopetype = ST_POSITIVE;
            }
            else
            {
                ld->slopetype = ST_NEGATIVE;
            }
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

        ld->sidenum[0] = SHORT(mld->sidenum[0]);
        ld->sidenum[1] = SHORT(mld->sidenum[1]);

        if (ld->sidenum[0] != NO_INDEX)
        {
            ld->frontsector = sides[ld->sidenum[0]].sector;
        }
        else
        {
            ld->frontsector = 0;
        }

        if (ld->sidenum[1] != NO_INDEX)
        {
            ld->backsector = sides[ld->sidenum[1]].sector;
        }
        else
        {
            ld->backsector = 0;
        }
    }

    // [crispy] warn about unknown linedef types
    if (warn)
    {
        fprintf(stderr, "P_LoadLineDefs: Found %d line%s with unknown linedef type.\n"
                        "THIS MAP MAY NOT WORK AS EXPECTED!\n", warn, (warn > 1) ? "s" : "");
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_LoadSideDefs
=
================================================================================
*/

void P_LoadSideDefs(int lump)
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
        // [crispy] smooth texture scrolling
        sd->oldtextureoffset = sd->textureoffset;
    }

    W_ReleaseLumpNum(lump);
}

/*
================================================================================
=
= P_CreateBlockMap
=
= [crispy] taken from mbfsrc/P_SETUP.C:547-707, slightly adapted
=
================================================================================
*/

static void P_CreateBlockMap(void)
{
    int i;
    fixed_t minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

    // First find limits of map

    for (i=0; i<numvertexes; i++)
    {
        if (vertexes[i].x >> FRACBITS < minx)
        minx = vertexes[i].x >> FRACBITS;
        else if (vertexes[i].x >> FRACBITS > maxx)
        maxx = vertexes[i].x >> FRACBITS;
        if (vertexes[i].y >> FRACBITS < miny)
        miny = vertexes[i].y >> FRACBITS;
        else if (vertexes[i].y >> FRACBITS > maxy)
        maxy = vertexes[i].y >> FRACBITS;
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
        bend = (((lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC) *
            bmapwidth + (((lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);

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
                bmap[b].list = I_Realloc(bmap[b].list,
               (bmap[b].nalloc = bmap[b].nalloc ?
				bmap[b].nalloc*2 : 8)*sizeof*bmap->list);

            // Add linedef to end of list
            bmap[b].list[bmap[b].n++] = i;

            // If we have reached the last block, exit
            if (b == bend)
            break;

            // Move in either the x or y direction to the next block
            if (diff < 0)
            diff += ady, b += dx;
            else
            diff -= adx, b += dy;
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

/*
================================================================================
=
= P_LoadBlockMap
=
= [crispy] remove BLOCKMAP limit
= adapted from boom202s/P_SETUP.C:1025-1076
=
================================================================================
*/

boolean P_LoadBlockMap (int lump)
{
    int i;
    int count;
    int lumplen;
    short *wadblockmaplump;

    // [crispy] (re-)create BLOCKMAP if necessary
    if (M_CheckParm("-blockmap")
    ||  lump >= numlumps
    || (lumplen = W_LumpLength(lump)) < 8
    || (count = lumplen / 2) >= 0x10000)
    {
        return false;
    }

    // [crispy] remove BLOCKMAP limit
    // adapted from boom202s/P_SETUP.C:1025-1076
    wadblockmaplump = Z_Malloc(lumplen, PU_LEVEL, NULL);
    W_ReadLump(lump, wadblockmaplump);
    blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, NULL);
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

    count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = Z_Malloc(count, PU_LEVEL, 0);
    memset(blocklinks, 0, count);

    // [crispy] (re-)create BLOCKMAP if necessary
    return true;
}

/*
================================================================================
=
= P_GroupLines
=
= Builds sector line lists and subsector sector numbers.
= Finds block bounding boxes for sectors.
================================================================================
*/

void P_GroupLines (void)
{

    int           i;
    int           j;
    int           block;
    fixed_t       bbox[4];
    line_t       *li;
    sector_t     *sector;
    subsector_t  *ss;
    seg_t        *seg;
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
    totallines = 0;
    for (i=0 ; i<numlines ; i++, li++)
    {
        totallines++;
        li->frontsector->linecount++;

        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            totallines++;
        }
    }

    // build line tables for each sector	
    linebuffer = Z_Malloc (totallines*sizeof(line_t *), PU_LEVEL, 0);

    for (i=0; i<numsectors; ++i)
    {
        // Assign the line buffer for this sector
        sectors[i].lines = linebuffer;
        linebuffer += sectors[i].linecount;

        // Reset linecount to zero so in the next stage we can count
        // lines into the list.
        sectors[i].linecount = 0;
    }

    // Assign lines to sectors
    for (i=0; i<numlines; ++i)
    { 
        li = &lines[i];

        if (li->frontsector != NULL)
        {
            sector = li->frontsector;
            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }

        if (li->backsector != NULL && li->frontsector != li->backsector)
        {
            sector = li->backsector;
            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }
    }

    // Generate bounding boxes for sectors
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
        M_ClearBox (bbox);

        for (j=0 ; j<sector->linecount; j++)
        {
                li = sector->lines[j];
                M_AddToBox (bbox, li->v1->x, li->v1->y);
                M_AddToBox (bbox, li->v2->x, li->v2->y);
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

/*
================================================================================
=
= P_RemoveSlimeTrails
=
= [crispy] remove slime trails
= mostly taken from Lee Killough's implementation in mbfsrc/P_SETUP.C:849-924,
= with the exception that not the actual vertex coordinates are modified,
= but pseudovertexes which are dummies that are *only* used in rendering,
= i.e. r_bsp.c:R_AddLine()
=
================================================================================
*/

static void P_RemoveSlimeTrails (void)
{
    int i;

    for (i = 0; i < numsegs; i++)
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
                // [crispy] if v doesn't point to the second vertex of the seg already, point it there
            } while ((v != segs[i].v2) && (v = segs[i].v2));
        }
    }
}

/*
================================================================================
=
= PadRejectArray
=
= Pad the REJECT lump with extra data when the lump is too small,
= to simulate a REJECT buffer overflow in Vanilla Heretic.
=
================================================================================
*/

static void PadRejectArray (byte *array, unsigned int len)
{
    unsigned int  i;
    unsigned int  byte_num;
    unsigned int  padvalue;
    byte         *dest;

    // Values to pad the REJECT array with:

    unsigned int rejectpad[4] =
    {
        ((totallines * 4 + 3) & ~3) + 24,     // Size
        0,                                    // Part of z_zone block header
        50,                                   // PU_LEVEL
        0x1d4a11                              // DOOM_CONST_ZONEID
    };

    // Copy values from rejectpad into the destination array.

    dest = array;

    for (i=0; i<len && i<sizeof(rejectpad); ++i)
    {
        byte_num = i % 4;
        *dest = (rejectpad[i / 4] >> (byte_num * 8)) & 0xff;
        ++dest;
    }

    // We only have a limited pad size.  Print a warning if the
    // REJECT lump is too small.

    if (len > sizeof(rejectpad))
    {
        fprintf(stderr, english_language ?
                        "PadRejectArray: REJECT lump too short to pad! (%i > %i)\n" :
                        "PadRejectArray: блок REJECT слишком мал для заполнения! (%i > %i)\n",
                        len, (int) sizeof(rejectpad));

        // Pad remaining space with 0 (or 0xff, if specified on command line).

        if (M_CheckParm("-reject_pad_with_ff"))
        {
            padvalue = 0xff;
        }
        else
        {
            padvalue = 0xf00;
        }

        memset(array + sizeof(rejectpad), padvalue, len - sizeof(rejectpad));
    }
}

/*
================================================================================
=
= P_LoadReject
=
================================================================================
*/

static void P_LoadReject (int lumpnum)
{
    int minlength;
    int lumplen;

    // Calculate the size that the REJECT lump *should* be.

    minlength = (numsectors * numsectors + 7) / 8;

    // If the lump meets the minimum length, it can be loaded directly.
    // Otherwise, we need to allocate a buffer of the correct size
    // and pad it with appropriate data.

    lumplen = W_LumpLength(lumpnum);

    if (lumplen >= minlength)
    {
        rejectmatrix = W_CacheLumpNum(lumpnum, PU_LEVEL);
    }
    else
    {
        rejectmatrix = Z_Malloc(minlength, PU_LEVEL, &rejectmatrix);
        W_ReadLump(lumpnum, rejectmatrix);

        PadRejectArray(rejectmatrix + lumplen, minlength - lumplen);
    }
}

/*
================================================================================
=
= P_CheckMapFormat
=
= [crispy] support maps with NODES in compressed or uncompressed ZDBSP
= format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
=
================================================================================
*/

mapformat_t P_CheckMapFormat (int lumpnum)
{
    int b;
    mapformat_t format = 0;
    byte *nodes = NULL;

    if ((b = lumpnum+ML_BLOCKMAP+1) < numlumps
    &&  !strncasecmp(lumpinfo[b]->name, "BEHAVIOR", 8))
    {
        fprintf(stderr, english_language ? 
                "P_CheckMapFormat: Hexen format (" :
                "P_CheckMapFormat: формат Hexen (");
        format |= HEXEN;
    }
    else
    {
        fprintf(stderr, english_language ?
                "P_CheckMapFormat: Doom format (" :
                "P_CheckMapFormat: формат Doom (");
    }

    if (!((b = lumpnum+ML_NODES) < numlumps
    && (nodes = W_CacheLumpNum(b, PU_CACHE))
    &&  W_LumpLength(b) > 0))
    {
        fprintf(stderr, english_language ? "no nodes" : "ноды отсутствуют");
    }
    else if (!memcmp(nodes, "xNd4\0\0\0\0", 8))
    {
        fprintf(stderr, "DeePBSP");
        format |= DEEPBSP;
    }
    else if (!memcmp(nodes, "XNOD", 4))
    {
        fprintf(stderr, "ZDBSP");
        format |= ZDBSPX;
    }
    else if (!memcmp(nodes, "ZNOD", 4))
    {
        fprintf(stderr, english_language ? "compressed ZDBSP" : "сжатые ZDBSP");
        format |= ZDBSPZ;
    }
    else
    {
        fprintf(stderr, "BSP");
    }
    fprintf(stderr, ")\n");

    if (nodes)
    W_ReleaseLumpNum(b);

    return format;
}

/*
================================================================================
=
= P_SetupLevel
=
================================================================================
*/

void P_SetupLevel (int episode, int map, int playermask, skill_t skill)
{
    int          i;
    int          parm;
    int          lumpnum;
    char         lumpname[9];
    mobj_t      *mobj;
    mapformat_t	 crispy_mapformat;
    boolean      crispy_validblockmap;

    totalkills = totalitems = totalsecret = 0;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        players[i].killcount = 
        players[i].secretcount =
        players[i].itemcount = 0;
    }
    players[consoleplayer].viewz = 1;   // will be set by player think

    S_Start();  // make sure all sounds are stopped before Z_FreeTags

    Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

    P_InitThinkers();

    // look for a regular (development) map first
    lumpname[0] = 'E';
    lumpname[1] = '0' + episode;
    lumpname[2] = 'M';
    lumpname[3] = '0' + map;
    lumpname[4] = 0;
    leveltime = 0;

    lumpnum = W_GetNumForName(lumpname);

    // [JN] Checking for multiple map lump names for allowing map fixes to work.
    // Adaptaken from Doom Retro, thanks Brad Harding!
    canmodify = (W_CheckMultipleLumps(lumpname) == 1
              && (!netgame && !vanillaparm && gamemode != shareware && singleplayer));

    // [crispy] check and log map and nodes format
    crispy_mapformat = P_CheckMapFormat(lumpnum);

    // note: most of this ordering is important	
    crispy_validblockmap = P_LoadBlockMap (lumpnum+ML_BLOCKMAP); // [crispy] (re-)create BLOCKMAP if necessary
    P_LoadVertexes (lumpnum+ML_VERTEXES);
    P_LoadSectors (lumpnum+ML_SECTORS);
    P_LoadSideDefs (lumpnum+ML_SIDEDEFS);

    if (crispy_mapformat & HEXEN)
    {
        P_LoadLineDefs_Hexen (lumpnum+ML_LINEDEFS);
    }
    else
    {
        P_LoadLineDefs (lumpnum+ML_LINEDEFS);
    }

    // [crispy] (re-)create BLOCKMAP if necessary
    if (!crispy_validblockmap)
    {
        P_CreateBlockMap();
    }

    if (crispy_mapformat & (ZDBSPX | ZDBSPZ))
    {
        P_LoadNodes_ZDBSP (lumpnum+ML_NODES, crispy_mapformat & ZDBSPZ);
    }
    else if (crispy_mapformat & DEEPBSP)
    {
        P_LoadSubsectors_DeePBSP (lumpnum+ML_SSECTORS);
        P_LoadNodes_DeePBSP (lumpnum+ML_NODES);
        P_LoadSegs_DeePBSP (lumpnum+ML_SEGS);
    }
    else
    {
        P_LoadSubsectors (lumpnum+ML_SSECTORS);
        P_LoadNodes (lumpnum+ML_NODES);
        P_LoadSegs (lumpnum+ML_SEGS);
    }

    P_GroupLines ();
    P_LoadReject (lumpnum+ML_REJECT);

    // [crispy] remove slime trails
    P_RemoveSlimeTrails();
    // [crispy] fix long wall wobble
    P_SegLengths();

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    P_InitAmbientSound();
    P_InitMonsters();
    P_OpenWeapons();

    if (crispy_mapformat & HEXEN)
    {
        P_LoadThings_Hexen (lumpnum+ML_THINGS);
    }
    else
    {
        P_LoadThings (lumpnum+ML_THINGS);
    }

    P_CloseWeapons();

    // if deathmatch, randomly spawn the active players
    TimerGame = 0;
    if (deathmatch)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {                   // must give a player spot before deathmatchspawn
                mobj = P_SpawnMobj(playerstarts[i].x << 16,
                                   playerstarts[i].y << 16, 0, MT_PLAYER);
                players[i].mo = mobj;
                G_DeathMatchSpawnPlayer(i);
                P_RemoveMobj(mobj);
            }
        }

        //!
        // @arg <n>
        // @category net
        // @vanilla
        //
        // For multiplayer games: exit each level after n minutes.
        //

        parm = M_CheckParmWithArgs("-timer", 1);
        if (parm)
        {
            TimerGame = atoi(myargv[parm + 1]) * 35 * 60;
        }
    }

    // set up world state
    P_SpawnSpecials();

    // preload graphics
    if (precache)
    {
        R_PrecacheLevel();
    }
}

/*
================================================================================
=
= P_Init
=
================================================================================
*/

void P_Init(void)
{
    P_InitSwitchList();
    P_InitPicAnims();
    P_InitTerrainTypes();
    P_InitLava();
    R_InitSprites(sprnames);
}
