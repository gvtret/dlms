#include "dlms/cosem/types/special_day_entry.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::types::Date;
using dlms::cosem::types::SpecialDayEntry;

Date MakeConcreteDate()
{
  Date d;
  EXPECT_TRUE(d.SetYear(2024u));
  EXPECT_TRUE(d.SetMonth(12u));
  EXPECT_TRUE(d.SetDayOfMonth(25u));
  return d;
}

Date MakeWildcardYearDate()
{
  Date d;
  // year defaults to YearUnspecifiedValue; only set month/day.
  EXPECT_TRUE(d.SetMonth(12u));
  EXPECT_TRUE(d.SetDayOfMonth(25u));
  return d;
}

} // namespace

TEST(TypesSpecialDayEntry, DefaultConstructedIsZeroAndUnspecified)
{
  SpecialDayEntry e;
  EXPECT_EQ(0u, e.Index());
  EXPECT_EQ(0u, e.DayId());
  EXPECT_TRUE(e.SpecialDayDate().YearUnspecified());
}

TEST(TypesSpecialDayEntry, ConstructAndAccess)
{
  const Date d = MakeWildcardYearDate(); // recurring Christmas
  SpecialDayEntry e(42u, d, 3u);
  EXPECT_EQ(42u, e.Index());
  EXPECT_EQ(d, e.SpecialDayDate());
  EXPECT_EQ(3u, e.DayId());
  EXPECT_TRUE(e.SpecialDayDate().YearUnspecified());
}

TEST(TypesSpecialDayEntry, SettersUpdateFields)
{
  SpecialDayEntry e;
  e.SetIndex(7u);
  e.SetSpecialDayDate(MakeConcreteDate());
  e.SetDayId(9u);
  EXPECT_EQ(7u, e.Index());
  EXPECT_EQ(MakeConcreteDate(), e.SpecialDayDate());
  EXPECT_EQ(9u, e.DayId());
}

TEST(TypesSpecialDayEntry, EqualityAndInequality)
{
  const SpecialDayEntry a(1u, MakeConcreteDate(), 5u);
  const SpecialDayEntry b(1u, MakeConcreteDate(), 5u);
  EXPECT_EQ(a, b);

  SpecialDayEntry diffIndex = a;
  diffIndex.SetIndex(2u);
  EXPECT_NE(a, diffIndex);

  SpecialDayEntry diffDate = a;
  diffDate.SetSpecialDayDate(MakeWildcardYearDate());
  EXPECT_NE(a, diffDate);

  SpecialDayEntry diffDay = a;
  diffDay.SetDayId(6u);
  EXPECT_NE(a, diffDay);
}
