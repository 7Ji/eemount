#ifndef HAVE_SYSTEMD_H
#define HAVE_SYSTEMD_H
#include "common.h"

#include <stdbool.h>

/**
 * @brief Encode a unit name to sd-bus valid path
 * 
 * @param unit Pointer to the raw name of a unit
 * @param path Pointer to a pointer to string to return the encoded path
 * @return true Encoded successfully
 * @return false Failed to encode
 */
bool systemd_encode_path(char *unit, char **path);

/**
 * @brief Initialize the systemd_bus
 * 
 * @return true Initialized successfully
 * @return false Failed to initialize
 */
bool systemd_init_bus();

/**
 * @brief Release the systemd_bus, only call this at most once after a successful initialization
 * 
 */
void systemd_release();

/**
 * @brief Check if a systemd unit is active
 * 
 * @param path The sd-bus path of the unit
 * @return true The unit is active
 * @return false The unit is inactive
 */
bool systemd_is_active(char *path);

struct systemd_mount_unit {
    char *name;
    char *system;
};

struct systemd_mount_unit_helper {
    struct systemd_mount_unit *mounts;
    struct systemd_mount_unit *root;
    unsigned int count;
    unsigned int alloc_mounts;
};

/**
 * @brief The struct to hold information about a systemd mount unit
 * 
 */
struct systemd_mount {
    /**
     * @brief The name of the systemd mount unit. It should always be valid since you should get this from systemd directly
     * 
     */
    char *name;
    /**
     * @brief The object path of the systemd mount unit
     * 
     */
    char *path;
    /**
     * @brief The name of the system mounted by this unit. If it's empty then it mounts /storage/roms itself.
     * 
     */
    char *system; // When this is empty, it means /storage/roms itself. It should never be empty
};

/**
 * @brief The helper struct of systemd mount units
 * 
 */
struct systemd_mount_helper {
    /**
     * @brief Array of struct mounts, all systemd mounts under /storage/roms
     * 
     */
    struct systemd_mount *mounts;
    /**
     * @brief The root mount, maintaining /storage/roms itself
     * 
     */
    struct systemd_mount *root; 
    /**
     * @brief The count of systemd mounts
     * 
     */
    unsigned int count;
};

bool systemd_reload();
bool systemd_start_unit_no_wait(const char *unit);
bool systemd_start_unit(const char *unit);
struct systemd_mount_unit_helper *systemd_get_units();
/**
 * @brief Get all systemd mounts under /storage/roms
 * 
 * @return struct systemd_mount_helper* A helper struct to all systemd mounts. NULL if none can be found
 */
struct systemd_mount_helper *systemd_get_mounts();

/**
 * @brief Free a systemd_mount_helper struct
 * 
 * @param mounts_helper Pointer to the pointer you've mallocted for a systemd_mount_helpers struct
 */
void systemd_mount_helper_free (struct systemd_mount_helper **mounts_helper);
#endif