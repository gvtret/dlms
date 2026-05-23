#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/action.hpp"
#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/initiate.hpp"
#include "dlms/apdu/set.hpp"
#include "dlms/apdu/xdlms.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <thread>
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
    , setCalls(0u)
    , actionCalls(0u)
    , lastActionHasParameter(false)
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
    const dlms::endpoint::ClientAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData)
  {
    ++setCalls;
    lastSetDescriptor = descriptor;
    lastSetData = encodedData;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus Action(
    const dlms::endpoint::ClientMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    std::vector<std::uint8_t>& encodedReturnParameter)
  {
    ++actionCalls;
    lastActionDescriptor = descriptor;
    lastActionHasParameter = hasParameter;
    lastActionParameter = encodedParameter;
    encodedReturnParameter = actionReturnData;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  bool open;
  std::size_t getCalls;
  std::size_t setCalls;
  std::size_t actionCalls;
  dlms::endpoint::ClientAttributeDescriptor lastGetDescriptor;
  dlms::endpoint::ClientAttributeDescriptor lastSetDescriptor;
  dlms::endpoint::ClientMethodDescriptor lastActionDescriptor;
  bool lastActionHasParameter;
  std::vector<std::uint8_t> getData;
  std::vector<std::uint8_t> lastSetData;
  std::vector<std::uint8_t> lastActionParameter;
  std::vector<std::uint8_t> actionReturnData;
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

class IntegrationDataObject : public dlms::cosem::ICosemObject
{
public:
  explicit IntegrationDataObject(
    const dlms::cosem::CosemByteBuffer& value,
    dlms::cosem::AttributeAccessMode attributeAccess =
      dlms::cosem::AttributeAccessMode::ReadOnly)
    : value_(value)
    , invokeCount_(0u)
    , lastInvokeMethodId_(0u)
  {
    descriptor_.key.classId = 3u;
    descriptor_.key.version = 0u;
    descriptor_.key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255);
    rights_.SetAttributeAccess(2u, attributeAccess);
    rights_.SetMethodAccess(1u, dlms::cosem::MethodAccessMode::Access);
    actionData_.push_back(0x12u);
    actionData_.push_back(0x24u);
    actionData_.push_back(0x68u);
  }

  dlms::cosem::CosemObjectDescriptor Descriptor() const
  {
    return descriptor_;
  }

  dlms::cosem::CosemAccessRights AccessRights() const
  {
    return rights_;
  }

  dlms::cosem::CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    dlms::cosem::CosemByteBuffer& output) const
  {
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }
    output = value_;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const dlms::cosem::CosemByteBuffer& input)
  {
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }
    value_ = input;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const dlms::cosem::CosemByteBuffer& input,
    dlms::cosem::CosemByteBuffer& output)
  {
    ++invokeCount_;
    lastInvokeMethodId_ = methodId;
    lastInvokeParameter_ = input;
    if (methodId != 1u) {
      return dlms::cosem::CosemStatus::MethodNotFound;
    }
    output = actionData_;
    return dlms::cosem::CosemStatus::Ok;
  }

  std::size_t InvokeCount() const
  {
    return invokeCount_;
  }

  std::uint8_t LastInvokeMethodId() const
  {
    return lastInvokeMethodId_;
  }

  dlms::cosem::CosemByteBuffer LastInvokeParameter() const
  {
    return lastInvokeParameter_;
  }

private:
  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  dlms::cosem::CosemByteBuffer value_;
  dlms::cosem::CosemByteBuffer actionData_;
  std::size_t invokeCount_;
  std::uint8_t lastInvokeMethodId_;
  dlms::cosem::CosemByteBuffer lastInvokeParameter_;
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

dlms::apdu::DlmsData MakeLongUnsignedData(std::uint16_t value)
{
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;
  return data;
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

std::vector<std::uint8_t> MakeAarq()
{
  const dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::XdlmsApdu xdlms(request);
  const dlms::apdu::AcseApdu aarq =
    dlms::apdu::MakeAarqWithInitiateRequest(xdlms);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(aarq, output));
  return output;
}

