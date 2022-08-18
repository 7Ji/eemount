#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>

int main() {
    mount("/var/media/EEROMS/roms/n64", "/storage/roms/n64", NULL, MS_BIND, NULL);
    return 0;
}