#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms
find lib/dlms-profile/include -name 'profile_status.hpp' -o -name 'profile_types.hpp' | head
echo ---
grep -l 'enum class ProfileStatus' lib/dlms-profile/include -r