std::vector<std::uint8_t> MakeSetRequest(
  std::uint8_t invokeIdAndPriority,
  const dlms::apdu::DlmsData& value)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::SetRequest;
  request.setRequestAny.choice = dlms::apdu::SetRequestChoice::Normal;
  request.setRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.setRequestAny.normal.descriptor.classId = 3u;
  request.setRequestAny.normal.descriptor.logicalName[0] = 1u;
  request.setRequestAny.normal.descriptor.logicalName[1] = 0u;
  request.setRequestAny.normal.descriptor.logicalName[2] = 1u;
  request.setRequestAny.normal.descriptor.logicalName[3] = 8u;
  request.setRequestAny.normal.descriptor.logicalName[4] = 0u;
  request.setRequestAny.normal.descriptor.logicalName[5] = 255u;
  request.setRequestAny.normal.descriptor.attributeId = 2u;
  request.setRequestAny.normal.hasSelection = false;
  request.setRequestAny.data = value;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

std::vector<std::uint8_t> MakeActionRequest(
  std::uint8_t invokeIdAndPriority,
  std::uint8_t methodId,
  const dlms::apdu::DlmsData& parameter)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::ActionRequest;
  request.actionRequestAny.choice = dlms::apdu::ActionRequestChoice::Normal;
  request.actionRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.actionRequestAny.normal.descriptor.classId = 3u;
  request.actionRequestAny.normal.descriptor.logicalName[0] = 1u;
  request.actionRequestAny.normal.descriptor.logicalName[1] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[2] = 1u;
  request.actionRequestAny.normal.descriptor.logicalName[3] = 8u;
  request.actionRequestAny.normal.descriptor.logicalName[4] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[5] = 255u;
  request.actionRequestAny.normal.descriptor.methodId = methodId;
  request.actionRequestAny.normal.hasInvocationParameter = true;
  request.actionRequestAny.normal.invocationParameter = parameter;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

dlms::endpoint::EndpointTransportOptions TcpListenerOptions()
{
  dlms::endpoint::EndpointTransportOptions options =
    dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Tcp;
  options.host = "127.0.0.1";
  options.port = 0u;
  options.timeoutMs = 1000u;
  return options;
}

dlms::endpoint::EndpointTransportOptions TcpClientOptions(std::uint16_t port)
{
  dlms::endpoint::EndpointTransportOptions options =
    dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Tcp;
  options.host = "127.0.0.1";
  options.port = port;
  options.timeoutMs = 1000u;
  return options;
}

dlms::endpoint::EndpointTransportOptions UdpListenerOptions()
{
  dlms::endpoint::EndpointTransportOptions options =
    dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Udp;
  options.host = "127.0.0.1";
  options.port = 0u;
  options.timeoutMs = 1000u;
  return options;
}

