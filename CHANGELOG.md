# Changelog

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
