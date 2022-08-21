/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#ifndef HAVE_SORT_H
#define HAVE_SORT_H
#include "common.h"

/**
 * @brief Compare two strings
 * 
 * @param a One of the string to compare
 * @param b The other string to sort
 * @return int >0 if a is greater, <0 if b is greater, 0 if equal
 */
int sort_compare_string(const void *a, const void *b);

/**
 * @brief Compare two drive struct
 * 
 * @param a One of the drive struct to compare 
 * @param b The other drive struct to compare
 * @return int >0 if a's name is greater, <0 if b's name is greater, 0 if equal
 */
int sort_compare_drive(const void *a, const void *b);

/**
 * @brief Comprare two systemd_mount_unit struct
 * 
 * @param a One of the systemd_mount_unit struct to compare
 * @param b The other systemd_mount_unit struct to compare
 * @return int >0 if a's system name is greater, <0 if b's system name is greater, 0 if equal
 */
int sort_compare_systemd_mount_unit(const void *a, const void *b);

#endif