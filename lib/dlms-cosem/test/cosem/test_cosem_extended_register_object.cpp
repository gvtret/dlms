// SPDX-License-Identifier: BSD-2-Clause
//
// Tests for `dlms::cosem::CosemExtendedRegisterObject` (IC "Extended
// Register", class_id=4, version=0) per IEC 62056-6-2 ED4 (2021)
// §4.3.3 / DLMS UA Blue Book Ed. 12.1 §4.3.3.
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
  return CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u);
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

// Wire form of scal_unit_type ::= structure(2) { integer scaler, enum unit }
CosemByteBuffer EncodeScalerUnitWire(std::int8_t scaler, std::uint8_t unit)
{
  return Bytes({
    0x02u, 0x02u,
    0x0Fu, static_cast<std::uint8_t>(scaler),
    0x16u, unit,
  });
}

// status ::= unsigned-byte (or any DLMS data CHOICE) -- IC 4 keeps it
// opaque for now (discriminated-union migration is a separate IC change).
CosemByteBuffer EncodeUnsignedStatus(std::uint8_t status)
{
  return Bytes({0x11u, status});
}

types::DateTime MakeCaptureTime()
{
  // 2023-06-15 (Thu) 10:30:00 local, no deviation/status.
  types::DateTime dt;
  dt.SetYear(2023u);
  dt.SetMonth(6u);
  dt.SetDayOfMonth(15u);
  dt.SetDayOfWeek(4u);
  dt.SetHour(10u);
  dt.SetMinute(30u);
  dt.SetSecond(0u);
  dt.SetHundredths(0u);
  return dt;
}

CosemByteBuffer EncodeDateTimeWire(const types::DateTime& dt)
{
  const auto bytes = dt.ToBytes();
  CosemByteBuffer out;
  out.push_back(0x09u);  // octet-string tag
  out.push_back(static_cast<std::uint8_t>(bytes.size()));
  out.insert(out.end(), bytes.begin(), bytes.end());
  return out;
}

TEST(CosemExtendedRegisterObject, DefaultsToClassId4Version0)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0x1234u),
    types::ScalerUnit(3, 30u),
    EncodeUnsignedStatus(0x05u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  EXPECT_EQ(obj.Descriptor().key.classId, 4u);
  EXPECT_EQ(obj.Descriptor().key.version, 0u);
  EXPECT_EQ(
    obj.Descriptor().key.version,
    CosemExtendedRegisterObject::MaxSupportedVersion);
}

TEST(CosemExtendedRegisterObject, NormalisesOverlongVersionToMaxSupported)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly,
    99u);
  EXPECT_EQ(
    obj.Descriptor().key.version,
    CosemExtendedRegisterObject::MaxSupportedVersion);
}

TEST(CosemExtendedRegisterObject, ReadsAllAttributesIncludingEncodedScalerAndCaptureTime)
{
  const auto value = EncodeDoubleLongUnsigned(0x1234u);
  const auto status = EncodeUnsignedStatus(0x05u);
  const types::DateTime ct = MakeCaptureTime();
  CosemExtendedRegisterObject obj(
    MakeName(),
    value,
    types::ScalerUnit(3, 30u),
    status,
    ct,
    AttributeAccessMode::ReadOnly);

  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(1u, out), CosemStatus::Ok);  // logical_name
  EXPECT_EQ(obj.ReadAttribute(2u, out), CosemStatus::Ok);  // value
  EXPECT_EQ(out, value);
  EXPECT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);  // scaler_unit
  EXPECT_EQ(out, EncodeScalerUnitWire(3, 30u));
  EXPECT_EQ(obj.ReadAttribute(4u, out), CosemStatus::Ok);  // status
  EXPECT_EQ(out, status);
  EXPECT_EQ(obj.ReadAttribute(5u, out), CosemStatus::Ok);  // capture_time
  EXPECT_EQ(out, EncodeDateTimeWire(ct));
}

TEST(CosemExtendedRegisterObject, UnknownAttributeReportsAttributeNotFound)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(6u, out), CosemStatus::AttributeNotFound);
  EXPECT_TRUE(out.empty());
  EXPECT_EQ(obj.ReadAttribute(99u, out), CosemStatus::AttributeNotFound);
}

TEST(CosemExtendedRegisterObject, ValueWriteAcceptsValidPayload)
{
  CosemExtendedRegisterObject rw(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadAndWrite);
  const auto updated = EncodeDoubleLongUnsigned(0xCAFEu);
  EXPECT_EQ(rw.WriteAttribute(2u, updated), CosemStatus::Ok);
  EXPECT_EQ(rw.Value(), updated);
  // Note: WriteAttribute itself is mode-agnostic per project convention;
  // access-mode enforcement is the caller's responsibility through
  // AccessRights(). See AccessRightsCarryConstructorMode test below.
}

