# Status-to-string contract

Status: stable. Last revised: VERSION 0.99.2 (P1 «Диагностика» §6).

## Summary

Every public status enum in `dlms::*` exposes a free function that maps the
enum value to a `static`-storage C string equal to the enum-value
identifier (`"Ok"`, `"NeedMoreData"`, `"InvalidArgument"`, ...).

Use these helpers for diagnostics, logs, error propagation, and test
assertions. **Do not** parse the result — it is not a wire format and it
is not a localizable label; it is a stable mirror of the C++ enumerator.

## Catalogue

| Enum                | Helper                              | Header                                                              |
| ------------------- | ----------------------------------- | ------------------------------------------------------------------- |
| `ApduStatus`        | `apdu::ApduStatusName(s)`           | `dlms/apdu/apdu_error.hpp`                                          |
| `AssociationStatus` | `association::AssociationStatusName(s)` | `dlms/association/association_status.hpp`                        |
| `ClientStatus`      | `client::ClientStatusName(s)`       | `dlms/client/client_status.hpp`                                     |
| `CosemStatus`       | `cosem::CosemStatusName(s)`         | `dlms/cosem/cosem_status.hpp`                                       |
| `EndpointStatus`    | `endpoint::ToString(s)` *           | `dlms/endpoint/endpoint_status.hpp`                                 |
| `HdlcStatus`        | `hdlc::HdlcStatusName(s)`           | `dlms/hdlc/hdlc_error.hpp`                                          |
| `LlcStatus`         | `llc::LlcStatusName(s)`             | `dlms/llc/llc_error.hpp`                                            |
| `ProfileStatus`     | `profile::ProfileStatusName(s)`     | `dlms/profile/profile_types.hpp`                                    |
| `SecurityStatus`    | `security::SecurityStatusName(s)`   | `dlms/security/security_status.hpp`                                 |
| `ServerStatus`      | `server::ServerStatusName(s)`       | `dlms/server/server_status.hpp`                                     |
| `TransportStatus`   | `transport::ToString(s)` *          | `dlms/transport/transport_status.hpp`                               |
| `WrapperStatus`     | `wrapper::WrapperStatusName(s)`     | `dlms/wrapper/wrapper_error.hpp`                                    |
| `XdlmsStatus`       | `xdlms::XdlmsStatusName(s)`         | `dlms/xdlms/xdlms_status.hpp`                                       |

`*` `EndpointStatus` and `TransportStatus` use the older `ToString` name
for historical reasons. Unifying on `*StatusName` is a tracked breaking
change (P1 «Диагностика» §5); both names will continue to work in the
meantime, and the next major bump will introduce alias-and-deprecate.

## Contract

For every helper above:

1. **Total**: every defined enumerator returns a non-null, non-empty
   C string equal to the enumerator identifier (no prefix, no namespace,
   no whitespace).
2. **Out-of-range**: when called with a `static_cast` value outside the
   defined enumerators, the helper returns the literal `"Unknown"`.
   It never crashes, asserts, or invokes UB; the underlying switch has
   no `default:` arm so adding a new enumerator without wiring its
   string trips `-Wswitch` at the call site.
3. **Lifetime**: the returned pointer has `static` storage duration and
   is safe to compare with `==` against another result from the same
   helper, but consumers should not rely on uniqueness across helpers
   (different enums may share string literals).
4. **Thread-safe**: helpers are pure and reentrant.
5. **No allocation**: helpers never allocate. Safe to call from signal
   handlers, fatal-error paths, and embedded code paths.
6. **ABI-stable**: helper signatures are part of the public C++ API and
   will not change shape across minor versions. Strings are stable
   across minor versions; an enumerator rename is a breaking change
   and will bump the major (or, pre-1.0, the minor).

## Test coverage

Every helper above is exercised by a `*StatusName.NameCoversEveryEnumValue`
test that asserts the string for each enumerator, plus a
`*StatusName.NameReturnsUnknownForInvalidValue` test that pins the
`"Unknown"` fallback. Adding a new enumerator without extending the test
breaks the build, not just the test run.

Locations:

- `lib/dlms-apdu/test/apdu/test_apdu_error.cpp`
- `lib/dlms-association/test/association/test_association_status.cpp`
- `lib/dlms-client/test/client/test_client_status.cpp`
- `lib/dlms-cosem/test/cosem/test_cosem_status.cpp`
- `lib/dlms-endpoint/test/endpoint/test_endpoint_status.cpp`
- `lib/dlms-hdlc/test/hdlc/test_hdlc_status.cpp`
- `lib/dlms-llc/test/llc/test_llc_status.cpp`
- `lib/dlms-profile/test/profile/test_profile_status.cpp`
- `lib/dlms-security/test/security/test_security_status.cpp`
- `lib/dlms-server/test/server/test_server_status.cpp`
- `lib/dlms-transport/test/transport/test_transport_status.cpp`
- `lib/dlms-wrapper/test/wrapper/test_wrapper_status.cpp`
- `lib/dlms-xdlms/test/xdlms/test_xdlms_status.cpp`

## See also

- `docs/trace_contracts.md` — every status above can be passed verbatim
  to a trace sink without leaking secrets.
- Per-module `01_*_api.md` files reference this document under
  "Diagnostic helpers".
