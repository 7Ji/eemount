#include "systemd_p.h"

bool systemd_encode_path(char *unit, char **path) {
    if (sd_bus_path_encode(SYSTEMD_PATH_UNIT, unit, path) < 0) {
        fprintf(stderr, "Failed to encode unit to sd-bus path\n");
        return false;
    } else {
        return true;
    }
}

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

bool systemd_is_active(char *path) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    char *msg = NULL;

    if(sd_bus_get_property_string(systemd_bus, SYSTEMD_DESTINATION, path, SYSTEMD_INTERFACE_UNIT, "ActiveState", &err, &msg) < 0) {
        logging(LOGGING_ERROR, "Failed to get status \n - Message: %s \n Error: %s \n", msg, err.message);
        sd_bus_error_free(&err);
        return false;
    }
    logging(LOGGING_DEBUG, "Got active state of unit: %s", msg);
    if (strcmp(msg, "active")) {
        return false;
    } else {
        return true;
    }
}

bool systemd_reload() {
    sd_bus_error error _cleanup_(sd_bus_error_free) = SD_BUS_ERROR_NULL;
    if (sd_bus_call_method(systemd_bus, SYSTEMD_DESTINATION, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "Reload",  &error, NULL, NULL) < 0) {
        logging(LOGGING_ERROR, "Failed to call Reload method, error: %s", error.message);
        return false;
    }
    return true;
}

bool systemd_start_unit_no_wait(const char *unit) {
    // This should only be used for systems. 
    logging(LOGGING_DEBUG, "Starting systemd unit '%s', no wait", unit);
    sd_bus_error error _cleanup_(sd_bus_error_free) = SD_BUS_ERROR_NULL;
    if (sd_bus_call_method(systemd_bus, SYSTEMD_DESTINATION, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "StartUnit", &error, NULL, "ss", unit, "replace") < 0) {
        logging(LOGGING_ERROR, "Failed to call StartUnit() method, error: %s", error.message);
        return false;
    }
    return true;
}

bool systemd_start_unit_barebone(const char *unit, uint32_t *job_id) {
    sd_bus_error error _cleanup_(sd_bus_error_free) = SD_BUS_ERROR_NULL;
    sd_bus_message *reply _cleanup_(sd_bus_message_unrefp) = NULL;
    if (sd_bus_call_method(systemd_bus, SYSTEMD_DESTINATION, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "StartUnit", &error, &reply, "ss", unit, "replace") < 0) {
        logging(LOGGING_ERROR, "Failed to call StartUnit method of systemd, error: %s", error.message);
        return false;
    }
    if (reply == NULL) {
        logging(LOGGING_ERROR, "Got no reply from systemd, dont know what job it started, assuming failed");
        return false;
    }
    char *job;
    if (sd_bus_message_read(reply, "o", &job) < 0) {
        logging(LOGGING_ERROR, "Failed to read systemd message, assuming failed");
        return false;
    }
    if (job == NULL) {
        logging(LOGGING_ERROR, "Got no valid job object path, can not confirm if started successfully, assuming failed");
        return false;
    }
    *job_id = strtoul(job + len_systemd_job_prefix, NULL, 10);
    return true;
}

