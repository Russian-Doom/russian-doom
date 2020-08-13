//
// Copyright(C) 2020 Dasperal
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

#define CMD_NOP 0 // 0 args
#define CMD_LSPEC3DIRECT 11 // 4 args
#define CMD_EQ 19 //0 args
#define CMD_DELAYDIRECT 56 //1 args
#define LAST_EXTERNAL_CMD 101
#define CMD_TABLE_DELAY_DIRECT (LAST_EXTERNAL_CMD + 1) // 1 arg
#define CMD_PRINT_BOLD_ALWAYS_WITH_TABLE_DELAY_DIRECT (LAST_EXTERNAL_CMD + 2) // 2 args
#define CMD_PRINT_BOLD_RUSSIAN_DIRECT (LAST_EXTERNAL_CMD + 3) // 1 arg
#define CMD_PRINT_NUMBER_OR_PRINT_STRING_DIRECT (LAST_EXTERNAL_CMD + 4) // 1 arg
#define CMD_PRINT_STRING_DIRECT_OR_PRINT_NUMBER (LAST_EXTERNAL_CMD + 5) // 1 arg
#define CMD_PRINT_ALWAYS_WITH_TABLE_DELAY_DIRECT (LAST_EXTERNAL_CMD + 6) // 2 args
#define CMD_PRINT_RUSSIAN_DIRECT (LAST_EXTERNAL_CMD + 7) // 1 arg
#define CMD_PRINT_SCRIPTVAR_AND_STRING_ENGLISH_DIRECT (LAST_EXTERNAL_CMD + 8) // 2 arg
#define CMD_PRINT_MAPVAR_AND_STRING_ENGLISH_DIRECT (LAST_EXTERNAL_CMD + 9) // 2 arg
#define CMD_GT2EQ (LAST_EXTERNAL_CMD + 10) // 0 arg

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
