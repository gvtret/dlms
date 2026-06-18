// SPDX-License-Identifier: BSD-2-Clause
//
// Tests for `dlms::cosem::CosemRegisterObject` (IC "Register",
// class_id=3, version=0) per IEC 62056-6-2 ED4 (2021) \u00a74.3.2.
//
// One IC = one test file (see docs/production_readiness_roadmap.md P2.4).
#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

#include <cstdint>

namespace dlms::cosem {
namespace {

CosemLogicalName MakeName(std::uint8_t group)
{
  return CosemLogicalName(1u, 0u, group, 8u, 0u, 255u);
}

CosemByteBuffer Bytes(std::initializer_list<std::uint8_t> bytes)
{
  return CosemByteBuffer(bytes.begin(), bytes.end());
}

// AXDR encoding of `double-long-unsigned` (tag 0x06) value -- a typical
// shape for IC 3 `value` payloads.
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

// Wire form of `scal_unit_type ::= structure(2) { integer scaler, enum unit }`
CosemByteBuffer EncodeScalerUnit(std::int8_t scaler, std::uint8_t unit)
{
  return Bytes({
    0x02u, 0x02u,
    0x0Fu, static_cast<std::uint8_t>(scaler),
    0x16u, unit,
  });
}

TEST(CosemRegisterObject, DefaultsToClassId3Version0)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(0x1234u),
    types::ScalerUnit(3, 30u),
    AttributeAccessMode::ReadOnly);

  const CosemObjectDescriptor d = obj.Descriptor();
  EXPECT_EQ(d.key.classId, 3u);
  EXPECT_EQ(d.key.version, 0u);
  EXPECT_EQ(d.key.logicalName, MakeName(3u));
}

TEST(CosemRegisterObject, NormalisesOverlongVersionToMaxSupported)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadOnly,
    7u);

  EXPECT_EQ(obj.Descriptor().key.version,
            CosemRegisterObject::MaxSupportedVersion);
}

TEST(CosemRegisterObject, ReadsLogicalNameValueAndEncodedScalerUnit)
{
  const auto value = EncodeDoubleLongUnsigned(0xCAFEBABEu);
  CosemRegisterObject obj(
    MakeName(3u),
    value,
    types::ScalerUnit(-1, 33u),
    AttributeAccessMode::ReadOnly);

  CosemByteBuffer out;
  ASSERT_EQ(obj.ReadAttribute(1u, out), CosemStatus::Ok);
  EXPECT_FALSE(out.empty());  // OBIS encoded.

  ASSERT_EQ(obj.ReadAttribute(2u, out), CosemStatus::Ok);
  EXPECT_EQ(out, value);

  ASSERT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeScalerUnit(-1, 33u));
}

TEST(CosemRegisterObject, UnknownAttributeReportsAttributeNotFound)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadOnly);

  CosemByteBuffer out = Bytes({0xAAu, 0xBBu});
  EXPECT_EQ(obj.ReadAttribute(99u, out), CosemStatus::AttributeNotFound);
  EXPECT_TRUE(out.empty());
}

TEST(CosemRegisterObject, WritingValueAcceptsAnyNonEmptyAxdr)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadAndWrite);

  const auto updated = EncodeDoubleLongUnsigned(0xDEADBEEFu);
  ASSERT_EQ(obj.WriteAttribute(2u, updated), CosemStatus::Ok);
  EXPECT_EQ(obj.Value(), updated);
}

TEST(CosemRegisterObject, WritingEmptyValueIsRejectedAsInvalidArgument)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(7u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(obj.WriteAttribute(2u, CosemByteBuffer()),
            CosemStatus::InvalidArgument);
  EXPECT_EQ(obj.Value(), EncodeDoubleLongUnsigned(7u));
}

TEST(CosemRegisterObject, LogicalNameAndScalerUnitAreReadOnlyOnWrite)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadAndWrite);

  const auto payload = EncodeDoubleLongUnsigned(2u);
  EXPECT_EQ(obj.WriteAttribute(1u, payload), CosemStatus::AccessDenied);
  EXPECT_EQ(obj.WriteAttribute(3u, payload), CosemStatus::AccessDenied);
}

