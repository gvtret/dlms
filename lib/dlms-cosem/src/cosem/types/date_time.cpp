#include "dlms/cosem/types/date_time.hpp"

namespace dlms {
namespace cosem {
namespace types {

const std::size_t DateTime::WireSize;
const std::uint16_t DateTime::YearUnspecifiedValue;
const std::uint16_t DateTime::YearMax;
const std::uint8_t DateTime::MonthUnspecifiedValue;
const std::uint8_t DateTime::MonthDstEndValue;
const std::uint8_t DateTime::MonthDstBeginValue;
const std::uint8_t DateTime::DayOfMonthUnspecifiedValue;
const std::uint8_t DateTime::DayOfMonthLastValue;
const std::uint8_t DateTime::DayOfMonthSecondLastValue;
const std::uint8_t DateTime::DayOfWeekUnspecifiedValue;
const std::uint8_t DateTime::HourUnspecifiedValue;
const std::uint8_t DateTime::MinuteUnspecifiedValue;
const std::uint8_t DateTime::SecondUnspecifiedValue;
const std::uint8_t DateTime::HundredthsUnspecifiedValue;
const std::int16_t DateTime::DeviationUnspecifiedValue;
const std::int16_t DateTime::DeviationMin;
const std::int16_t DateTime::DeviationMax;
const std::uint8_t DateTime::ClockStatusUnspecifiedValue;
const std::uint8_t DateTime::ClockStatusInvalidValueBit;
const std::uint8_t DateTime::ClockStatusDoubtfulValueBit;
const std::uint8_t DateTime::ClockStatusDifferentClockBaseBit;
const std::uint8_t DateTime::ClockStatusInvalidClockStatusBit;
const std::uint8_t DateTime::ClockStatusDaylightSavingActiveBit;
const std::uint8_t DateTime::ClockStatusReservedMask;

namespace {

bool IsValidMonth(std::uint8_t value)
{
  if (value == DateTime::MonthUnspecifiedValue) {
    return true;
  }
  if (value == DateTime::MonthDstEndValue ||
      value == DateTime::MonthDstBeginValue) {
    return true;
  }
  return value >= 1u && value <= 12u;
}

bool IsValidDayOfMonth(std::uint8_t value)
{
  if (value == DateTime::DayOfMonthUnspecifiedValue ||
      value == DateTime::DayOfMonthLastValue ||
      value == DateTime::DayOfMonthSecondLastValue) {
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
  if (value == DateTime::DayOfWeekUnspecifiedValue) {
    return true;
  }
  return value >= 1u && value <= 7u;
}

bool IsValidHourByte(std::uint8_t value)
{
  return value == DateTime::HourUnspecifiedValue || value <= 23u;
}

bool IsValidMinuteByte(std::uint8_t value)
{
  return value == DateTime::MinuteUnspecifiedValue || value <= 59u;
}

bool IsValidSecondByte(std::uint8_t value)
{
  return value == DateTime::SecondUnspecifiedValue || value <= 59u;
}

bool IsValidHundredthsByte(std::uint8_t value)
{
  return value == DateTime::HundredthsUnspecifiedValue || value <= 99u;
}

bool IsValidDeviation(std::int16_t value)
{
  if (value == DateTime::DeviationUnspecifiedValue) {
    return true;
  }
  return value >= DateTime::DeviationMin && value <= DateTime::DeviationMax;
}

} // namespace

DateTime::DateTime()
  : year_(YearUnspecifiedValue)
  , month_(MonthUnspecifiedValue)
  , dayOfMonth_(DayOfMonthUnspecifiedValue)
  , dayOfWeek_(DayOfWeekUnspecifiedValue)
  , hour_(HourUnspecifiedValue)
  , minute_(MinuteUnspecifiedValue)
  , second_(SecondUnspecifiedValue)
  , hundredths_(HundredthsUnspecifiedValue)
  , deviation_(DeviationUnspecifiedValue)
  , clockStatus_(ClockStatusUnspecifiedValue)
{
}

bool DateTime::TryFromBytes(
  const std::uint8_t* data,
  std::size_t size,
  DateTime& out)
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
  const std::uint8_t hour = data[5];
  const std::uint8_t minute = data[6];
  const std::uint8_t second = data[7];
  const std::uint8_t hundredths = data[8];
  const std::int16_t deviation =
    static_cast<std::int16_t>(
      (static_cast<std::uint16_t>(data[9]) << 8) |
      static_cast<std::uint16_t>(data[10]));
  const std::uint8_t clockStatus = data[11];

  if (!IsValidMonth(month)) {
    return false;
  }
  if (!IsValidDayOfMonth(dayOfMonth)) {
    return false;
  }
  if (!IsValidDayOfWeek(dayOfWeek)) {
    return false;
  }
  if (!IsValidHourByte(hour)) {
    return false;
  }
  if (!IsValidMinuteByte(minute)) {
    return false;
  }
  if (!IsValidSecondByte(second)) {
    return false;
  }
  if (!IsValidHundredthsByte(hundredths)) {
    return false;
  }
  if (!IsValidDeviation(deviation)) {
    return false;
  }

