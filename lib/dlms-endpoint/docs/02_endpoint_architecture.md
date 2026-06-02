# dlms-endpoint Architecture

## Scope

`dlms-endpoint` is a composition layer. It creates endpoint-level workflows by
connecting lower repositories that already own protocol semantics.

```mermaid
flowchart TB
  App["Application / example / service"]
  Endpoint["dlms-endpoint<br/>runtime composition"]

  Client["dlms-client<br/>client facade"]
  Server["dlms-server<br/>COSEM dispatch facade"]
  XDlms["dlms-xdlms<br/>service APDU processors"]
  Assoc["dlms-association<br/>association state"]
  Security["dlms-security<br/>auth and ciphering"]
  Cosem["dlms-cosem<br/>object model"]
  Profile["dlms-profile<br/>APDU channels"]
  Transport["dlms-transport<br/>TCP UDP serial"]

  App --> Endpoint
  Endpoint --> Client
  Endpoint --> Server
  Endpoint --> XDlms
  Endpoint --> Assoc
  Endpoint --> Security
  Endpoint --> Cosem
  Endpoint --> Profile
  Endpoint --> Transport
```

## Layer Boundary

`dlms-endpoint` may know which layers must be connected. It must not duplicate
the implementation owned by those layers.

| Concern | Owning layer |
|---|---|
| TCP, UDP, serial I/O | `dlms-transport` |
| Wrapper and HDLC/LLC APDU channels | `dlms-profile` |
| AARQ, AARE, release, association state | `dlms-association` |
| GET, SET, ACTION APDU services | `dlms-xdlms` |
| HLS, GMAC, ciphered APDU processing | `dlms-security` |
| COSEM objects and access rules | `dlms-cosem` |
| COSEM dispatch over decoded xDLMS indications | `dlms-server` |
| Ergonomic client operations | `dlms-client` |
| Runtime composition and lifecycle | `dlms-endpoint` |

## Client Endpoint Flow

```mermaid
sequenceDiagram
  participant App
  participant Endpoint as ClientEndpoint
  participant Transport as dlms-transport
  participant Profile as dlms-profile
  participant Client as dlms-client
  participant Assoc as dlms-association
  participant XDlms as dlms-xdlms

  App->>Endpoint: Open()
  Endpoint->>Transport: create/open connection
  Endpoint->>Profile: create APDU channel
  Endpoint->>Client: construct facade
  Client->>Assoc: open association
  App->>Endpoint: Get()
  Endpoint->>Client: Get()
  Client->>XDlms: service request
  XDlms->>Profile: send/receive APDU
  Endpoint-->>App: status/data
```

For High GMAC, `ClientEndpoint::Open()` maps endpoint security options into
`dlms-client` HLS GMAC options. If `cipheredApdu` is enabled, HLS pass 3 is
still sent through a plain xDLMS action because the association is not
authenticated until the HLS exchange completes. After that exchange succeeds,
normal GET, SET, and ACTION requests use `dlms-security::CipheredApduProcessor`
through `dlms-xdlms`. A configured `peerSystemTitle` is forwarded as the
client security context's remote/server title; otherwise `dlms-client` uses the
AARE responding application title discovered during HLS GMAC.

## Server Endpoint Flow

```mermaid
sequenceDiagram
  participant App
  participant Endpoint as ServerEndpoint
  participant Profile as dlms-profile
  participant XDlms as dlms-xdlms
  participant Server as IServerService
  participant Cosem as dlms-cosem

  App->>Endpoint: Open()
  Endpoint->>Profile: open caller-provided channel
  opt negotiateAssociation
    Endpoint->>Assoc: AssociationServer::Accept()
    Assoc->>Profile: receive AARQ
    Assoc->>Profile: send AARE
    Endpoint->>Endpoint: apply negotiated context
  end
  App->>Endpoint: RunOnce()
  Endpoint->>Profile: receive APDU
  Endpoint->>XDlms: process service APDU
  XDlms->>Server: decoded indication
  Server->>Cosem: read/write/invoke
  Server-->>XDlms: service result
  XDlms-->>Endpoint: response APDU
  Endpoint->>Profile: send APDU
```

When server-side High GMAC is enabled, endpoint creates the HLS strategy used
by `dlms-association::AssociationServer`. The strategy receives the client
system title from the AARQ calling application title before the AARE challenge
is built. If `cipheredApdu` is enabled, `ServerEndpoint` also owns a
`SecurityContext`, key store, invocation counter store, and
`CipheredApduProcessor` for post-HLS service APDUs. The endpoint reserves the
server invocation counters consumed by AARE challenge and HLS server reply so
the first ciphered service response cannot be rejected as replay by the client.
If `peerSystemTitle` is configured, it is installed as the initial remote
system title and may be replaced by the negotiated AARQ title during HLS.
These default server facade, xDLMS adapter, security, and xDLMS processor
objects are private endpoint-owned implementation state; the public
`ServerEndpoint` header exposes the channel and server-service ports without
requiring default server, adapter, or security implementation types.

