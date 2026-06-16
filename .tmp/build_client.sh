#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
cd /e/work/dlms/build-mingw64
time ninja.exe dlms_client_tests 2>&1 | tail -60
echo "EXIT=$?"
