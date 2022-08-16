#include "logging_p.h"

void logging(const int level, const char *format, ...) {
    if (level < logging_level) {
        return;
    }
    FILE *target;
    const char *prefix;
    switch(level) {
        case LOGGING_DEBUG:
            prefix = logging_prefix_debug;
            target = stdout;
            break;
        case LOGGING_INFO:
            prefix = logging_prefix_info;
            target = stdout;
            break;
        case LOGGING_WARNING:
            prefix = logging_prefix_warning;
            target = stderr;
            break;
        case LOGGING_ERROR:
            prefix = logging_prefix_error;
            target = stderr;
            break;
        case LOGGING_FATAL:
            prefix = logging_prefix_fatal;
            target = stderr;
            break;
        default:
            prefix = logging_prefix_missing;
            target = stdout;
            break;
    }
    fprintf(target, "[%s] ", prefix);
    va_list vargs;
    va_start(vargs, format);
    vfprintf(target, format, vargs);
    va_end(vargs);
    putc('\n', target);
}