TEST(CosemExtendedRegisterObject, WritingEmptyValueIsRejectedAsInvalidArgument)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(obj.WriteAttribute(2u, CosemByteBuffer()), CosemStatus::InvalidArgument);
}

TEST(CosemExtendedRegisterObject, ReadOnlyAttributesRejectWrites)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadAndWrite);
  const auto payload = EncodeDoubleLongUnsigned(1u);
  EXPECT_EQ(obj.WriteAttribute(1u, payload), CosemStatus::AccessDenied);
  EXPECT_EQ(obj.WriteAttribute(3u, payload), CosemStatus::AccessDenied);
  EXPECT_EQ(obj.WriteAttribute(4u, payload), CosemStatus::AccessDenied);
  EXPECT_EQ(obj.WriteAttribute(5u, payload), CosemStatus::AccessDenied);
  EXPECT_EQ(obj.WriteAttribute(99u, payload), CosemStatus::AttributeNotFound);
}

TEST(CosemExtendedRegisterObject, ResetMethodSurfacesUnsupportedFeature)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  CosemByteBuffer in = Bytes({0x00u});
  CosemByteBuffer out = Bytes({0xAAu, 0xBBu});
  EXPECT_EQ(obj.InvokeMethod(1u, in, out), CosemStatus::UnsupportedFeature);
  EXPECT_TRUE(out.empty());
}

TEST(CosemExtendedRegisterObject, UnknownMethodReportsMethodNotFound)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  CosemByteBuffer in;
  CosemByteBuffer out = Bytes({0xCCu});
  EXPECT_EQ(obj.InvokeMethod(2u, in, out), CosemStatus::MethodNotFound);
  EXPECT_TRUE(out.empty());
}

TEST(CosemExtendedRegisterObject, AccessRightsCarryConstructorMode)
{
  CosemExtendedRegisterObject rw(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadAndWrite);
  CosemExtendedRegisterObject ro(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(1u), AttributeAccessMode::ReadOnly);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(2u), AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(3u), AttributeAccessMode::ReadOnly);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(4u), AttributeAccessMode::ReadOnly);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(5u), AttributeAccessMode::ReadOnly);
  EXPECT_EQ(ro.AccessRights().AttributeAccess(2u), AttributeAccessMode::ReadOnly);
}

TEST(CosemExtendedRegisterObject, ConstructorDropsEmptyValueAsSafeFallback)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    CosemByteBuffer(),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  EXPECT_TRUE(obj.Value().empty());
}

TEST(CosemExtendedRegisterObject, SetValueRejectsEmptyBuffer)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(7u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_FALSE(obj.SetValue(CosemByteBuffer()));
  // Value preserved.
  EXPECT_EQ(obj.Value(), EncodeDoubleLongUnsigned(7u));
  EXPECT_TRUE(obj.SetValue(EncodeDoubleLongUnsigned(8u)));
  EXPECT_EQ(obj.Value(), EncodeDoubleLongUnsigned(8u));
}

TEST(CosemExtendedRegisterObject, SetScalerUnitUpdatesEncodedRead)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  obj.SetScalerUnit(types::ScalerUnit(-2, 33u));
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeScalerUnitWire(-2, 33u));
}

TEST(CosemExtendedRegisterObject, SetCaptureTimeUpdatesEncodedRead)
{
  CosemExtendedRegisterObject obj(
    MakeName(),
    EncodeDoubleLongUnsigned(0u),
    types::ScalerUnit(),
    EncodeUnsignedStatus(0u),
    MakeCaptureTime(),
    AttributeAccessMode::ReadOnly);
  types::DateTime updated;
  updated.SetYear(2024u);
  updated.SetMonth(1u);
  updated.SetDayOfMonth(1u);
  updated.SetDayOfWeek(1u);
  updated.SetHour(0u);
  updated.SetMinute(0u);
  updated.SetSecond(0u);
  updated.SetHundredths(0u);
  obj.SetCaptureTime(updated);
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(5u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeDateTimeWire(updated));
}

TEST(CosemExtendedRegisterObject, IsValidValueStaticChecker)
{
  EXPECT_FALSE(CosemExtendedRegisterObject::IsValidValue(CosemByteBuffer()));
  EXPECT_TRUE(CosemExtendedRegisterObject::IsValidValue(
    EncodeDoubleLongUnsigned(0u)));
}

}  // namespace
}  // namespace dlms::cosem
