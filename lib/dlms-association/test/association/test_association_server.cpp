#include "dlms/association/association_server.hpp"

#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/initiate.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/profile/profile_types.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

class FakeApduChannel : public dlms::profile::IApduChannel
{
public:
  FakeApduChannel()
    : openStatus(dlms::profile::ProfileStatus::Ok)
    , closeStatus(dlms::profile::ProfileStatus::Ok)
    , sendStatus(dlms::profile::ProfileStatus::Ok)
    , receiveStatus(dlms::profile::ProfileStatus::Ok)
    , open(false)
    , openCalls(0)
    , closeCalls(0)
    , sendCalls(0)
    , receiveCalls(0)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    ++openCalls;
    if (openStatus == dlms::profile::ProfileStatus::Ok ||
        openStatus == dlms::profile::ProfileStatus::AlreadyOpen) {
      open = true;
    }
    return openStatus;
  }

  dlms::profile::ProfileStatus Close()
  {
    ++closeCalls;
    if (closeStatus == dlms::profile::ProfileStatus::Ok) {
      open = false;
    }
    return closeStatus;
  }

  bool IsOpen() const
  {
    return open;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++sendCalls;
    sent.assign(apdu.data, apdu.data + apdu.size);
    return sendStatus;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    ++receiveCalls;
    if (receiveStatus == dlms::profile::ProfileStatus::Ok) {
      apdu = nextReceive;
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

    for (std::size_t i = 0; i < nextReceive.size(); ++i) {
      output.data[i] = nextReceive[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = nextReceive.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus openStatus;
  dlms::profile::ProfileStatus closeStatus;
  dlms::profile::ProfileStatus sendStatus;
  dlms::profile::ProfileStatus receiveStatus;
  bool open;
  int openCalls;
  int closeCalls;
  int sendCalls;
  int receiveCalls;
  std::vector<std::uint8_t> sent;
  std::vector<std::uint8_t> nextReceive;
};

class FakeHlsServerStrategy
  : public dlms::association::IHighLevelSecurityServerStrategy
{
public:
  FakeHlsServerStrategy()
    : validateStatus(dlms::association::AssociationStatus::Ok)
    , buildStatus(dlms::association::AssociationStatus::Ok)
    , validateCalls(0)
    , buildCalls(0)
    , lastMechanism(dlms::association::HighLevelSecurityMechanism::Unknown)
  {
    responseChallenge.push_back(0xA1);
    responseChallenge.push_back(0xA2);
  }

  dlms::association::AssociationStatus SetCallingApplicationTitle(
    const std::vector<std::uint8_t>& title) const
  {
    lastCallingApplicationTitle = title;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus ValidateInitialChallenge(
    dlms::association::HighLevelSecurityMechanism mechanism,
    const std::vector<std::uint8_t>& clientChallenge) const
  {
    ++validateCalls;
    lastMechanism = mechanism;
    lastClientChallenge = clientChallenge;
    return validateStatus;
  }

  dlms::association::AssociationStatus BuildResponseChallenge(
    dlms::association::HighLevelSecurityMechanism mechanism,
    std::vector<std::uint8_t>& output) const
  {
    ++buildCalls;
    lastMechanism = mechanism;
    output = responseChallenge;
    return buildStatus;
  }

  dlms::association::AssociationStatus validateStatus;
  dlms::association::AssociationStatus buildStatus;
  std::vector<std::uint8_t> responseChallenge;
  mutable int validateCalls;
  mutable int buildCalls;
  mutable dlms::association::HighLevelSecurityMechanism lastMechanism;
  mutable std::vector<std::uint8_t> lastClientChallenge;
  mutable std::vector<std::uint8_t> lastCallingApplicationTitle;
};

dlms::apdu::ByteView MakeByteView(const std::vector<std::uint8_t>& bytes)
{
  dlms::apdu::ByteView view = {};
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

std::vector<std::uint8_t> MakeAarqBytes()
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

std::vector<std::uint8_t> MakeAarqWithAuthenticationBytes()
{
  dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::XdlmsApdu xdlms(request);
  dlms::apdu::AcseApdu aarq =
    dlms::apdu::MakeAarqWithInitiateRequest(xdlms);

  std::vector<std::uint8_t> authField;
  authField.push_back(0xAC);
  authField.push_back(0x03);
  authField.push_back(0x80);
  authField.push_back(0x01);
  authField.push_back('p');
  dlms::apdu::AcseRawField field = {};
  field.tag = authField[0];
  field.encoded = MakeByteView(authField);
  aarq.aarq.fields.push_back(field);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(aarq, output));
  return output;
}

std::vector<std::uint8_t> MakeLlsAarqBytes(
  const std::vector<std::uint8_t>& credential)
{
  dlms::apdu::InitiateRequest request =
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

  dlms::apdu::AcseRawField requirementsField = {};
  requirementsField.tag = requirements[0];
  requirementsField.encoded.data = requirements;
  requirementsField.encoded.size = sizeof(requirements);
  aarq.aarq.fields.push_back(requirementsField);

  dlms::apdu::AcseRawField mechanismField = {};
  mechanismField.tag = mechanism[0];
  mechanismField.encoded.data = mechanism;
  mechanismField.encoded.size = sizeof(mechanism);
  aarq.aarq.fields.push_back(mechanismField);

  dlms::apdu::AcseRawField credentialRawField = {};
  credentialRawField.tag = credentialField[0];
  credentialRawField.encoded = MakeByteView(credentialField);
  aarq.aarq.fields.push_back(credentialRawField);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(aarq, output));
  return output;
}

std::vector<std::uint8_t> MakeHlsAarqBytes(
  std::uint8_t mechanismId,
  const std::vector<std::uint8_t>& challenge)
{
  dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::XdlmsApdu xdlms(request);
  dlms::apdu::AcseApdu aarq =
    dlms::apdu::MakeAarqWithInitiateRequest(xdlms);

  const std::uint8_t requirements[] = {0x8A, 0x02, 0x07, 0x80};
  const std::uint8_t mechanism[] = {
    0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, mechanismId};

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
  field.encoded = MakeByteView(challengeField);
  aarq.aarq.fields.push_back(field);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(aarq, output));
  return output;
}

std::vector<std::uint8_t> MakeHlsAarqWithCallingTitleBytes(
  std::uint8_t mechanismId,
  const std::vector<std::uint8_t>& challenge,
  const std::vector<std::uint8_t>& title)
{
  dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();
  const dlms::apdu::XdlmsApdu xdlms(request);
  dlms::apdu::AcseApdu aarq =
    dlms::apdu::MakeAarqWithInitiateRequest(xdlms);

  const std::uint8_t requirements[] = {0x8A, 0x02, 0x07, 0x80};
  const std::uint8_t mechanism[] = {
    0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, mechanismId};

  std::vector<std::uint8_t> titleField;
  titleField.push_back(0xA6);
  titleField.push_back(static_cast<std::uint8_t>(title.size() + 2u));
  titleField.push_back(0x04);
  titleField.push_back(static_cast<std::uint8_t>(title.size()));
  titleField.insert(titleField.end(), title.begin(), title.end());

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
  field.encoded = MakeByteView(titleField);
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
  field.encoded = MakeByteView(challengeField);
  aarq.aarq.fields.push_back(field);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(aarq, output));
  return output;
}

std::vector<std::uint8_t> FieldBytes(
  const dlms::apdu::AareApdu& aare,
  std::uint8_t tag)
{
  for (std::size_t i = 0u; i < aare.fields.size(); ++i) {
    if (aare.fields[i].tag == tag) {
      return std::vector<std::uint8_t>(
        aare.fields[i].encoded.data,
        aare.fields[i].encoded.data + aare.fields[i].encoded.size);
    }
  }
  return std::vector<std::uint8_t>();
}

std::vector<std::uint8_t> MakeRlrqBytes()
{
  const dlms::apdu::AcseApdu rlrq = dlms::apdu::MakeRlrq();

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeAcseApdu(rlrq, output));
  return output;
}

} // namespace

