#include "eeconfig.h"

#include <string.h>
#include <limits.h>

#include "logging.h"
#include "alloc.h"

#define EECONFIG_DIR       "/storage/.config/emuelec/configs"
#define EECONFIG_FILE      EECONFIG_DIR "/emuelec.conf"

static FILE *eeconfig = NULL;

static char *eeconfig_bool_true[]  = {
    "yes",
    "true",
    "y",
    "t",
    "1"
};

static char *eeconfig_bool_false[] = {
    "no",
    "false",
    "n",
    "f",
    "0"
};