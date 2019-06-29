//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2019 Julian Nechaevsky
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

#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"

#include "w_wad.h"

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"

#include "doomstat.h"
#include "r_sky.h"

#include  <alloca.h>


#include "r_data.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
// 



//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef struct
{
    short	originx;
    short	originy;
    short	patch;
    short	stepdir;
    short	colormap;
} mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
    char		name[8];
    boolean		masked;	
    short		width;
    short		height;
    void		**columndirectory;	// OBSOLETE
    short		patchcount;
    mappatch_t	patches[1];
} maptexture_t;


// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    int		originx;	
    int		originy;
    int		patch;
} texpatch_t;


// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.
typedef struct
{
    // Keep name for switch changing, etc.
    char	name[8];		
    short	width;
    short	height;
    
    // All the patches[patchcount]
    //  are drawn back to front into the cached texture.
    short	patchcount;
    texpatch_t	patches[1];		
    
} texture_t;



int		firstflat;
int		lastflat;
int		numflats;

int		firstpatch;
int		lastpatch;
int		numpatches;

int		firstspritelump;
int		lastspritelump;
int		numspritelumps;

int		numtextures;
texture_t**	textures;


int*			texturewidthmask;
// needed for texture pegging
fixed_t*		textureheight;		
int*			texturecompositesize;
short**			texturecolumnlump;
unsigned short**	texturecolumnofs;
byte**			texturecomposite;

// for global animation
int*		flattranslation;
int*		texturetranslation;

// needed for pre rendering
fixed_t*	spritewidth;	
fixed_t*	spriteoffset;
fixed_t*	spritetopoffset;

lighttable_t	*colormaps;

// [JN] Brightmaps
lighttable_t	*brightmaps_notgray;
lighttable_t	*brightmaps_notgrayorbrown;
lighttable_t	*brightmaps_redonly;
lighttable_t	*brightmaps_greenonly1;
lighttable_t	*brightmaps_greenonly2;
lighttable_t	*brightmaps_greenonly3;
lighttable_t	*brightmaps_orangeyellow;
lighttable_t	*brightmaps_dimmeditems;
lighttable_t	*brightmaps_brighttan;
lighttable_t	*brightmaps_redonly1;
lighttable_t	*brightmaps_explosivebarrel;
lighttable_t	*brightmaps_alllights;
lighttable_t	*brightmaps_candles;

//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//



//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
void
R_DrawColumnInCache
( column_t*	patch,
  byte*		cache,
  int		originy,
  int		cacheheight )
{
    int		count;
    int		position;
    byte*	source;
    byte*	dest;
	
    dest = (byte *)cache + 3;
	
    while (patch->topdelta != 0xff)
    {
	source = (byte *)patch + 3;
	count = patch->length;
	position = originy + patch->topdelta;

	if (position < 0)
	{
	    count += position;
	    position = 0;
	}

	if (position + count > cacheheight)
	    count = cacheheight - position;

	if (count > 0)
	    memcpy (cache + position, source, count);
		
	patch = (column_t *)(  (byte *)patch + patch->length + 4); 
    }
}



//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
void R_GenerateComposite (int texnum)
{
    byte*		block;
    texture_t*		texture;
    texpatch_t*		patch;	
    patch_t*		realpatch;
    int			x;
    int			x1;
    int			x2;
    int			i;
    column_t*		patchcol;
    short*		collump;
    unsigned short*	colofs;
	
    texture = textures[texnum];

    block = Z_Malloc (texturecompositesize[texnum],
		      PU_STATIC, 
		      &texturecomposite[texnum]);	

    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];
    
    // Composite the columns together.
    patch = texture->patches;
		
    for (i=0 , patch = texture->patches;
	 i<texture->patchcount;
	 i++, patch++)
    {
	realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
	x1 = patch->originx;
	x2 = x1 + SHORT(realpatch->width);

	if (x1<0)
	    x = 0;
	else
	    x = x1;
	
	if (x2 > texture->width)
	    x2 = texture->width;

	for ( ; x<x2 ; x++)
	{
	    // Column does not have multiple patches?
	    if (collump[x] >= 0)
		continue;
	    
	    patchcol = (column_t *)((byte *)realpatch
				    + LONG(realpatch->columnofs[x-x1]));
	    R_DrawColumnInCache (patchcol,
				 block + colofs[x],
				 patch->originy,
				 texture->height);
	}
						
    }

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    Z_ChangeTag (block, PU_CACHE);
}



