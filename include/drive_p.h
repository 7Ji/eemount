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

#define MOUNT_EXT_PARENT        "/var/media"
#define MOUNT_EXT_ROMS_PARENT   "roms"
#define MOUNT_EXT_MARK          "emuelecroms"

#define MOUNT_EECONFIG_DELAY    "ee_load.delay"
#define MOUNT_EECONFIG_RETRY    "ee_mount.retry"
#define MOUNT_EECONFIG_DRIVE    "global.externalmount"