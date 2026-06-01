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

namespace {

class FakeApduChannel : public dlms::profile::IApduChannel
{
public:
  FakeApduChannel()
    : openStatus(dlms::profile::ProfileStatus::Ok)
    , closeStatus(dlms::profile::ProfileStatus::Ok)
    , receiveStatus(dlms::profile::ProfileStatus::Ok)
    , sendStatus(dlms::profile::ProfileStatus::Ok)
    , open(false)
    , receiveCalls(0u)
    , sendCalls(0u)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    open = true;
    return openStatus;
  }

  dlms::profile::ProfileStatus Close()
  {
    open = false;
    return closeStatus;
  }

  bool IsOpen() const
  {
    return open;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++sendCalls;
    if (sendStatus == dlms::profile::ProfileStatus::Ok) {
      sent.assign(apdu.data, apdu.data + apdu.size);
      sentFrames.push_back(sent);
    }
    return sendStatus;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    ++receiveCalls;
    if (receiveStatus == dlms::profile::ProfileStatus::Ok) {
      if (!receiveQueue.empty()) {
        apdu = receiveQueue.front();
        receiveQueue.erase(receiveQueue.begin());
      } else {
        apdu = nextReceive;
      }
    }
    return receiveStatus;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    ++receiveCalls;
    if (receiveStatus != dlms::profile::ProfileStatus::Ok) {
      return receiveStatus;
    }
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

  dlms::profile::ProfileStatus openStatus;
  dlms::profile::ProfileStatus closeStatus;
  dlms::profile::ProfileStatus receiveStatus;
  dlms::profile::ProfileStatus sendStatus;
  bool open;
  std::size_t receiveCalls;
  std::size_t sendCalls;
  std::vector<std::uint8_t> nextReceive;
  std::vector<std::vector<std::uint8_t> > receiveQueue;
  std::vector<std::uint8_t> sent;
  std::vector<std::vector<std::uint8_t> > sentFrames;
};

class FakeGatewayUpstream : public dlms::endpoint::IGatewayUpstream
{
public:
  FakeGatewayUpstream()
    : openStatus(dlms::endpoint::EndpointStatus::Ok)
    , closeStatus(dlms::endpoint::EndpointStatus::Ok)
    , getStatus(dlms::endpoint::EndpointStatus::Ok)
    , setStatus(dlms::endpoint::EndpointStatus::Ok)
    , actionStatus(dlms::endpoint::EndpointStatus::Ok)
    , open(false)
    , getCalls(0u)
    , setCalls(0u)
    , actionCalls(0u)
    , lastHasActionParameter(false)
  {
  }

  dlms::endpoint::EndpointStatus Open()
  {
    open = openStatus == dlms::endpoint::EndpointStatus::Ok;
    return openStatus;
  }

  dlms::endpoint::EndpointStatus Close()
  {
    open = false;
    return closeStatus;
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
    if (getStatus == dlms::endpoint::EndpointStatus::Ok) {
      encodedData = getData;
    }
    return getStatus;
  }

  dlms::endpoint::EndpointStatus Set(
    const dlms::endpoint::ClientAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData)
  {
    ++setCalls;
    lastSetDescriptor = descriptor;
    lastSetData = encodedData;
    return setStatus;
  }

  dlms::endpoint::EndpointStatus Action(
    const dlms::endpoint::ClientMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    std::vector<std::uint8_t>& encodedReturnParameter)
  {
    ++actionCalls;
    lastActionDescriptor = descriptor;
    lastHasActionParameter = hasParameter;
    lastActionParameter = encodedParameter;
    if (actionStatus == dlms::endpoint::EndpointStatus::Ok) {
      encodedReturnParameter = actionReturnData;
    }
    return actionStatus;
  }

  dlms::endpoint::EndpointStatus openStatus;
  dlms::endpoint::EndpointStatus closeStatus;
  dlms::endpoint::EndpointStatus getStatus;
  dlms::endpoint::EndpointStatus setStatus;
  dlms::endpoint::EndpointStatus actionStatus;
  bool open;
  std::size_t getCalls;
  std::size_t setCalls;
  std::size_t actionCalls;
  dlms::endpoint::ClientAttributeDescriptor lastGetDescriptor;
  dlms::endpoint::ClientAttributeDescriptor lastSetDescriptor;
  dlms::endpoint::ClientMethodDescriptor lastActionDescriptor;
  bool lastHasActionParameter;
  std::vector<std::uint8_t> getData;
  std::vector<std::uint8_t> lastSetData;
  std::vector<std::uint8_t> lastActionParameter;
  std::vector<std::uint8_t> actionReturnData;
};

class FakeGatewayPolicy : public dlms::endpoint::IGatewayPolicy
{
public:
  FakeGatewayPolicy()
    : allowGet(true)
    , allowSet(true)
    , allowAction(true)
  {
  }

