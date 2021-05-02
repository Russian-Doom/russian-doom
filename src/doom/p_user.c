//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
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
//	Player related stuff.
//	Bobbing POV/weapon, movement.
//	Pending weapon.
//


#include "doomdef.h"
#include "d_event.h"
#include "p_local.h"
#include "doomstat.h"
#include "g_game.h"
#include "crispy.h"
#include "jn.h"


#define INVERSECOLORMAP     32  // Index of the special effects (INVUL inverse) map.
#define INFRAGREENCOLORMAP  1   // [JN] Infra green light amplification visor.
#define MAXBOB  0x100000        // 16 pixels of bob
#define ANG5    (ANG90/18)

boolean onground;


//
// P_Thrust
// Moves the given origin along a given angle.
//
void P_Thrust (player_t *player, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;

    player->mo->momx += FixedMul(move, finecosine[angle]); 
    player->mo->momy += FixedMul(move, finesine[angle]);
}


//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight (player_t *player, boolean safe)
{
    int      angle;
    fixed_t	 bob;
    extern boolean max_bobbing;

    // Regular movement bobbing
    // (needs to be calculated for gun swing
    // even if not on ground)
    // OPTIMIZE: tablify angle
    // Note: a LUT allows for effects
    //  like a ramp with low health.
    if (!safe)
    {
        player->bob =
        FixedMul (player->mo->momx, player->mo->momx) +
        FixedMul (player->mo->momy, player->mo->momy);

        // [JN] Reduce bobbing amplitude while not in full
        // speed movement. Affects both camera and weapon bobbing.
        if (singleplayer && !max_bobbing && !vanillaparm)
        {
            player->bob >>= 3;
        }
        else
        {
            player->bob >>= 2;
        }

        if (player->bob>MAXBOB)
        {
            player->bob = MAXBOB;
        }
    }

    if ((player->cheats & CF_NOMOMENTUM) || !onground)
    {
        player->viewz = player->mo->z + VIEWHEIGHT;

        if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
        {
            player->viewz = player->mo->ceilingz-4*FRACUNIT;
        }

        player->viewz = player->mo->z + player->viewheight;

        return;
    }

    angle = (FINEANGLES / 20 * leveltime) & FINEMASK;
    bob = FixedMul (player->bob / 2, finesine[angle]);

    // move viewheight
    if (!safe)
    {
        if (player->playerstate == PST_LIVE)
        {
            player->viewheight += player->deltaviewheight;

            if (player->viewheight > VIEWHEIGHT)
            {
                player->viewheight = VIEWHEIGHT;
                player->deltaviewheight = 0;
            }

            if (player->viewheight < VIEWHEIGHT / 2)
            {
                player->viewheight = VIEWHEIGHT / 2;

                if (player->deltaviewheight <= 0)
                {
                    player->deltaviewheight = 1;
                }
            }

            if (player->deltaviewheight)	
            {
                player->deltaviewheight += FRACUNIT / 4;

                if (!player->deltaviewheight)
                {
                    player->deltaviewheight = 1;
                }
            }

            // [crispy] squat down weapon sprite a bit after hitting the ground
            if (player->psp_dy_max)
            {
                player->psp_dy -= FRACUNIT;

                if (player->psp_dy < player->psp_dy_max)
                {
                    player->psp_dy = -player->psp_dy;
                }

                if (player->psp_dy == 0)
                {
                    player->psp_dy_max = 0;
                }
            }
        }
    }

    player->viewz = player->mo->z + player->viewheight + bob;

    if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
    {
        player->viewz = player->mo->ceilingz - 4 * FRACUNIT;
    }
}


//
// P_MovePlayer
//
void P_MovePlayer (player_t *player)
{
    ticcmd_t  *cmd = &player->cmd;
    int        look = cmd->lookfly & 15;;

    player->mo->angle += (cmd->angleturn << FRACBITS);

    // Do not let the player control movement if not onground.
    onground = (player->mo->z <= player->mo->floorz);

    if (cmd->forwardmove && onground)
    {
        P_Thrust (player, player->mo->angle, cmd->forwardmove*2048);
    }

    if (cmd->sidemove && onground)
    {
        P_Thrust (player, player->mo->angle-ANG90, cmd->sidemove*2048);
    }

    if ((cmd->forwardmove || cmd->sidemove) && player->mo->state == &states[S_PLAY])
    {
        P_SetMobjState (player->mo, S_PLAY_RUN1);
    }

    // [JN] Mouselook
    if (look > 7)
    {
        look -= 16;
    }
    if (look)
    {
        if (look == TOCENTER)
        {
            player->centering = true;
        }
        else
        {
            cmd->lookdir = MLOOKUNIT * 5 * look;
        }
    }
    if (player->centering)
    {
        if (player->lookdir > 0 || player->lookdir < 0)
        {
            player->lookdir = 0;
        }

        if (player->lookdir < 8 * MLOOKUNIT)
        {
            player->lookdir = 0;
            player->centering = false;
        }
    }
    if (!menuactive && !demoplayback)
    {
        player->lookdir = BETWEEN(-LOOKDIRMIN * MLOOKUNIT,
	                               LOOKDIRMAX * MLOOKUNIT,
	                               player->lookdir + cmd->lookdir);
    }
}	


