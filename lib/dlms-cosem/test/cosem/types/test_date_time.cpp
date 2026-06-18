#include "dlms/cosem/types/date_time.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>

namespace {

using dlms::cosem::types::DateTime;

std::array<std::uint8_t, 12> SampleBytes()
{
  // 2024-06-15 (Saturday=6) 12:34:56.78, deviation +180 (UTC+3),
  // clock_status = DST active.
  return {{0x07u, 0xE8u, 0x06u, 0x0Fu, 0x06u,
           0x0Cu, 0x22u, 0x38u, 0x4Eu,
           0x00u, 0xB4u, 0x80u}};
}

} // namespace

TEST(TypesDateTime, DefaultConstructedIsAllUnspecified)
{
  DateTime dt;

  EXPECT_TRUE(dt.YearUnspecified());
  EXPECT_TRUE(dt.MonthUnspecified());
  EXPECT_TRUE(dt.DayOfMonthUnspecified());
  EXPECT_TRUE(dt.DayOfWeekUnspecified());
  EXPECT_TRUE(dt.HourUnspecified());
  EXPECT_TRUE(dt.MinuteUnspecified());
  EXPECT_TRUE(dt.SecondUnspecified());
  EXPECT_TRUE(dt.HundredthsUnspecified());
  EXPECT_TRUE(dt.DeviationUnspecified());
  EXPECT_TRUE(dt.ClockStatusUnspecified());

  const std::array<std::uint8_t, 12> expected = {{
    0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu,
    0xFFu, 0xFFu, 0xFFu, 0xFFu,
    0x80u, 0x00u, 0xFFu}};
  EXPECT_EQ(expected, dt.ToBytes());
}

TEST(TypesDateTime, RoundTripsSampleBytes)
{
  const auto bytes = SampleBytes();

  DateTime parsed;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), parsed));

  EXPECT_EQ(2024u, parsed.Year());
  EXPECT_EQ(6u, parsed.Month());
  EXPECT_EQ(15u, parsed.DayOfMonth());
  EXPECT_EQ(6u, parsed.DayOfWeek());
  EXPECT_EQ(12u, parsed.Hour());
  EXPECT_EQ(34u, parsed.Minute());
  EXPECT_EQ(56u, parsed.Second());
  EXPECT_EQ(78u, parsed.Hundredths());
  EXPECT_EQ(180, parsed.Deviation());
  EXPECT_EQ(DateTime::ClockStatusDaylightSavingActiveBit, parsed.ClockStatus());

  EXPECT_FALSE(parsed.YearUnspecified());
  EXPECT_FALSE(parsed.DeviationUnspecified());
  EXPECT_FALSE(parsed.ClockStatusUnspecified());

  EXPECT_EQ(bytes, parsed.ToBytes());
}

TEST(TypesDateTime, ParsesNegativeDeviation)
{
  // deviation = -720 (0xFD30) — minimum allowed.
  std::array<std::uint8_t, 12> bytes = SampleBytes();
  bytes[9] = 0xFDu;
  bytes[10] = 0x30u;

  DateTime parsed;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(-720, parsed.Deviation());
  EXPECT_FALSE(parsed.DeviationUnspecified());
  EXPECT_EQ(bytes, parsed.ToBytes());
}

TEST(TypesDateTime, AcceptsAllWildcardFields)
{
  // Wildcards explicitly set: month=0xFF, day=0xFF, dow=0xFF, time=all 0xFF,
  // deviation=0x8000, clock_status=0xFF, year=0xFFFF.
  const std::array<std::uint8_t, 12> bytes = {{
    0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu,
    0xFFu, 0xFFu, 0xFFu, 0xFFu,
    0x80u, 0x00u, 0xFFu}};

  DateTime parsed;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_TRUE(parsed.YearUnspecified());
  EXPECT_TRUE(parsed.MonthUnspecified());
  EXPECT_TRUE(parsed.DeviationUnspecified());
  EXPECT_TRUE(parsed.ClockStatusUnspecified());
  EXPECT_EQ(bytes, parsed.ToBytes());
}

TEST(TypesDateTime, AcceptsMonthDstSentinels)
{
  std::array<std::uint8_t, 12> bytes = SampleBytes();

  bytes[2] = DateTime::MonthDstBeginValue;
  DateTime parsed;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(DateTime::MonthDstBeginValue, parsed.Month());

  bytes[2] = DateTime::MonthDstEndValue;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(DateTime::MonthDstEndValue, parsed.Month());
}

TEST(TypesDateTime, AcceptsLastAndSecondLastDayOfMonth)
{
  std::array<std::uint8_t, 12> bytes = SampleBytes();

  bytes[3] = DateTime::DayOfMonthLastValue;
  DateTime parsed;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(DateTime::DayOfMonthLastValue, parsed.DayOfMonth());

  bytes[3] = DateTime::DayOfMonthSecondLastValue;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(DateTime::DayOfMonthSecondLastValue, parsed.DayOfMonth());
}

