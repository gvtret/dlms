// SPDX-License-Identifier: BSD-2-Clause
#include "dlms/cosem/types/scaler_unit.hpp"

namespace dlms::cosem::types {

ScalerUnit::ScalerUnit()
  : scaler_(0)
  , unit_(NoUnit)
{
}

ScalerUnit::ScalerUnit(std::int8_t scaler, std::uint8_t unit)
  : scaler_(scaler)
  , unit_(unit)
{
}

}  // namespace dlms::cosem::types
