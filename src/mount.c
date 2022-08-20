#include "mount_p.h"
bool mount_umount_entry(struct mount_entry *entry) {
    if (umount2(entry->mount_point, MNT_FORCE)) {
        logging(LOGGING_ERROR, "Failed to umount %s", entry->mount_point);
        return false;
    } else {
        logging(LOGGING_INFO, "Successfully umounted %s", entry->mount_point);
        return true;
    }
}

bool mount_umount_entry_recursive(struct mount_entry *entry, struct mount_table *table, unsigned int entry_id) {
    bool ret = true;
    for (unsigned int i=entry_id+1; i<table->count; ++i) {
        if ((table->entries+i)->parent_id == entry->mount_id) {
            ret &= mount_umount_entry_recursive((table->entries+i), table, i);
        }
    }
    ret &= mount_umount_entry(entry);
    return ret;
}

struct mount_entry *mount_find_entry_by_mount_point(const char *mount_point, struct mount_table *table) {
    struct mount_entry *entry;
    for (unsigned i=0; i<table->count; ++i) {
        entry = table->entries + i;
        if (!strcmp(mount_point, entry->mount_point)) {
            return entry;
        }
    }
    return NULL;
}

struct mount_entry *mount_find_entry_by_mount_point_start_with(const char *mount_point, struct mount_table *table, size_t len) {
    struct mount_entry *entry;
    for (unsigned i=0; i<table->count; ++i) {
        entry = table->entries + i;
        if (!strncmp(mount_point, entry->mount_point, len)) {
            return entry;
        }
    }
    return NULL;
}

bool mount_is_mount_point(const char* path) {
    struct mount_table *table = mount_get_table();
    if (table == NULL) {
        return false;
    }
    for (unsigned i=0; i<table->count; ++i) {
        if (!strcmp(path, (table->entries+i)->mount_point)) {
            mount_free_table(&table);
            return true;
        }
    }
    mount_free_table(&table);
    return false;
}

void mount_free_table(struct mount_table **table) {
    if (*table) {
        if ((*table)->entries) {
            for (unsigned int i=0; i<(*table)->count; ++i) {
                free(((*table)->entries+i)->line);
            }
            free((*table)->entries);
        }
        free(*table);
        *table=NULL;
    }
}

struct libmnt_context *mount_context_reset_or_new(struct libmnt_context *cxt) {
    if (mnt_reset_context(cxt)) {
        logging(LOGGING_ERROR, "Failed to reset mount context");
        mnt_free_context(cxt);
        if ((cxt = mnt_new_context()) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate another mount context");
        }
    } 
    return cxt;
}

bool mount_dir_update(const char* path) {
    /* ONLY call this after EEROMS is successfully mounted 
       Otherwise this must be a util_mkdir_recursive() call since the dir itself (path) is not guaranteed to exist
    */
    if (mount_is_mount_point(MOUNT_POINT_UPDATE)) {
        return true;
    }
    if (!util_mkdir(path, 0755)) {
        return false;
    }
    if (mount(path, MOUNT_POINT_UPDATE, NULL, MS_BIND, NULL)) {
        return false;
    } else {
        return true;
    }
}

