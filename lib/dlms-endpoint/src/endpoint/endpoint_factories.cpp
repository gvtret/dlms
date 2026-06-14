#include "dlms/endpoint/endpoint_factories.hpp"

#include "dlms/profile/hdlc_profile_channel.hpp"
#include "dlms/profile/profile_types.hpp"
#include "dlms/profile/wrapper_tcp_profile_channel.hpp"
#include "dlms/profile/wrapper_udp_profile_channel.hpp"
#include "dlms/transport/serial_transport.hpp"
#include "dlms/transport/tcp_server_transport.hpp"
#include "dlms/transport/tcp_stream_transport.hpp"
#include "dlms/transport/udp_transport.hpp"

#include <utility>
#include <vector>

namespace {

bool IsEmpty(const char* value)
{
  return value == 0 || value[0] == '\0';
}

dlms::transport::TransportDuration Duration(
  std::uint32_t milliseconds)
{
  dlms::transport::TransportDuration duration;
  duration.milliseconds = milliseconds;
  return duration;
}

dlms::profile::ApduChannelOptions MakeApduChannelOptions(
  const dlms::endpoint::EndpointProfileOptions& options)
{
  dlms::profile::ApduChannelOptions channelOptions =
    dlms::profile::DefaultApduChannelOptions();
  channelOptions.localWrapperPort = options.clientSap;
  channelOptions.remoteWrapperPort = options.serverSap;
  channelOptions.hdlcClientAddress = options.hdlcClientAddress;
  channelOptions.hdlcLogicalDeviceAddress = options.hdlcLogicalDeviceAddress;
  channelOptions.hdlcPhysicalDeviceAddress = options.hdlcPhysicalDeviceAddress;
  channelOptions.hdlcUseSession = options.hdlcUseSession;
  channelOptions.maximumApduSize = options.maxApduSize;
  return channelOptions;
}

dlms::profile::ApduChannelOptions MakeAcceptedApduChannelOptions(
  const dlms::endpoint::EndpointProfileOptions& options)
{
  dlms::profile::ApduChannelOptions channelOptions =
    dlms::profile::DefaultApduChannelOptions();
  channelOptions.localWrapperPort = options.serverSap;
  channelOptions.remoteWrapperPort = options.clientSap;
  channelOptions.hdlcClientAddress = options.hdlcClientAddress;
  channelOptions.hdlcLogicalDeviceAddress = options.hdlcLogicalDeviceAddress;
  channelOptions.hdlcPhysicalDeviceAddress = options.hdlcPhysicalDeviceAddress;
  channelOptions.hdlcDirection =
    dlms::profile::HdlcProfileDirection::ServerToClient;
  channelOptions.hdlcRole = dlms::profile::HdlcProfileRole::Server;
  channelOptions.hdlcUseSession = options.hdlcUseSession;
  channelOptions.maximumApduSize = options.maxApduSize;
  return channelOptions;
}

dlms::endpoint::EndpointSecurityBundle EmptyEndpointSecurityBundle()
{
  dlms::endpoint::EndpointSecurityBundle bundle;
  bundle.authentication = dlms::endpoint::EndpointAuthenticationKind::None;
  bundle.context = dlms::security::EmptySecurityContext();
  bundle.requiresPassword = false;
  bundle.requiresCiphering = false;
  return bundle;
}

bool IsOptionalDisconnectReceiveStatus(dlms::profile::ProfileStatus status)
{
  return status == dlms::profile::ProfileStatus::WouldBlock ||
    status == dlms::profile::ProfileStatus::Timeout ||
    status == dlms::profile::ProfileStatus::NeedMoreData ||
    status == dlms::profile::ProfileStatus::ConnectionClosed ||
    status == dlms::profile::ProfileStatus::ReadFailed;
}

dlms::endpoint::EndpointStatus MapTransportStatus(
  dlms::transport::TransportStatus status)
{
  switch (status) {
    case dlms::transport::TransportStatus::Ok:
    case dlms::transport::TransportStatus::AlreadyOpen:
      return dlms::endpoint::EndpointStatus::Ok;
    case dlms::transport::TransportStatus::InvalidArgument:
      return dlms::endpoint::EndpointStatus::InvalidArgument;
    case dlms::transport::TransportStatus::NotOpen:
      return dlms::endpoint::EndpointStatus::InvalidState;
    case dlms::transport::TransportStatus::Timeout:
      return dlms::endpoint::EndpointStatus::Timeout;
    case dlms::transport::TransportStatus::ConnectionClosed:
      return dlms::endpoint::EndpointStatus::Closed;
    case dlms::transport::TransportStatus::UnsupportedFeature:
      return dlms::endpoint::EndpointStatus::UnsupportedProfile;
    case dlms::transport::TransportStatus::OpenFailed:
    case dlms::transport::TransportStatus::ReadFailed:
    case dlms::transport::TransportStatus::WriteFailed:
    case dlms::transport::TransportStatus::WouldBlock:
    case dlms::transport::TransportStatus::OutputBufferTooSmall:
      return dlms::endpoint::EndpointStatus::TransportFailed;
    case dlms::transport::TransportStatus::InternalError:
    default:
      return dlms::endpoint::EndpointStatus::InternalError;
  }
}

dlms::endpoint::EndpointStatus ValidateEndpointListenerTransportOptions(
  const dlms::endpoint::EndpointTransportOptions& options)
{
  if (options.timeoutMs == 0u) {
    return dlms::endpoint::EndpointStatus::InvalidArgument;
  }

  switch (options.kind) {
    case dlms::endpoint::EndpointTransportKind::Tcp:
      return IsEmpty(options.host)
        ? dlms::endpoint::EndpointStatus::InvalidArgument
        : dlms::endpoint::EndpointStatus::Ok;
    case dlms::endpoint::EndpointTransportKind::Udp:
    case dlms::endpoint::EndpointTransportKind::Serial:
      return dlms::endpoint::EndpointStatus::UnsupportedProfile;
    default:
      return dlms::endpoint::EndpointStatus::InvalidArgument;
  }
}

dlms::endpoint::EndpointStatus ValidateEndpointUdpPushListenerOptions(
  const dlms::endpoint::EndpointTransportOptions& transport,
  const dlms::endpoint::EndpointProfileOptions& profile)
{
  if (transport.timeoutMs == 0u || IsEmpty(transport.host)) {
    return dlms::endpoint::EndpointStatus::InvalidArgument;
  }
  if (transport.kind != dlms::endpoint::EndpointTransportKind::Udp) {
    return dlms::endpoint::EndpointStatus::InvalidArgument;
  }
  if (profile.kind != dlms::endpoint::EndpointProfileKind::Wrapper) {
    return dlms::endpoint::EndpointStatus::UnsupportedProfile;
  }
  return dlms::endpoint::ValidateEndpointProfileOptions(profile);
}

class AcceptedEndpointProfileChannel : public dlms::profile::IApduChannel
{
public:
  AcceptedEndpointProfileChannel(
    std::unique_ptr<dlms::transport::IByteStream> stream,
    std::unique_ptr<dlms::profile::IApduChannel> channel)
    : stream_(std::move(stream))
    , channel_(std::move(channel))
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    return channel_->Open();
  }

