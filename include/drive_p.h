/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "drive.h"

#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#include "alloc.h"
#include "logging.h"
#include "sort.h"
#include "eeconfig.h"
#include "util.h"
#include "paths.h"

#define DRIVE_EECONFIG_DELAY    "ee_load.delay"
#define DRIVE_EECONFIG_RETRY    "ee_mount.retry"
#define DRIVE_EECONFIG_DRIVE    "externalmount"

static const size_t len_drive_path_mark_anonymous = strlen(PATH_DIR_EXTERNAL"//"PATH_NAME_ROMS"/"PATH_NAME_MARK);