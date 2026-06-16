#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
cd /e/work/dlms/build-mingw64
# нужны все цели, чтобы линковка тестов прошла
time ninja.exe dlms_cosem_tests dlms_cosem 2>&1 | tail -80
echo "EXIT=$?"