//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
void P_DeathThink (player_t *player)
{
    angle_t angle;
    angle_t delta;

    P_MovePsprites (player);

    // fall to the ground
    if (player->viewheight > 6 * FRACUNIT)
    {
        player->viewheight -= FRACUNIT;
    }

    if (player->viewheight < 6 * FRACUNIT)
    {
        player->viewheight = 6 * FRACUNIT;
    }

    player->deltaviewheight = 0;
    onground = (player->mo->z <= player->mo->floorz);
    P_CalcHeight (player, false);

    // [JN] Mouselook: smoothed lookdir centering while dying
    if (player->lookdir >  8 * MLOOKUNIT)
    {
        player->lookdir -= 8 * MLOOKUNIT;
    }
    else 
    if (player->lookdir < -8 * MLOOKUNIT)
    {
        player->lookdir += 8 * MLOOKUNIT;
    }
    else
    if (player->lookdir)
    {
        player->lookdir = 0;
    }

    if (player->attacker && player->attacker != player->mo)
    {
        angle = R_PointToAngle2 (player->mo->x,
                                 player->mo->y,
                                 player->attacker->x,
                                 player->attacker->y);

        delta = angle - player->mo->angle;
	
        if (delta < ANG5 || delta > (unsigned)-ANG5)
        {
            // Looking at killer,
            //  so fade damage flash down.
            player->mo->angle = angle;

            if (player->damagecount)
            {
                player->damagecount--;
            }
        }
        else if (delta < ANG180)
        {
            player->mo->angle += ANG5;
        }
        else
        {
            player->mo->angle -= ANG5;
        }
    }
    else if (player->damagecount)
    {
        player->damagecount--;
    }

    if (player->cmd.buttons & BT_USE)
    {
        if (gamemode == pressbeta)
        {
            extern void D_StartTitle();

            // [JN] Press Beta: substract one life first
            lifecount--;

            // [JN] Press Beta: now checking for remaining amount
            if (lifecount < 0)
            {
                lifecount = 0;      // Don't display -1!
                D_StartTitle ();    // ... back to the Main Menu
                return;             // ... and don't go any farther.
            }
        }

        player->playerstate = PST_REBORN;
    }
}


