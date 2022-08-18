#include "systemd.h"

#include <dirent.h>
#include <systemd/sd-bus.h>

#include "logging.h"
#include "alloc.h"
#include "sort.h"

#define SYSTEMD_DESTINATION         "org.freedesktop.systemd1"
#define SYSTEMD_INTERFACE_UNIT      SYSTEMD_DESTINATION".Unit"
#define SYSTEMD_INTERFACE_MANAGER   SYSTEMD_DESTINATION".Manager"
#define SYSTEMD_PATH                "/org/freedesktop/systemd1"
#define SYSTEMD_PATH_UNIT           SYSTEMD_PATH"/unit"
#define SYSTEMD_MOUNT_ROOT          "storage-roms"
#define SYSTEMD_MOUNT_SUFFIX        ".mount"
#define SYSTEMD_MOUNT_PATTERN       SYSTEMD_MOUNT_ROOT"*"SYSTEMD_MOUNT_SUFFIX
#define SYSTEMD_MOUNT_ROOT_UNIT     SYSTEMD_MOUNT_ROOT SYSTEMD_MOUNT_SUFFIX
#define SYSTEMD_UNIT_DIR            "/storage/.config/system.d"

static const size_t len_systemd_mount_root = strlen(SYSTEMD_MOUNT_ROOT);
static const size_t len_systemd_suffix = strlen(SYSTEMD_MOUNT_SUFFIX);
static sd_bus *systemd_bus = NULL;