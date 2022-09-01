/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#include "eeconfig_p.h"

static inline long eeconfig_read_config(char **buffer) {
    FILE *fp = fopen(EECONFIG_FILE, "r");
    if (fp == NULL) {
        logging(LOGGING_ERROR, "Failed to open config file");
        return -1;
    }
    if (fseek(fp, 0, SEEK_END)) {
        logging(LOGGING_ERROR, "Failed to seek to the end of config file");
        fclose(fp);
        return -1;
    }
    long len_file = ftell(fp);
    if (len_file < 0) {
        logging(LOGGING_ERROR, "Failed to get config file length");
        fclose(fp);
        return len_file;
    }
    if (len_file == 0) {
        logging(LOGGING_ERROR, "Config file empty");
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET)) {
        logging(LOGGING_ERROR, "Failed to seek to the start of config file");
        fclose(fp);
        return 0;
    }
    if (!(*buffer = malloc(sizeof(char)*(len_file+1)))) {
        logging(LOGGING_ERROR, "Failed to allocate memory for buffer");
        fclose(fp);
        return 0;
    }
    if (!fread(*buffer, len_file, 1, fp)) {
        logging(LOGGING_ERROR, "Failed to read config file into buffer");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    (*buffer)[len_file] = '\0';
    return len_file;
}

static inline int eeconfig_report_setting(const enum eeconfig_get_type type, const char *value, const void *result) {
    char *c;
    long buffer;
    switch (type) {
        case EECONFIG_GET_STRING:
            *((char **)result) = value[0] ? strdup(value) : NULL;
            logging(LOGGING_DEBUG, "Got setting string: [%s]", *((char **)result));
            break;
        case EECONFIG_GET_LONG:
        case EECONFIG_GET_INT:
            buffer = strtol(value, &c, 10);
            if (type == EECONFIG_GET_LONG) {
                *((long *)result) = buffer;
                logging(LOGGING_DEBUG, "Got setting long: [%ld]", buffer);
            } else {
                *((int *)result) = util_int_from_long(buffer);
                logging(LOGGING_DEBUG, "Got setting int: [%d]", *((int *)result));
            }
            break;
        case EECONFIG_GET_BOOL:
            *((bool *)result) = value[0] ? util_bool_from_string(value) : false;
            logging(LOGGING_DEBUG, "Got setting boolean: [%s]", *((bool *)result) ? "true" : "false");
            break;
    }
    return 0;
}

static inline int eeconfig_get_setting_sanity_check(char *buffer, const long buffersz, const char *setting, const char *platform, const char *rom, const void *result) {
    // Sanity check
    if (!buffer || buffersz <= 0) {
        logging(LOGGING_ERROR, "Illegal buffer to get config from");
        return 1;
    }
    if (!setting) {
        logging(LOGGING_ERROR, "No setting to get is set");
        return 2;
    }
    if (setting[0] == '\0') {
        logging(LOGGING_ERROR, "Trying to get an empty setting");
        return 3;
    }
    if (rom) {
        if (rom[0] == '\0') {
            logging(LOGGING_ERROR, "Trying to get a setting with empty rom");
            return 4;
        }
        if (!platform) {
            logging(LOGGING_ERROR, "Trying to get a per-rom setting when platform is not given");
            return 5;
        }
    }
    if (platform && platform[0] == '\0') {
        logging(LOGGING_ERROR, "Trying to get a setting with empty platform");
        return 6;
    }
    if (!result) {
        logging(LOGGING_ERROR, "No return pointer given for config read");
        return 7;
    }
    return 0;
}

