#pragma once

#include "dlms/cosem/types/script.hpp"

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the `action_set` structure used by the
// `actions` attribute of IC 21 "Register monitor"
// (class_id=21, version=0) per IEC 62056-6-2 ED4 (2021) §4.5.6.2.4
// and DLMS UA Blue Book Ed. 12.1 §4.5.6:
//
//   action_set ::= structure
//   {
//     action_up:   action_item,   // crosses threshold upwards
//     action_down: action_item    // crosses threshold downwards
//   }
//
//   action_item ::= structure
//   {
//     script_logical_name: octet-string (6 bytes, OBIS),
//     script_selector:     long-unsigned
//   }
//
// `action_item` is exactly `dlms::cosem::types::Script` (same wire
// shape: OBIS + selector). An "absent" action is conventionally a
// `Script` with the all-zero logical_name 0.0.0.0.0.0 and selector 0:
// the backend treats that as "no script to invoke" when the
// threshold is crossed in that direction.
class ActionSet
{
public:
  ActionSet();
  ActionSet(const Script& actionUp, const Script& actionDown);

  const Script& ActionUp() const;
  const Script& ActionDown() const;

  void SetActionUp(const Script& value);
  void SetActionDown(const Script& value);

  bool operator==(const ActionSet& other) const;
  bool operator!=(const ActionSet& other) const;

private:
  Script actionUp_;
  Script actionDown_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
