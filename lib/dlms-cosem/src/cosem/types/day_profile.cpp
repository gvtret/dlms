#include "dlms/cosem/types/day_profile.hpp"

namespace dlms {
namespace cosem {
namespace types {

namespace {

bool TimeStrictlyLess(const Time& a, const Time& b)
{
  if (a.Hour() != b.Hour()) {
    return a.Hour() < b.Hour();
  }
  if (a.Minute() != b.Minute()) {
    return a.Minute() < b.Minute();
  }
  if (a.Second() != b.Second()) {
    return a.Second() < b.Second();
  }
  return a.Hundredths() < b.Hundredths();
}

} // namespace

DayProfile::DayProfile()
  : dayId_(0u)
  , schedule_()
{
}

DayProfile::DayProfile(
  std::uint8_t dayId,
  const std::vector<DayProfileAction>& schedule)
  : dayId_(dayId)
  , schedule_()
{
  if (IsValidSchedule(schedule)) {
    schedule_ = schedule;
  }
}

std::uint8_t DayProfile::DayId() const { return dayId_; }
const std::vector<DayProfileAction>& DayProfile::DaySchedule() const
{
  return schedule_;
}

void DayProfile::SetDayId(std::uint8_t value) { dayId_ = value; }

bool DayProfile::SetDaySchedule(
  const std::vector<DayProfileAction>& value)
{
  if (!IsValidSchedule(value)) {
    return false;
  }
  schedule_ = value;
  return true;
}

bool DayProfile::IsValid() const
{
  return IsValidSchedule(schedule_);
}

bool DayProfile::IsValidSchedule(
  const std::vector<DayProfileAction>& schedule)
{
  for (std::size_t i = 0u; i < schedule.size(); ++i) {
    if (!schedule[i].IsValid()) {
      return false;
    }
    if (i > 0u
        && !TimeStrictlyLess(
             schedule[i - 1u].StartTime(), schedule[i].StartTime())) {
      return false;
    }
  }
  return true;
}

bool DayProfile::operator==(const DayProfile& other) const
{
  return dayId_ == other.dayId_ && schedule_ == other.schedule_;
}

bool DayProfile::operator!=(const DayProfile& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
