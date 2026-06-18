#pragma once

#include "dlms/cosem/cosem_types.hpp"

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the `action_specification` structure used by
// IC 9 "Script table" (class_id=9, version=0) per IEC 62056-6-2 ED4
// (2021) §4.5.2 and DLMS UA Blue Book Ed. 12.1 §4.5.2:
//
//   action_specification ::= structure
//   {
//     service_id:   enum,            // 1 = write attribute,
//                                    // 2 = execute specific method
//     class_id:     long-unsigned,
//     logical_name: octet-string,    // OBIS (6 bytes)
//     index:        integer,         // attribute (>=1) or method (>=1)
//                                    // index of the referenced object;
//                                    // 0 only valid in dummy entry
//     parameter:    service-specific // opaque AXDR-encoded value
//   }
//
// A "dummy" action specification with every element 0 means the action
// is not configured (spec NOTE 2 in §4.5.2.2.2). Such a dummy is
// represented by `service_id = ServiceId::Dummy` with class_id=0,
// logical_name = 0.0.0.0.0.0 / index=0 / empty parameter.
//
// `parameter` is opaque to this library: the script-execute side has to
// route it to the target object, so we preserve the encoded AXDR bytes
// verbatim. An empty buffer means "no parameter" on the wire.

enum class ScriptServiceId : std::uint8_t
{
  Dummy = 0u,
  WriteAttribute = 1u,
  ExecuteMethod = 2u,
};

class ActionSpecification
{
public:
  // service_id sentinel for "action not configured" entries.
  static const std::uint8_t ServiceIdDummyValue = 0u;
  static const std::uint8_t ServiceIdWriteAttributeValue = 1u;
  static const std::uint8_t ServiceIdExecuteMethodValue = 2u;

  // The COSEM index element is a signed `integer` (-128..127).
  static const std::int8_t IndexMin = -128;
  static const std::int8_t IndexMax = 127;

  ActionSpecification();
  ActionSpecification(
    ScriptServiceId serviceId,
    std::uint16_t classId,
    const CosemLogicalName& logicalName,
    std::int8_t index,
    const CosemByteBuffer& parameter);

  ScriptServiceId ServiceId() const;
  std::uint16_t ClassId() const;
  const CosemLogicalName& LogicalName() const;
  std::int8_t Index() const;
  const CosemByteBuffer& Parameter() const;

  void SetServiceId(ScriptServiceId value);
  void SetClassId(std::uint16_t value);
  void SetLogicalName(const CosemLogicalName& value);
  void SetIndex(std::int8_t value);
  void SetParameter(const CosemByteBuffer& value);

  // True for the all-zero "action not configured" sentinel.
  bool IsDummy() const;

  // Whole-entry validation per §4.5.2.2.2:
  //   * Dummy entries (service_id == Dummy) must have class_id == 0,
  //     logical_name == 0.0.0.0.0.0, index == 0 and empty parameter.
  //   * WriteAttribute / ExecuteMethod entries must have index >= 1
  //     (the spec numbers attributes and methods starting from 1).
  //   * Any other service_id value is invalid.
  static bool IsValid(const ActionSpecification& entry);

  bool operator==(const ActionSpecification& other) const;
  bool operator!=(const ActionSpecification& other) const;

private:
  ScriptServiceId serviceId_;
  std::uint16_t classId_;
  CosemLogicalName logicalName_;
  std::int8_t index_;
  CosemByteBuffer parameter_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