TEST(AssociationServer, AcceptRequiresOpenChannel)
{
  FakeApduChannel channel;
  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  EXPECT_EQ(dlms::association::AssociationStatus::InvalidState,
            server.Accept());
  EXPECT_EQ(dlms::association::AssociationState::Closed, server.State());
}

TEST(AssociationServer, SuccessfulAcceptReceivesAarqSendsAareAndStoresResult)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());

  EXPECT_TRUE(server.IsAssociated());
  EXPECT_EQ(1, channel.receiveCalls);
  EXPECT_EQ(1, channel.sendCalls);

  dlms::apdu::AcseApdu sent = {};
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeAcseApdu(
              &channel.sent[0],
              channel.sent.size(),
              sent));
  ASSERT_EQ(dlms::apdu::AcseApduKind::Aare, sent.kind);
  EXPECT_TRUE(sent.aare.hasResult);
  EXPECT_EQ(0, sent.aare.result);
  EXPECT_TRUE(sent.aare.hasDiagnostic);
  EXPECT_EQ(0x0E, sent.aare.diagnostic);
  EXPECT_EQ(6u,
            sent.aare.initiateResponse.negotiatedDlmsVersionNumber);
  EXPECT_EQ(0x0200u,
            sent.aare.initiateResponse.serverMaxReceivePduSize);
  EXPECT_EQ(0x0007u, sent.aare.initiateResponse.vaaName);

  const dlms::association::AssociationResult& result = server.Result();
  EXPECT_EQ(6u, result.negotiatedDlmsVersionNumber);
  EXPECT_EQ(0x0200u, result.serverMaxReceivePduSize);
  EXPECT_EQ(0x0007u, result.vaaName);
  EXPECT_TRUE(result.hasAareResult);
  EXPECT_EQ(0, result.aareResult);
}

