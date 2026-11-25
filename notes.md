# Kodi-NX build notes

## What runs today

Kodi 23.0-ALPHA1 builds as a Switch 1 NRO. GUI, Estuary, navigation, settings,
local ZIP add-on install, and clean exit work. Output: `build/switch/kodi.nro`.

Install layout on the Switch SD card:

```
/switch/kodi.nro                  (hbmenu scans /switch root)
/switch/kodi/kodi.nro             (also kept in the data folder)
/switch/kodi/{addons,media,system,userdata}
/switch/kodi/boot.txt             (boot stage log)
/switch/kodi/.kodi/temp/kodi.log  (Kodi log)
```

## Toolchain and portlibs

- devkitA64 GCC 16.1.0, libnx from `~/nx-mod/libnx/nx` (built, v4.12.0).
- Switch portlibs in `/opt/devkitpro/portlibs/switch`, installed via pacman where
  possible: `switch-curl`, `switch-mbedtls`, `switch-libdrm_nouveau`,
  `switch-tinyxml2`, `switch-libpcre2`, `switch-libxml2`, `switch-liblzma`,
  `switch-libzstd`, `switch-liblzo2`, `switch-giflib`.
- Manually built portlibs:
  - Brotli 1.1.0, libiconv (GNU 1.17 with `-DUSE_DOS` for CP437 ZIP filenames),
    libuuid stand-in (`randomGet`), `libdl`/`librt`/`libutil` stubs.
  - curl 8.23.0 and mbedTLS 3.6.8 from dragonflylee/switchfin releases (the dkp
    curl 7.69.1 is too old for Kodi 23; dkp mbedTLS 2.28 lacks required symbols).
  - sqlite3 from `nx-mod/sqlite-nx` (native libnx VFS) as `libsqlite3.a`.
- Host build tools: autoconf, automake, libtool, gperf-free libiconv; host
  TexturePacker built from `tools/depends/native/TexturePacker/src` with
  `-DTARGET_POSIX`.

## Linker and packaging

libnx's loader applies static relocations itself and only maps three NRO
segments (text RX, rodata R, data RW). Kodi's large PIE has relative relocations
into what upstream puts in `.rodata`, so:

- `cmake/toolchain/switch.ld` moves `.rodata`, `.tls.align`, `.gcc_except_table`,
  `.eh_frame`, `.gnu_extab`, `.exception_ranges` into the writable segment,
  *after* `__data_start` so libnx's relro reprotection range stays valid.
- `tools/switch/configure.sh` installs that script over
  `$DEVKITPRO/libnx/switch.ld` (needs sudo).
- Link flags: `-z notext`, `-z nopack-relative-relocs`, `--wrap=pthread_create`,
  `--wrap=pthread_detach`, `-z max-page-size=4096`.
- Tail libraries after `--no-whole-archive`: `-ldrm_nouveau -lglapi -lEGL
  -luuid -lsqlite3 -lnx`.
- `elf2nro` computes the NRO asset offset from the sum of segment sizes. The
  linker leaves a 64 KiB gap before the RW segment, so `tools/switch/fix_nro_size.py`
  restores the overwritten data bytes and appends the ASET asset section at the
  real end, updating the NRO header size. Without this hbmenu reports
  `2347-0006` / unknown author+version.

## Platform code

- `xbmc/platform/switch`: main (sdmc mount, boot marks, socket/nxlink),
  `PosixCompat` (1 MiB thread stacks, detached-thread reaper, uname/getrusage/
  execvp/waitpid stubs), `DlStubs`, network (nifm), storage (SD card).
- `xbmc/windowing/switch`: EGL/GLES window system on `nwindowGetDefault()`,
  pad and touch input via `CAppInboundProtocol`; registered as "switch".
- `xbmc/cores/AudioEngine/Sinks/AESinkSwitch.*`: libnx `audout` PCM sink
  (48 kHz, s16 stereo, four aligned buffers), registered in `InitStageOne`.
- Force-included compat header: `xbmc/platform/switch/compat/switch_posix.h`.
- Kodi CMake touched with small `TARGET_SWITCH` guards; these are merge-conflict
  points when rebasing upstream.

## Known gaps

- Python and binary add-ons disabled/unsupported (no `dlopen`).
- Software video decode only; no HW decode or surround passthrough.
- Online repos need TLS trust: `system/certs/cacert.pem` plus
  `userdata/advancedsettings.xml` with `<catrustfile>`; still being verified.

## Outstanding reference work

- Port CI build and NRO updater from `haloce-nx`:
  `.github/workflows/{build,switch}.yml`, `tools/ci_build.py`.
- Add a crash handler writing `crash.txt` (currently only Atmosphere reports).
