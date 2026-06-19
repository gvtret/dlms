// Tests for IC 46 (SMTP Setup) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.9.6 / DLMS UA Blue Book Ed. 12.1 §4.4.7.
//
// Typed attributes (since 0.137.0):
//   2 server_port    : long-unsigned (uint16_t, IANA SMTP default 25)
//   3 user_name      : octet-string  (std::vector<uint8_t>)
//   4 login_password : octet-string  (std::vector<uint8_t>, empty = no auth)
//   5 server_address : octet-string  (std::vector<uint8_t>)
//   6 sender_address : octet-string  (std::vector<uint8_t>)
//
// Class version 0. No specific methods defined.

#include <cstdint>
#include <string>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 25u, 4u, 0u, 255u);
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

std::vector<std::uint8_t> AsBytes(const std::string& text)
{
  return std::vector<std::uint8_t>(text.begin(), text.end());
}

dlms::cosem::CosemByteBuffer EncodedOctetString(
  const std::vector<std::uint8_t>& value)
{
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(static_cast<std::uint8_t>(value.size()));
  out.insert(out.end(), value.begin(), value.end());
  return out;
}

dlms::cosem::CosemByteBuffer EncodedLongUnsigned(std::uint16_t value)
{
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0x12u);
  out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
  out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
  return out;
}

dlms::cosem::CosemSmtpSetupObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemSmtpSetupObject(
    MakeName(),
    /*server_port*/ 587u,
    /*user_name*/ AsBytes("meter"),
    /*login_password*/ AsBytes("secret"),
    /*server_address*/ AsBytes("smtp.example.com"),
    /*sender_address*/ AsBytes("a@b.c"),
    access);
}

} // namespace

TEST(CosemSmtpSetupObject, DescriptorAndVersion)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(46u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSmtpSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(MakeName(), object.Descriptor().key.logicalName);
}

TEST(CosemSmtpSetupObject, ReadsAllAttributes)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(EncodedLongUnsigned(587u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(EncodedOctetString(AsBytes("meter")), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(EncodedOctetString(AsBytes("secret")), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(EncodedOctetString(AsBytes("smtp.example.com")), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(EncodedOctetString(AsBytes("a@b.c")), out);
}

TEST(CosemSmtpSetupObject, ReadUnknownAttributeYieldsNotFound)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out = BytesFromList({0xFFu});
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemSmtpSetupObject, EmptyOctetStringsRoundTrip)
{
  dlms::cosem::CosemSmtpSetupObject object(
    MakeName(), 25u,
    /*user_name*/ {},
    /*login_password*/ {},
    /*server_address*/ {},
    /*sender_address*/ {},
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  for (std::uint8_t id : {3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(id, out))
      << "attr " << static_cast<unsigned>(id);
    EXPECT_EQ(EncodedOctetString({}), out)
      << "attr " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(EncodedLongUnsigned(25u), out);
}

TEST(CosemSmtpSetupObject, WritesUpdateAllAttributesWhenWritable)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, EncodedLongUnsigned(2525u)));
  EXPECT_EQ(2525u, object.ServerPort());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, EncodedOctetString(AsBytes("alice"))));
  EXPECT_EQ(AsBytes("alice"), object.UserName());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, EncodedOctetString({})));
  EXPECT_TRUE(object.LoginPassword().empty());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(
              5u, EncodedOctetString(AsBytes("10.0.0.1"))));
  EXPECT_EQ(AsBytes("10.0.0.1"), object.ServerAddress());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(
              6u, EncodedOctetString(AsBytes("noreply@x"))));
  EXPECT_EQ(AsBytes("noreply@x"), object.SenderAddress());
}

TEST(CosemSmtpSetupObject, ReadOnlyAccessRejectsWrites)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, EncodedLongUnsigned(1u)));
  EXPECT_EQ(587u, object.ServerPort());
  for (std::uint8_t id : {3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              object.WriteAttribute(id, EncodedOctetString(AsBytes("x"))))
      << "attr " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(AsBytes("meter"), object.UserName());
  EXPECT_EQ(AsBytes("secret"), object.LoginPassword());
  EXPECT_EQ(AsBytes("smtp.example.com"), object.ServerAddress());
  EXPECT_EQ(AsBytes("a@b.c"), object.SenderAddress());
}

TEST(CosemSmtpSetupObject, LogicalNameAlwaysReadOnly)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedLogicalName(MakeName())));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, EncodedOctetString({})));
}

TEST(CosemSmtpSetupObject, RejectsWrongTagOnServerPort)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // octet-string instead of long-unsigned
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, EncodedOctetString(AsBytes("nope"))));
  EXPECT_EQ(587u, object.ServerPort());

  // truncated long-unsigned (missing low byte)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u, 0x01u})));

  // trailing garbage after a valid long-unsigned
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u, 0x00u, 0x01u, 0xAAu})));

  // empty input
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, dlms::cosem::CosemByteBuffer{}));
  EXPECT_EQ(587u, object.ServerPort());
}

TEST(CosemSmtpSetupObject, RejectsMalformedOctetStringWrites)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  for (std::uint8_t id : {3u, 4u, 5u, 6u}) {
    // wrong tag (long-unsigned where octet-string is expected)
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, EncodedLongUnsigned(1u)))
      << "attr " << static_cast<unsigned>(id);
    // length says 5 but only 2 bytes follow
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(
                id, BytesFromList({0x09u, 0x05u, 0x61u, 0x62u})))
      << "attr " << static_cast<unsigned>(id);
    // trailing byte after declared payload
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(
                id, BytesFromList({0x09u, 0x01u, 0x61u, 0xAAu})))
      << "attr " << static_cast<unsigned>(id);
    // empty input
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, dlms::cosem::CosemByteBuffer{}))
      << "attr " << static_cast<unsigned>(id);
  }

  // unchanged after rejected writes
  EXPECT_EQ(AsBytes("meter"), object.UserName());
  EXPECT_EQ(AsBytes("secret"), object.LoginPassword());
  EXPECT_EQ(AsBytes("smtp.example.com"), object.ServerAddress());
  EXPECT_EQ(AsBytes("a@b.c"), object.SenderAddress());
}

TEST(CosemSmtpSetupObject, NoSpecificMethodsDefined)
{
  dlms::cosem::CosemSmtpSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 3u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(method, in, out))
      << "method " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSmtpSetupObject, NormalizesVersionAboveMax)
{
  dlms::cosem::CosemSmtpSetupObject object(
    MakeName(), 25u, {}, {}, {}, {},
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSmtpSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemSmtpSetupObject, AccessRightsHonorMutableFlag)
{
  dlms::cosem::CosemSmtpSetupObject writable =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const dlms::cosem::CosemAccessRights rights = writable.AccessRights();

  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "attr " << static_cast<unsigned>(id);
  }

  dlms::cosem::CosemSmtpSetupObject readOnly =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  const dlms::cosem::CosemAccessRights ro = readOnly.AccessRights();
  for (std::uint8_t id : {1u, 2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
              ro.AttributeAccess(id))
      << "attr " << static_cast<unsigned>(id);
  }
}
