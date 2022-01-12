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


#include <stdio.h>
#include <stdlib.h>
#include "h2def.h"
#include "i_system.h"
#include "i_swap.h"
#include "r_local.h"


typedef struct
{
    int x1, x2;

    int column;
    int topclip;
    int bottomclip;
} maskdraw_t;

typedef struct drawseg_xrange_item_s
{
    short x1, x2;
    drawseg_t *user;
} drawseg_xrange_item_t;

typedef struct drawsegs_xrange_s
{
    drawseg_xrange_item_t *items;
    int count;
} drawsegs_xrange_t;

/*
================================================================================

						INITIALIZATION FUNCTIONS

================================================================================
*/

// variables used to look up and range check thing_t sprites patches
spritedef_t *sprites;
int numsprites;

spriteframe_t sprtemp[30];
int maxframe;
char *spritename;

// constant arrays used for psprite clipping and initializing clipping
int negonearray[WIDESCREENWIDTH];       // [crispy] 32-bit integer math
int screenheightarray[WIDESCREENWIDTH]; // [crispy] 32-bit integer math

static size_t num_vissprite, num_vissprite_alloc, num_vissprite_ptrs; // [JN] killough
static vissprite_t *vissprites, **vissprite_ptrs;                     // [JN] killough

int *mfloorclip, *mceilingclip;   // [crispy] 32-bit integer math
fixed_t spryscale;
int64_t sprtopscreen, sprbotscreen;

// Sprite rotation 0 is facing the viewer, rotation 1 is one angle turn CLOCKWISE
// around the axis. This is not the same as the angle, which increases counter
// clockwise (protractor).  There was a lot of stuff grabbed wrong, so I changed it...
fixed_t pspritescale, pspriteiscale;

#define DS_RANGES_COUNT 3
static drawsegs_xrange_t drawsegs_xranges[DS_RANGES_COUNT];

static drawseg_xrange_item_t *drawsegs_xrange;
static unsigned int drawsegs_xrange_size = 0;
static int drawsegs_xrange_count = 0;

// [JN] Light tables and brightmaps
lighttable_t **spritelights;
lighttable_t **fullbrights_greenonly;
lighttable_t **fullbrights_redonly;
lighttable_t **fullbrights_blueonly;
lighttable_t **fullbrights_purpleonly;
lighttable_t **fullbrights_flame;
lighttable_t **fullbrights_yellowred;
lighttable_t **fullbrights_firebull;

boolean LevelUseFullBright;



/*
================================================================================
=
= R_InitSpritesRes
=
================================================================================
*/

void R_InitSpritesRes (void)
{
    if (xtoviewangle)
    {
        free(xtoviewangle);
    }
    if (linearskyangle)
    {
        free(linearskyangle);
    }
    /*
    if (negonearray)
    {
        free(negonearray);
    }
    if (screenheightarray)
    {
        free(screenheightarray);
    }
    */

    xtoviewangle = calloc(1, (screenwidth + 1) * sizeof(*xtoviewangle));
    linearskyangle = calloc(1, (screenwidth + 1) * sizeof(*linearskyangle));
    
    /*
    negonearray = calloc(1, screenwidth * sizeof(*negonearray));
    screenheightarray = calloc(1, screenwidth * sizeof(*screenheightarray));

    if (clipbot)
    {
        free(clipbot);
    }

    clipbot = calloc(1, 2 * screenwidth * sizeof(*clipbot));
    cliptop = clipbot + screenwidth;
    */
}

/*
=================
=
= R_InstallSpriteLump
=
= Local function for R_InitSprites
=================
*/

void R_InstallSpriteLump(int lump, unsigned frame, unsigned rotation,
                         boolean flipped)
{
    int r;

    if (frame >= 30 || rotation > 8)
        I_Error(english_language ?
                "R_InstallSpriteLump: Bad frame characters in lump %i" :
                "R_InstallSpriteLump: некорректные символы фрейма в блоке %i",
                lump);

    if ((int) frame > maxframe)
        maxframe = frame;

    if (rotation == 0)
    {
// the lump should be used for all rotations
        if (sprtemp[frame].rotate == false)
            I_Error(english_language ?
                    "R_InitSprites: Sprite %s frame %c has multip rot=0 lump" :
                    "R_InitSprites: фрейм %c спрайта %s имеет многократный блок rot=0",
                    spritename, 'A' + frame);
        if (sprtemp[frame].rotate == true)
            I_Error(english_language ?
                    "R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump" :
                    "R_InitSprites: фрейм %c спрайта %s имеет фреймы поворота и блок rot=0",
                    spritename, 'A' + frame);

        sprtemp[frame].rotate = false;
        for (r = 0; r < 8; r++)
        {
            sprtemp[frame].lump[r] = lump - firstspritelump;
            sprtemp[frame].flip[r] = (byte) flipped;
        }
        return;
    }

// the lump is only used for one rotation
    if (sprtemp[frame].rotate == false)
        I_Error(english_language ?
                "R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump" :
                "R_InitSprites: фрейм спрайта %c спрайта %s имеет фреймы поворота и блок rot=0",
                spritename, 'A' + frame);

    sprtemp[frame].rotate = true;

    rotation--;                 // make 0 based
    if (sprtemp[frame].lump[rotation] != -1)
        I_Error(english_language ?
                "R_InitSprites: Sprite %s : %c : %c has two lumps mapped to it" :
                "R_InitSprites: спрайу %s : %c : %c назначено несколько одинаковых блоков",
                spritename, 'A' + frame, '1' + rotation);

    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip[rotation] = (byte) flipped;
}

/*
=================
=
= R_InitSpriteDefs
=
= Pass a null terminated list of sprite names (4 chars exactly) to be used
= Builds the sprite rotation matrixes to account for horizontally flipped
= sprites.  Will report an error if the lumps are inconsistant
=Only called at startup
=
= Sprite lump names are 4 characters for the actor, a letter for the frame,
= and a number for the rotation, A sprite that is flippable will have an
= additional letter/number appended.  The rotation character can be 0 to
= signify no rotations
=================
*/

void R_InitSpriteDefs(char **namelist)
{
    char **check;
    int i, l, frame, rotation;
    int start, end;

// count the number of sprite names
    check = namelist;
    while (*check != NULL)
        check++;
    numsprites = check - namelist;

    if (!numsprites)
        return;

    sprites = Z_Malloc(numsprites * sizeof(*sprites), PU_STATIC, NULL);

    start = firstspritelump - 1;
    end = lastspritelump + 1;

// scan all the lump names for each of the names, noting the highest
// frame letter
// Just compare 4 characters as ints
    for (i = 0; i < numsprites; i++)
    {
        spritename = namelist[i];
        memset(sprtemp, -1, sizeof(sprtemp));

        maxframe = -1;

        //
        // scan the lumps, filling in the frames for whatever is found
        //
        for (l = start + 1; l < end; l++)
            if (!strncmp(lumpinfo[l]->name, namelist[i], 4))
            {
                frame = lumpinfo[l]->name[4] - 'A';
                rotation = lumpinfo[l]->name[5] - '0';
                R_InstallSpriteLump(l, frame, rotation, false);
                if (lumpinfo[l]->name[6])
                {
                    frame = lumpinfo[l]->name[6] - 'A';
                    rotation = lumpinfo[l]->name[7] - '0';
                    R_InstallSpriteLump(l, frame, rotation, true);
                }
            }

        //
        // check the frames that were found for completeness
        //
        if (maxframe == -1)
        {
            //continue;
            sprites[i].numframes = 0;
            if (gamemode == shareware)
                continue;
            I_Error(english_language ?
                    "R_InitSprites: No lumps found for sprite %s" :
                    "R_InitSprites: не найдены блоки в спрайте %s",
                    namelist[i]);
        }

        maxframe++;
        for (frame = 0; frame < maxframe; frame++)
        {
            switch ((int) sprtemp[frame].rotate)
            {
                case -1:       // no rotations were found for that frame at all
                    I_Error(english_language ?
                            "R_InitSprites: No patches found for %s frame %c" :
                            "R_InitSprites: не найдены патчи для спрайта %s, фрейма %c",
                            namelist[i], frame + 'A');
                case 0:        // only the first rotation is needed
                    break;

                case 1:        // must have all 8 frames
                    for (rotation = 0; rotation < 8; rotation++)
                        if (sprtemp[frame].lump[rotation] == -1)
                            I_Error (english_language ?
                                     "R_InitSprites: Sprite %s frame %c is missing rotations" :
                                     "R_InitSprites: в фрейме %c спрайта %s отсутствует информация о вращении",
                                     namelist[i], frame + 'A');
            }
        }

        //
        // allocate space for the frames present and copy sprtemp to it
        //
        sprites[i].numframes = maxframe;
        sprites[i].spriteframes =
            Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
        memcpy(sprites[i].spriteframes, sprtemp,
               maxframe * sizeof(spriteframe_t));
    }

}

