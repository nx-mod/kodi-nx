# Kodi-NX build notes

## Current state

Kodi 23.0-ALPHA1 builds and runs as a Switch 1 NRO. Verified working:
Estuary GUI, navigation, settings, audio playback (libnx `audout`), local ZIP
add-on installation, and clean exit back to hbmenu. Output: `build/switch/kodi.nro`.

Not working yet: online add-on repositories, Python/binary add-ons, hardware
video decode, and several image encoders (thumbnails). See "Gaps" below.

## SD card layout

```
/switch/kodi.nro                  NRO that hbmenu scans in /switch root
/switch/kodi/kodi.nro             same NRO, kept beside its data
/switch/kodi/{addons,media,system,userdata}
/switch/kodi/boot.txt             boot stage marks
/switch/kodi/.kodi/temp/kodi.log  Kodi log
```

Build output is also staged to `/storage/emulated/0/Download/kodi-nx/` when
that directory exists.

## Build

```
./build_nx.sh                 # configure + build + NRO post-processing + stage
./build_nx.sh --clean         # fresh build directory
JOBS=4 ./build_nx.sh          # job count (default 4; 8 locks up this host)
```

`build_nx.sh` installs the patched linker script, builds host TexturePacker if
missing, then calls `tools/switch/configure.sh` and `cmake --build`.
The NRO post-processing step is required (`tools/switch/fix_nro_size.py`).

## Toolchain and portlibs

- devkitA64 GCC 16.1.0, libnx from `~/nx-mod/libnx/nx` (built, v4.12.0).
- Portlibs in `/opt/devkitpro/portlibs/switch`:
  - pacman: `switch-curl`, `switch-mbedtls`, `switch-libdrm_nouveau`,
    `switch-tinyxml2`, `switch-libpcre2`, `switch-libxml2`, `switch-liblzma`,
    `switch-libzstd`, `switch-liblzo2`, `switch-giflib`.
  - curl 8.23.0 + mbedTLS 3.6.8 from dragonflylee/switchfin releases (dkp's
    curl 7.69.1 is too old for Kodi 23; dkp mbedTLS 2.28 lacks symbols).
  - sqlite3 from `nx-mod/sqlite-nx` with a native libnx VFS.
  - Brotli 1.1.0, libuuid stand-in (`randomGet`), empty `libdl`/`librt`/`libutil`.
  - GNU libiconv 1.17 built with `-DUSE_DOS` (CP437 zip filenames) and a small
    `__NEWLIB__` guard in `lib/loop_wchar.h`. Without `USE_DOS`, Kodi cannot
    read ZIP archives and reports `item count: 0`.
- Host tools: autoconf, automake, libtool; TexturePacker built from
  `tools/depends/native/TexturePacker/src` with `-DTARGET_POSIX`.

## Linker and packaging

libnx's loader applies static relocations itself and maps three NRO segments
(text RX, rodata R, data RW). Kodi's large PIE has relative relocations into
sections upstream places in read-only memory:

- `cmake/toolchain/switch.ld` moves `.rodata`, `.tls.align`, `.gcc_except_table`,
  `.eh_frame`, `.gnu_extab`, `.exception_ranges` into the writable segment,
  *after* `__data_start`, so libnx's relro reprotection range stays valid.
  Placing them before `__data_start` causes a `2001-0106` abort in `__nx_dynamic`.
- configure.sh installs that script over `$DEVKITPRO/libnx/switch.ld` (sudo needed).
- Link flags: `-z notext`, `-z nopack-relative-relocs`, `--wrap=pthread_create`,
  `--wrap=pthread_detach`, `-z max-page-size=4096`.
- Tail libraries after `--no-whole-archive`: `-ldrm_nouveau -lglapi -lEGL
  -luuid -lsqlite3 -lnx`.
- `elf2nro` computes the asset offset from the sum of segment sizes; the linker
  leaves a 64 KiB gap before RW, so `fix_nro_size.py` restores the overwritten
  data bytes and appends the ASET asset section at the real NRO end. Without it
  hbmenu shows `2347-0006` and unknown author/version.

## Platform code

- `xbmc/platform/switch`: main (sdmc mount, boot marks, sockets/nxlink),
  `PosixCompat` (1 MiB stacks, detached-thread reaper, POSIX stubs), `DlStubs`,
  network (nifm), storage (SD card).
- `xbmc/windowing/switch`: EGL/GLES window system on `nwindowGetDefault()`,
  pad/touch input via `CAppInboundProtocol`, registered as "switch".
- `xbmc/cores/AudioEngine/Sinks/AESinkSwitch.*`: libnx `audout` PCM sink
  (48 kHz, s16 stereo, four aligned buffers), registered in `InitStageOne`.
- Force-included compat header: `xbmc/platform/switch/compat/switch_posix.h`.
- Kodi CMake carries small `TARGET_SWITCH` guards; these are the upstream merge
  conflict points: root `CMakeLists.txt`, `FindTinyXML2`, DllLoader CMake,
  `XBTFReader`, `DNSNameCache`, `PlatformDefs.h`, `XTimeUtils`,
  `RecursiveMutex`, `DVDInputStreamNavigator`, `CurlFile.h` (reverted), VideoRenderers.

## Gaps and dead ends

- **Online repositories do not work.** HTTPS fails with `SSL peer certificate
  ... not OK (60)`. Tried and reverted: `catrustfile` advanced setting,
  `CURL_CA_BUNDLE` env var (a curl CLI feature, not automatic in libcurl),
  explicit `CURLOPT_CAINFO`, and `CURLOPT_SSL_VERIFYPEER=0`. The explicit CA
  path and verification-off attempts caused Atmosphere fatals (`2349-0008`,
  libnx binder `NoInit`) on repository access. Use local ZIP install instead.
- Python add-ons and binary add-ons are unsupported (`ENABLE_PYTHON=OFF`,
  `dlopen` stubbed).
- Software video decode only; no HW decode, no surround passthrough.
- Missing JPEG/PNG image encoders → thumbnail creation errors.
- No in-app crash handler; diagnosis uses Atmosphere crash/fatal reports plus
  `boot.txt` and `kodi.log`.
- Thread stack size and detached-thread reaping are workarounds, not upstream fixes.

## Outstanding work

- Port CI build and NRO updater from `haloce-nx`:
  `.github/workflows/{build,switch}.yml`, `tools/ci_build.py`.
- Consider a file-based exception handler writing `crash.txt`.
- Revisit online repos only with a proper libnx `ssl` CA trust solution.
