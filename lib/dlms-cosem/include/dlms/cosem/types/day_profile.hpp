#pragma once

#include "dlms/cosem/types/day_profile_action.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of `day_profile` inside IC 20 "Activity calendar"
// per IEC 62056-6-2 ED4 (2021) §4.5.5.2.5 and DLMS UA Blue Book Ed. 12.1
// §5.1.9:
//
//   day_profile ::= structure
//   {
//     day_id:       unsigned,
//     day_schedule: array of day_profile_action
//   }
//
// Invariants (enforced by SetDaySchedule / IsValid):
//   - every action is IsValid()
//   - start_time values are strictly ascending across the schedule
//     (the spec mandates sort-by-start_time AND each action represents
//     a distinct activation point, so equal start_time is rejected)
class DayProfile
{
public:
  DayProfile();
  DayProfile(
    std::uint8_t dayId,
    const std::vector<DayProfileAction>& schedule);

  std::uint8_t DayId() const;
  const std::vector<DayProfileAction>& DaySchedule() const;

  void SetDayId(std::uint8_t value);
  // Returns false (and does not mutate) if any action is invalid or if
  // start_time values are not strictly ascending.
  bool SetDaySchedule(const std::vector<DayProfileAction>& value);

  bool IsValid() const;

  bool operator==(const DayProfile& other) const;
  bool operator!=(const DayProfile& other) const;

  static bool IsValidSchedule(
    const std::vector<DayProfileAction>& schedule);

private:
  std::uint8_t dayId_;
  std::vector<DayProfileAction> schedule_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