TEST(AssociationServer, ReceiveFailureReturnsReceiveFailed)
{
  FakeApduChannel channel;
  channel.receiveStatus = dlms::profile::ProfileStatus::Timeout;

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::ReceiveFailed,
            server.Accept());
  EXPECT_EQ(dlms::association::AssociationState::Open, server.State());
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, MalformedAarqReturnsDecodeFailed)
{
  FakeApduChannel channel;
  channel.nextReceive.push_back(0x00);

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::DecodeFailed,
            server.Accept());
  EXPECT_EQ(dlms::association::AssociationState::Open, server.State());
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, AuthenticatedAarqReturnsUnsupportedAuthentication)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqWithAuthenticationBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::UnsupportedAuthentication,
            server.Accept());
  EXPECT_EQ(dlms::association::AssociationState::Open, server.State());
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, LowLevelSecurityAcceptsMatchingCredential)
{
  const std::uint8_t credentialBytes[] = {'p', 'w'};
  const std::vector<std::uint8_t> credential(
    credentialBytes,
    credentialBytes + sizeof(credentialBytes));

  FakeApduChannel channel;
  channel.nextReceive = MakeLlsAarqBytes(credential);
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::LowLevelSecurity;
  options.lowLevelSecurityCredential = credential;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());
  EXPECT_TRUE(server.IsAssociated());
  EXPECT_EQ(1, channel.receiveCalls);
  EXPECT_EQ(1, channel.sendCalls);
}

