# Multi-source ROMs mounting utility for EmuELEC

This is the new-generation mounting utility for EmuELEC, that can mount roms across tens of different storage mediums (cifs/nfs/sftp network shares, extenal drives, etc) altogether in at most under 3 seconds (If no systemd network mount is involved, this can be further pushed to under 0.2 seconds)

## emuelecroms mark syntax
When leaving empty, the ``/var/media/{drive}/roms/emuelec`` (``/roms/emuelec`` under the drive itself) still means the whole drive should be used for ``/storage/roms``, but you can now define only the systems you want from the drive in this file, one system per line. Empty line or lines containing reserved names (emuelecroms, ports_script) or invalid characters (/) will be ignored. For example:
```
nes
A Funky/Name that's invalid
emuelecroms
ports_script

snes
n64
ps2
```
will mount nes, snes, n64 and ps2

If you have an emuelecroms mark like this, the utility will bind-mount the nes, snes, n64 and ps2 subfolders (and create them if neccessary) to /storage/roms/{system}.


## Mounting order
(All mounting will only be done after ``/storage/roms`` is umounted recursively successfully)
### ``/storage/roms`` itself:
1. The systemd mount unit that provides mount for it (``/storage/.config/system.d/storage-roms.mount``)
2. Any external drives that has an empty ``emuelecroms`` under its ``roms`` subfolder


Example package.mk to build this under a LibreELEC build system:

```
PKG_NAME="mount_romfs"
PKG_VERSION="1"
PKG_URL=""
PKG_TOOLCHAIN="make"
PKG_DEPENDS_TARGET="toolchain systemd"

PKG_MAKE_OPTS_TARGET="CC=${TARGET_PREFIX}gcc"
PKG_MAKEINSTALL_OPTS_TARGET="STRIP=${TARGET_PREFIX}strip"
```