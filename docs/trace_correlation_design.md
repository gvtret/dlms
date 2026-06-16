# Cross-layer trace correlation metadata (design)

Status: design accepted. Last revised: VERSION 0.99.3 (P1 «Диагностика» §2,
design phase).

## Problem

A DLMS interaction crosses four observable layers (transport → wrapper/HDLC
profile → xDLMS APDU → association). Each layer exposes its own trace sink
(`docs/trace_contracts.md`). Today there is **no way to stitch a single
request together** from those four streams: when a `WrapperTcpTraceEvent`
fires you cannot tell which COSEM GET caused it, and when an `xDLMS`
service fails you cannot point at the exact wrapper frame that carried it.

For real support work this means: logs from a busy meter session look like
a thousand independent events. Anything beyond "did it succeed?" requires
hand-correlation by timestamp, which breaks under any concurrency.

## Constraints

1. **No new secrets on the wire and no new secrets in trace events.** The
   correlation must come from values that already exist in the protocol
   state machine (invoke-id, wrapper port pair, AARQ sequence, etc.) — not
   from random tokens we mint and ship.
2. **No ABI break in 0.x.** All four event structs are public POD. We are
   allowed to **append** plain-old-data fields (existing zero-initialised
   constructors keep wire-source events backwards compatible for both
   readers and writers compiled against the new headers), but not to remove
   or reorder.
3. **Trace sinks are synchronous and re-entrant** (`trace_contracts.md`).
   We cannot introduce a global mutex or a hidden thread-local stack — the
   correlation id must travel through call stacks, not through ambient
   state.
4. **Zero-cost when no sink is wired.** All four sinks today no-op when
   the pointer is null (`EmitTrace`/`EmitHdlcTrace`/`EmitTransportTrace`/
   `EmitAssociationTrace` guards). Correlation field population must be
   gated by the same null-check.
5. **AGENTS.md §3.** Surgical changes only; one struct extension per
   commit; do not "tidy up" adjacent event fields while wiring this.

## Identifiers already present per layer

| Layer              | Identifier that uniquely identifies a request | Source                                              |
| ------------------ | ------------------------------------------- | ---------------------------------------------------- |
| Transport          | `endpoint` string + `byteCount` direction   | already in `TransportTraceEvent`                     |
| Wrapper TCP        | `(sourcePort, destinationPort)` + monotone WPDU sequence | partly present, WPDU sequence implicit  |
| HDLC               | HDLC sequence numbers `N(R) / N(S)`         | inside framed bytes, not exposed                     |
| xDLMS APDU         | `invokeIdAndPriority & 0x0F` (4-bit invoke-id) | already used by `xdlms_client.cpp` (lines 19, 376, 410, 427, 487) |
| Association        | AARQ↔AARE pair (one per associated session) | implicit; no current id                              |

## Design

### Core idea

Introduce a single, opaque, **non-secret** 64-bit value called the
**conversation id**. One conversation = one application-level request from
the moment xDLMS hands an APDU down to the moment the response comes back
up. The xDLMS layer is the *origin* of the conversation id (it already
owns the invoke-id) and the profile/transport layers are *carriers* — they
copy the value into their trace events whenever a sink is attached.

The conversation id is **never** placed on the wire. It is purely a
diagnostic correlator carried through the in-process call stack.

```
+---------------------+ conversationId = (epoch_seed ^ invoke_id)
|     xDLMS layer     |---------------+
+---------------------+               |
                                      v
+---------------------+   wrapper trace event {... conversationId}
|  Profile (wrapper)  |---------------+
+---------------------+               |
                                      v
+---------------------+   transport trace event {... conversationId}
|     Transport       |
+---------------------+
```

### Field shape

Add to each of the four `*TraceEvent` structs, as the **last** field, in
this exact order to keep one definition per layer:

```cpp
// 0 means "no correlation context" (legacy / zero-initialised events).
std::uint64_t conversationId;
```

That is it. One field, one type, one meaning across all four layers.

We deliberately do **not** add:

- A separate "associationId" or "sessionId" — the conversationId is
  already monotonically per-association in practice, and adding a
  parallel id doubles the ABI surface.
- A pointer to anything — pointers in trace events have non-trivial
  lifetime rules (`trace_contracts.md` §"Non-owning byte spans"). A
  plain integer is reentrant-safe and copy-safe.
- Per-layer subfields ("wrapper-side seq", "hdlc N(S)") — those are
  layer-local diagnostics, not correlation. Layers that want to expose
  them get their own follow-up.

### Construction rule

```cpp
constexpr std::uint64_t MakeConversationId(
    std::uint64_t associationSeed,
    std::uint8_t invokeId) noexcept
{
  // associationSeed is xored, not shifted — invoke-id reuse across
  // associations still produces distinct ids as long as the seeds
  // differ. The low nibble is preserved so a human can still spot the
  // invoke-id in a hex dump.
  return (associationSeed & ~static_cast<std::uint64_t>(0x0F)) |
         (static_cast<std::uint64_t>(invokeId) & 0x0F);
}
```

