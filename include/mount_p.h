#include "mount.h"

#include <string.h>
#include <sys/mount.h>
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
static const char mount_point_roms[] = MOUNT_POINT_ROMS;
static const char mount_point_update[] = MOUNT_POINT_UPDATE;