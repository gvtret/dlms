#include "dlms/endpoint/endpoint.hpp"

#include <cstdint>

int main()
{
  dlms::endpoint::ClientEndpointOptions options =
    dlms::endpoint::DefaultClientEndpointOptions();
  options.transport.host = "127.0.0.1";
  options.transport.port = 4059u;
  options.profile.kind = dlms::endpoint::EndpointProfileKind::Wrapper;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::None;

  dlms::endpoint::ClientEndpoint client(options);

  dlms::endpoint::ClientAttributeDescriptor descriptor;
  descriptor.classId = 3u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  descriptor.attributeId = 2u;

  return client.IsOpen() ? 1 : 0;
}
