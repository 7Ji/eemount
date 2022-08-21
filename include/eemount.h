#ifndef HAVE_MOUNT_H
#define HAVE_MOUNT_H
#include "common.h"

#include "systemd.h"
#include "drive.h"

/**
 * @brief An entry in the partition  table
 * 
 */
struct eemount_entry {
    /**
     * @brief The mount ID of the entry
     * 
     */
    unsigned int mount_id;
    /**
     * @brief The mount ID of the entry's parent
     * 
     */
    unsigned int parent_id;
    /**
     * @brief The device major ID
     * 
     */
    unsigned int major;
    /**
     * @brief The device minor ID
     * 
     */
    unsigned int minor;
    /**
     * @brief (preserved) The line storing all other property strings
     * 
     */
    char *line;
    /**
     * @brief The major:minor pair of the entry
     * 
     */
    char *major_minor;
    /**
     * @brief The root in the filesystem of this mount
     * 
     */
    char *root;
    /**
     * @brief The mount point of the entry
     * 
     */
    char *mount_point;
    /**
     * @brief The mount options that kernel accepts
     * 
     */
    char *mount_options;
    /**
     * @brief The filesystem type of the mount
     * 
     */
    char *fstype;
    /**
     * @brief The mount source (filesystem/block)
     * 
     */
    char *mount_source;
    /**
     * @brief The per-super options for the filesystem drive
     * 
     */
    char *super_options;
};

/**
 * @brief The partition table reflecting the content of /proc/self/mountinfo
 * 
 */
struct eemount_table {
    /**
     * @brief Entries
     * 
     */
    struct eemount_entry *entries;
    /**
     * @brief Count of entries in the table
     * 
     */
    unsigned int count;
    /**
     * @brief Allocated memory for entries
     * 
     */
    unsigned int alloc_entries;
};

/**
 * @brief The helper for finished system mounts
 * 
 */
struct eemount_finished_helper {
    /**
     * @brief The count of mounted systems
     * 
     */
    unsigned int count;
    /**
     * @brief The count of allocated memory for systems
     * 
     */
    unsigned int alloc_systems;
    /**
     * @brief The array of system names, DO NOT free this, all pointers here point to the system names in other structs
     * 
     */
    char **systems;
};

/**
 * @brief Get a partition table
 * 
 * @return struct eemount_table* A table, or NULL if failed
 */
struct eemount_table* eemount_get_table();


/**
 * @brief Unmount a mount table entry
 * 
 * @param entry The entry
 * @return int 0 success, 1 failed
 */
int eemount_umount_entry(struct eemount_entry *entry);

/**
 * @brief Unmount a mount table entry recursively
 * 
 * @param entry The entry
 * @param table The table
 * @param entry_id The id of the entry
 * @return unsigned int 0 success, positive failed count
 */
unsigned int eemount_umount_entry_recursive(struct eemount_entry *entry, struct eemount_table *table, unsigned int entry_id);

/**
 * @brief Find a mount entry in the mount table by the mount point
 * 
 * @param mount_point The mount point
 * @param table The table
 * @return struct eemount_entry* The found entry, or NULL if failed
 */
struct eemount_entry *eemount_find_entry_by_mount_point(const char *mount_point, struct eemount_table *table);

/**
 * @brief Find a mount entry in the mount table whose mount point starts with certain pattern
 * 
 * @param mount_point The mount point starting portion
 * @param table The table
 * @param len The length of the pattern. Or 0 to calculate it. 
 * @return struct eemount_entry* 
 */
struct eemount_entry *eemount_find_entry_by_mount_point_start_with(const char *mount_point, struct eemount_table *table, size_t len);

/**
 * @brief Check if a path is a mount point
 * 
 * @param path The path to check
 * @param table The already read table to speed up, or NULL to read on demand
 * @return true The path is a mount point
 * @return false The path is not a mount point
 */
bool eemount_is_mount_point(const char* path, struct eemount_table *table);

/**
 * @brief Free a mount table
 * 
 * @param table The pointer to the table to free
 */
void eemount_free_table(struct eemount_table **table);
/**
 * @brief Get a list of all systems and how they should be mounted
 * 
 * @param systemd_helper A pointer to a systemd_mount_helper struct
 * @param drive_helper A pointer to a drive_helper struct
 * @return struct mount_helper* A pointer to mount_helper struct
 */

/**
 * @brief Perform the actual mounting routine
 * 
 * @return int 0 if success, 1 if failed
 */
int eemount_routine();
#endif