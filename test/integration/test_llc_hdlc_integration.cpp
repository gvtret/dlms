#include "dlms/hdlc/hdlc_address.hpp"
#include "dlms/hdlc/hdlc_codec.hpp"
#include "dlms/hdlc/hdlc_control.hpp"
#include "dlms/hdlc/hdlc_frame.hpp"
#include "dlms/hdlc/hdlc_types.hpp"
#include "dlms/llc/llc_codec.hpp"
#include "dlms/llc/llc_error.hpp"
#include "dlms/llc/llc_header.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using dlms::hdlc::DecodeFrame;
using dlms::hdlc::DefaultHdlcCodecLimits;
using dlms::hdlc::DlmsHdlcAddress;
using dlms::hdlc::EncodeFrame;
using dlms::hdlc::HdlcControl;
using dlms::hdlc::HdlcFrame;
using dlms::hdlc::HdlcFrameBuffer;
using dlms::hdlc::HdlcStatus;
using dlms::llc::DecodeLpdu;
using dlms::llc::EncodeDlmsRequest;
using dlms::llc::EncodeDlmsResponse;
using dlms::llc::LlcLpduBuffer;
using dlms::llc::LlcStatus;

const std::uint8_t kSpodesAarqFrame[] = {
  0x7e, 0xa0, 0x51, 0x02, 0x21, 0x61, 0x10, 0xf6,
  0x05, 0xe6, 0xe6, 0x00, 0x60, 0x42, 0x80, 0x02,
  0x02, 0x84, 0xa1, 0x09, 0x06, 0x07, 0x60, 0x85,
  0x74, 0x05, 0x08, 0x01, 0x01, 0x8a, 0x02, 0x07,
  0x80, 0x8b, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
  0x02, 0x02, 0xac, 0x12, 0x80, 0x10, 0x16, 0x1e,
  0x69, 0x35, 0x35, 0x25, 0x6c, 0x25, 0x42, 0x52,
  0x6b, 0x48, 0x26, 0x72, 0x17, 0x42, 0xbe, 0x10,
  0x04, 0x0e, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5f,
  0x1f, 0x04, 0x00, 0x62, 0x1e, 0x5d, 0x02, 0x00,
  0x1b, 0x89, 0x7e
};

const std::uint8_t kSpodesAareFrame[] = {
  0x7e, 0xa0, 0x5d, 0x61, 0x02, 0x21, 0x30, 0xd4,
  0x45, 0xe6, 0xe7, 0x00, 0x61, 0x4e, 0x80, 0x02,
  0x02, 0x84, 0xa1, 0x09, 0x06, 0x07, 0x60, 0x85,
  0x74, 0x05, 0x08, 0x01, 0x01, 0xa2, 0x03, 0x02,
  0x01, 0x00, 0xa3, 0x05, 0xa1, 0x03, 0x02, 0x01,
  0x0e, 0x88, 0x02, 0x07, 0x80, 0x89, 0x07, 0x60,
  0x85, 0x74, 0x05, 0x08, 0x02, 0x02, 0xaa, 0x12,
  0x80, 0x10, 0xc6, 0x69, 0x73, 0x51, 0xff, 0x4a,
  0xec, 0x29, 0xcd, 0xba, 0xab, 0xf2, 0xfb, 0xe3,
  0x46, 0x7c, 0xbe, 0x10, 0x04, 0x0e, 0x08, 0x00,
  0x06, 0x5f, 0x1f, 0x04, 0x00, 0x40, 0x18, 0x1d,
  0x02, 0x00, 0x00, 0x07, 0xe5, 0x47, 0x7e
};

const std::uint8_t kSpodesGetRequestFrame[] = {
  0x7e, 0xa0, 0x1a, 0x02, 0x21, 0x61, 0x54, 0x18,
  0x87, 0xe6, 0xe6, 0x00, 0xc0, 0x01, 0x81, 0x00,
  0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xff, 0x04,
  0x00, 0x81, 0x10, 0x7e
};

const std::uint8_t kSpodesGetResponseFrame[] = {
  0x7e, 0xa0, 0x16, 0x61, 0x02, 0x21, 0x74, 0x3a,
  0xc7, 0xe6, 0xe7, 0x00, 0xc4, 0x01, 0x81, 0x00,
  0x06, 0x00, 0x00, 0x07, 0x08, 0x3b, 0x1e, 0x7e
};

