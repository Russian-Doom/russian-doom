//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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


#include "i_swap.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "g_game.h"
#include "i_system.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_local.h"
#include "id_lang.h"
#include "jn.h"

// [crispy] support maps with compressed ZDBSP nodes
#include "miniz.h"


boolean canmodify;

// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.

int       numvertexes;
vertex_t *vertexes;
int       numsegs;
seg_t    *segs;
int       numsectors;
sector_t *sectors;
int          numsubsectors;
subsector_t *subsectors;
int       numnodes;
node_t   *nodes;
int       numlines;
line_t   *lines;
int       numsides;
side_t   *sides;

static int totallines;

// BLOCKMAP
// Created from axis aligned bounding box of the map, a rectangular array of
// blocks of size ... Used to speed up collision detection by spatial 
// subdivision in 2D.

// size in mapblocks
int      bmapwidth, bmapheight;
// int for larger maps
int32_t *blockmap;      // [crispy] BLOCKMAP limit
 // offsets in blockmap are from here
int32_t *blockmaplump;  // [crispy] BLOCKMAP limit
// origin of block map
fixed_t  bmaporgx, bmaporgy;
// for thing chains
mobj_t **blocklinks;		


// REJECT
// For fast sight rejection. Speeds up enemy AI by skipping detailed
// LineOf Sight calculation. Without special effect, this could be
// used as a PVS lookup as well.

byte *rejectmatrix;

// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS 10

mapthing_t  deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t *deathmatch_p;
mapthing_t  playerstarts[MAXPLAYERS];
boolean     playerstartsingame[MAXPLAYERS];

typedef enum
{
    DOOMBSP = 0x000,
    DEEPBSP = 0x001,
    ZDBSPX  = 0x002,
    ZDBSPZ  = 0x004,
    HEXEN   = 0x100,
} mapformat_t;

// =============================================================================
//
// Builtin map names.
// The actual names can be found in rd_lang.h.
//
// =============================================================================

char *level_name;

// DOOM 1 map names
#define HU_TITLE        (mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE_RUS    (mapnames_rus[(gameepisode-1)*9+gamemap-1])

// DOOM 2 map names
#define HU_TITLE2       (mapnames_commercial[gamemap-1])
#define HU_TITLE2_RUS   (mapnames_commercial_rus[gamemap-1])

// Plutonia map names
#define HU_TITLEP       (mapnames_commercial[gamemap-1 + 32])
#define HU_TITLEP_RUS   (mapnames_commercial_rus[gamemap-1 + 32])

// TNT map names
#define HU_TITLET       (mapnames_commercial[gamemap-1 + 64])
#define HU_TITLET_RUS   (mapnames_commercial_rus[gamemap-1 + 64])

// No Rest for the Living map names
#define HU_TITLEN       (mapnames_commercial[gamemap-1 + 96])
#define HU_TITLEN_RUS   (mapnames_commercial_rus[gamemap-1 + 96])

// Jaguar Doom map names
#define HU_TITLEJ       (mapnames_commercial[gamemap-1 + 105])
#define HU_TITLEJ_RUS   (mapnames_commercial_rus[gamemap-1 + 105])

#define HU_TITLE_CHEX   (mapnames_chex[(gameepisode-1)*9+gamemap-1])


static char* mapnames[] = // DOOM shareware/registered/retail (Ultimate) names.
{
    HUSTR_E1M1, HUSTR_E1M2, HUSTR_E1M3, HUSTR_E1M4, HUSTR_E1M5, 
    HUSTR_E1M6, HUSTR_E1M7, HUSTR_E1M8, HUSTR_E1M9,
    HUSTR_E2M1, HUSTR_E2M2, HUSTR_E2M3, HUSTR_E2M4, HUSTR_E2M5,
    HUSTR_E2M6, HUSTR_E2M7, HUSTR_E2M8, HUSTR_E2M9,
    HUSTR_E3M1, HUSTR_E3M2, HUSTR_E3M3, HUSTR_E3M4, HUSTR_E3M5,
    HUSTR_E3M6, HUSTR_E3M7, HUSTR_E3M8, HUSTR_E3M9,
    HUSTR_E4M1, HUSTR_E4M2, HUSTR_E4M3, HUSTR_E4M4, HUSTR_E4M5,
    HUSTR_E4M6, HUSTR_E4M7, HUSTR_E4M8, HUSTR_E4M9,

    // [crispy] Sigil
    HUSTR_E5M1, HUSTR_E5M2, HUSTR_E5M3, HUSTR_E5M4, HUSTR_E5M5,
    HUSTR_E5M6, HUSTR_E5M7, HUSTR_E5M8, HUSTR_E5M9,

    "NEWLEVEL", "NEWLEVEL", "NEWLEVEL", "NEWLEVEL", "NEWLEVEL",
    "NEWLEVEL", "NEWLEVEL", "NEWLEVEL", "NEWLEVEL"
};