  out.year_ = year;
  out.month_ = month;
  out.dayOfMonth_ = dayOfMonth;
  out.dayOfWeek_ = dayOfWeek;
  out.hour_ = hour;
  out.minute_ = minute;
  out.second_ = second;
  out.hundredths_ = hundredths;
  out.deviation_ = deviation;
  out.clockStatus_ = clockStatus;
  return true;
}

std::array<std::uint8_t, DateTime::WireSize> DateTime::ToBytes() const
{
  std::array<std::uint8_t, WireSize> bytes;
  bytes[0] = static_cast<std::uint8_t>((year_ >> 8) & 0xFFu);
  bytes[1] = static_cast<std::uint8_t>(year_ & 0xFFu);
  bytes[2] = month_;
  bytes[3] = dayOfMonth_;
  bytes[4] = dayOfWeek_;
  bytes[5] = hour_;
  bytes[6] = minute_;
  bytes[7] = second_;
  bytes[8] = hundredths_;
  const std::uint16_t deviationBits =
    static_cast<std::uint16_t>(deviation_);
  bytes[9] = static_cast<std::uint8_t>((deviationBits >> 8) & 0xFFu);
  bytes[10] = static_cast<std::uint8_t>(deviationBits & 0xFFu);
  bytes[11] = clockStatus_;
  return bytes;
}

std::uint16_t DateTime::Year() const { return year_; }
std::uint8_t DateTime::Month() const { return month_; }
std::uint8_t DateTime::DayOfMonth() const { return dayOfMonth_; }
std::uint8_t DateTime::DayOfWeek() const { return dayOfWeek_; }
std::uint8_t DateTime::Hour() const { return hour_; }
std::uint8_t DateTime::Minute() const { return minute_; }
std::uint8_t DateTime::Second() const { return second_; }
std::uint8_t DateTime::Hundredths() const { return hundredths_; }
std::int16_t DateTime::Deviation() const { return deviation_; }
std::uint8_t DateTime::ClockStatus() const { return clockStatus_; }

bool DateTime::SetYear(std::uint16_t value)
{
  // Year accepts 0x0000..0xFFFE plus the explicit 0xFFFF wildcard. Every
  // representable value is in-range; helper retained for API symmetry and
  // future tightening (e.g. real-calendar lower bound).
  (void)YearMax;
  year_ = value;
  return true;
}

bool DateTime::SetMonth(std::uint8_t value)
{
  if (!IsValidMonth(value)) {
    return false;
  }
  month_ = value;
  return true;
}

bool DateTime::SetDayOfMonth(std::uint8_t value)
{
  if (!IsValidDayOfMonth(value)) {
    return false;
  }
  dayOfMonth_ = value;
  return true;
}

bool DateTime::SetDayOfWeek(std::uint8_t value)
{
  if (!IsValidDayOfWeek(value)) {
    return false;
  }
  dayOfWeek_ = value;
  return true;
}

bool DateTime::SetHour(std::uint8_t value)
{
  if (!IsValidHourByte(value)) {
    return false;
  }
  hour_ = value;
  return true;
}

bool DateTime::SetMinute(std::uint8_t value)
{
  if (!IsValidMinuteByte(value)) {
    return false;
  }
  minute_ = value;
  return true;
}

bool DateTime::SetSecond(std::uint8_t value)
{
  if (!IsValidSecondByte(value)) {
    return false;
  }
  second_ = value;
  return true;
}

bool DateTime::SetHundredths(std::uint8_t value)
{
  if (!IsValidHundredthsByte(value)) {
    return false;
  }
  hundredths_ = value;
  return true;
}

bool DateTime::SetDeviation(std::int16_t value)
{
  if (!IsValidDeviation(value)) {
    return false;
  }
  deviation_ = value;
  return true;
}

void DateTime::SetClockStatus(std::uint8_t value)
{
  clockStatus_ = value;
}

bool DateTime::YearUnspecified() const
{
  return year_ == YearUnspecifiedValue;
}

bool DateTime::MonthUnspecified() const
{
  return month_ == MonthUnspecifiedValue;
}

bool DateTime::DayOfMonthUnspecified() const
{
  return dayOfMonth_ == DayOfMonthUnspecifiedValue;
}

bool DateTime::DayOfWeekUnspecified() const
{
  return dayOfWeek_ == DayOfWeekUnspecifiedValue;
}

bool DateTime::HourUnspecified() const
{
  return hour_ == HourUnspecifiedValue;
}

bool DateTime::MinuteUnspecified() const
{
  return minute_ == MinuteUnspecifiedValue;
}

bool DateTime::SecondUnspecified() const
{
  return second_ == SecondUnspecifiedValue;
}

bool DateTime::HundredthsUnspecified() const
{
  return hundredths_ == HundredthsUnspecifiedValue;
}

bool DateTime::DeviationUnspecified() const
{
  return deviation_ == DeviationUnspecifiedValue;
}

bool DateTime::ClockStatusUnspecified() const
{
  return clockStatus_ == ClockStatusUnspecifiedValue;
}

bool DateTime::operator==(const DateTime& other) const
{
  return year_ == other.year_ &&
         month_ == other.month_ &&
         dayOfMonth_ == other.dayOfMonth_ &&
         dayOfWeek_ == other.dayOfWeek_ &&
         hour_ == other.hour_ &&
         minute_ == other.minute_ &&
         second_ == other.second_ &&
         hundredths_ == other.hundredths_ &&
         deviation_ == other.deviation_ &&
         clockStatus_ == other.clockStatus_;
}

bool DateTime::operator!=(const DateTime& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
