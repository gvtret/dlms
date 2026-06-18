#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the DLMS/COSEM `time` octet-string defined in
// IEC 62056-6-2 ED4 (2021) section 4.1.6.1 and DLMS UA Blue Book Ed. 12.1
// section 4.1.6.1. The on-wire encoding is a 4-byte fixed-length octet-string
// with the following layout (identical to bytes [5..8] of `date-time`):
//
//   [0] hour (0..23, 0xFF = unspecified)
//   [1] minute (0..59, 0xFF = unspecified)
//   [2] second (0..59, 0xFF = unspecified)
//   [3] hundredths (0..99, 0xFF = unspecified)
//
// All setters reject values outside the spec-defined ranges and return `false`
// without mutating state. To assign a wildcard, pass the explicit
// `kUnspecified*` constant exposed below. Default-constructed `Time` has every
// field set to "unspecified".
class Time
{
public:
  static const std::size_t WireSize = 4u;

  static const std::uint8_t HourUnspecifiedValue = 0xFFu;
  static const std::uint8_t MinuteUnspecifiedValue = 0xFFu;
  static const std::uint8_t SecondUnspecifiedValue = 0xFFu;
  static const std::uint8_t HundredthsUnspecifiedValue = 0xFFu;

  Time();

  // Parse 4 bytes of DLMS time payload (no tag, no length prefix). Returns
  // `true` and overwrites `out` on success. On invalid size or out-of-range
  // fields returns `false` and leaves `out` unchanged.
  static bool TryFromBytes(
    const std::uint8_t* data,
    std::size_t size,
    Time& out);

  // Serialize to the 4-byte payload (no tag, no length prefix). Always
  // succeeds: a `Time` cannot hold an invalid field combination.
  std::array<std::uint8_t, WireSize> ToBytes() const;

  std::uint8_t Hour() const;
  std::uint8_t Minute() const;
  std::uint8_t Second() const;
  std::uint8_t Hundredths() const;

  bool SetHour(std::uint8_t value);
  bool SetMinute(std::uint8_t value);
  bool SetSecond(std::uint8_t value);
  bool SetHundredths(std::uint8_t value);

  bool HourUnspecified() const;
  bool MinuteUnspecified() const;
  bool SecondUnspecified() const;
  bool HundredthsUnspecified() const;

  bool operator==(const Time& other) const;
  bool operator!=(const Time& other) const;

private:
  std::uint8_t hour_;
  std::uint8_t minute_;
  std::uint8_t second_;
  std::uint8_t hundredths_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
