#if 0

int eeconfig_initialize() {
    if ((eeconfig = fopen(EECONFIG_FILE, "r")) == NULL) {
        logging(LOGGING_ERROR, "Failed to open emuelec config file: '"EECONFIG_FILE"'");
        return 1;
    }
    return 0;
}

void eeconfig_close() {
    if (eeconfig) {
        fclose(eeconfig);
        eeconfig = NULL;
    } else {
        logging(LOGGING_WARNING, "EE config not initialized yet, ignored close request");
    }
}

char *eeconfig_get_string(const char *key) {
    if (eeconfig == NULL) {
        logging(LOGGING_WARNING, "Config '%s' defaulting to NULL since we failed to initialize eeconfig", key);
        return NULL;
    }
    char *line;
    size_t size_line = 0;
    size_t len_line;
    size_t len_key = strlen(key);
    size_t len_value;
    size_t len_alloc = 0;
    char quote;
    char *value = NULL;
    char *buffer;
    char *line_value = NULL;
    fseek(eeconfig, 0, SEEK_SET);
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
            if (len_alloc == 0) {
                if ((value = malloc(sizeof(char))) == NULL) {
                    logging(LOGGING_ERROR, "Can not allocate memory for configuration value when reading config key '%s'", key);
                    return NULL;    
                }
                len_alloc = 1;
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
                line[len_line] = '\0'; // Just for printing's sake
                logging(LOGGING_WARNING, "Single quote and double quote not come in pair in emuelec config, please check you config file. Ignored Problematic line:\n%s", line);
                continue;
            }
            len_value = len_line - len_key - 3;
            line_value = line + len_key + 2;
        } else {
            len_value = len_line - len_key - 1;
            line_value = line + len_key + 1;
        }
        if (len_value + 1 > len_alloc) {
            if (len_alloc) {
                buffer = realloc(value, (len_value+1)*sizeof(char));
            } else {
                buffer = malloc((len_value+1)*sizeof(char));
            }
            if (buffer == NULL) {
                if (len_alloc) {
                    free(value);
                }
                logging(LOGGING_ERROR, "Can not allocate memory for configuration value when reading config key '%s'", key);
                return NULL;
            }
            value = buffer;
            len_alloc = len_value + 1;
        }
        strncpy(value, line_value, len_value);
        value[len_value] = '\0';
    }
    logging(LOGGING_DEBUG, "Read eeconfig '%s' value string '%s'", key, value);
    return value;
}

int eeconfig_get_int(const char *key) {
    if (eeconfig == NULL) {
        logging(LOGGING_WARNING, "Config '%s' defaulting to 0 since we failed to initialize eeconfig", key);
        return 0;
    }
    char *value = eeconfig_get_string(key);
    if (value == NULL) {
        logging(LOGGING_WARNING, "Configuration '%s' not found, defaulting to 0", key);
        return 0;
    }
    if (value[0] == '\0') {
        logging(LOGGING_WARNING, "Configuration '%s' is empty, defaulting to 0", key);
        return 0;
    }
    char *ptr;
    long long_value = strtol(value, &ptr, 10);
    logging(LOGGING_DEBUG, "Converting eeconfig option '%s' value string '%s' to integer %ld", key, value, long_value);
    free(value);
    return util_int_from_long(long_value);
}

bool eeconfig_get_bool(const char *key, const bool bool_default) {
    if (eeconfig == NULL) {
        logging(LOGGING_WARNING, "Config '%s' defaulting to false since we failed to initialize eeconfig", key);
        return false;
    }
    char *value = eeconfig_get_string(key);
    if (value == NULL) {
        logging(LOGGING_WARNING, "Configuration '%s' not found, using default value '%d'", key, bool_default);
        return bool_default;
    }
    if (value[0] == '\0') {
        logging(LOGGING_WARNING, "Configuration '%s' is empty, using default value '%d'", key, bool_default);
        return bool_default;
    }
    int i;
    puts(value);
    for (i=0; i<5; ++i) {
        if (!strcasecmp(value, eeconfig_bool_true[i])) {
            free(value);
            return true;
        }
        if (!strcasecmp(value, eeconfig_bool_false[i])) {
            free(value);
            return false;
        }
    }
    char *ptr;
    long long_value = strtol(value, &ptr, 10);
    free(value);
    if (long_value > 0) {
        return true;
    }
    if (long_value < 0) {
        return false;
    }
    logging(LOGGING_WARNING, "Configuration '%s' set but is none of the possible bool value: True (yes, true, y, t, or any positive number including 1) False (no, false, n, f, 0, or any negative number)", key);
    if (bool_default) {
        logging(LOGGING_WARNING, "Defaulting configuration '%s' to true", key);
    } else {
        logging(LOGGING_WARNING, "Defaulting configuration '%s' to false", key);
    }
    return bool_default;
}
#endif