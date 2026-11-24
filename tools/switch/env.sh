#!/usr/bin/env bash
set -e

source /etc/profile.d/devkit-env.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

export DEVKITA64="${DEVKITPRO}/devkitA64"
export PATH="${DEVKITA64}/bin:${DEVKITPRO}/tools/bin:${PATH}"
export LIBNX="${LIBNX:-${ROOT_DIR}/../libnx/nx}"

export PKG_CONFIG_LIBDIR="${DEVKITPRO}/portlibs/switch/lib/pkgconfig"
export PKG_CONFIG_PATH="${PKG_CONFIG_LIBDIR}"

export CMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/cmake/toolchain/Switch.cmake"

if [[ ! -f "${LIBNX}/lib/libnx.a" ]]; then
  echo "libnx not built at ${LIBNX}; build ../libnx first" >&2
  exit 1
fi

echo "Switch toolchain: ${DEVKITA64}"
echo "libnx: ${LIBNX}"
echo "portlibs: ${DEVKITPRO}/portlibs/switch"
