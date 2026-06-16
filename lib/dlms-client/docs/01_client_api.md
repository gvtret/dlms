# dlms-client API

## 0. Public Headers

Public headers:

```text
include/dlms/client/client_status.hpp
include/dlms/client/client_data.hpp
include/dlms/client/client_options.hpp
include/dlms/client/client_xdlms_service_interface.hpp
include/dlms/client/client.hpp
```

Applications that implement only the GET/SET/ACTION backend can include
`client_xdlms_service_interface.hpp`. Applications that use the facade include
`client.hpp`, which remains a source-compatible umbrella for the facade API.

## 1. Status Model

`ClientStatus` is the public status enum for the facade.

Current values:

```cpp
enum class ClientStatus
{
  Ok,
  InvalidArgument,
  InvalidState,
  TransportOpenFailed,
  ChannelOpenFailed,
  AssociationFailed,
  NotAssociated,
  SendFailed,
  ReceiveFailed,
  ServiceRejected,
  SecurityFailed,
  UnsupportedFeature,
  BlockTransferRequired,
  InvokeIdMismatch,
  CodecFailed,
  InternalError
};
```

The implementation maps lower-layer statuses into `ClientStatus`; applications
should not need to include lower-layer status enums for basic client usage.

### 1.1 Facade Status Mapping Policy

The facade owns a small public enum on purpose. Lower
layers (`dlms-transport`, `dlms-wrapper`, `dlms-hdlc`,
`dlms-llc`, `dlms-apdu`, `dlms-xdlms`, `dlms-association`,
`dlms-security`) each have a richer status enum, and the
facade collapses them so callers do not have to depend on
eight different headers to handle one Get/Set/Action call.
This section documents what the facade keeps and what it
intentionally drops.

#### What the facade preserves

Whenever a more specific public `ClientStatus` exists, the
mapper preserves it instead of folding the failure into
`InternalError`:

- `InvalidArgument`, `InvalidState`, `UnsupportedFeature` are
  preserved verbatim across every layer that exposes them.
- `SendFailed` / `ReceiveFailed` split the data-link / wrapper
  failure modes: open / write-side failures map to
  `SendFailed`; read / decode / timeout / would-block /
  need-more-data / output-buffer-too-small / invalid-frame /
  invalid-length / invalid-address / payload-too-large /
  connection-closed all map to `ReceiveFailed`. Both keep
  the caller informed about which direction failed without
  forcing them to pull in `ProfileStatus`.
- xDLMS-layer mismatches that used to be opaque are now
  first-class facade values:
  - `BlockTransferRequired` for `XdlmsStatus::BlockTransferRequired`,
  - `InvokeIdMismatch` for `XdlmsStatus::InvokeIdMismatch`,
  - `CodecFailed` for `XdlmsStatus::EncodeFailed` and
    `XdlmsStatus::DecodeFailed`.
  Before `0.97.0` these were silently collapsed to
  `InternalError` / `UnsupportedFeature`; callers can now
  decide whether to retry with a different block size, abort
  the association, or escalate the codec failure for
  debugging.
- `SecurityFailed` is preserved for every authentication,
  ciphering, replay, or key-store error from `dlms-security`.
  The narrower distinction (bad GMAC tag vs. wrong key role
  vs. counter rejected) is intentionally dropped at this
  boundary; see below.
- `AssociationFailed` / `NotAssociated` cover the entire
  ACSE/release/abort surface of `dlms-association`.
  Sub-states (AARQ vs. AARE vs. RLRQ vs. abort code) are
  intentionally dropped here.

#### What the facade intentionally drops

These collapses are deliberate; do not "fix" them by widening
`ClientStatus`:

- **Per-layer transport detail.** `TransportStatus::Timeout`,
  `WrapperStatus::InvalidFrame`, `HdlcStatus::InvalidLength`,
  and `LlcStatus::InvalidControl` all surface as
  `ReceiveFailed`. Distinguishing them at the public facade
  would leak the data-link choice (HDLC vs. WRAPPER vs.
  future profiles) into application code; profile selection
  is supposed to be transparent to facade consumers. Tools
  that need the underlying status should subscribe to the
  trace sinks (`IWrapperTcpTraceSink`, `IHdlcProfileTraceSink`)
  instead of fishing for it in the return value.
