#ifndef HAVE_LOGGING_H
#define HAVE_LOGGING_H
#include "common.h"


#include <stdarg.h>
#include <string.h>

enum logging_levels {
    LOGGING_DEBUG,
    LOGGING_INFO,
    LOGGING_WARNING,
    LOGGING_ERROR,
    LOGGING_FATAL
};

void logging(const int level, const char *format, ...);
#endif