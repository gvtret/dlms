#pragma once

namespace dlms {
namespace endpoint {

enum class EndpointStatus
{
  Ok,
  InvalidArgument,
  InvalidState,
  UnsupportedProfile,
  TransportFailed,
  ProfileFailed,
  AssociationFailed,
  SecurityFailed,
  ServiceFailed,
  Timeout,
  Closed,
  InternalError
};

const char* ToString(EndpointStatus status);

} // namespace endpoint
} // namespace dlms

