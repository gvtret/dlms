#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
rm -f /e/work/dlms/.tmp/dbg.obj /e/work/dlms/.tmp/c.log
time c++.exe \
  -IE:/work/dlms/lib/dlms-cosem/include \
  -IE:/work/dlms/lib/dlms-security/include \
  -c E:/work/dlms/lib/dlms-cosem/src/cosem/simple_objects.cpp \
  -o /e/work/dlms/.tmp/dbg.obj \
  > /e/work/dlms/.tmp/c.log 2>&1
RC=$?
echo "EXIT=$RC"
ls -la /e/work/dlms/.tmp/dbg.obj 2>&1 | head -1
echo "---log---"
tail -50 /e/work/dlms/.tmp/c.log
