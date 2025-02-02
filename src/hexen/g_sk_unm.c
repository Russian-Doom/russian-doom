//
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2025 Leonid Murin (Dasperal)
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

#include "l_sk_unm.h"
#include "g_sk_unm.h"

#include "info.h"
#include "m_fixed.h"

#define ENUMERATE_UNM_CHANGES(O)                                                           \
/* Monster's speed */                                                                      \
O(MT_FIREDEMON, speed, + 7)                  /* Affrit (13 to 20) */                       \
/* Monster's missiles speed */                                                             \
O(MT_FIREDEMON_FX6, speed, + (3 * FRACUNIT)) /* Affrit fireball (10 to 13) */              \
O(MT_DEMON2FX1, speed, + (3 * FRACUNIT))     /* Brown Chaos Serpent fireball (15 to 18) */ \
O(MT_BISH_FX, speed, + (3 * FRACUNIT))       /* Dark Bishop fireball (10 to 13) */         \
O(MT_DEMONFX1, speed, + (3 * FRACUNIT))      /* Green Chaos Serpent fireball (15 to 18) */ \
O(MT_WRAITHFX1, speed, + (3 * FRACUNIT))     /* Reiver fireball (14 to 17) */              \
O(MT_CENTAUR_FX, speed, + (3 * FRACUNIT))    /* Slaughtaur fireball (20 to 23) */          \
O(MT_SERPENTFX, speed, + (3 * FRACUNIT))     /* Stalker fireball (15 to 18) */             \
O(MT_ICEGUY_FX, speed, + (3 * FRACUNIT))     /* Wendige fireball 1 (14 to 17) */           \
O(MT_ICEGUY_FX2, speed, + (3 * FRACUNIT))    /* Wendige fireball 2 (10 to 13) */           \
/* Monster's damage */                                                                     \
O(MT_FIREDEMON_FX6, damage, + 1)              /* Fire Gargolye fireball (1 to 2) */        \
O(MT_DEMON2FX1, damage, + 1)                  /* Brown Chaos Serpent fireball (5 to 6) */  \
O(MT_BISH_FX, damage, + 1)                    /* Dark Bishop fireball (1 to 2) */          \
O(MT_DEMONFX1, damage, + 1)                   /* Green Chaos Serpent fireball (5 to 6) */  \
O(MT_WRAITHFX1, damage, + 1)                  /* Reiver fireball (5 to 6) */               \
O(MT_CENTAUR_FX, damage, + 1)                 /* Slaughtaur fireball (4 to 5) */           \
O(MT_SERPENTFX, damage, + 1)                  /* Stalker fireball (4 to 5) */

UNM_IMPLEMENT(ENUMERATE_UNM_CHANGES)
