#if 0
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
#endif