//
// P_PlayerThink
//
void P_PlayerThink (player_t *player)
{
    ticcmd_t     *cmd;
    weapontype_t  newweapon;

    // [AM] Assume we can interpolate at the beginning
    //      of the tic.
    player->mo->interp = true;

    // [AM] Store starting position for player interpolation.
    player->mo->oldx = player->mo->x;
    player->mo->oldy = player->mo->y;
    player->mo->oldz = player->mo->z;
    player->mo->oldangle = player->mo->angle;
    player->oldviewz = player->viewz;
    player->oldlookdir = player->lookdir;

    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
    {
        player->mo->flags |= MF_NOCLIP;
    }
    else
    {
        player->mo->flags &= ~MF_NOCLIP;
    }
    
    // chain saw run forward
    cmd = &player->cmd;

    if (player->mo->flags & MF_JUSTATTACKED)
    {
        cmd->angleturn = 0;
        cmd->forwardmove = 0xc800 / 512;
        cmd->sidemove = 0;
        player->mo->flags &= ~MF_JUSTATTACKED;
    }

    if (player->playerstate == PST_DEAD)
    {
        P_DeathThink (player);
        return;
    }

    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    if (player->mo->reactiontime)
    {
        player->mo->reactiontime--;
    }
    else
    {
        P_MovePlayer (player);
    }

    P_CalcHeight (player, false);

    if (player->mo->subsector->sector->special)
    {
        P_PlayerInSpecialSector (player);
    }

    // Check for weapon change.

    // A special event has no other buttons.
    if (cmd->buttons & BT_SPECIAL)
    {
        cmd->buttons = 0;			
    }

    if (cmd->buttons & BT_CHANGE)
    {
        // The actual changing of the weapon is done
        //  when the weapon psprite can do it
        //  (read: not in the middle of an attack).
        newweapon = (cmd->buttons&BT_WEAPONMASK) >> BT_WEAPONSHIFT;

        // [crispy] allow to toggle Fist/Chainsaw
        if (newweapon == wp_fist
        && player->weaponowned[wp_chainsaw]
        && !(player->readyweapon == wp_chainsaw
        && (player->powers[pw_strength] || (singleplayer && !vanillaparm))))
        {
            newweapon = wp_chainsaw;
        }

        if ((gamemode == commercial)
        && gamemission != jaguar    // [JN] No Super Shotgun in Atari Jaguar
        && newweapon == wp_shotgun 
        && player->weaponowned[wp_supershotgun]
        && player->readyweapon != wp_supershotgun)
        {
            newweapon = wp_supershotgun;
        }

        if (player->weaponowned[newweapon] && newweapon != player->readyweapon)
        {
            // Do not go to plasma or BFG in shareware, even if cheated.
            if ((newweapon != wp_plasma && newweapon != wp_bfg)
            || (gamemode != shareware) )
            {
                player->pendingweapon = newweapon;
            }
        }
    }

    // check for use
    if (cmd->buttons & BT_USE)
    {
        if (!player->usedown)
        {
            P_UseLines (player);
            player->usedown = true;
        }
    }
    else
    {
        player->usedown = false;
    }

    // cycle psprites
    P_MovePsprites (player);

    // Counters, time dependend power ups.

    // Strength counts up to diminish fade.
    if (player->powers[pw_strength])
    {
        player->powers[pw_strength]++;	
    }

    if (player->powers[pw_invulnerability])
    {
        player->powers[pw_invulnerability]--;
    }

    if (player->powers[pw_invisibility])
        if (! --player->powers[pw_invisibility])
            player->mo->flags &= ~MF_SHADOW;

    if (player->powers[pw_infrared])
    {
        player->powers[pw_infrared]--;
    }

    if (player->powers[pw_ironfeet])
    {
        player->powers[pw_ironfeet]--;
    }

    if (player->damagecount)
    {
        player->damagecount--;
    }

    if (player->bonuscount)
    {
        player->bonuscount--;
    }

    //
    // Handling colormaps.
    //

    if (player->powers[pw_invulnerability])
    {
        if ((player->powers[pw_invulnerability] > 4 * 32
        || (player->powers[pw_invulnerability] & 8))
        && gamemission != jaguar)   // [JN] Jaguar Doom: no full bright effect while invulnerability
        {
            player->fixedcolormap = INVERSECOLORMAP;
        }
        else
        {
            // [JN] & [crispy] Если у игрока активирован визор и неуязвимость,
            // при спадании неузязвимости неинвертированные цвета должны
            // быть в полной яркости.
            // [JN] Also care about infra green light visor.
            // TODO: fixme, full bright is not green
            // else if (infragreen_visor && !vanillaparm)
            // player->fixedcolormap = player->powers[pw_infrared] ? 33 : 0;
            player->fixedcolormap = player->powers[pw_infrared] ? 1 : 0;
        }
    }
    else if (player->powers[pw_infrared])	
    {
        if (player->powers[pw_infrared] > 4 * 32 || (player->powers[pw_infrared] & 8))
        {
            if (infragreen_visor && !vanillaparm)
            {
                // [JN] Infra green visor
                player->fixedcolormap = INFRAGREENCOLORMAP;
            }
            else
            {
                // almost full bright
                player->fixedcolormap = 1;
            }
        }
        else
        {
            player->fixedcolormap = 0;
        }
    }
    else
    {
        player->fixedcolormap = 0;
    }

    // [JN] Press Beta colormaps maze
    if (gamemode == pressbeta)
    {
        if (player->powers[pw_infrared] > 4 * 32 || player->powers[pw_infrared] & 8)
        {
            // [JN] Light Amplification Visor using green colormap (33)
            // In Beta INVERSECOLORMAP have a higher priority than INFRAGREENCOLORMAP. Account this.
            player->fixedcolormap = player->powers[pw_invisibility] ? INVERSECOLORMAP : INFRAGREENCOLORMAP;
        }
        else if (player->powers[pw_invisibility] > 4*32 || player->powers[pw_invisibility] & 8)
        {
            // [JN] Invisibility using inverse colormap (32)
            player->fixedcolormap = 3;
        }
        else
        {
            // [JN] Diminished lightning
            player->fixedcolormap = 0;
        }
    }
}