const std::uint8_t kSpodesSegmentFirstFrame[] = {
  0x7e, 0xa8, 0x87, 0x61, 0x02, 0x21, 0xda, 0x47,
  0x9f, 0xe6, 0xe7, 0x00, 0xc4, 0x02, 0x81, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x82, 0x01, 0xf4,
  0x01, 0x1a, 0x02, 0x05, 0x09, 0x0c, 0x07, 0xea,
  0x05, 0x03, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x05,
  0x09, 0x0c, 0x07, 0xea, 0x05, 0x03, 0x07, 0x07,
  0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x05, 0x09, 0x0c, 0x6f, 0x26,
  0x7e
};

const std::uint8_t kSpodesSegmentContinuationFrame[] = {
  0x7e, 0xa8, 0x87, 0x61, 0x02, 0x21, 0xdc, 0x71,
  0xfa, 0x07, 0xea, 0x05, 0x03, 0x07, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x02, 0x05, 0x09, 0x0c, 0x07, 0xea, 0x05,
  0x03, 0x07, 0x08, 0x1e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x05, 0x09,
  0x0c, 0x07, 0xea, 0x05, 0x03, 0x07, 0x09, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x7a,
  0x7e
};

HdlcFrame MakeInformationFrame(const std::vector<std::uint8_t>& information)
{
  HdlcFrame frame;
  frame.segmented = false;
  EXPECT_EQ(HdlcStatus::Ok, DlmsHdlcAddress::MakeServerAddress(1, 0, frame.destination));
  EXPECT_EQ(HdlcStatus::Ok, DlmsHdlcAddress::MakeClientAddress(16, frame.source));
  EXPECT_EQ(HdlcStatus::Ok, HdlcControl::Decode(0x10, frame.control));
  frame.informationData = information.empty() ? 0 : &information[0];
  frame.informationSize = information.size();
  return frame;
}

HdlcFrameBuffer RoundtripThroughHdlc(const std::vector<std::uint8_t>& information)
{
  const HdlcFrame frame = MakeInformationFrame(information);

  std::vector<std::uint8_t> encodedFrame;
  EXPECT_EQ(HdlcStatus::Ok,
            EncodeFrame(frame, DefaultHdlcCodecLimits(), encodedFrame));

  HdlcFrameBuffer decodedFrame;
  EXPECT_EQ(HdlcStatus::Ok,
            DecodeFrame(&encodedFrame[0],
                        encodedFrame.size(),
                        DefaultHdlcCodecLimits(),
                        decodedFrame));

  return decodedFrame;
}

HdlcFrameBuffer DecodeTraceFrame(
  const std::uint8_t* frameBytes,
  std::size_t frameSize)
{
  HdlcFrameBuffer decodedFrame;
  EXPECT_EQ(HdlcStatus::Ok,
            DecodeFrame(frameBytes,
                        frameSize,
                        DefaultHdlcCodecLimits(),
                        decodedFrame));
  return decodedFrame;
}

LlcLpduBuffer DecodeLlcFromTraceFrame(
  const std::uint8_t* frameBytes,
  std::size_t frameSize)
{
  const HdlcFrameBuffer decodedFrame = DecodeTraceFrame(frameBytes, frameSize);
  LlcLpduBuffer decodedLpdu;
  EXPECT_EQ(LlcStatus::Ok,
            DecodeLpdu(&decodedFrame.information[0],
                       decodedFrame.information.size(),
                       false,
                       decodedLpdu));
  return decodedLpdu;
}

TEST(LlcHdlcIntegration, ClientRequestLpduSurvivesHdlcRoundtrip)
{
  const std::uint8_t apdu[] = {
    0x60, 0x1d, 0xa1, 0x09, 0x06, 0x07, 0x60, 0x85,
    0x74, 0x05, 0x08, 0x01, 0x01
  };

  std::vector<std::uint8_t> lpdu;
  ASSERT_EQ(LlcStatus::Ok, EncodeDlmsRequest(apdu, sizeof(apdu), lpdu));

  const HdlcFrameBuffer hdlcFrame = RoundtripThroughHdlc(lpdu);
  ASSERT_EQ(lpdu, hdlcFrame.information);

  LlcLpduBuffer decodedLpdu;
  ASSERT_EQ(LlcStatus::Ok,
            DecodeLpdu(&hdlcFrame.information[0],
                       hdlcFrame.information.size(),
                       false,
                       decodedLpdu));

  ASSERT_EQ(sizeof(apdu), decodedLpdu.lsdu.size());
  EXPECT_EQ(std::vector<std::uint8_t>(apdu, apdu + sizeof(apdu)), decodedLpdu.lsdu);
}

