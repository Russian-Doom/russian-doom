//
// Copyright(C) 2022 Dasperal
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

/**
 * [Dasperal] This is only growing number that should be incremented when config structure changes
 * in a way that require use of migration mechanism
 */
#define CURRENT_CONFIG_VERSION 1

/**
 * Version of the read config
 */
extern int config_version;

void RD_ApplyMigration();
