# dlms-association API

## 1. Public Headers

```text
include/dlms/association/association_client.hpp
include/dlms/association/association_client_interface.hpp
include/dlms/association/association_server.hpp
include/dlms/association/association_server_interface.hpp
include/dlms/association/association_c_api.h
include/dlms/association/association_status.hpp
include/dlms/association/association_types.hpp
```

Applications that implement custom association lifecycles should include the
interface headers. The concrete `association_client.hpp` and
`association_server.hpp` headers include those interfaces and provide the
default ACSE-backed implementations.

## 2. Status

`AssociationStatus` is the stable C++ status contract:

- `Ok`
- `InvalidArgument`
- `InvalidState`
- `AlreadyAssociated`
- `UnsupportedAuthentication`
- `ChannelOpenFailed`
- `ChannelCloseFailed`
- `SendFailed`
- `ReceiveFailed`
- `EncodeFailed`
- `DecodeFailed`
- `AssociationRejected`
- `NegotiationFailed`
- `InternalError`

## 3. Options

`AssociationOptions` contains:

- `applicationContext`
- `authenticationMode`
- `lowLevelSecurityCredential`
- `highLevelSecurity`
- `proposedDlmsVersionNumber`
- `proposedConformance`
- `clientMaxReceivePduSize`

`DefaultAssociationOptions()` returns a no-authentication LN association
proposal using `dlms-apdu` default xDLMS initiate settings.

`AssociationServerOptions` contains the server-side response capabilities for
the initial bounded server processor:

- `applicationContext`
- `authenticationMode`
- `lowLevelSecurityCredential`
- `negotiatedDlmsVersionNumber`
- `negotiatedConformance`
- `serverMaxReceivePduSize`
- `vaaName`

`DefaultAssociationServerOptions()` returns a no-authentication LN response
using the same xDLMS version/conformance/PDU defaults as the client proposal
and VAA name `0x0007`.

## 4. Result

`AssociationResult` contains:

- `negotiatedDlmsVersionNumber`
- `negotiatedConformance`
- `serverMaxReceivePduSize`
- `vaaName`
- `aareResult`
- `aareDiagnostic`
- `highLevelSecurityServerChallenge`
- `respondingApplicationTitle`

## 5. Client

```cpp
class IAssociationClient
{
public:
  virtual ~IAssociationClient();

  virtual AssociationStatus Open() = 0;
  virtual AssociationStatus Close() = 0;
  virtual AssociationStatus Establish() = 0;
  virtual AssociationStatus Release() = 0;

  virtual AssociationState State() const = 0;
  virtual bool IsAssociated() const = 0;
  virtual const AssociationResult& Result() const = 0;
};

dlms::association::AssociationClient client(channel, options);

client.Open();
client.Establish();
client.Release();

const dlms::association::AssociationResult& result =
  client.Result();

client.Close();
```

`Release()` performs the confirmed association release exchange. It sends RLRQ,
expects RLRE, and closes the lower APDU channel on success. If sending,
receiving, or decoding RLRE fails, the client remains associated so `Close()`
can still be used as an unconfirmed fallback.

`AssociationClient` does not own the lower channel object. The caller must keep
the channel alive for the lifetime of the association client.

`AssociationClient` implements `IAssociationClient`. Higher layers should
include `association_client_interface.hpp` and depend on the interface when
they only need lifecycle/state/result behavior and do not need to construct the
default ACSE association implementation.

## 6. Server

```cpp
class IAssociationServer
{
public:
  virtual ~IAssociationServer();

  virtual AssociationStatus Open() = 0;
  virtual AssociationStatus Close() = 0;
  virtual AssociationStatus Accept() = 0;
  virtual AssociationStatus Release() = 0;
  virtual AssociationStatus Release(
    const std::vector<std::uint8_t>& rlrq) = 0;

  virtual AssociationState State() const = 0;
  virtual bool IsAssociated() const = 0;
  virtual const AssociationResult& Result() const = 0;
};

dlms::association::AssociationServer server(channel, options);

server.Open();
server.Accept();
server.Release();
server.Release(alreadyReceivedRlrq);

const dlms::association::AssociationResult& result =
  server.Result();

server.Close();
```

`Accept()` receives one AARQ from the lower APDU channel, decodes it through
`dlms-apdu`, sends one AARE, and stores the negotiated context in
`AssociationResult`.

`Release()` receives one RLRQ from the lower APDU channel, decodes it through
`dlms-apdu`, sends one RLRE, closes the lower APDU channel on success, clears
the negotiated context, and returns to `Closed`. Receive, decode, send, or
close failures leave the server associated so the caller can still use
`Close()` as an unconfirmed fallback.