  bool AllowGet(const dlms::endpoint::ClientAttributeDescriptor&) const
  {
    return allowGet;
  }

  bool AllowSet(const dlms::endpoint::ClientAttributeDescriptor&) const
  {
    return allowSet;
  }

  bool AllowAction(const dlms::endpoint::ClientMethodDescriptor&) const
  {
    return allowAction;
  }

  bool allowGet;
  bool allowSet;
  bool allowAction;
};

dlms::apdu::DlmsData MakeLongUnsignedData(std::uint16_t value)
{
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;
  return data;
}

std::vector<std::uint8_t> EncodeLongUnsigned(std::uint16_t value)
{
  const dlms::apdu::DlmsData data = MakeLongUnsignedData(value);
  std::uint8_t buffer[16] = {};
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeDlmsData(data, writer));
  return std::vector<std::uint8_t>(buffer, buffer + writer.WrittenSize());
}

void FillSetDescriptor(
  dlms::apdu::CosemAttributeDescriptorWithSelection& normal)
{
  normal.descriptor.classId = 3u;
  normal.descriptor.logicalName[0] = 1u;
  normal.descriptor.logicalName[1] = 0u;
  normal.descriptor.logicalName[2] = 1u;
  normal.descriptor.logicalName[3] = 8u;
  normal.descriptor.logicalName[4] = 0u;
  normal.descriptor.logicalName[5] = 255u;
  normal.descriptor.attributeId = 2u;
  normal.hasSelection = false;
}

void FillActionDescriptor(
  dlms::apdu::CosemMethodDescriptorWithParameter& normal)
{
  normal.descriptor.classId = 3u;
  normal.descriptor.logicalName[0] = 1u;
  normal.descriptor.logicalName[1] = 0u;
  normal.descriptor.logicalName[2] = 1u;
  normal.descriptor.logicalName[3] = 8u;
  normal.descriptor.logicalName[4] = 0u;
  normal.descriptor.logicalName[5] = 255u;
  normal.descriptor.methodId = 1u;
}

std::vector<std::uint8_t> EncodeApdu(const dlms::apdu::XdlmsApdu& apdu)
{
  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(apdu, output));
  return output;
}

std::vector<std::uint8_t> MakeGetRequest()
{
  return EncodeApdu(dlms::apdu::MakeGetRequestNormal(
    0x85u,
    3u,
    dlms::apdu::LogicalName(1, 0, 1, 8, 0, 255),
    2u));
}

std::vector<std::uint8_t> MakeSetRequest(std::uint16_t value)
{
  dlms::apdu::XdlmsApdu apdu;
  apdu.kind = dlms::apdu::XdlmsApduKind::SetRequest;
  apdu.setRequestAny.choice = dlms::apdu::SetRequestChoice::Normal;
  apdu.setRequestAny.invokeIdAndPriority = 0x86u;
  FillSetDescriptor(apdu.setRequestAny.normal);
  apdu.setRequestAny.data = MakeLongUnsignedData(value);
  return EncodeApdu(apdu);
}

std::vector<std::uint8_t> MakeActionRequest(std::uint16_t value)
{
  dlms::apdu::XdlmsApdu apdu;
  apdu.kind = dlms::apdu::XdlmsApduKind::ActionRequest;
  apdu.actionRequestAny.choice = dlms::apdu::ActionRequestChoice::Normal;
  apdu.actionRequestAny.invokeIdAndPriority = 0x87u;
  FillActionDescriptor(apdu.actionRequestAny.normal);
  apdu.actionRequestAny.normal.hasInvocationParameter = true;
  apdu.actionRequestAny.normal.invocationParameter = MakeLongUnsignedData(value);
  return EncodeApdu(apdu);
}

std::vector<std::uint8_t> EncodeAarq()
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

