# Trace contracts (audit map)

Status: living document. Last revised: VERSION 0.100.0 (P1 «Диагностика» §2 wiring closed end-to-end).

## Scope

This file is the canonical map of every public trace contract exposed by the
DLMS stack — what each sink observes, what guarantees the framework makes,
and what each consumer is responsible for. Treat it as the audit baseline:
when a layer adds a new sink or extends an existing trace event, update this
file in the same commit.

If you are wiring traces for the first time, the short version is:

- Pick the layer you need to observe (transport / wrapper / hdlc / association).
- Implement the corresponding `I*TraceSink` interface.
- Pass the pointer through the matching options struct (or through the
  endpoint façade in `dlms-endpoint`).
- Treat every event as **diagnostic only**. None of the fields are part of
  the wire-protocol contract.

## Inventory

| Sink interface          | Layer       | Header                                                    | Event struct              | Carries raw bytes? |
| ----------------------- | ----------- | --------------------------------------------------------- | ------------------------- | ------------------ |
| `ITransportTraceSink`   | transport   | `lib/dlms-transport/include/dlms/transport/transport_trace.hpp` | `TransportTraceEvent`     | no                 |
| `IWrapperTcpTraceSink`  | profile     | `lib/dlms-profile/include/dlms/profile/profile_types.hpp` | `WrapperTcpTraceEvent`    | **yes** (raw WPDU) |
| `IHdlcProfileTraceSink` | profile     | `lib/dlms-profile/include/dlms/profile/profile_types.hpp` | `HdlcProfileTraceEvent`   | **yes** (raw HDLC) |
| `IAssociationTraceSink` | association | `lib/dlms-association/include/dlms/association/association_types.hpp` | `AssociationTraceEvent` | no (size only)    |
| `IXdlmsTraceSink`       | xDLMS       | `lib/dlms-xdlms/include/dlms/xdlms/xdlms_trace.hpp`       | `XdlmsTraceEvent`         | no (sizes + ids)   |

There is intentionally **no** xDLMS-layer or COSEM-layer trace sink today;
APDU-level visibility is provided by the byte-carrying profile sinks above
(see "Known gaps" below).

## Per-sink contract

### `ITransportTraceSink`

- Defined in `dlms-transport`. Notifies on socket-level lifecycle (`Open`,
  `Close`, `Accept`) and I/O (`Read`, `Write`, `Send`, `Receive`).
- Carries `endpoint` (string, e.g. `"127.0.0.1:4059"`), `byteCount`,
  `direction`, `status` (`TransportStatus`), and `timestampMilliseconds`.
- Does **not** carry payload bytes. Safe to log verbatim in production
  modulo `endpoint` (host/port may be sensitive for some deployments).
- `DisabledTransportTraceSink` is provided as a default no-op. Use
  `EmitTransportTrace(sink, event)` to safely emit through a possibly-null
  pointer.

### `IWrapperTcpTraceSink`

- Defined in `dlms-profile`. Notifies on every wire write/read and on the
  resulting status of the WRAPPER decode step.
- Carries `kind` (`WireWrite`, `WireRead`, `ReadStatus`, `DecodeStatus`),
  `direction`, `status` (`ProfileStatus`), source/destination wPorts,
  `encodedSize`, `apduSize`, and a **non-owning** `bytes`/`byteSize` view
  into the wire buffer.
- The `bytes` pointer is only valid for the duration of the `OnWrapperTcpTrace`
  call. Implementations that store the payload must copy.
- **Security**: `bytes` contains the literal WPDU, including the ciphered
  APDU body, the HLS challenges during association, and GMAC tags. Consumers
  must either (a) refuse to log bytes by default and gate them behind an
  explicit opt-in (as `tools/live_meter_smoke.cpp` does with
  `DLMS_LIVE_TRACE_WIRE_BYTES`), (b) redact, or (c) emit to a sink that the
  user has already accepted is sensitive (HSM-backed analyzer, encrypted
  log channel, etc.). The framework does not redact for you.

### `IHdlcProfileTraceSink`

- Defined in `dlms-profile`. Notifies on HDLC frame-level wire write/read
  plus read/decode status.
- Carries the same shape as the wrapper sink: `kind`, `direction`,
  `status`, `encodedSize`, `apduSize`, plus `bytes`/`byteSize`.
- `bytes` validity and **security** semantics are identical to
  `IWrapperTcpTraceSink`: HDLC frames carry the same protected APDU
  payload; treat as a secret.

### `IAssociationTraceSink`

- Defined in `dlms-association`. Notifies on AARQ-build success/failure and
  on AARE-receive failure.
- Carries every association parameter that already exists in the
  `AssociationOptions` (context, authentication mode, HLS mechanism,
  conformance flags, proposed QoS, max receive PDU size), plus the
  encoded AARQ size and a list of `AssociationTraceField` (tag +
  encoded size — **not** value).
