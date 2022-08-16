#include "mount_p.h"

static bool mount_systems_alloc_optional_resize(struct mount_helper *mount_helper) {
    struct mount_system *buffer;
    if (++(mount_helper->count) > mount_helper->alloc_systems) {
        if (mount_helper->alloc_systems) {
            mount_helper->alloc_systems *= 2;
            buffer = realloc(mount_helper->systems, sizeof(struct mount_system)*mount_helper->alloc_systems);
        } else {
            mount_helper->alloc_systems = 16;
            buffer = malloc(sizeof(struct mount_system)*16);
        }
        if (buffer) {
            mount_helper->systems = buffer;
            return true;
        } else {
            return false;
        }
    }
    return true;
}

struct mount_helper *mount_get_systems(struct systemd_mount_helper *systemd_helper, struct drive_helper *drive_helper) {
    struct mount_helper *mount_helper = malloc(sizeof(struct mount_helper));
    if (mount_helper == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate memory for systems helper");
        return NULL;
    }
    mount_helper->count = 0;
    mount_helper->systems = NULL;
    mount_helper->alloc_systems = 0;
    unsigned int i, j;
    bool duplicate;
    struct mount_system *system;
    if (systemd_helper) {
        struct systemd_mount *systemd_mount;
        for (i=0; i<systemd_helper->count; ++i) {
            duplicate = false;
            systemd_mount = systemd_helper->mounts + i;
            if (systemd_mount->system[0] == '\0') {
                continue;
            }
            if (mount_helper->count) {
                for (j=0; j<mount_helper->count; ++j) {
                    system = (mount_helper->systems)+j;
                    if (!strcmp(systemd_mount->system, system->system)) {
                        logging(LOGGING_WARNING, "Duplicate mount entry for system '%s', only the first will be used, the one provided by systemd-unit '%s' is ignored", system->system, systemd_mount->name);
                        duplicate = true;
                        break;
                    }
                }
            }
            if (!duplicate) {
                if (!mount_systems_alloc_optional_resize(mount_helper)) {
                    logging(LOGGING_ERROR, "Can not allocate memory for hybrid mount systems array");
                    free(mount_helper->systems);
                    free(mount_helper);
                    return NULL;
                }
                system = (mount_helper->systems)+(mount_helper->count-1);
                system->type = MOUNT_SYSTEMD;
                system->system = systemd_mount->system;
                system->systemd = systemd_mount;
                system->drive = NULL;
                logging(LOGGING_INFO, "System '%s' will be provided by systemd mount unit '%s'", system->system, systemd_mount->name);
            }
        }
    }
    unsigned int k;
    if (drive_helper) {
        struct drive *drive;
        char *drive_system;
        for (i=0; i<drive_helper->count; ++i) {
            drive = drive_helper->drives + i;
            if (drive->count) {
                for (j=0; j<drive->count; ++j) {
                    duplicate = false;
                    drive_system = drive->systems[j];
                    if (mount_helper->count) {
                        for (k=0; k<mount_helper->count; ++k) {
                            system = (mount_helper->systems) + k;
                            if (!strcmp(system->system, drive_system)) {
                                logging(LOGGING_WARNING, "Duplicate mount entry for system '%s', only the first will be used, the one provided by external drive '%s' is ignored", drive_system, drive->name);
                                duplicate = true;
                                break;
                            }
                        }
                    }
                    if (!duplicate) {
                        if (!mount_systems_alloc_optional_resize(mount_helper)) {
                            logging(LOGGING_ERROR, "Can not allocate memory for hybrid mount systems array");
                            free(mount_helper->systems);
                            free(mount_helper);
                            return NULL;
                        }
                        system = (mount_helper->systems)+(mount_helper->count-1);
                        system->type = MOUNT_DRIVE;
                        system->system = drive_system;
                        system->systemd = NULL;
                        system->drive = drive;
                        logging(LOGGING_INFO, "System '%s' will be provided by external drive '%s'", drive_system, drive->name);
                    }
                }
            }

        }
    }
    if (mount_helper->count) {
        qsort(mount_helper->systems, mount_helper->count, sizeof(struct mount_system), sort_compare_mount_system);
    }
    return mount_helper;
}

void umount_drive() {
    umount2("/storage/roms", MNT_FORCE);
}