std::vector<std::uint8_t> EncodeRlrq()
{
  const dlms::apdu::AcseApdu rlrq = dlms::apdu::MakeRlrq();

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(rlrq, output));
  return output;
}

std::vector<std::uint8_t> EncodeLlsAarq(
  const std::vector<std::uint8_t>& credential)
{
  const dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::XdlmsApdu xdlms(request);
  dlms::apdu::AcseApdu aarq =
    dlms::apdu::MakeAarqWithInitiateRequest(xdlms);

  const std::uint8_t requirements[] = {0x8A, 0x02, 0x07, 0x80};
  const std::uint8_t mechanism[] = {
    0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x01};
  std::vector<std::uint8_t> credentialField;
  credentialField.push_back(0xAC);
  credentialField.push_back(
    static_cast<std::uint8_t>(credential.size() + 2u));
  credentialField.push_back(0x80);
  credentialField.push_back(static_cast<std::uint8_t>(credential.size()));
  credentialField.insert(
    credentialField.end(),
    credential.begin(),
    credential.end());

  dlms::apdu::AcseRawField field = {};
  field.tag = requirements[0];
  field.encoded.data = requirements;
  field.encoded.size = sizeof(requirements);
  aarq.aarq.fields.push_back(field);

  field.tag = mechanism[0];
  field.encoded.data = mechanism;
  field.encoded.size = sizeof(mechanism);
  aarq.aarq.fields.push_back(field);

  field.tag = credentialField[0];
  field.encoded.data = &credentialField[0];
  field.encoded.size = credentialField.size();
  aarq.aarq.fields.push_back(field);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(aarq, output));
  return output;
}

dlms::apdu::XdlmsApdu DecodeXdlmsResponse(
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

dlms::apdu::AcseApdu DecodeAcseResponse(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::apdu::AcseApdu response = {};
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeAcseApdu(
              bytes.empty() ? 0 : &bytes[0],
              bytes.size(),
              response));
  return response;
}

} // namespace

TEST(GatewayEndpoint, StartsClosedAndCloseIsIdempotent)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);

  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(GatewayEndpoint, OpensAndClosesDownstreamAndUpstream)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(channel.IsOpen());
  EXPECT_TRUE(upstream.IsOpen());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_FALSE(upstream.IsOpen());
}

TEST(GatewayEndpoint, CloseCleansDownstreamAfterUpstreamCloseFailure)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  upstream.closeStatus = dlms::endpoint::EndpointStatus::TransportFailed;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::TransportFailed,
            endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_FALSE(upstream.IsOpen());
}

TEST(GatewayEndpoint, RunOnceRequiresOpenEndpoint)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            endpoint.RunOnce());
  EXPECT_EQ(0u, channel.receiveCalls);
}

TEST(GatewayEndpoint, OpenCanNegotiateDownstreamAssociationBeforeRunOnce)
{
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(MakeGetRequest());
  FakeGatewayUpstream upstream;
  upstream.getData = EncodeLongUnsigned(0x2468u);
  FakeGatewayPolicy policy;

  dlms::endpoint::GatewayEndpointOptions options =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  options.downstream.negotiateAssociation = true;

  dlms::endpoint::GatewayEndpoint endpoint(channel, options, upstream, policy);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(upstream.IsOpen());

  ASSERT_EQ(1u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu aare = DecodeAcseResponse(channel.sentFrames[0]);
  ASSERT_EQ(dlms::apdu::AcseApduKind::Aare, aare.kind);
  EXPECT_TRUE(aare.aare.hasResult);
  EXPECT_EQ(0, aare.aare.result);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, upstream.getCalls);

  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu response =
    DecodeXdlmsResponse(channel.sentFrames[1]);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x2468u, response.getResponseAny.result.data.unsignedValue);
}

TEST(GatewayEndpoint, OpenCanNegotiateLowPasswordAssociationBeforeRunOnce)
{
  const std::uint8_t password[] = {'p', 'w'};
  const std::vector<std::uint8_t> credential(
    password,
    password + sizeof(password));
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(credential));
  channel.receiveQueue.push_back(MakeGetRequest());
  FakeGatewayUpstream upstream;
  upstream.getData = EncodeLongUnsigned(0x1357u);
  FakeGatewayPolicy policy;

  dlms::endpoint::GatewayEndpointOptions options =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  options.downstream.negotiateAssociation = true;
  options.downstream.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.downstream.security.password = password;
  options.downstream.security.passwordSize = sizeof(password);

  dlms::endpoint::GatewayEndpoint endpoint(channel, options, upstream, policy);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(upstream.IsOpen());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, upstream.getCalls);
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu response =
    DecodeXdlmsResponse(channel.sentFrames[1]);
  EXPECT_EQ(0x1357u, response.getResponseAny.result.data.unsignedValue);
}

