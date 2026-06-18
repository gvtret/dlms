// SPDX-License-Identifier: BSD-2-Clause
//
// `types::ScalerUnit` — typed representation of the `scal_unit_type`
// structure used by IC "Register" (class_id=3), "Extended register"
// (class_id=4), "Demand register" (class_id=5) and others, per
// IEC 62056-6-2 ED4 (2021) §4.3.2.2.3 and DLMS UA Blue Book Ed. 12.1
// §4.3.2.2.3.
//
//   scal_unit_type ::= structure
//   {
//     scaler: integer,    -- exponent (base 10) of the multiplication
//                         -- factor; 0 when value is not numerical.
//     unit:   enum        -- physical unit; see Table 5 in the spec.
//                         -- 255 = "no unit" / not used.
//   }
//
// The AXDR codec is provided by the consuming IC; this header carries
// only the typed POD plus simple invariants.
#pragma once

#include <cstdint>

namespace dlms::cosem::types {

class ScalerUnit
{
public:
  // "No unit" sentinel from Blue Book Table 5: when set together with
  // scaler=0 the IC "Register" value attribute may use any of the
  // simple data types allowed for IC "Data".
  static constexpr std::uint8_t NoUnit = 255u;

  // Default-constructed value: scaler=0, unit=NoUnit (i.e. the
  // "scaler_unit not used" case from the spec).
  ScalerUnit();
  ScalerUnit(std::int8_t scaler, std::uint8_t unit);

  std::int8_t Scaler() const { return scaler_; }
  std::uint8_t Unit() const { return unit_; }

  void SetScaler(std::int8_t scaler) { scaler_ = scaler; }
  void SetUnit(std::uint8_t unit) { unit_ = unit; }

  bool operator==(const ScalerUnit& other) const
  {
    return scaler_ == other.scaler_ && unit_ == other.unit_;
  }
  bool operator!=(const ScalerUnit& other) const { return !(*this == other); }

private:
  std::int8_t scaler_;
  std::uint8_t unit_;
};

}  // namespace dlms::cosem::types
