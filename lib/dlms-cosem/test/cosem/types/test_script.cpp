#include "dlms/cosem/types/script.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::CosemLogicalName;
using dlms::cosem::types::Script;

} // namespace

TEST(TypesScript, DefaultConstructedIsEmpty)
{
  Script s;
  EXPECT_TRUE(s.LogicalName().IsEmpty());
  EXPECT_EQ(0u, s.Selector());
}

TEST(TypesScript, ConstructAndAccess)
{
  CosemLogicalName ln(0u, 0u, 10u, 0u, 100u, 255u);
  Script s(ln, 1234u);
  EXPECT_EQ(ln, s.LogicalName());
  EXPECT_EQ(1234u, s.Selector());
}

TEST(TypesScript, SettersUpdateFields)
{
  Script s;
  CosemLogicalName ln(1u, 2u, 3u, 4u, 5u, 6u);
  s.SetLogicalName(ln);
  s.SetSelector(42u);
  EXPECT_EQ(ln, s.LogicalName());
  EXPECT_EQ(42u, s.Selector());
}

TEST(TypesScript, EqualityAndInequality)
{
  CosemLogicalName ln(0u, 0u, 10u, 0u, 100u, 255u);
  Script a(ln, 7u);
  Script b(ln, 7u);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  Script c(ln, 8u);
  EXPECT_FALSE(a == c);
  EXPECT_TRUE(a != c);

  CosemLogicalName ln2(0u, 0u, 10u, 0u, 100u, 254u);
  Script d(ln2, 7u);
  EXPECT_FALSE(a == d);
  EXPECT_TRUE(a != d);
}