`associationSeed` is generated once per `AssociationClient` /
`AssociationServer` and is a non-secret random `uint64_t`. It is **not**
the system-title and **not** the HLS challenge — it is purely a logging
salt with no security role.

### Propagation rule

- xDLMS APDU send paths (`SendGetRequest`, `SendSetRequest`,
  `SendActionRequest`, block-transfer continuation) compute the
  conversationId from the current invokeId and pass it down through the
  existing `IApduChannel` write/read calls.
- `IApduChannel` gets one new method:
  ```cpp
  virtual void SetCorrelation(std::uint64_t conversationId) noexcept;
  ```
  Default-implemented to ignore. Wrapper- and HDLC-backed channels store
  the value in a single `std::atomic<std::uint64_t>` (correlation is
  serialised against trace emission, not against I/O — sequential by
  construction).
- Each `EmitTrace`/`EmitHdlcTrace` site fills `event.conversationId`
  from that stored value before invoking the sink.
- The association layer stores the seed and stamps its three events
  (`AarqBuilt`, `AarqBuildFailed`, `AareReceiveFailed`) with
  `MakeConversationId(seed, /*invokeId=*/0)`. The low nibble of `0` is
  not a valid invoke-id, so association events stand out from
  service-level events from the same session.

### What this does **not** do

- Does **not** correlate across two physically separate processes
  (no distributed-trace id, no W3C traceparent). That is out of scope.
- Does **not** correlate ciphered-APDU pieces from independent block
  transfers issued on top of the same invoke-id (block transfer reuses
  the id by construction; consumers that need per-block correlation can
  combine `conversationId` with the existing `dataBlock.blockNumber`
  already in the trace path).
- Does **not** add a new sink. The single field is enough; if a
  consumer wants a per-conversation rollup it can keep its own
  `unordered_map<uint64_t, …>` keyed by `conversationId`.

## Implementation plan

Three commits, each independently testable, all backwards-compatible:

1. **Commit 1 — primitives only.** Add `MakeConversationId` to
   `dlms-xdlms` (header-only, `constexpr`, no link impact). Unit-tests
   pin: identity of the formula, invoke-id low-nibble preserved, two
   distinct seeds with the same invoke-id give distinct ids.

2. **Commit 2 — wire the field.** Extend `TransportTraceEvent`,
   `WrapperTcpTraceEvent`, `HdlcProfileTraceEvent`,
   `AssociationTraceEvent` with the `conversationId` field, defaulting
   to `0` in every constructor / zero-initialised path. `IApduChannel`
   gets the no-op `SetCorrelation`. Existing consumers continue to
   build (POD append + virtual default implementation = no ABI break
   for static/dynamic linkers in 0.x). Unit-tests pin: zero-init →
   `conversationId == 0`; structured init → value propagates.

3. **Commit 3 — populate the field.** xDLMS service paths call
   `channel.SetCorrelation(MakeConversationId(seed, invokeId))` once
   per request. `WrapperTcpProfileChannel` and `HdlcProfileChannel`
   override `SetCorrelation`, store the atomic, copy into emitted
   events. Association layer stamps its three events from its own
   seed. Integration tests pin: a single GET produces a sequence of
   trace events whose `conversationId` is equal across all four
   layers.

Each commit bumps `VERSION` patch (no behaviour change for callers
without sinks, no ABI break). The third commit closes P1 §2.

## Test plan

For each commit a corresponding test fixture lives next to the existing
trace tests:

- `lib/dlms-xdlms/test/xdlms/test_conversation_id.cpp` (commit 1)
- `lib/dlms-profile/test/profile/test_trace_correlation.cpp` (commit 2)
- `test/integration/test_cross_layer_correlation.cpp` (commit 3, uses
  the loopback wrapper from existing endpoint integration tests)

The integration test must assert that for *one* request the
`conversationId` is **bit-identical** across the wrapper sink, the
transport sink, and (where applicable) the association sink — that is
the success criterion for §2.

## Migration notes

- Consumers compiled against an older header continue to work; the
  new field is at the end of the struct and zero-initialised.
- Consumers compiled against the new header but linked against the
  old library see `conversationId == 0` because the old library does
  not populate the field. This is detectable (always-zero across many
  events from different requests) and harmless.
- After all three commits ship the field is a stable diagnostic
  contract: like `*StatusName`, it is for human/log consumption only
  and must not be parsed as a wire value or used as a security token.

## Status of the §2 deliverable

This document closes the **design** half of P1 §2. The three
implementation commits described above remain open; once commit 3
lands the §2 entry in `production_readiness_roadmap.md` flips to ✅.
