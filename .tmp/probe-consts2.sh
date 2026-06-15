#!/bin/bash
cd /e/work/dlms
for sym in kUtilityTables kRegisterTable kCompactData kStatusMapping kParameterMonitor kSensorManager kArbitrator; do
  echo "=== $sym ==="
  grep -nE "$sym" lib/dlms-cosem/src/cosem/simple_objects.cpp | head -35
done