bool mount_partition_eeroms(const char *mount_point) {
    if (mount_is_mount_point(mount_point)) {
        return true;
    }
    struct libmnt_context *cxt = mnt_new_context();
    if (cxt == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate mount context");
        return false;
    }
    logging(LOGGING_INFO, "Trying to mount EEROMS to '%s", mount_point);
    struct mount_table *table = mount_get_table();
    if (table) { // If we can get partition table, try the one providing .update and the 3rd partition of boot drive, this is optimal
        logging(LOGGING_INFO, "Trying to get the underlying partition of "MOUNT_POINT_UPDATE);
        struct mount_entry *entry = mount_find_entry_by_mount_point(MOUNT_POINT_UPDATE, table);
        if (entry) {
            logging(LOGGING_INFO, "'"MOUNT_POINT_UPDATE"' is mounted, using the underlying partition %s as EEROMS", entry->mount_source);
            if (mnt_context_set_source(cxt, entry->mount_source) || mnt_context_set_options(cxt, entry->mount_options) || mnt_context_set_target(cxt, mount_point) || mnt_context_mount(cxt)) {
                logging(LOGGING_ERROR, "Failed to mount the partition %s", entry->mount_source);
            } else {
                mnt_free_context(cxt);
                mount_free_table(&table);
                return true;
            }
            if ((cxt = mount_context_reset_or_new(cxt)) == NULL) {
                mount_free_table(&table);
                return false;
            }
        }
        // /storage/.update is not mounted, then check the drive providing /flash and /storage, get the 3rd partition of that drive. (use /flash first, then /storage, as /flash is mounted earlier than /storage during init)
        char mount_points[2][9] = {
            "/flash",
            "/storage"
        };
        size_t len_partition;
        char *partition;
        for (int i=0; i<2; ++i) {
            logging(LOGGING_INFO, "Trying to get the underlying disk of '%s' and use the 3rd partition there", mount_points[i]);
            if ((entry = mount_find_entry_by_mount_point(mount_points[i], table)) == NULL) {
                logging(LOGGING_ERROR, "Can not find the partitions under %s", mount_points[i]);
                continue;
            }
            if ((partition = strdup(entry->mount_source)) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for partition name");
                continue;
            }
            len_partition = strlen(partition);
            partition[len_partition-1] = '3';
            if (mnt_context_set_source(cxt, partition) || mnt_context_set_mflags(cxt, MS_NOATIME) || mnt_context_set_target(cxt, mount_point) || mnt_context_mount(cxt)) {
                logging(LOGGING_ERROR, "Failed to mount the partition %s", partition);
            } else {
                free(partition);
                mnt_free_context(cxt);
                mount_free_table(&table);
                return true;
            }
            if ((cxt = mount_context_reset_or_new(cxt)) == NULL) {
                free(partition);
                mount_free_table(&table);
                return false;
            }
        }
        mount_free_table(&table);
    }
    // Can not find any, at least try to find any other partition with LABEL=EEROMS
    logging(LOGGING_INFO, "Last hope, mounting LABEL=EEROMS directly, we cannot guarantee it's the correct EEROMS");
    if (mnt_context_set_source(cxt, "LABEL=EEROMS") || mnt_context_set_mflags(cxt, MS_NOATIME) || mnt_context_set_target(cxt, mount_point) || mnt_context_mount(cxt)) {
        logging(LOGGING_ERROR, "Failed to mount LABEL=EEROMS");
    } else {
        mnt_free_context(cxt);
        return true;
    }
    mnt_free_context(cxt);
    logging(LOGGING_ERROR, "All tries to mount '%s' failed", mount_point);
    return false;
}

