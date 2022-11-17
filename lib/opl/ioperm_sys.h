//
// Copyright(C) 2002, 2003 Marcel Telka
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
//     Interface to the ioperm.sys driver, based on code from the
//     Cygwin ioperm library.
//


#pragma once


int IOperm_EnablePortRange(unsigned int from, unsigned int num, int turn_on);
int IOperm_InstallDriver(void);
int IOperm_UninstallDriver(void);
