//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


#pragma once

#include "i_video.h"
#include "v_patch.h"


/*
================================================================================

                              INTERNAL MAP TYPES

================================================================================
*/

typedef struct
{
    fixed_t x, y;
    // [crispy] remove slime trails
    // pseudovertexes are dummies that have their coordinates modified to get
    // moved towards the linedef associated with their seg by projecting them
    // using the law of cosines in p_setup.c:P_RemoveSlimeTrails();
    // they are *only* used in rendering
    fixed_t	px;
    fixed_t	py;
    boolean	moved;
} vertex_t;

struct line_s;

typedef struct
{
    fixed_t floorheight, ceilingheight;
    // [JN] Improved column clipping.
    fixed_t floor_xoffs, floor_yoffs;
    fixed_t ceiling_xoffs, ceiling_yoffs;
    int   floorlightsec, ceilinglightsec;
    short floorpic, ceilingpic;
    short lightlevel;
    short special, tag;

    int soundtraversed;         // 0 = untraversed, 1,2 = sndlines -1
    mobj_t *soundtarget;        // thing that made a sound (or null)

    int blockbox[4];            // mapblock bounding box for height changes
    degenmobj_t soundorg;       // for any sounds played by the sector

    int validcount;             // if == validcount, already checked
    mobj_t *thinglist;          // list of mobjs in sector
    void *specialdata;          // thinker_t for reversable actions
    int linecount;
    struct line_s **lines;      // [linecount] size

    // [crispy] WiggleFix: [kb] for R_FixWiggle()
    int cachedheight;
    int scaleindex;    

    // [AM] Previous position of floor and ceiling before
    //      think.  Used to interpolate between positions.
    fixed_t	oldfloorheight;
    fixed_t	oldceilingheight;

    // [AM] Gametic when the old positions were recorded.
    //      Has a dual purpose; it prevents movement thinkers
    //      from storing old positions twice in a tic, and
    //      prevents the renderer from attempting to interpolate
    //      if old values were not updated recently.
    int         oldgametic;

    // [AM] Interpolated floor and ceiling height.
    //      Calculated once per tic and used inside
    //      the renderer.
    fixed_t interpfloorheight;
    fixed_t interpceilingheight;
} sector_t;

typedef struct
{
    fixed_t textureoffset;      // add this to the calculated texture col
    fixed_t rowoffset;          // add this to the calculated texture top
    short toptexture, bottomtexture, midtexture;
    sector_t *sector;
    fixed_t oldtextureoffset;   // [crispy] smooth texture scrolling
} side_t;

typedef enum
{ ST_HORIZONTAL, ST_VERTICAL, ST_POSITIVE, ST_NEGATIVE } slopetype_t;

typedef struct line_s
{
    vertex_t *v1, *v2;
    fixed_t dx, dy;             // v2 - v1 for side checking
    unsigned short flags;
    short special, tag;
    // sidenum[1] will be -1 (NO_INDEX) if one sided
    unsigned short sidenum[2];
    fixed_t bbox[4];
    slopetype_t slopetype;      // to aid move clipping
    sector_t *frontsector, *backsector;
    int validcount;             // if == validcount, already checked
    void *specialdata;          // thinker_t for reversable actions
    // [JN] Improved column clipping.
    int r_validcount;   // cph: if == gametic, r_flags already done
    enum {              // cph:
    RF_TOP_TILE  = 1,   // Upper texture needs tiling
    RF_MID_TILE  = 2,   // Mid texture needs tiling
    RF_BOT_TILE  = 4,   // Lower texture needs tiling
    RF_IGNORE    = 8,   // Renderer can skip this line
    RF_CLOSED    = 16,  // Line blocks view
    } r_flags;
    // [crispy] calculate sound origin of line to be its midpoint
    degenmobj_t	soundorg;
} line_t;


typedef struct subsector_s
{
    sector_t *sector;
    int numlines;
    int firstline;
} subsector_t;

