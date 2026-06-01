# dlms-endpoint Test Plan

## Goals

Tests must verify endpoint composition without duplicating lower-layer unit
tests.

## Unit Tests

| Area | Tests |
|---|---|
| status mapping | every lower-layer category maps to stable `EndpointStatus` |
| option validation | missing host, invalid port, missing serial device, unsupported profile, unsupported client transport, HDLC address ranges |
| transport factory | selects TCP, UDP, or serial construction path |
| profile factory | selects Wrapper or HDLC construction path |
| listener factory | selects TCP profile listener, selects UDP push listener, and rejects unsupported listener transports |
| security factory | selects none, low password, high password, or high GMAC options; validates GMAC key material and peer title shape; reports ciphering only when explicit `cipheredApdu` is selected |
| lifecycle | `Open`, `RunOnce`, and `Close` reject invalid states; runtime `Open`/`Close` are idempotent around listener state |
| cleanup | close paths drop local downstream/client state after peer or upstream close failures |

## Client Endpoint Tests

- opens a fake Wrapper/TCP APDU path;
- maps High GMAC endpoint security material into `dlms-client` options before
  network use;
- forwards a configured peer system title into the client security context;
- rejects ciphered APDU mode without the global unicast encryption key;
- forwards GET to `dlms-client`;
- forwards SET to `dlms-client`;
- forwards ACTION to `dlms-client`;
- closes all composed resources in reverse order.

## Server Endpoint Tests

- receives one APDU from a fake profile channel;
- rejects `RunOnce` before `Open`;
- forwards xDLMS service APDUs to `dlms-server`;
- forwards xDLMS service APDUs to a fake caller-provided
  `dlms::server::IServerService`;
- stores association metadata in `ServerAssociationContext`;
- optionally composes `dlms-association::AssociationServer` during `Open()`
  and then serves the first xDLMS APDU through the same channel;
- releases a negotiated association when the first APDU after AARE is RLRQ;
- negotiates Low Password association when endpoint security carries a
  matching configured password;
- rejects negotiated Low Password association when the client credential
  mismatches the configured password;
- negotiates High Password and High GMAC association through endpoint HLS
  strategies;
- serves ciphered APDUs only after explicit `cipheredApdu` selection and HLS
  completion;
- returns response APDUs through the same profile channel.

## Push Listener Tests

- opens and closes a caller-provided APDU channel;
- receives one APDU and calls `IPushIndicationHandler`;
- maps profile receive failures to `EndpointStatus`;
- propagates handler errors as endpoint status;
- rejects `RunOnce` before `Open`.

## Gateway Tests

- denies requests rejected by `IGatewayPolicy`;
- forwards allowed GET, SET, and ACTION upstream;
- maps upstream service errors to endpoint lifecycle status;
- returns downstream access/action result errors for policy denial;
- rejects `RunOnce` before `Open`;
- maps downstream receive/send failures to `EndpointStatus`.

## Listener Runtime Tests

- opens and closes the caller-provided listener;
- rejects `RunOnce` before `Open`;
- maps listener accept failures to `EndpointStatus`;
- rejects successful accepts that return no channel;
- server runtime accepts one channel and delegates one server endpoint
  `RunOnce`;
- server runtime accepts one channel and can delegate service dispatch through
  a fake caller-provided `dlms::server::IServerService`;
- push runtime accepts one channel and dispatches one push APDU;
- gateway runtime accepts one downstream channel and forwards one allowed GET
  upstream.

## Listener Factory Tests

- creates a closed TCP profile listener from endpoint options;
- creates a closed UDP Wrapper push listener from push endpoint options;
- rejects generic UDP listener construction and serial listener construction;
- opens a loopback TCP listener on an ephemeral port;
- accepts a local TCP connection and returns a non-null WRAPPER APDU channel;
- accepts a local TCP connection and returns a non-null HDLC APDU channel.
- accepts a local TCP HDLC connection with explicit data-link session enabled
  and completes SNRM/UA before APDU exchange.
- maps client endpoint HDLC profile options to no-session and session client
  data-link behavior.
- closes and drops local client state when graceful client association release
  fails after the peer has already closed the channel.
- opens a loopback UDP push listener on an ephemeral port and returns a
  non-null WRAPPER APDU channel.

## Integration Tests

Root integration tests should cover:

- client endpoint GET over fake Wrapper/TCP path;
- public client endpoint GET against server endpoint over in-memory channel;
- public TCP Wrapper client endpoint High GMAC with ciphered GET, SET, and
  ACTION against server endpoint;
- server endpoint GET/SET/ACTION against minimal COSEM object;
- optional live Wrapper/TCP smoke test gated by explicit environment variables.

Live meter tests must remain opt-in and must not run in the default suite.

Server-side AARQ/AARE endpoint tests should compose
`dlms-association::AssociationServer`. Endpoint tests may build AARQ fixtures
through `dlms-apdu`, but endpoint production code must not hand-roll ACSE
parsing or AARE encoding.

Gateway downstream AARQ/AARE endpoint tests follow the same boundary. The
gateway endpoint should prove that opt-in downstream association negotiation
emits AARE during `Open()` and then forwards the first xDLMS request during
`RunOnce()` without opening the upstream path before association succeeds.
They should also prove that a downstream RLRQ is released through RLRE without
invoking upstream GET, SET, or ACTION services.
Low Password gateway tests should cover a matching credential and a mismatch
rejection before the upstream path opens.

Push listener AARQ/AARE endpoint tests also compose
`dlms-association::AssociationServer`. They should prove that opt-in
association negotiation emits AARE during `Open()` and that the first raw push
APDU is dispatched unchanged during `RunOnce()`. Low Password push listener
tests should cover a matching credential and a mismatch rejection. They should
also prove that a downstream RLRQ is released through RLRE without dispatching
the release APDU to the push handler.
