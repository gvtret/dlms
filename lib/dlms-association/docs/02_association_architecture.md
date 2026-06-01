# dlms-association Architecture

## 1. Layer Position

```mermaid
flowchart TD
  Client["Future dlms-client"]
  Association["lib/dlms-association"]
  Apdu["lib/dlms-apdu"]
  Profile["lib/dlms-profile"]
  Lower["transport + hdlc/llc/wrapper"]

  Client --> Association
  Association --> Apdu
  Association --> Profile
  Profile --> Lower
```

## 2. Open Handshake

```mermaid
sequenceDiagram
  participant App as Caller
  participant Assoc as AssociationClient
  participant Apdu as dlms-apdu
  participant Channel as IApduChannel

  App->>Assoc: Open()
  Assoc->>Channel: Open()
  App->>Assoc: Establish()
  Assoc->>Apdu: Encode AARQ(InitiateRequest)
  Assoc->>Channel: SendApdu(AARQ)
  Assoc->>Channel: ReceiveApdu()
  Assoc->>Apdu: Decode AARE(InitiateResponse)
  Assoc-->>App: AssociationResult
```

## 3. Server Accept Handshake

```mermaid
sequenceDiagram
  participant App as Caller
  participant Assoc as AssociationServer
  participant Apdu as dlms-apdu
  participant Channel as IApduChannel

  App->>Assoc: Open()
  Assoc->>Channel: Open()
  App->>Assoc: Accept()
  Assoc->>Channel: ReceiveApdu(AARQ)
  Assoc->>Apdu: Decode AARQ(InitiateRequest)
  Assoc->>Assoc: Validate optional LLS credential
  Assoc->>Apdu: Encode AARE(InitiateResponse)
  Assoc->>Channel: SendApdu(AARE)
  Assoc-->>App: AssociationResult
```

The server processor owns only ACSE association negotiation. It exposes the
negotiated xDLMS context for higher-layer composition and deliberately does not
dispatch COSEM objects, run endpoint listeners, or manage background loops.

## 4. Server Release Handshake

```mermaid
sequenceDiagram
  participant App as Caller
  participant Assoc as AssociationServer
  participant Apdu as dlms-apdu
  participant Channel as IApduChannel

  App->>Assoc: Release()
  Assoc->>Channel: ReceiveApdu(RLRQ)
  Assoc->>Apdu: Decode RLRQ
  Assoc->>Apdu: Encode RLRE
  Assoc->>Channel: SendApdu(RLRE)
  Assoc->>Channel: Close()
  Assoc-->>App: Closed
```

Failed server release receive, decode, send, or close operations leave the
server associated so the caller can still fall back to `Close()`.
Endpoint-style callers that have already received the next APDU may call the
overload that accepts the RLRQ bytes; it skips only the receive step.

## 5. State Machine

```mermaid
stateDiagram-v2
  [*] --> Closed
  Closed --> Open: Open ok
  Open --> Associating: Establish
  Associating --> Associated: AARE accepted
  Associating --> Open: send/receive/decode/reject
  Associated --> Closed: Release ok
  Associated --> Associated: Release send/receive/decode failure
  Associated --> Closed: Close
  Open --> Closed: Close
```

`AssociationServer` uses the same state names. `Accept()` moves
`Open -> Associating -> Associated` on a successful AARQ/AARE exchange and
returns to `Open` on receive, decode, negotiation, or send failure.
`Release()` moves `Associated -> Closed` on a successful RLRQ/RLRE exchange and
leaves the server `Associated` on receive, decode, send, or close failure.

## 6. Class Interaction

```mermaid
classDiagram
  class AssociationClient {
    +Open() AssociationStatus
    +Close() AssociationStatus
    +Establish() AssociationStatus
    +Release() AssociationStatus
    +State() AssociationState
    +Result() AssociationResult
  }

  class AssociationServer {
    +Open() AssociationStatus
    +Close() AssociationStatus
    +Accept() AssociationStatus
    +Release() AssociationStatus
    +Release(vector~uint8_t~) AssociationStatus
    +State() AssociationState
    +Result() AssociationResult
  }

  class IApduChannel {
    +Open() ProfileStatus
    +Close() ProfileStatus
    +SendApdu(ProfileByteView) ProfileStatus
    +ReceiveApdu(vector~uint8_t~&) ProfileStatus
  }

  class AssociationOptions
  class AssociationServerOptions
  class AssociationResult
  class dlms_apdu {
    +EncodeAcseApdu()
    +DecodeAcseApdu()
  }

  AssociationClient --> IApduChannel
  AssociationClient --> AssociationOptions
  AssociationClient --> AssociationResult
  AssociationClient --> dlms_apdu
  AssociationServer --> IApduChannel
  AssociationServer --> AssociationServerOptions
  AssociationServer --> AssociationResult
  AssociationServer --> dlms_apdu
```

## 7. Ownership

`AssociationClient` and `AssociationServer` store references to
`dlms::profile::IApduChannel`. They do not own the channel and do not own
transport resources directly.

## 8. Authentication Boundary

```mermaid
flowchart TD
  Caller["Caller"]
  Options["AssociationOptions"]
  Client["AssociationClient"]
  Hls["IHighLevelSecurityStrategy"]
  Apdu["dlms-apdu ACSE codec"]
  FutureSecurity["Future security layer"]

  Caller --> Options
  Options --> Client
  Client --> Hls
  Hls --> FutureSecurity
  Client --> Apdu
```

```mermaid
sequenceDiagram
  participant App as Caller
  participant Assoc as AssociationClient
  participant Hls as IHighLevelSecurityStrategy
  participant Apdu as dlms-apdu

  App->>Assoc: Establish()
  Assoc->>Assoc: Validate authentication mode
  alt None
    Assoc->>Apdu: Encode AARQ without authentication fields
  else LLS
    Assoc->>Apdu: Add ACSE requirements, mechanism name, credential
  else HLS
    Assoc->>Hls: Mechanism()
    Assoc->>Hls: BuildInitialChallenge()
    Assoc-->>App: UnsupportedAuthentication until ACSE auth encode exists
  end
```

`dlms-association` owns only the association state machine, option validation,
raw LLS ACSE authentication field selection, and server-side Low Password byte
comparison. HLS challenge functions, ciphering, keys, and invocation counters
remain delegated to future client and security composition.
