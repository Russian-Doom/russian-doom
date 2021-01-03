//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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
//	Rendering main loop and setup functions,
//	 utility functions (BSP, geometry, trigonometry).
//	See tables.c, too.
//


#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_net.h"
#include "m_misc.h"
#include "r_local.h"
#include "r_segs.h"
#include "p_local.h"
#include "jn.h"


// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048	
#define DISTMAP     2

void (*colfunc) (void);
void (*basecolfunc) (void);
void (*fuzzcolfunc) (void);
void (*transcolfunc) (void);
void (*tlcolfunc) (void);
void (*spanfunc) (void);


int         viewangleoffset;

// increment every time a check is made
int         validcount = 1;		

// [JN] Screen size and detail (0 = high, 1 = low)
int         detailshift;
int         setblocks;
int         setdetail;
boolean     setsizeneeded;

int         centerx;
int         centery;

// [JN] Define, which diminished lighting we well use
int         maxlightz;
int         lightzshift;

fixed_t     centerxfrac;
fixed_t     centeryfrac;
fixed_t     projection;

// just for profiling purposes
int         framecount;	

int         sscount;
int         linecount;
int         loopcount;

// bumped light from gun blasts
int         extralight;

fixed_t     viewx;
fixed_t     viewy;
fixed_t     viewz;

angle_t     viewangle;

fixed_t     viewcos;
fixed_t     viewsin;
fixed_t    *finecosine = &finesine[FINEANGLES/4];

player_t        *viewplayer;
lighttable_t    *fixedcolormap;


//
// precalculated math tables
//
angle_t     clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X. 
int         viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t     xtoviewangle[SCREENWIDTH+1];


lighttable_t   *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *scalelightfixed[MAXLIGHTSCALE];
lighttable_t   *zlight[LIGHTLEVELS][MAXLIGHTZ];

// [JN] Floor brightmaps
lighttable_t   *fullbright_notgrayorbrown_floor[LIGHTLEVELS][MAXLIGHTZ];

