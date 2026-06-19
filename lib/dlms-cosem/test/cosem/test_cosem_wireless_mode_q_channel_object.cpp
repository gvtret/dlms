// Tests for IC 73 (Wireless Mode Q channel) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.8.4 / DLMS UA Blue Book Ed. 12.1 §4.8.3.
//
// Typed attributes (since 0.139.0):
//   2 addr_state      : enum 0..1 (AddrState)  — A-XDR enum tag 0x16
//   3 device_address  : octet-string           — A-XDR tag 0x09
//   4 address_mask    : octet-string           — A-XDR tag 0x09
//
// Class version 1. No specific methods defined.

#include <cstdint>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::AttributeAccessMode;
using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::CosemStatus;
using dlms::cosem::CosemWirelessModeQChannelObject;

CosemLogicalName MakeName()
{
  // 0-0:24.1.0.255 — example LN used by the legacy block.
  return CosemLogicalName(0u, 0u, 24u, 1u, 0u, 255u);
}

CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  CosemByteBuffer out;
  out.reserve(bytes.size());
  for (std::uint8_t b : bytes) out.push_back(b);
  return out;
}

CosemByteBuffer EncodedLogicalName(const CosemLogicalName& name)
{
  CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(0x06u);
  for (std::size_t i = 0; i < name.Size(); ++i) {
    out.push_back(name[i]);
  }
  return out;
}

CosemByteBuffer EncodedOctetString(
  std::initializer_list<std::uint8_t> data)
{
  CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(static_cast<std::uint8_t>(data.size()));
  for (std::uint8_t b : data) out.push_back(b);
  return out;
}

CosemWirelessModeQChannelObject MakeWritable(
  CosemWirelessModeQChannelObject::AddrState state =
    CosemWirelessModeQChannelObject::AddrState::Assigned,
  std::vector<std::uint8_t> deviceAddress = {0x12u, 0x34u, 0x56u, 0x78u},
  std::vector<std::uint8_t> addressMask = {0xFFu, 0xFFu, 0x00u, 0x00u})
{
  return CosemWirelessModeQChannelObject(
    MakeName(), state,
    std::move(deviceAddress), std::move(addressMask),
    AttributeAccessMode::ReadAndWrite);
}

} // namespace

TEST(CosemWirelessModeQChannelObject, DescriptorAndDefaultVersion)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  EXPECT_EQ(73u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(
    CosemWirelessModeQChannelObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemWirelessModeQChannelObject, NormalizesVersionAboveMax)
{
  CosemWirelessModeQChannelObject object(
    MakeName(),
    CosemWirelessModeQChannelObject::AddrState::Assigned,
    {0x12u, 0x34u}, {0xFFu, 0xFFu},
    AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    CosemWirelessModeQChannelObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemWirelessModeQChannelObject, AccessRightsLayout)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  const auto rights = object.AccessRights();
  EXPECT_EQ(AttributeAccessMode::ReadOnly, rights.AttributeAccess(1u));
  for (std::uint8_t id = 2u; id <= 4u; ++id) {
    EXPECT_EQ(AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "attr id " << static_cast<unsigned>(id);
  }
}

TEST(CosemWirelessModeQChannelObject, ReadAttributesRoundTrip)
{
  CosemWirelessModeQChannelObject object = MakeWritable();

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x01u}), out);
  EXPECT_EQ(CosemWirelessModeQChannelObject::AddrState::Assigned,
            object.GetAddrState());

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(EncodedOctetString({0x12u, 0x34u, 0x56u, 0x78u}), out);
  EXPECT_EQ((std::vector<std::uint8_t>{0x12u, 0x34u, 0x56u, 0x78u}),
            object.DeviceAddress());

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(EncodedOctetString({0xFFu, 0xFFu, 0x00u, 0x00u}), out);
  EXPECT_EQ((std::vector<std::uint8_t>{0xFFu, 0xFFu, 0x00u, 0x00u}),
            object.AddressMask());

  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemWirelessModeQChannelObject, ReadEmptyOctetStrings)
{
  CosemWirelessModeQChannelObject object = MakeWritable(
    CosemWirelessModeQChannelObject::AddrState::NotAssigned,
    {}, {});
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x09u, 0x00u}), out);
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x09u, 0x00u}), out);
}

TEST(CosemWirelessModeQChannelObject, ConstructorClampsOutOfRangeAddrState)
{
  CosemWirelessModeQChannelObject object(
    MakeName(),
    static_cast<CosemWirelessModeQChannelObject::AddrState>(0x10u),
    {0x01u}, {0x02u},
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(CosemWirelessModeQChannelObject::AddrState::NotAssigned,
            object.GetAddrState());
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x00u}), out);
}

