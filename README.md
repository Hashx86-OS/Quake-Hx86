# Quake-HX86

Port of the WinQuake engine to the [Hashx86 OS](https://github.com/Hashx86-OS/Hashx86-OS) kernel.

## Status

Software renderer, keyboard + mouse input, no sound. Runs single-player and listen-server multiplayer on localhost.

## Prerequisites

- GCC/g++ with multilib support (for `-m32`)
- GNU `ld` (for `-melf_i386`)
- `make`
- A [Hashx86-OS](https://github.com/Hashx86-OS/Hashx86-OS) checkout, for `libhx86`

## Build

By default the build expects the kernel checkout next to this repository:

```text
org/
  Hashx86-OS/
  Quake-Hx86/
```

```bash
make
```

Point `KERNEL_DIR` at a different checkout if needed:

```bash
make KERNEL_DIR=/path/to/Hashx86-OS
```

Output: `build/Quake.bin`

The build compiles `libhx86` and `libmbedtls` from `$KERNEL_DIR` and links
them statically. The `libhx86/` directory in this repository is a legacy
snapshot and is not used by the build.

## Game data

`pak0.pak` is **proprietary** — full/retail data must come from a legal Quake
installation (Steam, GOG, original CD). The shareware episode 1 data is
available through the Quake app itself.

The game resolves its data directory relative to the running binary's location,
so data lives in `<Quake dir>/id1/pak0.pak`. If the file is missing, the app
shows a dialog offering:

- a download of the original id Software `quake106.zip` from gamers.org,
- SHA-256 verification of the archive and extracted members,
- local ZIP+LHA extraction into `id1/pak0.pak`,
- installation of `SLICNSE.TXT` and `LICINFO.TXT` under `licenses/`,
- preservation of the original compressed archive under `shareware/`.

Hashx86 does not host the game data. Only the original compressed shareware
archive is downloaded; the app never redistributes a bare PAK. The shareware
license applies: episode 1 only, no commercial use.

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
| `Quake/prog.cpp` | Entry point and missing-data installer UI |
| `Quake/data/` | Shareware download, SHA-256 verification and ZIP+LHA extraction |
| `Quake/compat.c` | C standard library stubs + syscall wrappers |
| `Quake/sys_hx86.c` | System interface (file I/O, timing) |
| `Quake/vid_hx86.c` | Video output (VESA framebuffer) |
| `Quake/in_hx86.c` | Keyboard/mouse input |
| `Quake/linker.ld` | Linker script for Hashx86 OS binaries |
| `Quake/Makefile` | Build rules |
| `third_party/miniz/` | ZIP/deflate reader (MIT) |
| `third_party/lhasa/` | LHA/LH5 reader (ISC) |
| `libhx86/` | Legacy vendored snapshot; the build uses `$KERNEL_DIR/user_prog/libhx86` |

## License

- **Engine code** (`engine/`, `include/`): GPL v2 or later — Copyright (C) 1996-1997 id Software, Inc.
- **Port code** (`prog.cpp`, `compat.c`, `sys_hx86.c`, `vid_hx86.c`, `in_hx86.c`, `Makefile`, `linker.ld`): GPL v2 or later
- **Data installer** (`data/`): MIT
- **miniz** (`third_party/miniz/`): MIT; **lhasa** (`third_party/lhasa/`): ISC
- **Game data** (`pak0.pak`): Proprietary — not included
