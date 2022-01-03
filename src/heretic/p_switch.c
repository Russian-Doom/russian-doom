//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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



#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "jn.h"

//==================================================================
//
//      CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
//==================================================================
switchlist_t alphSwitchList[] =
{
    {"SW1OFF", "SW1ON", 1},
    {"SW2OFF", "SW2ON", 1},
    {"\0", "\0", 0}
};

int switchlist[MAXSWITCHES * 2];
int numswitches;
button_t buttonlist[MAXBUTTONS];

/*
===============
=
= P_InitSwitchList
=
= Only called at game initialization
=
===============
*/

void P_InitSwitchList(void)
{
    int i;
    int index;
    int episode;

    episode = 1;
    if (gamemode != shareware)
        episode = 2;

    for (index = 0, i = 0; i < MAXSWITCHES; i++)
    {
        if (!alphSwitchList[i].episode)
        {
            numswitches = index / 2;
            switchlist[index] = -1;
            break;
        }

        if (alphSwitchList[i].episode <= episode)
        {
            switchlist[index++] =
                R_TextureNumForName(DEH_String(alphSwitchList[i].name1));
            switchlist[index++] =
                R_TextureNumForName(DEH_String(alphSwitchList[i].name2));
        }
    }
}

//==================================================================
//
//      Start a button counting down till it turns off.
//
//==================================================================
void P_StartButton(line_t * line, bwhere_e w, int texture, int time)
{
    int i;

    for (i = 0; i < MAXBUTTONS; i++)
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = &line->soundorg; // [from-crispy] Corrected sound source
            return;
        }

    I_Error(english_language ?
            "P_StartButton: no button slots left!" :
            "P_StartButton: превышен лимит слотов для переключателей!");
}

//==================================================================
//
//      Function that changes wall texture.
//      Tell it if switch is ok to use again (1=yes, it's a button).
//
//==================================================================
void P_ChangeSwitchTexture(line_t * line, int useAgain)
{
    int texTop;
    int texMid;
    int texBot;
    int i;
    int sound;
    // [crispy] register up to three buttons at once 
    // for lines with more than one switch texture.
    boolean playsound = false;

    if (!useAgain)
        line->special = 0;

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;

    sound = sfx_switch;

    for (i = 0; i < numswitches * 2; i++)
    {
        if (switchlist[i] == texTop)
        {
            playsound = true;
            sides[line->sidenum[0]].toptexture = switchlist[i ^ 1];
            if (useAgain)
                P_StartButton(line, top, switchlist[i], BUTTONTIME);
        }
        if (switchlist[i] == texMid)
        {
            playsound = true;
            sides[line->sidenum[0]].midtexture = switchlist[i ^ 1];
            if (useAgain)
                P_StartButton(line, middle, switchlist[i], BUTTONTIME);
        }
        if (switchlist[i] == texBot)
        {
            playsound = true;
            sides[line->sidenum[0]].bottomtexture = switchlist[i ^ 1];
            if (useAgain)
                P_StartButton(line, bottom, switchlist[i], BUTTONTIME);
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

/*
==============================================================================
=
= P_UseSpecialLine
=
= Called when a thing uses a special line
= Only the front sides of lines are usable
===============================================================================
*/

boolean P_UseSpecialLine(mobj_t * thing, line_t * line)
{
    //
    //      Switches that other things can activate
    //
    if (!thing->player)
    {
        if (line->flags & ML_SECRET)
            return false;       // never open secret doors
        switch (line->special)
        {
            case 1:            // MANUAL DOOR RAISE
            case 32:           // MANUAL BLUE
            case 33:           // MANUAL RED
            case 34:           // MANUAL YELLOW
                break;
            default:
                return false;
        }
    }

    //
    // do something
    //      
    switch (line->special)
    {
            //===============================================
            //      MANUALS
            //===============================================
        case 1:                // Vertical Door
        case 26:               // Blue Door/Locked
        case 27:               // Yellow Door /Locked
        case 28:               // Red Door /Locked

        case 31:               // Manual door open
        case 32:               // Blue locked door open
        case 33:               // Red locked door open
        case 34:               // Yellow locked door open
            EV_VerticalDoor(line, thing);
            break;
            //===============================================
            //      SWITCHES
            //===============================================
        case 7:                // Switch_Build_Stairs (8 pixel steps)
            if (EV_BuildStairs(line, 8 * FRACUNIT))
            {
                P_ChangeSwitchTexture(line, 0);
            }
            break;
        case 107:              // Switch_Build_Stairs_16 (16 pixel steps)
            if (EV_BuildStairs(line, 16 * FRACUNIT))
            {
                P_ChangeSwitchTexture(line, 0);
            }
            break;
        case 9:                // Change Donut
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 11:               // Exit level
            G_ExitLevel();
            P_ChangeSwitchTexture(line, 0);
            break;
        case 14:               // Raise Floor 32 and change texture
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 15:               // Raise Floor 24 and change texture
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 18:               // Raise Floor to next highest floor
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 20:               // Raise Plat next highest floor and change texture
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 21:               // PlatDownWaitUpStay
            if (EV_DoPlat(line, downWaitUpStay, 0))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 23:               // Lower Floor to Lowest
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 29:               // Raise Door
            if (EV_DoDoor(line, vld_normal, VDOORSPEED))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 41:               // Lower Ceiling to Floor
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 71:               // Turbo Lower Floor
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 49:               // Lower Ceiling And Crush
            if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 50:               // Close Door
            if (EV_DoDoor(line, vld_close, VDOORSPEED))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 51:               // Secret EXIT
            G_SecretExitLevel();
            P_ChangeSwitchTexture(line, 0);
            break;
        case 55:               // Raise Floor Crush
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 101:              // Raise Floor
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 102:              // Lower Floor to Surrounding floor height
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 103:              // Open Door
            if (EV_DoDoor(line, vld_open, VDOORSPEED))
                P_ChangeSwitchTexture(line, 0);
            break;
            //===============================================
            //      BUTTONS
            //===============================================
        case 42:               // Close Door
            if (EV_DoDoor(line, vld_close, VDOORSPEED))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 43:               // Lower Ceiling to Floor
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 45:               // Lower Floor to Surrounding floor height
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 60:               // Lower Floor to Lowest
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 61:               // Open Door
            if (EV_DoDoor(line, vld_open, VDOORSPEED))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 62:               // PlatDownWaitUpStay
            if (EV_DoPlat(line, downWaitUpStay, 1))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 63:               // Raise Door
            if (EV_DoDoor(line, vld_normal, VDOORSPEED))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 64:               // Raise Floor to ceiling
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 66:               // Raise Floor 24 and change texture
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 67:               // Raise Floor 32 and change texture
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 65:               // Raise Floor Crush
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 68:               // Raise Plat to next highest floor and change texture
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 69:               // Raise Floor to next highest floor
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 70:               // Turbo Lower Floor
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 1);
            break;
    }

    return true;
}
