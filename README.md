# Kodi-NX

Work-in-progress, unofficial port of [Kodi](https://kodi.tv/) 23.0 to the
Nintendo Switch (libnx / devkitA64). Not affiliated with the Kodi project.

## Status

Boots to Estuary with working menus, navigation, settings, library databases,
local ZIP add-on installation, audio playback and clean exit.

## Build

```
./build_nx.sh                 # build, package and stage kodi.nro
./build_nx.sh --clean         # fresh build
```

Requires devkitA64, the flat libnx clone at `../libnx`, and the Switch portlibs
listed in `notes.md`. The NRO is staged in `/storage/emulated/0/Download/kodi-nx/`.

## Install

Copy `kodi.nro` to `/switch/` and the `addons`, `media`, `system` and `userdata`
folders to `/switch/kodi/`. Logs: `/switch/kodi/boot.txt` and
`/switch/kodi/.kodi/temp/kodi.log`.

## Not yet supported

Binary add-ons, hardware video decode, surround passthrough,
thumbnail encoders, event clients/JSON-RPC/web server, and automatic updates.

Upstream: [xbmc/xbmc](https://github.com/xbmc/xbmc)
