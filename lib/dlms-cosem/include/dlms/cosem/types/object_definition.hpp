// SPDX-License-Identifier: BSD-2-Clause
//
// `dlms::cosem::types::ObjectDefinition` — typed view of the
// COSEM `object_definition` AXDR structure used by IC 6 Register
// Activation `register_assignment` (and reused by other ICs that
// reference COSEM objects by class+LN).
//
//   object_definition ::= structure {
//     class_id   long-unsigned,
//     logical_name octet-string(6)
//   }
//
// Per IEC 62056-6-2 ED4 (2021) §4.3.5 and DLMS UA Blue Book Ed. 12.1
// §4.3.5.
#pragma once

#include "dlms/cosem/cosem_types.hpp"

#include <cstdint>

namespace dlms::cosem::types {

class ObjectDefinition
{
public:
  ObjectDefinition() = default;
  ObjectDefinition(std::uint16_t classId, const CosemLogicalName& logicalName)
    : classId_(classId), logicalName_(logicalName)
  {
  }

  std::uint16_t ClassId() const { return classId_; }
  const CosemLogicalName& LogicalName() const { return logicalName_; }

  void SetClassId(std::uint16_t classId) { classId_ = classId; }
  void SetLogicalName(const CosemLogicalName& name) { logicalName_ = name; }

  bool operator==(const ObjectDefinition& other) const
  {
    return classId_ == other.classId_ && logicalName_ == other.logicalName_;
  }
  bool operator!=(const ObjectDefinition& other) const
  {
    return !(*this == other);
  }

private:
  std::uint16_t classId_{0u};
  CosemLogicalName logicalName_{};
};

}  // namespace dlms::cosem::types
