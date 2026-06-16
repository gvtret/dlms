# Changelog

## 0.102.0 - 2026-06-17

- Feature (P1 §7 commit 3a — server-side dispatch trace sink in
  `dlms-server`, plus endpoint wiring for xDLMS + dispatch sinks):
  - New header `dlms/server/server_dispatch_trace.hpp` declares
    `IServerDispatchTraceSink` and `ServerDispatchTraceEvent` (with
    `ServerDispatchTraceKind::{GetDispatched, SetDispatched,
    ActionDispatched}`). Kept in `dlms-server` rather than
    `dlms-xdlms` so `dlms-xdlms` does not pick up a dependency on
    server-layer types — matches the "two sinks, not one" decision
    in `docs/xdlms_server_trace_design.md`.
  - New header `dlms/server/tracing_xdlms_server_dispatcher.hpp`
    declares `TracingXdlmsServerDispatcher` — a thin pass-through
    decorator around any `dlms::xdlms::IXdlmsServerDispatcher` that
    emits one `ServerDispatchTraceEvent` per `DispatchGet` /
    `DispatchSet` / `DispatchAction`. Zero-cost when the sink is
    `nullptr` (no event object constructed, no virtual call).
  - `ServerEndpointOptions` gains two opt-in pointer fields:
    `xdlms::IXdlmsTraceSink* xdlmsTraceSink = nullptr` and
    `server::IServerDispatchTraceSink* serverDispatchTraceSink =
    nullptr`. `GatewayEndpointOptions::downstream` (a
    `ServerEndpointOptions`) inherits the same two fields and they
    are now honoured by both `ServerEndpoint::ConfigureXdlmsProcessor`
    and `GatewayEndpoint`'s downstream construction. Defaults
    (`nullptr`) preserve existing behaviour: no events emitted, no
    decorator overhead.
  - `PushListenerEndpointOptions` and `ClientEndpointOptions` are
    deliberately **not** extended in this commit: the push-listener
    side does not run a dispatcher, and the client side already has
    `XdlmsClient::SetTraceSink` available for embedded code (proper
    `DlmsClient`/`ClientEndpoint` plumbing of `xdlmsTraceSink` will
    follow as its own commit once it has a real consumer — AGENTS.md
    §2: no speculative API).
  - Redaction contract identical to existing sinks: events publish
    only sizes and identifiers (invoke id, class id, attribute or
    method id, OBIS LN, request and response payload sizes, xDLMS
    status, conversation id). The `conversationId` field is
    currently always `kNoConversationId` (0); end-to-end correlation
    on the server side is the subject of §7 commit 3b together with
    the integration test.
  - Version bumped 0.101.0 → 0.102.0 (minor — additive public API
    on `dlms-server` and `dlms-endpoint`, no ABI / semantic break).
  - Tests: full suite 971/973 functional pass (the two remaining
    failures, `dlms_changelog_check` and
    `dlms_package_install_smoke`, are closed by this entry).

## 0.101.0 - 2026-06-17

- Feature (P1 §7 commit 2/3 — xDLMS server trace emission):
  `XdlmsServerApduProcessor` gains opt-in
  `SetTraceSink(IXdlmsTraceSink*) noexcept` /
  `TraceSink() const noexcept`, mirroring the client-side surface
  shipped in 0.100.0. The processor reuses the same
  `IXdlmsTraceSink` interface defined in
  `lib/dlms-xdlms/include/dlms/xdlms/xdlms_trace.hpp`, with
  `XdlmsTraceDirection::Inbound` on every server-emitted event. When
  the sink is unset (default), no event object is constructed and no
  virtual call is made — zero-cost on the hot path.

  **Emission sites in `XdlmsServerApduProcessor::ProcessRequest`:**
  - `SecurityFailed` — when `IXdlmsSecurityProcessor::Unprotect` of
    the request or `Protect` of the response returns non-Ok.
  - `DecodeFailed` — when `DecodeXdlmsApdu` rejects the plain
    request.
  - `RequestReceived` — once per successfully decoded APDU, carrying
    invoke id, parsed `ServiceOptions`, class id, attribute or method
    id, and 6-byte OBIS LN for Get / Set / Action.
  - `ResponseSent` — once per successful response, carrying the
    final (post-`Protect`, if ciphered) `apduSize`.
  - `InvokeIdRejected` — emitted from the per-service block
    transfer helpers (Get-next, Set continuation, Action
    continuation) when the inbound APDU's invoke id does not match
    the active block state.
  - `BlockTransferStep` — emitted after each successful encode of
    `SendNextGetResponseBlock` / `EncodeSetBlockAckResponse` /
    `EncodeActionNextPblockResponse`, carrying the block number and
    response APDU size.

  Events publish only sizes and identifiers — never raw APDU bytes,
  plaintext payload, keys, system titles, GMAC tags, or HLS
  challenges, matching the 0.100.0 redaction contract. The
  `conversationId` field on server-side events is currently always
  `kNoConversationId` (0); endpoint composition in §7 commit 3/3
  will publish the correlation id from the listening side.

  **Tests.** New `lib/dlms-xdlms/test/xdlms/test_xdlms_server_trace.cpp`
  adds 3 cases (`ServerEmitsRequestReceivedAndResponseSentForGet`,
  `ServerEmitsDecodeFailedOnGarbageInput`,
  `ServerWithoutSinkProcessesNormally`) that pin the happy-path
  Request/Response pairing, the malformed-input early-emit, and the
  zero-cost null-sink contract. Full ctest: 970 → 973.

  **Minor bump** (0.100.0 → 0.101.0) because two new public members
  (`SetTraceSink`, `TraceSink`) are added to a non-final class. ABI
  is preserved: no virtuals, no member reorder, no removed symbol —
  the new `IXdlmsTraceSink* traceSink_` field is appended last and
  initialized to `0` in all six constructors. Pre-existing callers
  that never touch the sink see no behavioural change.

  Out of scope (deferred to §7 commit 3/3): `IServerDispatchTraceSink`
  for `XdlmsServerDispatcher` and endpoint-level pass-through
  (`EndpointOptions::xdlmsTraceSink` and
  `serverDispatchTraceSink`).

## 0.100.0 - 2026-06-17

- Feature (P1 §7 commit 1/3 — xDLMS client trace + §2 wiring closure):
  new public xDLMS-layer trace surface plus end-to-end cross-layer
  correlation propagation. Two changes that share infrastructure and
  ship together:

  **xDLMS trace sink (§7, client side).** New header
  `lib/dlms-xdlms/include/dlms/xdlms/xdlms_trace.hpp` defines
  `IXdlmsTraceSink`, `XdlmsTraceEvent` (kind, direction, status, invoke
  id, options, class id, attribute/method id, OBIS LN, optional block
  number, APDU size, payload size, `conversationId`), `XdlmsTraceKind`
  (12 kinds covering Get/Set/Action request/response, block transfer
  step, request received, response sent, decode failure, invoke-id
  mismatch, security failure) and `XdlmsTraceDirection`. Events publish
  only sizes and identifiers — never raw APDU bytes, keys, system
  titles, GMAC tags, or HLS challenges. Same lifecycle / re-entrancy /
  no-throw / span-validity contract as the 4 existing sinks (see
  `docs/trace_contracts.md`). `XdlmsClient` gains opt-in
  `SetTraceSink(IXdlmsTraceSink*) noexcept` /
  `TraceSink() const noexcept` (additive, no new ctor overloads); when
  unset, behaviour is identical to before. Client-side emission is wired
  through `Get`, `Set` (normal + block-transfer), `Action` (normal +
  block-transfer): per-service `Request`/`Response`/`DecodeFailed`/
  `SecurityFailed` events from a single `SendAndReceive` site, plus
  `BlockTransferStep` events in the per-service block loops and
  `InvokeIdRejected` events on every invoke-id mismatch check.

  **Cross-layer correlation wiring (§2 closure).** `XdlmsClient::Get/
  Set/Action` now also call
  `channel_.SetCorrelation(MakeConversationId(seed, invokeId))` on the
  bound `IApduChannel` immediately after allocating the invoke id and
  before the first `SendApdu`. The seed comes from a new
  `IXdlmsAssociationState::ConversationSeed() const noexcept` virtual
  whose default returns `0` (ABI-safe: existing association
  implementations continue to return `0`, which yields
  `conversationId == kNoConversationId`, which is exactly the pre-0.99.6
  channel behaviour). Together with the channel-side stamping shipped
  in 0.99.6, this means xDLMS sink and channel sinks see bit-identical
  `conversationId` for the same service call — no manual plumbing in
  consumer code, no "who owns the id" coordination across layers.

  **Tests.** New integration fixture
  `test/integration/test_cross_layer_correlation.cpp` adds 3 cases
  (`GetEmitsMatchingConversationIdOnBothLayers`,
  `ZeroSeedYieldsZeroConversationId`,
  `NoTraceSinkStillCallsSetCorrelation`) that pin the invariant
  end-to-end through a custom `IApduChannel` capturing `SetCorrelation`
  and stamping a `WrapperTcpTraceEvent` with the captured id, alongside
  an `IXdlmsTraceSink` that captures the xDLMS-side events. Full ctest:
  967 → 970.

  Out of scope here (deferred to follow-up commits): server-side xDLMS
  trace emission through `XdlmsServerApduProcessor` (§7 commit 2/3),
  the `IServerDispatchTraceSink` and dispatcher wiring (§7 commit 3/3),
  and endpoint composition (`EndpointOptions::xdlmsTraceSink` +
  `serverDispatchTraceSink` pass-through). Also out of scope:
  `AssociationClient` still returns `ConversationSeed() == 0`, so real
  deployments still see `kNoConversationId` until a future change
  publishes a non-zero seed — at which point the wiring above will
  automatically carry it through every sink without further channel or
  client changes.

## 0.99.7 - 2026-06-17

- Docs (P1 §7 design phase): `docs/xdlms_server_trace_design.md`
  fixes the contracts for two new opt-in trace sinks that close the
  visibility gap called out by `docs/trace_contracts.md`:
  `IXdlmsTraceSink` (xDLMS layer, client + server) and
  `IServerDispatchTraceSink` (server-side dispatch). Two sinks, not
  one, so `dlms-xdlms` doesn't take a dependency on `dlms-server`
  types. Events publish only sizes and identifiers (class id,
  attribute/method id, OBIS LN, invoke id, sizes), never raw APDU or
  payload bytes — no new redaction obligation on consumers. Same
  lifecycle / no-throw / no-reentry / span-validity discipline as the
  4 existing sinks. Both events carry `conversationId` so events
  stitch with the §2 correlation work. 3-commit implementation plan
  laid out. No production code in this commit.

## 0.99.6 - 2026-06-17

- Feature (P1 §2 commit 3/3): `WrapperTcpProfileChannel` and
  `HdlcProfileChannel` now override `SetCorrelation(uint64_t)` and
  stamp the active `conversationId` onto every emitted trace event
  (`WireWrite`, `WireRead`, `ReadStatus`, `DecodeStatus` for both
  channels). Default conversation id stays `kNoConversationId = 0`, so
  consumers that never call `SetCorrelation` see exactly the same
  events as before. End-to-end test fixture extended with 2 new cases
  (`WrapperTcpChannelStampsSendAndReceiveEvents`,
  `HdlcChannelStampsWireWriteEvents`) that drive the channels through
  `FakeByteStream`, flip `SetCorrelation` on and off, and pin every
  event in the capture window to the expected id. Test fixture total:
  9 cases; full ctest unchanged at 967/967.

  Out of scope for this commit (intentionally deferred to a follow-up):
  AssociationClient does not yet generate or propagate a conversation
  id — once it does, the same `SetCorrelation` plumbing will carry it
  into trace consumers without any additional channel changes.

## 0.99.5 - 2026-06-17

- Feature (P1 §2 commit 2/3): wired the cross-layer correlation field
  through the four trace event structs and the apdu channel interface.
  Append-only changes, default `0` everywhere, no ABI break in 0.x:
  - `TransportTraceEvent`: new `conversationId` member, initialised to
    `0` in the existing user-defined default ctor.
  - `WrapperTcpTraceEvent`, `HdlcProfileTraceEvent`,
    `AssociationTraceEvent`: new `conversationId` member as the last
    field, default member initialiser `= 0` so aggregate `Event{}` and
    `Event ev; ev = {};` keep producing zero-context events.
  - `IApduChannel::SetCorrelation(uint64_t) noexcept`: new virtual
    method with default no-op body. Channels that do not emit traces
    do not need to override.
  New test fixture `test_trace_correlation.cpp` (7 cases, lives in
  `dlms-profile/test`) pins: every event default-inits its
  `conversationId` to `kNoConversationId = 0`; assigning the field
  round-trips; default `SetCorrelation` is reachable through the base
  interface, accepts any `uint64_t`, is `noexcept`; overriding channels
  receive the value verbatim. Per-test enumeration: 7 new gtest cases
  inside `dlms_profile_tests.exe` (ctest still sees one aggregate
  entry); ctest summary unchanged at 967/967.

## 0.99.4 - 2026-06-17

- Feature (P1 §2 commit 1/3): added `dlms/xdlms/xdlms_correlation.hpp` —
  header-only `constexpr noexcept MakeConversationId(seed, invokeId)`
  primitive plus `kNoConversationId` sentinel. Formula:
  `(seed & ~0x0F) | (invokeId & 0x0F)` — the low nibble preserves the
  human-readable invoke-id; the high 60 bits separate associations.
  Zero-impact addition: no existing code calls it yet, no ABI change,
  pure header. New test fixture `test_xdlms_correlation.cpp` (9 cases)
  pins the invariants: low-nibble round-trip, high-bits = seed high
  bits, invoke-id high nibble ignored, distinct seeds → distinct ids,
  distinct invoke-ids → distinct ids, seed low nibble cleared not
  mixed, constexpr + noexcept, sentinel = 0, seed=0/invoke=0 collapses
  to sentinel (documented edge case). ctest 958 → 967.

## 0.99.3 - 2026-06-17

- Docs (P1 «Диагностика» §2 design phase): added
  `docs/trace_correlation_design.md` — accepted design for cross-layer
  trace correlation. Introduces a single opaque non-secret 64-bit
  `conversationId` carried by every trace event in all four sinks
  (`TransportTraceEvent`, `WrapperTcpTraceEvent`,
  `HdlcProfileTraceEvent`, `AssociationTraceEvent`). The id originates
  in the xDLMS layer (as `(association_seed & ~0x0F) | invoke_id`,
  preserving the low nibble for human readability) and propagates down
  to the profile/transport sinks through a new no-op
  `IApduChannel::SetCorrelation()` virtual. The seed is a per-association
  logging salt with no security role; nothing about correlation appears
  on the wire. Design respects the existing ABI rules (POD append
  at end of struct, virtual default no-op, zero-init = no correlation
  context). Three follow-up commits planned: primitive +
  `MakeConversationId`; struct extension + default virtual; wiring +
  integration test. Docs-only; no code change in this version.

## 0.99.2 - 2026-06-17

- Docs (P1 «Диагностика» §6 closed): added
  `docs/status_to_string_contract.md` — the single canonical contract
  for every `*StatusName` / `ToString` helper in the stack (13 status
  enum across 13 modules). Documents totality, `"Unknown"` fallback,
  lifetime (static storage), thread-safety, no-allocation, and ABI
  stability guarantees, plus the full catalogue of helpers and the
  matching exhaustive-coverage test files. Each per-module
  `01_*_api.md` now ends with a short "Diagnostic helpers" section
  that names the module’s helper and links back to the central
  contract. Also records the `EndpointStatus::ToString` /
  `TransportStatus::ToString` naming inconsistency (vs the
  `*StatusName` form everywhere else) as P1 §5 follow-up; both names
  will continue to work until a major bump introduces an
  alias-and-deprecate. No code change; docs-only.

