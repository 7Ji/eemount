#include "eemount.h"

#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <libmount/libmount.h>
#include <blkid/blkid.h>

#include "logging.h"
#include "sort.h"
#include "util.h"
#include "alloc.h"
#include "drive.h"
#include "paths.h"

#define EEMOUNT_MOUNTINFO               "/proc/self/mountinfo"
#define EEMOUNT_PORTS_SCRIPTS_FS        "overlay"
#define EEMOUNT_PORTS_SCRIPTS_OPTIONS   "lowerdir="PATH_DIR_BIN_PORTS",upperdir="PATH_DIR_EMUELEC_PORTS",workdir="PATH_DIR_PSCRIPTS_WORKDIR