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
    unsigned int count_systems;
};

struct drive_helper {
    struct drive *drives;
    unsigned int count_drives;
};

void drive_helper_free(struct drive_helper **drive_helper);
struct drive_helper *drive_get_list();