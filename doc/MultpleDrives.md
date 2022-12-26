## Introduction

In some cases you would want to use ROMs across multiple external drives (USB/SD/NVMe). From [EmuELEC v4.6 stable](https://github.com/EmuELEC/EmuELEC/releases/tag/v4.6) (**not 4.6-test**) onwards this was made possible with the introduction of the new mount-handler [eemount], the functionality of which is not limited to just mixing external drives, but we'll only focus on this specific use case here.

For a quick example, jump to [example](#example)

## Functionality

### What this can do
 - Mixing ROMs from multiple external drives, **each system from a certain drive** e.g. NES, SNES and MD from drive1, CDI and PSP from drive2, etc.
 - Increase the capacity of the ROMs storage, where savestates, BIOS, cheat tables, etc could also be saved.

### What this **can not** do
 - Mixing ROMs from multiple external drives, **each ROM from a certain drive** e.g. contra.nes from drive1, mario.nes from drive2, both for NES.
 - Increase the capacity of the system storage, where screenshots, upgrade package, etc could also be saved. Please use a bigger drive if this is the case.

## Terminology
In this documentation we will call any external USB drive / SD card / NVMe SSD, etc that could be a possible ROMs source a **drive**, to save typing time.

The term **ROMs** is used for anything that's stored under `/storage/roms` in EmuELEC's filesystem hierarchy, it could also refer to things that's absolutely not actually ROMs.

## Preparation
 - A device running official EmuELEC image with version >= 4.6 stable (**not 4.6-test**).
 - Multiple external drives that can **each** be used as single external ROMs drive seperately without problem.  *As the multi-drive feature is introduced as a backwards-compatible feature with the old external USB media ROMs feature, you should read [the wiki about it][wiki ext] first.*  
 - (Suggestion) Beefy power supply with good quality, **especially if you want to mix multiple HDDs and/or SSDs**, these power-hungry drives, when combined, can easily fry your tiny box if they're not fed enough. *If you're using devices like HK1Box, X96 Max, etc that comes with janky 5V2A power supply, replace it with a good 5V3A or 5V5A after-market one*
 - (Suggestion) Possibly USB 3.0 HUB to avoid bottleneck, if you're mixing so many drives that you run out of the native USB 3.0 ports. Prefer those with external power supply. 

## Format
For each drive, it's recommended to have a MBR partition table with only one partition, formatted as exFAT (which should be the default for new USB drives you can get these days) and named with meaningful labels. (e.g. Sandisk-16G-NeoGeo). **This is only a guidance for new drives. If you already have one drive that's working, you don't need to re-format it, just continue using it.**

The MBR partition table should be the default for any drive that's less than 2TiB and you shouldn't touch it. If it's larger than 2TiB, then you'll need to use GPT instead.

Including exFAT, these are multiple filesystems that you could use when formatting the drive that EmuELEC supports (in the order of most recommended to least recommended for a Windows user): exFAT, FAT32, FAT16, NTFS, ext4. 

You could name the drive with **any label you like**, but note each filesystem has its limitation on the characters that could be used. **No not name it with any of the following reserved names:** `EMUELEC`, `STORAGE`, `EEROMS`

## Layout
For each drive you want to use, you should have a file `emuelecroms` **without any extension** that's stored inside the `roms` folder, which (`roms`) is then stored directly inside the drive/partition. On the same level of `emuelecroms` you would have multiple folders, each with the name of the systems your ROMs are for. The layout should look like this:
```
/
  |- roms/
      |- nes/           # Folder to store ROMs for NES
          |- rom1.zip     # NES ROMs
          |- rom2.zip
          |- ...
      |- snes/          # Folder to store ROMs for SNES
          |- rom1.zip     # SNES ROMs
          |- rom2.zip
          |- ...
      |- .../           # Folders to store ROMs for other systems
      |- emuelecroms    # The mark file, empty or populated

```
The markfile `emuelecroms` has special format and requirement about its content documented in the [next section](#markfile).

These system folders (e.g. `nes`, `snes`) would be created automatically by EE if you've used the whole drive for ROMs with the [method mentioned earlier][wiki ext], but you can also check [system name on the wiki][system name]. Most of the names are intuitive (e.g. nes for NES, snes for SNES, ps1 for PS1) so you could also just YOLO it.

Note the above is just a recomended layout if you want to pre-populate the drive. The folders for systems are not actually needed, both for systems you don't want to use and do want to, mostly useful when testing stuffs. It's possible to just omit all of them (hence, to have a bare-bone ROMs drive) and populate them later. The following layout contains the minimum files/folder needed for the drive to be used:
```
/
  |- roms/
      |- emuelecroms
```


## Markfile

After you've prepared the layout, you should then edit the markfile `emuelecroms` as **a plain-text file** (e.g. using Notepad on Windows) to tell EmuELEC to use the drive as a whole or only for certain systems, so multiple drives can be mixed:
 - When the file is empty, the whole `roms` folder will be used. There could be **only one drive** that provides the `roms` folder.
 - When the file is not empty, each line should contain a name of the system that should be used. There could be **only one drive** that provides a certain system, but each drive can provide multiple systems.


E.g. For a drive that you would want to use only `nes` and `snes` roms, you should write this in its `emuelecroms`
```
nes
snes
```
Similarly a drive for only `neogeo` and `mame`
```
neogeo
mame
```
For drives whose whole `roms` folder should be used, you should make sure the mark file `emuelecroms` is empty
```
(empty, don't write anything)
```

Note the one for `roms` + one for each system limitation is only for EmuELEC's construction of the whole ROMs storage. You could have multiple drives with empty `emuelecroms`, all for whole `roms`, and multiple drives with conflictin system names, all for a same system. When you have such conflicting drives all connected, only the first one in alphabetical order will be used.

E.g. When four of these drives are all connected, each with an `emuelecroms` with their corresponding content:
1. `SD-16G-NES`
    ```
    nes
    ```
2. `KS-64G-NINTENDO`
    ```
    nes
    snes
    n64
    gb
    ```
3. `WD-1T-ROMS`
    ```
    ```
4. `SS-2T-ROMS`
    ```
    ```
These four drives will be handled like this:
1. `SS-2T-ROMS` will be used for `roms`; its `nes`, `snes`, `n64`, `gb` (existing or not) will be replaced by the drive 3.
2. `WD-1T-ROMS` will not be used  
3. `KS-64G-NINTENDO` will provide `nes`, `snes`, `n64`, `gb`.
4. `SD-16G-NES` will not be used

## System
As documented in the last part, for the content of mark file `emuelecroms`, each line should contain a *valid* system name, but note *valid* here does not mean it is strictly supported by EmuELEC.   

Any name that isn't one of the reserved names (`ports_scripts`, `emuelecroms`) and does not contain illegal characters (`/` and other characters that's impossible to be used in a file name) are considered valid. [eemount] is designed like this so more systems can be supported without updating it.

Note the existence of the corresponding folder is **not checked**, so you can write a system without actually creating its folder and store ROMs in it. It will **still be used**, and the folder will be automatically created, so you can populate ROMs over network share later.

## Example

Let's assume you have 4 different pen drives, each is 64G, and you've installed EmuELEC on a 8GiB SD card to have fast booting speed. The card is so small that you can't have ROMs on it risking filling up the tiny card. Yet you want to play your 200G ROMs at the same time. What could you do besides going out and buying a new 256G drive/card?

A smart choice is to store your ROMs across your 4 pen drives, each storing ROMs for certain systems

Let's assume you organize your systems like this:
|label|size|systems|
|-|-|-|
|Samsung-PS1|64G|psx
|Sandisk-Arcade|64G|neogeo, mame
|Kingston-DC|64G|dreamcast
|Lexar-Random|64G|other small systems

You would then place the files and create 4 `emuelecroms` mark files like the following:
 - Samsung-PS1
   - Layout
        ```
        / 
          |- roms/
             |- psx/
                |- images/
                |- psx-rom1.iso
                |- psx-rom2.iso
                |...
             |- emuelecroms
        ```
    - `emuelecroms`' content
        ```
        psx
        ```
 - Sandisk-Arcade
   - Layout
        ```
        / 
          |- roms
             |- mame/
                |- images/
                |- mame-rom1.zip
                |- mame-rom2.zip
                |...
             |- neogeo/
                |- images/
                |- neogeo.zip
                |- neogeo-rom1.zip
                |- neogeo-rom2.zip
                |...
             |- emuelecroms
        ```
    - `emuelecroms`' content
         ```
         mame
         neogeo
         ```
 - Kingston-DC
    - Layout
        ```
        / 
          |- roms/
             |- dreamcast/
                |- images/
                |- dc-rom1.iso
                |- dc-rom2.iso
                |...
             |- emuelecroms
        ```
    - `emuelecroms`' content
        ```
        dreamcast
        ```
 - Lexar-Random
    - Layout
        ```
        / 
          |- roms/
             |- nes/
                |- images/
                |- nes-rom1.zip
                |- nes-rom2.zip
                |...
             |- snes/
                |- images/
                |- snes-rom1.zip
                |- snes-rom2.zip
                |...
             |...
             |- emuelecroms
        ```
    - `emuelecroms`' content
        ```
        nes
        snes
        (and other systems on this drive, one per line)
        ``` 
    - Alternative `emuelecroms`' content
        ```
        (keep the file empty, write nothing, the whole roms folder will be used)
        ```
## Troubleshooting
1. Some of my ROMs are not showed, but its corresponding system is displayed
    - Make sure you've written the correct system name if you're storing them on a per-system drive, check [system name] for an incomplete list of systems supported by now. 
    - Make sure there's no other drives providing that same system.
2. Some of my drives are not recognized
    - The logic of [eemount] will consider the scanning successful if it finds even only a single working drive. If you're using a slow drive (slow here for starting, not for reading) e.g. external HDD, you might need to delay the scanning via option `ee_load.delay` in `/emuelec/configs/emuelec.conf`. `ee_load.delay=10` should be a safe bet for most drives. (*There is also a corresponding GUI setting in EmulationStation, but it could be written differently if you're using a different language*)
    - Are your power supply beefy enough? Especially if you're using the stock power supply of cheap boxes like HK1Box, X96, etc.
    - Are your USB HUB powered? If you're using a HUB to connect multiple drives you'd better use one with external power supply. Without it, the HUB will draw power from its uplink (the box), use some for itself, then split it up to its downlinks (the drives). The drive that could work when directly connected could fail to work if connected through a HUB.
3. For each boot my ROMs list is different
    - Do you have different drives that provides same systems without unique labels? Label your drive with meaningful names as [eemount] will try them in alphabetical order.
    - Change the value of `ee_load.delay` like in 2, some of your drives possibly started slowlier than others but things flip between boots.
4. I've set up everything but only one drive is used, and it is its whole `roms` folder that's used
    - Make sure you're using **>= v4.6 stable**, anything older does not have this per-system feature, 4.6-TEST is one such older version.

[wiki ext]: https://github.com/EmuELEC/EmuELEC/wiki/ROMS-on-external-USB-media
[system name]: https://github.com/EmuELEC/EmuELEC/wiki/Supported-Platforms-And--Correct-Rom-Path
[eemount]: https://github.com/7Ji/eemount
