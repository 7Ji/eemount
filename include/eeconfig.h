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
 * @brief Helper enum for getting eeconfig
 * 
 */
enum eeconfig_get_type {
    /**
     * @brief Get a string, the returned string, if not NULL, should be cleaned by caller
     * 
     */
    EECONFIG_GET_STRING,
    /**
     * @brief Get a long
     * 
     */
    EECONFIG_GET_LONG,
    /**
     * @brief Get an integer
     * 
     */
    EECONFIG_GET_INT,
    /**
     * @brief Get a boolean
     * 
     */
    EECONFIG_GET_BOOL
};

/**
 * @brief Helper struct to get multiple eeconfig settings
 * 
 */
struct eeconfig_get_helper {
    /**
     * @brief The setting itself
     * 
     */
    const char *setting;
    /**
     * @brief The platform
     * 
     */
    const char *platform;
    /**
     * @brief The ROM
     * 
     */
    const char *rom;
    /**
     * @brief The type of the setting
     * 
     */
    const enum eeconfig_get_type type;
    /**
     * @brief Pointer to the returned setting
     * 
     */
    const void *result;
};

/**
 * @brief Get only one setting
 * 
 * @param setting The setting itself
 * @param platform The platform
 * @param rom The ROM
 * @param type The type of the setting
 * @param result Pointer to the returned setting
 * @return int 0 for success, positive for failure
 */
int eeconfig_get_setting_one(const char *setting, const char *platform, const char *rom, const enum eeconfig_get_type type, const void *result);

/**
 * @brief Get multiple settings
 * 
 * @param getter The helper struct array for each setting
 * @param get_count The count of settings to get
 * @return int 0 for success, positive for failure in getting settings, negative for failure in initialization
 */
int eeconfig_get_setting_many(struct eeconfig_get_helper *getter, unsigned int get_count);
#endif