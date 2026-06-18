#pragma once

#include "dlms/cosem/types/date.hpp"
#include "dlms/cosem/types/script.hpp"
#include "dlms/cosem/types/time.hpp"

#include <cstdint>

namespace dlms {
namespace cosem {
namespace types {

// Typed representation of a single `schedule_table_entry` structure as
// defined by IC 10 "Schedule" in DLMS UA Blue Book Ed. 12.1 /
// IEC 62056-6-2 ED4 §4.5.3.2.2:
//
//   schedule_table_entry ::= structure
//   {
//     index:               long-unsigned,
//     enable:              boolean,
//     script_logical_name: octet-string,        // -> types::Script
//     script_selector:     long-unsigned,       // -> types::Script
//     switch_time:         octet-string,        // -> types::Time (wildcards ok)
//     validity_window:     long-unsigned,       // 0xFFFF: always
//     exec_weekdays:       bit-string,          // 7 bits (Mon..Sun)
//     exec_specdays:       bit-string,          // <= 64 bits (day_id 0..63)
//     begin_date:          octet-string,        // -> types::Date
//     end_date:            octet-string         // -> types::Date
//   }
//
// `exec_weekdays` is a 7-bit mask, where bit 0 = Monday and bit 6 =
// Sunday. The high bit must be zero.
//
// `exec_specdays` is a 64-bit mask, where bit `i` enables the entry on
// the Special days table day_id equal to `i`. This caps the supported
// day_id range at 0..63 (see roadmap decision: typed schedule entries
// constrain exec_specdays to 64 bits). Any `day_id > 63` cannot be
// referenced from a typed schedule entry.
class ScheduleTableEntry
{
public:
  // exec_weekdays sentinel: every day of the week enabled.
  static const std::uint8_t WeekdaysAll;
  // exec_weekdays bit width (only the low 7 bits are valid).
  static const std::uint8_t WeekdaysBitWidth;
  // exec_specdays bit width (cap: 64 bits, i.e. day_id 0..63).
  static const std::uint8_t SpecdaysBitWidth;
  // validity_window sentinel: process any time (no power-fail window).
  static const std::uint16_t ValidityWindowAlways;

  ScheduleTableEntry();
  ScheduleTableEntry(
    std::uint16_t index,
    bool enable,
    const Script& script,
    const Time& switchTime,
    std::uint16_t validityWindow,
    std::uint8_t execWeekdays,
    std::uint64_t execSpecdays,
    const Date& beginDate,
    const Date& endDate);

  std::uint16_t Index() const;
  bool Enable() const;
  const Script& GetScript() const;
  const Time& SwitchTime() const;
  std::uint16_t ValidityWindow() const;
  std::uint8_t ExecWeekdays() const;
  std::uint64_t ExecSpecdays() const;
  const Date& BeginDate() const;
  const Date& EndDate() const;

  void SetIndex(std::uint16_t value);
  void SetEnable(bool value);
  void SetScript(const Script& value);
  void SetSwitchTime(const Time& value);
  void SetValidityWindow(std::uint16_t value);
  // Returns false (no mutation) when bits above bit 6 are set.
  bool SetExecWeekdays(std::uint8_t value);
  void SetExecSpecdays(std::uint64_t value);
  void SetBeginDate(const Date& value);
  void SetEndDate(const Date& value);

  // Whole-entry validation: per-field invariants (already enforced by
  // SetExecWeekdays) plus the cross-field rule that begin_date and
  // end_date must each be self-consistent Dates (true by construction
  // when built via the typed setters, but useful for callers that
  // build the entry from raw fields).
  static bool IsValid(const ScheduleTableEntry& entry);

  bool operator==(const ScheduleTableEntry& other) const;
  bool operator!=(const ScheduleTableEntry& other) const;

private:
  std::uint16_t index_;
  bool enable_;
  Script script_;
  Time switchTime_;
  std::uint16_t validityWindow_;
  std::uint8_t execWeekdays_;
  std::uint64_t execSpecdays_;
  Date beginDate_;
  Date endDate_;
};

} // namespace types
} // namespace cosem
} // namespace dlms
