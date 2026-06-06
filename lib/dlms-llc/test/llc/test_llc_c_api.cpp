#include "dlms/llc/llc_c_api.h"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

extern "C" int dlms_llc_c_header_smoke(void);

namespace {

TEST(LlcCApiTest, HeaderCompilesAsC)
{
  EXPECT_GT(dlms_llc_c_header_smoke(), 0);
}

TEST(LlcCApiTest, StatusValuesMatchStableAbi)
{
  EXPECT_EQ(0, DLMS_LLC_STATUS_OK);
  EXPECT_EQ(1, DLMS_LLC_STATUS_NEED_MORE_DATA);
  EXPECT_EQ(2, DLMS_LLC_STATUS_OUTPUT_BUFFER_TOO_SMALL);
  EXPECT_EQ(3, DLMS_LLC_STATUS_INVALID_ARGUMENT);
  EXPECT_EQ(4, DLMS_LLC_STATUS_INVALID_HEADER);
  EXPECT_EQ(5, DLMS_LLC_STATUS_INVALID_DSAP);
  EXPECT_EQ(6, DLMS_LLC_STATUS_INVALID_SSAP);
  EXPECT_EQ(7, DLMS_LLC_STATUS_INVALID_CONTROL);
  EXPECT_EQ(8, DLMS_LLC_STATUS_INVALID_LPDU_LENGTH);
  EXPECT_EQ(9, DLMS_LLC_STATUS_LSDU_TOO_LARGE);
  EXPECT_EQ(10, DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN);
  EXPECT_EQ(14, DLMS_LLC_STATUS_INTERNAL_ERROR);
}

TEST(LlcCApiTest, EncodesRequestIntoCallerBuffer)
{
  const std::uint8_t apdu[] = {0xC0, 0x01};
  std::uint8_t output[5] = {};
  std::size_t writtenSize = 99;

  EXPECT_EQ(DLMS_LLC_STATUS_OK,
            dlms_llc_encode_request(
              apdu, sizeof(apdu), output, sizeof(output), &writtenSize));

  EXPECT_EQ(sizeof(output), writtenSize);
  EXPECT_EQ(0xE6u, output[0]);
  EXPECT_EQ(0xE6u, output[1]);
  EXPECT_EQ(0x00u, output[2]);
  EXPECT_EQ(0xC0u, output[3]);
  EXPECT_EQ(0x01u, output[4]);
}

TEST(LlcCApiTest, EncodesResponseIntoCallerBuffer)
{
  const std::uint8_t apdu[] = {0xC4, 0x01};
  std::uint8_t output[5] = {};
  std::size_t writtenSize = 99;

  EXPECT_EQ(DLMS_LLC_STATUS_OK,
            dlms_llc_encode_response(
              apdu, sizeof(apdu), output, sizeof(output), &writtenSize));

  EXPECT_EQ(sizeof(output), writtenSize);
  EXPECT_EQ(0xE6u, output[0]);
  EXPECT_EQ(0xE7u, output[1]);
  EXPECT_EQ(0x00u, output[2]);
  EXPECT_EQ(0xC4u, output[3]);
  EXPECT_EQ(0x01u, output[4]);
}

TEST(LlcCApiTest, EncodesEmptyPayload)
{
  const dlms_llc_header_t header = {0xE6u, 0xE6u, 0x00u};
  std::uint8_t output[3] = {};
  std::size_t writtenSize = 99;

  EXPECT_EQ(DLMS_LLC_STATUS_OK,
            dlms_llc_encode_lpdu(
              &header, 0, 0u, output, sizeof(output), &writtenSize));

  EXPECT_EQ(sizeof(output), writtenSize);
  EXPECT_EQ(0xE6u, output[0]);
  EXPECT_EQ(0xE6u, output[1]);
  EXPECT_EQ(0x00u, output[2]);
}

TEST(LlcCApiTest, ReportsSmallOutputBuffer)
{
  const std::uint8_t apdu[] = {0xC0, 0x01};
  std::uint8_t output[4] = {};
  std::size_t writtenSize = 99;

  EXPECT_EQ(DLMS_LLC_STATUS_OUTPUT_BUFFER_TOO_SMALL,
            dlms_llc_encode_request(
              apdu, sizeof(apdu), output, sizeof(output), &writtenSize));
  EXPECT_EQ(5u, writtenSize);
  EXPECT_EQ(0u, output[0]);
  EXPECT_EQ(0u, output[1]);
  EXPECT_EQ(0u, output[2]);
  EXPECT_EQ(0u, output[3]);
}

TEST(LlcCApiTest, RejectsBroadcastDestinationForEncode)
{
  const dlms_llc_header_t header = {0xFFu, 0xE6u, 0x00u};
  const std::uint8_t apdu[] = {0xC0};
  std::uint8_t output[4] = {};
  std::size_t writtenSize = 99;

  EXPECT_EQ(DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN,
            dlms_llc_encode_lpdu(
              &header, apdu, sizeof(apdu), output, sizeof(output), &writtenSize));
  EXPECT_EQ(0u, writtenSize);
}

TEST(LlcCApiTest, ValidatesNullArguments)
{
  const dlms_llc_header_t header = {0xE6u, 0xE6u, 0x00u};
  const std::uint8_t apdu[] = {0xC0};
  std::uint8_t output[4] = {};
  std::size_t writtenSize = 0;

  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_ARGUMENT,
            dlms_llc_encode_lpdu(
              0, apdu, sizeof(apdu), output, sizeof(output), &writtenSize));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_ARGUMENT,
            dlms_llc_encode_lpdu(
              &header, 0, sizeof(apdu), output, sizeof(output), &writtenSize));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_ARGUMENT,
            dlms_llc_encode_lpdu(
              &header, apdu, sizeof(apdu), 0, sizeof(output), &writtenSize));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_ARGUMENT,
            dlms_llc_encode_lpdu(
              &header, apdu, sizeof(apdu), output, sizeof(output), 0));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_ARGUMENT,
            dlms_llc_decode_lpdu(output, sizeof(output), 0, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_ARGUMENT,
            dlms_llc_decode_lpdu(0, sizeof(output), 0, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_ARGUMENT,
            dlms_llc_validate_header(0, 0));
}

