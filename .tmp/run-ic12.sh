#!/usr/bin/env bash
set -euo pipefail
export PATH=/mingw64/bin:/usr/bin:$PATH
export TMP=E:/work/dlms/build-mingw64/tmp
export TEMP=$TMP
export TMPDIR=$TMP
cd /e/work/dlms
cmake --build build-mingw64 --target dlms_cosem_tests -j8 2>&1 | tail -25
echo '=== ic12 filter ==='
./build-mingw64/lib/dlms-cosem/test/dlms_cosem_tests.exe --gtest_filter='CosemAssociationSnObject.*' 2>&1 | tail -30