static char* mapnames_rus[] =
{
    HUSTR_E1M1_RUS, HUSTR_E1M2_RUS, HUSTR_E1M3_RUS, HUSTR_E1M4_RUS, HUSTR_E1M5_RUS,
    HUSTR_E1M6_RUS, HUSTR_E1M7_RUS, HUSTR_E1M8_RUS, HUSTR_E1M9_RUS,
    HUSTR_E2M1_RUS, HUSTR_E2M2_RUS, HUSTR_E2M3_RUS, HUSTR_E2M4_RUS, HUSTR_E2M5_RUS,
    HUSTR_E2M6_RUS, HUSTR_E2M7_RUS, HUSTR_E2M8_RUS, HUSTR_E2M9_RUS,

    HUSTR_E3M1_RUS, HUSTR_E3M2_RUS, HUSTR_E3M3_RUS, HUSTR_E3M4_RUS, HUSTR_E3M5_RUS,
    HUSTR_E3M6_RUS, HUSTR_E3M7_RUS, HUSTR_E3M8_RUS, HUSTR_E3M9_RUS,

    HUSTR_E4M1_RUS, HUSTR_E4M2_RUS, HUSTR_E4M3_RUS, HUSTR_E4M4_RUS, HUSTR_E4M5_RUS,
    HUSTR_E4M6_RUS, HUSTR_E4M7_RUS, HUSTR_E4M8_RUS, HUSTR_E4M9_RUS,

    // [JN] Sigil
    HUSTR_E5M1_RUS, HUSTR_E5M2_RUS, HUSTR_E5M3_RUS, HUSTR_E5M4_RUS, HUSTR_E5M5_RUS,
    HUSTR_E5M6_RUS, HUSTR_E5M7_RUS, HUSTR_E5M8_RUS, HUSTR_E5M9_RUS,

    // [JN] "НОВЫЙ УРОВЕНЬ"
    "YJDSQ EHJDTYM", "YJDSQ EHJDTYM", "YJDSQ EHJDTYM", "YJDSQ EHJDTYM", "YJDSQ EHJDTYM",
    "YJDSQ EHJDTYM", "YJDSQ EHJDTYM", "YJDSQ EHJDTYM", "YJDSQ EHJDTYM"
};

static char* mapnames_chex[] = // Chex Quest names.
{
    HUSTR_E1M1, HUSTR_E1M2, HUSTR_E1M3, HUSTR_E1M4, HUSTR_E1M5,
    HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5,
    HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5,
    HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5,
    HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5,
    HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5,
    HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5, HUSTR_E1M5,
    HUSTR_E1M5,

    "NEWLEVEL", "NEWLEVEL", "NEWLEVEL", "NEWLEVEL", "NEWLEVEL",
    "NEWLEVEL", "NEWLEVEL", "NEWLEVEL", "NEWLEVEL"
};

// List of names for levels in commercial IWADs
// (doom2.wad, plutonia.wad, tnt.wad).  These are stored in a
// single large array; WADs like pl2.wad have a MAP33, and rely on
// the layout in the Vanilla executable, where it is possible to
// overflow the end of one array into the next.

static char* mapnames_commercial[] =
{
    // DOOM 2 map names.
    HUSTR_1,  HUSTR_2,  HUSTR_3,  HUSTR_4,  HUSTR_5,
    HUSTR_6,  HUSTR_7,  HUSTR_8,  HUSTR_9,  HUSTR_10,
    HUSTR_11, HUSTR_12, HUSTR_13, HUSTR_14, HUSTR_15,
    HUSTR_16, HUSTR_17, HUSTR_18, HUSTR_19, HUSTR_20,
    HUSTR_21, HUSTR_22, HUSTR_23, HUSTR_24, HUSTR_25,
    HUSTR_26, HUSTR_27, HUSTR_28, HUSTR_29, HUSTR_30,
    HUSTR_31, HUSTR_32,

    // Plutonia WAD map names.
    PHUSTR_1,  PHUSTR_2,  PHUSTR_3,  PHUSTR_4,  PHUSTR_5,
    PHUSTR_6,  PHUSTR_7,  PHUSTR_8,  PHUSTR_9,  PHUSTR_10,
    PHUSTR_11, PHUSTR_12, PHUSTR_13, PHUSTR_14, PHUSTR_15,
    PHUSTR_16, PHUSTR_17, PHUSTR_18, PHUSTR_19, PHUSTR_20,
    PHUSTR_21, PHUSTR_22, PHUSTR_23, PHUSTR_24, PHUSTR_25,
    PHUSTR_26, PHUSTR_27, PHUSTR_28, PHUSTR_29, PHUSTR_30,
    PHUSTR_31, PHUSTR_32,

    // TNT WAD map names.
    THUSTR_1,  THUSTR_2,  THUSTR_3,  THUSTR_4,  THUSTR_5,
    THUSTR_6,  THUSTR_7,  THUSTR_8,  THUSTR_9,  THUSTR_10,
    THUSTR_11, THUSTR_12, THUSTR_13, THUSTR_14, THUSTR_15,
    THUSTR_16, THUSTR_17, THUSTR_18, THUSTR_19, THUSTR_20,
    THUSTR_21, THUSTR_22, THUSTR_23, THUSTR_24, THUSTR_25,
    THUSTR_26, THUSTR_27, THUSTR_28, THUSTR_29, THUSTR_30,
    THUSTR_31, THUSTR_32, 

    // No Rest for the Living names.
    NHUSTR_1, NHUSTR_2, NHUSTR_3, NHUSTR_4, NHUSTR_5,
    NHUSTR_6, NHUSTR_7, NHUSTR_8, NHUSTR_9,

    // [JN] Jaguar Doom.
    JHUSTR_1,  JHUSTR_2,  JHUSTR_3,  JHUSTR_4,  JHUSTR_5,
    JHUSTR_6,  JHUSTR_7,  JHUSTR_8,  JHUSTR_9,  JHUSTR_10,
    JHUSTR_11, JHUSTR_12, JHUSTR_13, JHUSTR_14, JHUSTR_15,
    JHUSTR_16, JHUSTR_17, JHUSTR_18, JHUSTR_19, JHUSTR_20,
    JHUSTR_21, JHUSTR_22, JHUSTR_23, JHUSTR_24, JHUSTR_25,
    JHUSTR_26
};