dlms::endpoint::EndpointTransportOptions UdpClientOptions(std::uint16_t port)
{
  dlms::endpoint::EndpointTransportOptions options =
    dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Udp;
  options.host = "127.0.0.1";
  options.port = port;
  options.timeoutMs = 1000u;
  return options;
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

void RunOneTcpListenerExchange(
  dlms::cosem::LogicalDevice& logicalDevice,
  const std::vector<std::uint8_t>& request,
  std::vector<std::uint8_t>& responseBytes,
  dlms::endpoint::EndpointProfileKind profileKind =
    dlms::endpoint::EndpointProfileKind::Wrapper,
  bool hdlcUseSession = false,
  bool negotiateAssociation = false)
{
  responseBytes.clear();

  dlms::endpoint::EndpointProfileOptions profile =
    dlms::endpoint::DefaultEndpointProfileOptions();
  profile.kind = profileKind;
  profile.hdlcUseSession = hdlcUseSession;

  dlms::endpoint::EndpointListenerBundle listenerBundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(
              TcpListenerOptions(),
              profile,
              listenerBundle));
  ASSERT_TRUE(listenerBundle.tcp.get() != 0);

  dlms::endpoint::ServerEndpointOptions serverOptions =
    dlms::endpoint::DefaultServerEndpointOptions();
  serverOptions.transport = TcpListenerOptions();
  serverOptions.profile = profile;
  serverOptions.negotiateAssociation = negotiateAssociation;

  dlms::endpoint::ServerListenerRuntime runtime(
    *listenerBundle.Listener(),
    serverOptions,
    logicalDevice);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());
  ASSERT_NE(0u, listenerBundle.tcp->LocalPort());

  dlms::endpoint::EndpointTransportBundle clientTransport;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              TcpClientOptions(listenerBundle.tcp->LocalPort()),
              clientTransport));
  dlms::endpoint::EndpointProfileBundle clientProfile;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              profile,
              clientTransport,
              clientProfile));
  ASSERT_TRUE(clientProfile.Channel() != 0);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Open());

  dlms::endpoint::EndpointStatus serverStatus =
    dlms::endpoint::EndpointStatus::InternalError;
  std::thread serverThread([&runtime, &serverStatus]() {
    serverStatus = runtime.RunOnce();
  });

  if (hdlcUseSession) {
    ASSERT_TRUE(clientProfile.hdlc.get() != 0);
    ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
              clientProfile.hdlc->ConnectDataLink());
  }

  if (negotiateAssociation) {
    const std::vector<std::uint8_t> aarq = MakeAarq();
    dlms::profile::ProfileByteView aarqView;
    aarqView.data = aarq.empty() ? 0 : &aarq[0];
    aarqView.size = aarq.size();
    ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
              clientProfile.Channel()->SendApdu(aarqView));

    std::vector<std::uint8_t> aareBytes;
    ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
              clientProfile.Channel()->ReceiveApdu(aareBytes));
    dlms::apdu::AcseApdu aare = {};
    ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
              dlms::apdu::DecodeAcseApdu(
                aareBytes.empty() ? 0 : &aareBytes[0],
                aareBytes.size(),
                aare));
    ASSERT_EQ(dlms::apdu::AcseApduKind::Aare, aare.kind);
    EXPECT_TRUE(aare.aare.hasResult);
    EXPECT_EQ(0, aare.aare.result);
  }

  dlms::profile::ProfileByteView requestView;
  requestView.data = request.empty() ? 0 : &request[0];
  requestView.size = request.size();
  const dlms::profile::ProfileStatus sendStatus =
    clientProfile.Channel()->SendApdu(requestView);

  const dlms::profile::ProfileStatus receiveStatus =
    clientProfile.Channel()->ReceiveApdu(responseBytes);

  serverThread.join();
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, sendStatus);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, receiveStatus);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, serverStatus);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Close());
}

