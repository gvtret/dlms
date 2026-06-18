#pragma once

#include "dlms/cosem/cosem_types.hpp"

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the DLMS/COSEM `script` structure used by the
// `executed_script` attribute of IC 22 (Single action schedule), IC 19
// (Special days), IC 20 (Activity calendar) and the action_item element
// elsewhere in the Blue Book Ed. 12.1 / IEC 62056-6-2 ED4. The structure
// is:
//
//   script ::= structure
//   {
//     script_logical_name: octet-string (6 bytes, OBIS),
//     script_selector:     long-unsigned
//   }
//
// `script_logical_name` is reused as-is from `CosemLogicalName`. The
// selector is the script_identifier inside the referenced script table.
class Script
{
public:
  Script();
  Script(const CosemLogicalName& logicalName, std::uint16_t selector);

  const CosemLogicalName& LogicalName() const;
  std::uint16_t Selector() const;

  void SetLogicalName(const CosemLogicalName& value);
  void SetSelector(std::uint16_t value);

  bool operator==(const Script& other) const;
  bool operator!=(const Script& other) const;

private:
  CosemLogicalName logicalName_;
  std::uint16_t selector_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
