#include "eeconfig.h"
#include "logging.h"
#include "alloc.h"

#define EECONFIG_DIR       "/storage/.config/emuelec/configs"
// #define EECONFIG_FILE      EECONFIG_DIR "/emuelec.conf"
#define EECONFIG_FILE "/tmp/emuelec.conf"

static FILE *eeconfig = NULL;

bool eeconfig_initialize() {
    if ((eeconfig = fopen(EECONFIG_FILE, "r")) == NULL) {
        logging(LOGGING_ERROR, "Failed to open emuelec config file: '"EECONFIG_FILE"'");
        return false;
    }
    return true;
}

void eeconfig_close() {
    if (eeconfig) {
        fclose(eeconfig);
        eeconfig = NULL;
    }
}

const char *eeconfig_get_string(char *key) {
    char *line;
    size_t size_line = 0;
    size_t len_line;
    size_t len_key = strlen(key);
    size_t len_value;
    char quote;
    char *value = NULL;
    char *line_value = NULL;
    while (getline(&line, &size_line, eeconfig) != -1) {
        switch (line[0]) {
            case '\0': // empty line
            case '#':  // commen line
                continue;
        }
        if (strncmp(line, key, len_key)) {
            continue;
        }
        len_line = strcspn(line, "\r\n");
        if (len_line <= len_key) {
            continue;
        }
        if (line[len_key] != '=') {
            continue;
        }
        if (len_line == len_key + 1) {
            if ((value = alloc_optional_resize(value, sizeof(char))) == NULL) {
                logging(LOGGING_ERROR, "Can not allocate memory for configuration value when reading config key '%s'", key);
                return NULL;
            }
            value[0] = '\0';
            continue;
        }
        switch (line[len_key+1]) {
            case '\'':
                quote = '\'';
                break;
            case '"':
                quote = '"';
                break;
            default:
                quote = '\0';
                break;
        }
        if (quote) {
            if (line[len_line-1] != quote) { // ' and " must come in pair
                logging(LOGGING_WARNING, "Single quote and double quote not come in pair in emuelec config, please check you config file. Ignored Problematic line:\n%s", line);
                continue;
            }
            len_value = len_line - len_key - 3;
            line_value = line + len_key + 2;
        } else {
            len_value = len_line - len_key - 1;
            line_value = line + len_key + 1;
        }
        if ((value = alloc_optional_resize(value, (len_value + 1)*sizeof(char))) == NULL) {
            logging(LOGGING_ERROR, "Can not allocate memory for configuration value when reading config key '%s'", key);
            return NULL;
        }
        strncpy(value, line_value, len_value);
    }
    return value;
}

// bool eeconfig_read_bool() {

// }

// char *eeconfig_read(char *config, int *type) {

// }