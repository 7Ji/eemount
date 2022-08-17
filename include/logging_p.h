#include "logging.h"

#include <stdarg.h>
#include <string.h>

static int logging_level = LOGGING_DEBUG;
static const char logging_prefix_debug[] = "DEBUG";
static const char logging_prefix_info[] = "INFO";
static const char logging_prefix_warning[] = "WARNING";
static const char logging_prefix_error[] = "ERROR";
static const char logging_prefix_fatal[] = "FATAL";
static const char logging_prefix_missing[] = "MISSING";