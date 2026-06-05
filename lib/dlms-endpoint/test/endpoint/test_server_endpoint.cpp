#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/initiate.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/security/hls_gmac_authenticator.hpp"
#include "dlms/security/hls_high_authenticator.hpp"
#include "dlms/security/in_memory_invocation_counter_store.hpp"
#include "dlms/security/in_memory_key_store.hpp"
#include "dlms/security/random_source.hpp"

#include <gtest/gtest.h>

#include <memory>

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

class FakeServerService : public dlms::server::IServerService
{
public:
  FakeServerService()
    : getCalls(0)
    , setCalls(0)
    , actionCalls(0)
  {
  }

  dlms::server::ServerGetResponse HandleGet(
    const dlms::server::ServerGetRequest& request)
  {
    ++getCalls;
    lastGetRequest = request;

    dlms::cosem::CosemByteBuffer data;
    data.push_back(0x12u);
    data.push_back(0x12u);
    data.push_back(0x34u);
    return dlms::server::MakeServerGetDataResponse(
      request.invokeId,
      data);
  }

  dlms::server::ServerSetResponse HandleSet(
    const dlms::server::ServerSetRequest& request)
  {
    ++setCalls;
    lastSetRequest = request;
    return dlms::server::MakeServerSetResponse(
      request.invokeId,
      dlms::server::ServerStatus::Ok);
  }

  dlms::server::ServerActionResponse HandleAction(
    const dlms::server::ServerActionRequest& request)
  {
    ++actionCalls;
    lastActionRequest = request;
    return dlms::server::MakeServerActionDataResponse(
      request.invokeId,
      dlms::cosem::CosemByteBuffer());
  }

  int getCalls;
  int setCalls;
  int actionCalls;
  dlms::server::ServerGetRequest lastGetRequest;
  dlms::server::ServerSetRequest lastSetRequest;
  dlms::server::ServerActionRequest lastActionRequest;
};

class FixedRandomSource : public dlms::security::IRandomSource
{
public:
  explicit FixedRandomSource(std::uint8_t seed)
    : seed_(seed)
  {
  }

  dlms::security::SecurityStatus Fill(
    std::uint8_t* output,
    std::size_t outputSize)
  {
    if (output == 0 && outputSize != 0u) {
      return dlms::security::SecurityStatus::InvalidArgument;
    }
    for (std::size_t i = 0u; i < outputSize; ++i) {
      output[i] = static_cast<std::uint8_t>(seed_ + i);
    }
    return dlms::security::SecurityStatus::Ok;
  }

private:
  std::uint8_t seed_;
};

