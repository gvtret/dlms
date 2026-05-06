#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/hdlc/hdlc_codec.hpp"
#include "dlms/hdlc/hdlc_segmentation.hpp"
#include "dlms/hdlc/hdlc_session.hpp"
#include "dlms/llc/llc_codec.hpp"
#include "dlms/llc/llc_header.hpp"
#include "dlms/profile/hdlc_profile_channel.hpp"
#include "dlms/profile/profile_types.hpp"
#include "dlms/profile/wrapper_tcp_profile_channel.hpp"
#include "dlms/profile/wrapper_udp_profile_channel.hpp"
#include "dlms/transport/fake_transport.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::DecodeAcseApdu;
using dlms::apdu::DecodeXdlmsApdu;
using dlms::apdu::EncodeAcseApdu;
using dlms::apdu::EncodeXdlmsApdu;
using dlms::apdu::MakeAarqWithInitiateRequest;
using dlms::apdu::MakeDefaultInitiateRequest;
using dlms::apdu::MakeGetRequestNormal;
using dlms::apdu::XdlmsApdu;
using dlms::profile::DefaultApduChannelOptions;
using dlms::profile::HdlcProfileRole;
using dlms::profile::HdlcProfileChannel;
using dlms::profile::ApduChannelOptions;
using dlms::profile::ProfileByteView;
using dlms::profile::ProfileStatus;
using dlms::profile::WrapperTcpProfileChannel;
using dlms::profile::WrapperUdpProfileChannel;
using dlms::transport::FakeByteStream;
using dlms::transport::FakeDatagramTransport;

ProfileByteView View(const std::vector<std::uint8_t>& bytes)
{
  ProfileByteView view;
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

std::vector<std::uint8_t> EncodeGetRequest()
{
  const XdlmsApdu getRequest = MakeGetRequestNormal(
    0x81,
    0x0007,
    dlms::apdu::LogicalName(1, 0, 99, 1, 0, 255),
    4);

  std::vector<std::uint8_t> encoded;
  EXPECT_EQ(ApduStatus::Ok, EncodeXdlmsApdu(getRequest, encoded));
  return encoded;
}

std::vector<std::uint8_t> EncodeAarq()
{
  const XdlmsApdu initiateRequest = MakeDefaultInitiateRequest();
  const dlms::apdu::AcseApdu aarq =
    MakeAarqWithInitiateRequest(initiateRequest);

  std::vector<std::uint8_t> encoded;
  EXPECT_EQ(ApduStatus::Ok, EncodeAcseApdu(aarq, encoded));
  return encoded;
}

dlms::hdlc::HdlcSessionOptions MakeHdlcSessionOptions(
  dlms::hdlc::HdlcSessionRole role)
{
  dlms::hdlc::HdlcSessionOptions options;
  options.role = role;
  EXPECT_EQ(dlms::hdlc::HdlcStatus::Ok,
            dlms::hdlc::DlmsHdlcAddress::MakeClientAddress(
              0x10u,
              options.clientAddress));
  EXPECT_EQ(dlms::hdlc::HdlcStatus::Ok,
            dlms::hdlc::DlmsHdlcAddress::MakeServerAddress(
              0x01u,
              0x00u,
              options.serverAddress));
  options.limits = dlms::hdlc::DefaultHdlcCodecLimits();
  options.limits.maximumReassembledInformationSize = 65538u;
  options.negotiationLimits =
    dlms::hdlc::DefaultHdlcSessionNegotiationLimits();
  options.negotiationLimits.maxInformationFieldLengthTransmit = 24u;
  options.negotiationLimits.maxInformationFieldLengthReceive = 24u;
  return options;
}

std::vector<std::uint8_t> EncodeSegmentedInformationFrame(
  const dlms::hdlc::HdlcFrameBuffer& baseBuffer,
  const std::vector<std::uint8_t>& information,
  std::size_t index)
{
  dlms::hdlc::HdlcFrame baseFrame;
  baseFrame.segmented = false;
  baseFrame.destination = baseBuffer.destination;
  baseFrame.source = baseBuffer.source;
  baseFrame.control = baseBuffer.control;
  baseFrame.informationData = 0;
  baseFrame.informationSize = 0u;

  dlms::hdlc::HdlcSegmentationOptions segmentationOptions;
  segmentationOptions.limits = dlms::hdlc::DefaultHdlcCodecLimits();
  segmentationOptions.limits.maximumInformationFieldSize = 24u;
  dlms::hdlc::HdlcSegmenter segmenter(segmentationOptions);
  std::vector<dlms::hdlc::HdlcFrameBuffer> frames;
  EXPECT_EQ(dlms::hdlc::HdlcStatus::Ok,
            segmenter.SegmentInformation(baseFrame,
                                         &information[0],
                                         information.size(),
                                         frames));
  EXPECT_LT(index, frames.size());

  dlms::hdlc::HdlcFrame frame;
  frame.segmented = frames[index].segmented;
  frame.destination = frames[index].destination;
  frame.source = frames[index].source;
  frame.control = frames[index].control;
  frame.informationData = &frames[index].information[0];
  frame.informationSize = frames[index].information.size();

  std::vector<std::uint8_t> encoded;
  EXPECT_EQ(dlms::hdlc::HdlcStatus::Ok,
            dlms::hdlc::EncodeFrame(frame,
                                    dlms::hdlc::DefaultHdlcCodecLimits(),
                                    encoded));
  return encoded;
}

TEST(ProfileIntegration, GetRequestSurvivesWrapperTcpProfileChannel)
{
  FakeByteStream stream;
  WrapperTcpProfileChannel channel(stream, DefaultApduChannelOptions());
  const std::vector<std::uint8_t> encodedGetRequest = EncodeGetRequest();

  ASSERT_EQ(ProfileStatus::Ok, channel.Open());
  ASSERT_EQ(ProfileStatus::Ok, channel.SendApdu(View(encodedGetRequest)));
  ASSERT_EQ(1u, stream.Writes().size());
  stream.ScriptRead(stream.Writes()[0]);

  std::vector<std::uint8_t> receivedApdu;
  ASSERT_EQ(ProfileStatus::Ok, channel.ReceiveApdu(receivedApdu));

  XdlmsApdu decoded;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeXdlmsApdu(&receivedApdu[0], receivedApdu.size(), decoded));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetRequest, decoded.kind);
}

