#include "mount.h"

#include <string.h>
#include <sys/mount.h>

#include "logging.h"
#include "sort.h"
#include "util.h"

#define MOUNT_MOUNTINFO     "/proc/self/mountinfo"