dlms::security::SecurityByteView SecurityView(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::security::SecurityByteView view;
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

dlms::security::SecurityKey MakeAuthenticationKey(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::security::SecurityKey key =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  key.size = bytes.size();
  for (std::size_t i = 0u; i < bytes.size() && i < sizeof(key.bytes); ++i) {
    key.bytes[i] = bytes[i];
  }
  return key;
}

dlms::security::SecurityContext MakeGmacContext(
  dlms::security::SecurityRole role,
  const std::vector<std::uint8_t>& clientTitle,
  const std::vector<std::uint8_t>& serverTitle)
{
  dlms::security::SecurityContext context =
    dlms::security::EmptySecurityContext();
  context.policy = dlms::security::SecurityPolicy::Authenticated;
  context.role = role;
  context.clientSap = 16u;
  context.serverSap = 1u;
  const std::vector<std::uint8_t>& local =
    role == dlms::security::SecurityRole::Client
      ? clientTitle
      : serverTitle;
  const std::vector<std::uint8_t>& remote =
    role == dlms::security::SecurityRole::Client
      ? serverTitle
      : clientTitle;
  for (std::size_t i = 0u; i < 8u; ++i) {
    context.localSystemTitle[i] = local[i];
    context.remoteSystemTitle[i] = remote[i];
  }
  return context;
}

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

std::vector<std::uint8_t> EncodeGetRequest()
{
  const dlms::apdu::XdlmsApdu request =
    dlms::apdu::MakeGetRequestNormal(
      0x85u,
      3u,
      dlms::apdu::LogicalName(1, 0, 1, 8, 0, 255),
      2u);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
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

std::vector<std::uint8_t> EncodeHlsAarq(
  const std::vector<std::uint8_t>& challenge)
{
  const dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::XdlmsApdu xdlms(request);
  dlms::apdu::AcseApdu aarq =
    dlms::apdu::MakeAarqWithInitiateRequest(xdlms);

  const std::uint8_t requirements[] = {0x8A, 0x02, 0x07, 0x80};
  const std::uint8_t mechanism[] = {
    0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x02};
  std::vector<std::uint8_t> challengeField;
  challengeField.push_back(0xAC);
  challengeField.push_back(
    static_cast<std::uint8_t>(challenge.size() + 2u));
  challengeField.push_back(0x80);
  challengeField.push_back(static_cast<std::uint8_t>(challenge.size()));
  challengeField.insert(
    challengeField.end(),
    challenge.begin(),
    challenge.end());

  dlms::apdu::AcseRawField field = {};
  field.tag = requirements[0];
  field.encoded.data = requirements;
  field.encoded.size = sizeof(requirements);
  aarq.aarq.fields.push_back(field);

  field.tag = mechanism[0];
  field.encoded.data = mechanism;
  field.encoded.size = sizeof(mechanism);
  aarq.aarq.fields.push_back(field);

  field.tag = challengeField[0];
  field.encoded.data = &challengeField[0];
  field.encoded.size = challengeField.size();
  aarq.aarq.fields.push_back(field);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(aarq, output));
  return output;
}

std::vector<std::uint8_t> EncodeHlsGmacAarq(
  const std::vector<std::uint8_t>& challenge,
  const std::vector<std::uint8_t>& clientTitle)
{
  const dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::XdlmsApdu xdlms(request);
  dlms::apdu::AcseApdu aarq =
    dlms::apdu::MakeAarqWithInitiateRequest(xdlms);

  std::vector<std::uint8_t> titleField;
  titleField.push_back(0xA6);
  titleField.push_back(static_cast<std::uint8_t>(clientTitle.size() + 2u));
  titleField.push_back(0x04);
  titleField.push_back(static_cast<std::uint8_t>(clientTitle.size()));
  titleField.insert(titleField.end(), clientTitle.begin(), clientTitle.end());

  const std::uint8_t requirements[] = {0x8A, 0x02, 0x07, 0x80};
  const std::uint8_t mechanism[] = {
    0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x05};
  std::vector<std::uint8_t> challengeField;
  challengeField.push_back(0xAC);
  challengeField.push_back(
    static_cast<std::uint8_t>(challenge.size() + 2u));
  challengeField.push_back(0x80);
  challengeField.push_back(static_cast<std::uint8_t>(challenge.size()));
  challengeField.insert(
    challengeField.end(),
    challenge.begin(),
    challenge.end());

  dlms::apdu::AcseRawField field = {};
  field.tag = titleField[0];
  field.encoded.data = &titleField[0];
  field.encoded.size = titleField.size();
  aarq.aarq.fields.push_back(field);

  field.tag = requirements[0];
  field.encoded.data = requirements;
  field.encoded.size = sizeof(requirements);
  aarq.aarq.fields.push_back(field);

  field.tag = mechanism[0];
  field.encoded.data = mechanism;
  field.encoded.size = sizeof(mechanism);
  aarq.aarq.fields.push_back(field);

  field.tag = challengeField[0];
  field.encoded.data = &challengeField[0];
  field.encoded.size = challengeField.size();
  aarq.aarq.fields.push_back(field);

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

std::vector<std::uint8_t> EncodeHlsReplyAction(
  const std::vector<std::uint8_t>& response)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::ActionRequest;
  request.actionRequestAny.choice = dlms::apdu::ActionRequestChoice::Normal;
  request.actionRequestAny.invokeIdAndPriority = 0x81u;
  request.actionRequestAny.normal.descriptor.classId = 15u;
  request.actionRequestAny.normal.descriptor.logicalName[0] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[1] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[2] = 40u;
  request.actionRequestAny.normal.descriptor.logicalName[3] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[4] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[5] = 255u;
  request.actionRequestAny.normal.descriptor.methodId = 1u;
  request.actionRequestAny.normal.hasInvocationParameter = true;
  request.actionRequestAny.normal.invocationParameter.type =
    dlms::apdu::DlmsDataType::OctetString;
  request.actionRequestAny.normal.invocationParameter.bytes.data =
    response.empty() ? 0 : &response[0];
  request.actionRequestAny.normal.invocationParameter.bytes.size =
    response.size();

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

std::vector<std::uint8_t> AuthenticationFieldValue(
  const dlms::apdu::AcseApdu& apdu,
  std::uint8_t tag)
{
  for (std::vector<dlms::apdu::AcseRawField>::const_iterator it =
         apdu.aare.fields.begin();
       it != apdu.aare.fields.end();
       ++it) {
    if (it->tag == tag &&
        it->encoded.size >= 4u &&
        it->encoded.data[2] == 0x80u) {
      return std::vector<std::uint8_t>(
        it->encoded.data + 4u,
        it->encoded.data + it->encoded.size);
    }
  }
  return std::vector<std::uint8_t>();
}

std::vector<std::uint8_t> OctetStringFieldValue(
  const dlms::apdu::AcseApdu& apdu,
  std::uint8_t tag)
{
  for (std::vector<dlms::apdu::AcseRawField>::const_iterator it =
         apdu.aare.fields.begin();
       it != apdu.aare.fields.end();
       ++it) {
    if (it->tag == tag &&
        it->encoded.size >= 4u &&
        it->encoded.data[2] == 0x04u) {
      return std::vector<std::uint8_t>(
        it->encoded.data + 4u,
        it->encoded.data + it->encoded.size);
    }
  }
  return std::vector<std::uint8_t>();
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

std::shared_ptr<dlms::cosem::CosemRegisterObject> MakeRegisterObject(
  std::uint16_t value)
{
  return std::shared_ptr<dlms::cosem::CosemRegisterObject>(
    new dlms::cosem::CosemRegisterObject(
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
      EncodeLongUnsigned(value),
      dlms::cosem::CosemByteBuffer(),
      dlms::cosem::AttributeAccessMode::ReadOnly));
}

} // namespace

TEST(ServerEndpoint, StartsClosedAndCloseIsIdempotent)
{
  FakeApduChannel channel;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerEndpoint endpoint(channel, logicalDevice);

  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(ServerEndpoint, OpenConfiguresAssociatedContext)
{
  FakeApduChannel channel;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.profile.clientSap = 32u;
  options.profile.serverSap = 1u;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  const std::uint8_t password[] = { 'p', 'w' };
  options.security.password = password;
  options.security.passwordSize = sizeof(password);

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(endpoint.Context().IsAssociated());
  EXPECT_EQ(32u, endpoint.Context().AssociationContext().clientSap);
  EXPECT_TRUE(endpoint.Context().AssociationContext().authenticated);
  EXPECT_FALSE(endpoint.Context().AssociationContext().ciphered);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(ServerEndpoint, RunOnceCanUseInjectedServerService)
{
  FakeApduChannel channel;
  channel.nextReceive = EncodeGetRequest();
  FakeServerService server;
  dlms::endpoint::ServerEndpoint endpoint(channel, server);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());

  EXPECT_EQ(1, server.getCalls);
  EXPECT_EQ(0, server.setCalls);
  EXPECT_EQ(0, server.actionCalls);
  EXPECT_EQ(5u, server.lastGetRequest.invokeId);
  EXPECT_EQ(3u, server.lastGetRequest.descriptor.object.classId);
  EXPECT_EQ(2u, server.lastGetRequest.descriptor.attributeId);

  ASSERT_EQ(1u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu response =
    DecodeXdlmsResponse(channel.sentFrames[0]);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.getResponseAny.result.data.type);
  EXPECT_EQ(0x1234u, response.getResponseAny.result.data.unsignedValue);
}

TEST(ServerEndpoint, RunOnceReceiveFailureDoesNotDispatch)
{
  FakeApduChannel channel;
  channel.receiveStatus = dlms::profile::ProfileStatus::ReadFailed;
  FakeServerService server;
  dlms::endpoint::ServerEndpoint endpoint(channel, server);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.RunOnce());

  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(0u, channel.sendCalls);
  EXPECT_TRUE(channel.sentFrames.empty());
  EXPECT_EQ(0, server.getCalls);
  EXPECT_EQ(0, server.setCalls);
  EXPECT_EQ(0, server.actionCalls);
}

TEST(ServerEndpoint, RunOnceMalformedApduDoesNotDispatch)
{
  FakeApduChannel channel;
  channel.nextReceive.push_back(0xC0u);
  channel.nextReceive.push_back(0x01u);
  FakeServerService server;
  dlms::endpoint::ServerEndpoint endpoint(channel, server);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InternalError,
            endpoint.RunOnce());

  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(0u, channel.sendCalls);
  EXPECT_TRUE(channel.sentFrames.empty());
  EXPECT_EQ(0, server.getCalls);
  EXPECT_EQ(0, server.setCalls);
  EXPECT_EQ(0, server.actionCalls);
}

TEST(ServerEndpoint, RunOnceSendFailureKeepsEndpointOpen)
{
  FakeApduChannel channel;
  channel.nextReceive = EncodeGetRequest();
  channel.sendStatus = dlms::profile::ProfileStatus::WriteFailed;
  FakeServerService server;
  dlms::endpoint::ServerEndpoint endpoint(channel, server);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.RunOnce());

  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(1u, channel.sendCalls);
  EXPECT_TRUE(channel.sentFrames.empty());
  EXPECT_EQ(1, server.getCalls);
  EXPECT_EQ(0, server.setCalls);
  EXPECT_EQ(0, server.actionCalls);
}

