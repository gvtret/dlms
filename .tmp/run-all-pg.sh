#!/usr/bin/env bash
set -euo pipefail
export PATH=/mingw64/bin:/usr/bin:$PATH
export TMP=E:/work/dlms/build-mingw64/tmp
export TEMP=E:/work/dlms/build-mingw64/tmp
export TMPDIR=E:/work/dlms/build-mingw64/tmp
cd /e/work/dlms
./build-mingw64/lib/dlms-cosem/test/dlms_cosem_tests.exe 2>&1 | tail -5
echo === CTEST ===
ctest --test-dir build-mingw64 --output-on-failure 2>&1 | tail -10
