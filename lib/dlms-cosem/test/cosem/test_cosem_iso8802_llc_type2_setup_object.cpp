// SPDX-License-Identifier: Apache-2.0
//
// Per-IC tests for CosemIso8802LlcType2SetupObject (class_id=58,
// version=0) per IEC 62056-6-2 ED4 (2021) §4.11.3 and DLMS UA
// Blue Book Ed. 12.1 §4.11.3. IC 58 carries nine attributes for
// connection-oriented LLC Type 2 operation. Parameter semantics
// per ISO/IEC 8802-2:1998 §7.8.1..7.8.4 give k/Rw a closed
// range of 1..127; other timer/size attributes accept the full
// numeric range. The class defines no specific methods.

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
  return dlms::cosem::CosemLogicalName(0u, 0u, 26u, 8u, 0u, 255u);
}

dlms::cosem::CosemIso8802LlcType2SetupObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIso8802LlcType2SetupObject(
    SampleName(),
    /*transmit_window_size_k*/ 1u,
    /*receive_window_size_rw*/ 1u,
    /*max_octets_i_pdu_n1*/ 128u,
    /*max_number_transmissions_n2*/ 8u,
    /*acknowledgement_timer*/ 5u,
    /*p_bit_timer*/ 5u,
    /*reject_timer*/ 5u,
    /*busy_state_timer*/ 10u,
    access);
}

} // namespace

TEST(CosemIso8802LlcType2SetupObject, DescriptorAndAccessRights)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(58u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIso8802LlcType2SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(SampleName(), object.Descriptor().key.logicalName);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id = 2u; id <= 9u; ++id) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "attr id " << static_cast<unsigned>(id);
  }
}

TEST(CosemIso8802LlcType2SetupObject, TypedGettersReflectCtor)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object(
    SampleName(),
    /*k*/ 7u,
    /*Rw*/ 11u,
    /*n1*/ 256u,
    /*n2*/ 3u,
    /*ack*/ 4u,
    /*P-bit*/ 6u,
    /*reject*/ 8u,
    /*busy*/ 12u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(7u, object.TransmitWindowSizeK());
  EXPECT_EQ(11u, object.ReceiveWindowSizeRw());
  EXPECT_EQ(256u, object.MaxOctetsIPduN1());
  EXPECT_EQ(3u, object.MaxNumberTransmissionsN2());
  EXPECT_EQ(4u, object.AcknowledgementTimer());
  EXPECT_EQ(6u, object.PBitTimer());
  EXPECT_EQ(8u, object.RejectTimer());
  EXPECT_EQ(12u, object.BusyStateTimer());
}

TEST(CosemIso8802LlcType2SetupObject, IsValidWindowSizeBoundaries)
{
  EXPECT_FALSE(
    dlms::cosem::CosemIso8802LlcType2SetupObject::IsValidWindowSize(0u));
  EXPECT_TRUE(
    dlms::cosem::CosemIso8802LlcType2SetupObject::IsValidWindowSize(1u));
  EXPECT_TRUE(
    dlms::cosem::CosemIso8802LlcType2SetupObject::IsValidWindowSize(127u));
  EXPECT_FALSE(
    dlms::cosem::CosemIso8802LlcType2SetupObject::IsValidWindowSize(128u));
  EXPECT_FALSE(
    dlms::cosem::CosemIso8802LlcType2SetupObject::IsValidWindowSize(255u));
}

TEST(CosemIso8802LlcType2SetupObject, CtorNormalizesInvalidWindowSize)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject zero(
    SampleName(), 0u, 0u, 128u, 8u, 5u, 5u, 5u, 10u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(1u, zero.TransmitWindowSizeK());
  EXPECT_EQ(1u, zero.ReceiveWindowSizeRw());

  dlms::cosem::CosemIso8802LlcType2SetupObject over(
    SampleName(), 200u, 250u, 128u, 8u, 5u, 5u, 5u, 10u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(1u, over.TransmitWindowSizeK());
  EXPECT_EQ(1u, over.ReceiveWindowSizeRw());
}

TEST(CosemIso8802LlcType2SetupObject, ReadAttributeEmitsTypedAxdr)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(SampleName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(Unsigned(1u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, out));
  EXPECT_EQ(Unsigned(1u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, out));
  EXPECT_EQ(LongUnsigned(128u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, out));
  EXPECT_EQ(Unsigned(8u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, out));
  EXPECT_EQ(LongUnsigned(5u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(7u, out));
  EXPECT_EQ(LongUnsigned(5u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(8u, out));
  EXPECT_EQ(LongUnsigned(5u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(9u, out));
  EXPECT_EQ(LongUnsigned(10u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, out));
}

TEST(CosemIso8802LlcType2SetupObject, WriteDecodesTypedValues)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, Unsigned(127u)));
  EXPECT_EQ(127u, object.TransmitWindowSizeK());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, Unsigned(64u)));
  EXPECT_EQ(64u, object.ReceiveWindowSizeRw());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, LongUnsigned(0xFFFFu)));
  EXPECT_EQ(0xFFFFu, object.MaxOctetsIPduN1());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(5u, Unsigned(0u)));
  EXPECT_EQ(0u, object.MaxNumberTransmissionsN2());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(5u, Unsigned(255u)));
  EXPECT_EQ(255u, object.MaxNumberTransmissionsN2());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(6u, LongUnsigned(0u)));
  EXPECT_EQ(0u, object.AcknowledgementTimer());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(7u, LongUnsigned(1234u)));
  EXPECT_EQ(1234u, object.PBitTimer());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(8u, LongUnsigned(9999u)));
  EXPECT_EQ(9999u, object.RejectTimer());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(9u, LongUnsigned(0xFFFFu)));
  EXPECT_EQ(0xFFFFu, object.BusyStateTimer());
}

