#include "dlms/endpoint/endpoint_status.hpp"

namespace dlms {
namespace endpoint {

const char* ToString(EndpointStatus status)
{
  switch (status) {
    case EndpointStatus::Ok:
      return "Ok";
    case EndpointStatus::InvalidArgument:
      return "InvalidArgument";
    case EndpointStatus::InvalidState:
      return "InvalidState";
    case EndpointStatus::UnsupportedProfile:
      return "UnsupportedProfile";
    case EndpointStatus::TransportFailed:
      return "TransportFailed";
    case EndpointStatus::ProfileFailed:
      return "ProfileFailed";
    case EndpointStatus::AssociationFailed:
      return "AssociationFailed";
    case EndpointStatus::SecurityFailed:
      return "SecurityFailed";
    case EndpointStatus::ServiceFailed:
      return "ServiceFailed";
    case EndpointStatus::Timeout:
      return "Timeout";
    case EndpointStatus::Closed:
      return "Closed";
    case EndpointStatus::InternalError:
      return "InternalError";
  }

  return "Unknown";
}

} // namespace endpoint
} // namespace dlms

