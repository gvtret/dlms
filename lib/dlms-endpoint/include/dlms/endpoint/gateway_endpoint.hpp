#pragma once

#include "dlms/endpoint/client_endpoint.hpp"
#include "dlms/endpoint/endpoint_options.hpp"
#include "dlms/endpoint/endpoint_status.hpp"

#include "dlms/association/association_server.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dlms {
namespace endpoint {

class IGatewayPolicy
{
public:
  virtual ~IGatewayPolicy();

  virtual bool AllowGet(
    const ClientAttributeDescriptor& descriptor) const = 0;

  virtual bool AllowSet(
    const ClientAttributeDescriptor& descriptor) const = 0;

  virtual bool AllowAction(
    const ClientMethodDescriptor& descriptor) const = 0;
};

class IGatewayUpstream
{
public:
  virtual ~IGatewayUpstream();

  virtual EndpointStatus Open() = 0;
  virtual EndpointStatus Close() = 0;
  virtual bool IsOpen() const = 0;

  virtual EndpointStatus Get(
    const ClientAttributeDescriptor& descriptor,
    std::vector<std::uint8_t>& encodedData) = 0;

  virtual EndpointStatus Set(
    const ClientAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData) = 0;

  virtual EndpointStatus Action(
    const ClientMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    std::vector<std::uint8_t>& encodedReturnParameter) = 0;
};

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

class GatewayEndpoint : private dlms::xdlms::IXdlmsServerHandler
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

  dlms::xdlms::XdlmsStatus HandleGet(
    const dlms::xdlms::GetIndication& indication,
    dlms::xdlms::GetResult& result);

  dlms::xdlms::XdlmsStatus HandleSet(
    const dlms::xdlms::SetIndication& indication,
    dlms::xdlms::SetResult& result);

  dlms::xdlms::XdlmsStatus HandleAction(
    const dlms::xdlms::ActionIndication& indication,
    dlms::xdlms::ActionResult& result);

  dlms::profile::IApduChannel& downstreamChannel_;
  GatewayEndpointOptions options_;
  std::unique_ptr<dlms::association::AssociationServer> association_;
  IGatewayUpstream& upstream_;
  IGatewayPolicy& policy_;
  dlms::xdlms::XdlmsServerDispatcher dispatcher_;
  dlms::xdlms::XdlmsServerApduProcessor processor_;
  bool open_;
};

dlms::xdlms::XdlmsStatus MapEndpointStatusToXdlmsStatus(
  EndpointStatus status);

} // namespace endpoint
} // namespace dlms
