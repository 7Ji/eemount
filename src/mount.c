#include "mount_p.h"

struct mount_system_helper *mount_get_systems(struct systemd_mount_helper *systemd_helper, struct drive_helper *drive_helper) {
    struct mount_system_helper *system_helper = malloc(sizeof(struct mount_system_helper));
    if (system_helper == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate memory for systems helper");
        return NULL;
    }
    system_helper->count = 0;
    system_helper->systems = NULL;
    unsigned int i, j;
    bool duplicate;
    struct mount_system *system;
    if (systemd_helper) {
        struct systemd_mount *systemd_mount;
        for (i=0; i<systemd_helper->count; ++i) {
            systemd_mount = systemd_helper->mounts + i;
            if (system_helper->count) {
                duplicate = false;
                for (j=0; j<systemd_helper->count; ++j) {
                    if (!strcmp(systemd_mount->system, ((system_helper->systems)+j)->system)) {
                        logging(LOGGING_WARNING, "Duplicate mount entry for system '%s', only the first will be used, the one provided by systemd-unit '%s' is ignored", systemd_mount->system, ((system_helper->systems)+j)->system);
                        duplicate = true;
                        break;
                    }
                }
            }
            if (!duplicate) {
                system = (system_helper->systems)+(system_helper->count)-1;
                system->type = MOUNT_SYSTEMD;
                system->system = systemd_mount->system;
                system->systemd = systemd_mount;
                system->drive = NULL;
            }
        }
    }
    if (drive_helper) {
        for (i=0; i<drive_helper->count; ++i) {

        }
    }
    


    return NULL;
}

void umount_drive() {
    umount2("/storage/roms", MNT_FORCE);
}