void RunOneTcpPushListenerExchange(
  const std::vector<std::uint8_t>& pushApdu,
  RecordingPushHandler& handler,
  dlms::endpoint::EndpointProfileKind profileKind =
    dlms::endpoint::EndpointProfileKind::Wrapper,
  bool hdlcUseSession = false)
{
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.transport = TcpListenerOptions();
  options.profile.kind = profileKind;
  options.profile.hdlcUseSession = hdlcUseSession;

  dlms::endpoint::EndpointListenerBundle listenerBundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(options, listenerBundle));
  ASSERT_TRUE(listenerBundle.tcp.get() != 0);

  dlms::endpoint::PushListenerRuntime runtime(
    *listenerBundle.Listener(),
    options,
    handler);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());
  ASSERT_NE(0u, listenerBundle.tcp->LocalPort());

  dlms::endpoint::EndpointTransportBundle clientTransport;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              TcpClientOptions(listenerBundle.tcp->LocalPort()),
              clientTransport));
  dlms::endpoint::EndpointProfileBundle clientProfile;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              options.profile,
              clientTransport,
              clientProfile));
  ASSERT_TRUE(clientProfile.Channel() != 0);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Open());

  dlms::endpoint::EndpointStatus runtimeStatus =
    dlms::endpoint::EndpointStatus::InternalError;
  std::thread runtimeThread([&runtime, &runtimeStatus]() {
    runtimeStatus = runtime.RunOnce();
  });

  if (hdlcUseSession) {
    ASSERT_TRUE(clientProfile.hdlc.get() != 0);
    ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
              clientProfile.hdlc->ConnectDataLink());
  }

  dlms::profile::ProfileByteView pushView;
  pushView.data = pushApdu.empty() ? 0 : &pushApdu[0];
  pushView.size = pushApdu.size();
  const dlms::profile::ProfileStatus sendStatus =
    clientProfile.Channel()->SendApdu(pushView);

  runtimeThread.join();
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, sendStatus);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtimeStatus);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Close());
}

void RunOneUdpPushListenerExchange(
  const std::vector<std::uint8_t>& pushApdu,
  RecordingPushHandler& handler)
{
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.transport = UdpListenerOptions();
  options.profile.kind = dlms::endpoint::EndpointProfileKind::Wrapper;

  dlms::endpoint::EndpointListenerBundle listenerBundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(options, listenerBundle));
  ASSERT_TRUE(listenerBundle.udpPush.get() != 0);

  dlms::endpoint::PushListenerRuntime runtime(
    *listenerBundle.Listener(),
    options,
    handler);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());
  ASSERT_NE(0u, listenerBundle.udpPush->LocalPort());

  dlms::endpoint::EndpointTransportBundle clientTransport;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              UdpClientOptions(listenerBundle.udpPush->LocalPort()),
              clientTransport));
  dlms::endpoint::EndpointProfileBundle clientProfile;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              options.profile,
              clientTransport,
              clientProfile));
  ASSERT_TRUE(clientProfile.Channel() != 0);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Open());

  dlms::endpoint::EndpointStatus runtimeStatus =
    dlms::endpoint::EndpointStatus::InternalError;
  std::thread runtimeThread([&runtime, &runtimeStatus]() {
    runtimeStatus = runtime.RunOnce();
  });

  dlms::profile::ProfileByteView pushView;
  pushView.data = pushApdu.empty() ? 0 : &pushApdu[0];
  pushView.size = pushApdu.size();
  const dlms::profile::ProfileStatus sendStatus =
    clientProfile.Channel()->SendApdu(pushView);

  runtimeThread.join();
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, sendStatus);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtimeStatus);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Close());
}

