#include "dlms/cosem/types/time.hpp"

namespace dlms {
namespace cosem {
namespace types {

const std::size_t Time::WireSize;
const std::uint8_t Time::HourUnspecifiedValue;
const std::uint8_t Time::MinuteUnspecifiedValue;
const std::uint8_t Time::SecondUnspecifiedValue;
const std::uint8_t Time::HundredthsUnspecifiedValue;

namespace {

bool IsValidHourByte(std::uint8_t value)
{
  return value == Time::HourUnspecifiedValue || value <= 23u;
}

bool IsValidMinuteByte(std::uint8_t value)
{
  return value == Time::MinuteUnspecifiedValue || value <= 59u;
}

bool IsValidSecondByte(std::uint8_t value)
{
  return value == Time::SecondUnspecifiedValue || value <= 59u;
}

bool IsValidHundredthsByte(std::uint8_t value)
{
  return value == Time::HundredthsUnspecifiedValue || value <= 99u;
}

} // namespace

Time::Time()
  : hour_(HourUnspecifiedValue)
  , minute_(MinuteUnspecifiedValue)
  , second_(SecondUnspecifiedValue)
  , hundredths_(HundredthsUnspecifiedValue)
{
}

bool Time::TryFromBytes(
  const std::uint8_t* data,
  std::size_t size,
  Time& out)
{
  if (data == nullptr || size != WireSize) {
    return false;
  }

  const std::uint8_t hour = data[0];
  const std::uint8_t minute = data[1];
  const std::uint8_t second = data[2];
  const std::uint8_t hundredths = data[3];

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

  out.hour_ = hour;
  out.minute_ = minute;
  out.second_ = second;
  out.hundredths_ = hundredths;
  return true;
}

std::array<std::uint8_t, Time::WireSize> Time::ToBytes() const
{
  std::array<std::uint8_t, WireSize> bytes;
  bytes[0] = hour_;
  bytes[1] = minute_;
  bytes[2] = second_;
  bytes[3] = hundredths_;
  return bytes;
}

std::uint8_t Time::Hour() const { return hour_; }
std::uint8_t Time::Minute() const { return minute_; }
std::uint8_t Time::Second() const { return second_; }
std::uint8_t Time::Hundredths() const { return hundredths_; }

bool Time::SetHour(std::uint8_t value)
{
  if (!IsValidHourByte(value)) {
    return false;
  }
  hour_ = value;
  return true;
}

bool Time::SetMinute(std::uint8_t value)
{
  if (!IsValidMinuteByte(value)) {
    return false;
  }
  minute_ = value;
  return true;
}

bool Time::SetSecond(std::uint8_t value)
{
  if (!IsValidSecondByte(value)) {
    return false;
  }
  second_ = value;
  return true;
}

bool Time::SetHundredths(std::uint8_t value)
{
  if (!IsValidHundredthsByte(value)) {
    return false;
  }
  hundredths_ = value;
  return true;
}

bool Time::HourUnspecified() const
{
  return hour_ == HourUnspecifiedValue;
}

bool Time::MinuteUnspecified() const
{
  return minute_ == MinuteUnspecifiedValue;
}

bool Time::SecondUnspecified() const
{
  return second_ == SecondUnspecifiedValue;
}

bool Time::HundredthsUnspecified() const
{
  return hundredths_ == HundredthsUnspecifiedValue;
}

bool Time::operator==(const Time& other) const
{
  return hour_ == other.hour_ &&
         minute_ == other.minute_ &&
         second_ == other.second_ &&
         hundredths_ == other.hundredths_;
}

bool Time::operator!=(const Time& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
