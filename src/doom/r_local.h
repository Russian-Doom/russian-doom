//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2013-2017 Brad Harding
// Copyright(C) 2017 Fabian Greffrath
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
//	Refresh (R_*) module, global header.
//	All the rendering/drawing stuff is here.
//


#pragma once

#include "d_items.h"
#include "i_video.h"
#include "v_patch.h"


// -----------------------------------------------------------------------------
// GLOBAL DEFINITIONS
// -----------------------------------------------------------------------------

// Silhouette, needed for clipping Segs (mainly) and sprites representing things.
#define SIL_NONE        0
#define SIL_BOTTOM      1
#define SIL_TOP         2
#define SIL_BOTH        3

// Your plain vanilla vertex.
// Note: transformed values not buffered locally, 
//  like some DOOM-alikes ("wt", "WebView") did.

typedef struct
{
    fixed_t x;
    fixed_t y;

    // [crispy] remove slime trails
    // pseudovertexes are dummies that have their coordinates modified to get
    // moved towards the linedef associated with their seg by projecting them
    // using the law of cosines in p_setup.c:P_RemoveSlimeTrails();
    // they are *only* used in rendering
    fixed_t	px;
    fixed_t	py;
    boolean	moved;

} vertex_t;

// Forward of LineDefs, for Sectors.
struct line_s;

//
// Each sector has a degenmobj_t in its center for sound origin purposes.
// I suppose this does not handle sound from moving objects (doppler), because
// position is prolly just buffered, not updated.
//

typedef struct
{
    thinker_t  thinker;  // not used for anything
    fixed_t    x;
    fixed_t    y;
    fixed_t    z;

} degenmobj_t;

//
// The SECTORS record, at runtime. Stores things/mobjs.
//

typedef	struct
{
    fixed_t floorheight;
    fixed_t ceilingheight;

    // [JN] Improved column clipping.
    fixed_t floor_xoffs,   floor_yoffs;
    fixed_t ceiling_xoffs, ceiling_yoffs;
    int     floorlightsec, ceilinglightsec;

    short   floorpic;
    short   ceilingpic;
    short   lightlevel;
    short   special;
    short   tag;
    short   flow;  // [JN] Flow effect of swirling liquids.

    // 0 = untraversed, 1,2 = sndlines -1
    int     soundtraversed;

    // thing that made a sound (or null)
    mobj_t *soundtarget;

    // mapblock bounding box for height changes
    int     blockbox[4];

    // origin for any sounds played by the sector
    degenmobj_t soundorg;

    // if == validcount, already checked
    int     validcount;

    // list of mobjs in sector
    mobj_t *thinglist;

    // thinker_t for reversable actions
    void   *specialdata;

    int             linecount;
    struct line_s **lines;	// [linecount] size

    // [crispy] WiggleFix: [kb] for R_FixWiggle()
    int     cachedheight;
    int     scaleindex;

    // [AM] Previous position of floor and ceiling before
    //      think.  Used to interpolate between positions.
    fixed_t oldfloorheight;
    fixed_t oldceilingheight;

    // [AM] Gametic when the old positions were recorded.
    //      Has a dual purpose; it prevents movement thinkers
    //      from storing old positions twice in a tic, and
    //      prevents the renderer from attempting to interpolate
    //      if old values were not updated recently.
    int     oldgametic;

    // [AM] Interpolated floor and ceiling height.
    //      Calculated once per tic and used inside
    //      the renderer.
    fixed_t interpfloorheight;
    fixed_t interpceilingheight;

} sector_t;

//
// The SideDef.
//

typedef struct
{
    // add this to the calculated texture column
    fixed_t textureoffset;
    
    // add this to the calculated texture top
    fixed_t rowoffset;

    // Texture indices. We do not maintain names here. 
    short   toptexture;
    short   bottomtexture;
    short   midtexture;

    // Sector the SideDef is facing.
    sector_t *sector;

    // [crispy] smooth texture scrolling
    fixed_t oldtextureoffset;
    fixed_t oldrowoffset;

} side_t;

//
// Move clipping aid for LineDefs.
//

typedef enum
{
    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE

} slopetype_t;

//
// A LineDef.
//

