#include "dlms/association/association_server.hpp"

#include "dlms/apdu/acse.hpp"

namespace dlms {
namespace association {

namespace {

constexpr std::uint8_t kSenderAcseRequirementsTag = 0x8A;
constexpr std::uint8_t kMechanismNameTag = 0x8B;
constexpr std::uint8_t kRespondingApTitleTag = 0xA4;
constexpr std::uint8_t kCallingApTitleTag = 0xA6;
constexpr std::uint8_t kCallingAuthenticationValueTag = 0xAC;
constexpr std::uint8_t kRespondingAuthenticationValueTag = 0xAA;
constexpr std::uint8_t kCharstringAuthenticationValueTag = 0x80;
constexpr std::uint8_t kOctetStringTag = 0x04;
constexpr std::size_t kMaxShortBerAuthenticationValueSize = 125u;

bool IsProfileOk(dlms::profile::ProfileStatus status)
{
  return status == dlms::profile::ProfileStatus::Ok ||
         status == dlms::profile::ProfileStatus::AlreadyOpen;
}

dlms::apdu::ByteView MakeByteView(const std::vector<std::uint8_t>& bytes)
{
  dlms::apdu::ByteView view = {};
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

bool HlsMechanismId(
  HighLevelSecurityMechanism mechanism,
  std::uint8_t& mechanismId)
{
  switch (mechanism) {
  case HighLevelSecurityMechanism::HlsHigh:
    mechanismId = 2u;
    return true;
  case HighLevelSecurityMechanism::HlsMd5:
    mechanismId = 3u;
    return true;
  case HighLevelSecurityMechanism::HlsSha1:
    mechanismId = 4u;
    return true;
  case HighLevelSecurityMechanism::HlsGmac:
    mechanismId = 5u;
    return true;
  case HighLevelSecurityMechanism::Unknown:
    break;
  }
  return false;
}

bool HlsMechanismFromId(
  std::uint8_t mechanismId,
  HighLevelSecurityMechanism& mechanism)
{
  switch (mechanismId) {
  case 2u:
    mechanism = HighLevelSecurityMechanism::HlsHigh;
    return true;
  case 3u:
    mechanism = HighLevelSecurityMechanism::HlsMd5;
    return true;
  case 4u:
    mechanism = HighLevelSecurityMechanism::HlsSha1;
    return true;
  case 5u:
    mechanism = HighLevelSecurityMechanism::HlsGmac;
    return true;
  default:
    mechanism = HighLevelSecurityMechanism::Unknown;
    return false;
  }
}

bool HasAuthenticationFields(const dlms::apdu::AarqApdu& aarq)
{
  for (std::size_t i = 0u; i < aarq.fields.size(); ++i) {
    const std::uint8_t tag = aarq.fields[i].tag;
    if (tag == kSenderAcseRequirementsTag ||
        tag == kMechanismNameTag ||
        tag == kCallingAuthenticationValueTag) {
      return true;
    }
  }
  return false;
}

std::vector<std::uint8_t> MakeSenderAcseRequirementsField()
{
  const std::uint8_t field[] = {
    kSenderAcseRequirementsTag,
    0x02,
    0x07,
    0x80};
  return std::vector<std::uint8_t>(field, field + sizeof(field));
}

std::vector<std::uint8_t> MakeLowLevelSecurityMechanismField()
{
  const std::uint8_t field[] = {
    kMechanismNameTag,
    0x07,
    0x60,
    0x85,
    0x74,
    0x05,
    0x08,
    0x02,
    0x01};
  return std::vector<std::uint8_t>(field, field + sizeof(field));
}

std::vector<std::uint8_t> MakeHighLevelSecurityMechanismField(
  HighLevelSecurityMechanism mechanism)
{
  std::uint8_t mechanismId = 0u;
  HlsMechanismId(mechanism, mechanismId);
  const std::uint8_t field[] = {
    kMechanismNameTag,
    0x07,
    0x60,
    0x85,
    0x74,
    0x05,
    0x08,
    0x02,
    mechanismId};
  return std::vector<std::uint8_t>(field, field + sizeof(field));
}

std::vector<std::uint8_t> MakeAuthenticationValueField(
  std::uint8_t tag,
  const std::vector<std::uint8_t>& value)
{
  std::vector<std::uint8_t> field;
  field.reserve(4u + value.size());
  field.push_back(tag);
  field.push_back(static_cast<std::uint8_t>(value.size() + 2u));
  field.push_back(kCharstringAuthenticationValueTag);
  field.push_back(static_cast<std::uint8_t>(value.size()));
  field.insert(field.end(), value.begin(), value.end());
  return field;
}

std::vector<std::uint8_t> MakeOctetStringField(
  std::uint8_t tag,
  const std::vector<std::uint8_t>& value)
{
  std::vector<std::uint8_t> field;
  field.reserve(4u + value.size());
  field.push_back(tag);
  field.push_back(static_cast<std::uint8_t>(value.size() + 2u));
  field.push_back(kOctetStringTag);
  field.push_back(static_cast<std::uint8_t>(value.size()));
  field.insert(field.end(), value.begin(), value.end());
  return field;
}

bool FindSingleField(
  const dlms::apdu::AarqApdu& aarq,
  std::uint8_t tag,
  const dlms::apdu::AcseRawField*& field)
{
  field = 0;
  for (std::size_t i = 0u; i < aarq.fields.size(); ++i) {
    if (aarq.fields[i].tag != tag) {
      continue;
    }
    if (field != 0) {
      return false;
    }
    field = &aarq.fields[i];
  }
  return field != 0;
}

bool FieldEquals(
  const dlms::apdu::AcseRawField& field,
  const std::uint8_t* expected,
  std::size_t expectedSize)
{
  if (field.encoded.size != expectedSize) {
    return false;
  }
  if (field.encoded.data == 0 && field.encoded.size != 0) {
    return false;
  }
  for (std::size_t i = 0u; i < expectedSize; ++i) {
    if (field.encoded.data[i] != expected[i]) {
      return false;
    }
  }
  return true;
}

bool DecodeHlsMechanism(
  const dlms::apdu::AcseRawField& field,
  HighLevelSecurityMechanism& mechanism)
{
  const std::uint8_t* bytes = field.encoded.data;
  const std::size_t size = field.encoded.size;
  if (bytes == 0 || size != 9u || bytes[0] != kMechanismNameTag ||
      bytes[1] != 0x07 || bytes[2] != 0x60 || bytes[3] != 0x85 ||
      bytes[4] != 0x74 || bytes[5] != 0x05 || bytes[6] != 0x08 ||
      bytes[7] != 0x02) {
    return false;
  }
  return HlsMechanismFromId(bytes[8], mechanism);
}

bool DecodeAuthenticationValue(
  const dlms::apdu::AcseRawField& field,
  std::uint8_t expectedTag,
  std::vector<std::uint8_t>& value)
{
  value.clear();
  const std::uint8_t* bytes = field.encoded.data;
  const std::size_t size = field.encoded.size;
  if (field.tag != expectedTag || bytes == 0 || size < 4u ||
      bytes[0] != expectedTag || bytes[2] != kCharstringAuthenticationValueTag) {
    return false;
  }

  const std::size_t valueSize = bytes[3];
  if (valueSize + 4u != size || valueSize + 2u != bytes[1]) {
    return false;
  }

  value.assign(bytes + 4u, bytes + 4u + valueSize);
  return true;
}

bool DecodeOctetStringField(
  const dlms::apdu::AcseRawField& field,
  std::uint8_t expectedTag,
  std::vector<std::uint8_t>& value)
{
  value.clear();
  const std::uint8_t* bytes = field.encoded.data;
  const std::size_t size = field.encoded.size;
  if (field.tag != expectedTag || bytes == 0 || size < 4u ||
      bytes[0] != expectedTag || bytes[2] != kOctetStringTag) {
    return false;
  }

  const std::size_t valueSize = bytes[3];
  if (valueSize + 4u != size || valueSize + 2u != bytes[1]) {
    return false;
  }

  value.assign(bytes + 4u, bytes + 4u + valueSize);
  return true;
}

bool DecodeCallingAuthenticationValue(
  const dlms::apdu::AcseRawField& field,
  std::vector<std::uint8_t>& credential)
{
  credential.clear();
  const std::uint8_t* bytes = field.encoded.data;
  const std::size_t size = field.encoded.size;
  if (bytes == 0 || size < 4u) {
    return false;
  }
  if (bytes[0] != kCallingAuthenticationValueTag ||
      bytes[2] != kCharstringAuthenticationValueTag) {
    return false;
  }

  const std::size_t credentialSize = bytes[3];
  if (credentialSize + 4u != size ||
      credentialSize + 2u != bytes[1]) {
    return false;
  }

  credential.assign(bytes + 4u, bytes + 4u + credentialSize);
  return true;
}

AssociationStatus ValidateLowLevelSecurityAarq(
  const dlms::apdu::AarqApdu& aarq,
  const std::vector<std::uint8_t>& expectedCredential)
{
  const std::uint8_t expectedRequirements[] = {0x8A, 0x02, 0x07, 0x80};
  const std::uint8_t expectedMechanism[] = {
    0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x01};

  const dlms::apdu::AcseRawField* requirements = 0;
  const dlms::apdu::AcseRawField* mechanism = 0;
  const dlms::apdu::AcseRawField* credentialField = 0;
  if (!FindSingleField(aarq, kSenderAcseRequirementsTag, requirements) ||
      !FindSingleField(aarq, kMechanismNameTag, mechanism) ||
      !FindSingleField(aarq, kCallingAuthenticationValueTag, credentialField)) {
    return AssociationStatus::UnsupportedAuthentication;
  }

  if (!FieldEquals(
        *requirements,
        expectedRequirements,
        sizeof(expectedRequirements)) ||
      !FieldEquals(*mechanism, expectedMechanism, sizeof(expectedMechanism))) {
    return AssociationStatus::UnsupportedAuthentication;
  }

  std::vector<std::uint8_t> credential;
  if (!DecodeCallingAuthenticationValue(*credentialField, credential)) {
    return AssociationStatus::UnsupportedAuthentication;
  }
  return credential == expectedCredential
    ? AssociationStatus::Ok
    : AssociationStatus::UnsupportedAuthentication;
}

AssociationStatus ValidateHighLevelSecurityAarq(
  const dlms::apdu::AarqApdu& aarq,
  const IHighLevelSecurityServerStrategy& strategy,
  const std::vector<std::uint8_t>& callingApplicationTitle,
  HighLevelSecurityMechanism& mechanism,
  std::vector<std::uint8_t>& serverChallenge)
{
  const std::uint8_t expectedRequirements[] = {0x8A, 0x02, 0x07, 0x80};

  const dlms::apdu::AcseRawField* requirements = 0;
  const dlms::apdu::AcseRawField* mechanismField = 0;
  const dlms::apdu::AcseRawField* challengeField = 0;
  if (!FindSingleField(aarq, kSenderAcseRequirementsTag, requirements) ||
      !FindSingleField(aarq, kMechanismNameTag, mechanismField) ||
      !FindSingleField(aarq, kCallingAuthenticationValueTag, challengeField)) {
    return AssociationStatus::UnsupportedAuthentication;
  }

  if (!FieldEquals(
        *requirements,
        expectedRequirements,
        sizeof(expectedRequirements)) ||
      !DecodeHlsMechanism(*mechanismField, mechanism)) {
    return AssociationStatus::UnsupportedAuthentication;
  }

  std::vector<std::uint8_t> clientChallenge;
  if (!DecodeAuthenticationValue(
        *challengeField,
        kCallingAuthenticationValueTag,
        clientChallenge) ||
      clientChallenge.empty()) {
    return AssociationStatus::UnsupportedAuthentication;
  }

  AssociationStatus status =
    strategy.SetCallingApplicationTitle(callingApplicationTitle);
  if (status != AssociationStatus::Ok) {
    return status;
  }

  status = strategy.ValidateInitialChallenge(mechanism, clientChallenge);
  if (status != AssociationStatus::Ok) {
    return status;
  }

  status = strategy.BuildResponseChallenge(mechanism, serverChallenge);
  if (status != AssociationStatus::Ok || serverChallenge.empty()) {
    return status == AssociationStatus::Ok
      ? AssociationStatus::UnsupportedAuthentication
      : status;
  }
  if (serverChallenge.size() > kMaxShortBerAuthenticationValueSize) {
    return AssociationStatus::InvalidArgument;
  }

  return AssociationStatus::Ok;
}

std::vector<std::uint8_t> MakeAssociationResultField(std::int32_t result)
{
  const std::uint8_t field[] = {
    0xA2,
    0x03,
    0x02,
    0x01,
    static_cast<std::uint8_t>(result & 0xff)};
  return std::vector<std::uint8_t>(field, field + sizeof(field));
}

std::vector<std::uint8_t> MakeResultSourceDiagnosticField(
  std::int32_t diagnostic)
{
  const std::uint8_t field[] = {
    0xA3,
    0x05,
    0xA1,
    0x03,
    0x02,
    0x01,
    static_cast<std::uint8_t>(diagnostic & 0xff)};
  return std::vector<std::uint8_t>(field, field + sizeof(field));
}

void AddRawField(
  const std::vector<std::uint8_t>& encoded,
  dlms::apdu::AareApdu& aare)
{
  dlms::apdu::AcseRawField field = {};
  field.tag = encoded.empty() ? 0u : encoded[0];
  field.encoded = MakeByteView(encoded);
  aare.fields.push_back(field);
}

} // namespace

AssociationServer::AssociationServer(
  dlms::profile::IApduChannel& channel,
  const AssociationServerOptions& options)
  : channel_(channel)
  , options_(options)
  , state_(AssociationState::Closed)
  , result_(EmptyAssociationResult())
  , highLevelSecurityMechanism_(HighLevelSecurityMechanism::Unknown)
{
}

AssociationStatus AssociationServer::Open()
{
  if (state_ != AssociationState::Closed) {
    return AssociationStatus::Ok;
  }

  const dlms::profile::ProfileStatus status = channel_.Open();
  if (!IsProfileOk(status)) {
    return AssociationStatus::ChannelOpenFailed;
  }

  result_ = EmptyAssociationResult();
  highLevelSecurityMechanism_ = HighLevelSecurityMechanism::Unknown;
  state_ = AssociationState::Open;
  return AssociationStatus::Ok;
}

AssociationStatus AssociationServer::Close()
{
  if (state_ == AssociationState::Closed) {
    return AssociationStatus::Ok;
  }

  const dlms::profile::ProfileStatus status = channel_.Close();
  if (status != dlms::profile::ProfileStatus::Ok) {
    return AssociationStatus::ChannelCloseFailed;
  }

  result_ = EmptyAssociationResult();
  highLevelSecurityMechanism_ = HighLevelSecurityMechanism::Unknown;
  state_ = AssociationState::Closed;
  return AssociationStatus::Ok;
}

AssociationStatus AssociationServer::Accept()
{
  if (state_ == AssociationState::Closed) {
    return AssociationStatus::InvalidState;
  }

  if (state_ == AssociationState::Associated) {
    return AssociationStatus::AlreadyAssociated;
  }

  const AssociationStatus optionStatus = ValidateOptions();
  if (optionStatus != AssociationStatus::Ok) {
    return optionStatus;
  }

  state_ = AssociationState::Associating;
  result_ = EmptyAssociationResult();

  std::vector<std::uint8_t> aarq;
  const dlms::profile::ProfileStatus receiveStatus =
    channel_.ReceiveApdu(aarq);
  if (receiveStatus != dlms::profile::ProfileStatus::Ok) {
    state_ = AssociationState::Open;
    return AssociationStatus::ReceiveFailed;
  }

  const AssociationStatus decodeStatus = DecodeAarq(aarq);
  if (decodeStatus != AssociationStatus::Ok) {
    state_ = AssociationState::Open;
    return decodeStatus;
  }

  std::vector<std::uint8_t> aare;
  const AssociationStatus buildStatus = BuildAare(aare);
  if (buildStatus != AssociationStatus::Ok) {
    state_ = AssociationState::Open;
    return buildStatus;
  }

  dlms::profile::ProfileByteView view = {
    aare.empty() ? 0 : &aare[0],
    aare.size()};
  const dlms::profile::ProfileStatus sendStatus = channel_.SendApdu(view);
  if (sendStatus != dlms::profile::ProfileStatus::Ok) {
    state_ = AssociationState::Open;
    return AssociationStatus::SendFailed;
  }

  result_.hasAareResult = true;
  result_.aareResult = 0;
  result_.hasAareDiagnostic = true;
  result_.aareDiagnostic = 0x0E;
  result_.negotiatedDlmsVersionNumber = options_.negotiatedDlmsVersionNumber;
  result_.negotiatedConformance = options_.negotiatedConformance;
  result_.serverMaxReceivePduSize = options_.serverMaxReceivePduSize;
  result_.vaaName = options_.vaaName;
  state_ = AssociationState::Associated;
  return AssociationStatus::Ok;
}

AssociationStatus AssociationServer::Release()
{
  if (state_ != AssociationState::Associated) {
    return AssociationStatus::InvalidState;
  }

  std::vector<std::uint8_t> rlrq;
  const dlms::profile::ProfileStatus receiveStatus =
    channel_.ReceiveApdu(rlrq);
  if (receiveStatus != dlms::profile::ProfileStatus::Ok) {
    return AssociationStatus::ReceiveFailed;
  }

  return Release(rlrq);
}

AssociationStatus AssociationServer::Release(
  const std::vector<std::uint8_t>& rlrq)
{
  if (state_ != AssociationState::Associated) {
    return AssociationStatus::InvalidState;
  }

  const AssociationStatus decodeStatus = DecodeRlrq(rlrq);
  if (decodeStatus != AssociationStatus::Ok) {
    return decodeStatus;
  }

  return SendRlreAndClose();
}

AssociationStatus AssociationServer::SendRlreAndClose()
{
  if (state_ != AssociationState::Associated) {
    return AssociationStatus::InvalidState;
  }

  std::vector<std::uint8_t> rlre;
  const AssociationStatus buildStatus = BuildRlre(rlre);
  if (buildStatus != AssociationStatus::Ok) {
    return buildStatus;
  }

  dlms::profile::ProfileByteView view = {
    rlre.empty() ? 0 : &rlre[0],
    rlre.size()};
  const dlms::profile::ProfileStatus sendStatus = channel_.SendApdu(view);
  if (sendStatus != dlms::profile::ProfileStatus::Ok) {
    return AssociationStatus::SendFailed;
  }

  const dlms::profile::ProfileStatus closeStatus = channel_.Close();
  if (closeStatus != dlms::profile::ProfileStatus::Ok) {
    return AssociationStatus::ChannelCloseFailed;
  }

  result_ = EmptyAssociationResult();
  state_ = AssociationState::Closed;
  return AssociationStatus::Ok;
}

AssociationState AssociationServer::State() const
{
  return state_;
}

bool AssociationServer::IsAssociated() const
{
  return state_ == AssociationState::Associated;
}

const AssociationResult& AssociationServer::Result() const
{
  return result_;
}

AssociationStatus AssociationServer::ValidateOptions() const
{
  if (options_.applicationContext != ApplicationContext::LogicalNameNoCiphering) {
    return AssociationStatus::UnsupportedApplicationContext;
  }

  if (options_.authenticationMode == AuthenticationMode::HighLevelSecurity) {
    if (options_.highLevelSecurity == 0) {
      return AssociationStatus::UnsupportedAuthentication;
    }
  }

  if (options_.authenticationMode == AuthenticationMode::LowLevelSecurity &&
      options_.lowLevelSecurityCredential.empty()) {
    return AssociationStatus::UnsupportedAuthentication;
  }

  if (!options_.respondingApplicationTitle.empty() &&
      options_.respondingApplicationTitle.size() >
        kMaxShortBerAuthenticationValueSize) {
    return AssociationStatus::InvalidArgument;
  }

  if (options_.negotiatedDlmsVersionNumber == 0 ||
      options_.serverMaxReceivePduSize == 0) {
    return AssociationStatus::InvalidArgument;
  }

  return AssociationStatus::Ok;
}

AssociationStatus AssociationServer::DecodeAarq(
  const std::vector<std::uint8_t>& input)
{
  if (input.empty()) {
    return AssociationStatus::DecodeFailed;
  }

  dlms::apdu::AcseApdu apdu = {};
  const dlms::apdu::ApduStatus status =
    dlms::apdu::DecodeAcseApdu(&input[0], input.size(), apdu);
  if (status != dlms::apdu::ApduStatus::Ok ||
      apdu.kind != dlms::apdu::AcseApduKind::Aarq) {
    return AssociationStatus::DecodeFailed;
  }

  if (options_.authenticationMode == AuthenticationMode::None &&
      HasAuthenticationFields(apdu.aarq)) {
    return AssociationStatus::UnsupportedAuthentication;
  }

  result_.callingApplicationTitle.clear();
  for (std::size_t i = 0u; i < apdu.aarq.fields.size(); ++i) {
    if (apdu.aarq.fields[i].tag == kCallingApTitleTag) {
      std::vector<std::uint8_t> title;
      if (!DecodeOctetStringField(
            apdu.aarq.fields[i],
            kCallingApTitleTag,
            title)) {
        return AssociationStatus::DecodeFailed;
      }
      result_.callingApplicationTitle = title;
      break;
    }
  }

  if (options_.authenticationMode == AuthenticationMode::LowLevelSecurity) {
    const AssociationStatus authenticationStatus =
      ValidateLowLevelSecurityAarq(
        apdu.aarq,
        options_.lowLevelSecurityCredential);
    if (authenticationStatus != AssociationStatus::Ok) {
      return authenticationStatus;
    }
  }

  if (options_.authenticationMode == AuthenticationMode::HighLevelSecurity) {
    const AssociationStatus authenticationStatus =
      ValidateHighLevelSecurityAarq(
        apdu.aarq,
        *options_.highLevelSecurity,
        result_.callingApplicationTitle,
        highLevelSecurityMechanism_,
        result_.highLevelSecurityServerChallenge);
    if (authenticationStatus != AssociationStatus::Ok) {
      return authenticationStatus;
    }
  }

  if (apdu.aarq.initiateRequest.proposedDlmsVersionNumber == 0 ||
      apdu.aarq.initiateRequest.clientMaxReceivePduSize == 0) {
    return AssociationStatus::NegotiationFailed;
  }

  return AssociationStatus::Ok;
}

AssociationStatus AssociationServer::BuildAare(
  std::vector<std::uint8_t>& output) const
{
  dlms::apdu::AcseApdu aare = {};
  aare.kind = dlms::apdu::AcseApduKind::Aare;
  aare.aare.hasResult = true;
  aare.aare.result = 0;
  aare.aare.hasDiagnostic = true;
  aare.aare.diagnostic = 0x0E;
  aare.aare.initiateResponse.hasNegotiatedQualityOfService = false;
  aare.aare.initiateResponse.negotiatedQualityOfService = 0;
  aare.aare.initiateResponse.negotiatedDlmsVersionNumber =
    options_.negotiatedDlmsVersionNumber;
  aare.aare.initiateResponse.negotiatedConformance =
    options_.negotiatedConformance;
  aare.aare.initiateResponse.serverMaxReceivePduSize =
    options_.serverMaxReceivePduSize;
  aare.aare.initiateResponse.vaaName = options_.vaaName;

  std::vector<std::vector<std::uint8_t> > encodedFields;
  encodedFields.push_back(MakeAssociationResultField(aare.aare.result));
  encodedFields.push_back(
    MakeResultSourceDiagnosticField(aare.aare.diagnostic));
  if (!options_.respondingApplicationTitle.empty()) {
    encodedFields.push_back(
      MakeOctetStringField(
        kRespondingApTitleTag,
        options_.respondingApplicationTitle));
  }
  if (options_.authenticationMode == AuthenticationMode::HighLevelSecurity) {
    encodedFields.push_back(MakeSenderAcseRequirementsField());
    encodedFields.push_back(
      MakeHighLevelSecurityMechanismField(highLevelSecurityMechanism_));
    encodedFields.push_back(
      MakeAuthenticationValueField(
        kRespondingAuthenticationValueTag,
        result_.highLevelSecurityServerChallenge));
  }
  AddRawField(encodedFields[0], aare.aare);
  AddRawField(encodedFields[1], aare.aare);
  for (std::size_t i = 2u; i < encodedFields.size(); ++i) {
    AddRawField(encodedFields[i], aare.aare);
  }

  const dlms::apdu::ApduStatus status =
    dlms::apdu::EncodeAcseApdu(aare, output);
  return status == dlms::apdu::ApduStatus::Ok
    ? AssociationStatus::Ok
    : AssociationStatus::EncodeFailed;
}

AssociationStatus AssociationServer::DecodeRlrq(
  const std::vector<std::uint8_t>& input) const
{
  if (input.empty()) {
    return AssociationStatus::DecodeFailed;
  }

  dlms::apdu::AcseApdu apdu = {};
  const dlms::apdu::ApduStatus status =
    dlms::apdu::DecodeAcseApdu(&input[0], input.size(), apdu);
  if (status != dlms::apdu::ApduStatus::Ok ||
      apdu.kind != dlms::apdu::AcseApduKind::Rlrq) {
    return AssociationStatus::DecodeFailed;
  }

  return AssociationStatus::Ok;
}

AssociationStatus AssociationServer::BuildRlre(
  std::vector<std::uint8_t>& output) const
{
  dlms::apdu::AcseApdu rlre = {};
  rlre.kind = dlms::apdu::AcseApduKind::Rlre;
  const dlms::apdu::ApduStatus status =
    dlms::apdu::EncodeAcseApdu(rlre, output);
  return status == dlms::apdu::ApduStatus::Ok
    ? AssociationStatus::Ok
    : AssociationStatus::EncodeFailed;
}

} // namespace association
} // namespace dlms
