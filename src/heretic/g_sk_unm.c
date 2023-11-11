//
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2023 Leonid Murin (Dasperal)
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

#define ENUMERATE_UNM_CHANGES(O)                                                \
/* Monster's speed */                                                           \
O(MT_IMP, speed, + 7)              /* Gargolye (10 to 17) */                    \
O(MT_IMPLEADER, speed, + 7)        /* Fire Gargolye (10 to 17) */               \
O(MT_MUMMY, speed, + 9)            /* Golem (12 to 21) */                       \
O(MT_MUMMYGHOST, speed, + 9)       /* Golem Ghost (12 to 21) */                 \
O(MT_MUMMYLEADER, speed, + 9)      /* Nitrogolem (12 to 21) */                  \
O(MT_MUMMYLEADERGHOST, speed, + 9) /* Nitrogolem Ghost (12 to 21) */            \
O(MT_CLINK, speed, + 7)            /* Sabreclaw (14 to 21) */                   \
O(MT_KNIGHT, speed, + 7)           /* Undead Warrior (12 to 19) */              \
O(MT_KNIGHTGHOST, speed, + 7)      /* Undead Warrior Ghost (12 to 19) */        \
O(MT_BEAST, speed, + 7)            /* Weredragon (14 to 21) */                  \
O(MT_SNAKE, speed, + 7)            /* Ophidian (10 to 17) */                    \
O(MT_WIZARD, speed, + 7)           /* Disciple of D'Sparil (12 to 19) */        \
O(MT_HEAD, speed, + 7)             /* Iron Lich (6 to 13) */                   \
O(MT_MINOTAUR, speed, + 5)         /* Maolotaur (16 to 21) */                   \
O(MT_SORCERER1, speed, + 5)        /* D'Sparil (riding) (16 to 21) */           \
O(MT_SORCERER2, speed, + 6)        /* D'Sparil (walking) (14 to 20) */          \
/* Monster's damage */                                                          \
O(MT_IMPBALL, damage, + 1)          /* Fire Gargolye fireball (1 to 2) */        \
O(MT_MUMMYFX1, damage, + 2)         /* Nitrogolem Ghost fireball (4 to 6) */     \
O(MT_KNIGHTAXE, damage, + 2)        /* Undead Warrior green axe (3 to 5) */      \
O(MT_REDAXE, damage, + 1)           /* Undead Warrior red axe (7 to 8) */        \
O(MT_BEASTBALL, damage, + 1)        /* Weredragon fireball (4 to 5) */           \
O(MT_SNAKEPRO_A, damage, + 1)       /* Ophidian fireball A (1 to 2) */           \
O(MT_SNAKEPRO_B, damage, + 1)       /* Ophidian fireball B (3 to 4) */           \
O(MT_WIZFX1, damage, + 1)           /* Disciple of D'Sparil fireball (3 to 4) */

UNM_IMPLEMENT(ENUMERATE_UNM_CHANGES)
