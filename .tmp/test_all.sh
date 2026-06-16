#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
cd /e/work/dlms/build-mingw64
./lib/dlms-cosem/test/dlms_cosem_tests.exe 2>&1 | tail -15
echo "EXIT=$?"
