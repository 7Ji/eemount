/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "sort_p.h"

int sort_compare_string(const void *a, const void *b){
    return strcmp(*(char**)a, *(char**)b);
}

int sort_compare_drive(const void *a, const void *b) {
    return strcmp(((struct drive *)a)->name, ((struct drive *)b)->name);
}

int sort_compare_systemd_mount_unit(const void *a, const void *b) {
    return strcmp(((struct systemd_mount_unit *)a)->system, ((struct systemd_mount_unit *)b)->system);
}