TEST(LlcCApiTest, DecodesLpduAsView)
{
  const std::uint8_t lpduBytes[] = {
    0xE6u,
    0xE7u,
    0x00u,
    0xC4u,
    0x01u
  };
  dlms_llc_lpdu_view_t lpdu;

  EXPECT_EQ(DLMS_LLC_STATUS_OK,
            dlms_llc_decode_lpdu(lpduBytes, sizeof(lpduBytes), 0, &lpdu));

  EXPECT_EQ(0xE6u, lpdu.header.dsap);
  EXPECT_EQ(0xE7u, lpdu.header.ssap);
  EXPECT_EQ(0x00u, lpdu.header.control);
  EXPECT_EQ(lpduBytes + 3, lpdu.lsdu);
  EXPECT_EQ(2u, lpdu.lsdu_size);
}

TEST(LlcCApiTest, DecodeReportsNeedMoreData)
{
  const std::uint8_t shortLpdu[] = {0xE6u, 0xE6u};
  dlms_llc_lpdu_view_t lpdu;
  lpdu.header.dsap = 0xAAu;
  lpdu.header.ssap = 0xBBu;
  lpdu.header.control = 0xCCu;
  lpdu.lsdu = shortLpdu;
  lpdu.lsdu_size = 99u;

  EXPECT_EQ(DLMS_LLC_STATUS_NEED_MORE_DATA,
            dlms_llc_decode_lpdu(shortLpdu, sizeof(shortLpdu), 0, &lpdu));
  EXPECT_EQ(0u, lpdu.header.dsap);
  EXPECT_EQ(0u, lpdu.header.ssap);
  EXPECT_EQ(0u, lpdu.header.control);
  EXPECT_EQ(static_cast<const std::uint8_t*>(0), lpdu.lsdu);
  EXPECT_EQ(0u, lpdu.lsdu_size);

  lpdu.header.dsap = 0xAAu;
  lpdu.header.ssap = 0xBBu;
  lpdu.header.control = 0xCCu;
  lpdu.lsdu = shortLpdu;
  lpdu.lsdu_size = 99u;
  EXPECT_EQ(DLMS_LLC_STATUS_NEED_MORE_DATA,
            dlms_llc_decode_lpdu(0, 0u, 0, &lpdu));
  EXPECT_EQ(0u, lpdu.header.dsap);
  EXPECT_EQ(0u, lpdu.header.ssap);
  EXPECT_EQ(0u, lpdu.header.control);
  EXPECT_EQ(static_cast<const std::uint8_t*>(0), lpdu.lsdu);
  EXPECT_EQ(0u, lpdu.lsdu_size);
}

TEST(LlcCApiTest, DecodeBroadcastDestinationRequiresExplicitPolicy)
{
  const std::uint8_t broadcastLpdu[] = {
    0xFFu,
    0xE6u,
    0x00u,
    0xC0u
  };
  dlms_llc_lpdu_view_t lpdu;

  EXPECT_EQ(DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN,
            dlms_llc_decode_lpdu(broadcastLpdu, sizeof(broadcastLpdu), 0, &lpdu));

  ASSERT_EQ(DLMS_LLC_STATUS_OK,
            dlms_llc_decode_lpdu(broadcastLpdu, sizeof(broadcastLpdu), 1, &lpdu));
  EXPECT_EQ(0xFFu, lpdu.header.dsap);
  EXPECT_EQ(0xE6u, lpdu.header.ssap);
  EXPECT_EQ(0x00u, lpdu.header.control);
  EXPECT_EQ(broadcastLpdu + 3, lpdu.lsdu);
  EXPECT_EQ(1u, lpdu.lsdu_size);
}

TEST(LlcCApiTest, ValidateHeaderUsesNamedStatuses)
{
  const dlms_llc_header_t valid = {0xE6u, 0xE6u, 0x00u};
  const dlms_llc_header_t invalidDsap = {0x01u, 0xE6u, 0x00u};
  const dlms_llc_header_t invalidSsap = {0xE6u, 0x01u, 0x00u};
  const dlms_llc_header_t invalidControl = {0xE6u, 0xE6u, 0x03u};
  const dlms_llc_header_t broadcast = {0xFFu, 0xE6u, 0x00u};

  EXPECT_EQ(DLMS_LLC_STATUS_OK, dlms_llc_validate_header(&valid, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_DSAP,
            dlms_llc_validate_header(&invalidDsap, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_INVALID_SSAP,
            dlms_llc_validate_header(&invalidSsap, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_UNSUPPORTED_CONTROL,
            dlms_llc_validate_header(&invalidControl, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN,
            dlms_llc_validate_header(&broadcast, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_OK,
            dlms_llc_validate_header(&broadcast, 1));
}

} // namespace