static char* mapnames_commercial_rus[] =
{
    // DOOM 2 map names.
    HUSTR_1_RUS,  HUSTR_2_RUS,  HUSTR_3_RUS,  HUSTR_4_RUS,  HUSTR_5_RUS,
    HUSTR_6_RUS,  HUSTR_7_RUS,  HUSTR_8_RUS,  HUSTR_9_RUS,  HUSTR_10_RUS,
    HUSTR_11_RUS, HUSTR_12_RUS, HUSTR_13_RUS, HUSTR_14_RUS, HUSTR_15_RUS,
    HUSTR_16_RUS, HUSTR_17_RUS, HUSTR_18_RUS, HUSTR_19_RUS, HUSTR_20_RUS,
    HUSTR_21_RUS, HUSTR_22_RUS, HUSTR_23_RUS, HUSTR_24_RUS, HUSTR_25_RUS,
    HUSTR_26_RUS, HUSTR_27_RUS, HUSTR_28_RUS, HUSTR_29_RUS, HUSTR_30_RUS,
    HUSTR_31_RUS, HUSTR_32_RUS,

    // Plutonia WAD map names.
    PHUSTR_1_RUS,  PHUSTR_2_RUS,  PHUSTR_3_RUS,  PHUSTR_4_RUS,  PHUSTR_5_RUS,
    PHUSTR_6_RUS,  PHUSTR_7_RUS,  PHUSTR_8_RUS,  PHUSTR_9_RUS,  PHUSTR_10_RUS,
    PHUSTR_11_RUS, PHUSTR_12_RUS, PHUSTR_13_RUS, PHUSTR_14_RUS, PHUSTR_15_RUS,
    PHUSTR_16_RUS, PHUSTR_17_RUS, PHUSTR_18_RUS, PHUSTR_19_RUS, PHUSTR_20_RUS,
    PHUSTR_21_RUS, PHUSTR_22_RUS, PHUSTR_23_RUS, PHUSTR_24_RUS, PHUSTR_25_RUS,
    PHUSTR_26_RUS, PHUSTR_27_RUS, PHUSTR_28_RUS, PHUSTR_29_RUS, PHUSTR_30_RUS,
    PHUSTR_31_RUS, PHUSTR_32_RUS, 

    // TNT WAD map names.
    THUSTR_1_RUS,  THUSTR_2_RUS,  THUSTR_3_RUS,  THUSTR_4_RUS,  THUSTR_5_RUS,
    THUSTR_6_RUS,  THUSTR_7_RUS,  THUSTR_8_RUS,  THUSTR_9_RUS,  THUSTR_10_RUS,
    THUSTR_11_RUS, THUSTR_12_RUS, THUSTR_13_RUS, THUSTR_14_RUS, THUSTR_15_RUS,
    THUSTR_16_RUS, THUSTR_17_RUS, THUSTR_18_RUS, THUSTR_19_RUS, THUSTR_20_RUS,
    THUSTR_21_RUS, THUSTR_22_RUS, THUSTR_23_RUS, THUSTR_24_RUS, THUSTR_25_RUS,
    THUSTR_26_RUS, THUSTR_27_RUS, THUSTR_28_RUS, THUSTR_29_RUS, THUSTR_30_RUS,
    THUSTR_31_RUS, THUSTR_32_RUS,

    // Нет покоя для живых.
    NHUSTR_1_RUS, NHUSTR_2_RUS, NHUSTR_3_RUS, NHUSTR_4_RUS, NHUSTR_5_RUS,
    NHUSTR_6_RUS, NHUSTR_7_RUS, NHUSTR_8_RUS, NHUSTR_9_RUS,

    // [JN] Atari Jaguar
    JHUSTR_1_RUS,  JHUSTR_2_RUS,  JHUSTR_3_RUS,  JHUSTR_4_RUS,  JHUSTR_5_RUS,
    JHUSTR_6_RUS,  JHUSTR_7_RUS,  JHUSTR_8_RUS,  JHUSTR_9_RUS,  JHUSTR_10_RUS,
    JHUSTR_11_RUS, JHUSTR_12_RUS, JHUSTR_13_RUS, JHUSTR_14_RUS, JHUSTR_15_RUS,
    JHUSTR_16_RUS, JHUSTR_17_RUS, JHUSTR_18_RUS, JHUSTR_19_RUS, JHUSTR_20_RUS,
    JHUSTR_21_RUS, JHUSTR_22_RUS, JHUSTR_23_RUS, JHUSTR_24_RUS, JHUSTR_25_RUS,
    JHUSTR_26_RUS
};

void P_LevelNameInit (void)
{
    char *s;

    switch (logical_gamemission)
    {
        case doom:
        s = english_language ? HU_TITLE : HU_TITLE_RUS;
        break;

        case doom2:
        s = english_language ? HU_TITLE2 : HU_TITLE2_RUS;
        break;

        case pack_plut:
        s = english_language ? HU_TITLEP : HU_TITLEP_RUS;
        break;

        case pack_tnt:
        s = english_language ? HU_TITLET : HU_TITLET_RUS;
        break;

        case pack_nerve:
        if (gamemap <= 9)
        s = english_language ? HU_TITLEN : HU_TITLEN_RUS;
        else
        s = english_language ? HU_TITLE2 : HU_TITLE2_RUS;
        break;

        case jaguar:
        if (gamemap <= 26)
        s = english_language ? HU_TITLEJ : HU_TITLEJ_RUS;
        else
        s = english_language ? HU_TITLE2 : HU_TITLE2_RUS;
        break;
        
        default:  // [JN] "Неизвестный уровень"
        s = english_language ? "Unknown level" : "ytbpdtcnysq ehjdtym";
        break;
    }

    if (logical_gamemission == doom && gameversion == exe_chex)
    {
        s = HU_TITLE_CHEX;
    }

    // dehacked substitution to get modified level name
    s = DEH_String(s);

    level_name = s;
}