void RunOneTcpGatewayListenerExchange(
  const std::vector<std::uint8_t>& request,
  FakeGatewayUpstream& upstream,
  AllowAllPolicy& policy,
  std::vector<std::uint8_t>& responseBytes,
  dlms::endpoint::EndpointProfileKind profileKind =
    dlms::endpoint::EndpointProfileKind::Wrapper,
  bool hdlcUseSession = false)
{
  responseBytes.clear();

  dlms::endpoint::GatewayEndpointOptions options =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  options.downstream.transport = TcpListenerOptions();
  options.downstream.profile.kind = profileKind;
  options.downstream.profile.hdlcUseSession = hdlcUseSession;

  dlms::endpoint::EndpointListenerBundle listenerBundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(
              options.downstream,
              listenerBundle));
  ASSERT_TRUE(listenerBundle.tcp.get() != 0);

  dlms::endpoint::GatewayListenerRuntime runtime(
    *listenerBundle.Listener(),
    options,
    upstream,
    policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Open());
  ASSERT_NE(0u, listenerBundle.tcp->LocalPort());

  dlms::endpoint::EndpointTransportBundle clientTransport;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              TcpClientOptions(listenerBundle.tcp->LocalPort()),
              clientTransport));
  dlms::endpoint::EndpointProfileBundle clientProfile;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              options.downstream.profile,
              clientTransport,
              clientProfile));
  ASSERT_TRUE(clientProfile.Channel() != 0);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Open());

  dlms::endpoint::EndpointStatus runtimeStatus =
    dlms::endpoint::EndpointStatus::InternalError;
  std::thread runtimeThread([&runtime, &runtimeStatus]() {
    runtimeStatus = runtime.RunOnce();
  });

  if (hdlcUseSession) {
    ASSERT_TRUE(clientProfile.hdlc.get() != 0);
    ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
              clientProfile.hdlc->ConnectDataLink());
  }

  dlms::profile::ProfileByteView requestView;
  requestView.data = request.empty() ? 0 : &request[0];
  requestView.size = request.size();
  const dlms::profile::ProfileStatus sendStatus =
    clientProfile.Channel()->SendApdu(requestView);

  const dlms::profile::ProfileStatus receiveStatus =
    clientProfile.Channel()->ReceiveApdu(responseBytes);

  runtimeThread.join();
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, sendStatus);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, receiveStatus);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtimeStatus);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, runtime.Close());
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

TEST(EndpointIntegration, TcpListenerRuntimeServesOneWrapperGet)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(
    dlms::cosem::CosemStatus::Ok,
    logicalDevice.RegisterObject(
      std::shared_ptr<dlms::cosem::CosemRegisterObject>(
        new dlms::cosem::CosemRegisterObject(
          dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
          EncodeLongUnsigned(0x5678u),
          dlms::cosem::CosemByteBuffer(),
          dlms::cosem::AttributeAccessMode::ReadOnly))));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeGetRequest(0x87u),
      responseBytes));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x5678u, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, TcpListenerRuntimeNegotiatesWrapperAssociationThenServesGet)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(
    dlms::cosem::CosemStatus::Ok,
    logicalDevice.RegisterObject(
      std::shared_ptr<dlms::cosem::CosemRegisterObject>(
        new dlms::cosem::CosemRegisterObject(
          dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
          EncodeLongUnsigned(0x789au),
          dlms::cosem::CosemByteBuffer(),
          dlms::cosem::AttributeAccessMode::ReadOnly))));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeGetRequest(0x84u),
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Wrapper,
      false,
      true));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x789au, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneHdlcGet)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(
    dlms::cosem::CosemStatus::Ok,
    logicalDevice.RegisterObject(
      std::shared_ptr<dlms::cosem::CosemRegisterObject>(
        new dlms::cosem::CosemRegisterObject(
          dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
          EncodeLongUnsigned(0x6789u),
          dlms::cosem::CosemByteBuffer(),
          dlms::cosem::AttributeAccessMode::ReadOnly))));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeGetRequest(0x8du),
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x6789u, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneHdlcSessionGet)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(
    dlms::cosem::CosemStatus::Ok,
    logicalDevice.RegisterObject(
      std::shared_ptr<dlms::cosem::CosemRegisterObject>(
        new dlms::cosem::CosemRegisterObject(
          dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
          EncodeLongUnsigned(0x2468u),
          dlms::cosem::CosemByteBuffer(),
          dlms::cosem::AttributeAccessMode::ReadOnly))));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeGetRequest(0x81u),
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc,
      true));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x2468u, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneHdlcSessionSet)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(
      EncodeLongUnsigned(0x1234u),
      dlms::cosem::AttributeAccessMode::ReadAndWrite));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeSetRequest(0x82u, MakeLongUnsignedData(0x3579u)),
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc,
      true));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            response.setResponseAny.choice);
  EXPECT_EQ(0x82u, response.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.setResponseAny.result);

  dlms::cosem::CosemByteBuffer stored;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object->ReadAttribute(2u, stored));
  EXPECT_EQ(EncodeLongUnsigned(0x3579u), stored);
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneHdlcSessionAction)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeActionRequest(0x83u, 1u, MakeLongUnsignedData(0x579bu)),
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc,
      true));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, response.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            response.actionResponseAny.choice);
  EXPECT_EQ(0x83u, response.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.actionResponseAny.normal.result);
  EXPECT_TRUE(response.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.actionResponseAny.normal.returnParameter.type);
  EXPECT_EQ(0x2468u,
            response.actionResponseAny.normal.returnParameter.unsignedValue);
  EXPECT_EQ(1u, object->InvokeCount());
  EXPECT_EQ(1u, object->LastInvokeMethodId());
  EXPECT_EQ(EncodeLongUnsigned(0x579bu), object->LastInvokeParameter());
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneHdlcSet)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(
      EncodeLongUnsigned(0x1234u),
      dlms::cosem::AttributeAccessMode::ReadAndWrite));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeSetRequest(0x8eu, MakeLongUnsignedData(0x2468u)),
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            response.setResponseAny.choice);
  EXPECT_EQ(0x8eu, response.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.setResponseAny.result);

  dlms::cosem::CosemByteBuffer stored;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object->ReadAttribute(2u, stored));
  EXPECT_EQ(EncodeLongUnsigned(0x2468u), stored);
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneHdlcAction)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeActionRequest(0x8fu, 1u, MakeLongUnsignedData(0x1357u)),
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, response.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            response.actionResponseAny.choice);
  EXPECT_EQ(0x8fu, response.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.actionResponseAny.normal.result);
  EXPECT_TRUE(response.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.actionResponseAny.normal.returnParameter.type);
  EXPECT_EQ(0x2468u,
            response.actionResponseAny.normal.returnParameter.unsignedValue);
  EXPECT_EQ(1u, object->InvokeCount());
  EXPECT_EQ(1u, object->LastInvokeMethodId());
  EXPECT_EQ(EncodeLongUnsigned(0x1357u), object->LastInvokeParameter());
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneWrapperSet)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(
      EncodeLongUnsigned(0x1234u),
      dlms::cosem::AttributeAccessMode::ReadAndWrite));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeSetRequest(0x88u, MakeLongUnsignedData(0x4321u)),
      responseBytes));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            response.setResponseAny.choice);
  EXPECT_EQ(0x88u, response.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.setResponseAny.result);

  dlms::cosem::CosemByteBuffer stored;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object->ReadAttribute(2u, stored));
  EXPECT_EQ(EncodeLongUnsigned(0x4321u), stored);
}