- **xDLMS service-rejected reasons.** The server can reject a
  service with any of the Blue Book confirmed-service-error
  codes (initiate-error, read-error, get-status, ...). The
  facade collapses every server-side rejection to
  `ServiceRejected`. Applications that need the raw reason
  should decode the result object that the facade returns
  alongside the status (for example, `GetResult.cosemStatus`
  carries the data-access result intact).
- **Security sub-classification.** `dlms-security`
  distinguishes "bad GMAC tag" from "replay counter rejected"
  from "missing key for role". The facade folds all of them
  to `SecurityFailed` so that an attacker observing return
  codes cannot distinguish the failure mode. The traces and
  the security layer's own log API keep the detail for the
  operator.
- **COSEM access-result vs. transport-level failure.** A
  successful round-trip that ends in `data-access-result =
  read-write-denied` is reported as `ClientStatus::Ok` with a
  non-`Ok` `cosemStatus` field inside the result object. The
  facade reserves its own status for "did the round-trip
  itself succeed"; the per-attribute access result lives in
  the result object. Mixing the two layers into one enum was
  considered and rejected because callers almost always need
  to branch on them separately.
- **`InternalError` as a last resort.** After the `0.97.x`
  audit, every `ClientStatus`-returning mapper in the facade
  is `switch`-exhaustive on its source enum and has no silent
  `default` arm. `InternalError` is therefore reserved for
  invariants the facade itself owns (null internal pointers,
  reached-unreachable assertions, internal state machine
  desync). It is not a catch-all for unmapped lower-layer
  statuses; if a new lower-layer status appears, the compiler
  forces the mapper to handle it explicitly.

#### Where the mapping lives

- `lib/dlms-client/src/client/client.cpp` -
  `MapDataLinkDisconnectStatus`, `MapXdlmsStatus`,
  `MapAssociationStatus`. Exposed for testing via
  `lib/dlms-client/src/client/client_internal.hpp`
  (`internal::` namespace; not installed).
- `lib/dlms-client/src/client/client_data.cpp` -
  `MapDataStatus` for the APDU codec helpers.
- `lib/dlms-endpoint/src/endpoint/client_endpoint.cpp` -
  `MapClientStatus` lifts `ClientStatus` into
  `EndpointStatus` with the same policy applied one layer up.
- `lib/dlms-server/src/server/server_status.cpp` -
  `MapCosemStatus` lifts COSEM access results into
  `ServerStatus` symmetrically.

If you add a new public facade status, update both this
section and the relevant mapper, and add a test in
`lib/dlms-client/test/client/test_client_internal.cpp` that
asserts the new mapping. Do not bypass the mapper by
returning a lower-layer status directly from the facade.

## 2. Options

