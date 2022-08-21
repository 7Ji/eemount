#include "sort_p.h"

int sort_compare_string(const void *a, const void *b){
    return strcmp(*(char**)a, *(char**)b);
}

int sort_compare_drive(const void *a, const void *b) {
    return strcmp(((struct drive *)a)->name, ((struct drive *)b)->name);
}

int sort_compare_systemd_mount_unit(const void *a, const void *b) {
    return strcmp(((struct systemd_mount_unit *)a)->system, ((struct systemd_mount_unit *)b)->system);
}