struct mount_table* mount_get_table() {
    struct mount_table *table = malloc(sizeof(struct mount_table));
    if (table == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate memory for mount table");
        return NULL;
    }
    if ((table->entries = malloc(sizeof(struct mount_entry)*16)) == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate memory for mount infos");
        goto free_table;
    }
    table->count = 0;
    table->alloc_entries = ALLOC_BASE_SIZE;
    FILE *fp = fopen(MOUNT_MOUNTINFO, "r");
    if (fp == NULL) {
        logging(LOGGING_ERROR, "Failed to open '"MOUNT_MOUNTINFO"' to read mounted table");
        goto free_entries;
    }
    char *line;
    size_t size_line = 0;
    size_t len_line;
    char *token;
    const char delim[] = " ";
    unsigned int segments = 0;
    struct mount_entry *entry;
    bool optional_end;
    char *endptr;
    /* An example line:
    36 35 98:0 /mnt1 /mnt2 rw,noatime master:1 - ext3 /dev/root rw,errors=continue
    */
    while (getline(&line, &size_line, fp) != -1) {
        if ((len_line = strcspn(line, "\r\n")) == 0) {
            continue;
        }
        if (++(table->count) > table->alloc_entries) {
            table->alloc_entries*=ALLOC_MULTIPLIER;
            if ((entry = realloc(table->entries, sizeof(struct mount_entry)*(table->alloc_entries))) == NULL) { // Pure evilness, entry is just buffer here
                logging(LOGGING_ERROR, "Failed to reallocate memory for table entries");
                goto free_fp;
            }
            table->entries = entry;
        }
        entry = table->entries + (table->count-1);
        if ((entry->line = strdup(line)) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for internal line of mount info");
            goto free_fp;
        }
        // We do these on our dupped line, so we don't need to malloc all strings
        optional_end = false;
        entry->line[len_line] = '\0';
        // logging(LOGGING_DEBUG, "Processing mountinfo line: %s", entry->line);
        token = strtok(entry->line, delim);
        segments = 0;
        while (token) {
            switch(++segments) {
                case 1: // mount ID
                    entry->mount_id = util_uint_from_ulong(strtoul(token, &endptr, 10));
                    // logging(LOGGING_DEBUG, "Mount ID is %u", entry->mount_id);
                    break;
                case 2: // parent ID
                    entry->parent_id = util_uint_from_ulong(strtoul(token, &endptr, 10));
                    // logging(LOGGING_DEBUG, "Parent ID is %u", entry->parent_id);
                    break;
                case 3: // major:minor
                    entry->major_minor = token;
                    // logging(LOGGING_DEBUG, "Major:Minor pair is %s", token);
                    entry->major = util_uint_from_ulong(strtoul(token, &endptr, 10));
                    entry->minor = util_uint_from_ulong(strtoul(endptr+1, &endptr, 10));
                    break;
                case 4: // root
                    entry->root = token;
                    util_unesacpe_mountinfo_in_place(entry->root);
                    // logging(LOGGING_DEBUG, "Root is %s", token);
                    break;
                case 5: // mount point
                    entry->mount_point = token;
                    util_unesacpe_mountinfo_in_place(entry->mount_point);
                    // logging(LOGGING_DEBUG, "Mount Point is %s", token);
                    break;
                case 6: // mount options
                    entry->mount_options = token;
                    // logging(LOGGING_DEBUG, "Mount Options are %s", token);
                    break;
                case 7:
                    if (optional_end) {
                        entry->fstype = token;
                        // logging(LOGGING_DEBUG, "Filesystem Type is %s", token);
                    } else {
                        if (!strcmp(token, "-")) {
                            optional_end = true;
                            // logging(LOGGING_DEBUG, "Optional field ends");
                        } else {
                            // logging(LOGGING_DEBUG, "Ignored optional field %s", token);
                        }
                        --segments;
                    }
                    break;
                case 8:
                    entry->mount_source = token;
                    // logging(LOGGING_DEBUG, "Mount Source is %s", token);
                    break;
                case 9:
                    entry->super_options = token;
                    // logging(LOGGING_DEBUG, "Super Options are %s", token);
                    break;
                default:
                    // logging(LOGGING_ERROR, "More columns found in mountinfo than expected");
                    break;
            }
            token = strtok(NULL, delim);
        }
    }
    fclose(fp);
    return table;
free_fp:
    fclose(fp);
free_entries:
    free(table->entries);
free_table:
    free(table);
    return NULL;
}

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

