#pragma once

#include "dlms/endpoint/endpoint_status.hpp"

#include "dlms/profile/apdu_channel.hpp"

#include <cstdint>
#include <memory>

namespace dlms {
namespace endpoint {

class IApduChannelListener
{
public:
  virtual ~IApduChannelListener();

  virtual EndpointStatus Open() = 0;
  virtual EndpointStatus Close() = 0;
  virtual bool IsOpen() const = 0;
  virtual std::uint16_t LocalPort() const = 0;

  virtual EndpointStatus Accept(
    std::unique_ptr<dlms::profile::IApduChannel>& channel) = 0;
};

} // namespace endpoint
} // namespace dlms
