/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "util.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "logging.h"

static char *util_bool_true[]  = {
    "yes",
    "true",
    "y",
    "t"
};

static char *util_bool_false[] = {
    "no",
    "false",
    "n",
    "f"
};