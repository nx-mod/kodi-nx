#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

source tools/switch/env.sh

BUILD_DIR="${BUILD_DIR:-$ROOT/build/switch}"
JOBS="${JOBS:-4}"

LINKER_SCRIPT="$DEVKITPRO/libnx/switch.ld"
if ! cmp -s cmake/toolchain/switch.ld "$LINKER_SCRIPT"; then
  echo "Installing patched Switch linker script to $LINKER_SCRIPT"
  if [ -w "$LINKER_SCRIPT" ]; then
    cp cmake/toolchain/switch.ld "$LINKER_SCRIPT"
  else
    sudo -n cp cmake/toolchain/switch.ld "$LINKER_SCRIPT" || {
      echo "Re-run with sudo access, or copy manually:" >&2
      echo "  sudo cp cmake/toolchain/switch.ld $LINKER_SCRIPT" >&2
      exit 1
    }
  fi
fi

if [ "${1:-}" = "--clean" ]; then
  rm -rf "$BUILD_DIR"
  shift
fi

if [ ! -x build/host/texturepacker/TexturePacker ]; then
  echo "Host TexturePacker is missing; building it" >&2
  cmake -S tools/depends/native/TexturePacker/src -B build/host/texturepacker \
    -G Ninja -DCMAKE_BUILD_TYPE=Release -DKODI_SOURCE_DIR="$ROOT" \
    -DCMAKE_C_FLAGS=-DTARGET_POSIX -DCMAKE_CXX_FLAGS=-DTARGET_POSIX
  cmake --build build/host/texturepacker -j4
fi

BUILD_DIR="$BUILD_DIR" ./tools/switch/configure.sh -DWITH_TEXTUREPACKER="$ROOT/build/host/texturepacker/TexturePacker" "$@"

cmake --build "$BUILD_DIR" -j"$JOBS"
python3 tools/switch/fix_nro_size.py "$BUILD_DIR/kodi.nro" "$BUILD_DIR/kodi.bin"

if [ -d /storage/emulated/0/Download/kodi-nx ]; then
  cp -a "$BUILD_DIR/kodi.nro" /storage/emulated/0/Download/kodi-nx/kodi.nro
fi

echo "Built $BUILD_DIR/kodi.nro"
