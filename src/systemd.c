/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "systemd_p.h"

int systemd_init_bus() {
    if(sd_bus_default_system(&systemd_bus) < 0) {
        fprintf(stderr, "Failed to open systemd bus\n");
        return 1;
    } else {
        return 0;
    }
}

void systemd_release() {
    if (systemd_bus) {
        sd_bus_flush_close_unref(systemd_bus);
        systemd_bus = NULL;
    } else {
        logging(LOGGING_WARNING, "Systemd bus not initialized yet, ignored release request");
    }
}

int systemd_reload() {
    logging(LOGGING_INFO, "Reloading systemd units...");
    if (systemd_bus == NULL) {
        logging(LOGGING_ERROR, "systemd not initialized yet, cannot reload");
        return 1;
    }
    sd_bus_slot *slot _cleanup_(sd_bus_slot_unrefp) = NULL;
    if (sd_bus_match_signal(systemd_bus, &slot, SYSTEMD_NAME, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "Reloading", NULL, NULL) < 0) {
        logging(LOGGING_ERROR, "Failed to match systemd signal, have not reloaded yet");
        return 1;
    }
    sd_bus_error error _cleanup_(sd_bus_error_free) = SD_BUS_ERROR_NULL;
    if (sd_bus_call_method(systemd_bus, SYSTEMD_NAME, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "Reload",  &error, NULL, NULL) < 0) {
        logging(LOGGING_ERROR, "Failed to call Reload method, error: %s", error.message);
        return 1;
    }
    bool active;
    int failed = 0;
    bool pending = false;
    sd_bus_message *msg _cleanup_(sd_bus_message_unrefp) = NULL;
    for (int i=0; i<SYSTEMD_START_TIMEOUT_LOOP; ++i) {
        while (sd_bus_process(systemd_bus, &msg)) {
            logging(LOGGING_DEBUG, "Processing sd_bus message...");
            if (msg) {
                if (sd_bus_message_read(msg, "b", &active) < 0) {
                    logging(LOGGING_WARNING, "Failed to read systemd message");
                    if (++failed == 3) {
                        logging(LOGGING_ERROR, "Failed to read for 3 times, assuming job failed");
                        return 1;
                    }
                    continue;
                }
                if (pending) {
                    if (!active) {
                        logging(LOGGING_INFO, "Waiting 1 extra second due to systemd being dishonest about the daemon reload status...");
                        sleep(1);
                        logging(LOGGING_INFO, "Successfully reloaded systemd units");
                        return 0;
                    }
                } else {
                    if (active) {
                        logging(LOGGING_INFO, "Reload in queue");
                        pending = true;
                    }
                }
            }
        }
        sd_bus_wait(systemd_bus, SYSTEMD_START_TIMEOUT);
        usleep(100000);
    }
    logging(LOGGING_INFO, "Failed to reload systemd units");
    return 1;
}

