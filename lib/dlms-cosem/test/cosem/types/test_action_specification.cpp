// Unit tests for types::ActionSpecification (IC 9 building block) per
// IEC 62056-6-2 ED4 (2021) §4.5.2 / DLMS UA Blue Book Ed. 12.1 §4.5.2.

#include "dlms/cosem/types/action_specification.hpp"

#include <gtest/gtest.h>

#include <cstdint>

namespace {

using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::types::ActionSpecification;
using dlms::cosem::types::ScriptServiceId;

const CosemLogicalName kZero(0u, 0u, 0u, 0u, 0u, 0u);
const CosemLogicalName kReg(1u, 0u, 1u, 8u, 0u, 255u);

} // namespace

TEST(TypesActionSpecification, DefaultConstructorIsDummy)
{
  ActionSpecification a;
  EXPECT_EQ(ScriptServiceId::Dummy, a.ServiceId());
  EXPECT_EQ(0u, a.ClassId());
  EXPECT_EQ(kZero, a.LogicalName());
  EXPECT_EQ(0, a.Index());
  EXPECT_TRUE(a.Parameter().empty());
  EXPECT_TRUE(a.IsDummy());
  EXPECT_TRUE(ActionSpecification::IsValid(a));
}

TEST(TypesActionSpecification, DummyWithExtraFieldsIsInvalid)
{
  ActionSpecification a(
    ScriptServiceId::Dummy, 3u, kReg, 0, CosemByteBuffer());
  EXPECT_FALSE(a.IsDummy());
  EXPECT_FALSE(ActionSpecification::IsValid(a));
}

TEST(TypesActionSpecification, WriteAttributeRequiresIndexAtLeastOne)
{
  ActionSpecification good(
    ScriptServiceId::WriteAttribute, 3u, kReg, 2,
    CosemByteBuffer{0x09u, 0x01u, 0x00u});
  EXPECT_TRUE(ActionSpecification::IsValid(good));

  ActionSpecification bad(
    ScriptServiceId::WriteAttribute, 3u, kReg, 0, CosemByteBuffer());
  EXPECT_FALSE(ActionSpecification::IsValid(bad));

  ActionSpecification negative(
    ScriptServiceId::WriteAttribute, 3u, kReg, -1, CosemByteBuffer());
  EXPECT_FALSE(ActionSpecification::IsValid(negative));
}

TEST(TypesActionSpecification, ExecuteMethodRequiresIndexAtLeastOne)
{
  ActionSpecification good(
    ScriptServiceId::ExecuteMethod, 3u, kReg, 1, CosemByteBuffer());
  EXPECT_TRUE(ActionSpecification::IsValid(good));

  ActionSpecification bad(
    ScriptServiceId::ExecuteMethod, 3u, kReg, 0, CosemByteBuffer());
  EXPECT_FALSE(ActionSpecification::IsValid(bad));
}

TEST(TypesActionSpecification, Equality)
{
  ActionSpecification a(
    ScriptServiceId::WriteAttribute, 3u, kReg, 2,
    CosemByteBuffer{0x09u, 0x01u, 0x00u});
  ActionSpecification b(
    ScriptServiceId::WriteAttribute, 3u, kReg, 2,
    CosemByteBuffer{0x09u, 0x01u, 0x00u});
  ActionSpecification c(
    ScriptServiceId::WriteAttribute, 3u, kReg, 3,
    CosemByteBuffer{0x09u, 0x01u, 0x00u});
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
}

TEST(TypesActionSpecification, Setters)
{
  ActionSpecification a;
  a.SetServiceId(ScriptServiceId::ExecuteMethod);
  a.SetClassId(70u);
  a.SetLogicalName(kReg);
  a.SetIndex(1);
  a.SetParameter(CosemByteBuffer{0xAAu});
  EXPECT_TRUE(ActionSpecification::IsValid(a));
  EXPECT_FALSE(a.IsDummy());
  EXPECT_EQ(CosemByteBuffer{0xAAu}, a.Parameter());
}