TEST(AssociationServer, LowLevelSecurityRejectsMismatchedCredential)
{
  const std::uint8_t clientCredentialBytes[] = {'p', 'w'};
  const std::uint8_t serverCredentialBytes[] = {'b', 'a', 'd'};
  FakeApduChannel channel;
  channel.nextReceive = MakeLlsAarqBytes(
    std::vector<std::uint8_t>(
      clientCredentialBytes,
      clientCredentialBytes + sizeof(clientCredentialBytes)));
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::LowLevelSecurity;
  options.lowLevelSecurityCredential.assign(
    serverCredentialBytes,
    serverCredentialBytes + sizeof(serverCredentialBytes));

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::UnsupportedAuthentication,
            server.Accept());
  EXPECT_EQ(dlms::association::AssociationState::Open, server.State());
  EXPECT_EQ(1, channel.receiveCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, LowLevelSecurityWithoutServerCredentialIsRejected)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeLlsAarqBytes(std::vector<std::uint8_t>(1u, 'p'));
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::LowLevelSecurity;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::UnsupportedAuthentication,
            server.Accept());
  EXPECT_EQ(0, channel.receiveCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, HighLevelSecurityServerOptionIsRejected)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeHlsAarqBytes(
    0x05,
    std::vector<std::uint8_t>(1u, 0xC1));
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::UnsupportedAuthentication,
            server.Accept());
  EXPECT_EQ(0, channel.receiveCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, HighLevelSecurityAcceptsStrategyChallenge)
{
  const std::uint8_t clientChallengeBytes[] = {0xC1, 0xC2};
  const std::vector<std::uint8_t> clientChallenge(
    clientChallengeBytes,
    clientChallengeBytes + sizeof(clientChallengeBytes));
  FakeApduChannel channel;
  channel.nextReceive = MakeHlsAarqBytes(0x05, clientChallenge);
  FakeHlsServerStrategy hls;
  hls.responseChallenge.clear();
  hls.responseChallenge.push_back(0xD1);
  hls.responseChallenge.push_back(0xD2);
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;
  options.highLevelSecurity = &hls;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());
  EXPECT_TRUE(server.IsAssociated());
  EXPECT_EQ(1, hls.validateCalls);
  EXPECT_EQ(1, hls.buildCalls);
  EXPECT_EQ(dlms::association::HighLevelSecurityMechanism::HlsGmac,
            hls.lastMechanism);
  EXPECT_EQ(clientChallenge, hls.lastClientChallenge);
  EXPECT_EQ(hls.responseChallenge,
            server.Result().highLevelSecurityServerChallenge);

  dlms::apdu::AcseApdu sent = {};
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeAcseApdu(
              &channel.sent[0],
              channel.sent.size(),
              sent));
  ASSERT_EQ(dlms::apdu::AcseApduKind::Aare, sent.kind);

  const std::uint8_t expectedRequirements[] = {0x8A, 0x02, 0x07, 0x80};
  const std::uint8_t expectedMechanism[] = {
    0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x05};
  const std::uint8_t expectedChallenge[] = {
    0xAA, 0x04, 0x80, 0x02, 0xD1, 0xD2};
  EXPECT_EQ(
    std::vector<std::uint8_t>(
      expectedRequirements,
      expectedRequirements + sizeof(expectedRequirements)),
    FieldBytes(sent.aare, 0x8A));
  EXPECT_EQ(
    std::vector<std::uint8_t>(
      expectedMechanism,
      expectedMechanism + sizeof(expectedMechanism)),
    FieldBytes(sent.aare, 0x8B));
  EXPECT_EQ(
    std::vector<std::uint8_t>(
      expectedChallenge,
      expectedChallenge + sizeof(expectedChallenge)),
    FieldBytes(sent.aare, 0xAA));
}

