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
//	Preparation of data for rendering,
//	generation of lookups, caching, retrieval by name.
//



#include <stdio.h>
#include <stdlib.h>

#include "deh_main.h"
#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "doomdef.h"
#include "m_misc.h"
#include "r_local.h"
#include "p_local.h"
#include "doomstat.h"
#include "r_data.h"
#include "r_bmaps.h"
#include "v_trans.h"
#include "v_video.h"
#include "jn.h"


// =============================================================================
// Graphics.
//
// DOOM graphics for walls and sprites is stored in vertical runs of opaque 
// pixels (posts). A column is composed of zero or more posts, a patch or sprite
// is composed of zero or more columns.
// =============================================================================

// -----------------------------------------------------------------------------
// Texture definition.
//
// Each texture is composed of one or more patches, with patches being lumps
// stored in the WAD. The lumps are referenced by number, and patched into the
// rectangular texture space using origin and possibly other attributes.
// -----------------------------------------------------------------------------

typedef struct
{
    short originx;
    short originy;
    short patch;
    short stepdir;
    short colormap;
} PACKEDATTR mappatch_t;

// -----------------------------------------------------------------------------
// Texture definition.
// A DOOM wall texture is a list of patches 
// which are to be combined in a predefined order.
// -----------------------------------------------------------------------------

typedef struct
{
    char       name[8];
    int	       masked;	
    short      width;
    short      height;
    int        obsolete;
    short      patchcount;
    mappatch_t patches[1];
} PACKEDATTR maptexture_t;

// -----------------------------------------------------------------------------
// A single patch from a texture definition, basically a rectangular
// area within the texture rectangle.
// -----------------------------------------------------------------------------

typedef struct
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    short  originx;	
    short  originy;
    int    patch;
} texpatch_t;

// -----------------------------------------------------------------------------
// A maptexturedef_t describes a rectangular texture, which is composed of 
// one or more mappatch_t structures that arrange graphic patches.
// -----------------------------------------------------------------------------

typedef struct texture_s texture_t;

struct texture_s
{
    // Keep name for switch changing, etc.
    char  name[8];		
    short width;
    short height;

    // Index in textures list
    int index;

    // Next in hash table chain
    texture_t *next;
    
    // All the patches[patchcount] are drawn back to front into the cached texture.
    short patchcount;
    texpatch_t patches[1];		
};

int		    firstflat, lastflat, numflats;
int		    firstpatch, lastpatch, numpatches;
int		    firstspritelump, lastspritelump, numspritelumps;
int         numtextures;
texture_t **textures;
texture_t **textures_hashtable;

int        *texturewidthmask;
int        *texturewidth;       // [crispy] texture width for wrapping column getter function

// needed for texture pegging
fixed_t    *textureheight;      // [crispy] texture height for Tutti-Frutti fix	
int        *texturecompositesize;
short     **texturecolumnlump;
unsigned  **texturecolumnofs;   // [crispy] column offsets for composited translucent mid-textures on 2S walls
unsigned  **texturecolumnofs2;  // [crispy] column offsets for composited opaque textures
byte      **texturecomposite;   // [crispy] composited translucent mid-textures on 2S walls
byte      **texturecomposite2;  // [crispy] composited opaque textures

// for global animation
int        *flattranslation, *texturetranslation;

// needed for pre rendering
fixed_t    *spritewidth, *spriteoffset, *spritetopoffset;

// colormaps
lighttable_t *colormaps;
lighttable_t *colormaps_rd; // [JN] Infragreen vison and B&W fuzz effect

// [JN] Brightmaps
lighttable_t *brightmaps_notgray;
lighttable_t *brightmaps_notgrayorbrown;
lighttable_t *brightmaps_redonly;
lighttable_t *brightmaps_greenonly1;
lighttable_t *brightmaps_greenonly2;
lighttable_t *brightmaps_greenonly3;
lighttable_t *brightmaps_orangeyellow;
lighttable_t *brightmaps_dimmeditems;
lighttable_t *brightmaps_brighttan;
lighttable_t *brightmaps_redonly1;
lighttable_t *brightmaps_explosivebarrel;
lighttable_t *brightmaps_alllights;
lighttable_t *brightmaps_candles;
lighttable_t *brightmaps_pileofskulls;
lighttable_t *brightmaps_redonly2;

// [JN] Brightmaps: define where they can be applied.
boolean brightmaps_allowed;


// =============================================================================
// MAPTEXTURE_T CACHING
// 
// When a texture is first needed, //  it counts the number of composite columns
// required in the texture and allocates space for a column directory and any
// new columns. The directory will simply point inside other patches if there
// is only one patch in a given column, but any columns with multiple patches
// will have new column_ts generated.
// =============================================================================


// -----------------------------------------------------------------------------
// [crispy] replace R_DrawColumnInCache(), R_GenerateComposite() and R_GenerateLookup()
// with Lee Killough's implementations found in MBF to fix Medusa bug
// taken from mbfsrc/R_DATA.C:136-425
//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
// -----------------------------------------------------------------------------

static void R_DrawColumnInCache (column_t *patch, byte *cache, int originy,
                                 int cacheheight, byte *marks)
{
    int   count;
    int   position;
    int   top = -1;
    byte *source;

    while (patch->topdelta != 0xff)
    {
        // [crispy] support for DeePsea tall patches
        if (patch->topdelta <= top)
        {
            top += patch->topdelta;
        }
        else
        {
            top = patch->topdelta;
        }

        source = (byte *)patch + 3;
        count = patch->length;
        position = originy + top;

        if (position < 0)
        {
            count += position;
            position = 0;
        }

        if (position + count > cacheheight)
        {
            count = cacheheight - position;
        }

        if (count > 0)
        {
            memcpy (cache + position, source, count);

            // killough 4/9/98: remember which cells in column have been drawn,
            // so that column can later be converted into a series of posts, to
            // fix the Medusa bug.

            memset (marks + position, 0xff, count);
        }

        patch = (column_t *)(  (byte *)patch + patch->length + 4); 
    }
}

