#pragma once

#include "dlms/endpoint/endpoint_status.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace endpoint {

enum class EndpointTransportKind
{
  Tcp,
  Udp,
  Serial
};

enum class EndpointProfileKind
{
  Wrapper,
  Hdlc
};

enum class EndpointAuthenticationKind
{
  None,
  LowPassword,
  HighPassword,
  HighGmac
};

struct EndpointTransportOptions
{
  EndpointTransportKind kind;
  const char* host;
  std::uint16_t port;
  const char* serialDevice;
  std::uint32_t baudRate;
  std::uint32_t timeoutMs;
};

struct EndpointProfileOptions
{
  EndpointProfileKind kind;
  std::uint16_t clientSap;
  std::uint16_t serverSap;
  bool hdlcUseSession;
  std::size_t maxApduSize;
};

struct EndpointSecurityOptions
{
  EndpointAuthenticationKind authentication;
  const std::uint8_t* password;
  std::size_t passwordSize;
  const std::uint8_t* systemTitle;
  std::size_t systemTitleSize;
  const std::uint8_t* peerSystemTitle;
  std::size_t peerSystemTitleSize;
  const std::uint8_t* globalUnicastEncryptionKey;
  std::size_t globalUnicastEncryptionKeySize;
  const std::uint8_t* authenticationKey;
  std::size_t authenticationKeySize;
  std::uint32_t invocationCounter;
  bool cipheredApdu;
};

struct ClientEndpointOptions
{
  EndpointTransportOptions transport;
  EndpointProfileOptions profile;
  EndpointSecurityOptions security;
};

struct ServerEndpointOptions
{
  EndpointTransportOptions transport;
  EndpointProfileOptions profile;
  EndpointSecurityOptions security;
  bool negotiateAssociation;
};

struct PushListenerEndpointOptions
{
  EndpointTransportOptions transport;
  EndpointProfileOptions profile;
  EndpointSecurityOptions security;
  bool negotiateAssociation;
};

struct GatewayEndpointOptions
{
  ServerEndpointOptions downstream;
  ClientEndpointOptions upstream;
};

EndpointTransportOptions DefaultEndpointTransportOptions();
EndpointProfileOptions DefaultEndpointProfileOptions();
EndpointSecurityOptions DefaultEndpointSecurityOptions();
ClientEndpointOptions DefaultClientEndpointOptions();
ServerEndpointOptions DefaultServerEndpointOptions();
PushListenerEndpointOptions DefaultPushListenerEndpointOptions();
GatewayEndpointOptions DefaultGatewayEndpointOptions();

EndpointStatus ValidateEndpointTransportOptions(
  const EndpointTransportOptions& options);
EndpointStatus ValidateEndpointProfileOptions(
  const EndpointProfileOptions& options);
EndpointStatus ValidateEndpointSecurityOptions(
  const EndpointSecurityOptions& options);
EndpointStatus ValidateClientEndpointOptions(
  const ClientEndpointOptions& options);
EndpointStatus ValidateServerEndpointOptions(
  const ServerEndpointOptions& options);
EndpointStatus ValidatePushListenerEndpointOptions(
  const PushListenerEndpointOptions& options);
EndpointStatus ValidateGatewayEndpointOptions(
  const GatewayEndpointOptions& options);

} // namespace endpoint
} // namespace dlms
