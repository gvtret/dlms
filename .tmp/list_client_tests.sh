#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms
find lib/dlms-client/test -type f 2>/dev/null
echo ---
ls lib/dlms-client/test 2>/dev/null