//
// R_GenerateLookup
//
void R_GenerateLookup (int texnum)
{
    texture_t*		texture;
    byte*		patchcount;	// patchcount[texture->width]
    texpatch_t*		patch;	
    patch_t*		realpatch;
    int			x;
    int			x1;
    int			x2;
    int			i;
    short*		collump;
    unsigned short*	colofs;
	
    texture = textures[texnum];

    // Composited texture not created yet.
    texturecomposite[texnum] = 0;
    
    texturecompositesize[texnum] = 0;
    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];
    
    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.
    patchcount = (byte *)alloca (texture->width);
    memset (patchcount, 0, texture->width);
    patch = texture->patches;
		
    for (i=0 , patch = texture->patches;
	 i<texture->patchcount;
	 i++, patch++)
    {
	realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
	x1 = patch->originx;
	x2 = x1 + SHORT(realpatch->width);
	
	if (x1 < 0)
	    x = 0;
	else
	    x = x1;

	if (x2 > texture->width)
	    x2 = texture->width;
	for ( ; x<x2 ; x++)
	{
	    patchcount[x]++;
	    collump[x] = patch->patch;
	    colofs[x] = LONG(realpatch->columnofs[x-x1])+3;
	}
    }
	
    for (x=0 ; x<texture->width ; x++)
    {
	if (!patchcount[x])
	{
	    printf ("R_GenerateLookup: Column %d is without a patch in texture %s\n",
		    x, texture->name);
	    return;
	}
	// I_Error ("R_GenerateLookup: column without a patch");
	
	if (patchcount[x] > 1)
	{
	    // Use the cached block.
	    collump[x] = -1;	
	    colofs[x] = texturecompositesize[texnum];
	    
	    if (texturecompositesize[texnum] > 0x10000-texture->height)
	    {
		I_Error ("R_GenerateLookup: Размер текстуры %i превышает 64 килобайта",
			 texnum);
	    }
	    
	    texturecompositesize[texnum] += texture->height;
	}
    }	
}




//
// R_GetColumn
//
byte*
R_GetColumn
( int		tex,
  int		col )
{
    int		lump;
    int		ofs;
	
    col &= texturewidthmask[tex];
    lump = texturecolumnlump[tex][col];
    ofs = texturecolumnofs[tex][col];
    
    if (lump > 0)
	return (byte *)W_CacheLumpNum(lump,PU_CACHE)+ofs;

    if (!texturecomposite[tex])
	R_GenerateComposite (tex);

    return texturecomposite[tex] + ofs;
}




