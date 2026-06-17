#pragma once

#include "dlms/endpoint/client_endpoint.hpp"
#include "dlms/endpoint/endpoint_options.hpp"
#include "dlms/endpoint/endpoint_status.hpp"
#include "dlms/endpoint/gateway_interfaces.hpp"

#include "dlms/association/association_server_interface.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/xdlms/xdlms_status.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dlms {
namespace endpoint {

class GatewayEndpointOwnedState;

class ClientEndpointGatewayUpstream : public IGatewayUpstream
{
public:
  explicit ClientEndpointGatewayUpstream(ClientEndpoint& client);

  EndpointStatus Open();
  EndpointStatus Close();
  bool IsOpen() const;

  EndpointStatus Get(
    const ClientAttributeDescriptor& descriptor,
    std::vector<std::uint8_t>& encodedData);

  EndpointStatus Set(
    const ClientAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData);

  EndpointStatus Action(
    const ClientMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    std::vector<std::uint8_t>& encodedReturnParameter);

private:
  ClientEndpoint& client_;
};

class GatewayEndpoint
{
public:
  GatewayEndpoint(
    dlms::profile::IApduChannel& downstreamChannel,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);

  GatewayEndpoint(
    dlms::profile::IApduChannel& downstreamChannel,
    const GatewayEndpointOptions& options,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);
  ~GatewayEndpoint();

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;

private:
  GatewayEndpoint(const GatewayEndpoint&);
  GatewayEndpoint& operator=(const GatewayEndpoint&);

  EndpointStatus NegotiateDownstreamAssociation();
  bool IsReleaseRequest(const std::vector<std::uint8_t>& requestApdu) const;
  EndpointStatus ReleaseDownstreamAssociation(
    const std::vector<std::uint8_t>& requestApdu);

  dlms::profile::IApduChannel& downstreamChannel_;
  GatewayEndpointOptions options_;
  std::unique_ptr<dlms::association::IAssociationServer> association_;
  IGatewayUpstream& upstream_;
  IGatewayPolicy& policy_;
  std::unique_ptr<GatewayEndpointOwnedState> owned_;
  std::vector<std::uint8_t> requestApdu_;
  std::vector<std::uint8_t> responseApdu_;
  bool open_;
};

dlms::xdlms::XdlmsStatus MapEndpointStatusToXdlmsStatus(
  EndpointStatus status);

} // namespace endpoint
} // namespace dlms