typedef struct
{
    vertex_t *v1, *v2;
    fixed_t offset;
    angle_t angle;
    side_t *sidedef;
    line_t *linedef;
    sector_t *frontsector;
    sector_t *backsector;       // NULL for one sided lines

    uint32_t length;   // [crispy] fix long wall wobble
    angle_t  r_angle;  // [crispy] re-calculated angle used for rendering
} seg_t;

typedef struct
{
    fixed_t x, y, dx, dy;       // partition line
    fixed_t bbox[2][4];         // bounding box for each child
    int children[2];            // if NF_SUBSECTOR its a subsector
} node_t;

/*
================================================================================

                              OTHER TYPES

================================================================================
*/

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
extern fixed_t fractionaltic;

typedef byte lighttable_t;      // this could be wider for >8 bit display

typedef struct visplane_s
{
    struct visplane_s *next; // [JN] Next visplane in hash chain -- killough
    fixed_t height;
    int picnum;
    int lightlevel;
    int special;
    int minx, maxx;
    unsigned int pad1;                // [crispy] hires / 32-bit integer math
    unsigned int top[MAXWIDTH];       // [crispy] hires / 32-bit integer math
    unsigned int pad2;                // [crispy] hires / 32-bit integer math
    unsigned int pad3;                // [crispy] hires / 32-bit integer math
    unsigned int bottom[MAXWIDTH];    // [crispy] hires / 32-bit integer math
    unsigned int pad4;                // [crispy] hires / 32-bit integer math
} visplane_t;

typedef struct drawseg_s
{
    seg_t *curline;
    int x1, x2;
    fixed_t scale1, scale2, scalestep;
    int silhouette;             // 0=none, 1=bottom, 2=top, 3=both
    fixed_t bsilheight;         // don't clip sprites above this
    fixed_t tsilheight;         // don't clip sprites below this
// pointers to lists for sprite clipping
    int*    sprtopclip;         // [crispy] 32-bit integer math
    int*    sprbottomclip;      // [crispy] 32-bit integer math
    int*    maskedtexturecol;   // [crispy] 32-bit integer math
} drawseg_t;

#define	SIL_NONE	0
#define	SIL_BOTTOM	1
#define SIL_TOP		2
#define	SIL_BOTH	3

#define	MAXDRAWSEGS		256

// A vissprite_t is a thing that will be drawn during a refresh
typedef struct vissprite_s
{
    struct vissprite_s *prev, *next;
    int x1, x2;
    fixed_t gx, gy;             // for line side calculation
    fixed_t gz, gzt;            // global bottom / top for silhouette clipping
    fixed_t startfrac;          // horizontal position of x1
    fixed_t scale;
    fixed_t xiscale;            // negative if flipped
    fixed_t texturemid;
    int patch;
    // [crispy] brightmaps for select sprites
    lighttable_t *colormap[2];
    const byte *brightmap;
    int mobjflags;              // for color translation and shadow draw
    boolean psprite;            // true if psprite
    fixed_t footclip;           // foot clipping
    byte *translation;
} vissprite_t;


extern visplane_t *floorplane, *ceilingplane;

// Sprites are patches with a special naming convention so they can be 
// recognized by R_InitSprites.  The sprite and frame specified by a 
// thing_t is range checked at run time.
// a sprite is a patch_t that is assumed to represent a three dimensional
// object and may have multiple rotations pre drawn.  Horizontal flipping 
// is used to save space. Some sprites will only have one picture used
// for all views.  

typedef struct
{
    boolean rotate;             // if false use 0 for any position
    short lump[8];              // lump to use for view angles 0-7
    byte flip[8];               // flip (1 = flip) to use for view angles 0-7
} spriteframe_t;

typedef struct
{
    int numframes;
    spriteframe_t *spriteframes;
} spritedef_t;

extern int numsprites;
extern spritedef_t *sprites;

extern int numvertexes;
extern vertex_t *vertexes;

extern int numsegs;
extern seg_t *segs;

extern int numsectors;
extern sector_t *sectors;

extern int numsubsectors;
extern subsector_t *subsectors;

extern int numnodes;
extern node_t *nodes;

