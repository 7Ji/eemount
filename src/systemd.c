#include "systemd.h"
#include "logging.h"

#define SYSTEMD_DESTINATION         "org.freedesktop.systemd1"
#define SYSTEMD_INTERFACE_UNIT      SYSTEMD_DESTINATION".Unit"
#define SYSTEMD_INTERFACE_MANAGER   SYSTEMD_DESTINATION".Manager"
#define SYSTEMD_PATH                "/org/freedesktop/systemd1"
#define SYSTEMD_PATH_UNIT           SYSTEMD_PATH"/unit"

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

char **systemd_list_service() {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *rep = NULL;
    // char *msg = NULL;
    if (sd_bus_call_method(systemd_bus, SYSTEMD_DESTINATION, SYSTEMD_PATH, SYSTEMD_INTERFACE_MANAGER, "ListUnits", &err, &rep, NULL) < 0) {
        logging(LOGGING_ERROR, "Failed to list units: %s", err.message);
        return NULL;
    }
    if (sd_bus_message_enter_container(rep, SD_BUS_TYPE_ARRAY, "(ssssssouso)") < 0) {
        logging(LOGGING_ERROR, "Failed to enter returned unit list");
        return NULL;
    }
    int r;
    for (;;) {
        const char *s1 = NULL;
        const char *s2 = NULL;
        const char *s3 = NULL;
        const char *s4 = NULL;
        const char *s5 = NULL;
        const char *s6 = NULL;
        const char *s7 = NULL;
        const uint32_t u8 = 0;
        const char *s9 = NULL;
        const char *s10 = NULL;
        r = sd_bus_message_read(rep, "(ssssssouso)", &s1, &s2, &s3, &s4, &s5, &s6, &s7, &u8, &s9, &s10);
        if (r<0) {
            logging(LOGGING_ERROR, "Error encountered when trying to list units");
            return NULL;
        }
        if (r==0) {
            break;
        }
        puts("----------------------------------------");
        printf("Service name: %s\nDescription: %s\nLoadState: %s\nActiveState: %s\nSubState: %s\nFollowing: %s\nObjectPath: %s\nQueued job ID (0 for none): %"PRIu32"\nJobType: %s\nJob Object Path:%s\n", s1, s2, s3, s4, s5, s6, s7, u8, s9, s10);
        puts("----------------------------------------");
    }
    return NULL;
}
// bool systemd_start_unit() {
//     sd_bus_call_method(systemd_bus, SYSTEMD_DESTINATION, )
// }