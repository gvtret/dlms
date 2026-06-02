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

struct ChannelState
{
  ChannelState()
    : open(false)
    , openCalls(0u)
    , closeCalls(0u)
    , receiveCalls(0u)
    , sendCalls(0u)
  {
  }

  bool open;
  std::size_t openCalls;
  std::size_t closeCalls;
  std::size_t receiveCalls;
  std::size_t sendCalls;
  std::vector<std::uint8_t> nextReceive;
  std::vector<std::uint8_t> sent;
};

class FakeApduChannel : public dlms::profile::IApduChannel
{
public:
  explicit FakeApduChannel(const std::shared_ptr<ChannelState>& state)
    : state_(state)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    state_->open = true;
    ++state_->openCalls;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    state_->open = false;
    ++state_->closeCalls;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return state_->open;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++state_->sendCalls;
    state_->sent.assign(apdu.data, apdu.data + apdu.size);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    ++state_->receiveCalls;
    apdu = state_->nextReceive;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    ++state_->receiveCalls;
    if (output.size < state_->nextReceive.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t i = 0u; i < state_->nextReceive.size(); ++i) {
      output.data[i] = state_->nextReceive[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = state_->nextReceive.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

private:
  std::shared_ptr<ChannelState> state_;
};

class FakeApduChannelListener
  : public dlms::endpoint::IApduChannelListener
{
public:
  FakeApduChannelListener()
    : openStatus(dlms::endpoint::EndpointStatus::Ok)
    , closeStatus(dlms::endpoint::EndpointStatus::Ok)
    , acceptStatus(dlms::endpoint::EndpointStatus::Ok)
    , open(false)
    , openCalls(0u)
    , closeCalls(0u)
    , acceptCalls(0u)
  {
  }

  dlms::endpoint::EndpointStatus Open()
  {
    ++openCalls;
    open = openStatus == dlms::endpoint::EndpointStatus::Ok;
    return openStatus;
  }

  dlms::endpoint::EndpointStatus Close()
  {
    ++closeCalls;
    open = false;
    return closeStatus;
  }

  bool IsOpen() const
  {
    return open;
  }

  std::uint16_t LocalPort() const
  {
    return 0u;
  }

  dlms::endpoint::EndpointStatus Accept(
    std::unique_ptr<dlms::profile::IApduChannel>& channel)
  {
    ++acceptCalls;
    if (acceptStatus != dlms::endpoint::EndpointStatus::Ok) {
      return acceptStatus;
    }
    if (!nextChannel) {
      channel.reset();
      return dlms::endpoint::EndpointStatus::Ok;
    }
    channel = std::move(nextChannel);
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus openStatus;
  dlms::endpoint::EndpointStatus closeStatus;
  dlms::endpoint::EndpointStatus acceptStatus;
  bool open;
  std::size_t openCalls;
  std::size_t closeCalls;
  std::size_t acceptCalls;
  std::unique_ptr<dlms::profile::IApduChannel> nextChannel;
};

class RecordingPushHandler
  : public dlms::endpoint::IPushIndicationHandler
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

class FakeServerService : public dlms::server::IServerService
{
public:
  FakeServerService()
    : getCalls(0u)
  {
  }

  dlms::server::ServerGetResponse HandleGet(
    const dlms::server::ServerGetRequest& request)
  {
    ++getCalls;
    lastGetRequest = request;

    dlms::cosem::CosemByteBuffer data;
    data.push_back(0x12u);
    data.push_back(0x22u);
    data.push_back(0x22u);
    return dlms::server::MakeServerGetDataResponse(
      request.invokeId,
      data);
  }

  dlms::server::ServerSetResponse HandleSet(
    const dlms::server::ServerSetRequest& request)
  {
    return dlms::server::MakeServerSetResponse(
      request.invokeId,
      dlms::server::ServerStatus::Ok);
  }

  dlms::server::ServerActionResponse HandleAction(
    const dlms::server::ServerActionRequest& request)
  {
    return dlms::server::MakeServerActionDataResponse(
      request.invokeId,
      dlms::cosem::CosemByteBuffer());
  }

  std::size_t getCalls;
  dlms::server::ServerGetRequest lastGetRequest;
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

std::unique_ptr<dlms::profile::IApduChannel> MakeChannel(
  const std::shared_ptr<ChannelState>& state)
{
  return std::unique_ptr<dlms::profile::IApduChannel>(
    new FakeApduChannel(state));
}

} // namespace

TEST(ListenerRuntime, ServerRuntimeOpensAndClosesListener)
{
  FakeApduChannelListener listener;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerListenerRuntime runtime(listener, logicalDevice);

  EXPECT_FALSE(runtime.IsOpen());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());
  EXPECT_TRUE(runtime.IsOpen());
  EXPECT_TRUE(listener.IsOpen());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Close());
  EXPECT_FALSE(runtime.IsOpen());
  EXPECT_FALSE(listener.IsOpen());
}

TEST(ListenerRuntime, RuntimeOpenIsIdempotentWhenOpen)
{
  FakeApduChannelListener serverListener;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerListenerRuntime serverRuntime(
    serverListener,
    logicalDevice);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, serverRuntime.Open());
  serverListener.openStatus = dlms::endpoint::EndpointStatus::TransportFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, serverRuntime.Open());
  EXPECT_EQ(1u, serverListener.openCalls);

  FakeApduChannelListener pushListener;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerRuntime pushRuntime(pushListener, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, pushRuntime.Open());
  pushListener.openStatus = dlms::endpoint::EndpointStatus::TransportFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, pushRuntime.Open());
  EXPECT_EQ(1u, pushListener.openCalls);

  FakeApduChannelListener gatewayListener;
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  dlms::endpoint::GatewayListenerRuntime gatewayRuntime(
    gatewayListener,
    upstream,
    policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, gatewayRuntime.Open());
  gatewayListener.openStatus =
    dlms::endpoint::EndpointStatus::TransportFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, gatewayRuntime.Open());
  EXPECT_EQ(1u, gatewayListener.openCalls);
}

