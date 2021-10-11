//
// Copyright(C) 2017-2018 Julian Nechaevsky
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
#include "doomstat.h"
#include "r_bmaps.h"
#include "jn.h"


// Floors and ceilings:
int bmapflatnum1, bmapflatnum2, bmapflatnum3;

// Walls:
int bmaptexture01, bmaptexture02, bmaptexture03, bmaptexture04, bmaptexture05,
    bmaptexture06, bmaptexture07, bmaptexture08, bmaptexture09, bmaptexture10,
    bmaptexture11, bmaptexture12, bmaptexture13, bmaptexture14, bmaptexture15,
    bmaptexture16, bmaptexture17, bmaptexture18, bmaptexture19, bmaptexture20,
    bmaptexture21, bmaptexture22, bmaptexture23, bmaptexture24, bmaptexture25,
    bmaptexture26, bmaptexture27, bmaptexture28, bmaptexture29, bmaptexture30,
    bmaptexture31, bmaptexture32, bmaptexture33, bmaptexture34, bmaptexture35,
    bmaptexture36, bmaptexture37, bmaptexture38, bmaptexture39, bmaptexture40,
    bmaptexture41, bmaptexture42, bmaptexture43, bmaptexture44, bmaptexture45,
    bmaptexture46, bmaptexture47, bmaptexture48, bmaptexture49, bmaptexture50,
    bmaptexture51, bmaptexture52, bmaptexture53, bmaptexture54, bmaptexture55,
    bmaptexture56, bmaptexture57, bmaptexture58, bmaptexture59, bmaptexture60,
    bmaptexture61, bmaptexture62, bmaptexture63, bmaptexture64, bmaptexture65,
    bmaptexture66, bmaptexture67, bmaptexture68, bmaptexture69, bmaptexture70,
    bmaptexture71, bmaptexture72, bmaptexture73, bmaptexture74, bmaptexture75,
    bmaptexture76, bmaptexture77, bmaptexture78, bmaptexture79, bmaptexture80,
    bmaptexture81, bmaptexture82, bmaptexture83, bmaptexture84, bmaptexture85,
    bmaptexture86, bmaptexture87, bmaptexture88, bmaptexture89, bmaptexture90,
    bmaptexture91, bmaptexture92, bmaptexture93, bmaptexture94;

//
// [JN] Lookup and init all the textures for brightmapping.
// This function is called at startup, see R_Init.
//

