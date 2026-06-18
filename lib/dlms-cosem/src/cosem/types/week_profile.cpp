#include "dlms/cosem/types/week_profile.hpp"

namespace dlms {
namespace cosem {
namespace types {

WeekProfile::WeekProfile()
  : name_()
  , monday_(0u)
  , tuesday_(0u)
  , wednesday_(0u)
  , thursday_(0u)
  , friday_(0u)
  , saturday_(0u)
  , sunday_(0u)
{
}

WeekProfile::WeekProfile(
  const CosemByteBuffer& name,
  std::uint8_t monday,
  std::uint8_t tuesday,
  std::uint8_t wednesday,
  std::uint8_t thursday,
  std::uint8_t friday,
  std::uint8_t saturday,
  std::uint8_t sunday)
  : name_(name)
  , monday_(monday)
  , tuesday_(tuesday)
  , wednesday_(wednesday)
  , thursday_(thursday)
  , friday_(friday)
  , saturday_(saturday)
  , sunday_(sunday)
{
}

const CosemByteBuffer& WeekProfile::Name() const { return name_; }
std::uint8_t WeekProfile::Monday() const { return monday_; }
std::uint8_t WeekProfile::Tuesday() const { return tuesday_; }
std::uint8_t WeekProfile::Wednesday() const { return wednesday_; }
std::uint8_t WeekProfile::Thursday() const { return thursday_; }
std::uint8_t WeekProfile::Friday() const { return friday_; }
std::uint8_t WeekProfile::Saturday() const { return saturday_; }
std::uint8_t WeekProfile::Sunday() const { return sunday_; }

void WeekProfile::SetName(const CosemByteBuffer& value) { name_ = value; }
void WeekProfile::SetMonday(std::uint8_t value) { monday_ = value; }
void WeekProfile::SetTuesday(std::uint8_t value) { tuesday_ = value; }
void WeekProfile::SetWednesday(std::uint8_t value) { wednesday_ = value; }
void WeekProfile::SetThursday(std::uint8_t value) { thursday_ = value; }
void WeekProfile::SetFriday(std::uint8_t value) { friday_ = value; }
void WeekProfile::SetSaturday(std::uint8_t value) { saturday_ = value; }
void WeekProfile::SetSunday(std::uint8_t value) { sunday_ = value; }

bool WeekProfile::operator==(const WeekProfile& other) const
{
  return name_ == other.name_
      && monday_ == other.monday_
      && tuesday_ == other.tuesday_
      && wednesday_ == other.wednesday_
      && thursday_ == other.thursday_
      && friday_ == other.friday_
      && saturday_ == other.saturday_
      && sunday_ == other.sunday_;
}

bool WeekProfile::operator!=(const WeekProfile& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
