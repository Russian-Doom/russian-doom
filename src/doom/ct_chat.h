//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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


#pragma once

void CT_Init (void);
void CT_Ticker (void);
void CT_Drawer (void);
char CT_dequeueChatChar (void);

boolean CT_Responder(event_t * ev);

extern patch_t *EngFontBaseLump;
extern patch_t *RusFontBaseLump;

extern boolean chatmodeon;
extern char *CT_FromPlrText[MAXPLAYERS];
extern char *chat_macros[10];
