#ifndef MULTICALL
// #include "systemd.h"
// #include "mount.h"
// #include "block.h"
#include "systemd.h"
int main() {
    if (!systemd_init_bus()) {
        puts("Failed to start system bus");
        return 1;
    }
    if (systemd_reload()) {
        puts("reloaded");
    } else {
        puts("Failed to reload");
        systemd_release();
        return 1;
    }
    if (systemd_start_unit("storage-roms.mount")) {
        puts("Started");
    } else {
        puts("Failed");
    }
    systemd_release();
    // if (block_initialize()) {
    //     puts("hi");
    //     block_list();
    //     puts("hi");
    //     block_free();
    // }
    

    // if (!systemd_init_bus()) {
    //     puts("Cannot initialize systemd");
    //     return 1;
    // }
    // char *path;
    // if (systemd_encode_path("storage-roms-nes.mount", &path)) {
    //     puts(path);
    // } else {
    //     systemd_release();
    //     return 1;
    // }
    // if (systemd_is_active(path)) {
    //     puts("It is running");
    // } else {
    //     puts("Not running");
    // }
    // systemd_release();

    // if (mount_prepare()) {
    //     puts("ready");
    // } else {
    //     puts("oops");
    // }
    // const char mount_point[] = "/mnt/test_storage";
    // struct mount_table* table = mount_get_table();
    // struct mount_entry* entry;
    // if (table) {
    //     entry = mount_find_entry_by_mount_point(mount_point, table);
    //     while (entry) {
    //         mount_umount_entry_recursive(entry, table, 0);
    //         mount_free_table(&table);
    //         table = mount_get_table();
    //         entry = mount_find_entry_by_mount_point(mount_point, table);
    //     }
    //     mount_free_table(&table);
    // }
    return 0;
}
#else
#define PRINT_ONLY
#include "common.h"
#include <string.h>

#include "drive.h"
#include "eeconfig.h"
#include "logging.h"
#include "systemd.h"
#include "mount.h"

#define EXECUTABLE_EE_BUSYBOX       "ee_busybox"
#define EXECUTABLE_MOUNT_ROMFS      "mount_romfs"
#define EXECUTABLE_GET_EE_SETTING   "get_ee_setting"
#define EXECUTABLE_SET_EE_SETTING   "set_ee_setting"

enum multicall_handler {
    MULTICALL_MISSING,
    MULTICALL_MOUNT_ROMFS,
    MULTICALL_GET_EE_SETTING,
    MULTICALL_SET_EE_SETTING
};
static const size_t len_executable_ee_busybox = strlen(EXECUTABLE_EE_BUSYBOX);
static const size_t len_executable_mount_romfs = strlen(EXECUTABLE_MOUNT_ROMFS);
static const size_t len_executable_ee_setting = strlen(EXECUTABLE_GET_EE_SETTING);

static void multicall_description() {
    puts("EmuELEC multi-call binary, the following functions are supported: \n - get_ee_setting: getting emuelec settings \n - set_ee_setting: setting emuelec settings, \n - mount_romfs: mounting rom");
}

