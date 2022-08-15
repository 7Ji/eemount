#ifndef HAVE_MOUNT_H
#define HAVE_MOUNT_H
#include "common.h"

#include <sys/mount.h>

#include "systemd.h"
#include "drive.h"

enum mount_type {
    MOUNT_SYSTEMD,
    MOUNT_DRIVE
};

struct mount_system {
    int type;
    char *system;
    struct systemd_mount *systemd;
    struct drive *drive;
};

struct mount_system_helper {
    unsigned count;
    struct mount_system *systems;
};

struct mount_system_helper *mount_get_systems(struct systemd_mount_helper *systemd_helper, struct drive_helper *drive_helper);
#endif