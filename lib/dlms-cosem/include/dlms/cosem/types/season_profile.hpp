#pragma once

#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/types/date_time.hpp"

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of `season` inside IC 20 "Activity calendar" per
// IEC 62056-6-2 ED4 (2021) §4.5.5.2.3 and DLMS UA Blue Book Ed. 12.1
// §5.1.9:
//
//   season ::= structure
//   {
//     season_profile_name: octet-string,
//     season_start:        octet-string(12) = date_time (wildcards allowed),
//     week_name:           octet-string
//   }
//
// Wildcards in `season_start` are accepted: an all-wildcard
// `season_start` means "this season never starts", per the spec note.
// Cross-collection invariants (unique `season_profile_name`, sorted by
// `season_start`, `week_name` resolves into week_profile_table) are
// owned by `CosemActivityCalendarObject`.
class SeasonProfile
{
public:
  SeasonProfile();
  SeasonProfile(
    const CosemByteBuffer& name,
    const DateTime& start,
    const CosemByteBuffer& weekName);

  const CosemByteBuffer& Name() const;
  const DateTime& Start() const;
  const CosemByteBuffer& WeekName() const;

  void SetName(const CosemByteBuffer& value);
  void SetStart(const DateTime& value);
  void SetWeekName(const CosemByteBuffer& value);

  bool operator==(const SeasonProfile& other) const;
  bool operator!=(const SeasonProfile& other) const;

private:
  CosemByteBuffer name_;
  DateTime start_;
  CosemByteBuffer weekName_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