typedef struct line_s
{
    // Vertices, from v1 to v2.
    vertex_t *v1;
    vertex_t *v2;

    // Precalculated v2 - v1 for side checking.
    fixed_t dx;
    fixed_t dy;

    // Animation related.
    unsigned short  flags;
    short           special;
    short           tag;
    short           fall; // [JN] Falling liquid linedef.

    // Visual appearance: SideDefs.
    //  sidenum[1] will be -1 (NO_INDEX) if one sided
    unsigned short  sidenum[2];	

    // Neat. Another bounding box, for the extent of the LineDef.
    fixed_t	bbox[4];

    // To aid move clipping.
    slopetype_t slopetype;

    // Front and back sector.
    // Note: redundant? Can be retrieved from SideDefs.
    sector_t *frontsector;
    sector_t *backsector;

    // if == validcount, already checked
    int     validcount;

    // thinker_t for reversable actions
    void   *specialdata;		

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

//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs, indicating the visible walls that 
// define (all or some) sides of a convex BSP leaf.
//

typedef struct subsector_s
{
    sector_t *sector;
    int numlines;
    int firstline;

} subsector_t;

//
// The LineSeg.
//

typedef struct
{
    vertex_t *v1;
    vertex_t *v2;
    fixed_t   offset;
    angle_t   angle;
    side_t   *sidedef;
    line_t   *linedef;

    // Sector references.
    // Could be retrieved from linedef, too.
    // backsector is NULL for one sided lines
    sector_t *frontsector;
    sector_t *backsector;

    uint32_t length;   // [crispy] fix long wall wobble
    angle_t  r_angle;  // [crispy] re-calculated angle used for rendering

} seg_t;

//
// BSP node.
//

typedef struct
{
    // Partition line.
    fixed_t x;
    fixed_t y;
    fixed_t dx;
    fixed_t dy;

    // Bounding box for each child.
    fixed_t bbox[2][4];

    // If NF_SUBSECTOR its a subsector.
    int children[2];

} node_t;

// This could be wider for >8 bit display. 
// Indeed, true color support is posibble precalculating 24bpp 
// lightmap/colormap LUT. From darkening PLAYPAL to all black.
// Could even us emore than 32 levels.

typedef byte lighttable_t;	

//
// DrawSegs.
//

typedef struct drawseg_s
{
    const seg_t *curline;
    int     x1;
    int     x2;

    fixed_t scale1;
    fixed_t scale2;
    fixed_t scalestep;

    // 0=none, 1=bottom, 2=top, 3=both
    int     silhouette;

    // do not clip sprites above this
    fixed_t bsilheight;

    // do not clip sprites below this
    fixed_t tsilheight;

    // Pointers to lists for sprite clipping,
    //  all three adjusted so [x1] is first value.
    int    *sprtopclip;        // [crispy] 32-bit integer math
    int    *sprbottomclip;     // [crispy] 32-bit integer math
    int    *maskedtexturecol;  // [crispy] 32-bit integer math

} drawseg_t;

//
// A vissprite_t is a thing that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
//

typedef struct vissprite_s
{
    // Doubly linked list.
    struct vissprite_s *prev;
    struct vissprite_s *next;

    int     x1;
    int     x2;

    // for line side calculation
    fixed_t gx;
    fixed_t gy;		

    // global bottom / top for silhouette clipping
    fixed_t gz;
    fixed_t gzt;

    // horizontal position of x1
    fixed_t startfrac;
    fixed_t scale;

    // negative if flipped
    fixed_t xiscale;	

    fixed_t texturemid;
    int     patch;

    // for color translation and shadow draw, maxbright frames as well
    // [crispy] brightmaps for select sprites
    lighttable_t *colormap[2];
    const byte   *brightmap;

    int     mobjflags;
    byte   *translation;

} vissprite_t;

// Sprites are patches with a special naming convention so they can be 
// recognized by R_InitSprites. The base name is NNNNFx or NNNNFxFx, with
// x indicating the rotation, x = 0, 1-7. The sprite and frame specified by 
// a thing_t is range checked at run time. A sprite is a patch_t that is assumed
// to represent a three dimensional object and may have multiple rotations 
// pre drawn. Horizontal flipping is used to save space, thus NNNNF2F5 defines
// a mirrored patch. 
// Some sprites will only have one picture used for all views: NNNNF0

typedef struct
{
    // If false use 0 for any position.
    // Note: as eight entries are available,
    //  we might as well insert the same name eight times.
    boolean rotate;

    // Lump to use for view angles 0-7.
    short   lump[16];  // [crispy] support 16 sprite rotations

    // Flip bit (1 = flip) to use for view angles 0-7.
    byte    flip[16]; // [crispy] support 16 sprite rotations

} spriteframe_t;

//
// A sprite definition: a number of animation frames.
//

typedef struct
{
    int            numframes;
    spriteframe_t *spriteframes;

} spritedef_t;

//
// Now what is a visplane, anyway?
//

typedef struct visplane_s
{
    struct visplane_s *next; // [JN] Next visplane in hash chain -- killough
    fixed_t height;
    int     picnum;
    int     lightlevel;
    int     flow;  // [JN] Flow effect of swirling liquids.
    int     minx;
    int     maxx;

    // leave pads for [minx-1]/[maxx+1]

    unsigned int pad1;  // [crispy] hires / 32-bit integer math
    // Here lies the rub for all dynamic resize/change of resolution.
    unsigned int top[WIDESCREENWIDTH];  // [crispy] hires / 32-bit integer math
    unsigned int pad2;  // [crispy] hires / 32-bit integer math
    unsigned int pad3;  // [crispy] hires / 32-bit integer math
    // See above.
    unsigned int bottom[WIDESCREENWIDTH];  // [crispy] hires / 32-bit integer math
    unsigned int pad4;  // [crispy] hires / 32-bit integer math

} visplane_t;

// -----------------------------------------------------------------------------
// R_BMAPS
// -----------------------------------------------------------------------------

#define MINBRIGHT   24  // [JN] Minimal COLORMAP level for half-brights.

extern void R_InitBrightmaps ();

extern const byte *R_BrightmapForTexName (const char *texname);
extern const byte *R_BrightmapForSprite (const int type);
extern const byte *R_BrightmapForFlatNum (const int num);
extern const byte *R_BrightmapForState (const int state);
extern const byte **texturebrightmap;

// -----------------------------------------------------------------------------
// R_BSP
// -----------------------------------------------------------------------------

extern const seg_t  *curline;
extern const side_t *sidedef;
extern line_t       *linedef;
extern sector_t     *frontsector;
extern sector_t     *backsector;
extern drawseg_t    *ds_p;
extern drawseg_t    *drawsegs;
extern unsigned      maxdrawsegs;
extern byte         *solidcol;
extern boolean       markfloor;		
extern boolean       markceiling;

void R_ClearClipSegs (void);
void R_ClearDrawSegs (void);
void R_InitClipSegs (void);
void R_RenderBSPNode (int bspnum);
void R_StoreWallRange (const int start, const int stop);

// -----------------------------------------------------------------------------
// R_DATA
// -----------------------------------------------------------------------------

#define LOOKDIRMIN	110 // [crispy] -110, actually
#define LOOKDIRMAX	90
#define LOOKDIRS	(LOOKDIRMIN+1+LOOKDIRMAX) // [crispy] lookdir range: -110..0..90

const byte *R_GetColumn (const int tex, int col);
const byte *R_GetColumnMod (const int tex, int col);
int R_CheckTextureNumForName (char *name);
int R_FlatNumForName (char *name);
int R_TextureNumForName (char *name);
void R_InitData (void);
void R_PrecacheLevel (void);
boolean R_IsPatchLump (const int lump);

// -----------------------------------------------------------------------------
// R_DRAW
// -----------------------------------------------------------------------------

extern const lighttable_t *dc_colormap[2];
extern const byte         *dc_source;
extern const byte         *dc_brightmap;
extern fixed_t     dc_x, dc_yl, dc_yh; 
extern fixed_t     dc_texheight;
extern fixed_t     dc_iscale;
extern fixed_t     dc_texturemid;
extern const byte *dc_translation;
extern byte       *translationtables;

extern fixed_t ds_y, ds_x1, ds_x2;
extern fixed_t ds_xfrac, ds_yfrac;
extern fixed_t ds_xstep, ds_ystep;

extern const lighttable_t *ds_colormap[2];
extern const byte         *ds_source;
extern const byte         *ds_brightmap;

void R_DrawColumn (void);
void R_DrawColumnLow (void);
void R_DrawFuzzColumn (void);
void R_DrawFuzzColumnBW (void);
void R_DrawFuzzColumnImproved (void);
void R_DrawFuzzColumnImprovedBW (void);
void R_DrawFuzzColumnLow (void);
void R_DrawFuzzColumnLowBW (void);
void R_DrawFuzzColumnLowImproved (void);
void R_DrawFuzzColumnLowImprovedBW (void);
void R_DrawFuzzColumnTranslucent (void);
void R_DrawFuzzColumnTranslucentLow (void);
void R_DrawGhostColumn (void);
void R_DrawGhostColumnLow (void);
void R_DrawSpan (void);
void R_DrawSpanLow (void);
void R_DrawTLColumn (void);
void R_DrawTLColumnLow (void);
void R_DrawTranslatedColumn (void);
void R_DrawTranslatedColumnLow (void);
void R_DrawTranslatedTLColumn (void);
void R_DrawTranslatedTLColumnLow (void);
void R_DrawViewBorder (void);
void R_FillBackScreen (void);
void R_InitBuffer (int width, int height);
void R_SetFuzzPosDraw (void);
void R_SetFuzzPosTic (void);
void R_VideoErase (unsigned ofs, const int count);

// -----------------------------------------------------------------------------
// R_MAIN
// -----------------------------------------------------------------------------

// Lighting constants. Now why not 32 levels here?
#define LIGHTLEVELS         16
#define LIGHTSEGSHIFT       4
#define MAXLIGHTSCALE       48
#define LIGHTSCALESHIFT     12
// [crispy] & [JN] smoother diminished lighting
#define MAXLIGHTZ           1024
#define LIGHTZSHIFT         17
// [JN] Vanilla values
#define MAXLIGHTZ_VANILLA   128
#define LIGHTZSHIFT_VANILLA 20
// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS 32
// Sky rendering
#define SKYFLATNAME "F_SKY1"    // Store the number for name.
#define ANGLETOSKYSHIFT 22      // The sky map is 256*128*4 maps.

extern boolean scaled_sky;
extern fixed_t centerxfrac, centeryfrac;
extern fixed_t fractionaltic;
extern fixed_t projection;
extern fixed_t viewcos, viewsin;
extern int centerx, centery;
extern int detailshift;
extern int extralight;
extern int maxlightz, lightzshift;
extern int rendered_segs, rendered_visplanes, rendered_vissprites;
extern int skyflatnum, skytexture, skytexturemid;
extern int validcount;
extern int viewwindowx, viewwindowy;
extern lighttable_t *fixedcolormap;
extern lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];
extern void (*basecolfunc) (void);
extern void (*colfunc) (void);
extern void (*fuzzcolfunc) (void);
extern void (*spanfunc) (void);
extern void (*tlcolfunc) (void);
extern void (*transcolfunc) (void);
extern void (*transtlcolfunc) (void);
extern void (*ghostcolfunc) (void);
extern void R_InitLightTables (void);
extern void R_ClearStats (void);

angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale);
angle_t R_PointToAngle (fixed_t x, fixed_t y);
angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
angle_t R_PointToAngleCrispy (fixed_t x, fixed_t y);
int R_PointOnSegSide (fixed_t x, fixed_t y, const seg_t *line);
int R_PointOnSide (fixed_t x, fixed_t y, const node_t *node);
subsector_t *R_PointInSubsector (fixed_t x, fixed_t y);
void R_ExecuteSetViewSize (void);
void R_Init (void);
void R_RenderMaskedSegRange (drawseg_t *ds, int x1, int x2);
void R_RenderPlayerView (player_t *player);
void R_SetViewSize (int blocks, int detail);

// -----------------------------------------------------------------------------
// R_PLANE
// -----------------------------------------------------------------------------

extern fixed_t  yslopes[LOOKDIRS][SCREENHEIGHT];
extern fixed_t *yslope, *distscale;
extern int     *floorclip, *ceilingclip; // dropoff overflow
extern int     *lastopening; // [crispy] 32-bit integer math

visplane_t *R_CheckPlane (visplane_t *pl, int start, int stop);
visplane_t *R_DupPlane (const visplane_t *pl, int start, int stop);
visplane_t *R_FindPlane (fixed_t height, const int picnum, const int lightlevel, const int flow);
void R_ClearPlanes (void);
void R_DrawPlanes (void);
void R_InitPlanesRes (void);
void R_InitVisplanesRes (void);

