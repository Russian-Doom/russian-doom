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
//
// DESCRIPTION:
//	Switches, buttons. Two-state animation. Exits.
//


#include "i_system.h"
#include "deh_main.h"
#include "p_local.h"
#include "g_game.h"
#include "s_sound.h"
#include "doomstat.h"
#include "i_swap.h"
#include "w_wad.h"
#include "z_zone.h"
#include "jn.h"


// =============================================================================
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
// =============================================================================

// [crispy] add support for SWITCHES lumps
static switchlist_t alphSwitchList_vanilla[] =
{
    // Doom shareware episode 1 switches
    {"SW1BRCOM",    "SW2BRCOM", 1},
    {"SW1BRN1",     "SW2BRN1",  1},
    {"SW1BRN2",     "SW2BRN2",  1},
    {"SW1BRNGN",    "SW2BRNGN", 1},
    {"SW1BROWN",    "SW2BROWN", 1},
    {"SW1COMM",     "SW2COMM",  1},
    {"SW1COMP",     "SW2COMP",  1},
    {"SW1DIRT",     "SW2DIRT",  1},
    {"SW1EXIT",     "SW2EXIT",  1},
    {"SW1GRAY",     "SW2GRAY",  1},
    {"SW1GRAY1",	"SW2GRAY1", 1},
    {"SW1METAL",	"SW2METAL", 1},
    {"SW1PIPE",     "SW2PIPE",  1},
    {"SW1SLAD",     "SW2SLAD",  1},
    {"SW1STARG",    "SW2STARG", 1},
    {"SW1STON1",    "SW2STON1", 1},
    {"SW1STON2",    "SW2STON2", 1},
    {"SW1STONE",    "SW2STONE", 1},
    {"SW1STRTN",    "SW2STRTN", 1},

    // Doom registered episodes 2&3 switches
    {"SW1BLUE",     "SW2BLUE",  2},
    {"SW1CMT",      "SW2CMT",   2},
    {"SW1GARG",     "SW2GARG",  2},
    {"SW1GSTON",    "SW2GSTON", 2},
    {"SW1HOT",      "SW2HOT",   2},
    {"SW1LION",     "SW2LION",  2},
    {"SW1SATYR",    "SW2SATYR", 2},
    {"SW1SKIN",     "SW2SKIN",  2},
    {"SW1VINE",     "SW2VINE",  2},
    {"SW1WOOD",     "SW2WOOD",  2},

    // Doom II switches
    {"SW1PANEL",    "SW2PANEL", 3},
    {"SW1ROCK",     "SW2ROCK",  3},
    {"SW1MET2",     "SW2MET2",  3},
    {"SW1WDMET",    "SW2WDMET", 3},
    {"SW1BRIK",     "SW2BRIK",  3},
    {"SW1MOD1",     "SW2MOD1",  3},
    {"SW1ZIM",      "SW2ZIM",   3},
    {"SW1STON6",    "SW2STON6", 3},
    {"SW1TEK",      "SW2TEK",   3},
    {"SW1MARB",     "SW2MARB",  3},
    {"SW1SKULL",    "SW2SKULL", 3},

    // [crispy] SWITCHES lumps are supposed to end like this
    {"\0",          "\0",       0}
};

// [crispy] remove MAXSWITCHES limit
static size_t  maxswitches;
static int    *switchlist;
static int     numswitches;
static int     maxbuttons; // [crispy] remove MAXBUTTONS limit
button_t      *buttonlist; // [crispy] remove MAXBUTTONS limit


// -----------------------------------------------------------------------------
// P_InitSwitchList
// Only called at game initialization.
// -----------------------------------------------------------------------------

