#include "systemd.h"
#include "logging.h"
#include "alloc.h"

#define SYSTEMD_DESTINATION         "org.freedesktop.systemd1"
#define SYSTEMD_INTERFACE_UNIT      SYSTEMD_DESTINATION".Unit"
#define SYSTEMD_INTERFACE_MANAGER   SYSTEMD_DESTINATION".Manager"
#define SYSTEMD_PATH                "/org/freedesktop/systemd1"
#define SYSTEMD_PATH_UNIT           SYSTEMD_PATH"/unit"
#define SYSTEMD_MOUNT_ROOT          "storage-roms"
#define SYSTEMD_MOUNT_SUFFIX        ".mount"
#define SYSTEMD_MOUNT_PATTERN       SYSTEMD_MOUNT_ROOT"*"SYSTEMD_MOUNT_SUFFIX
#define SYSTEMD_MOUNT_ROOT_UNIT     SYSTEMD_MOUNT_ROOT SYSTEMD_MOUNT_SUFFIX


/**
 The systemd bus we need to work on, you need to initialize it first before actual using it
*/
static sd_bus *systemd_bus;

bool systemd_encode_path(char *unit, char **path) {
    if (sd_bus_path_encode(SYSTEMD_PATH_UNIT, unit, path) < 0) {
        fprintf(stderr, "Failed to encode unit to sd-bus path\n");
        return false;
    } else {
        return true;
    }
}

bool systemd_init_bus() {
    if(sd_bus_default_system(&systemd_bus) < 0) {
        fprintf(stderr, "Failed to open systemd bus\n");
        return false;
    } else {
        return true;
    }
}

void systemd_release() {
    sd_bus_flush_close_unref(systemd_bus);
}

bool systemd_is_active(char *path) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    char *msg = NULL;

    if(sd_bus_get_property_string(systemd_bus, SYSTEMD_DESTINATION, path, SYSTEMD_INTERFACE_UNIT, "ActiveState", &err, &msg) < 0) {
        fprintf(stderr, "Failed to get status \n - Message: %s \n Error: %s \n", msg, err.message);
        return false;
    }
    if (strcmp(msg, "active")) {
        return false;
    } else {
        return true;
    }
}

struct systemd_mount_helper *systemd_list_service() {
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
        if (++(mounts_helper->count) > 1) {
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
        }
        if ((mount->name = malloc((len+1)*sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts");
            goto free_mount;
        }
        strcpy(mount->name, name);
        len = strlen(path);
        if ((mount->path = malloc((len+1)*sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Failed to allocate memory for systemd mounts");
            goto free_name;
        }
        strcpy(mount->path, path);
    }
    sd_bus_message_close_container(reply);
    sd_bus_message_unref(reply);
    return mounts_helper;

free_name:
    free(mount->name);
free_mount:
    --(mounts_helper->count);
    for (unsigned int i=0; i<mounts_helper->count; ++i) {
        free((mounts_helper->mounts+i)->name);
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
// bool systemd_start_unit() {
//     sd_bus_call_method(systemd_bus, SYSTEMD_DESTINATION, )
// }