```cpp
enum class ClientProfile
{
  WrapperTcp,
  HdlcTcp
};

enum class ClientSecurityMode
{
  None,
  AuthenticatedAndEncrypted
};

enum class ClientAuthenticationMode
{
  None,
  LowLevelSecurity,
  HighLevelSecurity,
  HighLevelSecurityGmac
};

struct ClientLowLevelSecurityOptions
{
  const std::uint8_t* credential;
  std::size_t credentialSize;
};

struct ClientHighLevelSecurityOptions
{
  const std::uint8_t* password;
  std::size_t passwordSize;
};

struct ClientSecurityOptions
{
  std::uint8_t clientSystemTitle[8];
  std::uint8_t serverSystemTitle[8];
  std::uint8_t globalUnicastEncryptionKey[16];
  std::uint8_t authenticationKey[16];
  std::uint32_t invocationCounter;
};

struct WrapperTcpEndpoint
{
  const char* host;
  std::uint16_t port;
  std::uint16_t sourceWPort;
  std::uint16_t destinationWPort;
};

struct HdlcTcpEndpoint
{
  const char* host;
  std::uint16_t port;
  std::uint8_t clientAddress;
  std::uint16_t logicalDeviceAddress;
  std::uint16_t physicalDeviceAddress;
  std::size_t maxInfoTx;
  std::size_t maxInfoRx;
  std::uint8_t windowSizeTx;
  std::uint8_t windowSizeRx;
  std::uint8_t retryCount;
  std::uint32_t retryDelayMs;
  bool useDataLinkSession;
};

struct DlmsClientOptions
{
  ClientProfile profile;
  ClientAuthenticationMode authenticationMode;
  ClientSecurityMode securityMode;
  WrapperTcpEndpoint wrapperTcp;
  HdlcTcpEndpoint hdlcTcp;
  ClientLowLevelSecurityOptions lowLevelSecurity;
  ClientHighLevelSecurityOptions highLevelSecurity;
  ClientSecurityOptions security;
  std::uint16_t clientSap;
  std::uint16_t serverSap;
  std::uint32_t connectTimeoutMs;
  std::uint32_t requestTimeoutMs;
};
```

## 3. Data Types

The facade uses xDLMS descriptors directly to avoid duplicating protocol
identity types:

```cpp
using CosemAttributeDescriptor = dlms::xdlms::CosemAttributeDescriptor;
using CosemMethodDescriptor = dlms::xdlms::CosemMethodDescriptor;
using SelectiveAccessDescriptor = dlms::xdlms::SelectiveAccessDescriptor;
```

GET, SET, and ACTION values cross the facade boundary as complete encoded DLMS
`Data` bytes, including the type tag. Typed convenience helpers remain thin
wrappers over this encoded-data API.

`client_data.hpp` exposes helpers for common GUI-facing scalar values:

```cpp
ClientStatus EncodeDlmsBoolean(bool value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsBoolean(const std::vector<std::uint8_t>& encoded, bool& value);

ClientStatus EncodeDlmsInteger(std::int8_t value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsInteger(const std::vector<std::uint8_t>& encoded, std::int8_t& value);

ClientStatus EncodeDlmsLong(std::int16_t value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsLong(const std::vector<std::uint8_t>& encoded, std::int16_t& value);

ClientStatus EncodeDlmsDoubleLong(std::int32_t value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsDoubleLong(const std::vector<std::uint8_t>& encoded, std::int32_t& value);

ClientStatus EncodeDlmsUnsigned(std::uint8_t value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsUnsigned(const std::vector<std::uint8_t>& encoded, std::uint8_t& value);

ClientStatus EncodeDlmsLongUnsigned(std::uint16_t value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsLongUnsigned(const std::vector<std::uint8_t>& encoded, std::uint16_t& value);

ClientStatus EncodeDlmsDoubleLongUnsigned(std::uint32_t value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsDoubleLongUnsigned(const std::vector<std::uint8_t>& encoded, std::uint32_t& value);

ClientStatus EncodeDlmsEnum(std::uint8_t value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsEnum(const std::vector<std::uint8_t>& encoded, std::uint8_t& value);

ClientStatus EncodeDlmsOctetString(
  const std::vector<std::uint8_t>& value,
  std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsOctetString(
  const std::vector<std::uint8_t>& encoded,
  std::vector<std::uint8_t>& value);

struct DlmsDate
{
  std::uint16_t year;
  std::uint8_t month;
  std::uint8_t dayOfMonth;
  std::uint8_t dayOfWeek;
};

struct DlmsTime
{
  std::uint8_t hour;
  std::uint8_t minute;
  std::uint8_t second;
  std::uint8_t hundredths;
};

struct DlmsDateTime
{
  DlmsDate date;
  DlmsTime time;
  std::int16_t deviation;
  std::uint8_t clockStatus;
};

ClientStatus EncodeDlmsDateTime(const DlmsDateTime& value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsDateTime(const std::vector<std::uint8_t>& encoded, DlmsDateTime& value);

ClientStatus EncodeDlmsDate(const DlmsDate& value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsDate(const std::vector<std::uint8_t>& encoded, DlmsDate& value);

ClientStatus EncodeDlmsTime(const DlmsTime& value, std::vector<std::uint8_t>& encoded);
ClientStatus DecodeDlmsTime(const std::vector<std::uint8_t>& encoded, DlmsTime& value);
```

