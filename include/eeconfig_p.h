/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "eeconfig.h"

#include <string.h>
#include <limits.h>

#include "logging.h"
#include "alloc.h"
#include "util.h"

#define EECONFIG_DIR       "/storage/.config/emuelec/configs"
#define EECONFIG_FILE      EECONFIG_DIR "/emuelec.conf"

#define EECONFIG_GLOBAL_PREFIX  "global"
static const char eeconfig_global_prefix[] = EECONFIG_GLOBAL_PREFIX;
static const size_t len_eeconfig_global_prefix = strlen(EECONFIG_GLOBAL_PREFIX);
static const size_t offset_eeconfig_global_setting = len_eeconfig_global_prefix + 1;