TEST(ServerEndpoint, OpenCanNegotiateAssociationBeforeRunOnce)
{
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(EncodeGetRequest());
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(MakeRegisterObject(0x1234u)));

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  EXPECT_FALSE(endpoint.Context().IsAssociated());
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.Context().IsAssociated());
  EXPECT_EQ(16u, endpoint.Context().AssociationContext().clientSap);
  EXPECT_EQ(1u, endpoint.Context().AssociationContext().serverSap);

  ASSERT_EQ(1u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu aare = DecodeAcseResponse(channel.sentFrames[0]);
  ASSERT_EQ(dlms::apdu::AcseApduKind::Aare, aare.kind);
  EXPECT_TRUE(aare.aare.hasResult);
  EXPECT_EQ(0, aare.aare.result);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu response =
    DecodeXdlmsResponse(channel.sentFrames[1]);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.getResponseAny.result.data.type);
  EXPECT_EQ(0x1234u, response.getResponseAny.result.data.unsignedValue);
}

TEST(ServerEndpoint, OpenCanNegotiateLowPasswordAssociationBeforeRunOnce)
{
  const std::uint8_t password[] = {'p', 'w'};
  const std::vector<std::uint8_t> credential(
    password,
    password + sizeof(password));
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(credential));
  channel.receiveQueue.push_back(EncodeGetRequest());
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(MakeRegisterObject(0x5678u)));

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = password;
  options.security.passwordSize = sizeof(password);

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.Context().IsAssociated());
  EXPECT_TRUE(endpoint.Context().AssociationContext().authenticated);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());

  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu response =
    DecodeXdlmsResponse(channel.sentFrames[1]);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(0x5678u, response.getResponseAny.result.data.unsignedValue);
}

