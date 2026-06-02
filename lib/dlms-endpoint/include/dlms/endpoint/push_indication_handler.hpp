#pragma once

#include "dlms/endpoint/endpoint_status.hpp"

#include <cstdint>
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

} // namespace endpoint
} // namespace dlms
