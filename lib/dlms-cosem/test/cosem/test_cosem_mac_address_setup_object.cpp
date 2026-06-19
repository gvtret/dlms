// Tests for IC 43 (MAC Address Setup) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.4.5 / DLMS UA Blue Book Ed. 12.1.
//
// One dynamic attribute, no methods:
//   2 mac_address : octet-string of length 6
//
// `mac_address` is typed as MacAddressBytes = std::array<uint8_t, 6>.
// Wire encoding on read is always octet-string tag 0x09, length 0x06,
// six MAC bytes. WriteAttribute validates tag/length strictly and
// rejects anything else with InvalidArgument while preserving the
// previously stored MAC.

#include <array>
#include <cstdint>
#include <initializer_list>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using Object = dlms::cosem::CosemMacAddressSetupObject;

dlms::cosem::CosemLogicalName MakeName()
{
  // 0-0-43-0-0-255 — canonical MAC-Address-setup OBIS code per
  // Blue Book annex.
  return dlms::cosem::CosemLogicalName(0u, 0u, 43u, 0u, 0u, 255u);
}

Object::MacAddressBytes SampleMac()
{
  return {0x00u, 0x1Au, 0x2Bu, 0x3Cu, 0x4Du, 0x5Eu};
}

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  dlms::cosem::CosemByteBuffer out;
  out.reserve(bytes.size());
  for (std::uint8_t b : bytes) out.push_back(b);
  return out;
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

dlms::cosem::CosemByteBuffer EncodedMac(
  const Object::MacAddressBytes& mac)
{
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(0x06u);
  for (std::uint8_t b : mac) out.push_back(b);
  return out;
}

} // namespace

TEST(CosemMacAddressSetupObject, DescriptorAndAccessRights)
{
  Object obj(MakeName(), SampleMac(),
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(43u, obj.Descriptor().key.classId);
  EXPECT_EQ(0u, obj.Descriptor().key.version);
  EXPECT_EQ(Object::MaxSupportedVersion, obj.Descriptor().key.version);

  const dlms::cosem::CosemAccessRights rights = obj.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rights.AttributeAccess(2u));
}

TEST(CosemMacAddressSetupObject, ReadAttributeEncodesTypedAxdr)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(2u, out));
  EXPECT_EQ(EncodedMac(mac), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(3u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(99u, out));
}

TEST(CosemMacAddressSetupObject, GetterReturnsTypedMac)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(mac, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, WriteMacAddressDecodesOctetString6)
{
  Object obj(MakeName(), SampleMac(),
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const Object::MacAddressBytes next = {
    0xAAu, 0xBBu, 0xCCu, 0xDDu, 0xEEu, 0xFFu};
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(2u, EncodedMac(next)));
  EXPECT_EQ(next, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, WriteRejectsWrongTag)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  // long-unsigned tag instead of octet-string
  const dlms::cosem::CosemByteBuffer bad =
    BytesFromList({0x12u, 0x06u, 0x00u, 0x1Au, 0x2Bu, 0x3Cu, 0x4Du, 0x5Eu});
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, bad));
  EXPECT_EQ(mac, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, WriteRejectsWrongLength)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  // declared length 5 instead of 6
  const dlms::cosem::CosemByteBuffer bad =
    BytesFromList({0x09u, 0x05u, 0x00u, 0x1Au, 0x2Bu, 0x3Cu, 0x4Du});
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, bad));
  EXPECT_EQ(mac, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, WriteRejectsTruncatedInput)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  // tag+length OK, only 5 bytes follow instead of 6
  const dlms::cosem::CosemByteBuffer bad =
    BytesFromList({0x09u, 0x06u, 0x00u, 0x1Au, 0x2Bu, 0x3Cu, 0x4Du});
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, bad));
  EXPECT_EQ(mac, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, WriteRejectsTrailingGarbage)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const dlms::cosem::CosemByteBuffer bad =
    BytesFromList({0x09u, 0x06u,
                   0x00u, 0x1Au, 0x2Bu, 0x3Cu, 0x4Du, 0x5Eu,
                   0xCCu});
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, bad));
  EXPECT_EQ(mac, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, WriteRejectsEmptyInput)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, dlms::cosem::CosemByteBuffer{}));
  EXPECT_EQ(mac, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, WriteLogicalNameAlwaysDenied)
{
  Object obj(MakeName(), SampleMac(),
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(1u, EncodedMac(SampleMac())));
}

TEST(CosemMacAddressSetupObject, WriteUnknownAttributeReportsNotFound)
{
  Object obj(MakeName(), SampleMac(),
             dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.WriteAttribute(99u, EncodedMac(SampleMac())));
}

TEST(CosemMacAddressSetupObject, ReadOnlyAccessRejectsWrites)
{
  const Object::MacAddressBytes mac = SampleMac();
  Object obj(MakeName(), mac,
             dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(2u, EncodedMac(SampleMac())));
  EXPECT_EQ(mac, obj.MacAddress());
}

TEST(CosemMacAddressSetupObject, InvokeMethodAlwaysReturnsMethodNotFound)
{
  Object obj(MakeName(), SampleMac(),
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

TEST(CosemMacAddressSetupObject, VersionAboveMaxNormalized)
{
  Object obj(MakeName(), SampleMac(),
             dlms::cosem::AttributeAccessMode::ReadAndWrite,
             99u);
  EXPECT_EQ(Object::MaxSupportedVersion, obj.Descriptor().key.version);
}