TEST(CosemRegisterObject, WritingUnknownAttributeReportsAttributeNotFound)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(obj.WriteAttribute(99u, EncodeDoubleLongUnsigned(2u)),
            CosemStatus::AttributeNotFound);
}

TEST(CosemRegisterObject, ResetMethodSurfacesUnsupportedFeature)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadOnly);

  CosemByteBuffer in = Bytes({0x0Fu, 0x00u});  // integer(0)
  CosemByteBuffer out = Bytes({0xAAu});
  EXPECT_EQ(obj.InvokeMethod(1u, in, out), CosemStatus::UnsupportedFeature);
  EXPECT_TRUE(out.empty());
}

TEST(CosemRegisterObject, UnknownMethodReportsMethodNotFound)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadOnly);

  CosemByteBuffer in;
  CosemByteBuffer out;
  EXPECT_EQ(obj.InvokeMethod(2u, in, out), CosemStatus::MethodNotFound);
  EXPECT_EQ(obj.InvokeMethod(0u, in, out), CosemStatus::MethodNotFound);
}

TEST(CosemRegisterObject, AccessRightsCarryConstructorMode)
{
  CosemRegisterObject ro(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadOnly);
  CosemRegisterObject rw(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadAndWrite);

  // Logical name and scaler_unit are always read-only.
  EXPECT_EQ(ro.AccessRights().AttributeAccess(1u),
            AttributeAccessMode::ReadOnly);
  EXPECT_EQ(ro.AccessRights().AttributeAccess(3u),
            AttributeAccessMode::ReadOnly);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(1u),
            AttributeAccessMode::ReadOnly);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(3u),
            AttributeAccessMode::ReadOnly);

  // value attribute honours the requested mode.
  EXPECT_EQ(ro.AccessRights().AttributeAccess(2u),
            AttributeAccessMode::ReadOnly);
  EXPECT_EQ(rw.AccessRights().AttributeAccess(2u),
            AttributeAccessMode::ReadAndWrite);
}

TEST(CosemRegisterObject, ConstructorDropsEmptyValueAsSafeFallback)
{
  CosemRegisterObject obj(
    MakeName(3u),
    CosemByteBuffer(),  // invalid: empty AXDR
    types::ScalerUnit(2, 30u),
    AttributeAccessMode::ReadAndWrite);

  EXPECT_TRUE(obj.Value().empty());
  // Backend can publish a real value later via setter.
  EXPECT_TRUE(obj.SetValue(EncodeDoubleLongUnsigned(42u)));
  EXPECT_EQ(obj.Value(), EncodeDoubleLongUnsigned(42u));
}

TEST(CosemRegisterObject, SetValueRejectsEmptyBuffer)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadAndWrite);

  EXPECT_FALSE(obj.SetValue(CosemByteBuffer()));
  EXPECT_EQ(obj.Value(), EncodeDoubleLongUnsigned(1u));
}

TEST(CosemRegisterObject, SetScalerUnitUpdatesEncodedRead)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadOnly);

  obj.SetScalerUnit(types::ScalerUnit(-3, 30u));
  EXPECT_EQ(obj.ScalerUnit(), types::ScalerUnit(-3, 30u));

  CosemByteBuffer out;
  ASSERT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeScalerUnit(-3, 30u));
}

TEST(CosemRegisterObject, ScalerUnitDefaultReadsAsZeroNoUnit)
{
  CosemRegisterObject obj(
    MakeName(3u),
    EncodeDoubleLongUnsigned(1u),
    types::ScalerUnit(),
    AttributeAccessMode::ReadOnly);

  CosemByteBuffer out;
  ASSERT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeScalerUnit(0, 255u));
}

TEST(CosemRegisterObject, IsValidValueStaticChecker)
{
  EXPECT_FALSE(CosemRegisterObject::IsValidValue(CosemByteBuffer()));
  EXPECT_TRUE(CosemRegisterObject::IsValidValue(Bytes({0x00u})));
  EXPECT_TRUE(CosemRegisterObject::IsValidValue(EncodeDoubleLongUnsigned(1u)));
}

}  // namespace
}  // namespace dlms::cosem
