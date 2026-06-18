#include "dlms/cosem/types/season_profile.hpp"

namespace dlms {
namespace cosem {
namespace types {

SeasonProfile::SeasonProfile()
  : name_()
  , start_()
  , weekName_()
{
}

SeasonProfile::SeasonProfile(
  const CosemByteBuffer& name,
  const DateTime& start,
  const CosemByteBuffer& weekName)
  : name_(name)
  , start_(start)
  , weekName_(weekName)
{
}

const CosemByteBuffer& SeasonProfile::Name() const { return name_; }
const DateTime& SeasonProfile::Start() const { return start_; }
const CosemByteBuffer& SeasonProfile::WeekName() const { return weekName_; }

void SeasonProfile::SetName(const CosemByteBuffer& value) { name_ = value; }
void SeasonProfile::SetStart(const DateTime& value) { start_ = value; }
void SeasonProfile::SetWeekName(const CosemByteBuffer& value)
{
  weekName_ = value;
}

bool SeasonProfile::operator==(const SeasonProfile& other) const
{
  return name_ == other.name_
      && start_ == other.start_
      && weekName_ == other.weekName_;
}

bool SeasonProfile::operator!=(const SeasonProfile& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
