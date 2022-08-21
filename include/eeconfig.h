/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#ifndef HAVE_EECONFIG_H
#define HAVE_EECONFIG_H
#include "common.h"

#include <stdbool.h>

/**
 * @brief Initialize the eeconfig. Open the corresponding config file
 * 
 * @return 0 The eeconfig file is opened successfully
 * @return 1 The eeconfig file is not opened successfully, all other eeconfig_ calls should not be used
 */
int eeconfig_initialize();

/**
 * @brief Close the eeconfig. Only use after eeconfig_initialize() returns true
 * 
 */
void eeconfig_close();

/**
 * @brief Get a config value string
 * 
 * @param key The key string, without =, e.g. global.externaldrive
 * @return char* The value string, quotes in pair will be stripped. NULL if nothing read (failed). The result should be freed by the caller
 */
char* eeconfig_get_string(const char *key);

/**
 * @brief Get a config value integer
 * 
 * @param key The key string, without =, e.g. ee_mount.delay
 * @return int The value, capped as integer, 0 if failed to read or illegal
 */
int eeconfig_get_int(const char *key);

/**
 * @brief Get a config value boolean
 * 
 * @param key The key string, without =, e.g. global.network
 * @param bool_default The default boolean to return if failed to read or illegal value found
 * @return true The result is either one of the following strings (case-insensitive): 
 *                  true, yes, t, y, [all positive numbers].
 * @return false The result is either one of the following strings (case-insensitive):
 *                  false, no, f, n, 0, [all negative numbers]
 */
bool eeconfig_get_bool(const char *key, const bool bool_default);
#endif