// -----------------------------------------------------------------------------
// R_STATE
// -----------------------------------------------------------------------------

extern fixed_t *textureheight;
extern fixed_t *spritewidth, *spriteoffset, *spritetopoffset;
extern lighttable_t *colormaps;
extern lighttable_t *colormaps_rd;
extern int  viewwidth, viewheight;
extern int  scaledviewwidth, scaledviewheight;
extern int *flipscreenwidth;
extern int *flipviewwidth;
extern int  firstflat;
extern int *flattranslation, *texturetranslation;
extern int  firstspritelump, lastspritelump, numspritelumps;

// Lookup tables for map data.
extern int          numsprites;
extern spritedef_t *sprites;
extern int          numvertexes;
extern vertex_t    *vertexes;
extern int          numsegs;
extern seg_t       *segs;
extern int          numsectors;
extern sector_t    *sectors;
extern int          numsubsectors;
extern subsector_t *subsectors;
extern int          numnodes;
extern node_t      *nodes;
extern int          numlines;
extern line_t      *lines;
extern int          numsides;
extern side_t      *sides;

// POV data.
extern fixed_t   viewx, viewy, viewz;
extern angle_t   viewangle;
extern player_t *viewplayer;

// ?
extern int      viewangletox[FINEANGLES/2];
extern angle_t  clipangle;
extern angle_t *xtoviewangle;
extern angle_t *linearskyangle;

