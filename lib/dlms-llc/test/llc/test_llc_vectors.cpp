#include "dlms/llc/llc_codec.hpp"

#include "dlms/llc/llc_types.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using dlms::llc::DecodeLpdu;
using dlms::llc::DecodeLpduFromBuffer;
using dlms::llc::EncodeDlmsRequest;
using dlms::llc::EncodeDlmsResponse;
using dlms::llc::kLlcControlDefault;
using dlms::llc::kLlcDsapDlms;
using dlms::llc::kLlcHeaderSize;
using dlms::llc::kLlcSsapClient;
using dlms::llc::kLlcSsapServer;
using dlms::llc::LlcLpdu;
using dlms::llc::LlcLpduBuffer;
using dlms::llc::LlcStatus;

const std::uint8_t kSpodesGetRequestLpdu[] = {
  0xe6, 0xe6, 0x00, 0xc0, 0x01, 0x81, 0x00, 0x07,
  0x01, 0x00, 0x63, 0x01, 0x00, 0xff, 0x04, 0x00
};

const std::uint8_t kSpodesGetResponseLpdu[] = {
  0xe6, 0xe7, 0x00, 0xc4, 0x01, 0x81,
  0x00, 0x06, 0x00, 0x00, 0x07, 0x08
};

const std::uint8_t kSpodesSegmentFirstInformationPrefix[] = {
  0xe6, 0xe7, 0x00, 0xc4, 0x02, 0x81,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00
};

const std::uint8_t kSpodesSegmentContinuationInformationPrefix[] = {
  0x07, 0xea, 0x05, 0x03, 0x07, 0x08,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x15
};

std::vector<std::uint8_t> ToVector(
  const std::uint8_t* data,
  std::size_t size)
{
  return std::vector<std::uint8_t>(data, data + size);
}

TEST(LlcVectorTest, DecodesSpodesGetRequestLpdu)
{
  LlcLpdu lpdu;
  ASSERT_EQ(LlcStatus::Ok,
            DecodeLpduFromBuffer(kSpodesGetRequestLpdu,
                                 sizeof(kSpodesGetRequestLpdu),
                                 false,
                                 lpdu));

  EXPECT_EQ(kLlcDsapDlms, lpdu.header.dsap);
  EXPECT_EQ(kLlcSsapClient, lpdu.header.ssap);
  EXPECT_EQ(kLlcControlDefault, lpdu.header.control);
  ASSERT_EQ(13u, lpdu.lsduSize);
  EXPECT_EQ(0xc0u, lpdu.lsduData[0]);

  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(LlcStatus::Ok,
            EncodeDlmsRequest(lpdu.lsduData, lpdu.lsduSize, encoded));
  EXPECT_EQ(ToVector(kSpodesGetRequestLpdu, sizeof(kSpodesGetRequestLpdu)),
            encoded);
}

TEST(LlcVectorTest, DecodesSpodesGetResponseLpdu)
{
  LlcLpduBuffer lpdu;
  ASSERT_EQ(LlcStatus::Ok,
            DecodeLpdu(kSpodesGetResponseLpdu,
                       sizeof(kSpodesGetResponseLpdu),
                       false,
                       lpdu));

  EXPECT_EQ(kLlcDsapDlms, lpdu.header.dsap);
  EXPECT_EQ(kLlcSsapServer, lpdu.header.ssap);
  EXPECT_EQ(kLlcControlDefault, lpdu.header.control);
  ASSERT_EQ(9u, lpdu.lsdu.size());
  EXPECT_EQ(0xc4u, lpdu.lsdu[0]);

  std::vector<std::uint8_t> encoded;
  ASSERT_EQ(LlcStatus::Ok,
            EncodeDlmsResponse(&lpdu.lsdu[0], lpdu.lsdu.size(), encoded));
  EXPECT_EQ(ToVector(kSpodesGetResponseLpdu, sizeof(kSpodesGetResponseLpdu)),
            encoded);
}

TEST(LlcVectorTest, AcceptsFirstSegmentInformationPrefixAsLpdu)
{
  LlcLpdu lpdu;
  ASSERT_EQ(LlcStatus::Ok,
            DecodeLpduFromBuffer(kSpodesSegmentFirstInformationPrefix,
                                 sizeof(kSpodesSegmentFirstInformationPrefix),
                                 false,
                                 lpdu));

  EXPECT_EQ(kLlcSsapServer, lpdu.header.ssap);
  ASSERT_EQ(sizeof(kSpodesSegmentFirstInformationPrefix) - kLlcHeaderSize,
            lpdu.lsduSize);
  EXPECT_EQ(0xc4u, lpdu.lsduData[0]);
}

TEST(LlcVectorTest, RejectsContinuationSegmentInformationPrefix)
{
  LlcLpdu lpdu;
  EXPECT_NE(LlcStatus::Ok,
            DecodeLpduFromBuffer(kSpodesSegmentContinuationInformationPrefix,
                                 sizeof(kSpodesSegmentContinuationInformationPrefix),
                                 false,
                                 lpdu));
}

} // namespace
