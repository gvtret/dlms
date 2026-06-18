// SPDX-License-Identifier: BSD-2-Clause
//
// Tests for `dlms::cosem::CosemDemandRegisterObject` (IC "Demand
// Register", class_id=5, version=0) per IEC 62056-6-2 ED4 (2021)
// §4.3.4 / DLMS UA Blue Book Ed. 12.1 §4.3.4.
//
// One IC = one test file (see docs/production_readiness_roadmap.md P2.4).
#include "dlms/cosem/simple_objects.hpp"
#include "dlms/cosem/types/date_time.hpp"
#include "dlms/cosem/types/scaler_unit.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>

namespace dlms::cosem {
namespace {

CosemLogicalName MakeName()
{
  return CosemLogicalName(1u, 0u, 31u, 4u, 0u, 255u);
}

CosemByteBuffer Bytes(std::initializer_list<std::uint8_t> bytes)
{
  return CosemByteBuffer(bytes.begin(), bytes.end());
}

CosemByteBuffer EncodeDoubleLongUnsigned(std::uint32_t v)
{
  return Bytes({
    0x06u,
    static_cast<std::uint8_t>((v >> 24) & 0xFFu),
    static_cast<std::uint8_t>((v >> 16) & 0xFFu),
    static_cast<std::uint8_t>((v >> 8) & 0xFFu),
    static_cast<std::uint8_t>(v & 0xFFu),
  });
}

CosemByteBuffer EncodeLongUnsigned(std::uint16_t v)
{
  return Bytes({
    0x12u,
    static_cast<std::uint8_t>((v >> 8) & 0xFFu),
    static_cast<std::uint8_t>(v & 0xFFu),
  });
}

// scal_unit_type ::= structure(2) { integer scaler, enum unit }
CosemByteBuffer EncodeScalerUnitWire(std::int8_t scaler, std::uint8_t unit)
{
  return Bytes({
    0x02u, 0x02u,
    0x0Fu, static_cast<std::uint8_t>(scaler),
    0x16u, unit,
  });
}

CosemByteBuffer EncodeUnsignedStatus(std::uint8_t status)
{
  return Bytes({0x11u, status});
}

types::DateTime MakeTimestamp(std::uint8_t hour)
{
  types::DateTime dt;
  dt.SetYear(2023u);
  dt.SetMonth(6u);
  dt.SetDayOfMonth(15u);
  dt.SetDayOfWeek(4u);
  dt.SetHour(hour);
  dt.SetMinute(0u);
  dt.SetSecond(0u);
  dt.SetHundredths(0u);
  return dt;
}

CosemByteBuffer EncodeDateTimeWire(const types::DateTime& dt)
{
  const auto bytes = dt.ToBytes();
  CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(static_cast<std::uint8_t>(bytes.size()));
  out.insert(out.end(), bytes.begin(), bytes.end());
  return out;
}

TEST(CosemDemandRegisterObject, DefaultsToClassId5Version0)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0x1234u),
    EncodeDoubleLongUnsigned(0x5678u),
    types::ScalerUnit(0, 30u),
    EncodeUnsignedStatus(0x05u),
    MakeTimestamp(10u),
    MakeTimestamp(9u),
    60u,
    15u);
  EXPECT_EQ(obj.Descriptor().key.classId, 5u);
  EXPECT_EQ(obj.Descriptor().key.version, 0u);
  EXPECT_EQ(
    obj.Descriptor().key.version,
    CosemDemandRegisterObject::MaxSupportedVersion);
}

TEST(CosemDemandRegisterObject, NormalisesOverlongVersionToMaxSupported)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u,
    99u);
  EXPECT_EQ(
    obj.Descriptor().key.version,
    CosemDemandRegisterObject::MaxSupportedVersion);
}

TEST(CosemDemandRegisterObject, ReadsAllAttributesIncludingEncodedScalerAndTimes)
{
  const auto cur = EncodeDoubleLongUnsigned(0x1234u);
  const auto last = EncodeDoubleLongUnsigned(0x5678u);
  const auto status = EncodeUnsignedStatus(0x05u);
  const auto ct = MakeTimestamp(10u);
  const auto st = MakeTimestamp(9u);
  CosemDemandRegisterObject obj(
    MakeName(), cur, last, types::ScalerUnit(3, 30u), status, ct, st,
    60u, 15u);

  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(1u, out), CosemStatus::Ok);  // logical_name
  EXPECT_EQ(obj.ReadAttribute(2u, out), CosemStatus::Ok);  // current_average_value
  EXPECT_EQ(out, cur);
  EXPECT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);  // last_average_value
  EXPECT_EQ(out, last);
  EXPECT_EQ(obj.ReadAttribute(4u, out), CosemStatus::Ok);  // scaler_unit
  EXPECT_EQ(out, EncodeScalerUnitWire(3, 30u));
  EXPECT_EQ(obj.ReadAttribute(5u, out), CosemStatus::Ok);  // status
  EXPECT_EQ(out, status);
  EXPECT_EQ(obj.ReadAttribute(6u, out), CosemStatus::Ok);  // capture_time
  EXPECT_EQ(out, EncodeDateTimeWire(ct));
  EXPECT_EQ(obj.ReadAttribute(7u, out), CosemStatus::Ok);  // start_time_current
  EXPECT_EQ(out, EncodeDateTimeWire(st));
  EXPECT_EQ(obj.ReadAttribute(8u, out), CosemStatus::Ok);  // period
  EXPECT_EQ(out, EncodeDoubleLongUnsigned(60u));
  EXPECT_EQ(obj.ReadAttribute(9u, out), CosemStatus::Ok);  // number_of_periods
  EXPECT_EQ(out, EncodeLongUnsigned(15u));
}

