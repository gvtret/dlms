#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms

files=(
  lib/dlms-cosem/include/dlms/cosem/simple_objects.hpp
  lib/dlms-cosem/src/cosem/simple_objects.cpp
  lib/dlms-cosem/test/cosem/test_simple_objects.cpp
)

for f in "${files[@]}"; do
  sed -i \
    -e 's/CosemPrimePlcMacNetworkStatisticsObject/CosemPrimePlcMacNetworkAdminDataObject/g' \
    -e 's/kPrimePlcMacNetStatsClassId/kPrimePlcMacNetworkAdminDataClassId/g' \
    -e 's/kPrimePlcMacNetStatsNodeRegistrationsId/kPrimePlcMacNetworkAdminDataNodeRegistrationsId/g' \
    -e 's/kPrimePlcMacNetStatsNodeUnregistrationsId/kPrimePlcMacNetworkAdminDataNodeUnregistrationsId/g' \
    -e 's/kPrimePlcMacNetStatsProcessedAliveMsgsId/kPrimePlcMacNetworkAdminDataProcessedAliveMsgsId/g' \
    -e 's/kPrimePlcMacNetStatsHandledPromotionsId/kPrimePlcMacNetworkAdminDataHandledPromotionsId/g' \
    -e 's/kPrimePlcMacNetStatsResetMethodId/kPrimePlcMacNetworkAdminDataResetMethodId/g' \
    -e 's/PrimePlcMacNetStatsBuffers/PrimePlcMacNetworkAdminDataBuffers/g' \
    -e 's/MakeSamplePrimePlcMacNetStats/MakeSamplePrimePlcMacNetworkAdminData/g' \
    -e 's/MakePrimePlcMacNetStatsObject/MakePrimePlcMacNetworkAdminDataObject/g' \
    "$f"
done
echo OK
