//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Refresh of things, i.e. objects represented by sprites.
//


#include "deh_main.h"
#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "r_local.h"
#include "doomstat.h"
#include "p_local.h"
#include "jn.h"


#define MINZ        (FRACUNIT*4)
#define BASEYCENTER (ORIGHEIGHT/2)


//
// Sprite rotation 0 is facing the viewer, rotation 1 is one angle turn 
// CLOCKWISE around the axis. This is not the same as the angle, which
// increases counter clockwise (protractor). 
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t pspritescale, pspriteiscale;

static lighttable_t **spritelights;

// psprite clipping and initializing clipping
int *negonearray;           // [JN] killough 2/8/98: // dropoff overflow
int *screenheightarray;     //      change to MAX_*  // dropoff overflow
static int *clipbot = NULL; // [JN] killough 2/8/98: // dropoff overflow
static int *cliptop = NULL; //      change to MAX_*  // dropoff overflow

// variables used to look up and range check thing_t sprites patches
int          numsprites;
int          maxframe;
char        *spritename;
spritedef_t *sprites;
static spriteframe_t sprtemp[29];

// initialization functions
int *mfloorclip, *mceilingclip;

fixed_t spryscale;
int64_t sprtopscreen; // [crispy] WiggleFix

static size_t num_vissprite, num_vissprite_alloc, num_vissprite_ptrs; // killough
static vissprite_t *vissprites, **vissprite_ptrs;                     // killough


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


// -----------------------------------------------------------------------------
// R_InitSpritesRes
// -----------------------------------------------------------------------------

