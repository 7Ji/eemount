#include "drive_p.h"

static FILE *drive_check(const char *drive) {
    char *path_mark;
    struct stat stat_mark;
    FILE *fp;

    size_t len_path_mark = (sizeof(MOUNT_EXT_PARENT) + sizeof(MOUNT_EXT_ROMS_PARENT) + sizeof(MOUNT_EXT_MARK))/sizeof(char) + strlen(drive);

    if ((path_mark = malloc((len_path_mark + 1) * sizeof(char))) == NULL) {
        logging(LOGGING_ERROR, "Skipped external drive '%s': Failed to allocate memory for mark path under it", drive);
        return NULL;
    }

    int len_path_mark_actual = snprintf(path_mark, len_path_mark + 1, MOUNT_EXT_PARENT"/%s/"MOUNT_EXT_ROMS_PARENT"/"MOUNT_EXT_MARK, drive);
    if ((size_t)len_path_mark_actual != len_path_mark) {
        logging(LOGGING_ERROR, "Formatted mark path '%s' length is wrong: expected %zu, actual %d", path_mark, len_path_mark, len_path_mark_actual);
        goto free_mark;
    }
    if (stat(path_mark, &stat_mark) != 0) {
        logging(LOGGING_INFO, "Skipped external drive '%s': mark file '%s' either does not exist or is inaccessiable", drive, path_mark);
        logging(LOGGING_DEBUG, "return value of stat(): %d, result: ", errno, strerror(errno));
        goto free_mark;
    }
    if ((stat_mark.st_mode & S_IFMT) != S_IFREG) {
        logging(LOGGING_ERROR, "Skipped external drive '%s': mark '%s' is not a regular file", drive, path_mark);
        goto free_mark;
    }
    if ((fp = fopen(path_mark, "r")) == NULL) {
        /* Should we assume this drive should be mounted as a whole? */
        logging(LOGGING_ERROR, "Skipped external drive '%s': can not open mark file '%s' as read-only", drive, path_mark);
        free(path_mark);
    }
    return fp;

free_mark:
    free(path_mark);
    return NULL;
}

