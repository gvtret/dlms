#include "dlms/cosem/types/action_set.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::CosemLogicalName;
using dlms::cosem::types::ActionSet;
using dlms::cosem::types::Script;

Script ScrUp() { return Script(CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u), 1u); }
Script ScrDown() { return Script(CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u), 2u); }

} // namespace

TEST(TypesActionSet, DefaultConstructedIsAllZero)
{
  ActionSet s;
  EXPECT_EQ(Script(), s.ActionUp());
  EXPECT_EQ(Script(), s.ActionDown());
}

TEST(TypesActionSet, ConstructAndAccess)
{
  ActionSet s(ScrUp(), ScrDown());
  EXPECT_EQ(ScrUp(), s.ActionUp());
  EXPECT_EQ(ScrDown(), s.ActionDown());
}

TEST(TypesActionSet, Setters)
{
  ActionSet s;
  s.SetActionUp(ScrUp());
  s.SetActionDown(ScrDown());
  EXPECT_EQ(ScrUp(), s.ActionUp());
  EXPECT_EQ(ScrDown(), s.ActionDown());
}

TEST(TypesActionSet, Equality)
{
  ActionSet a(ScrUp(), ScrDown());
  ActionSet b(ScrUp(), ScrDown());
  ActionSet c(ScrDown(), ScrDown());
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
}
