/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#ifndef HAVE_UTIL_H
#define HAVE_UTIL_H
#include "common.h"
#include <stdbool.h>

/**
 * @brief Convert an unsigned long to unsigned integer safely, limit the range between 0~UINT_MAX
 * 
 * @param value The unsinged long to convert
 * @return unsigned int The converted unsinged integer
 */
unsigned int util_uint_from_ulong(unsigned long value);

/**
 * @brief Convert a long to integer safely, limit the range between INT_MIN~INT_MAX
 * 
 * @param value The long to convert
 * @return int The converted integer
 */
int util_int_from_long(long value);

/**
 * @brief Unescape oct sequence in mountinfo's root & mountpoint in place (\040 -> ' ', \011 -> '\t', \012 -> '\n', \134 -> '\\)
 * 
 * @param escaped The character array (string) to unescape
 */
void util_unesacpe_mountinfo_in_place(char *escaped);

/**
 * @brief Create a directory on demand
 * 
 * @param path The path to the folder
 * @param mode The oct mode, usually should be 0755
 * @return int 0 if not exist and create successfully, or already exists; 1 already exists and not folder, or can't check
 */
int util_mkdir(const char *path, mode_t mode);

/**
 * @brief Create a directory recursively on demand
 * 
 * @param path The path to the folder
 * @param mode The oct mode, usually should be 0755
 * @return int 0 if not exist and create successfully, or already exists; 1 already exists and not folder, or can't check
 */
int util_mkdir_recursive(const char *path, mode_t mode);
#endif