#include "dlms/transport/iec62056_21_mode_e.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using dlms::transport::BuildIec62056_21ModeEAck;
using dlms::transport::BuildIec62056_21SignOnRequest;
using dlms::transport::Iec62056_21BaudRate;
using dlms::transport::Iec62056_21BaudRateCode;
using dlms::transport::Iec62056_21BaudRateValue;
using dlms::transport::Iec62056_21Identification;
using dlms::transport::ParseIec62056_21Identification;
using dlms::transport::SelectIec62056_21ModeEBaudRate;
using dlms::transport::TransportStatus;

TEST(Iec62056_21ModeE, BuildsSignOnRequest)
{
  std::string request;

  EXPECT_EQ(TransportStatus::Ok, BuildIec62056_21SignOnRequest(request));
  EXPECT_EQ("/?!\r\n", request);
}

TEST(Iec62056_21ModeE, ParsesIdentificationOptions)
{
  Iec62056_21Identification identification;

  EXPECT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC5METER\\W2\r\n", identification));

  EXPECT_TRUE(identification.supportsModeE);
  EXPECT_EQ(Iec62056_21BaudRate::Baud9600, identification.maxBaudRate);
  EXPECT_EQ("/ABC5METER\\W2\r\n", identification.raw);
}

TEST(Iec62056_21ModeE, SelectsRequestedBaudRate)
{
  Iec62056_21Identification identification;
  ASSERT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC6METER\\W2\r\n", identification));

  Iec62056_21BaudRate selected = Iec62056_21BaudRate::Baud300;
  EXPECT_EQ(
    TransportStatus::Ok,
    SelectIec62056_21ModeEBaudRate(
      identification,
      Iec62056_21BaudRate::Baud9600,
      selected));
  EXPECT_EQ(Iec62056_21BaudRate::Baud9600, selected);

  std::string ack;
  EXPECT_EQ(TransportStatus::Ok, BuildIec62056_21ModeEAck(selected, ack));
  ASSERT_EQ(6u, ack.size());
  EXPECT_EQ(0x06, static_cast<unsigned char>(ack[0]));
  EXPECT_EQ('2', ack[1]);
  EXPECT_EQ('5', ack[2]);
  EXPECT_EQ('2', ack[3]);
  EXPECT_EQ('\r', ack[4]);
  EXPECT_EQ('\n', ack[5]);
}

TEST(Iec62056_21ModeE, RejectsUnsupportedMode)
{
  Iec62056_21Identification identification;
  ASSERT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC5METER\\W1\r\n", identification));

  Iec62056_21BaudRate selected = Iec62056_21BaudRate::Baud300;
  EXPECT_EQ(
    TransportStatus::UnsupportedFeature,
    SelectIec62056_21ModeEBaudRate(
      identification,
      Iec62056_21BaudRate::Baud9600,
      selected));
}

TEST(Iec62056_21ModeE, DoesNotParseHdlcFrames)
{
  Iec62056_21Identification identification;

  EXPECT_EQ(
    TransportStatus::InvalidArgument,
    ParseIec62056_21Identification("~\xA0\x03\x00~", identification));
}

TEST(Iec62056_21ModeE, ReportsBaudRateCodesAndValues)
{
  EXPECT_EQ('0', Iec62056_21BaudRateCode(Iec62056_21BaudRate::Baud300));
  EXPECT_EQ('6', Iec62056_21BaudRateCode(Iec62056_21BaudRate::Baud19200));
  EXPECT_EQ(300u, Iec62056_21BaudRateValue(Iec62056_21BaudRate::Baud300));
  EXPECT_EQ(19200u, Iec62056_21BaudRateValue(Iec62056_21BaudRate::Baud19200));
}

TEST(Iec62056_21ModeE, BaudRateCodeAndValueAreDefinedForEveryRate)
{
  // Exhaustive coverage so any future enum extension breaks the test first.
  const Iec62056_21BaudRate rates[] = {
    Iec62056_21BaudRate::Baud300,
    Iec62056_21BaudRate::Baud600,
    Iec62056_21BaudRate::Baud1200,
    Iec62056_21BaudRate::Baud2400,
    Iec62056_21BaudRate::Baud4800,
    Iec62056_21BaudRate::Baud9600,
    Iec62056_21BaudRate::Baud19200,
  };
  const char expectedCodes[] = { '0', '1', '2', '3', '4', '5', '6' };
  const std::uint32_t expectedValues[] = {
    300u, 600u, 1200u, 2400u, 4800u, 9600u, 19200u
  };
  for (std::size_t i = 0; i < sizeof(rates) / sizeof(rates[0]); ++i) {
    EXPECT_EQ(expectedCodes[i], Iec62056_21BaudRateCode(rates[i]))
      << "index " << i;
    EXPECT_EQ(expectedValues[i], Iec62056_21BaudRateValue(rates[i]))
      << "index " << i;
  }
}

