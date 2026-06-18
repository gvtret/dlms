#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the DLMS/COSEM `date` octet-string defined in
// IEC 62056-6-2 ED4 (2021) section 4.1.6.1 and DLMS UA Blue Book Ed. 12.1
// section 4.1.6.1. The on-wire encoding is a 5-byte fixed-length octet-string
// with the following layout (identical to bytes [0..4] of `date-time`):
//
//   [0..1] year highbyte / year lowbyte (long-unsigned, 0xFFFF = unspecified)
//   [2]    month (1..12, 0xFD = DST end, 0xFE = DST begin, 0xFF = unspecified)
//   [3]    day-of-month (1..31, 0xFD = 2nd last day, 0xFE = last day,
//          0xE0..0xFC reserved, 0xFF = unspecified)
//   [4]    day-of-week (1..7, 1 = Monday, 0xFF = unspecified)
//
// All setters reject values outside the spec-defined ranges and return `false`
// without mutating state. To assign a wildcard, pass the explicit
// `kUnspecified*` constant exposed below. Default-constructed `Date` has every
// field set to "unspecified".
class Date
{
public:
  static const std::size_t WireSize = 5u;

  static const std::uint16_t YearUnspecifiedValue = 0xFFFFu;
  static const std::uint16_t YearMax = 0xFFFEu;

  static const std::uint8_t MonthUnspecifiedValue = 0xFFu;
  static const std::uint8_t MonthDstEndValue = 0xFDu;
  static const std::uint8_t MonthDstBeginValue = 0xFEu;

  static const std::uint8_t DayOfMonthUnspecifiedValue = 0xFFu;
  static const std::uint8_t DayOfMonthLastValue = 0xFEu;
  static const std::uint8_t DayOfMonthSecondLastValue = 0xFDu;

  static const std::uint8_t DayOfWeekUnspecifiedValue = 0xFFu;

  Date();

  // Parse 5 bytes of DLMS date payload (no tag, no length prefix). Returns
  // `true` and overwrites `out` on success. On invalid size or out-of-range
  // fields returns `false` and leaves `out` unchanged.
  static bool TryFromBytes(
    const std::uint8_t* data,
    std::size_t size,
    Date& out);

  // Serialize to the 5-byte payload (no tag, no length prefix). Always
  // succeeds: a `Date` cannot hold an invalid field combination.
  std::array<std::uint8_t, WireSize> ToBytes() const;

  std::uint16_t Year() const;
  std::uint8_t Month() const;
  std::uint8_t DayOfMonth() const;
  std::uint8_t DayOfWeek() const;

  bool SetYear(std::uint16_t value);
  bool SetMonth(std::uint8_t value);
  bool SetDayOfMonth(std::uint8_t value);
  bool SetDayOfWeek(std::uint8_t value);

  bool YearUnspecified() const;
  bool MonthUnspecified() const;
  bool DayOfMonthUnspecified() const;
  bool DayOfWeekUnspecified() const;

  bool operator==(const Date& other) const;
  bool operator!=(const Date& other) const;

private:
  std::uint16_t year_;
  std::uint8_t month_;
  std::uint8_t dayOfMonth_;
  std::uint8_t dayOfWeek_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
