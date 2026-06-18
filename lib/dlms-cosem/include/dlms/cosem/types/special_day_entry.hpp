#pragma once

#include "dlms/cosem/types/date.hpp"

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of a single `spec_day_entry` structure as defined
// by IC 11 "Special days table" in DLMS UA Blue Book Ed. 12.1 /
// IEC 62056-6-2 ED4 §4.5.4.2.2:
//
//   spec_day_entry ::= structure
//   {
//     index:           long-unsigned,
//     specialday_date: octet-string,   // date per §4.1.6.1, 5 bytes
//     day_id:          unsigned        // uint8, must match the
//                                      // bit-string length of
//                                      // exec_specdays in the related
//                                      // IC "Schedule" object
//   }
//
// `specialday_date` may contain wildcards (e.g. unspecified year for a
// recurring holiday like Christmas) — that is the whole point of this
// IC, so we reuse types::Date which already understands the spec
// sentinels.
class SpecialDayEntry
{
public:
  SpecialDayEntry();
  SpecialDayEntry(std::uint16_t index, const Date& date, std::uint8_t dayId);

  std::uint16_t Index() const;
  const Date& SpecialDayDate() const;
  std::uint8_t DayId() const;

  void SetIndex(std::uint16_t value);
  void SetSpecialDayDate(const Date& value);
  void SetDayId(std::uint8_t value);

  bool operator==(const SpecialDayEntry& other) const;
  bool operator!=(const SpecialDayEntry& other) const;

private:
  std::uint16_t index_;
  Date date_;
  std::uint8_t dayId_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