TEST(CosemWirelessModeQChannelObject, WriteAddrStateHappyPath)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0x00u})));
  EXPECT_EQ(CosemWirelessModeQChannelObject::AddrState::NotAssigned,
            object.GetAddrState());
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0x01u})));
  EXPECT_EQ(CosemWirelessModeQChannelObject::AddrState::Assigned,
            object.GetAddrState());
}

TEST(CosemWirelessModeQChannelObject, WriteAddrStateRejectsOutOfRange)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0x02u})));
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0xFFu})));
  EXPECT_EQ(CosemWirelessModeQChannelObject::AddrState::Assigned,
            object.GetAddrState());
}

TEST(CosemWirelessModeQChannelObject, WriteAddrStateRejectsBadPayloads)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  // wrong tag (unsigned 0x11)
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x11u, 0x00u})));
  // truncated
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u})));
  // trailing bytes
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x16u, 0x01u, 0x00u})));
  // empty input
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, CosemByteBuffer{}));
}

TEST(CosemWirelessModeQChannelObject, WriteDeviceAddressHappyPath)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(
              3u, EncodedOctetString({0xAAu, 0xBBu, 0xCCu, 0xDDu})));
  EXPECT_EQ((std::vector<std::uint8_t>{0xAAu, 0xBBu, 0xCCu, 0xDDu}),
            object.DeviceAddress());
  // empty octet-string is valid
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(3u, BytesFromList({0x09u, 0x00u})));
  EXPECT_TRUE(object.DeviceAddress().empty());
}

TEST(CosemWirelessModeQChannelObject, WriteAddressMaskHappyPath)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(
              4u, EncodedOctetString({0x00u, 0x00u, 0xFFu, 0xFFu})));
  EXPECT_EQ((std::vector<std::uint8_t>{0x00u, 0x00u, 0xFFu, 0xFFu}),
            object.AddressMask());
}

TEST(CosemWirelessModeQChannelObject, WriteOctetStringRejectsBadPayloads)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  for (std::uint8_t id : {3u, 4u}) {
    // wrong tag (enum)
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x16u, 0x01u})))
      << "attr id " << static_cast<unsigned>(id);
    // length says 4 but only 2 bytes follow
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(
                id, BytesFromList({0x09u, 0x04u, 0xAAu, 0xBBu})))
      << "attr id " << static_cast<unsigned>(id);
    // trailing bytes after declared length
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(
                id,
                BytesFromList({0x09u, 0x02u, 0xAAu, 0xBBu, 0xFFu})))
      << "attr id " << static_cast<unsigned>(id);
    // empty input
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(id, CosemByteBuffer{}))
      << "attr id " << static_cast<unsigned>(id);
  }
}

TEST(CosemWirelessModeQChannelObject, WriteLogicalNameIsAccessDenied)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::AccessDenied,
            object.WriteAttribute(1u, BytesFromList({0x16u, 0x01u})));
}

TEST(CosemWirelessModeQChannelObject, WriteUnknownAttrIsNotFound)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, BytesFromList({0x16u, 0x01u})));
}

TEST(CosemWirelessModeQChannelObject, ReadOnlyRejectsWrites)
{
  CosemWirelessModeQChannelObject readOnly(
    MakeName(),
    CosemWirelessModeQChannelObject::AddrState::Assigned,
    {0x12u}, {0xFFu},
    AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 4u; ++id) {
    EXPECT_EQ(CosemStatus::AccessDenied,
              readOnly.WriteAttribute(id, BytesFromList({0x16u, 0x00u})))
      << "attr id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(CosemWirelessModeQChannelObject::AddrState::Assigned,
            readOnly.GetAddrState());
}

TEST(CosemWirelessModeQChannelObject, InvokeMethodAlwaysMethodNotFound)
{
  CosemWirelessModeQChannelObject object = MakeWritable();
  const CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (unsigned method : {0u, 1u, 2u, 99u, 255u}) {
    CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << method;
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemWirelessModeQChannelObject, IsValidAddrState)
{
  EXPECT_TRUE(CosemWirelessModeQChannelObject::IsValidAddrState(0u));
  EXPECT_TRUE(CosemWirelessModeQChannelObject::IsValidAddrState(1u));
  EXPECT_FALSE(CosemWirelessModeQChannelObject::IsValidAddrState(2u));
  EXPECT_FALSE(CosemWirelessModeQChannelObject::IsValidAddrState(255u));
}