/*
================================================================================
=
= R_InitSprites
=
= Called at program start.
=
================================================================================
*/

void R_InitSprites(char **namelist)
{
    int i;

    for (i = 0; i < screenwidth; i++)
    {
        negonearray[i] = -1;
    }

    R_InitSpriteDefs(namelist);
}

/*
================================================================================
=
= R_ClearSprites
=
= Called at frame start.
=
================================================================================
*/

void R_ClearSprites(void)
{
    num_vissprite = 0;  // [JN] killough
}

/*
================================================================================
=
= R_NewVisSprite
=
================================================================================
*/

vissprite_t *R_NewVisSprite(void)
{
    if (num_vissprite >= num_vissprite_alloc)   // [JN] killough
    {
        size_t num_vissprite_alloc_prev = num_vissprite_alloc;

        num_vissprite_alloc = num_vissprite_alloc ? num_vissprite_alloc*2 : 128;
        vissprites = realloc(vissprites,num_vissprite_alloc*sizeof(*vissprites));

        // [JN] e6y: set all fields to zero
        memset(vissprites + num_vissprite_alloc_prev, 0,
        (num_vissprite_alloc - num_vissprite_alloc_prev)*sizeof(*vissprites));
    }
    return vissprites + num_vissprite++;
}


/*
================================================================================
=
= R_DrawMaskedColumn
=
= Used for sprites and masked mid textures.
= Masked means: partly transparent, i.e. stored in posts/runs of opaque pixels.
=
================================================================================
*/

void R_DrawMaskedColumn (column_t *column, signed int baseclip)
{
    int64_t	topscreen;      // [crispy] WiggleFix
    int64_t bottomscreen;   // [crispy] WiggleFix
    fixed_t	basetexturemid;
    int		top = -1;
	
    basetexturemid = dc_texturemid;
    dc_texheight = 0;       // [crispy] Tutti-Frutti fix
	
    while (column->topdelta != 0xff)
    {
        // [crispy] support for DeePsea tall patches
        if (column->topdelta <= top)
        {
            top += column->topdelta;
        }
        else
        {
            top = column->topdelta;
        }

        // calculate unclipped screen coordinates for post
        topscreen = sprtopscreen + spryscale*top;
        bottomscreen = topscreen + spryscale*column->length;

        dc_yl = (int)((topscreen+FRACUNIT-1)>>FRACBITS); // [crispy] WiggleFix
        dc_yh = (int)((bottomscreen-1)>>FRACBITS);       // [crispy] WiggleFix
		
        if (dc_yh >= mfloorclip[dc_x])
            dc_yh = mfloorclip[dc_x]-1;
        if (dc_yl <= mceilingclip[dc_x])
            dc_yl = mceilingclip[dc_x]+1;
        if (dc_yh >= baseclip && baseclip != -1)
            dc_yh = baseclip;

        // [JN] killough 3/2/98, 3/27/98: Failsafe against overflow/crash:
        if (dc_yl <= dc_yh && dc_yh < viewheight)
        {
            dc_source = (byte *)column + 3;
            dc_texturemid = basetexturemid - (top<<FRACBITS);

            // Drawn by either R_DrawColumn or (SHADOW) R_DrawTLColumn.
            colfunc ();	
        }

        column = (column_t *)(  (byte *)column + column->length + 4);
    }

    dc_texturemid = basetexturemid;
}

/*
================================================================================
=
= R_DrawVisSprite
=
= mfloorclip and mceilingclip should also be set.
=
================================================================================
*/

void R_DrawVisSprite(vissprite_t * vis, int x1, int x2)
{
    column_t *column;
    int texturecolumn;
    fixed_t frac;
    patch_t *patch;
    fixed_t baseclip;

    patch = W_CacheLumpNum(vis->patch + firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;

    if (vis->mobjflags & (MF_SHADOW | MF_ALTSHADOW))
    {
        if (vis->mobjflags & MF_TRANSLATION)
        {
            colfunc = transtlcolfunc;
            dc_translation = translationtables - 256
                + vis->class * ((maxplayers - 1) * 256) +
                ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
        }
        else if (vis->mobjflags & MF_SHADOW)
        {                       // Draw using shadow column function
            colfunc = tlcolfunc;
        }
        else
        {
            colfunc = R_DrawAltTLColumn;
        }
    }
    else if (vis->mobjflags & MF_TRANSLATION)
    {
        // Draw using translated column function
        colfunc = transcolfunc;
        dc_translation = translationtables - 256
            + vis->class * ((maxplayers - 1) * 256) +
            ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
    }

    dc_iscale = abs(vis->xiscale) >> (detailshift && !hires);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;

    sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);

    // check to see if vissprite is a weapon
    if (vis->psprite)
    {
        dc_texturemid += FixedMul(((centery - viewheight / 2) << FRACBITS),
                                  pspriteiscale);
        sprtopscreen += (viewheight / 2 - centery) << FRACBITS;
    }

    if (vis->floorclip && !vis->psprite)
    {
        sprbotscreen = sprtopscreen + FixedMul(SHORT(patch->height) << FRACBITS,
                                               spryscale);
        baseclip = (sprbotscreen - FixedMul(vis->floorclip,
                                            spryscale)) >> FRACBITS;
    }
    else
    {
        baseclip = -1;
    }

    for (dc_x = vis->x1; dc_x <= vis->x2; dc_x++, frac += vis->xiscale)
    {
        texturecolumn = frac >> FRACBITS;
#ifdef RANGECHECK
        if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
            I_Error(english_language ?
                    "R_DrawSpriteRange: bad texturecolumn" :
                    "R_DrawSpriteRange: некорректныая информация texturecolumn");
#endif
        column = (column_t *) ((byte *) patch +
                               LONG(patch->columnofs[texturecolumn]));
        R_DrawMaskedColumn(column, baseclip);
    }

    colfunc = basecolfunc;
}

/*
================================================================================
=
= R_ProjectSprite
=
= Generates a vissprite for a thing if it might be visible.
=
================================================================================
*/

void R_ProjectSprite(mobj_t * thing)
{
    int            x1, x2, lump, index;
    unsigned       rot;
    fixed_t        trx, try, tx, tz, gxt, gyt, xscale, iscale;
    fixed_t        gzt;    // [JN] killough 3/27/98
    angle_t        ang;
    boolean        flip;
    vissprite_t   *vis;
    spritedef_t   *sprdef;
    spriteframe_t *sprframe;
    fixed_t        interpx, interpy, interpz, interpangle;

    // [AM] Interpolate between current and last position,
    //      if prudent.
    if (uncapped_fps && !vanillaparm &&
        // Don't interpolate if the mobj did something
        // that would necessitate turning it off for a tic.
        thing->interp == true &&
        // Don't interpolate during a paused state.
        !paused && (!menuactive || demoplayback || netgame))
    {
        interpx = thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic);
        interpy = thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic);
        interpz = thing->oldz + FixedMul(thing->z - thing->oldz, fractionaltic);
        interpangle = R_InterpolateAngle(thing->oldangle, thing->angle, fractionaltic);
    }
    else
    {
        interpx = thing->x;
        interpy = thing->y;
        interpz = thing->z;
        interpangle = thing->angle;
    }

    if (thing->flags2 & MF2_DONTDRAW)
    {
        // Never make a vissprite when MF2_DONTDRAW is flagged.
        return;
    }

    // Transform the origin point.
    trx = interpx - viewx;
    try = interpy - viewy;
    gxt = FixedMul(trx, viewcos);
    gyt = -FixedMul(try, viewsin);
    tz = gxt - gyt;

    if (tz < MINZ)
    {
        // thing is behind view plane
        return;
    }

    xscale = FixedDiv(projection, tz);

    gxt = -FixedMul(trx, viewsin);
    gyt = FixedMul(try, viewcos);
    tx = -(gyt + gxt);

    // Too far off the side.
    if (abs(tx) > (tz << 2))
    {
        return;
    }

    // Decide which patch to use for sprite reletive to player.