void P_InitSwitchList (void)
{
    int i, slindex, episode;

    // [crispy] add support for SWITCHES lumps
    switchlist_t *alphSwitchList;
    boolean from_lump;

    if ((from_lump = (W_CheckNumForName("SWITCHES") != -1)))
    {
        alphSwitchList = W_CacheLumpName("SWITCHES", PU_STATIC);
    }
    else
    {
        alphSwitchList = alphSwitchList_vanilla;
    }

    // Note that this is called "episode" here but it's actually something
    // quite different. As we progress from Shareware->Registered->Doom II
    // we support more switch textures.
    switch (gamemode)
    {
        case registered:
        case retail:
        case pressbeta:
            episode = 2;
            break;
        case commercial:
            episode = 3;
            break;
        default:
            episode = 1;
            break;
    }

    slindex = 0;

    for (i = 0; alphSwitchList[i].episode; i++)
    {
        const short alphSwitchList_episode = from_lump ?
                    SHORT(alphSwitchList[i].episode) :
                    alphSwitchList[i].episode;

        // [crispy] remove MAXSWITCHES limit
        if (slindex + 1 >= maxswitches)
        {
            size_t newmax = maxswitches ? 2 * maxswitches : MAXSWITCHES;
            switchlist = I_Realloc(switchlist, newmax * sizeof(*switchlist));
            maxswitches = newmax;
        }

        // [crispy] ignore switches referencing unknown texture names,
        // warn if either one is missing, but only add if both are valid
        if (alphSwitchList_episode <= episode)
        {
            int texture1, texture2;
            char *name1 = DEH_String(alphSwitchList[i].name1);
            char *name2 = DEH_String(alphSwitchList[i].name2);

            texture1 = R_CheckTextureNumForName(name1);
            texture2 = R_CheckTextureNumForName(name2);

            if (texture1 == -1 || texture2 == -1)
            {
                fprintf(stderr, english_language ? 
                        "P_InitSwitchList: could not add %s(%d)/%s(%d)\n" :
                        "P_InitSwitchList: невозможно добавить %s(%d)/%s(%d)\n",
                        name1, texture1, name2, texture2);
            }
            else
            {
                switchlist[slindex++] = texture1;
                switchlist[slindex++] = texture2;
            }
        }
    }

    numswitches = slindex / 2;
    switchlist[slindex] = -1;

    // [crispy] add support for SWITCHES lumps
    if (from_lump)
    {
        W_ReleaseLumpName("SWITCHES");
    }

    // [crispy] pre-allocate some memory for the buttonlist[] array
    buttonlist = I_Realloc(NULL, sizeof(*buttonlist) * (maxbuttons = MAXBUTTONS));
    memset(buttonlist, 0, sizeof(*buttonlist) * maxbuttons);
}

// -----------------------------------------------------------------------------
// P_StartButton
// Start a button counting down till it turns off.
// -----------------------------------------------------------------------------

void P_StartButton (line_t *line, const bwhere_e w, const int texture, const int time)
{
    int i;

    // See if button is already pressed
    for (i = 0 ; i < maxbuttons ; i++)
    {
        if (buttonlist[i].btimer && buttonlist[i].line == line)
        {
            // [crispy] register up to three buttons at once for lines 
            // with more than one switch texture
            if (buttonlist[i].where == w)
            {
                return;
            }
        }
    }

    for (i = 0 ; i < maxbuttons ; i++)
    {
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = &line->soundorg; // [from-crispy] Corrected sound source
            return;
        }
    }

    // [crispy] remove MAXBUTTONS limit
    {
        maxbuttons = 2 * maxbuttons;
        buttonlist = I_Realloc(buttonlist, sizeof(*buttonlist) * maxbuttons);
        memset(buttonlist + maxbuttons/2, 0, sizeof(*buttonlist) * maxbuttons/2);
        return P_StartButton(line, w, texture, time);
    }

    I_Error(english_language ?
            "P_StartButton: no button slots left!" :
            "P_StartButton: превышен лимит слотов для переключателей!");
}

// -----------------------------------------------------------------------------
// P_ChangeSwitchTexture
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
// -----------------------------------------------------------------------------

