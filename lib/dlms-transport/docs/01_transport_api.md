# 01. Transport API

## 1. Public Headers

The public API exposes protocol-neutral transport interfaces, concrete default
transports, deterministic fake transports, tracing, event-loop helpers, and the
C ABI.

```text
include/dlms/transport/transport_status.hpp
include/dlms/transport/byte_stream.hpp
include/dlms/transport/datagram_transport.hpp
include/dlms/transport/timer_scheduler.hpp
include/dlms/transport/fake_transport.hpp
include/dlms/transport/tcp_stream_transport.hpp
include/dlms/transport/tcp_server_transport.hpp
include/dlms/transport/udp_transport.hpp
include/dlms/transport/serial_transport.hpp
include/dlms/transport/tls_stream_transport.hpp
include/dlms/transport/non_blocking_byte_stream.hpp
include/dlms/transport/non_blocking_datagram_transport.hpp
include/dlms/transport/transport_event_loop.hpp
include/dlms/transport/transport_trace.hpp
include/dlms/transport/iec62056_21_mode_e.hpp
include/dlms/transport/serial_port_discovery.hpp
include/dlms/transport/transport_c_api.h
```

## 2. Status Model

`TransportStatus` reports I/O and lifecycle failures only. Higher protocol
layers must not use transport statuses for malformed HDLC, Wrapper, APDU, or
COSEM data.

```text
Ok
InvalidArgument
NotOpen
AlreadyOpen
OpenFailed
ReadFailed
WriteFailed
Timeout
ConnectionClosed
WouldBlock
OutputBufferTooSmall
UnsupportedFeature
InternalError
```

`ToString(TransportStatus)` returns stable diagnostic strings for logs and
tests.

## 3. Byte Stream API

`IByteStream` is used for TCP and serial transports.

```cpp
class IByteStream
{
public:
  virtual ~IByteStream() {}

  virtual TransportStatus Open() = 0;
  virtual TransportStatus Close() = 0;
  virtual bool IsOpen() const = 0;

  virtual TransportStatus ReadSome(
    std::uint8_t* output,
    std::size_t outputSize,
    std::size_t& bytesRead) = 0;

  virtual TransportStatus WriteAll(
    const std::uint8_t* input,
    std::size_t inputSize) = 0;
};
```

`ReadSome` may return fewer bytes than requested. `WriteAll` is required to
write the complete input or fail with a status code.

## 4. Datagram API

`IDatagramTransport` is used for UDP-like transports.

```cpp
class IDatagramTransport
{
public:
  virtual ~IDatagramTransport() {}

  virtual TransportStatus Open() = 0;
  virtual TransportStatus Close() = 0;
  virtual bool IsOpen() const = 0;

  virtual TransportStatus Send(
    const std::uint8_t* input,
    std::size_t inputSize) = 0;

  virtual TransportStatus Receive(
    std::uint8_t* output,
    std::size_t outputSize,
    std::size_t& bytesRead) = 0;
};
```

`Receive` preserves datagram boundaries. If the output buffer is too small for
the next datagram, it returns `OutputBufferTooSmall`.

## 5. Timer API

`ITimerScheduler` is intentionally small. Higher layers own timeout and retry
policy.

```cpp
struct TransportDuration
{
  std::uint32_t milliseconds;
};

class ITimerScheduler
{
public:
  virtual ~ITimerScheduler() {}

  virtual TransportStatus MonotonicMilliseconds(std::uint64_t& value) const = 0;
  virtual TransportStatus SleepFor(TransportDuration duration) = 0;
};
```

## 6. Fake Transports

`FakeByteStream`, `FakeDatagramTransport`, and `FakeTimerScheduler` support
deterministic unit and integration tests without sockets or serial devices.

They are part of the public test-support API because upper layers such as
`dlms-profile` and `dlms-association` need stable fake channels.

Fake transports are not thread-safe.

## 7. Concrete Transports And Adapters

Concrete TCP, UDP, serial, TLS, and IEC 62056-21 helpers implement the
interfaces above. They remain byte-stream or datagram transports only: Wrapper,
HDLC, APDU, retry, association, and COSEM validation belongs to higher
components.

Applications can implement `IByteStream` or `IDatagramTransport` directly when
they own the socket, serial driver, embedded link, test harness, or custom
non-blocking runtime.
