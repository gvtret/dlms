# Release Checklist

Use this checklist for every tagged framework release.

## Before Tagging

1. Choose the next SemVer according to `docs/versioning.md`.
2. Update `VERSION`.
3. Add a matching `CHANGELOG.md` entry.
4. Update migration notes or public documentation when behavior changes.
5. Run local clean verification from an MSYS2 MinGW shell:

```sh
scripts/verify_release_mingw64.sh
```

The script must pass configure, build, CTest, and package creation. The expected
package path is:

```text
build-release-mingw64/DLMSFramework-<version>.zip
```

The default CTest suite includes package smoke checks that must pass before
tagging:

- `dlms_package_install_smoke` installs the package, audits exported CMake
  target metadata, verifies the documented components
  (`codec`, `io`, `protocol`, `cosem_server`, `runtime`, `framework`), and
  builds the install-tree consumer examples;
- `dlms_package_artifact_smoke` builds the ZIP, requires release metadata under
  `share/doc/DLMSFramework`, and rejects bundled GoogleTest/GMock entries.

Consumer-facing CMake examples live under `examples/package-consumers` and must
continue to configure against the installed package, not the source tree.

## Tagging

Create release tags from `master` only after the release commit is pushed and
the CI workflow has passed.

Use a `v`-prefixed SemVer tag that exactly matches `VERSION`:

```sh
git tag -a v<version> -m "DLMSFramework <version>"
git push origin v<version>
```

Do not create or move a release tag when `VERSION`, `CHANGELOG.md`, and the
verified package artifact disagree.

When CI runs from a tag push, `dlms_release_tag_check` validates that the tag
name exactly matches `v<version>` from `VERSION`.

## CI Artifact

The CI workflow runs the same clean verification script and uploads the
generated `DLMSFramework-<version>.zip` file as `DLMSFramework-package`.
For `v<version>` tag pushes, CI also publishes a GitHub release and attaches the
same verified ZIP package as the release asset.