#ifdef RANGECHECK
    if ((unsigned) thing->sprite >= numsprites)
        I_Error(english_language ?
                "R_ProjectSprite: invalid sprite number %i " :
                "R_ProjectSprite: некорректный номер спрайта %i ",
                thing->sprite);
#endif
    sprdef = &sprites[thing->sprite];
#ifdef RANGECHECK
    if ((thing->frame & FF_FRAMEMASK) >= sprdef->numframes)
        I_Error(english_language ?
                "R_ProjectSprite: invalid sprite frame %i : %i " :
                "R_ProjectSprite: некорректный фрейм спрайта %i : %i ",
                thing->sprite, thing->frame);
#endif
    sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        ang = R_PointToAngle(interpx, interpy);
        rot = (ang - interpangle + (unsigned) (ANG45 / 2) * 9) >> 29;
        lump = sprframe->lump[rot];
        flip = (boolean) sprframe->flip[rot];
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = (boolean) sprframe->flip[0];
    }

    // [crispy] flip death sprites and corpses randomly
    if (randomly_flipcorpses && !vanillaparm)
    {
        // Not to be flipped
        if ((thing->flags & MF_CORPSE
        &&   thing->type != MT_MINOTAUR        // Centaur [107]
        &&   thing->type != MT_ETTIN           // Ettin [10030]
        &&   thing->type != MT_MAGE_BOSS       // Menelkir [10102]
        &&   thing->type != MT_CENTAURLEADER   // Slaughtaur [115]
        &&   thing->type != MT_CLERIC_BOSS     // Traductus [10101]
        &&   thing->type != MT_FIGHTER_BOSS)   // Zedek [10100]
        // Obstacles
        || thing->type == MT_ZBARREL           // Barrel [8100]
        || thing->type == MT_MISC70            // Big Rock (brown) [97]
        || thing->type == MT_MISC72            // Big Rock (gray) [99]
        || thing->type == MT_MISC7             // Dead Tree [24]
        || thing->type == MT_MISC32            // Dead Tree (mossy) [60]
// [JN] Not safe for flipping:
//      || thing->type == MT_ZXMAS_TREE        // Evergreen Tree (mossy) [8068]
        || thing->type == MT_MISC59            // Gnarled Tree 1 [80]
        || thing->type == MT_MISC60            // Gnarled Tree 2 [87]
        || thing->type == MT_MISC66            // Ice Spike (large) [93]
        || thing->type == MT_MISC67            // Ice Spike (medium) [94]
        || thing->type == MT_MISC68            // Ice Spike (small) [95]
        || thing->type == MT_MISC69            // Ice Spike (tiny) [96]
        || thing->type == MT_MISC62            // Icecle (large) [89]
        || thing->type == MT_MISC63            // Icecle (medium) [90]
        || thing->type == MT_MISC64            // Icecle (small) [91]
        || thing->type == MT_MISC65            // Icecle (tiny) [92]
        || thing->type == MT_MISC8             // Leafless Tree [25]
        || thing->type == MT_MISC61            // Log [88]
        || thing->type == MT_MISC9             // Mossy Tree 1 [26]
        || thing->type == MT_MISC10            // Mossy Tree 2 [27]
        || thing->type == MT_TREEDESTRUCTIBLE  // Rotten Dead Tree [8062]
        || thing->type == MT_MISC71            // Small Rock (brown) [98]
        || thing->type == MT_MISC27            // Stalactite (large) [52]
        || thing->type == MT_MISC28            // Stalactite (medium) [56]
        || thing->type == MT_MISC29            // Stalactite (small) [57]
        || thing->type == MT_MISC24            // Stalacmite (large) [49]
        || thing->type == MT_MISC25            // Stalacmite (medium) [50]
        || thing->type == MT_MISC23            // Stalacmite (pillar) [48]
        || thing->type == MT_MISC26            // Stalacmite (small) [51]
        || thing->type == MT_MISC57            // Tall Tree 1 [78]
        || thing->type == MT_MISC58            // Tall Tree 2 [79]
        || thing->type == MT_MISC12            // Tree Stump (bare) [29]
        || thing->type == MT_MISC11            // Tree Stump (burned) [28]
        || thing->type == MT_MISC13            // Tree Stump 1 [37]
        || thing->type == MT_MISC14            // Tree Stump 2 [38]
        // Light sources
        || thing->type == MT_MISC79            // 3 Candles [119]
        || thing->type == MT_ZBLUE_CANDLE      // Blue Candle [8066]
        || thing->type == MT_BRASSTORCH        // Blazier with Flame [8061]
        || thing->type == MT_TABLE_SHIT3       // Candle with Web [8502]
        || thing->type == MT_ZCAULDRON         // Cauldron (lit) [8069]
        || thing->type == MT_ZCAULDRON_UNLIT   // Cauldron (unlit) [8070]
        || thing->type == MT_MISC5             // Chandeiler (lit) [17]
        || thing->type == MT_MISC6             // Chandeiler (unlit) [8063]
        || thing->type == MT_FLAME_LARGE       // Large flame [10503]
        || thing->type == MT_FLAME_LARGE_TEMP  // Large flame (timed) [10502]
        || thing->type == MT_ZFIREBULL         // Minotaur Statue (lit) [8042]
        || thing->type == MT_ZFIREBULL_UNLIT   // Minotaur Statue (unlit) [8043]
        || thing->type == MT_FIRETHING         // Skull with Flame [8060]
        || thing->type == MT_TABLE_SHIT5       // Small Candle (unlit) [8504]
        || thing->type == MT_FLAME_SMALL       // Small Flame [10501]
        || thing->type == MT_FLAME_SMALL_TEMP  // Small Flame (timed) [10500]
        || thing->type == MT_TABLE_SHIT4       // Small Gray Candle (unlit) [8503]
        || thing->type == MT_ZTWINEDTORCH      // Twined Torch (lit) [116]
        || thing->type == MT_ZTWINEDTORCH_UNLIT // Twined Torch (unlit) [117]
        || thing->type == MT_ZWALLTORCH        // Wall Torch (lit) [54]
        || thing->type == MT_ZWALLTORCH_UNLIT  // Wall Torch (unlit) [55]
        // Decoration
        || thing->type == MT_MISC4             // Big Rock (mossy) [38]
        || thing->type == MT_MISC73            // Brick Rubble (large) [100]
        || thing->type == MT_MISC75            // Brick Rubble (medium) [102]
        || thing->type == MT_MISC74            // Brick Rubble (small) [101]
        || thing->type == MT_ZCHAINBIT64       // Chain (long) [8072]
        || thing->type == MT_ZCHAINBIT32       // Chain (short) [8071]
// [JN] Not safe for flipping:
//      || thing->type == MT_ZCHAINEND_HOOK1   // Chain with Large Hook [8074]
//      || thing->type == MT_ZCHAINEND_HOOK2   // Chain with Small Hook [8075]
//      || thing->type == MT_ZCHAINEND_SPIKE   // Chain with Spike Ball [8076]
        || thing->type == MT_MISC78            // Corpse (sitting) [110]
        || thing->type == MT_MISC34            // Corpse (sleeping) [62]
        || thing->type == MT_TABLE_SHIT9       // Goblet (silver) [8508]
        || thing->type == MT_TABLE_SHIT8       // Goblet (small) [8507]
        || thing->type == MT_TABLE_SHIT6       // Goblet (spilled) [8505]
        || thing->type == MT_TABLE_SHIT7       // Goblet (tall) [8506]
        || thing->type == MT_ZBUCKET           // Hanging Bucket [8103]
        || thing->type == MT_MISC42            // Hanging Corpse [71]
        || thing->type == MT_MISC30            // Hanging Moss 1 [58]
        || thing->type == MT_MISC31            // Hanging Moss 2 [59]
// [JN] Not safe for flipping:
//      || thing->type == MT_ZCHAINEND_HEART   // Hook with Heart [8073]
//      || thing->type == MT_ZCHAINEND_SKULL   // Hook with Skull [8077]
        || thing->type == MT_MISC33            // Impaled Corpse [61]
        || thing->type == MT_TABLE_SHIT1       // Large Brown Stein [8500]
        || thing->type == MT_ZPOISONSHROOM     // Large Mushroom (explodes) [8104]
        || thing->type == MT_MISC15            // Large Mushroom 1 [39]
        || thing->type == MT_MISC16            // Large Mushroom 2 [40]
        || thing->type == MT_MISC77            // Lynched Corpse [108]
        || thing->type == MT_ZLYNCHED_NOHEART  // Lynched Corpse (heartless) [109]
        || thing->type == MT_TABLE_SHIT10      // Meat Cleaver [8509]
        || thing->type == MT_MISC17            // Medium Mushroom [41]
        || thing->type == MT_MISC3             // Medium Rock (mossy) [9]
        || thing->type == MT_BLOODPOOL         // Pool of Blood [111]
        || thing->type == MT_POTTERY2          // Pot (medium) [105]
        || thing->type == MT_POTTERY3          // Pot (short) [106]
        || thing->type == MT_POTTERY1          // Pot (tall) [104]
// [JN] Not safe for flipping:
//      ||  mobj->type == MT_ZSHRUB1           // Shrub [8101]
//      ||  mobj->type == MT_ZSHRUB2           // Shrub 2 [8102]
        || thing->type == MT_MISC18            // Small Mushroom 1 [42]
        || thing->type == MT_MISC19            // Small Mushroom 2 [44]
        || thing->type == MT_MISC20            // Small Mushroom 3 [45]
        || thing->type == MT_MISC21            // Small Mushroom 4 [46]
        || thing->type == MT_MISC22            // Small Mushroom 5 [47]
        || thing->type == MT_MISC2             // Small Rock (mossy) [7]
        || thing->type == MT_TABLE_SHIT2       // Small Silver Stein [8501]
        || thing->type == MT_MISC1             // Tiny Rock (mossy) [8501]
        // Breakable objects
        || thing->type == MT_POTTERYBIT1       // Pot shards
        || thing->type == MT_SGSHARD1          // Stained glass shards
        || thing->type == MT_SGSHARD2
        || thing->type == MT_SGSHARD3
        || thing->type == MT_SGSHARD4
        || thing->type == MT_SGSHARD5
        || thing->type == MT_SGSHARD6
        || thing->type == MT_SGSHARD7
        || thing->type == MT_SGSHARD8
        || thing->type == MT_SGSHARD9
        || thing->type == MT_SGSHARD0
        // Spashes
        || thing->type == MT_SPLASHBASE        // Water splash base [-1]
        || thing->type == MT_SPLASH            // Water small splash [-1]
        || thing->type == MT_LAVASPLASH        // Lava splash base [-1]
        || thing->type == MT_LAVASMOKE         // Lava smoke [-1]
        || thing->type == MT_SLUDGESPLASH      // Sludge splash base [-1]
        || thing->type == MT_SLUDGECHUNK)      // Sludge small chunk [-1]
        {
            if (thing->health & 1)
            {
                flip = true;
            }
        }
    }

    // Calculate edges of the shape.
    tx -= spriteoffset[lump];
    x1 = (centerxfrac + FixedMul(tx, xscale)) >> FRACBITS;

    // Off the right side.
    if (x1 > viewwidth)
    {
        return;
    }

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FixedMul(tx, xscale)) >> FRACBITS) - 1;

    // Off the left side.
    if (x2 < 0)
    {
        return;
    }

    // [JN] killough 4/9/98: clip things which are out of view due to height
    gzt = interpz + spritetopoffset[lump];

    if (interpz > viewz + FixedDiv(viewheight << FRACBITS, xscale)
    ||  gzt     < (int64_t)viewz - FixedDiv((viewheight << (FRACBITS + 1))-viewheight, xscale))
    return;

    // [JN] Quickly reject sprites with bad x ranges.
    if (x1 >= x2)
    {
        return;
    }

    // Store information in a vissprite.
    vis = R_NewVisSprite();
    vis->mobjflags = thing->flags;
    vis->psprite = false;
    vis->scale = xscale << (detailshift && !hires);
    vis->gx = interpx;
    vis->gy = interpy;
    vis->gz = interpz;
    vis->gzt = gzt;  // [JN] killough 3/27/98

    // Color translation.
    if (thing->flags & MF_TRANSLATION)
    {
        if (thing->player)
        {
            vis->class = thing->player->class;
        }
        else
        {
            vis->class = thing->special1.i;
        }
        if (vis->class > 2)
        {
            vis->class = 0;
        }
    }

    // Foot clipping.
    vis->floorclip = thing->floorclip;
    vis->texturemid = gzt - viewz - vis->floorclip;
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth - 1 : x2;
    iscale = FixedDiv(FRACUNIT, xscale);

    if (flip)
    {
        vis->startfrac = spritewidth[lump] - 1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }
    if (vis->x1 > x1)
    {
        vis->startfrac += vis->xiscale * (vis->x1 - x1);
    }

    vis->patch = lump;

    // Get light level.
    if (fixedcolormap)
    {
        // Fixed map.
        vis->colormap = fixedcolormap;
    }
    else if (LevelUseFullBright && thing->frame & FF_FULLBRIGHT)
    {
        // Full bright.
        vis->colormap = colormaps;
    }
    else
    {
        // Diminished light.
        index = xscale >> (LIGHTSCALESHIFT - detailshift + hires);

        if (index >= MAXLIGHTSCALE)
        {
            index = MAXLIGHTSCALE - 1;
        }

        vis->colormap = spritelights[index];

        // [JN] Applying brightmaps to sprites,
        // but don't apply in foggy maps.
        if (brightmaps && !vanillaparm && LevelUseFullBright)
        {
            // - Red only -

            // Banishment Device
            if (thing->type == MT_TELEPORTOTHER) 
            vis->colormap = fullbrights_redonly[index];

            // Chaos Device
            if (thing->type == MT_ARTITELEPORT)
            vis->colormap = fullbrights_redonly[index];

            // Korax
            if (thing->type == MT_KORAX)
            vis->colormap = fullbrights_redonly[index];

            // Flame Mask
            if (thing->type == MT_ARTIPUZZSKULL2)
            vis->colormap = fullbrights_redonly[index];            

            // - Blue only -

            // Crater of Might
            if (thing->type == MT_SPEEDBOOTS)
            vis->colormap = fullbrights_blueonly[index];

            // Blue candle (lit)
            if (thing->type == MT_ZBLUE_CANDLE)
            vis->colormap = fullbrights_blueonly[index];

            // Wendigo
            if (thing->type == MT_ICEGUY)
            vis->colormap = fullbrights_blueonly[index];

            // Mystic Ambit Incant
            if (thing->type == MT_HEALRADIUS)
            vis->colormap = fullbrights_blueonly[index];

            // - Purple only -

            // Crater of Might
            if (thing->type == MT_BOOSTMANA)
            vis->colormap = fullbrights_purpleonly[index];

            // Dragonskin Bracers
            if (thing->type == MT_BOOSTARMOR)
            vis->colormap = fullbrights_purpleonly[index];

            // Icon of the Defender
            if (thing->type == MT_ARTIINVULNERABILITY)
            vis->colormap = fullbrights_purpleonly[index];

            // - Flame -

            // Torch (Artifact)
            if (thing->type == MT_ARTITORCH)
            vis->colormap = fullbrights_flame[index];

            // 3 candles (lit)
            if (thing->type == MT_BRASSTORCH)
            vis->colormap = fullbrights_flame[index];

            // Skull with Flame
            if (thing->type == MT_FIRETHING)
            vis->colormap = fullbrights_flame[index];

            // Twined Torch
            if (thing->type == MT_ZTWINEDTORCH || thing->type == MT_ZTWINEDTORCH_UNLIT)
            vis->colormap = fullbrights_flame[index];

            // Wall torch
            if (thing->type == MT_ZWALLTORCH || thing->type == MT_ZWALLTORCH_UNLIT)
            vis->colormap = fullbrights_flame[index];

            // Chandelier
            if (thing->type == MT_MISC5 || thing->type == MT_MISC6)
            vis->colormap = fullbrights_flame[index];

            // Cauldron
            if (thing->type == MT_ZCAULDRON)
            vis->colormap = fullbrights_flame[index];
        
            // - Fire Bull -
            
            if (thing->type == MT_ZFIREBULL || thing->type == MT_ZFIREBULL_UNLIT)
            vis->colormap = fullbrights_firebull[index];
        }
        
        // [JN] Fallback. If we are not using brightmaps, apply full brightness
        // to the objects, that no longer lighten up in info.c.
        if ((!brightmaps || vanillaparm) && LevelUseFullBright)
        {
            if (thing->type == MT_ARTITORCH
            ||  thing->type == MT_SPEEDBOOTS
            ||  thing->type == MT_BOOSTMANA
            ||  thing->type == MT_BOOSTARMOR
            ||  thing->type == MT_HEALRADIUS
            ||  thing->type == MT_ZTWINEDTORCH
            ||  thing->type == MT_ZWALLTORCH
            ||  thing->type == MT_ZFIREBULL
            ||  thing->type == MT_FIRETHING
            ||  thing->type == MT_BRASSTORCH
            ||  thing->type == MT_ZBLUE_CANDLE
            ||  thing->type == MT_ZCAULDRON)
            vis->colormap = colormaps;
        }
    }
}

