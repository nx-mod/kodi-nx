# Kodi-NX

Work-in-progress, unofficial port of [Kodi](https://kodi.tv/) 23.0 to the
Nintendo Switch (libnx / devkitA64). Not affiliated with the Kodi project.

## Status

The NRO boots into Estuary: menus, navigation, settings, audio playback, local
ZIP add-on installation and clean exit work. Online repositories, Python and
binary add-ons, and hardware video decode do not work yet. See `notes.md` for
the full picture.

## Build

```
./build_nx.sh                 # build, package and stage kodi.nro
./build_nx.sh --clean         # fresh build
```

Requires devkitA64, the flat libnx clone at `../libnx`, and the Switch portlibs
listed in `notes.md`. The script installs the patched Switch linker script,
builds host tools, configures CMake, builds with `-j4`, fixes the NRO packaging,
and stages the result in `/storage/emulated/0/Download/kodi-nx/` when present.

## Install

Copy `kodi.nro` to the root of `/switch/` (hbmenu scans there), and the
`addons`, `media`, `system` and `userdata` folders into `/switch/kodi/`.
Logs: `/switch/kodi/boot.txt` and `/switch/kodi/.kodi/temp/kodi.log`.

Upstream: [xbmc/xbmc](https://github.com/xbmc/xbmc)
