#include "drive.h"

#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#include "alloc.h"
#include "logging.h"
#include "sort.h"
#include "eeconfig.h"
#include "util.h"
#include "paths.h"

#define DRIVE_EECONFIG_DELAY    "ee_load.delay"
#define DRIVE_EECONFIG_RETRY    "ee_mount.retry"
#define DRIVE_EECONFIG_DRIVE    "global.externalmount"