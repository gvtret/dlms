#include "dlms/cosem/types/single_action_schedule_type.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::types::SingleActionScheduleType;

} // namespace

TEST(TypesSingleActionScheduleType, DefaultConstructedIsTypeOne)
{
  SingleActionScheduleType t;
  EXPECT_EQ(1u, t.Value());
  EXPECT_TRUE(t.RequiresSingleEntry());
  EXPECT_FALSE(t.RequiresUniformTime());
  EXPECT_FALSE(t.ForbidsWildcardsInDate());
}

TEST(TypesSingleActionScheduleType, ExplicitConstructorValidates)
{
  EXPECT_EQ(3u, SingleActionScheduleType(3u).Value());
  // Out-of-range falls back to the safe default (1).
  EXPECT_EQ(1u, SingleActionScheduleType(0u).Value());
  EXPECT_EQ(1u, SingleActionScheduleType(6u).Value());
}

TEST(TypesSingleActionScheduleType, IsValidMatchesRange)
{
  EXPECT_FALSE(SingleActionScheduleType::IsValid(0u));
  EXPECT_TRUE(SingleActionScheduleType::IsValid(1u));
  EXPECT_TRUE(SingleActionScheduleType::IsValid(2u));
  EXPECT_TRUE(SingleActionScheduleType::IsValid(3u));
  EXPECT_TRUE(SingleActionScheduleType::IsValid(4u));
  EXPECT_TRUE(SingleActionScheduleType::IsValid(5u));
  EXPECT_FALSE(SingleActionScheduleType::IsValid(6u));
  EXPECT_FALSE(SingleActionScheduleType::IsValid(0xFFu));
}

TEST(TypesSingleActionScheduleType, SetValueValidatesAndKeepsOnFailure)
{
  SingleActionScheduleType t;
  EXPECT_TRUE(t.SetValue(4u));
  EXPECT_EQ(4u, t.Value());
  EXPECT_FALSE(t.SetValue(0u));
  EXPECT_EQ(4u, t.Value());
  EXPECT_FALSE(t.SetValue(6u));
  EXPECT_EQ(4u, t.Value());
  EXPECT_TRUE(t.SetValue(5u));
  EXPECT_EQ(5u, t.Value());
}

TEST(TypesSingleActionScheduleType, InvariantsPerSpecValue)
{
  SingleActionScheduleType t;

  ASSERT_TRUE(t.SetValue(1u));
  EXPECT_TRUE(t.RequiresSingleEntry());
  EXPECT_FALSE(t.RequiresUniformTime());
  EXPECT_FALSE(t.ForbidsWildcardsInDate());

  ASSERT_TRUE(t.SetValue(2u));
  EXPECT_FALSE(t.RequiresSingleEntry());
  EXPECT_TRUE(t.RequiresUniformTime());
  EXPECT_TRUE(t.ForbidsWildcardsInDate());

  ASSERT_TRUE(t.SetValue(3u));
  EXPECT_FALSE(t.RequiresSingleEntry());
  EXPECT_TRUE(t.RequiresUniformTime());
  EXPECT_FALSE(t.ForbidsWildcardsInDate());

  ASSERT_TRUE(t.SetValue(4u));
  EXPECT_FALSE(t.RequiresSingleEntry());
  EXPECT_FALSE(t.RequiresUniformTime());
  EXPECT_TRUE(t.ForbidsWildcardsInDate());

  ASSERT_TRUE(t.SetValue(5u));
  EXPECT_FALSE(t.RequiresSingleEntry());
  EXPECT_FALSE(t.RequiresUniformTime());
  EXPECT_FALSE(t.ForbidsWildcardsInDate());
}

TEST(TypesSingleActionScheduleType, EqualityAndInequality)
{
  SingleActionScheduleType a(3u);
  SingleActionScheduleType b(3u);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  SingleActionScheduleType c(4u);
  EXPECT_FALSE(a == c);
  EXPECT_TRUE(a != c);
}