TEST(CosemDemandRegisterObject, UnknownAttributeReportsAttributeNotFound)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(10u, out), CosemStatus::AttributeNotFound);
  EXPECT_TRUE(out.empty());
  EXPECT_EQ(obj.ReadAttribute(99u, out), CosemStatus::AttributeNotFound);
}

TEST(CosemDemandRegisterObject, AllAttributesAreReadOnly)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  const auto probe = EncodeDoubleLongUnsigned(1u);
  for (std::uint8_t attr = 1u; attr <= 9u; ++attr) {
    EXPECT_EQ(obj.WriteAttribute(attr, probe), CosemStatus::AccessDenied);
    EXPECT_EQ(obj.AccessRights().AttributeAccess(attr),
              AttributeAccessMode::ReadOnly);
  }
  EXPECT_EQ(obj.WriteAttribute(99u, probe), CosemStatus::AttributeNotFound);
}

TEST(CosemDemandRegisterObject, ResetAndNextPeriodSurfaceUnsupportedFeature)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  CosemByteBuffer in = Bytes({0x00u});
  CosemByteBuffer out = Bytes({0xAAu, 0xBBu});
  EXPECT_EQ(obj.InvokeMethod(1u, in, out), CosemStatus::UnsupportedFeature);
  EXPECT_TRUE(out.empty());
  out = Bytes({0xCCu});
  EXPECT_EQ(obj.InvokeMethod(2u, in, out), CosemStatus::UnsupportedFeature);
  EXPECT_TRUE(out.empty());
}

TEST(CosemDemandRegisterObject, UnknownMethodReportsMethodNotFound)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  CosemByteBuffer in;
  CosemByteBuffer out = Bytes({0xDDu});
  EXPECT_EQ(obj.InvokeMethod(3u, in, out), CosemStatus::MethodNotFound);
  EXPECT_TRUE(out.empty());
}

TEST(CosemDemandRegisterObject, ConstructorDropsEmptyAverageValuesAsSafeFallback)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    CosemByteBuffer(),  // empty current_average_value
    CosemByteBuffer(),  // empty last_average_value
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  EXPECT_TRUE(obj.CurrentAverageValue().empty());
  EXPECT_TRUE(obj.LastAverageValue().empty());
}

TEST(CosemDemandRegisterObject, SettersRejectEmptyAverageValues)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(7u),
    EncodeDoubleLongUnsigned(8u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  EXPECT_FALSE(obj.SetCurrentAverageValue(CosemByteBuffer()));
  EXPECT_FALSE(obj.SetLastAverageValue(CosemByteBuffer()));
  // Preserved.
  EXPECT_EQ(obj.CurrentAverageValue(), EncodeDoubleLongUnsigned(7u));
  EXPECT_EQ(obj.LastAverageValue(), EncodeDoubleLongUnsigned(8u));
  EXPECT_TRUE(obj.SetCurrentAverageValue(EncodeDoubleLongUnsigned(11u)));
  EXPECT_TRUE(obj.SetLastAverageValue(EncodeDoubleLongUnsigned(12u)));
  EXPECT_EQ(obj.CurrentAverageValue(), EncodeDoubleLongUnsigned(11u));
  EXPECT_EQ(obj.LastAverageValue(), EncodeDoubleLongUnsigned(12u));
}

TEST(CosemDemandRegisterObject, SetScalerUnitUpdatesEncodedRead)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  obj.SetScalerUnit(types::ScalerUnit(-2, 33u));
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(4u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeScalerUnitWire(-2, 33u));
}

TEST(CosemDemandRegisterObject, SetCaptureTimeAndStartTimeUpdateEncodedReads)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  const auto ct = MakeTimestamp(12u);
  const auto st = MakeTimestamp(11u);
  obj.SetCaptureTime(ct);
  obj.SetStartTimeCurrent(st);
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(6u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeDateTimeWire(ct));
  EXPECT_EQ(obj.ReadAttribute(7u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeDateTimeWire(st));
}

TEST(CosemDemandRegisterObject, SetPeriodAndNumberOfPeriodsUpdateEncodedReads)
{
  CosemDemandRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeTimestamp(0u),
    MakeTimestamp(0u),
    60u,
    15u);
  obj.SetPeriod(300u);
  obj.SetNumberOfPeriods(7u);
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(8u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeDoubleLongUnsigned(300u));
  EXPECT_EQ(obj.ReadAttribute(9u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeLongUnsigned(7u));
}

TEST(CosemDemandRegisterObject, IsValidAverageValueStaticChecker)
{
  EXPECT_FALSE(CosemDemandRegisterObject::IsValidAverageValue(CosemByteBuffer()));
  EXPECT_TRUE(CosemDemandRegisterObject::IsValidAverageValue(
    EncodeDoubleLongUnsigned(0u)));
}

}  // namespace
}  // namespace dlms::cosem
