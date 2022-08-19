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

long util_file_get_length(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

long util_file_get_length_and_rollback(FILE *fp) {
    long mark = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, mark, SEEK_SET);
    return length;
}

long util_file_get_length_and_restart(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return length;
}


// A few specific characters in mountinfo path entries (root and mountpoint)
// are escaped using a backslash followed by a character's ascii code in octal.
//
//   space              -- as \040
//   tab (aka \t)       -- as \011
//   newline (aka \n)   -- as \012
//   backslash (aka \\) -- as \134
char *util_unescape_mountinfo(char *escaped) {
    size_t len = strlen(escaped);
    char *raw = malloc((len+1)*sizeof(char));
    if (raw == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate memory when unescaping mountinfo");
        return NULL;
    }
    size_t diff = 0;
    for (size_t i=0; i<len; ++i) {
        if ((i<len-3) && (escaped[i] == '\\')) {
            switch (escaped[i+1]) {
                case '0':
                    switch (escaped[i+2]) {
                        case '1':
                            switch (escaped[i+3]) {
                                case '1':
                                    raw[i-diff] = '\t';
                                    i+=3;
                                    diff+=3;
                                    continue;
                                case '2':
                                    raw[i-diff] = '\n';
                                    diff+=3;
                                    i+=3;
                                    continue;
                            }
                            break;
                        case '4':
                            if (escaped[i+3] == '0') {
                                raw[i-diff] = ' ';
                                diff+=3;
                                i+=3;
                                continue;
                            }
                            break;
                    }
                    break;
                case '1':
                    if ((escaped[i+2] == '3') && (escaped[i+3] == '4')) {
                        raw[i-diff] = '\\';
                        diff+=3;
                        i+=3;
                        continue;
                    }
                    break;
            }
        }
        raw[i-diff] = escaped[i];
    }
    raw[len-diff] = '\0';
    return raw;
}

void util_unesacpe_mountinfo_in_place(char *escaped) {
    size_t len = strlen(escaped);
    size_t diff = 0;
    for (size_t i=0; i<len; ++i) {
        if ((i<len-3) && (escaped[i] == '\\')) {
            switch (escaped[i+1]) {
                case '0':
                    switch (escaped[i+2]) {
                        case '1':
                            switch (escaped[i+3]) {
                                case '1':
                                    escaped[i-diff] = '\t';
                                    i+=3;
                                    diff+=3;
                                    continue;
                                case '2':
                                    escaped[i-diff] = '\n';
                                    diff+=3;
                                    i+=3;
                                    continue;
                            }
                            break;
                        case '4':
                            if (escaped[i+3] == '0') {
                                escaped[i-diff] = ' ';
                                diff+=3;
                                i+=3;
                                continue;
                            }
                            break;
                    }
                    break;
                case '1':
                    if ((escaped[i+2] == '3') && (escaped[i+3] == '4')) {
                        escaped[i-diff] = '\\';
                        diff+=3;
                        i+=3;
                        continue;
                    }
                    break;
            }
        }
        if (diff) {
            escaped[i-diff] = escaped[i];
        }
    }
    escaped[len-diff] = '\0';
}

bool util_mkdir(const char *path, mode_t mode) {
    errno = 0;
    if (mkdir(path, mode) == 0) {
        logging(LOGGING_DEBUG, "Successfully created directory '%s'", path);
        return true;
    }
    if (errno != EEXIST) {
        logging(LOGGING_ERROR, "Failed to create directory '%s' due to system failure", path);
        return false;
    }
    struct stat stat_path;
    if (stat(path, &stat_path)) {
        logging(LOGGING_ERROR, "Failed to check existing path '%s'", path);
        return false;
    }
    if (!S_ISDIR(stat_path.st_mode)) {
        logging(LOGGING_ERROR, "Existing path '%s' is not directory", path);
        return false;
    }
    logging(LOGGING_DEBUG, "Path '%s' exists and is directory, no need to create it", path);
    return true;
}

bool util_mkdir_recursive(const char *path, mode_t mode) {
    char *_path = strdup(path);
    if (_path == NULL) {
        logging(LOGGING_ERROR, "Failed to duplicate path '%s' for mkdir", path);
        return false;
    }
    for (char *p = _path + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (!util_mkdir(_path, mode)) {
                free(_path);
                logging(LOGGING_ERROR, "Failed to mkdir '%s' recursively", path);
                return false;
            }
            *p = '/';
        }
    }
    if (!util_mkdir(_path, mode)) {
        free(_path);
        logging(LOGGING_ERROR, "Failed to mkdir '%s' recursively", path);
        return false;
    }
    free(_path);
    logging(LOGGING_INFO, "Successfully mkdir '%s' recursively", path);
    return true;
}