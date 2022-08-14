#include "systemd.h"

#define SYSTEMD_DESTINATION     "org.freedesktop.systemd1"
#define SYSTEMD_INTERFACE       SYSTEMD_DESTINATION".Unit"
#define SYSTEMD_PATH_PARENT     "/org/freedesktop/systemd1/unit"

/**
 The systemd bus we need to work on, you need to initialize it first before actual using it
*/
static sd_bus *systemd_bus;

bool systemd_encode_path(char *unit, char **path) {
    if (sd_bus_path_encode(SYSTEMD_PATH_PARENT, unit, path) < 0) {
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

    if(sd_bus_get_property_string(systemd_bus, SYSTEMD_DESTINATION, path, SYSTEMD_INTERFACE, "ActiveState", &err, &msg) < 0) {
        fprintf(stderr, "Failed to get status \n - Message: %s \n Error: %s \n", msg, err.message);
        return false;
    }
    if (strcmp(msg, "active")) {
        return false;
    } else {
        return true;
    }
}