TEST(ServerEndpoint, OpenCanNegotiateHighPasswordAssociationWithHlsReply)
{
  const std::uint8_t passwordBytes[] = {'s', 'e', 'c', 'r', 'e', 't'};
  const std::vector<std::uint8_t> password(
    passwordBytes,
    passwordBytes + sizeof(passwordBytes));
  std::vector<std::uint8_t> clientChallenge(16u, 0u);
  for (std::size_t i = 0u; i < clientChallenge.size(); ++i) {
    clientChallenge[i] = static_cast<std::uint8_t>(0x20u + i);
  }

  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeHlsAarq(clientChallenge));
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(MakeRegisterObject(0x1357u)));

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighPassword;
  options.security.password = &passwordBytes[0];
  options.security.passwordSize = sizeof(passwordBytes);

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_FALSE(endpoint.Context().IsAssociated());

  ASSERT_EQ(1u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu aare = DecodeAcseResponse(channel.sentFrames[0]);
  const std::vector<std::uint8_t> serverChallenge =
    AuthenticationFieldValue(aare, 0xAAu);
  ASSERT_FALSE(serverChallenge.empty());

  FixedRandomSource random(0x44u);
  dlms::security::HlsHighAuthenticator clientHls(
    SecurityView(password),
    random);
  std::vector<std::uint8_t> clientResponse;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            clientHls.BuildResponse(
              SecurityView(serverChallenge),
              clientResponse));
  channel.receiveQueue.push_back(EncodeHlsReplyAction(clientResponse));
  channel.receiveQueue.push_back(EncodeGetRequest());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_TRUE(endpoint.Context().IsAssociated());
  EXPECT_TRUE(endpoint.Context().AssociationContext().authenticated);
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu hlsResponse =
    DecodeXdlmsResponse(channel.sentFrames[1]);
  ASSERT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, hlsResponse.kind);
  ASSERT_TRUE(hlsResponse.actionResponseAny.normal.hasReturnParameter);
  ASSERT_EQ(dlms::apdu::DlmsDataType::OctetString,
            hlsResponse.actionResponseAny.normal.returnParameter.type);
  const dlms::apdu::ByteView serverResponse =
    hlsResponse.actionResponseAny.normal.returnParameter.bytes;
  EXPECT_EQ(dlms::security::SecurityStatus::Ok,
            clientHls.VerifyResponse(
              SecurityView(clientChallenge),
              dlms::security::SecurityByteView{
                serverResponse.data,
                serverResponse.size}));

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  ASSERT_EQ(3u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu getResponse =
    DecodeXdlmsResponse(channel.sentFrames[2]);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, getResponse.kind);
  EXPECT_EQ(0x1357u, getResponse.getResponseAny.result.data.unsignedValue);
}

