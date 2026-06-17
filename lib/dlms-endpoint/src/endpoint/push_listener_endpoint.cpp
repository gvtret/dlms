#include "dlms/endpoint/push_listener_endpoint.hpp"

#include "dlms/apdu/acse.hpp"
#include "dlms/association/association_server.hpp"
#include "dlms/endpoint/server_endpoint.hpp"

#include <utility>

namespace {

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

IPushIndicationHandler::~IPushIndicationHandler()
{
}

PushListenerEndpoint::PushListenerEndpoint(
  dlms::profile::IApduChannel& channel,
  IPushIndicationHandler& handler)
  : channel_(channel)
  , options_(DefaultPushListenerEndpointOptions())
  , association_()
  , handler_(handler)
  , open_(false)
{
}

PushListenerEndpoint::PushListenerEndpoint(
  dlms::profile::IApduChannel& channel,
  const PushListenerEndpointOptions& options,
  IPushIndicationHandler& handler)
  : channel_(channel)
  , options_(options)
  , association_()
  , handler_(handler)
  , open_(false)
{
}

EndpointStatus PushListenerEndpoint::NegotiateAssociation()
{
  if (!options_.negotiateAssociation) {
    return EndpointStatus::Ok;
  }

  if (options_.security.authentication != EndpointAuthenticationKind::None &&
      options_.security.authentication != EndpointAuthenticationKind::LowPassword) {
    return EndpointStatus::AssociationFailed;
  }

  std::unique_ptr<dlms::association::AssociationServer> association(
    new dlms::association::AssociationServer(
      channel_,
      MakeAssociationServerOptions(options_.security)));
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

bool PushListenerEndpoint::IsReleaseRequest(
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

EndpointStatus PushListenerEndpoint::ReleaseAssociation(
  const std::vector<std::uint8_t>& requestApdu)
{
  if (association_.get() == 0) {
    return EndpointStatus::InvalidState;
  }

  const EndpointStatus status =
    MapAssociationStatus(association_->Release(requestApdu));
  if (status == EndpointStatus::Ok) {
    association_.reset();
    open_ = false;
  }
  return status;
}

EndpointStatus PushListenerEndpoint::Open()
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

  status = MapProfileStatus(channel_.Open());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = NegotiateAssociation();
  if (status != EndpointStatus::Ok) {
    channel_.Close();
    return status;
  }

  open_ = true;
  return EndpointStatus::Ok;
}

EndpointStatus PushListenerEndpoint::RunOnce()
{
  if (!open_) {
    return EndpointStatus::InvalidState;
  }

  apdu_.clear();
  EndpointStatus status = MapProfileStatus(channel_.ReceiveApdu(apdu_));
  if (status != EndpointStatus::Ok) {
    return status;
  }

  if (IsReleaseRequest(apdu_)) {
    return ReleaseAssociation(apdu_);
  }

  return handler_.OnPushApdu(apdu_);
}

EndpointStatus PushListenerEndpoint::Close()
{
  if (!open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = MapProfileStatus(channel_.Close());
  if (status == EndpointStatus::Ok) {
    association_.reset();
    open_ = false;
  }
  return status;
}

bool PushListenerEndpoint::IsOpen() const
{
  return open_;
}

} // namespace endpoint
} // namespace dlms
