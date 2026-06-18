#include "dlms/cosem/types/special_day_entry.hpp"

namespace dlms {
namespace cosem {
namespace types {

SpecialDayEntry::SpecialDayEntry()
  : index_(0u)
  , date_()
  , dayId_(0u)
{
}

SpecialDayEntry::SpecialDayEntry(
  std::uint16_t index, const Date& date, std::uint8_t dayId)
  : index_(index)
  , date_(date)
  , dayId_(dayId)
{
}

std::uint16_t SpecialDayEntry::Index() const
{
  return index_;
}

const Date& SpecialDayEntry::SpecialDayDate() const
{
  return date_;
}

std::uint8_t SpecialDayEntry::DayId() const
{
  return dayId_;
}

void SpecialDayEntry::SetIndex(std::uint16_t value)
{
  index_ = value;
}

void SpecialDayEntry::SetSpecialDayDate(const Date& value)
{
  date_ = value;
}

void SpecialDayEntry::SetDayId(std::uint8_t value)
{
  dayId_ = value;
}

bool SpecialDayEntry::operator==(const SpecialDayEntry& other) const
{
  return index_ == other.index_ && date_ == other.date_ &&
         dayId_ == other.dayId_;
}

bool SpecialDayEntry::operator!=(const SpecialDayEntry& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
