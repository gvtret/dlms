# dlms-endpoint API

## Header Layout

Public headers:

```text
include/dlms/endpoint/endpoint.hpp
include/dlms/endpoint/endpoint_status.hpp
include/dlms/endpoint/endpoint_options.hpp
include/dlms/endpoint/client_endpoint.hpp
include/dlms/endpoint/server_endpoint.hpp
include/dlms/endpoint/push_listener_endpoint.hpp
include/dlms/endpoint/gateway_endpoint.hpp
include/dlms/endpoint/listener_runtime.hpp
include/dlms/endpoint/endpoint_factories.hpp
```

## Status Model

```cpp
enum class EndpointStatus
{
  Ok,
  InvalidArgument,
  InvalidState,
  UnsupportedProfile,
  TransportFailed,
  ProfileFailed,
  AssociationFailed,
  SecurityFailed,
  ServiceFailed,
  Timeout,
  Closed,
  InternalError
};
```

Statuses map lower-layer failures without hiding their layer category. Detailed
lower-layer traces remain owned by the originating layer.

## Endpoint Options

```cpp
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

struct ServerEndpointOptions
{
  EndpointTransportOptions transport;
  EndpointProfileOptions profile;
  EndpointSecurityOptions security;
  bool negotiateAssociation;
};
```

The endpoint layer translates these options into existing layer-specific
configuration objects. `systemTitle` is the local system title for the endpoint
being opened. `peerSystemTitle` is the optional remote system title for peers
that are known before association; HLS GMAC clients and servers can still
discover it from AARE/AARQ titles. `authenticationKey` is required for High
GMAC authentication; `globalUnicastEncryptionKey` is optional for High GMAC
authentication-only profiles and required when `cipheredApdu` is `true`. It
does not parse JSON or own persistent configuration.
For HDLC profiles, `clientSap` must fit the single-octet HDLC client address
range (`1..127`) and `serverSap` must fit the HDLC logical device address
range (`1..16383`). Wrapper profiles keep the full 16-bit wrapper port range.

`EndpointAuthenticationKind::HighGmac` selects the HLS GMAC association
mechanism. It does not, by itself, select ciphered APDUs. This separation is
intentional because DLMS deployments can use HLS GMAC only for association
authentication and still exchange plain service APDUs afterward.

`cipheredApdu == true` selects authenticated-and-encrypted xDLMS service APDUs
after association succeeds. The endpoint validates this mode only with
High GMAC, an 8-byte local `systemTitle`, a 16-byte `authenticationKey`, and a
16-byte `globalUnicastEncryptionKey`. Client endpoints use
`peerSystemTitle` when provided, otherwise they discover the server system
title from the AARE responding application title during HLS GMAC before the
first ciphered APDU. Server endpoints discover the client system title from the
AARQ calling application title. The Association LN HLS reply itself remains
plain; ciphered APDUs start with subsequent GET, SET, and ACTION services.

## Client Endpoint

```cpp
using ClientAttributeDescriptor = dlms::client::CosemAttributeDescriptor;
using ClientMethodDescriptor = dlms::client::CosemMethodDescriptor;
```

```cpp
class ClientEndpoint
{
public:
  explicit ClientEndpoint(const ClientEndpointOptions& options);

  EndpointStatus Open();
  EndpointStatus Close();

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
};
```

`ClientEndpoint` owns the composed transport/profile/client objects behind
private endpoint state. It supports Wrapper/TCP and HDLC/TCP client
composition; serial client endpoints remain unsupported by the public endpoint
facade. Client endpoint validation returns `UnsupportedProfile` for non-TCP
transports.
`Close()` first attempts graceful association release. If that release fails,
it still closes and drops the local client resources, then returns the release
error to the caller.

## Server Endpoint

```cpp
class ServerEndpoint
{
public:
  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    dlms::cosem::LogicalDevice& logicalDevice);

  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    dlms::server::IServerService& server);

  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    const ServerEndpointOptions& options,
    dlms::cosem::LogicalDevice& logicalDevice);

  ServerEndpoint(
    dlms::profile::IApduChannel& channel,
    const ServerEndpointOptions& options,
    dlms::server::IServerService& server);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;
  dlms::server::ServerContext& Context();
};
```

`ServerEndpoint` runs the already-associated server APDU path over a caller
provided `dlms::profile::IApduChannel`. It does not own COSEM object storage:
the caller owns the logical device and registered objects. By default
`ServerEndpointOptions::negotiateAssociation` is `false`, preserving the
already-associated path for callers that have already completed ACSE
association negotiation.

The logical-device constructors compose the default `dlms::server::DlmsServer`
dispatcher. The `IServerService&` constructors keep the endpoint lifecycle,
association, security, and xDLMS APDU processing in `dlms-endpoint`, but route
decoded GET/SET/ACTION requests to a caller-provided server implementation.

`ServerListenerRuntime` mirrors this boundary for accepted channels:

```cpp
class ServerListenerRuntime
{
public:
  ServerListenerRuntime(
    IApduChannelListener& listener,
    dlms::cosem::LogicalDevice& logicalDevice);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    dlms::server::IServerService& server);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    const ServerEndpointOptions& options,
    dlms::cosem::LogicalDevice& logicalDevice);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    const ServerEndpointOptions& options,
    dlms::server::IServerService& server);
};
```

