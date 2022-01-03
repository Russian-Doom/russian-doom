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
#include "doomdef.h"
#include "deh_str.h"
#include "i_swap.h"
#include "i_system.h"
#include "r_local.h"
#include "v_trans.h"
#include "jn.h"


// Sprite rotation 0 is facing the viewer, rotation 1 is one angle turn CLOCKWISE
// around the axis. This is not the same as the angle, which increases counter
// clockwise (protractor).  There was a lot of stuff grabbed wrong, so I changed it...
fixed_t pspritescale, pspriteiscale;

// [JN] Light tables and brightmaps.
static lighttable_t **spritelights;
static lighttable_t **fullbrights_greenonly;
static lighttable_t **fullbrights_redonly;
static lighttable_t **fullbrights_blueonly;
static lighttable_t **fullbrights_purpleonly;
static lighttable_t **fullbrights_notbronze;
static lighttable_t **fullbrights_flame;
static lighttable_t **fullbrights_greenonly_dim;
static lighttable_t **fullbrights_redonly_dim;
static lighttable_t **fullbrights_blueonly_dim;
static lighttable_t **fullbrights_yellowonly_dim;
static lighttable_t **fullbrights_ethereal;

// Constant arrays used for psprite clipping and initializing clipping.
int *negonearray;           // [JN] killough 2/8/98: // dropoff overflow
int *screenheightarray;     //      change to MAX_*  // dropoff overflow
static int *clipbot = NULL; // [JN] killough 2/8/98: // dropoff overflow
static int *cliptop = NULL; //      change to MAX_*  // dropoff overflow

// variables used to look up and range check thing_t sprites patches
int            numsprites;
int            maxframe;
char          *spritename;
spritedef_t   *sprites;
spriteframe_t  sprtemp[26];

// Initialization functions.
int *mfloorclip, *mceilingclip;  // [crispy] 32-bit integer math

fixed_t spryscale;
int64_t sprtopscreen, sprbotscreen;  // [crispy] WiggleFix

static size_t num_vissprite, num_vissprite_alloc, num_vissprite_ptrs; // [JN] killough
static vissprite_t *vissprites, **vissprite_ptrs;                     // [JN] killough


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

#define DS_RANGES_COUNT 3
static drawsegs_xrange_t drawsegs_xranges[DS_RANGES_COUNT];

static drawseg_xrange_item_t *drawsegs_xrange;
static unsigned int drawsegs_xrange_size = 0;
static int drawsegs_xrange_count = 0;


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
    if (negonearray)
    {
        free(negonearray);
    }
    if (screenheightarray)
    {
        free(screenheightarray);
    }

    xtoviewangle = calloc(1, (screenwidth + 1) * sizeof(*xtoviewangle));
    linearskyangle = calloc(1, (screenwidth + 1) * sizeof(*linearskyangle));
    negonearray = calloc(1, screenwidth * sizeof(*negonearray));
    screenheightarray = calloc(1, screenwidth * sizeof(*screenheightarray));

    if (clipbot)
    {
        free(clipbot);
    }

    clipbot = calloc(1, 2 * screenwidth * sizeof(*clipbot));
    cliptop = clipbot + screenwidth;
}

/*
=================
=
= R_InstallSpriteLump
=
= Local function for R_InitSprites
=================
*/

