# dlms-association Test Plan

## 1. Unit Tests

Required first-phase tests:

- default options match the xDLMS initiate defaults used by `dlms-apdu`
- `Open()` maps lower channel open success/failure
- `Close()` is idempotent enough to return to `Closed`
- `Establish()` sends an encoded AARQ
- successful AARE transitions to `Associated`
- rejected AARE returns `AssociationRejected`
- malformed AARE returns `DecodeFailed`
- `Establish()` from `Closed` returns `InvalidState`
- repeated `Establish()` after success returns `AlreadyAssociated`
- unsupported authentication mode returns `UnsupportedAuthentication`
- `Release()` from non-associated states returns `InvalidState`
- successful `Release()` sends RLRQ, receives RLRE, closes the channel, and
  returns to `Closed`
- malformed or non-RLRE release response returns `DecodeFailed`
- release receive failure leaves the client `Associated` so `Close()` can be
  used as fallback
- C API status values mirror the C++ status enum values
- C API default options expose the same no-authentication LN proposal
- C API callback client can open, establish, release, and close
- C API callback server can open, accept, release, and close
- C API rejects missing required callbacks and null handles
- C API public header compiles as C
- default authentication boundary has no LLS credential and no HLS strategy
- LLS without credential returns `UnsupportedAuthentication`
- LLS with credential sends ACSE authentication fields in the AARQ
- LLS AARQ preserves credential bytes exactly
- HLS without strategy returns `UnsupportedAuthentication`
- HLS strategy failures return `UnsupportedAuthentication`
- HLS unsupported mechanisms return `UnsupportedAuthentication`
- C API can carry an LLS credential pointer/size without taking ownership
- C API rejects HLS modes without callback table
- server default options mirror no-authentication LN initiate defaults
- server `Accept()` from `Closed` returns `InvalidState`
- server `Accept()` receives one encoded AARQ and sends one encoded AARE
- successful server accept transitions to `Associated` and stores negotiated
  context
- server receive failure returns `ReceiveFailed`
- malformed or non-AARQ input returns `DecodeFailed`
- AARQ authentication fields return `UnsupportedAuthentication` when the server
  is configured for no authentication
- server LLS with a matching configured credential accepts the AARQ and sends
  AARE
- server LLS with missing or mismatched configured credential returns
  `UnsupportedAuthentication`
- server HLS authentication options without a server strategy return
  `UnsupportedAuthentication` before reading the channel
- server HLS authentication accepts a valid strategy, mechanism, client
  challenge, and server challenge, then emits AARE authentication fields
- server HLS authentication rejects unknown mechanisms, empty client
  challenges, strategy failures, empty server challenges, and oversized server
  challenges
- server `Close()` returns to `Closed`

## 2. Integration Tests

Root integration shall later verify association over:

- Wrapper/TCP profile fake stream
- Wrapper/UDP profile fake datagram
- HDLC/LLC profile fake byte stream

The first phase only requires unit-level fake `IApduChannel` tests.

Authentication integration is intentionally deferred until the APDU layer
provides ACSE authentication field encoding and a security layer provides HLS
challenge algorithms.

Server association integration into endpoint listener runtimes is intentionally
deferred until the standalone server processor contract is covered by unit
tests.

## 3. Verification Commands

Use an existing build directory when one is already configured by the workspace.
The expected root checks are:

```text
cmake --build <build-dir>
ctest --test-dir <build-dir> --output-on-failure
```
