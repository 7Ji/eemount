#if 0
static bool mount_systems_alloc_optional_resize(struct mount_helper *mount_helper) {
    struct mount_system *buffer;
    if (++(mount_helper->count) > mount_helper->alloc_systems) {
        if (mount_helper->alloc_systems) {
            mount_helper->alloc_systems *= ALLOC_MULTIPLIER;
            buffer = realloc(mount_helper->systems, sizeof(struct mount_system)*mount_helper->alloc_systems);
        } else {
            mount_helper->alloc_systems = ALLOC_BASE_SIZE;
            buffer = malloc(sizeof(struct mount_system)*ALLOC_BASE_SIZE);
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
// void umount_drive() {
//     umount2("/storage/roms", MNT_FORCE);
// }

static int eemount_umount_roms_sub() {
    /*
        This is seperate from umount_roms, as a result of mount_find_entry_by_mount_point_start_with() not used there because we don't want to umount folders like /stoarge/roms_backup
    */
   logging(LOGGING_INFO, "Umounting all mount points under "MOUNT_POINT_ROMS"...");
    struct eemount_table *table = eemount_get_table();
    if (table) {
        struct eemount_entry *entry = eemount_find_entry_by_mount_point_start_with(MOUNT_POINT_ROMS"/", table, len_mount_point_roms+1);
        while (entry) {
            if (eemount_umount_entry_recursive(entry, table, 0)) {
                eemount_free_table(&table);
                return 1;
            }
            eemount_free_table(&table);
            if ((table = eemount_get_table()) == NULL) {
                return 1;
            }
            entry = eemount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
        }
        eemount_free_table(&table);
        return 0;
    } else {
        logging(LOGGING_INFO, "Failed to umount all mount points under "MOUNT_POINT_ROMS"");
        return 1;
    }
}

static int eemount_prepare() {
    if (eemount_umount_roms()) {
        logging(LOGGING_ERROR, "Failed to prepare mount");
        return 1;
    } else {
        logging(LOGGING_INFO, "All mountpoints under "MOUNT_POINT_ROMS" umounted, ready for actual mount work");
        return 0;
    }
    struct mount_table *table = mount_get_table();
    if (table) {
        struct mount_entry *entry = mount_find_entry_by_mount_point_start_with(MOUNT_POINT_ROMS, table, len_mount_point_roms);
        while (entry) {
            if (!mount_umount_entry_recursive(entry, table, 0)) {
                mount_free_table(&table);
                return false;
            }
            mount_free_table(&table);
            if ((table = mount_get_table()) == NULL) {
                return false;
            }
            entry = mount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
        }
        mount_free_table(&table);
        return true;
    } else {
        return false;
    }
    struct mount_entry *entry, *last;
    for (int i=0; i<3; ++i) {
        if (table == NULL) {
            return false;
        }
        entry = mount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
        last = NULL;
        while (entry) {
            if (!mount_umount_entry_recursive(entry, table, 0)) {
                mount_free_table(&table);
                return false;
            }
            mount_free_table(&table);
            if ((table = mount_get_table()) == NULL) {
                return false;
            }
            last = entry;
            entry = mount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
        }
        if (last && ((entry = mount_find_entry_by_mount_point(MOUNT_POINT_EXT, table)) == NULL)) {
            /* Make sure the EEROMS partition is mounted to /var/media/EEROMS */
            struct libmnt_context *cxt = mnt_new_context();
            if (cxt == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for mount context");
                mount_free_table(&table);
                return false;
            }
            mnt_context_set_source(cxt, last->mount_source);
            mnt_context_set_mflags(cxt, MS_NOATIME);
            mnt_context_set_target(cxt, MOUNT_POINT_EXT);
            if (mnt_context_mount(cxt)) {
                // mkdir();
            }
            mnt_free_context(cxt);
        }
        entry = mount_find_entry_by_mount_point_start_with(MOUNT_POINT_ROMS, table);
        mount_free_table(&table);
        table = mount_get_table();
    }
    return true;
}
#endif