The helpers decode only the exact matching DLMS `Data` tag and return
`InvalidArgument` for malformed input or a different tag. They clear output
values on failure so GUI state does not retain stale data.
Date/time structs preserve DLMS wildcard and special values such as `0xFF`,
`0xFE`, `0xFD`, deviation `0x8000`, and raw clock-status bits instead of
normalizing them into host calendar types.

GUI-facing helpers can work directly with class id, OBIS logical name and
attribute/method id while still using the same encoded-data payload:

```cpp
struct ClientGetResult
{
  ClientStatus status;
  std::uint8_t invokeId;
  bool hasData;
  std::vector<std::uint8_t> encodedData;
  bool hasAccessResult;
  std::uint8_t accessResult;
};

struct ClientSetResult
{
  ClientStatus status;
  std::uint8_t invokeId;
  std::uint8_t accessResult;
};

struct ClientActionResult
{
  ClientStatus status;
  std::uint8_t invokeId;
  std::uint8_t actionResult;
  bool hasData;
  std::vector<std::uint8_t> encodedReturnParameter;
};
```

## 4. Client Service Interface

```cpp
class IClientXdlmsService
{
public:
  virtual ~IClientXdlmsService();

  virtual dlms::xdlms::XdlmsStatus Get(
    const CosemAttributeDescriptor& descriptor,
    dlms::xdlms::GetResult& result) = 0;

  virtual dlms::xdlms::XdlmsStatus Set(
    const CosemAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData,
    dlms::xdlms::SetResult& result) = 0;

  virtual dlms::xdlms::XdlmsStatus Action(
    const CosemMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    dlms::xdlms::ActionResult& result) = 0;
};
```

## 5. Client Class

```cpp
class DlmsClient
{
public:
  explicit DlmsClient(const DlmsClientOptions& options);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association,
    IClientXdlmsService& xdlms);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association,
    IClientXdlmsService& xdlms);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association,
    dlms::security::CipheredApduProcessor& security);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association,
    dlms::xdlms::IXdlmsSecurityProcessor& security);

  ClientStatus Connect();
  ClientStatus OpenAssociation();
  ClientStatus ReleaseAssociation();
  ClientStatus Close();

  ClientState State() const;
  bool IsConnected() const;
  bool IsAssociated() const;

  ClientStatus Get(
    const CosemAttributeDescriptor& descriptor,
    std::vector<std::uint8_t>& encodedData);

  ClientStatus ReadAttribute(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t attributeId,
    ClientGetResult& result);

  ClientStatus ReadAttribute(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t attributeId,
    const SelectiveAccessDescriptor& selectiveAccess,
    ClientGetResult& result);

  ClientStatus Set(
    const CosemAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData);

  ClientStatus WriteAttribute(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t attributeId,
    const std::vector<std::uint8_t>& encodedData,
    ClientSetResult& result);

  ClientStatus Action(
    const CosemMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    std::vector<std::uint8_t>& encodedReturnParameter);

  ClientStatus CallMethod(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t methodId,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    ClientActionResult& result);
};
```

## 5. Lifecycle Rules

- The options constructor owns a TCP byte stream, selected APDU channel,
  association client, xDLMS service client, and optional security
  stores/processor.
- For `ClientProfile::WrapperTcp`, `Connect()` opens the Wrapper/TCP APDU
  channel.
- For `ClientProfile::HdlcTcp`, `Connect()` opens the TCP-backed HDLC profile
  channel. When `HdlcTcpEndpoint::useDataLinkSession` is `true`, it also
  establishes the HDLC data link before returning `Ok`. When it is `false`,
  the profile uses no-session HDLC framing.
