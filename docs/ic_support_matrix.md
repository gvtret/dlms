# COSEM IC Support Matrix

## Scope

This matrix tracks implemented COSEM interface class coverage against
DLMS/COSEM, СПОДЭС and СПОДУС needs. It is intentionally stricter than the
generic object extension point: a class is `Supported` only when a built-in
object exposes the class-specific attributes or methods and has deterministic
tests.

Status values:

- `Supported`: built-in implementation and tests exist.
- `Partial`: generic extension point or minimal object exists, but class
  semantics are incomplete.
- `Planned`: required for production coverage, not implemented yet.
- `Application-provided`: the framework can host it through `ICosemObject`, but
  there is no built-in implementation.

## Core Model

| Area | Status | Notes |
| --- | --- | --- |
| `ICosemObject` extension point | Supported | Users can implement custom IC behavior. |
| `ObjectRegistry` | Supported | Object lookup by class id and logical name exists. |
| `LogicalDevice` | Supported | Attribute/method dispatch and association view construction exist. |
| Access rights checks | Partial | Basic attribute/method modes exist; association/security-context-specific policy needs expansion. |
| Association object list | Partial | Built from registered objects; full access-rights modeling per association is incomplete. |
| OBIS catalog | Planned | No СПОДЭС/СПОДУС catalog or validation layer exists. |

## Interface Classes

