#include "drive.h"
#include "alloc.h"
#include "logging.h"
#include "sort.h"
#include "eeconfig.h"

#define MOUNT_EXT_PARENT        "/var/media"
#define MOUNT_EXT_ROMS_PARENT   "roms"
#define MOUNT_EXT_MARK          "emuelecroms"

#define MOUNT_EECONFIG_DELAY    "ee_load.delay"

static FILE *drive_check(const char *drive) {
    char *path_mark;
    struct stat stat_mark;
    FILE *fp;

    size_t len_path_mark = (sizeof(MOUNT_EXT_PARENT) + sizeof(MOUNT_EXT_ROMS_PARENT) + sizeof(MOUNT_EXT_MARK))/sizeof(char) + strlen(drive);

    if ((path_mark = malloc((len_path_mark + 1) * sizeof(char))) == NULL) {
        logging(LOGGING_WARNING, "Skipped external drive '%s': Failed to allocate memory for mark path under it", drive);
        return NULL;
    }

    int len_path_mark_actual = snprintf(path_mark, len_path_mark + 1, MOUNT_EXT_PARENT"/%s/"MOUNT_EXT_ROMS_PARENT"/"MOUNT_EXT_MARK, drive);
    if ((size_t)len_path_mark_actual != len_path_mark) {
        logging(LOGGING_ERROR, "Formatted mark path '%s' length is wrong: expected %zu, actual %d", path_mark, len_path_mark, len_path_mark_actual);
        free(path_mark);
        return NULL;
    }
    if (stat(path_mark, &stat_mark) != 0) {
        logging(LOGGING_INFO, "Skipped external drive '%s': mark file '%s' either does not exist or is inaccessiable", drive, path_mark);
        logging(LOGGING_DEBUG, "return value of stat(): %d, result: ", errno, strerror(errno));
        free(path_mark);
        return NULL;
    }
    if ((stat_mark.st_mode & S_IFMT) != S_IFREG) {
        logging(LOGGING_WARNING, "Skipped external drive '%s': mark '%s' is not a regular file", drive, path_mark);
        free(path_mark);
        return NULL;
    }
    if ((fp = fopen(path_mark, "r")) == NULL) {
        /* Should we assume this drive should be mounted as a whole? */
        logging(LOGGING_ERROR, "Skipped external drive '%s': can not open mark file '%s' as read-only", drive, path_mark);
        free(path_mark);
    }
    return fp;
}

