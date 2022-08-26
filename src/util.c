/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "util_p.h"

unsigned int util_uint_from_ulong(unsigned long value) {
    if (value > UINT_MAX) {
        logging(LOGGING_WARNING, "Limiting unsigned long %lu to %u as it is too big", value, UINT_MAX);
        return UINT_MAX;
    }
    return value;
}

int util_int_from_long(long value) {
    if (value > INT_MAX) {
        logging(LOGGING_WARNING, "Limiting long %ld to %d as it is too big", value, INT_MAX);
        return INT_MAX;
    } 
    if (value < INT_MIN) {
        logging(LOGGING_WARNING, "Limiting long %ld to %d as it is too small", value, INT_MIN);
        return INT_MIN;
    }
    return value;
}

void util_unesacpe_mountinfo_in_place(char *escaped) {
    size_t diff = 0;
    char *c;
    for (c=escaped; *c; ++c) {
        if ((escaped-c) < 3 && *c == '\\') {
            switch (*(c+1)) {
                case '0':
                    switch (*(c+2)) {
                        case '1':
                            switch (*(c+3)) {
                                case '1':
                                    *(c-diff) = '\t';
                                    c+=3;
                                    diff+=3;
                                    continue;
                                case '2':
                                    *(c-diff) = '\n';
                                    c+=3;
                                    diff+=3;
                                    continue;
                            }
                            break;
                        case '4':
                            if (*(c+3) == '0') {
                                *(c-diff) = ' ';
                                c+=3;
                                diff+=3;
                                continue;
                            }
                            break;
                    }
                    break;
                case '1':
                    if ((*(c+2) == '3') && (*(c+3) == '4')) {
                        *(c-diff) = '\\';
                        diff+=3;
                        c+=3;
                        continue;
                    }
                    break;
            }
        }
        if (diff) {
            *(c-diff) = *(c);
        }
    }
    *(c-diff) = '\0';
}

int util_mkdir(const char *path, mode_t mode) {
    errno = 0;
    if (mkdir(path, mode) == 0) {
        logging(LOGGING_DEBUG, "Successfully created directory '%s'", path);
        return 0;
    }
    if (errno != EEXIST) {
        logging(LOGGING_ERROR, "Failed to create directory '%s' due to system failure", path);
        return 1;
    }
    struct stat stat_path;
    if (stat(path, &stat_path)) {
        logging(LOGGING_ERROR, "Failed to check existing path '%s'", path);
        return 1;
    }
    if (!S_ISDIR(stat_path.st_mode)) {
        logging(LOGGING_ERROR, "Existing path '%s' is not directory", path);
        return 1;
    }
    logging(LOGGING_DEBUG, "Path '%s' exists and is directory, no need to create it", path);
    return 0;
}

int util_mkdir_recursive(const char *path, mode_t mode) {
    char *_path = strdup(path);
    if (_path == NULL) {
        logging(LOGGING_ERROR, "Failed to duplicate path '%s' for mkdir", path);
        return 1;
    }
    for (char *p = _path + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (util_mkdir(_path, mode)) {
                free(_path);
                logging(LOGGING_ERROR, "Failed to mkdir '%s' recursively", path);
                return 1;
            }
            *p = '/';
        }
    }
    if (util_mkdir(_path, mode)) {
        free(_path);
        logging(LOGGING_ERROR, "Failed to mkdir '%s' recursively", path);
        return 1;
    }
    free(_path);
    logging(LOGGING_INFO, "Successfully mkdir '%s' recursively", path);
    return 0;
}