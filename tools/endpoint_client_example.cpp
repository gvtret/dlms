#include "dlms/endpoint/endpoint.hpp"

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

  if (dlms::endpoint::ValidateClientEndpointOptions(options) !=
      dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }

  return client.IsOpen() ? 1 : 0;
}
