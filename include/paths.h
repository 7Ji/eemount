#ifndef HAVE_PATHS_H
#define HAVE_PATHS_H
#include "common.h"
#include <string.h>

#define PATH_NAME_ROMS      "roms"
#define PATH_NAME_STORAGE   "storage"
#define PATH_NAME_PSCRIPTS  "ports_scripts"
#define PATH_NAME_UPDATE    ".update"
#define PATH_NAME_MARK      "emuelecroms"

#define PATH_NAME_VAR       "var"
#define PATH_NAME_MEDIA     "media"
#define PATH_NAME_EEROMS    "EEROMS"

#define PATH_DIR_STORAGE    "/"PATH_NAME_STORAGE
#define PATH_DIR_ROMS       PATH_DIR_STORAGE"/"PATH_NAME_ROMS
#define PATH_DIR_PSCRIPTS   PATH_DIR_ROMS"/"PATH_NAME_PSCRIPTS

#define PATH_DIR_EXTERNAL   "/"PATH_NAME_VAR"/"PATH_NAME_MEDIA
#define PATH_DIR_EXTERNAL_EEROMS    PATH_DIR_EXTERNAL"/"PATH_NAME_EEROMS

#define PATH_DIR_UPDATE     PATH_DIR_STORAGE"/"PATH_NAME_UPDATE
#define PATH_DIR_UPDATE_INT PATH_DIR_ROMS"/"PATH_NAME_UPDATE
#define PATH_DIR_UPDATE_EXT PATH_DIR_EXTERNAL_EEROMS"/"PATH_NAME_UPDATE

static const size_t len_path_name_roms = strlen(PATH_NAME_ROMS);
static const size_t len_path_name_mark = strlen(PATH_NAME_MARK);
static const size_t len_path_name_pscripts = strlen(PATH_NAME_PSCRIPTS);
static const size_t len_path_dir_roms = strlen(PATH_DIR_ROMS);
static const size_t len_path_dir_external = strlen(PATH_DIR_EXTERNAL);
#endif