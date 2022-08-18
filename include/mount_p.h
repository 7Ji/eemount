#include "mount.h"

#include <string.h>
#include <sys/mount.h>

#include "logging.h"
#include "sort.h"
#include "util.h"
#include "alloc.h"

#define MOUNT_MOUNTINFO     "/proc/self/mountinfo"
#define MOUNT_STORAGE       "/storage"
static const char mount_point_roms[] = MOUNT_STORAGE"/roms";
static const char mount_point_update[] = MOUNT_STORAGE"/.update";