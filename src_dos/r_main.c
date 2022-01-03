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
#include "z_zone.h"
#include "jn.h"


// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048	


int           detailshift;     // 0 = high, 1 = low
int           viewangleoffset;
int           validcount = 1;  // increment every time a check is made
int           centerx, centery;
fixed_t       centerxfrac, centeryfrac;
fixed_t       projection;
fixed_t       viewx, viewy, viewz;
angle_t       viewangle;
fixed_t       viewcos, viewsin;
player_t     *viewplayer;
lighttable_t *fixedcolormap;

int     setblocks, setdetail;
boolean setsizeneeded;

int extralight;              // bumped light from gun blasts
int maxlightz, lightzshift;  // [JN] Smooth and vanilla diminished lighting

//
// precalculated math tables
//
angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X. 
int viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
// [JN] e6y: resolution limitation is removed
angle_t *xtoviewangle;  // killough 2/8/98

// [crispy] calculate the linear sky angle component here
// [JN] resolution limitation is removed
angle_t *linearskyangle;

lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *scalelightfixed[MAXLIGHTSCALE];
lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

// [JN] Floor brightmaps
lighttable_t *fullbright_notgrayorbrown_floor[LIGHTLEVELS][MAXLIGHTZ];

// [JN] Wall and thing brightmaps
lighttable_t *fullbright_redonly[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_notgray[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_notgrayorbrown[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_greenonly1[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_greenonly2[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_greenonly3[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_orangeyellow[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_dimmeditems[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_brighttan[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_redonly1[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_explosivebarrel[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_alllights[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_candles[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_pileofskulls[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_redonly2[LIGHTLEVELS][MAXLIGHTSCALE];

// sky mapping
int skyflatnum, skytexture, skytexturemid;

void (*colfunc) (void);
void (*basecolfunc) (void);
void (*fuzzcolfunc) (void);
void (*transcolfunc) (void);
void (*tlcolfunc) (void);
void (*spanfunc) (void);


// -----------------------------------------------------------------------------
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
// [JN] killough 5/2/98: reformatted
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// R_PointOnSegSide
// [JN] killough 5/2/98: reformatted
// -----------------------------------------------------------------------------

int R_PointOnSegSide (fixed_t x, fixed_t y, seg_t *line)
{
    fixed_t lx = line->v1->x;
    fixed_t ly = line->v1->y;
    fixed_t ldx = line->v2->x - lx;
    fixed_t ldy = line->v2->y - ly;

    if (!ldx)
    {
        return x <= lx ? ldy > 0 : ldy < 0;
    }

    if (!ldy)
    {
        return y <= ly ? ldx < 0 : ldx > 0;
    }

    x -= lx;
    y -= ly;

    // Try to quickly decide by looking at sign bits.
    if ((ldy ^ ldx ^ x ^ y) < 0)
    {
        return (ldy ^ x) < 0;  // (left is negative)
    }

    return FixedMul(y, ldx>>FRACBITS) >= FixedMul(ldy>>FRACBITS, x);
}

// -----------------------------------------------------------------------------
// R_PointToAngle
// To get a global angle from cartesian coordinates, the coordinates are 
// flipped until they are in the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a tangent (slope) value 
// which is looked up in the tantoangle[] table.
//
// [crispy] turned into a general R_PointToAngle() flavor
// called with either slope_div = SlopeDivCrispy() from R_PointToAngleCrispy()
// or slope_div = SlopeDiv() else
// -----------------------------------------------------------------------------

angle_t R_PointToAngleSlope (fixed_t x, fixed_t y,
                             int (*slope_div)(unsigned int num, unsigned int den))
{	
    x -= viewx;
    y -= viewy;
    
    if ((!x) && (!y))
    {
        return 0;
    }

    if (x>= 0)
    {
        // x >=0
        if (y>= 0)
        {
            // y>= 0
            if (x>y)
            {
                // octant 0
                return tantoangle[slope_div(y,x)];
            }
            else
            {
                // octant 1
                return ANG90-1-tantoangle[slope_div(x,y)];
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 8
                return -tantoangle[slope_div(y,x)];
            }
            else
            {
                // octant 7
                return ANG270+tantoangle[slope_div(x,y)];
            }
        }
    }
    else
    {
        // x<0
        x = -x;

        if (y>= 0)
        {
            // y>= 0
            if (x>y)
            {
                // octant 3
                return ANG180-1-tantoangle[slope_div(y,x)];
            }
            else
            {
                // octant 2
                return ANG90 + tantoangle[slope_div(x,y)];
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 4
                return ANG180+tantoangle[slope_div(y,x)];
            }
            else
            {
                // octant 5
                return ANG270-1-tantoangle[slope_div(x,y)];
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------
// R_PointToAngle
// -----------------------------------------------------------------------------

angle_t R_PointToAngle (fixed_t x, fixed_t y)
{
    return R_PointToAngleSlope (x, y, SlopeDiv);
}

// -----------------------------------------------------------------------------
// R_PointToAngleCrispy
// [crispy] overflow-safe R_PointToAngle() flavor
// called only from R_CheckBBox(), R_AddLine() and P_SegLengths()
// -----------------------------------------------------------------------------

angle_t R_PointToAngleCrispy (fixed_t x, fixed_t y)
{
    // [crispy] fix overflows for very long distances
    int64_t y_viewy = (int64_t)y - viewy;
    int64_t x_viewx = (int64_t)x - viewx;

    // [crispy] the worst that could happen is e.g. INT_MIN-INT_MAX = 2*INT_MIN
    if (x_viewx < INT_MIN || x_viewx > INT_MAX
    ||  y_viewy < INT_MIN || y_viewy > INT_MAX)
    {
        // [crispy] preserving the angle by halfing the distance in both directions
        x = x_viewx / 2 + viewx;
        y = y_viewy / 2 + viewy;
    }

    return R_PointToAngleSlope (x, y, SlopeDivCrispy);
}

// -----------------------------------------------------------------------------
// R_PointToAngle2
// -----------------------------------------------------------------------------

angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{	
    viewx = x1;
    viewy = y1;

    // [crispy] R_PointToAngle2() is never called during rendering
    return R_PointToAngleSlope (x2, y2, SlopeDiv);
}

// -----------------------------------------------------------------------------
// R_PointToDist
// -----------------------------------------------------------------------------

fixed_t R_PointToDist (fixed_t x, fixed_t y)
{
    int     angle;
    fixed_t dx, dy;
    fixed_t temp, dist, frac;

    dx = abs(x - viewx);
    dy = abs(y - viewy);

    if (dy>dx)
    {
        temp = dx;
        dx = dy;
        dy = temp;
    }

    // Fix crashes in udm1.wad

    if (dx != 0)
    {
        frac = FixedDiv(dy, dx);
    }
    else
    {
        frac = 0;
    }

    angle = (tantoangle[frac>>DBITS]+ANG90) >> ANGLETOFINESHIFT;

    // use as cosine
    dist = FixedDiv (dx, finesine[angle] );	

    return dist;
}

// -----------------------------------------------------------------------------
// R_InitTextureMapping
// -----------------------------------------------------------------------------

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

    for (i = 0 ; i < FINEANGLES/2 ; i++)
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

    for (x = 0 ; x <= viewwidth ; x++)
    {
        i = 0;

        while (viewangletox[i]>x)
        {
            i++;
        }

        xtoviewangle[x] = (i<<ANGLETOFINESHIFT)-ANG90;
        // [crispy] calculate sky angle for drawing horizontally linear skies.
        // Taken from GZDoom and refactored for integer math.
        linearskyangle[x] = ((viewwidth / 2 - x) * ((SCREENWIDTH<<6) / viewwidth)) 
                                                 * (ANG90 / (SCREENWIDTH<<6));
    }

    // Take out the fencepost cases from viewangletox.
    for (i = 0; i < FINEANGLES/2; i++)
    {
        if (viewangletox[i] == -1)
        {
            viewangletox[i] = 0;
        }
        else if (viewangletox[i] == viewwidth+1)
        {
            viewangletox[i] = viewwidth;
        }
    }

    clipangle = xtoviewangle[0];
}

// -----------------------------------------------------------------------------
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
// -----------------------------------------------------------------------------

#define DISTMAP 2

static void R_InitLightTables (void)
{
    int i, j;
    int level;
    int scale;

    // [JN] Define, which diminished lighting to use
    lightzshift = vanilla ? LIGHTZSHIFT_VANILLA : LIGHTZSHIFT;
    maxlightz = vanilla ? MAXLIGHTZ_VANILLA : MAXLIGHTZ;

    // Calculate the light levels to use
    //  for each level / distance combination.
    for (i = 0 ; i < LIGHTLEVELS ; i++)
    {
        const int startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;

        // [JN] No smoother diminished lighting in -vanilla mode
        for (j = 0 ; j < maxlightz ; j++)
        {
            scale = FixedDiv ((SCREENWIDTH/2*FRACUNIT), ((j+1)<<lightzshift));
            scale >>= LIGHTSCALESHIFT;
            level = startmap - scale/DISTMAP;

            if (level < 0)
            {
                level = 0;
            }

            if (level >= NUMCOLORMAPS)
            {
                level = NUMCOLORMAPS-1;
            }

            zlight[i][j] = colormaps + level*256;

            // [JN] Floor brightmaps
            fullbright_notgrayorbrown_floor[i][j] = brightmaps_notgrayorbrown + level * 256;
        }
    }
}

// -----------------------------------------------------------------------------
// R_InitSkyMap
// Called at program startup.
// -----------------------------------------------------------------------------

static void R_InitSkyMap (void)
{
    skyflatnum = R_FlatNumForName(SKYFLATNAME);
    skytexturemid = 100*FRACUNIT;
}

// -----------------------------------------------------------------------------
// R_InitTranslationTables
// Creates the translation tables to map the green color ramp to gray, 
// brown, red. Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
// -----------------------------------------------------------------------------

static void R_InitTranslationTables (void)
{
    int i;

    translationtables = Z_Malloc (256*3+255, PU_STATIC, 0);
    translationtables = (byte *)(( (int)translationtables + 255 )& ~255);

    // translate just the 16 green colors
    for (i=0 ; i<256 ; i++)
    {
        if (i >= 0x70 && i<= 0x7f)
        {
            // map green ramp to gray, brown, red
            translationtables[i] = 0x60 + (i&0xf);
            translationtables [i+256] = 0x40 + (i&0xf);
            translationtables [i+512] = 0x20 + (i&0xf);
        }
        else
        {
            // Keep all other colors as is.
            translationtables[i] = translationtables[i+256] 
            = translationtables[i+512] = i;
        }
    }
}

// -----------------------------------------------------------------------------
// R_SetViewSize
// Do not really change anything here, because it might be in the 
// middle of a refresh. The change will take effect next refresh.
// -----------------------------------------------------------------------------

void R_SetViewSize (int blocks, int detail)
{
    setsizeneeded = true;
    setblocks = blocks;
    setdetail = detail;
}

// -----------------------------------------------------------------------------
// R_ExecuteSetViewSize
// -----------------------------------------------------------------------------

void R_ExecuteSetViewSize (void)
{
    int     i, j;
    int     level;
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
    for (i = 0 ; i < viewwidth ; i++)
    {
        screenheightarray[i] = viewheight;
    }

    // planes
    for (i = 0 ; i < viewheight ; i++)
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

    for (i = 0 ; i < viewwidth ; i++)
    {
        cosadj = abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv(FRACUNIT, cosadj);
    }

    // Calculate the light levels to use
    //  for each level / scale combination.
    for (i = 0 ; i < LIGHTLEVELS ; i++)
    {
        const int startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;

        for (j = 0 ; j < MAXLIGHTSCALE ; j++)
        {
            level = startmap - j*SCREENWIDTH/(viewwidth<<detailshift)/DISTMAP;

            if (level < 0)
            {
                level = 0;
            }
            if (level >= NUMCOLORMAPS)
            {
                level = NUMCOLORMAPS-1;
            }

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

// -----------------------------------------------------------------------------
// R_Init
// -----------------------------------------------------------------------------

void R_Init (void)
{
    R_InitClipSegs();
    R_InitPlanesRes ();
    R_InitSpritesRes ();
    R_InitVisplanesRes ();
    
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
}

// -----------------------------------------------------------------------------
// R_PointInSubsector
//
// [JN] killough 5/2/98: reformatted, cleaned up
// -----------------------------------------------------------------------------

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    int nodenum = numnodes-1;

    // special case for trivial maps (single subsector, no nodes)
    if (numnodes == 0)
    {
        return subsectors;
    }

    while (!(nodenum & NF_SUBSECTOR))
    {
        nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];
    }

    return &subsectors[nodenum & ~NF_SUBSECTOR];
}

// -----------------------------------------------------------------------------
// R_SetupFrame
// -----------------------------------------------------------------------------

static void R_SetupFrame (player_t *player)
{		
    int i;
    int tempCentery;

    viewplayer = player;
    viewx = player->mo->x;
    viewy = player->mo->y;
    viewz = player->viewz;
    viewangle = player->mo->angle + viewangleoffset;
    extralight = player->extralight;

    // [JN] Mouse look: rendering routines, HIGH and LOW detail modes friendly.
    tempCentery = viewheight / 2 + (player->lookdir / MLOOKUNIT)
                * (screenblocks < 11 ? screenblocks : 11) / 10;

    // [JN] TODO - upgrade to main version
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

        for (i = 0 ; i < MAXLIGHTSCALE ; i++)
        {
            scalelightfixed[i] = fixedcolormap;
        }
    }
    else
    {
        fixedcolormap = 0;
    }

    validcount++;
    destview = destscreen + (viewwindowy*SCREENWIDTH/4) + (viewwindowx >> 2);
}

// -----------------------------------------------------------------------------
// R_RenderView
// -----------------------------------------------------------------------------

void R_RenderPlayerView (player_t *player)
{	
    R_SetupFrame (player);

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
