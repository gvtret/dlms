#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build-release-mingw64"
TMP_DIR="${BUILD_DIR}/tmp"

case "${ROOT_DIR}" in
  /?/*) ;;
  *)
    echo "Unsupported root path: ${ROOT_DIR}" >&2
    exit 1
    ;;
esac

rm -rf "${BUILD_DIR}"
mkdir -p "${TMP_DIR}"

export PATH="/mingw64/bin:/usr/bin:${PATH}"
export TMP="${TMP_DIR}"
export TEMP="${TMP_DIR}"
export TMPDIR="${TMP_DIR}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja
cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure
cmake --build "${BUILD_DIR}" --target package
