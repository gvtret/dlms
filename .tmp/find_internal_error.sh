#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms
echo "=== Все return InternalError в lib/ ==="
grep -rn 'return.*InternalError\|return.*kInternalError\|= InternalError\|= kInternalError' lib/ \
  --include='*.cpp' --include='*.hpp' --include='*.h' \
  -- ':!**/test/**' ':!**/tests/**' 2>/dev/null | \
  grep -v '//' | \
  head -80
echo
echo "=== Counts per file ==="
grep -rl 'return.*InternalError\|return.*kInternalError' lib/ \
  --include='*.cpp' --include='*.hpp' --include='*.h' \
  -- ':!**/test/**' ':!**/tests/**' 2>/dev/null | \
  xargs -I {} sh -c 'echo "$(grep -c "return.*InternalError\|return.*kInternalError" "{}") {}"' | sort -rn | head -30
