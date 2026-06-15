#!/bin/bash
cd /e/work/dlms
for sym in kModemConfiguration kAutoConnect kAutoAnswer kIecTwistedPair kMBusSlavePortSetup; do
  echo "=== $sym ==="
  grep -nE "$sym" lib/dlms-cosem/src/cosem/simple_objects.cpp | head -25
done
