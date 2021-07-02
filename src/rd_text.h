//
// Copyright(C) 2021 Dasperal
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

#ifndef RD_TEXT_H
#define RD_TEXT_H

#include "v_trans.h"

// [JN] Colored message type, used by P_SetMessage.
typedef enum
{
    msg_uncolored,  // 0: never colored
    msg_pickup,     // 1: item pickup
    msg_secret,     // 2: revealed secret
    msg_system,     // 3: system message
    msg_chat        // 4: netgame chat
} MessageType_t;

extern Translation_CR_t messages_pickup_color_set;
extern Translation_CR_t messages_secret_color_set;
extern Translation_CR_t messages_system_color_set;
extern Translation_CR_t messages_chat_color_set;

extern void M_RD_Define_Msg_Color(MessageType_t messageType, int color);

#endif //RD_TEXT_H