## 0.99.1 - 2026-06-17

- Docs (P1 «Диагностика» §1 closed): added `docs/trace_contracts.md`,
  the audit map of every public trace sink in the stack
  (`ITransportTraceSink`, `IWrapperTcpTraceSink`, `IHdlcProfileTraceSink`,
  `IAssociationTraceSink`). For each sink the doc records the layer,
  defining header, event struct, whether raw bytes are carried, the
  security semantics (wrapper/hdlc carry the literal protected APDU —
  treat as secret; association carries only sizes), framework guarantees
  (lifecycle, re-entrancy, byte-span validity, no-exception-escape), and
  consumer responsibilities (redaction, thread safety, backpressure,
  filtering). Also makes the endpoint pass-through pattern explicit and
  records two known gaps (no public xDLMS-layer or server-side trace
  sink) as roadmap §7 follow-ups. No code change; audit-only.

## 0.99.0 - 2026-06-17

- API / diagnostics (P1 “Диагностика” §3 status-to-string
  completeness, etap 2): added public `HdlcStatusName(HdlcStatus)`,
  `LlcStatusName(LlcStatus)`, `WrapperStatusName(WrapperStatus)`, and
  `ProfileStatusName(ProfileStatus)` helpers that return the enum-value
  identifier as a `static`-storage C string (`"Ok"`, `"NeedMoreData"`,
  ...) or `"Unknown"` for out-of-range casts. With this every public
  status enum in `dlms::*` now exposes a stable, exhaustive `Name()`
  helper for diagnostics, logs, and cross-layer error propagation. Each
  implementation is a plain switch with no `default:` arm so future enum
  additions will fire `-Wswitch` until a string is wired. Adds 8 new
  `*StatusName` coverage tests (2 per module) covering every enum value
  plus the Unknown fallback; full ctest 958/958.
- Internal cleanup: `tools/live_meter_smoke.cpp` had a private
  `ProfileStatusName` copy from before the public helper existed; it has
  been removed and the two call sites now use the public
  `dlms::profile::ProfileStatusName`.

## 0.98.3 - 2026-06-17

- Tests / status hygiene (P1 "Диагностика" §3 status-to-string
  completeness, etap 1): added exhaustive `*StatusName` /
  `ToString` coverage tests for `ApduStatus`, `AssociationStatus`,
  `CosemStatus`, `ServerStatus`, `XdlmsStatus`, and brought
  `EndpointStatus` test up to exhaustive (covered 12/12 values
  instead of the previous 4/12). Each new test pins the stable
  string form for every enum value plus the `Unknown` fallback.
  Also dropped a stale `default: return "Unknown";` from
  `endpoint_status.cpp` so `-Wswitch` will fire if a future
  `EndpointStatus` value forgets a `ToString` arm (consistent
  with the same hygiene applied to status mappers in 0.97.1 /
  0.97.2). Test-only behavioural change is a single warning-on-
  miss surface; full ctest 952/952 (942 → 952, +10 new cases).

## 0.98.2 - 2026-06-17

- Tests / C ABI: every public `*_c_api.h` header now has a
  dedicated C-only smoke executable that links and runs from a
  pure-C TU, closing P0 §1.4 "Проверить все C headers smoke
  tests". Previously six of the seven `test_*_c_header.c` files
  were just compiled into their C++ gtest binary (compile-only
  canary, never invoked); only `dlms_association_c_header_smoke`
  ran as a real C-only executable. Now each of the seven
  modules (`apdu`, `association`, `hdlc`, `llc`, `profile`,
  `transport`, `wrapper`) gets `dlms_<mod>_c_header_smoke`,
  registered as `<Module>CApi.CHeaderCompilesAsC` in ctest, that
  actually calls the C smoke function at runtime.
  The existing `test_*_c_header.c` files keep their original
  function signatures so the corresponding C++ gtest cases that
  already linked against them continue to work unchanged; a
  thin `test_*_c_header_main.c` companion supplies the `main()`
  for the smoke executable. Test-only change, no library-side
  impact; full ctest 942/942 (936 → 942).

## 0.98.1 - 2026-06-17

- Tests / endpoint: added three idempotency regression tests
  covering `Open()` after a failed `Open()` for the remaining
  server-side endpoints, closing the second half of P0 §2.3
  "regression tests для cleanup при неуспешном
  open/association":
  - `ServerEndpoint::OpenAfterFailedOpenIsIdempotentAndRetries`
  - `PushListenerEndpoint::OpenAfterFailedOpenIsIdempotentAndRetries`
  - `GatewayEndpoint::OpenAfterFailedDownstreamOpenIsIdempotentAndRetries`

  Each test pins: a failed channel `Open()` returns the mapped
  status and leaves the endpoint closed; `Close()` after a failed
  open is a no-op; a second `Open()` actually re-invokes the
  channel (no stale short-circuit); clearing the failure lets the
  next open succeed without any residual state. The gateway test
  additionally pins that the upstream is *not* opened when the
  downstream fails first.
- Test hygiene / endpoint: the three test-only `FakeApduChannel`
  fixtures in `test_server_endpoint.cpp`,
  `test_push_listener_endpoint.cpp`, `test_gateway_endpoint.cpp`
  were setting `open = true` unconditionally in `Open()`, even
  when `openStatus` was non-`Ok`. Now they set
  `open = (openStatus == Ok)` to match real channel semantics,
  aligning with how `test_listener_runtime.cpp`'s fake already
  behaved. Test-only change, no library-side impact; full ctest
  936/936 confirms no existing test depended on the broken
  behaviour.

## 0.98.0 - 2026-06-17 (BREAKING bugfix)

- BREAKING fix / endpoint: `ClientEndpoint::Close()` now always
  drops the owned `DlmsClient` instance, even when the underlying
  `client->Close()` returned a non-`Ok` status. Previously a failed
  close left the stale instance attached to the endpoint, which
  meant the next `Open()` could either short-circuit on stale
  `IsAssociated()` or silently leak the old instance via
  `move`-assignment inside `CreateClient()`. The status returned by
  `Close()` is unchanged; only the post-condition is stricter.
  The behaviour change is observable, hence the minor bump.
- Tests / endpoint: added four regression tests in
  `lib/dlms-endpoint/test/endpoint/test_client_endpoint.cpp`
  covering the cleanup contract for failed `Open()` /
  `Associate()` paths:
  - `OpenAfterFailedOpenIsIdempotentAndRetries` - a failed open
    leaves no stale state and a second open retries.
  - `CloseAfterFailedOpenLeavesNoStateBehind` - repeated
    `Close()` after a failed open is always `Ok`, and services
    return `InvalidState`, not `InternalError`.
  - `OpenIsIdempotentAfterValidationFailure` - validation
    failures (bad host) before any network use are repeatable
    and leave the endpoint closed.
  - `DestructorClosesAfterFailedOpenWithoutLeak` - destructor
    must not throw / abort after a failed open; loop-runs the
    pattern three times for ASan to catch leaks if rerun there.
  Closes the first half of P0 §2.3 "regression tests для
  cleanup при неуспешном open/association".

## 0.97.7 - 2026-06-17

- Docs / client: added §1.1 “Facade Status Mapping Policy”
  to `lib/dlms-client/docs/01_client_api.md` that pins what
  the public `ClientStatus` enum preserves vs. what it
  intentionally drops, after the `0.96.0`–`0.97.3`
  exhaustive-`switch` audit. Documents the
  per-direction send/receive split, the new first-class
  `BlockTransferRequired` / `InvokeIdMismatch` /
  `CodecFailed` values, the deliberate collapse of
  per-layer transport detail / service-rejected reasons /
  security sub-classification / COSEM access-result, and
  reserves `InternalError` for facade-owned invariants only.
  Also lists every mapper site (`client.cpp`,
  `client_data.cpp`, `client_endpoint.cpp`,
  `server_status.cpp`) and the testing entry point in
  `test_client_internal.cpp`. Closes the second bullet of
  P0 §1.3 “status mapping”. Also refreshed the enum listing
  in the same doc to include the three values added in
  `0.97.0`.

## 0.97.6 - 2026-06-17

- Docs / security: added §5.1 “Storage, Ownership and
  Lifetime” to `lib/dlms-security/docs/01_security_api.md`,
  pinning the contract callers must honour for `IKeyStore`,
  `IMutableKeyStore` and `IInvocationCounterStore`. Covers
  ownership (caller owns, store outlives every processor /
  authenticator), lifetime (per-process or per-association
  both acceptable), storage backends (TPM/HSM/keyring/
  encrypted blob; no in-tree production in-RAM key store),
  invocation-counter persistence (monotonic across restarts;
  reserve-window pattern; remote high-water mark per system
  title), reset semantics (`ResetAfterKeyRotation` atomic
  with `SetKey` from the caller’s perspective; matches the
  in-tree `simple_objects` global-key-transfer path), and
  thread safety (caller-side responsibility;
  `InMemoryInvocationCounterStore` is not internally
  synchronised). Closes the last open bullet of P0 §3.6.

## 0.97.5 - 2026-06-17

- Tools / security: extracted the wire-byte hex dump policy
  for `tools/live_meter_smoke` into
  `tools/live_meter_smoke_byte_emit.hpp` so the redaction
  rule (default off, opt-in via
  `DLMS_LIVE_TRACE_WIRE_BYTES=1`) lives in one inline header
  used by both the tool and the new test target.
- Added `dlms_live_meter_smoke_redaction_tests` (10 cases)
  pinning the policy: wire payload omitted by default, only
  emitted on the explicit env flag, non-wire trace kinds and
  empty byte spans stay silent regardless. Test target is
  gated behind `DLMS_BUILD_LIVE_TESTS=ON` to match the
  existing `LiveMeterSmoke` opt-in. Closes the
  belt-and-braces follow-up promised in `0.97.4`.

## 0.97.4 - 2026-06-17

- Tools / security: `tools/live_meter_smoke` no longer dumps
  on-wire bytes from `OnWrapperTcpTrace` / `OnHdlcProfileTrace`
  unless the operator explicitly sets
  `DLMS_LIVE_TRACE_WIRE_BYTES=1`. Previously the smoke tool
  printed raw `event.bytes` as hex whenever
  `DLMS_LIVE_TRACE=1`, which leaked HLS challenges, GMAC tags,
  and any ciphered/clear protected APDU payload to the
  operator console. Trace metadata (kind, direction,
  status, ports, encoded/apdu sizes, byte count) is still
  printed under `DLMS_LIVE_TRACE=1`; only the byte payload
  itself is gated. Closes P0 §3.6 trace-redaction concern for
  the live smoke tool.

## 0.97.3 - 2026-06-17

- Docs / audit: P0 §2.4 “bounded loops do not swallow Timeout /
  Closed / InvalidState” marked as audited. Every `for(;;)` and
  `while(...)` loop in `lib/dlms-*` was reviewed:
  - Transport-facing loops in `dlms-profile` (HDLC and Wrapper TCP
    channel `Receive*` paths) propagate any non-`Ok` `ProfileStatus`
    immediately, including `Timeout`, `ConnectionClosed`, and
    `InvalidState`.
  - xDLMS client block-transfer loops (`Get`, `Set`, `Action`) return
    the underlying `XdlmsStatus` from `SendAndReceive` /
    `ReceiveGetResponse` / `ReceiveActionResponse` verbatim.
  - Stream / decoder loops (`hdlc_stream_decoder`,
    `wrapper_stream_decoder`) surface `NeedMoreData` only when no
    bytes were read; `ReadSome` failures fall through unchanged.
  - Remaining loops are pure buffer/grow / parser loops with no
    transport interaction (`client_data.cpp EncodeData`, BER/AXDR,
    HDLC segmentation, AXDR length decoder).
  No code change; the roadmap entry now records the audit and
  cites the verification scope.

## 0.97.2 - 2026-06-17

- Hygiene (status mapping, continued): Removed remaining
  `default:` arms from five more status mappers so future
  enum extensions trip `-Wswitch` at compile time. No
  observable runtime behaviour changes for any known input.
  Touched mappers:
  - `dlms-server/xdlms_server_adapter.cpp`
    `MapServerStatusToDataAccessResult` and
    `MapServerStatusToActionResult` (now list every
    `ServerStatus` value explicitly; values without a
    dedicated wire code still collapse to `other-reason`
    (250) per IEC 62056-5-3).
  - `dlms-endpoint/gateway_endpoint.cpp`
    `MapAssociationStatus`.
  - `dlms-endpoint/push_listener_endpoint.cpp`
    `MapAssociationStatus`.
  - `dlms-endpoint/server_endpoint.cpp`
    `MapAssociationStatus`.
- Known cleanup target: `MapAssociationStatus` is now
  duplicated across three endpoint files. Extracting it into
  a shared helper is intentionally deferred to keep this
  change surgical; tracked as a future refactor.

## 0.97.1 - 2026-06-17

- Hygiene (status mapping): Removed `default:` arms from six
  status-enum mappers so future additions to the source enum
  trip `-Wswitch` at compile time instead of silently falling
  into `InternalError`. No observable runtime behaviour
  changes for any known input. Touched mappers:
  - `dlms-endpoint/client_endpoint.cpp` `MapClientStatus`
    (added explicit arms for `BlockTransferRequired`,
    `InvokeIdMismatch`, `CodecFailed` collapsing to
    `ServiceFailed`; the endpoint facade keeps coarse
    service-layer granularity by design).
  - `dlms-endpoint/endpoint_factories.cpp` `MapTransportStatus`.
  - `dlms-endpoint/gateway_endpoint.cpp`
    `MapEndpointStatusToXdlmsStatus`.
  - `dlms-endpoint/server_endpoint.cpp` `MapProfileStatus` and
    `MapXdlmsStatus`.
  - `dlms-server/xdlms_server_adapter.cpp`
    `MapServerStatusToXdlmsStatus`.
  Each mapper now ends with a defensive `return InternalError`
  outside the switch to keep ABI-drift behaviour for unknown
  integer values.
- Deferred: extending `dlms::endpoint::EndpointStatus` with
  finer categories (`BlockTransferRequired`, `CodecFailed`,
  `InvokeIdMismatch`) is intentionally not done in this patch.
  Callers that need that granularity should consume
  `dlms::client::DlmsClient` directly; lifting the categories
  to the endpoint layer is a separate BREAKING change.

## 0.97.0 - 2026-06-17

- BREAKING (C++ API): Extended `dlms::client::ClientStatus` with
  three new values that were previously collapsed into other
  categories. Existing switches with `default` branches keep
  building; exhaustive switches over `ClientStatus` need a new
  arm per value. The full list of added values:
  - `BlockTransferRequired` — the peer asked the client to
    switch to block transfer; the simple non-block path cannot
    complete the request. Distinct from `UnsupportedFeature`
    because the protocol feature exists, the client just did
    not engage it.
  - `InvokeIdMismatch` — the response was framed and decoded
    fine, but its invoke-id does not match the outstanding
    request. Distinct from `ReceiveFailed`.
  - `CodecFailed` — APDU encode or decode failed inside the
    xDLMS layer. Distinct from `InternalError` because it
    implies wire-level corruption or a spec mismatch with the
    peer, not a library bug.
- Fix (status mapping): `MapXdlmsStatus()` no longer collapses
  `XdlmsStatus::BlockTransferRequired` into
  `ClientStatus::UnsupportedFeature` and no longer collapses
  `EncodeFailed`, `DecodeFailed`, and `InvokeIdMismatch` into
  `ClientStatus::InternalError`. The mapper now routes each to
  its dedicated public category (P0 §1.3 from
  `docs/production_readiness_roadmap.md`).
- Updated `ClientStatusName()` and `test_client_status.cpp` to
  recognise the three new values.
