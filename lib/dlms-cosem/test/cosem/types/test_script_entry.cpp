// Unit tests for types::ScriptEntry (IC 9 building block) per
// IEC 62056-6-2 ED4 (2021) §4.5.2 / DLMS UA Blue Book Ed. 12.1 §4.5.2.

#include "dlms/cosem/types/script_entry.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace {

using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::types::ActionSpecification;
using dlms::cosem::types::ScriptEntry;
using dlms::cosem::types::ScriptServiceId;

const CosemLogicalName kReg(1u, 0u, 1u, 8u, 0u, 255u);

ActionSpecification MakeWrite()
{
  return ActionSpecification(
    ScriptServiceId::WriteAttribute, 3u, kReg, 2,
    CosemByteBuffer{0x09u, 0x01u, 0x00u});
}

ActionSpecification MakeBad()
{
  // WriteAttribute with index 0 -> invalid.
  return ActionSpecification(
    ScriptServiceId::WriteAttribute, 3u, kReg, 0, CosemByteBuffer());
}

} // namespace

TEST(TypesScriptEntry, DefaultIsNullScriptWithNoActions)
{
  ScriptEntry e;
  EXPECT_EQ(ScriptEntry::NullScriptIdentifier, e.Identifier());
  EXPECT_TRUE(e.Actions().empty());
  EXPECT_TRUE(ScriptEntry::IsValid(e));
}

TEST(TypesScriptEntry, ConstructorAcceptsValidActions)
{
  std::vector<ActionSpecification> actions = {MakeWrite()};
  ScriptEntry e(7u, actions);
  EXPECT_EQ(7u, e.Identifier());
  ASSERT_EQ(1u, e.Actions().size());
  EXPECT_TRUE(ActionSpecification::IsValid(e.Actions()[0]));
}

TEST(TypesScriptEntry, ConstructorDropsInvalidActions)
{
  std::vector<ActionSpecification> actions = {MakeWrite(), MakeBad()};
  ScriptEntry e(7u, actions);
  EXPECT_EQ(7u, e.Identifier());
  // Safe-fallback: the whole actions vector is dropped on any
  // invariant violation.
  EXPECT_TRUE(e.Actions().empty());
}

TEST(TypesScriptEntry, SetActionsRejectsInvalidAndDoesNotMutate)
{
  ScriptEntry e(7u, {MakeWrite()});
  EXPECT_FALSE(e.SetActions({MakeBad()}));
  ASSERT_EQ(1u, e.Actions().size());
  EXPECT_TRUE(ActionSpecification::IsValid(e.Actions()[0]));
}

TEST(TypesScriptEntry, SetActionsAcceptsValidReplacement)
{
  ScriptEntry e(7u, {MakeWrite()});
  std::vector<ActionSpecification> next = {MakeWrite(), MakeWrite()};
  EXPECT_TRUE(e.SetActions(next));
  EXPECT_EQ(2u, e.Actions().size());
}

TEST(TypesScriptEntry, Equality)
{
  ScriptEntry a(7u, {MakeWrite()});
  ScriptEntry b(7u, {MakeWrite()});
  ScriptEntry c(8u, {MakeWrite()});
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
}
