# Quake-HX86

Port of the WinQuake engine to the [Hashx86 OS](https://github.com/Hashx86-OS/Hashx86-OS) kernel.

## Status

Software renderer, keyboard + mouse input, no sound. Runs single-player and listen-server multiplayer on localhost.

## Prerequisites

- GCC/g++ with multilib support (for `-m32`)
- GNU `ld` (for `-melf_i386`)
- `make`

## Build

```bash
make
```

Output: `build/Quake.bin`

## Game data

`pak0.pak` is **proprietary** — you must provide your own copy from a legal Quake installation (Steam, GOG, original CD). The shareware `pak0.pak` also works for the first episode.

The game resolves its data directory relative to the running binary's location, so `pak0.pak` must be placed in an `id1/` folder next to `Quake.bin` (i.e. `<Quake dir>/id1/pak0.pak`). If the file is missing, the game shows a GUI dialog with the expected path and an Exit button.

## Install to Hashx86 OS disk image

Mount the VHD/VDI and copy the files directly:

```bash
# 1. Load NBD module
sudo modprobe nbd max_part=16

# 2. Connect the disk image
sudo qemu-nbd --connect=/dev/nbd0 /path/to/Hashx86-OS/hdd.vdi

# 3. Mount partition 1
sudo mkdir -p /mnt/vdi_p1
sudo mount /dev/nbd0p1 /mnt/vdi_p1

# 4. Create directories
sudo mkdir -p /mnt/vdi_p1/apps/Quake/id1

# 5. Copy Quake binary and game data
sudo cp build/Quake.bin /mnt/vdi_p1/apps/Quake/Quake.bin
sudo cp /path/to/your/pak0.pak /mnt/vdi_p1/apps/Quake/id1/pak0.pak

# 6. Cleanup
sudo umount /mnt/vdi_p1
sudo qemu-nbd --disconnect /dev/nbd0
```

Disk image layout after copying:

```
/
└── apps/
    └── Quake/
        ├── Quake.bin        ← the game binary
        └── id1/
            └── pak0.pak     ← game data (not included)
```

## Run

Launch Quake from the Hashx86 OS (`File Explorer` → `Apps` → `Quake`).

## Files

| Path | Description |
|---|---|
| `Quake/engine/` | WinQuake engine source (GPL 2.0) |
| `Quake/include/` | WinQuake header files (GPL 2.0) |
| `Quake/prog.cpp` | Entry point for Hashx86 OS |
| `Quake/compat.c` | C standard library stubs + syscall wrappers |
| `Quake/sys_hx86.c` | System interface (file I/O, timing) |
| `Quake/vid_hx86.c` | Video output (VESA framebuffer) |
| `Quake/in_hx86.c` | Keyboard/mouse input |
| `Quake/linker.ld` | Linker script for Hashx86 OS binaries |
| `Quake/Makefile` | Build rules |
| `libhx86/` | Hashx86 OS user-space library |

## License

- **Engine code** (`engine/`, `include/`): GPL v2 or later — Copyright (C) 1996-1997 id Software, Inc.
- **Port code** (`prog.cpp`, `compat.c`, `sys_hx86.c`, `vid_hx86.c`, `in_hx86.c`, `Makefile`, `linker.ld`): GPL v2 or later
- **Game data** (`pak0.pak`): Proprietary — not included