TEST(ServerEndpoint, HighPasswordRejectsInvalidHlsReply)
{
  const std::uint8_t passwordBytes[] = {'s', 'e', 'c', 'r', 'e', 't'};
  std::vector<std::uint8_t> clientChallenge(16u, 0x22u);
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeHlsAarq(clientChallenge));
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighPassword;
  options.security.password = &passwordBytes[0];
  options.security.passwordSize = sizeof(passwordBytes);

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  ASSERT_EQ(1u, channel.sentFrames.size());
  channel.receiveQueue.push_back(
    EncodeHlsReplyAction(std::vector<std::uint8_t>(16u, 0x00u)));

  EXPECT_EQ(dlms::endpoint::EndpointStatus::SecurityFailed,
            endpoint.RunOnce());
  EXPECT_FALSE(endpoint.Context().IsAssociated());
  EXPECT_EQ(1u, channel.sentFrames.size());
}

TEST(ServerEndpoint, OpenCanNegotiateHighGmacAssociationWithHlsReply)
{
  const std::uint8_t serverTitleBytes[] =
    {'S', 'R', 'V', 'T', 'I', 'T', 'L', 'E'};
  const std::uint8_t clientTitleBytes[] =
    {'C', 'L', 'I', 'T', 'I', 'T', 'L', 'E'};
  const std::uint8_t keyBytes[] = {
    0x40, 0x41, 0x42, 0x43,
    0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B,
    0x4C, 0x4D, 0x4E, 0x4F};
  const std::vector<std::uint8_t> serverTitle(
    serverTitleBytes,
    serverTitleBytes + sizeof(serverTitleBytes));
  const std::vector<std::uint8_t> clientTitle(
    clientTitleBytes,
    clientTitleBytes + sizeof(clientTitleBytes));
  const std::vector<std::uint8_t> authenticationKey(
    keyBytes,
    keyBytes + sizeof(keyBytes));
  std::vector<std::uint8_t> clientChallenge(16u, 0u);
  for (std::size_t i = 0u; i < clientChallenge.size(); ++i) {
    clientChallenge[i] = static_cast<std::uint8_t>(0x30u + i);
  }

  FakeApduChannel channel;
  channel.receiveQueue.push_back(
    EncodeHlsGmacAarq(clientChallenge, clientTitle));
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(MakeRegisterObject(0x2468u)));

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighGmac;
  options.security.systemTitle = &serverTitleBytes[0];
  options.security.systemTitleSize = sizeof(serverTitleBytes);
  options.security.authenticationKey = &keyBytes[0];
  options.security.authenticationKeySize = sizeof(keyBytes);
  options.security.invocationCounter = 9u;

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_FALSE(endpoint.Context().IsAssociated());

  ASSERT_EQ(1u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu aare = DecodeAcseResponse(channel.sentFrames[0]);
  const std::vector<std::uint8_t> serverChallenge =
    AuthenticationFieldValue(aare, 0xAAu);
  ASSERT_FALSE(serverChallenge.empty());
  EXPECT_EQ(serverTitle, OctetStringFieldValue(aare, 0xA4u));

  dlms::security::InMemoryKeyStore keys;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keys.SetKey(MakeAuthenticationKey(authenticationKey)));
  dlms::security::InMemoryInvocationCounterStore clientCounters;
  clientCounters.SetLocalCounter(3u);
  FixedRandomSource random(0x44u);
  dlms::security::HlsGmacAuthenticator clientHls(
    MakeGmacContext(
      dlms::security::SecurityRole::Client,
      clientTitle,
      serverTitle),
    keys,
    clientCounters,
    random);

  std::vector<std::uint8_t> clientResponse;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            clientHls.BuildResponse(
              SecurityView(serverChallenge),
              clientResponse));
  channel.receiveQueue.push_back(EncodeHlsReplyAction(clientResponse));
  channel.receiveQueue.push_back(EncodeGetRequest());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_TRUE(endpoint.Context().IsAssociated());
  EXPECT_TRUE(endpoint.Context().AssociationContext().authenticated);
  EXPECT_FALSE(endpoint.Context().AssociationContext().ciphered);
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu hlsResponse =
    DecodeXdlmsResponse(channel.sentFrames[1]);
  ASSERT_TRUE(hlsResponse.actionResponseAny.normal.hasReturnParameter);
  const dlms::apdu::ByteView serverResponse =
    hlsResponse.actionResponseAny.normal.returnParameter.bytes;
  EXPECT_EQ(dlms::security::SecurityStatus::Ok,
            clientHls.VerifyResponse(
              SecurityView(clientChallenge),
              dlms::security::SecurityByteView{
                serverResponse.data,
                serverResponse.size}));

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  ASSERT_EQ(3u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu getResponse =
    DecodeXdlmsResponse(channel.sentFrames[2]);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, getResponse.kind);
  EXPECT_EQ(0x2468u, getResponse.getResponseAny.result.data.unsignedValue);
}

