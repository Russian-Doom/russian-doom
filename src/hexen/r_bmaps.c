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


#include "h2def.h"
#include "r_bmaps.h"


// Walls:
int bmaptexture01, bmaptexture02, bmaptexture03, bmaptexture04, bmaptexture05,
    bmaptexture06, bmaptexture07, bmaptexture08, bmaptexture09, bmaptexture10,
    bmaptexture11, bmaptexture12, bmaptexture13, bmaptexture14, bmaptexture15,
    bmaptexture16, bmaptexture17, bmaptexture18, bmaptexture19, bmaptexture20,
    bmaptexture21, bmaptexture22, bmaptexture23;


/*
================================================================================
=
= R_InitBrightmappedTextures
=
= [JN] Lookup and init all the textures for brightmapping.
=
================================================================================
*/

void R_InitBrightmappedTextures (void)
{
    // -------------------------------------------------------
    //  Textures
    // -------------------------------------------------------

    // brightmap_greenonly
    bmaptexture01 = R_CheckTextureNumForName("SW_1_MD");
    
    // brightmap_redonly
    bmaptexture02 = R_CheckTextureNumForName("SW_2_DN");
    
    // brightmap_blueonly
    bmaptexture03 = R_CheckTextureNumForName("SW_1_DN");
    bmaptexture04 = R_CheckTextureNumForName("SW_2_MD");
    
    // brightmap_flame
    bmaptexture05 = R_CheckTextureNumForName("SPAWN03");
    bmaptexture06 = R_CheckTextureNumForName("SPAWN12");
    bmaptexture08 = R_CheckTextureNumForName("SW52_ON");
    bmaptexture11 = R_CheckTextureNumForName("X_FAC01"); // Korax
    bmaptexture12 = R_CheckTextureNumForName("X_FAC02"); // Korax
    bmaptexture13 = R_CheckTextureNumForName("X_FAC03"); // Korax
    bmaptexture14 = R_CheckTextureNumForName("X_FAC04"); // Korax
    bmaptexture15 = R_CheckTextureNumForName("X_FAC05"); // Korax
    bmaptexture16 = R_CheckTextureNumForName("X_FAC06"); // Korax
    bmaptexture17 = R_CheckTextureNumForName("X_FAC07"); // Korax
    bmaptexture18 = R_CheckTextureNumForName("X_FAC08"); // Korax
    bmaptexture19 = R_CheckTextureNumForName("X_FAC09"); // Korax
    bmaptexture20 = R_CheckTextureNumForName("X_FAC10"); // Korax
    bmaptexture21 = R_CheckTextureNumForName("X_FAC11"); // Korax
    bmaptexture22 = R_CheckTextureNumForName("X_FAC12"); // Korax
    bmaptexture23 = R_CheckTextureNumForName("TPORTX");  // Gate's frame

    if (gamemode != shareware)
    bmaptexture07 = R_CheckTextureNumForName("SW51_ON");
    
    // brightmap_yellowred
    bmaptexture09 = R_CheckTextureNumForName("SPAWN09");
    bmaptexture10 = R_CheckTextureNumForName("SPAWN10");
}

