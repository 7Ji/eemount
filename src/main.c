#include "systemd.h"
#include "eeconfig.h"
#include "logging.h"
#include "mount.h"
int main() {
    if (systemd_init_bus()) {
        logging(LOGGING_FATAL, "Failed to initialize systemd bus");
        return 1;
    }
    if (eeconfig_initialize()) {
        logging(LOGGING_WARNING, "Failed to initialize eeconfig, all config values will be defaulted");
    }
    if (eemount_routine()) {
        logging(LOGGING_ERROR, "Mount routine failed");
    } else {
        logging(LOGGING_INFO, "All mount successful, good retro-gaming");
    }
    systemd_release();
    eeconfig_close();
    return 0;
}