  dlms::profile::ProfileStatus Close()
  {
    return channel_->Close();
  }

  bool IsOpen() const
  {
    return channel_->IsOpen();
  }

  dlms::profile::ProfileStatus SendApdu(
    dlms::profile::ProfileByteView apdu)
  {
    return channel_->SendApdu(apdu);
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    std::vector<std::uint8_t>& apdu)
  {
    return channel_->ReceiveApdu(apdu);
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    return channel_->ReceiveApdu(output);
  }

private:
  AcceptedEndpointProfileChannel(const AcceptedEndpointProfileChannel&);
  AcceptedEndpointProfileChannel& operator=(
    const AcceptedEndpointProfileChannel&);

  std::unique_ptr<dlms::transport::IByteStream> stream_;
  std::unique_ptr<dlms::profile::IApduChannel> channel_;
};

class AcceptedEndpointHdlcSessionProfileChannel
  : public dlms::profile::IApduChannel
{
public:
  AcceptedEndpointHdlcSessionProfileChannel(
    std::unique_ptr<dlms::transport::IByteStream> stream,
    std::unique_ptr<dlms::profile::HdlcProfileChannel> channel)
    : stream_(std::move(stream))
    , channel_(std::move(channel))
    , dataLinkAccepted_(false)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    if (dataLinkAccepted_ && channel_->IsOpen()) {
      return dlms::profile::ProfileStatus::Ok;
    }

    dlms::profile::ProfileStatus status = channel_->Open();
    if (status != dlms::profile::ProfileStatus::Ok &&
        status != dlms::profile::ProfileStatus::AlreadyOpen) {
      return status;
    }
    status = channel_->AcceptDataLink();
    if (status == dlms::profile::ProfileStatus::Ok) {
      dataLinkAccepted_ = true;
    }
    return status;
  }

  dlms::profile::ProfileStatus Close()
  {
    dlms::profile::ProfileStatus result = dlms::profile::ProfileStatus::Ok;
    if (dataLinkAccepted_ && channel_->IsOpen()) {
      result = channel_->AcceptDisconnectDataLink();
      if (IsOptionalDisconnectReceiveStatus(result)) {
        result = dlms::profile::ProfileStatus::Ok;
      }
    }

    const dlms::profile::ProfileStatus status = channel_->Close();
    if (status == dlms::profile::ProfileStatus::Ok) {
      dataLinkAccepted_ = false;
    }
    return result == dlms::profile::ProfileStatus::Ok ? status : result;
  }

  bool IsOpen() const
  {
    return channel_->IsOpen();
  }

  dlms::profile::ProfileStatus SendApdu(
    dlms::profile::ProfileByteView apdu)
  {
    return channel_->SendApdu(apdu);
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    std::vector<std::uint8_t>& apdu)
  {
    return channel_->ReceiveApdu(apdu);
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    return channel_->ReceiveApdu(output);
  }