TEST(ListenerRuntime, RuntimeCloseIsIdempotentWhenClosed)
{
  FakeApduChannelListener serverListener;
  serverListener.closeStatus = dlms::endpoint::EndpointStatus::TransportFailed;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerListenerRuntime serverRuntime(
    serverListener,
    logicalDevice);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, serverRuntime.Close());
  EXPECT_EQ(0u, serverListener.closeCalls);

  FakeApduChannelListener pushListener;
  pushListener.closeStatus = dlms::endpoint::EndpointStatus::TransportFailed;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerRuntime pushRuntime(pushListener, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, pushRuntime.Close());
  EXPECT_EQ(0u, pushListener.closeCalls);

  FakeApduChannelListener gatewayListener;
  gatewayListener.closeStatus =
    dlms::endpoint::EndpointStatus::TransportFailed;
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  dlms::endpoint::GatewayListenerRuntime gatewayRuntime(
    gatewayListener,
    upstream,
    policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, gatewayRuntime.Close());
  EXPECT_EQ(0u, gatewayListener.closeCalls);
}

TEST(ListenerRuntime, ServerRuntimeRequiresOpenAndMapsAcceptFailure)
{
  FakeApduChannelListener listener;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerListenerRuntime runtime(listener, logicalDevice);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            runtime.RunOnce());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());
  listener.acceptStatus = dlms::endpoint::EndpointStatus::TransportFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::TransportFailed,
            runtime.RunOnce());
  EXPECT_EQ(1u, listener.acceptCalls);
}

TEST(ListenerRuntime, ServerRuntimeRejectsNullAcceptedChannel)
{
  FakeApduChannelListener listener;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerListenerRuntime runtime(listener, logicalDevice);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InternalError,
            runtime.RunOnce());
}