TEST(GatewayEndpoint, OpenCanRetryAfterNegotiatedUpstreamOpenFailure)
{
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(MakeGetRequest());
  FakeGatewayUpstream upstream;
  upstream.openStatus = dlms::endpoint::EndpointStatus::TransportFailed;
  upstream.getData = EncodeLongUnsigned(0x2468u);
  FakeGatewayPolicy policy;

  dlms::endpoint::GatewayEndpointOptions options =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  options.downstream.negotiateAssociation = true;

  dlms::endpoint::GatewayEndpoint endpoint(channel, options, upstream, policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::TransportFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_FALSE(upstream.IsOpen());
  ASSERT_EQ(1u, channel.sentFrames.size());
  EXPECT_EQ(dlms::apdu::AcseApduKind::Aare,
            DecodeAcseResponse(channel.sentFrames[0]).kind);

  upstream.openStatus = dlms::endpoint::EndpointStatus::Ok;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(channel.IsOpen());
  EXPECT_TRUE(upstream.IsOpen());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, upstream.getCalls);
  ASSERT_EQ(3u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu response =
    DecodeXdlmsResponse(channel.sentFrames[2]);
  EXPECT_EQ(0x2468u, response.getResponseAny.result.data.unsignedValue);
}

TEST(GatewayEndpoint, RunOnceCanReleaseNegotiatedDownstreamAssociation)
{
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(EncodeRlrq());
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;

  dlms::endpoint::GatewayEndpointOptions options =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  options.downstream.negotiateAssociation = true;

  dlms::endpoint::GatewayEndpoint endpoint(channel, options, upstream, policy);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(upstream.IsOpen());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());

  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_FALSE(upstream.IsOpen());
  EXPECT_EQ(0u, upstream.getCalls);
  EXPECT_EQ(0u, upstream.setCalls);
  EXPECT_EQ(0u, upstream.actionCalls);
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu rlre = DecodeAcseResponse(channel.sentFrames[1]);
  EXPECT_EQ(dlms::apdu::AcseApduKind::Rlre, rlre.kind);
}

TEST(GatewayEndpoint, OpenRejectsNegotiatedLowPasswordCredentialMismatch)
{
  const std::uint8_t clientPassword[] = {'p', 'w'};
  const std::uint8_t serverPassword[] = {'b', 'a', 'd'};
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(
    std::vector<std::uint8_t>(
      clientPassword,
      clientPassword + sizeof(clientPassword))));
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpointOptions options =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  options.downstream.negotiateAssociation = true;
  options.downstream.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.downstream.security.password = serverPassword;
  options.downstream.security.passwordSize = sizeof(serverPassword);

  dlms::endpoint::GatewayEndpoint endpoint(channel, options, upstream, policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_FALSE(upstream.IsOpen());
  EXPECT_EQ(0u, channel.sentFrames.size());
}

TEST(GatewayEndpoint, OpenRejectsNegotiatedAssociationWithHighAuthentication)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpointOptions options =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  options.downstream.negotiateAssociation = true;
  options.downstream.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighPassword;
  const std::uint8_t password[] = { 'p', 'w' };
  options.downstream.security.password = password;
  options.downstream.security.passwordSize = sizeof(password);

  dlms::endpoint::GatewayEndpoint endpoint(channel, options, upstream, policy);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_FALSE(upstream.IsOpen());
}

TEST(GatewayEndpoint, RunOnceForwardsGetToUpstream)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  upstream.getData = EncodeLongUnsigned(0x1234u);
  channel.nextReceive = MakeGetRequest();

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, upstream.getCalls);
  EXPECT_EQ(3u, upstream.lastGetDescriptor.classId);
  EXPECT_EQ(2u, upstream.lastGetDescriptor.attributeId);
  ASSERT_EQ(1u, channel.sendCalls);

  const dlms::apdu::XdlmsApdu response = DecodeXdlmsResponse(channel.sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(0x1234u, response.getResponseAny.result.data.unsignedValue);
}

