#include "dlms/client/client_data.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

std::vector<std::uint8_t> Bytes(
  std::uint8_t a,
  std::uint8_t b)
{
  std::vector<std::uint8_t> output;
  output.push_back(a);
  output.push_back(b);
  return output;
}

std::vector<std::uint8_t> Bytes3(
  std::uint8_t a,
  std::uint8_t b,
  std::uint8_t c)
{
  std::vector<std::uint8_t> output;
  output.push_back(a);
  output.push_back(b);
  output.push_back(c);
  return output;
}

} // namespace

TEST(ClientDataHelpers, EncodeAndDecodeBoolean)
{
  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsBoolean(true, encoded));
  EXPECT_EQ(Bytes(0x03u, 0x01u), encoded);

  bool value = false;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsBoolean(encoded, value));
  EXPECT_TRUE(value);
}

TEST(ClientDataHelpers, EncodeAndDecodeSignedScalars)
{
  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsInteger(-2, encoded));
  EXPECT_EQ(Bytes(0x0Fu, 0xFEu), encoded);

  std::int8_t integerValue = 0;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsInteger(encoded, integerValue));
  EXPECT_EQ(-2, integerValue);

  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsLong(-128, encoded));
  EXPECT_EQ(Bytes3(0x10u, 0xFFu, 0x80u), encoded);

  std::int16_t longValue = 0;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsLong(encoded, longValue));
  EXPECT_EQ(-128, longValue);

  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsDoubleLong(-256, encoded));
  const std::uint8_t expected[] = {0x05u, 0xFFu, 0xFFu, 0xFFu, 0x00u};
  EXPECT_EQ(
    std::vector<std::uint8_t>(expected, expected + sizeof(expected)),
    encoded);

  std::int32_t doubleLongValue = 0;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsDoubleLong(encoded, doubleLongValue));
  EXPECT_EQ(-256, doubleLongValue);
}

TEST(ClientDataHelpers, EncodeAndDecodeUnsignedScalars)
{
  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsUnsigned(0x7Fu, encoded));
  EXPECT_EQ(Bytes(0x11u, 0x7Fu), encoded);

  std::uint8_t unsignedValue = 0u;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsUnsigned(encoded, unsignedValue));
  EXPECT_EQ(0x7Fu, unsignedValue);

  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsLongUnsigned(0x1234u, encoded));
  EXPECT_EQ(Bytes3(0x12u, 0x12u, 0x34u), encoded);

  std::uint16_t longUnsignedValue = 0u;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsLongUnsigned(encoded, longUnsignedValue));
  EXPECT_EQ(0x1234u, longUnsignedValue);

  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsDoubleLongUnsigned(0x12345678u, encoded));
  const std::uint8_t expected[] = {0x06u, 0x12u, 0x34u, 0x56u, 0x78u};
  EXPECT_EQ(
    std::vector<std::uint8_t>(expected, expected + sizeof(expected)),
    encoded);

  std::uint32_t doubleLongUnsignedValue = 0u;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsDoubleLongUnsigned(
      encoded,
      doubleLongUnsignedValue));
  EXPECT_EQ(0x12345678u, doubleLongUnsignedValue);
}

TEST(ClientDataHelpers, EncodeAndDecodeEnumAndOctetString)
{
  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsEnum(5u, encoded));
  EXPECT_EQ(Bytes(0x16u, 0x05u), encoded);

  std::uint8_t enumValue = 0u;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsEnum(encoded, enumValue));
  EXPECT_EQ(5u, enumValue);

  const std::vector<std::uint8_t> octets = Bytes3(0x01u, 0x02u, 0x03u);
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsOctetString(octets, encoded));
  const std::uint8_t expected[] = {0x09u, 0x03u, 0x01u, 0x02u, 0x03u};
  EXPECT_EQ(
    std::vector<std::uint8_t>(expected, expected + sizeof(expected)),
    encoded);

  std::vector<std::uint8_t> decoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsOctetString(encoded, decoded));
  EXPECT_EQ(octets, decoded);
}

TEST(ClientDataHelpers, DecodeRejectsWrongTypeAndClearsOutput)
{
  bool booleanValue = true;
  EXPECT_EQ(
    dlms::client::ClientStatus::InvalidArgument,
    dlms::client::DecodeDlmsBoolean(Bytes(0x11u, 0x01u), booleanValue));
  EXPECT_FALSE(booleanValue);

  std::vector<std::uint8_t> octets;
  octets.push_back(0xAAu);
  EXPECT_EQ(
    dlms::client::ClientStatus::InvalidArgument,
    dlms::client::DecodeDlmsOctetString(Bytes(0x13u, 0x00u), octets));
  EXPECT_TRUE(octets.empty());
}
