#pragma once

#include "dlms/endpoint/endpoint_descriptors.hpp"
#include "dlms/endpoint/endpoint_status.hpp"

#include <cstdint>
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

} // namespace endpoint
} // namespace dlms