TEST(TypesDateTime, RejectsWrongSize)
{
  const auto bytes = SampleBytes();
  DateTime dt;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), 0u, dt));
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), 11u, dt));
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), 13u, dt));
  EXPECT_FALSE(DateTime::TryFromBytes(nullptr, 12u, dt));
}

TEST(TypesDateTime, RejectsOutOfRangeMonth)
{
  std::array<std::uint8_t, 12> bytes = SampleBytes();
  bytes[2] = 0u;
  DateTime dt;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes[2] = 13u;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes[2] = 0xFCu; // Not one of the named sentinels.
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
}

TEST(TypesDateTime, RejectsReservedDayOfMonth)
{
  std::array<std::uint8_t, 12> bytes = SampleBytes();
  bytes[3] = 0u;
  DateTime dt;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes[3] = 32u;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes[3] = 0xE0u; // Reserved range.
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes[3] = 0xFCu; // Last byte of reserved range.
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
}

TEST(TypesDateTime, RejectsOutOfRangeDayOfWeek)
{
  std::array<std::uint8_t, 12> bytes = SampleBytes();
  bytes[4] = 0u;
  DateTime dt;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes[4] = 8u;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
}

TEST(TypesDateTime, RejectsOutOfRangeTimeFields)
{
  std::array<std::uint8_t, 12> bytes = SampleBytes();

  bytes[5] = 24u;
  DateTime dt;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes = SampleBytes();

  bytes[6] = 60u;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes = SampleBytes();

  bytes[7] = 60u;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
  bytes = SampleBytes();

  bytes[8] = 100u;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
}

TEST(TypesDateTime, RejectsOutOfRangeDeviation)
{
  std::array<std::uint8_t, 12> bytes = SampleBytes();
  // +721 = 0x02D1
  bytes[9] = 0x02u;
  bytes[10] = 0xD1u;
  DateTime dt;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));

  // -721 = 0xFD2F
  bytes[9] = 0xFDu;
  bytes[10] = 0x2Fu;
  EXPECT_FALSE(DateTime::TryFromBytes(bytes.data(), bytes.size(), dt));
}

TEST(TypesDateTime, SettersValidateRanges)
{
  DateTime dt;

  EXPECT_TRUE(dt.SetMonth(12u));
  EXPECT_EQ(12u, dt.Month());
  EXPECT_FALSE(dt.SetMonth(0u));
  EXPECT_FALSE(dt.SetMonth(13u));
  EXPECT_EQ(12u, dt.Month()); // Unchanged after failed set.
  EXPECT_TRUE(dt.SetMonth(DateTime::MonthUnspecifiedValue));
  EXPECT_TRUE(dt.MonthUnspecified());

  EXPECT_FALSE(dt.SetDayOfMonth(0u));
  EXPECT_FALSE(dt.SetDayOfMonth(32u));
  EXPECT_FALSE(dt.SetDayOfMonth(0xE5u));
  EXPECT_TRUE(dt.SetDayOfMonth(DateTime::DayOfMonthLastValue));
  EXPECT_EQ(DateTime::DayOfMonthLastValue, dt.DayOfMonth());

  EXPECT_FALSE(dt.SetDayOfWeek(0u));
  EXPECT_FALSE(dt.SetDayOfWeek(8u));
  EXPECT_TRUE(dt.SetDayOfWeek(1u));
  EXPECT_EQ(1u, dt.DayOfWeek());

  EXPECT_FALSE(dt.SetHour(24u));
  EXPECT_TRUE(dt.SetHour(23u));
  EXPECT_FALSE(dt.SetMinute(60u));
  EXPECT_TRUE(dt.SetMinute(59u));
  EXPECT_FALSE(dt.SetSecond(60u));
  EXPECT_TRUE(dt.SetSecond(59u));
  EXPECT_FALSE(dt.SetHundredths(100u));
  EXPECT_TRUE(dt.SetHundredths(99u));

  EXPECT_FALSE(dt.SetDeviation(-721));
  EXPECT_FALSE(dt.SetDeviation(721));
  EXPECT_TRUE(dt.SetDeviation(-720));
  EXPECT_EQ(-720, dt.Deviation());
  EXPECT_TRUE(dt.SetDeviation(DateTime::DeviationUnspecifiedValue));
  EXPECT_TRUE(dt.DeviationUnspecified());

  // Clock status accepts any byte.
  dt.SetClockStatus(0x00u);
  EXPECT_EQ(0x00u, dt.ClockStatus());
  EXPECT_FALSE(dt.ClockStatusUnspecified());
  dt.SetClockStatus(0xFFu);
  EXPECT_TRUE(dt.ClockStatusUnspecified());
}

TEST(TypesDateTime, EqualityAndInequality)
{
  const auto bytes = SampleBytes();
  DateTime a;
  DateTime b;
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), a));
  ASSERT_TRUE(DateTime::TryFromBytes(bytes.data(), bytes.size(), b));
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  ASSERT_TRUE(b.SetMinute(0u));
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}