Each accepted channel is wrapped in a short-lived `ServerEndpoint` using either
the default logical-device dispatcher or the supplied server service.

When `negotiateAssociation` is `true`, `Open()` composes
`dlms-association::AssociationServer` over the same APDU channel before the
first service APDU. It receives one AARQ, emits one AARE, then stores the
negotiated context in `ServerContext`. Endpoint stores the negotiated
association through `dlms::association::IAssociationServer`; endpoint code does
not decode ACSE or construct AARE itself.

After negotiated association, `RunOnce()` treats an incoming RLRQ as a
server-side release request. It delegates RLRE emission and channel close to
`dlms-association::AssociationServer`, clears `ServerContext` association
metadata, and marks the endpoint closed. Endpoint listener/runtime factories
own TCP accept loops and profile-specific accepted-channel construction.

When endpoint listener factories create an HDLC accepted channel with
`EndpointProfileOptions::hdlcUseSession == true`, the accepted server-side
channel performs `HdlcProfileChannel::AcceptDataLink()` during endpoint
`Open()`. Client endpoints pass the same flag into `dlms-client`; clients call
`ConnectDataLink()` only when the flag is enabled. The default remains
no-session HDLC framing.

Association negotiation composition supports the no-security LN path, Low
Password, High Password, and High GMAC authentication. Low Password passes
`EndpointSecurityOptions::password` to
`dlms-association::AssociationServer`. High Password and High GMAC use endpoint
HLS strategies backed by `dlms-security`. With High GMAC, endpoint ACSE carries
the local system title as application title metadata so both peers can build
the GMAC security context deterministically.

## Push Listener Endpoint

```cpp
class IPushIndicationHandler
{
public:
  virtual ~IPushIndicationHandler();
  virtual EndpointStatus OnPushApdu(
    const std::vector<std::uint8_t>& apdu) = 0;
};

class PushListenerEndpoint
{
public:
  PushListenerEndpoint(
    dlms::profile::IApduChannel& channel,
    IPushIndicationHandler& handler);

  PushListenerEndpoint(
    dlms::profile::IApduChannel& channel,
    const PushListenerEndpointOptions& options,
    IPushIndicationHandler& handler);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;
};
```

`PushListenerEndpoint` owns the endpoint lifecycle and receives one push APDU
from a caller-provided `dlms::profile::IApduChannel` per `RunOnce()` call. It
passes the raw APDU to `IPushIndicationHandler`. Push APDU semantic decoding
will be narrowed once push service primitives are present in `dlms-xdlms`.

By default `PushListenerEndpointOptions::negotiateAssociation` is `false`,
preserving the raw already-associated push APDU path. When enabled, `Open()`
composes `dlms-association::AssociationServer` over the same APDU channel
before the first raw push APDU is dispatched. Endpoint code still does not
decode ACSE or construct AARE itself, and stores the negotiated association
through `dlms::association::IAssociationServer`. The initial push listener
composition is no-security LN or configured Low Password, matching the current
association server contract.

After negotiated association, `RunOnce()` treats an incoming RLRQ as a
server-side release request. It delegates RLRE emission and channel close to
`dlms-association::AssociationServer` and does not dispatch the release APDU to
`IPushIndicationHandler`.

## Gateway Endpoint

```cpp
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

class GatewayEndpoint
{
public:
  GatewayEndpoint(
    dlms::profile::IApduChannel& downstreamChannel,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);

  GatewayEndpoint(
    dlms::profile::IApduChannel& downstreamChannel,
    const GatewayEndpointOptions& options,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;
};
```

`GatewayEndpoint` receives one downstream xDLMS request APDU from a
caller-provided channel, uses `dlms-xdlms` server processing to decode it into a
GET, SET, or ACTION indication, checks `IGatewayPolicy`, forwards allowed
services to `IGatewayUpstream`, and sends the encoded response APDU downstream.
Denied requests are encoded as xDLMS access/action result `access-denied`.
`ClientEndpointGatewayUpstream` adapts an existing `ClientEndpoint` to
`IGatewayUpstream`.

The gateway layer composes downstream request handling and upstream client
calls. It does not implement COSEM object semantics or own listener transport.
Its internal xDLMS server handler and APDU processor are private
implementation state; the public endpoint contract is the downstream channel,
`IGatewayUpstream`, and `IGatewayPolicy`.
`Close()` reports an upstream close failure when one occurs, but still attempts
to close the downstream channel and clears local gateway state if downstream
close succeeds.

By default `GatewayEndpointOptions::downstream.negotiateAssociation` is
`false`, preserving the already-associated gateway path. When enabled,
`GatewayEndpoint::Open()` composes
`dlms-association::AssociationServer` over the downstream APDU channel before
opening the upstream path. It accepts one downstream AARQ, emits one AARE, and
then `RunOnce()` forwards the first xDLMS service APDU. Endpoint code still
does not decode ACSE or construct AARE itself, and stores the negotiated
association through `dlms::association::IAssociationServer`. The gateway
composition supports no-security LN and configured Low Password, matching the
current association server contract.