TEST(GatewayEndpoint, PolicyDeniedGetReturnsAccessDeniedResult)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  policy.allowGet = false;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive = MakeGetRequest();

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(0u, upstream.getCalls);

  const dlms::apdu::XdlmsApdu response = DecodeXdlmsResponse(channel.sent);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::DataAccessError,
            response.getResponseAny.result.choice);
  EXPECT_EQ(3u, response.getResponseAny.result.dataAccessError);
}

TEST(GatewayEndpoint, RunOnceForwardsSetToUpstream)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive = MakeSetRequest(0x4321u);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, upstream.setCalls);
  EXPECT_EQ(EncodeLongUnsigned(0x4321u), upstream.lastSetData);

  const dlms::apdu::XdlmsApdu response = DecodeXdlmsResponse(channel.sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, response.kind);
  EXPECT_EQ(0u, response.setResponseAny.result);
}

TEST(GatewayEndpoint, PolicyDeniedSetReturnsAccessDeniedResult)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  policy.allowSet = false;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive = MakeSetRequest(0x4321u);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(0u, upstream.setCalls);

  const dlms::apdu::XdlmsApdu response = DecodeXdlmsResponse(channel.sent);
  EXPECT_EQ(3u, response.setResponseAny.result);
}

TEST(GatewayEndpoint, RunOnceForwardsActionToUpstream)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  upstream.actionReturnData = EncodeLongUnsigned(0x5678u);
  channel.nextReceive = MakeActionRequest(0x1111u);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, upstream.actionCalls);
  EXPECT_TRUE(upstream.lastHasActionParameter);
  EXPECT_EQ(EncodeLongUnsigned(0x1111u), upstream.lastActionParameter);

  const dlms::apdu::XdlmsApdu response = DecodeXdlmsResponse(channel.sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, response.kind);
  EXPECT_EQ(0u, response.actionResponseAny.normal.result);
  EXPECT_TRUE(response.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(0x5678u,
            response.actionResponseAny.normal.returnParameter.unsignedValue);
}

TEST(GatewayEndpoint, PolicyDeniedActionReturnsAccessDeniedResult)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  policy.allowAction = false;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive = MakeActionRequest(0x1111u);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(0u, upstream.actionCalls);

  const dlms::apdu::XdlmsApdu response = DecodeXdlmsResponse(channel.sent);
  EXPECT_EQ(3u, response.actionResponseAny.normal.result);
}

TEST(GatewayEndpoint, RunOnceMapsProfileAndUpstreamFailures)
{
  FakeApduChannel channel;
  FakeGatewayUpstream upstream;
  FakeGatewayPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(channel, upstream, policy);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.receiveStatus = dlms::profile::ProfileStatus::ReadFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.RunOnce());

  channel.receiveStatus = dlms::profile::ProfileStatus::Ok;
  channel.nextReceive = MakeGetRequest();
  upstream.getStatus = dlms::endpoint::EndpointStatus::ServiceFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ServiceFailed,
            endpoint.RunOnce());

  upstream.getStatus = dlms::endpoint::EndpointStatus::Ok;
  upstream.getData = EncodeLongUnsigned(0x1234u);
  channel.sendStatus = dlms::profile::ProfileStatus::WriteFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.RunOnce());
}

TEST(GatewayEndpoint, MapsEndpointStatusesToXdlmsStatuses)
{
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            dlms::endpoint::MapEndpointStatusToXdlmsStatus(
              dlms::endpoint::EndpointStatus::Ok));
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::InvalidArgument,
            dlms::endpoint::MapEndpointStatusToXdlmsStatus(
              dlms::endpoint::EndpointStatus::InvalidArgument));
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::SecurityFailed,
            dlms::endpoint::MapEndpointStatusToXdlmsStatus(
              dlms::endpoint::EndpointStatus::SecurityFailed));
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::UnsupportedFeature,
            dlms::endpoint::MapEndpointStatusToXdlmsStatus(
              dlms::endpoint::EndpointStatus::UnsupportedProfile));
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::ServiceRejected,
            dlms::endpoint::MapEndpointStatusToXdlmsStatus(
              dlms::endpoint::EndpointStatus::Timeout));
}
