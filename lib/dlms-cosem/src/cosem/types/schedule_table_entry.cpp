#include "dlms/cosem/types/schedule_table_entry.hpp"

namespace dlms {
namespace cosem {
namespace types {

const std::uint8_t  ScheduleTableEntry::WeekdaysAll      = 0x7Fu;
const std::uint8_t  ScheduleTableEntry::WeekdaysBitWidth = 7u;
const std::uint8_t  ScheduleTableEntry::SpecdaysBitWidth = 64u;
const std::uint16_t ScheduleTableEntry::ValidityWindowAlways = 0xFFFFu;

ScheduleTableEntry::ScheduleTableEntry()
  : index_(0u)
  , enable_(false)
  , script_()
  , switchTime_()
  , validityWindow_(ValidityWindowAlways)
  , execWeekdays_(WeekdaysAll)
  , execSpecdays_(0u)
  , beginDate_()
  , endDate_()
{}

ScheduleTableEntry::ScheduleTableEntry(
  std::uint16_t index,
  bool enable,
  const Script& script,
  const Time& switchTime,
  std::uint16_t validityWindow,
  std::uint8_t execWeekdays,
  std::uint64_t execSpecdays,
  const Date& beginDate,
  const Date& endDate)
  : index_(index)
  , enable_(enable)
  , script_(script)
  , switchTime_(switchTime)
  , validityWindow_(validityWindow)
  , execWeekdays_(static_cast<std::uint8_t>(execWeekdays & WeekdaysAll))
  , execSpecdays_(execSpecdays)
  , beginDate_(beginDate)
  , endDate_(endDate)
{}

std::uint16_t   ScheduleTableEntry::Index() const          { return index_; }
bool            ScheduleTableEntry::Enable() const         { return enable_; }
const Script&   ScheduleTableEntry::GetScript() const      { return script_; }
const Time&     ScheduleTableEntry::SwitchTime() const     { return switchTime_; }
std::uint16_t   ScheduleTableEntry::ValidityWindow() const { return validityWindow_; }
std::uint8_t    ScheduleTableEntry::ExecWeekdays() const   { return execWeekdays_; }
std::uint64_t   ScheduleTableEntry::ExecSpecdays() const   { return execSpecdays_; }
const Date&     ScheduleTableEntry::BeginDate() const      { return beginDate_; }
const Date&     ScheduleTableEntry::EndDate() const        { return endDate_; }

void ScheduleTableEntry::SetIndex(std::uint16_t value)       { index_ = value; }
void ScheduleTableEntry::SetEnable(bool value)                { enable_ = value; }
void ScheduleTableEntry::SetScript(const Script& value)       { script_ = value; }
void ScheduleTableEntry::SetSwitchTime(const Time& value)     { switchTime_ = value; }
void ScheduleTableEntry::SetValidityWindow(std::uint16_t v)   { validityWindow_ = v; }
void ScheduleTableEntry::SetExecSpecdays(std::uint64_t value) { execSpecdays_ = value; }
void ScheduleTableEntry::SetBeginDate(const Date& value)      { beginDate_ = value; }
void ScheduleTableEntry::SetEndDate(const Date& value)        { endDate_ = value; }

bool ScheduleTableEntry::SetExecWeekdays(std::uint8_t value)
{
  // High bit (Sun.. wait — only 7 bits used) must be zero.
  if ((value & static_cast<std::uint8_t>(~WeekdaysAll)) != 0u) {
    return false;
  }
  execWeekdays_ = value;
  return true;
}

bool ScheduleTableEntry::IsValid(const ScheduleTableEntry& entry)
{
  if ((entry.execWeekdays_ & static_cast<std::uint8_t>(~WeekdaysAll)) != 0u) {
    return false;
  }
  return true;
}

bool ScheduleTableEntry::operator==(const ScheduleTableEntry& other) const
{
  return index_           == other.index_
      && enable_          == other.enable_
      && script_          == other.script_
      && switchTime_      == other.switchTime_
      && validityWindow_  == other.validityWindow_
      && execWeekdays_    == other.execWeekdays_
      && execSpecdays_    == other.execSpecdays_
      && beginDate_       == other.beginDate_
      && endDate_         == other.endDate_;
}

bool ScheduleTableEntry::operator!=(const ScheduleTableEntry& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