- Updated two existing regression tests in `test_client.cpp`
  (`GetMapsRealMalformedResponseAndKeepsAssociated`,
  `GetMapsRealInvokeIdMismatchAndKeepsAssociated`) to expect
  the new specific statuses instead of `InternalError`.
- Exposed `internal::MapXdlmsStatus()` via
  `client_internal.hpp` and added 7 unit tests in
  `test_client_internal.cpp` pinning the full mapping table,
  including a defensive fall-through for unknown enum values.

## 0.96.0 - 2026-06-17

- Fix (status mapping): `DlmsClient::Close()` and
  `DlmsClient::ReleaseAssociation()` no longer collapse every
  HDLC DataLink `DisconnectDataLink()` failure into
  `ClientStatus::InternalError`. The previous mapper used a
  catch-all `default` branch that hid `Timeout`,
  `ConnectionClosed`, malformed UA frames and IO errors behind a
  generic library-bug status, contradicting the production
  readiness roadmap rule P0 §1.3 ("don't collapse useful errors
  to InternalError when a more specific public status exists").
  The mapper now enumerates every `ProfileStatus` explicitly so
  drift in the source enum becomes a compile error and surfaces
  the most actionable category to callers:
  - `Ok`, `NotOpen` → `Ok` (idempotent disconnect).
  - `InvalidArgument` → `InvalidArgument`.
  - `AlreadyOpen` → `InvalidState`.
  - `OpenFailed`, `WriteFailed` → `SendFailed` (DISC could not
    be transmitted to the meter).
  - `ReadFailed`, `Timeout`, `ConnectionClosed`, `WouldBlock`,
    `NeedMoreData`, `OutputBufferTooSmall`, `InvalidFrame`,
    `InvalidLength`, `InvalidAddress`, `PayloadTooLarge` →
    `ReceiveFailed` (no usable UA response from the meter).
  - `UnsupportedFeature` → `UnsupportedFeature`.
  - `InternalError` → `InternalError`.
- Added `lib/dlms-client/src/client/client_internal.hpp` to
  expose `internal::MapDataLinkDisconnectStatus` for unit
  testing without spinning up a real `HdlcProfileChannel`. The
  header is source-tree-only and not part of the install
  surface.
- Added `test_client_internal.cpp` with 8 cases pinning every
  `ProfileStatus` value (including a defensive fall-through for
  unknown integer values).

## 0.95.0 - 2026-06-17

- Fix (C++ API semantics): `CosemRegisterObject::InvokeMethod` now
  recognises method `1` `reset` (data ::= integer(0)) per IEC
  62056-6-2 ED4 (2021) §4.3.2 / DLMS UA Blue Book Ed. 12.1 and
  surfaces it as `CosemStatus::UnsupportedFeature`. Previously
  all method ids returned `CosemStatus::MethodNotFound`, which
  hid `reset` from clients enumerating IC 3 methods. Other
  method ids still report `MethodNotFound`. The built-in object
  remains application-agnostic: reset semantics are decided by
  the backend that owns the underlying register storage.
- Added `kRegisterResetMethodId` to `simple_objects.cpp` and a
  dedicated `CosemRegisterObject, ResetMethodIsUnsupportedAndOtherIdsNotFound`
  unit test. Adjusted the existing `WritesValueAndRejectsUnsupportedMembers`
  test to probe an unrelated method id for the `MethodNotFound`
  branch.
- Refreshed the COSEM IC support matrix row `3` to document the
  new method status mapping.

## 0.94.0 - 2026-06-17

- BREAKING (C++ API and wire semantics): Completed the
  `CosemPrimePlcMacFunctionalParametersObject` (PRIME NB OFDM PLC
  MAC functional parameters, class_id `83`, version `0`) to the
  full 14-attribute spec form per IEC 62056-6-2 ED4 (2021)
  §4.12.7 / DLMS UA Blue Book Ed. 12.1. Previously the object
  exposed only 9 attributes with `mac_capabilities` placed on
  attribute id `9`; per spec `mac_capabilities` lives on
  attribute id `14` and ids `9..13` belong to the beacon family.
  The constructor signature has been replaced and now takes 13
  buffers in spec order: `lnid, lsid, sid, sna, state,
  scpLength, nodeHierarchyLevel, beaconSlotCount, beaconRxSlot,
  beaconTxSlot, beaconRxFrequency, beaconTxFrequency,
  capabilities`. The previous parameters `sct` and `scd` have
  been renamed to `scpLength` (attribute `7`,
  `mac_scp_length`, type `long`) and `nodeHierarchyLevel`
  (attribute `8`, `mac_node_hierarchy_level`, type `unsigned`,
  range `0..63`). Five new attributes were added:
  `mac_beacon_slot_count` (`9`, `unsigned`, `0..7`),
  `mac_beacon_rx_slot` (`10`, `unsigned`, `0..7`),
  `mac_beacon_tx_slot` (`11`, `unsigned`, `0..7`),
  `mac_beacon_rx_frequency` (`12`, `unsigned`, `0..31`),
  `mac_beacon_tx_frequency` (`13`, `unsigned`, `0..31`).
  Internal constant identifiers `kPrimePlcMacFunctionalParamsSctId`
  and `kPrimePlcMacFunctionalParamsScdId` have been renamed to
  `kPrimePlcMacFunctionalParamsScpLengthId` and
  `kPrimePlcMacFunctionalParamsNodeHierarchyLevelId`, the value of
  `kPrimePlcMacFunctionalParamsCapabilitiesId` moved from `9` to
  `14`, and new identifiers were added for the beacon block.
  Public attribute accessors `Sct()` / `Scd()` were renamed to
  `ScpLength()` / `NodeHierarchyLevel()` and accessors
  `BeaconSlotCount()`, `BeaconRxSlot()`, `BeaconTxSlot()`,
  `BeaconRxFrequency()`, `BeaconTxFrequency()` were added.
  Downstream callers must update their constructor argument
  lists and any direct attribute-id reads.
- Refreshed unit tests, COSEM API guide and COSEM IC support
  matrix accordingly. The support matrix row `83` now lists 14
  attributes and notes that the beacon block was completed.

## 0.93.0 - 2026-06-17

- BREAKING (C++ API): Renamed the PRIME PLC MAC Network
  Statistics built-in object from
  `CosemPrimePlcMacNetworkStatisticsObject` to
  `CosemPrimePlcMacNetworkAdminDataObject` to match the
  IEC 62056-6-2 ED4 (2021) §4.12.9 / DLMS UA Blue Book Ed. 12.1
  spec name `PRIME NB OFDM PLC MAC network administration data`
  (class_id `85`, version `0`). The class id, attribute layout,
  access semantics and method behavior are unchanged; only the
  C++ type name and its internal constant identifiers
  (`kPrimePlcMacNetworkAdminData*`) were renamed. Public
  attribute accessors (`NodeRegistrations()`,
  `NodeUnregistrations()`, `ProcessedAliveMsgs()`,
  `HandledPromotions()`) are unchanged.
- Downstream code referencing the old type name must rename to
  the new one; no header path or include changes are required.
- Refreshed unit tests, COSEM API guide and COSEM IC support
  matrix accordingly. The support matrix row `85` now reads
  `PRIME PLC MAC Network Administration Data` and notes the
  rename.

## 0.92.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC Application
  identification built-in object
  (`CosemPrimePlcApplicationIdentificationObject`) from `85`
  to `86` per IEC 62056-6-2 ED4 (2021) §4.12.11 and DLMS UA
  Blue Book Ed. 12.1. Class_id `85` is reserved by the spec
  for the unrelated `PRIME NB OFDM PLC MAC network
  administration data` IC (already corrected in 0.90.0).
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly.

## 0.91.0 - 2026-06-17

- Removed the duplicate `CosemPrimePlcMacAddressSetupObject`
  built-in object (class_id `84`). IEC 62056-6-2 ED4 (2021)
  §4.12.10 places the PRIME NB OFDM PLC MAC address setup
  IC at class_id `43`, which is already covered by the
  existing built-in `CosemMacAddressSetupObject`. The
  duplicate carried the wrong class_id (`84`) and would not
  interoperate with a spec-conformant client/server.
- Refreshed unit tests (4 removed), COSEM IC support matrix,
  COSEM API guide and COSEM test plan accordingly. The
  support matrix now lists row `84` as reserved/
  application-provided and points callers at row `43`.

## 0.90.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC network
  administration data (a.k.a. network statistics) built-in
  object (`CosemPrimePlcMacNetworkStatisticsObject`) from
  `83` to `85` per IEC 62056-6-2 ED4 (2021) §4.12.9 and
  DLMS UA Blue Book Ed. 12.1. Class_id `83` is reserved by
  the spec for the unrelated `PRIME NB OFDM PLC MAC
  functional parameters` IC (already corrected in 0.88.0).
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly.

## 0.89.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC counters
  built-in object (`CosemPrimePlcMacCountersObject`) from
  `82` to `84` per IEC 62056-6-2 ED4 (2021) §4.12.8 and
  DLMS UA Blue Book Ed. 12.1. Class_id `82` is reserved by
  the spec for the unrelated `PRIME NB OFDM PLC MAC setup`
  IC (already corrected in 0.87.0).
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly.

## 0.88.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC functional
  parameters built-in object
  (`CosemPrimePlcMacFunctionalParametersObject`) from `81`
  to `83` per IEC 62056-6-2 ED4 (2021) §4.12.7 and DLMS UA
  Blue Book Ed. 12.1. Class_id `81` is reserved by the spec
  for the unrelated `PRIME NB OFDM PLC Physical layer
  counters` IC.
- The built-in still exposes only the legacy 8-attribute
  surface (`mac_LNID`/`mac_LSID`/`mac_SID`/`mac_SNA`/
  `mac_state`/`sct`/`scd`/`capabilities`). The spec defines
  14 attributes; bringing the remaining ones online
  (`mac_node_hierarchy_level`, `mac_beacon_*`,
  `mac_capabilities` at id `14`) is queued for a follow-up
  rebuild.
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly. The matrix now lists IC `81`
  separately as `Application-provided`.

## 0.87.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC setup
  built-in object (`CosemPrimePlcMacSetupObject`) from `80`
  to `82` per IEC 62056-6-2 ED4 (2021) §4.12.6 and DLMS UA
  Blue Book Ed. 12.1. Class_id `80` is reserved by the spec
  for the unrelated `61334-4-32 LLC SSCS setup` IC and was
  never the right id for PRIME PLC MAC setup.
- Refreshed unit test, COSEM IC support matrix and COSEM API
  guide accordingly. The IC support matrix now lists IC `80`
  separately as `Application-provided` (no built-in
  implementation).

## 0.86.0 - 2026-06-17

- Rebuilt the M-Bus Diagnostic built-in object
  (`CosemMBusDiagnosticObject`) to match IEC 62056-6-2 ED4
  (2021) §4.8.7 (class_id 77, version 0):
  - Replaced the prior 6-attribute layout
    (`received_signal_quality`, `transmitter_signal_quality`,
    `bbc`, `fcs_ok_frames_counter`, `fcs_nok_frames_counter`,
    `capture_time`) with the spec-defined 8 dynamic attributes:
    `2 received_signal_strength` (unsigned, dBm),
    `3 channel_id` (unsigned), `4 link_status` (enum),
    `5 broadcast_frames_counter` (array),
    `6 transmissions_counter` (double-long-unsigned),
    `7 fcs_ok_frames_counter` (double-long-unsigned),
    `8 fcs_nok_frames_counter` (double-long-unsigned),
    `9 capture_time` (date-time octet-string(12)).
  - Renamed accessors accordingly
    (`ReceivedSignalStrength`, `ChannelId`, `LinkStatus`,
    `BroadcastFramesCounter`, `TransmissionsCounter`,
    plus the existing FCS counters and `CaptureTime`).
  - Surfaced the optional `1 reset` method as
    `UnsupportedFeature` (built-in object does not own the
    counter sources); undefined method ids still return
    `MethodNotFound`.
- Refreshed unit tests, COSEM IC support matrix, COSEM API
  guide and COSEM test plan.

## 0.85.0 - 2026-06-17

- Fixed the class_id of the M-Bus Master Port Setup built-in
  object (`CosemMBusMasterPortSetupObject`) from `73` to `74`
  to match IEC 62056-6-2 ED4 (2021) §4.8.5 (and the
  DLMS UA Blue Book Ed. 12.1 §4.8.4). class_id `73` is
  reserved for the Wireless Mode Q channel IC, which is not
  shipped as a built-in object.
- Updated the `ExposesAllAttributes` unit test to assert
  `classId == 74u`.
- Refreshed the COSEM IC support matrix (split the previous
  `74`-`76` row, added an explicit Wireless Mode Q channel
  entry for `73`, moved the M-Bus Master Port Setup row to
  `74`) and the COSEM API guide.

## 0.84.0 - 2026-06-17

- Rebuilt the Sensor Manager IC `67` built-in object
  (`CosemSensorManagerObject`) to match the current
  IEC 62056-6-2 ED4 (2021) §4.5.11 layout: class_id `67`,
  version `0` with fifteen attributes (`1 logical_name`,
  `2 serial_number`, `3 metrological_identification`,
  `4 output_type`, `5 adjustment_method`, `6 sealing_method`,
  `7 raw_value`, `8 scaler_unit`, `9 status`, `10 capture_time`,
  `11 raw_value_thresholds`, `12 raw_value_actions`,
  `13 processed_value`, `14 processed_value_thresholds`,
  `15 processed_value_actions`) and one optional specific method
  (`1 reset(data)`). Previously the object exposed an ad-hoc
  layout of eleven attributes with vendor-style names
  (`status`, `serial_number`, `device_type`, `manufacturer_id`,
  `firmware_version`, `metrology_firmware_version`, `driver`,
  `communication_desc`, `setup_desc`, `measurement_desc`) that
  did not match any DLMS UA Blue Book / IEC 62056-6-2 edition
  and declared no methods.
- Method `1` `reset(data)` is surfaced as `UnsupportedFeature`
  (the built-in object does not own the sensor lifecycle); other
  method ids remain `MethodNotFound`.
- Constructor signature reworked to take the fourteen mutable
  payload buffers in spec attribute order followed by the shared
  `AttributeAccessMode` (and the optional version).
- Renamed C++ accessors to follow the spec names
  (`SerialNumber`, `MetrologicalIdentification`, `OutputType`,
  `AdjustmentMethod`, `SealingMethod`, `RawValue`, `ScalerUnit`,
  `Status`, `CaptureTime`, `RawValueThresholds`,
  `RawValueActions`, `ProcessedValue`,
  `ProcessedValueThresholds`, `ProcessedValueActions`); removed
  the old `DeviceType`, `ManufacturerId`, `FirmwareVersion`,
  `MetrologyFirmwareVersion`, `Driver`, `CommunicationDesc`,
  `SetupDesc`, `MeasurementDesc` accessors.
- Updated unit tests to cover the new fifteen-attribute layout,
  the renamed accessors and the new
  `ResetMethodIsUnsupportedFeature` expectation.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the ED4 Sensor Manager layout.

## 0.83.0 - 2026-06-17

- Rebuilt the Parameter Monitor IC `65` built-in object
  (`CosemParameterMonitorObject`) to match the current
  IEC 62056-6-2 ED4 (2021) §4.5.10 layout: class_id `65`,
  version `1` with eight attributes (`1 logical_name`,
  `2 changed_parameter`, `3 capture_time`, `4 parameter_list`,
  `5 parameter_list_name`, `6 hash_algorithm_id`,
  `7 parameter_value_digest`, `8 parameter_values`) and two
  specific methods (`1 add_parameter`, `2 delete_parameter`).
  Previously the object was pinned to legacy version `0`
  (Blue Book Ed. 12.1 §5.4.1) with only four attributes.
- Bumped `CosemParameterMonitorObject::MaxSupportedVersion` from
  `0` to `1`; the short constructor now defaults to version `1`.
  The constructor signature gained four new octet-string buffer
  parameters for attributes `5`-`8`. Passing version `0` keeps
  the legacy behaviour: the four extended buffers are cleared at
  construction time and attributes `5`-`8` return
  `AttributeNotFound` on both read and write.
- Methods `1 add_parameter` and `2 delete_parameter` continue to
  surface as `UnsupportedFeature` (the built-in object does not
  manage the monitored-parameters table; backend is expected to
  republish the buffers after evaluating parameter changes
  out-of-band).
- Updated unit tests to cover the new attributes, the renamed
  methods (`add_parameter`/`delete_parameter`) and the legacy
  version-0 fallback (`LegacyVersion0RejectsExtendedAttrs`).
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the ED4 Parameter Monitor layout.

## 0.82.0 - 2026-06-17

- Aligned the Compact Data IC `62` built-in object
  (`CosemCompactDataObject`) class version with
  IEC 62056-6-2 ED4 (2021) §4.3.10 / DLMS UA Blue Book Ed. 12.1.
  The Compact Data IC is defined as class_id `62`, version `1`
  with five attributes (`1 logical_name`, `2 buffer`,
  `3 capture_objects`, `4 template_id`, `5 template_description`,
  `6 capture_method`) and two specific methods (`1 reset`,
  `2 capture`). The attribute and method layout already matched
  the spec; only `MaxSupportedVersion` was wrong (`0` instead of
  `1`), so the descriptor advertised an obsolete legacy version.
- Bumped `CosemCompactDataObject::MaxSupportedVersion` from `0`
  to `1` and changed the default constructor to pass `1` so
  freshly built objects advertise the spec version. Reads and
  writes of attributes `2`-`6` and the `UnsupportedFeature`
  dispatch for methods `1 reset` / `2 capture` are unchanged.
- Updated `CosemCompactDataObject.ExposesAllAttributes` to
  expect `Descriptor().key.version == 1`.
- Refreshed the COSEM IC support matrix and COSEM API guide to
  describe Compact Data as class version `1` per
  IEC 62056-6-2 ED4 §4.3.10.

## 0.81.0 - 2026-06-17

- Rebuilt the Register Table IC `61` built-in object
  (`CosemRegisterTableObject`) to match IEC 62056-6-2 ED4 (2021)
  §4.3.8 and DLMS UA Blue Book Ed. 12.1. The spec defines four
  attributes (`1 logical_name`, `2 table_cell_values`,
  `3 table_cell_definition`, `4 scaler_unit`) and two specific
  methods (`1 reset`, `2 capture`). The previous implementation had
  five attributes with a phantom `single_buffer` (id 3) and a
  phantom `table_entries` (id 5), no `scaler_unit`, and surfaced
  the methods as `table_entry` / `update_table_entry` rather than
  the spec-defined `reset` / `capture`.
- Replaced the four-buffer constructor pair with the spec
  three-buffer signature `(logical_name, table_cell_values,
  table_cell_definition, scaler_unit, access[, version])` and
  renamed accessors accordingly
  (`SingleBuffer/TableEntries` removed, `ScalerUnit` added).
- `InvokeMethod` now version-gates the spec-defined methods: both
  `1 reset` and `2 capture` return `UnsupportedFeature` because the
  captured payload lifecycle is owned by the backend. Every other
  method id continues to report `MethodNotFound`.
- Updated `CosemRegisterTableObject` tests to drive the new
  three-buffer ctor, the renamed accessors, the four-attribute
  layout (probe `5` -> `AttributeNotFound`) and the `scaler_unit`
  round-trip; kept the existing `MethodsReturnUnsupportedFeature`
  test for spec methods `1` / `2`.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the four-attribute spec layout,
  the spec-defined `reset` / `capture` methods and the dropped
  `single_buffer` / `table_entries` fields.

## 0.80.0 - 2026-06-17

- Aligned the Auto Connect IC `29` built-in object
  (`CosemAutoConnectObject`) with IEC 62056-6-2 ED4 (2021)
  §4.7.6. The spec defines class version `2` ("Auto connect")
  with attributes `1..6` and one specific method
  `1 connect (data)`; the legacy class version `0`
  ("PSTN auto dial") keeps the same six attributes but defines
  no methods. The previous implementation was pinned to
  `MaxSupportedVersion = 0` and unconditionally reported
  `MethodNotFound` for every id, hiding the v2 method.
- Raised `MaxSupportedVersion` to `2` and made the short
  constructor default to class version `2`; the longer ctor that
  accepts an explicit version still allows v0 ("PSTN auto dial")
  for backward compatibility, and any out-of-range version is
  normalized to `MaxSupportedVersion = 2`.
- `InvokeMethod` now version-gates the spec-defined method:
  `method id 1` returns `UnsupportedFeature` for instances at
  class version `>= 2` (the built-in object does not own the
  dialler / radio stack), and `MethodNotFound` for legacy v0
  instances. Every other method id continues to report
  `MethodNotFound`.
- Replaced the `CosemAutoConnectObject.NoMethodsDefined` test
  with `ConnectMethodIsUnsupportedFeature` and
  `LegacyVersion0ReportsMethodNotFound`; updated
  `ExposesAllAttributes` to assert the new default version `2`.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the new default version, the
  spec-defined `connect` method and the legacy-v0 escape hatch.

## 0.79.0 - 2026-06-17

- Rebuilt the SMTP Setup IC `46` built-in object
  (`CosemSmtpSetupObject`) to match IEC 62056-6-2 ED4 (2021)
  §4.9.6 and DLMS UA Blue Book Ed. 12.1 §4.9.6. The spec defines
  six attributes: `1 logical_name`, `2 server_port`,
  `3 user_name`, `4 login_password`, `5 server_address`,
  `6 sender_address`. The previous implementation invented a
  non-spec `smtp_server` octet-string at id 2, shifted every
  subsequent attribute by one, used different names
  (`sender` vs `sender_address`), and added a non-existent
  seventh `receivers` array attribute. Constants, ctor parameter
  list, access-rights table, read/write switches and accessors
  are now `kSmtpSetup{ServerPort,UserName,LoginPassword,
  ServerAddress,SenderAddress}AttributeId` with attribute ids
  `2..6`. `ReadAttribute(7, ...)` now reports `AttributeNotFound`.
  Methods remain absent (the IC defines none); `InvokeMethod`
  continues to report `MethodNotFound` for every id.
- Updated `CosemSmtpSetupObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`, `NoMethodsDefined` and
  `NormalizesVersionAboveMax` to match the spec layout. The
  writable id set in the mutable-attribute test is now
  `{2, 3, 4, 5, 6}`; the `AttributeNotFound` probe shifts from
  id `8` to id `7`. The sample `SmtpSetupBuffers` helper now
  encodes a `long-unsigned 587` `server_port`, an octet-string
  `"smtp.example.com"` `server_address` and an octet-string
  `"a@b.c"` `sender_address` (no more `smtpServer`/`receivers`
  fields).
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe IC 46 attributes `2..6` and the
  new mutable set `{2, 3, 4, 5, 6}`.

## 0.78.0 - 2026-06-17

- Added the missing `7 list_of_allowed_callers` attribute to the
  Auto Answer IC `28` (`CosemAutoAnswerObject`). IEC 62056-6-2
  ED4 (2021) §4.7.5 and DLMS UA Blue Book Ed. 12.1 §4.6.4 define
  this attribute as an `array of allowed_caller_element`
  structures listing the caller identifications that the meter
  accepts; the built-in object previously stopped at
  attribute `6` and reported `AttributeNotFound` for the spec-
  defined attribute `7`. The constructors now take an extra
  `listOfAllowedCallers` content buffer (positioned after
  `numberOfRings`), the new attribute joins the writable set
  governed by the caller-selected `AttributeAccessMode`, and a
  `ListOfAllowedCallers()` accessor exposes the stored buffer.
  Methods remain absent (the IC defines none); `InvokeMethod`
  continues to report `MethodNotFound` for every id.
- Updated `CosemAutoAnswerObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode` and
  `NormalizesVersionAboveMax` to round-trip the new attribute, and
  expanded the writable id set in the mutable-attribute test to
  `{2, 3, 5, 6, 7}` so both writable and read-only paths assert
  the new behavior. `AttributeNotFound` is now expected at id `8`
  (previously `7`). The sample `AutoAnswerBuffers` helper now
  carries an empty `array(0)` payload as the
  `list_of_allowed_callers` default.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to document attribute `7 list_of_allowed_callers`
  and the new mutable set `{2, 3, 5, 6, 7}` for IC 28.

## 0.77.0 - 2026-06-17

- Rebuilt the M-Bus slave port setup IC `25`
  (`CosemMBusSlavePortSetupObject`) to match the five-attribute,
  zero-method spec layout defined in IEC 62056-6-2 ED4 (2021)
  §4.8.2 and DLMS UA Blue Book Ed. 12.1 §4.8.1. The class
  definition lists: `1 logical_name` (octet-string, static,
  read-only), `2 default_baud` (enum, static), `3 avail_baud`
  (enum, static), `4 addr_state` (enum, static, indicating whether
  the slave has been assigned a bus address) and `5 bus_address`
  (unsigned, static). The built-in object previously used the
  non-spec attribute names `status` and `mbus_port_reference` for
  attributes `4` and `5`, mismodelling them as a status enum and an
  octet-string reference to another object respectively. The
  constructors and accessor signatures have been rewritten to take
  the four content buffers (`default_baud`, `avail_baud`,
  `addr_state`, `bus_address`) as encoded DLMS Data buffers
  prepared by the caller; attributes `2`-`5` share the
  caller-selected `AttributeAccessMode` and accept in-place writes
  when permitted. The IC defines no specific methods
  (`Specific methods | m/o` column is empty in both editions);
  `InvokeMethod` now returns `MethodNotFound` for every id and
  clears method output. Earlier revisions surfaced a phantom
  `reset` method (id `1`) as `UnsupportedFeature`; that method is
  not defined by any published edition of the spec and has been
  removed along with the `kMBusSlavePortSetupResetMethodId`
  constant.
- Replaced the existing
  `CosemMBusSlavePortSetupObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnUnsupportedFeature` and `NormalizesVersionAboveMax`
  regression tests with versions that exercise the new attribute
  layout (sample `MBusSlavePortSetupBuffers` now carries
  enum-encoded `default_baud` 9 600 bps, `avail_baud` 38 400 bps,
  `addr_state` 1 "assigned" and unsigned-encoded `bus_address`
  0x42). The method test has been renamed `NoMethodsDefined` and
  asserts `MethodNotFound` plus cleared method output for every
  probed method id (`1`, `2`, `3`, `99`), matching the
  zero-method spec surface.
- Updated the COSEM IC support matrix and COSEM API guide to
  document the new spec-shaped attribute layout
  (`default_baud`, `avail_baud`, `addr_state`, `bus_address`),
  call out the removed `reset` method, and note that
  `InvokeMethod` now reports `MethodNotFound` for every id. The
  COSEM test plan's M-Bus slave port setup checklist has been
  refreshed to reflect the new attribute names and the
  zero-method `MethodNotFound` semantics.

## 0.76.0 - 2026-06-17

- Rebuilt the IEC twisted pair (1) Setup IC `24`
  (`CosemIecTwistedPairSetupObject`) to match the five-attribute
  spec layout defined in IEC 62056-6-2 ED4 (2021) §4.7.3 and DLMS
  UA Blue Book Ed. 12.1 §4.7.3. The class definition lists:
  `1 logical_name` (octet-string, static, read-only),
  `2 secondary_address` (octet-string of size 6, static),
  `3 primary_address_list` (array of octet-string, static),
  `4 tabi_list` (array of integer, static) and
  `5 fatal_error` (enum, dynamic) carrying the latest occurrence of
  one of the IEC 62056-31 protocol fatal errors. The built-in
  object previously exposed only two attributes using a mismatched
  naming and layout (`primary_address` long-unsigned at id `2` and
  `tabis` array of long-unsigned at id `3`) which did not match
  any published edition of the spec. The constructors and
  accessor signatures have been rewritten to take the four content
  buffers (`secondary_address`, `primary_address_list`,
  `tabi_list`, `fatal_error`) as encoded DLMS Data buffers prepared
  by the caller. Attributes `2`, `3` and `4` share the
  caller-selected `AttributeAccessMode` and accept in-place writes
  when permitted; `5 fatal_error` is server-managed and remains
  read-only at the wire surface regardless of the caller-selected
  mode (backends republish the buffer when the underlying stack
  observes a new fatal error). The IC defines no specific methods
  (`Specific methods | m/o` column is empty in both editions);
  `InvokeMethod` continues to return `MethodNotFound` for every id
  and clears method output.
- Replaced the existing
  `CosemIecTwistedPairSetupObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode` and `NormalizesVersionAboveMax`
  regression tests with versions that exercise the new five-attribute
  surface (sample `IecTwistedPairSetupBuffers` now carries the
  spec-shaped payloads, including a six-octet secondary address, an
  array of one-octet primary addresses, an array of TAB(i) integers
  and an enum-encoded fatal error). `MutableAttributesHonorAccessMode`
  additionally pins fatal_error's read-only semantics on writable
  instances. `NoMethodsDefined` keeps asserting `MethodNotFound` for
  every method id.
- Updated the IC support matrix (`docs/ic_support_matrix.md`) and
  the COSEM API guide (`lib/dlms-cosem/docs/01_cosem_api.md`) to
  describe the spec-aligned five-attribute layout and the
  read-only-by-design fatal_error semantics.

  BREAKING CHANGE: `CosemIecTwistedPairSetupObject`'s constructors
  and accessors have changed shape. The constructors now take
  four content buffers in the order
  `(secondary_address, primary_address_list, tabi_list, fatal_error)`
  instead of the previous two
  `(primary_address, tabis)`, and the accessor pair
  `PrimaryAddress()`/`Tabis()` has been replaced by
  `SecondaryAddress()`/`PrimaryAddressList()`/`TabiList()`/
  `FatalError()`. Callers constructing the IC at runtime or reading
  back its content must update their call sites and the encoded
  payloads they pass in (note the spec data-type changes: primary
  addresses are now array elements of octet-string of size 1, and
  TAB(i) entries are integers rather than long-unsigned values).
  The on-wire attribute count also grows from `3` to `5`; clients
  iterating over attributes `2..N` based on the previous
  implementation now observe ids `2..5` and must be prepared for
  the additional payloads.

## 0.75.0 - 2026-06-17

- Aligned the IPv4 Setup IC `42` (`CosemIpv4SetupObject`) method
  surface with IEC 62056-6-2 ED4 (2021) clause 4.9.2.3. The spec
  defines three specific methods: `1` `add_mc_IP_address`,
  `2` `delete_mc_IP_address` and `3` `get_nbof_mc_IP_addresses`.
  `InvokeMethod` previously dispatched only ids `1` and `2`,
  silently returning `MethodNotFound` for id `3` and hiding the
  spec method. The built-in object does not own multicast
  subscription policy nor expose the runtime
  `multicast_IP_address` array size, so method `3` is now surfaced
  explicitly as `UnsupportedFeature` alongside `1` and `2`; unknown
  method ids still report `MethodNotFound`. Not a breaking change
  at the public surface: any caller that was already receiving
  `MethodNotFound` for method `3` was out of spec, and callers
  that respect the documented status set handle the new status
  without changes.
- Removed the phantom `reset` method (id `1`) from the GSM
  Diagnostic IC `47` (`CosemGsmDiagnosticObject`). IEC 62056-6-2
  ED4 (2021) §5.6.8 and DLMS UA Blue Book Ed. 12.1 §5.6.8 define
  class_id `47`, version `0` with **no** specific methods (the
  "Specific methods | m/o" column is empty in both editions). The
  built-in object previously surfaced a fabricated `reset` method
  that the spec never defines; `InvokeMethod` now returns
  `MethodNotFound` for every method id, matching the spec. The
  associated `kGsmDiagnosticResetMethodId` constant has been
  removed.
- Updated the `CosemIpv4SetupObject.MulticastMethodsReturnUnsupportedFeature`
  regression test to assert all three spec methods return
  `UnsupportedFeature` and only ids `4`+ return `MethodNotFound`.
- Renamed `CosemGsmDiagnosticObject.MethodsReturnUnsupportedFeature`
  to `AllMethodsReturnMethodNotFound` and rewrote it to assert the
  new spec-aligned behaviour (every id, including the former `1`,
  returns `MethodNotFound`).
- Updated `docs/ic_support_matrix.md` and
  `lib/dlms-cosem/docs/01_cosem_api.md` to document the corrected
  IPv4 Setup and GSM Diagnostic method surfaces with spec
  citations.
- Bumped `VERSION` to `0.75.0`.

## 0.74.0 - 2026-06-17

- Surfaced the SAP Assignment IC `17` `connect_logical_device` method
  (id `1`) per IEC 62056-6-2 ED4 (2021) clause 4.4.4 and DLMS UA Blue
  Book Ed. 12.1 clause 5.3.4. `CosemSapAssignmentObject::InvokeMethod`
  previously returned `MethodNotFound` for every method id, silently
  hiding the only spec-defined method. The built-in object does not
  own the SAP / logical-device attachment policy, so method `1` is now
  surfaced explicitly as `UnsupportedFeature` (matching the pattern
  used by IC 5, 9, 10, 11 and 22); unknown method ids still report
  `MethodNotFound`. Not a breaking change at the public surface: any
  caller that was already receiving `MethodNotFound` for method 1 was
  out of spec, and callers that respect the documented status set
  (Ok / AccessDenied / UnsupportedFeature / MethodNotFound) handle
  the new status without changes.
- Updated the `DiscoveryObjects.RejectUnsupportedAttributesWritesAndMethods`
  regression test to assert the new SAP Assignment method semantics
  (id `1` -> `UnsupportedFeature`, id `99` -> `MethodNotFound`).
- Updated `docs/ic_support_matrix.md` and
  `lib/dlms-cosem/docs/01_cosem_api.md` to document the SAP Assignment
  method surface.
- Bumped `VERSION` to `0.74.0`.

## 0.73.0 - 2026-06-17

- Aligned the Schedule IC `10` built-in object (`CosemScheduleObject`)
  method ids with IEC 62056-6-2 ED4 (2021) clause 4.5.3 and DLMS UA
  Blue Book Ed. 12.1 clause 5.1.7. The spec defines three specific
  methods (`1` `enable_disable`, `2` `insert`, `3` `delete`); the
  implementation previously only recognised two methods and used the
  wrong ids (`1` `insert`, `2` `delete`), so clients targeting the
  spec id for `insert` actually triggered `enable_disable`, clients
  targeting the spec id for `delete` triggered `insert`, and method
  id `3` (`delete`) silently returned `MethodNotFound`. All three
  spec ids now return `UnsupportedFeature` (the built-in object does
  not own schedule-entry mutation policy); method id `0` and ids
  `>= 4` continue to return `MethodNotFound`. This is a breaking
  change for any caller that hardcoded the old (non-spec) method
  ids; callers passing spec ids are unaffected (they now reach the
  correct branch).
- Updated the `CosemScheduleObject.MethodsReturnUnsupportedFeature`
  regression test to cover the new id set (`{1, 2, 3}` ->
  `UnsupportedFeature`, `{0, 4, 5}` -> `MethodNotFound`).
- Updated `docs/ic_support_matrix.md` and
  `lib/dlms-cosem/docs/01_cosem_api.md` to list the spec-aligned
  method ids.
- Bumped `VERSION` to `0.73.0`.

## 0.72.0 - 2026-06-16

- Aligned the M-Bus Client IC `72` built-in object
  (`CosemMBusClientObject`) with IEC 62056-6-2 ED4 (2021) clause
  4.8.3 and DLMS UA Blue Book Ed. 12.1 clause 5.7.1. Attributes
  `13` (`configuration`) and `14` (`encryption_key_status`) are now
  exposed only when the requested class version is `>=1`; v0
  instances (Blue Book table for class_id=72, version=0) report
  `NoAccess` for these attribute ids in their `CosemAccessRights`
  and return `AttributeNotFound` from `ReadAttribute` and
  `WriteAttribute`, matching the spec which stops the v0 attribute
  table at attribute `12`. Previously both attributes leaked onto
  v0 instances and were freely readable and writable. The
  constructor signatures, supported method ids (`1`..`8`) and
  `MaxSupportedVersion = 1` are unchanged; the default constructors
  continue to instantiate v1 objects.
- Added a `CosemMBusClientObject.Version0DoesNotExposeConfigurationOrEncryptionKeyStatus`
  regression test covering the new v0 gating (access rights,
  reads, writes, and that attributes `1`..`12` remain available).
- Bumped `VERSION` to `0.72.0`.

## 0.71.0 - 2026-06-15

- Aligned the Profile Generic IC `7` built-in object
  (`CosemProfileGenericObject`) with IEC 62056-6-2 ED4 (2021) clause
  4.3.6 and DLMS UA Blue Book Ed. 12.1 clause 5.2.1. Both class
  version `0` (legacy) and version `1` (current) expose the full
  method set defined by the standard: `reset` (1), `capture` (2),
  `get_buffer_by_range` (3), and `get_buffer_by_index` (4). Previously
  methods `3` and `4` were silently hidden on the default v1
  instances (the gating predicate was also inverted relative to the
  spec, which does not gate the methods by version at all) and
  returned `MethodNotFound` instead of `UnsupportedFeature`. All four
  ids are now advertised in access rights as `Access` and return
  `UnsupportedFeature` pending the capture and journal execution
  policy.
- Made `sort_method` (attribute `5`) and `sort_object` (attribute
  `6`) explicit static properties of the IC instead of synthesising
  them. The basic constructor defaults to
  `CosemProfileGenericSortMethod::Fifo` with an empty
  `ObjectDefinition` (`class_id = 0`, zeroed logical name,
  `attribute_index = 0`, `data_index = 0`). New constructor
  overloads accept a `CosemProfileGenericSortMethod` and a
  `CosemCaptureObject` (combined with or without an explicit
  version) for callers that publish a sorted profile, and the new
  `SortMethod()` / `SortObject()` getters return the published
  values. The previous behaviour of returning the hardcoded `Fifo`
  enum for attribute `5` and deriving attribute `6` from the first
  capture object was incorrect: the standard treats them as
  independently configurable static attributes.
- Added regression tests
  `CosemProfileGenericObject.HonorsConfigurableSortMethodAndSortObject`
  and `CosemProfileGenericObject.DefaultSortMethodIsFifoWithEmptySortObject`
  that lock the new sort surface, extended
  `CosemProfileGenericObject.RejectsWritesAndReportsUnsupportedMethods`
  to assert all four method ids report `Access` /
  `UnsupportedFeature`, and rewrote
  `CosemProfileGenericObject.AcceptsExplicitVersion` so it exercises
  the corrected v0 and v1 method tables instead of the inverted
  gating.

## 0.70.0 - 2026-06-15

- Re-aligned the Association SN IC `12` built-in object
  (`CosemAssociationSnObject`) with IEC 62056-6-2 ED4 (2021) clause
  4.4.3 and DLMS UA Blue Book Ed. 12.1 clause 5.4.5. The specific
  method ids were renumbered to match the standard: `3`
  `read_by_logicalname`, `5` `change_secret`, `8`
  `reply_to_HLS_authentication`, `9` `add_user` (v3+) and `10`
  `remove_user` (v3+); ids `1`, `2`, `4`, `6`, `7` and `11+` are
  reserved or undefined and now report `MethodNotFound` instead of
  responding to the previously fabricated `add_object` /
  `remove_object` / `change_HLS_secret` mapping. `MaxSupportedVersion`
  was raised from `3` to `4` to match the current ED4 ceiling.
- Gated the version-dependent surface: `security_setup_reference`
  (attribute `4`) is exposed for v>=2, and `user_list` (attribute
  `5`) plus `current_user` (attribute `6`) for v>=3. Constructing a
  lower-version instance clears the gated buffers, reports
  `NoAccess` for them in the access-rights list and returns
  `AttributeNotFound` on reads or writes; `add_user` / `remove_user`
  return `MethodNotFound` outside v3+.
- Added regression tests
  `CosemAssociationSnObject.Version0DoesNotExposeSecuritySetupOrUserAttributes`
  and `CosemAssociationSnObject.Version2ExposesSecuritySetupButNotUserAttributes`
  that lock the new per-version gating, and updated the existing
  `MethodsReturnUnsupportedFeature` test to exercise the corrected
  method ids.

## 0.69.0 - 2026-06-15

- Tightened the per-version surface of the Association LN IC `15`
  built-in object (`CosemAssociationLnObject`) to match IEC 62056-6-2
  ED4 (2021) clause 4.4.4 and DLMS UA Blue Book Ed. 12.1 clause 4.4.4:
  the `user_list` (attribute `10`), `current_user` (attribute `11`),
  `add_user` (method `5`) and `remove_user` (method `6`) elements are
  defined for class version `3` and were previously surfaced on
  version `2` instances as well. Construction of a v2 instance now
  clears any caller-supplied `users`/`currentUser` payload, reports
  `NoAccess` for attributes `10`-`11` and methods `5`-`6` in the
  access-rights list, returns `AttributeNotFound` for reads of `10`
  or `11`, and returns `MethodNotFound` for invocations of `5` or
  `6`. The v0, v1 and v3 surfaces and the access-rights/object-list
  encoders are unchanged. `MaxSupportedVersion` stays at `3`.
- Added regression test
  `CosemAssociationLnObject.Version2DoesNotExposeUserAttributesOrMethods`
  that locks the v2 surface.

## 0.68.0 - 2026-06-15

- Removed phantom `port_speed` attribute (`10`) from the IEC Local
  Port Setup IC `19` built-in object (`CosemIecLocalPortSetupObject`).
  IEC 62056-6-2 ED4 (2021) clauses 4.7.1 and 5.6.1, and DLMS UA
  Blue Book Ed. 12.1 clause 4.7.1, define IC 19 with exactly nine
  attributes (`1`-`9`) in both versions `0` and `1` and no specific
  methods. The previous implementation surfaced an extra `port_speed`
  enum at attribute id `10`, gated to v1, which is not part of the
  standard.
- **Breaking change**: both `CosemIecLocalPortSetupObject` constructors
  drop the `portSpeed` parameter, and the `PortSpeed()` getter is
  removed. Callers must drop the `port_speed` buffer from construction
  sites. Reads of attribute `10` now report `AttributeNotFound`;
  attributes `2`-`9` continue to honor the caller-selected
  `AttributeAccessMode`. `MaxSupportedVersion` stays at `1`, and
  `InvokeMethod` still reports `MethodNotFound` for all method ids.

## 0.67.0 - 2026-06-15

- Bumped Push Setup IC `40` built-in object
  (`CosemPushSetupObject`) from `MaxSupportedVersion = 1` to
  `MaxSupportedVersion = 2`, aligning the per-version surface with
  IEC 62056-6-2 ED4 (2021):
  - v0 exposes attributes `1`-`7` (logical_name plus the v0 surface);
  - v1 adds attributes `8` port_reference, `9` push_client_SAP and
    `10` push_protection_parameters;
  - v2 adds attributes `11` push_operation_method,
    `12` confirmation_parameters and
    `13` last_confirmation_date_time.
  Reads and writes to attributes that are not part of the negotiated
  class version report `AttributeNotFound`; this fixes a regression
  in `0.26.0` that surfaced attributes `8`-`13` as if they all
  belonged to v1.
- Added method `2` `reset` (defined for v2 only): on a v2 instance
  it clears the stored `last_confirmation_date_time` buffer (the
  only persistent state owned by the built-in object) and returns
  `Ok`; on v0/v1 instances it returns `MethodNotFound`. Method `1`
  `push` continues to report `UnsupportedFeature` and undefined
  method ids continue to report `MethodNotFound`; method output is
  always cleared.
- Documented that the `repetition_delay` attribute (`7`) is
  long-unsigned in v0/v1 and a `{repetition_delay_min,
  repetition_delay_exponent, repetition_delay_max}` structure in
  v2. The buffer remains opaque; the caller is responsible for
  encoding the value that matches the negotiated version.

## 0.66.0 - 2026-06-15

- Added S-FSK Active Initiator IC `51` built-in object
  (`CosemSFskActiveInitiatorObject`) with class version `0`,
  exposing `active_initiator` (structure {system_title:
  octet-string(8), MAC_address: long-unsigned,
  L_SAP_selector: unsigned}, per IEC 62056-6-2 ED4 4.10.4.2.2
  and DLMS UA Blue Book IC 51) as an opaque encoded DLMS Data
  buffer prepared by the caller.
- Attribute `2` honours a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset_new_not_synchronized` returns
  `UnsupportedFeature`; undefined method ids return
  `MethodNotFound`; method output is always cleared.

## 0.65.0 - 2026-06-15

- Added S-FSK PLC PHY & MAC Setup IC `50` built-in object
  (`CosemSFskPlcPhyMacSetupObject`) with class version `1`,
  exposing initiator/delta electrical phase, max
  receiving/transmitting gain, search_initiator_threshold,
  frequencies (structure { mark_frequency: double-long-unsigned,
  space_frequency: double-long-unsigned }), mac_address,
  mac_group_addresses, repeater, repeater_status,
  min_delta_credit, initiator_mac_address,
  synchronization_locked and transmission_speed as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`15` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature`; undefined
  method ids return `MethodNotFound`; method output is always
  cleared.

## 0.64.0 - 2026-06-15

- Added PRIME PLC Application Identification IC `85` built-in
  object (`CosemPrimePlcApplicationIdentificationObject`) with
  class version `0`, exposing `application_identifier`
  (octet-string) as an opaque encoded DLMS Data buffer prepared
  by the caller.
- Attribute `2` uses a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.
- With Application Identification the PRIME PLC family is now
  fully covered (ICs `81`-`85` all have built-in opaque-buffer
  objects).

## 0.63.0 - 2026-06-15

- Added PRIME PLC MAC Address Setup IC `84` built-in object
  (`CosemPrimePlcMacAddressSetupObject`) with class version
  `0`, exposing `mac_address` (long-unsigned) as an opaque
  encoded DLMS Data buffer prepared by the caller.
- Attribute `2` uses a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.

## 0.62.0 - 2026-06-15

- Added PRIME PLC MAC Network Statistics IC `83` built-in
  object (`CosemPrimePlcMacNetworkStatisticsObject`) with class
  version `0`, exposing `node_registrations`,
  `node_unregistrations`, `processed_alive_msgs` and
  `handled_promotions` (double-long-unsigned) as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`5` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature`; undefined
  method ids return `MethodNotFound`; method output is always
  cleared.

## 0.61.0 - 2026-06-15

- Added PRIME PLC MAC Counters IC `82` built-in object
  (`CosemPrimePlcMacCountersObject`) with class version `0`,
  exposing `txdatapkt_count`, `rxdatapkt_count`,
  `txctrlpkt_count`, `rxctrlpkt_count`, `csmafail_count` and
  `csmachbusy_count` (double-long-unsigned) as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`7` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature`; undefined
  method ids return `MethodNotFound`; method output is always
  cleared.

## 0.60.0 - 2026-06-15

- Added PRIME PLC MAC Functional Parameters IC `81` built-in
  object (`CosemPrimePlcMacFunctionalParametersObject`) with
  class version `0`, exposing `lnid` (long-unsigned), `lsid`,
  `sid` (unsigned), `sna` (octet-string EUI-48), `state` (enum),
  `sct`, `scd` (long-unsigned) and `capabilities` (bit-string)
  as opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`9` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.

## 0.59.0 - 2026-06-15

- Added PRIME PLC MAC Setup IC `80` built-in object
  (`CosemPrimePlcMacSetupObject`) with class version `0`,
  exposing `mac_min_con_window`, `mac_max_con_window`
  (long-unsigned), `mac_channel_access_fairness_limit`
  (unsigned), `mac_EMA`, `mac_SAR_size`, `mac_max_PDU_size`,
  `mac_min_switch_search_time`, `mac_max_promotion_PDU`,
  `mac_promotion_PDU_TX_period` (long-unsigned),
  `mac_beacons_per_frame`, `mac_scp_max_TX_attempts`,
  `mac_CTL_re_TX_timer` (unsigned) and `mac_max_LNID`
  (long-unsigned) as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`14` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.

## 0.58.0 - 2026-06-15

- Added M-Bus Diagnostic IC `77` built-in object
  (`CosemMBusDiagnosticObject`) with class version `0`, exposing
  `received_signal_quality` (unsigned),
  `transmitter_signal_quality` (unsigned), `bbc` (long-unsigned),
  `fcs_ok_frames_counter` (double-long-unsigned),
  `fcs_nok_frames_counter` (double-long-unsigned) and
  `capture_time` (date-time octet-string(12)) as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`7` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so
  the backend can republish refreshed signal quality, frame
  counters and capture time after polling the M-Bus link
  out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports `MethodNotFound`
  for every method id and clears method output.

## 0.57.0 - 2026-06-15

- Added M-Bus Master Port Setup IC `73` built-in object
  (`CosemMBusMasterPortSetupObject`) with class version `0`,
  exposing `comm_speed` (enum) as an opaque encoded DLMS Data
  buffer prepared by the caller.
- Attribute `2` `comm_speed` honors a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports `MethodNotFound`
  for every method id and clears method output.

## 0.56.0 - 2026-06-15

- Added M-Bus Client IC `72` built-in object
  (`CosemMBusClientObject`) with class version `1`, exposing
  `mbus_port_reference` (octet-string(6) LN to IEC HDLC Setup),
  `capture_definition` (array of structure {`data_link_reference`:
  octet-string, `value_information_block`: octet-string}),
  `capture_period` (double-long-unsigned, seconds),
  `primary_address` (unsigned), `identification_number`
  (double-long-unsigned), `manufacturer_id` (long-unsigned),
  `version` (unsigned, M-Bus device version), `device_type`
  (unsigned), `access_number` (unsigned), `status` (unsigned),
  `alarm` (unsigned), `configuration` (long-unsigned, v1) and
  `encryption_key_status` (enum, v1) as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `2`-`14` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted, so the backend can republish refreshed
  identification, status, alarm, configuration and key-status
  payloads after driving the M-Bus slave out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `slave_install`, `2` `slave_deinstall`,
  `3` `capture`, `4` `reset_alarm`, `5` `synchronise_clock`,
  `6` `send_data`, `7` `set_encryption_key` and
  `8` `transfer_key` return `UnsupportedFeature` and clear method
  output (the built-in object does not drive the M-Bus slave);
  other method ids return `MethodNotFound`.

## 0.55.0 - 2026-06-15

- Added Association SN IC `12` built-in object
  (`CosemAssociationSnObject`) with class version `3`, exposing
  `object_list` (array of structure {`base_name`: long-int,
  `class_id`: long-unsigned, `version`: unsigned, `logical_name`:
  octet-string(6), `access_rights`: structure}),
  `access_rights_list` (array of structure),
  `security_setup_reference` (octet-string(6) LN to Security
  Setup), `user_list` (array of structure {`user_id`: unsigned,
  `user_name`: visible-string}) and `current_user` (structure
  {`user_id`: unsigned, `user_name`: visible-string}) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed object list, access rights list,
  security setup reference and user lists after HLS authentication,
  HLS secret rotation or list mutations performed out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `reply_to_HLS_authentication`, `2`
  `change_HLS_secret`, `3` `add_object`, `4` `remove_object`,
  `5` `add_user` and `6` `remove_user` return `UnsupportedFeature`
  and clear method output (the built-in object does not perform
  authentication or list mutations); other method ids return
  `MethodNotFound`.

## 0.54.0 - 2026-06-15

- Added IEC Local Port Setup IC `19` built-in object
  (`CosemIecLocalPortSetupObject`) with class version `1`,
  exposing `default_mode` (enum), `default_baud` (enum),
  `proposed_baud` (enum), `response_time` (enum),
  `device_address` (octet-string with the device-address logical
  name) and `password_1` / `password_2` / `password_5` (octet-strings
  carrying the level-1, level-2 and level-5 passwords) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`9` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed mode, baud, response time, device
  address and passwords after configuration changes out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC defines no methods; `InvokeMethod` reports `MethodNotFound`
  for all method ids and clears method output.

## 0.53.0 - 2026-06-15

- Added Data Protection IC `30` built-in object
  (`CosemDataProtectionObject`) with class version `0`, exposing
  `protection_buffer` (octet-string carrying the protected
  payload), `protection_object_list` (array of structure
  {`protection_type`: enum, `protection_options`: structure,
  `protection_parameters_id`: octet-string}),
  `protection_parameters_get` (array of structure
  {`protection_type`: enum, `protection_options`: structure}),
  `protection_parameters_set` (array of structure
  {`protection_type`: enum, `protection_options`: structure}) and
  `required_protection` (bit-string with authentication /
  encryption / digital-signature bits) as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed protection buffer, object list,
  parameter tables and required-protection mask after performing
  the protected operations out-of-band); logical_name (`1`) is
  read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `get_protected_attributes`, `2`
  `set_protected_attributes` and `3` `invoke_protected_method`
  return `UnsupportedFeature` and clear method output (the
  built-in object does not perform protected operations); other
  method ids return `MethodNotFound`.

