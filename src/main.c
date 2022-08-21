#include "systemd.h"
#include "eeconfig.h"
#include "logging.h"
#include "eemount.h"
int main() {
    if (systemd_init_bus()) {
        logging(LOGGING_FATAL, "Failed to initialize systemd bus");
        return 1;
    }
    systemd_stop_unit("smbd.service");
    if (eeconfig_initialize()) {
        logging(LOGGING_WARNING, "Failed to initialize eeconfig, all config values will be defaulted");
    }
    if (eemount_routine()) {
        logging(LOGGING_ERROR, "Mount routine failed");
    } else {
        logging(LOGGING_INFO, "All mount successful, happy retro-gaming");
    }
    systemd_start_unit("smbd.service");
    systemd_release();
    eeconfig_close();
    return 0;
}