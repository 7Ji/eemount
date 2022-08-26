/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "logging.h"

#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

#define LOGGING_PREFIX_DEBUG    "DEBUG"
#define LOGGING_PREFIX_INFO     "INFO"
#define LOGGING_PREFIX_WARNING  "WARNING"
#define LOGGING_PREFIX_ERROR    "ERROR"
#define LOGGING_PREFIX_FATAL    "FATAL"
#define LOGGING_PREFIX_MISSING  "MISSING"

static int logging_level = LOGGING_DEBUG;
static const char logging_prefix_debug[] = LOGGING_PREFIX_DEBUG;
static const char logging_prefix_info[] = LOGGING_PREFIX_INFO;
static const char logging_prefix_warning[] = LOGGING_PREFIX_WARNING;
static const char logging_prefix_error[] = LOGGING_PREFIX_ERROR;
static const char logging_prefix_fatal[] = LOGGING_PREFIX_FATAL;
static const char logging_prefix_missing[] = LOGGING_PREFIX_MISSING;

struct timeval logging_time_begin, logging_time_current;