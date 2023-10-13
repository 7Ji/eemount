# Multi-source ROMs mounting utility for EmuELEC

**This is already a part of EmuELEC since v4.6, you shouldn't need to install this manually by yourself**

This is the new-generation mounting utility for EmuELEC, that can mount roms across tens of different storage mediums (cifs/nfs/sftp network shares, extenal drives, etc) altogether in at most under 3 seconds (If no systemd network mount is involved, this can be further pushed to under 0.2 seconds)

Combination of roms from the following 3 types media can be mixed at well:
 - systemd mount units
 - external drives (removable) not handled by systemd mount units
 - internal drive (the bootup drive) not handled by systemd mount units and init mount logic

## Mounting order
1. ### ``/storage/roms`` itself:
    (If any one of the following is mounted successfully, the remanining steps are ignored)
    1. The systemd mount unit that provides mount for it (``/storage/.config/system.d/storage-roms.mount``)
    2. Any external drives that has an empty ``emuelecroms`` (or populated but with none valid system names, read the system mounts below to check its syntax) under its ``roms`` subfolder, in alphabetical order
    3. ``EEROMS``:
        1. The exact partition that provides mount for ``/storage/.update`` (which should be the exact EEROMS partition chosen during init)
        2. The 3rd partition on the boot drive, if the block devices for partitions on the boot drive is named in /dev/prefixN style (USB, SDcard, NVMe, eMMC with a valid partition table, etc; *eMMC using well-known name as partition block names (/dev/data, /dev/system) is ignored*)
        3. Any partition with a label ``EEROMS`` (this is least favored as there could be multiple partitions with label ``EEROMS``)
2. ### ``/var/media/EEROMS``
    In case option 1 or 2 is used in step 1, the utility will try to mount ``EEROMS`` with the same logic to ``/var/media/EEROMS``
3. ### ``/storage/.update``
    .update subfolder under the EEROMS partition will be binded to ``/storage/.update``
4. ### ``/storage/roms/ports_script``
    This is now handled by the mounter instead of emustation-setup, to speed up the whole process, and since it's under roms we should do it like this anyway

    The mount is essentially an overlay that combines the read-only lower folder ``/usr/bin/ports`` with writable ``/emuelec/ports`` upper folder and use ``/storage/.tmp/ports-workdir`` as workdir

5. ### systems under ``/storage/roms``
    (If any system is already mounted, it won't be mounted again in the remaining steps)  
    Scenario 1: systemd mount units √ external drives √
    1. Systemd mount units that provide mount for layer-1 systems (``storage-roms-nes.mount``, ``storage-roms-snes.mount``, etc) in alphabetical order
    2. Any external drive that has an ``emuelecroms`` with populated **valid** system names. Drives in alphabetical order then systems in that drive in alphabetical order. One system per line, empty lines and lines containing invalid characters (/) or reserved names (emuelec, ports_script) are ignored.  
    E.g. A emuelecroms mark with the following content
        ```
        nes
        A Funky/Name that's invalid
        emuelecroms
        ports_script

        snes
        n64
        ps


        ```
        will only provide systems nes, snes, n64 and ps  
        An emuelecroms mark with the following content
        ```


        Wow I'm using a really strange name :/
        ```
        is considered a dirve that should be used for ``/storage/roms`` itself.
    3. Systemd mount units that provide mount for multi-layer systems (``storage-roms-nes-images.mount``, ``storage-roms-snes-images.mount``, etc)


    Scenario 2: systemd mount units √ external drives ×
     - Systemd mount units that provide mount for them (``/storage/config/system.d/storage-roms-*.mount``) in alphabetical order.   

    Scenario 3: systemd mount units × external drives √
     - Any external drive that has an ``emuelecroms`` with populated **valid** system names. Drives in alphabetical order then systems in that drive in alphabetical order

### Compatability
As this is written purely for EmuELEC, the tool is only guaranteed to work on EmuELEC

Example package.mk to build this under a LibreELEC build system:

```
# SPDX-License-Identifier: GPL-3.0
# Copyright (C) 2022-present 7Ji (https://github.com/7Ji)

PKG_NAME="eemount"
PKG_VERSION="367de8248d32b6acec092f2f3a1582621c977322"
PKG_SHA256="5c3924186be79b02ee8af85b5ae2b23d61dac3cba1e566ee46cc64b47ef2eb81"
PKG_SITE="https://github.com/7Ji/eemount"
PKG_URL="${PKG_SITE}/archive/${PKG_VERSION}.tar.gz"
PKG_DEPENDS_TARGET="toolchain systemd"
PKG_LONGDESC="Multi-source ROMs mounting utility for EmuELEC"
PKG_TOOLCHAIN="make"
PKG_MAKE_OPTS_TARGET="LOGGING_ALL_TO_STDOUT=1"
```

# License
**eemount** (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)
 * Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
