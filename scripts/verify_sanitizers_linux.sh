#!/usr/bin/env sh
# Verify the framework under AddressSanitizer + UndefinedBehaviorSanitizer.
#
# Linux-only (clang). Builds a Debug tree with -fsanitize=address,undefined
# and runs the full ctest suite. Live meter smoke tests stay opt-out
# (DLMS_BUILD_LIVE_TESTS not set) and package install/artifact smoke tests
# are intentionally not exercised here: sanitizer builds are not packaged
# or installed.
#
# Intended for CI; safe to run locally on Linux with clang and ninja.
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build-sanitize-linux"

: "${CC:=clang}"
: "${CXX:=clang++}"

export CC CXX

# Symbolize sanitizer reports and fail fast on the first detection.
# We deliberately exclude package install/artifact smoke and live tests
# from the sanitized run; both are validated by the existing release job.
export ASAN_OPTIONS="halt_on_error=1:abort_on_error=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_leaks=0:print_summary=1"
export UBSAN_OPTIONS="halt_on_error=1:abort_on_error=1:print_stacktrace=1:print_summary=1"
export LSAN_OPTIONS="exitcode=23"

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DDLMS_SANITIZE=address,undefined \
  -DDLMS_INSTALL=OFF

cmake --build "${BUILD_DIR}"

# Exclude install/artifact smoke tests: they configure a separate consumer
# build that ignores -DDLMS_SANITIZE and would link against the un-sanitized
# install tree, which is not the contract this script verifies.
ctest --test-dir "${BUILD_DIR}" --output-on-failure \
  -E '^dlms_package_(install|artifact)_smoke$'
