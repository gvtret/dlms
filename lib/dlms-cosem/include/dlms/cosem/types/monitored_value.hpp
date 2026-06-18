#pragma once

#include "dlms/cosem/cosem_types.hpp"

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the `value_definition` structure used by
// the `monitored_value` attribute of IC 21 "Register monitor"
// (class_id=21, version=0) per IEC 62056-6-2 ED4 (2021) §4.5.6.2.3
// and DLMS UA Blue Book Ed. 12.1 §4.5.6:
//
//   value_definition ::= structure
//   {
//     class_id:        long-unsigned,
//     logical_name:    octet-string (6 bytes, OBIS),
//     attribute_index: integer
//   }
//
// Identifies which attribute of which object is being monitored. The
// spec restricts the referenced attribute to one with a "simple" data
// type, but that constraint is enforced by the backend that resolves
// the reference, not by this typed value.
class MonitoredValue
{
public:
  // Attribute indices are 1-based in COSEM; 0 would denote "the whole
  // object" which is not a valid target for monitoring.
  static const std::int8_t AttributeIndexMin = 1;
  static const std::int8_t AttributeIndexMax = 127;

  MonitoredValue();
  MonitoredValue(
    std::uint16_t classId,
    const CosemLogicalName& logicalName,
    std::int8_t attributeIndex);

  std::uint16_t ClassId() const;
  const CosemLogicalName& LogicalName() const;
  std::int8_t AttributeIndex() const;

  void SetClassId(std::uint16_t value);
  void SetLogicalName(const CosemLogicalName& value);
  // Returns false (no mutation) when value < AttributeIndexMin.
  bool SetAttributeIndex(std::int8_t value);

  // Whole-value validation: attribute_index >= 1.
  static bool IsValid(const MonitoredValue& value);

  bool operator==(const MonitoredValue& other) const;
  bool operator!=(const MonitoredValue& other) const;

private:
  std::uint16_t classId_;
  CosemLogicalName logicalName_;
  std::int8_t attributeIndex_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
