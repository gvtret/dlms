#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/initiate.hpp"
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
    , openCalls(0u)
    , receiveCalls(0u)
    , sendCalls(0u)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    ++openCalls;
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
  std::size_t openCalls;
  std::size_t receiveCalls;
  std::size_t sendCalls;
  std::vector<std::uint8_t> nextReceive;
  std::vector<std::vector<std::uint8_t> > receiveQueue;
  std::vector<std::uint8_t> sent;
  std::vector<std::vector<std::uint8_t> > sentFrames;
};

class RecordingPushHandler
  : public dlms::endpoint::IPushIndicationHandler
{
public:
  RecordingPushHandler()
    : status(dlms::endpoint::EndpointStatus::Ok)
    , calls(0u)
  {
  }

  dlms::endpoint::EndpointStatus OnPushApdu(
    const std::vector<std::uint8_t>& apdu)
  {
    ++calls;
    lastApdu = apdu;
    return status;
  }

  dlms::endpoint::EndpointStatus status;
  std::size_t calls;
  std::vector<std::uint8_t> lastApdu;
};

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

TEST(PushListenerEndpoint, StartsClosedAndCloseIsIdempotent)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);

  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(PushListenerEndpoint, OpensAndClosesChannel)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_TRUE(channel.IsOpen());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
}

TEST(PushListenerEndpoint, OpenIsIdempotentWhenAlreadyOpen)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_EQ(1u, channel.openCalls);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_EQ(1u, channel.openCalls);
}

TEST(PushListenerEndpoint, CloseFailureKeepsEndpointOpen)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  channel.closeStatus = dlms::profile::ProfileStatus::WriteFailed;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.Close());
  EXPECT_TRUE(endpoint.IsOpen());
}

TEST(PushListenerEndpoint, RunOnceRequiresOpenEndpoint)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            endpoint.RunOnce());
  EXPECT_EQ(0u, channel.receiveCalls);
  EXPECT_EQ(0u, handler.calls);
}

TEST(PushListenerEndpoint, OpenCanNegotiateAssociationBeforeRunOnce)
{
  FakeApduChannel channel;
  const std::uint8_t pushApdu[] = { 0x0fu, 0x03u, 0x12u, 0x34u };
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(
    std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)));
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());

  ASSERT_EQ(1u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu aare = DecodeAcseResponse(channel.sentFrames[0]);
  ASSERT_EQ(dlms::apdu::AcseApduKind::Aare, aare.kind);
  EXPECT_TRUE(aare.aare.hasResult);
  EXPECT_EQ(0, aare.aare.result);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)),
            handler.lastApdu);
}

TEST(PushListenerEndpoint, RunOnceReleasesNegotiatedAssociation)
{
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(EncodeRlrq());
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  ASSERT_TRUE(endpoint.IsOpen());
  ASSERT_EQ(1u, channel.sentFrames.size());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu response = DecodeAcseResponse(
    channel.sentFrames[1]);
  EXPECT_EQ(dlms::apdu::AcseApduKind::Rlre, response.kind);
  EXPECT_EQ(0u, handler.calls);
}

TEST(PushListenerEndpoint, OpenCanNegotiateLowPasswordAssociationBeforeRunOnce)
{
  const std::uint8_t password[] = {'p', 'w'};
  const std::uint8_t pushApdu[] = {0x0fu, 0x04u, 0x56u, 0x78u};
  const std::vector<std::uint8_t> credential(
    password,
    password + sizeof(password));
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(credential));
  channel.receiveQueue.push_back(
    std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)));
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = password;
  options.security.passwordSize = sizeof(password);

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)),
            handler.lastApdu);
}

TEST(PushListenerEndpoint, RunOnceReleasesNegotiatedLowPasswordAssociation)
{
  const std::uint8_t password[] = {'p', 'w'};
  const std::vector<std::uint8_t> credential(
    password,
    password + sizeof(password));
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(credential));
  channel.receiveQueue.push_back(EncodeRlrq());
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = password;
  options.security.passwordSize = sizeof(password);

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  ASSERT_TRUE(endpoint.IsOpen());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  ASSERT_EQ(2u, channel.sentFrames.size());
  const dlms::apdu::AcseApdu response = DecodeAcseResponse(
    channel.sentFrames[1]);
  EXPECT_EQ(dlms::apdu::AcseApduKind::Rlre, response.kind);
  EXPECT_EQ(0u, handler.calls);
}

