#include "block_p.h"

bool block_initialize() {
    block_pr = blkid_new_probe_from_filename("/dev/block/259:0");
    if (block_pr) {
        return true;
    } else {
        block_pr = NULL;
        return false;
    }
}

void block_list() {
    puts("1");
    blkid_partlist ls = blkid_probe_get_partitions(block_pr);
    if (ls) {
        puts("2");
        int nparts = blkid_partlist_numof_partitions(ls);
        puts("3");
        logging(LOGGING_DEBUG, "Found %d partitions", nparts);
        if (nparts <= 0) {
            logging(LOGGING_ERROR, "Partition number illegal!");
            return;
        }
        // for (int i=0; i<nparts; ++i) {
        //     blkid_probe

        // }
        return;
    } else {
        logging(LOGGING_ERROR, "Can't obtain partlist");
    }
}

void block_free() {
    if (block_pr) {
        blkid_free_probe(block_pr);
        block_pr = NULL;
    }
}