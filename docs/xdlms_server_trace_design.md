# xDLMS / server-side trace sink design

Status: **proposal** (P1 Диагностика §7).
Scope: add a public, opt-in trace surface for the xDLMS layer and for
the server-side request dispatch path, closing the visibility gap
that `docs/trace_contracts.md` calls out: today APDU- and service-level
events are only observable indirectly via the byte-carrying profile
sinks, which forces consumers to redo APDU parsing just to know what
service the framework just dispatched.

This document fixes the contracts *before* code lands, so that the
three implementation commits stay small and uncontroversial.

## Goals

1. Make every xDLMS request/response and every server-side dispatch
   observable by a consumer without (a) re-parsing APDU bytes and
   (b) coupling the consumer to internal types.
2. Reuse the same lifecycle, no-exception-escape, span-validity and
   redaction discipline already pinned for the 4 byte-layer sinks in
   `docs/trace_contracts.md`.
3. Carry the `conversationId` (already wired in §2, v0.99.4–v0.99.6)
   so a consumer can stitch xDLMS / server-side events with the
   underlying profile, association and transport events into a single
   conversation.
4. Default-off; zero observable cost (no allocation, no virtual call,
   no event construction) when the sink pointer is null.

## Non-goals

- Do **not** turn the sinks into a logging framework. They are POD
  events handed to a consumer-owned hook, identical in spirit to the
  4 existing sinks.
- Do **not** expose internal handler state (block buffers, security
  processor internals, replay state). Events describe *what happened
  on the public surface*, not how it was processed inside.
- Do **not** introduce a new transport for events (no queues, no
  async). Calls are synchronous, on the framework thread, like the
  other sinks.
- Do **not** rename or restructure the existing 4 sinks.

## Two sinks, not one

The xDLMS layer (`dlms-xdlms`) and the server-side dispatch layer
(`dlms-server`) are owned by different modules and run at different
abstraction levels, even though both currently live on the server
path:

- xDLMS sees **request/response APDUs and block-transfer state**. It
  is symmetric: both `XdlmsClient` (outbound) and
  `XdlmsServerApduProcessor` (inbound) produce events here.
- Server-side dispatch sees **resolved indications** (`GetIndication`,
  `SetIndication`, `ActionIndication`) and **the result the handler
  returned**. It is server-only.

Collapsing them into one sink would force the client side to depend
on `IXdlmsServerHandler` indication types. Splitting them keeps each
module's public types self-contained:

| Sink                          | Owner module    | Header                                 | Events emitted by               |
| ----------------------------- | --------------- | -------------------------------------- | ------------------------------- |
| `IXdlmsTraceSink`             | `dlms-xdlms`    | `dlms/xdlms/xdlms_trace.hpp`           | `XdlmsClient`, `XdlmsServerApduProcessor` |
| `IServerDispatchTraceSink`    | `dlms-server`   | `dlms/server/server_dispatch_trace.hpp`| `XdlmsServerDispatcher`         |

The endpoint composes both: `EndpointOptions` gets two new optional
pointer fields next to the existing `wrapperTcpTraceSink`,
`hdlcProfileTraceSink`, `associationTraceSink`. No old field changes.

## Sink 1 — `IXdlmsTraceSink`

### Event kinds

```
enum class XdlmsTraceKind : std::uint8_t
{
  // Outbound (XdlmsClient)
  GetRequest,
  SetRequest,
  ActionRequest,
  GetResponse,        // observed = received from peer
  SetResponse,
  ActionResponse,
  BlockTransferStep,  // any get-with-list / set-with-list / action block,
                      // direction encoded in the event

  // Inbound (XdlmsServerApduProcessor)
  RequestReceived,    // after APDU decode, before dispatch
  ResponseSent,       // after dispatch + APDU encode, before send

  // Either side
  DecodeFailed,       // APDU decode produced a non-Ok XdlmsStatus
  InvokeIdRejected,   // ValidateInvokeId / mismatch on response
  SecurityFailed,     // ciphering / replay / counter exhaustion
};
```

### Event struct

