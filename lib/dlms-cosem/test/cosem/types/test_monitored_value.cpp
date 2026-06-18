#include "dlms/cosem/types/monitored_value.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::CosemLogicalName;
using dlms::cosem::types::MonitoredValue;

CosemLogicalName SampleLn()
{
  return CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u);
}

} // namespace

TEST(TypesMonitoredValue, DefaultConstructedIsAttrIndex1)
{
  MonitoredValue v;
  EXPECT_EQ(0u, v.ClassId());
  EXPECT_EQ(CosemLogicalName(), v.LogicalName());
  EXPECT_EQ(1, v.AttributeIndex());
  EXPECT_TRUE(MonitoredValue::IsValid(v));
}

TEST(TypesMonitoredValue, ConstructorAcceptsValidIndex)
{
  MonitoredValue v(3u, SampleLn(), 2);
  EXPECT_EQ(3u, v.ClassId());
  EXPECT_EQ(SampleLn(), v.LogicalName());
  EXPECT_EQ(2, v.AttributeIndex());
  EXPECT_TRUE(MonitoredValue::IsValid(v));
}

TEST(TypesMonitoredValue, ConstructorClampsZeroToOne)
{
  // attribute_index 0 means "the whole object" — not a valid target;
  // safe-fallback ctor clamps to the minimum (1).
  MonitoredValue v(3u, SampleLn(), 0);
  EXPECT_EQ(1, v.AttributeIndex());
  EXPECT_TRUE(MonitoredValue::IsValid(v));
}

TEST(TypesMonitoredValue, ConstructorClampsNegativeToOne)
{
  MonitoredValue v(3u, SampleLn(), -5);
  EXPECT_EQ(1, v.AttributeIndex());
}

TEST(TypesMonitoredValue, SetAttributeIndexRejectsBelowOne)
{
  MonitoredValue v(3u, SampleLn(), 2);
  EXPECT_FALSE(v.SetAttributeIndex(0));
  EXPECT_FALSE(v.SetAttributeIndex(-1));
  EXPECT_EQ(2, v.AttributeIndex());
  EXPECT_TRUE(v.SetAttributeIndex(7));
  EXPECT_EQ(7, v.AttributeIndex());
}

TEST(TypesMonitoredValue, Equality)
{
  MonitoredValue a(3u, SampleLn(), 2);
  MonitoredValue b(3u, SampleLn(), 2);
  MonitoredValue c(4u, SampleLn(), 2);
  MonitoredValue d(3u, SampleLn(), 3);
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(a, d);
}
