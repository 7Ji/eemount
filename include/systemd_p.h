/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "systemd.h"

#include <dirent.h>
#include <systemd/sd-bus.h>

#include "logging.h"
#include "alloc.h"
#include "sort.h"
#include "util.h"
#include "eemount.h"
#include "paths.h"

#define SYSTEMD_NAME_ORG            "org"
#define SYSTEMD_NAME_FREEDESKTOP    "freedesktop"
#define SYSTEMD_NAME_SYSTEMD        "systemd1"
#define SYSTEMD_NAME_UNIT           "Unit"
#define SYSTEMD_NAME_MANAGER        "Manager"
#define SYSTEMD_NAME_UNIT_LOWER     "unit"
#define SYSTEMD_NAME_JOB            "job"
#define SYSTEMD_NAME                SYSTEMD_NAME_ORG"."SYSTEMD_NAME_FREEDESKTOP"."SYSTEMD_NAME_SYSTEMD
#define SYSTEMD_INTERFACE           SYSTEMD_NAME
#define SYSTEMD_INTERFACE_UNIT      SYSTEMD_NAME"."SYSTEMD_NAME_UNIT
#define SYSTEMD_INTERFACE_MANAGER   SYSTEMD_NAME"."SYSTEMD_NAME_MANAGER
#define SYSTEMD_PATH                "/"SYSTEMD_NAME_ORG"/"SYSTEMD_NAME_FREEDESKTOP"/"SYSTEMD_NAME_SYSTEMD""
#define SYSTEMD_PATH_UNIT           SYSTEMD_PATH"/"SYSTEMD_NAME_UNIT_LOWER
#define SYSTEMD_PATH_JOB            SYSTEMD_PATH"/"SYSTEMD_NAME_JOB
#define SYSTEMD_MOUNT_ROOT          PATH_NAME_STORAGE"-"PATH_NAME_ROMS
#define SYSTEMD_MOUNT_SUFFIX        ".mount"
#define SYSTEMD_MOUNT_PATTERN       SYSTEMD_MOUNT_ROOT"*"SYSTEMD_MOUNT_SUFFIX
#define SYSTEMD_MOUNT_ROOT_UNIT     SYSTEMD_MOUNT_ROOT SYSTEMD_MOUNT_SUFFIX
#define SYSTEMD_UNIT_DIR            PATH_DIR_STORAGE"/.config/system.d"
#define SYSTEMD_START_TIMEOUT       10

#define SYSTEMD_NAME_START          "Start"
#define SYSTEMD_NAME_STOP           "Stop"

#define SYSTEMD_METHOD_START_UNIT   SYSTEMD_NAME_START SYSTEMD_NAME_UNIT
#define SYSTEMD_METHOD_STOP_UNIT    SYSTEMD_NAME_STOP SYSTEMD_NAME_UNIT

static const size_t len_systemd_unit_dir = strlen(SYSTEMD_UNIT_DIR);
static const size_t len_systemd_mount_root = strlen(SYSTEMD_MOUNT_ROOT);
static const size_t len_systemd_suffix = strlen(SYSTEMD_MOUNT_SUFFIX);
static const size_t len_systemd_job_prefix = strlen(SYSTEMD_PATH_JOB) + 1; // 1 for the extra /
static sd_bus *systemd_bus = NULL;

#define _cleanup_(x) __attribute__((__cleanup__(x)))