```
enum class XdlmsTraceDirection : std::uint8_t { Outbound, Inbound };

struct XdlmsTraceEvent
{
  XdlmsTraceKind     kind;
  XdlmsTraceDirection direction;
  XdlmsStatus        status;          // Ok unless the event reports a failure
  std::uint8_t       invokeId;        // 0 if not applicable
  ServiceOptions     options;         // priority/service-class/etc, value type
  std::uint16_t      classId;         // 0 if not a request/response with descriptor
  std::uint8_t       attributeOrMethodId;
  std::uint8_t       logicalName[6];  // zero-filled if not applicable
  bool               hasBlockNumber;
  std::uint32_t      blockNumber;
  std::size_t        apduSize;        // raw APDU length on the wire-facing side
  std::size_t        payloadSize;     // data/parameter size where meaningful, 0 otherwise
  std::uint64_t      conversationId;  // = 0 (kNoConversationId) by default
};
```

What is intentionally **not** in the struct:

- Raw APDU bytes. Profile-level sinks already carry those.
- Decoded `Data` payload. That would force `dlms-xdlms` to depend on
  full COSEM Data semantics and would put plaintext APDU into the
  trace. Consumers that want decoded data have direct access via
  `XdlmsClient` / `IXdlmsServerHandler` already.
- Security keys, system titles, GMAC tags, challenges. Same redaction
  contract as `AssociationTraceEvent`.

### Interface

```
class IXdlmsTraceSink
{
public:
  virtual ~IXdlmsTraceSink();
  virtual void OnXdlmsTrace(const XdlmsTraceEvent& event) = 0;
};
```

Same hard contract as the 4 existing sinks:

- Synchronous, called from the framework thread that owns the
  request/response.
- Must not throw. Implementations that may throw must catch at the
  boundary.
- The `event` reference is valid only for the call duration. Pointer
  members (none in this struct — payload sizes are sizes, not spans)
  intentionally avoid the span-validity tripwire.
- Must not call back into the originating `XdlmsClient` /
  `XdlmsServerApduProcessor` from inside the hook.

### Wiring

- `XdlmsClient` gets a constructor overload accepting
  `IXdlmsTraceSink*`. Default constructors stay; field is a stored
  raw pointer, default null. Each request/response path emits one
  event; block transfer emits one `BlockTransferStep` per block.
- `XdlmsServerApduProcessor` gets the same. `ProcessRequest` emits
  one `RequestReceived` after decode, one `ResponseSent` (or
  `DecodeFailed` / `SecurityFailed`) before returning the response
  APDU.
- Both classes already receive `conversationId` from the channel via
  the §2 plumbing. Events are stamped with it. If the consumer never
  set a correlation id, every event carries `kNoConversationId = 0`.

## Sink 2 — `IServerDispatchTraceSink`

### Event kinds

```
enum class ServerDispatchTraceKind : std::uint8_t
{
  GetDispatched,      // after handler returned, before adapter encodes result
  SetDispatched,
  ActionDispatched,
  HandlerThrew,       // dispatcher caught an exception from the handler
                      // (status surfaces as InternalError to the wire)
};
```

### Event struct

```
struct ServerDispatchTraceEvent
{
  ServerDispatchTraceKind kind;
  XdlmsStatus             status;       // handler return value (or InternalError on throw)
  std::uint8_t            invokeId;
  ServiceOptions          options;
  std::uint16_t           classId;
  std::uint8_t            attributeOrMethodId;
  std::uint8_t            logicalName[6];
  std::size_t             inputSize;    // request data/parameter size (0 for GET)
  std::size_t             outputSize;   // result data size (0 for SET / non-data ACTION)
  std::uint64_t           conversationId;
};
```

No exception payload (message, stack) is exposed: consumers should
add their own structured logging if they want exception bodies.

### Interface

```
class IServerDispatchTraceSink
{
public:
  virtual ~IServerDispatchTraceSink();
  virtual void OnServerDispatchTrace(const ServerDispatchTraceEvent& event) = 0;
};
```

Same lifecycle/no-throw/no-reentry contract.

### Wiring

- `XdlmsServerDispatcher` gets an optional `IServerDispatchTraceSink*`
  ctor parameter. Each of `DispatchGet` / `DispatchSet` /
  `DispatchAction` emits one event after the handler returns (or one
  `HandlerThrew` event from the catch site, then re-maps the
  exception to a wire-safe status as it does today).
- `conversationId` is propagated by the same `SetCorrelation` hook
  already added in §2 (the dispatcher receives it from its owner —
  the endpoint or the test fixture).

