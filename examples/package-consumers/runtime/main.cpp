#include "dlms/client/client_options.hpp"
#include "dlms/endpoint/endpoint_options.hpp"

int main()
{
  const dlms::client::DlmsClientOptions client =
    dlms::client::DefaultDlmsClientOptions();
  const dlms::endpoint::ClientEndpointOptions endpoint =
    dlms::endpoint::DefaultClientEndpointOptions();

  return client.requestTimeoutMs > 0 && endpoint.transport.timeoutMs > 0
    ? 0
    : 1;
}