TEST(CosemIso8802LlcType2SetupObject, WriteRejectsInvalidWindowSize)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, Unsigned(0u)));
  EXPECT_EQ(1u, object.TransmitWindowSizeK());
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, Unsigned(128u)));
  EXPECT_EQ(1u, object.TransmitWindowSizeK());
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, Unsigned(200u)));
  EXPECT_EQ(1u, object.ReceiveWindowSizeRw());
}

TEST(CosemIso8802LlcType2SetupObject, WriteRejectsWrongTag)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, LongUnsigned(1u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, LongUnsigned(1u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, Unsigned(5u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(5u, LongUnsigned(8u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, Unsigned(5u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(7u, Unsigned(5u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(8u, Unsigned(5u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(9u, Unsigned(10u)));

  EXPECT_EQ(1u, object.TransmitWindowSizeK());
  EXPECT_EQ(1u, object.ReceiveWindowSizeRw());
  EXPECT_EQ(128u, object.MaxOctetsIPduN1());
  EXPECT_EQ(8u, object.MaxNumberTransmissionsN2());
  EXPECT_EQ(5u, object.AcknowledgementTimer());
  EXPECT_EQ(5u, object.PBitTimer());
  EXPECT_EQ(5u, object.RejectTimer());
  EXPECT_EQ(10u, object.BusyStateTimer());
}

TEST(CosemIso8802LlcType2SetupObject, WriteRejectsTruncatedAndTrailing)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x11u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x12u, 0x00u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              4u, BytesFromList({0x12u, 0x00u, 0x80u, 0xAAu})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              5u, BytesFromList({0x11u, 0x08u, 0xAAu})));
  EXPECT_EQ(1u, object.TransmitWindowSizeK());
  EXPECT_EQ(128u, object.MaxOctetsIPduN1());
  EXPECT_EQ(8u, object.MaxNumberTransmissionsN2());
}

TEST(CosemIso8802LlcType2SetupObject, WriteRejectsEmptyInput)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  for (std::uint8_t id = 2u; id <= 9u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, dlms::cosem::CosemByteBuffer{}))
      << "attr id " << static_cast<unsigned>(id);
  }
}

TEST(CosemIso8802LlcType2SetupObject, WriteLogicalNameAlwaysDenied)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedLogicalName(SampleName())));
}

TEST(CosemIso8802LlcType2SetupObject, WriteUnknownAttributeNotFound)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(10u, LongUnsigned(0u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, LongUnsigned(0u)));
}

TEST(CosemIso8802LlcType2SetupObject, ReadOnlyRejectsWritesAcrossAttributes)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, Unsigned(1u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(3u, Unsigned(1u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(4u, LongUnsigned(128u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(5u, Unsigned(8u)));
  for (std::uint8_t id = 6u; id <= 9u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              object.WriteAttribute(id, LongUnsigned(0u)))
      << "attr id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(1u, object.TransmitWindowSizeK());
  EXPECT_EQ(128u, object.MaxOctetsIPduN1());
  EXPECT_EQ(10u, object.BusyStateTimer());
}

TEST(CosemIso8802LlcType2SetupObject, InvokeMethodAlwaysReturnsMethodNotFound)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object =
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

TEST(CosemIso8802LlcType2SetupObject, VersionAboveMaxNormalized)
{
  dlms::cosem::CosemIso8802LlcType2SetupObject object(
    SampleName(), 1u, 1u, 128u, 8u, 5u, 5u, 5u, 10u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    /*version*/ 99u);
  EXPECT_EQ(
    dlms::cosem::CosemIso8802LlcType2SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}
