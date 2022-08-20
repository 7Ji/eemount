#include "mount.h"

#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <libmount/libmount.h>
#include <blkid/blkid.h>

#include "logging.h"
#include "sort.h"
#include "util.h"
#include "alloc.h"

#define MOUNT_MOUNTINFO     "/proc/self/mountinfo"
#define MOUNT_STORAGE       "/storage"
#define MOUNT_POINT_ROMS    MOUNT_STORAGE"/roms"
#define MOUNT_POINT_UPDATE  MOUNT_STORAGE"/.update"
#define MOUNT_POINT_PORTS_SCRIPTS   MOUNT_POINT_ROMS"/ports_scripts"
#define MOUNT_EXT_PARENT    "/var/media"
#define MOUNT_POINT_EXT     MOUNT_EXT_PARENT"EEROMS"
#define MOUNT_PORTS_SCRIPTS_NAME    "ports"
#define MOUNT_PORTS_SCRIPTS_FS      "overlay"
#define MOUNT_PORTS_SCRIPTS_OPTIONS "lowerdir=/usr/bin/ports,upperdir=/emuelec/ports,workdir=/storage/.tmp/ports-workdir"
// static const char mount_point_roms[] = MOUNT_POINT_ROMS;
// static const char mount_point_update[] = MOUNT_POINT_UPDATE;
// static const char mount_point_ports_scripts[] = MOUNT_POINT_PORTS_SCRIPTS;
// static const char mount_point_ext[] = MOUNT_POINT_EXT;

static const size_t len_mount_ext_parent = strlen(MOUNT_EXT_PARENT);
static const size_t len_mount_point_roms = strlen(MOUNT_POINT_ROMS);