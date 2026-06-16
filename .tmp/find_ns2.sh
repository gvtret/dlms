#!/usr/bin/env bash
set -euo pipefail
grep -n 'namespace client\|namespace dlms' /e/work/dlms/lib/dlms-client/src/client/client.cpp | tail -10
