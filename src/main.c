/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "systemd.h"
#include "eeconfig.h"
#include "logging.h"
#include "eemount.h"

#include <string.h>

int main(int argc, char **argv) {
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
    if (argc > 1 && !strcmp(argv[1], "--esrestart")) {
        logging(LOGGING_INFO, "Restarting EmulationStation per request");
        systemd_restart_unit("emustation.service");
    }
    systemd_release();
    eeconfig_close();
    return 0;
}