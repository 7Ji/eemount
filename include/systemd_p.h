#include "systemd.h"

#include <dirent.h>
#include <systemd/sd-bus.h>

#include "logging.h"
#include "alloc.h"
#include "sort.h"
#include "util.h"

#define SYSTEMD_DESTINATION         "org.freedesktop.systemd1"
#define SYSTEMD_INTERFACE_UNIT      SYSTEMD_DESTINATION".Unit"
#define SYSTEMD_INTERFACE_MANAGER   SYSTEMD_DESTINATION".Manager"
#define SYSTEMD_PATH                "/org/freedesktop/systemd1"
#define SYSTEMD_PATH_UNIT           SYSTEMD_PATH"/unit"
#define SYSTEMD_PATH_JOB            SYSTEMD_PATH"/job"
#define SYSTEMD_MOUNT_ROOT          "storage-roms"
#define SYSTEMD_MOUNT_SUFFIX        ".mount"
#define SYSTEMD_MOUNT_PATTERN       SYSTEMD_MOUNT_ROOT"*"SYSTEMD_MOUNT_SUFFIX
#define SYSTEMD_MOUNT_ROOT_UNIT     SYSTEMD_MOUNT_ROOT SYSTEMD_MOUNT_SUFFIX
#define SYSTEMD_UNIT_DIR            "/storage/.config/system.d"
#define SYSTEMD_START_TIMEOUT       10

#define SYSTEMD_SYSTEM_RESERVED_MARK              "emuelecroms"
#define SYSTEMD_SYSTEM_RESERVED_PORTS_SCRIPTS     "ports_scripts"

static const size_t len_systemd_reserved_mark = strlen(SYSTEMD_SYSTEM_RESERVED_MARK);
static const unsigned int len_systemd_reserved_ports_scripts = strlen(SYSTEMD_SYSTEM_RESERVED_PORTS_SCRIPTS);

static const size_t len_systemd_unit_dir = strlen(SYSTEMD_UNIT_DIR);
static const size_t len_systemd_mount_root = strlen(SYSTEMD_MOUNT_ROOT);
static const size_t len_systemd_suffix = strlen(SYSTEMD_MOUNT_SUFFIX);
static const size_t len_systemd_job_prefix = strlen(SYSTEMD_PATH_JOB) + 1; // 1 for the extra /
static sd_bus *systemd_bus = NULL;

#define _cleanup_(x) __attribute__((__cleanup__(x)))