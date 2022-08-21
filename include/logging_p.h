#include "logging.h"

#include <stdarg.h>
#include <string.h>

#define LOGGING_PREFIX_DEBUG    "DEBUG"
#define LOGGING_PREFIX_INFO     "INFO"
#define LOGGING_PREFIX_WARNING  "WARNING"
#define LOGGING_PREFIX_ERROR    "ERROR"
#define LOGGING_PREFIX_FATAL    "FATAL"
#define LOGGING_PREFIX_MISSING  "MISSING"

static int logging_level = LOGGING_DEBUG;
static const char logging_prefix_debug[] = LOGGING_PREFIX_DEBUG;
static const char logging_prefix_info[] = LOGGING_PREFIX_INFO;
static const char logging_prefix_warning[] = LOGGING_PREFIX_WARNING;
static const char logging_prefix_error[] = LOGGING_PREFIX_ERROR;
static const char logging_prefix_fatal[] = LOGGING_PREFIX_FATAL;
static const char logging_prefix_missing[] = LOGGING_PREFIX_MISSING;