TEST(Iec62056_21ModeE, SignOnRequestOverwritesPreviousOutput)
{
  std::string output = "garbage";
  EXPECT_EQ(TransportStatus::Ok, BuildIec62056_21SignOnRequest(output));
  EXPECT_EQ("/?!\r\n", output);
}

TEST(Iec62056_21ModeE, ParseRejectsMissingLeadingSlash)
{
  Iec62056_21Identification identification;
  EXPECT_EQ(
    TransportStatus::InvalidArgument,
    ParseIec62056_21Identification("ABC5METER\r\n", identification));
}

TEST(Iec62056_21ModeE, ParseRejectsMissingCrLfTerminator)
{
  Iec62056_21Identification identification;
  EXPECT_EQ(
    TransportStatus::InvalidArgument,
    ParseIec62056_21Identification("/ABC5METER", identification));
  EXPECT_EQ(
    TransportStatus::InvalidArgument,
    ParseIec62056_21Identification("/ABC5METER\n", identification));
  EXPECT_EQ(
    TransportStatus::InvalidArgument,
    ParseIec62056_21Identification("/ABC5METER\r", identification));
}

TEST(Iec62056_21ModeE, ParseRejectsTooShortFrame)
{
  Iec62056_21Identification identification;
  // "/A\r\n" is 4 chars; spec requires at least 5 (slash + 3 manufacturer + baud code).
  EXPECT_EQ(
    TransportStatus::InvalidArgument,
    ParseIec62056_21Identification("/A\r\n", identification));
}

TEST(Iec62056_21ModeE, ParseRejectsUnknownBaudCode)
{
  Iec62056_21Identification identification;
  EXPECT_EQ(
    TransportStatus::InvalidArgument,
    ParseIec62056_21Identification("/ABCXMETER\r\n", identification));
}

TEST(Iec62056_21ModeE, ParseAcceptsFrameWithoutModeMarker)
{
  Iec62056_21Identification identification;
  EXPECT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC5METER\r\n", identification));
  EXPECT_FALSE(identification.supportsModeE);
  EXPECT_EQ(Iec62056_21BaudRate::Baud9600, identification.maxBaudRate);
}

TEST(Iec62056_21ModeE, ParseRejectsModeMarkerForNonModeE)
{
  // \W1 means Mode B; \W3 means Mode D in IEC 62056-21. Both must NOT be reported as Mode E.
  Iec62056_21Identification identification;
  ASSERT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC5METER\\W1\r\n", identification));
  EXPECT_FALSE(identification.supportsModeE);

  ASSERT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC5METER\\W3\r\n", identification));
  EXPECT_FALSE(identification.supportsModeE);
}

TEST(Iec62056_21ModeE, ParseHandlesTruncatedModeMarker)
{
  // \W at the very end with no following digit: must not over-read.
  Iec62056_21Identification identification;
  ASSERT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC5METER\\W\r\n", identification));
  EXPECT_FALSE(identification.supportsModeE);
}

TEST(Iec62056_21ModeE, SelectDowngradesToMeterMaxWhenClientAsksHigher)
{
  Iec62056_21Identification identification;
  // Meter advertises Mode E with max baud = 2400 (code '3').
  ASSERT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC3METER\\W2\r\n", identification));

  Iec62056_21BaudRate selected = Iec62056_21BaudRate::Baud300;
  EXPECT_EQ(
    TransportStatus::Ok,
    SelectIec62056_21ModeEBaudRate(
      identification,
      Iec62056_21BaudRate::Baud19200,
      selected));
  EXPECT_EQ(Iec62056_21BaudRate::Baud2400, selected);
}

TEST(Iec62056_21ModeE, SelectHonoursClientCapWhenMeterAdvertisesHigher)
{
  Iec62056_21Identification identification;
  ASSERT_EQ(
    TransportStatus::Ok,
    ParseIec62056_21Identification("/ABC6METER\\W2\r\n", identification));

  Iec62056_21BaudRate selected = Iec62056_21BaudRate::Baud300;
  EXPECT_EQ(
    TransportStatus::Ok,
    SelectIec62056_21ModeEBaudRate(
      identification,
      Iec62056_21BaudRate::Baud2400,
      selected));
  EXPECT_EQ(Iec62056_21BaudRate::Baud2400, selected);
}

TEST(Iec62056_21ModeE, BuildModeEAckRejectsUnknownBaudRate)
{
  std::string output = "residual";
  // Force an out-of-enum value via cast. Status code is the contract; output must be cleared.
  const Iec62056_21BaudRate bogus = static_cast<Iec62056_21BaudRate>(99);
  EXPECT_EQ(TransportStatus::InvalidArgument, BuildIec62056_21ModeEAck(bogus, output));
  EXPECT_TRUE(output.empty());
}

} // namespace
