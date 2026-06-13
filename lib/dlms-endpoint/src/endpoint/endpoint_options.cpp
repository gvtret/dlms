#include "dlms/endpoint/endpoint_options.hpp"

#include "dlms/hdlc/hdlc_address.hpp"

namespace {

bool IsEmpty(const char* value)
{
  return value == 0 || value[0] == '\0';
}

} // namespace

namespace dlms {
namespace endpoint {

EndpointTransportOptions DefaultEndpointTransportOptions()
{
  EndpointTransportOptions options;
  options.kind = EndpointTransportKind::Tcp;
  options.host = 0;
  options.port = 0u;
  options.serialDevice = 0;
  options.baudRate = 0u;
  options.timeoutMs = 1000u;
  return options;
}

EndpointProfileOptions DefaultEndpointProfileOptions()
{
  EndpointProfileOptions options;
  options.kind = EndpointProfileKind::Wrapper;
  options.clientSap = 16u;
  options.serverSap = 1u;
  options.hdlcClientAddress = 0x10u;
  options.hdlcLogicalDeviceAddress = 1u;
  options.hdlcPhysicalDeviceAddress = 0u;
  options.hdlcUseSession = false;
  options.maxApduSize = 1024u;
  return options;
}

EndpointSecurityOptions DefaultEndpointSecurityOptions()
{
  EndpointSecurityOptions options;
  options.authentication = EndpointAuthenticationKind::None;
  options.password = 0;
  options.passwordSize = 0u;
  options.systemTitle = 0;
  options.systemTitleSize = 0u;
  options.peerSystemTitle = 0;
  options.peerSystemTitleSize = 0u;
  options.globalUnicastEncryptionKey = 0;
  options.globalUnicastEncryptionKeySize = 0u;
  options.authenticationKey = 0;
  options.authenticationKeySize = 0u;
  options.invocationCounter = 0u;
  options.cipheredApdu = false;
  return options;
}

ClientEndpointOptions DefaultClientEndpointOptions()
{
  ClientEndpointOptions options;
  options.transport = DefaultEndpointTransportOptions();
  options.profile = DefaultEndpointProfileOptions();
  options.security = DefaultEndpointSecurityOptions();
  return options;
}

ServerEndpointOptions DefaultServerEndpointOptions()
{
  ServerEndpointOptions options;
  options.transport = DefaultEndpointTransportOptions();
  options.profile = DefaultEndpointProfileOptions();
  options.security = DefaultEndpointSecurityOptions();
  options.negotiateAssociation = false;
  return options;
}

PushListenerEndpointOptions DefaultPushListenerEndpointOptions()
{
  PushListenerEndpointOptions options;
  options.transport = DefaultEndpointTransportOptions();
  options.profile = DefaultEndpointProfileOptions();
  options.security = DefaultEndpointSecurityOptions();
  options.negotiateAssociation = false;
  return options;
}

GatewayEndpointOptions DefaultGatewayEndpointOptions()
{
  GatewayEndpointOptions options;
  options.downstream = DefaultServerEndpointOptions();
  options.upstream = DefaultClientEndpointOptions();
  return options;
}

EndpointStatus ValidateEndpointTransportOptions(
  const EndpointTransportOptions& options)
{
  if (options.timeoutMs == 0u) {
    return EndpointStatus::InvalidArgument;
  }

  switch (options.kind) {
    case EndpointTransportKind::Tcp:
    case EndpointTransportKind::Udp:
      if (IsEmpty(options.host) || options.port == 0u) {
        return EndpointStatus::InvalidArgument;
      }
      return EndpointStatus::Ok;
    case EndpointTransportKind::Serial:
      if (IsEmpty(options.serialDevice) || options.baudRate == 0u) {
        return EndpointStatus::InvalidArgument;
      }
      return EndpointStatus::Ok;
    default:
      return EndpointStatus::InvalidArgument;
  }
}

EndpointStatus ValidateEndpointProfileOptions(
  const EndpointProfileOptions& options)
{
  if (options.clientSap == 0u || options.serverSap == 0u
      || options.maxApduSize == 0u) {
    return EndpointStatus::InvalidArgument;
  }

  switch (options.kind) {
    case EndpointProfileKind::Wrapper:
      return EndpointStatus::Ok;
    case EndpointProfileKind::Hdlc:
      if (options.clientSap > 0x7Fu ||
          options.serverSap > 0x3FFFu ||
          options.hdlcClientAddress == 0u ||
          options.hdlcClientAddress > 0x7Fu ||
          options.hdlcLogicalDeviceAddress > 0x3FFFu ||
          options.hdlcPhysicalDeviceAddress > 0x3FFFu) {
        return EndpointStatus::InvalidArgument;
      }
      {
        dlms::hdlc::HdlcAddress serverAddress;
        if (dlms::hdlc::DlmsHdlcAddress::MakeServerAddress(
              options.hdlcLogicalDeviceAddress,
              options.hdlcPhysicalDeviceAddress,
              serverAddress) != dlms::hdlc::HdlcStatus::Ok) {
          return EndpointStatus::InvalidArgument;
        }
      }
      return EndpointStatus::Ok;
    default:
      return EndpointStatus::UnsupportedProfile;
  }
}

EndpointStatus ValidateEndpointSecurityOptions(
  const EndpointSecurityOptions& options)
{
  if ((options.peerSystemTitle == 0 && options.peerSystemTitleSize != 0u) ||
      (options.peerSystemTitle != 0 && options.peerSystemTitleSize != 8u)) {
    return EndpointStatus::InvalidArgument;
  }

  switch (options.authentication) {
    case EndpointAuthenticationKind::None:
      if (options.cipheredApdu) {
        return EndpointStatus::InvalidArgument;
      }
      return EndpointStatus::Ok;
    case EndpointAuthenticationKind::LowPassword:
    case EndpointAuthenticationKind::HighPassword:
      if (options.cipheredApdu) {
        return EndpointStatus::InvalidArgument;
      }
      if (options.password == 0 || options.passwordSize == 0u) {
        return EndpointStatus::InvalidArgument;
      }
      return EndpointStatus::Ok;
    case EndpointAuthenticationKind::HighGmac:
      if (options.systemTitle == 0 || options.systemTitleSize != 8u ||
          options.authenticationKey == 0 ||
          options.authenticationKeySize != 16u) {
        return EndpointStatus::InvalidArgument;
      }
      if ((options.globalUnicastEncryptionKey == 0 &&
           options.globalUnicastEncryptionKeySize != 0u) ||
          (options.globalUnicastEncryptionKey != 0 &&
           options.globalUnicastEncryptionKeySize != 16u)) {
        return EndpointStatus::InvalidArgument;
      }
      if (options.cipheredApdu &&
          options.globalUnicastEncryptionKey == 0) {
        return EndpointStatus::InvalidArgument;
      }
      return EndpointStatus::Ok;
    default:
      return EndpointStatus::InvalidArgument;
  }
}

EndpointStatus ValidateClientEndpointOptions(
  const ClientEndpointOptions& options)
{
  EndpointStatus status = ValidateEndpointTransportOptions(options.transport);
  if (status != EndpointStatus::Ok) {
    return status;
  }
  if (options.transport.kind != EndpointTransportKind::Tcp) {
    return EndpointStatus::UnsupportedProfile;
  }

  status = ValidateEndpointProfileOptions(options.profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  return ValidateEndpointSecurityOptions(options.security);
}

EndpointStatus ValidateServerEndpointOptions(
  const ServerEndpointOptions& options)
{
  EndpointStatus status = ValidateEndpointTransportOptions(options.transport);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointProfileOptions(options.profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  return ValidateEndpointSecurityOptions(options.security);
}

EndpointStatus ValidatePushListenerEndpointOptions(
  const PushListenerEndpointOptions& options)
{
  EndpointStatus status = ValidateEndpointTransportOptions(options.transport);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointProfileOptions(options.profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  return ValidateEndpointSecurityOptions(options.security);
}

EndpointStatus ValidateGatewayEndpointOptions(
  const GatewayEndpointOptions& options)
{
  EndpointStatus status = ValidateServerEndpointOptions(options.downstream);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  return ValidateClientEndpointOptions(options.upstream);
}

} // namespace endpoint
} // namespace dlms
