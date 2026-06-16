#include "dlms/endpoint/client_endpoint.hpp"

#include "dlms/client/client.hpp"

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

class ClientEndpointOwnedState
{
public:
  std::unique_ptr<dlms::client::DlmsClient> client;
};

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
  , owned_(new ClientEndpointOwnedState())
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

EndpointStatus ClientEndpoint::CreateClient()
{
  EndpointStatus status = ValidateClientEndpointOptions(options_);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  if (options_.transport.kind != EndpointTransportKind::Tcp) {
    return EndpointStatus::UnsupportedProfile;
  }

  dlms::client::DlmsClientOptions clientOptions =
    dlms::client::DefaultDlmsClientOptions();
  clientOptions.clientSap = options_.profile.clientSap;
  clientOptions.serverSap = options_.profile.serverSap;
  clientOptions.connectTimeoutMs = options_.transport.timeoutMs;
  clientOptions.requestTimeoutMs = options_.transport.timeoutMs;

  switch (options_.profile.kind) {
    case EndpointProfileKind::Wrapper:
      clientOptions.profile = dlms::client::ClientProfile::WrapperTcp;
      clientOptions.wrapperTcp.host = options_.transport.host;
      clientOptions.wrapperTcp.port = options_.transport.port;
      clientOptions.wrapperTcp.sourceWPort = options_.profile.clientSap;
      clientOptions.wrapperTcp.destinationWPort = options_.profile.serverSap;
      break;
    case EndpointProfileKind::Hdlc:
      clientOptions.profile = dlms::client::ClientProfile::HdlcTcp;
      clientOptions.hdlcTcp.host = options_.transport.host;
      clientOptions.hdlcTcp.port = options_.transport.port;
      clientOptions.hdlcTcp.clientAddress = options_.profile.hdlcClientAddress;
      clientOptions.hdlcTcp.logicalDeviceAddress =
        options_.profile.hdlcLogicalDeviceAddress;
      clientOptions.hdlcTcp.physicalDeviceAddress =
        options_.profile.hdlcPhysicalDeviceAddress;
      clientOptions.hdlcTcp.useDataLinkSession =
        options_.profile.hdlcUseSession;
      break;
    default:
      return EndpointStatus::UnsupportedProfile;
  }

  switch (options_.security.authentication) {
    case EndpointAuthenticationKind::None:
      clientOptions.authenticationMode =
        dlms::client::ClientAuthenticationMode::None;
      break;
    case EndpointAuthenticationKind::LowPassword:
      clientOptions.authenticationMode =
        dlms::client::ClientAuthenticationMode::LowLevelSecurity;
      clientOptions.lowLevelSecurity.credential =
        password_.empty() ? 0 : &password_[0];
      clientOptions.lowLevelSecurity.credentialSize = password_.size();
      break;
    case EndpointAuthenticationKind::HighPassword:
      clientOptions.authenticationMode =
        dlms::client::ClientAuthenticationMode::HighLevelSecurity;
      clientOptions.highLevelSecurity.password =
        password_.empty() ? 0 : &password_[0];
      clientOptions.highLevelSecurity.passwordSize = password_.size();
      break;
    case EndpointAuthenticationKind::HighGmac:
      clientOptions.authenticationMode =
        dlms::client::ClientAuthenticationMode::HighLevelSecurityGmac;
      if (options_.security.cipheredApdu) {
        clientOptions.securityMode =
          dlms::client::ClientSecurityMode::AuthenticatedAndEncrypted;
      }
      clientOptions.security.invocationCounter =
        options_.security.invocationCounter;
      for (std::size_t i = 0u; i < systemTitle_.size() && i < 8u; ++i) {
        clientOptions.security.clientSystemTitle[i] = systemTitle_[i];
      }
      for (std::size_t i = 0u; i < peerSystemTitle_.size() && i < 8u; ++i) {
        clientOptions.security.serverSystemTitle[i] = peerSystemTitle_[i];
      }
      for (std::size_t i = 0u;
           i < globalUnicastEncryptionKey_.size() && i < 16u;
           ++i) {
        clientOptions.security.globalUnicastEncryptionKey[i] =
          globalUnicastEncryptionKey_[i];
      }
      for (std::size_t i = 0u; i < authenticationKey_.size() && i < 16u;
           ++i) {
        clientOptions.security.authenticationKey[i] = authenticationKey_[i];
      }
      break;
    default:
      return EndpointStatus::InvalidArgument;
  }

  clientOptions.wrapperTcpTraceSink = options_.wrapperTcpTraceSink;
  clientOptions.hdlcProfileTraceSink = options_.hdlcProfileTraceSink;
  clientOptions.associationTraceSink = options_.associationTraceSink;

  status = MapClientStatus(
    dlms::client::ValidateDlmsClientOptions(clientOptions));
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

  owned_->client = std::move(client);
  return EndpointStatus::Ok;
}