void R_InitSpritesRes(void)
{
    if (flipscreenwidth)
    {
        free(flipscreenwidth);
    }
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

    flipscreenwidth = calloc(1, (screenwidth + 1) * sizeof(*flipscreenwidth));
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

// -----------------------------------------------------------------------------
// R_InstallSpriteLump
// Local function for R_InitSprites.
// -----------------------------------------------------------------------------

static void R_InstallSpriteLump (const int lump, unsigned const frame, char rot, boolean flipped)
{
    int r;
    // [crispy] support 16 sprite rotations
    unsigned rotation = (rot >= 'A') ? rot - 'A' + 10 : (rot >= '0') ? rot - '0' : 17;

    if (frame >= 29 || rotation > 16) // [crispy] support 16 sprite rotations
    {
	    I_Error(english_language ?
                "R_InstallSpriteLump: Bad frame characters in lump %i" :
                "R_InstallSpriteLump: некорректный фрейм в блоке %i", lump);
    }

    if ((int)frame > maxframe)
    {
        maxframe = frame;
    }

    if (rotation == 0)
    {
        // the lump should be used for all rotations
        // [crispy] make non-fatal
        if (sprtemp[frame].rotate == false)
        {
            fprintf (stderr, "R_InitSprites: Sprite %s frame %c has "
                     "multip rot=0 lump\n", spritename, 'A'+frame);
        }

	    // [crispy] make non-fatal
	    if (sprtemp[frame].rotate == true)
        {
	        fprintf (stderr, "R_InitSprites: Sprite %s frame %c has rotations "
	    	         "and a rot=0 lump\n", spritename, 'A'+frame);
        }
			
        for (r = 0 ; r < 8 ; r++)
        {
            // [crispy] only if not yet substituted
            if (sprtemp[frame].lump[r] == -1)
            {
                sprtemp[frame].lump[r] = lump - firstspritelump;
                sprtemp[frame].flip[r] = (byte)flipped;
                sprtemp[frame].rotate = false;
            }
        }

        return;
    }

    // the lump is only used for one rotation
    // [crispy] make non-fatal
    if (sprtemp[frame].rotate == false)
    {
        fprintf (stderr, "R_InitSprites: Sprite %s frame %c has rotations "
                 "and a rot=0 lump\n", spritename, 'A'+frame);
    }
		
    // make 0 based
    rotation--;

    if (sprtemp[frame].lump[rotation] != -1)
    {
        // [crispy] make non-fatal
        fprintf (stderr, "R_InitSprites: Sprite %s : %c : %c "
                 "has two lumps mapped to it\n", spritename, 'A'+frame, '1'+rotation);
        return;
    }
		
    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip[rotation] = (byte)flipped;
    sprtemp[frame].rotate = true;
}

// -----------------------------------------------------------------------------
// R_InitSpriteDefs
// Pass a null terminated list of sprite names (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account for horizontally 
// flipped sprites. Will report an error if the lumps are inconsistant. 
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor, a letter for the frame,
// and a number for the rotation. A sprite that is flippable will have an 
// additional letter/number appended.
// The rotation character can be 0 to signify no rotations.
// -----------------------------------------------------------------------------

static void R_InitSpriteDefs (char **namelist)
{
    int    i;
    int    l;
    int    frame;
    int    rotation;
    int    start;
    int    end;
    int    patched;
    char **check;

    // count the number of sprite names
    check = namelist;
    while (*check != NULL)
    {
        check++;
    }

    numsprites = check-namelist;

    if (!numsprites)
    {
        return;
    }

    sprites = Z_Malloc(numsprites *sizeof(*sprites), PU_STATIC, NULL);

    start = firstspritelump-1;
    end = lastspritelump+1;

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i = 0 ; i < numsprites ; i++)
    {
        spritename = DEH_String(namelist[i]);
        memset (sprtemp,-1, sizeof(sprtemp));

        maxframe = -1;

        // scan the lumps,
        //  filling in the frames for whatever is found
        for (l = start+1 ; l < end ; l++)
        {
            if (!strncasecmp(lumpinfo[l]->name, spritename, 4))
            {
                frame = lumpinfo[l]->name[4] - 'A';
                rotation = lumpinfo[l]->name[5];

                if (modifiedgame)
                {
                    patched = W_GetNumForName (lumpinfo[l]->name);
                }
                else
                {
                    patched = l;
                }

                R_InstallSpriteLump (patched, frame, rotation, false);

                if (lumpinfo[l]->name[6])
                {
                    frame = lumpinfo[l]->name[6] - 'A';
                    rotation = lumpinfo[l]->name[7];
                    R_InstallSpriteLump (l, frame, rotation, true);
                }
            }
        }

        // check the frames that were found for completeness
        if (maxframe == -1)
        {
            sprites[i].numframes = 0;
            continue;
        }

        maxframe++;

        for (frame = 0 ; frame < maxframe ; frame++)
        {
            switch ((int)sprtemp[frame].rotate)
            {
                case -1:
                // no rotations were found for that frame at all
                // [crispy] make non-fatal
                fprintf (stderr, english_language ?
                         "R_InitSprites: No patches found for %s frame %c\n" :
                         "R_InitSprites: не найдены патчи для спрайта %s, фрейма %c\n", spritename, frame+'A');
                break;

                case 0:
                // only the first rotation is needed
                break;

                case 1:
                // must have all 8 frames
                for (rotation=0 ; rotation<8 ; rotation++)
                    if (sprtemp[frame].lump[rotation] == -1)
                    I_Error (english_language ?
                            "R_InitSprites: Sprite %s frame %c is missing rotations" :
                            "R_InitSprites: в фрейме %c спрайта %s отсутствует информация о вращении", spritename, frame+'A');

                // [crispy] support 16 sprite rotations
                sprtemp[frame].rotate = 2;
                for ( ; rotation < 16 ; rotation++)
                if (sprtemp[frame].lump[rotation] == -1)
                {
                    sprtemp[frame].rotate = 1;
                    break;
                }

                break;
            }
        }

        // allocate space for the frames present and copy sprtemp to it
        sprites[i].numframes = maxframe;
        sprites[i].spriteframes = Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
        memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }
}

// -----------------------------------------------------------------------------
// R_InitSprites
// Called at program start.
// -----------------------------------------------------------------------------

void R_InitSprites (char **namelist)
{
    int i;

    for (i = 0 ; i < screenwidth ; i++)
    {
        negonearray[i] = -1;
    }

    R_InitSpriteDefs (namelist);
}

