// SPDX-License-Identifier: BSD-2-Clause
#include "dlms/cosem/types/object_definition.hpp"

#include <gtest/gtest.h>

namespace dlms::cosem::types {
namespace {

CosemLogicalName MakeName(std::uint8_t a)
{
  return CosemLogicalName(a, 0u, 1u, 0u, 0u, 255u);
}

TEST(ObjectDefinition, DefaultsToZeroClassIdAndDefaultName)
{
  ObjectDefinition def;
  EXPECT_EQ(def.ClassId(), 0u);
  EXPECT_EQ(def.LogicalName(), CosemLogicalName{});
}

TEST(ObjectDefinition, ConstructorStoresFields)
{
  ObjectDefinition def(3u, MakeName(1u));
  EXPECT_EQ(def.ClassId(), 3u);
  EXPECT_EQ(def.LogicalName(), MakeName(1u));
}

TEST(ObjectDefinition, MutatorsUpdateFields)
{
  ObjectDefinition def;
  def.SetClassId(7u);
  def.SetLogicalName(MakeName(2u));
  EXPECT_EQ(def.ClassId(), 7u);
  EXPECT_EQ(def.LogicalName(), MakeName(2u));
}

TEST(ObjectDefinition, EqualityComparesBothFields)
{
  ObjectDefinition a(3u, MakeName(1u));
  ObjectDefinition b(3u, MakeName(1u));
  ObjectDefinition c(4u, MakeName(1u));
  ObjectDefinition d(3u, MakeName(2u));
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(a, d);
}

}  // namespace
}  // namespace dlms::cosem::types
