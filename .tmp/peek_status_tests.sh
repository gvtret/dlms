#!/usr/bin/env bash
set -euo pipefail
grep -n 'TEST(DlmsClientStatus' /e/work/dlms/lib/dlms-client/test/client/test_client_status.cpp
echo ---
head -20 /e/work/dlms/lib/dlms-client/test/client/test_client_status.cpp
