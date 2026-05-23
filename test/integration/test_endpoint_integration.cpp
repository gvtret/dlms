#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace {

class FakeApduChannel : public dlms::profile::IApduChannel
{
public:
  FakeApduChannel()
    : open(false)
    , receiveCalls(0u)
    , sendCalls(0u)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    open = true;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    open = false;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return open;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++sendCalls;
    sent.assign(apdu.data, apdu.data + apdu.size);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    ++receiveCalls;
    apdu = nextReceive;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    ++receiveCalls;
    if (output.size < nextReceive.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t i = 0u; i < nextReceive.size(); ++i) {
      output.data[i] = nextReceive[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = nextReceive.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  bool open;
  std::size_t receiveCalls;
  std::size_t sendCalls;
  std::vector<std::uint8_t> nextReceive;
  std::vector<std::uint8_t> sent;
};

class RecordingPushHandler : public dlms::endpoint::IPushIndicationHandler
{
public:
  RecordingPushHandler()
    : calls(0u)
  {
  }

  dlms::endpoint::EndpointStatus OnPushApdu(
    const std::vector<std::uint8_t>& apdu)
  {
    ++calls;
    lastApdu = apdu;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  std::size_t calls;
  std::vector<std::uint8_t> lastApdu;
};

class FakeGatewayUpstream : public dlms::endpoint::IGatewayUpstream
{
public:
  FakeGatewayUpstream()
    : open(false)
    , getCalls(0u)
  {
  }

  dlms::endpoint::EndpointStatus Open()
  {
    open = true;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus Close()
  {
    open = false;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  bool IsOpen() const
  {
    return open;
  }

  dlms::endpoint::EndpointStatus Get(
    const dlms::endpoint::ClientAttributeDescriptor& descriptor,
    std::vector<std::uint8_t>& encodedData)
  {
    ++getCalls;
    lastGetDescriptor = descriptor;
    encodedData = getData;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus Set(
    const dlms::endpoint::ClientAttributeDescriptor&,
    const std::vector<std::uint8_t>&)
  {
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus Action(
    const dlms::endpoint::ClientMethodDescriptor&,
    bool,
    const std::vector<std::uint8_t>&,
    std::vector<std::uint8_t>& encodedReturnParameter)
  {
    encodedReturnParameter.clear();
    return dlms::endpoint::EndpointStatus::Ok;
  }

  bool open;
  std::size_t getCalls;
  dlms::endpoint::ClientAttributeDescriptor lastGetDescriptor;
  std::vector<std::uint8_t> getData;
};

class AllowAllPolicy : public dlms::endpoint::IGatewayPolicy
{
public:
  bool AllowGet(const dlms::endpoint::ClientAttributeDescriptor&) const
  {
    return true;
  }

  bool AllowSet(const dlms::endpoint::ClientAttributeDescriptor&) const
  {
    return true;
  }

  bool AllowAction(const dlms::endpoint::ClientMethodDescriptor&) const
  {
    return true;
  }
};

std::vector<std::uint8_t> EncodeLongUnsigned(std::uint16_t value)
{
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;

  std::uint8_t buffer[16] = {};
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeDlmsData(data, writer));
  return std::vector<std::uint8_t>(buffer, buffer + writer.WrittenSize());
}

std::vector<std::uint8_t> MakeGetRequest(std::uint8_t invokeIdAndPriority)
{
  const dlms::apdu::XdlmsApdu request =
    dlms::apdu::MakeGetRequestNormal(
      invokeIdAndPriority,
      3u,
      dlms::apdu::LogicalName(1, 0, 1, 8, 0, 255),
      2u);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

dlms::apdu::XdlmsApdu DecodeResponse(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::apdu::XdlmsApdu response;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              bytes.empty() ? 0 : &bytes[0],
              bytes.size(),
              response));
  return response;
}

} // namespace

TEST(EndpointIntegration, ServerEndpointServesCosemGetThroughProfileChannel)
{
  FakeApduChannel channel;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(
    dlms::cosem::CosemStatus::Ok,
    logicalDevice.RegisterObject(
      std::shared_ptr<dlms::cosem::CosemRegisterObject>(
        new dlms::cosem::CosemRegisterObject(
          dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
          EncodeLongUnsigned(0x1234u),
          dlms::cosem::CosemByteBuffer(),
          dlms::cosem::AttributeAccessMode::ReadOnly))));

  dlms::endpoint::ServerEndpoint endpoint(channel, logicalDevice);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive = MakeGetRequest(0x85u);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(1u, channel.sendCalls);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(channel.sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.getResponseAny.result.data.type);
  EXPECT_EQ(0x1234u, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, PushListenerEndpointForwardsRawPushApdu)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive.push_back(0x0fu);
  channel.nextReceive.push_back(0x01u);
  channel.nextReceive.push_back(0x02u);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(channel.nextReceive, handler.lastApdu);
}

TEST(EndpointIntegration, GatewayEndpointForwardsGetToInjectedUpstream)
{
  FakeApduChannel downstream;
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(downstream, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  upstream.getData = EncodeLongUnsigned(0x4321u);
  downstream.nextReceive = MakeGetRequest(0x86u);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, downstream.receiveCalls);
  EXPECT_EQ(1u, downstream.sendCalls);
  EXPECT_EQ(1u, upstream.getCalls);
  EXPECT_EQ(3u, upstream.lastGetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastGetDescriptor.attributeId);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(downstream.sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x4321u, response.getResponseAny.result.data.unsignedValue);
}
