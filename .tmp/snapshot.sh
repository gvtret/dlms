#!/usr/bin/env bash
set -eu
cd /e/work/dlms
mkdir -p .tmp/wip
files=(
  CHANGELOG.md
  VERSION
  docs/ic_support_matrix.md
  lib/dlms-cosem/docs/01_cosem_api.md
  lib/dlms-cosem/docs/03_cosem_test_plan.md
  lib/dlms-cosem/include/dlms/cosem/simple_objects.hpp
  lib/dlms-cosem/src/cosem/simple_objects.cpp
  lib/dlms-cosem/test/cosem/test_simple_objects.cpp
)
for f in "${files[@]}"; do
  d=$(dirname ".tmp/wip/$f")
  mkdir -p "$d"
  cp "$f" ".tmp/wip/$f"
done
ls -R .tmp/wip
