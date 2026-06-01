#include "dlms/endpoint/endpoint_factories.hpp"

#include "dlms/profile/profile_types.hpp"

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
  channelOptions.hdlcClientAddress =
    static_cast<std::uint8_t>(options.clientSap);
  channelOptions.hdlcLogicalDeviceAddress = options.serverSap;
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
  channelOptions.hdlcClientAddress =
    static_cast<std::uint8_t>(options.clientSap);
  channelOptions.hdlcLogicalDeviceAddress = options.serverSap;
  channelOptions.hdlcDirection =
    dlms::profile::HdlcProfileDirection::ServerToClient;
  channelOptions.hdlcRole = dlms::profile::HdlcProfileRole::Server;
  channelOptions.hdlcUseSession = options.hdlcUseSession;
  channelOptions.maximumApduSize = options.maxApduSize;
  return channelOptions;
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
    const dlms::profile::ProfileStatus status = channel_->Close();
    if (status == dlms::profile::ProfileStatus::Ok) {
      dataLinkAccepted_ = false;
    }
    return status;
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

EndpointTransportBundle::EndpointTransportBundle()
{
}

void EndpointTransportBundle::Reset()
{
  tcp.reset();
  udp.reset();
  serial.reset();
}

dlms::transport::IByteStream* EndpointTransportBundle::ByteStream() const
{
  if (tcp.get() != 0) {
    return tcp.get();
  }
  return serial.get();
}

dlms::transport::IDatagramTransport* EndpointTransportBundle::Datagram() const
{
  return udp.get();
}

EndpointProfileBundle::EndpointProfileBundle()
{
}

void EndpointProfileBundle::Reset()
{
  wrapperTcp.reset();
  wrapperUdp.reset();
  hdlc.reset();
}

dlms::profile::IApduChannel* EndpointProfileBundle::Channel() const
{
  if (wrapperTcp.get() != 0) {
    return wrapperTcp.get();
  }
  if (wrapperUdp.get() != 0) {
    return wrapperUdp.get();
  }
  return hdlc.get();
}

EndpointTcpProfileListener::EndpointTcpProfileListener(
  const EndpointTransportOptions& transport,
  const EndpointProfileOptions& profile)
  : transportOptions_(transport)
  , profileOptions_(profile)
  , host_(transport.host == 0 ? "" : transport.host)
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

  tcp_ = std::move(tcp);
  return EndpointStatus::Ok;
}

EndpointStatus EndpointTcpProfileListener::Close()
{
  if (tcp_.get() == 0) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = MapTransportStatus(tcp_->Close());
  if (status == EndpointStatus::Ok) {
    tcp_.reset();
  }
  return status;
}

bool EndpointTcpProfileListener::IsOpen() const
{
  return tcp_.get() != 0 && tcp_->IsOpen();
}

std::uint16_t EndpointTcpProfileListener::LocalPort() const
{
  return tcp_.get() == 0 ? 0u : tcp_->LocalPort();
}

EndpointStatus EndpointTcpProfileListener::Accept(
  std::unique_ptr<dlms::profile::IApduChannel>& channel)
{
  channel.reset();

  if (tcp_.get() == 0) {
    return EndpointStatus::InvalidState;
  }

  std::unique_ptr<dlms::transport::IByteStream> stream;
  EndpointStatus status = MapTransportStatus(tcp_->Accept(stream));
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

  udp_ = std::move(udp);
  return EndpointStatus::Ok;
}

EndpointStatus EndpointUdpPushProfileListener::Close()
{
  if (udp_.get() == 0) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = MapTransportStatus(udp_->Close());
  if (status == EndpointStatus::Ok) {
    udp_.reset();
  }
  return status;
}

bool EndpointUdpPushProfileListener::IsOpen() const
{
  return udp_.get() != 0 && udp_->IsOpen();
}

std::uint16_t EndpointUdpPushProfileListener::LocalPort() const
{
  return udp_.get() == 0 ? 0u : udp_->LocalPort();
}

EndpointStatus EndpointUdpPushProfileListener::Accept(
  std::unique_ptr<dlms::profile::IApduChannel>& channel)
{
  channel.reset();

  if (udp_.get() == 0) {
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
    new dlms::profile::WrapperUdpProfileChannel(*udp_, channelOptions));

  channel.reset(new BorrowedEndpointProfileChannel(std::move(profile)));
  return EndpointStatus::Ok;
}

EndpointListenerBundle::EndpointListenerBundle()
{
}

void EndpointListenerBundle::Reset()
{
  tcp.reset();
  udpPush.reset();
}

IApduChannelListener* EndpointListenerBundle::Listener() const
{
  if (tcp.get() != 0) {
    return tcp.get();
  }
  return udpPush.get();
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
      output.tcp.reset(
        new dlms::transport::TcpStreamTransport(transportOptions));
      return EndpointStatus::Ok;
    }
    case EndpointTransportKind::Udp: {
      dlms::transport::UdpTransportOptions transportOptions;
      transportOptions.remoteHost = options.host;
      transportOptions.remotePort = options.port;
      transportOptions.receiveTimeout = Duration(options.timeoutMs);
      transportOptions.sendTimeout = Duration(options.timeoutMs);
      output.udp.reset(new dlms::transport::UdpTransport(transportOptions));
      return EndpointStatus::Ok;
    }
    case EndpointTransportKind::Serial: {
      dlms::transport::SerialTransportOptions transportOptions;
      transportOptions.deviceName = options.serialDevice;
      transportOptions.baudRate = options.baudRate;
      transportOptions.readTimeout = Duration(options.timeoutMs);
      transportOptions.writeTimeout = Duration(options.timeoutMs);
      output.serial.reset(
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
        output.wrapperTcp.reset(
          new dlms::profile::WrapperTcpProfileChannel(
            *transport.ByteStream(),
            channelOptions));
        return EndpointStatus::Ok;
      }
      if (transport.Datagram() != 0) {
        output.wrapperUdp.reset(
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
      output.hdlc.reset(
        new dlms::profile::HdlcProfileChannel(
          *transport.ByteStream(),
          channelOptions));
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
  output.tcp.reset(new EndpointTcpProfileListener(transport, profile));
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
    output.udpPush.reset(
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
  EndpointStatus status = ValidateEndpointProfileOptions(profile);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = ValidateEndpointSecurityOptions(options);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  output.authentication = options.authentication;
  output.context = dlms::security::EmptySecurityContext();
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
