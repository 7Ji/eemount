#ifndef HAVE_SYSTEMD_H
#define HAVE_SYSTEMD_H
#include "common.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enum for unit methods
 * 
 */
enum systemd_unit_methods {
    /**
     * @brief Method to start a unit
     * 
     */
    SYSTEMD_START_UNIT,
    /**
     * @brief Method to stop a unit
     * 
     */
    SYSTEMD_STOP_UNIT
};

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

/**
 * @brief Reload the systemd units
 * 
 * @return int 0 for success, 1 for failed
 */
int  systemd_reload();

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
 * @brief Stop a systemd unit
 * 
 * @param unit The unit name
 * @return int 0 for success, 1 for failed
 */
int systemd_stop_unit(const char *unit);

/**
 * @brief Start systemd units for system mounts
 * 
 * @param shelper The helper to systemd units
 * @return struct eemount_finished_helper* A helper struct containing successfully mounted systems
 */
struct eemount_finished_helper *systemd_start_unit_systems(struct systemd_mount_unit_helper *shelper);

#endif