static bool drive_scan(struct drive *drive, FILE *fp) {
    char *line, **system;
    bool note_empty = false;
    size_t size_line = 0;
    size_t len_line;
    while (getline(&line, &size_line, fp) != -1) {
        len_line = strcspn(line, "\r\n");
        if (len_line == 0) {
            if (!note_empty) {
                logging(LOGGING_WARNING, "Empty line(s) found in mark file of drive '%s', ignoring it/them.", drive->name);
                note_empty = true;
            }
            continue;
        }
        if (len_line > 256) {
            logging(LOGGING_WARNING, "Line ignored as it is too long, please fix the mark file for drive '%s' and fix the following line: %s", drive->name, line);
            continue;
        }
        if ((drive->systems = alloc_optional_resize(drive->systems, sizeof(char*)*(++(drive->count_systems)))) == NULL) {
            logging(LOGGING_ERROR, "Can not create/resize systems array when scanning drive '%s'", drive->name);
            return false;
        }
        system = &(drive->systems[drive->count_systems-1]);
        if ((*system = calloc(len_line + 1, sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Failed to re-allocate memory for new system name '%s' when scanning drive '%s'", line, drive->name);
            return false;
        } 
        strncpy(*system, line, len_line);
        logging(LOGGING_DEBUG, "Drive '%s': Found system '%s'", drive->name, *system);
    }
    free(line);
    logging(LOGGING_DEBUG, "Drive '%s': Found %d system(s)", drive->name, drive->count_systems);
    if (drive->count_systems > 1) {
        qsort(drive->systems, drive->count_systems, sizeof(char *), sort_compare_string);
        logging(LOGGING_DEBUG, "Sorted %d systems alphabetically of drive '%s'", drive->count_systems, drive->name);
    }
    return true;
}

static void drive_free(struct drive *drive) {
    for (unsigned int i=0; i<drive->count_systems; ++i) {
        alloc_free_if_used((void **)((drive->systems)+i));
    }
    alloc_free_if_used((void **)&(drive->systems));
    alloc_free_if_used((void **)&(drive->name));
}

void drive_helper_free(struct drive_helper **drive_helper) {
    for (unsigned int i=0; i<(*drive_helper)->count_drives; ++i) {
        drive_free(((*drive_helper)->drives) + i);
    }
    alloc_free_if_used((void **)&((*drive_helper)->drives));
}

struct drive_helper *drive_get_list() {
    DIR *dir;
    struct dirent *dir_entry;
    struct drive *drive, *buffer;
    struct drive_helper *drive_helper = NULL;
    FILE *fp;
    int delay = eeconfig_get_int(MOUNT_EECONFIG_DELAY);
    if (delay > 0) {
        logging(LOGGING_INFO, "Waiting %d seconds before getting drive list...", delay);
        sleep(delay);
    } else if (delay < 0) {
        logging(LOGGING_WARNING, "Configuration '"MOUNT_EECONFIG_DELAY"' is set to a negative number %d, it is ignored and you should fix the config", delay);
    }
    if ((dir = opendir(MOUNT_EXT_PARENT)) == NULL) {
        logging(LOGGING_ERROR, "Can not open '%s' to check all directories", MOUNT_EXT_PARENT);
        return NULL;
    }
    while ((dir_entry = readdir(dir)) != NULL) {
        if ((dir_entry->d_type != DT_DIR) || !strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, "..") || ((fp = drive_check(dir_entry->d_name)) == NULL)) {
            continue;
        }
        if (drive_helper == NULL) {
            logging(LOGGING_INFO, "Start scanning drive '%s'", dir_entry->d_name);
            if ((drive_helper = malloc(sizeof(struct drive_helper))) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for mount drive helper");
                goto free_file;
            }
            drive_helper->drives = NULL;
            drive_helper->count_drives = 0;
        }
        if (++(drive_helper->count_drives) > 1) {
            if ((buffer = realloc(drive_helper->drives, sizeof(struct drive)*(drive_helper->count_drives))) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for mount drive");
                --(drive_helper->count_drives);
                goto free_drives;
            }
            drive_helper->drives = buffer;
        } else {
            if ((drive_helper->drives = malloc(sizeof(struct drive))) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for mount drive");
                goto free_helper;
            }
        }
        drive = drive_helper->drives + drive_helper->count_drives - 1;
        drive->systems = NULL;
        drive->count_systems = 0;
        if ((drive->name = malloc((strlen(dir_entry->d_name)+1)*sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for drive name when finishing scanning drive '%s'", dir_entry->d_name);
            --(drive_helper->count_drives);
            goto free_drives;
        }
        strcpy(drive->name, dir_entry->d_name);
        if (drive_scan(drive, fp)) {
            logging(LOGGING_INFO, "Finished scanning drive '%s'", drive->name);
        } else {
            logging(LOGGING_ERROR, "Failed to scanning drive '%s'", drive->name);
        }
        fclose(fp);
    }
    closedir(dir);
    if (drive_helper && drive_helper->count_drives > 1) {
        qsort(drive_helper->drives, drive_helper->count_drives, sizeof(struct drive), sort_compare_drive);
        logging(LOGGING_DEBUG, "Sorted %d drives alphabetically", drive_helper->count_drives);
    }
    return drive_helper;

free_drives:
    unsigned int i, j;
    for (i=0; i<drive_helper->count_drives; ++i) {
        for (j=0; j<((drive_helper->drives)+i)->count_systems; ++j) {
            free(((drive_helper->drives)+i)->systems[j]);
        }
        free(((drive_helper->drives)+i)->name);
        free(((drive_helper->drives)+i)->systems);
    }
    free(drive_helper->drives);
free_helper:
    free(drive_helper);
free_file:
    fclose(fp);
    closedir(dir);
    return NULL;
}