## 0.52.0 - 2026-06-15

- Added Compact Data IC `62` built-in object
  (`CosemCompactDataObject`) with class version `0`, exposing
  `buffer` (octet-string carrying the compact-encoded data),
  `capture_objects` (array of structure {`class_id`: long-unsigned,
  `logical_name`: octet-string(6), `attribute_index`: integer,
  `data_index`: long-unsigned}), `template_id` (unsigned),
  `template_description` (octet-string with the A-XDR template)
  and `capture_method` (enum, `1` invoke / `2` implicit) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can refresh the compact buffer, template metadata and
  capture method after acquiring fresh capture data out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `reset` and `2` `capture` return
  `UnsupportedFeature` and clear method output (the built-in object
  does not manage capture); other method ids return
  `MethodNotFound`.

## 0.51.0 - 2026-06-15

- Added Parameter Monitor IC `65` built-in object
  (`CosemParameterMonitorObject`) with class version `0`, exposing
  `changed_parameter` (structure {`class_id`: long-unsigned,
  `logical_name`: octet-string(6), `attribute_index`: integer,
  `value`: data}), `capture_time` (date-time octet-string(12)) and
  `parameters` (array of structure {`class_id`, `logical_name`,
  `attribute_index`}) as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish the most recent changed parameter, capture
  time and monitored-parameters table after evaluating parameter
  changes out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `insert` and `2` `delete` return `UnsupportedFeature`
  and clear method output (the built-in object does not manage the
  monitored-parameters table); other method ids return
  `MethodNotFound`.