void R_InitBrightmaps(void)
{
    // Print informative message while startup
    printf(english_language ?
           "\nR_Init: Init brightmapping." :
           "\nR_Init: Инициализация брайтмаппинга.");

    // Texture lookup. There are many strict definitions,
    // for example, no need to lookup Doom 1 textures in TNT.

    // -------------------------------------------------------
    //  Flats and ceilings (available in all games)
    // -------------------------------------------------------
    {
        bmapflatnum1 = R_FlatNumForName("CONS1_1");
        bmapflatnum2 = R_FlatNumForName("CONS1_5");
        bmapflatnum3 = R_FlatNumForName("CONS1_7");
    }

    // -------------------------------------------------------
    //  Not in Shareware
    // -------------------------------------------------------
    if (!shareware)
    {
        // Red only
        bmaptexture08 = R_TextureNumForName("SW2WOOD");
        bmaptexture09 = R_TextureNumForName("WOOD4");
        bmaptexture11 = R_TextureNumForName("SLADSKUL");
        bmaptexture16 = R_TextureNumForName("SW2BLUE");
        bmaptexture17 = R_TextureNumForName("SW2GSTON");
        bmaptexture23 = R_TextureNumForName("WOODGARG");
        bmaptexture34 = R_TextureNumForName("EXITSTON");

        // Green only 1
        bmaptexture73 = R_TextureNumForName("SW2VINE");

        // Bright tan
        bmaptexture86 = R_TextureNumForName("SW2SATYR");
        bmaptexture87 = R_TextureNumForName("SW2LION");
        bmaptexture88 = R_TextureNumForName("SW2GARG");

        // Red only 2
        bmaptexture93 = R_TextureNumForName("SW2HOT");
    }

    // -------------------------------------------------------
    //  Doom 1 only, not in Shareware
    // -------------------------------------------------------
    if (registered || retail)
    {
        // Red only
        bmaptexture10 = R_TextureNumForName("WOODSKUL");
    }

    // -------------------------------------------------------
    //  Doom 1 only
    // -------------------------------------------------------
    if (shareware || registered || retail)
    {
        // Not gray
        bmaptexture30 = R_TextureNumForName("PLANET1");
        bmaptexture38 = R_TextureNumForName("LITEBLU2");

        // Not gray or brown
        bmaptexture40 = R_TextureNumForName("COMP2");
        bmaptexture41 = R_TextureNumForName("COMPUTE2");
        bmaptexture43 = R_TextureNumForName("COMPUTE1");
        bmaptexture44 = R_TextureNumForName("COMPUTE3");

        // Red only 1
        bmaptexture89 = R_TextureNumForName("TEKWALL2");
        bmaptexture90 = R_TextureNumForName("TEKWALL5");

        // SW2STON2 is a special case.
        // It's presented in both Doom 1 and Doom 2,
        // but uses "redonly" in Doom 1 and "greenonly2" in Doom 2.
        // See below for `else` condition and different bmap name.
        bmaptexture24 = R_TextureNumForName("SW2STON2");
    }
    else
    {
        bmaptexture94 = R_TextureNumForName("SW2STON2");
    }

    // -------------------------------------------------------
    //  Sigil
    // -------------------------------------------------------
    if (sigil)
    {
        bmaptexture92 = R_TextureNumForName("SIGIL");
    }

    // -------------------------------------------------------
    //  Not in Doom 1
    // -------------------------------------------------------
    if (commercial)
    {
        // Red only
        bmaptexture01 = R_TextureNumForName("SW1STARG");
        bmaptexture02 = R_TextureNumForName("SW2MARB");
        bmaptexture06 = R_TextureNumForName("SW2PANEL");
        bmaptexture12 = R_TextureNumForName("SW1BRIK");
        bmaptexture14 = R_TextureNumForName("SW1MET2");
        bmaptexture18 = R_TextureNumForName("SW2ROCK");
        bmaptexture19 = R_TextureNumForName("SW2STON6");
        bmaptexture20 = R_TextureNumForName("SW2ZIM");
        bmaptexture25 = R_TextureNumForName("SW1BRN1");
        bmaptexture26 = R_TextureNumForName("SW1STON2");
        bmaptexture91 = R_TextureNumForName("METAL3");

        // Not gray or brown
        bmaptexture35 = R_TextureNumForName("SILVER2");
        bmaptexture42 = R_TextureNumForName("SILVER3");

        // Green only 1
        bmaptexture45 = R_TextureNumForName("SW2MOD1");
        bmaptexture58 = R_TextureNumForName("SPCDOOR3");
        bmaptexture66 = R_TextureNumForName("SW2TEK");
        bmaptexture67 = R_TextureNumForName("SW2BRIK");
        bmaptexture71 = R_TextureNumForName("SW2MET2");
        bmaptexture74 = R_TextureNumForName("PIPEWAL1");
        bmaptexture75 = R_TextureNumForName("TEKLITE2");

        // Green only 2
        bmaptexture61 = R_TextureNumForName("SW2STARG");
        bmaptexture62 = R_TextureNumForName("SW2BRN1");

        // Orange and yellow
        bmaptexture81 = R_TextureNumForName("TEKBRON2");
    }

    // -------------------------------------------------------
    //  Doom 2 only
    // -------------------------------------------------------
    if (commercial && !tnt && !plutonia)
    {
        // Green only 2
        bmaptexture78 = R_TextureNumForName("SW2SKULL");
    }

    // -------------------------------------------------------
    //  TNT Evilution only
    // -------------------------------------------------------
    if (tnt)
    {
        // Red only
        bmaptexture27 = R_TextureNumForName("LITERED2");
        bmaptexture28 = R_TextureNumForName("PNK4EXIT");

        // Not gray or brown
        bmaptexture46 = R_TextureNumForName("BTNTMETL");
        bmaptexture47 = R_TextureNumForName("BTNTSLVR");
        bmaptexture48 = R_TextureNumForName("SLAD2");
        bmaptexture49 = R_TextureNumForName("SLAD3");
        bmaptexture50 = R_TextureNumForName("SLAD4");
        bmaptexture51 = R_TextureNumForName("SLAD5");
        bmaptexture52 = R_TextureNumForName("SLAD6");
        bmaptexture53 = R_TextureNumForName("SLAD7");
        bmaptexture54 = R_TextureNumForName("SLAD8");
        bmaptexture55 = R_TextureNumForName("SLAD9");
        bmaptexture56 = R_TextureNumForName("SLAD10");
        bmaptexture57 = R_TextureNumForName("SLAD11");
        bmaptexture59 = R_TextureNumForName("SLADRIP1");
        bmaptexture60 = R_TextureNumForName("SLADRIP3");

        // Green only 2
        bmaptexture79 = R_TextureNumForName("M_TEC");

        // Orange and yellow
        bmaptexture82 = R_TextureNumForName("LITEYEL2");
        bmaptexture83 = R_TextureNumForName("LITEYEL3");
        bmaptexture84 = R_TextureNumForName("YELMETAL");
    }
    // -------------------------------------------------------
    //  Plutonia only
    // -------------------------------------------------------
    if (plutonia)
    {
        // Dimmed items (red color)
        bmaptexture85 = R_TextureNumForName("SW2SKULL");
    }

    // -------------------------------------------------------
    //  All games
    // -------------------------------------------------------
    {
        // Red only
        bmaptexture03 = R_TextureNumForName("SW1BRCOM");
        bmaptexture04 = R_TextureNumForName("SW1DIRT");
        bmaptexture05 = R_TextureNumForName("SW1STRTN");
        bmaptexture07 = R_TextureNumForName("SW2SLAD");
        bmaptexture13 = R_TextureNumForName("SW1COMM");
        bmaptexture15 = R_TextureNumForName("SW1STON1");
        bmaptexture21 = R_TextureNumForName("SW2COMP");
        bmaptexture22 = R_TextureNumForName("SW1STONE");
        bmaptexture39 = R_TextureNumForName("EXITSIGN");

        // Not gray
        bmaptexture29 = R_TextureNumForName("COMPSTA2");
        bmaptexture31 = R_TextureNumForName("SW2EXIT");
        bmaptexture32 = R_TextureNumForName("SW2GRAY1");
        bmaptexture33 = R_TextureNumForName("COMPSTA1");
        bmaptexture36 = R_TextureNumForName("LITEBLU1");
        bmaptexture37 = R_TextureNumForName("SW2GRAY");

        // Green only 1
        bmaptexture68 = R_TextureNumForName("SW2BRN2");
        bmaptexture69 = R_TextureNumForName("SW2COMM");
        bmaptexture72 = R_TextureNumForName("SW2STRTN");

        // Green only 2
        bmaptexture63 = R_TextureNumForName("SW2BRCOM");
        bmaptexture64 = R_TextureNumForName("SW2STON1");
        bmaptexture65 = R_TextureNumForName("SW2STONE");
        bmaptexture70 = R_TextureNumForName("SW2DIRT");

        // Green only 3
        bmaptexture77 = R_TextureNumForName("SW2BRNGN");
        bmaptexture80 = R_TextureNumForName("SW2METAL");
    }
}
