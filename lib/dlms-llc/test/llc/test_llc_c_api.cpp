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

TEST(LlcCApiTest, ReportsSmallOutputBuffer)
{
  const std::uint8_t apdu[] = {0xC0, 0x01};
  std::uint8_t output[4] = {};
  std::size_t writtenSize = 99;

  EXPECT_EQ(DLMS_LLC_STATUS_OUTPUT_BUFFER_TOO_SMALL,
            dlms_llc_encode_request(
              apdu, sizeof(apdu), output, sizeof(output), &writtenSize));
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

  EXPECT_EQ(DLMS_LLC_STATUS_NEED_MORE_DATA,
            dlms_llc_decode_lpdu(shortLpdu, sizeof(shortLpdu), 0, &lpdu));
}

TEST(LlcCApiTest, ValidateHeaderUsesNamedStatuses)
{
  const dlms_llc_header_t valid = {0xE6u, 0xE6u, 0x00u};
  const dlms_llc_header_t invalidControl = {0xE6u, 0xE6u, 0x03u};

  EXPECT_EQ(DLMS_LLC_STATUS_OK, dlms_llc_validate_header(&valid, 0));
  EXPECT_EQ(DLMS_LLC_STATUS_UNSUPPORTED_CONTROL,
            dlms_llc_validate_header(&invalidControl, 0));
}

} // namespace
