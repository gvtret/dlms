// Tests for IC 25 (M-Bus slave port setup) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.8.3 / DLMS UA Blue Book Ed. 12.1 §4.8.2.
//
// Typed attributes (since 0.141.0):
//   2 default_baud : enum 0..7 (Baud)          — A-XDR enum tag 0x16
//   3 avail_baud   : enum 0..7 (Baud)          — A-XDR enum tag 0x16
//   4 addr_state   : enum 0..1 (AddrState)     — A-XDR enum tag 0x16
//   5 bus_address  : unsigned                  — A-XDR tag 0x11
//
// Class version 0. No specific methods defined.

#include <cstdint>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::AttributeAccessMode;
using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::CosemMBusSlavePortSetupObject;
using dlms::cosem::CosemStatus;

CosemLogicalName MakeName()
{
  return CosemLogicalName(0u, 0u, 24u, 0u, 0u, 255u);
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

CosemMBusSlavePortSetupObject MakeWritable(
  CosemMBusSlavePortSetupObject::Baud defaultBaud =
    CosemMBusSlavePortSetupObject::Baud::Baud9600,
  CosemMBusSlavePortSetupObject::Baud availBaud =
    CosemMBusSlavePortSetupObject::Baud::Baud38400,
  CosemMBusSlavePortSetupObject::AddrState addrState =
    CosemMBusSlavePortSetupObject::AddrState::Assigned,
  std::uint8_t busAddress = 0x42u)
{
  return CosemMBusSlavePortSetupObject(
    MakeName(), defaultBaud, availBaud, addrState, busAddress,
    AttributeAccessMode::ReadAndWrite);
}

} // namespace

TEST(CosemMBusSlavePortSetupObject, DescriptorAndDefaultVersion)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  EXPECT_EQ(25u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    CosemMBusSlavePortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemMBusSlavePortSetupObject, NormalizesVersionAboveMax)
{
  CosemMBusSlavePortSetupObject object(
    MakeName(),
    CosemMBusSlavePortSetupObject::Baud::Baud9600,
    CosemMBusSlavePortSetupObject::Baud::Baud38400,
    CosemMBusSlavePortSetupObject::AddrState::Assigned,
    0x42u,
    AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    CosemMBusSlavePortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemMBusSlavePortSetupObject, AccessRightsLayout)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  const auto rights = object.AccessRights();
  EXPECT_EQ(AttributeAccessMode::ReadOnly, rights.AttributeAccess(1u));
  for (std::uint8_t id = 2u; id <= 5u; ++id) {
    EXPECT_EQ(AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "attr id " << static_cast<unsigned>(id);
  }
}

TEST(CosemMBusSlavePortSetupObject, ReadAttributesRoundTrip)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x05u}), out);
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud9600,
            object.GetDefaultBaud());

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x07u}), out);
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud38400,
            object.GetAvailBaud());

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x01u}), out);
  EXPECT_EQ(CosemMBusSlavePortSetupObject::AddrState::Assigned,
            object.GetAddrState());

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(BytesFromList({0x11u, 0x42u}), out);
  EXPECT_EQ(0x42u, object.BusAddress());

  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemMBusSlavePortSetupObject, ConstructorClampsOutOfRangeEnums)
{
  CosemMBusSlavePortSetupObject object(
    MakeName(),
    static_cast<CosemMBusSlavePortSetupObject::Baud>(0x10u),
    static_cast<CosemMBusSlavePortSetupObject::Baud>(0xFFu),
    static_cast<CosemMBusSlavePortSetupObject::AddrState>(0x10u),
    0u,
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud300,
            object.GetDefaultBaud());
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud300,
            object.GetAvailBaud());
  EXPECT_EQ(CosemMBusSlavePortSetupObject::AddrState::NotAssigned,
            object.GetAddrState());
}

TEST(CosemMBusSlavePortSetupObject, WriteDefaultBaudHappyPath)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  for (std::uint8_t raw = 0u; raw <= 7u; ++raw) {
    EXPECT_EQ(CosemStatus::Ok,
              object.WriteAttribute(2u, BytesFromList({0x16u, raw})))
      << "raw " << static_cast<unsigned>(raw);
    EXPECT_EQ(static_cast<CosemMBusSlavePortSetupObject::Baud>(raw),
              object.GetDefaultBaud());
  }
}

TEST(CosemMBusSlavePortSetupObject, WriteAvailBaudHappyPath)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(3u, BytesFromList({0x16u, 0x00u})));
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud300,
            object.GetAvailBaud());
}

TEST(CosemMBusSlavePortSetupObject, WriteAddrStateHappyPath)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0x00u})));
  EXPECT_EQ(CosemMBusSlavePortSetupObject::AddrState::NotAssigned,
            object.GetAddrState());
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0x01u})));
  EXPECT_EQ(CosemMBusSlavePortSetupObject::AddrState::Assigned,
            object.GetAddrState());
}

