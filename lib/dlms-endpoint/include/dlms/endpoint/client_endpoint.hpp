#pragma once

#include "dlms/endpoint/endpoint_options.hpp"
#include "dlms/endpoint/endpoint_status.hpp"

#include "dlms/client/client_status.hpp"
#include "dlms/xdlms/xdlms_types.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace dlms {
namespace endpoint {

using ClientAttributeDescriptor = dlms::xdlms::CosemAttributeDescriptor;
using ClientMethodDescriptor = dlms::xdlms::CosemMethodDescriptor;

class ClientEndpointOwnedState;

class ClientEndpoint
{
public:
  explicit ClientEndpoint(const ClientEndpointOptions& options);
  ~ClientEndpoint();

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
  ClientEndpoint(const ClientEndpoint&);
  ClientEndpoint& operator=(const ClientEndpoint&);

  EndpointStatus CreateClient();

  ClientEndpointOptions options_;
  std::string host_;
  std::string serialDevice_;
  std::vector<std::uint8_t> password_;
  std::vector<std::uint8_t> systemTitle_;
  std::vector<std::uint8_t> peerSystemTitle_;
  std::vector<std::uint8_t> globalUnicastEncryptionKey_;
  std::vector<std::uint8_t> authenticationKey_;
  std::unique_ptr<ClientEndpointOwnedState> owned_;
};

EndpointStatus MapClientStatus(dlms::client::ClientStatus status);

} // namespace endpoint
} // namespace dlms
