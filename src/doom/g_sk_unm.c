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

#define ENUMERATE_UNM_CHANGES(O)                                            \
/* Monster's speed */                                                       \
O(MT_POSSESSED, speed, + 4)              /* Zombieman (8 to 12) */          \
O(MT_SHOTGUY, speed, + 4)                /* Shotgun guy (8 to 12) */        \
O(MT_CHAINGUY, speed, + 4)               /* Chaingunner (8 to 12) */        \
O(MT_TROOP, speed, + 5)                  /* Imp (8 to 13) */                \
O(MT_SKULL, speed, + 6)                  /* Lost Soul (8 to 14) */          \
O(MT_HEAD, speed, + 6)                   /* Cacodemon (8 to 14) */          \
O(MT_KNIGHT, speed, + 6)                 /* Hell Knight (8 to 14) */        \
O(MT_BRUISER, speed, + 6)                /* Baron of Hell (8 to 14) */      \
O(MT_BABY, speed, + 5)                   /* Arachnotron (12 to 17) */       \
O(MT_PAIN, speed, + 6)                   /* Pain Elemental (8 to 14) */     \
O(MT_UNDEAD, speed, + 4)                 /* Revenant (10 to 14) */          \
O(MT_FATSO, speed, + 3)                  /* Mancubus (8 to 12) */           \
O(MT_VILE, speed, + 3)                   /* Archvile (15 to 18) */          \
O(MT_SPIDER, speed, + 5)                 /* Spider Mastermind (12 to 17) */ \
O(MT_CYBORG, speed, + 4)                 /* Cyberdemon (16 to 20) */        \
O(MT_WOLFSS, speed, + 5)                 /* Wolf SS (8 to 13) */            \
/* Monster's missiles speed */                                              \
O(MT_ARACHPLAZ, speed, + (5 * FRACUNIT)) /* Arachnotron (25 to 30) */       \
O(MT_FATSHOT, speed, + (5 * FRACUNIT))   /* Mancubus (20 to 25) */          \
/* Monster's damage */                                                      \
O(MT_TROOPSHOT, damage, + 1)             /* Imp (3 to 4) */                 \
O(MT_HEADSHOT, damage, + 2)              /* Cacodemon (5 to 7) */           \
O(MT_BRUISERSHOT, damage, + 2)           /* Knight / Baron (8 to 10) */     \
O(MT_ARACHPLAZ, damage, + 2)             /* Arachnotron (5 to 7) */         \
O(MT_TRACER, damage, + 2)                /* Revenant (10 to 12) */          \
O(MT_FATSHOT, damage, + 4)               /* Mancubus (8 to 12) */

UNM_IMPLEMENT(ENUMERATE_UNM_CHANGES)