bool systemd_is_job_success(const char *result) {
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

bool systemd_start_unit(const char *unit) {
    // Since we handle all these mount units by ourselves, overlapped enabled systemd mount units should be disabled during init:
    // rm -f /storage/.config/system.d/*.wants/storage-roms*.mount
    sd_bus_slot *slot _cleanup_(sd_bus_slot_unrefp) = NULL;
    if (sd_bus_match_signal(systemd_bus, &slot, SYSTEMD_DESTINATION, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "JobRemoved", NULL, NULL) < 0) {
        logging(LOGGING_ERROR, "Failed to match systemd signal, have not started job yet");
        return false;
    }
    uint32_t job_id;
    if (!systemd_start_unit_barebone(unit, &job_id)) {
        return false;
    }
    logging(LOGGING_INFO, "Started systemd job '%"PRIu32"', waiting for it to finish", job_id);
    uint32_t id;
    char *result;
    int failed = 0;
    sd_bus_message *msg _cleanup_(sd_bus_message_unrefp) = NULL;
    for (int i=0; i<SYSTEMD_START_TIMEOUT; ++i) {
        while (sd_bus_process(systemd_bus, &msg)) {
            logging(LOGGING_DEBUG, "Processing sd_bus message...");
            if (msg) {
                if (sd_bus_message_read(msg, "uoss", &id, NULL, NULL, &result) < 0) {
                    logging(LOGGING_WARNING, "Failed to read systemd message");
                    if (++failed == 3) {
                        logging(LOGGING_ERROR, "Failed to read for 3 times, assuming job failed");
                        return false;
                    }
                    continue;
                }
                logging(LOGGING_DEBUG, "Got message, ID: %"PRIu32, id);
                if (id == job_id) {
                    return systemd_is_job_success(result);
                    // logging(LOGGING_INFO, "Job finished, checking result");
                    // if (result) {
                    //     if (strcmp(result, "done")) {
                    //         logging(LOGGING_ERROR, "Job failed");
                    //         return false;
                    //     } else {
                    //         logging(LOGGING_INFO, "Job finished successfully");
                    //         return true;
                    //     }
                    // } else {
                    //     logging(LOGGING_ERROR, "Job result invalid, assuming failed");
                    //     return false;
                    // }
                }
            }
        }
        sd_bus_wait(systemd_bus, SYSTEMD_START_TIMEOUT);
        sleep(1);
    }
    logging(LOGGING_WARNING, "Waited timeout after %d seconds, assumming job failed", SYSTEMD_START_TIMEOUT);
    return false;
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
    if (sd_bus_match_signal(systemd_bus, &slot, SYSTEMD_DESTINATION, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "JobRemoved", NULL, NULL) < 0) {
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
        if (!systemd_start_unit_barebone(unit->name, &(job->job_id))) {
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
    for (int i=0; i<SYSTEMD_START_TIMEOUT; ++i) {
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
        sleep(1);
    }
    return mhelper;
// jobs_finished:
//     struct mount_system_simple *system_head = NULL, *system = NULL, *next, *buffer;
//     bool insert;
//     for (job_array_id = 0; job_array_id<jobs_count; ++job_array_id) {
//         job = jobs+job_array_id;
//         if (!(job->success)) {
//             continue;
//         }
//         if ((buffer = malloc(sizeof(struct mount_system_simple))) == NULL) {
//             logging(LOGGING_ERROR, "Failed to allocate memory for simple mount system chain table");
//             return system_head;
//         }
//         buffer->system = job->system;
//         if (system_head == NULL) {
//             buffer->next = NULL;
//             system_head = buffer;
//         } else {
//             system = system_head;
//             insert = false;
//             while ((next = system->next)) {
//                 if (strcmp(job->system, system->system)>0 && strcmp(job->system, next->system)<0) {
//                     // buffer->system = job->system;
//                     buffer->next = next;
//                     system->next = buffer;
//                     insert = true;
//                     break;
//                 }
//                 system = system->next;
//             }
//             if (!insert) {
//                 buffer->next = NULL;
//                 system->next = buffer;
//             }
//         }
//     }
//     return system_head;
}

/*
static char *systemd_system_from_name(const char *name) {
    size_t len = strlen(name);
    char *system;
    if (len == 18) { // Special case for storage-roms.mount
        if ((system = malloc(sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Can not allocate memory for system name when scanning systemd units");
            return NULL;
        }
        system[0] = '\0';
        return system;
    }
    size_t len_system = len - len_systemd_mount_root - len_systemd_suffix - 1;
    if ((system = malloc((len_system + 1)*sizeof(char))) == NULL) {
        logging(LOGGING_ERROR, "Can not allocate memory for system name when scanning systemd units");
        return NULL;
    }
    strncpy(system, name + len_systemd_mount_root + 1, len_system);
    system[len_system] = '\0';
    return system;
}
*/

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
            if (len_system == len_systemd_reserved_mark) {
                if (!strncmp(SYSTEMD_SYSTEM_RESERVED_MARK, dir_entry->d_name+len_systemd_mount_root+1, len_systemd_reserved_mark)) {
                    logging(LOGGING_WARNING, "Ignored systemd mount unit %s since the system it provides ("SYSTEMD_SYSTEM_RESERVED_MARK") is reserved", dir_entry->d_name);
                    continue;
                }
            }
            if (len_system == len_systemd_reserved_ports_scripts) {
                if (!strncmp(SYSTEMD_SYSTEM_RESERVED_PORTS_SCRIPTS, dir_entry->d_name+len_systemd_mount_root+1, len_systemd_reserved_ports_scripts)) {
                    logging(LOGGING_WARNING, "Ignored systemd mount unit %s since the system it provides ("SYSTEMD_SYSTEM_RESERVED_PORTS_SCRIPTS") is reserved", dir_entry->d_name);
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

#ifdef SYSTEMD_USE_DBUS_SCANNING
void systemd_mount_free(struct systemd_mount *mount) {
    alloc_free_if_used((void**)&(mount->name));
    alloc_free_if_used((void**)&(mount->system));
    alloc_free_if_used((void**)&(mount->path));
}

void systemd_mount_helper_free (struct systemd_mount_helper **mounts_helper) {
    for (unsigned int i=0; i<(*mounts_helper)->count; ++i) {
        systemd_mount_free(((*mounts_helper)->mounts) + i);
    }
    alloc_free_if_used((void **)&((*mounts_helper)->mounts));
    alloc_free_if_used((void **)mounts_helper);
}

struct systemd_mount_helper *systemd_get_mounts() {
    sd_bus_message *method = NULL;
    if (sd_bus_message_new_method_call(systemd_bus, &method, SYSTEMD_DESTINATION, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "ListUnitsByPatterns") < 0) {
        logging(LOGGING_ERROR, "Failed to initiallize systemd method call");
        return NULL;
    }
    char *active_states[] = {
        "activating",
        "active",
        "reloading",
        "deactivating",
        "inactive",
        NULL
    };
    char *patterns[] = {
        SYSTEMD_MOUNT_PATTERN
    };
    if (sd_bus_message_append_strv(method, active_states) < 0 || sd_bus_message_append_strv(method, patterns) < 0) {
        logging(LOGGING_ERROR, "Failed to append message");
        return NULL;
    }
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    if (sd_bus_call(systemd_bus, method, 0, &error, &reply) < 0) {
        logging(LOGGING_ERROR, "Failed to call systemd bus method, Error: %s (%s)", error.name, error.message);
        return NULL;
    }
    if (sd_bus_message_unref(method)) {
        logging(LOGGING_ERROR, "Failed to clean up systemd bus call method");
    }
    sd_bus_error_free(&error);
    if (sd_bus_message_enter_container(reply, SD_BUS_TYPE_ARRAY, "(ssssssouso)") < 0) {
        logging(LOGGING_ERROR, "Failed to enter returned unit list");
        goto free_reply;
    }
    struct systemd_mount_helper *mounts_helper = NULL;
    int r, len;
    struct systemd_mount *mount, *buffer;
    const char *name = NULL;
    const char *path = NULL;
    while(true) {
        r = sd_bus_message_read(reply, "(ssssssouso)", &name, NULL, NULL, NULL, NULL, NULL, &path, NULL, NULL, NULL);
        if (r < 0) {
            logging(LOGGING_ERROR, "Error encountered when trying to list units");
            goto free_container;
        }
        if (r == 0) {
            break;
        }
        if ((len = strlen(name)) > 255) {
            logging(LOGGING_WARNING, "Systemd unit name too long, ignored it: %s", name);
            continue;
        }
        if (mounts_helper == NULL) {
            if ((mounts_helper = malloc(sizeof(struct systemd_mount_helper))) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts helper");
                goto free_container;
            }
            mounts_helper->mounts = NULL;
            mounts_helper->root = NULL;
            mounts_helper->count = 0;
        }
        if ((mounts_helper->count)++) {
            if ((buffer = realloc(mounts_helper->mounts, sizeof(struct systemd_mount)*(mounts_helper->count))) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts");
                goto free_mount;
            }
            mounts_helper->mounts = buffer;
        } else {
            if ((mounts_helper->mounts = malloc(sizeof(struct systemd_mount))) == NULL) {
                logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts");
                goto free_container;
            }
        }
        mount = mounts_helper->mounts + mounts_helper->count - 1;
        if (!strcmp(name, SYSTEMD_MOUNT_ROOT_UNIT)) {
            if (mounts_helper->root) {
                logging(LOGGING_ERROR, "Two "SYSTEMD_MOUNT_ROOT_UNIT" units found, this should not happen");
                goto free_mount;
            }
            mounts_helper->root = mount;
            // mount->system = NULL;
        }
        if ((mount->name = strdup(name)) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts name");
            goto free_mount;
        }
        // Note, we allow a sub-directory to be mounted here, this is not a bug, since we mount all system one by one, a sub-system will always be mounted after the parent-system. It's safe and nice to have.
        if ((mount->system = systemd_system_from_name(name)) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts system");
            goto free_name;
        }
        len = strlen(path);
        if ((mount->path = strdup(path)) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts");
            goto free_system;
        }
    }
    sd_bus_message_close_container(reply);
    sd_bus_message_unref(reply);
    if (mounts_helper) {
        qsort(mounts_helper->mounts, mounts_helper->count, sizeof(struct systemd_mount), sort_compare_systemd_mount);
        logging(LOGGING_DEBUG, "Sorted %d systemd mount units alphabetically", mounts_helper->count);
    }
    return mounts_helper;

free_system:
    free(mount->system);
free_name:
    free(mount->name);
free_mount:
    --(mounts_helper->count);
    for (unsigned int i=0; i<mounts_helper->count; ++i) {
        free((mounts_helper->mounts+i)->name);
        free((mounts_helper->mounts+i)->system);
        free((mounts_helper->mounts+i)->path);
    }
    free(mounts_helper->mounts);
    free(mounts_helper);
free_container:
    sd_bus_message_exit_container(reply);
free_reply:
    sd_bus_message_unref(reply);
    return NULL;
}

void systemd_to_mount() {
    
}
#endif