void R_InstallSpriteLump(int lump, unsigned frame, unsigned rotation, boolean flipped)
{
    int r;

    if (frame >= 26 || rotation > 8)
    {
        I_Error(english_language ?
                "R_InstallSpriteLump: bad frame characters in lump %i" :
                "R_InstallSpriteLump: некорректные символы фрейма в блоке %i",
                lump);
    }

    if ((int) frame > maxframe)
    {
        maxframe = frame;
    }

    if (rotation == 0)
    {
        // the lump should be used for all rotations
        if (sprtemp[frame].rotate == false)
        {
            I_Error (english_language ?
                    "R_InitSprites: sprite %s frame %c has multip rot=0 lump" :
                    "R_InitSprites: фрейм %c спрайта %s имеет многократный блок rot=0",
                    spritename, 'A' + frame);
        }
        if (sprtemp[frame].rotate == true)
        {
            I_Error (english_language ?
                "R_InitSprites: sprite %s frame %c has rotations and a rot=0 lump" :
                "R_InitSprites: фрейм %c спрайта %s имеет фреймы поворота и блок rot=0",
                 spritename, 'A' + frame);
        }

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
    {
        I_Error (english_language ?
                 "R_InitSprites: sprite %s frame %c has rotations and a rot=0 lump" :
                 "R_InitSprites: фрейм спрайта %c спрайта %s имеет фреймы поворота и блок rot=0",
                 spritename, 'A' + frame);
    }

    sprtemp[frame].rotate = true;
    rotation--;  // make 0 based

    if (sprtemp[frame].lump[rotation] != -1)
    {
        I_Error (english_language ?
                 "R_InitSprites: sprite %s : %c : %c has two lumps mapped to it" :
                 "R_InitSprites: спрайу %s : %c : %c назначено несколько одинаковых блоков",            
                 spritename, 'A' + frame, '1' + rotation);
    }

    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip[rotation] = (byte) flipped;
}

/*
================================================================================
=
= R_InitSpriteDefs
=
= Pass a null terminated list of sprite names (4 chars exactly) to be used
= Builds the sprite rotation matrixes to account for horizontally flipped
= sprites.  Will report an error if the lumps are inconsistant
= Only called at startup.
=
= Sprite lump names are 4 characters for the actor, a letter for the frame,
= and a number for the rotation, A sprite that is flippable will have an
= additional letter/number appended.  The rotation character can be 0 to
= signify no rotations.
================================================================================
*/

void R_InitSpriteDefs(char **namelist)
{
    char **check;
    int i, l, frame, rotation;
    int start, end;

    // count the number of sprite names
    check = namelist;
    while (*check != NULL)
    {
        check++;
    }

    numsprites = check - namelist;

    if (!numsprites)
    {
        return;
    }

    sprites = Z_Malloc(numsprites * sizeof(*sprites), PU_STATIC, NULL);

    start = firstspritelump - 1;
    end = lastspritelump + 1;

    // Scan all the lump names for each of the names, noting the highest
    // frame letter. Just compare 4 characters as ints.
    for (i = 0; i < numsprites; i++)
    {
        spritename = DEH_String(namelist[i]);
        memset(sprtemp, -1, sizeof(sprtemp));

        maxframe = -1;

        //
        // scan the lumps, filling in the frames for whatever is found
        //
        for (l = start + 1; l < end; l++)
            if (!strncasecmp(lumpinfo[l]->name, spritename, 4))
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
            sprites[i].numframes = 0;
            if (gamemode == shareware)
            {
                continue;
            }
            I_Error(english_language ?
                    "R_InitSprites: no lumps found for sprite %s" :
                    "R_InitSprites: не найдены блоки в спрайте %s",
                    spritename);
        }

        maxframe++;

        for (frame = 0; frame < maxframe; frame++)
        {
            switch ((int) sprtemp[frame].rotate)
            {
                case -1:       // no rotations were found for that frame at all
                    I_Error(english_language ?
                            "R_InitSprites: no patches found for %s frame %c" :
                            "R_InitSprites: не найдены патчи для спрайта %s, фрейма %c",
                            spritename, frame + 'A');
                case 0:        // only the first rotation is needed
                    break;

                case 1:        // must have all 8 frames
                    for (rotation = 0; rotation < 8; rotation++)
                        if (sprtemp[frame].lump[rotation] == -1)
                            I_Error(english_language ?
                                    "R_InitSprites: sprite %s frame %c is missing rotations" :
                                    "R_InitSprites: в фрейме %c спрайта %s отсутствует информация о вращении",
                                    spritename, frame + 'A');
            }
        }

        //
        // allocate space for the frames present and copy sprtemp to it
        //
        sprites[i].numframes = maxframe;
        sprites[i].spriteframes = Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
        memcpy(sprites[i].spriteframes, sprtemp, maxframe * sizeof(spriteframe_t));
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

void R_InitSprites (char **namelist)
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

void R_ClearSprites (void)
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

vissprite_t *R_NewVisSprite (void)
{
    if (num_vissprite >= num_vissprite_alloc)   // [JN] killough
    {
        size_t num_vissprite_alloc_prev = num_vissprite_alloc;

        num_vissprite_alloc = num_vissprite_alloc ? num_vissprite_alloc*2 : 128;
        vissprites = I_Realloc(vissprites,num_vissprite_alloc*sizeof(*vissprites));

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
        {
            dc_yh = mfloorclip[dc_x]-1;
        }
        if (dc_yl <= mceilingclip[dc_x])
        {
            dc_yl = mceilingclip[dc_x]+1;
        }
        if (dc_yh >= baseclip && baseclip != -1)
        {
            dc_yh = baseclip;
        }

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

void R_DrawVisSprite (vissprite_t *vis, int x1, int x2)
{
    column_t *column;
    int       texturecolumn;
    fixed_t   frac;
    patch_t  *patch;
    fixed_t   baseclip;

    patch = W_CacheLumpNum(vis->patch + firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;

    if (vis->mobjflags & MF_SHADOW)
    {
        if (vis->mobjflags & MF_TRANSLATION)
        {
            colfunc = transtlcolfunc;
            dc_translation = translationtables - 256
                           + ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
        }
        else
        {                       // Draw using shadow column function
            colfunc = tlcolfunc;
        }
    }
    else if (vis->mobjflags & MF_EXTRATRANS && translucency && !vanillaparm)
    {
        // [JN] Draw using extra translucent column function.
        colfunc = extratlcolfunc;
    }
    else if (vis->mobjflags & MF_TRANSLATION)
    {
        // Draw using translated column function
        colfunc = transcolfunc;
        dc_translation = translationtables - 256
                       + ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
    }
    else if (vis->translation)
    {
        colfunc = transcolfunc;
        dc_translation = vis->translation;
    }

    dc_iscale = abs(vis->xiscale) >> (detailshift && !hires);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;

    sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);

    // check to see if weapon is a vissprite
    if (vis->psprite)
    {
        dc_texturemid += FixedMul(((centery - viewheight / 2) << FRACBITS), pspriteiscale);
        sprtopscreen += (viewheight / 2 - centery) << FRACBITS;
    }

    if (vis->footclip && !vis->psprite)
    {
        sprbotscreen = sprtopscreen + FixedMul(SHORT(patch->height) << FRACBITS, spryscale);
        baseclip = (sprbotscreen - FixedMul(vis->footclip << FRACBITS, spryscale)) >> FRACBITS;
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
        {
            I_Error(english_language ?
                    "R_DrawSpriteRange: bad texturecolumn" :
                    "R_DrawSpriteRange: некорректныая информация texturecolumn");
        }
#endif
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[texturecolumn]));
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

void R_ProjectSprite (mobj_t *thing)
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

    // [JN] Never draw a blood splat for Liches if colored blood is not set.
    if ((!colored_blood || vanillaparm) 
    &&  thing->type == MT_BLOODSPLATTER && thing->target
    &&  thing->target->type == MT_HEAD)
    {
        return;
    }

    // Transform the origin point.
    trx = interpx - viewx;
    try = interpy - viewy;
    gxt = FixedMul(trx, viewcos);
    gyt = -FixedMul(try, viewsin);
    tz = gxt - gyt;

    // Thing is behind view plane.
    if (tz < MINZ)
    {
        return;                 
    }

    xscale = FixedDiv(projection, tz);

    gxt = -FixedMul(trx, viewsin);
    gyt = FixedMul(try, viewcos);
    tx  = -(gyt + gxt);

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
                "R_ProjectSprite: некорректный номер спрайта %i ", thing->sprite);
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
        if ((thing->flags & MF_CORPSE && thing->type != MT_MINOTAUR
        &&   thing->type != MT_SORCERER1 && thing->type != MT_SORCERER2)
        // Scenery objects
        || thing->type == MT_MISC8        // Moss 1 [48]
        || thing->type == MT_MISC9        // Moss 2 [49]
        || thing->type == MT_MISC11       // Hanging Corpse [51]
        || thing->type == MT_SKULLHANG70  // Hanging Skull 1 [17]
        || thing->type == MT_SKULLHANG60  // Hanging Skull 2 [24]
        || thing->type == MT_SKULLHANG45  // Hanging Skull 3 [25]
        || thing->type == MT_SKULLHANG35  // Hanging Skull 4 [26]
        || thing->type == MT_SPLASHBASE   // Water splash base [-1]
        || thing->type == MT_SPLASH       // Water small splash [-1]
        || thing->type == MT_LAVASPLASH   // Lava splash base [-1]
        || thing->type == MT_LAVASMOKE    // Lava smoke [-1]
        || thing->type == MT_SLUDGESPLASH // Sludge splash base [-1]
        || thing->type == MT_SLUDGECHUNK  // Sludge small chunk [-1]
        || thing->type == MT_IMPCHUNK1    // Gargoyle chunk 1 [-1]
        || thing->type == MT_IMPCHUNK2)   // Gargoyle chunk 2 [-1]
        {
            if (thing->health & 1)
            {
                flip = true;
            }
        }
    }

    // calculate edges of the shape
    // [crispy] fix sprite offsets for mirrored sprites
    tx -= flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump];
    x1 = (centerxfrac + FixedMul(tx, xscale)) >> FRACBITS;

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FixedMul(tx, xscale)) >> FRACBITS) - 1;

    gzt = interpz + spritetopoffset[lump];

    // Off the side.
    if (x1 > viewwidth || x2 < 0)
    {
        return;
    }

    // [JN] killough 4/9/98: clip things which are out of view due to height
    if (interpz > viewz + FixedDiv(viewheight << FRACBITS, xscale)
    ||  gzt     < (int64_t)viewz - FixedDiv((viewheight << (FRACBITS + 1))-viewheight, xscale))
    {
        return;
    }

    // [JN] Quickly reject sprites with bad x ranges.
    if (x1 >= x2)
    {
        return;
    }

    // store information in a vissprite
    vis = R_NewVisSprite();
    vis->translation = NULL;
    vis->mobjflags = thing->flags;
    vis->psprite = false;
    vis->scale = xscale << (detailshift && !hires);
    vis->gx = interpx;
    vis->gy = interpy;
    vis->gz = interpz;
    vis->gzt = gzt;     // [JN] killough 3/27/98

    // foot clipping
    if (thing->flags2 & MF2_FEETARECLIPPED
    &&  thing->z <= thing->subsector->sector->floorheight)
    {
        vis->footclip = 10;
    }
    // [JN] Smaller clipping.
    else if (thing->flags2 & MF2_FEETARECLIPPED2
    &&  thing->z <= thing->subsector->sector->floorheight)
    {
        vis->footclip = 3;
    }
    else
    {
        vis->footclip = 0;
    }

    vis->texturemid = gzt - viewz - (vis->footclip << FRACBITS);
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
    else if (thing->frame & FF_FULLBRIGHT)
    {
        // Full bright.
        vis->colormap = colormaps;
    }
    else
    {
        // Diminished light.
        index = xscale >> (LIGHTSCALESHIFT - detailshift + hires);

        if (index >= MAXLIGHTSCALE)
            index = MAXLIGHTSCALE - 1;

        vis->colormap = spritelights[index];

        // [JN] Applying brightmaps to sprites...
        if (brightmaps && !vanillaparm)
        {
            // Green only
            if (thing->type == MT_ARTIEGG       // Morph Ovum
            ||  thing->type == MT_AMCBOWHEFTY)  // Quiver of Etherial Arrows
            {
                vis->colormap = fullbrights_greenonly[index];
            }

            // Red only
            if (thing->type == MT_AMSKRDWIMPY   // Lesser Runes
            ||  thing->type == MT_AMSKRDHEFTY   // Greater Runes
            ||  thing->type == MT_ARTITELEPORT  // Chaos Device
            ||  thing->type == MT_MUMMYSOUL     // Golem's freed ghost
            ||  thing->type == MT_HEAD)         // Iron Lich
            {
                vis->colormap = fullbrights_redonly[index];
            }

            // Blue only
            if (thing->type == MT_SORCERER1     // D'Sparil on Serpent
            ||  thing->type == MT_SORCERER2     // D'Sparil walking
            ||  thing->type == MT_SOR2TELEFADE) // D'Sparil teleporting
            {
                vis->colormap = fullbrights_blueonly[index];
            }

            // Not bronze
            if (thing->type == MT_ARTIINVULNERABILITY) // Ring of Invulnerability
            {
                vis->colormap = fullbrights_notbronze[index];
            }

            // Purple only
            if (thing->type == MT_WIZARD
            &&  thing->state - states != S_WIZARD_DIE8) // Disciple of D'Sparil
            {
                vis->colormap = fullbrights_purpleonly[index];
            }

            // Flame
            if (thing->type == MT_AMPHRDWIMPY  // Flame Orb
            ||  thing->type == MT_AMPHRDHEFTY  // Inferno Orb
            ||  thing->type == MT_MISC4        // Torch (Artifact)
            ||  thing->type == MT_CHANDELIER   // Chandelier
            ||  thing->type == MT_MISC10       // Torch
            ||  thing->type == MT_SERPTORCH    // Serpent Torch
            ||  thing->type == MT_MISC6        // Fire Brazier
            ||  thing->type == MT_MISC12       // Volcano
            ||  thing->state - states == S_CLINK_DIE1 // Sabreclaw's death sequence
            ||  thing->state - states == S_CLINK_DIE2
            ||  thing->state - states == S_CLINK_DIE3
            ||  thing->state - states == S_CLINK_DIE4
            ||  thing->state - states == S_CLINK_DIE5
            ||  thing->state - states == S_CLINK_DIE6)
            {
                vis->colormap = fullbrights_flame[index];
            }

            // Green only (diminished)
            if (thing->type == MT_MISC15        // Etherial Crossbow
            ||  thing->type == MT_AMCBOWWIMPY   // Etherial Arrows
            ||  thing->state - states == S_KNIGHT_ATK1  // Undead Warrior's attacking and death sequence
            ||  thing->state - states == S_KNIGHT_ATK2
            ||  thing->state - states == S_KNIGHT_ATK3
            ||  thing->state - states == S_KNIGHT_ATK4
            ||  thing->state - states == S_KNIGHT_ATK5
            ||  thing->state - states == S_KNIGHT_ATK6
            ||  thing->state - states == S_KNIGHT_DIE1
            ||  thing->state - states == S_KNIGHT_DIE2
            ||  thing->state - states == S_KNIGHT_DIE3
            ||  thing->state - states == S_KNIGHT_DIE4
            ||  thing->state - states == S_KNIGHT_DIE5
            ||  thing->state - states == S_KNIGHT_DIE6)
            {
                vis->colormap = fullbrights_greenonly_dim[index];
            }

            // Red only (diminished)
            if (thing->type == MT_WSKULLROD     // Hellstaff
            ||  thing->type == MT_WPHOENIXROD   // Phoenix Rod
            ||  thing->type == MT_ITEMSHIELD2   // Enchanted Shield
            ||  thing->state - states == S_BEAST_ATK1 // Weredragon's attacking frames
            ||  thing->state - states == S_BEAST_ATK2)
            {
                vis->colormap = fullbrights_redonly_dim[index];
            }

            // Blue only
            if (thing->type == MT_AMBLSRWIMPY   // Claw Orb
            ||  thing->type == MT_AMBLSRHEFTY)  // Energy Orb
            {
                vis->colormap = fullbrights_blueonly_dim[index];
            }

            // Yellow only
            if (thing->type == MT_AMGWNDWIMPY   // Wand Crystal
            ||  thing->type == MT_AMGWNDHEFTY   // Crystal Geode
            ||  thing->state - states == S_IMP_MEATK2   // Gargoyle melee attack
            ||  thing->state - states == S_IMP_MEATK3
            ||  thing->state - states == S_IMP_MSATK2_1 // Gargoyle missle attack
            ||  thing->state - states == S_IMP_MSATK2_2)
            {
                vis->colormap = fullbrights_yellowonly_dim[index];
            }

            // Standard full bright formula
            if (thing->type == MT_BEASTBALL       // Weredragon's fireball
            ||  thing->type == MT_BURNBALL        // Weredragon's fireball
            ||  thing->type == MT_BURNBALLFB      // Weredragon's fireball
            ||  thing->type == MT_PUFFY           // Weredragon's fireball
            ||  thing->type == MT_HEADFX3         // Iron Lich's fire column
            ||  thing->type == MT_VOLCANOBLAST    // Volcano blast
            ||  thing->type == MT_VOLCANOTBLAST)  // Volcano blast (impact)
            {
                vis->colormap = colormaps;
            }
        }
        
        // [JN] Fallback. If we are not using brightmaps, apply full brightness
        // to the objects, that no longer lighten up in info.c:
        // (S_FIREBRAZIER* and S_WALLTORCH*).
        if (!brightmaps || vanillaparm)
        {
            if (thing->type == MT_MISC4    // Torch (Artifact)
            ||  thing->type == MT_MISC6    // S_FIREBRAZIER*
            ||  thing->type == MT_MISC10)  // S_WALLTORCH*
            {
                vis->colormap = colormaps;
            }
        }
    }

    // [JN] Colored blood
    if (colored_blood && !vanillaparm &&  thing->type == MT_BLOODSPLATTER && thing->target)
    {
        if (thing->target->type == MT_WIZARD)
        {
            vis->translation = cr[CR_RED2MAGENTA_HERETIC];
        }
        else if (thing->target->type == MT_HEAD)
        {
            vis->translation = cr[CR_RED2GRAY_HERETIC];
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

void R_AddSprites (sector_t *sec)
{
    mobj_t *thing;
    int     lightnum;

    lightnum = ((sec->lightlevel + level_brightness) >> LIGHTSEGSHIFT) + extralight;

    if (lightnum < 0)
    {
        spritelights = scalelight[0];

        // [JN] Brightmaps
        fullbrights_greenonly = fullbright_greenonly[0];
        fullbrights_redonly = fullbright_redonly[0];
        fullbrights_blueonly = fullbright_blueonly[0];
        fullbrights_purpleonly = fullbright_purpleonly[0];
        fullbrights_notbronze = fullbright_notbronze[0];
        fullbrights_flame = fullbright_flame[0];
        fullbrights_greenonly_dim = fullbright_greenonly_dim[0];
        fullbrights_redonly_dim = fullbright_redonly_dim[0];
        fullbrights_blueonly_dim = fullbright_blueonly_dim[0];
        fullbrights_yellowonly_dim = fullbright_yellowonly_dim[0];
        fullbrights_ethereal = fullbright_ethereal[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        spritelights = scalelight[LIGHTLEVELS - 1];

        // [JN] Brightmaps
        fullbrights_greenonly = fullbright_greenonly[LIGHTLEVELS - 1];
        fullbrights_redonly = fullbright_redonly[LIGHTLEVELS - 1];
        fullbrights_blueonly = fullbright_blueonly[LIGHTLEVELS - 1];
        fullbrights_purpleonly = fullbright_purpleonly[LIGHTLEVELS - 1];
        fullbrights_notbronze = fullbright_notbronze[LIGHTLEVELS - 1];
        fullbrights_flame = fullbright_flame[LIGHTLEVELS - 1];
        fullbrights_greenonly_dim = fullbright_greenonly_dim[LIGHTLEVELS - 1];
        fullbrights_redonly_dim = fullbright_redonly_dim[LIGHTLEVELS - 1];
        fullbrights_blueonly_dim = fullbright_blueonly_dim[LIGHTLEVELS - 1];
        fullbrights_yellowonly_dim = fullbright_yellowonly_dim[LIGHTLEVELS - 1];
        fullbrights_ethereal = fullbright_ethereal[LIGHTLEVELS - 1];
    }
    else
    {
        spritelights = scalelight[lightnum];

        // [JN] Brightmaps
        fullbrights_greenonly = fullbright_greenonly[lightnum];
        fullbrights_redonly = fullbright_redonly[lightnum];
        fullbrights_blueonly = fullbright_blueonly[lightnum];
        fullbrights_purpleonly = fullbright_purpleonly[lightnum];
        fullbrights_notbronze = fullbright_notbronze[lightnum];
        fullbrights_flame = fullbright_flame[lightnum];
        fullbrights_greenonly_dim = fullbright_greenonly_dim[lightnum];
        fullbrights_redonly_dim = fullbright_redonly_dim[lightnum];
        fullbrights_blueonly_dim = fullbright_blueonly_dim[lightnum];
        fullbrights_yellowonly_dim = fullbright_yellowonly_dim[lightnum];
        fullbrights_ethereal = fullbright_ethereal[lightnum];
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
= [crispy] apply bobbing (or centering) to the player's weapon sprite.
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
= R_ApplyWeaponBob
=
= [crispy] & [JN] Halfed bobbing amplitude while firing.
=
================================================================================
*/

static inline void R_ApplyWeaponFiringBob (fixed_t *sx, boolean bobx, fixed_t *sy, boolean boby)
{
    const angle_t angle = (128 * leveltime) & FINEMASK;

    if (sx)
    {
        *sx = FRACUNIT;
    
        if (bobx)
        {
            *sx += FixedMul(viewplayer->bob, finecosine[angle] / 2);
        }
    }

    if (sy)
    {
        *sy = 32 * FRACUNIT; // [crispy] WEAPONTOP
    
        if (boby)
        {
            *sy += FixedMul(viewplayer->bob, finesine[angle & (FINEANGLES / 2 - 1)] / 2);
        }
    }
}

/*
================================================================================
=
= R_ApplyRaiseLowerBob
=
= [JN] Smooth bobbing for raise and lowering weapons.
=
================================================================================
*/

static inline void R_ApplyRaiseLowerBob (fixed_t *sx, boolean bobx)
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
}

/*
================================================================================
=
= R_ApplyWeaponBob
=
= [crispy] & [JN] Chicken's special bobbingю
=
================================================================================
*/

static inline void R_ApplyChickenBob (fixed_t *sx, boolean bobx, fixed_t *sy, boolean boby)
{
    const angle_t angle = (128 * leveltime) & FINEMASK;

    if (sx)
    {
        *sx = FRACUNIT;
    
        if (bobx)
        *sx += FixedMul(viewplayer->bob, finecosine[angle] / 18);
    }

    if (sy)
    {
        *sy = 32 * FRACUNIT; // [crispy] WEAPONTOP
    
        if (boby)
        *sy += FixedMul(viewplayer->bob, finesine[angle & (FINEANGLES / 2 - 1)] / 6);
    }
}

/*
================================================================================
=
= R_DrawPSprite
=
================================================================================
*/

static int PSpriteSY[NUMWEAPONS] = {
    0,                          // staff
    5 * FRACUNIT,               // goldwand
    15 * FRACUNIT,              // crossbow
    15 * FRACUNIT,              // blaster
    15 * FRACUNIT,              // skullrod
    15 * FRACUNIT,              // phoenix rod
    15 * FRACUNIT,              // mace
    15 * FRACUNIT,              // gauntlets
    15 * FRACUNIT               // beak
};

void R_DrawPSprite (pspdef_t *psp)
{
    fixed_t tx;
    int x1, x2;
    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int lump;
    boolean flip;
    vissprite_t *vis, avis;
    fixed_t psp_sx = psp->sx, psp_sy = psp->sy;
    // [JN] We need to define what "state" actually is (from Crispy)
    const int state = viewplayer->psprites[ps_weapon].state - states;

    int tempangle;

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

    // [JN] Applying weapon bobbing for ready states, halfed for firing states 
    // and x-only for raise/lower states. "Plus" means activated Tome of Power.
    if (weapon_bobbing && singleplayer && !vanillaparm)
    {
        if (/* Staff+   */ state == S_STAFFREADY2_1    || state == S_STAFFREADY2_2    || state == S_STAFFREADY2_3    ||
        /* Gauntlets+   */ state == S_GAUNTLETREADY2_1 || state == S_GAUNTLETREADY2_2 || state == S_GAUNTLETREADY2_3 ||
        /* CrBow        */ state == S_CRBOWATK1_6      || state == S_CRBOWATK1_7      || state == S_CRBOWATK1_8      ||
        /* CrBow+       */ state == S_CRBOWATK2_6      || state == S_CRBOWATK2_7      || state == S_CRBOWATK2_8      ||
        /* HellStaff+   */ state == S_HORNRODATK2_5    || state == S_HORNRODATK2_6    || state == S_HORNRODATK2_7    || state == S_HORNRODATK2_8 || state == S_HORNRODATK2_9 ||
        /* Phoenix Rod  */ state == S_PHOENIXATK1_4    || state == S_PHOENIXATK1_5    ||
        /* Phoenix Rod+ */ state == S_PHOENIXATK2_4    ||
        /* Firemace+    */ state == S_MACEATK2_4)
        {
            R_ApplyWeaponBob(&psp_sx, true, &psp_sy, true);
        }

        if (/* Gauntlets*/ state == S_GAUNTLETATK1_1 || state == S_GAUNTLETATK1_2 || state == S_GAUNTLETATK1_3 || state == S_GAUNTLETATK1_4 || state == S_GAUNTLETATK1_5 || state == S_GAUNTLETATK1_6 || state == S_GAUNTLETATK1_7 ||
        /* Gauntlets+   */ state == S_GAUNTLETATK2_1 || state == S_GAUNTLETATK2_2 || state == S_GAUNTLETATK2_3 || state == S_GAUNTLETATK2_4 || state == S_GAUNTLETATK2_5 || state == S_GAUNTLETATK2_6 || state == S_GAUNTLETATK2_7 ||
        /* Staff        */ state == S_STAFFATK1_1    || state == S_STAFFATK1_2    || state == S_STAFFATK1_3    ||
        /* Staff+       */ state == S_STAFFATK2_1    || state == S_STAFFATK2_2    || state == S_STAFFATK2_3    ||
        /* Wand         */ state == S_GOLDWANDATK1_1 || state == S_GOLDWANDATK1_2 || state == S_GOLDWANDATK1_3 || state == S_GOLDWANDATK1_4 ||
        /* Wand+        */ state == S_GOLDWANDATK2_1 || state == S_GOLDWANDATK2_2 || state == S_GOLDWANDATK2_3 || state == S_GOLDWANDATK2_4 ||
        /* CrBow        */ state == S_CRBOWATK1_1    || state == S_CRBOWATK1_2    || state == S_CRBOWATK1_3    || state == S_CRBOWATK1_4    || state == S_CRBOWATK1_5    ||
        /* CrBow+       */ state == S_CRBOWATK2_1    || state == S_CRBOWATK2_2    || state == S_CRBOWATK2_3    || state == S_CRBOWATK2_4    || state == S_CRBOWATK2_5    ||
        /* DrClaw       */ state == S_BLASTERATK1_1  || state == S_BLASTERATK1_2  || state == S_BLASTERATK1_3  || state == S_BLASTERATK1_4  || state == S_BLASTERATK1_5  || state == S_BLASTERATK1_6  ||
        /* DrClaw+      */ state == S_BLASTERATK2_1  || state == S_BLASTERATK2_2  || state == S_BLASTERATK2_3  || state == S_BLASTERATK2_4  || state == S_BLASTERATK2_5  || state == S_BLASTERATK2_6  ||
        /* HlStaff      */ state == S_HORNRODATK1_1  || state == S_HORNRODATK1_2  || state == S_HORNRODATK1_3  ||
        /* HlStaff+     */ state == S_HORNRODATK2_1  || state == S_HORNRODATK2_2  || state == S_HORNRODATK2_3  || state == S_HORNRODATK2_4  ||
        /* Phoenix Rod  */ state == S_PHOENIXATK1_1  || state == S_PHOENIXATK1_2  || state == S_PHOENIXATK1_3  ||
        /* Phoenix Rod+ */ state == S_PHOENIXATK2_1  || state == S_PHOENIXATK2_2  || state == S_PHOENIXATK2_3  ||
        /* Firemace     */ state == S_MACEATK1_1     || state == S_MACEATK1_2     || state == S_MACEATK1_3     || state == S_MACEATK1_4     || state == S_MACEATK1_5     || state == S_MACEATK1_6     || state == S_MACEATK1_7     || state == S_MACEATK1_8 || state == S_MACEATK1_9 || state == S_MACEATK1_10 ||
        /* Firemace+    */ state == S_MACEATK2_1     || state == S_MACEATK2_2     || state == S_MACEATK2_3)
        {
            R_ApplyWeaponFiringBob(&psp_sx, true, &psp_sy, true);
        }

        if (/* Gauntlets*/ state == S_GAUNTLETDOWN || state == S_GAUNTLETUP ||
        /* Staff        */ state == S_STAFFDOWN    || state == S_STAFFUP    ||
        /* Staff+       */ state == S_STAFFDOWN2   || state == S_STAFFUP2   ||
        /* Wand         */ state == S_GOLDWANDDOWN || state == S_GOLDWANDUP ||
        /* CrBow        */ state == S_CRBOWDOWN    || state == S_CRBOWUP    ||
        /* DrClaw       */ state == S_BLASTERDOWN  || state == S_BLASTERUP  ||
        /* HlStaff      */ state == S_HORNRODDOWN  || state == S_HORNRODUP  ||
        /* Phoenix Rod  */ state == S_PHOENIXDOWN  || state == S_PHOENIXUP  ||
        /* Firemace     */ state == S_MACEDOWN     || state == S_MACEUP)
        {
            R_ApplyRaiseLowerBob(&psp_sx, true);
        }

        // [JN] Applying special chicken's bobbing:
        if (state == S_BEAKREADY || state == S_BEAKATK1_1 || state == S_BEAKATK2_1)
        {
            R_ApplyChickenBob(&psp_sx, true, &psp_sy, true);
        }
    }

    // [crispy] squat down weapon sprite a bit after hitting the ground
    if (weapon_bobbing && !vanillaparm)
    psp_sy += abs(viewplayer->psp_dy);
        
//
// calculate edges of the shape
//
    tx = psp_sx - 160 * FRACUNIT;

    // [crispy] fix sprite offsets for mirrored sprites
    tx -= flip ? 2 * tx - spriteoffset[lump] + spritewidth[lump] :
                                               spriteoffset[lump];
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
    vis->translation = NULL;
    vis->mobjflags = 0;
    vis->psprite = true;
    // [crispy] weapons drawn 1 pixel too high when player is idle
    vis->texturemid = (BASEYCENTER<<FRACBITS)
                    + FRACUNIT/4-(psp_sy-spritetopoffset[lump]);
    if (screenblocks >= 11)
    {
        vis->texturemid -= PSpriteSY[players[consoleplayer].readyweapon];
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

    if (viewplayer->powers[pw_invisibility] > 4 * 32 ||
        viewplayer->powers[pw_invisibility] & 8)
    {
        // [JN] Fixed vanilla bug: translucent HUD weapons 
        // should also be affected by yellow invulnerability palette.
        // Invisibility
        vis->colormap = fixedcolormap ? fixedcolormap : 
                                        spritelights[MAXLIGHTSCALE - 1];
        vis->mobjflags |= MF_SHADOW;
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
    if (uncapped_fps && weapon_bobbing && !vanillaparm)
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

void R_DrawPlayerSprites (void)
{
    int i, lightnum;
    pspdef_t *psp;
    // [JN] We need to define what "state" actually is (from Crispy)
    const int state = viewplayer->psprites[ps_weapon].state - states;

//
// get light level
//
    lightnum = ((viewplayer->mo->subsector->sector->lightlevel 
             + level_brightness) >> LIGHTSEGSHIFT) + extralight;
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
            // Staff+
            if (state == S_STAFFDOWN2 || state == S_STAFFUP2
            ||  state == S_STAFFREADY2_1 || state == S_STAFFREADY2_2
            ||  state == S_STAFFREADY2_3 || state == S_STAFFATK2_1
            ||  state == S_STAFFATK2_2 || state == S_STAFFATK2_3)
            spritelights = fullbright_blueonly[lightnum];
            // Gauntlets
            else
            if (state == S_GAUNTLETATK1_1 || state == S_GAUNTLETATK1_2
            ||  state == S_GAUNTLETATK1_3 || state == S_GAUNTLETATK1_4
            ||  state == S_GAUNTLETATK1_5 || state == S_GAUNTLETATK1_6
            ||  state == S_GAUNTLETATK1_7)
            spritelights = fullbright_greenonly[lightnum];
            // Gauntlets+
            else
            if (state == S_GAUNTLETREADY2_1 || state == S_GAUNTLETREADY2_2 || state == S_GAUNTLETREADY2_3
            ||  state == S_GAUNTLETDOWN2  || state == S_GAUNTLETUP2
            ||  state == S_GAUNTLETATK2_1 || state == S_GAUNTLETATK2_2
            ||  state == S_GAUNTLETATK2_3 || state == S_GAUNTLETATK2_4
            ||  state == S_GAUNTLETATK2_5 || state == S_GAUNTLETATK2_6
            ||  state == S_GAUNTLETATK2_7)
            spritelights = fullbright_redonly[lightnum];
            // Wand
            else
            if (state == S_GOLDWANDATK1_1 || state == S_GOLDWANDATK1_2
            ||  state == S_GOLDWANDATK1_3 || state == S_GOLDWANDATK1_4)
            spritelights = fullbright_flame[lightnum];
            // Wand+
            else
            if (state == S_GOLDWANDATK2_1 || state == S_GOLDWANDATK2_2
            ||  state == S_GOLDWANDATK2_3 || state == S_GOLDWANDATK2_4)
            spritelights = fullbright_flame[lightnum];
            // Crossbow
            else
            if (state == S_CRBOWDOWN || state == S_CRBOWUP
            ||  state == S_CRBOW1 || state == S_CRBOW2
            ||  state == S_CRBOW3 || state == S_CRBOW4
            ||  state == S_CRBOW5 || state == S_CRBOW6
            ||  state == S_CRBOW7 || state == S_CRBOW8
            ||  state == S_CRBOW9 || state == S_CRBOW10
            ||  state == S_CRBOW11 || state == S_CRBOW12
            ||  state == S_CRBOW13 || state == S_CRBOW14
            ||  state == S_CRBOW15 || state == S_CRBOW16
            ||  state == S_CRBOW17 || state == S_CRBOW18
            ||  state == S_CRBOWATK1_1 || state == S_CRBOWATK1_2
            ||  state == S_CRBOWATK1_3 || state == S_CRBOWATK1_4
            ||  state == S_CRBOWATK1_5 || state == S_CRBOWATK1_6
            ||  state == S_CRBOWATK1_7 || state == S_CRBOWATK1_8)
            spritelights = fullbright_ethereal[lightnum];
            // Crossbow+
            else
            if (state == S_CRBOWATK2_1 || state == S_CRBOWATK2_2
            ||  state == S_CRBOWATK2_3 || state == S_CRBOWATK2_4
            ||  state == S_CRBOWATK2_5 || state == S_CRBOWATK2_6
            ||  state == S_CRBOWATK2_7 || state == S_CRBOWATK2_8)
            spritelights = fullbright_ethereal[lightnum];
            // Dragon Claw
            else
            if (state == S_BLASTERREADY || state == S_BLASTERDOWN
            ||  state == S_BLASTERUP || state == S_BLASTERATK1_1
            ||  state == S_BLASTERATK1_2 || state == S_BLASTERATK1_3
            ||  state == S_BLASTERATK1_4 || state == S_BLASTERATK1_5
            ||  state == S_BLASTERATK1_6)
            spritelights = fullbright_blueonly[lightnum];
            // Dragon Claw+
            else
            if (state == S_BLASTERATK2_1 || state == S_BLASTERATK2_2
            ||  state == S_BLASTERATK2_3 || state == S_BLASTERATK2_4
            ||  state == S_BLASTERATK2_5 || state == S_BLASTERATK2_6)
            spritelights = fullbright_blueonly[lightnum];
            // Hell Staff 
            else
            if (state == S_HORNRODATK1_1 || state == S_HORNRODATK1_2
            ||  state == S_HORNRODATK1_3)
            spritelights = fullbright_redonly[lightnum];
            // Hell Staff+
            else
            if (state == S_HORNRODATK2_1 || state == S_HORNRODATK2_2
            ||  state == S_HORNRODATK2_3 || state == S_HORNRODATK2_4
            ||  state == S_HORNRODATK2_5 || state == S_HORNRODATK2_6
            ||  state == S_HORNRODATK2_7 || state == S_HORNRODATK2_8
            ||  state == S_HORNRODATK2_9)
            spritelights = fullbright_redonly[lightnum];
            // Phoenix Rod
            else
            if (state == S_PHOENIXATK1_1 || state == S_PHOENIXATK1_2
            ||  state == S_PHOENIXATK1_3 || state == S_PHOENIXATK1_4)
            spritelights = fullbright_flame[lightnum];
            // Phoenix Rod+
            else
            if (state == S_PHOENIXATK2_1 || state == S_PHOENIXATK2_3
            ||  state == S_PHOENIXATK2_4)
            spritelights = fullbright_flame[lightnum];
            // Phoenix Rod's red gem
            else
            if (state == S_PHOENIXREADY || state == S_PHOENIXDOWN
            ||  state == S_PHOENIXUP)
            spritelights = fullbright_redonly[lightnum];
        }
    }

    // [JN] Fallback. If we are not using brightmaps, apply full brightness
    // to the objects, that no longer lighten up in info.c:
    // (S_GAUNTLETATK1_3-5 and S_GAUNTLETATK2_3-5).
    if (!brightmaps || vanillaparm)
    {
        if (state == S_GAUNTLETATK1_3 || state == S_GAUNTLETATK1_4
        ||  state == S_GAUNTLETATK1_5 || state == S_GAUNTLETATK2_3
        ||  state == S_GAUNTLETATK2_4 || state == S_GAUNTLETATK2_5)
        spritelights = scalelight[LIGHTLEVELS-1]; 
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
    int x, r1, r2;
    fixed_t scale, lowscale;

    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
        clipbot[x] = cliptop[x] = -2;
    }

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
        {
            clipbot[x] = viewheight;
        }
        if (cliptop[x] == -2)
        {
            cliptop[x] = -1;
        }
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
    drawseg_t*      ds;
    int cx = screenwidth / 2;

    R_SortVisSprites();

    // [JN] e6y
    // Makes sense for scenes with huge amount of drawsegs.
    for (i = 0 ; i < DS_RANGES_COUNT ; i++)
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
    rendered_vissprites = num_vissprite;
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
