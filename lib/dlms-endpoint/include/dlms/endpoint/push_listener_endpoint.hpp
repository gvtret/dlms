#pragma once

#include "dlms/endpoint/endpoint_options.hpp"
#include "dlms/endpoint/endpoint_status.hpp"
#include "dlms/endpoint/push_indication_handler.hpp"

#include "dlms/association/association_server_interface.hpp"
#include "dlms/profile/apdu_channel.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dlms {
namespace endpoint {

class PushListenerEndpoint
{
public:
  PushListenerEndpoint(
    dlms::profile::IApduChannel& channel,
    IPushIndicationHandler& handler);

  PushListenerEndpoint(
    dlms::profile::IApduChannel& channel,
    const PushListenerEndpointOptions& options,
    IPushIndicationHandler& handler);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;

private:
  PushListenerEndpoint(const PushListenerEndpoint&);
  PushListenerEndpoint& operator=(const PushListenerEndpoint&);

  EndpointStatus NegotiateAssociation();
  bool IsReleaseRequest(const std::vector<std::uint8_t>& requestApdu) const;
  EndpointStatus ReleaseAssociation(
    const std::vector<std::uint8_t>& requestApdu);

  dlms::profile::IApduChannel& channel_;
  PushListenerEndpointOptions options_;
  std::unique_ptr<dlms::association::IAssociationServer> association_;
  IPushIndicationHandler& handler_;
  std::vector<std::uint8_t> apdu_;
  bool open_;
};

} // namespace endpoint
} // namespace dlms
