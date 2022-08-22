/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "logging_p.h"

static inline FILE *logging_get_target(const int level) {
    switch(level) {
        case LOGGING_FATAL:
        case LOGGING_ERROR:
        case LOGGING_WARNING:
            return stderr;
        default:
            return stdout;
    }
}

int logging(const int level, const char *format, ...) {
    if (level > logging_level) {
        return 1;
    }
    FILE *target = logging_get_target(level);
    switch(level) {
        case LOGGING_DEBUG:
            fprintf(target, "[%s] ", logging_prefix_debug);
            break;
        case LOGGING_INFO:
            fprintf(target, "[%s] ", logging_prefix_info);
            break;
        case LOGGING_WARNING:
            fprintf(target, "[%s] ", logging_prefix_warning);
            break;
        case LOGGING_ERROR:
            fprintf(target, "[%s] ", logging_prefix_error);
            break;
        case LOGGING_FATAL:
            fprintf(target, "[%s] ", logging_prefix_fatal);
            break;
        default:
            fprintf(target, "[%s] ", logging_prefix_missing);
            break;
    }
    va_list vargs;
    va_start(vargs, format);
    vfprintf(target, format, vargs);
    va_end(vargs);
    putc('\n', target);
    return 0;
}

int logging_set_level(int level) {
    if ((level >= LOGGING_DISABLED) && (level <= LOGGING_DEBUG)) {
        logging_level = level;
        return 0;
    }
    return 1;
}