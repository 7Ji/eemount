#include "sort_p.h"

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
#endif
}

int sort_compare_mount_system(const void *a, const void *b) {
    return strcmp(((struct mount_system *)a)->system, ((struct mount_system *)b)->system);
}