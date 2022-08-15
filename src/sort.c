#include "sort.h"
#include "string.h"
#include "drive.h"
#include "systemd.h"

int sort_compare_string(const void *a, const void *b){
    return strcmp(*(char**)a, *(char**)b);
}

int sort_compare_drive(const void *a, const void *b) {
    return strcmp(((struct drive *)a)->name, ((struct drive *)b)->name);
}

int sort_compare_systemd_mount(const void *a, const void *b) {
#ifdef SORT_COMPARE_SYSTEMD_MOUNT_USE_NAME
    return strcmp(((struct systemd_mount *)a)->name, ((struct systemd_mount *)b)->name);
#else
    return strcmp(((struct systemd_mount *)a)->system, ((struct systemd_mount *)b)->system);
    // if (((struct systemd_mount *)a)->system) {
    //     if (((struct systemd_mount *)b)->system) {
    //         return strcmp(((struct systemd_mount *)a)->system, ((struct systemd_mount *)b)->system);
    //     } else {
    //         return 1;
    //     }
    // } else {
    //     if (((struct systemd_mount *)b)->system) {
    //         return -1;
    //     } else { 
    //         return 0; // There should not be two empty systems, in that case there're two storage-roms.mount, which is impossible, but we still cover it any way.
    //     }
    // }
#endif
}