// -----------------------------------------------------------------------------
// R_GenerateComposite
// Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
// -----------------------------------------------------------------------------

static void R_GenerateComposite (int texnum)
{
    int			x, x1, x2, i;
    short      *collump;
    unsigned   *colofs, *colofs2; // killough 4/9/98: make 32-bit
    byte       *block, *block2;
    byte       *marks;  // killough 4/9/98: transparency marks
    byte       *source; // killough 4/9/98: temporary column
    texture_t  *texture;
    texpatch_t *patch;	
    patch_t    *realpatch;
    column_t   *patchcol;

    texture = textures[texnum];

    block = Z_Malloc (texturecompositesize[texnum], PU_STATIC, &texturecomposite[texnum]);	
    // [crispy] memory block for opaque textures
    block2 = Z_Malloc (texture->width * texture->height, PU_STATIC, &texturecomposite2[texnum]);

    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];
    colofs2 = texturecolumnofs2[texnum];

    // killough 4/9/98: marks to identify transparent regions in merged textures
    marks = calloc(texture->width, texture->height);

    // [crispy] initialize composite background to palette index 0 (usually black)
    memset(block, 0, texturecompositesize[texnum]);

    // Composite the columns together.
    for (i = 0, patch = texture->patches; i < texture->patchcount ; i++, patch++)
    {
        realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
        x1 = patch->originx;
        x2 = x1 + SHORT(realpatch->width);

        if (x1<0)
        {
            x = 0;
        }
        else
        {
            x = x1;
        }

        if (x2 > texture->width)
        {
            x2 = texture->width;
        }

        for ( ; x < x2 ; x++)
        {
            patchcol = (column_t *)((byte *)realpatch + LONG(realpatch->columnofs[x-x1]));
            R_DrawColumnInCache (patchcol,
				                 block + colofs[x],
				                 // [crispy] single-patched columns are normally not composited
				                 // but directly read from the patch lump ignoring their originy
				                 collump[x] >= 0 ? 0 : patch->originy,
				                 texture->height,
				                 marks + x * texture->height);
        }
    }

    // killough 4/9/98: Next, convert multipatched columns into true columns,
    // to fix Medusa bug while still allowing for transparent regions.

    source = I_Realloc(NULL, texture->height); // temporary column

    for (i = 0; i < texture->width; i++)
    {
        // [crispy] generate composites for all columns
        {
            column_t *col = (column_t *)(block + colofs[i] - 3); // cached column
            const byte *mark = marks + i * texture->height;
            int j = 0;
            // [crispy] absolut topdelta for first 254 pixels, then relative
            int abstop, reltop = 0;
            boolean relative = false;

            // save column in temporary so we can shuffle it around
            memcpy(source, (byte *) col + 3, texture->height);
            // [crispy] copy composited columns to opaque texture
            memcpy(block2 + colofs2[i], source, texture->height);

            for ( ; ; ) // reconstruct the column by scanning transparency marks
            {
                unsigned len; // killough 12/98

                while (j < texture->height && reltop < 254 && !mark[j]) // skip transparent cells
                j++, reltop++;

                if (j >= texture->height) // if at end of column
                {
                    col->topdelta = -1; // end-of-column marker
                    break;
                }

                // [crispy] absolut topdelta for first 254 pixels, then relative
                col->topdelta = relative ? reltop : j; // starting offset of post

                // [crispy] once we pass the 254 boundary, topdelta becomes relative
                if ((abstop = j) >= 254)
                {
                    relative = true;
                    reltop = 0;
                }

                // killough 12/98:
                // Use 32-bit len counter, to support tall 1s multipatched textures

                for (len = 0; j < texture->height && reltop < 254 && mark[j]; j++, reltop++)
                {
                    len++; // count opaque cells
                }

                col->length = len; // killough 12/98: intentionally truncate length

                // copy opaque cells from the temporary back into the column
                memcpy((byte *) col + 3, source + abstop, len);
                col = (column_t *)((byte *) col + len + 4); // next post
            }
        }
    }

    free(source); // free temporary column
    free(marks); // free transparency marks

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    Z_ChangeTag (block, PU_CACHE);
    Z_ChangeTag (block2, PU_CACHE);
}

// -----------------------------------------------------------------------------
// R_GenerateLookup
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
// -----------------------------------------------------------------------------

