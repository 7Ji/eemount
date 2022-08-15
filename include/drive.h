#ifndef HAVE_DRIVE_H
#define HAVE_DRIVE_H
#include "common.h"

#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include <sys/stat.h>

struct drive {
    char *name;
    char **systems;
    unsigned int count;
};

struct drive_helper {
    struct drive *drives;
    unsigned int count;
};

void drive_helper_free(struct drive_helper **drive_helper);
struct drive_helper *drive_get_mounts();
#endif