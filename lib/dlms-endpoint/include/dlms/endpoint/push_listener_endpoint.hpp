#pragma once

#include "dlms/endpoint/endpoint_options.hpp"
#include "dlms/endpoint/endpoint_status.hpp"

#include "dlms/association/association_server.hpp"
#include "dlms/profile/apdu_channel.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dlms {
namespace endpoint {

class IPushIndicationHandler
{
public:
  virtual ~IPushIndicationHandler();

  virtual EndpointStatus OnPushApdu(
    const std::vector<std::uint8_t>& apdu) = 0;
};

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
  std::unique_ptr<dlms::association::AssociationServer> association_;
  IPushIndicationHandler& handler_;
  bool open_;
};

} // namespace endpoint
} // namespace dlms
