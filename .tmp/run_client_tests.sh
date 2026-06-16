#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
cd /e/work/dlms/build-mingw64
./lib/dlms-client/test/dlms_client_tests.exe --gtest_filter='MapDataLinkDisconnectStatus.*' 2>&1 | tail -40
echo "---ALL CLIENT---"
./lib/dlms-client/test/dlms_client_tests.exe 2>&1 | tail -10