//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
void R_InitTextures (void)
{
    maptexture_t*	mtexture;
    texture_t*		texture;
    mappatch_t*		mpatch;
    texpatch_t*		patch;

    int			i;
    int			j;

    int*		maptex;
    int*		maptex2;
    int*		maptex1;
    
    char		name[9];
    char*		names;
    char*		name_p;
    
    int*		patchlookup;
    
    int			totalwidth;
    int			nummappatches;
    int			offset;
    int			maxoff;
    int			maxoff2;
    int			numtextures1;
    int			numtextures2;

    int*		directory;
    
    int			temp1;
    int			temp2;
    int			temp3;

    
    // Load the patch names from pnames.lmp.
    name[8] = 0;	
    names = W_CacheLumpName ("PNAMES", PU_STATIC);
    nummappatches = LONG ( *((int *)names) );
    name_p = names+4;
    patchlookup = malloc (nummappatches*sizeof(*patchlookup));  // killough
    
    for (i=0 ; i<nummappatches ; i++)
    {
	strncpy (name,name_p+i*8, 8);
	patchlookup[i] = W_CheckNumForName (name);
    }
    Z_Free (names);
    
    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex = maptex1 = W_CacheLumpName ("TEXTURE1", PU_STATIC);
    numtextures1 = LONG(*maptex);
    maxoff = W_LumpLength (W_GetNumForName ("TEXTURE1"));
    directory = maptex+1;
	
    if (W_CheckNumForName ("TEXTURE2") != -1)
    {
	maptex2 = W_CacheLumpName ("TEXTURE2", PU_STATIC);
	numtextures2 = LONG(*maptex2);
	maxoff2 = W_LumpLength (W_GetNumForName ("TEXTURE2"));
    }
    else
    {
	maptex2 = NULL;
	numtextures2 = 0;
	maxoff2 = 0;
    }
    numtextures = numtextures1 + numtextures2;

    // killough 4/9/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof

    textures = Z_Malloc (numtextures*sizeof*(textures), PU_STATIC, 0);
    texturecolumnlump = Z_Malloc (numtextures*sizeof*(texturecolumnlump), PU_STATIC, 0);
    texturecolumnofs = Z_Malloc (numtextures*sizeof*(texturecolumnofs), PU_STATIC, 0);
    texturecomposite = Z_Malloc (numtextures*sizeof*(texturecomposite), PU_STATIC, 0);
    texturecompositesize = Z_Malloc (numtextures*sizeof*(texturecompositesize), PU_STATIC, 0);
    texturewidthmask = Z_Malloc (numtextures*sizeof*(texturewidthmask), PU_STATIC, 0);
    textureheight = Z_Malloc (numtextures*sizeof*(textureheight), PU_STATIC, 0);

    totalwidth = 0;
    
    //	Really complex printing shit...
    temp1 = W_GetNumForName ("S_START");  // P_???????
    temp2 = W_GetNumForName ("S_END") - 1;
    temp3 = ((temp2-temp1+63)/64) + ((numtextures+63)/64);
    printf("[");
    for (i = 0; i < temp3; i++)
	printf(" ");
    printf("         ]");
    for (i = 0; i < temp3; i++)
	printf("\x8");
    printf("\x8\x8\x8\x8\x8\x8\x8\x8\x8\x8");	
	
    for (i=0 ; i<numtextures ; i++, directory++)
    {
	if (!(i&63))
	    printf (".");

	if (i == numtextures1)
	{
	    // Start looking in second texture file.
	    maptex = maptex2;
	    maxoff = maxoff2;
	    directory = maptex+1;
	}
		
	offset = LONG(*directory);

	if (offset > maxoff)
	    I_Error ("R_InitTextures: Некорректная директория текстур");
	
	mtexture = (maptexture_t *) ( (byte *)maptex + offset);

	texture = textures[i] =
	    Z_Malloc (sizeof(texture_t)
		      + sizeof(texpatch_t)*(SHORT(mtexture->patchcount)-1),
		      PU_STATIC, 0);
	
	texture->width = SHORT(mtexture->width);
	texture->height = SHORT(mtexture->height);
	texture->patchcount = SHORT(mtexture->patchcount);

	memcpy (texture->name, mtexture->name, sizeof(texture->name));
	mpatch = &mtexture->patches[0];
	patch = &texture->patches[0];

	for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
	{
	    patch->originx = SHORT(mpatch->originx);
	    patch->originy = SHORT(mpatch->originy);
	    patch->patch = patchlookup[SHORT(mpatch->patch)];
	    if (patch->patch == -1)
        {
        // [crispy] make non-fatal
        fprintf (stderr, "R_InitTextures: Отсутствует патч в текстуре %s\n", texture->name);
        patch->patch = 0;
        }
	}		

    // killough 4/9/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    // killough 12/98: fix sizeofs

    texturecolumnlump[i] = Z_Malloc (texture->width*sizeof**texturecolumnlump, PU_STATIC,0);
    texturecolumnofs[i] = Z_Malloc(texture->width*sizeof**texturecolumnofs, PU_STATIC,0);

	j = 1;
	while (j*2 <= texture->width)
	    j<<=1;

	texturewidthmask[i] = j-1;
	textureheight[i] = texture->height<<FRACBITS;
		
	totalwidth += texture->width;
    }

    free(patchlookup);  // killough

    Z_Free (maptex1);
    if (maptex2)
	Z_Free (maptex2);
    
    // Precalculate whatever possible.	
    for (i=0 ; i<numtextures ; i++)
	R_GenerateLookup (i);
    
    // killough 4/9/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    texturetranslation = Z_Malloc((numtextures+1)*sizeof*texturetranslation, PU_STATIC, 0);
    
    for (i=0 ; i<numtextures ; i++)
	texturetranslation[i] = i;
}



