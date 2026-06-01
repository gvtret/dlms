#include "dlms/endpoint/client_endpoint.hpp"

namespace {

std::vector<std::uint8_t> CopyBytes(
  const std::uint8_t* input,
  std::size_t inputSize)
{
  if (input == 0 || inputSize == 0u) {
    return std::vector<std::uint8_t>();
  }
  return std::vector<std::uint8_t>(input, input + inputSize);
}

} // namespace

namespace dlms {
namespace endpoint {

ClientEndpoint::ClientEndpoint(const ClientEndpointOptions& options)
  : options_(options)
  , host_(options.transport.host == 0 ? "" : options.transport.host)
  , serialDevice_(
      options.transport.serialDevice == 0 ? "" : options.transport.serialDevice)
  , password_(
      CopyBytes(options.security.password, options.security.passwordSize))
  , systemTitle_(
      CopyBytes(options.security.systemTitle, options.security.systemTitleSize))
  , peerSystemTitle_(
      CopyBytes(
        options.security.peerSystemTitle,
        options.security.peerSystemTitleSize))
  , globalUnicastEncryptionKey_(
      CopyBytes(
        options.security.globalUnicastEncryptionKey,
        options.security.globalUnicastEncryptionKeySize))
  , authenticationKey_(
      CopyBytes(
        options.security.authenticationKey,
        options.security.authenticationKeySize))
{
  options_.transport.host = host_.c_str();
  options_.transport.serialDevice = serialDevice_.c_str();
  options_.security.password = password_.empty() ? 0 : &password_[0];
  options_.security.passwordSize = password_.size();
  options_.security.systemTitle =
    systemTitle_.empty() ? 0 : &systemTitle_[0];
  options_.security.systemTitleSize = systemTitle_.size();
  options_.security.peerSystemTitle =
    peerSystemTitle_.empty() ? 0 : &peerSystemTitle_[0];
  options_.security.peerSystemTitleSize = peerSystemTitle_.size();
  options_.security.globalUnicastEncryptionKey =
    globalUnicastEncryptionKey_.empty()
      ? 0
      : &globalUnicastEncryptionKey_[0];
  options_.security.globalUnicastEncryptionKeySize =
    globalUnicastEncryptionKey_.size();
  options_.security.authenticationKey =
    authenticationKey_.empty() ? 0 : &authenticationKey_[0];
  options_.security.authenticationKeySize = authenticationKey_.size();
}

ClientEndpoint::~ClientEndpoint()
{
  Close();
}

EndpointStatus ClientEndpoint::MakeClientOptions(
  dlms::client::DlmsClientOptions& output) const
{
  EndpointStatus status = ValidateClientEndpointOptions(options_);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  if (options_.transport.kind != EndpointTransportKind::Tcp) {
    return EndpointStatus::UnsupportedProfile;
  }

  output = dlms::client::DefaultDlmsClientOptions();
  output.clientSap = options_.profile.clientSap;
  output.serverSap = options_.profile.serverSap;
  output.connectTimeoutMs = options_.transport.timeoutMs;
  output.requestTimeoutMs = options_.transport.timeoutMs;

  switch (options_.profile.kind) {
    case EndpointProfileKind::Wrapper:
      output.profile = dlms::client::ClientProfile::WrapperTcp;
      output.wrapperTcp.host = options_.transport.host;
      output.wrapperTcp.port = options_.transport.port;
      output.wrapperTcp.sourceWPort = options_.profile.clientSap;
      output.wrapperTcp.destinationWPort = options_.profile.serverSap;
      break;
    case EndpointProfileKind::Hdlc:
      output.profile = dlms::client::ClientProfile::HdlcTcp;
      output.hdlcTcp.host = options_.transport.host;
      output.hdlcTcp.port = options_.transport.port;
      output.hdlcTcp.clientAddress =
        static_cast<std::uint8_t>(options_.profile.clientSap);
      output.hdlcTcp.logicalDeviceAddress = options_.profile.serverSap;
      output.hdlcTcp.useDataLinkSession = options_.profile.hdlcUseSession;
      break;
    default:
      return EndpointStatus::UnsupportedProfile;
  }

  switch (options_.security.authentication) {
    case EndpointAuthenticationKind::None:
      output.authenticationMode =
        dlms::client::ClientAuthenticationMode::None;
      break;
    case EndpointAuthenticationKind::LowPassword:
      output.authenticationMode =
        dlms::client::ClientAuthenticationMode::LowLevelSecurity;
      output.lowLevelSecurity.credential =
        password_.empty() ? 0 : &password_[0];
      output.lowLevelSecurity.credentialSize = password_.size();
      break;
    case EndpointAuthenticationKind::HighPassword:
      output.authenticationMode =
        dlms::client::ClientAuthenticationMode::HighLevelSecurity;
      output.highLevelSecurity.password =
        password_.empty() ? 0 : &password_[0];
      output.highLevelSecurity.passwordSize = password_.size();
      break;
    case EndpointAuthenticationKind::HighGmac:
      output.authenticationMode =
        dlms::client::ClientAuthenticationMode::HighLevelSecurityGmac;
      if (options_.security.cipheredApdu) {
        output.securityMode =
          dlms::client::ClientSecurityMode::AuthenticatedAndEncrypted;
      }
      output.security.invocationCounter =
        options_.security.invocationCounter;
      for (std::size_t i = 0u; i < systemTitle_.size() && i < 8u; ++i) {
        output.security.clientSystemTitle[i] = systemTitle_[i];
      }
      for (std::size_t i = 0u; i < peerSystemTitle_.size() && i < 8u; ++i) {
        output.security.serverSystemTitle[i] = peerSystemTitle_[i];
      }
      for (std::size_t i = 0u;
           i < globalUnicastEncryptionKey_.size() && i < 16u;
           ++i) {
        output.security.globalUnicastEncryptionKey[i] =
          globalUnicastEncryptionKey_[i];
      }
      for (std::size_t i = 0u; i < authenticationKey_.size() && i < 16u;
           ++i) {
        output.security.authenticationKey[i] = authenticationKey_[i];
      }
      break;
    default:
      return EndpointStatus::InvalidArgument;
  }

  return MapClientStatus(dlms::client::ValidateDlmsClientOptions(output));
}

EndpointStatus ClientEndpoint::Open()
{
  if (client_.get() != 0 && client_->IsAssociated()) {
    return EndpointStatus::Ok;
  }

  dlms::client::DlmsClientOptions clientOptions =
    dlms::client::DefaultDlmsClientOptions();
  EndpointStatus status = MakeClientOptions(clientOptions);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  std::unique_ptr<dlms::client::DlmsClient> client(
    new dlms::client::DlmsClient(clientOptions));

  status = MapClientStatus(client->Connect());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = MapClientStatus(client->OpenAssociation());
  if (status != EndpointStatus::Ok) {
    client->Close();
    return status;
  }

  client_ = std::move(client);
  return EndpointStatus::Ok;
}

EndpointStatus ClientEndpoint::Close()
{
  if (client_.get() == 0) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus releaseStatus =
    MapClientStatus(client_->ReleaseAssociation());
  const EndpointStatus closeStatus = MapClientStatus(client_->Close());
  if (closeStatus == EndpointStatus::Ok) {
    client_.reset();
  }
  return releaseStatus == EndpointStatus::Ok ? closeStatus : releaseStatus;
}

bool ClientEndpoint::IsOpen() const
{
  return client_.get() != 0 && client_->IsAssociated();
}

EndpointStatus ClientEndpoint::Get(
  const ClientAttributeDescriptor& descriptor,
  std::vector<std::uint8_t>& encodedData)
{
  encodedData.clear();
  if (!IsOpen()) {
    return EndpointStatus::InvalidState;
  }
  return MapClientStatus(client_->Get(descriptor, encodedData));
}

EndpointStatus ClientEndpoint::Set(
  const ClientAttributeDescriptor& descriptor,
  const std::vector<std::uint8_t>& encodedData)
{
  if (!IsOpen()) {
    return EndpointStatus::InvalidState;
  }
  return MapClientStatus(client_->Set(descriptor, encodedData));
}

EndpointStatus ClientEndpoint::Action(
  const ClientMethodDescriptor& descriptor,
  bool hasParameter,
  const std::vector<std::uint8_t>& encodedParameter,
  std::vector<std::uint8_t>& encodedReturnParameter)
{
  encodedReturnParameter.clear();
  if (!IsOpen()) {
    return EndpointStatus::InvalidState;
  }
  return MapClientStatus(
    client_->Action(
      descriptor,
      hasParameter,
      encodedParameter,
      encodedReturnParameter));
}

EndpointStatus MapClientStatus(dlms::client::ClientStatus status)
{
  switch (status) {
    case dlms::client::ClientStatus::Ok:
      return EndpointStatus::Ok;
    case dlms::client::ClientStatus::InvalidArgument:
      return EndpointStatus::InvalidArgument;
    case dlms::client::ClientStatus::InvalidState:
    case dlms::client::ClientStatus::NotAssociated:
      return EndpointStatus::InvalidState;
    case dlms::client::ClientStatus::TransportOpenFailed:
      return EndpointStatus::TransportFailed;
    case dlms::client::ClientStatus::ChannelOpenFailed:
      return EndpointStatus::ProfileFailed;
    case dlms::client::ClientStatus::AssociationFailed:
      return EndpointStatus::AssociationFailed;
    case dlms::client::ClientStatus::SendFailed:
    case dlms::client::ClientStatus::ReceiveFailed:
    case dlms::client::ClientStatus::ServiceRejected:
      return EndpointStatus::ServiceFailed;
    case dlms::client::ClientStatus::SecurityFailed:
      return EndpointStatus::SecurityFailed;
    case dlms::client::ClientStatus::UnsupportedFeature:
      return EndpointStatus::UnsupportedProfile;
    case dlms::client::ClientStatus::InternalError:
    default:
      return EndpointStatus::InternalError;
  }
}

} // namespace endpoint
} // namespace dlms