TEST(ProfileIntegration, GetRequestSurvivesWrapperUdpProfileChannel)
{
  FakeDatagramTransport datagram;
  WrapperUdpProfileChannel channel(datagram, DefaultApduChannelOptions());
  const std::vector<std::uint8_t> encodedGetRequest = EncodeGetRequest();

  ASSERT_EQ(ProfileStatus::Ok, channel.Open());
  ASSERT_EQ(ProfileStatus::Ok, channel.SendApdu(View(encodedGetRequest)));
  ASSERT_EQ(1u, datagram.SentDatagrams().size());
  datagram.ScriptReceive(datagram.SentDatagrams()[0]);

  std::vector<std::uint8_t> receivedApdu;
  ASSERT_EQ(ProfileStatus::Ok, channel.ReceiveApdu(receivedApdu));

  XdlmsApdu decoded;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeXdlmsApdu(&receivedApdu[0], receivedApdu.size(), decoded));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetRequest, decoded.kind);
}

TEST(ProfileIntegration, AarqSurvivesHdlcLlcProfileChannel)
{
  FakeByteStream stream;
  HdlcProfileChannel channel(stream, DefaultApduChannelOptions());
  const std::vector<std::uint8_t> encodedAarq = EncodeAarq();

  ASSERT_EQ(ProfileStatus::Ok, channel.Open());
  ASSERT_EQ(ProfileStatus::Ok, channel.SendApdu(View(encodedAarq)));
  ASSERT_EQ(1u, stream.Writes().size());
  stream.ScriptRead(stream.Writes()[0]);

  std::vector<std::uint8_t> receivedApdu;
  ASSERT_EQ(ProfileStatus::Ok, channel.ReceiveApdu(receivedApdu));

  dlms::apdu::AcseApdu decoded;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeAcseApdu(&receivedApdu[0], receivedApdu.size(), decoded));
  EXPECT_EQ(dlms::apdu::AcseApduKind::Aarq, decoded.kind);
}

TEST(ProfileIntegration, AarqSurvivesSessionHdlcLlcProfileChannel)
{
  FakeByteStream stream;
  ApduChannelOptions options = DefaultApduChannelOptions();
  options.hdlcUseSession = true;
  options.hdlcRole = HdlcProfileRole::Server;
  options.hdlcMaxInformationFieldLengthTransmit = 24u;
  options.hdlcMaxInformationFieldLengthReceive = 24u;
  HdlcProfileChannel channel(stream, options);

  dlms::hdlc::HdlcSession client(
    MakeHdlcSessionOptions(dlms::hdlc::HdlcSessionRole::Client));
  std::vector<std::uint8_t> snrm;
  ASSERT_EQ(dlms::hdlc::HdlcStatus::Ok, client.BuildConnectRequest(snrm));

  ASSERT_EQ(ProfileStatus::Ok, channel.Open());
  stream.ScriptRead(snrm);
  ASSERT_EQ(ProfileStatus::Ok, channel.AcceptDataLink());
  ASSERT_EQ(1u, stream.Writes().size());

  dlms::hdlc::HdlcFrameBuffer ua;
  ASSERT_EQ(dlms::hdlc::HdlcStatus::Ok,
            dlms::hdlc::DecodeFrame(&stream.Writes()[0][0],
                                    stream.Writes()[0].size(),
                                    dlms::hdlc::DefaultHdlcCodecLimits(),
                                    ua));
  ASSERT_EQ(dlms::hdlc::HdlcStatus::Ok, client.ReceiveFrame(ua));

  const std::vector<std::uint8_t> encodedAarq = EncodeAarq();
  std::vector<std::uint8_t> lpdu;
  ASSERT_EQ(dlms::llc::LlcStatus::Ok,
            dlms::llc::EncodeLpdu(
              dlms::llc::MakeLlcHeader(dlms::llc::LlcDirection::ClientToServer),
              &encodedAarq[0],
              encodedAarq.size(),
              lpdu));

  std::vector<std::uint8_t> baseBytes;
  ASSERT_EQ(dlms::hdlc::HdlcStatus::Ok,
            client.BuildInformationFrame(0, 0u, true, baseBytes));
  dlms::hdlc::HdlcFrameBuffer baseBuffer;
  ASSERT_EQ(dlms::hdlc::HdlcStatus::Ok,
            dlms::hdlc::DecodeFrame(&baseBytes[0],
                                    baseBytes.size(),
                                    dlms::hdlc::DefaultHdlcCodecLimits(),
                                    baseBuffer));

  stream.ScriptRead(EncodeSegmentedInformationFrame(baseBuffer, lpdu, 0u));

  std::vector<std::uint8_t> receivedApdu;
  ASSERT_EQ(ProfileStatus::Ok, channel.ReceiveApdu(receivedApdu));

  dlms::apdu::AcseApdu decoded;
  ASSERT_EQ(ApduStatus::Ok,
            DecodeAcseApdu(&receivedApdu[0], receivedApdu.size(), decoded));
  EXPECT_EQ(dlms::apdu::AcseApduKind::Aarq, decoded.kind);
}

} // namespace
