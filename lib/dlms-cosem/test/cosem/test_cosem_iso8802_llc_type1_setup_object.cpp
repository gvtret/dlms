// SPDX-License-Identifier: Apache-2.0
//
// Per-IC tests for CosemIso8802LlcType1SetupObject (class_id=57,
// version=0) per IEC 62056-6-2 ED4 (2021) §4.11.2 and DLMS UA
// Blue Book Ed. 12.1 §4.11.2. IC 57 holds a single long-unsigned
// max_octets_ui_pdu attribute (default 128); the class defines
// no specific methods.

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "dlms/cosem/cosem_status.hpp"
#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/simple_objects.hpp"

namespace {

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  return dlms::cosem::CosemByteBuffer(bytes.begin(), bytes.end());
}

dlms::cosem::CosemByteBuffer LongUnsigned(std::uint16_t value)
{
  return BytesFromList({
    0x12u,
    static_cast<std::uint8_t>((value >> 8) & 0xFFu),
    static_cast<std::uint8_t>(value & 0xFFu)});
}

dlms::cosem::CosemByteBuffer EncodedLogicalName(
  const dlms::cosem::CosemLogicalName& name)
{
  return BytesFromList({
    0x09u, 0x06u,
    name[0], name[1], name[2], name[3], name[4], name[5]});
}

dlms::cosem::CosemLogicalName SampleName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 26u, 7u, 0u, 255u);
}

dlms::cosem::CosemIso8802LlcType1SetupObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIso8802LlcType1SetupObject(
    SampleName(),
    /*max_octets_ui_pdu*/ 128u,
    access);
}

} // namespace

TEST(CosemIso8802LlcType1SetupObject, DescriptorAndAccessRights)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(57u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIso8802LlcType1SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(SampleName(), object.Descriptor().key.logicalName);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rights.AttributeAccess(2u));
}

TEST(CosemIso8802LlcType1SetupObject, TypedGetterReflectsCtor)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object(
    SampleName(),
    /*max_octets_ui_pdu*/ 1500u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(1500u, object.MaxOctetsUiPdu());
}

TEST(CosemIso8802LlcType1SetupObject, ReadAttributeEmitsTypedAxdr)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(SampleName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(LongUnsigned(128u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemIso8802LlcType1SetupObject, WriteDecodesLongUnsigned)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, LongUnsigned(1500u)));
  EXPECT_EQ(1500u, object.MaxOctetsUiPdu());

  // Boundary values across the full uint16 range.
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, LongUnsigned(0u)));
  EXPECT_EQ(0u, object.MaxOctetsUiPdu());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, LongUnsigned(0xFFFFu)));
  EXPECT_EQ(0xFFFFu, object.MaxOctetsUiPdu());
}

TEST(CosemIso8802LlcType1SetupObject, WriteRejectsWrongTag)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // unsigned (0x11) instead of long-unsigned (0x12)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x11u, 0x05u})));
  EXPECT_EQ(128u, object.MaxOctetsUiPdu());
}

TEST(CosemIso8802LlcType1SetupObject, WriteRejectsTruncatedInput)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u, 0x05u})));
  EXPECT_EQ(128u, object.MaxOctetsUiPdu());
}

TEST(CosemIso8802LlcType1SetupObject, WriteRejectsTrailingGarbage)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x12u, 0x00u, 0x80u, 0xAAu})));
  EXPECT_EQ(128u, object.MaxOctetsUiPdu());
}

TEST(CosemIso8802LlcType1SetupObject, WriteRejectsEmptyInput)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, dlms::cosem::CosemByteBuffer{}));
  EXPECT_EQ(128u, object.MaxOctetsUiPdu());
}

TEST(CosemIso8802LlcType1SetupObject, WriteLogicalNameAlwaysDenied)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedLogicalName(SampleName())));
}

TEST(CosemIso8802LlcType1SetupObject, WriteUnknownAttributeNotFound)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, LongUnsigned(128u)));
}

TEST(CosemIso8802LlcType1SetupObject, ReadOnlyRejectsWrite)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, LongUnsigned(1500u)));
  EXPECT_EQ(128u, object.MaxOctetsUiPdu());
}

TEST(CosemIso8802LlcType1SetupObject, InvokeMethodAlwaysReturnsMethodNotFound)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemIso8802LlcType1SetupObject, VersionAboveMaxNormalized)
{
  dlms::cosem::CosemIso8802LlcType1SetupObject object(
    SampleName(),
    /*max_octets_ui_pdu*/ 128u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    /*version*/ 99u);
  EXPECT_EQ(
    dlms::cosem::CosemIso8802LlcType1SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}
