#pragma once

#include "dlms/endpoint/endpoint_options.hpp"
#include "dlms/endpoint/endpoint_status.hpp"

#include "dlms/association/association_server_interface.hpp"
#include "dlms/cosem/cosem.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/server/server_context.hpp"
#include "dlms/server/server_service_interface.hpp"
#include "dlms/xdlms/xdlms_status.hpp"

#include <memory>
#include <vector>

namespace dlms {
namespace endpoint {

class ServerEndpointOwnedState;

class ServerEndpoint
{
public:
  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    dlms::cosem::ILogicalDevice& logicalDevice);

  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    dlms::server::IServerService& server);

  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    const ServerEndpointOptions& options,
    dlms::cosem::ILogicalDevice& logicalDevice);

  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    const ServerEndpointOptions& options,
    dlms::server::IServerService& server);
  ~ServerEndpoint();

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;
  dlms::server::ServerContext& Context();
  const dlms::server::ServerContext& Context() const;

private:
  ServerEndpoint(const ServerEndpoint&);
  ServerEndpoint& operator=(const ServerEndpoint&);

  void ConfigureAssociationContext();
  void ConfigureXdlmsProcessor();
  EndpointStatus ApplyCipheredAssociationContext();
  EndpointStatus NegotiateAssociation();
  bool IsReleaseRequest(const std::vector<std::uint8_t>& requestApdu) const;
  EndpointStatus ReleaseAssociation(
    const std::vector<std::uint8_t>& requestApdu);
  EndpointStatus HandleHlsReply(
    const std::vector<std::uint8_t>& requestApdu,
    std::vector<std::uint8_t>& responseApdu,
    bool& handled);

  dlms::profile::IApduChannel& channel_;
  ServerEndpointOptions options_;
  std::unique_ptr<dlms::association::IAssociationServer> association_;
  dlms::server::ServerContext context_;
  std::unique_ptr<ServerEndpointOwnedState> owned_;
  bool open_;
  bool hlsPending_;
};

EndpointStatus MapProfileStatus(dlms::profile::ProfileStatus status);
EndpointStatus MapXdlmsStatus(dlms::xdlms::XdlmsStatus status);

} // namespace endpoint
} // namespace dlms
