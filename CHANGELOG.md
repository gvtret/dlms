# Changelog

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
