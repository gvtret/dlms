// Tests for IC 52 (S-FSK MAC Synchronization Timeouts) — per-IC file
// (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.10.5 / DLMS UA Blue Book Ed. 12.1
// §4.10.5. class_id 52, version 0, five attributes, no methods.
//
// All four dynamic timer attributes are long-unsigned (uint16):
//   2 search_initiator_timeout              (seconds)
//   3 synchronization_confirmation_timeout  (seconds)
//   4 time_out_not_addressed                (minutes)
//   5 time_out_frame_not_OK                 (minutes)
//
// Per the referenced MIB variables there is no specified upper bound;
// the full uint16 range is accepted on write.

#include <cstdint>
#include <initializer_list>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using Object = dlms::cosem::CosemSFskMacSyncTimeoutsObject;

dlms::cosem::CosemLogicalName MakeName()
{
  // 0-0-26-2-0-255 — canonical S-FSK MAC sync timeouts OBIS code.
  return dlms::cosem::CosemLogicalName(0u, 0u, 26u, 2u, 0u, 255u);
}

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  dlms::cosem::CosemByteBuffer out;
  out.reserve(bytes.size());
  for (std::uint8_t b : bytes) out.push_back(b);
  return out;
}

dlms::cosem::CosemByteBuffer EncodedLU(std::uint16_t value)
{
  return BytesFromList({0x12u,
                        static_cast<std::uint8_t>((value >> 8) & 0xFFu),
                        static_cast<std::uint8_t>(value & 0xFFu)});
}

dlms::cosem::CosemByteBuffer EncodedLogicalName(
  const dlms::cosem::CosemLogicalName& name)
{
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(0x06u);
  for (std::size_t i = 0u; i < name.Size(); ++i) out.push_back(name[i]);
  return out;
}

} // namespace

TEST(CosemSFskMacSyncTimeoutsObject, DescriptorAndAccessRights)
{
  Object obj(MakeName(), 5u, 10u, 15u, 20u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(52u, obj.Descriptor().key.classId);
  EXPECT_EQ(0u, obj.Descriptor().key.version);
  EXPECT_EQ(Object::MaxSupportedVersion, obj.Descriptor().key.version);

  const dlms::cosem::CosemAccessRights rights = obj.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(static_cast<std::uint8_t>(id)))
      << "attribute id " << static_cast<unsigned>(id);
  }
}

TEST(CosemSFskMacSyncTimeoutsObject, TypedGettersReflectCtor)
{
  Object obj(MakeName(), 100u, 200u, 300u, 65535u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(100u, obj.SearchInitiatorTimeout());
  EXPECT_EQ(200u, obj.SynchronizationConfirmationTimeout());
  EXPECT_EQ(300u, obj.TimeOutNotAddressed());
  EXPECT_EQ(65535u, obj.TimeOutFrameNotOk());
}

TEST(CosemSFskMacSyncTimeoutsObject, ReadAttributeEmitsTypedAxdr)
{
  Object obj(MakeName(), 7u, 250u, 0u, 1024u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(2u, out));
  EXPECT_EQ(EncodedLU(7u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(3u, out));
  EXPECT_EQ(EncodedLU(250u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(4u, out));
  EXPECT_EQ(EncodedLU(0u), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(5u, out));
  EXPECT_EQ(EncodedLU(1024u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(6u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(99u, out));
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteSearchInitiatorTimeoutDecodes)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(2u, EncodedLU(60000u)));
  EXPECT_EQ(60000u, obj.SearchInitiatorTimeout());
  EXPECT_EQ(2u, obj.SynchronizationConfirmationTimeout());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteSyncConfirmationTimeoutDecodes)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(3u, EncodedLU(0u)));
  EXPECT_EQ(0u, obj.SynchronizationConfirmationTimeout());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteTimeOutNotAddressedDecodes)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(4u, EncodedLU(65535u)));
  EXPECT_EQ(65535u, obj.TimeOutNotAddressed());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteTimeOutFrameNotOkDecodes)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(5u, EncodedLU(12345u)));
  EXPECT_EQ(12345u, obj.TimeOutFrameNotOk());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteRejectsWrongTag)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  // unsigned (uint8) instead of long-unsigned
  const dlms::cosem::CosemByteBuffer bad =
    BytesFromList({0x11u, 0x05u});
  for (std::uint8_t id : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              obj.WriteAttribute(static_cast<std::uint8_t>(id), bad))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(1u, obj.SearchInitiatorTimeout());
  EXPECT_EQ(2u, obj.SynchronizationConfirmationTimeout());
  EXPECT_EQ(3u, obj.TimeOutNotAddressed());
  EXPECT_EQ(4u, obj.TimeOutFrameNotOk());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteRejectsTruncated)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  // missing low byte
  const dlms::cosem::CosemByteBuffer bad =
    BytesFromList({0x12u, 0x01u});
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, bad));
  EXPECT_EQ(1u, obj.SearchInitiatorTimeout());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteRejectsTrailingGarbage)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const dlms::cosem::CosemByteBuffer bad =
    BytesFromList({0x12u, 0x00u, 0x10u, 0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(3u, bad));
  EXPECT_EQ(2u, obj.SynchronizationConfirmationTimeout());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteRejectsEmptyInput)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(4u, dlms::cosem::CosemByteBuffer{}));
  EXPECT_EQ(3u, obj.TimeOutNotAddressed());
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteLogicalNameAlwaysDenied)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(1u, EncodedLU(5u)));
}

TEST(CosemSFskMacSyncTimeoutsObject, WriteUnknownAttributeReportsNotFound)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.WriteAttribute(99u, EncodedLU(5u)));
}

TEST(CosemSFskMacSyncTimeoutsObject, ReadOnlyRejectsAllTimerWrites)
{
  Object obj(MakeName(), 11u, 22u, 33u, 44u,
             dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              obj.WriteAttribute(static_cast<std::uint8_t>(id),
                                 EncodedLU(99u)))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(11u, obj.SearchInitiatorTimeout());
  EXPECT_EQ(22u, obj.SynchronizationConfirmationTimeout());
  EXPECT_EQ(33u, obj.TimeOutNotAddressed());
  EXPECT_EQ(44u, obj.TimeOutFrameNotOk());
}

TEST(CosemSFskMacSyncTimeoutsObject, InvokeMethodAlwaysReturnsMethodNotFound)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 3u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              obj.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSFskMacSyncTimeoutsObject, VersionAboveMaxNormalized)
{
  Object obj(MakeName(), 1u, 2u, 3u, 4u,
             dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(Object::MaxSupportedVersion, obj.Descriptor().key.version);
}
