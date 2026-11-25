#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/env.sh"

BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/switch}"

LINKER_SCRIPT="${DEVKITPRO}/libnx/switch.ld"
if ! cmp -s "${ROOT_DIR}/cmake/toolchain/switch.ld" "${LINKER_SCRIPT}"; then
  sudo -n cp "${ROOT_DIR}/cmake/toolchain/switch.ld" "${LINKER_SCRIPT}" 2>/dev/null || \
    echo "WARNING: install cmake/toolchain/switch.ld to ${LINKER_SCRIPT} with sudo before building"
fi

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}" \
  -DCORE_SYSTEM_NAME=switch \
  -DCORE_PLATFORM_NAME=switch \
  -DAPP_RENDER_SYSTEM=gles \
  -DENABLE_INTERNAL_FFMPEG=OFF \
  -DFFMPEG_PATH="${DEVKITPRO}/portlibs/switch" \
  -DKODI_DEPENDSBUILD=OFF \
  -DENABLE_INTERNAL_CURL=OFF \
  -DENABLE_INTERNAL_BLURAY=OFF \
  -DENABLE_INTERNAL_LIBDVD=OFF \
  -DENABLE_INTERNAL_ASS=OFF \
  -DENABLE_INTERNAL_FRIBIDI=OFF \
  -DENABLE_INTERNAL_TINYXML2=ON \
  -DENABLE_INTERNAL_PCRE2=OFF \
  -DENABLE_INTERNAL_FSTRCMP=ON \
  "$@"
