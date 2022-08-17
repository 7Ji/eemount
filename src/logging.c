#include "logging_p.h"

static FILE *logging_get_target(const int level) {
    switch(level) {
        case LOGGING_FATAL:
        case LOGGING_ERROR:
        case LOGGING_WARNING:
            return stderr;
        default:
            return stdout;
    }
}

void logging(const int level, const char *format, ...) {
    if (level > logging_level) {
        return;
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
}

void logging_set_level(int level) {
    if ((level >= LOGGING_DISABLED) && (level <= LOGGING_DEBUG)) {
        logging_level = level;
    }
}