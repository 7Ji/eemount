#include "block.h"

#include <blkid/blkid.h>

#include "logging.h"

blkid_probe block_pr = NULL;