// -----------------------------------------------------------------------------
// R_ClearSprites
// Called at frame start.
// -----------------------------------------------------------------------------

void R_ClearSprites (void)
{
    num_vissprite = 0;  // [JN] killough
}

// -----------------------------------------------------------------------------
// R_NewVisSprite
// -----------------------------------------------------------------------------

static vissprite_t* R_NewVisSprite (void)
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

// -----------------------------------------------------------------------------
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored in posts/runs of opaque pixels.
// -----------------------------------------------------------------------------

void R_DrawMaskedColumn (const column_t *column)
{
    int64_t	topscreen, bottomscreen; // [crispy] WiggleFix
    fixed_t	basetexturemid;
    int     top = -1;

    basetexturemid = dc_texturemid;
    dc_texheight = 0; // [crispy] Tutti-Frutti fix

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
        dc_yh = (int)((bottomscreen-1)>>FRACBITS); // [crispy] WiggleFix

        if (dc_yh >= mfloorclip[dc_x])
        {
            dc_yh = mfloorclip[dc_x]-1;
        }
        if (dc_yl <= mceilingclip[dc_x])
        {
            dc_yl = mceilingclip[dc_x]+1;
        }

        // [JN] killough 3/2/98, 3/27/98: Failsafe against overflow/crash:
        if (dc_yl <= dc_yh && dc_yh < viewheight)
        {
            dc_source = (byte *)column + 3;
            dc_texturemid = basetexturemid - (top<<FRACBITS);
    
            // Drawn by either R_DrawColumn
            //  or (SHADOW) R_DrawFuzzColumn.
            colfunc ();	
        }

        column = (column_t *)(  (byte *)column + column->length + 4);
    }

    dc_texturemid = basetexturemid;
}

// -----------------------------------------------------------------------------
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
// -----------------------------------------------------------------------------

static void R_DrawVisSprite (const vissprite_t *vis, const int x1, const int x2)
{
    column_t *column;
    int       texturecolumn;
    fixed_t   frac;
    patch_t  *patch;

    patch = W_CacheLumpNum (vis->patch+firstspritelump, PU_CACHE);

    // [crispy] brightmaps for select sprites
    dc_colormap[0] = vis->colormap[0];
    dc_colormap[1] = vis->colormap[1];
    dc_brightmap = vis->brightmap;

    if (!dc_colormap[0])
    {
        // NULL colormap = shadow draw
        colfunc = fuzzcolfunc;
    }
    else if (vis->mobjflags & MF_TRANSLATION)
    {
        colfunc = transcolfunc;
        dc_translation = translationtables - 256 
                       + ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8));
    }
    else if (vis->translation)
    {
        colfunc = transcolfunc;
        dc_translation = vis->translation;
    }

    // [crispy] translucent sprites
    if (translucency && !vanillaparm && (vis->mobjflags & MF_TRANSLUCENT))
    {
        colfunc = tlcolfunc;
    }

    // [JN] Translucent fuzz effect.
    if (vis->mobjflags & MF_SHADOW && improved_fuzz == 4)
    {
        colfunc = fuzzcolfunc;
    }

    dc_iscale = abs(vis->xiscale)>>(detailshift && !hires);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);

    for (dc_x = vis->x1 ; dc_x <= vis->x2 ; dc_x++, frac += vis->xiscale)
    {
        texturecolumn = frac>>FRACBITS;
#ifdef RANGECHECK
        if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
        I_Error (english_language ?
                 "R_DrawVisSprite: bad texturecolumn" :
                 "R_DrawVisSprite: некорректныая информация texturecolumn");
#endif
        column = (column_t *) ((byte *)patch + LONG(patch->columnofs[texturecolumn]));
        R_DrawMaskedColumn (column);
    }

    colfunc = basecolfunc;
}

// -----------------------------------------------------------------------------
// R_AnimateBrightmaps
// [JN] Animate brightmaps by randomizing light level.
// -----------------------------------------------------------------------------

