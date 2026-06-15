#!/usr/bin/env bash
export PATH=/mingw64/bin:/usr/bin:$PATH
cd /e/work/dlms/build-mingw64
./lib/dlms-cosem/test/dlms_cosem_tests.exe \
  --gtest_filter='CosemPushSetupObject.*' 2>&1 | tail -25
echo "EXIT=$?"
