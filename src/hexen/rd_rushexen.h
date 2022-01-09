//
// Copyright(C) 2020-2022 Dasperal
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



#include "doomtype.h"

#ifndef __RD_RUSHEXEN_H__
#define __RD_RUSHEXEN_H__

typedef struct
{
    uint32_t address;
    union
    {
        struct
        {
            uint32_t cmd;
            uint32_t arg;
        };
        uint64_t value;
    };
} CMDInjectionRecord_t;

extern boolean hasUnknownPWads;
extern boolean isHexenDemo;
extern boolean isDK;

extern const int delayTable[][2];

const char** GetRusStringTable(int map);

const CMDInjectionRecord_t* GetCMDInjectionTable(int map);

boolean cantApplyACSInstrumentation(int map);

#endif //__RD_RUSHEXEN_H__
