#include "eeconfig.h"
#include "logging.h"

#define EECONFIG_DIR       "/storage/.config/emuelec/configs"
// #define EECONFIG_FILE      EECONFIG_DIR "/emuelec.conf"
#define EECONFIG_FILE "/tmp/emuelec.conf"

static FILE *eeconfig = NULL;

const bool eeconfig_initialize() {
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
    char quote;
    char *value;
    while (getline(&line, &size_line, eeconfig) != -1) {
        if (strncmp(line, key, strlen(key))) {
            continue;
        }
        len_line = strcspn(line, "\r\n");
        if (len_line <= len_key) {
            continue;
        }
        if (len_line == len_key + 1) {
            if (line[len_key] == '=') {
                value = malloc(sizeof(char));
                value[0] = '\0';
                return value;
            } else {
                continue;
            }
        }
        if (line[len_key] != '=') {
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
        if (quote != '\0') {
            if (line[len_line-1] != quote) {
                logging(LOGGING_WARNING, "Single quote and double quote not come in pair in emuelec config, please check you config file. Ignored Problematic line:\n%s", line);
                continue;
            }
        }

        puts("config found");
    }
    return NULL;
}

bool eeconfig_read_bool() {

}

char *eeconfig_read(char *config, int *type) {

}