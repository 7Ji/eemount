#if 0
bool systemd_encode_path(char *unit, char **path) {
    if (sd_bus_path_encode(SYSTEMD_PATH_UNIT, unit, path) < 0) {
        fprintf(stderr, "Failed to encode unit to sd-bus path\n");
        return false;
    } else {
        return true;
    }
}

bool systemd_is_active(char *path) {
    if (systemd_bus == NULL) {
        return false;
    }
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

struct eemount_finished_helper *systemd_start_unit_systems(struct systemd_mount_unit_helper *shelper) {
jobs_finished:
    struct mount_system_simple *system_head = NULL, *system = NULL, *next, *buffer;
    bool insert;
    for (job_array_id = 0; job_array_id<jobs_count; ++job_array_id) {
        job = jobs+job_array_id;
        if (!(job->success)) {
            continue;
        }
        if ((buffer = malloc(sizeof(struct mount_system_simple))) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for simple mount system chain table");
            return system_head;
        }
        buffer->system = job->system;
        if (system_head == NULL) {
            buffer->next = NULL;
            system_head = buffer;
        } else {
            system = system_head;
            insert = false;
            while ((next = system->next)) {
                if (strcmp(job->system, system->system)>0 && strcmp(job->system, next->system)<0) {
                    // buffer->system = job->system;
                    buffer->next = next;
                    system->next = buffer;
                    insert = true;
                    break;
                }
                system = system->next;
            }
            if (!insert) {
                buffer->next = NULL;
                system->next = buffer;
            }
        }
    }
    return system_head;
}

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