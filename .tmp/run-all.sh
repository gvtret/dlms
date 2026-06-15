#!/usr/bin/env bash
cd /e/work/dlms/build-mingw64
./lib/dlms-cosem/test/dlms_cosem_tests.exe > /tmp/gt.out 2>&1
ec=$?
echo "EXIT=$ec"
echo "--- tail ---"
tail -40 /tmp/gt.out
echo "--- size: $(wc -l < /tmp/gt.out) lines ---"
