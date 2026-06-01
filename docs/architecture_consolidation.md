# Architecture Consolidation Decision

## Decision

The project is moving from one Git repository per protocol layer to one root
monorepository. Layer boundaries remain logical API and dependency boundaries,
not repository boundaries.

This keeps the important DLMS/COSEM separation:

- codecs stay transport-independent;
- profiles bind codecs to I/O channels;
- association, security, xDLMS, COSEM, client, server, and endpoint code keep
  their documented responsibilities;
- higher layers depend on public abstract ports instead of concrete lower-layer
  implementations where runtime replacement is expected.

## Package Direction

The physical source tree may stay under `lib/dlms-*` while the build and release
model consolidates around the root repository. Future packaging can group the
logical components into fewer deliverables without moving code first:

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

This is a packaging plan, not an instruction to merge responsibilities. A file
move or target rename should happen only when it reduces real maintenance cost
and can be verified without changing protocol behavior.

## Migration Scope

The initial migration uses a flattening approach:

- remove `.gitmodules`;
- replace submodule gitlink entries with the current checked-out file contents;
- do not import old submodule commit history into the root repository;
- keep old standalone repositories only as archival history before the
  migration point.

## Success Criteria

- Root `git status` tracks component files as normal files.
- Default root CMake configure, build, and CTest continue to pass.
- Root `VERSION` is the single SemVer source of truth.
- Aggregate CMake package targets compile and link through a root smoke test.
- Documentation no longer tells contributors to create one repository per
  layer.