TEST(AssociationServer, HighLevelSecurityStoresCallingAndRespondingTitles)
{
  const std::uint8_t clientTitleBytes[] =
    {'C', 'L', 'I', 'T', 'I', 'T', 'L', 'E'};
  const std::uint8_t serverTitleBytes[] =
    {'S', 'R', 'V', 'T', 'I', 'T', 'L', 'E'};
  const std::vector<std::uint8_t> clientTitle(
    clientTitleBytes,
    clientTitleBytes + sizeof(clientTitleBytes));
  const std::vector<std::uint8_t> serverTitle(
    serverTitleBytes,
    serverTitleBytes + sizeof(serverTitleBytes));
  std::vector<std::uint8_t> clientChallenge;
  clientChallenge.push_back(0xC1);
  clientChallenge.push_back(0xC2);
  FakeApduChannel channel;
  channel.nextReceive =
    MakeHlsAarqWithCallingTitleBytes(0x05, clientChallenge, clientTitle);
  FakeHlsServerStrategy strategy;
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;
  options.highLevelSecurity = &strategy;
  options.respondingApplicationTitle = serverTitle;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());
  EXPECT_EQ(clientTitle, server.Result().callingApplicationTitle);
  EXPECT_EQ(clientTitle, strategy.lastCallingApplicationTitle);

  dlms::apdu::AcseApdu aare = {};
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeAcseApdu(
              &channel.sent[0],
              channel.sent.size(),
              aare));
  const std::uint8_t expectedTitle[] = {
    0xA4, 0x0A, 0x04, 0x08,
    'S', 'R', 'V', 'T', 'I', 'T', 'L', 'E'};
  EXPECT_EQ(std::vector<std::uint8_t>(expectedTitle,
                                      expectedTitle + sizeof(expectedTitle)),
            FieldBytes(aare.aare, 0xA4));
}

TEST(AssociationServer, HighLevelSecurityRejectsUnknownMechanism)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeHlsAarqBytes(
    0x7f,
    std::vector<std::uint8_t>(1u, 0xC1));
  FakeHlsServerStrategy hls;
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;
  options.highLevelSecurity = &hls;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::UnsupportedAuthentication,
            server.Accept());
  EXPECT_EQ(0, hls.validateCalls);
  EXPECT_EQ(0, hls.buildCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, HighLevelSecurityRejectsEmptyClientChallenge)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeHlsAarqBytes(0x05, std::vector<std::uint8_t>());
  FakeHlsServerStrategy hls;
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;
  options.highLevelSecurity = &hls;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::UnsupportedAuthentication,
            server.Accept());
  EXPECT_EQ(0, hls.validateCalls);
  EXPECT_EQ(0, hls.buildCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, HighLevelSecurityStrategyFailureIsRejected)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeHlsAarqBytes(
    0x05,
    std::vector<std::uint8_t>(1u, 0xC1));
  FakeHlsServerStrategy hls;
  hls.validateStatus = dlms::association::AssociationStatus::InternalError;
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;
  options.highLevelSecurity = &hls;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::InternalError,
            server.Accept());
  EXPECT_EQ(1, hls.validateCalls);
  EXPECT_EQ(0, hls.buildCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, HighLevelSecurityRejectsInvalidServerChallenge)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeHlsAarqBytes(
    0x05,
    std::vector<std::uint8_t>(1u, 0xC1));
  FakeHlsServerStrategy hls;
  hls.responseChallenge.clear();
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;
  options.highLevelSecurity = &hls;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::UnsupportedAuthentication,
            server.Accept());
  EXPECT_EQ(1, hls.validateCalls);
  EXPECT_EQ(1, hls.buildCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, HighLevelSecurityRejectsOversizeServerChallenge)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeHlsAarqBytes(
    0x05,
    std::vector<std::uint8_t>(1u, 0xC1));
  FakeHlsServerStrategy hls;
  hls.responseChallenge.assign(126u, 0xD1);
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  options.authenticationMode =
    dlms::association::AuthenticationMode::HighLevelSecurity;
  options.highLevelSecurity = &hls;

  dlms::association::AssociationServer server(channel, options);

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::InvalidArgument,
            server.Accept());
  EXPECT_EQ(1, hls.validateCalls);
  EXPECT_EQ(1, hls.buildCalls);
  EXPECT_EQ(0, channel.sendCalls);
}

