#!/usr/bin/env bash
cd /e/work/dlms
git show HEAD:docs/ic_support_matrix.md | awk '/^\| `40`/' > /tmp/ic40-head.line
wc -c /tmp/ic40-head.line
head -c 300 /tmp/ic40-head.line
echo
echo "---"
git diff docs/ic_support_matrix.md | head -10