static int drive_scan(struct drive *drive, FILE *fp) {
    char *line, **dsystem, **buffer;
    bool note_empty = false;
    bool illegal_char;
    size_t size_line = 0;
    size_t len_line;
    size_t len_drive;
    char *path = NULL;
    while (getline(&line, &size_line, fp) != -1) {
        len_line = strcspn(line, "\r\n");
        if (len_line == 0) {
            if (!note_empty) {
                logging(LOGGING_WARNING, "Empty line(s) found in mark file of drive '%s', ignoring it/them.", drive->name);
                note_empty = true;
            }
            continue;
        }
        if (len_line > 255) {
            logging(LOGGING_WARNING, "Line ignored as it is too long, please fix the mark file for drive '%s' and fix the following line: %s", drive->name, line);
            continue;
        }
        illegal_char = false;
        for (size_t i=0; i<len_line; ++i) {
            if (line[i] == '/') {
                illegal_char = true;
                break;
            }
        }
        if (illegal_char) {
            line[len_line] = '\0';
            logging(LOGGING_WARNING, "Ignored system '%s' defined for drive '%s' since the name contains illegal character", line, drive->name);
            continue;
        }
        if (len_line == len_drive_reserved_ports_scripts) {
            if (!strncmp(line, DRIVE_SYSTEM_RESERVED_PORTS_SCRIPTS, len_drive_reserved_ports_scripts)) {
                logging(LOGGING_WARNING, "Ignored system '"DRIVE_SYSTEM_RESERVED_PORTS_SCRIPTS"' for drive '%s' since it's reserved", drive->name);
                continue;
            }
        }
        if (len_line == len_drive_reserved_mark) {
            if (!strncmp(line, DRIVE_SYSTEM_RESERVED_MARK, len_drive_reserved_mark)) {
                logging(LOGGING_WARNING, "Ignored system '"DRIVE_SYSTEM_RESERVED_MARK"' for drive '%s' since it's reserved", drive->name);
                continue;
            }
        }
        if (path == NULL) {
            len_drive = strlen(drive->name);
            if ((path = malloc((len_mount_ext_parent + len_drive + len_mount_ext_roms_parent + 259 )*sizeof(char))) == NULL) {
                logging(LOGGING_ERROR, "Can not allocate memory for full path of external drive system");
                goto free_systems;
            }
        }
        snprintf(path, len_mount_ext_parent + len_drive + len_mount_ext_roms_parent + len_line + 4, MOUNT_EXT_PARENT"/%s/"MOUNT_EXT_ROMS_PARENT"/%s", drive->name, line);
        logging(LOGGING_DEBUG, "Checking if external system directory '%s' exists and create it if neccessary", path);
        if (util_mkdir(path, 0755)) {
            logging(LOGGING_WARNING, "Failed to create/verify directory '%s', omitting corresponding system");
            continue;
        }
        if ((++(drive->count)) > drive->alloc_systems) {
            if (drive->alloc_systems) {
                (drive->alloc_systems) *= ALLOC_MULTIPLIER;
                buffer = realloc(drive->systems, sizeof(char*)*(drive->alloc_systems));

            } else {
                drive->alloc_systems = ALLOC_BASE_SIZE;
                buffer = malloc(sizeof(char*)*ALLOC_BASE_SIZE);
            }
            if (buffer) {
                drive->systems = buffer;
            } else {
                logging(LOGGING_ERROR, "Can not create/resize systems array when scanning drive '%s'", drive->name);
                goto free_system;
            }
        }
        dsystem = (drive->systems) + (drive->count) - 1;
        if ((*dsystem = malloc((len_line + 1)*sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for new system name '%s' when scanning drive '%s'", line, drive->name);
            goto free_system;
        } 
        strncpy(*dsystem, line, len_line);
        (*dsystem)[len_line] = '\0';
        logging(LOGGING_DEBUG, "Drive '%s': Found system '%s'", drive->name, *dsystem);
    }
    if (path) {
        free(path);
    }
    free(line);
    logging(LOGGING_DEBUG, "Drive '%s': Found %d system(s)", drive->name, drive->count);
    if (drive->count > 1) {
        qsort(drive->systems, drive->count, sizeof(char *), sort_compare_string);
        logging(LOGGING_DEBUG, "Sorted %d systems alphabetically of drive '%s'", drive->count, drive->name);
    }
    return 0;

free_system:
    free(path);
    --(drive->count);
free_systems:
    for (unsigned int i=0; i<drive->count; ++i) {
        free(drive->systems[i]);
    }
    free(drive->systems);
    return 1;
}

static void drive_free(struct drive *drive) {
    for (unsigned int i=0; i<drive->count; ++i) {
        alloc_free_if_used((void **)((drive->systems)+i));
    }
    alloc_free_if_used((void **)&(drive->systems));
    alloc_free_if_used((void **)&(drive->name));
}

void drive_helper_free(struct drive_helper **drive_helper) {
    for (unsigned int i=0; i<(*drive_helper)->count; ++i) {
        drive_free(((*drive_helper)->drives) + i);
    }
    alloc_free_if_used((void **)&((*drive_helper)->drives));
    alloc_free_if_used((void**)drive_helper);
}

static bool drive_is_name_invalid(const char *name) {
    switch (name[0]) {
        case '\0':
            return true;
        case '.':
            switch (name[1]) {
                case '\0': // .
                    return true;
                case '.':   
                    if (name[2] == '\0') {      // ..
                        return true;
                    }
            }
            break;
        case 'E':
            if (!strcmp(name+1, DRIVE_NAME_RESERVED_EEROMS+1)) { // EEROMS
                return true;
            }
            break;
    }
    return false;
}

struct drive_helper *drive_get_mounts() {
    DIR *dir;
    struct dirent *dir_entry;
    struct drive *drive, *buffer;
    struct drive_helper *drive_helper = NULL;
    FILE *fp;
    int delay = eeconfig_get_int(MOUNT_EECONFIG_DELAY);
    char *target = eeconfig_get_string(MOUNT_EECONFIG_DRIVE);
    int retry = eeconfig_get_int(MOUNT_EECONFIG_RETRY);
    if (delay < 0) {
        logging(LOGGING_WARNING, "Configuration '"MOUNT_EECONFIG_DELAY"' (time in seconds we should wait before each external drive scan) is set to a negative number %d, it is ignored and you should fix the config", delay);
    }
    if (retry < 0) {
        logging(LOGGING_WARNING, "Configuration '"MOUNT_EECONFIG_RETRY"' (counts we should retry scanning external drives) is set to a negative number %d, it is ignored and you should fix the config");
    } 
    for (int try = 0; try < retry+1; ++try) {
        /*  Only under specific conditions will we retry:
            1. /var/media must be accessible (Which means we should start after systemd-tmpfiles-setup.service)
            2. No drive is found
        In the follow situations we straightly abandon the scan:
            1. Memory allocation failed (The system must be utterly broken)
            2. At least one drive is found, even if there're still other drives to be scanned
        So if you have at least one slow drive among all your external drives, you must set the delay according to the slowest one
        */
        logging(LOGGING_INFO, "Scanning external drives, try %d of %d", try+1, retry+1);
        if (delay > 0) {
            logging(LOGGING_INFO, "Waiting %d seconds before getting drive list...", delay);
            sleep(delay);
        }
        if ((dir = opendir(MOUNT_EXT_PARENT)) == NULL) {
            logging(LOGGING_ERROR, "Can not open '%s' to check all directories", MOUNT_EXT_PARENT);
            return NULL;
        }
        while ((dir_entry = readdir(dir))) {
            if ((dir_entry->d_type != DT_DIR) || drive_is_name_invalid(dir_entry->d_name) || (target && strcmp(dir_entry->d_name, target)) || ((fp = drive_check(dir_entry->d_name)) == NULL)) {
                /*
                Skip the entry in case of:
                    1. The entry is not directory
                    2. The entry is either . (/var/media itself) or .. (/var) or EEROMS
                    3. The entry's name is different from global.externalmount, if it is set (If it's empty then we ignore this)
                    4. The markfile under it can't be opened (either becase it can't be opened or does not exist)
                */
                continue;
            }
            logging(LOGGING_INFO, "Start scanning drive '%s'", dir_entry->d_name);
            if (drive_helper == NULL) {
                if ((drive_helper = malloc(sizeof(struct drive_helper))) == NULL) {
                    logging(LOGGING_ERROR, "Failed to allocate memory for mount drive helper");
                    goto free_file;
                }
                if ((drive_helper->drives = malloc(sizeof(struct drive)*ALLOC_BASE_SIZE)) == NULL) {
                    logging(LOGGING_ERROR, "Failed to allocate memory for mount drive entry '%s'", dir_entry->d_name);
                    goto free_helper;
                }
                drive_helper->count = 0;
                drive_helper->alloc_drives = ALLOC_BASE_SIZE;
            }
            if ((++(drive_helper->count)) > drive_helper->alloc_drives) {
                drive_helper->alloc_drives *= ALLOC_MULTIPLIER;
                buffer = realloc(drive_helper->drives, sizeof(struct drive)*(drive_helper->alloc_drives));
                if (buffer) {
                    drive_helper->drives = buffer;
                } else {
                    logging(LOGGING_ERROR, "Failed to reallocate memory for mount drive entry '%s'", dir_entry->d_name);
                    goto free_drives;
                }
            }
            drive = drive_helper->drives + drive_helper->count - 1;
            drive->systems = NULL;
            drive->count = 0;
            drive->alloc_systems = 0;
            if ((drive->name = strdup(dir_entry->d_name)) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for drive name of drive '%s'", dir_entry->d_name);
                goto free_drives;
            }
            logging(LOGGING_INFO, "Start reading mark file for systems of drive '%s'", drive->name);
            if (drive_scan(drive, fp)) {
                logging(LOGGING_ERROR, "Failed to scanning drive '%s'", drive->name);
            } else {
                logging(LOGGING_INFO, "Finished scanning drive '%s'", drive->name);
            }
            fclose(fp);
        }
        closedir(dir);
        if (drive_helper) {
            if (drive_helper->count > 1) {
                qsort(drive_helper->drives, drive_helper->count, sizeof(struct drive), sort_compare_drive);
                logging(LOGGING_DEBUG, "Sorted %u drives alphabetically", drive_helper->count);
            }
            logging(LOGGING_INFO, "Found %u usable external drives", drive_helper->count);
            return drive_helper;
        }
    }
    logging(LOGGING_WARNING, "No external rom drives found");
    return NULL;

free_drives:
    if (--(drive_helper->count)) {
        unsigned int i, j;
        for (i=0; i<drive_helper->count; ++i) {
            // free(((drive_helper->drives)+i)->content);
            for (j=0; j<((drive_helper->drives)+i)->count; ++j) {
                free(((drive_helper->drives)+i)->systems[j]);
            }
            free(((drive_helper->drives)+i)->name);
            free(((drive_helper->drives)+i)->systems);
        }
        free(drive_helper->drives);
    }
free_helper:
    free(drive_helper);
free_file:
    fclose(fp);
    closedir(dir);
    return NULL;
}