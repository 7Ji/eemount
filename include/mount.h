#ifndef HAVE_MOUNT_H
#define HAVE_MOUNT_H
#include "common.h"

#include "systemd.h"
#include "drive.h"

struct mount_entry {
    unsigned int mount_id;
    unsigned int parent_id;
    unsigned int major;
    unsigned int minor;
    char *line;
    char *major_minor;
    char *root;
    char *mount_point;
    char *mount_options;
    char *optional;
    char *fstype;
    char *mount_source;
    char *super_options;
};

struct mount_table {
    struct mount_entry *entries;
    unsigned int count;
    unsigned int alloc_entries;
};

/**
 * @brief The enum of mount types, systemd or drive
 * 
 */
enum mount_type {
    /**
     * @brief Mount provided by a systemd mount unit
     * 
     */
    MOUNT_SYSTEMD,
    /**
     * @brief Mount provided by an external drive under /var/media
     * 
     */
    MOUNT_DRIVE
};

/**
 * @brief Struct to describe how a system should be mounted
 * 
 */
struct mount_system {
    /**
     * @brief The type of the mount, systemd or drive
     * 
     */
    int type;
    /**
     * @brief The name of the system
     * 
     */
    char *system;
    /**
     * @brief The corresponding systemd mount struct to describe how it should be mounted
     * 
     */
    struct systemd_mount *systemd;
    /**
     * @brief The corresponding external drive mount struct to describe how it should be mounted
     * 
     */
    struct drive *drive;
};

struct mount_system_simple {
    char *system;
    // struct mount_system_simple *prev;
    struct mount_system_simple *next;
};

/**
 * @brief The helper struct of all systems that should be mounted
 * 
 */
struct mount_helper {
    /**
     * @brief The count of systems
     * 
     */
    unsigned int count;
    /**
     * @brief Array of mount_system structs, each of them expalining how to mount itself
     * 
     */
    struct mount_system *systems;
    /**
     * @brief How many systems we've allocated memory
     * 
     */
    unsigned int alloc_systems;
};
bool mount_umount_entry(struct mount_entry *entry);
bool mount_umount_entry_recursive(struct mount_entry *entry, struct mount_table *table, unsigned int entry_id);
struct mount_entry *mount_find_entry_by_mount_point(const char *mount_point, struct mount_table *table);
void mount_free_table(struct mount_table **table);
struct mount_table* mount_get_table();
/**
 * @brief Get a list of all systems and how they should be mounted
 * 
 * @param systemd_helper A pointer to a systemd_mount_helper struct
 * @param drive_helper A pointer to a drive_helper struct
 * @return struct mount_helper* A pointer to mount_helper struct
 */

struct mount_helper *mount_get_systems(struct systemd_mount_helper *systemd_helper, struct drive_helper *drive_helper);
bool mount_prepare();
bool mount_ports_scripts();
#endif