static void multicall_mountromfs() {

}
static void multicall_get_ee_setting(int argc, int arg_start, char **argv) {

}
static void multicall_set_ee_setting() {

}
int main(int argc, char **argv) {
    char *executable = basename(argv[0]);
    int arg_start;
    if (!strcmp(executable, EXECUTABLE_EE_BUSYBOX)) {
        if (argc > 1) {
            executable = argv[1];
            arg_start = 2;
        } else {
            multicall_description();
            return 0;
        }
    } else {
        arg_start = 1;
    }
    int arg_want = 0;
    size_t len_executable = strlen(executable);
    int multicall = MULTICALL_MISSING;
    if (len_executable >= len_executable_ee_busybox) {
        if (len_executable >= len_executable_mount_romfs) {
            if (len_executable >= len_executable_ee_setting) {
                if (!strcmp(executable, EXECUTABLE_GET_EE_SETTING)) {
                    multicall = MULTICALL_GET_EE_SETTING;
                    arg_want = 1;
                } else if (!strcmp(executable, EXECUTABLE_SET_EE_SETTING)) {
                    multicall = MULTICALL_SET_EE_SETTING;
                    arg_want = 1;
                }
            } else if (!strcmp(executable, EXECUTABLE_MOUNT_ROMFS)) {
                multicall = MULTICALL_MOUNT_ROMFS;
            }
        } else if (!strcmp(executable, EXECUTABLE_EE_BUSYBOX)) {
            multicall_description();
            return 0;
        }
    }
    if (arg_start + arg_want  > argc) {
        logging(LOGGING_ERROR, "%s needs at least %d arguments", executable, arg_want);
        return 1;
    }
    // Initialization
    switch (multicall) {
        case MULTICALL_MISSING:
            logging(LOGGING_FATAL, "Unknown multicall applet: %s", executable);
            return 1;
            break;
        case MULTICALL_MOUNT_ROMFS:
            if (!systemd_init_bus()) {
                logging(LOGGING_FATAL, "Failed to initialize systemd bus");
                return 1;
            }
            __attribute__((fallthrough));
        case MULTICALL_GET_EE_SETTING:
            logging_set_level(LOGGING_ERROR); // Since the caller expects our output on stdout
            if (arg_start > argc) {
                logging(LOGGING_FATAL, "Applet get_ee_setting expects at least one argument");
            }
            __attribute__((fallthrough));
        case MULTICALL_SET_EE_SETTING:
            if (!eeconfig_initialize()) {
                logging(LOGGING_FATAL, "Failed to initialize eeconfig");
                return 1;
            }
            break;
    }
    logging(LOGGING_DEBUG, "Initialization done");
    switch (multicall) {
        case MULTICALL_MOUNT_ROMFS:
            multicall_mount_romfs();
            break;
        case MULTICALL_GET_EE_SETTING:
            multicall_get_ee_config();
            break;
        case MULTICALL_SET_EE_SETTING:
            multicall_set_ee_config();
            break;
    }
    switch (multicall) {
        case MULTICALL_MOUNT_ROMFS:
            systemd_release();
            __attribute__((fallthrough));
        case MULTICALL_GET_EE_SETTING:
        case MULTICALL_SET_EE_SETTING:
            eeconfig_close();
            break;
    }
    return 0;
    if (argc > 1) {
        





        return 1;
    } else {
        puts("EmuELEC multi-call binary, the following functions are supported: \n - get_ee_setting: getting emuelec settings \n - set_ee_setting: setting emuelec settings, \n - mount_romfs: mounting rom");
    }
    // char escaped[] = "/home/nomad7ji/Development/eemount/wierd\\040ass\\040\\011\\012\\040name";
    // puts(escaped);
    // char *raw = mount_unescape_mountinfo(escaped);
    // if (raw) {
    //     puts(raw);
    // }
    // return 0;
    logging_set_level(LOGGING_DISABLED);
    struct mount_table *table = mount_get_table();
    return 0;
    // struct mount_info *info;
    free(table);
    // if(table) {
    //     for (unsigned int n=0; n<table->count; ++n) {
    //         info = (table->entries + n);
    //         puts(info->line);
    //         printf("%u: Source: %s, Root: %s, Mountpoint: %s\n", n, info->mount_source, info->root, info->mount_point);
    //     }
    // }
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
    struct mount_helper *mount_helper = mount_get_systems(systemd_helper, list);
    if (mount_helper) {
        struct mount_system *mount_system;
        for (i=0; i<mount_helper->count; ++i) {
            mount_system = mount_helper->systems + i;
            printf("System '%s' is provided by ", mount_system->system);
            switch (mount_system->type ) {
                case MOUNT_SYSTEMD:
                    printf("systemd unit '%s'\n", mount_system->systemd->name);
                    break;
                case MOUNT_DRIVE:
                    printf("external drive '%s'\n", mount_system->drive->name);
                    break;
                default:
                    printf("What? This can not be\n");
                    break;
            }
        }
    }


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
#endif