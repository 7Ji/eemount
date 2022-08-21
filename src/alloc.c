/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "alloc_p.h"

void *alloc_optional_resize(void *ptr, size_t size) {
    void *buffer;
    if (ptr) {
        buffer = realloc(ptr, size);
    } else {
        buffer = malloc(size);
    }
    if (buffer == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate memory");
        if (ptr) {
            free(ptr);
        }
    }
    return buffer;
}

void alloc_free_if_used(void **ptr) {
    if (*ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}