//
// R_InitFlats
//
void R_InitFlats (void)
{
    int     i;

    firstflat = W_GetNumForName ("F_START") + 1;
    lastflat  = W_GetNumForName ("F_END") - 1;
    numflats  = lastflat - firstflat + 1;

    // Create translation table for global animation.
    // killough 4/9/98: make column offsets 32-bit;
    // clean up malloc-ing to use sizeof
    flattranslation = Z_Malloc ((numflats+1)*sizeof(*flattranslation), PU_STATIC, 0);

    for (i=0 ; i<numflats ; i++)
    flattranslation[i] = i;
}


//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
void R_InitSpriteLumps (void)
{
    int     i;
    patch_t *patch;

    firstspritelump = W_GetNumForName ("S_START") + 1;
    lastspritelump  = W_GetNumForName ("S_END") - 1;
    numspritelumps  = lastspritelump - firstspritelump + 1;

    // killough 4/9/98: make columnd offsets 32-bit;
    // clean up malloc-ing to use sizeof

    spritewidth = Z_Malloc (numspritelumps*sizeof*(spritewidth), PU_STATIC, 0);
    spriteoffset = Z_Malloc (numspritelumps*sizeof*(spriteoffset), PU_STATIC, 0);
    spritetopoffset = Z_Malloc (numspritelumps*sizeof*(spritetopoffset), PU_STATIC, 0);

    for (i=0 ; i< numspritelumps ; i++)
    {
        if (!(i&63))
        printf (".");

        patch = W_CacheLumpNum (firstspritelump+i, PU_CACHE);

        spritewidth[i] = SHORT(patch->width)<<FRACBITS;
        spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
        spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
    }
}



//
// R_InitColormaps
//
void R_InitColormaps (void)
{
    int lump, lump2, lump3, lump4, lump5, lump6, lump7, lump8, lump9;
    int lump10, lump11, lump12, lump13, lump14;
    int length;
    
    // Load in the light tables, 
    //  256 byte align tables.
    lump = W_GetNumForName("COLORMAP"); 
    length = W_LumpLength (lump) + 255; 
    colormaps = Z_Malloc (length, PU_STATIC, 0); 
    colormaps = (byte *)( ((int)colormaps + 255)&~0xff); 
    W_ReadLump (lump,colormaps); 
    
    // [JN] Loading brightmaps
    // Note: tables as well as it's valuaes are taken from Doom Retro (r_data.c).
    // Many thanks to Brad Harding for his amazing research of brightmap tables and colors!

    lump2 = W_GetNumForName("BRTMAP1"); 
    brightmaps_notgray = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_notgray = (byte *)( ((int)brightmaps_notgray + 255)&~0xff); 
    W_ReadLump (lump2,brightmaps_notgray);

    lump3 = W_GetNumForName("BRTMAP2"); 
    brightmaps_notgrayorbrown = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_notgrayorbrown = (byte *)( ((int)brightmaps_notgrayorbrown + 255)&~0xff); 
    W_ReadLump (lump3,brightmaps_notgrayorbrown); 

    lump4 = W_GetNumForName("BRTMAP3"); 
    brightmaps_redonly = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_redonly = (byte *)( ((int)brightmaps_redonly + 255)&~0xff); 
    W_ReadLump (lump4,brightmaps_redonly);
    
    lump5 = W_GetNumForName("BRTMAP4"); 
    brightmaps_greenonly1 = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_greenonly1 = (byte *)( ((int)brightmaps_greenonly1 + 255)&~0xff); 
    W_ReadLump (lump5,brightmaps_greenonly1); 
    
    lump6 = W_GetNumForName("BRTMAP5"); 
    brightmaps_greenonly2 = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_greenonly2 = (byte *)( ((int)brightmaps_greenonly2 + 255)&~0xff); 
    W_ReadLump (lump6,brightmaps_greenonly2);

    lump7 = W_GetNumForName("BRTMAP6"); 
    brightmaps_greenonly3 = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_greenonly3 = (byte *)( ((int)brightmaps_greenonly3 + 255)&~0xff); 
    W_ReadLump (lump7,brightmaps_greenonly3);
    
    lump8 = W_GetNumForName("BRTMAP7"); 
    brightmaps_orangeyellow = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_orangeyellow = (byte *)( ((int)brightmaps_orangeyellow + 255)&~0xff); 
    W_ReadLump (lump8,brightmaps_orangeyellow);
    
    lump9 = W_GetNumForName("BRTMAP8"); 
    brightmaps_dimmeditems = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_dimmeditems = (byte *)( ((int)brightmaps_dimmeditems + 255)&~0xff); 
    W_ReadLump (lump9,brightmaps_dimmeditems);

    lump10 = W_GetNumForName("BRTMAP9"); 
    brightmaps_brighttan = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_brighttan = (byte *)( ((int)brightmaps_brighttan + 255)&~0xff); 
    W_ReadLump (lump10,brightmaps_brighttan);

    lump11 = W_GetNumForName("BRTMAP10"); 
    brightmaps_redonly1 = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_redonly1 = (byte *)( ((int)brightmaps_redonly1 + 255)&~0xff); 
    W_ReadLump (lump11,brightmaps_redonly1);

    lump12 = W_GetNumForName("BRTMAP11"); 
    brightmaps_explosivebarrel = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_explosivebarrel = (byte *)( ((int)brightmaps_explosivebarrel + 255)&~0xff); 
    W_ReadLump (lump12,brightmaps_explosivebarrel);

    lump13 = W_GetNumForName("BRTMAP12"); 
    brightmaps_alllights = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_alllights = (byte *)( ((int)brightmaps_alllights + 255)&~0xff); 
    W_ReadLump (lump13,brightmaps_alllights);

    lump14 = W_GetNumForName("BRTMAP13"); 
    brightmaps_candles = Z_Malloc (length, PU_STATIC, 0); 
    brightmaps_candles = (byte *)( ((int)brightmaps_candles + 255)&~0xff); 
    W_ReadLump (lump14,brightmaps_candles);    
}