// -----------------------------------------------------------------------------
// GetOffset
// [crispy] recalculate seg offsets
// adapted from prboom-plus/src/p_setup.c:474-482
// -----------------------------------------------------------------------------

static const fixed_t GetOffset (const vertex_t *v1, const vertex_t *v2)
{
    fixed_t dx, dy;
    fixed_t r;

    dx = (v1->x - v2->x)>>FRACBITS;
    dy = (v1->y - v2->y)>>FRACBITS;
    r = (fixed_t)(sqrt(dx*dx + dy*dy))<<FRACBITS;

    return r;
}

// -----------------------------------------------------------------------------
// P_LoadVertexes
// -----------------------------------------------------------------------------

static void P_LoadVertexes (const int lump)
{
    byte        *data;
    vertex_t    *li;
    mapvertex_t *ml;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t), PU_LEVEL,0);	

    // Load data into cache.
    data = W_CacheLumpNum (lump, PU_STATIC);

    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (int i = 0 ; i < numvertexes ; i++, li++, ml++)
    {
        li->x = SHORT(ml->x) << FRACBITS;
        li->y = SHORT(ml->y) << FRACBITS;
    
        // [crispy] initialize pseudovertexes with actual vertex coordinates
        li->px = li->x;
        li->py = li->y;
        li->moved = false;

        // [BH] Apply any map-specific fixes.
        if (canmodify && fix_map_errors)
        {
            for (int j = 0; selected_vertexfix[j].mission != -1; j++)
            {
                if (i == selected_vertexfix[j].vertex && gamemission == selected_vertexfix[j].mission
                && gameepisode == selected_vertexfix[j].epsiode && gamemap == selected_vertexfix[j].map
                && vertexes[i].x == SHORT(selected_vertexfix[j].oldx) << FRACBITS
                && vertexes[i].y == SHORT(selected_vertexfix[j].oldy) << FRACBITS)
                {
                    vertexes[i].px = SHORT(selected_vertexfix[j].newx) << FRACBITS;
                    vertexes[i].py = SHORT(selected_vertexfix[j].newy) << FRACBITS;

                    break;
                }
            }
        }

    }

    // Free buffer memory.
    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// GetSectorAtNullAddress
// -----------------------------------------------------------------------------

static sector_t *GetSectorAtNullAddress (void)
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

// -----------------------------------------------------------------------------
// P_LoadSegs
// -----------------------------------------------------------------------------

