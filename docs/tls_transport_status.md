# TLS Transport Status

**Status: framework ships no built-in TLS backend.** The DLMS framework
publishes a TLS *adapter slot*, not a TLS implementation. Consumers wire
their own TLS stack into that slot.

This document is the canonical statement of what TLS support means in this
repository, and what it does not mean. It exists so that integrators do not
treat the published headers as evidence of production TLS readiness.

## What is shipped

`lib/dlms-transport/include/dlms/transport/tls_stream_transport.hpp` defines
three public types:

- `TlsStreamTransportOptions` — minimal POD config (server name + peer
  verification flag). Certificate/cipher policy is owned by the backend, not
  the framework.
- `ITlsStreamBackend` — pure-virtual integration interface
  (`Handshake` / `ReadSome` / `WriteAll` / `Close`).
- `TlsStreamTransport` — `IByteStream` adapter that owns the lifecycle
  (`Open` performs `lower->Open()` then `backend->Handshake(...)`; `Close` is
  idempotent and tears down both layers).
- `UnsupportedTlsStreamBackend` — concrete backend whose `Handshake`,
  `ReadSome`, and `WriteAll` all return `TransportStatus::UnsupportedFeature`.
  Provided so that code paths that *optionally* go through TLS can be
  exercised in builds without a real backend, and so that the absence of a
  backend is an explicit and observable contract (not a silent stub).

## What is NOT shipped

The framework does not ship any of the following:

- An OpenSSL, mbedTLS, BoringSSL, SChannel, or Secure Transport
  implementation of `ITlsStreamBackend`.
- Certificate parsing, validation, pinning, or revocation logic.
- Cipher-suite or protocol-version policy.
- Session resumption, ALPN, or SNI semantics beyond the single
  `TlsStreamTransportOptions::serverName` string.
- IEC 62056-4-7 or DLMS-UA Green Book TLS profile policy enforcement (cert
  usage extensions, pre-shared keys, etc.).

Anything in the list above is the integrator's responsibility.

## Why this design

The DLMS framework runs in environments that range from MinGW64 development
boxes to embedded RTOS targets where the TLS stack is dictated by the
platform (mbedTLS), the certificate store (OS keychain), or the compliance
regime (FIPS module, hardware secure element). Shipping a single TLS backend
would either:

- Pull a heavy mandatory dependency into every consumer, or
- Force consumers to compile out a backend they cannot use anyway.

A pluggable `ITlsStreamBackend` lets each integrator bind their own stack
without forking the framework. The framework guarantees only the
lifecycle/state-machine half — see test coverage in
`lib/dlms-transport/test/transport/test_tls_stream_transport.cpp`
(`InvalidConfigurationReturnsInvalidArgument`, `WrapsByteStreamLifecycle`,
`ReportsHandshakeFailure`, `UnsupportedBackendReportsUnsupportedFeature`).

## How to integrate a real backend

1. Implement `ITlsStreamBackend` against your TLS stack of choice. The four
   methods are sufficient — the framework never touches certificates,
   contexts, or session state directly.
2. Construct `TlsStreamTransport(lower, backend, options)` with your own
   `IByteStream` (TCP socket, serial channel, etc.) and your backend instance.
3. Use the resulting `TlsStreamTransport` anywhere an `IByteStream` is
   accepted — `WrapperTcpProfileChannel` and `HdlcProfileChannel` are
   transport-agnostic and will speak xDLMS over the encrypted stream
   transparently.
4. Apply your own policy in the backend's `Handshake`: verify peer name,
   pin certificates, enforce cipher suites, refuse legacy TLS versions, etc.

## Failure semantics

When `TlsStreamTransport` is constructed with `UnsupportedTlsStreamBackend`
(or any backend whose `Handshake` returns `UnsupportedFeature`):

- `Open()` returns `TransportStatus::UnsupportedFeature`.
- The lower `IByteStream` is closed before `Open()` returns.
- `IsOpen()` returns `false`.
- `ReadSome` and `WriteAll` return `TransportStatus::NotOpen` (because
  `open_` was never set), not `UnsupportedFeature`.

This makes "no TLS backend wired" a loud failure at connection time, not a
silent failure during the first read.

## Positioning

Production deployments that require TLS must:

- Ship their own `ITlsStreamBackend`.
- Document which TLS library is in use, including version and policy.
- Validate against the DLMS-UA Green Book TLS profile if that compliance
  level is required.

The framework itself does not claim TLS as a production-ready surface and
will not, until a vetted reference backend is upstreamed. Until that
happens, `TransportStatus::UnsupportedFeature` from `Open()` is the
contract.
