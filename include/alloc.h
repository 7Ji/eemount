/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#ifndef HAVE_ALLOC_H
#define HAVE_ALLOC_H

#include "common.h"

#define ALLOC_BASE_SIZE     16
#define ALLOC_MULTIPLIER   1.5

/**
 * @brief (Re)allocate memory according to given for pointer
 * 
 * @param ptr pointer to the original pointer which could have allocated memory, if it's a pointer, use realloc, else, use malloc
 * @param size the new size of the memory
 * @return void* the new pointer if allocate success, or NULL if failed
 */
void *alloc_optional_resize(void *ptr, size_t size);

/**
 * @brief Free the memory and set pointer to NULL if it's used
 * 
 * @param ptr pointer to free
 */

void alloc_free_if_used(void **ptr);
#endif