//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2022 Julian Nechaevsky
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


#include "i_timer.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "h2def.h"
#include "rd_psx.h"
#include "s_sound.h"


// EXTERNAL DATA DECLARATIONS --------------------------------------------------

extern int demosequence;
extern int pagetic;
extern char *pagename;
extern boolean advancedemo;

// CODE ------------------------------------------------------------------------


/*
================================================================================
=
= PageDrawerPSX
=
= [JN] Own routine for demo sequence pages drawing.
=
================================================================================
*/

void PageDrawerPSX (void)
{
    // Fill background with black color.
    V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);

    // Cycle through LEGAL/GT/RAVEN pages with fade to black effect.
    if (demosequence < 4)
    {
        if (pagetic <= 170 && pagetic >= 5)
        {
            V_DrawPatch(0, 0, W_CacheLumpName(pagename, PU_CACHE),
                        pagetic >= 168 ? transtable10 :
                        pagetic >= 166 ? transtable20 :
                        pagetic >= 164 ? transtable30 :
                        pagetic >= 162 ? transtable40 :
                        pagetic >= 160 ? transtable50 :
                        pagetic >= 158 ? transtable60 :
                        pagetic >= 156 ? transtable70 :
                        pagetic >= 154 ? transtable80 :
                        pagetic >= 152 ? transtable90 :
                        pagetic >=  23 ? NULL :
                        pagetic >=  21 ? transtable90 :
                        pagetic >=  19 ? transtable80 :
                        pagetic >=  17 ? transtable70 :
                        pagetic >=  15 ? transtable60 :
                        pagetic >=  13 ? transtable50 :
                        pagetic >=  11 ? transtable40 :
                        pagetic >=   9 ? transtable30 :
                        pagetic >=   7 ? transtable20 :
                                         transtable10);
        }
    }
    else
    {
        // Draw title graphics normally.
        V_DrawPatchFullScreen(W_CacheLumpName(pagename, PU_CACHE), false);
    }

    UpdateState |= I_FULLSCRN;
}

/*
================================================================================
=
= H2_DoAdvanceDemoPSX
=
= [JN] Own routine for demo sequence pages cycling.
=
================================================================================
*/

void H2_DoAdvanceDemoPSX (void)
{
    S_ResumeSound();    // [JN] Fix vanilla Hexen bug: resume music playing
    players[consoleplayer].playerstate = PST_LIVE;      // don't reborn
    advancedemo = false;
    usergame = false;           // can't save/end game here
    paused = false;
    gameaction = ga_nothing;
    gamestate = GS_DEMOSCREEN;
    demosequence = (demosequence + 1) % 5;
 
    switch (demosequence)
    {
        case 0:
        {
            // Blank screen, just for small delay before other pages.
            pagetic = 2 * TICRATE;
            gamestate = GS_DEMOSCREEN;
            pagename = "TNT1A0";
        }
        break;

        case 1:
        {
            pagetic = 5 * TICRATE;
            gamestate = GS_DEMOSCREEN;
            pagename = "LEGAL";
        }
        break;

        case 2:
        {
            pagetic = 5 * TICRATE;
            gamestate = GS_DEMOSCREEN;
            pagename = "GT";
        }
        break;

        case 3:
        {
            pagetic = 5 * TICRATE;
            gamestate = GS_DEMOSCREEN;
            pagename = "RAVEN";
        }
        break;

        case 4:
        {
            pagetic = 60 * TICRATE;
            gamestate = GS_DEMOSCREEN;
            pagename = "TITLE";
            S_StartSongName("hexen", false);
        }
        break;
    }
}