`ServerEndpoint` can also be constructed with a caller-provided
`dlms::server::IServerService`. In that mode the endpoint still owns channel
lifecycle, optional association negotiation, security processing, and xDLMS
APDU processing, but the decoded service dispatch is delegated to the supplied
server service instead of the default logical-device dispatcher.

## Push Listener Flow

```mermaid
sequenceDiagram
  participant App
  participant Endpoint as PushListenerEndpoint
  participant Profile as dlms-profile
  participant Handler as IPushIndicationHandler

  App->>Endpoint: Open()
  Endpoint->>Profile: open caller-provided channel
  App->>Endpoint: RunOnce()
  Endpoint->>Profile: receive APDU
  Endpoint->>Handler: OnPushApdu(apdu)
  Handler-->>Endpoint: status
```

## Gateway Flow

```mermaid
sequenceDiagram
  participant Downstream
  participant Gateway as GatewayEndpoint
  participant Profile as dlms-profile
  participant XDlms as dlms-xdlms
  participant Policy as IGatewayPolicy
  participant Client as IGatewayUpstream
  participant Upstream

  Gateway->>Profile: open caller-provided downstream channel
  Gateway->>Client: open upstream client path
  Downstream->>Gateway: request APDU via RunOnce()
  Gateway->>Profile: receive APDU
  Gateway->>XDlms: decode to service indication
  XDlms->>Gateway: HandleGet/Set/Action()
  Gateway->>Policy: allow request?
  Gateway->>Client: forward allowed service
  Client->>Upstream: xDLMS request
  Upstream-->>Client: xDLMS response
  Client-->>Gateway: status/data
  Gateway-->>XDlms: service result
  XDlms-->>Gateway: response APDU
  Gateway->>Profile: send APDU
  Gateway-->>Downstream: response APDU
```

`GatewayEndpoint` keeps the xDLMS server handler, dispatcher, and APDU
processor in private endpoint-owned state. Applications customize gateway
behavior through `IGatewayPolicy` and `IGatewayUpstream`, not by implementing
the xDLMS server handler directly.

## Listener Runtime Flow

```mermaid
sequenceDiagram
  participant App
  participant Runtime as ListenerRuntime
  participant Listener as IApduChannelListener
  participant Endpoint as Server/Push/Gateway Endpoint
  participant Profile as accepted IApduChannel

  App->>Runtime: Open()
  Runtime->>Listener: Open()
  App->>Runtime: RunOnce()
  Runtime->>Listener: Accept()
  Listener-->>Runtime: IApduChannel
  Runtime->>Endpoint: construct with accepted channel
  Runtime->>Endpoint: Open()
  Endpoint->>Profile: Open()
  Runtime->>Endpoint: RunOnce()
  Runtime->>Endpoint: Close()
  Endpoint->>Profile: Close()
```

Listener runtime classes own only endpoint orchestration. The listener adapter
owns how a profile channel is accepted or constructed. Each `RunOnce()` handles
at most one accepted channel, which keeps tests deterministic and leaves thread
ownership to applications.
`IApduChannelListener::LocalPort()` is part of the listener port so callers can
bind clients to ephemeral listener ports without depending on a TCP or UDP
implementation class.
The listener port lives in its own public header; concrete listener factories
and runtime classes consume that same port without requiring callers to include
both APIs.

`ServerListenerRuntime` supports both default logical-device dispatch and a
caller-provided `dlms::server::IServerService`. The runtime owns only listener
lifecycle and accepted-channel orchestration; service dispatch ownership stays
with the selected server service implementation.

For UDP push listener runtime, `Accept()` means "provide a Wrapper/UDP APDU
channel over the already-open datagram listener" rather than a connection
accept. The UDP listener owns socket lifecycle; the borrowed channel close is a
no-op so the runtime lifecycle remains attached to the listener.

## Listener Factory Flow

```mermaid
sequenceDiagram
  participant App
  participant Factory as EndpointListenerFactory
  participant TcpServer as dlms-transport TcpServerTransport
  participant Listener as EndpointTcpProfileListener
  participant Profile as dlms-profile channel

  App->>Factory: CreateEndpointListener(options)
  Factory-->>App: EndpointTcpProfileListener
  App->>Listener: Open()
  Listener->>TcpServer: Open()
  App->>Listener: Accept()
  Listener->>TcpServer: Accept(IByteStream)
  Listener->>Profile: construct Wrapper/HDLC over stream
  Listener-->>App: IApduChannel
```

The concrete listener adapter does not implement TCP accept or profile
encoding. It maps endpoint options into lower-layer objects and owns the
accepted stream/channel lifetime.

Endpoint factory bundles keep default composition private by exposing only
layer interfaces: `IByteStream`/`IDatagramTransport` for transport,
`IApduChannel` plus optional `IHdlcDataLinkSession` for profile, and
`IApduChannelListener` for listeners. This keeps applications free to replace
any layer implementation with their own object that satisfies the same
abstract port.

