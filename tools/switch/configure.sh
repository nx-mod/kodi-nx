#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/env.sh"

BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/switch}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}" \
  -DCORE_SYSTEM_NAME=switch \
  -DCORE_PLATFORM_NAME=switch \
  -DAPP_RENDER_SYSTEM=gles \
  -DENABLE_INTERNAL_FFMPEG=OFF \
  -DFFMPEG_PATH="${DEVKITPRO}/portlibs/switch" \
  -DKODI_DEPENDSBUILD=OFF \
  "$@"
