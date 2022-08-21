#ifndef HAVE_SYSTEMD_H
#define HAVE_SYSTEMD_H
#include "common.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Strcut for systemd mount units
 * 
 */
struct systemd_mount_unit {
    /**
     * @brief The name of the unit
     * 
     */
    char *name;
    /**
     * @brief The corresponding system, cannot be NULL even for root
     * 
     */
    char *system;
};

/**
 * @brief Helper struct for systemd mount units
 * 
 */
struct systemd_mount_unit_helper {
    /**
     * @brief Mount units
     * 
     */
    struct systemd_mount_unit *mounts;
    /**
     * @brief The mount unit for roms root (/storage/roms)
     * 
     */
    struct systemd_mount_unit *root;
    /**
     * @brief Count of mount units
     * 
     */
    unsigned int count;
    /**
     * @brief Allocated count for mounts
     * 
     */
    unsigned int alloc_mounts;
};

/**
 * @brief Struct for mount unit jobs
 * 
 */
struct systemd_mount_unit_job {
    /**
     * @brief The system name
     * 
     */
    char *system;
    /**
     * @brief Job id
     * 
     */
    uint32_t job_id;
    // bool success;
    /**
     * @brief If it's already finished
     * 
     */
    bool finished;
};
#if 0
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
/**
 * @brief Encode a unit name to sd-bus valid path
 * 
 * @param unit Pointer to the raw name of a unit
 * @param path Pointer to a pointer to string to return the encoded path
 * @return true Encoded successfully
 * @return false Failed to encode
 */
bool systemd_encode_path(char *unit, char **path);
#endif

/**
 * @brief Initialize the systemd_bus
 * 
 * @return 0 Initialized successfully
 * @return 1 Failed to initialize
 */
int systemd_init_bus();

/**
 * @brief Release the systemd_bus, only call this at most once after a successful initialization
 * 
 */
void systemd_release();

#if 0
/**
 * @brief Check if a systemd unit is active
 * 
 * @param path The sd-bus path of the unit
 * @return true The unit is active
 * @return false The unit is inactive
 */
bool systemd_is_active(char *path);
#endif

/**
 * @brief Reload the systemd units
 * 
 * @return int 0 for success, 1 for failed
 */
int  systemd_reload();

#if 0
bool systemd_start_unit_no_wait(const char *unit);

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



/**
 * @brief Get all systemd mount units under /storage/roms
 * 
 * @return struct systemd_mount_unit_helper* A helper strcut to all systemd mount units
 */
struct systemd_mount_unit_helper *systemd_get_units();

/**
 * @brief Free a systemd_mount_unit_helper struct
 * 
 * @param shelper Pointer to a systemd_mount_unit_helper struct
 */
void systemd_mount_unit_helper_free(struct systemd_mount_unit_helper **shelper);

/**
 * @brief Start a systemd unit
 * 
 * @param unit The unit name
 * @return int 0 for success, 1 for failed
 */
int systemd_start_unit(const char *unit);

/**
 * @brief Start systemd units for system mounts
 * 
 * @param shelper The helper to systemd units
 * @return struct eemount_finished_helper* A helper struct containing successfully mounted systems
 */
struct eemount_finished_helper *systemd_start_unit_systems(struct systemd_mount_unit_helper *shelper);

#endif