TEST(ServerEndpoint, HighGmacRejectsInvalidHlsReply)
{
  const std::uint8_t serverTitle[] =
    {'S', 'R', 'V', 'T', 'I', 'T', 'L', 'E'};
  const std::uint8_t clientTitleBytes[] =
    {'C', 'L', 'I', 'T', 'I', 'T', 'L', 'E'};
  const std::uint8_t key[] = {
    0x40, 0x41, 0x42, 0x43,
    0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B,
    0x4C, 0x4D, 0x4E, 0x4F};
  const std::vector<std::uint8_t> clientTitle(
    clientTitleBytes,
    clientTitleBytes + sizeof(clientTitleBytes));
  std::vector<std::uint8_t> clientChallenge(16u, 0x33u);
  FakeApduChannel channel;
  channel.receiveQueue.push_back(
    EncodeHlsGmacAarq(clientChallenge, clientTitle));
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighGmac;
  options.security.systemTitle = serverTitle;
  options.security.systemTitleSize = sizeof(serverTitle);
  options.security.authenticationKey = key;
  options.security.authenticationKeySize = sizeof(key);
  options.security.invocationCounter = 9u;

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  channel.receiveQueue.push_back(
    EncodeHlsReplyAction(std::vector<std::uint8_t>(21u, 0x00u)));

  EXPECT_EQ(dlms::endpoint::EndpointStatus::SecurityFailed,
            endpoint.RunOnce());
  EXPECT_FALSE(endpoint.Context().IsAssociated());
  EXPECT_EQ(1u, channel.sentFrames.size());
}

TEST(ServerEndpoint, RunOnceCanReleaseNegotiatedAssociation)
{
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(EncodeRlrq());
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(endpoint.Context().IsAssociated());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());

  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(endpoint.Context().IsAssociated());
  EXPECT_FALSE(channel.IsOpen());
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu rlre = DecodeAcseResponse(channel.sentFrames[1]);
  EXPECT_EQ(dlms::apdu::AcseApduKind::Rlre, rlre.kind);
  EXPECT_EQ(2u, channel.receiveCalls);
  EXPECT_EQ(2u, channel.sendCalls);
}

