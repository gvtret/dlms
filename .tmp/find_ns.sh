#!/usr/bin/env bash
set -euo pipefail
grep -n '^namespace\|^} // namespace\|^}$' /e/work/dlms/lib/dlms-client/src/client/client.cpp | head -40
