// SPDX-License-Identifier: BSD-2-Clause
#include "dlms/cosem/types/scaler_unit.hpp"

#include <gtest/gtest.h>

namespace dlms::cosem::types {
namespace {

TEST(TypesScalerUnit, DefaultIsNoUnit)
{
  ScalerUnit s;
  EXPECT_EQ(s.Scaler(), 0);
  EXPECT_EQ(s.Unit(), ScalerUnit::NoUnit);
  EXPECT_EQ(s.Unit(), 255u);
}

TEST(TypesScalerUnit, RoundTripsScalerAndUnit)
{
  // Example from Blue Book Ed. 12.1 Table 6: 10^3 * Wh (kWh).
  ScalerUnit kwh(3, 30u);  // unit 30 = Wh per Table 5.
  EXPECT_EQ(kwh.Scaler(), 3);
  EXPECT_EQ(kwh.Unit(), 30u);

  ScalerUnit negScaler(-2, 33u);  // 33 = W per Table 5.
  EXPECT_EQ(negScaler.Scaler(), -2);
  EXPECT_EQ(negScaler.Unit(), 33u);
}

TEST(TypesScalerUnit, MutatorsUpdateFields)
{
  ScalerUnit s;
  s.SetScaler(-1);
  s.SetUnit(35u);
  EXPECT_EQ(s.Scaler(), -1);
  EXPECT_EQ(s.Unit(), 35u);
}

TEST(TypesScalerUnit, EqualityOperatorsCompareBothFields)
{
  EXPECT_EQ(ScalerUnit(0, 255u), ScalerUnit());
  EXPECT_EQ(ScalerUnit(3, 30u), ScalerUnit(3, 30u));
  EXPECT_NE(ScalerUnit(3, 30u), ScalerUnit(2, 30u));
  EXPECT_NE(ScalerUnit(3, 30u), ScalerUnit(3, 31u));
}

}  // namespace
}  // namespace dlms::cosem::types
