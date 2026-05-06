#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/profile/hdlc_profile_channel.hpp"
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
using dlms::profile::HdlcProfileChannel;
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

} // namespace

