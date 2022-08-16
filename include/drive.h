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
    /**
     * @brief The names array of systems under the drive, NULL if no system is found (the drive should be used as a whole)
     * 
     */
    char **systems;
    /**
     * @brief The count of systems under the drive. 0 if no system is found and it should be used as a whole
     * 
     */
    unsigned int count;
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