EndpointStatus ClientEndpoint::Open()
{
  if (owned_->client.get() != 0 && owned_->client->IsAssociated()) {
    return EndpointStatus::Ok;
  }

  return CreateClient();
}

EndpointStatus ClientEndpoint::Close()
{
  if (owned_->client.get() == 0) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus releaseStatus =
    MapClientStatus(owned_->client->ReleaseAssociation());
  const EndpointStatus closeStatus = MapClientStatus(owned_->client->Close());
  // Always drop the client instance after Close(), even if the underlying
  // client reported a non-Ok status. Keeping a client that already refused
  // to close around just papers over the failure: the next Open() would
  // either short-circuit on stale IsAssociated() or silently leak the old
  // instance via move-assignment in CreateClient(). The caller still sees
  // the failure via the returned status.
  owned_->client.reset();
  return releaseStatus == EndpointStatus::Ok ? closeStatus : releaseStatus;
}

bool ClientEndpoint::IsOpen() const
{
  return owned_->client.get() != 0 && owned_->client->IsAssociated();
}

EndpointStatus ClientEndpoint::Get(
  const ClientAttributeDescriptor& descriptor,
  std::vector<std::uint8_t>& encodedData)
{
  encodedData.clear();
  if (!IsOpen()) {
    return EndpointStatus::InvalidState;
  }
  return MapClientStatus(owned_->client->Get(descriptor, encodedData));
}

EndpointStatus ClientEndpoint::Set(
  const ClientAttributeDescriptor& descriptor,
  const std::vector<std::uint8_t>& encodedData)
{
  if (!IsOpen()) {
    return EndpointStatus::InvalidState;
  }
  return MapClientStatus(owned_->client->Set(descriptor, encodedData));
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
    owned_->client->Action(
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
    // BlockTransferRequired / InvokeIdMismatch / CodecFailed are
    // service-layer outcomes too. The endpoint layer is a thin
    // facade and intentionally collapses every service-side
    // failure into `ServiceFailed`; callers that need the finer
    // distinction should consume `DlmsClient` directly.
    case dlms::client::ClientStatus::BlockTransferRequired:
    case dlms::client::ClientStatus::InvokeIdMismatch:
    case dlms::client::ClientStatus::CodecFailed:
      return EndpointStatus::ServiceFailed;
    case dlms::client::ClientStatus::SecurityFailed:
      return EndpointStatus::SecurityFailed;
    case dlms::client::ClientStatus::UnsupportedFeature:
      return EndpointStatus::UnsupportedProfile;
    case dlms::client::ClientStatus::InternalError:
      return EndpointStatus::InternalError;
  }

  // Defensive fall-through for ABI drift / unknown integer values.
  // Reachable code (no `default:` above) so `-Wswitch` will warn
  // when `ClientStatus` is extended.
  return EndpointStatus::InternalError;
}

} // namespace endpoint
} // namespace dlms
