#!/usr/bin/env sh
# Release-verification driver for Linux (Ubuntu / WSL / native).
#
# Mirrors scripts/verify_release_mingw64.sh: clean build, full ctest,
# then build the CPack tarball. The MinGW64 variant is the canonical
# Windows-host pipeline; this script is the Linux counterpart that
# closed P1 Package §4 in v0.106.6+.
#
# Toolchain expectations:
#   * cmake >= 3.20, ninja
#   * gcc/g++ >= 12 (verified on gcc 15.2)
#   * OpenSSL development headers (libssl-dev / openssl-devel)
#
# Usage:
#   scripts/verify_release_linux.sh
#   (run from any cwd; uses the script's own location to find the repo)

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build-release-linux"
TMP_DIR="${BUILD_DIR}/tmp"

rm -rf "${BUILD_DIR}"
mkdir -p "${TMP_DIR}"

export TMPDIR="${TMP_DIR}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure
cmake --build "${BUILD_DIR}" --target package
