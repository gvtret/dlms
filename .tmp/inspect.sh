#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
cd /e/work/dlms
echo "--- .gitignore tmp/wip rules ---"
grep -nE 'tmp|wip' .gitignore || echo "(no match)"
echo "--- prior commits touching .tmp/ ---"
git log --oneline -- .tmp 2>&1 | head -10