TEST(PushListenerEndpoint, OpenRejectsNegotiatedLowPasswordCredentialMismatch)
{
  const std::uint8_t clientPassword[] = {'p', 'w'};
  const std::uint8_t serverPassword[] = {'b', 'a', 'd'};
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(
    std::vector<std::uint8_t>(
      clientPassword,
      clientPassword + sizeof(clientPassword))));
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = serverPassword;
  options.security.passwordSize = sizeof(serverPassword);

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_EQ(0u, handler.calls);
  EXPECT_EQ(0u, channel.sentFrames.size());
}

TEST(PushListenerEndpoint, OpenCanRetryAfterLowPasswordCredentialMismatch)
{
  const std::uint8_t clientPassword[] = {'p', 'w'};
  const std::uint8_t serverPassword[] = {'b', 'a', 'd'};
  const std::uint8_t pushApdu[] = {0x0fu, 0x05u, 0x13u, 0x57u};
  FakeApduChannel channel;
  channel.receiveQueue.push_back(EncodeLlsAarq(
    std::vector<std::uint8_t>(
      clientPassword,
      clientPassword + sizeof(clientPassword))));
  channel.receiveQueue.push_back(EncodeLlsAarq(
    std::vector<std::uint8_t>(
      serverPassword,
      serverPassword + sizeof(serverPassword))));
  channel.receiveQueue.push_back(
    std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)));
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = serverPassword;
  options.security.passwordSize = sizeof(serverPassword);

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_EQ(0u, handler.calls);

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)),
            handler.lastApdu);
}

TEST(PushListenerEndpoint, OpenCanRetryAfterMalformedNegotiation)
{
  const std::uint8_t pushApdu[] = {0x0fu, 0x06u, 0x24u, 0x68u};
  FakeApduChannel channel;
  channel.receiveQueue.push_back(std::vector<std::uint8_t>(1u, 0x00u));
  channel.receiveQueue.push_back(EncodeAarq());
  channel.receiveQueue.push_back(
    std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)));
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_EQ(0u, handler.calls);
  EXPECT_EQ(0u, channel.sentFrames.size());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  EXPECT_TRUE(endpoint.IsOpen());
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());
  EXPECT_EQ(1u, handler.calls);
  EXPECT_EQ(std::vector<std::uint8_t>(pushApdu, pushApdu + sizeof(pushApdu)),
            handler.lastApdu);
  EXPECT_EQ(1u, channel.sentFrames.size());
}

TEST(PushListenerEndpoint, OpenRejectsNegotiatedAssociationWithHighAuthentication)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.negotiateAssociation = true;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighPassword;
  const std::uint8_t password[] = { 'p', 'w' };
  options.security.password = password;
  options.security.passwordSize = sizeof(password);

  dlms::endpoint::PushListenerEndpoint endpoint(channel, options, handler);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_FALSE(channel.IsOpen());
  EXPECT_EQ(0u, handler.calls);
}

TEST(PushListenerEndpoint, RunOnceDispatchesReceivedApduToHandler)
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

TEST(PushListenerEndpoint, RunOnceMapsReceiveFailure)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.receiveStatus = dlms::profile::ProfileStatus::ReadFailed;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.RunOnce());
  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(0u, handler.calls);
}

TEST(PushListenerEndpoint, RunOncePropagatesHandlerFailure)
{
  FakeApduChannel channel;
  RecordingPushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());

  channel.nextReceive.push_back(0x0fu);
  handler.status = dlms::endpoint::EndpointStatus::ServiceFailed;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::ServiceFailed,
            endpoint.RunOnce());
  EXPECT_EQ(1u, channel.receiveCalls);
  EXPECT_EQ(1u, handler.calls);
}