## 0.50.0 - 2026-06-15

- Added Status Mapping IC `63` built-in object
  (`CosemStatusMappingObject`) with class version `0`, exposing
  `status_word` (bit-string carrying the raw status value) and
  `mappings` (array of structure {`status_value`: bit-string,
  `mapped_value`: bit-string}) as opaque encoded DLMS Data buffers
  prepared by the caller.
- Attributes `2`-`3` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed status and mapping tables after
  evaluating the status word out-of-band); logical_name (`1`) is
  read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Status Mapping IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.49.0 - 2026-06-15

- Added Arbitrator IC `68` built-in object
  (`CosemArbitratorObject`) with class version `0`, exposing
  `actions` (array of structure {`script_logical_name`,
  `script_selector`}), `permissions_table` (array of bit-string,
  one row per actor), `weightings_table` (array of array of
  long-unsigned), `most_recent_requests_table` (array of
  bit-string) and `last_outcome` (unsigned, index of the winning
  script) as opaque encoded DLMS Data buffers prepared by the
  caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed arbitration state after driving
  arbitration out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `request_action` and `2` `reset` return
  `UnsupportedFeature` and clear method output (the built-in object
  does not own arbitration semantics); other method ids return
  `MethodNotFound`.

## 0.48.0 - 2026-06-15

