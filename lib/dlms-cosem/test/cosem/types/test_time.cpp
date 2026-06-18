#include "dlms/cosem/types/time.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>

namespace {

using dlms::cosem::types::Time;

std::array<std::uint8_t, 4> SampleBytes()
{
  // 12:34:56.78 — same fields as bytes [5..8] of the DateTime sample.
  return {{0x0Cu, 0x22u, 0x38u, 0x4Eu}};
}

} // namespace

TEST(TypesTime, DefaultConstructedIsAllUnspecified)
{
  Time t;

  EXPECT_TRUE(t.HourUnspecified());
  EXPECT_TRUE(t.MinuteUnspecified());
  EXPECT_TRUE(t.SecondUnspecified());
  EXPECT_TRUE(t.HundredthsUnspecified());

  const std::array<std::uint8_t, 4> expected = {{
    0xFFu, 0xFFu, 0xFFu, 0xFFu}};
  EXPECT_EQ(expected, t.ToBytes());
}

TEST(TypesTime, RoundTripsSampleBytes)
{
  const auto bytes = SampleBytes();

  Time parsed;
  ASSERT_TRUE(Time::TryFromBytes(bytes.data(), bytes.size(), parsed));

  EXPECT_EQ(12u, parsed.Hour());
  EXPECT_EQ(34u, parsed.Minute());
  EXPECT_EQ(56u, parsed.Second());
  EXPECT_EQ(78u, parsed.Hundredths());

  EXPECT_FALSE(parsed.HourUnspecified());
  EXPECT_FALSE(parsed.MinuteUnspecified());

  EXPECT_EQ(bytes, parsed.ToBytes());
}

TEST(TypesTime, AcceptsAllWildcardFields)
{
  const std::array<std::uint8_t, 4> bytes = {{
    0xFFu, 0xFFu, 0xFFu, 0xFFu}};

  Time parsed;
  ASSERT_TRUE(Time::TryFromBytes(bytes.data(), bytes.size(), parsed));
  EXPECT_TRUE(parsed.HourUnspecified());
  EXPECT_TRUE(parsed.MinuteUnspecified());
  EXPECT_TRUE(parsed.SecondUnspecified());
  EXPECT_TRUE(parsed.HundredthsUnspecified());
  EXPECT_EQ(bytes, parsed.ToBytes());
}

TEST(TypesTime, RejectsWrongSize)
{
  const auto bytes = SampleBytes();
  Time t;
  EXPECT_FALSE(Time::TryFromBytes(bytes.data(), 0u, t));
  EXPECT_FALSE(Time::TryFromBytes(bytes.data(), 3u, t));
  EXPECT_FALSE(Time::TryFromBytes(bytes.data(), 5u, t));
  EXPECT_FALSE(Time::TryFromBytes(nullptr, 4u, t));
}

TEST(TypesTime, RejectsOutOfRangeFields)
{
  std::array<std::uint8_t, 4> bytes = SampleBytes();

  bytes[0] = 24u;
  Time t;
  EXPECT_FALSE(Time::TryFromBytes(bytes.data(), bytes.size(), t));
  bytes = SampleBytes();

  bytes[1] = 60u;
  EXPECT_FALSE(Time::TryFromBytes(bytes.data(), bytes.size(), t));
  bytes = SampleBytes();

  bytes[2] = 60u;
  EXPECT_FALSE(Time::TryFromBytes(bytes.data(), bytes.size(), t));
  bytes = SampleBytes();

  bytes[3] = 100u;
  EXPECT_FALSE(Time::TryFromBytes(bytes.data(), bytes.size(), t));
}

TEST(TypesTime, SettersValidateRanges)
{
  Time t;

  EXPECT_FALSE(t.SetHour(24u));
  EXPECT_TRUE(t.SetHour(23u));
  EXPECT_EQ(23u, t.Hour());
  EXPECT_FALSE(t.SetMinute(60u));
  EXPECT_TRUE(t.SetMinute(59u));
  EXPECT_FALSE(t.SetSecond(60u));
  EXPECT_TRUE(t.SetSecond(59u));
  EXPECT_FALSE(t.SetHundredths(100u));
  EXPECT_TRUE(t.SetHundredths(99u));

  EXPECT_TRUE(t.SetHour(Time::HourUnspecifiedValue));
  EXPECT_TRUE(t.HourUnspecified());
}

TEST(TypesTime, EqualityAndInequality)
{
  const auto bytes = SampleBytes();
  Time a;
  Time b;
  ASSERT_TRUE(Time::TryFromBytes(bytes.data(), bytes.size(), a));
  ASSERT_TRUE(Time::TryFromBytes(bytes.data(), bytes.size(), b));
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  ASSERT_TRUE(b.SetSecond(0u));
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}