private:
  AcceptedEndpointHdlcSessionProfileChannel(
    const AcceptedEndpointHdlcSessionProfileChannel&);
  AcceptedEndpointHdlcSessionProfileChannel& operator=(
    const AcceptedEndpointHdlcSessionProfileChannel&);

  std::unique_ptr<dlms::transport::IByteStream> stream_;
  std::unique_ptr<dlms::profile::HdlcProfileChannel> channel_;
  bool dataLinkAccepted_;
};

class BorrowedEndpointProfileChannel : public dlms::profile::IApduChannel
{
public:
  explicit BorrowedEndpointProfileChannel(
    std::unique_ptr<dlms::profile::IApduChannel> channel)
    : channel_(std::move(channel))
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    return channel_->IsOpen()
      ? dlms::profile::ProfileStatus::Ok
      : channel_->Open();
  }

  dlms::profile::ProfileStatus Close()
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return channel_->IsOpen();
  }

  dlms::profile::ProfileStatus SendApdu(
    dlms::profile::ProfileByteView apdu)
  {
    return channel_->SendApdu(apdu);
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    std::vector<std::uint8_t>& apdu)
  {
    return channel_->ReceiveApdu(apdu);
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    return channel_->ReceiveApdu(output);
  }

private:
  BorrowedEndpointProfileChannel(const BorrowedEndpointProfileChannel&);
  BorrowedEndpointProfileChannel& operator=(
    const BorrowedEndpointProfileChannel&);

  std::unique_ptr<dlms::profile::IApduChannel> channel_;
};

void CopySystemTitle(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::uint8_t output[8])
{
  for (std::size_t index = 0u; index < 8u; ++index) {
    output[index] = index < inputSize ? input[index] : 0u;
  }
}

} // namespace

