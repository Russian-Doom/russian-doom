//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2013-2017 Brad Harding
// Copyright(C) 2017 Fabian Greffrath
// Copyright(C) 2017-2022 Julian Nechaevsky
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
//	Brightmap textures and flats lookup routine.
//


#include "h2def.h"
#include "r_bmaps.h"
#include "jn.h"




/*
================================================================================
=
= [crispy] brightmap data
=
================================================================================
*/

static const byte nobrightmap[256] = {0};

static const byte fullbright[256] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const byte surfaces1[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
    0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte surfaces2[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte artifacts[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte flame1[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte flame2[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
    1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte firebull[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte mana[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte greenonly[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte blueonly[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

const byte *dc_brightmap = nobrightmap;

/*
================================================================================
=
= [crispy] brightmaps for textures
=
================================================================================
*/

typedef struct
{
    const char *const texture;
    const byte *colormask;
} fullbright_t;

static const fullbright_t fullbright_walls[] = {
    // [JN] common textures
    {"SPAWN03", surfaces1},
    {"SPAWN09", surfaces1},
    {"SPAWN10", surfaces1},
    {"SW_1_DN", surfaces1},
    {"SW_1_MD", surfaces1},
    {"SW_2_DN", surfaces1},
    {"SW_2_MD", surfaces1},
    {"SPAWN12", surfaces2},
    {"SW51_ON", surfaces2},
    {"SW52_ON", surfaces2},
    {"X_FAC01", fullbright},
    {"X_FAC02", fullbright},
    {"X_FAC03", fullbright},
    {"X_FAC04", fullbright},
    {"X_FAC05", fullbright},
    {"X_FAC06", fullbright},
    {"X_FAC07", fullbright},
    {"X_FAC08", fullbright},
    {"X_FAC09", fullbright},
    {"X_FAC10", fullbright},
    {"X_FAC11", fullbright},
    {"X_FAC12", fullbright},

};

const byte *R_BrightmapForTexName (const char *texname)
{
    int i;

    for (i = 0; i < arrlen(fullbright_walls); i++)
    {
        const fullbright_t *fullbright = &fullbright_walls[i];

        if (!strncasecmp(fullbright->texture, texname, 8))
        {
            return fullbright->colormask;
        }
    }

    return nobrightmap;
}

/*
================================================================================
=
= [crispy] brightmaps for sprites
=
================================================================================
*/

const byte *R_BrightmapForSprite (const int state)
{
    if (brightmaps)
    {
        switch (state)
        {
            // Banishment Device
            case S_ARTI_TELOTHER1:
            case S_ARTI_TELOTHER2:
            case S_ARTI_TELOTHER3:
            case S_ARTI_TELOTHER4:
            // Boots of Speed
            case S_ARTI_BOOTS1:
            case S_ARTI_BOOTS2:
            case S_ARTI_BOOTS3:
            case S_ARTI_BOOTS4:
            case S_ARTI_BOOTS5:
            case S_ARTI_BOOTS6:
            case S_ARTI_BOOTS7:
            case S_ARTI_BOOTS8:
            // Chaos Device
            case S_ARTI_ATLP1:
            case S_ARTI_ATLP2:
            case S_ARTI_ATLP3:
            case S_ARTI_ATLP4:
            // Dragonskin Bracers
            case S_ARTI_ARMOR1:
            case S_ARTI_ARMOR2:
            case S_ARTI_ARMOR3:
            case S_ARTI_ARMOR4:
            case S_ARTI_ARMOR5:
            case S_ARTI_ARMOR6:
            case S_ARTI_ARMOR7:
            case S_ARTI_ARMOR8:
            // Icon of the Defender
            case S_ARTI_INVU1:
            case S_ARTI_INVU2:
            case S_ARTI_INVU3:
            case S_ARTI_INVU4:
            // Krater of Might
            case S_ARTI_MANA:
            // Mystic Ambit Incant
            case S_ARTI_HEALRAD1:
            case S_ARTI_HEALRAD2:
            case S_ARTI_HEALRAD3:
            case S_ARTI_HEALRAD4:
            case S_ARTI_HEALRAD5:
            case S_ARTI_HEALRAD6:
            case S_ARTI_HEALRAD7:
            case S_ARTI_HEALRAD8:
            case S_ARTI_HEALRAD9:
            case S_ARTI_HEALRAD0:
            case S_ARTI_HEALRADA:
            case S_ARTI_HEALRADB:
            case S_ARTI_HEALRADC:
            case S_ARTI_HEALRADD:
            case S_ARTI_HEALRADE:
            case S_ARTI_HEALRADF:
            // Torch
            case S_ARTI_TRCH1:
            case S_ARTI_TRCH2:
            case S_ARTI_TRCH3:
            // Flame Mask
            case S_ARTIPUZZSKULL2:
            // Blue Candle
            case S_ZBLUE_CANDLE1:
            case S_ZBLUE_CANDLE2:
            case S_ZBLUE_CANDLE3:
            case S_ZBLUE_CANDLE4:
            case S_ZBLUE_CANDLE5:
            {
                return artifacts;
                break;
            }

            // Blue Mana
            case S_MANA1_1:
            case S_MANA1_2:
            case S_MANA1_3:
            case S_MANA1_4:
            case S_MANA1_5:
            case S_MANA1_6:
            case S_MANA1_7:
            case S_MANA1_8:
            case S_MANA1_9:
            // Green Mana
            case S_MANA2_1:
            case S_MANA2_2:
            case S_MANA2_3:
            case S_MANA2_4:
            case S_MANA2_5:
            case S_MANA2_6:
            case S_MANA2_7:
            case S_MANA2_8:
            case S_MANA2_9:
            case S_MANA2_10:
            case S_MANA2_11:
            case S_MANA2_12:
            case S_MANA2_13:
            case S_MANA2_14:
            case S_MANA2_15:
            case S_MANA2_16:
            {
                return mana;
                break;
            }

            // Brazier
            case S_ZBRASSTORCH1:
            case S_ZBRASSTORCH2:
            case S_ZBRASSTORCH3:
            case S_ZBRASSTORCH4:
            case S_ZBRASSTORCH5:
            case S_ZBRASSTORCH6:
            case S_ZBRASSTORCH7:
            case S_ZBRASSTORCH8:
            case S_ZBRASSTORCH9:
            case S_ZBRASSTORCH10:
            case S_ZBRASSTORCH11:
            case S_ZBRASSTORCH12:
            case S_ZBRASSTORCH13:
            // Cauldron
            case S_ZCAULDRON1:
            case S_ZCAULDRON2:
            case S_ZCAULDRON3:
            case S_ZCAULDRON4:
            case S_ZCAULDRON5:
            case S_ZCAULDRON6:
            case S_ZCAULDRON7:
            {
                return flame1;
                break;
            }

            // Afrit
            case S_FIRED_LOOK1:
            case S_FIRED_LOOK2:
            case S_FIRED_LOOK3:
            case S_FIRED_LOOK4:
            case S_FIRED_LOOK5:
            case S_FIRED_LOOK6:
            case S_FIRED_LOOK7:
            case S_FIRED_LOOK8:
            case S_FIRED_LOOK9:
            case S_FIRED_LOOK0:
            case S_FIRED_LOOKA:
            case S_FIRED_LOOKB:
            case S_FIRED_WALK1:
            case S_FIRED_WALK2:
            case S_FIRED_WALK3:
            case S_FIRED_ATTACK1:
            case S_FIRED_ATTACK2:
            case S_FIRED_ATTACK3:
            case S_FIRED_ATTACK4:
            case S_FIRED_DEATH1:
            case S_FIRED_DEATH2:
            case S_FIRED_DEATH3:
            case S_FIRED_DEATH4:
            case S_FIRED_XDEATH1:
            case S_FIRED_XDEATH2:
            case S_FIRED_XDEATH3:
            // Chandeiler
            case S_ZCHANDELIER1:
            case S_ZCHANDELIER2:
            case S_ZCHANDELIER3:
            // Wall Torch
            case S_ZWALLTORCH1:
            case S_ZWALLTORCH2:
            case S_ZWALLTORCH3:
            case S_ZWALLTORCH4:
            case S_ZWALLTORCH5:
            case S_ZWALLTORCH6:
            case S_ZWALLTORCH7:
            case S_ZWALLTORCH8:
            // Twined TOrch
            case S_ZTWINEDTORCH_1:
            case S_ZTWINEDTORCH_2:
            case S_ZTWINEDTORCH_3:
            case S_ZTWINEDTORCH_4:
            case S_ZTWINEDTORCH_5:
            case S_ZTWINEDTORCH_6:
            case S_ZTWINEDTORCH_7:
            case S_ZTWINEDTORCH_8:
            // Skull with Flame
            case S_ZFIRETHING1:
            case S_ZFIRETHING2:
            case S_ZFIRETHING3:
            case S_ZFIRETHING4:
            case S_ZFIRETHING5:
            case S_ZFIRETHING6:
            case S_ZFIRETHING7:
            case S_ZFIRETHING8:
            case S_ZFIRETHING9:
            {
                return flame2;
                break;
            }

            // Fire Bull
            case S_ZFIREBULL1:
            case S_ZFIREBULL2:
            case S_ZFIREBULL3:
            case S_ZFIREBULL4:
            case S_ZFIREBULL5:
            case S_ZFIREBULL6:
            case S_ZFIREBULL7:
            {
                return firebull;
                break;
            }

            // Wendigo
            case S_ICEGUY_LOOK:
            case S_ICEGUY_DORMANT:
            case S_ICEGUY_WALK1:
            case S_ICEGUY_WALK2:
            case S_ICEGUY_WALK3:
            case S_ICEGUY_WALK4:
            case S_ICEGUY_ATK1:
            case S_ICEGUY_ATK2:
            case S_ICEGUY_ATK3:
            case S_ICEGUY_ATK4:
            case S_ICEGUY_PAIN1:
            // Reiver
            case S_WRAITH_ATK1_1:
            case S_WRAITH_ATK1_2:
            case S_WRAITH_ATK1_3:
            case S_WRAITH_ATK2_1:
            case S_WRAITH_ATK2_2:
            case S_WRAITH_ATK2_3:
            // Korax
            case S_KORAX_LOOK1:
            case S_KORAX_CHASE1:
            case S_KORAX_CHASE2:
            case S_KORAX_CHASE3:
            case S_KORAX_CHASE4:
            case S_KORAX_CHASE5:
            case S_KORAX_CHASE6:
            case S_KORAX_CHASE7:
            case S_KORAX_CHASE8:
            case S_KORAX_CHASE9:
            case S_KORAX_CHASE0:
            case S_KORAX_CHASEA:
            case S_KORAX_CHASEB:
            case S_KORAX_CHASEC:
            case S_KORAX_CHASED:
            case S_KORAX_CHASEE:
            case S_KORAX_CHASEF:
            case S_KORAX_PAIN1:
            case S_KORAX_PAIN2:
            {
                return surfaces1;
                break;
            }
        }            
    }
    else
    {
        switch (state)
        {
            // Banishment Device
            case S_ARTI_TELOTHER1:
            case S_ARTI_TELOTHER2:
            case S_ARTI_TELOTHER3:
            case S_ARTI_TELOTHER4:
            // Boots of Speed
            case S_ARTI_BOOTS1:
            case S_ARTI_BOOTS2:
            case S_ARTI_BOOTS3:
            case S_ARTI_BOOTS4:
            case S_ARTI_BOOTS5:
            case S_ARTI_BOOTS6:
            case S_ARTI_BOOTS7:
            case S_ARTI_BOOTS8:
            // Chaos Device
            case S_ARTI_ATLP1:
            case S_ARTI_ATLP2:
            case S_ARTI_ATLP3:
            case S_ARTI_ATLP4:
            // Dragonskin Bracers
            case S_ARTI_ARMOR1:
            case S_ARTI_ARMOR2:
            case S_ARTI_ARMOR3:
            case S_ARTI_ARMOR4:
            case S_ARTI_ARMOR5:
            case S_ARTI_ARMOR6:
            case S_ARTI_ARMOR7:
            case S_ARTI_ARMOR8:
            // Krater of Might
            case S_ARTI_MANA:
            // Mystic Ambit Incant
            case S_ARTI_HEALRAD1:
            case S_ARTI_HEALRAD2:
            case S_ARTI_HEALRAD3:
            case S_ARTI_HEALRAD4:
            case S_ARTI_HEALRAD5:
            case S_ARTI_HEALRAD6:
            case S_ARTI_HEALRAD7:
            case S_ARTI_HEALRAD8:
            case S_ARTI_HEALRAD9:
            case S_ARTI_HEALRAD0:
            case S_ARTI_HEALRADA:
            case S_ARTI_HEALRADB:
            case S_ARTI_HEALRADC:
            case S_ARTI_HEALRADD:
            case S_ARTI_HEALRADE:
            case S_ARTI_HEALRADF:
            // Torch
            case S_ARTI_TRCH1:
            case S_ARTI_TRCH2:
            case S_ARTI_TRCH3:
            
            // Blue Candle
            case S_ZBLUE_CANDLE1:
            case S_ZBLUE_CANDLE2:
            case S_ZBLUE_CANDLE3:
            case S_ZBLUE_CANDLE4:
            case S_ZBLUE_CANDLE5:
            // Brazier
            case S_ZBRASSTORCH1:
            case S_ZBRASSTORCH2:
            case S_ZBRASSTORCH3:
            case S_ZBRASSTORCH4:
            case S_ZBRASSTORCH5:
            case S_ZBRASSTORCH6:
            case S_ZBRASSTORCH7:
            case S_ZBRASSTORCH8:
            case S_ZBRASSTORCH9:
            case S_ZBRASSTORCH10:
            case S_ZBRASSTORCH11:
            case S_ZBRASSTORCH12:
            case S_ZBRASSTORCH13:
            // Cauldron
            case S_ZCAULDRON1:
            case S_ZCAULDRON2:
            case S_ZCAULDRON3:
            case S_ZCAULDRON4:
            case S_ZCAULDRON5:
            case S_ZCAULDRON6:
            case S_ZCAULDRON7:
            // Chandeiler
            case S_ZCHANDELIER1:
            case S_ZCHANDELIER2:
            case S_ZCHANDELIER3:
            // Wall Torch
            case S_ZWALLTORCH1:
            case S_ZWALLTORCH2:
            case S_ZWALLTORCH3:
            case S_ZWALLTORCH4:
            case S_ZWALLTORCH5:
            case S_ZWALLTORCH6:
            case S_ZWALLTORCH7:
            case S_ZWALLTORCH8:
            // Twined TOrch
            case S_ZTWINEDTORCH_1:
            case S_ZTWINEDTORCH_2:
            case S_ZTWINEDTORCH_3:
            case S_ZTWINEDTORCH_4:
            case S_ZTWINEDTORCH_5:
            case S_ZTWINEDTORCH_6:
            case S_ZTWINEDTORCH_7:
            case S_ZTWINEDTORCH_8:
            // Skull with Flame
            case S_ZFIRETHING1:
            case S_ZFIRETHING2:
            case S_ZFIRETHING3:
            case S_ZFIRETHING4:
            case S_ZFIRETHING5:
            case S_ZFIRETHING6:
            case S_ZFIRETHING7:
            case S_ZFIRETHING8:
            case S_ZFIRETHING9:
            // Fire Bull
            case S_ZFIREBULL1:
            case S_ZFIREBULL2:
            case S_ZFIREBULL3:
            case S_ZFIREBULL4:
            case S_ZFIREBULL5:
            case S_ZFIREBULL6:
            case S_ZFIREBULL7:

            // Blue Mana
            case S_MANA1_1:
            case S_MANA1_2:
            case S_MANA1_3:
            case S_MANA1_4:
            case S_MANA1_5:
            case S_MANA1_6:
            case S_MANA1_7:
            case S_MANA1_8:
            case S_MANA1_9:
            // Green Mana
            case S_MANA2_1:
            case S_MANA2_2:
            case S_MANA2_3:
            case S_MANA2_4:
            case S_MANA2_5:
            case S_MANA2_6:
            case S_MANA2_7:
            case S_MANA2_8:
            case S_MANA2_9:
            case S_MANA2_10:
            case S_MANA2_11:
            case S_MANA2_12:
            case S_MANA2_13:
            case S_MANA2_14:
            case S_MANA2_15:
            case S_MANA2_16:
            {
                return fullbright;
                break;
            }
        }
    }

    return nobrightmap;
}

/*
================================================================================
=
= [crispy] brightmaps for states
=
================================================================================
*/

const byte *R_BrightmapForState (const int state)
{
    if (brightmaps)
    {
        switch (state)
        {
            // Fighter: Axe
            case S_FAXEDOWN_G:
            case S_FAXEUP_G:
            case S_FAXEREADY_G:
            case S_FAXEREADY_G1:
            case S_FAXEREADY_G2:
            case S_FAXEREADY_G3:
            case S_FAXEREADY_G4:
            case S_FAXEREADY_G5:
            case S_FAXEATK_G1:
            case S_FAXEATK_G2:
            case S_FAXEATK_G3:
            case S_FAXEATK_G4:
            case S_FAXEATK_G5:
            case S_FAXEATK_G6:
            case S_FAXEATK_G7:
            case S_FAXEATK_G8:
            case S_FAXEATK_G9:
            case S_FAXEATK_G10:
            case S_FAXEATK_G11:
            case S_FAXEATK_G12:
            case S_FAXEATK_G13:
            // Fighter: Sword
            case S_FSWORDDOWN:
            case S_FSWORDUP:
            case S_FSWORDREADY:
            case S_FSWORDREADY1:
            case S_FSWORDREADY2:
            case S_FSWORDREADY3:
            case S_FSWORDREADY4:
            case S_FSWORDREADY5:
            case S_FSWORDREADY6:
            case S_FSWORDREADY7:
            case S_FSWORDREADY8:
            case S_FSWORDREADY9:
            case S_FSWORDREADY10:
            case S_FSWORDREADY11:
            case S_FSWORDATK_1:
            case S_FSWORDATK_2:
            case S_FSWORDATK_3:
            case S_FSWORDATK_4:
            case S_FSWORDATK_5:
            case S_FSWORDATK_6:
            case S_FSWORDATK_7:
            case S_FSWORDATK_8:
            case S_FSWORDATK_9:
            case S_FSWORDATK_10:
            case S_FSWORDATK_11:
            case S_FSWORDATK_12:
            // Cleric: Flame
            case S_CFLAMEDOWN:
            case S_CFLAMEUP:
            case S_CFLAMEREADY1:
            case S_CFLAMEREADY2:
            case S_CFLAMEREADY3:
            case S_CFLAMEREADY4:
            case S_CFLAMEREADY5:
            case S_CFLAMEREADY6:
            case S_CFLAMEREADY7:
            case S_CFLAMEREADY8:
            case S_CFLAMEREADY9:
            case S_CFLAMEREADY10:
            case S_CFLAMEREADY11:
            case S_CFLAMEREADY12:
            case S_CFLAMEATK_1:
            case S_CFLAMEATK_2:
            case S_CFLAMEATK_3:
            case S_CFLAMEATK_7:
            case S_CFLAMEATK_8:
            // Mage: Arc
            case S_MSTAFFREADY:
            case S_MSTAFFREADY2:
            case S_MSTAFFREADY3:
            case S_MSTAFFREADY4:
            case S_MSTAFFREADY5:
            case S_MSTAFFREADY6:
            case S_MSTAFFREADY7:
            case S_MSTAFFREADY8:
            case S_MSTAFFREADY9:
            case S_MSTAFFREADY10:
            case S_MSTAFFREADY11:
            case S_MSTAFFREADY12:
            case S_MSTAFFREADY13:
            case S_MSTAFFREADY14:
            case S_MSTAFFREADY15:
            case S_MSTAFFREADY16:
            case S_MSTAFFREADY17:
            case S_MSTAFFREADY18:
            case S_MSTAFFREADY19:
            case S_MSTAFFREADY20:
            case S_MSTAFFREADY21:
            case S_MSTAFFREADY22:
            case S_MSTAFFREADY23:
            case S_MSTAFFREADY24:
            case S_MSTAFFREADY25:
            case S_MSTAFFREADY26:
            case S_MSTAFFREADY27:
            case S_MSTAFFREADY28:
            case S_MSTAFFREADY29:
            case S_MSTAFFREADY30:
            case S_MSTAFFREADY31:
            case S_MSTAFFREADY32:
            case S_MSTAFFREADY33:
            case S_MSTAFFREADY34:
            case S_MSTAFFREADY35:
            case S_MSTAFFDOWN:
            case S_MSTAFFUP:
            case S_MSTAFFATK_1:
            case S_MSTAFFATK_4:
            case S_MSTAFFATK_5:
            case S_MSTAFFATK_6:
            case S_MSTAFFATK_7:
            {
                return surfaces1;
                break;
            }

            // Cleric: Serpent Staff
            case S_CSTAFFATK_1:
            case S_CSTAFFATK_2:
            case S_CSTAFFATK_3:
            case S_CSTAFFATK_4:
            case S_CSTAFFATK2_1:
            {
                return greenonly;
                break;
            }

            // Mage: Frost
            case S_CONEATK1_2: 
            case S_CONEATK1_3:
            case S_CONEATK1_4:
            case S_CONEATK1_5:
            case S_CONEATK1_6:
            case S_CONEATK1_7:
            case S_CONEATK1_8:
            // Mage: Lightning
            case S_MLIGHTNINGREADY:
            case S_MLIGHTNINGREADY2:
            case S_MLIGHTNINGREADY3:
            case S_MLIGHTNINGREADY4:
            case S_MLIGHTNINGREADY5:
            case S_MLIGHTNINGREADY6:
            case S_MLIGHTNINGREADY7:
            case S_MLIGHTNINGREADY8:
            case S_MLIGHTNINGREADY9:
            case S_MLIGHTNINGREADY10:
            case S_MLIGHTNINGREADY11:
            case S_MLIGHTNINGREADY12:
            case S_MLIGHTNINGREADY13:
            case S_MLIGHTNINGREADY14:
            case S_MLIGHTNINGREADY15:
            case S_MLIGHTNINGREADY16:
            case S_MLIGHTNINGREADY17:
            case S_MLIGHTNINGREADY18:
            case S_MLIGHTNINGREADY19:
            case S_MLIGHTNINGREADY20:
            case S_MLIGHTNINGREADY21:
            case S_MLIGHTNINGREADY22:
            case S_MLIGHTNINGREADY23:
            case S_MLIGHTNINGREADY24:
            case S_MLIGHTNINGDOWN:
            case S_MLIGHTNINGUP:
            case S_MLIGHTNINGATK_1:
            case S_MLIGHTNINGATK_2:
            case S_MLIGHTNINGATK_3:
            case S_MLIGHTNINGATK_4:
            case S_MLIGHTNINGATK_5:
            case S_MLIGHTNINGATK_6:
            case S_MLIGHTNINGATK_7:
            case S_MLIGHTNINGATK_8:
            case S_MLIGHTNINGATK_9:
            case S_MLIGHTNINGATK_10:
            case S_MLIGHTNINGATK_11:
            {
                return blueonly;
                break;
            }

            default:
            {
                return nobrightmap;
                break;
            }
        }
	}

    return nobrightmap;
}

/*
================================================================================
= R_InitBrightmaps
=
= [JN] Predefince Korax textures to apply on twosided midtextures.
=
================================================================================
*/

int bmaptexture01, bmaptexture02, bmaptexture03, bmaptexture04,
    bmaptexture05, bmaptexture06, bmaptexture07, bmaptexture08,
    bmaptexture09, bmaptexture10, bmaptexture11, bmaptexture12;

void R_InitBrightmaps (void)
{
    bmaptexture01 = R_CheckTextureNumForName("X_FAC01");
    bmaptexture02 = R_CheckTextureNumForName("X_FAC02");
    bmaptexture03 = R_CheckTextureNumForName("X_FAC03");
    bmaptexture04 = R_CheckTextureNumForName("X_FAC04");
    bmaptexture05 = R_CheckTextureNumForName("X_FAC05");
    bmaptexture06 = R_CheckTextureNumForName("X_FAC06");
    bmaptexture07 = R_CheckTextureNumForName("X_FAC07");
    bmaptexture08 = R_CheckTextureNumForName("X_FAC08");
    bmaptexture09 = R_CheckTextureNumForName("X_FAC09");
    bmaptexture10 = R_CheckTextureNumForName("X_FAC10");
    bmaptexture11 = R_CheckTextureNumForName("X_FAC11");
    bmaptexture12 = R_CheckTextureNumForName("X_FAC12");
}
