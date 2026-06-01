#pragma once

#include "dlms/endpoint/listener_runtime.hpp"
#include "dlms/endpoint/endpoint_options.hpp"

#include "dlms/profile/apdu_channel.hpp"
#include "dlms/profile/hdlc_profile_channel.hpp"
#include "dlms/profile/wrapper_tcp_profile_channel.hpp"
#include "dlms/profile/wrapper_udp_profile_channel.hpp"
#include "dlms/security/security_types.hpp"
#include "dlms/transport/byte_stream.hpp"
#include "dlms/transport/datagram_transport.hpp"
#include "dlms/transport/serial_transport.hpp"
#include "dlms/transport/tcp_server_transport.hpp"
#include "dlms/transport/tcp_stream_transport.hpp"
#include "dlms/transport/udp_transport.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace dlms {
namespace endpoint {

class EndpointTransportBundle
{
public:
  EndpointTransportBundle();

  void Reset();

  dlms::transport::IByteStream* ByteStream() const;
  dlms::transport::IDatagramTransport* Datagram() const;

  std::unique_ptr<dlms::transport::TcpStreamTransport> tcp;
  std::unique_ptr<dlms::transport::UdpTransport> udp;
  std::unique_ptr<dlms::transport::SerialTransport> serial;

private:
  EndpointTransportBundle(const EndpointTransportBundle&);
  EndpointTransportBundle& operator=(const EndpointTransportBundle&);
};

class EndpointProfileBundle
{
public:
  EndpointProfileBundle();

  void Reset();

  dlms::profile::IApduChannel* Channel() const;

  std::unique_ptr<dlms::profile::WrapperTcpProfileChannel> wrapperTcp;
  std::unique_ptr<dlms::profile::WrapperUdpProfileChannel> wrapperUdp;
  std::unique_ptr<dlms::profile::HdlcProfileChannel> hdlc;

private:
  EndpointProfileBundle(const EndpointProfileBundle&);
  EndpointProfileBundle& operator=(const EndpointProfileBundle&);
};

class EndpointTcpProfileListener : public IApduChannelListener
{
public:
  EndpointTcpProfileListener(
    const EndpointTransportOptions& transport,
    const EndpointProfileOptions& profile);
  ~EndpointTcpProfileListener();

  EndpointStatus Open();
  EndpointStatus Close();
  bool IsOpen() const;
  std::uint16_t LocalPort() const;

  EndpointStatus Accept(
    std::unique_ptr<dlms::profile::IApduChannel>& channel);

private:
  EndpointTcpProfileListener(const EndpointTcpProfileListener&);
  EndpointTcpProfileListener& operator=(const EndpointTcpProfileListener&);

  EndpointTransportOptions transportOptions_;
  EndpointProfileOptions profileOptions_;
  std::string host_;
  std::unique_ptr<dlms::transport::TcpServerTransport> tcp_;
};

class EndpointUdpPushProfileListener : public IApduChannelListener
{
public:
  EndpointUdpPushProfileListener(
    const EndpointTransportOptions& transport,
    const EndpointProfileOptions& profile);
  ~EndpointUdpPushProfileListener();

  EndpointStatus Open();
  EndpointStatus Close();
  bool IsOpen() const;
  std::uint16_t LocalPort() const;

  EndpointStatus Accept(
    std::unique_ptr<dlms::profile::IApduChannel>& channel);

private:
  EndpointUdpPushProfileListener(const EndpointUdpPushProfileListener&);
  EndpointUdpPushProfileListener& operator=(
    const EndpointUdpPushProfileListener&);

  EndpointTransportOptions transportOptions_;
  EndpointProfileOptions profileOptions_;
  std::string host_;
  std::unique_ptr<dlms::transport::UdpTransport> udp_;
};

class EndpointListenerBundle
{
public:
  EndpointListenerBundle();

  void Reset();

  IApduChannelListener* Listener() const;

  std::unique_ptr<EndpointTcpProfileListener> tcp;
  std::unique_ptr<EndpointUdpPushProfileListener> udpPush;

private:
  EndpointListenerBundle(const EndpointListenerBundle&);
  EndpointListenerBundle& operator=(const EndpointListenerBundle&);
};

struct EndpointSecurityBundle
{
  EndpointAuthenticationKind authentication;
  dlms::security::SecurityContext context;
  bool requiresPassword;
  bool requiresCiphering;
};

EndpointStatus CreateEndpointTransport(
  const EndpointTransportOptions& options,
  EndpointTransportBundle& output);

EndpointStatus CreateEndpointProfile(
  const EndpointProfileOptions& options,
  EndpointTransportBundle& transport,
  EndpointProfileBundle& output);

EndpointStatus CreateEndpointListener(
  const EndpointTransportOptions& transport,
  const EndpointProfileOptions& profile,
  EndpointListenerBundle& output);

EndpointStatus CreateEndpointListener(
  const ServerEndpointOptions& options,
  EndpointListenerBundle& output);

EndpointStatus CreateEndpointListener(
  const PushListenerEndpointOptions& options,
  EndpointListenerBundle& output);

EndpointStatus CreateEndpointSecurity(
  const EndpointProfileOptions& profile,
  const EndpointSecurityOptions& options,
  EndpointSecurityBundle& output);

} // namespace endpoint
} // namespace dlms
