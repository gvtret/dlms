# 01. Security API

## 1. API Shape

The public C++ API is split into small protocol concepts:

- status and name helpers;
- security suite and policy enums;
- security context;
- key-store interface;
- invocation-counter interface;
- random-source interface;
- ciphered APDU processor;
- authentication helpers.

Phase 0 defines the contract only. Header implementation begins in Phase 1.

## 2. Status Model

Planned status enum:

```cpp
enum class SecurityStatus
{
  Ok,
  InvalidArgument,
  InvalidContext,
  UnsupportedFeature,
  MissingKey,
  InvalidKeyLength,
  InvalidSystemTitle,
  InvalidInvocationCounter,
  InvocationCounterExhausted,
  ReplayDetected,
  AuthenticationFailed,
  CipherFailed,
  DecipherFailed,
  OutputBufferTooSmall,
  InternalError
};
```

All runtime functions return `SecurityStatus`.

## 3. Security Context

`SecurityContext` describes one security association view:

```cpp
struct SecurityContext
{
  SecuritySuite suite;
  SecurityPolicy policy;
  SecurityRole role;
  std::uint16_t clientSap;
  std::uint16_t serverSap;
  std::uint8_t localSystemTitle[8];
  std::uint8_t remoteSystemTitle[8];
};
```

The context is immutable during a single APDU protect/unprotect operation.
The processor derives the Suite 0 security-control byte from `SecurityPolicy`.

## 4. Key Store

```cpp
class IKeyStore
{
public:
  virtual ~IKeyStore() {}

  virtual SecurityStatus GetKey(
    SecurityKeyRole role,
    SecurityKey& output) const = 0;
};
```

`SecurityKey` shall own fixed-size key bytes and an explicit key length.

## 5. Invocation Counters

```cpp
class IInvocationCounterStore
{
public:
  virtual ~IInvocationCounterStore() {}

  virtual SecurityStatus NextLocal(
    std::uint32_t& invocationCounter) = 0;

  virtual SecurityStatus ValidateRemote(
    std::uint32_t invocationCounter) = 0;
};
```

## 5.1 Storage, Ownership and Lifetime

This section pins the contract the layer expects from any
production implementation of `IKeyStore`, `IMutableKeyStore`
and `IInvocationCounterStore`. It also constrains what the
bundled `InMemoryInvocationCounterStore` and any future
in-memory key store are allowed to do, so that callers can
reason about loss of secrets across process restarts and
about replay safety across power cuts.

### Ownership

- The caller (typically the endpoint or application wiring)
  owns every store instance. `dlms-security` never
  `new`s or `delete`s a store, never wraps it in a smart
  pointer, and never copies the bytes the store returns into
  any longer-lived global.
- `CipheredApduProcessor` and `HlsGmacAuthenticator` hold a
  `const IKeyStore&` and a non-const
  `IInvocationCounterStore&` for their entire lifetime. The
  store **must outlive** every processor / authenticator
  that was constructed with it.
- Stores are not transferred across threads by the security
  layer. Cross-thread sharing, if any, is the caller's
  responsibility and must be backed by the thread-safety
  guarantee declared below.

### Lifetime

- A store may be created at process startup and reused for
  the full process lifetime. Nothing in the security layer
  forces a per-association store.
- A store may also be created per-association if the caller
  wants strict isolation between associations; the layer
  does not depend on identity comparison of store pointers.
- When the caller tears down an association, the store may
  outlive it. The security layer issues no "final"
  notification to the store at association release; the
  store is therefore free to keep buffered counters / keys.
- During the destructor of any processor / authenticator,
  the store reference is read at most once for a graceful
  shutdown and never after the destructor returns.

### Storage

- `IKeyStore` is purely a lookup. It does not have to be
  backed by RAM. Production deployments are expected to back
  it with one of:
  - a hardware-backed key vault (TPM, HSM, secure element),
    where `GetKey` returns a wrapped handle plus key bytes
    only for the brief moment the security layer needs them;
  - an OS keyring (DPAPI, libsecret, macOS Keychain);
  - an encrypted on-disk blob unlocked at process start.
- The in-tree code provides no "in-memory key store" for
  production. A bare in-RAM key store, if one is added for
  tests, must be named so that it cannot be mistaken for a
  production component (for example,
  `TestOnlyInMemoryKeyStore`).
- `IMutableKeyStore::SetKey` is the only path through which
  the security layer mutates a key store. Any persistence,
  wrapping, audit log or replication is the implementer's
  job; the security layer treats `SetKey` as a fire-and-
  forget request and only inspects the returned
  `SecurityStatus`.

### Invocation counter persistence

- The local invocation counter `IInvocationCounterStore::
  NextLocal` returns must be **monotonically increasing
  across the entire lifetime of the client's system title /
  key combination**, including across process restarts and
  power cuts. The Blue Book treats counter reuse with the
  same key as a hard cryptographic failure; production
  stores must therefore persist the counter to non-volatile
  storage with safe-restart semantics.
- The bundled `InMemoryInvocationCounterStore` is a
  development and test helper only. It loses its state on
  destruction, so it must not be wired into a deployment
  that exchanges ciphered or HLS-GMAC traffic with a real
  meter that retains its own counter state.
