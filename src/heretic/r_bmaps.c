//
// Copyright(C) 2017-2019 Julian Nechaevsky
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
//	Brightmap textures and flats lookup routine.
//


#include "doomdef.h"
#include "deh_str.h"
#include "r_bmaps.h"
#include "jn.h"


// Floors and ceilings:
int bmapflatnum1, bmapflatnum2, bmapflatnum3, bmapflatnum4, bmapflatnum5;

// Walls:
int bmaptexture01, bmaptexture02, bmaptexture03, bmaptexture04, bmaptexture05, 
    bmaptexture06, bmaptexture07;


/*
================================================================================
=
= R_InitBrightmappedTextures
=
= [JN] Lookup and init all the textures for brightmapping.
=
================================================================================
*/

void R_InitBrightmappedTextures(void)
{
    // -------------------------------------------------------
    //  Floors and ceilings
    // -------------------------------------------------------

    // blue_only_floor
    bmapflatnum1 = R_FlatNumForName("FLOOR21");
    bmapflatnum2 = R_FlatNumForName("FLOOR22");
    bmapflatnum3 = R_FlatNumForName("FLOOR23");
    bmapflatnum4 = R_FlatNumForName("FLOOR24");
    bmapflatnum5 = R_FlatNumForName("FLOOR26");

    // -------------------------------------------------------
    //  Textures
    // -------------------------------------------------------

    // brightmap_redonly
    bmaptexture01 = R_CheckTextureNumForName("GRSKULL3");
    // brightmap_blueonly
    bmaptexture02 = R_CheckTextureNumForName("DOOREXIT");
    bmaptexture03 = R_CheckTextureNumForName("SW2ON");
    bmaptexture04 = R_CheckTextureNumForName("SW2OFF");
    bmaptexture05 = R_CheckTextureNumForName("SW2OFF");
    // brightmap_notbronze
    bmaptexture06 = R_CheckTextureNumForName("SW1ON");
    bmaptexture07 = R_CheckTextureNumForName("SW1OFF");
}

