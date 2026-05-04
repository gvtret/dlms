#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/hdlc/hdlc_codec.hpp"
#include "dlms/hdlc/hdlc_frame.hpp"
#include "dlms/llc/llc_codec.hpp"
#include "dlms/llc/llc_header.hpp"

#if DLMS_APDU_STACK_HAS_WRAPPER
#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/wrapper/wrapper_ports.hpp"
#endif

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::DecodeAcseApdu;
using dlms::apdu::DecodeGetResponseNormal;
using dlms::apdu::DecodeXdlmsApdu;
using dlms::apdu::EncodeAcseApdu;
using dlms::apdu::EncodeXdlmsApdu;
using dlms::apdu::GetDataResultChoice;
using dlms::apdu::GetResponseNormal;
using dlms::apdu::MakeAarqWithInitiateRequest;
using dlms::apdu::MakeGetRequestNormal;
using dlms::apdu::XdlmsApdu;
using dlms::hdlc::DecodeFrame;
using dlms::hdlc::DefaultHdlcCodecLimits;
using dlms::hdlc::HdlcFrameBuffer;
using dlms::hdlc::HdlcStatus;
using dlms::llc::DecodeLpdu;
using dlms::llc::EncodeDlmsRequest;
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

const std::uint8_t kSpodesGetRequestFrame[] = {
  0x7e, 0xa0, 0x1a, 0x02, 0x21, 0x61, 0x54, 0x18,
  0x87, 0xe6, 0xe6, 0x00, 0xc0, 0x01, 0x81, 0x00,
  0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xff, 0x04,
  0x00, 0x81, 0x10, 0x7e
};

#if DLMS_APDU_STACK_HAS_WRAPPER
const std::uint8_t kSpodesWrapperGetRequestWpdu[] = {
  0x00, 0x01, 0x00, 0x30, 0x00, 0x10, 0x00, 0x0D,
  0xC0, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00, 0x63,
  0x01, 0x00, 0xFF, 0x07, 0x00
};

const std::uint8_t kSpodesWrapperGetResponseWpdu[] = {
  0x00, 0x01, 0x00, 0x10, 0x00, 0x30, 0x00, 0x09,
  0xC4, 0x01, 0x81, 0x00, 0x06, 0x00, 0x00, 0x09,
  0xF1
};
#endif

std::vector<std::uint8_t> ExtractApduFromHdlcFrame(
  const std::uint8_t* frame,
  std::size_t frameSize)
{
  HdlcFrameBuffer decodedFrame;
  EXPECT_EQ(HdlcStatus::Ok,
            DecodeFrame(frame,
                        frameSize,
                        DefaultHdlcCodecLimits(),
                        decodedFrame));

  LlcLpduBuffer decodedLpdu;
  EXPECT_EQ(LlcStatus::Ok,
            DecodeLpdu(&decodedFrame.information[0],
                       decodedFrame.information.size(),
                       false,
                       decodedLpdu));

  return decodedLpdu.lsdu;
}

#if DLMS_APDU_STACK_HAS_WRAPPER
std::vector<std::uint8_t> ExtractApduFromWrapperWpdu(
  const std::uint8_t* wpdu,
  std::size_t wpduSize)
{
  dlms::wrapper::WrapperFrameBuffer decodedWrapper;
  EXPECT_EQ(dlms::wrapper::WrapperStatus::Ok,
            dlms::wrapper::DecodeWpdu(wpdu,
                                      wpduSize,
                                      dlms::wrapper::DefaultWrapperCodecLimits(),
                                      decodedWrapper));

  return decodedWrapper.data;
}
#endif

std::vector<std::uint8_t> RoundtripThroughLlcRequest(
  const std::vector<std::uint8_t>& apdu)
{
  std::vector<std::uint8_t> lpdu;
  EXPECT_EQ(LlcStatus::Ok, EncodeDlmsRequest(&apdu[0], apdu.size(), lpdu));

  LlcLpduBuffer decodedLpdu;
  EXPECT_EQ(LlcStatus::Ok,
            DecodeLpdu(&lpdu[0], lpdu.size(), false, decodedLpdu));

  return decodedLpdu.lsdu;
}

TEST(ApduStackIntegration, AarqSurvivesLlcBoundary)
{
  const XdlmsApdu initiateRequest = dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::AcseApdu aarq = MakeAarqWithInitiateRequest(initiateRequest);

  std::vector<std::uint8_t> encodedAarq;
  ASSERT_EQ(ApduStatus::Ok, EncodeAcseApdu(aarq, encodedAarq));

  const std::vector<std::uint8_t> decodedPayload =
    RoundtripThroughLlcRequest(encodedAarq);

  dlms::apdu::AcseApdu decodedAarq;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeAcseApdu(&decodedPayload[0], decodedPayload.size(), decodedAarq));
  EXPECT_EQ(dlms::apdu::AcseApduKind::Aarq, decodedAarq.kind);
}

