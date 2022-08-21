#if 0
int sort_compare_systemd_mount(const void *a, const void *b) {
#ifdef SORT_COMPARE_SYSTEMD_MOUNT_USE_NAME
    return strcmp(((struct systemd_mount *)a)->name, ((struct systemd_mount *)b)->name);
#else
    return strcmp(((struct systemd_mount *)a)->system, ((struct systemd_mount *)b)->system);
#endif
}

int sort_compare_mount_system(const void *a, const void *b) {
    return strcmp(((struct eemount_system *)a)->system, ((struct eemount_system *)b)->system);
}
#endif