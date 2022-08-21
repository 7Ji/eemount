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
#define EEMOUNT_PORTS_SCRIPTS_NAME      "ports"
#define EEMOUNT_PORTS_SCRIPTS_FS        "overlay"
#define EEMOUNT_PORTS_SCRIPTS_OPTIONS   "lowerdir=/usr/bin/ports,upperdir=/emuelec/ports,workdir=/storage/.tmp/ports-workdir"
