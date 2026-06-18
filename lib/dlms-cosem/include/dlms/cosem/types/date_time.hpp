#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the DLMS/COSEM `date-time` octet-string defined in
// IEC 62056-6-2 ED4 (2021) section 4.1.6.1 and DLMS UA Blue Book Ed. 12.1
// section 4.1.6.1. The on-wire encoding is a 12-byte fixed-length octet-string
// with the following layout:
//
//   [0..1] year highbyte / year lowbyte (long-unsigned, 0xFFFF = unspecified)
//   [2]    month (1..12, 0xFD = DST end, 0xFE = DST begin, 0xFF = unspecified)
//   [3]    day-of-month (1..31, 0xFD = 2nd last day, 0xFE = last day,
//          0xE0..0xFC reserved, 0xFF = unspecified)
//   [4]    day-of-week (1..7, 1 = Monday, 0xFF = unspecified)
//   [5]    hour (0..23, 0xFF = unspecified)
//   [6]    minute (0..59, 0xFF = unspecified)
//   [7]    second (0..59, 0xFF = unspecified)
//   [8]    hundredths (0..99, 0xFF = unspecified)
//   [9..10] deviation highbyte / lowbyte (long, -720..+720, 0x8000 = unspecified)
//   [11]   clock_status bitmask (bit0=invalid, bit1=doubtful,
//          bit2=different_clock_base, bit3=invalid_clock_status, bit7=DST_active,
//          0xFF = unspecified)
//
// All setters reject values outside the spec-defined ranges and return `false`
// without mutating state. To assign a wildcard, pass the explicit
// `kUnspecified*` constant exposed below. Default-constructed `DateTime` has
// every field set to "unspecified".
class DateTime
{
public:
  static const std::size_t WireSize = 12u;

  static const std::uint16_t YearUnspecifiedValue = 0xFFFFu;
  static const std::uint16_t YearMax = 0xFFFEu;

  static const std::uint8_t MonthUnspecifiedValue = 0xFFu;
  static const std::uint8_t MonthDstEndValue = 0xFDu;
  static const std::uint8_t MonthDstBeginValue = 0xFEu;

  static const std::uint8_t DayOfMonthUnspecifiedValue = 0xFFu;
  static const std::uint8_t DayOfMonthLastValue = 0xFEu;
  static const std::uint8_t DayOfMonthSecondLastValue = 0xFDu;

  static const std::uint8_t DayOfWeekUnspecifiedValue = 0xFFu;

  static const std::uint8_t HourUnspecifiedValue = 0xFFu;
  static const std::uint8_t MinuteUnspecifiedValue = 0xFFu;
  static const std::uint8_t SecondUnspecifiedValue = 0xFFu;
  static const std::uint8_t HundredthsUnspecifiedValue = 0xFFu;

  static const std::int16_t DeviationUnspecifiedValue = static_cast<std::int16_t>(0x8000);
  static const std::int16_t DeviationMin = -720;
  static const std::int16_t DeviationMax = 720;

  static const std::uint8_t ClockStatusUnspecifiedValue = 0xFFu;
  static const std::uint8_t ClockStatusInvalidValueBit = 0x01u;
  static const std::uint8_t ClockStatusDoubtfulValueBit = 0x02u;
  static const std::uint8_t ClockStatusDifferentClockBaseBit = 0x04u;
  static const std::uint8_t ClockStatusInvalidClockStatusBit = 0x08u;
  static const std::uint8_t ClockStatusDaylightSavingActiveBit = 0x80u;
  // Bits 4..6 are reserved per spec.
  static const std::uint8_t ClockStatusReservedMask = 0x70u;

  DateTime();

  // Parse 12 bytes of DLMS date-time payload (no tag, no length prefix). Returns
  // `true` and overwrites `out` on success. On invalid size or out-of-range
  // fields returns `false` and leaves `out` unchanged.
  static bool TryFromBytes(
    const std::uint8_t* data,
    std::size_t size,
    DateTime& out);

  // Serialize to the 12-byte payload (no tag, no length prefix). Always
  // succeeds: a `DateTime` cannot hold an invalid field combination.
  std::array<std::uint8_t, WireSize> ToBytes() const;

  std::uint16_t Year() const;
  std::uint8_t Month() const;
  std::uint8_t DayOfMonth() const;
  std::uint8_t DayOfWeek() const;
  std::uint8_t Hour() const;
  std::uint8_t Minute() const;
  std::uint8_t Second() const;
  std::uint8_t Hundredths() const;
  std::int16_t Deviation() const;
  std::uint8_t ClockStatus() const;

  bool SetYear(std::uint16_t value);
  bool SetMonth(std::uint8_t value);
  bool SetDayOfMonth(std::uint8_t value);
  bool SetDayOfWeek(std::uint8_t value);
  bool SetHour(std::uint8_t value);
  bool SetMinute(std::uint8_t value);
  bool SetSecond(std::uint8_t value);
  bool SetHundredths(std::uint8_t value);
  bool SetDeviation(std::int16_t value);
  // clock_status is an opaque bitmask: every byte value is representable,
  // including 0xFF for "unspecified". No range check.
  void SetClockStatus(std::uint8_t value);

  bool YearUnspecified() const;
  bool MonthUnspecified() const;
  bool DayOfMonthUnspecified() const;
  bool DayOfWeekUnspecified() const;
  bool HourUnspecified() const;
  bool MinuteUnspecified() const;
  bool SecondUnspecified() const;
  bool HundredthsUnspecified() const;
  bool DeviationUnspecified() const;
  bool ClockStatusUnspecified() const;

  bool operator==(const DateTime& other) const;
  bool operator!=(const DateTime& other) const;

private:
  std::uint16_t year_;
  std::uint8_t month_;
  std::uint8_t dayOfMonth_;
  std::uint8_t dayOfWeek_;
  std::uint8_t hour_;
  std::uint8_t minute_;
  std::uint8_t second_;
  std::uint8_t hundredths_;
  std::int16_t deviation_;
  std::uint8_t clockStatus_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