//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
    R_InitTextures ();
    printf (".");
    R_InitFlats ();
    printf (".");
    R_InitSpriteLumps ();
    printf (".");
    R_InitColormaps ();
}



//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName (char* name)
{
    int		i;
    char	namet[9];

    i = W_CheckNumForName (name);

    if (i == -1)
    {
	namet[8] = 0;
	memcpy (namet, name,8);
	// [crispy] since there is no "No Flat" marker,
	// render missing flats as SKY
	return skyflatnum;
    }
    return i - firstflat;
}




//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int	R_CheckTextureNumForName (char *name)
{
    int		i;

    // "NoTexture" marker.
    if (name[0] == '-')		
	return 0;
		
    for (i=0 ; i<numtextures ; i++)
	if (!strncasecmp (textures[i]->name, name, 8) )
	    return i;
		
    return -1;
}



//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int	R_TextureNumForName (char* name)
{
    int		i;
	
    i = R_CheckTextureNumForName (name);

    if (i==-1)
    {
        // [crispy] make non-fatal
        fprintf (stderr, "R_TextureNumForName: Текстура %s не найдена\n", name);
        return 0;
    }
    return i;
}




//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
// Totally rewritten by Lee Killough to use less memory,
// to avoid using alloca(), and to improve performance.

void R_PrecacheLevel (void)
{
    register int i;
    register byte *hitlist;

    if (demoplayback)
    return;

    {
        size_t size = numflats > numsprites  ? numflats : numsprites;
        hitlist = malloc(numtextures > size ? numtextures : size);
    }

    // Precache flats.

    memset(hitlist, 0, numflats);

    for (i = numsectors; --i >= 0; )
    hitlist[sectors[i].floorpic] = hitlist[sectors[i].ceilingpic] = 1;

    for (i = numflats; --i >= 0; )
        if (hitlist[i])
            W_CacheLumpNum(firstflat + i, PU_CACHE);

    // Precache textures.

    memset(hitlist, 0, numtextures);

    for (i = numsides; --i >= 0;)
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

    for (i = numtextures; --i >= 0; )
        if (hitlist[i])
        {
            texture_t *texture = textures[i];
            int j = texture->patchcount;

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




