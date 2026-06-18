#pragma once

#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/types/time.hpp"

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of `day_profile_action` inside IC 20
// "Activity calendar" per IEC 62056-6-2 ED4 (2021) §4.5.5.2.5 and
// DLMS UA Blue Book Ed. 12.1 §5.1.9:
//
//   day_profile_action ::= structure
//   {
//     start_time:          octet-string(4) = time,
//     script_logical_name: octet-string(6) = OBIS,
//     script_selector:     long-unsigned
//   }
//
// Per the spec note, `start_time` does NOT allow wildcards (in
// contrast to `season_start`). The on-wire field is still a
// `types::Time`, but `IsValid()` rejects wildcards.
class DayProfileAction
{
public:
  DayProfileAction();
  DayProfileAction(
    const Time& startTime,
    const CosemLogicalName& scriptLogicalName,
    std::uint16_t scriptSelector);

  const Time& StartTime() const;
  const CosemLogicalName& ScriptLogicalName() const;
  std::uint16_t ScriptSelector() const;

  // SetStartTime rejects wildcard values per spec note.
  bool SetStartTime(const Time& value);
  void SetScriptLogicalName(const CosemLogicalName& value);
  void SetScriptSelector(std::uint16_t value);

  bool IsValid() const;

  bool operator==(const DayProfileAction& other) const;
  bool operator!=(const DayProfileAction& other) const;

private:
  Time startTime_;
  CosemLogicalName scriptLogicalName_;
  std::uint16_t scriptSelector_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