- The injected constructor receives an already constructed APDU channel and
  association client for deterministic tests or external composition. New
  integrations should depend on `dlms::association::IAssociationClient` when
  they provide their own association lifecycle implementation.
- The injected xDLMS service constructor keeps facade lifecycle management over
  the supplied APDU channel and association client, but forwards GET/SET/ACTION
  to caller-provided `IClientXdlmsService`.
- The injected security constructor also receives an already constructed
  xDLMS security processor. New integrations should depend on
  `dlms::xdlms::IXdlmsSecurityProcessor` when they provide their own
  protection/unprotection implementation; the `CipheredApduProcessor` overload
  remains a compatibility shortcut for the default security implementation.
- `Connect()` opens the APDU channel through `AssociationClient`.
- `OpenAssociation()` requires a connected channel.
- `Get()`, `Set()`, `Action()`, `ReadAttribute()`, `WriteAttribute()`, and
  `CallMethod()` require an established association.
- `ReadAttribute()`, `WriteAttribute()`, and `CallMethod()` clear their result
  structs before dispatch and preserve lower xDLMS invoke id plus
  access/action result bytes when the service response reaches the client.
- The selective-access `ReadAttribute()` overload forwards generic xDLMS
  `SelectiveAccessDescriptor` values. The descriptor contains the selector
  byte and complete encoded DLMS `Data` parameters; Profile Generic selector
  `1` and `2` parameter builders remain a higher-level helper concern.
- `ReleaseAssociation()` is idempotent when already not associated. In the
  injected-channel phase a successful release closes the lower channel through
  `AssociationClient::Release()` and returns the facade to disconnected state.
- `Close()` closes the channel and returns the client to the disconnected state.

## 6. Error Mapping

| Lower layer | Facade mapping |
|---|---|
| transport open failure | `TransportOpenFailed` |
| profile open failure | `ChannelOpenFailed` |
| association open/establish failure | `AssociationFailed` |
| xDLMS `NotAssociated` | `NotAssociated` |
| xDLMS send failure | `SendFailed` |
| xDLMS receive failure | `ReceiveFailed` |
| xDLMS service rejection | `ServiceRejected` |
| xDLMS security failure | `SecurityFailed` |
| unsupported lower-layer feature | `UnsupportedFeature` |

## 7. Security Option Rules

`ClientAuthenticationMode::None` keeps the default lowest-level association
authentication behavior.

`ClientAuthenticationMode::LowLevelSecurity` requires a non-null, non-empty
credential. The options-owned constructor copies those bytes into
`dlms-association` and does not transform them.

`ClientAuthenticationMode::HighLevelSecurity` requires a non-null, non-empty
password. The options-owned constructor wires a password-based HLS High
strategy into `dlms-association`. `OpenAssociation()` completes AARQ/AARE,
invokes Association LN `reply_to_HLS_authentication`, and verifies the server
response before reporting the facade as associated. This mode corresponds to
COSEM HLS mechanism id `2` and to certification profiles where `bGMAC=false`
and `HLSPassword=HiPassword`.

`ClientAuthenticationMode::HighLevelSecurityGmac` requires a valid client
system title and a 16-byte authentication key in `ClientSecurityOptions`. The
server system title may be supplied explicitly or discovered from an 8-byte
AARE responding AP title. The options-owned constructor wires an HLS GMAC
strategy into `dlms-association`. `OpenAssociation()` completes the AARQ/AARE
exchange, resolves the remote system title, invokes Association LN
`reply_to_HLS_authentication` through xDLMS ACTION, and verifies the server
response before reporting the facade as associated.

`ClientSecurityMode::None` ignores `ClientSecurityOptions`.

`ClientSecurityMode::AuthenticatedAndEncrypted` requires:

- valid non-zero client and server system titles;
- 16-byte global unicast encryption key;
- 16-byte authentication key;
- a configured invocation counter start value.

The facade maps these fields into a `dlms::security::SecurityContext`,
`InMemoryKeyStore`, `InMemoryInvocationCounterStore`, and
`CipheredApduProcessor`. It does not own persistent key storage and does not
derive LLS/HLS authentication material.