/*
================================================================================
=
= R_AddSprites
=
= During BSP traversal, this adds sprites by sector.
=
================================================================================
*/

void R_AddSprites(sector_t *sec)
{
    mobj_t *thing;
    int lightnum;

    if (sec->validcount == validcount)
    {
        // Already added.
        return;
    }

    sec->validcount = validcount;

    lightnum = ((sec->lightlevel + level_brightness) >> LIGHTSEGSHIFT) + extralight;
    if (lightnum < 0)
    {
        spritelights = scalelight[0];

        // [JN] Brightmaps
        fullbrights_greenonly = fullbright_greenonly[0];
        fullbrights_redonly = fullbright_redonly[0];
        fullbrights_blueonly = fullbright_blueonly[0];
        fullbrights_purpleonly = fullbright_purpleonly[0];
        fullbrights_flame = fullbright_flame[0];
        fullbrights_yellowred = fullbright_yellowred[0];
        fullbrights_firebull = fullbright_firebull[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        spritelights = scalelight[LIGHTLEVELS - 1];

        // [JN] Brightmaps
        fullbrights_greenonly = fullbright_greenonly[LIGHTLEVELS - 1];
        fullbrights_redonly = fullbright_redonly[LIGHTLEVELS - 1];
        fullbrights_blueonly = fullbright_blueonly[LIGHTLEVELS - 1];
        fullbrights_purpleonly = fullbright_purpleonly[LIGHTLEVELS - 1];
        fullbrights_flame = fullbright_flame[LIGHTLEVELS - 1];
        fullbrights_yellowred = fullbright_yellowred[LIGHTLEVELS - 1];
        fullbrights_firebull = fullbright_firebull[LIGHTLEVELS - 1];
    }
    else
    {
        spritelights = scalelight[lightnum];

        // [JN] Brightmaps
        fullbrights_greenonly = fullbright_greenonly[lightnum];
        fullbrights_redonly = fullbright_redonly[lightnum];
        fullbrights_blueonly = fullbright_blueonly[lightnum];
        fullbrights_purpleonly = fullbright_purpleonly[lightnum];
        fullbrights_flame = fullbright_flame[lightnum];
        fullbrights_yellowred = fullbright_yellowred[lightnum];
        fullbrights_firebull = fullbright_firebull[lightnum];
    }

    for (thing = sec->thinglist; thing; thing = thing->snext)
    {
        R_ProjectSprite(thing);
    }
}

/*
================================================================================
=
= R_ApplyWeaponBob
=
= [crispy] apply bobbing (or centering) to the player's weapon sprite
=
================================================================================
*/

static inline void R_ApplyWeaponBob (fixed_t *sx, boolean bobx, fixed_t *sy, boolean boby)
{
    const angle_t angle = (128 * leveltime) & FINEMASK;

    if (sx)
    {
        *sx = FRACUNIT;

        if (bobx)
        {
            *sx += FixedMul(viewplayer->bob, finecosine[angle]);
        }
    }

    if (sy)
    {
        *sy = 32 * FRACUNIT; // [crispy] WEAPONTOP

        if (boby)
        {
            *sy += FixedMul(viewplayer->bob, finesine[angle & (FINEANGLES / 2 - 1)]);
        }
    }
}

/*
================================================================================
=
= R_DrawPSprite
=
================================================================================
*/

// Y-adjustment values for full screen (4 weapons)
int PSpriteSY[NUMCLASSES][NUMWEAPONS] = {
    {0, -12 * FRACUNIT, -10 * FRACUNIT, 10 * FRACUNIT}, // Fighter
    {-8 * FRACUNIT, 10 * FRACUNIT, 10 * FRACUNIT, 0},   // Cleric
    {9 * FRACUNIT, 20 * FRACUNIT, 20 * FRACUNIT, 20 * FRACUNIT},        // Mage 
    {10 * FRACUNIT, 10 * FRACUNIT, 10 * FRACUNIT, 10 * FRACUNIT}        // Pig
};

void R_DrawPSprite(pspdef_t * psp)
{
    fixed_t tx;
    int x1, x2;
    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int lump;
    boolean flip;
    vissprite_t *vis, avis;

    int tempangle;
    const int state = viewplayer->psprites[ps_weapon].state - states;

//
// decide which patch to use
//
#ifdef RANGECHECK
    if ((unsigned) psp->state->sprite >= numsprites)
        I_Error(english_language ?
                "R_ProjectSprite: invalid sprite number %i " :
                "R_ProjectSprite: некорректный номер спрайта %i ",
                psp->state->sprite);
#endif
    sprdef = &sprites[psp->state->sprite];
#ifdef RANGECHECK
    if ((psp->state->frame & FF_FRAMEMASK) >= sprdef->numframes)
        I_Error(english_language ?
                "R_ProjectSprite: invalid sprite frame %i : %i " :
                "R_ProjectSprite: некорректный фрейм спрайта %i : %i ",
                psp->state->sprite, psp->state->frame);
#endif
    sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0] ^ flip_levels ^ flip_weapons;

    // [JN] Smoothern Serpent Staff ready state bobbing.
    if (singleplayer && !vanillaparm)
    {
        if (state == S_CSTAFFREADY1  ||  state == S_CSTAFFREADY2
        ||  state == S_CSTAFFREADY3  ||  state == S_CSTAFFREADY4
        ||  state == S_CSTAFFREADY5  ||  state == S_CSTAFFREADY6
        ||  state == S_CSTAFFREADY7  ||  state == S_CSTAFFREADY8
        ||  state == S_CSTAFFREADY9  ||  state == S_CSTAFFBLINK1
        ||  state == S_CSTAFFBLINK2  ||  state == S_CSTAFFBLINK3
        ||  state == S_CSTAFFBLINK4  ||  state == S_CSTAFFBLINK5
        ||  state == S_CSTAFFBLINK6  ||  state == S_CSTAFFBLINK7
        ||  state == S_CSTAFFBLINK8  ||  state == S_CSTAFFBLINK9
        ||  state == S_CSTAFFBLINK10 ||  state == S_CSTAFFBLINK11)
        {
            R_ApplyWeaponBob(&psp->sx, true, &psp->sy, true);
        }
    }

//
// calculate edges of the shape
//
    tx = psp->sx - 160 * FRACUNIT;

    // [crispy] fix sprite offsets for mirrored sprites
    tx -= flip ? 2 * tx - spriteoffset[lump] + spritewidth[lump] : spriteoffset[lump];
    if (viewangleoffset)
    {
        tempangle =
            ((centerxfrac / 1024) * (viewangleoffset >> ANGLETOFINESHIFT));
    }
    else
    {
        tempangle = 0;
    }
    x1 = (centerxfrac + FixedMul(tx, pspritescale) + tempangle) >> FRACBITS;
    if (x1 > viewwidth)
        return;                 // off the right side
    tx += spritewidth[lump];
    x2 = ((centerxfrac + FixedMul(tx, pspritescale) +
           tempangle) >> FRACBITS) - 1;
    if (x2 < 0)
        return;                 // off the left side

//
// store information in a vissprite
//
    vis = &avis;
    vis->mobjflags = 0;
    vis->class = 0;
    vis->psprite = true;
    // [crispy] weapons drawn 1 pixel too high when player is idle
    vis->texturemid = (BASEYCENTER<<FRACBITS)+FRACUNIT/4-(psp->sy-spritetopoffset[lump]);

    if (screenblocks >= 11)
    {
        vis->texturemid -= PSpriteSY[viewplayer->class]
            [players[consoleplayer].readyweapon];
    }
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth - 1 : x2;
    vis->scale = pspritescale << (detailshift && !hires);
    if (flip)
    {
        vis->xiscale = -pspriteiscale;
        vis->startfrac = spritewidth[lump] - 1;
    }
    else
    {
        vis->xiscale = pspriteiscale;
        vis->startfrac = 0;
    }
    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale * (vis->x1 - x1);
    vis->patch = lump;

    if (viewplayer->powers[pw_invulnerability] && viewplayer->class
        == PCLASS_CLERIC)
    {
        vis->colormap = spritelights[MAXLIGHTSCALE - 1];
        if (viewplayer->powers[pw_invulnerability] > 4 * 32)
        {
            if (viewplayer->mo->flags2 & MF2_DONTDRAW)
            {                   // don't draw the psprite
                vis->mobjflags |= MF_SHADOW;
            }
            else if (viewplayer->mo->flags & MF_SHADOW)
            {
                vis->mobjflags |= MF_ALTSHADOW;
            }
        }
        else if (viewplayer->powers[pw_invulnerability] & 8)
        {
            vis->mobjflags |= MF_SHADOW;
        }
    }
    else if (fixedcolormap)
    {
        // Fixed color
        vis->colormap = fixedcolormap;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
        // Full bright
        vis->colormap = colormaps;
    }
    else
    {
        // local light
        vis->colormap = spritelights[MAXLIGHTSCALE - 1];
    }

    // [JN] e6y: interpolation for weapon bobbing
    if (uncapped_fps && !vanillaparm)
    {
        typedef struct interpolate_s
        {
            int x1;
            int x1_prev;
            int texturemid;
            int texturemid_prev;
            int lump;
        } psp_interpolate_t;

        static psp_interpolate_t psp_inter;

        if (realframe && !skippsprinterp)
        {
            psp_inter.x1 = psp_inter.x1_prev;
            psp_inter.texturemid = psp_inter.texturemid_prev;
        }

        psp_inter.x1_prev = vis->x1;
        psp_inter.texturemid_prev = vis->texturemid;

        if (lump == psp_inter.lump && !skippsprinterp)
        {
            int deltax = vis->x2 - vis->x1;

            vis->x1 = psp_inter.x1 + FixedMul(fractionaltic, vis->x1 - psp_inter.x1);
            vis->x2 = vis->x1 + deltax;
            vis->texturemid = psp_inter.texturemid
                            + FixedMul(fractionaltic, vis->texturemid - psp_inter.texturemid);
        }
        else
        {
            psp_inter.x1 = vis->x1;
            psp_inter.texturemid = vis->texturemid;
            psp_inter.lump = lump;
            skippsprinterp = false;
        }
    }

    R_DrawVisSprite(vis, vis->x1, vis->x2);
}