TEST(LlcHdlcIntegration, ServerResponseLpduSurvivesHdlcRoundtrip)
{
  const std::uint8_t apdu[] = {
    0x61, 0x13, 0xa1, 0x09, 0x06, 0x07, 0x60, 0x85,
    0x74, 0x05, 0x08, 0x01, 0x01
  };

  std::vector<std::uint8_t> lpdu;
  ASSERT_EQ(LlcStatus::Ok, EncodeDlmsResponse(apdu, sizeof(apdu), lpdu));

  const HdlcFrameBuffer hdlcFrame = RoundtripThroughHdlc(lpdu);
  ASSERT_EQ(lpdu, hdlcFrame.information);

  LlcLpduBuffer decodedLpdu;
  ASSERT_EQ(LlcStatus::Ok,
            DecodeLpdu(&hdlcFrame.information[0],
                       hdlcFrame.information.size(),
                       false,
                       decodedLpdu));

  ASSERT_EQ(sizeof(apdu), decodedLpdu.lsdu.size());
  EXPECT_EQ(std::vector<std::uint8_t>(apdu, apdu + sizeof(apdu)), decodedLpdu.lsdu);
}

TEST(LlcHdlcIntegration, SpodesTraceAarqFrameDecodesAsClientRequest)
{
  const LlcLpduBuffer decodedLpdu =
    DecodeLlcFromTraceFrame(kSpodesAarqFrame, sizeof(kSpodesAarqFrame));

  EXPECT_EQ(0xe6u, decodedLpdu.header.dsap);
  EXPECT_EQ(0xe6u, decodedLpdu.header.ssap);
  ASSERT_EQ(68u, decodedLpdu.lsdu.size());
  EXPECT_EQ(0x60u, decodedLpdu.lsdu[0]);
  EXPECT_EQ(0x42u, decodedLpdu.lsdu[1]);
}

TEST(LlcHdlcIntegration, SpodesTraceAareFrameDecodesAsServerResponse)
{
  const LlcLpduBuffer decodedLpdu =
    DecodeLlcFromTraceFrame(kSpodesAareFrame, sizeof(kSpodesAareFrame));

  EXPECT_EQ(0xe6u, decodedLpdu.header.dsap);
  EXPECT_EQ(0xe7u, decodedLpdu.header.ssap);
  ASSERT_EQ(80u, decodedLpdu.lsdu.size());
  EXPECT_EQ(0x61u, decodedLpdu.lsdu[0]);
  EXPECT_EQ(0x4eu, decodedLpdu.lsdu[1]);
}

TEST(LlcHdlcIntegration, SpodesTraceGetRequestResponseDecode)
{
  const LlcLpduBuffer request =
    DecodeLlcFromTraceFrame(kSpodesGetRequestFrame, sizeof(kSpodesGetRequestFrame));
  const LlcLpduBuffer response =
    DecodeLlcFromTraceFrame(kSpodesGetResponseFrame, sizeof(kSpodesGetResponseFrame));

  EXPECT_EQ(0xe6u, request.header.ssap);
  ASSERT_EQ(13u, request.lsdu.size());
  EXPECT_EQ(0xc0u, request.lsdu[0]);

  EXPECT_EQ(0xe7u, response.header.ssap);
  ASSERT_EQ(9u, response.lsdu.size());
  EXPECT_EQ(0xc4u, response.lsdu[0]);
}