- A safe persistence pattern is "reserve a window, commit
  the high-water mark before use":
  1. On startup, read the persisted high-water mark `H`.
  2. Reserve a window `[H + 1, H + W]` by writing `H + W`
     back to non-volatile storage before serving any
     `NextLocal` call.
  3. Serve `NextLocal` values out of the window.
  4. When the window is exhausted, reserve the next window
     before issuing more counters.
  5. On graceful shutdown, persist the actual last issued
     value so the next window is tight.
  This wastes at most `W - 1` counters on a crash and never
  reissues a counter that was already handed out.
- The remote high-water mark, if tracked per system title
  (`ValidateRemoteForSystemTitle`), should be persisted on
  the same schedule. Without persistence, a freshly
  restarted client cannot reject replays of frames it had
  already seen before the restart.

### Reset semantics

- `IInvocationCounterResetPolicy::ResetAfterKeyRotation` is
  the only sanctioned way to roll the local counter back to
  zero. It must be invoked as part of an atomic
  rotate-and-reset operation, where no `NextLocal` call can
  observe the new key paired with an old counter or the new
  counter paired with an old key.
- The in-tree global-key-transfer path in `simple_objects`
  satisfies the requirement by calling
  `ResetAfterKeyRotation` and `IMutableKeyStore::SetKey`
  back-to-back from the same synchronous COSEM action, with
  no `NextLocal` call interleaved. Either order inside that
  window is acceptable; mixing the two operations with any
  concurrent counter consumer is not.
- A reset that drops the counter without rotating the key
  violates the Blue Book replay rules and the security
  layer will not detect that misuse: it is the caller's
  responsibility to keep reset and rotation paired.

### Thread safety

- All store methods may be called from any thread, but the
  security layer does not serialise calls on the caller's
  behalf. Implementations that serve more than one
  processor / authenticator concurrently **must** be
  internally thread-safe, in particular `NextLocal` must
  be linearisable so that two threads never observe the
  same counter.
- `InMemoryInvocationCounterStore` is **not** internally
  synchronised. It is safe only when each instance is used
  by a single thread or guarded by an external mutex.

## 6. Random Source

```cpp
class IRandomSource
{
public:
  virtual ~IRandomSource() {}

  virtual SecurityStatus Fill(
    std::uint8_t* output,
    std::size_t outputSize) = 0;
};
```

The layer shall provide deterministic fake implementations for tests only.

## 7. Ciphered APDU Processor

```cpp
class CipheredApduProcessor
{
public:
  CipheredApduProcessor(
    const SecurityContext& context,
    const IKeyStore& keys,
    IInvocationCounterStore& counters);

  SecurityStatus Protect(
    SecurityByteView plainApdu,
    std::vector<std::uint8_t>& protectedApdu);

  SecurityStatus Unprotect(
    SecurityByteView protectedApdu,
    std::vector<std::uint8_t>& plainApdu);
};
```

The processor treats APDUs as opaque bytes.

The first implementation supports:

- `None` as pass-through;
- Suite 0 `AuthenticatedAndEncrypted`;
- explicit `UnsupportedFeature` for standalone `Authenticated` and
  `Encrypted` policies until their COSEM mapping is implemented.

The protected byte body owned by this layer is:

```text
security-control(1) || invocation-counter(4, big endian) ||
ciphertext(N) || authentication-tag(16)
```

xDLMS and association layers are responsible for wrapping that body in the
proper protocol APDU tag.

## 8. HLS High Authenticator

`HlsHighAuthenticator` models the password-based COSEM HLS mechanism id `2`.
It is separate from GMAC and does not use system titles, keys, or invocation
counters.

```cpp
class HlsHighAuthenticator
{
public:
  static const std::size_t kChallengeSize = 16u;

  HlsHighAuthenticator(
    SecurityByteView password,
    IRandomSource& random);

  SecurityStatus BuildChallenge(std::vector<std::uint8_t>& challenge);

  SecurityStatus BuildResponse(
    SecurityByteView challenge,
    std::vector<std::uint8_t>& response) const;

  SecurityStatus VerifyResponse(
    SecurityByteView challenge,
    SecurityByteView response) const;
};
```

`BuildChallenge` produces the client-to-server random challenge for AARQ.
`BuildResponse` applies the legacy HLS High AES-1 transform to the remote
challenge using the configured password bytes. `VerifyResponse` applies the
same transform to the local challenge and compares the server response.

## 9. HLS GMAC Authenticator

```cpp
class HlsGmacAuthenticator
{
public:
  static const std::size_t kChallengeSize = 16u;
  static const std::size_t kResponseTagSize = 16u;

  HlsGmacAuthenticator(
    const SecurityContext& context,
    const IKeyStore& keys,
    IInvocationCounterStore& counters,
    IRandomSource& random);

  SecurityStatus BuildChallenge(std::vector<std::uint8_t>& challenge);

  SecurityStatus BuildResponse(
    SecurityByteView challenge,
    std::vector<std::uint8_t>& response);

  SecurityStatus VerifyResponse(
    SecurityByteView challenge,
    SecurityByteView response);
};
```

The first response body format is:

```text
security-control(1, GMAC/authentication) || invocation-counter(4, big endian) ||
gmac-tag(16)
```

`BuildResponse` uses the local system title and `NextLocal`.
`VerifyResponse` uses the remote system title and accepts the remote invocation
counter only after the GMAC tag has been verified.

## 10. C ABI

C ABI is explicitly deferred. The first implementation shall stabilize the C++
API and test vectors before adding C wrappers.
