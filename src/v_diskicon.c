//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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
//	Disk load indicator.
//


#include "i_video.h"
#include "m_argv.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "v_diskicon.h"

// [JN] Moved from d_main.c for hot swapping.
int show_diskicon = 1;

// Only display the disk icon if more then this much bytes have been read
// during the previous tic.

static const int diskicon_threshold = 20*1024;


static int loading_disk_xoffs = 0;
static int loading_disk_yoffs = 0;

// Number of bytes read since the last call to V_DrawDiskIcon().
static size_t recent_bytes_read = 0;

// [JN] Initially false, will be allowed only in Doom while D_DoomLoop().
boolean disk_allowed = false;

// [JN] Was disk icon drawn (true) or not (false).
boolean disk_drawn;

// [JN] Which icon to use, diskette or cdrom.
char *disk_lump_name;


void V_EnableLoadingDisk(int xoffs, int yoffs)
{
    disk_lump_name = M_CheckParm("-cdrom") > 0 ? "STCDROM" : "STDISK";

    loading_disk_xoffs = xoffs >> hires;
    loading_disk_yoffs = yoffs >> hires;
}

void V_BeginRead(size_t nbytes)
{
    recent_bytes_read += nbytes;
}

void V_DrawDiskIcon(void)
{
    if (recent_bytes_read > diskicon_threshold && disk_drawn == false)
    {
        V_DrawPatch(loading_disk_xoffs, loading_disk_yoffs, 
                    W_CacheLumpName(disk_lump_name, PU_CACHE));
        disk_drawn = true;
    }

    recent_bytes_read = 0;
}