static void R_GenerateLookup (int texnum)
{
    int			x, x1, x2, i;
    int			csize = 0;  // killough 10/98
    int			err = 0;    // killough 10/98
    short      *collump;
    unsigned   *colofs, *colofs2; // killough 4/9/98: make 32-bit
    texture_t  *texture;
    byte       *patchcount;	// patchcount[texture->width]
    byte       *postcount;  // killough 4/9/98: keep count of posts in addition to patches.
    texpatch_t *patch;	
    patch_t    *realpatch;

    texture = textures[texnum];

    // Composited texture not created yet.
    texturecomposite[texnum] = 0;
    texturecomposite2[texnum] = 0;

    texturecompositesize[texnum] = 0;
    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];
    colofs2 = texturecolumnofs2[texnum];

    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.
    patchcount = (byte *) Z_Malloc(texture->width, PU_STATIC, &patchcount);
    postcount = (byte *) Z_Malloc(texture->width, PU_STATIC, &postcount);
    memset (patchcount, 0, texture->width);
    memset (postcount, 0, texture->width);

    for (i=0, patch = texture->patches ; i < texture->patchcount ; i++, patch++)
    {
        realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
        x1 = patch->originx;
        x2 = x1 + SHORT(realpatch->width);

        if (x1 < 0)
        {
            x = 0;
        }
        else
        {
            x = x1;
        }

        if (x2 > texture->width)
        {
            x2 = texture->width;
        }

        for ( ; x < x2 ; x++)
        {
            patchcount[x]++;
            collump[x] = patch->patch;
            colofs[x] = LONG(realpatch->columnofs[x-x1])+3;
        }
    }

    // killough 4/9/98: keep a count of the number of posts in column,
    // to fix Medusa bug while allowing for transparent multipatches.
    //
    // killough 12/98:
    // Post counts are only necessary if column is multipatched,
    // so skip counting posts if column comes from a single patch.
    // This allows arbitrarily tall textures for 1s walls.
    //
    // If texture is >= 256 tall, assume it's 1s, and hence it has
    // only one post per column. This avoids crashes while allowing
    // for arbitrarily tall multipatched 1s textures.
    
    // [crispy] generate composites for all textures
    {
        // killough 12/98: Warn about a common column construction bug
        unsigned limit = texture->height * 3 + 3; // absolute column size limit

        for (i = texture->patchcount, patch = texture->patches ; --i >= 0 ; )
        {
            int pat = patch->patch;
            const patch_t *realpatch = W_CacheLumpNum(pat, PU_CACHE);
            int x, x1 = patch++->originx, x2 = x1 + SHORT(realpatch->width);
            const int *cofs = realpatch->columnofs - x1;

            if (x2 > texture->width)
            {
                x2 = texture->width;
            }
            if (x1 < 0)
            {
                x1 = 0;
            }

            for (x = x1 ; x < x2 ; x++)
            {
                // [crispy] generate composites for all columns
                const column_t *col = (const column_t*)((const byte*) realpatch + LONG(cofs[x]));
                const byte *base = (const byte *) col;

                // count posts
                for ( ; col->topdelta != 0xff ; postcount[x]++)
                {
                    if ((unsigned)((const byte *) col - base) <= limit)
                    {
                        col = (const column_t *)((const byte *) col + col->length + 4);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.

    for (x=0 ; x<texture->width ; x++)
    {
        if (!patchcount[x] && !err++) // killough 10/98: non-verbose output
        {
            // [crispy] fix absurd texture name in error message
            printf (english_language ?
                    "R_GenerateLookup: column without a patch (%.8s)\n" :
                    "R_GenerateLookup: текстурная колонна без патча (%.8s)\n",
                    texture->name);
            // [crispy] do not return yet
            // return;
        }

        // [crispy] treat patch-less columns the same as multi-patched
        if (patchcount[x] > 1 || !patchcount[x])
        {
            // Use the cached block.
            // [crispy] moved up here, the rest in this loop
            // applies to single-patched textures as well
            collump[x] = -1;	
        }

        // killough 1/25/98, 4/9/98:
        //
        // Fix Medusa bug, by adding room for column header
        // and trailer bytes for each post in merged column.
        // For now, just allocate conservatively 4 bytes
        // per post per patch per column, since we don't
        // yet know how many posts the merged column will
        // require, and it's bounded above by this limit.

        colofs[x] = csize + 3; // three header bytes in a column
        // killough 12/98: add room for one extra post
        csize += 4 * postcount[x] + 5; // 1 stop byte plus 4 bytes per post
        csize += texture->height; // height bytes of texture data
        // [crispy] initialize opaque texture column offset
        colofs2[x] = x * texture->height;
    }

    texturecompositesize[texnum] = csize;

    Z_Free(patchcount);
    Z_Free(postcount);
}

// -----------------------------------------------------------------------------
// R_GetColumn
// Retrieve column data for span blitting.
// -----------------------------------------------------------------------------

byte *R_GetColumn (int tex, int col)
{
    int ofs;

    col &= texturewidthmask[tex];
    ofs = texturecolumnofs2[tex][col];

    if (!texturecomposite2[tex])
    {
        R_GenerateComposite(tex);
    }

    return texturecomposite2[tex] + ofs;
}

// -----------------------------------------------------------------------------
// [crispy] wrapping column getter function for composited 
// translucent mid-textures on 2S walls
// -----------------------------------------------------------------------------

byte *R_GetColumnMod (int tex, int col)
{
    int ofs;

    while (col < 0)
    {
        col += texturewidth[tex];
    }

    col %= texturewidth[tex];
    ofs = texturecolumnofs[tex][col];

    if (!texturecomposite[tex])
    {
        R_GenerateComposite(tex);
    }

    return texturecomposite[tex] + ofs;
}

// -----------------------------------------------------------------------------
// GenerateTextureHashTable
// -----------------------------------------------------------------------------

static void GenerateTextureHashTable(void)
{
    int i;
    int key;
    texture_t **rover;

    textures_hashtable = Z_Malloc(sizeof(texture_t *) * numtextures, PU_STATIC, 0);

    memset(textures_hashtable, 0, sizeof(texture_t *) * numtextures);

    // Add all textures to hash table

    for (i=0; i<numtextures; ++i)
    {
        // Store index

        textures[i]->index = i;

        // Vanilla Doom does a linear search of the texures array
        // and stops at the first entry it finds.  If there are two
        // entries with the same name, the first one in the array
        // wins. The new entry must therefore be added at the end
        // of the hash chain, so that earlier entries win.

        key = W_LumpNameHash(textures[i]->name) % numtextures;

        rover = &textures_hashtable[key];

        while (*rover != NULL)
        {
            rover = &(*rover)->next;
        }

        // Hook into hash table

        textures[i]->next = NULL;
        *rover = textures[i];
    }
}

// -----------------------------------------------------------------------------
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
// [crispy] partly rewritten to merge PNAMES and TEXTURE1/2 lumps
// -----------------------------------------------------------------------------

static void R_InitTextures (void)
{
    int			   i, j, k;
    int			   temp1, temp2, temp3;
    int			   totalwidth;
    int			   nummappatches;
    int			   offset;
    int			   maxoff = 0;
    char		   name[9];
    int           *maptex = NULL;
    int           *directory = NULL;
    int           *patchlookup;
    texture_t     *texture;
    mappatch_t    *mpatch;
    texpatch_t    *patch;
    maptexture_t  *mtexture;

    typedef struct
    {
        int lumpnum;
        void  *names;
        short  nummappatches;
        short  summappatches;
        char  *name_p;
    } pnameslump_t;

    typedef struct
    {
        int   lumpnum;
        int  *maptex;
        int   maxoff;
        short numtextures;
        short sumtextures;
        short pnamesoffset;
    } texturelump_t;

    pnameslump_t  *pnameslumps = NULL;
    texturelump_t *texturelumps = NULL, *texturelump;

    int	 maxpnameslumps = 1;  // PNAMES
    int	 maxtexturelumps = 2; // TEXTURE1, TEXTURE2
    int	 numpnameslumps = 0;
    int	 numtexturelumps = 0;

    // [crispy] allocate memory for the pnameslumps and texturelumps arrays
    pnameslumps = I_Realloc(pnameslumps, maxpnameslumps * sizeof(*pnameslumps));
    texturelumps = I_Realloc(texturelumps, maxtexturelumps * sizeof(*texturelumps));

    // [crispy] make sure the first available TEXTURE1/2 lumps
    // are always processed first
    texturelumps[numtexturelumps++].lumpnum = W_GetNumForName(DEH_String("TEXTURE1"));
    if ((i = W_CheckNumForName(DEH_String("TEXTURE2"))) != -1)
    {
        texturelumps[numtexturelumps++].lumpnum = i;
    }
    else
    {
        texturelumps[numtexturelumps].lumpnum = -1;
    }

    // [crispy] fill the arrays with all available PNAMES lumps
    // and the remaining available TEXTURE1/2 lumps
    nummappatches = 0;

    for (i = numlumps - 1 ; i >= 0 ; i--)
    {
        if (!strncasecmp(lumpinfo[i]->name, DEH_String("PNAMES"), 6))
        {
            if (numpnameslumps == maxpnameslumps)
            {
                maxpnameslumps++;
                pnameslumps = I_Realloc(pnameslumps, maxpnameslumps * sizeof(*pnameslumps));
            }

            pnameslumps[numpnameslumps].lumpnum = i;
            pnameslumps[numpnameslumps].names = W_CacheLumpNum(pnameslumps[numpnameslumps].lumpnum, PU_STATIC);
            pnameslumps[numpnameslumps].nummappatches = LONG(*((int *) pnameslumps[numpnameslumps].names));

            // [crispy] accumulated number of patches in the lookup tables
            // excluding the current one
            pnameslumps[numpnameslumps].summappatches = nummappatches;
            pnameslumps[numpnameslumps].name_p = (char*)pnameslumps[numpnameslumps].names + 4;

            // [crispy] calculate total number of patches
            nummappatches += pnameslumps[numpnameslumps].nummappatches;
            numpnameslumps++;
        }
        else if (!strncasecmp(lumpinfo[i]->name, DEH_String("TEXTURE"), 7))
        {
            // [crispy] support only TEXTURE1/2 lumps, not TEXTURE3 etc.
            if (lumpinfo[i]->name[7] != '1' && lumpinfo[i]->name[7] != '2')
            {
                continue;
            }

            // [crispy] make sure the first available TEXTURE1/2 lumps are not processed again
            if (i == texturelumps[0].lumpnum || i == texturelumps[1].lumpnum) // [crispy] may still be -1
            {
                continue;
            }

            if (numtexturelumps == maxtexturelumps)
            {
                maxtexturelumps++;
                texturelumps = I_Realloc(texturelumps, maxtexturelumps * sizeof(*texturelumps));
            }

            // [crispy] do not proceed any further, yet
            // we first need a complete pnameslumps[] array and need
            // to process texturelumps[0] (and also texturelumps[1]) as well
            texturelumps[numtexturelumps].lumpnum = i;
            numtexturelumps++;
        }
    }

    // [crispy] fill up the patch lookup table
    name[8] = 0;
    patchlookup = Z_Malloc(nummappatches * sizeof(*patchlookup), PU_STATIC, NULL);

    for (i = 0, k = 0 ; i < numpnameslumps ; i++)
    {
        for (j = 0; j < pnameslumps[i].nummappatches; j++)
        {
            int p, po;

            M_StringCopy(name, pnameslumps[i].name_p + j * 8, sizeof(name));
            p = po = W_CheckNumForName(name);

            // [crispy] prevent flat lumps from being mistaken as patches
            while (p >= firstflat && p <= lastflat)
            {
                p = W_CheckNumForNameFromTo (name, p - 1, 0);
            }

            // [crispy] if the name is unambiguous, use the lump we found
            patchlookup[k++] = (p == -1) ? po : p;
        }
    }

    // [crispy] calculate total number of textures
    numtextures = 0;

    for (i = 0 ; i < numtexturelumps ; i++)
    {
        texturelumps[i].maptex = W_CacheLumpNum(texturelumps[i].lumpnum, PU_STATIC);
        texturelumps[i].maxoff = W_LumpLength(texturelumps[i].lumpnum);
        texturelumps[i].numtextures = LONG(*texturelumps[i].maptex);

        // [crispy] accumulated number of textures in the texture files
        // including the current one
        numtextures += texturelumps[i].numtextures;
        texturelumps[i].sumtextures = numtextures;

        // [crispy] link textures to their own WAD's patch lookup table (if any)
        texturelumps[i].pnamesoffset = 0;

        for (j = 0 ; j < numpnameslumps ; j++)
        {
            // [crispy] both are from the same WAD?
            if (lumpinfo[texturelumps[i].lumpnum]->wad_file
            == lumpinfo[pnameslumps[j].lumpnum]->wad_file)
            {
                texturelumps[i].pnamesoffset = pnameslumps[j].summappatches;
                break;
            }
        }
    }

    // [crispy] release memory allocated for patch lookup tables
    for (i = 0 ; i < numpnameslumps ; i++)
    {
        W_ReleaseLumpNum(pnameslumps[i].lumpnum);
    }
    free(pnameslumps);

    // [crispy] pointer to (i.e. actually before) the first texture file
    texturelump = texturelumps - 1; // [crispy] gets immediately increased below

    textures = Z_Malloc (numtextures * sizeof(*textures), PU_STATIC, 0);
    texturecolumnlump = Z_Malloc (numtextures * sizeof(*texturecolumnlump), PU_STATIC, 0);
    texturecolumnofs = Z_Malloc (numtextures * sizeof(*texturecolumnofs), PU_STATIC, 0);
    texturecolumnofs2 = Z_Malloc (numtextures * sizeof(*texturecolumnofs2), PU_STATIC, 0);
    texturecomposite = Z_Malloc (numtextures * sizeof(*texturecomposite), PU_STATIC, 0);
    texturecomposite2 = Z_Malloc (numtextures * sizeof(*texturecomposite2), PU_STATIC, 0);
    texturecompositesize = Z_Malloc (numtextures * sizeof(*texturecompositesize), PU_STATIC, 0);
    texturewidthmask = Z_Malloc (numtextures * sizeof(*texturewidthmask), PU_STATIC, 0);
    texturewidth = Z_Malloc (numtextures * sizeof(*texturewidth), PU_STATIC, 0);
    textureheight = Z_Malloc (numtextures * sizeof(*textureheight), PU_STATIC, 0);

    totalwidth = 0;

    //	Really complex printing shit...
    temp1 = W_GetNumForName (DEH_String("S_START"));  // P_???????
    temp2 = W_GetNumForName (DEH_String("S_END")) - 1;
    temp3 = ((temp2-temp1+63)/64) + ((numtextures+63)/64);

    // [JN] Printed D_DoomMain()
    //printf("[");

    for (i = 0; i < temp3 + 9; i++)
    printf(" ");

    printf("]");

    for (i = 0; i < temp3 + 10; i++)
    printf("\b");

    for (i=0 ; i<numtextures ; i++, directory++)
    {
        if (!(i&63))
        {
            printf (".");
        }

        // [crispy] initialize for the first texture file lump,
        // skip through empty texture file lumps which do not contain any texture
        while (texturelump == texturelumps - 1 || i == texturelump->sumtextures)
        {
            // [crispy] start looking in next texture file
            texturelump++;
            maptex = texturelump->maptex;
            maxoff = texturelump->maxoff;
            directory = maptex+1;
        }

        offset = LONG(*directory);

        if (offset > maxoff)
        {
            I_Error (english_language ?
                     "R_InitTextures: bad texture directory" :
                     "R_InitTextures: некорректная директория текстур");
        }

        mtexture = (maptexture_t *) ( (byte *)maptex + offset);

        texture = textures[i] = Z_Malloc (sizeof(texture_t)
		        + sizeof(texpatch_t)*(SHORT(mtexture->patchcount)-1), PU_STATIC, 0);

        texture->width = SHORT(mtexture->width);
        texture->height = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);

        memcpy (texture->name, mtexture->name, sizeof(texture->name));
        mpatch = &mtexture->patches[0];
        patch = &texture->patches[0];

        for (j=0 ; j < texture->patchcount ; j++, mpatch++, patch++)
        {
            short p;
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            // [crispy] apply offset for patches not in the
            // first available patch offset table
            p = SHORT(mpatch->patch) + texturelump->pnamesoffset;

            // [crispy] catch out-of-range patches
            if (p < nummappatches)
            {
                patch->patch = patchlookup[p];
            }
            if (patch->patch == -1 || p >= nummappatches)
            {
                char texturename[9];

                texturename[8] = '\0';
                memcpy (texturename, texture->name, 8);
                // [crispy] make non-fatal
                fprintf (stderr, english_language ? 
                         "R_InitTextures: Missing patch in texture %s\n" :
                         "R_InitTextures: отсутствует патч в текстуре %s\n", texturename);
                patch->patch = 0;
            }
        }

        texturecolumnlump[i] = Z_Malloc (texture->width*sizeof(**texturecolumnlump), PU_STATIC,0);
        texturecolumnofs[i] = Z_Malloc (texture->width*sizeof(**texturecolumnofs), PU_STATIC,0);
        texturecolumnofs2[i] = Z_Malloc (texture->width*sizeof(**texturecolumnofs2), PU_STATIC,0);

        j = 1;

        while (j*2 <= texture->width)
	    j<<=1;

        texturewidthmask[i] = j-1;
        textureheight[i] = texture->height<<FRACBITS;
        // [crispy] texture width for wrapping column getter function
        texturewidth[i] = texture->width;

        totalwidth += texture->width;
    }

    Z_Free(patchlookup);

    // [crispy] release memory allocated for texture files
    for (i = 0; i < numtexturelumps; i++)
    {
        W_ReleaseLumpNum(texturelumps[i].lumpnum);
    }
    free(texturelumps);
    
    // Precalculate whatever possible.	
    for (i=0 ; i<numtextures ; i++)
    {
        R_GenerateLookup (i);
    }

    // Create translation table for global animation.
    texturetranslation = Z_Malloc ((numtextures+1)*sizeof(*texturetranslation), PU_STATIC, 0);

    for (i=0 ; i<numtextures ; i++)
    {
        texturetranslation[i] = i;
    }

    GenerateTextureHashTable();
}

// -----------------------------------------------------------------------------
// R_InitFlats
// -----------------------------------------------------------------------------

static void R_InitFlats (void)
{
    int i;

    firstflat = W_GetNumForName (DEH_String("F_START")) + 1;
    lastflat = W_GetNumForName (DEH_String("F_END")) - 1;
    numflats = lastflat - firstflat + 1;

    // Create translation table for global animation.
    flattranslation = Z_Malloc ((numflats+1)*sizeof(*flattranslation), PU_STATIC, 0);

    for (i=0 ; i < numflats ; i++)
    {
        flattranslation[i] = i;
    }
}

// -----------------------------------------------------------------------------
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad, so the sprite 
// does not need to be cached completely just for having the header info 
// ready during rendering.
// -----------------------------------------------------------------------------

static void R_InitSpriteLumps (void)
{
    int      i;
    patch_t *patch;

    firstspritelump = W_GetNumForName (DEH_String("S_START")) + 1;
    lastspritelump = W_GetNumForName (DEH_String("S_END")) - 1;

    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = Z_Malloc (numspritelumps*sizeof(*spritewidth), PU_STATIC, 0);
    spriteoffset = Z_Malloc (numspritelumps*sizeof(*spriteoffset), PU_STATIC, 0);
    spritetopoffset = Z_Malloc (numspritelumps*sizeof(*spritetopoffset), PU_STATIC, 0);

    for (i=0 ; i< numspritelumps ; i++)
    {
        if (!(i&63))
        {
            printf (".");
        }

        patch = W_CacheLumpNum (firstspritelump+i, PU_CACHE);
        spritewidth[i] = SHORT(patch->width)<<FRACBITS;
        spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
        spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
    }
}

// -----------------------------------------------------------------------------
// R_InitColormaps
// -----------------------------------------------------------------------------

static void R_InitColormaps (void)
{
    // Load in the light tables 256 byte align tables.
    colormaps = W_CacheLumpNum(W_GetNumForName(DEH_String("COLORMAP")), PU_STATIC);

    // [JN] RD special colormaps, have three rows:
    // 0) Standard full bright colors (dummy).
    // 1) Green colors for Infragreen Visor.
    // 2) Black and white color for B&W fuzz effect.
    // 3) Inversed colormap for Press Beta invisibilite effect.
    colormaps_rd = W_CacheLumpNum(W_GetNumForName(DEH_String("COLORMRD")), PU_STATIC);
}

// -----------------------------------------------------------------------------
// [crispy] initialize translucency filter map
// based in parts on the implementation from boom202s/R_DATA.C:676-787
//
// [JN] Shortened to generate three maps:
// 1) Common translucency for sprites (tintmap).
// 2) Translucency for shadows casted by texts (shademap).
// 3) Fuzz effect translucency, improved_full №4 (fuzzmap).
// -----------------------------------------------------------------------------

enum {
    r, g, b
} rgb_t;


static void R_InitTransMaps (void)
{
    // Compose a default transparent filter map based on PLAYPAL.
    unsigned char *playpal = W_CacheLumpName("PLAYPAL", PU_STATIC);
    const int tint_filter_pct = 85;     // [JN] Common translucency
    const int shade_filter_pct = 60;    // [JN] Text shadows
    const int fuzz_filter_pct = 25;     // [JN] Translucent fuzz effect

    tintmap = Z_Malloc(256*256, PU_STATIC, 0);
    shademap = Z_Malloc(256*256, PU_STATIC, 0);
    fuzzmap = Z_Malloc(256*256, PU_STATIC, 0);
    // [JN] Fading effect for messages:
    transtable90 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable80 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable70 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable60 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable50 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable40 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable30 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable20 = Z_Malloc(256*256, PU_STATIC, 0);
    transtable10 = Z_Malloc(256*256, PU_STATIC, 0);

    // [JN] Always generate translucency tables dynamically.
    {
        byte *fg, *bg, blend[3];
        byte *tp1 = tintmap;
        byte *tp2 = shademap;
        byte *tp3 = fuzzmap;
        byte *tp90 = transtable90;
        byte *tp80 = transtable80;
        byte *tp70 = transtable70;
        byte *tp60 = transtable60;
        byte *tp50 = transtable50;
        byte *tp40 = transtable40;
        byte *tp30 = transtable30;
        byte *tp20 = transtable20;
        byte *tp10 = transtable10;
        int i, j;

        // [crispy] background color
        for (i = 0; i < 256; i++)
        {
            // [crispy] foreground color
            for (j = 0; j < 256; j++)
            {
                // [crispy] shortcut: identical foreground and background
                if (i == j)
                {
                    *tp1++ = i;
                    *tp2++ = i;
                    *tp3++ = i;
                    *tp90++ = i; *tp80++ = i; *tp70++ = i;
                    *tp60++ = i; *tp50++ = i; *tp40++ = i;
                    *tp30++ = i; *tp20++ = i; *tp10++ = i;
                    continue;
                }

                bg = playpal + 3*i;
                fg = playpal + 3*j;

                blend[r] = (tint_filter_pct * fg[r] + (100 - tint_filter_pct) * bg[r]) / 100;
                blend[g] = (tint_filter_pct * fg[g] + (100 - tint_filter_pct) * bg[g]) / 100;
                blend[b] = (tint_filter_pct * fg[b] + (100 - tint_filter_pct) * bg[b]) / 100;
                *tp1++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (shade_filter_pct * fg[r] + (100 - shade_filter_pct) * bg[r]) / 100;
                blend[g] = (shade_filter_pct * fg[g] + (100 - shade_filter_pct) * bg[g]) / 100;
                blend[b] = (shade_filter_pct * fg[b] + (100 - shade_filter_pct) * bg[b]) / 100;
                *tp2++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (fuzz_filter_pct * fg[r] + (100 - fuzz_filter_pct) * bg[r]) / 100;
                blend[g] = (fuzz_filter_pct * fg[g] + (100 - fuzz_filter_pct) * bg[g]) / 100;
                blend[b] = (fuzz_filter_pct * fg[b] + (100 - fuzz_filter_pct) * bg[b]) / 100;
                *tp3++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (90 * fg[r] + (100 - 90) * bg[r]) / 100;
                blend[g] = (90 * fg[g] + (100 - 90) * bg[g]) / 100;
                blend[b] = (90 * fg[b] + (100 - 90) * bg[b]) / 100;
                *tp90++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (80 * fg[r] + (100 - 80) * bg[r]) / 100;
                blend[g] = (80 * fg[g] + (100 - 80) * bg[g]) / 100;
                blend[b] = (80 * fg[b] + (100 - 80) * bg[b]) / 100;
                *tp80++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (70 * fg[r] + (100 - 70) * bg[r]) / 100;
                blend[g] = (70 * fg[g] + (100 - 70) * bg[g]) / 100;
                blend[b] = (70 * fg[b] + (100 - 70) * bg[b]) / 100;
                *tp70++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (60 * fg[r] + (100 - 60) * bg[r]) / 100;
                blend[g] = (60 * fg[g] + (100 - 60) * bg[g]) / 100;
                blend[b] = (60 * fg[b] + (100 - 60) * bg[b]) / 100;
                *tp60++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (50 * fg[r] + (100 - 50) * bg[r]) / 100;
                blend[g] = (50 * fg[g] + (100 - 50) * bg[g]) / 100;
                blend[b] = (50 * fg[b] + (100 - 50) * bg[b]) / 100;
                *tp50++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (40 * fg[r] + (100 - 40) * bg[r]) / 100;
                blend[g] = (40 * fg[g] + (100 - 40) * bg[g]) / 100;
                blend[b] = (40 * fg[b] + (100 - 40) * bg[b]) / 100;
                *tp40++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (30 * fg[r] + (100 - 30) * bg[r]) / 100;
                blend[g] = (30 * fg[g] + (100 - 30) * bg[g]) / 100;
                blend[b] = (30 * fg[b] + (100 - 30) * bg[b]) / 100;
                *tp30++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (20 * fg[r] + (100 - 20) * bg[r]) / 100;
                blend[g] = (20 * fg[g] + (100 - 20) * bg[g]) / 100;
                blend[b] = (20 * fg[b] + (100 - 20) * bg[b]) / 100;
                *tp20++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);

                blend[r] = (10 * fg[r] + (100 - 10) * bg[r]) / 100;
                blend[g] = (10 * fg[g] + (100 - 10) * bg[g]) / 100;
                blend[b] = (10 * fg[b] + (100 - 10) * bg[b]) / 100;
                *tp10++ = V_GetPaletteIndex(playpal, blend[r], blend[g], blend[b]);
            }
        }
    }

    W_ReleaseLumpName("PLAYPAL");
}


// -----------------------------------------------------------------------------
// R_InitBrightmaps
// [JN] Load in the brightmaps.
// Note: some tables as well as it's valuaes are taken from Doom Retro (r_data.c).
// Many thanks to Brad Harding for his amazing research of brightmap tables and colors!
// -----------------------------------------------------------------------------

static void R_InitBrightmaps (void)
{
    brightmaps_notgray         = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP1")), PU_STATIC);
    brightmaps_notgrayorbrown  = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP2")), PU_STATIC);
    brightmaps_redonly         = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP3")), PU_STATIC);
    brightmaps_greenonly1      = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP4")), PU_STATIC);
    brightmaps_greenonly2      = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP5")), PU_STATIC);
    brightmaps_greenonly3      = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP6")), PU_STATIC);
    brightmaps_orangeyellow    = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP7")), PU_STATIC);
    brightmaps_dimmeditems     = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP8")), PU_STATIC);
    brightmaps_brighttan       = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP9")), PU_STATIC);
    brightmaps_redonly1        = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP10")), PU_STATIC);
    brightmaps_explosivebarrel = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP11")), PU_STATIC);
    brightmaps_alllights       = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP12")), PU_STATIC);    
    brightmaps_candles         = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP13")), PU_STATIC);    
    brightmaps_pileofskulls    = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP14")), PU_STATIC);    
    brightmaps_redonly2        = W_CacheLumpNum(W_GetNumForName(DEH_String("BRTMAP15")), PU_STATIC);

    // [JN] Brightmaps are loaded. But where can they be applied?
    brightmaps_allowed = !vanillaparm && gamevariant != freedoom 
                                      && gamevariant != freedm;
}

// -----------------------------------------------------------------------------
// R_InitData
// Locates all the lumps that will be used by all views.
// Must be called after W_Init.
// -----------------------------------------------------------------------------

void R_InitData (void)
{
    // [JN] Moved R_InitFlats to the top, needed for 
    // R_GenerateComposite ivoking while level loading.
    R_InitFlats ();
    printf (".");
    R_InitTextures ();
    printf (".");
    R_InitSpriteLumps ();
    printf (".");
    R_InitColormaps ();

    // [JN] Generate translucency tables
    R_InitTransMaps ();
    printf (".");

    if (gamevariant != freedoom && gamevariant != freedm)
    {
        R_InitBrightmaps ();
        printf (".");
        R_InitBrightmappedTextures ();
    }
}

// -----------------------------------------------------------------------------
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
// -----------------------------------------------------------------------------

int R_FlatNumForName (char *name)
{
    char namet[9];
    int i = W_CheckNumForNameFromTo (name, lastflat, firstflat);

    if (i == -1)
    {
        namet[8] = 0;
        memcpy (namet, name,8);
        // [crispy] make non-fatal
        fprintf (stderr, english_language ?
                 "R_FlatNumForName: %s not found\n" :
                 "R_FlatNumForName: текстура поверхности %s не найдена\n",
                 namet);
        // [crispy] since there is no "No Flat" marker,
        // render missing flats as SKY
        return skyflatnum;
    }

    return i - firstflat;
}

// -----------------------------------------------------------------------------
// R_CheckTextureNumForName
// Check whether texture is available. Filter out NoTexture indicator.
// -----------------------------------------------------------------------------

int	R_CheckTextureNumForName (char *name)
{
    int key = W_LumpNameHash(name) % numtextures;
    texture_t *texture;

    // "NoTexture" marker.
    if (name[0] == '-')
    {
        return 0;
    }

    texture=textures_hashtable[key]; 

    while (texture != NULL)
    {
        if (!strncasecmp (texture->name, name, 8))
        {
            return texture->index;
        }

        texture = texture->next;
    }

    return -1;
}

// -----------------------------------------------------------------------------
// R_TextureNumForName
// Calls R_CheckTextureNumForName, aborts with error message.
// -----------------------------------------------------------------------------
int	R_TextureNumForName (char *name)
{
    int i = R_CheckTextureNumForName(name);

    if (i==-1)
    {
        // [crispy] make non-fatal
        fprintf (stderr, english_language ?
                 "R_TextureNumForName: %s not found\n" :
                 "R_TextureNumForName: текстура %s не найдена\n",
                 name);
        return 0;
    }

    return i;
}

// -----------------------------------------------------------------------------
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
// Totally rewritten by Lee Killough to use less memory,
// to avoid using alloca(), and to improve performance.
// -----------------------------------------------------------------------------

void R_PrecacheLevel (void)
{
    int   i;
    byte *hitlist;

    if (demoplayback)
    {
        return;
    }

    {
        size_t size = numflats > numsprites  ? numflats : numsprites;
        hitlist = malloc(numtextures > size ? numtextures : size);
    }

    // Precache flats.

    memset(hitlist, 0, numflats);

    for (i = numsectors ; --i >= 0 ; )
    hitlist[sectors[i].floorpic] = hitlist[sectors[i].ceilingpic] = 1;

    for (i = numflats ; --i >= 0 ; )
        if (hitlist[i])
            W_CacheLumpNum(firstflat + i, PU_CACHE);

    // Precache textures.

    memset(hitlist, 0, numtextures);

    for (i = numsides ; --i >= 0 ; )
    hitlist[sides[i].bottomtexture] =
    hitlist[sides[i].toptexture] =
    hitlist[sides[i].midtexture] = 1;

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependend
    //  name.

    hitlist[skytexture] = 1;

    for (i = numtextures ; --i >= 0 ; )
        if (hitlist[i])
        {
            texture_t *texture = textures[i];
            int j = texture->patchcount;

            // [crispy] precache composite textures
            R_GenerateComposite(i);

            while (--j >= 0)
            W_CacheLumpNum(texture->patches[j].patch, PU_CACHE);
        }

    // Precache sprites.
    memset(hitlist, 0, numsprites);

    {
        thinker_t *th;
        for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
            if (th->function.acp1 == (actionf_p1)P_MobjThinker)
            hitlist[((mobj_t *)th)->sprite] = 1;
    }

    for (i=numsprites; --i >= 0;)
        if (hitlist[i])
        {
            int j = sprites[i].numframes;

            while (--j >= 0)
            {
                short *sflump = sprites[i].spriteframes[j].lump;
                int k = 7;

                do
                W_CacheLumpNum(firstspritelump + sflump[k], PU_CACHE);
                while (--k >= 0);
            }
        }

    free(hitlist);
}