The overload taking an already received RLRQ performs the same decode, RLRE
send, close, and state transition without reading another APDU. It is intended
for endpoint runtimes that receive the next APDU before dispatching it.

The initial server contract is deliberately narrow:

- logical-name, no-ciphering application context only;
- no authentication or configured Low Password authentication only;
- one caller-controlled accept operation, no background loop;
- no COSEM or endpoint dependency.

LLS AARQ authentication fields are accepted only when
`AssociationServerOptions::authenticationMode` is `LowLevelSecurity`, the
server option carries a non-empty `lowLevelSecurityCredential`, the AARQ uses
the low-level-security mechanism name, and the calling-authentication-value
matches the configured credential exactly. Missing or mismatched LLS
credentials return `UnsupportedAuthentication`.

HLS AARQ authentication fields are accepted only when
`AssociationServerOptions::authenticationMode` is `HighLevelSecurity` and the
server option carries a non-null `highLevelSecurity` server strategy. The
strategy validates the client challenge and supplies the AARE server challenge.
Pass-3/pass-4 execution remains outside `dlms-association`.

`AssociationServer` does not own the lower channel object. The caller must keep
the channel alive for the lifetime of the association server.

`AssociationServer` implements `IAssociationServer`. Endpoint and other
runtime composition layers should include `association_server_interface.hpp`
and depend on the interface when they only need server-side association
lifecycle/state/result behavior.

## 7. C API

The C ABI mirrors the C++ client and server lifecycles through opaque
`dlms_association_client_t` and `dlms_association_server_t` handles:

```c
dlms_association_client_t* client =
  dlms_association_create_client_from_callbacks(&callbacks, &options);

dlms_association_open(client);
dlms_association_establish(client);
dlms_association_release(client);
dlms_association_close(client);
dlms_association_destroy_client(client);

dlms_association_server_t* server =
  dlms_association_create_server_from_callbacks(&callbacks, &options);

dlms_association_server_open(server);
dlms_association_accept(server);
dlms_association_server_release(server);
dlms_association_server_close(server);
dlms_association_destroy_server(server);
```

The C API does not own a `dlms-profile` channel from another C ABI. It owns a
small callback-based APDU channel adapter supplied by the caller. Callback
return values are association statuses and are mapped to profile-channel
statuses internally.

Use `dlms_association_default_options()` before changing individual fields.
Use `dlms_association_get_result()` after a successful establish to copy the
negotiated context into a caller-owned result struct.
Use `dlms_association_server_get_result()` after a successful accept to copy
the server-side negotiated context.

## 8. Authentication Boundary

Phase 4 extends the options model with explicit authentication inputs while
leaving ACSE authentication encoding and HLS cryptography outside this repo.

LLS is represented by a caller-owned credential byte vector. Establishing an
LLS association without a credential returns `UnsupportedAuthentication`.
Establishing with a credential sends the bytes exactly as supplied in the ACSE
calling-authentication-value field and proposes the COSEM low-level-security
mechanism name.

HLS is represented by a non-owning `IHighLevelSecurityStrategy` pointer. The
strategy supplies the mechanism and initial client-to-server challenge for the
AARQ boundary. Missing strategies, unsupported mechanisms, and strategy
failures return `UnsupportedAuthentication`.

The HLS AARQ boundary is limited to ACSE field exchange. The association layer
encodes sender ACSE requirements, mechanism-name, and the client-to-server
calling-authentication-value. It also exposes the AARE server-to-client
challenge from responding-authentication-value in
`AssociationResult::highLevelSecurityServerChallenge` once that field is
decoded. It also exposes the AARE responding AP title in
`AssociationResult::respondingApplicationTitle` when the meter sends that ACSE
field. HLS GMAC clients may use an 8-byte responding AP title as the remote
system title for pass-4 authentication. The pass-3/pass-4 xDLMS ACTION
exchange remains a higher layer responsibility.

The C API exposes the same boundary through option fields:

- LLS credential pointer and size
- HLS callback table pointer
- HLS user context pointer

The C API does not take ownership of credential memory or callback state.

## Diagnostic helpers

This module exposes `association::AssociationStatusName(s)` for mapping its status enum to a stable,
static-storage C string (`"Ok"`, `"InvalidArgument"`, ...). The full
contract — totality, `"Unknown"` fallback, lifetime, thread-safety,
no-allocation, ABI stability — is documented once in
[`docs/status_to_string_contract.md`](../../../docs/status_to_string_contract.md).

Use the helper for logs, error propagation, and test assertions. Do not
parse the result; it is not a wire format.