- Added Sensor Manager IC `67` built-in object
  (`CosemSensorManagerObject`) with class version `0`, exposing
  `status` (enum), `serial_number` (octet-string), `device_type`
  (octet-string), `manufacturer_id` (long-unsigned),
  `firmware_version` (octet-string), `metrology_firmware_version`
  (octet-string), `driver` (octet-string), `communication_desc`,
  `setup_desc` and `measurement_desc` (arrays of structure) as
  opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`11` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed sensor metadata after polling the
  slave out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Sensor Manager IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.47.0 - 2026-06-15

- Added Utility Tables IC `26` built-in object
  (`CosemUtilityTablesObject`) with class version `0`, exposing
  `table_id` (long-unsigned), `length` (double-long-unsigned) and
  `buffer` (octet-string carrying the raw table payload) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Utility Tables IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.46.0 - 2026-06-15

- Added IPv6 Setup IC `48` built-in object (`CosemIpv6SetupObject`)
  with class version `0`, exposing `data_link_layer_reference`
  (octet-string LN), `address_config_mode` (enum),
  `unicast_ip_address` and `multicast_ip_address` (arrays of
   16-byte octet-string), `gateway_ip_address`,
  `primary_dns_address`, `secondary_dns_address` (16-byte
  octet-string), `traffic_class` (unsigned) and
  `neighbor_discovery_setup` (array of structure) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`10` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed addressing after running the
  network stack out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `add_address` and `2` `remove_address` return
  `UnsupportedFeature` and clear method output (the built-in object
  does not own network-stack semantics); other method ids return
  `MethodNotFound`.

## 0.45.0 - 2026-06-15

- Added M-Bus slave port setup IC `25` built-in object
  (`CosemMBusSlavePortSetupObject`) with class version `0`, exposing
  `default_baud` (enum), `available_baud` (enum), `status` (enum)
  and `mbus_port_reference` (octet-string referencing an IEC HDLC
  Setup logical name) as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`5` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature` and clears method
  output (the built-in object does not own slave-port reset
  semantics); other method ids return `MethodNotFound`.

## 0.44.0 - 2026-06-15

- Added IEC twisted pair (1) Setup IC `24` built-in object
  (`CosemIecTwistedPairSetupObject`) with class version `0`,
  exposing `primary_address` (long-unsigned) and `tabis` (array of
  long-unsigned listing the registered secondary addresses) as
  opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2` and `3` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IEC twisted pair (1) Setup IC defines no methods; `InvokeMethod`
  reports `MethodNotFound` for all method ids and clears method
  output.

## 0.43.0 - 2026-06-15

- Added GSM Diagnostic IC `47` built-in object
  (`CosemGsmDiagnosticObject`) with class version `0`, exposing
  `operator` (octet-string), `status` (enum),
  `circuit_switched_status` (enum), `packet_switched_status` (enum),
  `cell_info` (structure of cell_id/location_id/signal_quality/ber/
  mcc/mnc/channel_number), `adjacent_cells` (array of cell_id/
  signal_quality structures) and `capture_time` (date_time
  octet-string) as opaque encoded DLMS Data buffers prepared by the
  caller.
- Attributes `2`-`8` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can publish refreshed diagnostic snapshots when read-write
  is granted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature` and clears method
  output (the built-in object does not own modem reset semantics);
  other method ids return `MethodNotFound`.

## 0.42.0 - 2026-06-15

- Added SMTP Setup IC `46` built-in object (`CosemSmtpSetupObject`)
  with class version `0`, exposing `SMTP_server` (octet-string),
  `SMTP_server_port` (long-unsigned), `user_name` (octet-string),
  `login_password` (octet-string), `sender` (octet-string) and
  `receivers` (array of octet-string) as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `2`-`7` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- SMTP Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.41.0 - 2026-06-15

- Added PPP Setup IC `44` built-in object (`CosemPppSetupObject`)
  with class version `0`, exposing `PHY_reference` (octet-string),
  `LCP_options` (array of structure), `IPCP_options` (array of
  structure) and `PPP_authentication` (structure of
  user_name/password) as opaque encoded DLMS Data buffers prepared by
  the caller.
- Attributes `2`-`5` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- PPP Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.40.0 - 2026-06-15

- Added MAC Address Setup IC `43` built-in object
  (`CosemMacAddressSetupObject`) with class version `0`, exposing
  `mac_address` (octet-string(6)) as an opaque encoded DLMS Data
  buffer prepared by the caller.
- Attribute `2` honors a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- MAC Address Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.39.0 - 2026-06-15

- Added IPv4 Setup IC `42` built-in object (`CosemIpv4SetupObject`)
  with class version `0`, exposing `DL_reference`, `IP_address`,
  `multicast_IP_address`, `IP_options`, `subnet_mask`,
  `gateway_IP_address`, `use_DHCP_flag`, `primary_DNS_address` and
  `secondary_DNS_address` as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`10` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `add_mc_IP_address` and `2` `delete_mc_IP_address`
  return `UnsupportedFeature` with cleared output (built-in object
  does not own multicast subscription policy); other method ids return
  `MethodNotFound` with cleared output.

## 0.38.0 - 2026-06-15

- Added Auto Answer IC `28` built-in object (`CosemAutoAnswerObject`)
  with class version `0`, exposing `mode`, `listening_window`,
  `status`, `number_of_calls` and `number_of_rings` as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`, `3`, `5` and `6` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) and status (`4`) are read-only.
- `SetStatus` exposes backend-driven refresh of status regardless of
  the access mode used for the mutable attributes.
- Constructors normalize versions above `MaxSupportedVersion`.
- Auto Answer IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.37.0 - 2026-06-15

- Added GPRS Modem Setup IC `45` built-in object
  (`CosemGprsModemSetupObject`) with class version `0`, exposing
  `APN`, `PIN code` and `quality_of_service` as opaque encoded DLMS
  Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- GPRS Modem Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.36.0 - 2026-06-15

- Added Auto Connect IC `29` built-in object (`CosemAutoConnectObject`)
  with class version `0`, exposing `mode`, `repetitions`,
  `repetition_delay`, `calling_window` and `destination_list` as
  opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Auto Connect IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.35.0 - 2026-06-15

- Added Modem Configuration IC `27` built-in object
  (`CosemModemConfigurationObject`) with class version `1`, exposing
  `communication_speed`, `initialisation_strings` and `modem_profile`
  as opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Modem Configuration IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.34.0 - 2026-06-15

- Added Single Action Schedule IC `22` built-in object
  (`CosemSingleActionScheduleObject`) with class version `0`, exposing
  `executed_script`, `type` and `execution_time` as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Single Action Schedule IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.33.0 - 2026-06-15

- Added Special Days Table IC `11` built-in object
  (`CosemSpecialDaysTableObject`) with class version `0`, exposing
  `entries` (array of special_day_entry) as an opaque encoded DLMS
  Data buffer prepared by the caller.
- Attribute `2` (entries) honors a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only. A setter exposes
  backend-driven refresh of entries regardless of access mode.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Special Days Table methods `1` `insert` and `2` `delete` as
  explicit `UnsupportedFeature` (application-defined special-day
  entry mutation); other method ids report `MethodNotFound`.

