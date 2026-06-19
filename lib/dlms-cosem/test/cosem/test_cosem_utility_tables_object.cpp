// Tests for IC 26 (Utility Tables) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.6.5 / DLMS UA Blue Book Ed. 12.1 §4.6.5.
//
// Typed attributes (since 0.135.0):
//   2 table_id  : long-unsigned         (uint16_t)
//   3 length    : double-long-unsigned  (uint32_t)
//   4 buffer    : octet-string          (std::vector<uint8_t>)
//
// Class version 0. No specific methods defined.

#include <cstdint>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 94u, 7u, 0u, 255u);
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

dlms::cosem::CosemUtilityTablesObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemUtilityTablesObject(
    MakeName(),
    /*tableId=*/0x0010u,
    /*length=*/0x00000020u,
    /*buffer=*/std::vector<std::uint8_t>{0xDEu, 0xADu, 0xBEu, 0xEFu},
    access);
}

} // namespace

TEST(CosemUtilityTablesObject, DescriptorAndAccessRights)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(26u, object.Descriptor().key.classId);
  EXPECT_EQ(
    dlms::cosem::CosemUtilityTablesObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(0u, object.Descriptor().key.version);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "attribute id " << static_cast<unsigned>(id);
  }
}

TEST(CosemUtilityTablesObject, TypedGetters)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(0x0010u, object.TableId());
  EXPECT_EQ(0x00000020u, object.Length());
  ASSERT_EQ(4u, object.Buffer().size());
  EXPECT_EQ(0xDEu, object.Buffer()[0]);
  EXPECT_EQ(0xADu, object.Buffer()[1]);
  EXPECT_EQ(0xBEu, object.Buffer()[2]);
  EXPECT_EQ(0xEFu, object.Buffer()[3]);
}

TEST(CosemUtilityTablesObject, ReadAttributeEncodesAllAttributes)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x10u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x20u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x09u, 0x04u, 0xDEu, 0xADu, 0xBEu, 0xEFu}),
            out);
}

TEST(CosemUtilityTablesObject, UnknownAttributeIsNotFound)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu, 0xBBu});
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
  EXPECT_TRUE(out.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, out));
}

TEST(CosemUtilityTablesObject, WriteRoundTripsTableId)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // long-unsigned 0xCAFE
  const dlms::cosem::CosemByteBuffer input =
    BytesFromList({0x12u, 0xCAu, 0xFEu});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, input));
  EXPECT_EQ(0xCAFEu, object.TableId());

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(input, out);
}

TEST(CosemUtilityTablesObject, WriteRoundTripsLength)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // double-long-unsigned 0x12345678
  const dlms::cosem::CosemByteBuffer input =
    BytesFromList({0x06u, 0x12u, 0x34u, 0x56u, 0x78u});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, input));
  EXPECT_EQ(0x12345678u, object.Length());

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(input, out);
}

TEST(CosemUtilityTablesObject, WriteRoundTripsBuffer)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // octet-string(3) = {0x01, 0x02, 0x03}
  const dlms::cosem::CosemByteBuffer input =
    BytesFromList({0x09u, 0x03u, 0x01u, 0x02u, 0x03u});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, input));
  ASSERT_EQ(3u, object.Buffer().size());
  EXPECT_EQ(0x01u, object.Buffer()[0]);
  EXPECT_EQ(0x02u, object.Buffer()[1]);
  EXPECT_EQ(0x03u, object.Buffer()[2]);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(input, out);
}

TEST(CosemUtilityTablesObject, WriteEmptyBufferIsAccepted)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // octet-string(0) — explicitly empty payload (length-byte 0x00).
  const dlms::cosem::CosemByteBuffer input =
    BytesFromList({0x09u, 0x00u});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, input));
  EXPECT_TRUE(object.Buffer().empty());
}

TEST(CosemUtilityTablesObject, WriteRejectsWrongTag)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // unsigned instead of long-unsigned for table_id
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x11u, 0x00u})));
  EXPECT_EQ(0x0010u, object.TableId());

  // long-unsigned instead of double-long-unsigned for length
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, BytesFromList({0x12u, 0x00u, 0x10u})));
  EXPECT_EQ(0x00000020u, object.Length());

  // visible-string instead of octet-string for buffer
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x0Au, 0x01u, 0x41u})));
  ASSERT_EQ(4u, object.Buffer().size());
  EXPECT_EQ(0xDEu, object.Buffer()[0]);
}

TEST(CosemUtilityTablesObject, WriteRejectsTruncation)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // long-unsigned missing the low byte
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u, 0xCAu})));
  // double-long-unsigned with only 3 of 4 payload bytes
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              3u, BytesFromList({0x06u, 0x12u, 0x34u, 0x56u})));
  // octet-string with length 3 but only 2 bytes of payload
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              4u, BytesFromList({0x09u, 0x03u, 0x01u, 0x02u})));
}

TEST(CosemUtilityTablesObject, WriteRejectsTrailingBytes)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x12u, 0x00u, 0x10u, 0xFFu})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              3u, BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x01u, 0xAAu})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              4u, BytesFromList({0x09u, 0x01u, 0x01u, 0x99u})));
}

TEST(CosemUtilityTablesObject, WriteRejectsEmptyInput)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer empty;
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, empty))
      << "attribute id " << static_cast<unsigned>(id);
  }
}

TEST(CosemUtilityTablesObject, ReadOnlyDeniesAllWrites)
{
  dlms::cosem::CosemUtilityTablesObject readOnly =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);

  // Even well-formed payloads are denied before parsing happens.
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, BytesFromList({0x12u, 0xCAu, 0xFEu})));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(
              3u, BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x01u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(4u, BytesFromList({0x09u, 0x00u})));

  EXPECT_EQ(0x0010u, readOnly.TableId());
  EXPECT_EQ(0x00000020u, readOnly.Length());
  ASSERT_EQ(4u, readOnly.Buffer().size());
}

TEST(CosemUtilityTablesObject, LogicalNameWriteAlwaysDenied)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedLogicalName(MakeName())));
}

TEST(CosemUtilityTablesObject, UnknownAttributeWriteIsNotFound)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(5u, BytesFromList({0x12u, 0x00u, 0x01u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, BytesFromList({0x12u, 0x00u, 0x01u})));
}

TEST(CosemUtilityTablesObject, NoMethodsDefined)
{
  dlms::cosem::CosemUtilityTablesObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(method, in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemUtilityTablesObject, NormalizesVersionAboveMax)
{
  dlms::cosem::CosemUtilityTablesObject object(
    MakeName(),
    /*tableId=*/0u,
    /*length=*/0u,
    /*buffer=*/std::vector<std::uint8_t>{},
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    /*version=*/99u);
  EXPECT_EQ(
    dlms::cosem::CosemUtilityTablesObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}
