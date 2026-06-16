# Trace contracts (audit map)

Status: living document. Last revised: VERSION 0.99.1 (P1 «Диагностика» §1).

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

## Known gaps and follow-ups

| Item                                                                                         | Tracked in                                                       |
| -------------------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| Correlation metadata across layers (e.g. invoke-id, association-id) without leaking secrets. | P1 «Диагностика» §2.                                             |
| No public xDLMS-layer trace sink — APDU-level diagnostics are only available indirectly via the byte-carrying profile sinks. | P1 «Диагностика» §7 (added in 0.99.1). |
| No public server-side trace sink — server-side observability mirrors the client-side gap above. | P1 «Диагностика» §7.                                             |
| `*StatusName` naming inconsistency (`EndpointStatus::ToString` vs `*StatusName`).            | P1 «Диагностика» §5.                                             |

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