extern int numlines;
extern line_t *lines;

extern int numsides;
extern side_t *sides;

extern angle_t *linearskyangle;
extern angle_t *xtoviewangle;
extern angle_t clipangle;
extern angle_t viewangle;

extern fixed_t viewx, viewy, viewz;

extern int viewangletox[FINEANGLES / 2];

extern player_t *viewplayer;

/*
================================================================================
=
= R_BMAPS
=
================================================================================
*/

#define MINBRIGHT  24  // [JN] Minimal COLORMAP level for half-brights.

extern void R_InitBrightmaps ();

extern const byte **texturebrightmap;
extern const byte *R_BrightmapForTexName (const char *texname);
extern const byte *R_BrightmapForSprite (const int state);
extern const byte *R_BrightmapForFlatNum (const int num);
extern const byte *R_BrightmapForState (const int state);

/*
================================================================================
=
= R_BPS
=
================================================================================
*/

typedef void (*drawfunc_t) (int start, int stop);

extern boolean markceiling;
extern boolean markfloor;  // false if the back side is the same plane
extern boolean skymap;

extern byte *solidcol;  // [JN] Improved column clipping.

extern drawseg_t *drawsegs, *ds_p;

extern lighttable_t **hscalelight, **vscalelight, **dscalelight;

extern line_t   *linedef;
extern sector_t *frontsector, *backsector;
extern seg_t    *curline;
extern side_t   *sidedef;

extern unsigned maxdrawsegs;  // [JN] killough: New code which removes 2s linedef limit

extern void R_ClearClipSegs (void);
extern void R_ClearDrawSegs (void);
extern void R_InitClipSegs (void);
extern void R_InitSkyMap (void);
extern void R_RenderBSPNode (int bspnum);
extern void R_StoreWallRange (const int start, const int stop);

/*
================================================================================
=
= R_DATA
=
================================================================================
*/

extern const byte *R_GetColumn (const int tex, int col, const boolean opaque);

extern fixed_t *spriteoffset;
extern fixed_t *spritetopoffset;
extern fixed_t *spritewidth;    // needed for pre rendering (fracs)
extern fixed_t *textureheight;  // needed for texture pegging

extern int *flattranslation;    // for global animation
extern int *flipscreenwidth;    // [crispy] lookup table for horizontal screen coordinates
extern int *flipviewwidth;      // [crispy] lookup table for horizontal screen coordinates
extern int *texturetranslation; // for global animation

extern int firstflat;
extern int firstspritelump, lastspritelump, numspritelumps;
extern int numflats;

extern lighttable_t *colormaps;

extern void R_InitData (void);
extern void R_PrecacheLevel (void);

/*
================================================================================
=
= R_DRAW
=
================================================================================
*/

extern byte *dc_translation;
extern byte *translationtables;

extern const byte *dc_brightmap;
extern const byte *dc_source;         // first pixel in a column
extern const byte *ds_brightmap;
extern const byte *ds_source;         // start of a 64*64 tile image
extern const lighttable_t *dc_colormap[2];
extern const lighttable_t *ds_colormap[2];

extern fixed_t dc_iscale;
extern fixed_t dc_texheight;
extern fixed_t dc_texturemid;
extern fixed_t dc_x;
extern fixed_t dc_yh;
extern fixed_t dc_yl;
extern fixed_t skyiscale;
extern fixed_t skyiscale_low;
extern fixed_t skytextureheight;
extern int     skytexturemid;

extern void R_DrawColumn (void);
extern void R_DrawColumnLow (void);
extern void R_DrawExtraTLColumn (void);
extern void R_DrawExtraTLColumnLow (void);
extern void R_DrawSkyColumn (void);
extern void R_DrawSkyColumnLow (void);
extern void R_DrawTLColumn (void);
extern void R_DrawTLColumnLow (void);
extern void R_DrawTranslatedColumn (void);
extern void R_DrawTranslatedColumnLow (void);
extern void R_DrawTranslatedTLColumn (void);
extern void R_DrawTranslatedTLColumnLow (void);
extern void R_DrawSpan (fixed_t x1, fixed_t x2, const fixed_t y,
                        fixed_t ds_xfrac, const fixed_t ds_xstep,
                        fixed_t ds_yfrac, const fixed_t ds_ystep);