static inline int eeconfig_get_setting_from_buffer(char *buffer, const long buffersz, const char *setting, const char *platform, const char *rom, const enum eeconfig_get_type type, const void *result) {
    if (eeconfig_get_setting_sanity_check(buffer, buffersz, setting, platform, rom, result)) {
        return 1;
    }
    const size_t len_setting = strlen(setting);
    const size_t len_global_key = len_setting + len_eeconfig_global_prefix + 1;
    const size_t offset_global_value = len_global_key + 1;
    const size_t offset_raw_value = len_setting + 1;
    size_t len_platform, len_platform_key, offset_platform_value;
    if (platform) {
        len_platform = strlen(platform);
        len_platform_key = len_platform + len_setting + 1;
        offset_platform_value = len_platform_key + 1;
    } else {
        len_platform = 0;
        len_platform_key = 0;
        offset_platform_value = 0;
    }
    size_t len_rom, len_rom_key, offset_rom_value, offset_rom_lquote, offset_rom_rquote, offset_rom_rsquare, offset_rom_sep, offset_rom_rom, offset_rom_setting;
    if (rom) {
        len_rom = strlen(rom);
        len_rom_key = len_platform_key + len_rom + 4;
        offset_rom_value = len_rom_key + 1;
        offset_rom_lquote = len_platform + 1;
        offset_rom_rom = offset_rom_lquote + 1;
        offset_rom_rquote = offset_rom_rom + len_rom;
        offset_rom_rsquare = offset_rom_rquote + 1;
        offset_rom_sep = offset_rom_rsquare + 1;
        offset_rom_setting = offset_rom_sep + 1;
    } else {
        len_rom = 0;
        len_rom_key = 0;
        offset_rom_value = 0;
        offset_rom_lquote = 0;
        offset_rom_rom = 0;
        offset_rom_rquote = 0;
        offset_rom_rsquare = 0;
        offset_rom_sep = 0;
        offset_rom_setting = 0;
    }
    size_t len_line;
    char *line;
    char *result_rom = NULL, *result_platform = NULL, *result_global = NULL;
    bool line_valid;
    bool line_process=false;
    long i;
    char *c;
    for (i=0; i<buffersz; ++i) {
        if (line_process) {
            switch (buffer[i]) {
                case '\0':
                case '\n':
                case '\r':
                    if (line_valid) {
                        for (c = buffer + i - 1; c > line; --c) {
                            if (*c == ' ') {
                                *c = '\0';
                            } else {
                                break;
                            }
                        }
                        buffer[i] = '\0';
                        len_line = buffer + i - line;
                        if (rom && len_line > len_rom_key && line[len_rom_key] == '=' && line[len_platform] == '[' && line[offset_rom_lquote] == '"' && line[offset_rom_rquote] == '"' && line[offset_rom_rsquare] == ']') {
                            switch (line[offset_rom_sep]) {
                                case '-':
                                case '.':
                                    if (!(strncmp(line, platform, len_platform) || strncmp(line + offset_rom_rom, rom, len_rom) || strncmp(line + offset_rom_setting, setting, len_setting))) {
                                        result_rom = line + offset_rom_value;
                                    }
                                    break;
                            }
                        }
                        if (!result_rom) {
                            if (platform && len_line > len_platform_key && line[len_platform_key == '=']) {
                                switch (line[len_platform]) {
                                    case '-':
                                    case '.':
                                        if (!(strncmp(line, platform, len_platform) || strncmp(line+len_platform+1, setting, len_setting))) {
                                            result_platform = line + offset_platform_value;
                                        }
                                        break;
                                }
                            }
                            if (!result_platform) {
                                if (len_line > len_global_key && line[len_global_key] == '=') {
                                    switch (line[len_eeconfig_global_prefix]) {
                                        case '-':
                                        case '.':
                                            if (!(strncmp(line, eeconfig_global_prefix, len_eeconfig_global_prefix) || strncmp(line+offset_eeconfig_global_setting, setting, len_setting))) {
                                                result_global = line + offset_global_value;
                                            }
                                            break;
                                    }
                                }
                                if (!result_global) {
                                    if (len_line > len_setting && line[len_setting] == '=') {
                                        if (!strncmp(line, setting, len_setting)) {
                                            result_global = line + offset_raw_value;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    line_process = false;
                    break;
            }
        } else {
            switch (buffer[i]) {
                case '\0':
                case '\n':
                case '\r':
                    buffer[i] = '\0';
                    break;
                case '#':
                    line_process = true;
                    line_valid = false;
                    break;
                default:
                    line_process = true;
                    line_valid = true;
                    line = buffer + i;
                    break;
            }
        }
    }
    if (result_rom) {
        eeconfig_report_setting(type, result_rom, result);
        return 0;
    }
    if (result_platform) {
        eeconfig_report_setting(type, result_platform, result);
        return 0;
    }
    if (result_global) {
        eeconfig_report_setting(type, result_global, result);
        return 0;
    }
    logging(LOGGING_ERROR, "Setting not found");
    return 1;
}

static inline int eeconfig_get_setting_from_buffer_and_report(char *buffer, const long buffersz, const char *setting, const char *platform, const char *rom, const enum eeconfig_get_type type, const void *result) {
    int rtr = eeconfig_get_setting_from_buffer(buffer, buffersz, setting, platform, rom, type, result);
    if (rtr) {
        logging(LOGGING_ERROR, "Failed to get setting %s for platform %s rom %s", setting, platform, rom);
    } else {
        logging(LOGGING_INFO, "Successfully got setting %s for platform %s rom %s", setting, platform, rom);
    }
    return rtr;
}

int eeconfig_get_setting_one(const char *setting, const char *platform, const char *rom, const enum eeconfig_get_type type, const void *result) {
    char *buffer;
    long buffersz = eeconfig_read_config(&buffer);
    if (buffersz < 0) {
        return 1;
    }
    int rtr = eeconfig_get_setting_from_buffer_and_report(buffer, buffersz, setting, platform, rom, type, result);
    free(buffer);
    return rtr;
}

int eeconfig_get_setting_many(struct eeconfig_get_helper *getter, unsigned int get_count) {
    if (!get_count) {
        logging(LOGGING_ERROR, "No config to get");
        return -1;
    }
    char *buffer;
    long buffersz = eeconfig_read_config(&buffer);
    if (buffersz < 0) {
        return -2;
    }
    int rtr = 0;
    struct eeconfig_get_helper *current;
    for (unsigned int i=0; i<get_count; ++i) {
        current = getter + i;
        rtr += eeconfig_get_setting_from_buffer_and_report(buffer, buffersz, current->setting, current->platform, current->rom, current->type, current->result);
    }
    free(buffer);
    return rtr;
}