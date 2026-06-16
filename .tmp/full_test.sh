#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
cd /e/work/dlms/build-mingw64
time ninja.exe 2>&1 | tail -20
echo "---CTEST---"
ctest --output-on-failure 2>&1 | tail -20