TEST(EndpointIntegration, TcpListenerRuntimeServesOneWrapperAction)
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpListenerExchange(
      logicalDevice,
      MakeActionRequest(0x89u, 1u, MakeLongUnsignedData(0x4321u)),
      responseBytes));

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, response.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            response.actionResponseAny.choice);
  EXPECT_EQ(0x89u, response.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.actionResponseAny.normal.result);
  EXPECT_TRUE(response.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.actionResponseAny.normal.returnParameter.type);
  EXPECT_EQ(0x2468u,
            response.actionResponseAny.normal.returnParameter.unsignedValue);
  EXPECT_EQ(1u, object->InvokeCount());
  EXPECT_EQ(1u, object->LastInvokeMethodId());
  EXPECT_EQ(EncodeLongUnsigned(0x4321u), object->LastInvokeParameter());
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

TEST(EndpointIntegration, TcpPushListenerRuntimeForwardsOneWrapperApdu)
{
  std::vector<std::uint8_t> pushApdu;
  pushApdu.push_back(0x0fu);
  pushApdu.push_back(0x02u);
  pushApdu.push_back(0x11u);
  pushApdu.push_back(0x22u);

  RecordingPushHandler handler;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpPushListenerExchange(pushApdu, handler));

  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(pushApdu, handler.lastApdu);
}

