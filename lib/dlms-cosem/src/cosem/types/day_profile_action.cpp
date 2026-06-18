#include "dlms/cosem/types/day_profile_action.hpp"

namespace dlms {
namespace cosem {
namespace types {

DayProfileAction::DayProfileAction()
  : startTime_()
  , scriptLogicalName_()
  , scriptSelector_(0u)
{
}

DayProfileAction::DayProfileAction(
  const Time& startTime,
  const CosemLogicalName& scriptLogicalName,
  std::uint16_t scriptSelector)
  : startTime_(startTime)
  , scriptLogicalName_(scriptLogicalName)
  , scriptSelector_(scriptSelector)
{
}

const Time& DayProfileAction::StartTime() const { return startTime_; }
const CosemLogicalName& DayProfileAction::ScriptLogicalName() const
{
  return scriptLogicalName_;
}
std::uint16_t DayProfileAction::ScriptSelector() const
{
  return scriptSelector_;
}

bool DayProfileAction::SetStartTime(const Time& value)
{
  // Per spec note (§4.5.5.2.5): start_time disallows wildcards.
  if (value.HourUnspecified() || value.MinuteUnspecified()
      || value.SecondUnspecified() || value.HundredthsUnspecified()) {
    return false;
  }
  startTime_ = value;
  return true;
}

void DayProfileAction::SetScriptLogicalName(const CosemLogicalName& value)
{
  scriptLogicalName_ = value;
}

void DayProfileAction::SetScriptSelector(std::uint16_t value)
{
  scriptSelector_ = value;
}

bool DayProfileAction::IsValid() const
{
  // No wildcards in start_time.
  return !startTime_.HourUnspecified()
      && !startTime_.MinuteUnspecified()
      && !startTime_.SecondUnspecified()
      && !startTime_.HundredthsUnspecified();
}

bool DayProfileAction::operator==(const DayProfileAction& other) const
{
  return startTime_ == other.startTime_
      && scriptLogicalName_ == other.scriptLogicalName_
      && scriptSelector_ == other.scriptSelector_;
}

bool DayProfileAction::operator!=(const DayProfileAction& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
