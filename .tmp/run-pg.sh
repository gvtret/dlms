#!/usr/bin/env bash
set -euo pipefail
export PATH=/mingw64/bin:/usr/bin:$PATH
export TMP=E:/work/dlms/build-mingw64/tmp
export TEMP=E:/work/dlms/build-mingw64/tmp
export TMPDIR=E:/work/dlms/build-mingw64/tmp
mkdir -p "$TMPDIR"
cd /e/work/dlms
cmake --build build-mingw64 --target dlms_cosem_tests -- -j8 2>&1 | tail -30
echo === RUN ===
./build-mingw64/lib/dlms-cosem/test/dlms_cosem_tests.exe --gtest_filter='CosemProfileGenericObject.*'