void P_ChangeSwitchTexture (line_t *line, const int useAgain)
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     sound = sfx_swtchn;
    int     i;
    boolean playsound = false;

    if (correct_endlevel_sfx && !vanillaparm)
    {
        // EXIT SWITCH?
        if (line->special == 11 || line->special == 51)
        {
            sound = sfx_swtchx;
        }
    }
    else if (gamemission == doom2 && gamemap == 31 && canmodify && !vanillaparm)
    {
        // [JN] Exit switch sound from Wolfenstein 3D
        if (line->special == 11 || line->special == 51)
        {
            sound = sfx_swtchw;
        }
    }

    if (!useAgain)
    {
        line->special = 0;
    }

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;

    // Fix vanilla bug of non-working switch animations in some instances.
    // Code by Fabian Greffrath (previously by Brad Harding),
    // discovered by Julian Nechaevsky (17.03.2018).
    for (i = 0 ; i < numswitches*2 ; i++)
    {
        if (switchlist[i] == texTop)
        {
            playsound = true;
            sides[line->sidenum[0]].toptexture = switchlist[i^1];

            if (useAgain)
            {
                P_StartButton(line, top, switchlist[i], BUTTONTIME);
            }
        }

        // [crispy] register up to three buttons at once for lines 
        // with more than one switch texture
        if (switchlist[i] == texMid)
        {
            playsound = true;
            sides[line->sidenum[0]].midtexture = switchlist[i^1];

            if (useAgain)
            {
                P_StartButton(line, middle, switchlist[i], BUTTONTIME);
            }
        }

        if (switchlist[i] == texBot)
        {
            playsound = true;
            sides[line->sidenum[0]].bottomtexture = switchlist[i^1];

            if (useAgain)
            {
                P_StartButton(line, bottom,switchlist[i],BUTTONTIME);
            }
        }
    }

    // [crispy] corrected sound source
    if (playsound)
    {
        // [JN] Z-axis sfx distance: sound invoked from the floor segmented source
        if (line->backsector 
        &&  line->backsector->floorheight > line->frontsector->floorheight)
        {
            line->soundorg.z = (line->backsector->floorheight 
                                -  line->frontsector->floorheight) / 2;
        }
        // [JN] Z-axis sfx distance: sound invoked from the ceiling segmented source
        else 
        if (line->backsector 
        &&  line->backsector->ceilingheight < line->frontsector->ceilingheight)
        {
            line->soundorg.z = (line->frontsector->ceilingheight
                             +  line->backsector->ceilingheight) / 2;
        }
        // [JN] Z-axis sfx distance: sound invoked from the middle of the line
        else
        {
            line->soundorg.z = (line->frontsector->ceilingheight
                             +  line->frontsector->floorheight) / 2;
        }

        S_StartSound(vanillaparm ? &line->soundorg : buttonlist->soundorg,sound);
    }
}

// -----------------------------------------------------------------------------
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
// -----------------------------------------------------------------------------