TEST(ServerEndpoint, OpenRejectsNegotiatedLowPasswordCredentialMismatch)
{
  const std::uint8_t clientPassword[] = {'p', 'w'};
  const std::uint8_t serverPassword[] = {'b', 'a', 'd'};
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(
    std::vector<std::uint8_t>(
      clientPassword,
      clientPassword + sizeof(clientPassword))));
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = serverPassword;
  options.security.passwordSize = sizeof(serverPassword);

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_EQ(0u, channel.sentFrames.size());
}

TEST(ServerEndpoint, OpenCanRetryAfterLowPasswordCredentialMismatch)
{
  const std::uint8_t clientPassword[] = {'p', 'w'};
  const std::uint8_t serverPassword[] = {'b', 'a', 'd'};
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(
    std::vector<std::uint8_t>(
      clientPassword,
      clientPassword + sizeof(clientPassword))));
  channel.receiveQueue.push_back(EncodeLlsAarq(
    std::vector<std::uint8_t>(
      serverPassword,
      serverPassword + sizeof(serverPassword))));
  channel.receiveQueue.push_back(EncodeGetRequest());
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(MakeRegisterObject(0x1357u)));

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = serverPassword;
  options.security.passwordSize = sizeof(serverPassword);

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(endpoint.Context().IsAssociated());
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());

  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::XdlmsApdu response =
    DecodeXdlmsResponse(channel.sentFrames[1]);
  EXPECT_EQ(0x1357u, response.getResponseAny.result.data.unsignedValue);
}

TEST(ServerEndpoint, RunOnceRequiresOpenEndpoint)
{
  FakeApduChannel channel;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerEndpoint endpoint(channel, logicalDevice);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            endpoint.RunOnce());
}

TEST(ServerEndpoint, RunOnceDispatchesGetRequestToCosemObject)
{
  FakeApduChannel channel;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(MakeRegisterObject(0x1234u)));

  dlms::endpoint::ServerEndpoint endpoint(channel, logicalDevice);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive = EncodeGetRequest();
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());

  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(1u, channel.sendCalls);
  ASSERT_FALSE(channel.sent.empty());

  const dlms::apdu::XdlmsApdu response = DecodeXdlmsResponse(channel.sent);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, response.kind);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            response.getResponseAny.result.choice);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            response.getResponseAny.result.data.type);
  EXPECT_EQ(0x1234u, response.getResponseAny.result.data.unsignedValue);
}

TEST(ServerEndpoint, RunOnceMapsReceiveAndSendFailures)
{
  FakeApduChannel channel;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::endpoint::ServerEndpoint endpoint(channel, logicalDevice);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.receiveStatus = dlms::profile::ProfileStatus::ReadFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.RunOnce());

  channel.receiveStatus = dlms::profile::ProfileStatus::Ok;
  channel.nextReceive = EncodeGetRequest();
  channel.sendStatus = dlms::profile::ProfileStatus::WriteFailed;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.RunOnce());
}

TEST(ServerEndpoint, MapsProfileAndXdlmsStatuses)
{
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::MapProfileStatus(
              dlms::profile::ProfileStatus::AlreadyOpen));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            dlms::endpoint::MapProfileStatus(
              dlms::profile::ProfileStatus::NotOpen));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Timeout,
            dlms::endpoint::MapProfileStatus(
              dlms::profile::ProfileStatus::Timeout));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Closed,
            dlms::endpoint::MapProfileStatus(
              dlms::profile::ProfileStatus::ConnectionClosed));

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::MapXdlmsStatus(
              dlms::xdlms::XdlmsStatus::Ok));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            dlms::endpoint::MapXdlmsStatus(
              dlms::xdlms::XdlmsStatus::NotAssociated));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::SecurityFailed,
            dlms::endpoint::MapXdlmsStatus(
              dlms::xdlms::XdlmsStatus::SecurityFailed));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::UnsupportedProfile,
            dlms::endpoint::MapXdlmsStatus(
              dlms::xdlms::XdlmsStatus::BlockTransferRequired));
}