After downstream association negotiation, `RunOnce()` treats an incoming RLRQ
as a downstream release request. It delegates RLRE emission and downstream
channel close to `dlms-association::AssociationServer`, closes the already-open
upstream path, and marks the gateway closed without invoking GET, SET, or
ACTION upstream services.

## Listener Runtime

```cpp
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

class ServerListenerRuntime
{
public:
  ServerListenerRuntime(
    IApduChannelListener& listener,
    dlms::cosem::LogicalDevice& logicalDevice);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    dlms::server::IServerService& server);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    const ServerEndpointOptions& options,
    dlms::cosem::LogicalDevice& logicalDevice);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    const ServerEndpointOptions& options,
    dlms::server::IServerService& server);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();
  bool IsOpen() const;
};

class PushListenerRuntime
{
public:
  PushListenerRuntime(
    IApduChannelListener& listener,
    IPushIndicationHandler& handler);

  PushListenerRuntime(
    IApduChannelListener& listener,
    const PushListenerEndpointOptions& options,
    IPushIndicationHandler& handler);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();
  bool IsOpen() const;
};

class GatewayListenerRuntime
{
public:
  GatewayListenerRuntime(
    IApduChannelListener& downstreamListener,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);

  GatewayListenerRuntime(
    IApduChannelListener& downstreamListener,
    const GatewayEndpointOptions& options,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();
  bool IsOpen() const;
};
```

`IApduChannelListener` abstracts the transport/profile accept step. The
endpoint layer only controls lifecycle and accepted APDU-channel ownership; TCP
accept, Wrapper/HDLC construction, and serial details stay in the lower layers
or caller-provided adapters.

`ServerListenerRuntime`, `PushListenerRuntime`, and `GatewayListenerRuntime`
accept one channel per `RunOnce()` and then delegate to `ServerEndpoint`,
`PushListenerEndpoint`, or `GatewayEndpoint`. They do not own background
threads or unbounded loops.

## Listener Factories

```cpp
class EndpointTcpProfileListener : public IApduChannelListener
{
public:
  EndpointTcpProfileListener(
    const EndpointTransportOptions& transport,
    const EndpointProfileOptions& profile);

  EndpointStatus Open();
  EndpointStatus Close();
  bool IsOpen() const;
  std::uint16_t LocalPort() const;

  EndpointStatus Accept(
    std::unique_ptr<dlms::profile::IApduChannel>& channel);
};

class EndpointUdpPushProfileListener : public IApduChannelListener
{
public:
  EndpointUdpPushProfileListener(
    const EndpointTransportOptions& transport,
    const EndpointProfileOptions& profile);

  EndpointStatus Open();
  EndpointStatus Close();
  bool IsOpen() const;
  std::uint16_t LocalPort() const;

  EndpointStatus Accept(
    std::unique_ptr<dlms::profile::IApduChannel>& channel);
};

class EndpointListenerBundle
{
public:
  void Reset();
  IApduChannelListener* Listener() const;

  std::unique_ptr<IApduChannelListener> listener;
};

class EndpointTransportBundle
{
public:
  void Reset();
  dlms::transport::IByteStream* ByteStream() const;
  dlms::transport::IDatagramTransport* Datagram() const;

  std::unique_ptr<dlms::transport::IByteStream> byteStream;
  std::unique_ptr<dlms::transport::IDatagramTransport> datagram;
};

class EndpointProfileBundle
{
public:
  void Reset();
  dlms::profile::IApduChannel* Channel() const;
  dlms::profile::IHdlcDataLinkSession* HdlcDataLink() const;

  std::unique_ptr<dlms::profile::IApduChannel> channel;
  dlms::profile::IHdlcDataLinkSession* hdlcDataLink;
};

struct EndpointSecurityBundle
{
  EndpointAuthenticationKind authentication;
  dlms::security::SecurityContext context;
  bool requiresPassword;
  bool requiresCiphering;
};

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
```

`EndpointTcpProfileListener` adapts `dlms-transport::TcpServerTransport` to
`IApduChannelListener` and constructs the accepted `dlms-profile` channel over
the accepted byte stream. Accepted channel ownership includes both the byte
stream and the profile channel, so runtime callers only handle
`IApduChannel`.

`EndpointUdpPushProfileListener` adapts one opened
`dlms-transport::UdpTransport` to `IApduChannelListener` for push listener
runtime only. `Accept()` returns a Wrapper/UDP APDU channel over the listener
datagram transport; endpoint channel `Close()` does not close the UDP listener,
so caller-controlled runtime lifecycle remains on the listener.

Factory bundles expose lower-layer resources through abstract layer ports.
`EndpointTransportBundle` owns either an `IByteStream` or an
`IDatagramTransport`, `EndpointProfileBundle` owns an `IApduChannel`, and HDLC
profiles expose an optional `IHdlcDataLinkSession*` when explicit data-link
session control is available. `EndpointListenerBundle` owns only an
`IApduChannelListener`. Concrete default transports and profile channels are
created in the endpoint factory implementation, not exposed as bundle fields.