struct systemd_mount_unit_helper *systemd_get_units() {
    DIR *dir = opendir(SYSTEMD_UNIT_DIR);
    if (dir == NULL) {
        logging(LOGGING_ERROR, "Failed to open '"SYSTEMD_UNIT_DIR"' to scan systemd mount units");
        return NULL;
    }
    struct dirent *dir_entry;
    struct systemd_mount_unit_helper *helper = NULL;
    struct systemd_mount_unit *mount, *buffer;
    size_t len;
    size_t len_system;
    bool root;
    while ((dir_entry = readdir(dir))) {
        switch (dir_entry->d_type) {
            case DT_REG:
            case DT_LNK:
                break;
            default:
                continue;
        }
        if ((len = strlen(dir_entry->d_name)) < len_systemd_mount_root) {
            continue;
        }
        if (len > 255) {
            logging(LOGGING_WARNING, "File name too long (this hould not happen), ignored: %s", dir_entry->d_name);
        }
        if (strcmp(dir_entry->d_name + len - len_systemd_suffix, SYSTEMD_MOUNT_SUFFIX) || strncmp(dir_entry->d_name, SYSTEMD_MOUNT_ROOT, len_systemd_mount_root)) {
            continue;
        }
        if (helper == NULL) {
            if ((helper = malloc(sizeof(struct systemd_mount_unit_helper))) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for systemd mount unit helper!");
                goto free_dir;
            }
            if ((helper->mounts = malloc(sizeof(struct systemd_mount_unit)*ALLOC_BASE_SIZE)) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for systemd mount units!");
                goto free_helper;
            }
            helper->count = 0;
            helper->root = NULL;
            helper->alloc_mounts = ALLOC_BASE_SIZE;
        }
        if (++(helper->count) > helper->alloc_mounts) {
            helper->alloc_mounts *= ALLOC_MULTIPLIER;
            buffer = realloc(helper->mounts, sizeof(struct systemd_mount_unit)*(helper->alloc_mounts));
            if (buffer) {
                helper->mounts = buffer;
            } else {
                logging(LOGGING_ERROR, "Failed to reallocate for systemd mount units");
                goto free_mounts;
            }
        }
        len_system = len - len_systemd_suffix - len_systemd_mount_root;
        if (len_system) {
            // logging(LOGGING_INFO, "Encountered a systemd mount unit that possibly provides mount for system: %s", dir_entry->d_name); 
            if (--len_system == 0 || dir_entry->d_name[len_systemd_mount_root] != '-') {
                logging(LOGGING_INFO, "Omitted systemd mount unit %s");
                continue;
            }
            if (len_system == len_path_name_mark) {
                if (!strncmp(PATH_NAME_MARK, dir_entry->d_name+len_systemd_mount_root+1, len_path_name_mark)) {
                    logging(LOGGING_WARNING, "Ignored systemd mount unit %s since the system it provides ("PATH_NAME_MARK") is reserved", dir_entry->d_name);
                    continue;
                }
            }
            if (len_system == len_path_name_pscripts) {
                if (!strncmp(PATH_NAME_PSCRIPTS, dir_entry->d_name+len_systemd_mount_root+1, len_path_name_pscripts)) {
                    logging(LOGGING_WARNING, "Ignored systemd mount unit %s since the system it provides ("PATH_NAME_PSCRIPTS") is reserved", dir_entry->d_name);
                    continue;
                }
            }
            mount = helper->mounts + helper->count - 1;
            root = false;
        } else {
            // logging(LOGGING_INFO, "Encountered a systemd mount unit that possibly provides mount for roms root: %s", dir_entry->d_name); 
            if (helper->root) {
                logging(LOGGING_WARNING, "Multiple systemd units provided mount for roms root are found, only the first one will be used");
                continue;
            }
            helper->root = (mount = helper->mounts + helper->count - 1);
            root = true;
        }
        if ((mount->name = malloc((len+1)*sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for system mount unit's name");
            goto free_mounts;
        }
        if ((mount->system = malloc((len_system+1)*sizeof(char))) == NULL) {
            free(mount->name);
            logging(LOGGING_ERROR, "Failed to allocate memory for system mount unit's system name");
            goto free_mounts;
        }
        strcpy(mount->name, dir_entry->d_name);
        strncpy(mount->system, dir_entry->d_name+len_systemd_mount_root+1-root, len_system);
        mount->system[len_system] = '\0';
        if (mount->system[0]) {
            logging(LOGGING_INFO, "Found systemd mount unit '%s', providing mount for system '%s'", mount->name, mount->system);
        } else {
            logging(LOGGING_INFO, "Found systemd mount unit '%s', providing mount for roms dir itself", mount->name);
        }
    }
    if (helper) {
        if (helper->count > 1) {
            qsort(helper->mounts, helper->count, sizeof(struct systemd_mount_unit), sort_compare_systemd_mount_unit);
            // Root might change
            if (helper->root && helper->root->system[0] != '\0') {
                logging(LOGGING_DEBUG, "Root pointer not root anymore due to sorting, finding it again");
                for (unsigned int i=0; i<helper->count; ++i) {
                    if ((helper->mounts+i)->system[0] == '\0') {
                        helper->root = helper->mounts+i;
                        break;
                    }
                }
            }
            logging(LOGGING_DEBUG, "Sorted %d systemd mount units alphabetically", helper->count);
        }
        logging(LOGGING_DEBUG, "Found %u usable systemd mount units", helper->count);
    }
    closedir(dir);
    return helper;

free_mounts:
    if (--(helper->count)) {
        for (unsigned int i=0; i<helper->count; ++i) {
            free((helper->mounts+i)->name);
            free((helper->mounts+i)->system);
        }
        free(helper->mounts);
    }
free_helper:
    free(helper);
free_dir:
    closedir(dir);
    return NULL;
}

void systemd_mount_unit_helper_free(struct systemd_mount_unit_helper **shelper) {
    if (*shelper) {
        struct systemd_mount_unit *unit;
        if ((*shelper)->count) {
            for (unsigned int i=0; i<(*shelper)->count; ++i) {
                unit = (*shelper)->mounts + i;
                free(unit->name);
                free(unit->system);
            }
            free((*shelper)->mounts);
        }
        free(*shelper);
        *shelper = NULL;
    }
}

static int systemd_call_unit_method_on_manager(const char *unit, const char *method, uint32_t *job_id) {
    sd_bus_error error _cleanup_(sd_bus_error_free) = SD_BUS_ERROR_NULL;
    sd_bus_message *reply _cleanup_(sd_bus_message_unrefp) = NULL;
    if (sd_bus_call_method(systemd_bus, SYSTEMD_NAME, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, method, &error, &reply, "ss", unit, "replace") < 0) {
        logging(LOGGING_ERROR, "Failed to call %s method of systemd, error: %s", method, error.message);
        return 1;
    }
    if (reply == NULL) {
        logging(LOGGING_ERROR, "Got no reply from systemd, dont know what job it started, assuming failed");
        return 1;
    }
    char *job;
    if (sd_bus_message_read(reply, "o", &job) < 0) {
        logging(LOGGING_ERROR, "Failed to read systemd message, assuming failed");
        return 1;
    }
    if (job == NULL) {
        logging(LOGGING_ERROR, "Got no valid job object path, can not confirm if started successfully, assuming failed");
        return 1;
    }
    *job_id = strtoul(job + len_systemd_job_prefix, NULL, 10);
    return 0;
}

static inline int systemd_start_unit_barebone(const char *unit, uint32_t *job_id) {
    return systemd_call_unit_method_on_manager(unit, SYSTEMD_METHOD_START_UNIT, job_id);
}

static inline int systemd_stop_unit_barebone(const char *unit, uint32_t *job_id) {
    return systemd_call_unit_method_on_manager(unit, SYSTEMD_METHOD_STOP_UNIT, job_id);
}

static inline bool systemd_is_job_success(const char *result) {
    logging(LOGGING_INFO, "Job finished, checking result");
    if (result) {
        if (strcmp(result, "done")) {
            logging(LOGGING_ERROR, "Job failed");
            return false;
        } else {
            logging(LOGGING_INFO, "Job finished successfully");
            return true;
        }
    } else {
        logging(LOGGING_ERROR, "Job result invalid, assuming failed");
        return false;
    }
}

static int systemd_start_stop_unit(const char *unit, int method) {
    static int (*func)(const char*, uint32_t*);
    switch (method) {
        case (SYSTEMD_START_UNIT):
            func = &systemd_start_unit_barebone;
            break;
        case (SYSTEMD_STOP_UNIT):
            func = &systemd_stop_unit_barebone;
            break;
        default:
            logging(LOGGING_ERROR, "Failed to start/stop unit: method %d is not a valid enum int", method);
            return 1;
    }
    sd_bus_slot *slot _cleanup_(sd_bus_slot_unrefp) = NULL;
    if (sd_bus_match_signal(systemd_bus, &slot, SYSTEMD_NAME, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "JobRemoved", NULL, NULL) < 0) {
        logging(LOGGING_ERROR, "Failed to match systemd signal, have not started job yet");
        return 1;
    }
    uint32_t job_id;
    if ((*func)(unit, &job_id)) {
        return 1;
    }
    logging(LOGGING_INFO, "Started systemd job '%"PRIu32"', waiting for it to finish", job_id);
    uint32_t id;
    char *result;
    int failed = 0;
    sd_bus_message *msg _cleanup_(sd_bus_message_unrefp) = NULL;
    for (int i=0; i<SYSTEMD_START_TIMEOUT_LOOP; ++i) {
        while (sd_bus_process(systemd_bus, &msg)) {
            logging(LOGGING_DEBUG, "Processing sd_bus message...");
            if (msg) {
                if (sd_bus_message_read(msg, "uoss", &id, NULL, NULL, &result) < 0) {
                    logging(LOGGING_WARNING, "Failed to read systemd message");
                    if (++failed == 3) {
                        logging(LOGGING_ERROR, "Failed to read for 3 times, assuming job failed");
                        return 1;
                    }
                    continue;
                }
                logging(LOGGING_DEBUG, "Got message, ID: %"PRIu32, id);
                if (id == job_id) {
                    return !systemd_is_job_success(result);
                }
            }
        }
        sd_bus_wait(systemd_bus, SYSTEMD_START_TIMEOUT);
        usleep(100000);
    }
    logging(LOGGING_WARNING, "Waited timeout after %d seconds, assumming job failed", SYSTEMD_START_TIMEOUT);
    return 1;
}

int systemd_start_unit(const char *unit) {
    // Since we handle all these mount units by ourselves, overlapped enabled systemd mount units should be disabled during init:
    // rm -f /storage/.config/system.d/*.wants/storage-roms*.mount
    logging(LOGGING_INFO, "Starting systemd unit '%s'", unit);
    return systemd_start_stop_unit(unit, SYSTEMD_START_UNIT);
}

int systemd_stop_unit(const char *unit) {
    logging(LOGGING_INFO, "Stopping systemd unit '%s'", unit);
    return systemd_start_stop_unit(unit, SYSTEMD_STOP_UNIT);
}

struct eemount_finished_helper *systemd_start_unit_systems(struct systemd_mount_unit_helper *shelper) {
    // This should return an array of started systems
    struct systemd_mount_unit *root = shelper->root; // It's the caller's duty to make sure there's only one mount unit providing /storage/roms, systemd_get_mounts() should do that
    unsigned int jobs_count = shelper->count-(bool)root; // Actual count should minus one as the [root] one should be mounted earlier, not here
    if (!jobs_count) {
        logging(LOGGING_INFO, "No systems provided by systemd units");
        return NULL;
    }
    sd_bus_slot *slot _cleanup_(sd_bus_slot_unrefp) = NULL;
    if (sd_bus_match_signal(systemd_bus, &slot, SYSTEMD_NAME, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "JobRemoved", NULL, NULL) < 0) {
        logging(LOGGING_ERROR, "Failed to match systemd signal, have not started job yet");
        return NULL;
    }
    struct systemd_mount_unit *unit;
    struct systemd_mount_unit_job jobs[jobs_count];
    struct systemd_mount_unit_job *job;
    unsigned int job_array_id = 0;
    for (unsigned i=0; i<shelper->count; ++i) {
        unit = shelper->mounts+i;
        if (unit == root) { // Don't ever start root unit here
            continue;
        }
        job = jobs + job_array_id++;
        if (systemd_start_unit_barebone(unit->name, &(job->job_id))) {
            return NULL;
        }
        job->system = unit->system;
        // job->success = false;
        job->finished = false;
        logging(LOGGING_INFO, "Started systemd job %"PRIu32", will wait for it to finish later", job->job_id);
    }
    if (job_array_id != jobs_count) {
        logging(LOGGING_ERROR, "Jobs array mismatch");
        return NULL;
    }
    uint32_t id;
    char *result;
    int failed = 0;
    unsigned int finished = 0;
    char **buffer;
    struct eemount_finished_helper *mhelper = NULL;
    sd_bus_message *msg _cleanup_(sd_bus_message_unrefp) = NULL;
    logging(LOGGING_INFO, "Starting to wait for %u jobs to finish", jobs_count);
    for (int i=0; i<SYSTEMD_START_TIMEOUT_LOOP; ++i) {
        while (sd_bus_process(systemd_bus, &msg)) {
            logging(LOGGING_DEBUG, "Processing sd_bus message...");
            if (msg) {
                if (sd_bus_message_read(msg, "uoss", &id, NULL, NULL, &result) < 0) {
                    logging(LOGGING_WARNING, "Failed to read systemd message");
                    if (++failed == 3) {
                        logging(LOGGING_ERROR, "Failed to read for 3 times, assuming job failed");
                        return NULL;
                    }
                    continue;
                }
                logging(LOGGING_DEBUG, "Got message, job ID: %"PRIu32, id);
                for (job_array_id=0; job_array_id<jobs_count; ++job_array_id) {
                    job = jobs + job_array_id;
                    if (job->finished) {
                        continue;
                    }
                    if (id == job->job_id) {
                        job->finished = true;
                        if (systemd_is_job_success(result)) {
                            if (mhelper == NULL) {
                                if ((mhelper = malloc(sizeof(struct eemount_finished_helper))) == NULL) {
                                    logging(LOGGING_ERROR, "Failed to allocate memory for global mount helper");
                                    return NULL;
                                }
                                if ((mhelper->systems = malloc(sizeof(char *) * ALLOC_BASE_SIZE)) == NULL) {
                                    logging(LOGGING_ERROR, "Failed to allocate memory for mounted systems");
                                    free(mhelper);
                                    return NULL;
                                }
                                mhelper->alloc_systems = ALLOC_BASE_SIZE;
                                mhelper->count = 0;
                            }
                            if (++(mhelper->count) > mhelper->alloc_systems) {
                                mhelper->alloc_systems *= ALLOC_MULTIPLIER;
                                if ((buffer = realloc(mhelper->systems, sizeof(char *) * (mhelper->alloc_systems))) == NULL) {
                                    logging(LOGGING_ERROR, "Failed to reallocate memory for mounted systems");
                                    free(mhelper->systems);
                                    free(mhelper);
                                    return NULL;
                                }
                                mhelper->systems = buffer;
                            }
                            mhelper->systems[mhelper->count - 1] = job->system;
                        }
                        if (++finished == jobs_count) {
                            logging(LOGGING_INFO, "All systemd units started");
                            return mhelper;
                        }
                    }
                }
            }
        }
        sd_bus_wait(systemd_bus, SYSTEMD_START_TIMEOUT);
        usleep(100000);
    }
    return mhelper;
}