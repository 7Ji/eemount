/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#ifndef HAVE_DRIVE_H
#define HAVE_DRIVE_H
#include "common.h"

/**
 * @brief The external drive struct
 * 
 */
struct drive {
    /**
     * @brief The name of the external drive
     * 
     */
    char *name;
    // /**
    //  * @brief The content of the drive mark file
    //  * 
    //  */
    // char *content;
    /**
     * @brief The names array of systems under the drive, NULL if no system is found (the drive should be used as a whole). The array itself is malloced, but the char* in them points to the area in content;
     * 
     */
    char **systems;
    /**
     * @brief The count of systems under the drive. 0 if no system is found and it should be used as a whole
     * 
     */
    unsigned int count;

    /**
     * @brief The allocated count of systems
     * 
     */
    unsigned int alloc_systems;
};

/**
 * @brief The helper struct of external drives
 * 
 */
struct drive_helper {
    /**
     * @brief The array of all external drives. NULL if no drive is found
     * 
     */
    struct drive *drives;
    /**
     * @brief The count of drives. 0 if no drive is found
     * 
     */
    unsigned int count;

    /**
     * @brief The allocated count of drives
     * 
     */
    unsigned int alloc_drives;
};

/**
 * @brief Free a drive_helper struct
 * 
 * @param drive_helper The pointer of the pointer to the drive_helper sturct
 */
void drive_helper_free(struct drive_helper **drive_helper);

/**
 * @brief Get all external drives mount
 * 
 * @return struct drive_helper* A pointer to a drive_helper struct containing all external drives and drive count.
 */
struct drive_helper *drive_get_mounts();
#endif