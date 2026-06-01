# DLMS/COSEM Versioning

## Source Of Truth

The framework uses one SemVer version for the root monorepository. The canonical
version lives in the root `VERSION` file.

CMake reads `VERSION` through `cmake/DlmsVersion.cmake`. The root project and
component projects use the same numeric `MAJOR.MINOR.PATCH` project version.
The full SemVer string may also include prerelease or build metadata, for
example `0.2.0-alpha.1` or `1.0.0+build.7`.

The installed `DLMSFramework` CMake package version is generated from the same
root SemVer source.

## SemVer Policy

Until `1.0.0`, the framework is still pre-stable. Minor releases may change
public APIs, but every breaking change must be documented in the release notes
and migration notes.

Starting with `1.0.0`:

- increment `MAJOR` for breaking public API, ABI, wire-behavior, or documented
  runtime semantic changes;
- increment `MINOR` for backward-compatible public APIs, protocol coverage, or
  behavior additions;
- increment `PATCH` for backward-compatible fixes, documentation, tests, and
  build changes.

Public compatibility covers:

- installed/public headers and C ABI headers;
- documented C++ interfaces, status codes, option structures, and lifecycle
  behavior;
- deterministic behavior promised by public examples and architecture docs;
- CMake target names and documented build options.

Internal tests, private helper classes, and implementation-only file layout are
not public API unless they are documented as extension points.

## Release Rules

Every release candidate must pass the default root CTest suite. The
`dlms_semver_check` CTest validates that `VERSION` is a valid SemVer string.
The `dlms_changelog_check` CTest validates that the current `VERSION` has a
matching `CHANGELOG.md` release entry.
The `dlms_release_tag_check` CTest is a no-op for normal branch and pull
request runs, but validates that a release tag context uses the exact
`v<version>` form that matches `VERSION`.
The package smoke tests validate that the installed `DLMSFramework` CMake
package is consumable and that the root build can produce a non-empty ZIP
package artifact.
For local release verification on Windows/MSYS2, run
`scripts/verify_release_mingw64.sh` from an MSYS2 shell. The script creates a
fresh `build-release-mingw64` tree, configures with MinGW first in `PATH`,
builds, runs CTest, and builds the package target.
The GitHub Actions CI workflow uses the same script for push, pull request,
manual verification, and `v*` tag pushes, then uploads the verified ZIP package
artifact. For `v<version>` tag pushes, CI publishes a GitHub release with the
same verified ZIP attached.
Tagged release steps are documented in `docs/release_checklist.md`.

Release changes should update:

- `VERSION`;
- `CHANGELOG.md`;
- migration notes when behavior changes;
- any public documentation affected by the compatibility scope.

The root repository is authoritative after the submodule flattening. Former
standalone layer repositories are archive sources for history before the
monorepo migration; new work should land in the root repository.
