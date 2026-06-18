#include "dlms/cosem/types/single_action_schedule_type.hpp"

namespace dlms {
namespace cosem {
namespace types {

const std::uint8_t SingleActionScheduleType::MinValue;
const std::uint8_t SingleActionScheduleType::MaxValue;

SingleActionScheduleType::SingleActionScheduleType()
  : value_(MinValue)
{
}

SingleActionScheduleType::SingleActionScheduleType(std::uint8_t value)
  : value_(IsValid(value) ? value : MinValue)
{
}

bool SingleActionScheduleType::IsValid(std::uint8_t value)
{
  return value >= MinValue && value <= MaxValue;
}

std::uint8_t SingleActionScheduleType::Value() const { return value_; }

bool SingleActionScheduleType::SetValue(std::uint8_t value)
{
  if (!IsValid(value)) {
    return false;
  }
  value_ = value;
  return true;
}

bool SingleActionScheduleType::RequiresSingleEntry() const
{
  return value_ == 1u;
}

bool SingleActionScheduleType::RequiresUniformTime() const
{
  return value_ == 2u || value_ == 3u;
}

bool SingleActionScheduleType::ForbidsWildcardsInDate() const
{
  return value_ == 2u || value_ == 4u;
}

bool SingleActionScheduleType::operator==(
  const SingleActionScheduleType& other) const
{
  return value_ == other.value_;
}

bool SingleActionScheduleType::operator!=(
  const SingleActionScheduleType& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
