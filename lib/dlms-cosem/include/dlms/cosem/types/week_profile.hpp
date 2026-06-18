#pragma once

#include "dlms/cosem/cosem_types.hpp"

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of `week_profile` inside IC 20 "Activity calendar"
// per IEC 62056-6-2 ED4 (2021) §4.5.5.2.4 and DLMS UA Blue Book Ed. 12.1
// §5.1.9:
//
//   week_profile ::= structure
//   {
//     week_profile_name: octet-string,
//     monday:    day_id (unsigned),
//     tuesday:   day_id,
//     wednesday: day_id,
//     thursday:  day_id,
//     friday:    day_id,
//     saturday:  day_id,
//     sunday:    day_id
//   }
//
// The cross-collection invariant (every day_id MUST exist in
// day_profile_table) is owned by `CosemActivityCalendarObject`, not by
// this type.
class WeekProfile
{
public:
  WeekProfile();
  WeekProfile(
    const CosemByteBuffer& name,
    std::uint8_t monday,
    std::uint8_t tuesday,
    std::uint8_t wednesday,
    std::uint8_t thursday,
    std::uint8_t friday,
    std::uint8_t saturday,
    std::uint8_t sunday);

  const CosemByteBuffer& Name() const;
  std::uint8_t Monday() const;
  std::uint8_t Tuesday() const;
  std::uint8_t Wednesday() const;
  std::uint8_t Thursday() const;
  std::uint8_t Friday() const;
  std::uint8_t Saturday() const;
  std::uint8_t Sunday() const;

  void SetName(const CosemByteBuffer& value);
  void SetMonday(std::uint8_t value);
  void SetTuesday(std::uint8_t value);
  void SetWednesday(std::uint8_t value);
  void SetThursday(std::uint8_t value);
  void SetFriday(std::uint8_t value);
  void SetSaturday(std::uint8_t value);
  void SetSunday(std::uint8_t value);

  bool operator==(const WeekProfile& other) const;
  bool operator!=(const WeekProfile& other) const;

private:
  CosemByteBuffer name_;
  std::uint8_t monday_;
  std::uint8_t tuesday_;
  std::uint8_t wednesday_;
  std::uint8_t thursday_;
  std::uint8_t friday_;
  std::uint8_t saturday_;
  std::uint8_t sunday_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