For HDLC listener channels, endpoint profile options choose between the default
no-session APDU framing and explicit HDLC data-link setup. With
`hdlcUseSession == true`, the accepted server channel performs the lower-layer
`AcceptDataLink()` handshake when the endpoint opens the channel; the client
side remains responsible for calling `ConnectDataLink()`.

Server-side AARQ/AARE negotiation remains below `dlms-endpoint`. The endpoint
layer may compose `dlms-association::AssociationServer`, but it must not decode
ACSE, negotiate xDLMS context, or synthesize AARE itself. The default endpoint
server/push/gateway runtime path still starts from an already-associated APDU
channel; explicit server, push listener, and gateway downstream association
negotiation is opt-in and currently limited to the no-security LN and
configured Low Password contracts exposed by `dlms-association`.

High Password and High GMAC are also composed through
`dlms-association::AssociationServer` when endpoint negotiation is enabled.
High GMAC authentication and ciphered service APDUs remain separate endpoint
options: authentication selects the ACSE/HLS mechanism, while `cipheredApdu`
selects protected xDLMS service APDUs after association. `systemTitle` denotes
the local endpoint title; `peerSystemTitle` denotes an optional preconfigured
remote endpoint title.

## Class Diagram

```mermaid
classDiagram
  class ClientEndpoint {
    +Open()
    +Get()
    +Set()
    +Action()
    +Close()
  }

  class ClientEndpointOwnedState {
    -DlmsClient client
  }

  class ServerEndpoint {
    +Open()
    +RunOnce()
    +Close()
  }

  class ServerEndpointOwnedState {
    -DlmsServer server
    -XdlmsServerAdapter adapter
  }

  class PushListenerEndpoint {
    +Open()
    +RunOnce()
    +Close()
  }

  class GatewayEndpoint {
    +Open()
    +RunOnce()
    +Close()
  }

  class IApduChannelListener {
    +Open()
    +Accept()
    +Close()
    +LocalPort()
  }

  class ServerListenerRuntime {
    +Open()
    +RunOnce()
    +Close()
  }

  class PushListenerRuntime {
    +Open()
    +RunOnce()
    +Close()
  }

  class GatewayListenerRuntime {
    +Open()
    +RunOnce()
    +Close()
  }

  class EndpointTcpProfileListener {
    +Open()
    +Accept()
    +Close()
    +LocalPort()
  }

  class EndpointUdpPushProfileListener {
    +Open()
    +Accept()
    +Close()
    +LocalPort()
  }

  class EndpointTransportFactory
  class EndpointProfileFactory
  class EndpointListenerFactory
  class EndpointSecurityFactory
  class IGatewayPolicy
  class IGatewayUpstream
  class IPushIndicationHandler
  class IServerService

  ClientEndpoint --> ClientEndpointOwnedState
  ClientEndpoint --> EndpointTransportFactory
  ClientEndpoint --> EndpointProfileFactory
  ClientEndpoint --> EndpointSecurityFactory
  ServerEndpoint --> EndpointTransportFactory
  ServerEndpoint --> EndpointProfileFactory
  ServerEndpoint --> EndpointSecurityFactory
  ServerEndpoint --> IServerService
  ServerEndpoint --> ServerEndpointOwnedState
  PushListenerEndpoint --> Profile
  PushListenerEndpoint --> IPushIndicationHandler
  GatewayEndpoint --> Profile
  GatewayEndpoint --> XDlms
  GatewayEndpoint --> IGatewayUpstream
  GatewayEndpoint --> IGatewayPolicy
  IGatewayUpstream <|.. ClientEndpointGatewayUpstream
  ClientEndpointGatewayUpstream --> ClientEndpoint
  ServerListenerRuntime --> IApduChannelListener
  ServerListenerRuntime --> ServerEndpoint
  ServerListenerRuntime --> IServerService
  PushListenerRuntime --> IApduChannelListener
  PushListenerRuntime --> PushListenerEndpoint
  GatewayListenerRuntime --> IApduChannelListener
  GatewayListenerRuntime --> GatewayEndpoint
  EndpointTcpProfileListener ..|> IApduChannelListener
  EndpointUdpPushProfileListener ..|> IApduChannelListener
  EndpointListenerFactory --> EndpointTcpProfileListener
  EndpointListenerFactory --> EndpointUdpPushProfileListener
```

## State Model

```mermaid
stateDiagram-v2
  [*] --> Closed
  Closed --> Opening: Open()
  Opening --> Open: lower layers ready
  Opening --> Faulted: error
  Open --> Running: RunOnce() / request
  Running --> Open: completed
  Running --> Faulted: unrecoverable error
  Open --> Closing: Close()
  Faulted --> Closing: Close()
  Closing --> Closed
```

## Error Model

Endpoint errors are coarse lifecycle results. The originating layer remains
responsible for precise protocol status and trace details.

## Test Strategy

The endpoint repository owns orchestration tests only. Protocol vector tests
remain in lower-layer repositories.