TEST(ListenerRuntime, ServerRuntimeAcceptsOneChannelAndRunsServerEndpoint)
{
  FakeApduChannelListener listener;
  const std::shared_ptr<ChannelState> channelState(new ChannelState());
  channelState->nextReceive = MakeGetRequest(0x85u);
  listener.nextChannel = MakeChannel(channelState);

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

  dlms::endpoint::ServerListenerRuntime runtime(listener, logicalDevice);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.RunOnce());
  EXPECT_EQ(1u, listener.acceptCalls);
  EXPECT_EQ(1u, channelState->openCalls);
  EXPECT_EQ(1u, channelState->receiveCalls);
  EXPECT_EQ(1u, channelState->sendCalls);
  EXPECT_EQ(1u, channelState->closeCalls);
  EXPECT_FALSE(channelState->open);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(channelState->sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(0x1234u, response.getResponseAny.result.data.unsignedValue);
}

TEST(ListenerRuntime, ServerRuntimeCanUseInjectedServerService)
{
  FakeApduChannelListener listener;
  const std::shared_ptr<ChannelState> channelState(new ChannelState());
  channelState->nextReceive = MakeGetRequest(0x85u);
  listener.nextChannel = MakeChannel(channelState);

  FakeServerService server;
  dlms::endpoint::ServerListenerRuntime runtime(listener, server);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.RunOnce());
  EXPECT_EQ(1u, server.getCalls);
  EXPECT_EQ(5u, server.lastGetRequest.invokeId);
  EXPECT_EQ(1u, listener.acceptCalls);
  EXPECT_EQ(1u, channelState->openCalls);
  EXPECT_EQ(1u, channelState->receiveCalls);
  EXPECT_EQ(1u, channelState->sendCalls);
  EXPECT_EQ(1u, channelState->closeCalls);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(channelState->sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(0x2222u, response.getResponseAny.result.data.unsignedValue);
}

TEST(ListenerRuntime, PushRuntimeAcceptsOneChannelAndDispatchesPushApdu)
{
  FakeApduChannelListener listener;
  const std::shared_ptr<ChannelState> channelState(new ChannelState());
  channelState->nextReceive.push_back(0x0fu);
  channelState->nextReceive.push_back(0x01u);
  listener.nextChannel = MakeChannel(channelState);

  RecordingPushHandler handler;
  dlms::endpoint::PushListenerRuntime runtime(listener, handler);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.RunOnce());
  EXPECT_EQ(1u, listener.acceptCalls);
  EXPECT_EQ(1u, channelState->openCalls);
  EXPECT_EQ(1u, channelState->receiveCalls);
  EXPECT_EQ(1u, channelState->closeCalls);
  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(channelState->nextReceive, handler.lastApdu);
}

TEST(ListenerRuntime, GatewayRuntimeAcceptsOneChannelAndForwardsGet)
{
  FakeApduChannelListener listener;
  const std::shared_ptr<ChannelState> channelState(new ChannelState());
  channelState->nextReceive = MakeGetRequest(0x86u);
  listener.nextChannel = MakeChannel(channelState);

  FakeGatewayUpstream upstream;
  upstream.getData = EncodeLongUnsigned(0x4321u);
  AllowAllPolicy policy;
  dlms::endpoint::GatewayListenerRuntime runtime(listener, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.RunOnce());
  EXPECT_EQ(1u, listener.acceptCalls);
  EXPECT_EQ(1u, upstream.getCalls);
  EXPECT_EQ(1u, channelState->openCalls);
  EXPECT_EQ(1u, channelState->receiveCalls);
  EXPECT_EQ(1u, channelState->sendCalls);
  EXPECT_EQ(1u, channelState->closeCalls);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(channelState->sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(0x4321u, response.getResponseAny.result.data.unsignedValue);
}
