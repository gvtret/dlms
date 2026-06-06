# Architecture Consolidation Decision

## Decision

The project uses one root monorepository. Former per-layer source locations are
no longer active development locations. Component boundaries remain logical API
and dependency boundaries, not repository boundaries.

This keeps the important DLMS/COSEM separation:

- codecs stay transport-independent;
- profiles bind codecs to I/O channels;
- association, security, xDLMS, COSEM, client, server, and endpoint code keep
  their documented responsibilities;
- higher layers depend on public abstract ports instead of concrete lower-layer
  implementations where runtime replacement is expected.

## Package Direction

The physical source tree stays under `lib/dlms-*`, while the build and release
model is owned by the root repository. The installed CMake package groups
logical components into aggregate targets:

| Package | Logical components |
|---|---|
| `dlms-codec` | `dlms-hdlc`, `dlms-llc`, `dlms-wrapper`, `dlms-apdu` |
| `dlms-io` | `dlms-transport`, `dlms-profile` |
| `dlms-protocol` | `dlms-association`, `dlms-security`, `dlms-xdlms` |
| `dlms-cosem-server` | `dlms-cosem`, `dlms-server` |
| `dlms-runtime` | `dlms-client`, `dlms-endpoint`, examples and integration runtime composition |

The root CMake build exposes matching aggregate interface targets:

| CMake target | Linked component targets |
|---|---|
| `dlms_codec` / `dlms::codec` | `dlms_hdlc`, `dlms_llc`, `dlms_wrapper`, `dlms_apdu` |
| `dlms_io` / `dlms::io` | `dlms_transport`, `dlms_profile` |
| `dlms_protocol` / `dlms::protocol` | `dlms_association`, `dlms_security`, `dlms_xdlms` |
| `dlms_cosem_server` / `dlms::cosem_server` | `dlms_cosem`, `dlms_server` |
| `dlms_runtime` / `dlms::runtime` | `dlms_client`, `dlms_endpoint` |
| `dlms_framework` / `dlms::framework` | all aggregate targets above |

The root install step exports these targets as a CMake package named
`DLMSFramework`. A downstream CMake project should request the component it
uses:

```cmake
find_package(DLMSFramework REQUIRED CONFIG COMPONENTS runtime)
target_link_libraries(app PRIVATE dlms::runtime)
```

Applications that intentionally need every aggregate can request `framework`:

```cmake
find_package(DLMSFramework REQUIRED CONFIG COMPONENTS framework)
target_link_libraries(app PRIVATE dlms::framework)
```

This is a packaging plan, not an instruction to merge responsibilities. A file
move or target rename should happen only when it reduces real maintenance cost
and can be verified without changing protocol behavior.

## Migration Scope

The completed migration used a flattening approach:

- remove the legacy external-module manifest;
- replace external-module entries with the current checked-out file contents;
- do not import old external-module commit history into the root repository;
- keep old standalone sources only as archival history before the migration
  point.

## Success Criteria

- Root `git status` tracks component files as normal files.
- Default root CMake configure, build, and CTest continue to pass.
- Root `VERSION` is the single SemVer source of truth.
- Aggregate CMake package targets compile and link through a root smoke test.
- The installed `DLMSFramework` CMake package configures and links a downstream
  smoke consumer through `dlms::framework`.
- Documentation no longer tells contributors to create one source repository
  per component.