// angle to line origin
extern int rw_angle1;

extern visplane_t *floorplane;
extern visplane_t *ceilingplane;

// -----------------------------------------------------------------------------
// R_SWIRL
// -----------------------------------------------------------------------------

typedef struct
{
    int   mission;
    int   epsiode;
    int   map;
    int   sector;
    short flow;
} flow_t;

typedef struct
{
    int   mission;
    int   epsiode;
    int   map;
    int   linedef;
    short fall;
} fall_t;

extern const flow_t flow[];
extern const fall_t fall[];
extern void R_FlowPlane (const int flow);
extern fixed_t FlowFactor_X, FlowFactor_X_old;
extern fixed_t FlowFactor_Y, FlowFactor_Y_old;
extern fixed_t FlowDelta_X;
extern fixed_t FlowDelta_Y;
extern fixed_t FallFactor_100, FallFactor_100_old;
extern fixed_t FallFactor_101, FallFactor_101_old;
extern fixed_t FallFactor_102, FallFactor_102_old;
extern fixed_t FallFactor_103, FallFactor_103_old;
extern fixed_t FallFactor_104, FallFactor_104_old;

const char *R_DistortedFlat (const int flatnum);
void R_InitDistortedFlats (void);
void R_FallLinedef (void);

// -----------------------------------------------------------------------------
// R_THINGS
// -----------------------------------------------------------------------------

extern int     *negonearray;       // [JN] killough 2/8/98: // dropoff overflow
extern int     *screenheightarray; //      change to MAX_*  // dropoff overflow
extern int     *mfloorclip;
extern int     *mceilingclip;
extern fixed_t  spryscale;
extern int64_t  sprtopscreen; // [crispy] WiggleFix
extern fixed_t  pspritescale;
extern fixed_t  pspriteiscale;
extern int      bmap_flick;   // [JN] Animated brightmaps.
extern int      bmap_glow;

void R_AddPSprites (void);
void R_AddSprites (const sector_t *sec);
void R_ClearSprites (void);
void R_ClipVisSprite (vissprite_t *vis, int xl, int xh);
void R_DrawMasked (void);
void R_DrawMaskedColumn (const column_t *column);
void R_DrawSprites (void);
void R_InitSprites (char **namelist);
void R_InitSpritesRes (void);



