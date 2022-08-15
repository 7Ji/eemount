#if 0
int main() {
    if (!eeconfig_initialize()) {
        return 1;
    } 
    char *value = eeconfig_get_string("ee_load.drive");
    if (value) {
        printf("load drive is %s\n", value);
        free(value);
    } else {
        puts("no load drive");
    }
    bool bvalue = eeconfig_get_bool("ee_load.legacy", false);
    if (bvalue) {
        puts("using old mount");
    } else {
        puts("using new mount");
    }
    eeconfig_close();
    return 0;
}
#endif

#define PRINT_ONLY
#include "drive.h"
#include "eeconfig.h"
#include "logging.h"
#include "systemd.h"
int main() {
    if (!systemd_init_bus() || !eeconfig_initialize()) {
        logging(LOGGING_FATAL, "Failed to initialize");
        return 1;
    }
    struct systemd_mount_helper *systemd_helper = systemd_get_mounts();
#ifndef PRINT_ONLY
    bool root_mounted = false;
    if (systemd_helper) {
        if (systemd_helper->root) {
            logging(LOGGING_INFO, "Using systemd unit '%s' as mount handler for /storage/roms", systemd_helper->root->name);
            systemd_start(systemd_helper->root->path);
            root_mounted = true;
        }
    }
#endif
    unsigned int i;
    struct drive_helper *list = drive_get_mounts();
#ifndef PRINT_ONLY
    if (list && !root_mounted) {
        for (i=0; i<list->count_drives; ++i) {
            if (list->drives[i].count == 0) {
                logging(LOGGING_INFO, "Mounting roms under external drive '%s' to /storage/roms", list->drives[i].name);
                drive_mount(list->drives[i].name);
                root_mounted = true;
                break;
            }
        }
    }
    if (!root_mounted) {
        logging(LOGGING_WARNING, "Neither systemd unit nor external drive mount for /storage/roms is found, mounting EEROMS");
        mount_eeroms();
    }
#endif
    unsigned int j;
    if (list) {
        for (i=0; i<list->count; ++i) {
            printf("DRIVE %d / %d: ", i+1, list->count);
            puts(list->drives[i].name);
            for (j=0; j<list->drives[i].count; ++j) {
                printf(" - SYSTEM %d / %d: ", j+1, list->drives[i].count-1);
                puts(list->drives[i].systems[j]);
            }
        }
        drive_helper_free(&list);
    }

    if (systemd_helper) {
        for (i = 0; i<systemd_helper->count; ++i) {
            printf("system: %s\nname: %s\npath:%s\n------\n", systemd_helper->mounts[i].system, systemd_helper->mounts[i].name, systemd_helper->mounts[i].path);
        }
        systemd_mount_helper_free(&systemd_helper);
    }

    systemd_release();
    eeconfig_close();
    return 0;
}

#if 0
int main() {
    #include "systemd.h"
    char *path;
    if (systemd_encode_path("srv-netshare.mount", &path)) {
        puts(path);
    } else {
        return 1;
    }
    if (systemd_init_bus()) {
        puts("Systemd bus initialized successfully");
    } else {
        return 1;
    }

    if (systemd_is_active(path)) {
        printf("Unit is running\n");
    } else {
        printf("Unit is not running\n");
    }

    systemd_release();

    // if (!systemd_init_bus()) {
    //     return 1;
    // }
    // if (systemd_is_active(unit = escape_string("sshd.service")) ) {
    //     puts("active");
    // }
    // else {
    //     puts("inactive");
    // }
    // free(unit);
    // systemd_release();
    return 0;
}
#endif