| IC | Name | Status | Notes |
| --- | --- | --- | --- |
| `1` | Data | Supported | `CosemDataObject` exposes logical name and value, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`; built-in helpers cover logical device name and public invocation counter objects. |
| `3` | Register | Supported | `CosemRegisterObject` exposes value and scaler/unit, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. |
| `4` | Extended Register | Partial | `CosemExtendedRegisterObject` exposes value, scaler/unit, status and capture_time attributes, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. Value attribute access mode is caller-selected; status, scaler_unit and capture_time are read-only. Method `1` `reset` returns explicit `UnsupportedFeature` (application-defined semantics); other method ids report `MethodNotFound`. |
| `5` | Demand Register | Partial | `CosemDemandRegisterObject` exposes current_average_value, last_average_value, scaler_unit, status, capture_time, start_time_current, period and number_of_periods attributes, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. All attributes are read-only; period is encoded as DLMS Data `double-long-unsigned`, number_of_periods as `long-unsigned`. Methods `1` `reset` and `2` `next_period` return explicit `UnsupportedFeature` (application-defined semantics); other method ids report `MethodNotFound`. |
| `6` | Register Activation | Planned | Needed for tariff/register activation scenarios. |
| `7` | Profile Generic | Partial | `CosemProfileGenericObject` defaults to class version `1`, allows explicit descriptor version selection up to `MaxSupportedVersion`, exposes version `0` legacy buffer methods `3`/`4` as unsupported features, and exposes read-only profile attributes plus class-level helpers for `buffer`, `capture_objects`, `sort_object`, range descriptor and entry descriptor composite encoding/decoding. Capture execution and journal schemas remain planned. |
| `8` | Clock | Partial | `CosemClockObject` exposes class version `0` documented clock attributes with read/write support where implemented; adjust/preset/shift methods are explicit unsupported features. |
| `9` | Script Table | Planned | Needed for control actions and scripts. |
| `10` | Schedule | Planned | Needed for scheduled operations. |
| `11` | Special Days Table | Planned | Needed for tariff calendars. |
| `12` | Association SN | Application-provided | Framework currently focuses on LN referencing. |
| `15` | Association LN | Partial | Built-in object supports caller-selected versions up to `3` with version-gated object list, association status, optional security setup reference, user-list/current-user attributes, explicit unsupported methods `1`-`6`, and class-level encode/decode helpers for `object_list` and access-right structures. Full HLS method execution and association-specific policy rebinding are incomplete. |
| `17` | SAP Assignment | Supported | `CosemSapAssignmentObject` exposes class version `0` and a version-taking constructor normalized to `MaxSupportedVersion`. |
| `18` | Image Transfer | Planned | Needed for firmware/image update flows. |
| `19` | IEC Local Port Setup | Planned | Needed for local optical-port setup. |
| `20` | Activity Calendar | Planned | Needed for tariff and activity calendars. |
| `21` | Register Monitor | Planned | Needed for monitoring/control scenarios. |
| `22` | Single Action Schedule | Planned | Needed for scheduled single actions. |
| `23` | IEC HDLC Setup | Planned | Needed to expose HDLC setup object state. |
| `24` | IEC twisted pair setup | Application-provided | No built-in implementation. |
| `25` | M-Bus slave port setup | Application-provided | No built-in implementation. |
| `26` | Utility Tables | Application-provided | No built-in implementation. |
| `27` | Modem Configuration | Application-provided | No built-in implementation. |
| `28` | Auto Answer | Application-provided | No built-in implementation. |
| `29` | Auto Connect | Application-provided | No built-in implementation. |
| `30` | Data Protection | Planned | Needed for complete protected data model. |
| `40` | Push Setup | Planned | Required for initiative messages and protected push. |
| `41` | TCP-UDP Setup | Planned | Needed for network setup model. |
| `42` | IPv4 Setup | Application-provided | No built-in implementation. |
| `43` | MAC Address Setup | Application-provided | No built-in implementation. |
| `44` | PPP Setup | Application-provided | No built-in implementation. |
| `45` | GPRS Modem Setup | Application-provided | No built-in implementation. |
| `46` | SMTP Setup | Application-provided | No built-in implementation. |
| `47` | GSM Diagnostic | Application-provided | No built-in implementation. |
| `48` | IPv6 Setup | Application-provided | No built-in implementation. |
| `50`-`59` | PLC / LLC setup classes | Application-provided | No built-in implementation. |
| `61` | Register Table | Planned | Needed for broader meter data tables. |
| `62` | Compact Data | Planned | Needed for compact transfer support. |
| `63` | Status Mapping | Planned | Needed for status word mapping. |
| `64` | Security Setup | Partial | `CosemSecuritySetupObject` defaults to class version `1`, allows explicit descriptor version selection up to `MaxSupportedVersion`, gates attribute `6` and methods `3`-`8` to version `1`, and exposes logical name, encoded security policy, security suite, client/server system titles. Attribute `6` `certificates` is encoded as a DLMS Data array of `certificate_info` structures from a pluggable `ICosemCertificateStore` (empty array when no store entries exist). `security_activate` enforces monotonic policy strengthening; suite `0` `global_key_transfer` is implemented through a mutable key store. Methods `6` (`import_certificate`), `7` (`export_certificate`) and `8` (`remove_certificate`) parse the Blue Book selector structures and dispatch to the certificate store backend, returning `UnsupportedFeature` when no store is attached; methods `3`/`4`/`5` (key agreement / generate key pair / generate certificate request) remain `UnsupportedFeature` until an ECDSA/X.509 stack is wired in. |
| `65` | Parameter Monitor | Planned | Needed for parameter monitoring. |
| `67` | Sensor Manager | Application-provided | No built-in implementation. |
| `68` | Arbitrator | Application-provided | No built-in implementation. |
| `70` | Disconnect Control | Planned | Needed for remote disconnect/reconnect. |
| `71` | Limiter | Planned | Needed for active power/current/voltage limiting parameters. |
| `72`-`77` | M-Bus classes | Application-provided | No built-in implementation. |
| `80`-`85` | PRIME PLC classes | Application-provided | No built-in implementation. |

## СПОДЭС/СПОДУС Application Model

| Area | Status | Notes |
| --- | --- | --- |
| Meter categories A/B/C/D parameter lists | Planned | Need catalog definitions and conformance tests. |
| СПОДУС ИВКЭ object model | Planned | Needs separate object catalog for ИВКЭ. |
| Event code table | Planned | Required by СПОДЭС/СПОДУС event journals. |
| Status word formats | Planned | Required by СПОДЭС status model. |
| Profile `0.0.94.7.131.255` discovered meters | Planned | Profile Generic with fixed column schema. |
| Data `0.0.94.7.132.255` meter access policies | Planned | Includes security policy/key list modeling. |
| Profile `0.0.94.7.135.255` exchange status journal | Planned | Profile Generic with task status columns. |
| Profile `0.0.94.7.136.255` object correction journal | Planned | Profile Generic with correction timestamps. |
| Profile `0.0.94.7.137.255` meter parameter journal | Planned | Profile Generic with one parameter value per row. |
| Profile `0.0.94.7.140.255` event aggregation | Planned | Profile Generic with event metadata and transfer status. |
| Push notification payload model | Planned | Needed to send only actual data not yet transferred/confirmed where required. |

## Production Gate

The COSEM layer is not complete for СПОДЭС/СПОДУС production use until:

1. The IC matrix is covered by automated tests for every claimed `Supported`
   class.
2. СПОДЭС/СПОДУС OBIS catalogs and category-specific parameter lists are
   represented as data, not informal documentation.
3. Profile Generic selective access, capture objects and fixed journal schemas
   are implemented.
4. Association LN object list and access rights are generated for the active
   association/security context.
5. Unsupported ICs return explicit access/service errors through server and
   endpoint paths.

The `0.4.11` knowledge-base audit reconciled this matrix against ГОСТ Р
58940-2020 table 7.1 and the DLMS UA Blue Book class list. Built-in coverage
was limited to Data `1`, Register `3`, Association LN `15`, SAP Assignment
`17`, and partial Security Setup `64`; `0.5.0` adds partial Profile Generic
`7` scaffolding.