bool mount_umount_roms() {
    struct mount_table *table = mount_get_table();
    if (table) {
        struct mount_entry *entry = mount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
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
}

bool mount_umount_roms_sub() {
    /*
        This is seperate from umount_roms, as a result of mount_find_entry_by_mount_point_start_with() not used there because we don't want to umount folders like /stoarge/roms_backup
    */
    struct mount_table *table = mount_get_table();
    if (table) {
        struct mount_entry *entry = mount_find_entry_by_mount_point_start_with(MOUNT_POINT_ROMS"/", table, len_mount_point_roms+1);
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
}


bool mount_prepare() {
    if (mount_umount_roms() && mount_umount_roms_sub()) {
        logging(LOGGING_INFO, "All mountpoints under "MOUNT_POINT_ROMS" umounted, ready for actual mount work");
        return true;
    } else {
        logging(LOGGING_ERROR, "Failed to prepare mount");
        return false;
    }
    // struct mount_table *table = mount_get_table();
    // if (table) {
    //     struct mount_entry *entry = mount_find_entry_by_mount_point_start_with(MOUNT_POINT_ROMS, table, len_mount_point_roms);
    //     while (entry) {
    //         if (!mount_umount_entry_recursive(entry, table, 0)) {
    //             mount_free_table(&table);
    //             return false;
    //         }
    //         mount_free_table(&table);
    //         if ((table = mount_get_table()) == NULL) {
    //             return false;
    //         }
    //         entry = mount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
    //     }
    //     mount_free_table(&table);
    //     return true;
    // } else {
    //     return false;
    // }
    // struct mount_entry *entry, *last;
    // for (int i=0; i<3; ++i) {
    //     if (table == NULL) {
    //         return false;
    //     }
    //     entry = mount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
    //     last = NULL;
    //     while (entry) {
    //         if (!mount_umount_entry_recursive(entry, table, 0)) {
    //             mount_free_table(&table);
    //             return false;
    //         }
    //         mount_free_table(&table);
    //         if ((table = mount_get_table()) == NULL) {
    //             return false;
    //         }
    //         last = entry;
    //         entry = mount_find_entry_by_mount_point(MOUNT_POINT_ROMS, table);
    //     }
    //     if (last && ((entry = mount_find_entry_by_mount_point(MOUNT_POINT_EXT, table)) == NULL)) {
    //         /* Make sure the EEROMS partition is mounted to /var/media/EEROMS */
    //         struct libmnt_context *cxt = mnt_new_context();
    //         if (cxt == NULL) {
    //             logging(LOGGING_ERROR, "Failed to allocate memory for mount context");
    //             mount_free_table(&table);
    //             return false;
    //         }
    //         mnt_context_set_source(cxt, last->mount_source);
    //         mnt_context_set_mflags(cxt, MS_NOATIME);
    //         mnt_context_set_target(cxt, MOUNT_POINT_EXT);
    //         if (mnt_context_mount(cxt)) {
    //             // mkdir();
    //         }
    //         mnt_free_context(cxt);
    //     }
    //     entry = mount_find_entry_by_mount_point_start_with(MOUNT_POINT_ROMS, table);
    //     mount_free_table(&table);
    //     table = mount_get_table();
    // }
    // return true;
}

int mount_root(struct systemd_mount_unit_helper *shelper, struct drive_helper *dhelper) {
    if (!util_mkdir(MOUNT_POINT_ROMS, 0755)) {
        logging(LOGGING_ERROR, "Can not create/valid directory '"MOUNT_POINT_ROMS"', all mount operations cancelled");
        return -1;
    }
    // Only one providing /storage/roms, that is storage-roms.mount
    if (shelper && shelper->root && systemd_start_unit(shelper->root->name)) {
        logging(LOGGING_INFO, "Successfully mounted "MOUNT_POINT_ROMS" through systemd");
        if (mount_partition_eeroms(MOUNT_POINT_EXT)) {
            mount_dir_update(MOUNT_POINT_EXT"/.update");
        }
        return 0;
    }
    if (dhelper) {
        char path[len_mount_ext_parent + 262];
        char *name;
        // Multiple can provide /storage/roms, we go alphabetically
        for (unsigned i=0; i<dhelper->count; ++i) {
            if ((dhelper->drives+i)->count == 0) { 
                name = (dhelper->drives+i)->name;
                snprintf(path, len_mount_ext_parent + strlen(name) + 7, MOUNT_EXT_PARENT"/%s/roms", name);
                logging(LOGGING_INFO, "Binding '%s' to '"MOUNT_POINT_ROMS"'", path);
                if (!mount(path, MOUNT_POINT_ROMS, NULL, MS_BIND, NULL)) {
                    logging(LOGGING_INFO, "Successfully binded "MOUNT_POINT_ROMS);
                    if (mount_partition_eeroms(MOUNT_POINT_EXT)) {
                        mount_dir_update(MOUNT_POINT_EXT"/.update");
                    }
                    return 0;
                }
            }
        }
    }
    // Since all failed, try to get EEROMS back
    if (mount_partition_eeroms(MOUNT_POINT_ROMS)) {
        mount_dir_update(MOUNT_POINT_ROMS"/.update"); // Optionally mount .update
        return 0;
    }
    return 1;
}

struct mount_finished_helper *mount_drive_systems(struct drive_helper *dhelper, struct mount_finished_helper *mhelper) {
    unsigned int i, j, k;
    struct drive *drive;
    char *dsystem, **buffer;
    char path_source[len_mount_ext_parent+517]; // +1 for /, +255 for drive name, +1 for /, +4 for roms, +255 for system name, +1 for null
    char path_target[len_mount_point_roms+257]; // +1 for /, +255 for name, +1 for null
    size_t len_system;
    size_t len_drive;
    bool unique;
    for (i=0; i<dhelper->count; ++i) {
        drive = dhelper->drives+i;
        len_drive = 0;
        for (j=0; j<drive->count; ++j) {
            dsystem = drive->systems[j];
            unique = true;
            for (k=0; k<mhelper->count; ++k) {
                if (!strcmp(mhelper->systems[k], dsystem)) {
                    unique = false;
                    break;
                }
            }
            if (unique) {
                len_system = strlen(dsystem);
                snprintf(path_target, len_mount_point_roms + len_system + 2, MOUNT_POINT_ROMS"/%s", dsystem);
                if (!util_mkdir(path_target, 0755)) {
                    logging(LOGGING_ERROR, "Failed to binding system %s under drive %s on %s as we failed to confirm the target is a folder and further create if not", dsystem, drive->name, path_target);
                    continue;
                }
                if (len_drive == 0) {
                    len_drive = strlen(drive->name);
                }
                snprintf(path_source, len_mount_ext_parent + len_drive + len_system + 8, MOUNT_EXT_PARENT"/%s/roms/%s", drive->name, dsystem);
                logging(LOGGING_DEBUG, "Binding '%s' to '%s'", path_source, path_target);
                if (mount(path_source, path_target, NULL, MS_BIND, NULL)) {
                    logging(LOGGING_ERROR, "Failed to bind '%s' to '%s'", path_source, path_target);
                    continue;
                }
                logging(LOGGING_INFO, "Successfully binded '%s' to '%s'", path_source, path_target);
                if (mhelper == NULL) {
                    if ((mhelper = malloc(sizeof(struct mount_finished_helper))) == NULL) {
                        return NULL;
                    }
                    if ((mhelper->systems = malloc(sizeof(char *) * ALLOC_BASE_SIZE)) == NULL) {
                        free(mhelper);
                        return NULL;
                    }
                    mhelper->alloc_systems = ALLOC_BASE_SIZE;
                    mhelper->count = 0;
                }
                if (++(mhelper->count) > mhelper->alloc_systems) {
                    mhelper->alloc_systems *= ALLOC_MULTIPLIER;
                    if ((buffer = realloc(mhelper->systems, sizeof(char *) * mhelper->alloc_systems)) == NULL) {
                        free(mhelper->systems);
                        free(mhelper);
                        return NULL;
                    }
                    mhelper->systems = buffer;
                }
                mhelper->systems[mhelper->count-1] = dsystem;
            }
        }
    }
    return mhelper;
}

struct mount_finished_helper *mount_systems(struct systemd_mount_unit_helper *shelper, struct drive_helper *dhelper) {
    struct mount_finished_helper *mhelper = NULL;
    if (shelper) {
        mhelper = systemd_start_unit_systems(shelper);
    }
    if (dhelper) {
        mhelper = mount_drive_systems(dhelper, mhelper);
    }
    return mhelper;
}

bool mount_ports_scripts() {
    // ports on /storage/roms/ports_scripts type overlay (rw,relatime,lowerdir=/usr/bin/ports,upperdir=/emuelec/ports,workdir=/storage/.tmp/ports-workdir)
    if (!util_mkdir(MOUNT_POINT_PORTS_SCRIPTS, 0755)) {
        return false;
    }
    struct libmnt_context *cxt = mnt_new_context();
    if (cxt == NULL) {
        logging(LOGGING_ERROR, "Failed to obtain mount context for "MOUNT_POINT_PORTS_SCRIPTS);
        return false;
    }
    if (mnt_context_set_source(cxt, MOUNT_PORTS_SCRIPTS_NAME) || mnt_context_set_fstype(cxt, MOUNT_PORTS_SCRIPTS_FS) || mnt_context_set_target(cxt, MOUNT_POINT_PORTS_SCRIPTS) || mnt_context_set_options(cxt, MOUNT_PORTS_SCRIPTS_OPTIONS) || mnt_context_mount(cxt)) {
        logging(LOGGING_ERROR, "Failed to mount "MOUNT_PORTS_SCRIPTS_NAME);
        mnt_free_context(cxt);
        return false;
    }
    logging(LOGGING_INFO, "Successfully mounted "MOUNT_PORTS_SCRIPTS_NAME);
    mnt_free_context(cxt);
    return true;
}


bool mount_routine() {
    if (!mount_prepare())  {
        logging(LOGGING_ERROR, "Failed to prepare mounting");
        return false;
    }
    struct systemd_mount_unit_helper *shelper = systemd_get_units();
    struct drive_helper *dhelper = drive_get_mounts();
    if (shelper) {
        systemd_reload();
    }
    if (mount_root(shelper, dhelper) < 0) {
        systemd_mount_unit_helper_free(&shelper);
        drive_helper_free(&dhelper);
        return false;
    }
    mount_ports_scripts();
    struct mount_finished_helper *mhelper = mount_systems(shelper, dhelper);
    if (mhelper && mhelper->count) {
        logging(LOGGING_DEBUG, "The following systems are mounted:");
        for (unsigned int i=0; i<mhelper->count; ++i) {
            logging(LOGGING_DEBUG, " -> %s", mhelper->systems[i]);
        }
        free(mhelper->systems);
        free(mhelper);
    }
    return true;
}