# Security Support Matrix

## Scope

This matrix tracks DLMS/COSEM security support against СПОДЭС/СПОДУС needs.
It separates implemented framework capabilities from application-model
requirements that still need COSEM objects and conformance tests.

Status values:

- `Supported`: public API and deterministic tests exist.
- `Partial`: public API or internal implementation exists, but production
  behavior is incomplete.
- `Planned`: required by the roadmap, not implemented yet.
- `Unsupported`: intentionally not provided in the current release.

## Security Suites

| Area | Status | Notes |
| --- | --- | --- |
| Suite 0 AES-GCM-128 primitives | Supported | `Suite0AesGcm`, key validation, GMAC and ciphered APDU tests exist. |
| Suite 0 global unicast encryption key | Supported | `SecurityKeyRole::GlobalUnicastEncryption` is implemented through the key store. |
| Suite 0 authentication key | Supported | `SecurityKeyRole::Authentication` is used by GMAC and ciphering. |
| Suite 0 key encryption key | Partial | Key role exists, but key transfer and Security Setup IC methods return explicit unsupported status. |
| Suite 0 global broadcast encryption key | Partial | Key role exists, but broadcast APDU policy is not implemented. |
| Suite 0 dedicated key | Partial | Key role exists, but dedicated-key negotiation, lifetime and `ded_*` APDU use are not implemented. |
| Suite 1 | Planned | Types allow `Suite1`, but ECDSA, ECDH, certificates and suite-specific APDU handling are not implemented. |
| Suite 2 | Planned | Types allow `Suite2`, but AES-GCM-256, SHA-384, ECDSA P-384, ECDH P-384 and certificates are not implemented. |

## Authentication

| Mechanism | Status | Notes |
| --- | --- | --- |
| No security / public client | Supported | Endpoint options support unauthenticated association. |
| Low password | Supported | Association and endpoint paths cover low-password AARQ/AARE behavior. |
| High password | Supported | `HlsHighAuthenticator` and endpoint/server strategy exist. |
| HLS GMAC, mechanism 5 | Supported | `HlsGmacAuthenticator` and client/server endpoint paths exist for Suite 0. |
| HLS SHA-256, mechanism 6 | Planned | Required for deployments that use this mechanism; no implementation yet. |
| HLS ECDSA, mechanism 7 | Planned | Requires Suite 1/2 certificate and signature infrastructure. |
| `reply_to_HLS_authentication` | Partial | Client/server endpoint paths invoke Association LN method 1, but there is no full Association LN IC method object implementation. |
| `change_HLS_secret` | Planned | Association LN method is not implemented. |

## Ciphered APDU

| Area | Status | Notes |
| --- | --- | --- |
| Global ciphered APDU protect/unprotect | Supported | `CipheredApduProcessor` covers Suite 0 protected APDU path. |
| `glo_*` xDLMS APDU coverage | Partial | Endpoint integration uses ciphered get paths, but APDU support matrix is not complete for every xDLMS service. |
| Service-specific global ciphering | Planned | Needs explicit APDU coverage and tests. |
| Dedicated ciphering | Planned | Needs dedicated-key association lifetime, APDU tags and tests. |
| General ciphering | Planned | Needed for Suite 1/2 key agreement flows and general-ciphering APDU. |
| Ciphered push | Planned | Needs Push Setup model and protected notification tests. |

## Invocation Counter

| Requirement | Status | Notes |
| --- | --- | --- |
| Counter store abstraction | Supported | `IInvocationCounterStore` and in-memory implementation exist. |
| Monotonic local counter increment | Supported | Current protect paths use counter store. |
| IV construction `system_title[8] || counter[4]` | Supported | Covered by Suite 0 AES-GCM tests. |
| Reject received counter replay | Partial | Needs per-sender persistence and explicit reject tests for `counter <= last accepted`. |
| Refuse encryption at `2^32 - 1` | Supported | `InMemoryInvocationCounterStore` returns `InvocationCounterExhausted`; protected APDU and HLS GMAC response paths propagate it without emitting output. |
| Public invocation counter object `0.0.43.1.0.255`, class id `1` | Supported | `MakeInvocationCounterObject()` exposes a read-only Data object encoded as AXDR `double-long-unsigned`. |
| Counter reset on key rotation | Planned | Depends on Security Setup key transfer implementation. |

## Security Setup IC `64`

| Attribute / method | Status | Notes |
| --- | --- | --- |
| Attribute `security_policy` | Partial | `CosemSecuritySetupObject` exposes the encoded read-only value and `security_activate` enforces monotonic bitmask strengthening. Full association policy rebinding is not complete. |
| Attribute `security_suite` | Partial | `CosemSecuritySetupObject` exposes the encoded read-only value; association binding is not complete. |
| Client and server system titles | Partial | `SecurityContext` stores titles and `CosemSecuritySetupObject` exposes encoded read-only system titles. |
| Certificate array | Planned | Needed for Suite 1/2. |
| `security_activate` | Partial | Implemented for AXDR enum policy activation with monotonic strengthening; association/session rebinding and full СПОДЭС/СПОДУС policy profiles remain incomplete. |
| `global_key_transfer` / key transfer | Planned | Needs KEK wrapping, key ids and counter reset behavior. |
| `key_agreement` | Planned | Needs ECDH and general-ciphering APDU. |
| `generate_key_pair` | Planned | Needed for Suite 1/2 server key management. |
| `generate_certificate_request` | Planned | Needed for certificate lifecycle. |
| `import_certificate` / `export_certificate` / `remove_certificate` | Planned | Needed for Suite 1/2 certificate lifecycle. |

## Production Gate

The security layer is not complete for СПОДЭС/СПОДУС production use until:

1. `Security Setup` IC `64` implements policy activation, key transfer and
   certificate operations, or these operations are explicitly excluded from the
   claimed supported profile.
2. Invocation counter replay rejection and overflow refusal are deterministic
   tests, not only storage helpers.
3. Suite 0 key transfer and dedicated ciphering decisions are implemented or
   documented as unsupported.
4. Suite 1/2 support is either implemented or excluded from the claimed
   supported profile.