TEST(ApduStackIntegration, GetRequestSurvivesLlcBoundary)
{
  const XdlmsApdu getRequest = MakeGetRequestNormal(
    0x81,
    0x0007,
    dlms::apdu::LogicalName(1, 0, 99, 1, 0, 255),
    4);

  std::vector<std::uint8_t> encodedGetRequest;
  ASSERT_EQ(ApduStatus::Ok, EncodeXdlmsApdu(getRequest, encodedGetRequest));

  const std::vector<std::uint8_t> decodedPayload =
    RoundtripThroughLlcRequest(encodedGetRequest);

  XdlmsApdu decodedGetRequest;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeXdlmsApdu(&decodedPayload[0],
                            decodedPayload.size(),
                            decodedGetRequest));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetRequest, decodedGetRequest.kind);
}

TEST(ApduStackIntegration, SpodesAarqTraceDecodesToAcseApdu)
{
  const std::vector<std::uint8_t> apdu =
    ExtractApduFromHdlcFrame(kSpodesAarqFrame, sizeof(kSpodesAarqFrame));

  dlms::apdu::AcseApdu decoded;
  ASSERT_EQ(ApduStatus::Ok, DecodeAcseApdu(&apdu[0], apdu.size(), decoded));
  EXPECT_EQ(dlms::apdu::AcseApduKind::Aarq, decoded.kind);
}

TEST(ApduStackIntegration, SpodesGetRequestTraceDecodesToXdlmsApdu)
{
  const std::vector<std::uint8_t> apdu =
    ExtractApduFromHdlcFrame(kSpodesGetRequestFrame,
                             sizeof(kSpodesGetRequestFrame));

  XdlmsApdu decoded;
  ASSERT_EQ(ApduStatus::Ok, DecodeXdlmsApdu(&apdu[0], apdu.size(), decoded));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetRequest, decoded.kind);
}

TEST(ApduStackIntegration, PayloadByte7eInsideDataIsPreservedByLowerLayers)
{
  const XdlmsApdu getRequest = MakeGetRequestNormal(
    0x81,
    0x0007,
    dlms::apdu::LogicalName(1, 0, 99, 1, 0, 255),
    4);

  std::vector<std::uint8_t> encodedGetRequest;
  ASSERT_EQ(ApduStatus::Ok, EncodeXdlmsApdu(getRequest, encodedGetRequest));
  encodedGetRequest.push_back(0x7e);

  EXPECT_EQ(encodedGetRequest, RoundtripThroughLlcRequest(encodedGetRequest));
}

#if DLMS_APDU_STACK_HAS_WRAPPER
TEST(ApduStackIntegration, GetRequestSurvivesWrapperBoundary)
{
  const XdlmsApdu getRequest = MakeGetRequestNormal(
    0x81,
    0x0007,
    dlms::apdu::LogicalName(1, 0, 99, 1, 0, 255),
    4);

  std::vector<std::uint8_t> encodedGetRequest;
  ASSERT_EQ(ApduStatus::Ok, EncodeXdlmsApdu(getRequest, encodedGetRequest));

  dlms::wrapper::WrapperFrame frame;
  frame.sourcePort = dlms::wrapper::kPublicClient;
  frame.destinationPort = dlms::wrapper::kManagementLogicalDevice;
  frame.data = &encodedGetRequest[0];
  frame.dataSize = encodedGetRequest.size();

  std::vector<std::uint8_t> encodedWrapper;
  ASSERT_EQ(dlms::wrapper::WrapperStatus::Ok,
            dlms::wrapper::EncodeWpdu(frame,
                                      dlms::wrapper::DefaultWrapperCodecLimits(),
                                      encodedWrapper));

  dlms::wrapper::WrapperFrameBuffer decodedWrapper;
  ASSERT_EQ(dlms::wrapper::WrapperStatus::Ok,
            dlms::wrapper::DecodeWpdu(&encodedWrapper[0],
                                      encodedWrapper.size(),
                                      dlms::wrapper::DefaultWrapperCodecLimits(),
                                      decodedWrapper));

  XdlmsApdu decoded;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeXdlmsApdu(&decodedWrapper.data[0],
                            decodedWrapper.data.size(),
                            decoded));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetRequest, decoded.kind);
}

TEST(ApduStackIntegration, SpodesWrapperTraceGetRequestResponseDecodeToApdu)
{
  const std::vector<std::uint8_t> requestApdu =
    ExtractApduFromWrapperWpdu(kSpodesWrapperGetRequestWpdu,
                               sizeof(kSpodesWrapperGetRequestWpdu));

  XdlmsApdu decodedRequest;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeXdlmsApdu(&requestApdu[0],
                            requestApdu.size(),
                            decodedRequest));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetRequest, decodedRequest.kind);
  EXPECT_EQ(0x0007u, decodedRequest.getRequest.descriptor.classId);
  EXPECT_EQ(0x07u, decodedRequest.getRequest.descriptor.attributeId);

  const std::vector<std::uint8_t> responseApdu =
    ExtractApduFromWrapperWpdu(kSpodesWrapperGetResponseWpdu,
                               sizeof(kSpodesWrapperGetResponseWpdu));

  GetResponseNormal decodedResponse = {};
  ASSERT_EQ(ApduStatus::Ok,
            DecodeGetResponseNormal(&responseApdu[0],
                                    responseApdu.size(),
                                    8,
                                    decodedResponse));
  EXPECT_EQ(GetDataResultChoice::Data, decodedResponse.resultChoice);
  EXPECT_EQ(0x000009F1u, decodedResponse.data.unsignedValue);
}
#endif

} // namespace