const boolean P_UseSpecialLine (mobj_t *thing, line_t *line, const int side)
{               
    if (side)
    {
        // [JN] Do not check for back sides. It was supposed 
        // to be used only for special 124 (Sliding doors).
        return false;
    }

    // Switches that other things can activate.
    if (!thing->player)
    {
        // never open secret doors
        if (line->flags & ML_SECRET)
        {
            return false;
        }

        switch (line->special)
        {
            case 1:     // MANUAL DOOR RAISE
            case 32:    // MANUAL BLUE
            case 33:    // MANUAL RED
            case 34:    // MANUAL YELLOW
            break;
	    
            default:
                return false;
            break;
        }
    }
    
    // do something  
    switch (line->special)
    {
        // MANUALS
        case 1:     // Vertical Door
        case 26:    // Blue Door/Locked
        case 27:    // Yellow Door /Locked
        case 28:    // Red Door /Locked

        case 31:    // Manual door open
        case 32:    // Blue locked door open
        case 33:    // Red locked door open
        case 34:    // Yellow locked door open

        case 117:   // Blazing door raise
        case 118:   // Blazing door open
        EV_VerticalDoor (line, thing);
        break;
	
        // [JN] Non-switches, just play an "oof" sound by pressing "use":
        case 24:    // Floor Raise to Lowest Ceiling
        case 46:    // Open Door Impact
        case 47:    // Floor Raise to Next Higher Floor (changes texture)
        case 48:    // Scroll Texture Left
        case 85:    // Scroll Texture Right (BOOM)
        if (PTR_NoWayAudible(line))
        {
            S_StartSound(NULL,sfx_oof);
        }
        break;

        // SWITCHES
        case 7:
        // Build Stairs
        if (EV_BuildStairs(line,build8))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 9:
        // Change Donut
        if (EV_DoDonut(line))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 11:
        // Exit level
        P_ChangeSwitchTexture(line,0);
        G_ExitLevel ();
        break;
	
        case 14:
        // Raise Floor 32 and change texture
        if (EV_DoPlat(line,raiseAndChange,32))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 15:
        // Raise Floor 24 and change texture
        if (EV_DoPlat(line,raiseAndChange,24))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 18:
        // Raise Floor to next highest floor
        if (EV_DoFloor(line, raiseFloorToNearest))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 20:
        // Raise Plat next highest floor and change texture
        if (EV_DoPlat(line,raiseToNearestAndChange,0))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 21:
        // PlatDownWaitUpStay
        if (EV_DoPlat(line,downWaitUpStay,0))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 23:
        // Lower Floor to Lowest
        if (EV_DoFloor(line,lowerFloorToLowest))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 29:
        // Raise Door
        if (EV_DoDoor(line,vld_normal))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 41:
        // Lower Ceiling to Floor
        if (EV_DoCeiling(line,lowerToFloor))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 71:
        // Turbo Lower Floor
        if (EV_DoFloor(line,turboLower))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 49:
        // Ceiling Crush And Raise
        if (EV_DoCeiling(line,crushAndRaise))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 50:
        // Close Door
        if (EV_DoDoor(line,vld_close))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 51:
        // Secret EXIT
        P_ChangeSwitchTexture(line,0);
        G_SecretExitLevel ();
        break;

        case 55:
        // Raise Floor Crush
        if (EV_DoFloor(line,raiseFloorCrush))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 101:
        // Raise Floor
        if (EV_DoFloor(line,raiseFloor))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 102:
        // Lower Floor to Surrounding floor height
        if (EV_DoFloor(line,lowerFloor))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 103:
        // Open Door
        if (EV_DoDoor(line,vld_open))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 111:
        // Blazing Door Raise (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeRaise))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 112:
        // Blazing Door Open (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeOpen))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 113:
        // Blazing Door Close (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeClose))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 122:
        // Blazing PlatDownWaitUpStay
        if (EV_DoPlat(line,blazeDWUS,0))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 127:
        // Build Stairs Turbo 16
        if (EV_BuildStairs(line,turbo16))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 131:
        // Raise Floor Turbo
        if (EV_DoFloor(line,raiseFloorTurbo))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 133:
        // BlzOpenDoor BLUE
        case 135:
        // BlzOpenDoor RED
        case 137:
        // BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,vld_blazeOpen,thing))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        case 140:
        // Raise Floor 512
        if (EV_DoFloor(line,raiseFloor512))
        {
            P_ChangeSwitchTexture(line,0);
        }
        break;

        // BUTTONS
        case 42:
        // Close Door
        if (EV_DoDoor(line,vld_close))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 43:
        // Lower Ceiling to Floor
        if (EV_DoCeiling(line,lowerToFloor))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 45:
        // Lower Floor to Surrounding floor height
        if (EV_DoFloor(line,lowerFloor))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 60:
        // Lower Floor to Lowest
        if (EV_DoFloor(line,lowerFloorToLowest))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 61:
        // Open Door
        if (EV_DoDoor(line,vld_open))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 62:
        // PlatDownWaitUpStay
        if (EV_DoPlat(line,downWaitUpStay,1))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 63:
        // Raise Door
        if (EV_DoDoor(line,vld_normal))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 64:
        // Raise Floor to ceiling
        if (EV_DoFloor(line,raiseFloor))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 66:
        // Raise Floor 24 and change texture
        if (EV_DoPlat(line,raiseAndChange,24))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 67:
        // Raise Floor 32 and change texture
        if (EV_DoPlat(line,raiseAndChange,32))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 65:
        // Raise Floor Crush
        if (EV_DoFloor(line,raiseFloorCrush))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 68:
        // Raise Plat to next highest floor and change texture
        if (EV_DoPlat(line,raiseToNearestAndChange,0))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 69:
        // Raise Floor to next highest floor
        if (EV_DoFloor(line, raiseFloorToNearest))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 70:
        // Turbo Lower Floor
        if (EV_DoFloor(line,turboLower))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 114:
        // Blazing Door Raise (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeRaise))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 115:
        // Blazing Door Open (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeOpen))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 116:
        // Blazing Door Close (faster than TURBO!)
        if (EV_DoDoor (line,vld_blazeClose))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 123:
        // Blazing PlatDownWaitUpStay
        if (EV_DoPlat(line,blazeDWUS,0))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 132:
        // Raise Floor Turbo
        if (EV_DoFloor(line,raiseFloorTurbo))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 99:
        // BlzOpenDoor BLUE
        case 134:
        // BlzOpenDoor RED
        case 136:
        // BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,vld_blazeOpen,thing))
        {
            P_ChangeSwitchTexture(line,1);
        }
        break;

        case 138:
        // Light Turn On
        EV_LightTurnOn(line,255);
        P_ChangeSwitchTexture(line,1);
        break;

        case 139:
        // Light Turn Off
        EV_LightTurnOn(line,35);
        P_ChangeSwitchTexture(line,1);
        break;
    }

    return true;
}
