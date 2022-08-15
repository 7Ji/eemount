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

#include "drive.h"
#include "eeconfig.h"
#include "logging.h"
#include "systemd.h"
int main() {
    // if (!systemd_init_bus()) {
    // // if (!systemd_init_bus() || !eeconfig_initialize()) {
    //     logging(LOGGING_FATAL, "Failed to initialize");
    //     return 1;
    // }
    // char *systemd_path;
    // if (systemd_encode_path("storage-roms.mount", &systemd_path)) {
    //     puts(systemd_path);
    // }
    // struct systemd_mount_helper *systemd_helper = systemd_list_service();
    // if (systemd_helper) {
    //     for (unsigned int i = 0; i<systemd_helper->count; ++i) {
    //         printf("name: %s\npath:%s\n------\n", systemd_helper->mounts[i].name, systemd_helper->mounts[i].path);
    //     }
    // }

    if (!eeconfig_initialize()) {
        return 1;
    }
    unsigned int i,j;
    struct drive_helper *list = drive_get_list();
    if (list) {
        for (i=0; i<list->count_drives; ++i) {
            printf("DRIVE %d / %d: ", i+1, list->count_drives);
            puts(list->drives[i].name);
            for (j=0; j<list->drives[i].count_systems; ++j) {
                printf(" - SYSTEM %d / %d: ", j+1, list->drives[i].count_systems-1);
                puts(list->drives[i].systems[j]);
            }
        }
        drive_helper_free(&list);
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