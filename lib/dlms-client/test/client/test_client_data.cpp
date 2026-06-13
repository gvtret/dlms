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

TEST(ClientDataHelpers, EncodeAndDecodeDateTime)
{
  dlms::client::DlmsDateTime value;
  value.date.year = 2024u;
  value.date.month = 6u;
  value.date.dayOfMonth = 13u;
  value.date.dayOfWeek = 5u;
  value.time.hour = 16u;
  value.time.minute = 32u;
  value.time.second = 48u;
  value.time.hundredths = 0u;
  value.deviation = -180;
  value.clockStatus = 0x80u;

  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsDateTime(value, encoded));
  const std::uint8_t expected[] = {
    0x19u,
    0x07u,
    0xE8u,
    0x06u,
    0x0Du,
    0x05u,
    0x10u,
    0x20u,
    0x30u,
    0x00u,
    0xFFu,
    0x4Cu,
    0x80u};
  EXPECT_EQ(
    std::vector<std::uint8_t>(expected, expected + sizeof(expected)),
    encoded);

  dlms::client::DlmsDateTime decoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsDateTime(encoded, decoded));
  EXPECT_EQ(value.date.year, decoded.date.year);
  EXPECT_EQ(value.date.month, decoded.date.month);
  EXPECT_EQ(value.date.dayOfMonth, decoded.date.dayOfMonth);
  EXPECT_EQ(value.date.dayOfWeek, decoded.date.dayOfWeek);
  EXPECT_EQ(value.time.hour, decoded.time.hour);
  EXPECT_EQ(value.time.minute, decoded.time.minute);
  EXPECT_EQ(value.time.second, decoded.time.second);
  EXPECT_EQ(value.time.hundredths, decoded.time.hundredths);
  EXPECT_EQ(value.deviation, decoded.deviation);
  EXPECT_EQ(value.clockStatus, decoded.clockStatus);
}

TEST(ClientDataHelpers, EncodeAndDecodeDateAndTimeWildcards)
{
  dlms::client::DlmsDate date;
  date.year = 0xFFFFu;
  date.month = 0xFEu;
  date.dayOfMonth = 0xFDu;
  date.dayOfWeek = 0xFFu;

  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsDate(date, encoded));
  const std::uint8_t expectedDate[] = {
    0x1Au,
    0xFFu,
    0xFFu,
    0xFEu,
    0xFDu,
    0xFFu};
  EXPECT_EQ(
    std::vector<std::uint8_t>(
      expectedDate,
      expectedDate + sizeof(expectedDate)),
    encoded);

  dlms::client::DlmsDate decodedDate;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsDate(encoded, decodedDate));
  EXPECT_EQ(date.year, decodedDate.year);
  EXPECT_EQ(date.month, decodedDate.month);
  EXPECT_EQ(date.dayOfMonth, decodedDate.dayOfMonth);
  EXPECT_EQ(date.dayOfWeek, decodedDate.dayOfWeek);

  dlms::client::DlmsTime time;
  time.hour = 0xFFu;
  time.minute = 0xFFu;
  time.second = 0xFFu;
  time.hundredths = 0xFFu;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::EncodeDlmsTime(time, encoded));
  const std::uint8_t expectedTime[] = {
    0x1Bu,
    0xFFu,
    0xFFu,
    0xFFu,
    0xFFu};
  EXPECT_EQ(
    std::vector<std::uint8_t>(
      expectedTime,
      expectedTime + sizeof(expectedTime)),
    encoded);

  dlms::client::DlmsTime decodedTime;
  ASSERT_EQ(
    dlms::client::ClientStatus::Ok,
    dlms::client::DecodeDlmsTime(encoded, decodedTime));
  EXPECT_EQ(time.hour, decodedTime.hour);
  EXPECT_EQ(time.minute, decodedTime.minute);
  EXPECT_EQ(time.second, decodedTime.second);
  EXPECT_EQ(time.hundredths, decodedTime.hundredths);
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

  dlms::client::DlmsDate date;
  date.year = 2024u;
  EXPECT_EQ(
    dlms::client::ClientStatus::InvalidArgument,
    dlms::client::DecodeDlmsDate(Bytes(0x11u, 0x01u), date));
  EXPECT_EQ(0u, date.year);
}
