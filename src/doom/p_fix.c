//
// Copyright(C) 2013-2018 Brad Harding
// Copyright(C) 2018 Julian Nechaevsky
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
//	Verious level-specific fixes.
//
//  Note: The idea and the code has beed taken from Doom Retro.
//  Many thanks to Brad Harding for approval of using his work! 
//


#include "p_fix.h"
#include "r_defs.h"


linefix_t linefix[] =
{
   // mission,    episode, map, linedef, side, toptexture, middletexture, bottomtexture,  offset, rowoffset, flags,                     special,                                    tag

    { doom,             1,   1,      42,    0, "",         "TEKWALL5",    "",            DEFAULT,   DEFAULT, ML_SECRET,                 DEFAULT,                                    DEFAULT },

    { -1,               0,   0,       0,    0, "",         "",            "",            DEFAULT,   DEFAULT, DEFAULT,                   DEFAULT,                                    DEFAULT }
};

sectorfix_t sectorfix[] =
{
   // mission,    episode, map, sector, floorpic,  ceilingpic, floorheight, ceilingheight, special,                            oldtag,  newtag

    { doom,             1,   1,     14, "FLAT18",  "",             DEFAULT,       DEFAULT, DEFAULT,                           DEFAULT, DEFAULT },

    { -1,               0,   0,      0, "",        "",             DEFAULT,       DEFAULT, DEFAULT,                           DEFAULT, DEFAULT },
};