namespace dlms {
namespace endpoint {

class EndpointTcpProfileListenerState
{
public:
  std::unique_ptr<dlms::transport::TcpServerTransport> tcp;
};

class EndpointUdpPushProfileListenerState
{
public:
  std::unique_ptr<dlms::transport::UdpTransport> udp;
};

EndpointTransportBundle::EndpointTransportBundle()
{
}

void EndpointTransportBundle::Reset()
{
  byteStream.reset();
  datagram.reset();
}

dlms::transport::IByteStream* EndpointTransportBundle::ByteStream() const
{
  return byteStream.get();
}

dlms::transport::IDatagramTransport* EndpointTransportBundle::Datagram() const
{
  return datagram.get();
}

EndpointProfileBundle::EndpointProfileBundle()
  : channel()
  , hdlcDataLink(0)
{
}

void EndpointProfileBundle::Reset()
{
  channel.reset();
  hdlcDataLink = 0;
}

dlms::profile::IApduChannel* EndpointProfileBundle::Channel() const
{
  return channel.get();
}

dlms::profile::IHdlcDataLinkSession*
EndpointProfileBundle::HdlcDataLink() const
{
  return hdlcDataLink;
}

EndpointTcpProfileListener::EndpointTcpProfileListener(
  const EndpointTransportOptions& transport,
  const EndpointProfileOptions& profile)
  : transportOptions_(transport)
  , profileOptions_(profile)
  , host_(transport.host == 0 ? "" : transport.host)
  , state_(new EndpointTcpProfileListenerState())
{
  transportOptions_.host = host_.c_str();
}

EndpointTcpProfileListener::~EndpointTcpProfileListener()
{
  Close();
}

EndpointStatus EndpointTcpProfileListener::Open()
{
  if (IsOpen()) {
    return EndpointStatus::Ok;
  }

  EndpointStatus status =
    ValidateEndpointListenerTransportOptions(transportOptions_);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointProfileOptions(profileOptions_);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  dlms::transport::TcpServerTransportOptions options;
  options.host = transportOptions_.host;
  options.port = transportOptions_.port;
  options.acceptTimeout = Duration(transportOptions_.timeoutMs);
  options.readTimeout = Duration(transportOptions_.timeoutMs);
  options.writeTimeout = Duration(transportOptions_.timeoutMs);

  std::unique_ptr<dlms::transport::TcpServerTransport> tcp(
    new dlms::transport::TcpServerTransport(options));
  status = MapTransportStatus(tcp->Open());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  state_->tcp = std::move(tcp);
  return EndpointStatus::Ok;
}

EndpointStatus EndpointTcpProfileListener::Close()
{
  if (state_->tcp.get() == 0) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = MapTransportStatus(state_->tcp->Close());
  if (status == EndpointStatus::Ok) {
    state_->tcp.reset();
  }
  return status;
}

bool EndpointTcpProfileListener::IsOpen() const
{
  return state_->tcp.get() != 0 && state_->tcp->IsOpen();
}

std::uint16_t EndpointTcpProfileListener::LocalPort() const
{
  return state_->tcp.get() == 0 ? 0u : state_->tcp->LocalPort();
}

EndpointStatus EndpointTcpProfileListener::Accept(
  std::unique_ptr<dlms::profile::IApduChannel>& channel)
{
  channel.reset();

  if (state_->tcp.get() == 0) {
    return EndpointStatus::InvalidState;
  }

  std::unique_ptr<dlms::transport::IByteStream> stream;
  EndpointStatus status = MapTransportStatus(state_->tcp->Accept(stream));
  if (status != EndpointStatus::Ok) {
    return status;
  }
  if (stream.get() == 0) {
    return EndpointStatus::InternalError;
  }

  const dlms::profile::ApduChannelOptions channelOptions =
    MakeAcceptedApduChannelOptions(profileOptions_);
  std::unique_ptr<dlms::profile::IApduChannel> profile;

  switch (profileOptions_.kind) {
    case EndpointProfileKind::Wrapper:
      profile.reset(new dlms::profile::WrapperTcpProfileChannel(
        *stream,
        channelOptions));
      break;
    case EndpointProfileKind::Hdlc:
      if (profileOptions_.hdlcUseSession) {
        std::unique_ptr<dlms::profile::HdlcProfileChannel> hdlc(
          new dlms::profile::HdlcProfileChannel(*stream, channelOptions));
        channel.reset(new AcceptedEndpointHdlcSessionProfileChannel(
          std::move(stream),
          std::move(hdlc)));
        return EndpointStatus::Ok;
      }
      profile.reset(new dlms::profile::HdlcProfileChannel(
        *stream,
        channelOptions));
      break;
    default:
      return EndpointStatus::UnsupportedProfile;
  }

  channel.reset(
    new AcceptedEndpointProfileChannel(std::move(stream), std::move(profile)));
  return EndpointStatus::Ok;
}

EndpointUdpPushProfileListener::EndpointUdpPushProfileListener(
  const EndpointTransportOptions& transport,
  const EndpointProfileOptions& profile)
  : transportOptions_(transport)
  , profileOptions_(profile)
  , host_(transport.host == 0 ? "" : transport.host)
  , state_(new EndpointUdpPushProfileListenerState())
{
  transportOptions_.host = host_.c_str();
}

EndpointUdpPushProfileListener::~EndpointUdpPushProfileListener()
{
  Close();
}

EndpointStatus EndpointUdpPushProfileListener::Open()
{
  if (IsOpen()) {
    return EndpointStatus::Ok;
  }

  EndpointStatus status =
    ValidateEndpointUdpPushListenerOptions(transportOptions_, profileOptions_);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  dlms::transport::UdpTransportOptions options;
  options.localHost = transportOptions_.host;
  options.localPort = transportOptions_.port;
  options.receiveTimeout = Duration(transportOptions_.timeoutMs);
  options.sendTimeout = Duration(transportOptions_.timeoutMs);

  std::unique_ptr<dlms::transport::UdpTransport> udp(
    new dlms::transport::UdpTransport(options));
  status = MapTransportStatus(udp->Open());
  if (status != EndpointStatus::Ok) {
    return status;
  }

  state_->udp = std::move(udp);
  return EndpointStatus::Ok;
}

EndpointStatus EndpointUdpPushProfileListener::Close()
{
  if (state_->udp.get() == 0) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = MapTransportStatus(state_->udp->Close());
  if (status == EndpointStatus::Ok) {
    state_->udp.reset();
  }
  return status;
}

bool EndpointUdpPushProfileListener::IsOpen() const
{
  return state_->udp.get() != 0 && state_->udp->IsOpen();
}

std::uint16_t EndpointUdpPushProfileListener::LocalPort() const
{
  return state_->udp.get() == 0 ? 0u : state_->udp->LocalPort();
}

EndpointStatus EndpointUdpPushProfileListener::Accept(
  std::unique_ptr<dlms::profile::IApduChannel>& channel)
{
  channel.reset();

  if (state_->udp.get() == 0) {
    return EndpointStatus::InvalidState;
  }

  EndpointStatus status =
    ValidateEndpointUdpPushListenerOptions(transportOptions_, profileOptions_);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  const dlms::profile::ApduChannelOptions channelOptions =
    MakeAcceptedApduChannelOptions(profileOptions_);
  std::unique_ptr<dlms::profile::IApduChannel> profile(
    new dlms::profile::WrapperUdpProfileChannel(
      *state_->udp,
      channelOptions));

  channel.reset(new BorrowedEndpointProfileChannel(std::move(profile)));
  return EndpointStatus::Ok;
}

EndpointListenerBundle::EndpointListenerBundle()
{
}

void EndpointListenerBundle::Reset()
{
  listener.reset();
}

IApduChannelListener* EndpointListenerBundle::Listener() const
{
  return listener.get();
}

EndpointStatus CreateEndpointTransport(
  const EndpointTransportOptions& options,
  EndpointTransportBundle& output)
{
  EndpointStatus status = ValidateEndpointTransportOptions(options);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  output.Reset();

  switch (options.kind) {
    case EndpointTransportKind::Tcp: {
      dlms::transport::TcpStreamTransportOptions transportOptions;
      transportOptions.host = options.host;
      transportOptions.port = options.port;
      transportOptions.connectTimeout = Duration(options.timeoutMs);
      transportOptions.readTimeout = Duration(options.timeoutMs);
      transportOptions.writeTimeout = Duration(options.timeoutMs);
      output.byteStream.reset(
        new dlms::transport::TcpStreamTransport(transportOptions));
      return EndpointStatus::Ok;
    }
    case EndpointTransportKind::Udp: {
      dlms::transport::UdpTransportOptions transportOptions;
      transportOptions.remoteHost = options.host;
      transportOptions.remotePort = options.port;
      transportOptions.receiveTimeout = Duration(options.timeoutMs);
      transportOptions.sendTimeout = Duration(options.timeoutMs);
      output.datagram.reset(
        new dlms::transport::UdpTransport(transportOptions));
      return EndpointStatus::Ok;
    }
    case EndpointTransportKind::Serial: {
      dlms::transport::SerialTransportOptions transportOptions;
      transportOptions.deviceName = options.serialDevice;
      transportOptions.baudRate = options.baudRate;
      transportOptions.readTimeout = Duration(options.timeoutMs);
      transportOptions.writeTimeout = Duration(options.timeoutMs);
      output.byteStream.reset(
        new dlms::transport::SerialTransport(transportOptions));
      return EndpointStatus::Ok;
    }
    default:
      return EndpointStatus::InvalidArgument;
  }
}

EndpointStatus CreateEndpointProfile(
  const EndpointProfileOptions& options,
  EndpointTransportBundle& transport,
  EndpointProfileBundle& output)
{
  EndpointStatus status = ValidateEndpointProfileOptions(options);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  output.Reset();

  const dlms::profile::ApduChannelOptions channelOptions =
    MakeApduChannelOptions(options);

  switch (options.kind) {
    case EndpointProfileKind::Wrapper:
      if (transport.ByteStream() != 0) {
        output.channel.reset(
          new dlms::profile::WrapperTcpProfileChannel(
            *transport.ByteStream(),
            channelOptions));
        return EndpointStatus::Ok;
      }
      if (transport.Datagram() != 0) {
        output.channel.reset(
          new dlms::profile::WrapperUdpProfileChannel(
            *transport.Datagram(),
            channelOptions));
        return EndpointStatus::Ok;
      }
      return EndpointStatus::InvalidState;
    case EndpointProfileKind::Hdlc:
      if (transport.ByteStream() == 0) {
        return EndpointStatus::InvalidState;
      }
      {
        std::unique_ptr<dlms::profile::HdlcProfileChannel> hdlc(
          new dlms::profile::HdlcProfileChannel(
            *transport.ByteStream(),
            channelOptions));
        output.hdlcDataLink = hdlc.get();
        output.channel = std::move(hdlc);
      }
      return EndpointStatus::Ok;
    default:
      return EndpointStatus::UnsupportedProfile;
  }
}

EndpointStatus CreateEndpointListener(
  const EndpointTransportOptions& transport,
  const EndpointProfileOptions& profile,
  EndpointListenerBundle& output)
{
  EndpointStatus status = ValidateEndpointListenerTransportOptions(transport);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointProfileOptions(profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  output.Reset();
  output.listener.reset(new EndpointTcpProfileListener(transport, profile));
  return EndpointStatus::Ok;
}

EndpointStatus CreateEndpointListener(
  const ServerEndpointOptions& options,
  EndpointListenerBundle& output)
{
  EndpointStatus status = ValidateEndpointSecurityOptions(options.security);
  if (status != EndpointStatus::Ok) {
    return status;
  }
  return CreateEndpointListener(options.transport, options.profile, output);
}

EndpointStatus CreateEndpointListener(
  const PushListenerEndpointOptions& options,
  EndpointListenerBundle& output)
{
  EndpointStatus status = ValidateEndpointSecurityOptions(options.security);
  if (status != EndpointStatus::Ok) {
    return status;
  }
  if (options.transport.kind == EndpointTransportKind::Udp) {
    status = ValidateEndpointUdpPushListenerOptions(
      options.transport,
      options.profile);
    if (status != EndpointStatus::Ok) {
      return status;
    }
    output.Reset();
    output.listener.reset(
      new EndpointUdpPushProfileListener(options.transport, options.profile));
    return EndpointStatus::Ok;
  }
  return CreateEndpointListener(options.transport, options.profile, output);
}

EndpointStatus CreateEndpointSecurity(
  const EndpointProfileOptions& profile,
  const EndpointSecurityOptions& options,
  EndpointSecurityBundle& output)
{
  output = EmptyEndpointSecurityBundle();

  EndpointStatus status = ValidateEndpointProfileOptions(profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointSecurityOptions(options);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  output.authentication = options.authentication;
  output.context.suite = dlms::security::SecuritySuite::Suite0;
  output.context.role = dlms::security::SecurityRole::Client;
  output.context.clientSap = profile.clientSap;
  output.context.serverSap = profile.serverSap;
  output.requiresPassword = false;
  output.requiresCiphering = false;

  switch (options.authentication) {
    case EndpointAuthenticationKind::None:
      output.context.policy = dlms::security::SecurityPolicy::None;
      return EndpointStatus::Ok;
    case EndpointAuthenticationKind::LowPassword:
    case EndpointAuthenticationKind::HighPassword:
      output.context.policy = dlms::security::SecurityPolicy::Authenticated;
      output.requiresPassword = true;
      return EndpointStatus::Ok;
    case EndpointAuthenticationKind::HighGmac:
      output.context.policy = options.cipheredApdu
        ? dlms::security::SecurityPolicy::AuthenticatedAndEncrypted
        : dlms::security::SecurityPolicy::Authenticated;
      output.requiresCiphering = options.cipheredApdu;
      CopySystemTitle(
        options.systemTitle,
        options.systemTitleSize,
        output.context.localSystemTitle);
      CopySystemTitle(
        options.peerSystemTitle,
        options.peerSystemTitleSize,
        output.context.remoteSystemTitle);
      return EndpointStatus::Ok;
    default:
      return EndpointStatus::InvalidArgument;
  }
}

} // namespace endpoint
} // namespace dlms
