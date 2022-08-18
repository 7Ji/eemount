#include "mount.h"

#include <string.h>
#include <sys/mount.h>

#include "logging.h"
#include "sort.h"
#include "util.h"
#include "alloc.h"

#define MOUNT_MOUNTINFO     "/proc/self/mountinfo"
static const char mount_point_roms[] = "/storage/roms";