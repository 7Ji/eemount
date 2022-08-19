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

bool mount_partition_eeroms(struct mount_table *table) {
    // Check if /storage/.update is mounted, if so, use that partition
    logging(LOGGING_INFO, "Trying to mount EEROMS back to '"MOUNT_POINT_ROMS"'");
    struct mount_entry *entry = mount_find_entry_by_mount_point(mount_point_update, table);
    if (entry) {
        logging(LOGGING_INFO, "Found mount point '"MOUNT_POINT_UPDATE"', mount source: %s", entry->mount_source);
        if (mount(entry->mount_source, mount_point_roms, entry->fstype, MS_NOATIME, entry->super_options)) {
            logging(LOGGING_ERROR, "Failed to mount the partition %s");
        } else {
            return true;
        }
    }
    char *name = blkid_evaluate_tag("LABEL", "EEROMS", NULL);
    puts(name);
    // libmnt_optmap();
    // /storage/.update is not mounted, then check the drive providing /flash and /storage, get the 3rd partition of that drive. (If these two are on different drives, use the drive providing /flash, as it is mounted earlier during init than /storage)
    
    // Can't find /storage/.update in 
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

void umount_drive() {
    umount2("/storage/roms", MNT_FORCE);
}

bool mount_prepare() {
    struct mount_table* table = mount_get_table();
    struct mount_entry* entry;
    if (table) {
        entry = mount_find_entry_by_mount_point(mount_point_roms, table);
        while (entry) {
            if (!mount_umount_entry_recursive(entry, table, 0)) {
                mount_free_table(&table);
                return false;
            }
            mount_free_table(&table);
            if ((table = mount_get_table()) == NULL) {
                return false;
            }
            entry = mount_find_entry_by_mount_point(mount_point_roms, table);
        }
        mount_free_table(&table);
        return true;
    } else {
        return false;
    }
}