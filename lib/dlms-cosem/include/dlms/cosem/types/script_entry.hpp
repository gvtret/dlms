#pragma once

#include "dlms/cosem/types/action_specification.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of a single `script` table entry as defined by
// IC 9 "Script table" (class_id=9, version=0) per IEC 62056-6-2 ED4
// (2021) §4.5.2 and DLMS UA Blue Book Ed. 12.1 §4.5.2:
//
//   script ::= structure
//   {
//     script_identifier: long-unsigned,
//     actions:           array action_specification
//   }
//
// `script_identifier == 0` is the reserved null script: invoking
// `execute(0)` performs no actions, regardless of `actions` content.
// The library still accepts and round-trips an arbitrary `actions`
// list for `script_identifier == 0` (the spec does not forbid it; it
// merely says the actions will not be executed).
class ScriptEntry
{
public:
  static const std::uint16_t NullScriptIdentifier = 0u;

  ScriptEntry();
  ScriptEntry(
    std::uint16_t identifier,
    const std::vector<ActionSpecification>& actions);

  std::uint16_t Identifier() const;
  const std::vector<ActionSpecification>& Actions() const;

  void SetIdentifier(std::uint16_t value);
  // Returns false (no mutation) when any action fails IsValid().
  bool SetActions(const std::vector<ActionSpecification>& value);

  // Whole-entry validation: every action_specification must satisfy
  // ActionSpecification::IsValid(). Empty `actions` is allowed
  // (matches the null script and "configured but no actions" cases).
  static bool IsValid(const ScriptEntry& entry);

  bool operator==(const ScriptEntry& other) const;
  bool operator!=(const ScriptEntry& other) const;

private:
  std::uint16_t identifier_;
  std::vector<ActionSpecification> actions_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
