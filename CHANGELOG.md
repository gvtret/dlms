# Changelog

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