## 0.32.0 - 2026-06-15

- Added Schedule IC `10` built-in object (`CosemScheduleObject`) with
  class version `0`, exposing `entries` (array of
  Schedule_table_entry) as an opaque encoded DLMS Data buffer prepared
  by the caller.
- Attribute `2` (entries) honors a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only. A setter exposes
  backend-driven refresh of entries regardless of access mode.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Schedule methods `1` `insert` and `2` `delete` as explicit
  `UnsupportedFeature` (application-defined schedule-entry mutation);
  other method ids report `MethodNotFound`.

## 0.31.0 - 2026-06-15

- Added TCP-UDP Setup IC `41` built-in object
  (`CosemTcpUdpSetupObject`) with class version `0`, exposing
  `tcp_udp_port`, `ip_reference`, `mss`, `nb_of_sim_conn` and
  `inactivity_time_out` as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- TCP-UDP Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.30.0 - 2026-06-15

- Added Register Table IC `61` built-in object
  (`CosemRegisterTableObject`) with class version `0`, exposing
  `table_cell_values`, `single_buffer`, `table_cell_definition` and
  `table_entries` as opaque encoded DLMS Data buffers prepared by the
  caller.
- Attributes `3`-`5` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name and table_cell_values (`1`, `2`) are read-only with a
  setter that exposes backend-driven refresh of `table_cell_values`
  from a future register-table backend.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Register Table methods `1` `table_entry` and `2`
  `update_table_entry` as explicit `UnsupportedFeature`
  (application-defined column selection and update); other method ids
  report `MethodNotFound`.

## 0.29.0 - 2026-06-15

- Added IEC HDLC Setup IC `23` built-in object
  (`CosemIecHdlcSetupObject`) with `MaxSupportedVersion = 1`, exposing
  `comm_speed`, `window_size_transmit`, `window_size_receive`,
  `max_info_field_length_transmit`, `max_info_field_length_receive`,
  `inter_octet_time_out`, `inactivity_time_out` and `device_address`
  as opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`8` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name and device_address (`1`, `9`) are read-only with a
  setter that exposes backend-driven refresh of the assigned HDLC
  address.
- Constructors normalize versions above `MaxSupportedVersion`.
- IEC HDLC Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.28.0 - 2026-06-15

- Added Limiter IC `71` built-in object (`CosemLimiterObject`) with
  class version `0`, exposing `monitored_value`, `threshold_active`,
  `threshold_normal`, `threshold_emergency`,
  `min_over_threshold_duration`, `min_under_threshold_duration`,
  `emergency_profile`, `emergency_profile_group_id_list`,
  `emergency_profile_active` and `actions` as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `3`-`11` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name and monitored_value (`1`, `2`) are read-only with
  setters that expose backend-driven refresh of `threshold_active` and
  `emergency_profile_active` from a future limiter backend.
- Constructors normalize versions above `MaxSupportedVersion`.
- Limiter IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.27.0 - 2026-06-15

- Added Disconnect Control IC `70` built-in object
  (`CosemDisconnectControlObject`) with class version `0`, exposing
  `output_state`, `control_state` and `control_mode` as opaque encoded
  DLMS Data buffers prepared by the caller.
- `control_mode` (attribute 4) shares a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place when
  permitted); logical_name, `output_state` and `control_state` are
  read-only with setters that expose backend-driven refresh of the
  output/control state from a future relay backend.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Disconnect Control methods `1` `remote_disconnect` and `2`
  `remote_reconnect` as explicit `UnsupportedFeature`
  (application-defined relay switching and state transitions); other
  method ids report `MethodNotFound`.

## 0.26.0 - 2026-06-15

- Added Push Setup IC `40` built-in object (`CosemPushSetupObject`) with
  `MaxSupportedVersion = 1`, exposing the v0 surface
  (`push_object_list`, `send_destination_and_method`,
  `communication_window`, `randomisation_start_interval`,
  `number_of_retries`, `repetition_delay`) and the v1 surface
  (`port_reference`, `push_client_SAP`, `push_protection_parameters`,
  `push_operation_method`, `confirmation_parameters`,
  `last_confirmation_date_time`) as opaque encoded DLMS Data buffers
  prepared by the caller.
- v0 objects hide attributes 8-13 (`AttributeNotFound` on read and
  write). v1 objects expose attributes 2-12 with a caller-selected
  `AttributeAccessMode`; logical_name and `last_confirmation_date_time`
  (`13`) are read-only, and a setter exposes backend-driven refresh of
  the last-confirmation timestamp.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Push Setup method `1` `push` as explicit `UnsupportedFeature`
  (application-defined scheduling, transport selection and
  confirmation tracking); other method ids report `MethodNotFound`.

## 0.25.0 - 2026-06-15

- Added Image Transfer IC `18` built-in object
  (`CosemImageTransferObject`) with class version `0`, exposing
  attributes `1` logical_name, `2` image_block_size, `3`
  image_transferred_blocks_status, `4`
  image_first_not_transferred_block_number, `5`
  image_transfer_enabled, `6` image_transfer_status and `7`
  image_to_activate_info as opaque encoded DLMS Data buffers prepared
  by the caller.
- `image_transfer_enabled` (attribute 5) shares a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place when
  permitted); logical_name and all other attributes (2-4, 6-7) are
  read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Image Transfer; constructors normalize versions
  above the maximum. Setters expose dynamic refresh of
  transferred-blocks status, first-not-transferred counter,
  image-transfer status and image-to-activate info.
- Added Image Transfer methods `1` `image_transfer_initiate`, `2`
  `image_block_transfer`, `3` `image_verify` and `4` `image_activate`
  as explicit `UnsupportedFeature` (application-defined firmware
  transfer/storage semantics); other method ids report
  `MethodNotFound`.

## 0.24.0 - 2026-06-15

- Added Activity Calendar IC `20` built-in object
  (`CosemActivityCalendarObject`) with class version `0`, exposing
  attributes `1` logical_name, `2`-`5` active calendar snapshot
  (calendar_name_active, season_profile_active,
  week_profile_table_active, day_profile_table_active), `6`-`9` passive
  calendar (calendar_name_passive, season_profile_passive,
  week_profile_table_passive, day_profile_table_passive) and `10`
  activate_passive_calendar_time as opaque encoded DLMS Data buffers
  prepared by the caller.
- Passive attributes (`6`-`10`) share a caller-selected access mode
  (writes replace the stored buffer in-place when permitted); logical_name
  and active snapshot attributes (`2`-`5`) are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Activity Calendar; constructors normalize versions
  above the maximum.
- Added Activity Calendar method `1` `activate_passive_calendar` as
  explicit `UnsupportedFeature` (application-defined activation policy);
  other method ids report `MethodNotFound`.

## 0.23.0 - 2026-06-15

- Added Script Table IC `9` built-in object
  (`CosemScriptTableObject`) with class version `0`, exposing attributes
  `1` logical_name and `2` scripts as an opaque encoded DLMS Data buffer
  prepared by the caller. `scripts` access mode is caller-selected.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Script Table; constructors normalize versions above the
  maximum.
- Added Script Table method `1` `execute` as explicit `UnsupportedFeature`
  (application-defined script semantics); other method ids report
  `MethodNotFound`.

## 0.22.0 - 2026-06-15

- Added Register Monitor IC `21` built-in object
  (`CosemRegisterMonitorObject`) with class version `0`, exposing
  attributes `1` logical_name, `2` thresholds, `3` monitored_value and
  `4` actions as opaque encoded DLMS Data buffers prepared by the caller.
  `thresholds` access mode is caller-selected; `monitored_value` and
  `actions` are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Register Monitor; constructors normalize versions above
  the maximum.
- Register Monitor v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id.

## 0.21.0 - 2026-06-15

- Added Register Activation IC `6` built-in object
  (`CosemRegisterActivationObject`) with class version `0`, exposing
  attributes `1` logical_name, `2` register_assignment, `3` mask_list and
  `4` active_mask as opaque encoded DLMS Data buffers prepared by the caller.
  All attributes are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Register Activation; constructors normalize versions above
  the maximum.
- Added Register Activation methods `1` `add_register`, `2` `add_mask` and
  `3` `delete_mask` as explicit `UnsupportedFeature` (application-defined
  semantics); other method ids report `MethodNotFound`.

## 0.20.0 - 2026-06-15

- Added Demand Register IC `5` built-in object
  (`CosemDemandRegisterObject`) with class version `0`, exposing attributes
  `1` logical_name, `2` current_average_value, `3` last_average_value,
  `4` scaler_unit, `5` status, `6` capture_time, `7` start_time_current,
  `8` period (encoded as DLMS Data `double-long-unsigned`) and
  `9` number_of_periods (encoded as DLMS Data `long-unsigned`). All
  attributes are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking constructor
  for Demand Register; constructors normalize versions above the maximum.
- Added Demand Register methods `1` `reset` and `2` `next_period` as explicit
  `UnsupportedFeature` (application-defined semantics); other method ids
  report `MethodNotFound`.

## 0.19.0 - 2026-06-15

- Added Extended Register IC `4` built-in object
  (`CosemExtendedRegisterObject`) with class version `0`, exposing attributes
  `1` logical_name (read-only), `2` value (caller-selected access mode),
  `3` scaler_unit (read-only), `4` status (read-only) and `5` capture_time
  (read-only DLMS date-time octet-string).
- Added explicit `MaxSupportedVersion` constant and version-taking constructor
  for Extended Register; constructors normalize versions above the maximum.
- Added Extended Register method `1` `reset` as explicit `UnsupportedFeature`
  (application-defined semantics); other method ids report `MethodNotFound`.

## 0.18.0 - 2026-06-15

- Added pluggable `ICosemCertificateStore` interface and
  `InMemoryCosemCertificateStore` reference backend for Security Setup IC `64`
  version `1`.
- Changed Security Setup attribute `6` `certificates` to encode the attached
  certificate store entries as a DLMS Data array of `certificate_info`
  structures (entity enum, type enum, serial / issuer / subject /
  subject-alt-name octet-strings).
- Added Security Setup method `6` `import_certificate` (octet-string payload),
  method `7` `export_certificate` and method `8` `remove_certificate` with
  Blue Book `by_entity` / `by_serial` selector parsing dispatched to the
  certificate store backend. Without an attached store these methods return
  `UnsupportedFeature` and clear method output. Methods `3`/`4`/`5` (key
  agreement, generate key pair, generate certificate request) remain
  `UnsupportedFeature` until an ECDSA / X.509 stack is wired in.

## 0.17.0 - 2026-06-14

- Added caller-selected descriptor version constructors and
  `MaxSupportedVersion` constants for built-in Data, Register, Clock,
  Profile Generic, SAP Assignment, and Security Setup COSEM objects.
- Changed built-in Security Setup to publish class version `1` by default,
  matching the implemented security attributes and key-transfer method surface.
- Added version-gated method exposure for Profile Generic version `0` legacy
  buffer methods and Security Setup version `0`/`1` method surfaces.
- Added version-gated Security Setup version `1` certificates attribute support
  as an encoded empty DLMS Data array when no certificate store is configured.
- Fixed HDLC session-mode segmented inbound APDU handling so RR frames
  acknowledge each accepted I-frame segment with the updated `N(R)` value.

## 0.16.2 - 2026-06-13

- Fixed HDLC session teardown so owned HDLC/TCP clients perform the mandatory
  DISC/UA data-link disconnect after application association release and before
  closing the underlying transport.
- Fixed confirmed release against meters that return `RLRE` with
  `user-information` and non-canonical BER length fields around the embedded
  xDLMS `InitiateResponse`.

## 0.16.1 - 2026-06-13

- Fixed HDLC session-mode APDU exchange so a final client I-frame no longer
  waits for a separate RR acknowledgement, and piggybacked server I-frame
  responses are queued for `ReceiveApdu()` instead of being consumed as control
  acknowledgements.

## 0.16.0 - 2026-06-13

- Added explicit HDLC client, logical-device, and physical-device addressing
  fields to endpoint/profile options, so COSEM SAPs are no longer reused as
  HDLC link-layer addresses.
- Added HDLC profile wire trace hooks and changed Wrapper/TCP trace event names
  to `WireWrite` and `WireRead` for wire-level diagnostics.

## 0.15.0 - 2026-06-07

- Added version-gated Association LN configuration up to class version `3`,
  including user-list/current-user attributes for version `2+`.

## 0.14.0 - 2026-06-07

- Added Association LN status and optional security setup reference attributes,
  and exposed documented Association LN methods as explicit unsupported
  features.

## 0.13.0 - 2026-06-07

- Added a built-in partial Clock IC `8` object with read/write support for
  documented clock attributes and explicit unsupported clock methods.

## 0.12.1 - 2026-06-07

- Added a GUI-oriented СПОДЭС OBIS read example and allowed Profile Generic
  buffer row decoding to validate `date-time`, `date`, and `time` values.

## 0.12.0 - 2026-06-07

- Added DLMS `date-time`, `date`, and `time` Data codec support plus
  GUI-facing typed client helpers.

## 0.11.0 - 2026-06-07

- Added Association LN object-list and access-rights encode/decode helpers for
  the built-in COSEM discovery model.

## 0.10.0 - 2026-06-07

- Added Profile Generic selective access range and entry descriptor helpers for
  building and validating selector `1` and selector `2` parameters.

## 0.9.0 - 2026-06-07

- Added Profile Generic composite attribute encode/decode helpers for
  `capture_objects`, `sort_object`, and `buffer` row framing.

## 0.8.0 - 2026-06-07

- Added xDLMS and client facade support for GET selective access requests with
  encoded DLMS Data selection parameters.

## 0.7.1 - 2026-06-07

- Added a GUI-oriented client read example that uses `ReadAttribute` and typed
  DLMS Data decoding without requiring application code to parse A-XDR bytes.

## 0.7.0 - 2026-06-07

- Added `dlms-client` typed DLMS Data encode/decode helpers for GUI consumers
  that should not manipulate A-XDR bytes directly for common scalar values.

## 0.6.0 - 2026-06-07

- Added detailed `DlmsClient` attribute/method helpers for GUI consumers that
  need class-id/OBIS calls plus access/action result details.

## 0.5.0 - 2026-06-07

- Added a built-in partial Profile Generic IC `7` object with read-only
  profile attributes and explicit unsupported reset/capture methods.

## 0.4.11 - 2026-06-07

- Reconciled the IC and security production-readiness documentation against
  the knowledge-base class lists and current built-in COSEM coverage.

## 0.4.10 - 2026-06-07

- Extended the package install smoke audit to verify aggregate target
  `INTERFACE_LINK_LIBRARIES` for all documented DLMSFramework components.

## 0.4.9 - 2026-06-07

- Added profile C API regression coverage for datagram receive callbacks that
  return a non-OK status after reporting bytes.

## 0.4.8 - 2026-06-07

- Hardened transport C API read and receive entry points so `bytes_read`
  remains zero for non-OK backend statuses.

## 0.4.7 - 2026-06-07

- Hardened the association C API callback APDU adapter so receive callbacks
  cannot leave stale output or accept over-reported received sizes.

## 0.4.6 - 2026-06-07

- Documented the complete `DLMSFramework` component-to-target package mapping
  in the README and release versioning notes.

## 0.4.5 - 2026-06-07

- Added install-tree consumer examples for the `io`, `cosem_server`, and
  `framework` package components and included them in package smoke coverage.

## 0.4.4 - 2026-06-07

- Added an io-only install-tree smoke test with OpenSSL disabled to verify that
  `find_package(DLMSFramework COMPONENTS io)` stays independent from security
  dependencies.

## 0.4.3 - 2026-06-07

- Scoped the package OpenSSL dependency to components that transitively use
  security and added a codec-only install-tree smoke test with OpenSSL disabled.

## 0.4.2 - 2026-06-07

