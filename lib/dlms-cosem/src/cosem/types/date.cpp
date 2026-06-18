#include "dlms/cosem/types/date.hpp"

namespace dlms {
namespace cosem {
namespace types {

const std::size_t Date::WireSize;
const std::uint16_t Date::YearUnspecifiedValue;
const std::uint16_t Date::YearMax;
const std::uint8_t Date::MonthUnspecifiedValue;
const std::uint8_t Date::MonthDstEndValue;
const std::uint8_t Date::MonthDstBeginValue;
const std::uint8_t Date::DayOfMonthUnspecifiedValue;
const std::uint8_t Date::DayOfMonthLastValue;
const std::uint8_t Date::DayOfMonthSecondLastValue;
const std::uint8_t Date::DayOfWeekUnspecifiedValue;

namespace {

bool IsValidMonth(std::uint8_t value)
{
  if (value == Date::MonthUnspecifiedValue) {
    return true;
  }
  if (value == Date::MonthDstEndValue ||
      value == Date::MonthDstBeginValue) {
    return true;
  }
  return value >= 1u && value <= 12u;
}

bool IsValidDayOfMonth(std::uint8_t value)
{
  if (value == Date::DayOfMonthUnspecifiedValue ||
      value == Date::DayOfMonthLastValue ||
      value == Date::DayOfMonthSecondLastValue) {
    return true;
  }
  // 0xE0..0xFC are reserved per spec and rejected.
  if (value >= 0xE0u) {
    return false;
  }
  return value >= 1u && value <= 31u;
}

bool IsValidDayOfWeek(std::uint8_t value)
{
  if (value == Date::DayOfWeekUnspecifiedValue) {
    return true;
  }
  return value >= 1u && value <= 7u;
}

} // namespace

Date::Date()
  : year_(YearUnspecifiedValue)
  , month_(MonthUnspecifiedValue)
  , dayOfMonth_(DayOfMonthUnspecifiedValue)
  , dayOfWeek_(DayOfWeekUnspecifiedValue)
{
}

bool Date::TryFromBytes(
  const std::uint8_t* data,
  std::size_t size,
  Date& out)
{
  if (data == nullptr || size != WireSize) {
    return false;
  }

  const std::uint16_t year =
    static_cast<std::uint16_t>(
      (static_cast<std::uint16_t>(data[0]) << 8) |
      static_cast<std::uint16_t>(data[1]));
  const std::uint8_t month = data[2];
  const std::uint8_t dayOfMonth = data[3];
  const std::uint8_t dayOfWeek = data[4];

  if (!IsValidMonth(month)) {
    return false;
  }
  if (!IsValidDayOfMonth(dayOfMonth)) {
    return false;
  }
  if (!IsValidDayOfWeek(dayOfWeek)) {
    return false;
  }

  out.year_ = year;
  out.month_ = month;
  out.dayOfMonth_ = dayOfMonth;
  out.dayOfWeek_ = dayOfWeek;
  return true;
}

std::array<std::uint8_t, Date::WireSize> Date::ToBytes() const
{
  std::array<std::uint8_t, WireSize> bytes;
  bytes[0] = static_cast<std::uint8_t>((year_ >> 8) & 0xFFu);
  bytes[1] = static_cast<std::uint8_t>(year_ & 0xFFu);
  bytes[2] = month_;
  bytes[3] = dayOfMonth_;
  bytes[4] = dayOfWeek_;
  return bytes;
}

std::uint16_t Date::Year() const { return year_; }
std::uint8_t Date::Month() const { return month_; }
std::uint8_t Date::DayOfMonth() const { return dayOfMonth_; }
std::uint8_t Date::DayOfWeek() const { return dayOfWeek_; }

bool Date::SetYear(std::uint16_t value)
{
  (void)YearMax;
  year_ = value;
  return true;
}

bool Date::SetMonth(std::uint8_t value)
{
  if (!IsValidMonth(value)) {
    return false;
  }
  month_ = value;
  return true;
}

bool Date::SetDayOfMonth(std::uint8_t value)
{
  if (!IsValidDayOfMonth(value)) {
    return false;
  }
  dayOfMonth_ = value;
  return true;
}

bool Date::SetDayOfWeek(std::uint8_t value)
{
  if (!IsValidDayOfWeek(value)) {
    return false;
  }
  dayOfWeek_ = value;
  return true;
}

bool Date::YearUnspecified() const
{
  return year_ == YearUnspecifiedValue;
}

bool Date::MonthUnspecified() const
{
  return month_ == MonthUnspecifiedValue;
}

bool Date::DayOfMonthUnspecified() const
{
  return dayOfMonth_ == DayOfMonthUnspecifiedValue;
}

bool Date::DayOfWeekUnspecified() const
{
  return dayOfWeek_ == DayOfWeekUnspecifiedValue;
}

bool Date::operator==(const Date& other) const
{
  return year_ == other.year_ &&
         month_ == other.month_ &&
         dayOfMonth_ == other.dayOfMonth_ &&
         dayOfWeek_ == other.dayOfWeek_;
}

bool Date::operator!=(const Date& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
