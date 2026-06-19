// Tests for IC 74 (M-Bus Master Port Setup) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.8.5 / DLMS UA Blue Book Ed. 12.1 §4.8.4.
//
// Typed attributes (since 0.138.0):
//   2 comm_speed : enum 0..7 (CommSpeed) — A-XDR enum tag 0x16
//
// Class version 0. No specific methods defined.

#include <cstdint>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::AttributeAccessMode;
using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::CosemMBusMasterPortSetupObject;
using dlms::cosem::CosemStatus;

CosemLogicalName MakeName()
{
  // 0-0:24.6.0.255 — example LN used by the legacy block.
  return CosemLogicalName(0u, 0u, 24u, 6u, 0u, 255u);
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
  out.push_back(0x09u); // octet-string
  out.push_back(0x06u);
  for (std::size_t i = 0; i < name.Size(); ++i) {
    out.push_back(name[i]);
  }
  return out;
}

CosemMBusMasterPortSetupObject MakeWritable(
  CosemMBusMasterPortSetupObject::CommSpeed speed =
    CosemMBusMasterPortSetupObject::CommSpeed::Baud2400)
{
  return CosemMBusMasterPortSetupObject(
    MakeName(), speed, AttributeAccessMode::ReadAndWrite);
}

} // namespace

TEST(CosemMBusMasterPortSetupObject, DescriptorAndDefaultVersion)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  EXPECT_EQ(74u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    CosemMBusMasterPortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemMBusMasterPortSetupObject, NormalizesVersionAboveMax)
{
  CosemMBusMasterPortSetupObject object(
    MakeName(),
    CosemMBusMasterPortSetupObject::CommSpeed::Baud9600,
    AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    CosemMBusMasterPortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemMBusMasterPortSetupObject, AccessRightsLayout)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  const auto rights = object.AccessRights();
  EXPECT_EQ(AttributeAccessMode::ReadOnly, rights.AttributeAccess(1u));
  EXPECT_EQ(AttributeAccessMode::ReadAndWrite,
            rights.AttributeAccess(2u));
}

TEST(CosemMBusMasterPortSetupObject, ReadAttributesRoundTrip)
{
  CosemMBusMasterPortSetupObject object = MakeWritable(
    CosemMBusMasterPortSetupObject::CommSpeed::Baud9600);

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  // comm_speed: enum tag 0x16 + raw byte 5 (Baud9600)
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x05u}), out);

  EXPECT_EQ(CosemMBusMasterPortSetupObject::CommSpeed::Baud9600,
            object.GetCommSpeed());

  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemMBusMasterPortSetupObject, ConstructorClampsOutOfRangeSpeed)
{
  // Raw 0x10 (out of 0..7) collapses to default Baud2400 (raw 3).
  CosemMBusMasterPortSetupObject object(
    MakeName(),
    static_cast<CosemMBusMasterPortSetupObject::CommSpeed>(0x10u),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(CosemMBusMasterPortSetupObject::CommSpeed::Baud2400,
            object.GetCommSpeed());
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x03u}), out);
}

TEST(CosemMBusMasterPortSetupObject, WriteCommSpeedHappyPath)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::Ok,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0x06u})));
  EXPECT_EQ(CosemMBusMasterPortSetupObject::CommSpeed::Baud19200,
            object.GetCommSpeed());

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x06u}), out);
}

TEST(CosemMBusMasterPortSetupObject, WriteCommSpeedAllValidValues)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  for (std::uint8_t raw = 0u; raw <= 7u; ++raw) {
    EXPECT_EQ(CosemStatus::Ok,
              object.WriteAttribute(2u, BytesFromList({0x16u, raw})))
      << "raw=" << static_cast<unsigned>(raw);
    EXPECT_EQ(raw, static_cast<std::uint8_t>(object.GetCommSpeed()));
  }
}

TEST(CosemMBusMasterPortSetupObject, WriteCommSpeedRejectsOutOfRange)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0x08u})));
  EXPECT_EQ(CosemMBusMasterPortSetupObject::CommSpeed::Baud2400,
            object.GetCommSpeed());

  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0xFFu})));
  EXPECT_EQ(CosemMBusMasterPortSetupObject::CommSpeed::Baud2400,
            object.GetCommSpeed());
}

TEST(CosemMBusMasterPortSetupObject, WriteCommSpeedRejectsWrongTag)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  // unsigned (0x11) instead of enum (0x16)
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x11u, 0x05u})));
  EXPECT_EQ(CosemMBusMasterPortSetupObject::CommSpeed::Baud2400,
            object.GetCommSpeed());
}

TEST(CosemMBusMasterPortSetupObject, WriteCommSpeedRejectsTruncated)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  // Only the tag, missing value byte
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u})));
}

TEST(CosemMBusMasterPortSetupObject, WriteCommSpeedRejectsTrailingBytes)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x16u, 0x05u, 0x00u})));
}

TEST(CosemMBusMasterPortSetupObject, WriteCommSpeedRejectsEmptyInput)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, CosemByteBuffer{}));
}

TEST(CosemMBusMasterPortSetupObject, WriteLogicalNameIsAccessDenied)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::AccessDenied,
            object.WriteAttribute(1u, BytesFromList({0x16u, 0x05u})));
}

TEST(CosemMBusMasterPortSetupObject, WriteUnknownAttrIsNotFound)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, BytesFromList({0x16u, 0x05u})));
}

TEST(CosemMBusMasterPortSetupObject, ReadOnlyRejectsCommSpeedWrites)
{
  CosemMBusMasterPortSetupObject readOnly(
    MakeName(),
    CosemMBusMasterPortSetupObject::CommSpeed::Baud2400,
    AttributeAccessMode::ReadOnly);
  EXPECT_EQ(CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, BytesFromList({0x16u, 0x05u})));
  EXPECT_EQ(CosemMBusMasterPortSetupObject::CommSpeed::Baud2400,
            readOnly.GetCommSpeed());
}

TEST(CosemMBusMasterPortSetupObject, InvokeMethodAlwaysMethodNotFound)
{
  CosemMBusMasterPortSetupObject object = MakeWritable();
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

TEST(CosemMBusMasterPortSetupObject, IsValidCommSpeed)
{
  for (std::uint8_t raw = 0u; raw <= 7u; ++raw) {
    EXPECT_TRUE(CosemMBusMasterPortSetupObject::IsValidCommSpeed(raw))
      << "raw=" << static_cast<unsigned>(raw);
  }
  EXPECT_FALSE(CosemMBusMasterPortSetupObject::IsValidCommSpeed(8u));
  EXPECT_FALSE(CosemMBusMasterPortSetupObject::IsValidCommSpeed(255u));
}