- Cleaned up exported CMake targets so installed concrete targets expose the
  package include directory only once.

## 0.4.1 - 2026-06-07

- Hardened profile mutable receive small-buffer failures so Wrapper TCP,
  Wrapper UDP and HDLC channels report the required APDU size.

## 0.4.0 - 2026-06-07

- Added the Wrapper C API stream decoder push entry point so C consumers can
  incrementally decode WPDU streams and drain pending frames.

## 0.3.48 - 2026-06-07

- Hardened HDLC C stream decoder and reassembler small-buffer failures so the
  information size output reports the required payload size.

## 0.3.47 - 2026-06-07

- Hardened endpoint security bundle creation so validation failures reset the
  caller-provided security bundle output.

## 0.3.46 - 2026-06-07

- Hardened xDLMS server dispatcher failures so stale get, set and action
  result objects are cleared on validation and handler errors.

## 0.3.45 - 2026-06-07

- Hardened xDLMS server adapter status-level failures so stale get, set and
  action result objects are cleared before returning an error status.

## 0.3.44 - 2026-06-07

- Hardened COSEM object registry read and method invocation failures so stale
  caller output buffers are cleared on missing objects, denied access and
  object-level errors.

## 0.3.43 - 2026-06-07

- Hardened built-in simple COSEM objects so missing attributes and methods
  clear stale output buffers on public object API failures.

## 0.3.42 - 2026-06-07

- Hardened Security Setup method invocation failures so stale COSEM output
  data is cleared for invalid activation requests and unsupported methods.

## 0.3.41 - 2026-06-07

- Hardened Security Setup key transfer so invocation-counter reset policy
  failures reject key rotation before installing transferred keys.

## 0.3.40 - 2026-06-07

- Hardened HDLC C decode buffer validation to allow a null information buffer
  only when its size is zero.

## 0.3.39 - 2026-06-06

- Added Security Setup regression coverage for the currently unsupported
  method id range.

## 0.3.38 - 2026-06-06

- Added Security Setup key-transfer regression coverage for unsupported
  security suites.

## 0.3.37 - 2026-06-06

- Hardened transport C write/send entry points to reject null input with a
  non-zero size at the C ABI boundary.

## 0.3.36 - 2026-06-06

- Hardened profile C callback adapters so failed read/receive callbacks do not
  propagate non-zero byte counts.

## 0.3.35 - 2026-06-06

- Added Security Setup key-transfer malformed input regression coverage for
  unsupported key identifiers and trailing bytes.

## 0.3.34 - 2026-06-06

- Added invocation-counter reset policy hook for key rotation.
- Updated `InMemoryInvocationCounterStore` to reset local and remote replay
  state after key rotation.
- Wired Security Setup key transfer to invoke the reset hook after successful
  key installation.

## 0.3.33 - 2026-06-06

- Wired Security Setup IC `64` method `global_key_transfer` for Suite 0 key
  transfer through the mutable key store abstraction.
- Added AXDR key-data parsing for wrapped global unicast, broadcast,
  authentication and key-encryption keys.
- Added COSEM regression coverage for installing an unwrapped authentication
  key through Security Setup method 2.

## 0.3.32 - 2026-06-06

- Added `IMutableKeyStore` as the public mutable key sink contract for
  Security Setup key transfer.
- Updated `InMemoryKeyStore` to implement the mutable key store abstraction.
- Added regression coverage for writing keys through the abstract interface.

## 0.3.31 - 2026-06-06

- Added Suite 0 AES key wrap/unwrap primitives for Security Setup key transfer.
- Added RFC 3394 regression coverage for wrapping and unwrapping 128-bit keys
  with a 128-bit key encryption key.
- Updated the security roadmap and support matrix for the new key wrapping
  primitive.

## 0.3.30 - 2026-06-06

- Added endpoint lifecycle regression coverage for retrying HLS GMAC
  authentication after an invalid client reply.
- Verified `ServerEndpoint` remains open, unassociated and able to accept a
  later valid GMAC HLS reply after the first verification failure.
- Updated the production-ready roadmap for HLS GMAC negotiated failure
  coverage.

## 0.3.29 - 2026-06-06

- Added endpoint lifecycle regression coverage for retrying high-password HLS
  authentication after an invalid client reply.
- Verified `ServerEndpoint` remains open, unassociated and able to accept a
  later valid HLS reply after the first HLS verification failure.
- Updated the production-ready roadmap for high-authentication negotiated
  failure coverage.

## 0.3.28 - 2026-06-06

- Added endpoint lifecycle regression coverage for retrying negotiated server
  and push listener opens after malformed association requests.
- Verified failed negotiated opens close the APDU channel and do not leave
  endpoints logically open.
- Updated the production-ready roadmap for negotiated open-failure cleanup
  coverage.

## 0.3.27 - 2026-06-06

- Added endpoint lifecycle regression coverage for gateway downstream close
  failures.
- Verified `GatewayEndpoint` remains logically open when the downstream APDU
  channel reports a close failure.
- Updated the production-ready roadmap for gateway close-failure lifecycle
  coverage.

## 0.3.26 - 2026-06-06

- Added endpoint lifecycle regression coverage for close failures on server and
  push listener endpoints.
- Verified endpoints remain logically open when their underlying APDU channel
  reports a close failure.
- Updated the production-ready roadmap for close-failure lifecycle coverage.

## 0.3.25 - 2026-06-06

- Added endpoint lifecycle regression coverage for idempotent `Open()` on
  server, push listener and gateway endpoints.
- Verified repeated `Open()` calls do not reopen downstream channels or
  gateway upstreams when endpoints are already open.
- Updated the production-ready roadmap for endpoint lifecycle idempotency
  coverage.

## 0.3.24 - 2026-06-06

- Added sender-aware invocation counter validation to the invocation counter
  store abstraction.
- Updated `InMemoryInvocationCounterStore` to track remote replay state per
  remote system title.
- Updated protected APDU and HLS GMAC verification paths to validate remote
  invocation counters against the remote system title.
- Added regression coverage for independent replay state across remote system
  titles.

## 0.3.23 - 2026-06-06

- Added `InvocationCounterObjectName()` for the public invocation counter OBIS
  `0.0.43.1.0.255`.
- Added `MakeInvocationCounterObject()` to expose the current invocation
  counter as a read-only COSEM Data object encoded as AXDR
  `double-long-unsigned`.
- Added deterministic COSEM coverage for the public invocation counter object.

## 0.3.22 - 2026-06-06

- Added regression coverage for invocation counter exhaustion through
  `CipheredApduProcessor::Protect`.
- Added regression coverage for invocation counter exhaustion through HLS GMAC
  response generation.
- Updated security documentation to mark local invocation counter exhaustion
  refusal as covered at the protected APDU and HLS GMAC boundaries.

## 0.3.21 - 2026-06-06

- Implemented Security Setup IC `64` `security_activate` method for encoded
  security policy activation.
- Enforced monotonic security policy activation so active policy bits cannot be
  weakened through `security_activate`.
- Added direct and registry regression coverage for Security Setup activation,
  malformed activation input and policy weakening rejection.

## 0.3.20 - 2026-06-06

- Added `CosemSecuritySetupObject` for Security Setup IC `64` as a read-only
  COSEM extension point exposing security policy, security suite and
  client/server system titles.
- Added explicit unsupported method behavior for Security Setup activation,
  key transfer, key agreement and certificate operation slots.
- Updated security, IC and production-ready roadmap documentation for the
  current Security Setup support level.

## 0.3.19 - 2026-06-06

- Fixed Profile C API receive validation so invalid caller output buffers and
  null `written_size` are rejected at the C ABI boundary before the underlying
  APDU channel is called.
- Documented the Profile receive output-buffer validation contract.

## 0.3.18 - 2026-06-06

- Fixed HDLC strict encode and C API encode small-buffer handling so
  `written_size` reports the required encoded frame size on
  `OutputBufferTooSmall` without writing a partial frame.
- Documented the HDLC small-output-buffer size contract.

## 0.3.17 - 2026-06-06

- Fixed Wrapper strict encode and C API encode small-buffer handling so
  `written_size` reports the required WPDU size on `OutputBufferTooSmall`
  without writing a partial WPDU.
- Documented the Wrapper small-output-buffer size contract.

## 0.3.16 - 2026-06-06

- Added the production-ready roadmap based on code review findings.
- Fixed APDU raw xDLMS C API small-buffer handling so `written_size` reports
  the required encoded APDU size on `OutputBufferTooSmall` without writing a
  partial APDU.
- Documented the APDU small-output-buffer size contract.

## 0.3.15 - 2026-06-06

- Fixed LLC strict encode and C API encode small-buffer handling so
  `written_size` reports the required LPDU size on `OutputBufferTooSmall`
  without writing a partial LPDU.
- Documented the LLC small-output-buffer size contract.

## 0.3.14 - 2026-06-06

- Refreshed root, package, release, architecture, and component documentation
  for the monorepo component model and `DLMSFramework` CMake components.
- Documented current aggregate targets, install-tree consumer examples, and
  public extension points after layer modernization.
- Removed obsolete submodule and per-layer repository wording from active
  documentation and implementation plans.

## 0.3.13 - 2026-06-06

- Fixed LLC C API decode validation so null input with zero size follows the
  C++ codec contract and returns `NeedMoreData`.
- Added LLC C API regression coverage for empty null-input decode cleanup.

## 0.3.12 - 2026-06-06

- Fixed Wrapper C API decode validation so each provided output pointer is
  cleared before null output-pointer validation errors are returned.
- Added Wrapper C API regression coverage for partial output cleanup.

## 0.3.11 - 2026-06-06

- Fixed APDU C API encode validation so provided `written_size` outputs are
  cleared before null input validation errors.
- Added APDU C API regression coverage for null input output-size cleanup.

## 0.3.10 - 2026-06-06

- Fixed Association C API option validation so invalid application contexts,
  authentication modes, and null low-level credentials with non-zero sizes are
  rejected during handle creation.
- Added Association C API regression coverage for invalid option rejection.

## 0.3.9 - 2026-06-06

- Fixed HDLC C API decode, stream decoder, and reassembler entry points so
  provided output frame structures are cleared before validation errors.
- Added HDLC C API regression coverage for output frame cleanup.

## 0.3.8 - 2026-06-06

- Fixed Association C API result accessors so provided result outputs are
  cleared before validation errors are returned.
- Added Association C API regression coverage for get-result output cleanup.

## 0.3.7 - 2026-06-06

- Fixed Wrapper C API decode validation so null data output buffers are rejected
  before `data_size` is written for non-empty decoded payloads.
- Added Wrapper C API regression coverage for decode output cleanup.

## 0.3.6 - 2026-06-06

- Fixed Transport C API receive validation so null output buffers are rejected
  before lower transport calls when a non-zero output size is requested.
- Added Transport C API regression coverage for null receive output buffers.

## 0.3.5 - 2026-06-06

- Fixed Profile C API receive validation so a provided `written_size` output is
  cleared before `dlms_profile_receive_apdu()` returns validation errors.
- Added Profile C API regression coverage for receive output-size cleanup.

## 0.3.4 - 2026-06-06

- Fixed APDU C API xDLMS encode validation so null output buffers return
  `INVALID_ARGUMENT` while still clearing `written_size` when it is provided.
- Added an APDU C API guard for payload sizes that would overflow the encoded
  APDU size calculation.
- Added APDU C API regression coverage for those edge cases.

## 0.3.3 - 2026-06-06

- Fixed HDLC C API limit conversion so zero fields in `dlms_hdlc_limits_t`
  preserve the documented default limits.
- Fixed HDLC C API stream decoder error handling to reset the decoder after
  codec errors.
- Fixed HDLC C API stream decoder and reassembler payload output validation to
  reject null information buffers when a non-empty Information field is
  returned.
- Added HDLC C API regression coverage for those edge cases.

## 0.3.2 - 2026-06-05

- Fixed listener runtimes to close accepted APDU channels when constructing or
  opening the per-connection server, push, or gateway endpoint fails.
- Added listener runtime regression coverage for accepted-channel cleanup on
  endpoint open failures.

## 0.3.1 - 2026-06-05

- Added install-tree package consumer examples for `dlms::codec`,
  `dlms::protocol`, and `dlms::runtime`.
- Extended package install smoke verification to build those examples against
  the installed `DLMSFramework` package and audit exported CMake target files.
- Documented CMake components in the versioning and release checklist docs.

## 0.3.0 - 2026-06-05

- Added CMake package component support for the documented aggregate targets:
  `codec`, `io`, `protocol`, `cosem_server`, `runtime`, and `framework`.
- Extended the install package smoke test to require those components through
  `find_package(DLMSFramework COMPONENTS ...)`.

## 0.2.2 - 2026-06-05

- Added `README.md`, `CHANGELOG.md`, and `VERSION` to the installed
  `DLMSFramework` package metadata under `share/doc/DLMSFramework`.
- Extended the package artifact smoke test to require those release metadata
  files in generated ZIP artifacts.

## 0.2.1 - 2026-06-05

- Removed bundled GoogleTest/GMock headers and libraries from the
  `DLMSFramework` release ZIP when tests use the fetched test dependency.
- Extended the package artifact smoke test to reject release ZIPs that contain
  `include/gtest`, `include/gmock`, or GoogleTest/GMock libraries.

## 0.2.0 - 2026-06-05

- Completed the layer modernization pass with narrow abstract interface headers
  for client xDLMS services, server services, endpoint listeners, push/gateway
  ports, and COSEM logical-device dispatch.
- Kept default implementations source-compatible while allowing users to
  implement custom layer ports without including concrete runtime classes.
- Documented the final layer audit and interface header inventory in the system
  architecture guide.

## 0.1.9 - 2026-06-01

- Added CI release publication for `v*` tag pushes, attaching the verified
  `DLMSFramework-<version>.zip` package artifact to the GitHub release.
- Documented that tagged releases publish the same ZIP artifact that passed
  clean release verification.

## 0.1.8 - 2026-06-01

- Added a release tag consistency CTest that validates `v<version>` tag names
  against the root `VERSION` when a tag context is present.
- Extended CI to run release verification for `v*` tag pushes.

## 0.1.7 - 2026-06-01

- Added release checklist documentation that ties SemVer version bumps,
  changelog entries, clean verification, Git tags, and CI package artifacts
  into one release procedure.

## 0.1.6 - 2026-06-01

- Added GitHub Actions artifact upload for the generated
  `DLMSFramework-<version>.zip` package after CI release verification passes.
- Documented that CI preserves the verified ZIP package as a workflow artifact.

## 0.1.5 - 2026-06-01

- Added GitHub Actions CI metadata that runs the local MSYS2 MinGW clean
  release verification script on push, pull request, and manual dispatch.
- Documented that CI uses the same release verification recipe as local
  release checks.

## 0.1.4 - 2026-06-01

- Added a local MSYS2 MinGW clean release verification script that configures,
  builds, tests, and packages the root framework from a fresh build directory.
- Documented the release verification script in the README and SemVer release
  rules.

## 0.1.3 - 2026-06-01

- Added a default CTest smoke check that builds the root `DLMSFramework` ZIP
  package artifact and verifies that it is produced.
- Documented package artifact verification in the SemVer release rules and
  install quickstart.

## 0.1.2 - 2026-06-01

- Added a default CTest check that requires the current root `VERSION` to have
  a matching `CHANGELOG.md` release entry.
- Documented the changelog requirement in the SemVer release rules.

## 0.1.1 - 2026-06-01

- Documented the root monorepo quickstart, component map, build/test workflow,
  install command, and `DLMSFramework` CMake package consumption.
- Added this changelog as the release-note anchor for future SemVer bumps.

## 0.1.0 - 2026-06-01

- Established the root monorepo as the canonical source tree.
- Added root SemVer source file and CMake SemVer validation.
- Added aggregate CMake framework targets and root install/export support.
