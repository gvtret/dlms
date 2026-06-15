#!/bin/bash
set -e
cd /e/work/dlms
for sym in ImageTransfer ActivityCalendar IecTwistedPairSetup AutoAnswer AutoConnect DataProtection UtilityTables; do
  echo "=========== ${sym} ==========="
  grep -n "k${sym}ClassId\|k${sym}[A-Z][A-Za-z]*AttributeId\|k${sym}[A-Z][A-Za-z]*MethodId" lib/dlms-cosem/src/cosem/simple_objects.cpp | head -30
  echo "--- InvokeMethod ---"
  awk "/^CosemStatus Cosem${sym}Object::InvokeMethod/,/^}/" lib/dlms-cosem/src/cosem/simple_objects.cpp
  echo "--- class header ---"
  grep -n -A3 "class Cosem${sym}Object" lib/dlms-cosem/include/dlms/cosem/simple_objects.hpp | head -8
done
