// Tests for IC 86 (PRIME NB OFDM PLC Application Identification) —
// per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.12.11 / DLMS UA Blue Book Ed. 12.1
// §4.12.11.
//
// Typed attributes (since 0.137.0):
//   2 firmware_version : octet-string         (std::vector<uint8_t>)
//   3 vendor_Id        : long-unsigned        (uint16_t)
//   4 product_Id       : long-unsigned        (uint16_t)
//
// Class version 0. No specific methods defined.

#include <cstdint>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 28u, 6u, 0u, 255u);
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
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x09u);
  bytes.push_back(0x06u);
  for (std::size_t i = 0u; i < name.Size(); ++i) bytes.push_back(name[i]);
  return bytes;
}

dlms::cosem::CosemByteBuffer EncodedOctetString(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(static_cast<std::uint8_t>(bytes.size()));
  for (std::uint8_t b : bytes) out.push_back(b);
  return out;
}

dlms::cosem::CosemByteBuffer EncodedLongUnsigned(std::uint16_t value)
{
  return BytesFromList({
    0x12u,
    static_cast<std::uint8_t>((value >> 8) & 0xFFu),
    static_cast<std::uint8_t>(value & 0xFFu)});
}

// "v1.23"
const std::vector<std::uint8_t> kSampleFirmware =
  {0x76u, 0x31u, 0x2Eu, 0x32u, 0x33u};

dlms::cosem::CosemPrimePlcApplicationIdentificationObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPrimePlcApplicationIdentificationObject(
    MakeName(),
    /*firmwareVersion=*/kSampleFirmware,
    /*vendorId=*/        0x1234u,
    /*productId=*/       0x5678u,
    access);
}

} // namespace

TEST(CosemPrimePlcApplicationIdentificationObject, ExposesAllAttributes)
{
  auto object = MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(86u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcApplicationIdentificationObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(EncodedOctetString(kSampleFirmware), out);
  EXPECT_EQ(kSampleFirmware, object.FirmwareVersion());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, out));
  EXPECT_EQ(EncodedLongUnsigned(0x1234u), out);
  EXPECT_EQ(0x1234u, object.VendorId());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, out));
  EXPECT_EQ(EncodedLongUnsigned(0x5678u), out);
  EXPECT_EQ(0x5678u, object.ProductId());

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemPrimePlcApplicationIdentificationObject,
     MutableAttributesHonorAccessMode)
{
  const std::vector<std::uint8_t> newFirmware =
    {0x76u, 0x32u, 0x2Eu, 0x30u};
  const dlms::cosem::CosemByteBuffer newFirmwareEnc =
    EncodedOctetString(newFirmware);
  const dlms::cosem::CosemByteBuffer newVendorEnc =
    EncodedLongUnsigned(0xABCDu);
  const dlms::cosem::CosemByteBuffer newProductEnc =
    EncodedLongUnsigned(0xEF01u);

  auto writable = MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, newFirmwareEnc));
  EXPECT_EQ(newFirmware, writable.FirmwareVersion());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(3u, newVendorEnc));
  EXPECT_EQ(0xABCDu, writable.VendorId());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(4u, newProductEnc));
  EXPECT_EQ(0xEF01u, writable.ProductId());

  // logical_name is always read-only
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, newFirmwareEnc));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, newFirmwareEnc));

  auto readOnly = MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, newFirmwareEnc));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(3u, newVendorEnc));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(4u, newProductEnc));
  EXPECT_EQ(kSampleFirmware, readOnly.FirmwareVersion());
  EXPECT_EQ(0x1234u, readOnly.VendorId());
  EXPECT_EQ(0x5678u, readOnly.ProductId());
}

TEST(CosemPrimePlcApplicationIdentificationObject,
     WriteRejectsMalformedFirmware)
{
  auto object = MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // Empty input
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, dlms::cosem::CosemByteBuffer{}));
  // Wrong tag (integer)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x0Fu, 0x05u})));
  // Truncated payload: length=5 but only 3 bytes follow
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u,
              BytesFromList({0x09u, 0x05u, 0x76u, 0x31u, 0x2Eu})));
  // Trailing bytes after a valid octet-string
  dlms::cosem::CosemByteBuffer trailing =
    EncodedOctetString(kSampleFirmware);
  trailing.push_back(0xAAu);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, trailing));

  // State unchanged
  EXPECT_EQ(kSampleFirmware, object.FirmwareVersion());
}

TEST(CosemPrimePlcApplicationIdentificationObject,
     WriteRejectsMalformedVendorAndProduct)
{
  auto object = MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  for (std::uint8_t attr : {3u, 4u}) {
    // Empty
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(attr, dlms::cosem::CosemByteBuffer{}));
    // Wrong tag (octet-string instead of long-unsigned)
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(
                attr, BytesFromList({0x09u, 0x02u, 0x00u, 0x01u})));
    // Truncated long-unsigned: only one byte after tag
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(attr, BytesFromList({0x12u, 0x12u})));
    // Trailing byte after a valid long-unsigned
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(
                attr,
                BytesFromList({0x12u, 0x12u, 0x34u, 0xAAu})));
  }

  EXPECT_EQ(0x1234u, object.VendorId());
  EXPECT_EQ(0x5678u, object.ProductId());
}

TEST(CosemPrimePlcApplicationIdentificationObject, NoMethodsDefined)
{
  auto object = MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(method, in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPrimePlcApplicationIdentificationObject,
     NormalizesVersionAboveMax)
{
  dlms::cosem::CosemPrimePlcApplicationIdentificationObject object(
    MakeName(),
    kSampleFirmware,
    /*vendorId=*/0x1234u,
    /*productId=*/0x5678u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    /*version=*/99u);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcApplicationIdentificationObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemPrimePlcApplicationIdentificationObject, AccessRightsAreCorrect)
{
  auto writable = MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  auto rw = writable.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rw.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rw.AttributeAccess(2u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rw.AttributeAccess(3u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rw.AttributeAccess(4u));

  auto readOnly = MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  auto ro = readOnly.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            ro.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            ro.AttributeAccess(2u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            ro.AttributeAccess(3u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            ro.AttributeAccess(4u));
}
