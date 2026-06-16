#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms
grep -rn 'MapDataLinkDisconnectStatus\|DisconnectDataLink' lib/dlms-client/test/ 2>/dev/null
echo ---
grep -rn 'ReleaseAssociation\|Close()' lib/dlms-client/test/ 2>/dev/null | head -30