extern void R_DrawSpanLow (fixed_t x1, fixed_t x2, const fixed_t y,
                           fixed_t ds_xfrac, const fixed_t ds_xstep,
                           fixed_t ds_yfrac, const fixed_t ds_ystep);
extern void R_InitBuffer (const int width, const int height);

/*
================================================================================
=
= R_MAIN
=
================================================================================
*/

#define	FIELDOFVIEW 2048  // fineangles in the SCREENWIDTH wide window
#define LOOKDIRMIN  160   // [crispy] -110, actually ([JN] increased to 160 (i.e. -160))
#define LOOKDIRMAX  90
#define LOOKDIRS    (LOOKDIRMIN+1+LOOKDIRMAX) // [crispy] lookdir range: -160..0..90

// [JN] Doubled versions for quad resolution, used only for rendering.
#define LOOKDIRMIN2 (LOOKDIRMIN << quadres)
#define LOOKDIRMAX2 (LOOKDIRMIN << quadres)
#define LOOKDIRS2   (LOOKDIRMIN2+1+LOOKDIRMAX2)

// Lighting constants
#define	LIGHTLEVELS			16
#define	LIGHTSEGSHIFT		4
#define	MAXLIGHTSCALE		48
#define	LIGHTSCALESHIFT		12
// [crispy] & [JN] smoother diminished lighting
#define MAXLIGHTZ			1024
#define LIGHTZSHIFT			17
// [JN] Vanilla values
#define MAXLIGHTZ_VANILLA   128
#define LIGHTZSHIFT_VANILLA 20
#define	NUMCOLORMAPS		32      // number of diminishing
#define	INVERSECOLORMAP		32

// Define the different areas for the dirty map
#define I_NOUPDATE  0
#define I_FULLVIEW  1
#define I_STATBAR   2
#define I_MESSAGES  4
#define I_FULLSCRN  8

// [AM] Interpolate between two angles.
extern const angle_t R_InterpolateAngle (const angle_t oangle, const angle_t nangle, const fixed_t scale);
extern const angle_t R_PointToAngle2 (const fixed_t x1, const fixed_t y1, const fixed_t x2, const fixed_t y2);
extern const angle_t R_PointToAngleCrispy (fixed_t x, fixed_t y);
extern const angle_t R_PointToAngle (const fixed_t x, const fixed_t y);

extern boolean setsizeneeded;
extern boolean BorderNeedRefresh;
extern boolean BorderTopRefresh;

extern fixed_t centerxfrac;
extern fixed_t centeryfrac;
extern fixed_t projection;
extern fixed_t R_PointToDist (fixed_t x, fixed_t y);
extern fixed_t viewcos, viewsin;

extern int centerx, centery;
extern int extralight;
extern int flyheight;
extern int maxlightz, lightzshift;
extern int rendered_segs, rendered_visplanes, rendered_vissprites;
extern int scaledviewwidth;
extern int UpdateState;
extern int validcount;
extern int viewwidth, viewheight, scaledviewheight, viewwindowx, viewwindowy;
extern const int R_CheckTextureNumForName (const char *name);
extern const int R_FlatNumForName (const char *name);
extern const int R_PointOnSegSide (fixed_t x, fixed_t y, const seg_t *line);
extern const int R_PointOnSide (fixed_t x, fixed_t y, const node_t *node);
extern const int R_TextureNumForName (const char *name);

extern lighttable_t **walllights;
extern lighttable_t *fixedcolormap;
extern lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

extern const subsector_t *R_PointInSubsector (const fixed_t x, const fixed_t y);