TEST(CosemMBusSlavePortSetupObject, WriteBusAddressHappyPath)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(5u, BytesFromList({0x11u, 0x00u})));
  EXPECT_EQ(0u, object.BusAddress());
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(5u, BytesFromList({0x11u, 0xFFu})));
  EXPECT_EQ(0xFFu, object.BusAddress());
}

TEST(CosemMBusSlavePortSetupObject, WriteBaudRejectsOutOfRange)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  for (std::uint8_t id : {2u, 3u}) {
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x16u, 0x08u})))
      << "attr id " << static_cast<unsigned>(id);
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x16u, 0xFFu})))
      << "attr id " << static_cast<unsigned>(id);
  }
  // Original values preserved.
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud9600,
            object.GetDefaultBaud());
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud38400,
            object.GetAvailBaud());
}

TEST(CosemMBusSlavePortSetupObject, WriteAddrStateRejectsOutOfRange)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0x02u})));
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0xFFu})));
  EXPECT_EQ(CosemMBusSlavePortSetupObject::AddrState::Assigned,
            object.GetAddrState());
}

TEST(CosemMBusSlavePortSetupObject, WriteRejectsBadPayloads)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  // Enum attributes: wrong tag (unsigned 0x11) / truncated / trailing /
  // empty.
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x11u, 0x00u})))
      << "attr id " << static_cast<unsigned>(id);
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x16u})))
      << "attr id " << static_cast<unsigned>(id);
    EXPECT_EQ(
      CosemStatus::InvalidArgument,
      object.WriteAttribute(
        id, BytesFromList({0x16u, 0x00u, 0x00u})))
      << "attr id " << static_cast<unsigned>(id);
    EXPECT_EQ(CosemStatus::InvalidArgument,
              object.WriteAttribute(id, CosemByteBuffer{}))
      << "attr id " << static_cast<unsigned>(id);
  }
  // bus_address: wrong tag (enum 0x16) / truncated / trailing / empty.
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(5u, BytesFromList({0x16u, 0x42u})));
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(5u, BytesFromList({0x11u})));
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(
              5u, BytesFromList({0x11u, 0x42u, 0x00u})));
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(5u, CosemByteBuffer{}));
}

TEST(CosemMBusSlavePortSetupObject, WriteLogicalNameIsAccessDenied)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::AccessDenied,
            object.WriteAttribute(1u, BytesFromList({0x16u, 0x00u})));
}

TEST(CosemMBusSlavePortSetupObject, WriteUnknownAttrIsNotFound)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, BytesFromList({0x16u, 0x00u})));
  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.WriteAttribute(0u, BytesFromList({0x16u, 0x00u})));
}

TEST(CosemMBusSlavePortSetupObject, ReadOnlyRejectsWrites)
{
  CosemMBusSlavePortSetupObject readOnly(
    MakeName(),
    CosemMBusSlavePortSetupObject::Baud::Baud9600,
    CosemMBusSlavePortSetupObject::Baud::Baud38400,
    CosemMBusSlavePortSetupObject::AddrState::Assigned,
    0x42u,
    AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 5u; ++id) {
    EXPECT_EQ(CosemStatus::AccessDenied,
              readOnly.WriteAttribute(id, BytesFromList({0x16u, 0x00u})))
      << "attr id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(CosemMBusSlavePortSetupObject::Baud::Baud9600,
            readOnly.GetDefaultBaud());
  EXPECT_EQ(0x42u, readOnly.BusAddress());
}

TEST(CosemMBusSlavePortSetupObject, InvokeMethodAlwaysMethodNotFound)
{
  CosemMBusSlavePortSetupObject object = MakeWritable();
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

TEST(CosemMBusSlavePortSetupObject, IsValidHelpers)
{
  for (unsigned raw = 0u; raw <= 7u; ++raw) {
    EXPECT_TRUE(CosemMBusSlavePortSetupObject::IsValidBaud(
      static_cast<std::uint8_t>(raw)));
  }
  EXPECT_FALSE(CosemMBusSlavePortSetupObject::IsValidBaud(8u));
  EXPECT_FALSE(CosemMBusSlavePortSetupObject::IsValidBaud(255u));

  EXPECT_TRUE(CosemMBusSlavePortSetupObject::IsValidAddrState(0u));
  EXPECT_TRUE(CosemMBusSlavePortSetupObject::IsValidAddrState(1u));
  EXPECT_FALSE(CosemMBusSlavePortSetupObject::IsValidAddrState(2u));
  EXPECT_FALSE(CosemMBusSlavePortSetupObject::IsValidAddrState(255u));
}
