#!/usr/bin/env bash
set -u
cd /e/work/dlms
files=(
  CHANGELOG.md
  docs/ic_support_matrix.md
  lib/dlms-cosem/docs/01_cosem_api.md
  lib/dlms-cosem/docs/03_cosem_test_plan.md
  lib/dlms-cosem/include/dlms/cosem/simple_objects.hpp
  lib/dlms-cosem/src/cosem/simple_objects.cpp
  lib/dlms-cosem/test/cosem/test_simple_objects.cpp
)
for f in "${files[@]}"; do
  echo "=== $f ==="
  git diff -- "$f" | grep -E '^@@'
done