TEST(LlcHdlcIntegration, SpodesTraceSegmentedContinuationIsNotCompleteLlcLpdu)
{
  const HdlcFrameBuffer firstSegment =
    DecodeTraceFrame(kSpodesSegmentFirstFrame, sizeof(kSpodesSegmentFirstFrame));
  const HdlcFrameBuffer continuation =
    DecodeTraceFrame(kSpodesSegmentContinuationFrame,
                     sizeof(kSpodesSegmentContinuationFrame));

  ASSERT_TRUE(firstSegment.segmented);
  ASSERT_TRUE(continuation.segmented);

  LlcLpduBuffer decodedFirstSegment;
  EXPECT_EQ(LlcStatus::Ok,
            DecodeLpdu(&firstSegment.information[0],
                       firstSegment.information.size(),
                       false,
                       decodedFirstSegment));
  EXPECT_EQ(0xe7u, decodedFirstSegment.header.ssap);
  EXPECT_EQ(0xc4u, decodedFirstSegment.lsdu[0]);

  LlcLpduBuffer decodedContinuation;
  EXPECT_NE(LlcStatus::Ok,
            DecodeLpdu(&continuation.information[0],
                       continuation.information.size(),
                       false,
                       decodedContinuation));
}

TEST(LlcHdlcIntegration, HistoricalDlmsVectorDecodesLlcHeader)
{
  const std::uint8_t frameBytes[] = {
    0x7e, 0xa0, 0x1a, 0x02, 0x23, 0xc9, 0x32, 0xaf,
    0x55, 0xe6, 0xe6, 0x00, 0xc0, 0x01, 0x40, 0x00,
    0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0xff, 0x02,
    0x00, 0xea, 0xdd, 0x7e
  };

  HdlcFrameBuffer decodedFrame;
  ASSERT_EQ(HdlcStatus::Ok,
            DecodeFrame(frameBytes,
                        sizeof(frameBytes),
                        DefaultHdlcCodecLimits(),
                        decodedFrame));

  LlcLpduBuffer decodedLpdu;
  ASSERT_EQ(LlcStatus::Ok,
            DecodeLpdu(&decodedFrame.information[0],
                       decodedFrame.information.size(),
                       false,
                       decodedLpdu));

  EXPECT_EQ(0xe6u, decodedLpdu.header.dsap);
  EXPECT_EQ(0xe6u, decodedLpdu.header.ssap);
  EXPECT_EQ(0x00u, decodedLpdu.header.control);
  ASSERT_EQ(13u, decodedLpdu.lsdu.size());
  EXPECT_EQ(0xc0u, decodedLpdu.lsdu[0]);
}

TEST(LlcHdlcIntegration, RejectsHdlcInformationWithoutLlcHeader)
{
  const std::uint8_t apduWithoutLlc[] = {0xc0, 0x01};
  const HdlcFrameBuffer hdlcFrame = RoundtripThroughHdlc(
    std::vector<std::uint8_t>(apduWithoutLlc, apduWithoutLlc + sizeof(apduWithoutLlc)));

  LlcLpduBuffer decodedLpdu;
  EXPECT_EQ(LlcStatus::NeedMoreData,
            DecodeLpdu(&hdlcFrame.information[0],
                       hdlcFrame.information.size(),
                       false,
                       decodedLpdu));
}

TEST(LlcHdlcIntegration, RejectsUnsupportedLlcControlAfterHdlcDecode)
{
  const std::uint8_t malformedLpdu[] = {0xe6, 0xe6, 0x03, 0xc0, 0x01};
  const HdlcFrameBuffer hdlcFrame = RoundtripThroughHdlc(
    std::vector<std::uint8_t>(malformedLpdu, malformedLpdu + sizeof(malformedLpdu)));

  LlcLpduBuffer decodedLpdu;
  EXPECT_NE(LlcStatus::Ok,
            DecodeLpdu(&hdlcFrame.information[0],
                       hdlcFrame.information.size(),
                       false,
                       decodedLpdu));
}

TEST(LlcHdlcIntegration, PayloadByte7eInsideApduSurvivesBothLayers)
{
  const std::uint8_t apdu[] = {0xc0, 0x01, 0x7e, 0x00, 0xff};

  std::vector<std::uint8_t> lpdu;
  ASSERT_EQ(LlcStatus::Ok, EncodeDlmsRequest(apdu, sizeof(apdu), lpdu));

  const HdlcFrameBuffer hdlcFrame = RoundtripThroughHdlc(lpdu);

  LlcLpduBuffer decodedLpdu;
  ASSERT_EQ(LlcStatus::Ok,
            DecodeLpdu(&hdlcFrame.information[0],
                       hdlcFrame.information.size(),
                       false,
                       decodedLpdu));

  EXPECT_EQ(std::vector<std::uint8_t>(apdu, apdu + sizeof(apdu)), decodedLpdu.lsdu);
}

} // namespace
