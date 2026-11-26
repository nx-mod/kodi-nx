# Kodi-NX notes

## What works

- Kodi 23.0-ALPHA1 runs as a Switch 1 NRO: Estuary GUI, menus, navigation,
  settings, library databases, local ZIP add-on installation, audio playback
  and clean exit.
- SQLite uses `nx-mod/sqlite-nx` with a native libnx VFS. All Kodi databases
  (Addons, ViewModes, Textures, Music, Video, TV, EPG) initialize cleanly.
- Audio uses libnx `audout`: 48 kHz signed 16-bit stereo.
- Input: Switch pad mapped to Kodi keys; touch mapped to mouse; key repeat.
- Boot progress is written to `sdmc:/switch/kodi/boot.txt`; full log in
  `sdmc:/switch/kodi/.kodi/temp/kodi.log`.

## Build

```
./build_nx.sh                 # configure, build, package, stage
./build_nx.sh --clean         # fresh build
JOBS=4 ./build_nx.sh          # 4 jobs (8 locks up this host)
```

`build_nx.sh` sets up devkitA64 + flat libnx, installs the patched Switch
linker script, builds host TexturePacker, runs CMake, and post-processes the
NRO. Result: `build/switch/kodi.nro`, also staged to
`/storage/emulated/0/Download/kodi-nx/`.

## Install on Switch

```
/switch/kodi.nro                  NRO hbmenu launches
/switch/kodi/{addons,media,system,userdata}
/switch/kodi/.kodi/temp/kodi.log  log
```

## Port facts

- Toolchain: devkitA64 GCC 16.1.0, libnx 4.12.0 from `~/nx-mod/libnx/nx`.
- Key portlibs: curl 8.23.0 + mbedTLS 3.6.8 (switchfin releases), OpenSSL 3.0.22,
  sqlite-nx, Brotli, full GNU libiconv with `-DUSE_DOS` (ZIP filenames),
  libuuid stand-in, `libdl`/`librt`/`libutil` stubs.
- `cmake/toolchain/switch.ld` places relocatable read-only sections in the
  writable NRO segment; `tools/switch/fix_nro_size.py` fixes the ASET asset
  offset. Both are required for hbmenu to accept the NRO.
- Threads get 1 MiB stacks; detached threads are reaped. Kodi's CMake carries
  small `TARGET_SWITCH` guards (upstream merge conflict points).
- HTTPS: embedded compact CA bundle via `CURLOPT_CAINFO_BLOB` (curl's mbedTLS
  backend supports it). Verification stays enabled.

## Remaining work

- Python add-ons: static CPython 3.14.6 from `nx-mod/python-nx` linked into the
  NRO; stdlib zip installed at `/switch/kodi/lib/python314.zip`. Awaiting
  hardware verification of add-on execution.
- Binary add-ons: not available; `dlopen` is stubbed. Would require building
  add-ons statically into the NRO.
- Online repositories: HTTPS path implemented; awaiting hardware verification.
- Hardware video decode, surround passthrough, JPEG/PNG thumbnail encoders.
- Event clients / JSON-RPC / web server / auto-update remain disabled.
- No in-app crash handler yet; Atmosphere reports plus `boot.txt` are used.