/*
================================================================================
=
= R_DrawPlayerSprites
=
================================================================================
*/

void R_DrawPlayerSprites(void)
{
    int i, lightnum;
    pspdef_t *psp;
    const int state = viewplayer->psprites[ps_weapon].state - states; // [from-crispy] We need to define what "state" actually is

//
// get light level
//
    lightnum =
        ((viewplayer->mo->subsector->sector->lightlevel + level_brightness) >> LIGHTSEGSHIFT) +
        extralight;
    if (lightnum < 0)
    {
        spritelights = scalelight[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        spritelights = scalelight[LIGHTLEVELS - 1];
    }
    else
    {
        spritelights = scalelight[lightnum];
        
        // [JN] Applying brightmaps to HUD weapons...
        if (brightmaps && !vanillaparm)
        {
            // Fighter: Axe
            if (state == S_FAXEREADY_G || state == S_FAXEREADY_G1 || state == S_FAXEREADY_G2 || state == S_FAXEREADY_G3 || state == S_FAXEREADY_G4 || state == S_FAXEREADY_G5 || state == S_FAXEDOWN_G || state == S_FAXEUP_G || state == S_FAXEUP_G || state == S_FAXEATK_G1 || state == S_FAXEATK_G2 || state == S_FAXEATK_G3 || state == S_FAXEATK_G4 || state == S_FAXEATK_G5 || state == S_FAXEATK_G6 || state == S_FAXEATK_G7 || state == S_FAXEATK_G8 || state == S_FAXEATK_G9 || state == S_FAXEATK_G10 || state == S_FAXEATK_G11 || state == S_FAXEATK_G12 || state == S_FAXEATK_G13)
                spritelights = fullbright_blueonly[lightnum];
            // Fighter: Sword
            else if (state == S_FSWORDREADY || state == S_FSWORDREADY1 || state == S_FSWORDREADY2 || state == S_FSWORDREADY3 || state == S_FSWORDREADY4 || state == S_FSWORDREADY5 || state == S_FSWORDREADY6 || state == S_FSWORDREADY7 || state == S_FSWORDREADY8 || state == S_FSWORDREADY9 || state == S_FSWORDREADY10 || state == S_FSWORDREADY11 || state == S_FSWORDDOWN || state == S_FSWORDUP || state == S_FSWORDATK_1 || state == S_FSWORDATK_2 || state == S_FSWORDATK_3 || state == S_FSWORDATK_4 || state == S_FSWORDATK_5 || state == S_FSWORDATK_6 || state == S_FSWORDATK_7 || state == S_FSWORDATK_8 || state == S_FSWORDATK_9 || state == S_FSWORDATK_10 || state == S_FSWORDATK_11 || state == S_FSWORDATK_12)
                spritelights = fullbright_greenonly[lightnum];
            // Cleric: Serpent Staff
            else if (state == S_CSTAFFATK_1 || state == S_CSTAFFATK_2 || state == S_CSTAFFATK_3 || state == S_CSTAFFATK_4 || state == S_CSTAFFATK2_1)
                spritelights = fullbright_greenonly[lightnum];
            // Cleric: Flame
            else if (state == S_CFLAMEDOWN || state == S_CFLAMEUP || state == S_CFLAMEREADY1 || state == S_CFLAMEREADY2 || state == S_CFLAMEREADY3 || state == S_CFLAMEREADY4 || state == S_CFLAMEREADY5 || state == S_CFLAMEREADY6 || state == S_CFLAMEREADY7 || state == S_CFLAMEREADY8 || state == S_CFLAMEREADY9 || state == S_CFLAMEREADY10 || state == S_CFLAMEREADY11 || state == S_CFLAMEREADY12 || state == S_CFLAMEATK_1 || state == S_CFLAMEATK_2 || state == S_CFLAMEATK_3 || state == S_CFLAMEATK_7 || state == S_CFLAMEATK_8)
                spritelights = fullbright_yellowred[lightnum];
            // Mage: Frost
            else if (state == S_CONEATK1_2 || state == S_CONEATK1_3 || state == S_CONEATK1_4 || state == S_CONEATK1_5 || state == S_CONEATK1_6 || state == S_CONEATK1_7 || state == S_CONEATK1_8)
                spritelights = fullbright_blueonly[lightnum];
            // Mage: Lightning
            else if (state == S_MLIGHTNINGREADY || state == S_MLIGHTNINGREADY2 || state == S_MLIGHTNINGREADY3 || state == S_MLIGHTNINGREADY4 || state == S_MLIGHTNINGREADY5 || state == S_MLIGHTNINGREADY6 || state == S_MLIGHTNINGREADY7 || state == S_MLIGHTNINGREADY8 || state == S_MLIGHTNINGREADY9 || state == S_MLIGHTNINGREADY10 || state == S_MLIGHTNINGREADY11 || state == S_MLIGHTNINGREADY12 || state == S_MLIGHTNINGREADY13 || state == S_MLIGHTNINGREADY14 || state == S_MLIGHTNINGREADY15 || state == S_MLIGHTNINGREADY16 || state == S_MLIGHTNINGREADY17 || state == S_MLIGHTNINGREADY18 || state == S_MLIGHTNINGREADY19 || state == S_MLIGHTNINGREADY20 || state == S_MLIGHTNINGREADY21 || state == S_MLIGHTNINGREADY22 || state == S_MLIGHTNINGREADY23 || state == S_MLIGHTNINGREADY24 || state == S_MLIGHTNINGDOWN || state == S_MLIGHTNINGUP || state == S_MLIGHTNINGATK_1 || state == S_MLIGHTNINGATK_2 || state == S_MLIGHTNINGATK_3 || state == S_MLIGHTNINGATK_4 || state == S_MLIGHTNINGATK_5 || state == S_MLIGHTNINGATK_6 || state == S_MLIGHTNINGATK_7 || state == S_MLIGHTNINGATK_8 || state == S_MLIGHTNINGATK_9 || state == S_MLIGHTNINGATK_10 || state == S_MLIGHTNINGATK_11)
                spritelights = fullbright_blueonly[lightnum];
            // Mage: Arc
            else if (state == S_MSTAFFREADY || state == S_MSTAFFREADY2 || state == S_MSTAFFREADY3 || state == S_MSTAFFREADY4 || state == S_MSTAFFREADY5 || state == S_MSTAFFREADY6 || state == S_MSTAFFREADY7 || state == S_MSTAFFREADY8 || state == S_MSTAFFREADY9 || state == S_MSTAFFREADY10 || state == S_MSTAFFREADY11 || state == S_MSTAFFREADY12 || state == S_MSTAFFREADY13 || state == S_MSTAFFREADY14 || state == S_MSTAFFREADY15 || state == S_MSTAFFREADY16 || state == S_MSTAFFREADY17 || state == S_MSTAFFREADY18 || state == S_MSTAFFREADY19 || state == S_MSTAFFREADY20 || state == S_MSTAFFREADY21 || state == S_MSTAFFREADY22 || state == S_MSTAFFREADY23 || state == S_MSTAFFREADY24 || state == S_MSTAFFREADY25 || state == S_MSTAFFREADY26 || state == S_MSTAFFREADY27 || state == S_MSTAFFREADY28 || state == S_MSTAFFREADY29 || state == S_MSTAFFREADY30 || state == S_MSTAFFREADY31 || state == S_MSTAFFREADY32 || state == S_MSTAFFREADY33 || state == S_MSTAFFREADY34 || state == S_MSTAFFREADY35 || state == S_MSTAFFDOWN || state == S_MSTAFFUP || state == S_MSTAFFATK_1 || state == S_MSTAFFATK_4 || state == S_MSTAFFATK_5 || state == S_MSTAFFATK_6 || state == S_MSTAFFATK_7)
                spritelights = fullbright_yellowred[lightnum];
        }
        // [JN] Fallback. If we are not using brightmaps, apply full brightness
        // to the objects, that no longer lighten up in info.c.
        if (!brightmaps || vanillaparm)
        {
            if (state == S_FSWORDREADY || state == S_FSWORDREADY1 || state == S_FSWORDREADY2 || state == S_FSWORDREADY3 || state == S_FSWORDREADY4 || state == S_FSWORDREADY5 || state == S_FSWORDREADY6 || state == S_FSWORDREADY7 || state == S_FSWORDREADY8 || state == S_FSWORDREADY9 || state == S_FSWORDREADY10 || state == S_FSWORDREADY11 || state == S_FSWORDDOWN || state == S_FSWORDUP || state == S_FSWORDATK_1 || state == S_FSWORDATK_2 || state == S_FSWORDATK_3 || state == S_FSWORDATK_4 || state == S_FSWORDATK_5 || state == S_FSWORDATK_6 || state == S_FSWORDATK_7 || state == S_FSWORDATK_8 || state == S_FSWORDATK_9 || state == S_FSWORDATK_10 || state == S_FSWORDATK_11 || state == S_FSWORDATK_12 || state == S_MLIGHTNINGREADY || state == S_MLIGHTNINGREADY2 || state == S_MLIGHTNINGREADY3 || state == S_MLIGHTNINGREADY4 || state == S_MLIGHTNINGREADY5 || state == S_MLIGHTNINGREADY6 || state == S_MLIGHTNINGREADY7 || state == S_MLIGHTNINGREADY8 || state == S_MLIGHTNINGREADY9 || state == S_MLIGHTNINGREADY10 || state == S_MLIGHTNINGREADY11 || state == S_MLIGHTNINGREADY12 || state == S_MLIGHTNINGREADY13 || state == S_MLIGHTNINGREADY14 || state == S_MLIGHTNINGREADY15 || state == S_MLIGHTNINGREADY16 || state == S_MLIGHTNINGREADY17 || state == S_MLIGHTNINGREADY18 || state == S_MLIGHTNINGREADY19 || state == S_MLIGHTNINGREADY20 || state == S_MLIGHTNINGREADY21 || state == S_MLIGHTNINGREADY22 || state == S_MLIGHTNINGREADY23 || state == S_MLIGHTNINGREADY24 || state == S_MLIGHTNINGDOWN || state == S_MLIGHTNINGUP || state == S_MLIGHTNINGATK_1 || state == S_MLIGHTNINGATK_2 || state == S_MLIGHTNINGATK_3 || state == S_MLIGHTNINGATK_4 || state == S_MLIGHTNINGATK_5 || state == S_MLIGHTNINGATK_6 || state == S_MLIGHTNINGATK_7 || state == S_MLIGHTNINGATK_8 || state == S_MLIGHTNINGATK_9 || state == S_MLIGHTNINGATK_10 || state == S_MLIGHTNINGATK_11)
                spritelights = scalelight[LIGHTLEVELS - 1];
        }
    }
//
// clip to screen bounds
//
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

//
// add all active psprites
//
    for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
        if (psp->state)
            R_DrawPSprite(psp);

}

/*
================================================================================
=
= R_SortVisSprites
=
= Rewritten by Lee Killough to avoid using unnecessary
= linked lists, and to use faster sorting algorithm.
================================================================================
*/

#define bcopyp(d, s, n) memcpy(d, s, (n) * sizeof(void *))

// killough 9/2/98: merge sort

static void msort(vissprite_t **s, vissprite_t **t, int n)
{
    if (n >= 16)
    {
        int n1 = n/2, n2 = n - n1;
        vissprite_t **s1 = s, **s2 = s + n1, **d = t;

        msort(s1, t, n1);
        msort(s2, t, n2);

        while ((*s1)->scale > (*s2)->scale ?
              (*d++ = *s1++, --n1) : (*d++ = *s2++, --n2));

        if (n2)
        bcopyp(d, s2, n2);
        else
        bcopyp(d, s1, n1);

        bcopyp(s, t, n);
    }
    else
    {
        int i;

        for (i = 1; i < n; i++)
        {
            vissprite_t *temp = s[i];

            if (s[i-1]->scale < temp->scale)
            {
                int j = i;

                while ((s[j] = s[j-1])->scale < temp->scale && --j);
                s[j] = temp;
            }
        }
    }
}

/*
================================================================================
=
= R_SortVisSprites
=
================================================================================
*/

void R_SortVisSprites (void)
{
    if (num_vissprite)
    {
        int i = num_vissprite;

        // If we need to allocate more pointers for the vissprites,
        // allocate as many as were allocated for sprites -- killough
        // killough 9/22/98: allocate twice as many

        if (num_vissprite_ptrs < num_vissprite*2)
        {
            free(vissprite_ptrs);  // better than realloc -- no preserving needed
            vissprite_ptrs = malloc((num_vissprite_ptrs = num_vissprite_alloc*2)
                                    * sizeof *vissprite_ptrs);
        }

        while (--i>=0)
        vissprite_ptrs[i] = vissprites+i;

        // killough 9/22/98: replace qsort with merge sort, since the keys
        // are roughly in order to begin with, due to BSP rendering.

        msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

/*
================================================================================
=
= R_DrawSprite
=
================================================================================
*/

void R_DrawSprite (vissprite_t *spr)
{
    drawseg_t *ds;
    int clipbot[WIDESCREENWIDTH]; // [crispy] 32-bit integer math
    int cliptop[WIDESCREENWIDTH]; // [crispy] 32-bit integer math
    int x, r1, r2;
    fixed_t scale, lowscale;

    for (x = spr->x1 ; x<=spr->x2 ; x++)
    clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.

    // [JN] e6y: optimization
    if (drawsegs_xrange_size)
    {
        const drawseg_xrange_item_t *last = &drawsegs_xrange[drawsegs_xrange_count - 1];

        drawseg_xrange_item_t *curr = &drawsegs_xrange[-1];

        while (++curr <= last)
        {
            // determine if the drawseg obscures the sprite
            if (curr->x1 > spr->x2 || curr->x2 < spr->x1)
            {
                // does not cover sprite
                continue;
            }

            ds = curr->user;

            if (ds->scale1 > ds->scale2)
            {
                lowscale = ds->scale2;
                scale = ds->scale1;
            }
            else
            {
                lowscale = ds->scale1;
                scale = ds->scale2;
            }
    
            if (scale < spr->scale || (lowscale < spr->scale
            && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
            {
                // masked mid texture?
                if (ds->maskedtexturecol)
                {
                    r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
                    r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;
                    R_RenderMaskedSegRange(ds, r1, r2);
                }
                // seg is behind sprite
                continue;
            }

            r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
            r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

            // clip this piece of the sprite
            // [JN] killough 3/27/98: optimized and made much shorter

            if (ds->silhouette&SIL_BOTTOM && spr->gz < ds->bsilheight) //bottom sil
                for (x=r1 ; x<=r2 ; x++)
                    if (clipbot[x] == -2)
                        clipbot[x] = ds->sprbottomclip[x];

            if (ds->silhouette&SIL_TOP && spr->gzt > ds->tsilheight)   // top sil
                for (x=r1 ; x<=r2 ; x++)
                    if (cliptop[x] == -2)
                        cliptop[x] = ds->sprtopclip[x];
        }
    }
    else
    {
        for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
        {   
            // determine if the drawseg obscures the sprite
            if (ds->x1 > spr->x2 || ds->x2 < spr->x1
            || (!ds->silhouette && !ds->maskedtexturecol))
            {
                // does not cover sprite
                continue;
            }

            r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
            r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

            if (ds->scale1 > ds->scale2)
            {
                lowscale = ds->scale2;
                scale = ds->scale1;
            }
            else
            {
                lowscale = ds->scale1;
                scale = ds->scale2;
            }

            if (scale < spr->scale || (lowscale < spr->scale
            && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
            {
                // masked mid texture?
                if (ds->maskedtexturecol)
                {
                    R_RenderMaskedSegRange(ds, r1, r2);
                }
                // seg is behind sprite
                continue;
            }

            // clip this piece of the sprite
            // [JN] killough 3/27/98: optimized and made much shorter

            if (ds->silhouette&SIL_BOTTOM && spr->gz < ds->bsilheight)  // bottom sil
                for (x=r1 ; x<=r2 ; x++)
                    if (clipbot[x] == -2)
                        clipbot[x] = ds->sprbottomclip[x];

            if (ds->silhouette&SIL_TOP && spr->gzt > ds->tsilheight)    // top sil
                for (x=r1 ; x<=r2 ; x++)
                    if (cliptop[x] == -2)
                        cliptop[x] = ds->sprtopclip[x];
        }
    }

    // all clipping has been performed, so draw the sprite
    // check for unclipped columns
    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
        if (clipbot[x] == -2)		
            clipbot[x] = viewheight;

        if (cliptop[x] == -2)
            cliptop[x] = -1;
    }

    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}

/*
================================================================================
=
= R_DrawMasked
=
================================================================================
*/

void R_DrawMasked (void)
{
    int i;
    drawseg_t *ds;
    int cx = screenwidth / 2;

    R_SortVisSprites();

    // [JN] e6y
    // Makes sense for scenes with huge amount of drawsegs.
    for(i = 0 ; i < DS_RANGES_COUNT ; i++)
    {
        drawsegs_xranges[i].count = 0;
    }

    if (num_vissprite > 0)
    {
        if (drawsegs_xrange_size < maxdrawsegs)
        {
            drawsegs_xrange_size = 2 * maxdrawsegs;

            for(i = 0; i < DS_RANGES_COUNT; i++)
            {
                drawsegs_xranges[i].items = I_Realloc(
                drawsegs_xranges[i].items,
                drawsegs_xrange_size * sizeof(drawsegs_xranges[i].items[0]));
            }
        }

        for (ds = ds_p; ds-- > drawsegs;)
        {
            if (ds->silhouette || ds->maskedtexturecol)
            {
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].x1 = ds->x1;
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].x2 = ds->x2;
                drawsegs_xranges[0].items[drawsegs_xranges[0].count].user = ds;

                if (ds->x1 < cx)
                {
                    drawsegs_xranges[1].items[drawsegs_xranges[1].count] = 
                    drawsegs_xranges[0].items[drawsegs_xranges[0].count];
                    drawsegs_xranges[1].count++;
                }
                if (ds->x2 >= cx)
                {
                    drawsegs_xranges[2].items[drawsegs_xranges[2].count] = 
                    drawsegs_xranges[0].items[drawsegs_xranges[0].count];
                    drawsegs_xranges[2].count++;
                }

                drawsegs_xranges[0].count++;
            }
        }
    }

    // Draw all vissprites back to front.
    for (i = num_vissprite ; --i>=0 ; )
    {
        vissprite_t* spr = vissprite_ptrs[i];

        if (spr->x2 < cx)
        {
            drawsegs_xrange = drawsegs_xranges[1].items;
            drawsegs_xrange_count = drawsegs_xranges[1].count;
        }
        else if (spr->x1 >= cx)
        {
            drawsegs_xrange = drawsegs_xranges[2].items;
            drawsegs_xrange_count = drawsegs_xranges[2].count;
        }
        else
        {
            drawsegs_xrange = drawsegs_xranges[0].items;
            drawsegs_xrange_count = drawsegs_xranges[0].count;
        }

        R_DrawSprite(vissprite_ptrs[i]);
    }

    // Render any remaining masked mid textures.
    for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);


    // Draw the psprites on top of everything, but does not draw on side views.
    if (!viewangleoffset)
    {
        R_DrawPlayerSprites ();
    }
}