## Endpoint composition

Add two pointer fields to `EndpointOptions`:

```
IXdlmsTraceSink*           xdlmsTraceSink            = nullptr;
IServerDispatchTraceSink*  serverDispatchTraceSink   = nullptr;
```

Endpoint factories pass them through to whichever component owns the
constructor. Existing endpoint behaviour is unchanged when both
remain null.

## Headers and dependencies

- New files:
  - `lib/dlms-xdlms/include/dlms/xdlms/xdlms_trace.hpp`
  - `lib/dlms-xdlms/src/xdlms/xdlms_trace.cpp` (just out-of-line dtor +
    `Empty*` helpers)
  - `lib/dlms-server/include/dlms/server/server_dispatch_trace.hpp`
  - `lib/dlms-server/src/server/server_dispatch_trace.cpp` (same)
- `dlms-xdlms` already exports `XdlmsStatus`, `ServiceOptions`,
  `CosemAttributeDescriptor` shape via `xdlms_types.hpp` — the new
  trace header includes only `xdlms_status.hpp` + `xdlms_types.hpp`,
  no new transitive deps.
- `dlms-server` already depends on `dlms-xdlms`. The dispatch trace
  header pulls `XdlmsStatus` from there and adds no new deps.

## ABI impact

- New header, new struct, new interface, new optional ctor params and
  two new optional `EndpointOptions` fields. POD append to
  `EndpointOptions`. No changes to existing structs.
- Per pre-1.0 SemVer policy: additive public API → minor bump per
  commit that adds API. The 3 commits will land as v0.99.7 / v0.99.8
  / v0.99.9 (or fold to a single minor if they ship together — see
  commit plan).

## Implementation plan — 3 commits

1. **commit 1/3 — `IXdlmsTraceSink` introduced, wired into
   `XdlmsClient` and `XdlmsServerApduProcessor`.**
   New header + impl, ctor overloads accepting `IXdlmsTraceSink*`,
   emission sites for `GetRequest` / `SetRequest` / `ActionRequest` /
   `GetResponse` / `SetResponse` / `ActionResponse` /
   `BlockTransferStep` / `RequestReceived` / `ResponseSent` /
   `DecodeFailed` / `InvokeIdRejected` / `SecurityFailed`. Capturing
   sink in tests pins event counts, kinds and `conversationId`
   propagation. No endpoint wiring yet.

2. **commit 2/3 — `IServerDispatchTraceSink` introduced, wired into
   `XdlmsServerDispatcher`.** New header + impl, optional ctor
   parameter, emission for `GetDispatched` / `SetDispatched` /
   `ActionDispatched` / `HandlerThrew`. Capturing sink in tests pins
   the dispatch contract.

3. **commit 3/3 — Endpoint composition + integration test.** Add the
   two `EndpointOptions` fields, thread them through the factories,
   add one end-to-end test that drives a server endpoint through a
   GET, asserts the full event sequence is observable from both new
   sinks plus the existing wrapper/HDLC sink, all sharing the same
   `conversationId`. Update `docs/trace_contracts.md` to document the
   two new sinks alongside the existing 4. Update roadmap §7.

## Security review

Both sinks publish only **sizes and identifiers** (class id, attribute
id, OBIS LN, invoke id), never bytes. The same rule that
`AssociationTraceEvent` already enforces for the calling auth value
applies here: counts/sizes are fine, raw payload is not. Therefore the
new sinks add no new redaction obligations on consumers — but the
existing redaction contract in `docs/trace_contracts.md` (consumer
responsibility for any byte-carrying sink, framework responsibility
for size-only sinks) is restated for these two when §7's commit 3/3
updates `trace_contracts.md`.

## Open questions deferred to follow-up

- A *client-side* dispatch sink (`IClientFacadeTraceSink`) symmetric
  to `IServerDispatchTraceSink` would close the symmetry for the
  client facade (`DlmsClient`). It is not required by §7 (which is
  scoped to xDLMS + server-side) and would be a separate roadmap
  item; mention it once at the end of `trace_contracts.md` update.
- Whether to expose a `Reason` enum on `HandlerThrew` (instead of
  just the rewritten `InternalError` status) is deferred until a
  consumer asks for it.