TEST(AssociationServer, CloseReturnsToClosed)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());
  EXPECT_EQ(dlms::association::AssociationStatus::Ok, server.Close());
  EXPECT_EQ(dlms::association::AssociationState::Closed, server.State());
  EXPECT_FALSE(server.IsAssociated());
}

TEST(AssociationServer, ReleaseRequiresAssociatedState)
{
  FakeApduChannel channel;

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  EXPECT_EQ(dlms::association::AssociationStatus::InvalidState,
            server.Release());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  EXPECT_EQ(dlms::association::AssociationStatus::InvalidState,
            server.Release());
}

TEST(AssociationServer, SuccessfulReleaseReceivesRlrqSendsRlreAndCloses)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());

  channel.nextReceive = MakeRlrqBytes();
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Release());

  EXPECT_EQ(dlms::association::AssociationState::Closed, server.State());
  EXPECT_FALSE(server.IsAssociated());
  EXPECT_EQ(2, channel.receiveCalls);
  EXPECT_EQ(2, channel.sendCalls);
  EXPECT_EQ(1, channel.closeCalls);
  EXPECT_FALSE(channel.open);
  EXPECT_EQ(0u, server.Result().serverMaxReceivePduSize);

  dlms::apdu::AcseApdu sent = {};
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeAcseApdu(
              &channel.sent[0],
              channel.sent.size(),
              sent));
  EXPECT_EQ(dlms::apdu::AcseApduKind::Rlre, sent.kind);
}

TEST(AssociationServer, SuccessfulReleaseCanUseAlreadyReceivedRlrq)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());

  const std::vector<std::uint8_t> rlrq = MakeRlrqBytes();
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Release(rlrq));

  EXPECT_EQ(dlms::association::AssociationState::Closed, server.State());
  EXPECT_FALSE(server.IsAssociated());
  EXPECT_EQ(1, channel.receiveCalls);
  EXPECT_EQ(2, channel.sendCalls);
  EXPECT_EQ(1, channel.closeCalls);

  dlms::apdu::AcseApdu sent = {};
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeAcseApdu(
              &channel.sent[0],
              channel.sent.size(),
              sent));
  EXPECT_EQ(dlms::apdu::AcseApduKind::Rlre, sent.kind);
}

TEST(AssociationServer, MalformedReleaseRequestLeavesAssociated)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());

  channel.nextReceive.clear();
  channel.nextReceive.push_back(0x63);
  EXPECT_EQ(dlms::association::AssociationStatus::DecodeFailed,
            server.Release());
  EXPECT_EQ(dlms::association::AssociationState::Associated, server.State());
  EXPECT_TRUE(server.IsAssociated());
}

TEST(AssociationServer, ReleaseReceiveFailureLeavesAssociated)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());

  channel.receiveStatus = dlms::profile::ProfileStatus::Timeout;
  EXPECT_EQ(dlms::association::AssociationStatus::ReceiveFailed,
            server.Release());
  EXPECT_EQ(dlms::association::AssociationState::Associated, server.State());
  EXPECT_TRUE(server.IsAssociated());
}

TEST(AssociationServer, ReleaseSendFailureLeavesAssociated)
{
  FakeApduChannel channel;
  channel.nextReceive = MakeAarqBytes();

  dlms::association::AssociationServer server(
    channel,
    dlms::association::DefaultAssociationServerOptions());

  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, server.Accept());

  channel.nextReceive = MakeRlrqBytes();
  channel.sendStatus = dlms::profile::ProfileStatus::WriteFailed;
  EXPECT_EQ(dlms::association::AssociationStatus::SendFailed,
            server.Release());
  EXPECT_EQ(dlms::association::AssociationState::Associated, server.State());
  EXPECT_TRUE(server.IsAssociated());
}
