#include "dlms/endpoint/server_endpoint.hpp"

#include "dlms/apdu/acse.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/association/association_server.hpp"
#include "dlms/security/ciphered_apdu_processor.hpp"
#include "dlms/security/hls_gmac_authenticator.hpp"
#include "dlms/security/hls_high_authenticator.hpp"
#include "dlms/security/in_memory_invocation_counter_store.hpp"
#include "dlms/security/in_memory_key_store.hpp"
#include "dlms/security/random_source.hpp"

#include <limits>
#include <openssl/rand.h>
#include <utility>

namespace {

dlms::server::ServerAssociationContext MakeAssociationContext(
  const dlms::endpoint::ServerEndpointOptions& options)
{
  dlms::server::ServerAssociationContext context =
    dlms::server::EmptyServerAssociationContext();
  context.associated = true;
  context.clientSap = options.profile.clientSap;
  context.serverSap = options.profile.serverSap;
  context.authenticated =
    options.security.authentication !=
      dlms::endpoint::EndpointAuthenticationKind::None;
  context.ciphered = options.security.cipheredApdu;
  return context;
}

dlms::server::ServerAssociationContext MakeAssociationContext(
  const dlms::endpoint::ServerEndpointOptions& options,
  const dlms::association::AssociationResult& result)
{
  dlms::server::ServerAssociationContext context =
    dlms::server::EmptyServerAssociationContext();
  context.associated = result.negotiatedDlmsVersionNumber != 0u &&
                       result.serverMaxReceivePduSize != 0u;
  context.clientSap = options.profile.clientSap;
  context.serverSap = options.profile.serverSap;
  context.authenticated =
    options.security.authentication !=
      dlms::endpoint::EndpointAuthenticationKind::None;
  context.ciphered = options.security.cipheredApdu;
  return context;
}

dlms::security::SecurityByteView SecurityView(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::security::SecurityByteView view;
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

bool IsAssociationLnReplyMethod(
  const dlms::apdu::CosemMethodDescriptor& descriptor)
{
  return descriptor.classId == 15u &&
         descriptor.logicalName[0] == 0u &&
         descriptor.logicalName[1] == 0u &&
         descriptor.logicalName[2] == 40u &&
         descriptor.logicalName[3] == 0u &&
         descriptor.logicalName[4] == 0u &&
         descriptor.logicalName[5] == 255u &&
         descriptor.methodId == 1u;
}

dlms::endpoint::EndpointStatus EncodeOctetStringActionResponse(
  std::uint8_t invokeIdAndPriority,
  const std::vector<std::uint8_t>& bytes,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::ActionResponse;
  response.actionResponseAny.choice =
    dlms::apdu::ActionResponseChoice::Normal;
  response.actionResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.actionResponseAny.normal.result = 0u;
  response.actionResponseAny.normal.hasReturnParameter = true;
  response.actionResponseAny.normal.returnParameter.type =
    dlms::apdu::DlmsDataType::OctetString;
  response.actionResponseAny.normal.returnParameter.bytes.data =
    bytes.empty() ? 0 : &bytes[0];
  response.actionResponseAny.normal.returnParameter.bytes.size = bytes.size();

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? dlms::endpoint::EndpointStatus::Ok
    : dlms::endpoint::EndpointStatus::InternalError;
}

dlms::association::AssociationServerOptions MakeAssociationServerOptions(
  const dlms::endpoint::EndpointSecurityOptions& security,
  const dlms::association::IHighLevelSecurityServerStrategy* hls)
{
  dlms::association::AssociationServerOptions options =
    dlms::association::DefaultAssociationServerOptions();
  if (security.authentication ==
      dlms::endpoint::EndpointAuthenticationKind::LowPassword) {
    options.authenticationMode =
      dlms::association::AuthenticationMode::LowLevelSecurity;
    options.lowLevelSecurityCredential.assign(
      security.password,
      security.password + security.passwordSize);
  } else if (security.authentication ==
               dlms::endpoint::EndpointAuthenticationKind::HighPassword ||
             security.authentication ==
               dlms::endpoint::EndpointAuthenticationKind::HighGmac) {
    options.authenticationMode =
      dlms::association::AuthenticationMode::HighLevelSecurity;
    options.highLevelSecurity = hls;
    if (security.authentication ==
        dlms::endpoint::EndpointAuthenticationKind::HighGmac) {
      options.respondingApplicationTitle.assign(
        security.systemTitle,
        security.systemTitle + security.systemTitleSize);
    }
  }
  return options;
}

dlms::endpoint::EndpointStatus MapAssociationStatus(
  dlms::association::AssociationStatus status)
{
  switch (status) {
    case dlms::association::AssociationStatus::Ok:
      return dlms::endpoint::EndpointStatus::Ok;
    case dlms::association::AssociationStatus::InvalidArgument:
      return dlms::endpoint::EndpointStatus::InvalidArgument;
    case dlms::association::AssociationStatus::InvalidState:
      return dlms::endpoint::EndpointStatus::InvalidState;
    case dlms::association::AssociationStatus::ReceiveFailed:
      return dlms::endpoint::EndpointStatus::ProfileFailed;
    case dlms::association::AssociationStatus::UnsupportedApplicationContext:
    case dlms::association::AssociationStatus::UnsupportedAuthentication:
    case dlms::association::AssociationStatus::SendFailed:
    case dlms::association::AssociationStatus::EncodeFailed:
    case dlms::association::AssociationStatus::DecodeFailed:
    case dlms::association::AssociationStatus::AssociationRejected:
    case dlms::association::AssociationStatus::NegotiationFailed:
    case dlms::association::AssociationStatus::ChannelOpenFailed:
    case dlms::association::AssociationStatus::ChannelCloseFailed:
    case dlms::association::AssociationStatus::AlreadyAssociated:
    case dlms::association::AssociationStatus::InternalError:
    default:
      return dlms::endpoint::EndpointStatus::AssociationFailed;
  }
}

} // namespace

namespace dlms {
namespace endpoint {

class EndpointOpenSslRandomSource : public dlms::security::IRandomSource
{
public:
  dlms::security::SecurityStatus Fill(
    std::uint8_t* output,
    std::size_t outputSize) override
  {
    if (output == 0 && outputSize != 0u) {
      return dlms::security::SecurityStatus::InvalidArgument;
    }
    if (outputSize == 0u) {
      return dlms::security::SecurityStatus::Ok;
    }
    return RAND_bytes(output, static_cast<int>(outputSize)) == 1
      ? dlms::security::SecurityStatus::Ok
      : dlms::security::SecurityStatus::InternalError;
  }
};

class ServerEndpointHlsHighStrategy
  : public dlms::association::IHighLevelSecurityServerStrategy
{
public:
  ServerEndpointHlsHighStrategy(
    const std::uint8_t* password,
    std::size_t passwordSize)
    : password_()
    , random_()
    , hls_()
    , clientChallenge_()
    , serverChallenge_()
  {
    if (password != 0 && passwordSize != 0u) {
      password_.assign(password, password + passwordSize);
    }
    hls_.reset(
      new dlms::security::HlsHighAuthenticator(
        SecurityView(password_),
        random_));
  }

  dlms::association::AssociationStatus ValidateInitialChallenge(
    dlms::association::HighLevelSecurityMechanism mechanism,
    const std::vector<std::uint8_t>& clientChallenge) const override
  {
    if (mechanism !=
        dlms::association::HighLevelSecurityMechanism::HlsHigh ||
        clientChallenge.empty()) {
      return dlms::association::AssociationStatus::UnsupportedAuthentication;
    }
    clientChallenge_ = clientChallenge;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus BuildResponseChallenge(
    dlms::association::HighLevelSecurityMechanism mechanism,
    std::vector<std::uint8_t>& output) const override
  {
    if (mechanism !=
        dlms::association::HighLevelSecurityMechanism::HlsHigh) {
      return dlms::association::AssociationStatus::UnsupportedAuthentication;
    }
    if (hls_->BuildChallenge(output) != dlms::security::SecurityStatus::Ok) {
      return dlms::association::AssociationStatus::InternalError;
    }
    serverChallenge_ = output;
    return dlms::association::AssociationStatus::Ok;
  }

  EndpointStatus VerifyClientResponse(
    const std::vector<std::uint8_t>& response) const
  {
    if (serverChallenge_.empty()) {
      return EndpointStatus::AssociationFailed;
    }
    return hls_->VerifyResponse(SecurityView(serverChallenge_),
                                SecurityView(response)) ==
        dlms::security::SecurityStatus::Ok
      ? EndpointStatus::Ok
      : EndpointStatus::SecurityFailed;
  }

  EndpointStatus BuildServerResponse(
    std::vector<std::uint8_t>& response) const
  {
    if (clientChallenge_.empty()) {
      return EndpointStatus::AssociationFailed;
    }
    return hls_->BuildResponse(SecurityView(clientChallenge_), response) ==
        dlms::security::SecurityStatus::Ok
      ? EndpointStatus::Ok
      : EndpointStatus::SecurityFailed;
  }

private:
  std::vector<std::uint8_t> password_;
  mutable EndpointOpenSslRandomSource random_;
  std::unique_ptr<dlms::security::HlsHighAuthenticator> hls_;
  mutable std::vector<std::uint8_t> clientChallenge_;
  mutable std::vector<std::uint8_t> serverChallenge_;
};

dlms::security::SecurityKey MakeSecurityKey(
  dlms::security::SecurityKeyRole role,
  const std::uint8_t* bytes,
  std::size_t size)
{
  dlms::security::SecurityKey key =
    dlms::security::EmptySecurityKey(role);
  key.size = size;
  for (std::size_t i = 0u; i < size && i < sizeof(key.bytes); ++i) {
    key.bytes[i] = bytes[i];
  }
  return key;
}

class ServerEndpointHlsGmacStrategy
  : public dlms::association::IHighLevelSecurityServerStrategy
{
public:
  ServerEndpointHlsGmacStrategy(
    const EndpointProfileOptions& profile,
    const EndpointSecurityOptions& security)
    : context_(dlms::security::EmptySecurityContext())
    , keys_()
    , counters_()
    , random_()
    , hls_()
    , clientChallenge_()
    , serverChallenge_()
  {
    context_.policy = dlms::security::SecurityPolicy::Authenticated;
    context_.role = dlms::security::SecurityRole::Server;
    context_.clientSap = profile.clientSap;
    context_.serverSap = profile.serverSap;
    for (std::size_t i = 0u; i < 8u && i < security.systemTitleSize; ++i) {
      context_.localSystemTitle[i] = security.systemTitle[i];
    }
    keys_.SetKey(
      MakeSecurityKey(
        dlms::security::SecurityKeyRole::Authentication,
        security.authenticationKey,
        security.authenticationKeySize));
    counters_.SetLocalCounter(security.invocationCounter);
    hls_.reset(
      new dlms::security::HlsGmacAuthenticator(
        context_,
        keys_,
        counters_,
        random_));
  }

  dlms::association::AssociationStatus SetCallingApplicationTitle(
    const std::vector<std::uint8_t>& title) const override
  {
    if (title.size() != 8u) {
      return dlms::association::AssociationStatus::UnsupportedAuthentication;
    }
    for (std::size_t i = 0u; i < 8u; ++i) {
      context_.remoteSystemTitle[i] = title[i];
    }
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus ValidateInitialChallenge(
    dlms::association::HighLevelSecurityMechanism mechanism,
    const std::vector<std::uint8_t>& clientChallenge) const override
  {
    if (mechanism !=
        dlms::association::HighLevelSecurityMechanism::HlsGmac ||
        clientChallenge.empty()) {
      return dlms::association::AssociationStatus::UnsupportedAuthentication;
    }
    clientChallenge_ = clientChallenge;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus BuildResponseChallenge(
    dlms::association::HighLevelSecurityMechanism mechanism,
    std::vector<std::uint8_t>& output) const override
  {
    if (mechanism !=
        dlms::association::HighLevelSecurityMechanism::HlsGmac) {
      return dlms::association::AssociationStatus::UnsupportedAuthentication;
    }
    if (hls_->BuildChallenge(output) != dlms::security::SecurityStatus::Ok) {
      return dlms::association::AssociationStatus::InternalError;
    }
    serverChallenge_ = output;
    return dlms::association::AssociationStatus::Ok;
  }

  EndpointStatus VerifyClientResponse(
    const std::vector<std::uint8_t>& response) const
  {
    if (serverChallenge_.empty()) {
      return EndpointStatus::AssociationFailed;
    }
    return hls_->VerifyResponse(SecurityView(serverChallenge_),
                                SecurityView(response)) ==
        dlms::security::SecurityStatus::Ok
      ? EndpointStatus::Ok
      : EndpointStatus::SecurityFailed;
  }

  EndpointStatus BuildServerResponse(
    std::vector<std::uint8_t>& response) const
  {
    if (clientChallenge_.empty()) {
      return EndpointStatus::AssociationFailed;
    }
    return hls_->BuildResponse(SecurityView(clientChallenge_), response) ==
        dlms::security::SecurityStatus::Ok
      ? EndpointStatus::Ok
      : EndpointStatus::SecurityFailed;
  }

private:
  mutable dlms::security::SecurityContext context_;
  dlms::security::InMemoryKeyStore keys_;
  mutable dlms::security::InMemoryInvocationCounterStore counters_;
  mutable EndpointOpenSslRandomSource random_;
  std::unique_ptr<dlms::security::HlsGmacAuthenticator> hls_;
  mutable std::vector<std::uint8_t> clientChallenge_;
  mutable std::vector<std::uint8_t> serverChallenge_;
};

class ServerEndpointOwnedState
{
public:
  ~ServerEndpointOwnedState();

  std::unique_ptr<ServerEndpointHlsHighStrategy> hlsHigh;
  std::unique_ptr<ServerEndpointHlsGmacStrategy> hlsGmac;
  std::unique_ptr<dlms::xdlms::XdlmsServerDispatcher> dispatcher;
  std::unique_ptr<dlms::security::SecurityContext> securityContext;
  std::unique_ptr<dlms::security::InMemoryKeyStore> keys;
  std::unique_ptr<dlms::security::InMemoryInvocationCounterStore> counters;
  std::unique_ptr<dlms::security::CipheredApduProcessor> security;
  std::unique_ptr<dlms::xdlms::XdlmsServerApduProcessor> processor;
};

ServerEndpointOwnedState::~ServerEndpointOwnedState()
{
}

ServerEndpoint::ServerEndpoint(
  dlms::profile::IApduChannel& channel,
  dlms::cosem::ILogicalDevice& logicalDevice)
  : channel_(channel)
  , options_(DefaultServerEndpointOptions())
  , association_()
  , context_()
  , server_(context_)
  , adapter_(server_)
  , owned_(new ServerEndpointOwnedState())
  , open_(false)
  , hlsPending_(false)
{
  context_.AttachLogicalDevice(&logicalDevice);
  ConfigureXdlmsProcessor();
  ConfigureAssociationContext();
}

ServerEndpoint::ServerEndpoint(
  dlms::profile::IApduChannel& channel,
  dlms::server::IServerService& server)
  : channel_(channel)
  , options_(DefaultServerEndpointOptions())
  , association_()
  , context_()
  , server_(context_)
  , adapter_(server)
  , owned_(new ServerEndpointOwnedState())
  , open_(false)
  , hlsPending_(false)
{
  ConfigureXdlmsProcessor();
  ConfigureAssociationContext();
}

ServerEndpoint::ServerEndpoint(
  dlms::profile::IApduChannel& channel,
  const ServerEndpointOptions& options,
  dlms::cosem::ILogicalDevice& logicalDevice)
  : channel_(channel)
  , options_(options)
  , association_()
  , context_()
  , server_(context_)
  , adapter_(server_)
  , owned_(new ServerEndpointOwnedState())
  , open_(false)
  , hlsPending_(false)
{
  context_.AttachLogicalDevice(&logicalDevice);
  ConfigureXdlmsProcessor();
  ConfigureAssociationContext();
}

ServerEndpoint::ServerEndpoint(
  dlms::profile::IApduChannel& channel,
  const ServerEndpointOptions& options,
  dlms::server::IServerService& server)
  : channel_(channel)
  , options_(options)
  , association_()
  , context_()
  , server_(context_)
  , adapter_(server)
  , owned_(new ServerEndpointOwnedState())
  , open_(false)
  , hlsPending_(false)
{
  ConfigureXdlmsProcessor();
  ConfigureAssociationContext();
}

ServerEndpoint::~ServerEndpoint()
{
}

void ServerEndpoint::ConfigureAssociationContext()
{
  if (options_.negotiateAssociation) {
    context_.ClearAssociationContext();
    return;
  }

  context_.SetAssociationContext(MakeAssociationContext(options_));
}

void ServerEndpoint::ConfigureXdlmsProcessor()
{
  owned_->processor.reset();
  owned_->security.reset();
  owned_->counters.reset();
  owned_->keys.reset();
  owned_->securityContext.reset();
  owned_->dispatcher.reset(new dlms::xdlms::XdlmsServerDispatcher(adapter_));

  if (!options_.security.cipheredApdu) {
    owned_->processor.reset(
      new dlms::xdlms::XdlmsServerApduProcessor(*owned_->dispatcher));
    return;
  }

  owned_->securityContext.reset(
    new dlms::security::SecurityContext(
      dlms::security::EmptySecurityContext()));
  owned_->securityContext->policy =
    dlms::security::SecurityPolicy::AuthenticatedAndEncrypted;
  owned_->securityContext->role = dlms::security::SecurityRole::Server;
  owned_->securityContext->clientSap = options_.profile.clientSap;
  owned_->securityContext->serverSap = options_.profile.serverSap;
  for (std::size_t i = 0u;
       i < 8u && i < options_.security.systemTitleSize;
       ++i) {
    owned_->securityContext->localSystemTitle[i] =
      options_.security.systemTitle[i];
  }
  for (std::size_t i = 0u;
       i < 8u && i < options_.security.peerSystemTitleSize;
       ++i) {
    owned_->securityContext->remoteSystemTitle[i] =
      options_.security.peerSystemTitle[i];
  }

  owned_->keys.reset(new dlms::security::InMemoryKeyStore());
  if (options_.security.globalUnicastEncryptionKey != 0 &&
      options_.security.globalUnicastEncryptionKeySize == 16u) {
    owned_->keys->SetKey(
      MakeSecurityKey(
        dlms::security::SecurityKeyRole::GlobalUnicastEncryption,
        options_.security.globalUnicastEncryptionKey,
        options_.security.globalUnicastEncryptionKeySize));
  }
  if (options_.security.authenticationKey != 0 &&
      options_.security.authenticationKeySize == 16u) {
    owned_->keys->SetKey(
      MakeSecurityKey(
        dlms::security::SecurityKeyRole::Authentication,
        options_.security.authenticationKey,
        options_.security.authenticationKeySize));
  }

  owned_->counters.reset(
    new dlms::security::InMemoryInvocationCounterStore());
  owned_->counters->SetLocalCounter(options_.security.invocationCounter + 2u);
  owned_->security.reset(
    new dlms::security::CipheredApduProcessor(
      *owned_->securityContext,
      *owned_->keys,
      *owned_->counters));
  owned_->processor.reset(
    new dlms::xdlms::XdlmsServerApduProcessor(
      *owned_->dispatcher,
      *owned_->security));
}

EndpointStatus ServerEndpoint::ApplyCipheredAssociationContext()
{
  if (!options_.security.cipheredApdu) {
    return EndpointStatus::Ok;
  }
  if (owned_->securityContext.get() == 0 || association_.get() == 0) {
    return EndpointStatus::InternalError;
  }
  const std::vector<std::uint8_t>& title =
    association_->Result().callingApplicationTitle;
  if (title.size() != 8u) {
    return EndpointStatus::SecurityFailed;
  }
  for (std::size_t i = 0u; i < 8u; ++i) {
    owned_->securityContext->remoteSystemTitle[i] = title[i];
  }
  return EndpointStatus::Ok;
}

EndpointStatus ServerEndpoint::NegotiateAssociation()
{
  if (options_.security.authentication != EndpointAuthenticationKind::None &&
      options_.security.authentication !=
        EndpointAuthenticationKind::LowPassword &&
      options_.security.authentication !=
        EndpointAuthenticationKind::HighPassword &&
      options_.security.authentication !=
        EndpointAuthenticationKind::HighGmac) {
    return EndpointStatus::AssociationFailed;
  }

  owned_->hlsHigh.reset();
  owned_->hlsGmac.reset();
  if (options_.security.authentication ==
      EndpointAuthenticationKind::HighPassword) {
    owned_->hlsHigh.reset(
      new ServerEndpointHlsHighStrategy(
        options_.security.password,
        options_.security.passwordSize));
  } else if (options_.security.authentication ==
             EndpointAuthenticationKind::HighGmac) {
    owned_->hlsGmac.reset(
      new ServerEndpointHlsGmacStrategy(
        options_.profile,
        options_.security));
  }

  const dlms::association::IHighLevelSecurityServerStrategy* hls =
    owned_->hlsHigh.get() != 0
      ? static_cast<const dlms::association::IHighLevelSecurityServerStrategy*>(
          owned_->hlsHigh.get())
      : static_cast<const dlms::association::IHighLevelSecurityServerStrategy*>(
          owned_->hlsGmac.get());

  std::unique_ptr<dlms::association::AssociationServer> association(
    new dlms::association::AssociationServer(
      channel_,
      MakeAssociationServerOptions(options_.security, hls)));
  EndpointStatus status = MapAssociationStatus(association->Open());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = MapAssociationStatus(association->Accept());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  association_ = std::move(association);
  status = ApplyCipheredAssociationContext();
  if (status != EndpointStatus::Ok) {
    association_.reset();
    return status;
  }

  hlsPending_ = owned_->hlsHigh.get() != 0 || owned_->hlsGmac.get() != 0;
  if (hlsPending_) {
    context_.ClearAssociationContext();
  } else {
    context_.SetAssociationContext(
      MakeAssociationContext(options_, association_->Result()));
  }
  return EndpointStatus::Ok;
}

bool ServerEndpoint::IsReleaseRequest(
  const std::vector<std::uint8_t>& requestApdu) const
{
  if (!options_.negotiateAssociation || association_.get() == 0 ||
      requestApdu.empty()) {
    return false;
  }

  dlms::apdu::AcseApdu apdu = {};
  const dlms::apdu::ApduStatus status =
    dlms::apdu::DecodeAcseApdu(&requestApdu[0], requestApdu.size(), apdu);
  return status == dlms::apdu::ApduStatus::Ok &&
         apdu.kind == dlms::apdu::AcseApduKind::Rlrq;
}

EndpointStatus ServerEndpoint::ReleaseAssociation(
  const std::vector<std::uint8_t>& requestApdu)
{
  if (association_.get() == 0) {
    return EndpointStatus::InvalidState;
  }

  const EndpointStatus status =
    MapAssociationStatus(association_->Release(requestApdu));
  if (status == EndpointStatus::Ok) {
    association_.reset();
    owned_->hlsHigh.reset();
    owned_->hlsGmac.reset();
    hlsPending_ = false;
    context_.ClearAssociationContext();
    open_ = false;
  }
  return status;
}

EndpointStatus ServerEndpoint::HandleHlsReply(
  const std::vector<std::uint8_t>& requestApdu,
  std::vector<std::uint8_t>& responseApdu,
  bool& handled)
{
  handled = false;
  if (!hlsPending_ ||
      (owned_->hlsHigh.get() == 0 && owned_->hlsGmac.get() == 0) ||
      requestApdu.empty()) {
    return EndpointStatus::Ok;
  }

  std::vector<std::uint8_t> plainRequest = requestApdu;
  if (options_.security.cipheredApdu) {
    if (owned_->security.get() == 0) {
      return EndpointStatus::InternalError;
    }
    dlms::security::SecurityByteView protectedRequest;
    protectedRequest.data = &requestApdu[0];
    protectedRequest.size = requestApdu.size();
    owned_->security->Unprotect(protectedRequest, plainRequest);
    if (plainRequest.empty()) {
      plainRequest = requestApdu;
    }
  }

  dlms::apdu::XdlmsApdu request;
  if (plainRequest.empty() ||
      dlms::apdu::DecodeXdlmsApdu(&plainRequest[0], plainRequest.size(),
                                  request) != dlms::apdu::ApduStatus::Ok ||
      request.kind != dlms::apdu::XdlmsApduKind::ActionRequest ||
      request.actionRequestAny.choice !=
        dlms::apdu::ActionRequestChoice::Normal ||
      !IsAssociationLnReplyMethod(
        request.actionRequestAny.normal.descriptor)) {
    return EndpointStatus::Ok;
  }

  handled = true;
  if (!request.actionRequestAny.normal.hasInvocationParameter ||
      request.actionRequestAny.normal.invocationParameter.type !=
        dlms::apdu::DlmsDataType::OctetString) {
    return EndpointStatus::SecurityFailed;
  }

  const dlms::apdu::ByteView clientResponseView =
    request.actionRequestAny.normal.invocationParameter.bytes;
  std::vector<std::uint8_t> clientResponse;
  if (clientResponseView.data != 0 && clientResponseView.size != 0u) {
    clientResponse.assign(
      clientResponseView.data,
      clientResponseView.data + clientResponseView.size);
  }

  EndpointStatus status = owned_->hlsHigh.get() != 0
    ? owned_->hlsHigh->VerifyClientResponse(clientResponse)
    : owned_->hlsGmac->VerifyClientResponse(clientResponse);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  std::vector<std::uint8_t> serverResponse;
  status = owned_->hlsHigh.get() != 0
    ? owned_->hlsHigh->BuildServerResponse(serverResponse)
    : owned_->hlsGmac->BuildServerResponse(serverResponse);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status =
    EncodeOctetStringActionResponse(
      request.actionRequestAny.invokeIdAndPriority,
      serverResponse,
      responseApdu);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  context_.SetAssociationContext(
    MakeAssociationContext(options_, association_->Result()));
  hlsPending_ = false;
  return EndpointStatus::Ok;
}

EndpointStatus ServerEndpoint::Open()
{
  if (open_) {
    return EndpointStatus::Ok;
  }

  EndpointStatus status = ValidateEndpointProfileOptions(options_.profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointSecurityOptions(options_.security);
  if (status != EndpointStatus::Ok) {
    return status;
  }
  if (options_.security.cipheredApdu && !options_.negotiateAssociation) {
    return EndpointStatus::InvalidArgument;
  }
  if (options_.security.cipheredApdu &&
      options_.security.invocationCounter >
        std::numeric_limits<std::uint32_t>::max() - 2u) {
    return EndpointStatus::InvalidArgument;
  }

  status = MapProfileStatus(channel_.Open());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  if (options_.negotiateAssociation) {
    status = NegotiateAssociation();
    if (status != EndpointStatus::Ok) {
      channel_.Close();
      return status;
    }
  }

  open_ = true;
  return EndpointStatus::Ok;
}

EndpointStatus ServerEndpoint::RunOnce()
{
  if (!open_) {
    return EndpointStatus::InvalidState;
  }

  std::vector<std::uint8_t> requestApdu;
  EndpointStatus status =
    MapProfileStatus(channel_.ReceiveApdu(requestApdu));
  if (status != EndpointStatus::Ok) {
    return status;
  }

  std::vector<std::uint8_t> responseApdu;
  if (IsReleaseRequest(requestApdu)) {
    return ReleaseAssociation(requestApdu);
  }

  bool handled = false;
  status = HandleHlsReply(requestApdu, responseApdu, handled);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  if (!handled) {
    status =
      MapXdlmsStatus(
        owned_->processor->ProcessRequest(requestApdu, responseApdu));
  }
  if (status != EndpointStatus::Ok) {
    return status;
  }

  dlms::profile::ProfileByteView response;
  response.data = responseApdu.empty() ? 0 : &responseApdu[0];
  response.size = responseApdu.size();
  return MapProfileStatus(channel_.SendApdu(response));
}

EndpointStatus ServerEndpoint::Close()
{
  if (!open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = MapProfileStatus(channel_.Close());
  if (status == EndpointStatus::Ok) {
    association_.reset();
    owned_->hlsHigh.reset();
    owned_->hlsGmac.reset();
    hlsPending_ = false;
    open_ = false;
  }
  return status;
}

bool ServerEndpoint::IsOpen() const
{
  return open_;
}

dlms::server::ServerContext& ServerEndpoint::Context()
{
  return context_;
}

const dlms::server::ServerContext& ServerEndpoint::Context() const
{
  return context_;
}

EndpointStatus MapProfileStatus(dlms::profile::ProfileStatus status)
{
  switch (status) {
    case dlms::profile::ProfileStatus::Ok:
    case dlms::profile::ProfileStatus::AlreadyOpen:
      return EndpointStatus::Ok;
    case dlms::profile::ProfileStatus::InvalidArgument:
      return EndpointStatus::InvalidArgument;
    case dlms::profile::ProfileStatus::NotOpen:
      return EndpointStatus::InvalidState;
    case dlms::profile::ProfileStatus::OpenFailed:
      return EndpointStatus::ProfileFailed;
    case dlms::profile::ProfileStatus::Timeout:
      return EndpointStatus::Timeout;
    case dlms::profile::ProfileStatus::ConnectionClosed:
      return EndpointStatus::Closed;
    case dlms::profile::ProfileStatus::UnsupportedFeature:
      return EndpointStatus::UnsupportedProfile;
    case dlms::profile::ProfileStatus::ReadFailed:
    case dlms::profile::ProfileStatus::WriteFailed:
    case dlms::profile::ProfileStatus::WouldBlock:
    case dlms::profile::ProfileStatus::NeedMoreData:
    case dlms::profile::ProfileStatus::OutputBufferTooSmall:
    case dlms::profile::ProfileStatus::InvalidFrame:
    case dlms::profile::ProfileStatus::InvalidLength:
    case dlms::profile::ProfileStatus::InvalidAddress:
    case dlms::profile::ProfileStatus::PayloadTooLarge:
      return EndpointStatus::ProfileFailed;
    case dlms::profile::ProfileStatus::InternalError:
    default:
      return EndpointStatus::InternalError;
  }
}

EndpointStatus MapXdlmsStatus(dlms::xdlms::XdlmsStatus status)
{
  switch (status) {
    case dlms::xdlms::XdlmsStatus::Ok:
      return EndpointStatus::Ok;
    case dlms::xdlms::XdlmsStatus::InvalidArgument:
      return EndpointStatus::InvalidArgument;
    case dlms::xdlms::XdlmsStatus::InvalidState:
    case dlms::xdlms::XdlmsStatus::NotAssociated:
      return EndpointStatus::InvalidState;
    case dlms::xdlms::XdlmsStatus::SendFailed:
    case dlms::xdlms::XdlmsStatus::ReceiveFailed:
    case dlms::xdlms::XdlmsStatus::ServiceRejected:
      return EndpointStatus::ServiceFailed;
    case dlms::xdlms::XdlmsStatus::SecurityFailed:
      return EndpointStatus::SecurityFailed;
    case dlms::xdlms::XdlmsStatus::UnsupportedFeature:
    case dlms::xdlms::XdlmsStatus::BlockTransferRequired:
      return EndpointStatus::UnsupportedProfile;
    case dlms::xdlms::XdlmsStatus::EncodeFailed:
    case dlms::xdlms::XdlmsStatus::DecodeFailed:
    case dlms::xdlms::XdlmsStatus::InvokeIdMismatch:
    case dlms::xdlms::XdlmsStatus::InternalError:
    default:
      return EndpointStatus::InternalError;
  }
}

} // namespace endpoint
} // namespace dlms