TEST(EndpointIntegration, TcpPushListenerRuntimeForwardsOneHdlcApdu)
{
  std::vector<std::uint8_t> pushApdu;
  pushApdu.push_back(0x0fu);
  pushApdu.push_back(0x03u);
  pushApdu.push_back(0x33u);
  pushApdu.push_back(0x44u);

  RecordingPushHandler handler;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpPushListenerExchange(
      pushApdu,
      handler,
      dlms::endpoint::EndpointProfileKind::Hdlc));

  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(pushApdu, handler.lastApdu);
}

TEST(EndpointIntegration, TcpPushListenerRuntimeForwardsOneHdlcSessionApdu)
{
  std::vector<std::uint8_t> pushApdu;
  pushApdu.push_back(0x0fu);
  pushApdu.push_back(0x05u);
  pushApdu.push_back(0x77u);
  pushApdu.push_back(0x88u);

  RecordingPushHandler handler;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpPushListenerExchange(
      pushApdu,
      handler,
      dlms::endpoint::EndpointProfileKind::Hdlc,
      true));

  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(pushApdu, handler.lastApdu);
}

TEST(EndpointIntegration, UdpPushListenerRuntimeForwardsOneWrapperApdu)
{
  std::vector<std::uint8_t> pushApdu;
  pushApdu.push_back(0x0fu);
  pushApdu.push_back(0x04u);
  pushApdu.push_back(0x55u);
  pushApdu.push_back(0x66u);

  RecordingPushHandler handler;
  ASSERT_NO_FATAL_FAILURE(
    RunOneUdpPushListenerExchange(pushApdu, handler));

  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(pushApdu, handler.lastApdu);
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

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneWrapperGet)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  upstream.getData = EncodeLongUnsigned(0x2468u);

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeGetRequest(0x8au),
      upstream,
      policy,
      responseBytes));

  EXPECT_EQ(1u, upstream.getCalls);
  EXPECT_EQ(3u, upstream.lastGetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastGetDescriptor.attributeId);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x2468u, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneHdlcGet)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  upstream.getData = EncodeLongUnsigned(0x8642u);

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeGetRequest(0x8du),
      upstream,
      policy,
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc));

  EXPECT_EQ(1u, upstream.getCalls);
  EXPECT_EQ(3u, upstream.lastGetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastGetDescriptor.attributeId);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x8642u, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneHdlcSessionGet)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  upstream.getData = EncodeLongUnsigned(0x1357u);

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeGetRequest(0x84u),
      upstream,
      policy,
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc,
      true));

  EXPECT_EQ(1u, upstream.getCalls);
  EXPECT_EQ(3u, upstream.lastGetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastGetDescriptor.attributeId);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x1357u, response.getResponseAny.result.data.unsignedValue);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneWrapperSet)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeSetRequest(0x8bu, MakeLongUnsignedData(0x1357u)),
      upstream,
      policy,
      responseBytes));

  EXPECT_EQ(1u, upstream.setCalls);
  EXPECT_EQ(3u, upstream.lastSetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastSetDescriptor.attributeId);
  EXPECT_EQ(EncodeLongUnsigned(0x1357u), upstream.lastSetData);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            response.setResponseAny.choice);
  EXPECT_EQ(0x8bu, response.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.setResponseAny.result);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneHdlcSet)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeSetRequest(0x8eu, MakeLongUnsignedData(0x7531u)),
      upstream,
      policy,
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc));

  EXPECT_EQ(1u, upstream.setCalls);
  EXPECT_EQ(3u, upstream.lastSetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastSetDescriptor.attributeId);
  EXPECT_EQ(EncodeLongUnsigned(0x7531u), upstream.lastSetData);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            response.setResponseAny.choice);
  EXPECT_EQ(0x8eu, response.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.setResponseAny.result);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneHdlcSessionSet)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeSetRequest(0x85u, MakeLongUnsignedData(0x468au)),
      upstream,
      policy,
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc,
      true));

  EXPECT_EQ(1u, upstream.setCalls);
  EXPECT_EQ(3u, upstream.lastSetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastSetDescriptor.attributeId);
  EXPECT_EQ(EncodeLongUnsigned(0x468au), upstream.lastSetData);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            response.setResponseAny.choice);
  EXPECT_EQ(0x85u, response.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.setResponseAny.result);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneWrapperAction)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  upstream.actionReturnData = EncodeLongUnsigned(0x9753u);

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeActionRequest(0x8cu, 1u, MakeLongUnsignedData(0x2468u)),
      upstream,
      policy,
      responseBytes));

  EXPECT_EQ(1u, upstream.actionCalls);
  EXPECT_EQ(3u, upstream.lastActionDescriptor.classId);
  EXPECT_EQ(1u, upstream.lastActionDescriptor.methodId);
  EXPECT_TRUE(upstream.lastActionHasParameter);
  EXPECT_EQ(EncodeLongUnsigned(0x2468u), upstream.lastActionParameter);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, response.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            response.actionResponseAny.choice);
  EXPECT_EQ(0x8cu, response.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.actionResponseAny.normal.result);
  EXPECT_TRUE(response.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.actionResponseAny.normal.returnParameter.type);
  EXPECT_EQ(0x9753u,
            response.actionResponseAny.normal.returnParameter.unsignedValue);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneHdlcAction)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  upstream.actionReturnData = EncodeLongUnsigned(0x6420u);

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeActionRequest(0x8fu, 1u, MakeLongUnsignedData(0x7531u)),
      upstream,
      policy,
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc));

  EXPECT_EQ(1u, upstream.actionCalls);
  EXPECT_EQ(3u, upstream.lastActionDescriptor.classId);
  EXPECT_EQ(1u, upstream.lastActionDescriptor.methodId);
  EXPECT_TRUE(upstream.lastActionHasParameter);
  EXPECT_EQ(EncodeLongUnsigned(0x7531u), upstream.lastActionParameter);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, response.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            response.actionResponseAny.choice);
  EXPECT_EQ(0x8fu, response.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.actionResponseAny.normal.result);
  EXPECT_TRUE(response.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.actionResponseAny.normal.returnParameter.type);
  EXPECT_EQ(0x6420u,
            response.actionResponseAny.normal.returnParameter.unsignedValue);
}

