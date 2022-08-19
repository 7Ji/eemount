#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <blkid/blkid.h>

int main() {
    char *name = blkid_evaluate_tag("LABEL", "EEROMS", NULL);
    if (name) {
        puts(name);
        free(name);
        if (mount("/dev/sda3", "newroms", "vfat", MS_NOATIME, "rw,fmask=0022,dmask=0022,codepage=936,iocharset=utf8,shortname=mixed,utf8,errors=remount-ro")) {
            puts("failed");
        }
    }
    // mount("/var/media/EEROMS/roms/n64", "/storage/roms/n64", NULL, MS_BIND, NULL);
    return 0;
}