static int brightmap_anim;

void R_AnimateBrightmaps (void)
{
    if (brightmaps && !vanillaparm)
    {
        brightmap_anim = rand() % 16;
    }
    else
    {
        brightmap_anim = 0;
    }
}

// -----------------------------------------------------------------------------
// R_ProjectSprite
// Generates a vissprite for a thing if it might be visible.
// -----------------------------------------------------------------------------

static void R_ProjectSprite (const mobj_t *thing, const int lightnum)
{
    int            x1, x2, lump, index;
    unsigned       rot;
    fixed_t        tr_x, tr_y;
    fixed_t        gxt, gyt;
    fixed_t        gzt;    // [JN] killough 3/27/98
    fixed_t        tx, tz;
    fixed_t        xscale, iscale;
    vissprite_t   *vis;
    spritedef_t   *sprdef;
    spriteframe_t *sprframe;
    boolean        flip;
    angle_t        ang;    
    fixed_t        interpx, interpy, interpz, interpangle;
    const boolean  can_animate_bmap = leveltime & 31 && (rand() % 255 > 220) 
                                      && gametic & 1 &&  leveltime > oldleveltime;

    // [AM] Interpolate between current and last position,
    //      if prudent.
    if (uncapped_fps && !vanillaparm &&
        // Don't interpolate if the mobj did something
        // that would necessitate turning it off for a tic.
        thing->interp == true &&
        // Don't interpolate during a paused state.
        leveltime > oldleveltime)
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

    // transform the origin point
    tr_x = interpx - viewx;
    tr_y = interpy - viewy;

    gxt = FixedMul(tr_x,viewcos); 
    gyt = -FixedMul(tr_y,viewsin);

    tz = gxt-gyt; 

    // thing is behind view plane?
    if (tz < MINZ)
    {
        return;
    }

    xscale = FixedDiv(projection, tz);

    gxt = -FixedMul(tr_x,viewsin); 
    gyt = FixedMul(tr_y,viewcos); 
    tx = -(gyt+gxt); 

    // too far off the side?
    if (abs(tx)>(tz<<2))
    {
        return;
    }

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned int) thing->sprite >= (unsigned int) numsprites)
    I_Error (english_language ?
             "R_ProjectSprite: invalid sprite number %i" :
             "R_ProjectSprite: некорректный номер спрайта %i",
             thing->sprite);
#endif
    sprdef = &sprites[thing->sprite];
#ifdef RANGECHECK
    if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes )
    I_Error (english_language ?
             "R_ProjectSprite: invalid sprite frame %i : %i " :
             "R_ProjectSprite: некорректный фрейм спрайта %i : %i ",
             thing->sprite, thing->frame);