static void P_LoadSegs (const int lump)
{
    int       linedef, side, sidenum;
    byte     *data;
    seg_t    *li;
    line_t   *ldef;
    mapseg_t *ml;

    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);	
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    ml = (mapseg_t *)data;
    li = segs;

    for (int i=0 ; i<numsegs ; i++, li++, ml++)
    {
        li->v1 = &vertexes[(unsigned short)SHORT(ml->v1)];  // [crispy] extended nodes
        li->v2 = &vertexes[(unsigned short)SHORT(ml->v2)];  // [crispy] extended nodes
        li->angle = (SHORT(ml->angle)) << FRACBITS;
    
        linedef = (unsigned short)SHORT(ml->linedef);  // [crispy] extended nodes
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);

        // Andrey Budko: check for wrong indexes
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

        // [BH] Apply any map-specific fixes.
        if (canmodify && fix_map_errors)
        {
            for (int j = 0; selected_linefix[j].mission != -1; j++)
            {
                if (linedef == selected_linefix[j].linedef && gamemission == selected_linefix[j].mission
                && gameepisode == selected_linefix[j].epsiode && gamemap == selected_linefix[j].map
                && side == selected_linefix[j].side)
                {
                    if (*selected_linefix[j].toptexture)
                    {
                        li->sidedef->toptexture = R_TextureNumForName(selected_linefix[j].toptexture);
                    }
                    if (*selected_linefix[j].middletexture)
                    {
                        li->sidedef->midtexture = R_TextureNumForName(selected_linefix[j].middletexture);
                    }
                    if (*selected_linefix[j].bottomtexture)
                    {
                        li->sidedef->bottomtexture = R_TextureNumForName(selected_linefix[j].bottomtexture);
                    }
                    if (selected_linefix[j].offset != DEFAULT)
                    {
                        li->offset = SHORT(selected_linefix[j].offset) << FRACBITS;
                        li->sidedef->textureoffset = 0;
                    }
                    if (selected_linefix[j].rowoffset != DEFAULT)
                    {
                        li->sidedef->rowoffset = SHORT(selected_linefix[j].rowoffset) << FRACBITS;
                    }
                    if (selected_linefix[j].flags != DEFAULT)
                    {
                        if (li->linedef->flags & selected_linefix[j].flags)
                            li->linedef->flags &= ~selected_linefix[j].flags;
                        else
                            li->linedef->flags |= selected_linefix[j].flags;
                    }

                    break;
                }
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// P_LoadSegs_DeePBSP
// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:633-752
// -----------------------------------------------------------------------------

static void P_LoadSegs_DeePBSP (const int lump)
{
    mapseg_deepbsp_t *data;

    numsegs = W_LumpLength(lump) / sizeof(mapseg_deepbsp_t);
    segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
    data = (mapseg_deepbsp_t *)W_CacheLumpNum(lump, PU_STATIC);

    for (int i = 0; i < numsegs; i++)
    {
        seg_t *li = segs + i;
        mapseg_deepbsp_t *ml = data + i;
        int side, linedef;
        line_t *ldef;

        li->v1 = &vertexes[ml->v1];
        li->v2 = &vertexes[ml->v2];
        li->angle = (SHORT(ml->angle)) << 16;

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

// -----------------------------------------------------------------------------
// P_SegLengths
// [crispy] fix long wall wobble
// -----------------------------------------------------------------------------

static void P_SegLengths (void)
{
    for (int i = 0; i < numsegs; i++)
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

// -----------------------------------------------------------------------------
// P_LoadSubsectors
// -----------------------------------------------------------------------------

static void P_LoadSubsectors (const int lump)
{
    byte           *data;
    subsector_t    *ss;
    mapsubsector_t *ms;

    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t), PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);

    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;

    for (int i = 0 ; i < numsubsectors ; i++, ss++, ms++)
    {
        ss->numlines = (unsigned short)SHORT(ms->numsegs);
        ss->firstline = (unsigned short)SHORT(ms->firstseg);
    }

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// P_LoadSubsectors_DeePBSP
// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:843-863
// -----------------------------------------------------------------------------

static void P_LoadSubsectors_DeePBSP (const int lump)
{
    mapsubsector_deepbsp_t *data;

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_deepbsp_t);
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
    data = (mapsubsector_deepbsp_t *)W_CacheLumpNum(lump, PU_STATIC);

    for (int i = 0; i < numsubsectors; i++)
    {
        subsectors[i].numlines = (int)data[i].numsegs;
        subsectors[i].firstline = (int)data[i].firstseg;
    }

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// P_LoadSectors
// -----------------------------------------------------------------------------

static void P_LoadSectors (const int lump)
{
    byte        *data;
    sector_t    *ss;
    mapsector_t *ms;

    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t), PU_LEVEL,0);	
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump, PU_STATIC);

    ms = (mapsector_t *)data;
    ss = sectors;

    for (int i = 0 ; i<numsectors ; i++, ss++, ms++)
    {
        ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
        ss->floorpic = R_FlatNumForName(ms->floorpic);
        ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
        ss->lightlevel = SHORT(ms->lightlevel);
        ss->special = SHORT(ms->special);
        ss->tag = SHORT(ms->tag);
        ss->flow = 0;  // [JN] Initialize flowing of swirling liquids.
        ss->thinglist = NULL;
        // [JN] Improved column clipping.
        ss->floor_xoffs = 0;
        ss->floor_yoffs = 0;
        ss->ceiling_xoffs = 0;
        ss->ceiling_yoffs = 0;
        // [crispy] WiggleFix: [kb] for R_FixWiggle()
        ss->cachedheight = 0;
        // [AM] Sector interpolation.  Even if we're
        //      not running uncapped, the renderer still
        //      uses this data.
        ss->oldfloorheight = ss->floorheight;
        ss->interpfloorheight = ss->floorheight;
        ss->oldceilingheight = ss->ceilingheight;
        ss->interpceilingheight = ss->ceilingheight;
        // [crispy] inhibit sector interpolation during the 0th gametic
        ss->oldgametic = -1;

        // [BH] Apply any level-specific fixes.
        if (canmodify)
        {
            if (fix_map_errors)
            {
                for (int j = 0; selected_sectorfix[j].mission != -1; j++)
                {
                    if (i == selected_sectorfix[j].sector && gamemission == selected_sectorfix[j].mission
                    && gameepisode == selected_sectorfix[j].epsiode && gamemap == selected_sectorfix[j].map)
                    {
                        if (*selected_sectorfix[j].floorpic)
                        {
                            ss->floorpic = R_FlatNumForName(selected_sectorfix[j].floorpic);
                        }
                        if (*selected_sectorfix[j].ceilingpic)
                        {
                            ss->ceilingpic = R_FlatNumForName(selected_sectorfix[j].ceilingpic);
                        }

                        break;
                    }
                }
            }

            // [JN] Inject flow effect to swirling liquids.
            for (int j = 0; selected_flow[j].mission != -1; j++)
            {
                if (i == selected_flow[j].sector && gamemission == selected_flow[j].mission
                && gameepisode == selected_flow[j].epsiode && gamemap == selected_flow[j].map)
                {
                    if (selected_flow[j].flow)
                    {
                        ss->flow = SHORT(selected_flow[j].flow);
                    }
                    break;
                }
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// P_LoadNodes
// -----------------------------------------------------------------------------

static void P_LoadNodes (const int lump)
{
    byte      *data;
    node_t    *no;
    mapnode_t *mn;

    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    nodes = Z_Malloc (numnodes*sizeof(node_t), PU_LEVEL,0);	
    data = W_CacheLumpNum (lump, PU_STATIC);

    mn = (mapnode_t *)data;
    no = nodes;

    for (int i = 0 ; i < numnodes ; i++, no++, mn++)
    {
        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;

        for (int j = 0 ; j < 2 ; j++)
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
                {
                    no->children[j] = 0;
                }

                no->children[j] |= NF_SUBSECTOR;
            }

            for (int k = 0 ; k < 4 ; k++)
            {
                no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
            }
        }
    }
	
    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:995-1038
// -----------------------------------------------------------------------------

static void P_LoadNodes_DeePBSP (const int lump)
{
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

    for (int i = 0; i < numnodes; i++)
    {
        node_t *no = nodes + i;
        mapnode_deepbsp_t *mn = (mapnode_deepbsp_t *) data + i;

        no->x = SHORT(mn->x)<<FRACBITS;
        no->y = SHORT(mn->y)<<FRACBITS;
        no->dx = SHORT(mn->dx)<<FRACBITS;
        no->dy = SHORT(mn->dy)<<FRACBITS;

        for (int j = 0; j < 2; j++)
        {
            no->children[j] = (unsigned int)(mn->children[j]);

            for (int k = 0; k < 4; k++)
            {
                no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// P_LoadNodes_ZDBSP
// [crispy] support maps with compressed or uncompressed ZDBSP nodes
// adapted from prboom-plus/src/p_setup.c:1040-1331
// heavily modified, condensed and simplyfied
// - removed most paranoid checks, brought in line with Vanilla P_LoadNodes()
// - removed const type punning pointers
// - inlined P_LoadZSegs()
// - added support for compressed ZDBSP nodes
// - added support for flipped levels
// -----------------------------------------------------------------------------

static void P_LoadNodes_ZDBSP (const int lump, const boolean compressed)
{
    byte *data;
    unsigned int i;
    byte *output;

    unsigned int orgVerts, newVerts;
    unsigned int numSubs, currSeg;
    unsigned int numSegs;
    unsigned int numNodes;
    vertex_t *newvertarray = NULL;

    data = W_CacheLumpNum(lump, PU_LEVEL);

    // 0. Uncompress nodes lump (or simply skip header)

    if (compressed)
    {
        int len =  W_LumpLength(lump);
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
        {
	        I_Error(english_language ?
                    "P_LoadNodes: Error during ZDBSP nodes decompression initialization!" :
                    "P_LoadNodes: ошибка при инициализации распаковки нодов ZDBSP!");
        }

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
        {
            I_Error(english_language ?
                    "P_LoadNodes: Error during ZDBSP nodes decompression!" :
                    "P_LoadNodes: ошибка при распаковке нодов ZDBSP!");
        }

        fprintf(stderr, english_language ?
                "P_LoadNodes: ZDBSP nodes compression ratio %.3f\n" :
                "P_LoadNodes: степень сжатия нодов ZDBSP: %.3f\n",
                (float)zstream->total_out/zstream->total_in);

        data = output;
        len = zstream->total_out;

        if (inflateEnd(zstream) != Z_OK)
        {
            I_Error(english_language ?
                    "P_LoadNodes: Error during ZDBSP nodes decompression shut-down!" :
                    "P_LoadNodes: ошибка при завершении распаковки нодов ZDBSP!");
        }

        // release the original data lump
        W_ReleaseLumpNum(lump);
        free(zstream);
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
    {
        I_Error(english_language ?
                "P_LoadNodes: No subsectors in map!" :
                "P_LoadNodes: на уровне отсутствуют подсекторы!");
    }

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

        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;

        for (j = 0; j < 2; j++)
        {
            no->children[j] = (unsigned int)(mn->children[j]);
    
            for (k = 0; k < 4; k++)
            no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);

    if (compressed)
    {
        Z_Free(output);
    }
}

// -----------------------------------------------------------------------------
// P_LoadThings
// -----------------------------------------------------------------------------

static void P_LoadThings (const int lump)
{
    int         i;
    int         numthings;
    byte       *data;
    mapthing_t *mt;
    mapthing_t  spawnthing;
    boolean     spawn;

    data = W_CacheLumpNum (lump, PU_STATIC);
    numthings = W_LumpLength (lump) / sizeof(mapthing_t);

    mt = (mapthing_t *)data;

    for (i=0 ; i<numthings ; i++, mt++)
    {
        spawn = true;

        // Do not spawn cool, new monsters if !commercial
        if (gamemode != commercial)
        {
            switch (SHORT(mt->type))
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
            // [crispy] TNT MAP31 has a yellow key that is erroneously marked as multi-player only
            if (gamemission == pack_tnt && gamemap == 31 && mt->type == 6)
            {
                mt->options &= ~16;
            }
        }

        // Do spawn all other stuff. 
        spawnthing.x = SHORT(mt->x);
        spawnthing.y = SHORT(mt->y);
        spawnthing.angle = SHORT(mt->angle);
        spawnthing.type = SHORT(mt->type);
        spawnthing.options = SHORT(mt->options);

        P_SpawnMapThing(&spawnthing);
    }

    if (!deathmatch)
        for (i = 0; i < MAXPLAYERS; i++)
            if (playeringame[i] && !playerstartsingame[i])
                I_Error(english_language ? 
                        "P_LoadThings: Player %d start missing" :
                        "P_LoadThings: Отсутствует стартовая точка игрока №%d", i + 1);

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// P_LoadThings_Hexen
// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:348-400
// -----------------------------------------------------------------------------

static void P_LoadThings_Hexen (const int lump)
{
    int               numthings;
    byte             *data;
    mapthing_t        spawnthing;
    mapthing_hexen_t *mt;

    data = W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_hexen_t);

    mt = (mapthing_hexen_t *) data;

    for (int i = 0; i < numthings; i++, mt++)
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

// -----------------------------------------------------------------------------
// P_LoadLineDefs
// Also counts secret lines for intermissions.
// -----------------------------------------------------------------------------

static void P_LoadLineDefs (const int lump)
{
    byte         *data;
    line_t       *ld;
    maplinedef_t *mld;
    vertex_t     *v1, *v2;

    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t), PU_LEVEL,0);	
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    mld = (maplinedef_t *)data;
    ld = lines;

    for (int i=0 ; i<numlines ; i++, mld++, ld++)
    {
        ld->flags = (unsigned short)SHORT(mld->flags);
        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);
        ld->fall = 0;  // [JN] Initialize falling liquid linedef.

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

        // [JN] Inject fall effect to liquid linedefs on vanilla maps.
        if (canmodify)
        {
            for (int j = 0; selected_fall[j].mission != -1; j++)
            {
                if (i == selected_fall[j].linedef && gamemission == selected_fall[j].mission
                && gameepisode == selected_fall[j].epsiode && gamemap == selected_fall[j].map)
                {
                    if (selected_fall[j].fall)
                    {
                        ld->fall = SHORT(selected_fall[j].fall);
                    }
                    break;
                }
            }
        }
    }

    W_ReleaseLumpNum(lump);
}

// -----------------------------------------------------------------------------
// P_LoadLineDefs_Hexen
// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:410-490
// -----------------------------------------------------------------------------

static void P_LoadLineDefs_Hexen (const int lump)
{
    byte               *data;
    line_t             *ld;
    maplinedef_hexen_t *mld;
    vertex_t           *v1, *v2;
    int warn;  // [crispy] warn about unknown linedef types

    numlines = W_LumpLength(lump) / sizeof(maplinedef_hexen_t);
    lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
    memset(lines, 0, numlines * sizeof(line_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    mld = (maplinedef_hexen_t *) data;
    ld = lines;
    warn = 0; // [crispy] warn about unknown linedef types

    for (int i = 0; i < numlines; i++, mld++, ld++)
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

// -----------------------------------------------------------------------------
// P_LoadSideDefs
// -----------------------------------------------------------------------------

static void P_LoadSideDefs (const int lump)
{
    byte         *data;
    side_t       *sd;
    mapsidedef_t *msd;

    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t), PU_LEVEL,0);	
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump, PU_STATIC);
	
    msd = (mapsidedef_t *)data;
    sd = sides;

    for (int i=0 ; i < numsides ; i++, msd++, sd++)
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

// -----------------------------------------------------------------------------
// P_CreateBlockMap
// [crispy] taken from mbfsrc/P_SETUP.C:547-707, slightly adapted
// -----------------------------------------------------------------------------

static void P_CreateBlockMap (void)
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
                    bmap[b].list = I_Realloc(bmap[b].list,
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

// -----------------------------------------------------------------------------
// P_LoadBlockMap
//
// [crispy] remove BLOCKMAP limit
// adapted from boom202s/P_SETUP.C:1025-1076
// -----------------------------------------------------------------------------

static const boolean P_LoadBlockMap (const int lump)
{
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

    for (int i=4; i<count; i++)
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

// -----------------------------------------------------------------------------
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
// -----------------------------------------------------------------------------

static void P_GroupLines (void)
{
    int			i, j, block;
    line_t     **linebuffer;
    line_t      *li;
    sector_t    *sector;
    fixed_t      bbox[4];
	
    // look up sector number for each subsector
    // [JN] Fix infinite loop if subsector a part of no sector.
    // Written by figgi, adapted from PrBoom+.
    for (i = 0 ; i < numsubsectors ; i++)
    {
        seg_t *seg = &segs[subsectors[i].firstline];
        subsectors[i].sector = NULL;
        for (j = 0 ; j < subsectors[i].numlines ; j++)
        {
            if (seg->sidedef)
            {
                subsectors[i].sector = seg->sidedef->sector;
                break;
            }
            seg++;
        }
        if (subsectors[i].sector == NULL)
        {
            I_Error(english_language ?
                    "P_GroupLines: Subsector %d a part of no sector!\n" :
                    "P_GroupLines: подсектор %d не является частью сектора!\n", subsectors[i].sector);
        }
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

    for (i = 0; i < numsectors; ++i)
    {
        // Assign the line buffer for this sector

        sectors[i].lines = linebuffer;
        linebuffer += sectors[i].linecount;

        // Reset linecount to zero so in the next stage we can count
        // lines into the list.

        sectors[i].linecount = 0;
    }

    // Assign lines to sectors

    for (i = 0; i < numlines; ++i)
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

    for (i = 0 ; i < numsectors ; i++, sector++)
    {
        M_ClearBox (bbox);

        for (j = 0 ; j < sector->linecount; j++)
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

// -----------------------------------------------------------------------------
// [crispy] remove slime trails
// mostly taken from Lee Killough's implementation in mbfsrc/P_SETUP.C:849-924,
// with the exception that not the actual vertex coordinates are modified,
// but pseudovertexes which are dummies that are *only* used in rendering,
// i.e. r_bsp.c:R_AddLine()
// -----------------------------------------------------------------------------

static void P_RemoveSlimeTrails (void)
{
    for (int i = 0; i < numsegs; i++)
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

// -----------------------------------------------------------------------------
// Pad the REJECT lump with extra data when the lump is too small,
// to simulate a REJECT buffer overflow in Vanilla Doom.
// -----------------------------------------------------------------------------

static void PadRejectArray (byte *array, unsigned const int len)
{
    unsigned int  i;
    unsigned int  byte_num;
    unsigned int  padvalue;
    byte         *dest;

    // Values to pad the REJECT array with:

    unsigned int rejectpad[4] =
    {
        0,                                  // Size
        0,                                  // Part of z_zone block header
        50,                                 // PU_LEVEL
        0x1d4a11                            // DOOM_CONST_ZONEID
    };

    rejectpad[0] = ((totallines * 4 + 3) & ~3) + 24;

    // Copy values from rejectpad into the destination array.

    dest = array;

    for (i = 0; i < len && i < sizeof(rejectpad); ++i)
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
                "PadRejectArray: REJECT lump too short to pad! (%i > %i), " :
                "PadRejectArray: блок REJECT слишком мал для заполнения! (%i > %i), ",
                len, (int) sizeof(rejectpad));

        // Pad remaining space with 0 (or 0xff, if specified on command line).

        if (M_CheckParm("-reject_pad_with_ff"))
        {
            padvalue = 0xff;
        }
        else
        {
            padvalue = 0x00;
        }

        memset(array + sizeof(rejectpad), padvalue, len - sizeof(rejectpad));
    }
}

// -----------------------------------------------------------------------------
// P_LoadReject
// -----------------------------------------------------------------------------

static void P_LoadReject (const int lumpnum)
{
    int minlength, lumplen;

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

// -----------------------------------------------------------------------------
// [crispy] support maps with NODES in compressed or uncompressed ZDBSP
// format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
// -----------------------------------------------------------------------------

static const mapformat_t P_CheckMapFormat (const int lumpnum)
{
    int b;
    mapformat_t format = 0;
    byte *nodes = NULL;

    if ((b = lumpnum+ML_BLOCKMAP+1) < numlumps
    && !strncasecmp(lumpinfo[b]->name, "BEHAVIOR", 8))
    {
        fprintf(stderr, english_language ? 
                "Hexen format (" :
                "формат Hexen (");
        format |= HEXEN;
    }
    else
    {
        fprintf(stderr, english_language ?
                "Doom format (" :
                "формат Doom (");
    }

    if (!((b = lumpnum+ML_NODES) < numlumps
    && (nodes = W_CacheLumpNum(b, PU_CACHE))
    && W_LumpLength(b) > 0))
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
    fprintf(stderr, "), ");

    if (nodes)
    {
        W_ReleaseLumpNum(b);
    }

    return format;
}

// -----------------------------------------------------------------------------
// P_SetupLevel
// -----------------------------------------------------------------------------

void P_SetupLevel (const int episode, const int map, const skill_t skill)
{
    int		i;
    char	lumpname[9];
    int		lumpnum;
    mapformat_t	crispy_mapformat;
    boolean crispy_validblockmap;
    unsigned const int starttime = SDL_GetTicks();
    unsigned int endtime;

    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;

    for (i = 0 ; i < MAXPLAYERS ; i++)
    {
        players[i].killcount = players[i].extrakillcount 
                             = players[i].secretcount
                             = players[i].itemcount = 0;
    }

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1; 

    // [crispy] stop demo warp mode now
    if (demowarp == map)
    {
        demowarp = 0;
        nodrawers = false;
        singletics = false;
    }

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start ();			

    Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);

    P_InitThinkers ();

    // if working with a devlopment map, reload it
    W_Reload ();

    // find map name
    if (gamemode == commercial)
    {
        DEH_snprintf(lumpname, 9, "MAP%02d", map);
    }
    else
    {
        DEH_snprintf(lumpname, 9, "E%dM%d", episode, map);
    }

    lumpnum = W_GetNumForName (lumpname);

    // [JN] Checking for multiple map lump names for allowing map fixes to work.
    // Adaptaken from DOOM Retro, thanks Brad Harding!
    //  Fixes also should not work for: network game, shareware, IWAD versions below 1.9,
    //  vanilla game mode, Press Beta, Atari Jaguar, Freedoom and FreeDM.
    canmodify = (((W_CheckMultipleLumps(lumpname) == 1)
             && (!vanillaparm
             && gamemode != shareware
             && gameversion >= exe_doom_1_9
             && gamemode != pressbeta
             && gamemission != jaguar
             && gamevariant != freedoom && gamevariant != freedm)));

    // [JN] If level can be modified, setup it's fixes and flow/fall effects.
    if (canmodify)
    {
        P_SetupFixes(episode, map);
    }

    leveltime = 0;
    oldleveltime = 0; // [crispy] Track if game is running

    // [JN] Indicate the map we are loading
    if (gamemode == commercial)
    {
        fprintf(stderr, "P_SetupLevel: MAP%02d, ", gamemap);
    }
    else
    {
        fprintf(stderr, "P_SetupLevel: E%dM%d, ", gameepisode, gamemap);
    }

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
    // [crispy] blinking key or skull in the status bar
    memset(st_keyorskull, 0, sizeof(st_keyorskull));

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;

    if (crispy_mapformat & HEXEN)
    {
        P_LoadThings_Hexen (lumpnum+ML_THINGS);
    }
    else
    {
        P_LoadThings (lumpnum+ML_THINGS);
    }

    // if deathmatch, randomly spawn the active players
    if (deathmatch)
    {
        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (playeringame[i])
            {
            players[i].mo = NULL;
            G_DeathMatchSpawnPlayer (i);
            }
        }
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

    // [JN] Set level name.
    P_LevelNameInit();

    endtime = SDL_GetTicks() - starttime;
    DEH_printf(english_language ? "loaded in %d ms.\n" :
                                  "загружен за %d мс.\n", endtime);
}

// -----------------------------------------------------------------------------
// P_Init
// -----------------------------------------------------------------------------

void P_Init (void)
{
    P_InitSwitchList ();
    P_InitPicAnims ();
    R_InitSprites (sprnames);
}
