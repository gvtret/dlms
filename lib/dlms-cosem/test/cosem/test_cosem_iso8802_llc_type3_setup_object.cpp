// SPDX-License-Identifier: Apache-2.0
//
// Per-IC tests for CosemIso8802LlcType3SetupObject (class_id=59,
// version=0) per IEC 62056-6-2 ED4 (2021) §4.11.4 and DLMS UA
// Blue Book Ed. 12.1 §4.11.4. IC 59 holds five typed attributes
// for ISO/IEC 8802-2 LLC Type 3 acknowledged-connectionless
// operation; the class defines no specific methods.

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>

#include "dlms/cosem/cosem_status.hpp"
#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/simple_objects.hpp"

namespace {

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  return dlms::cosem::CosemByteBuffer(bytes.begin(), bytes.end());
}

dlms::cosem::CosemByteBuffer Unsigned(std::uint8_t value)
{
  return BytesFromList({0x11u, value});
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
  return dlms::cosem::CosemLogicalName(0u, 0u, 26u, 9u, 0u, 255u);
}

dlms::cosem::CosemIso8802LlcType3SetupObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIso8802LlcType3SetupObject(
    SampleName(),
    /*max_octets_acn_pdu_n3*/ 128u,
    /*max_number_transmissions_n4*/ 3u,
    /*acknowledgement_time_t1*/ 2u,
    /*receive_lifetime_var_t2*/ 60u,
    /*transmit_lifetime_var_t3*/ 60u,
    access);
}

} // namespace

TEST(CosemIso8802LlcType3SetupObject, DescriptorAndAccessRights)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(59u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIso8802LlcType3SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(SampleName(), object.Descriptor().key.logicalName);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id = 2u; id <= 6u; ++id) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "attr id " << static_cast<unsigned>(id);
  }
}

TEST(CosemIso8802LlcType3SetupObject, TypedGettersReflectCtor)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object(
    SampleName(),
    /*max_octets_acn_pdu_n3*/ 256u,
    /*max_number_transmissions_n4*/ 7u,
    /*acknowledgement_time_t1*/ 5u,
    /*receive_lifetime_var_t2*/ 120u,
    /*transmit_lifetime_var_t3*/ 180u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(256u, object.MaxOctetsAcnPduN3());
  EXPECT_EQ(7u, object.MaxNumberTransmissionsN4());
  EXPECT_EQ(5u, object.AcknowledgementTimeT1());
  EXPECT_EQ(120u, object.ReceiveLifetimeVarT2());
  EXPECT_EQ(180u, object.TransmitLifetimeVarT3());
}

TEST(CosemIso8802LlcType3SetupObject, ReadAttributeEmitsTypedAxdr)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(SampleName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(LongUnsigned(128u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, out));
  EXPECT_EQ(Unsigned(3u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, out));
  EXPECT_EQ(LongUnsigned(2u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, out));
  EXPECT_EQ(LongUnsigned(60u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, out));
  EXPECT_EQ(LongUnsigned(60u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemIso8802LlcType3SetupObject, WriteDecodesTypedValues)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, LongUnsigned(1500u)));
  EXPECT_EQ(1500u, object.MaxOctetsAcnPduN3());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, Unsigned(0u)));
  EXPECT_EQ(0u, object.MaxNumberTransmissionsN4());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, Unsigned(255u)));
  EXPECT_EQ(255u, object.MaxNumberTransmissionsN4());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, LongUnsigned(0xFFFFu)));
  EXPECT_EQ(0xFFFFu, object.AcknowledgementTimeT1());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(5u, LongUnsigned(0u)));
  EXPECT_EQ(0u, object.ReceiveLifetimeVarT2());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(6u, LongUnsigned(12345u)));
  EXPECT_EQ(12345u, object.TransmitLifetimeVarT3());
}

TEST(CosemIso8802LlcType3SetupObject, WriteRejectsWrongTag)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // long-unsigned (0x12) sent where unsigned (0x11) is expected.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, LongUnsigned(3u)));
  EXPECT_EQ(3u, object.MaxNumberTransmissionsN4());

  // unsigned (0x11) sent where long-unsigned (0x12) is expected.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, Unsigned(5u)));
  EXPECT_EQ(128u, object.MaxOctetsAcnPduN3());

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, Unsigned(5u)));
  EXPECT_EQ(2u, object.AcknowledgementTimeT1());

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(5u, Unsigned(5u)));
  EXPECT_EQ(60u, object.ReceiveLifetimeVarT2());

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, Unsigned(5u)));
  EXPECT_EQ(60u, object.TransmitLifetimeVarT3());
}

TEST(CosemIso8802LlcType3SetupObject, WriteRejectsTruncatedInput)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u, 0x00u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, BytesFromList({0x11u})));
  EXPECT_EQ(128u, object.MaxOctetsAcnPduN3());
  EXPECT_EQ(3u, object.MaxNumberTransmissionsN4());
}

TEST(CosemIso8802LlcType3SetupObject, WriteRejectsTrailingGarbage)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x12u, 0x00u, 0x80u, 0xAAu})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, BytesFromList({0x11u, 0x03u, 0xAAu})));
  EXPECT_EQ(128u, object.MaxOctetsAcnPduN3());
  EXPECT_EQ(3u, object.MaxNumberTransmissionsN4());
}

TEST(CosemIso8802LlcType3SetupObject, WriteRejectsEmptyInput)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  for (std::uint8_t id = 2u; id <= 6u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, dlms::cosem::CosemByteBuffer{}))
      << "attr id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(128u, object.MaxOctetsAcnPduN3());
  EXPECT_EQ(3u, object.MaxNumberTransmissionsN4());
}

TEST(CosemIso8802LlcType3SetupObject, WriteLogicalNameAlwaysDenied)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedLogicalName(SampleName())));
}

TEST(CosemIso8802LlcType3SetupObject, WriteUnknownAttributeNotFound)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(7u, LongUnsigned(128u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, LongUnsigned(128u)));
}

TEST(CosemIso8802LlcType3SetupObject, ReadOnlyRejectsWritesAcrossAttributes)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, LongUnsigned(1500u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(3u, Unsigned(7u)));
  for (std::uint8_t id = 4u; id <= 6u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              object.WriteAttribute(id, LongUnsigned(99u)))
      << "attr id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(128u, object.MaxOctetsAcnPduN3());
  EXPECT_EQ(3u, object.MaxNumberTransmissionsN4());
  EXPECT_EQ(2u, object.AcknowledgementTimeT1());
  EXPECT_EQ(60u, object.ReceiveLifetimeVarT2());
  EXPECT_EQ(60u, object.TransmitLifetimeVarT3());
}

TEST(CosemIso8802LlcType3SetupObject, InvokeMethodAlwaysReturnsMethodNotFound)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object =
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

TEST(CosemIso8802LlcType3SetupObject, VersionAboveMaxNormalized)
{
  dlms::cosem::CosemIso8802LlcType3SetupObject object(
    SampleName(),
    /*max_octets_acn_pdu_n3*/ 128u,
    /*max_number_transmissions_n4*/ 3u,
    /*acknowledgement_time_t1*/ 2u,
    /*receive_lifetime_var_t2*/ 60u,
    /*transmit_lifetime_var_t3*/ 60u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    /*version*/ 99u);
  EXPECT_EQ(
    dlms::cosem::CosemIso8802LlcType3SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}
