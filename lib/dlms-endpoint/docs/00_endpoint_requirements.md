# dlms-endpoint Requirements

## Scope

`dlms-endpoint` composes existing DLMS/COSEM framework repositories into
runtime endpoints:

- synchronous client endpoint;
- server endpoint;
- push listener endpoint;
- gateway endpoint.

The layer exists to keep transport/profile/association loops out of lower
service repositories. It is allowed to depend on facade and orchestration
repositories, but those repositories must not depend on `dlms-endpoint`.

## Normative Model

The DLMS/COSEM architecture separates:

- modelling: COSEM objects and logical devices;
- messaging: ACSE and xDLMS APDUs;
- transporting: communication profiles and transport connections.

`dlms-endpoint` sits above those layers as an application process composition
layer. It does not redefine APDU codecs, xDLMS services, security algorithms,
COSEM object behaviour, or transport framing.

## In Scope

- endpoint option structures for client/server/push/gateway modes;
- construction of TCP, UDP, serial, Wrapper, and HDLC profile channels;
- composition of association, security, xDLMS, client, server, and COSEM
  components;
- abstract endpoint composition seams for caller-provided channel, server
  service, push handler, gateway policy, and gateway upstream implementations;
- synchronous `RunOnce` style server and push listener loops;
- bounded `RunUntil` loops driven by caller-owned stop conditions;
- listener runtimes that can compose accepted channels with either default
  logical-device dispatch or caller-provided server service implementations;
- gateway bridge between a downstream endpoint and an upstream client endpoint;
- deterministic fake-channel tests for endpoint orchestration.

## Out Of Scope

- TCP, UDP, and serial implementations;
- HDLC, LLC, Wrapper, ACSE, and xDLMS codecs;
- association negotiation rules;
- HLS, GMAC, AES-GCM, or password algorithm implementations;
- COSEM object storage and business logic;
- long-running thread ownership in the first MVP phase;
- persistence of keys, invocation counters, or meter data;
- application-specific SPODES object sets.

## Dependency Rules

Allowed direct dependencies:

- `dlms-transport`;
- `dlms-profile`;
- `dlms-association`;
- `dlms-xdlms`;
- `dlms-security`;
- `dlms-cosem`;
- `dlms-client`;
- `dlms-server`.

Forbidden dependencies:

- any example application repository;
- root integration tests;
- generated certification utility code;
- platform GUI or service frameworks.

No lower layer may include `dlms-endpoint` headers.

## MVP Requirements

### Client Endpoint

- open a Wrapper/TCP profile;
- open a no-security or password association through `dlms-client`;
- perform GET, SET, and ACTION through the existing client facade;
- release and close deterministically.

### Server Endpoint

- accept or receive APDUs from a caller-provided profile channel;
- process association APDUs using server-side association support when it is
  available;
- forward decoded xDLMS requests to `dlms-server`;
- allow callers to provide a custom `dlms::server::IServerService`
  implementation instead of the default COSEM logical-device dispatcher;
- keep client/server SAP and authenticated/ciphered metadata in
  `ServerAssociationContext`;
- release a negotiated association by delegating incoming RLRQ/RLRE handling
  to `dlms-association`;
- build response APDUs through `dlms-xdlms`.

### Push Listener Endpoint

- receive push APDUs from Wrapper/TCP or HDLC profile channels;
- decode push payloads through existing APDU/xDLMS primitives when available;
- expose received payloads to caller code without storing them.

### Gateway Endpoint

- accept downstream APDUs;
- optionally terminate downstream association and security;
- release a negotiated downstream association by delegating incoming RLRQ/RLRE
  handling to `dlms-association`;
- forward selected requests upstream through `dlms-client`;
- return upstream response status and data to the downstream server path.

## Non-Functional Requirements

- C++11 public API;
- status-code based runtime errors;
- no exceptions in runtime control flow;
- deterministic ownership model documented in API;
- no background threads unless explicitly requested by a later phase;
- tests must not require a live meter by default.
