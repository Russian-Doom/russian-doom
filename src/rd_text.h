//
// Copyright(C) 2021-2022 Dasperal
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
    msg_chat,       // 4: netgame chat
    hud_stats,      // 5: HUD stats (KIS, time)
    hud_coords,     // 6: HUD coords (x/y/z/ang)
    hud_level,      // 7: HUD level name
    hud_values      // 8: HUD values
    
} MessageType_t;

extern Translation_CR_t messages_pickup_color_set;
extern Translation_CR_t messages_secret_color_set;
extern Translation_CR_t messages_system_color_set;
extern Translation_CR_t messages_chat_color_set;

extern void M_RD_Define_Msg_Color(MessageType_t messageType, int color);

void RD_M_InitFonts(char* FontA, char* FontB, char* FontC,
                    char* SmallEngFont, char* BigEngFont,
                    char* SmallRusFont, char* BigRusFont);

/** Draw text using replaceable English font A*/
void RD_M_DrawTextA(char *text, int x, int y);
/** Draw text using replaceable English font A with fading effect*/
void RD_M_DrawTextAFade(char *text, int x, int y, byte *table);
/** Returns the pixel width of a string using replaceable English font A*/
int RD_M_TextAWidth(char *text);

/**Draw text using replaceable English font B*/
void RD_M_DrawTextB(char *text, int x, int y);
/** Returns the pixel width of a string using replaceable English font B*/
int RD_M_TextBWidth(char *text);

/** [JN] Draw small time digits using font C*/
void RD_M_DrawTextC(char *text, int x, int y);

/** [JN] Draw text string with unreplaceable small English font*/
void RD_M_DrawTextSmallENG(char *text, int x, int y, Translation_CR_t translation);
/** [JN] Returns the pixel width of a string using unreplaceable small English font*/
int RD_M_TextSmallENGWidth(char *text);

/** [JN] Draw text string with unreplaceable big English font*/
void RD_M_DrawTextBigENG(char *text, int x, int y);
/** [JN] Returns the pixel width of a string using unreplaceable big English font */
int RD_M_TextBigENGWidth(char *text);

/** [JN] Draw text string with unreplaceable small Russian font*/
void RD_M_DrawTextSmallRUS(char* text, int x, int y, Translation_CR_t translation);
/** [JN] Draw text using unreplaceable small Russian font with fading effect*/
void RD_M_DrawTextSmallRUSFade(char* text, int x, int y, byte *table);
/** [JN] Returns the pixel width of a string using unreplaceable small Russian font*/
int RD_M_TextSmallRUSWidth(char* text);

/** [JN] Draw text string with unreplaceable big Russian font*/
void RD_M_DrawTextBigRUS(char* text, int x, int y);
/** [JN] Returns the pixel width of a string using unreplaceable big Russian font*/
int RD_M_TextBigRUSWidth(char* text);

#endif //RD_TEXT_H
