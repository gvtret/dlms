#pragma once

#include "dlms/endpoint/apdu_channel_listener.hpp"
#include "dlms/endpoint/endpoint_options.hpp"

#include "dlms/profile/apdu_channel.hpp"
#include "dlms/profile/hdlc_data_link_session.hpp"
#include "dlms/security/security_types.hpp"
#include "dlms/transport/byte_stream.hpp"
#include "dlms/transport/datagram_transport.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace dlms {
namespace endpoint {

class EndpointTcpProfileListenerState;
class EndpointUdpPushProfileListenerState;

class EndpointTransportBundle
{
public:
  EndpointTransportBundle();

  void Reset();

  dlms::transport::IByteStream* ByteStream() const;
  dlms::transport::IDatagramTransport* Datagram() const;

  std::unique_ptr<dlms::transport::IByteStream> byteStream;
  std::unique_ptr<dlms::transport::IDatagramTransport> datagram;

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
  dlms::profile::IHdlcDataLinkSession* HdlcDataLink() const;

  std::unique_ptr<dlms::profile::IApduChannel> channel;
  dlms::profile::IHdlcDataLinkSession* hdlcDataLink;

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
  std::unique_ptr<EndpointTcpProfileListenerState> state_;
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
  std::unique_ptr<EndpointUdpPushProfileListenerState> state_;
};

class EndpointListenerBundle
{
public:
  EndpointListenerBundle();

  void Reset();

  IApduChannelListener* Listener() const;

  std::unique_ptr<IApduChannelListener> listener;

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