#endif
    sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        ang = R_PointToAngle (interpx, interpy);

        // [crispy] now made non-fatal
        if (sprframe->rotate == -1)
        {
            return;
        }
        else if (sprframe->rotate == 2) // [crispy] support 16 sprite rotations
        {
            const unsigned rot2 = (ang-interpangle+(unsigned)(ANG45/4)*17);
            rot = (rot2>>29) + ((rot2>>25)&8);
        }
        else
        {
            rot = (ang-interpangle+(unsigned)(ANG45/2)*9)>>29;
        }

        lump = sprframe->lump[rot];
        flip = (boolean)sprframe->flip[rot];
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = (boolean)sprframe->flip[0];
    }

    // [crispy] flip death sprites and corpses randomly
    if (randomly_flipcorpses && !vanillaparm)
    {
        if ((thing->flags & MF_CORPSE
        && thing->type != MT_CYBORG
        && thing->type != MT_BARREL
        && thing->type != MT_BOSSBRAIN)
        || thing->info->spawnstate == S_PLAY_DIE7
        || thing->info->spawnstate == S_PLAY_XDIE9)
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
    x1 = (centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS;

    tx +=  spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS) - 1;

    gzt = interpz + spritetopoffset[lump];

    // off the side?
    if (x1 > viewwidth || x2 < 0)
    {
        return;
    }

    // [JN] killough 4/9/98: clip things which are out of view due to height
    if (interpz > viewz + FixedDiv(viewheight << FRACBITS, xscale)
    ||  gzt     < (int64_t)viewz - FixedDiv((viewheight << FRACBITS)-viewheight, xscale))
    {
        return;
    }

    // [JN] quickly reject sprites with bad x ranges
    if (x1 >= x2)
    {
        return;
    }

    // store information in a vissprite
    vis = R_NewVisSprite ();
    vis->translation = NULL;
    vis->mobjflags = thing->flags;
    vis->scale = xscale<<(detailshift && !hires);
    vis->gx = interpx;
    vis->gy = interpy;
    vis->gz = interpz;
    vis->gzt = gzt;  // [JN] killough 3/27/98
    vis->texturemid = gzt - viewz;
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    iscale = FixedDiv (FRACUNIT, xscale);

    if (flip)
    {
        vis->startfrac = spritewidth[lump]-1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
    {
        vis->startfrac += vis->xiscale*(vis->x1-x1);
    }
    vis->patch = lump;

    // get light level
    // [JN] improved_fuzz == 4 is a translucent fuzz effect,
    // must use diminished lighting.
    if (thing->flags & MF_SHADOW && improved_fuzz < 4)
    {
        // shadow draw
        vis->colormap[0] = vis->colormap[1] = NULL;
    }
    else if (fixedcolormap)
    {
        // fixed map
        vis->colormap[0] = vis->colormap[1] = fixedcolormap;
    }
    else if (thing->frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap[0] = vis->colormap[1] = colormaps;
    }
    else
    {
        // diminished light
        index = xscale >> (LIGHTSCALESHIFT-detailshift+hires);

        if (index >= MAXLIGHTSCALE)
        {
            index = MAXLIGHTSCALE-1;
        }

        // [crispy] brightmaps for select sprites
        vis->colormap[0] = spritelights[index];

        // [JN] Apply different types half-brights for certain objects.
        //  Not to be confused:
        //   * Semi-bright. Lits up brightmapped pixels with non-full power.
        //     If sector brightness < 96, apply semi-bright. Otherwise, 
        //     use standard diminished lighting.
        //   * Demi-bright. Lits up brightmapped pixels with full power,
        //     and non-brightmapped pixels with distance index miltipled by 2.
        //   * Hemi-bright. Lits up brightmapped pixels with full power,
        //     and non-brightmapped pixels with distance index miltipled by 4.
        
        // Semi-brigths:
        if (thing->sprite == SPR_BON2   // Armor Bonus
        ||  thing->sprite == SPR_BAR1)  // Explosive Barrel
        {
            int semi_bright = index;

            if (semi_bright < MINBRIGHT)
            {
                semi_bright = MINBRIGHT;
            }
            vis->colormap[1] = lightnum < 6 ? &colormaps[MINBRIGHT*256] :
                                              spritelights[semi_bright];
        }
        // Demi-brigths:
        else
        if (thing->sprite == SPR_CAND   // Candestick
        ||  thing->sprite == SPR_CBRA   // Candelabra
        ||  thing->sprite == SPR_COLU   // Floor Lamp
        ||  thing->sprite == SPR_FCAN   // Flaming Barrel
        ||  thing->sprite == SPR_TLMP   // Tall Tech Lamp
        ||  thing->sprite == SPR_TLP2)  // Short Tech Lamp
        {
            int demi_bright = index*2;
            
            // Apply some extra randomness and prevent too fast animation.
            if (can_animate_bmap)
            {
                vis->brightmap_anim = brightmap_anim;
            }

            if (demi_bright > 47)
            {
                demi_bright = 47;
            }
            vis->colormap[0] = spritelights[demi_bright];
            
            // Animated brightmaps:
            if (thing->sprite == SPR_CAND   // Candestick
            ||  thing->sprite == SPR_CBRA)  // Candelabra
            {
                vis->colormap[1] = &colormaps[vis->brightmap_anim*256];
            }
            else
            {
                vis->colormap[1] = colormaps;
            }
        }
        // Hemi-brigths:
        else
        if (thing->sprite == SPR_TBLU   // Tall Blue Torch
        ||  thing->sprite == SPR_TGRN   // Tall Green Torch
        ||  thing->sprite == SPR_TRED)  // Tall Red Torch
        {
            int hemi_bright = index*4;
            
            // Apply some extra randomness and prevent too fast animation.
            // Animation amplitude is lower here (/3).
            if (can_animate_bmap)
            {
                vis->brightmap_anim = brightmap_anim;
            }

            if (hemi_bright > 47)
            {
                hemi_bright = 47;
            }
            vis->colormap[0] = spritelights[hemi_bright];
            vis->colormap[1] = &colormaps[vis->brightmap_anim/3*256];
        }
        // Just animated
        else
        if (thing->sprite == SPR_SMBT   // Short Blue Torch
        ||  thing->sprite == SPR_SMGT   // Short Green Torch
        ||  thing->sprite == SPR_SMRT)  // Short Red Torch
        {
            // Apply some extra randomness and prevent too fast animation.
            // Animation amplitude is lower here (/4).
            if (can_animate_bmap)
            {
                vis->brightmap_anim = brightmap_anim;
            }

            vis->colormap[0] = vis->colormap[1] = &colormaps[vis->brightmap_anim/4*256];
        }
        else
        if (thing->sprite == SPR_CEYE)  // Evil Eye
        {
            if (can_animate_bmap)
            {
                vis->brightmap_anim = brightmap_anim;
            }

            vis->colormap[0] = vis->colormap[1] = &colormaps[(vis->brightmap_anim/2)*256];
        }
        else
        if (thing->sprite == SPR_POL3)  // Pile of Skulls and Candles
        {
            if (can_animate_bmap)
            {
                vis->brightmap_anim = brightmap_anim;
            }

            vis->colormap[0] = vis->colormap[1] = &colormaps[(vis->brightmap_anim/3)*256];
        }
        // Normal brightmap:
        else
        {
            vis->colormap[1] = colormaps;
        }
    }

    vis->brightmap = R_BrightmapForSprite(thing->sprite);

    // [crispy] colored blood
    if (colored_blood && !vanillaparm
    && (thing->type == MT_BLOOD || thing->state - states == S_GIBS)
    && thing->target)
    { 
        if (thing->target->type == MT_HEAD)
        {
            vis->translation =  cr[CR_RED2BLUE];
        }
        else if (thing->target->type == MT_BRUISER
        || thing->target->type == MT_KNIGHT)
        {
            vis->translation = cr[CR_RED2GREEN];
        }
    }
}

// -----------------------------------------------------------------------------
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
// -----------------------------------------------------------------------------

void R_AddSprites (const sector_t *sec)
{
    const mobj_t *thing;
    const int lightnum = (sec->lightlevel >> LIGHTSEGSHIFT)+extralight;

    if (lightnum < 0)		
    {
        spritelights = scalelight[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        spritelights = scalelight[LIGHTLEVELS-1];
    }
    else
    {
        spritelights = scalelight[lightnum];
    }

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
    {
        R_ProjectSprite (thing, lightnum);
    }
}

// -----------------------------------------------------------------------------
// R_ApplyWeaponBob
// [crispy] apply bobbing (or centering) to the player's weapon sprite
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// R_DrawPSprite
// -----------------------------------------------------------------------------

static void R_DrawPSprite (const pspdef_t *psp)
{
    int             x1, x2;
    int             lump;
    fixed_t         tx;
    fixed_t         psp_sx = psp->sx, psp_sy = psp->sy;
    spritedef_t    *sprdef;
    spriteframe_t  *sprframe;
    vissprite_t    *vis;
    vissprite_t     avis;
    boolean         flip;

    const int state = viewplayer->psprites[ps_weapon].state - states;       // [crispy]
    const weaponinfo_t *const winfo = &weaponinfo[viewplayer->readyweapon]; // [crispy]

    // decide which patch to use
#ifdef RANGECHECK
    if ( (unsigned)psp->state->sprite >= (unsigned int) numsprites)
    I_Error (english_language ?
             "R_DrawPSprite: invalid sprite number %i " :
             "R_DrawPSprite: некорректный номер спрайта %i ",
             psp->state->sprite);
#endif
    sprdef = &sprites[psp->state->sprite];
#ifdef RANGECHECK
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
    I_Error (english_language ?
             "R_DrawPSprite: invalid sprite frame %i : %i " :
             "R_DrawPSprite: некорректный фрейм спрайта %i : %i ",
             psp->state->sprite, psp->state->frame);
#endif
    sprframe = &sprdef->spriteframes[ psp->state->frame & FF_FRAMEMASK ];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0] ^ flip_levels ^ flip_weapons;

    if (weapon_bobbing && !vanillaparm)
    {
        // [JN] Always apply bobbing for all states...
        if (state != winfo->downstate && state != winfo->upstate)
        {
            R_ApplyWeaponBob(&psp_sx, true, &psp_sy, true);
        }
        // [JN] ...except X-bobbing only for raising and lowering states.
        else
        {
            R_ApplyWeaponBob(&psp_sx, true, 0, false);
        }

        // [crispy] squat down weapon sprite a bit after hitting the ground
        psp_sy += abs(viewplayer->psp_dy);
    }

    // calculate edges of the shape
    tx = psp_sx-(ORIGWIDTH/2)*FRACUNIT;

    // [crispy] fix sprite offsets for mirrored sprites
    tx -= flip ? 2 * tx - spriteoffset[lump] + spritewidth[lump] : spriteoffset[lump];
    x1 = (centerxfrac + FixedMul (tx,pspritescale) ) >>FRACBITS;

    // off the right side
    if (x1 > viewwidth)
    {
        return;
    }

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
    {
        return;
    }

    // store information in a vissprite
    vis = &avis;
    vis->translation = NULL;
    vis->mobjflags = 0;

    // [crispy] weapons drawn 1 pixel too high when player is idle
    // [JN] Jaguar weapon placement: 10 px higher above STBAR, not in full screen mode.
    if (gamemission == jaguar && screenblocks <= 10)
    {
        vis->texturemid = (BASEYCENTER << FRACBITS) + FRACUNIT/4 
                        + FRACUNIT*10 - (psp_sy-spritetopoffset[lump]);
    }
    else
    {
        vis->texturemid = (BASEYCENTER << FRACBITS) + FRACUNIT/4
                        - (psp_sy-spritetopoffset[lump]);
    }

    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    vis->scale = pspritescale<<(detailshift && !hires);

    if (flip)
    {
        vis->xiscale = -pspriteiscale;
        vis->startfrac = spritewidth[lump]-1;
    }
    else
    {
        vis->xiscale = pspriteiscale;
        vis->startfrac = 0;
    }

    if (vis->x1 > x1)
    {
        vis->startfrac += vis->xiscale*(vis->x1-x1);
    }

    vis->patch = lump;

    if (viewplayer->powers[pw_invisibility] > 4*32 || viewplayer->powers[pw_invisibility] & 8)
    {
        if (gamemode == pressbeta)
        {
            // [JN] Press Beta: always use inverted palette for Partial Invisibility
            vis->colormap[0] = vis->colormap[1] = fixedcolormap ? fixedcolormap : spritelights[MAXLIGHTSCALE-1];
        }
        else
        {
            if (improved_fuzz < 4)
            {
                // shadow draw
                vis->colormap[0] = vis->colormap[1] = NULL;
            }
            else
            {
                // [JN] Translucent fuzz effect.
                vis->colormap[0] = vis->colormap[1] = fixedcolormap ? fixedcolormap : spritelights[MAXLIGHTSCALE-1];
                vis->mobjflags |= MF_SHADOW;
            }
        }
    }
    else if (fixedcolormap)
    {
        // fixed color
        vis->colormap[0] = vis->colormap[1] = fixedcolormap;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap[0] = vis->colormap[1] = colormaps;
    }
    else
    {
        // local light
        vis->colormap[0] = spritelights[MAXLIGHTSCALE-1];
        vis->colormap[1] = colormaps;
    }
    
    vis->brightmap = R_BrightmapForState(psp->state - states);
	
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

    // [JN] Mouselook: also move HUD weapons while mouse look
    vis->texturemid += FixedMul(((centery - viewheight / 2) << FRACBITS), pspriteiscale);

    R_DrawVisSprite (vis, vis->x1, vis->x2);
}

// -----------------------------------------------------------------------------
// R_DrawPlayerSprites
// -----------------------------------------------------------------------------

static void R_DrawPlayerSprites (void)
{
    int i;
    const pspdef_t *psp;
    // get light level
    const int lightnum = (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) + extralight;

    if (lightnum < 0)		
    {
        spritelights = scalelight[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        spritelights = scalelight[LIGHTLEVELS-1];
    }
    else
    {
        spritelights = scalelight[lightnum];
    }

    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

    // add all active psprites
    for (i = 0, psp = viewplayer->psprites ; i < NUMPSPRITES ; i++, psp++)
    {
        if (psp->state)
        {
            R_DrawPSprite (psp);
        }
    }
}

// -----------------------------------------------------------------------------
// R_SortVisSprites
//
// Rewritten by Lee Killough to avoid using unnecessary
// linked lists, and to use faster sorting algorithm.
// -----------------------------------------------------------------------------

#define bcopyp(d, s, n) memcpy(d, s, (n) * sizeof(void *))

// killough 9/2/98: merge sort

static void msort(vissprite_t **s, vissprite_t **t, const int n)
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

// -----------------------------------------------------------------------------
// R_SortVisSprites
// -----------------------------------------------------------------------------

static void R_SortVisSprites (void)
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

        while (--i >= 0)
        {
            vissprite_ptrs[i] = vissprites+i;
        }

        // killough 9/22/98: replace qsort with merge sort, since the keys
        // are roughly in order to begin with, due to BSP rendering.

        msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

// -------------------------------------------------------------------------
// R_DrawSprite
// -------------------------------------------------------------------------

static void R_DrawSprite (const vissprite_t *spr)
{
    int x, r1, r2;
    drawseg_t *ds;
    fixed_t scale, lowscale;

    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
        clipbot[x] = cliptop[x] = -2;
    }

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.

    // Modified by Lee Killough:
    // (pointer check was originally nonportable
    // and buggy, by going past LEFT end of array):

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
            continue;      // does not cover sprite
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
            if (ds->maskedtexturecol)       // masked mid texture?
            {
                r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
                r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;
                R_RenderMaskedSegRange(ds, r1, r2);
            }
            continue;               // seg is behind sprite
        }
    
        r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
        r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;
    
        // clip this piece of the sprite
        // killough 3/27/98: optimized and made much shorter
    
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

// -------------------------------------------------------------------------
//
// R_DrawMasked
//
// -------------------------------------------------------------------------

void R_DrawMasked (void)
{
    int        i;
    const int  cx = screenwidth / 2;
    drawseg_t *ds;

    R_SortVisSprites();

    // [JN] e6y
    // Makes sense for scenes with huge amount of drawsegs.
    // ~12% of speed improvement on epic.wad map05
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

                // [JN] e6y: ~13% of speed improvement on sunder.wad map10
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

    // draw all vissprites back to front

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

        R_DrawSprite(vissprite_ptrs[i]);    // [JN] killough
    }

    // render any remaining masked mid textures

    // Modified by Lee Killough:
    // (pointer check was originally nonportable
    // and buggy, by going past LEFT end of array):
    for (ds = ds_p ; ds-- > drawsegs ; )
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange (ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    //  but does not draw on side views
    if (!viewangleoffset)
    {
        R_DrawPlayerSprites ();
    }
}
