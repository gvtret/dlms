#include "dlms/endpoint/gateway_endpoint.hpp"

#include "dlms/association/association_server.hpp"
#include "dlms/apdu/acse.hpp"
#include "dlms/endpoint/server_endpoint.hpp"
#include "dlms/server/tracing_xdlms_server_dispatcher.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

#include <utility>

namespace {

const std::uint8_t kAccessDeniedResult = 3u;

dlms::association::AssociationServerOptions MakeAssociationServerOptions(
  const dlms::endpoint::EndpointSecurityOptions& security)
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
      return dlms::endpoint::EndpointStatus::AssociationFailed;
  }

  // Defensive fall-through for ABI drift / unknown integer values.
  // No `default:` above so `-Wswitch` warns when `AssociationStatus`
  // is extended.
  return dlms::endpoint::EndpointStatus::AssociationFailed;
}

} // namespace

namespace dlms {
namespace endpoint {

IGatewayPolicy::~IGatewayPolicy()
{
}

IGatewayUpstream::~IGatewayUpstream()
{
}

ClientEndpointGatewayUpstream::ClientEndpointGatewayUpstream(
  ClientEndpoint& client)
  : client_(client)
{
}

EndpointStatus ClientEndpointGatewayUpstream::Open()
{
  return client_.Open();
}

EndpointStatus ClientEndpointGatewayUpstream::Close()
{
  return client_.Close();
}

bool ClientEndpointGatewayUpstream::IsOpen() const
{
  return client_.IsOpen();
}

EndpointStatus ClientEndpointGatewayUpstream::Get(
  const ClientAttributeDescriptor& descriptor,
  std::vector<std::uint8_t>& encodedData)
{
  return client_.Get(descriptor, encodedData);
}

EndpointStatus ClientEndpointGatewayUpstream::Set(
  const ClientAttributeDescriptor& descriptor,
  const std::vector<std::uint8_t>& encodedData)
{
  return client_.Set(descriptor, encodedData);
}

EndpointStatus ClientEndpointGatewayUpstream::Action(
  const ClientMethodDescriptor& descriptor,
  bool hasParameter,
  const std::vector<std::uint8_t>& encodedParameter,
  std::vector<std::uint8_t>& encodedReturnParameter)
{
  return client_.Action(
    descriptor,
    hasParameter,
    encodedParameter,
    encodedReturnParameter);
}

class GatewayEndpointServerHandler
  : public dlms::xdlms::IXdlmsServerHandler
{
public:
  GatewayEndpointServerHandler(
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy)
    : upstream_(upstream)
    , policy_(policy)
  {
  }

  dlms::xdlms::XdlmsStatus HandleGet(
    const dlms::xdlms::GetIndication& indication,
    dlms::xdlms::GetResult& result) override
  {
    result = dlms::xdlms::EmptyGetResult();
    result.invokeId = indication.invokeId;

    if (!policy_.AllowGet(indication.descriptor)) {
      result.hasAccessResult = true;
      result.accessResult = kAccessDeniedResult;
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    result.hasData = true;
    return MapEndpointStatusToXdlmsStatus(
      upstream_.Get(indication.descriptor, result.data));
  }

  dlms::xdlms::XdlmsStatus HandleSet(
    const dlms::xdlms::SetIndication& indication,
    dlms::xdlms::SetResult& result) override
  {
    result = dlms::xdlms::EmptySetResult();
    result.invokeId = indication.invokeId;

    if (!policy_.AllowSet(indication.descriptor)) {
      result.accessResult = kAccessDeniedResult;
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    const EndpointStatus status =
      upstream_.Set(indication.descriptor, indication.data);
    result.accessResult = 0u;
    return MapEndpointStatusToXdlmsStatus(status);
  }

  dlms::xdlms::XdlmsStatus HandleAction(
    const dlms::xdlms::ActionIndication& indication,
    dlms::xdlms::ActionResult& result) override
  {
    result = dlms::xdlms::EmptyActionResult();
    result.invokeId = indication.invokeId;

    if (!policy_.AllowAction(indication.descriptor)) {
      result.actionResult = kAccessDeniedResult;
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    const EndpointStatus status =
      upstream_.Action(
        indication.descriptor,
        indication.hasParameter,
        indication.parameter,
        result.data);
    if (status == EndpointStatus::Ok && !result.data.empty()) {
      result.hasData = true;
    }
    result.actionResult = 0u;
    return MapEndpointStatusToXdlmsStatus(status);
  }

private:
  IGatewayUpstream& upstream_;
  IGatewayPolicy& policy_;
};

class GatewayEndpointOwnedState
{
public:
  GatewayEndpointOwnedState(
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy,
    dlms::server::IServerDispatchTraceSink* dispatchTraceSink)
    : handler(upstream, policy)
    , dispatcher(handler)
    , tracingDispatcher(dispatcher, dispatchTraceSink)
    , processor(tracingDispatcher)
  {
  }

  GatewayEndpointServerHandler handler;
  dlms::xdlms::XdlmsServerDispatcher dispatcher;
  dlms::server::TracingXdlmsServerDispatcher tracingDispatcher;
  dlms::xdlms::XdlmsServerApduProcessor processor;
};

GatewayEndpoint::GatewayEndpoint(
  dlms::profile::IApduChannel& downstreamChannel,
  IGatewayUpstream& upstream,
  IGatewayPolicy& policy)
  : downstreamChannel_(downstreamChannel)
  , options_(DefaultGatewayEndpointOptions())
  , association_()
  , upstream_(upstream)
  , policy_(policy)
  , owned_(new GatewayEndpointOwnedState(
      upstream_,
      policy_,
      options_.downstream.serverDispatchTraceSink))
  , open_(false)
{
  owned_->tracingDispatcher.SetCorrelationChannel(&downstreamChannel_);
  owned_->processor.SetApduChannel(&downstreamChannel_);
  owned_->processor.SetTraceSink(options_.downstream.xdlmsTraceSink);
}

GatewayEndpoint::GatewayEndpoint(
  dlms::profile::IApduChannel& downstreamChannel,
  const GatewayEndpointOptions& options,
  IGatewayUpstream& upstream,
  IGatewayPolicy& policy)
  : downstreamChannel_(downstreamChannel)
  , options_(options)
  , association_()
  , upstream_(upstream)
  , policy_(policy)
  , owned_(new GatewayEndpointOwnedState(
      upstream_,
      policy_,
      options_.downstream.serverDispatchTraceSink))
  , open_(false)
{
  owned_->tracingDispatcher.SetCorrelationChannel(&downstreamChannel_);
  owned_->processor.SetApduChannel(&downstreamChannel_);
  owned_->processor.SetTraceSink(options_.downstream.xdlmsTraceSink);
}

GatewayEndpoint::~GatewayEndpoint()
{
}

EndpointStatus GatewayEndpoint::Open()
{
  if (open_) {
    return EndpointStatus::Ok;
  }

  EndpointStatus status =
    ValidateEndpointProfileOptions(options_.downstream.profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointSecurityOptions(options_.downstream.security);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = MapProfileStatus(downstreamChannel_.Open());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = NegotiateDownstreamAssociation();
  if (status != EndpointStatus::Ok) {
    association_.reset();
    downstreamChannel_.Close();
    return status;
  }

  status = upstream_.Open();
  if (status != EndpointStatus::Ok) {
    association_.reset();
    downstreamChannel_.Close();
    return status;
  }

  open_ = true;
  return EndpointStatus::Ok;
}

EndpointStatus GatewayEndpoint::NegotiateDownstreamAssociation()
{
  if (!options_.downstream.negotiateAssociation) {
    return EndpointStatus::Ok;
  }

  if (options_.downstream.security.authentication !=
        EndpointAuthenticationKind::None &&
      options_.downstream.security.authentication !=
        EndpointAuthenticationKind::LowPassword) {
    return EndpointStatus::AssociationFailed;
  }

  std::unique_ptr<dlms::association::AssociationServer> association(
    new dlms::association::AssociationServer(
      downstreamChannel_,
      MakeAssociationServerOptions(options_.downstream.security)));
  EndpointStatus status = MapAssociationStatus(association->Open());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = MapAssociationStatus(association->Accept());
  if (status == EndpointStatus::Ok) {
    association_ = std::move(association);
  }
  return status;
}

bool GatewayEndpoint::IsReleaseRequest(
  const std::vector<std::uint8_t>& requestApdu) const
{
  if (!options_.downstream.negotiateAssociation ||
      association_.get() == 0 ||
      requestApdu.empty()) {
    return false;
  }

  dlms::apdu::AcseApdu apdu = {};
  const dlms::apdu::ApduStatus status =
    dlms::apdu::DecodeAcseApdu(&requestApdu[0], requestApdu.size(), apdu);
  return status == dlms::apdu::ApduStatus::Ok &&
         apdu.kind == dlms::apdu::AcseApduKind::Rlrq;
}

EndpointStatus GatewayEndpoint::ReleaseDownstreamAssociation(
  const std::vector<std::uint8_t>& requestApdu)
{
  if (association_.get() == 0) {
    return EndpointStatus::InvalidState;
  }

  EndpointStatus status =
    MapAssociationStatus(association_->Release(requestApdu));
  if (status != EndpointStatus::Ok) {
    return status;
  }

  association_.reset();
  status = upstream_.Close();
  if (status == EndpointStatus::Ok) {
    open_ = false;
  }
  return status;
}

EndpointStatus GatewayEndpoint::RunOnce()
{
  if (!open_) {
    return EndpointStatus::InvalidState;
  }

  std::vector<std::uint8_t> requestApdu;
  EndpointStatus status =
    MapProfileStatus(downstreamChannel_.ReceiveApdu(requestApdu));
  if (status != EndpointStatus::Ok) {
    return status;
  }

  std::vector<std::uint8_t> responseApdu;
  if (IsReleaseRequest(requestApdu)) {
    return ReleaseDownstreamAssociation(requestApdu);
  }

  status =
    MapXdlmsStatus(owned_->processor.ProcessRequest(requestApdu, responseApdu));
  if (status != EndpointStatus::Ok) {
    return status;
  }

  dlms::profile::ProfileByteView response;
  response.data = responseApdu.empty() ? 0 : &responseApdu[0];
  response.size = responseApdu.size();
  return MapProfileStatus(downstreamChannel_.SendApdu(response));
}

EndpointStatus GatewayEndpoint::Close()
{
  if (!open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus upstreamStatus = upstream_.Close();
  const EndpointStatus downstreamStatus =
    MapProfileStatus(downstreamChannel_.Close());
  if (downstreamStatus == EndpointStatus::Ok) {
    association_.reset();
    open_ = false;
  }
  return upstreamStatus == EndpointStatus::Ok
    ? downstreamStatus
    : upstreamStatus;
}

bool GatewayEndpoint::IsOpen() const
{
  return open_;
}

dlms::xdlms::XdlmsStatus MapEndpointStatusToXdlmsStatus(
  EndpointStatus status)
{
  switch (status) {
    case EndpointStatus::Ok:
      return dlms::xdlms::XdlmsStatus::Ok;
    case EndpointStatus::InvalidArgument:
      return dlms::xdlms::XdlmsStatus::InvalidArgument;
    case EndpointStatus::InvalidState:
    case EndpointStatus::AssociationFailed:
      return dlms::xdlms::XdlmsStatus::InvalidState;
    case EndpointStatus::SecurityFailed:
      return dlms::xdlms::XdlmsStatus::SecurityFailed;
    case EndpointStatus::UnsupportedProfile:
      return dlms::xdlms::XdlmsStatus::UnsupportedFeature;
    case EndpointStatus::TransportFailed:
    case EndpointStatus::ProfileFailed:
    case EndpointStatus::ServiceFailed:
    case EndpointStatus::Timeout:
    case EndpointStatus::Closed:
      return dlms::xdlms::XdlmsStatus::ServiceRejected;
    case EndpointStatus::InternalError:
      return dlms::xdlms::XdlmsStatus::InternalError;
  }

  // Defensive fall-through for ABI drift / unknown integer values.
  // No `default:` above so `-Wswitch` warns when `EndpointStatus` is
  // extended.
  return dlms::xdlms::XdlmsStatus::InternalError;
}

} // namespace endpoint
} // namespace dlms
