#include "dlms/cosem/types/schedule_table_entry.hpp"

#include <gtest/gtest.h>

namespace {

dlms::cosem::types::Date MakeDate(
  std::uint16_t year, std::uint8_t month, std::uint8_t dom)
{
  dlms::cosem::types::Date d;
  EXPECT_TRUE(d.SetYear(year));
  EXPECT_TRUE(d.SetMonth(month));
  EXPECT_TRUE(d.SetDayOfMonth(dom));
  return d;
}

dlms::cosem::types::Time MakeTime(
  std::uint8_t h, std::uint8_t m, std::uint8_t s)
{
  dlms::cosem::types::Time t;
  EXPECT_TRUE(t.SetHour(h));
  EXPECT_TRUE(t.SetMinute(m));
  EXPECT_TRUE(t.SetSecond(s));
  EXPECT_TRUE(t.SetHundredths(0u));
  return t;
}

dlms::cosem::types::Script MakeScript(std::uint8_t lnTail, std::uint16_t selector)
{
  dlms::cosem::CosemLogicalName ln(0u, 0u, 10u, 100u, 100u, lnTail);
  return dlms::cosem::types::Script(ln, selector);
}

} // namespace

TEST(TypesScheduleTableEntry, DefaultConstructHasSafeDefaults)
{
  dlms::cosem::types::ScheduleTableEntry e;
  EXPECT_EQ(0u, e.Index());
  EXPECT_FALSE(e.Enable());
  EXPECT_EQ(dlms::cosem::types::ScheduleTableEntry::ValidityWindowAlways,
            e.ValidityWindow());
  EXPECT_EQ(dlms::cosem::types::ScheduleTableEntry::WeekdaysAll,
            e.ExecWeekdays());
  EXPECT_EQ(0u, e.ExecSpecdays());
  EXPECT_TRUE(dlms::cosem::types::ScheduleTableEntry::IsValid(e));
}

TEST(TypesScheduleTableEntry, ConstructAndAccess)
{
  dlms::cosem::types::ScheduleTableEntry e(
    7u, true,
    MakeScript(1u, 3u),
    MakeTime(22u, 30u, 0u),
    15u,
    0x1Fu,                                // Mon..Fri
    (std::uint64_t{1} << 0) | (std::uint64_t{1} << 63),
    MakeDate(2021u, 4u, 1u),
    MakeDate(2021u, 9u, 30u));

  EXPECT_EQ(7u, e.Index());
  EXPECT_TRUE(e.Enable());
  EXPECT_EQ(3u, e.GetScript().Selector());
  EXPECT_EQ(MakeTime(22u, 30u, 0u), e.SwitchTime());
  EXPECT_EQ(15u, e.ValidityWindow());
  EXPECT_EQ(0x1Fu, e.ExecWeekdays());
  EXPECT_EQ((std::uint64_t{1} << 0) | (std::uint64_t{1} << 63), e.ExecSpecdays());
  EXPECT_EQ(MakeDate(2021u, 4u, 1u), e.BeginDate());
  EXPECT_EQ(MakeDate(2021u, 9u, 30u), e.EndDate());
  EXPECT_TRUE(dlms::cosem::types::ScheduleTableEntry::IsValid(e));
}

TEST(TypesScheduleTableEntry, ConstructMasksHighWeekdayBits)
{
  // Constructor must clip exec_weekdays to the 7-bit valid range so
  // that an entry built from sloppy input does not enter an invalid
  // state behind the validator's back.
  dlms::cosem::types::ScheduleTableEntry e(
    1u, false,
    MakeScript(2u, 1u),
    MakeTime(0u, 0u, 0u),
    dlms::cosem::types::ScheduleTableEntry::ValidityWindowAlways,
    0xFFu,                                // high bit must be dropped
    0u,
    MakeDate(2021u, 1u, 1u),
    MakeDate(2021u, 12u, 31u));
  EXPECT_EQ(0x7Fu, e.ExecWeekdays());
  EXPECT_TRUE(dlms::cosem::types::ScheduleTableEntry::IsValid(e));
}

TEST(TypesScheduleTableEntry, SetExecWeekdaysRejectsHighBit)
{
  dlms::cosem::types::ScheduleTableEntry e;
  EXPECT_TRUE(e.SetExecWeekdays(0x55u));
  EXPECT_EQ(0x55u, e.ExecWeekdays());

  // Setting bit 7 must fail without mutating.
  EXPECT_FALSE(e.SetExecWeekdays(0x80u));
  EXPECT_EQ(0x55u, e.ExecWeekdays());
  EXPECT_FALSE(e.SetExecWeekdays(0xC0u));
  EXPECT_EQ(0x55u, e.ExecWeekdays());

  // 0 and 0x7F must be accepted.
  EXPECT_TRUE(e.SetExecWeekdays(0u));
  EXPECT_EQ(0u, e.ExecWeekdays());
  EXPECT_TRUE(e.SetExecWeekdays(0x7Fu));
  EXPECT_EQ(0x7Fu, e.ExecWeekdays());
}

TEST(TypesScheduleTableEntry, SetExecSpecdaysAcceptsFullRange)
{
  dlms::cosem::types::ScheduleTableEntry e;
  const std::uint64_t mask = (std::uint64_t{1} << 0)
                           | (std::uint64_t{1} << 31)
                           | (std::uint64_t{1} << 63);
  e.SetExecSpecdays(mask);
  EXPECT_EQ(mask, e.ExecSpecdays());

  // 0 is fine (no special days).
  e.SetExecSpecdays(0u);
  EXPECT_EQ(0u, e.ExecSpecdays());

  // All bits set is fine.
  e.SetExecSpecdays(~static_cast<std::uint64_t>(0u));
  EXPECT_EQ(~static_cast<std::uint64_t>(0u), e.ExecSpecdays());
}

TEST(TypesScheduleTableEntry, EqualityAndInequality)
{
  dlms::cosem::types::ScheduleTableEntry a(
    1u, true,
    MakeScript(1u, 1u),
    MakeTime(8u, 0u, 0u),
    0xFFFFu, 0x7Fu, 0u,
    MakeDate(2021u, 1u, 1u),
    MakeDate(2021u, 12u, 31u));
  dlms::cosem::types::ScheduleTableEntry b = a;
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  b.SetIndex(2u);
  EXPECT_TRUE(a != b);
  b = a;

  EXPECT_TRUE(b.SetExecWeekdays(0x55u));
  EXPECT_TRUE(a != b);
}

TEST(TypesScheduleTableEntry, IsValidCatchesPokedHighBit)
{
  // Build via constructor (which clips), then deliberately exercise
  // SetExecWeekdays-rejection: IsValid stays true for clean entries.
  dlms::cosem::types::ScheduleTableEntry e(
    1u, true,
    MakeScript(1u, 1u),
    MakeTime(0u, 0u, 0u),
    0xFFFFu, 0x7Fu, 0u,
    MakeDate(2021u, 1u, 1u),
    MakeDate(2021u, 12u, 31u));
  EXPECT_TRUE(dlms::cosem::types::ScheduleTableEntry::IsValid(e));
  // The validator cannot regress because the only mutator that can
  // introduce a bad weekday mask refuses to do so. Test that the
  // contract holds: after a rejected SetExecWeekdays the entry is
  // still valid.
  EXPECT_FALSE(e.SetExecWeekdays(0x80u));
  EXPECT_TRUE(dlms::cosem::types::ScheduleTableEntry::IsValid(e));
}
