#!/usr/bin/env bash
set -euo pipefail
export PATH="/c/msys/mingw64/bin:/c/msys/usr/bin:$PATH"
cd /e/work/dlms
git add -A
git status --short
echo "---"
git diff --cached --stat
echo "---"
git commit -m "refactor(cosem)!: rename PRIME PLC MAC NetworkStatistics -> NetworkAdminData

The PRIME NB OFDM PLC MAC IC at class_id 85 was carrying the legacy
\"network statistics\" name in the built-in object surface. IEC
62056-6-2 ED4 (2021) §4.12.9 and DLMS UA Blue Book Ed. 12.1 name
this IC 'PRIME NB OFDM PLC MAC network administration data'. Rename
the C++ type and its internal constants to match the spec wording.

BREAKING (C++ API):
- CosemPrimePlcMacNetworkStatisticsObject ->
  CosemPrimePlcMacNetworkAdminDataObject

class_id (85), version (0), attribute layout, access semantics and
method behavior are unchanged. Public attribute accessors
(NodeRegistrations, NodeUnregistrations, ProcessedAliveMsgs,
HandledPromotions) are unchanged. No header path or include changes.

Refreshed unit tests, COSEM API guide, COSEM IC support matrix and
CHANGELOG. Bumped VERSION 0.92.0 -> 0.93.0.

dlms_cosem_tests: 273/273 green."
echo "---"
git log --oneline -3
