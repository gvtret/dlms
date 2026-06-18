#pragma once

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of the `type` attribute of IC 22 (Single action
// schedule), DLMS UA Blue Book Ed. 12.1 / IEC 62056-6-2 ED4 §4.5.7.2.3.
//
// The on-wire encoding is `enum (unsigned8)` with the values:
//   (1) size of execution_time = 1; wildcard in date allowed
//   (2) size of execution_time = n; all time values are the same,
//       wildcards in date not allowed
//   (3) size of execution_time = n; all time values are the same,
//       wildcards in date are allowed
//   (4) size of execution_time = n; time values may be different,
//       wildcards in date not allowed
//   (5) size of execution_time = n; time values may be different,
//       wildcards in date are allowed
//
// The wrapper rejects values outside 1..5. Convenience predicates encode
// the size/time-uniformity/wildcard-in-date rules so the caller can
// validate `execution_time` against the chosen type in a single place.
class SingleActionScheduleType
{
public:
  static const std::uint8_t MinValue = 1u;
  static const std::uint8_t MaxValue = 5u;

  // Default-constructs to value 1 (single-entry schedule, the simplest
  // form), which is always a valid combination with a single-element
  // execution_time array.
  SingleActionScheduleType();
  explicit SingleActionScheduleType(std::uint8_t value);

  static bool IsValid(std::uint8_t value);

  std::uint8_t Value() const;

  // Returns `true` and updates the stored value on success, `false` and
  // leaves the value unchanged on out-of-range input.
  bool SetValue(std::uint8_t value);

  // Spec-driven invariants over the companion execution_time array.
  // RequiresSingleEntry(): type 1 — the array must hold exactly one entry.
  // RequiresUniformTime(): types 2 and 3 — every entry must share the same time.
  // ForbidsWildcardsInDate(): types 2 and 4 — date fields must not contain
  //   any wildcard sentinel.
  bool RequiresSingleEntry() const;
  bool RequiresUniformTime() const;
  bool ForbidsWildcardsInDate() const;

  bool operator==(const SingleActionScheduleType& other) const;
  bool operator!=(const SingleActionScheduleType& other) const;

private:
  std::uint8_t value_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
