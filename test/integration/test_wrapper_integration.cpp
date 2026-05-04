#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/wrapper/wrapper_error.hpp"
#include "dlms/wrapper/wrapper_ports.hpp"
#include "dlms/wrapper/wrapper_stream_decoder.hpp"

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using dlms::wrapper::DecodeWpdu;
using dlms::wrapper::DefaultWrapperCodecLimits;
using dlms::wrapper::EncodeWpdu;
using dlms::wrapper::WrapperFrame;
using dlms::wrapper::WrapperFrameBuffer;
using dlms::wrapper::WrapperStatus;
using dlms::wrapper::WrapperStreamDecoder;
using dlms::wrapper::WrapperStreamDecoderOptions;
using dlms::wrapper::kManagementLogicalDevice;
using dlms::wrapper::kPublicClient;

const std::uint8_t kAarqApdu[] = {
  0x60, 0x1d, 0xa1, 0x09, 0x06, 0x07, 0x60, 0x85,
  0x74, 0x05, 0x08, 0x01, 0x01, 0x8a, 0x02, 0x07,
  0x80, 0x8b, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
  0x02, 0x02, 0xbe, 0x10, 0x04, 0x0e, 0x01
};

const std::uint8_t kGetRequestApdu[] = {
  0xc0, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00,
  0x63, 0x01, 0x00, 0xff, 0x04, 0x00
};

WrapperFrame MakeFrame(const std::vector<std::uint8_t>& apdu)
{
  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = apdu.empty() ? 0 : &apdu[0];
  frame.dataSize = apdu.size();
  return frame;
}

std::vector<std::uint8_t> EncodeApdu(const std::vector<std::uint8_t>& apdu)
{
  std::vector<std::uint8_t> encoded;
  EXPECT_EQ(WrapperStatus::Ok,
            EncodeWpdu(MakeFrame(apdu), DefaultWrapperCodecLimits(), encoded));
  return encoded;
}

WrapperFrameBuffer DecodeBytes(const std::vector<std::uint8_t>& encoded)
{
  WrapperFrameBuffer decoded;
  EXPECT_EQ(WrapperStatus::Ok,
            DecodeWpdu(&encoded[0],
                       encoded.size(),
                       DefaultWrapperCodecLimits(),
                       decoded));
  return decoded;
}

TEST(WrapperIntegration, AarqApduSurvivesWrapperRoundtrip)
{
  const std::vector<std::uint8_t> apdu(kAarqApdu, kAarqApdu + sizeof(kAarqApdu));

  const WrapperFrameBuffer decoded = DecodeBytes(EncodeApdu(apdu));

  EXPECT_EQ(kPublicClient, decoded.sourcePort);
  EXPECT_EQ(kManagementLogicalDevice, decoded.destinationPort);
  EXPECT_EQ(apdu, decoded.data);
}

TEST(WrapperIntegration, GetRequestApduSurvivesWrapperRoundtrip)
{
  const std::vector<std::uint8_t> apdu(
    kGetRequestApdu,
    kGetRequestApdu + sizeof(kGetRequestApdu));

  const WrapperFrameBuffer decoded = DecodeBytes(EncodeApdu(apdu));

  EXPECT_EQ(apdu, decoded.data);
}

TEST(WrapperIntegration, TcpChunksReassembleWpdu)
{
  const std::vector<std::uint8_t> apdu(kAarqApdu, kAarqApdu + sizeof(kAarqApdu));
  const std::vector<std::uint8_t> encoded = EncodeApdu(apdu);

  WrapperStreamDecoderOptions options;
  options.limits = DefaultWrapperCodecLimits();
  WrapperStreamDecoder decoder(options);

  std::vector<WrapperFrameBuffer> frames;
  ASSERT_EQ(WrapperStatus::NeedMoreData, decoder.Push(&encoded[0], 3, frames));
  EXPECT_TRUE(frames.empty());

  ASSERT_EQ(WrapperStatus::NeedMoreData, decoder.Push(&encoded[3], 5, frames));
  EXPECT_TRUE(frames.empty());

  ASSERT_EQ(WrapperStatus::Ok,
            decoder.Push(&encoded[8], encoded.size() - 8, frames));

  ASSERT_EQ(1u, frames.size());
  EXPECT_EQ(apdu, frames[0].data);
}

TEST(WrapperIntegration, MultipleWpdusPreserveBoundaries)
{
  const std::vector<std::uint8_t> aarq(kAarqApdu, kAarqApdu + sizeof(kAarqApdu));
  const std::vector<std::uint8_t> getRequest(
    kGetRequestApdu,
    kGetRequestApdu + sizeof(kGetRequestApdu));

  std::vector<std::uint8_t> stream = EncodeApdu(aarq);
  const std::vector<std::uint8_t> second = EncodeApdu(getRequest);
  stream.insert(stream.end(), second.begin(), second.end());

  WrapperStreamDecoderOptions options;
  options.limits = DefaultWrapperCodecLimits();
  WrapperStreamDecoder decoder(options);

  std::vector<WrapperFrameBuffer> frames;
  ASSERT_EQ(WrapperStatus::Ok, decoder.Push(&stream[0], stream.size(), frames));

  ASSERT_EQ(2u, frames.size());
  EXPECT_EQ(aarq, frames[0].data);
  EXPECT_EQ(getRequest, frames[1].data);
}

TEST(WrapperIntegration, PayloadByte7eIsData)
{
  const std::uint8_t payload[] = {0xc0, 0x01, 0x7e, 0x00, 0xff};
  const std::vector<std::uint8_t> apdu(payload, payload + sizeof(payload));

  const WrapperFrameBuffer decoded = DecodeBytes(EncodeApdu(apdu));

  EXPECT_EQ(apdu, decoded.data);
}

TEST(WrapperIntegration, InvalidDestinationPortIsRejectedByPolicy)
{
  const std::uint8_t apdu[] = {0xc0, 0x01};
  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = 0x0000;
  frame.data = apdu;
  frame.dataSize = sizeof(apdu);

  std::vector<std::uint8_t> encoded;
  EXPECT_NE(WrapperStatus::Ok,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), encoded));
}

} // namespace
