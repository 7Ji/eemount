#if 0
struct eemount_entry {
    /**
     * @brief Optional fileds
     * 
     */
    char *optional;
}


/**
 * @brief The enum of mount types, systemd or drive
 * 
 */
enum eemount_type {
    /**
     * @brief Mount provided by a systemd mount unit
     * 
     */
    EEMOUNT_SYSTEMD,
    /**
     * @brief Mount provided by an external drive under /var/media
     * 
     */
    EEMOUNT_DRIVE
};

/**
 * @brief Struct to describe how a system should be mounted
 * 
 */
struct eemount_system {
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


/**
 * @brief The helper struct of all systems that should be mounted
 * 
 */
struct eemount_helper {
    /**
     * @brief The count of systems
     * 
     */
    unsigned int count;
    /**
     * @brief Array of mount_system structs, each of them expalining how to mount itself
     * 
     */
    struct eemount_system *systems;
    /**
     * @brief How many systems we've allocated memory
     * 
     */
    unsigned int alloc_systems;
};

struct mount_system_simple {
    char *system;
    // struct mount_system_simple *prev;
    struct mount_system_simple *next;
};

struct mount_system_success_helper {
    char **systems;
    unsigned int count;
    unsigned int alloc_systems;
};
#endif