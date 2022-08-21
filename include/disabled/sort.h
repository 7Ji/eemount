#if 0
/**
 * @brief Compare two systemd_mount struct
 * 
 * @param a One of the systemd_mount struct to compare
 * @param b The other systemd_mount struct to compare
 * @return int >0 if a's name is greater, <0 if b's name is greater, 0 if equal
 */
int sort_compare_systemd_mount(const void *a, const void *b);

/**
 * @brief Compare two mount_system struct
 * 
 * @param a One of the mount_system struct to compare
 * @param b The other mount_system struct to compare
 * @return int >0 if a's system(name string) is greater, <0 if b's system is greater, 0 if equal
 */
int sort_compare_mount_system(const void *a, const void *b);
#endif