TEST(EndpointIntegration, TcpGatewayListenerRuntimeForwardsOneHdlcSessionAction)
{
  FakeGatewayUpstream upstream;
  AllowAllPolicy policy;
  upstream.actionReturnData = EncodeLongUnsigned(0x8642u);

  std::vector<std::uint8_t> responseBytes;
  ASSERT_NO_FATAL_FAILURE(
    RunOneTcpGatewayListenerExchange(
      MakeActionRequest(0x86u, 1u, MakeLongUnsignedData(0x2468u)),
      upstream,
      policy,
      responseBytes,
      dlms::endpoint::EndpointProfileKind::Hdlc,
      true));

  EXPECT_EQ(1u, upstream.actionCalls);
  EXPECT_EQ(3u, upstream.lastActionDescriptor.classId);
  EXPECT_EQ(1u, upstream.lastActionDescriptor.methodId);
  EXPECT_TRUE(upstream.lastActionHasParameter);
  EXPECT_EQ(EncodeLongUnsigned(0x2468u), upstream.lastActionParameter);

  const dlms::apdu::XdlmsApdu response = DecodeResponse(responseBytes);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, response.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            response.actionResponseAny.choice);
  EXPECT_EQ(0x86u, response.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, response.actionResponseAny.normal.result);
  EXPECT_TRUE(response.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.actionResponseAny.normal.returnParameter.type);
  EXPECT_EQ(0x8642u,
            response.actionResponseAny.normal.returnParameter.unsignedValue);
}
