#include "dlms/cosem/types/date.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>

namespace {

using dlms::cosem::types::Date;

std::array<std::uint8_t, 5> SampleBytes()
{
  // 2024-06-15 (Saturday=6) — same calendar fields as the DateTime sample.
  return {{0x07u, 0xE8u, 0x06u, 0x0Fu, 0x06u}};
}

} // namespace

TEST(TypesDate, DefaultConstructedIsAllUnspecified)
{
  Date d;

  EXPECT_TRUE(d.YearUnspecified());
  EXPECT_TRUE(d.MonthUnspecified());
  EXPECT_TRUE(d.DayOfMonthUnspecified());
  EXPECT_TRUE(d.DayOfWeekUnspecified());

  const std::array<std::uint8_t, 5> expected = {{
    0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}};
  EXPECT_EQ(expected, d.ToBytes());
}

TEST(TypesDate, RoundTripsSampleBytes)
{
  const auto bytes = SampleBytes();

  Date parsed;
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), parsed));

  EXPECT_EQ(2024u, parsed.Year());
  EXPECT_EQ(6u, parsed.Month());
  EXPECT_EQ(15u, parsed.DayOfMonth());
  EXPECT_EQ(6u, parsed.DayOfWeek());

  EXPECT_FALSE(parsed.YearUnspecified());
  EXPECT_FALSE(parsed.MonthUnspecified());

  EXPECT_EQ(bytes, parsed.ToBytes());
}

TEST(TypesDate, AcceptsAllWildcardFields)
{
  const std::array<std::uint8_t, 5> bytes = {{
    0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}};

  Date parsed;
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_TRUE(parsed.YearUnspecified());
  EXPECT_TRUE(parsed.MonthUnspecified());
  EXPECT_TRUE(parsed.DayOfMonthUnspecified());
  EXPECT_TRUE(parsed.DayOfWeekUnspecified());
  EXPECT_EQ(bytes, parsed.ToBytes());
}

TEST(TypesDate, AcceptsMonthDstSentinels)
{
  std::array<std::uint8_t, 5> bytes = SampleBytes();

  bytes[2] = Date::MonthDstBeginValue;
  Date parsed;
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(Date::MonthDstBeginValue, parsed.Month());

  bytes[2] = Date::MonthDstEndValue;
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(Date::MonthDstEndValue, parsed.Month());
}

TEST(TypesDate, AcceptsLastAndSecondLastDayOfMonth)
{
  std::array<std::uint8_t, 5> bytes = SampleBytes();

  bytes[3] = Date::DayOfMonthLastValue;
  Date parsed;
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(Date::DayOfMonthLastValue, parsed.DayOfMonth());

  bytes[3] = Date::DayOfMonthSecondLastValue;
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_EQ(Date::DayOfMonthSecondLastValue, parsed.DayOfMonth());
}

TEST(TypesDate, RejectsWrongSize)
{
  const auto bytes = SampleBytes();
  Date d;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), 0u, d));
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), 4u, d));
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), 6u, d));
  EXPECT_FALSE(Date::TryFromBytes(nullptr, 5u, d));
}

TEST(TypesDate, RejectsOutOfRangeMonth)
{
  std::array<std::uint8_t, 5> bytes = SampleBytes();
  bytes[2] = 0u;
  Date d;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
  bytes[2] = 13u;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
  bytes[2] = 0xFCu; // Not one of the named sentinels.
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
}

TEST(TypesDate, RejectsReservedDayOfMonth)
{
  std::array<std::uint8_t, 5> bytes = SampleBytes();
  bytes[3] = 0u;
  Date d;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
  bytes[3] = 32u;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
  bytes[3] = 0xE0u; // Reserved range.
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
  bytes[3] = 0xFCu;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
}

TEST(TypesDate, RejectsOutOfRangeDayOfWeek)
{
  std::array<std::uint8_t, 5> bytes = SampleBytes();
  bytes[4] = 0u;
  Date d;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
  bytes[4] = 8u;
  EXPECT_FALSE(Date::TryFromBytes(bytes.data(), bytes.size(), d));
}

TEST(TypesDate, SettersValidateRanges)
{
  Date d;

  EXPECT_TRUE(d.SetMonth(12u));
  EXPECT_EQ(12u, d.Month());
  EXPECT_FALSE(d.SetMonth(0u));
  EXPECT_FALSE(d.SetMonth(13u));
  EXPECT_EQ(12u, d.Month());
  EXPECT_TRUE(d.SetMonth(Date::MonthUnspecifiedValue));
  EXPECT_TRUE(d.MonthUnspecified());

  EXPECT_FALSE(d.SetDayOfMonth(0u));
  EXPECT_FALSE(d.SetDayOfMonth(32u));
  EXPECT_FALSE(d.SetDayOfMonth(0xE5u));
  EXPECT_TRUE(d.SetDayOfMonth(Date::DayOfMonthLastValue));
  EXPECT_EQ(Date::DayOfMonthLastValue, d.DayOfMonth());

  EXPECT_FALSE(d.SetDayOfWeek(0u));
  EXPECT_FALSE(d.SetDayOfWeek(8u));
  EXPECT_TRUE(d.SetDayOfWeek(7u));
  EXPECT_EQ(7u, d.DayOfWeek());

  EXPECT_TRUE(d.SetYear(2024u));
  EXPECT_EQ(2024u, d.Year());
  EXPECT_TRUE(d.SetYear(Date::YearUnspecifiedValue));
  EXPECT_TRUE(d.YearUnspecified());
}

TEST(TypesDate, EqualityAndInequality)
{
  const auto bytes = SampleBytes();
  Date a;
  Date b;
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), a));
  ASSERT_TRUE(Date::TryFromBytes(bytes.data(), bytes.size(), b));
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  ASSERT_TRUE(b.SetDayOfMonth(1u));
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}