// [JN] Wall and thing brightmaps
lighttable_t   *fullbright_redonly[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_notgray[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_notgrayorbrown[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_greenonly1[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_greenonly2[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_greenonly3[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_orangeyellow[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_dimmeditems[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_brighttan[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_redonly1[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_explosivebarrel[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_alllights[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_candles[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_pileofskulls[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t   *fullbright_redonly2[LIGHTLEVELS][MAXLIGHTSCALE];

//
// sky mapping
//
int         skyflatnum;
int         skytexture;
int         skytexturemid;


//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
// killough 5/2/98: reformatted
//
int R_PointOnSide (fixed_t x, fixed_t y, node_t *node)
{
    if (!node->dx)
    {
        return x <= node->x ? node->dy > 0 : node->dy < 0;
    }

    if (!node->dy)
    {
        return y <= node->y ? node->dx < 0 : node->dx > 0;
    }

    x -= node->x;
    y -= node->y;

    // Try to quickly decide by looking at sign bits.
    if ((node->dy ^ node->dx ^ x ^ y) < 0)
    return (node->dy ^ x) < 0;  // (left is negative)

    return FixedMul(y, node->dx>>FRACBITS) >= FixedMul(node->dy>>FRACBITS, x);
}


// killough 5/2/98: reformatted
int R_PointOnSegSide (fixed_t x, fixed_t y, seg_t *line)
{
    fixed_t lx = line->v1->x;
    fixed_t ly = line->v1->y;
    fixed_t ldx = line->v2->x - lx;
    fixed_t ldy = line->v2->y - ly;

    if (!ldx)
    return x <= lx ? ldy > 0 : ldy < 0;

    if (!ldy)
    return y <= ly ? ldx < 0 : ldx > 0;

    x -= lx;
    y -= ly;

    // Try to quickly decide by looking at sign bits.
    if ((ldy ^ ldx ^ x ^ y) < 0)
    return (ldy ^ x) < 0;          // (left is negative)

    return FixedMul(y, ldx>>FRACBITS) >= FixedMul(ldy>>FRACBITS, x);
}


int SlopeDiv (unsigned int num, unsigned int den)
{
    int64_t ans;
    
    if (den < 512)
    {
        return SLOPERANGE;
    }
    else
    {
        ans = ((int64_t) num << 3) / (den >> 8);

        if (ans <= SLOPERANGE)
        {
            return (int) ans;
        }
        else
        {
            return SLOPERANGE;
        }
    }
}


//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table. The +1 size of tantoangle[]
//  is to handle the case when x==y without additional
//  checking.
//
// killough 5/2/98: reformatted, cleaned up
//
angle_t R_PointToAngle (fixed_t x, fixed_t y)
{       
  return (y -= viewy, (x -= viewx) || y) ?
    x >= 0 ?
      y >= 0 ? 
        (x > y) ? tantoangle[SlopeDiv(y,x)] :                      // octant 0 
                ANG90-1-tantoangle[SlopeDiv(x,y)] :                // octant 1
        x > (y = -y) ? -tantoangle[SlopeDiv(y,x)] :                // octant 8
                       ANG270+tantoangle[SlopeDiv(x,y)] :          // octant 7
      y >= 0 ? (x = -x) > y ? ANG180-1-tantoangle[SlopeDiv(y,x)] : // octant 3
                            ANG90 + tantoangle[SlopeDiv(x,y)] :    // octant 2
        (x = -x) > (y = -y) ? ANG180+tantoangle[ SlopeDiv(y,x)] :  // octant 4
                              ANG270-1-tantoangle[SlopeDiv(x,y)] : // octant 5
    0;
}


angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    viewx = x1;
    viewy = y1;

    return R_PointToAngle (x2, y2);
}


// killough 5/2/98: simplified
fixed_t R_PointToDist (fixed_t x, fixed_t y)
{
    fixed_t dx = abs(x - viewx);
    fixed_t dy = abs(y - viewy);

    if (dy > dx)
    {
        fixed_t t = dx;
        dx = dy;
        dy = t;
    }

    return dx ? FixedDiv(dx, finesine[(tantoangle[FixedDiv(dy,dx) >> DBITS]
                        + ANG90) >> ANGLETOFINESHIFT]) : 0;
}


//
// R_InitTextureMapping
//
// killough 5/2/98: reformatted
static void R_InitTextureMapping (void)
{
    int      i, x;
    fixed_t  focallength;

    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.
    //
    // Calc focallength
    //  so FIELDOFVIEW angles covers SCREENWIDTH.

    focallength = FixedDiv(centerxfrac, finetangent[FINEANGLES/4+FIELDOFVIEW/2]);

    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
        int t;

        if (finetangent[i] > FRACUNIT*2)
        {
            t = -1;
        }
        else if (finetangent[i] < -FRACUNIT*2)
        {
            t = viewwidth+1;
        }
        else
        {
            t = FixedMul(finetangent[i], focallength);
            t = (centerxfrac - t + FRACUNIT-1) >> FRACBITS;

            if (t < -1)
            {
                t = -1;
            }
            else if (t > viewwidth+1)
            {
                t = viewwidth+1;
            }
        }

        viewangletox[i] = t;
    }
    
    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.

    for (x=0 ; x <= viewwidth ; x++)
    {
        for (i=0; viewangletox[i] > x; i++)
        ;
        xtoviewangle[x] = (i<<ANGLETOFINESHIFT)-ANG90;
    }

    // Take out the fencepost cases from viewangletox.
    for (i=0; i<FINEANGLES/2; i++)
        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == viewwidth+1)
            viewangletox[i] = viewwidth;

    clipangle = xtoviewangle[0];
}



//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
void R_InitLightTables (void)
{
    int i;
    int j;
    int level;
    int startmap; 	
    int scale;

    // [JN] Define, which diminished lighting to use
    lightzshift = vanilla ? LIGHTZSHIFT_VANILLA : LIGHTZSHIFT;
    maxlightz = vanilla ? MAXLIGHTZ_VANILLA : MAXLIGHTZ;

    // Calculate the light levels to use
    //  for each level / distance combination.
    for (i=0 ; i< LIGHTLEVELS ; i++)
    {
        startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;

        // [JN] No smoother diminished lighting in -vanilla mode
        for (j=0 ; j < maxlightz ; j++)
        {
            scale = FixedDiv ((SCREENWIDTH/2*FRACUNIT), ((j+1)<<lightzshift));
            scale >>= LIGHTSCALESHIFT;
            level = startmap - scale/DISTMAP;

            if (level < 0)
            level = 0;

            if (level >= NUMCOLORMAPS)
            level = NUMCOLORMAPS-1;

            zlight[i][j] = colormaps + level*256;

            // [JN] Floor brightmaps
            fullbright_notgrayorbrown_floor[i][j] = brightmaps_notgrayorbrown + level * 256;
        }
    }
}


//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap (void)
{
    skyflatnum = R_FlatNumForName(SKYFLATNAME);
    skytexturemid = 100*FRACUNIT;
}


//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
void R_SetViewSize (int blocks, int detail)
{
    setsizeneeded = true;
    setblocks = blocks;
    setdetail = detail;
}


//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize (void)
{
    int     i;
    int     j;
    int     level;
    int     startmap;
    fixed_t cosadj;
    fixed_t dy;

    setsizeneeded = false;

    // [JN] Crispy HUD screen sizes is also "full screen"
    if (setblocks >= 11 && setblocks <= 14)
    {
        scaledviewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT;
    }
    else
    {
        scaledviewwidth = setblocks*32;
        viewheight = (setblocks*168/10)&~7;
    }

    detailshift = setdetail;
    viewwidth = scaledviewwidth>>detailshift;

    centery = viewheight/2;
    centerx = viewwidth/2;
    centerxfrac = centerx<<FRACBITS;
    centeryfrac = centery<<FRACBITS;
    projection = centerxfrac;

    if (!detailshift)
    {
        colfunc = basecolfunc = R_DrawColumn;
        fuzzcolfunc = R_DrawFuzzColumn;
        transcolfunc = R_DrawTranslatedColumn;
        tlcolfunc = R_DrawTLColumn;
        spanfunc = noflats ? R_DrawSpanNoTexture : R_DrawSpan;
    }
    else
    {
        colfunc = basecolfunc = R_DrawColumnLow;
        fuzzcolfunc = R_DrawFuzzColumnLow;
        transcolfunc = R_DrawTranslatedColumnLow;
        tlcolfunc = R_DrawTLColumnLow;
        spanfunc = noflats ? R_DrawSpanLowNoTexture : R_DrawSpanLow;
    }

    R_InitBuffer (scaledviewwidth, viewheight);
    R_InitTextureMapping ();

    // psprite scales
    pspritescale = FRACUNIT*viewwidth/SCREENWIDTH;
    pspriteiscale = FRACUNIT*SCREENWIDTH/viewwidth;

    // thing clipping
    for (i=0 ; i<viewwidth ; i++)
    screenheightarray[i] = viewheight;

    // planes
    for (i=0 ; i<viewheight ; i++)
    {
        const fixed_t num = (viewwidth<<detailshift)/2*FRACUNIT;

        for (j = 0; j < LOOKDIRS; j++)
        {
            dy = ((i-(viewheight/2 + ((j-LOOKDIRMIN) << (!detailshift))
               * (screenblocks < 11 ? screenblocks : 11) / 10))<<FRACBITS)+FRACUNIT/2;

            dy = abs(dy);
            yslopes[j][i] = FixedDiv (num, dy);
        }
    }
    yslope = yslopes[LOOKDIRMIN];

    for (i=0 ; i<viewwidth ; i++)
    {
        cosadj = abs(finecosine[xtoviewangle[i]>>ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv (FRACUNIT,cosadj);
    }

    // Calculate the light levels to use
    //  for each level / scale combination.
    for (i=0 ; i< LIGHTLEVELS ; i++)
    {
        startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;

        for (j=0 ; j<MAXLIGHTSCALE ; j++)
        {
            level = startmap - j*SCREENWIDTH/(viewwidth<<detailshift)/DISTMAP;

            if (level < 0)
            level = 0;

            if (level >= NUMCOLORMAPS)
            level = NUMCOLORMAPS-1;

            scalelight[i][j] = colormaps + level*256;

            // [JN] Brightmaps
            fullbright_redonly[i][j] = brightmaps_redonly + level*256;
            fullbright_notgray[i][j] = brightmaps_notgray + level*256;
            fullbright_notgrayorbrown[i][j] = brightmaps_notgrayorbrown + level*256;
            fullbright_greenonly1[i][j] = brightmaps_greenonly1 + level*256;
            fullbright_greenonly2[i][j] = brightmaps_greenonly2 + level*256;
            fullbright_greenonly3[i][j] = brightmaps_greenonly3 + level*256;
            fullbright_orangeyellow[i][j] = brightmaps_orangeyellow + level*256;
            fullbright_dimmeditems[i][j] = brightmaps_dimmeditems + level*256;
            fullbright_brighttan[i][j] = brightmaps_brighttan + level*256;
            fullbright_redonly1[i][j] = brightmaps_redonly1 + level*256;
            fullbright_explosivebarrel[i][j] = brightmaps_explosivebarrel + level*256;
            fullbright_alllights[i][j] = brightmaps_alllights + level*256;
            fullbright_candles[i][j] = brightmaps_candles + level*256;
            fullbright_pileofskulls[i][j] = brightmaps_pileofskulls + level*256;
            fullbright_redonly2[i][j] = brightmaps_redonly2 + level*256;
        }
    }
}


//
// R_Init
//
void R_Init (void)
{
    R_InitData ();
    // [JN] Double dots because of removed R_InitPointToAngle and R_InitTables
    printf (".."); 

    R_SetViewSize (screenblocks, detailLevel);
    printf (".");
    R_InitLightTables ();
    printf (".");
    R_InitSkyMap ();
    printf (".");
    R_InitTranslationTables ();

    // [JN] Lookup and init all the textures for brightmapping
    if (!vanilla)
    {
        R_InitBrightmaps ();
    }

    framecount = 0;
}


//
// R_PointInSubsector
//
// killough 5/2/98: reformatted, cleaned up
//
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    int nodenum = numnodes-1;

    while (!(nodenum & NF_SUBSECTOR))
    nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];

    return &subsectors[nodenum & ~NF_SUBSECTOR];
}


//
// R_SetupFrame
//
void R_SetupFrame (player_t *player)
{		
    int i;
    int tempCentery;

    viewplayer = player;
    viewx = player->mo->x;
    viewy = player->mo->y;
    viewangle = player->mo->angle + viewangleoffset;
    extralight = player->extralight;

    viewz = player->viewz;

    // [JN] Mouse look: rendering routines, HIGH and LOW detail modes friendly.
    tempCentery = viewheight / 2 + (player->lookdir / MLOOKUNIT)
                * (screenblocks < 11 ? screenblocks : 11) / 10;

    if (centery != tempCentery)
    {
        centery = tempCentery;
        centeryfrac = centery << FRACBITS;

        for (i = 0; i < viewheight; i++)
        {
            yslope[i] = FixedDiv((viewwidth << detailshift) / 2
                      * FRACUNIT, abs(((i - centery) << FRACBITS) + FRACUNIT / 2));
        }
    }

    viewsin = finesine[viewangle>>ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle>>ANGLETOFINESHIFT];

    sscount = 0;

    if (player->fixedcolormap)
    {
        // [JN] Fix aftermath of "Invulnerability colormap bug" fix,
        // when sky texture was slightly affected by changing to
        // fixed (non-inversed) colormap.
        // https://doomwiki.org/wiki/Invulnerability_colormap_bug
        if (player->powers[pw_invulnerability])
        {
            fixedcolormap = colormaps + player->fixedcolormap 
                                      * 256 * sizeof(lighttable_t);
        }
        else
        {
            fixedcolormap = colormaps;
        }

        walllights = scalelightfixed;
        walllights_top = scalelightfixed;
        walllights_middle = scalelightfixed;
        walllights_bottom = scalelightfixed;

        for (i=0 ; i<MAXLIGHTSCALE ; i++)
        scalelightfixed[i] = fixedcolormap;
    }
    else
    {
        fixedcolormap = 0;
    }

    framecount++;
    validcount++;
    destview = destscreen + (viewwindowy*SCREENWIDTH/4) + (viewwindowx >> 2);
}


//
// R_RenderView
//
void R_RenderPlayerView (player_t *player)
{	
    R_SetupFrame (player);

    // [JN] Don't render game screen while in help screens.
    if (inhelpscreens)
    {
        return;
    }

    // Clear buffers.
    R_ClearClipSegs();
    R_ClearDrawSegs();

    if (automapactive)
    {
        R_RenderBSPNode (numnodes-1);
        return;
    }

    R_ClearPlanes();
    R_ClearSprites();

    // check for new console commands.
    NetUpdate();

    // The head node is the last node output.
    R_RenderBSPNode(numnodes-1);

    // Check for new console commands.
    NetUpdate ();

    R_DrawPlanes ();

    // Check for new console commands.
    NetUpdate ();

    // [crispy] draw fuzz effect independent of rendering frame rate
    // [JN] Continue fuzz animation in paused states in -vanilla mode
    if (!vanilla)
    {
        R_SetFuzzPosDraw();
    }

    R_DrawMasked ();

    // Check for new console commands.
    NetUpdate ();				
}

