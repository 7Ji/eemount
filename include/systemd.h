#ifndef HAVE_SYSTEMD_H
#define HAVE_SYSTEMD_H
#include "common.h"

#include <stdbool.h>

/**
 Encode a unit name to sd-bus valid path
 \param unit pointer to the raw name of a unit
 \param path pointer to a pointer to string to return the encoded path

 \returns true if encode successfully, false if not
*/
bool systemd_encode_path(char *unit, char **path);

/**
 Call to initialize the systemd_bus

 \returns true if initialized successfully, false it not
*/
bool systemd_init_bus();

/**
 Release the systemd_bus
*/
void systemd_release();

/**
 Get the status of a systemd unit

 \param path the escaped path of the unit, you may need to call escape_string() to get a valid name first

 \returns true if active, false if not
*/
bool systemd_is_active(char *path);

struct systemd_mount {
    char *name;
    char *path;
    char *system; // When this is empty, it means /storage/roms itself. It should never be empty
};

struct systemd_mount_helper {
    struct systemd_mount *mounts;
    struct systemd_mount *root; // /storage/roms itself
    unsigned int count;
};

struct systemd_mount_helper *systemd_get_mounts();

void systemd_mount_helper_free (struct systemd_mount_helper **mounts_helper);
#endif