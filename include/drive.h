#ifndef HAVE_DRIVE_H
#define HAVE_DRIVE_H
#include "common.h"

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