extern void (*basecolfunc) (void);
extern void (*colfunc) (void);
extern void (*extratlcolfunc) (void);
extern void (*skycolfunc) (void);
extern void (*tlcolfunc) (void);
extern void (*transcolfunc) (void);
extern void (*transtlcolfunc) (void);
extern void (*spanfunc) (fixed_t x1, fixed_t x2, const fixed_t y,
                         fixed_t ds_xfrac, const fixed_t ds_xstep,
                         fixed_t ds_yfrac, const fixed_t ds_ystep);

extern void R_ClearStats (void); // [JN] Used by perfomance counter.
extern void R_DrawTopBorder (void);
extern void R_DrawViewBorder (void);
extern void R_ExecuteSetViewSize (void);
extern void R_Init (void);
extern void R_InitLightTables (void);
extern void R_InterpolateTextureOffsets (void); // [crispy] smooth texture scrolling
extern void R_RenderPlayerView (const player_t *player);
extern void R_SetViewSize (const int blocks);

/*
================================================================================
=
= R_PLANE
=
================================================================================
*/

#define	ANGLETOSKYSHIFT 22  // sky map is 256*128*4 maps

typedef void (*planefunction_t) (int top, int bottom);

extern fixed_t *yslope, *distscale;
extern fixed_t yslopes[MAXHEIGHT][MAXHEIGHT];
// [JN] Smooth plane scrolling.
extern fixed_t FlatScrollFactor_X, FlatScrollFactor_X_old;
extern fixed_t FlatScrollFactor_Y, FlatScrollFactor_Y_old;
// [JN] Circular flowing effect for swirling liquids.
extern fixed_t FlowAmplitude_X, FlowAmplitude_Y;

extern int *floorclip, *ceilingclip; // [JN] dropoff overflow
extern int *lastopening; // [crispy] 32-bit integer math
extern int  skyflatnum;

extern planefunction_t floorfunc, ceilingfunc;

extern const visplane_t *R_CheckPlane (visplane_t *pl, const int start, const int stop);
extern const visplane_t *R_DupPlane (const visplane_t *pl, const int start, const int stop);
extern const visplane_t *R_FindPlane (fixed_t height, const int picnum, const int lightlevel, const int special);

extern void R_ClearPlanes (void);
extern void R_DrawPlanes (void);
extern void R_InitPlanes (void);
extern void R_InitPlanesRes (void);
extern void R_InitVisplanesRes (void);

/*
================================================================================
=
= R_SEGS
=
================================================================================
*/

// [BH] Compensate for rounding errors in DOOM's renderer by stretching wall
//  columns by 1px. This eliminates the randomly-colored pixels ("sparkles")
//  that appear at the bottom of some columns.
#define SPARKLEFIX 64

extern int rw_angle1;  // angle to line origin

extern void R_RenderMaskedSegRange (drawseg_t *ds, const int x1, const int x2);

/*
================================================================================
=
= R_SWIRL
=
================================================================================
*/

extern const char *R_DistortedFlat (const int flatnum);

extern void R_InitDistortedFlats ();

/*
================================================================================
=
= R_THINGS
=
================================================================================
*/

#define	BASEYCENTER  100
#define	MINZ         (FRACUNIT*4)

// constant arrays used for psprite clipping and initializing clipping
extern int *negonearray;       // [JN] killough 2/8/98: // dropoff overflow
extern int *screenheightarray; //      change to MAX_*  // dropoff overflow

extern fixed_t pspritescale, pspriteiscale;
extern fixed_t spryscale;

extern int *mceilingclip; // [crispy] 32-bit integer math
extern int *mfloorclip;   // [crispy] 32-bit integer math

extern int64_t sprbotscreen;
extern int64_t sprtopscreen;

extern void R_AddPSprites (void);
extern void R_AddSprites (const sector_t *sec);
extern void R_ClearSprites (void);
extern void R_ClipVisSprite (vissprite_t *vis, int xl, int xh);
extern void R_DrawMasked (void);
extern void R_DrawMaskedColumn (const column_t *column, signed const int baseclip);
extern void R_DrawSprites (void);
extern void R_InitSprites (char **namelist);
extern void R_InitSpritesRes (void);
extern void R_SortVisSprites (void);