- Carries `callingAuthenticationValueSize` (a count, in bytes) but
  **never** the actual authentication value. This is intentional — the
  authentication value is the HLS challenge or LLS secret and must not
  leave the security layer.
- Safe to log verbatim in production.

## Endpoint reuse

`dlms-endpoint` does not introduce its own trace types. `EndpointOptions`
exposes pass-through pointers:

- `wrapperTcpTraceSink` → forwarded to the profile layer
- `hdlcProfileTraceSink` → forwarded to the profile layer
- `associationTraceSink` → forwarded to the association layer

This keeps `dlms-endpoint` a thin façade and means audit findings on the
underlying sinks propagate to endpoint users for free.

## What the framework guarantees

For every sink interface:

1. **Lifecycle**: the framework never invokes a sink after the owning
   client/server/endpoint has been destroyed.
2. **Re-entrancy**: a sink callback may itself call public framework APIs;
   no internal lock is held across the call.
3. **Non-owning byte spans**: `bytes` (where present) is borrowed from the
   wire buffer and is valid only during the callback. Copy if you need to
   defer.
4. **No exception escape**: implementations must not throw out of the
   callback. The framework wraps APIs in status-code paths; an exception
   crossing back into the framework is undefined behaviour.

## What the consumer is responsible for

1. **Redaction**: see security notes above for wrapper/hdlc sinks.
2. **Thread safety**: a sink may be called from whatever thread owns the
   profile/transport pump. If your sink is shared, synchronise inside the
   sink.
3. **Backpressure**: sinks are synchronous. Slow sinks (disk I/O, network)
   block the protocol loop. Async/queue inside the sink if needed.
4. **Filtering**: every event is emitted unconditionally. Filter on `kind`
   or `status` inside the sink if you only want a subset.

## Cross-layer correlation (P1 §2, closed in 0.100.0)

- Every profile-layer trace event (`WrapperTcpTraceEvent`,
  `HdlcProfileTraceEvent`), every association-layer event
  (`AssociationTraceEvent`), and every xDLMS-layer event
  (`XdlmsTraceEvent`) carries an opaque `std::uint64_t conversationId`.
- The id is computed by `MakeConversationId(seed, invokeId)`
  (`lib/dlms-xdlms/include/dlms/xdlms/xdlms_correlation.hpp`) from an
  association-supplied logging seed and the current xDLMS invoke id.
  `seed == 0` ⇒ `conversationId == kNoConversationId == 0`, which is the
  default for any consumer that has not opted in.
- The seed is **logging salt only**: it is not a system title, not the HLS
  challenge, has no role in authentication or ciphering, and is never put on
  the wire. It only exists to make events from the same invocation cluster
  in logs.
- Propagation is automatic: `XdlmsClient::Get/Set/Action` calls
  `IApduChannel::SetCorrelation(conversationId)` on its bound channel just
  after allocating the invoke id. `WrapperTcpProfileChannel` and
  `HdlcProfileChannel` stamp every subsequent trace event with that id.
  Consumers do **not** need to thread the id manually.
- `IApduChannel::SetCorrelation` defaults to a no-op virtual, so custom
  channel implementations that don't care about correlation continue to
  compile and link unchanged.
- See `docs/trace_correlation_design.md` for the full contract.

## Known gaps and follow-ups

| Item                                                                                         | Tracked in                                                       |
| -------------------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| No public server-side xDLMS trace sink — client-side `IXdlmsTraceSink` shipped in 0.100.0, server-side `IXdlmsTraceSink` emission through `XdlmsServerApduProcessor` is the next deliverable. | P1 «Диагностика» §7 commit 2/3. |
| No public server-dispatch trace sink — dispatcher-level visibility (object lookup, access-rights, registry routing).                                                                       | P1 «Диагностика» §7 commit 3/3. |
| `*StatusName` naming inconsistency (`EndpointStatus::ToString` vs `*StatusName`).            | P1 «Диагностика» §5 (deferred; BREAKING).                        |

## See also

- `lib/dlms-profile/docs/04_wrapper_tcp_trace_plan.md` — wrapper-trace
  design notes.
- `lib/dlms-association/docs/07_association_trace_plan.md` — association
  trace design.
- `lib/dlms-client/docs/10_wrapper_tcp_trace_plan.md`,
  `lib/dlms-client/docs/11_association_trace_plan.md` — client-side
  consumer guidance.
- `lib/dlms-transport/docs/03_transport_test_plan.md` — transport trace
  test coverage.
- `tools/live_meter_smoke.cpp` + `tools/live_meter_smoke_byte_emit.hpp` —
  reference consumer that gates wire-byte hex dump behind
  `DLMS_LIVE_TRACE_WIRE_BYTES`.
