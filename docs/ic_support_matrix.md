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
| `6` | Register Activation | Partial | `CosemRegisterActivationObject` exposes register_assignment, mask_list and active_mask attributes as opaque encoded DLMS Data buffers, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. All attributes are read-only; mutation is owned by the caller-supplied snapshot. Methods `1` `add_register`, `2` `add_mask` and `3` `delete_mask` return explicit `UnsupportedFeature` (application-defined semantics); other method ids report `MethodNotFound`. |
| `7` | Profile Generic | Partial | `CosemProfileGenericObject` defaults to class version `1`, allows explicit descriptor version selection up to `MaxSupportedVersion`, exposes version `0` legacy buffer methods `3`/`4` as unsupported features, and exposes read-only profile attributes plus class-level helpers for `buffer`, `capture_objects`, `sort_object`, range descriptor and entry descriptor composite encoding/decoding. Capture execution and journal schemas remain planned. |
| `8` | Clock | Partial | `CosemClockObject` exposes class version `0` documented clock attributes with read/write support where implemented; adjust/preset/shift methods are explicit unsupported features. |
| `9` | Script Table | Partial | `CosemScriptTableObject` exposes the scripts attribute as an opaque encoded DLMS Data buffer prepared by the caller, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. `scripts` access mode is caller-selected. Method `1` `execute` is surfaced as `UnsupportedFeature` (application-defined script semantics); other method ids report `MethodNotFound`. |
| `10` | Schedule | Partial | `CosemScheduleObject` exposes `entries` (array of Schedule_table_entry) as an opaque encoded DLMS Data buffer prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; entries (`2`) honor a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). A setter exposes backend-driven refresh of entries regardless of access mode. Methods `1` `insert` and `2` `delete` dispatch application-defined schedule-entry mutation and are surfaced as `UnsupportedFeature`; other method ids report `MethodNotFound`. |
| `11` | Special Days Table | Partial | `CosemSpecialDaysTableObject` exposes `entries` (array of special_day_entry) as an opaque encoded DLMS Data buffer prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; entries (`2`) honor a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). A setter exposes backend-driven refresh of entries regardless of access mode. Methods `1` `insert` and `2` `delete` dispatch application-defined special-day entry mutation and are surfaced as `UnsupportedFeature`; other method ids report `MethodNotFound`. |
| `12` | Association SN | Application-provided | Framework currently focuses on LN referencing. |
| `15` | Association LN | Partial | Built-in object supports caller-selected versions up to `3` with version-gated object list, association status, optional security setup reference, user-list/current-user attributes, explicit unsupported methods `1`-`6`, and class-level encode/decode helpers for `object_list` and access-right structures. Full HLS method execution and association-specific policy rebinding are incomplete. |
| `17` | SAP Assignment | Supported | `CosemSapAssignmentObject` exposes class version `0` and a version-taking constructor normalized to `MaxSupportedVersion`. |
| `18` | Image Transfer | Partial | `CosemImageTransferObject` exposes image_block_size, image_transferred_blocks_status, image_first_not_transferred_block_number, image_transfer_enabled, image_transfer_status and image_to_activate_info attributes as opaque encoded DLMS Data buffers prepared by the caller, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. `image_transfer_enabled` (attr 5) access mode is caller-selected; logical_name and all other attributes are read-only. Methods `1` `image_transfer_initiate`, `2` `image_block_transfer`, `3` `image_verify` and `4` `image_activate` are surfaced as `UnsupportedFeature` (application-defined firmware transfer/storage); other method ids report `MethodNotFound`. |
| `19` | IEC Local Port Setup | Planned | Needed for local optical-port setup. |
| `20` | Activity Calendar | Partial | `CosemActivityCalendarObject` exposes calendar_name_active, season_profile_active, week_profile_table_active, day_profile_table_active, calendar_name_passive, season_profile_passive, week_profile_table_passive, day_profile_table_passive and activate_passive_calendar_time attributes as opaque encoded DLMS Data buffers, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. The passive attributes (6-9) and activate_passive_calendar_time (10) share a caller-selected access mode; the active attributes (2-5) and logical_name are read-only. Method `1` `activate_passive_calendar` is surfaced as `UnsupportedFeature` (application-defined activation policy); other method ids report `MethodNotFound`. |
| `21` | Register Monitor | Partial | `CosemRegisterMonitorObject` exposes thresholds, monitored_value and actions attributes as opaque encoded DLMS Data buffers, class version `0`, and a version-taking constructor normalized to `MaxSupportedVersion`. `thresholds` access mode is caller-selected; `monitored_value` and `actions` are read-only. Register Monitor v0 defines no methods, so every method id returns `MethodNotFound`. |
| `22` | Single Action Schedule | Partial | `CosemSingleActionScheduleObject` exposes executed_script (structure), type (enum) and execution_time (array of structures) as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; attributes `2`-`4` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `23` | IEC HDLC Setup | Partial | `CosemIecHdlcSetupObject` exposes comm_speed, window_size_transmit, window_size_receive, max_info_field_length_transmit, max_info_field_length_receive, inter_octet_time_out and inactivity_time_out as opaque encoded DLMS Data buffers prepared by the caller, with a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted), and device_address as an opaque encoded DLMS Data buffer that is read-only with a setter for backend-driven refresh of the assigned HDLC address. Class version `1`, constructor normalization to `MaxSupportedVersion`. IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `24` | IEC twisted pair setup | Application-provided | No built-in implementation. |
| `25` | M-Bus slave port setup | Application-provided | No built-in implementation. |
| `26` | Utility Tables | Application-provided | No built-in implementation. |
| `27` | Modem Configuration | Partial | `CosemModemConfigurationObject` exposes communication_speed (enum), initialisation_strings (array of structure) and modem_profile (array of octet-string) as opaque encoded DLMS Data buffers prepared by the caller. Class version `1`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; attributes `2`-`4` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `28` | Auto Answer | Partial | `CosemAutoAnswerObject` exposes mode (enum), listening_window (array of structure), status (enum, RO with setter for backend-driven refresh), number_of_calls (unsigned) and number_of_rings (structure of in/out-of-window ring counts) as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) and status (`4`) are read-only; attributes `2`, `3`, `5` and `6` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `29` | Auto Connect | Partial | `CosemAutoConnectObject` exposes mode (enum), repetitions (unsigned), repetition_delay (long-unsigned), calling_window (array of structure) and destination_list (array of octet-string) as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; attributes `2`-`6` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `30` | Data Protection | Planned | Needed for complete protected data model. |
| `40` | Push Setup | Partial | `CosemPushSetupObject` exposes push_object_list, send_destination_and_method, communication_window, randomisation_start_interval, number_of_retries and repetition_delay (v0 surface) plus port_reference, push_client_SAP, push_protection_parameters, push_operation_method, confirmation_parameters and last_confirmation_date_time (v1 surface) as opaque encoded DLMS Data buffers prepared by the caller, with `MaxSupportedVersion = 1` and constructor normalization. The mutable attributes (2-12 on v1) share a caller-selected `AttributeAccessMode`; logical_name and last_confirmation_date_time (13) are read-only (a setter exposes backend-driven refresh of attribute 13). V0 objects report `AttributeNotFound` for attributes 8-13 on both read and write. Method `1` `push` is surfaced as `UnsupportedFeature` (application-defined push backend); other method ids report `MethodNotFound`. |
| `41` | TCP-UDP Setup | Partial | `CosemTcpUdpSetupObject` exposes tcp_udp_port, ip_reference, mss, nb_of_sim_conn and inactivity_time_out as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; attributes `2`-`6` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `42` | IPv4 Setup | Partial | `CosemIpv4SetupObject` exposes DL_reference (octet-string), IP_address (double-long-unsigned), multicast_IP_address (array of double-long-unsigned), IP_options (array of structure), subnet_mask (double-long-unsigned), gateway_IP_address (double-long-unsigned), use_DHCP_flag (boolean), primary_DNS_address (double-long-unsigned) and secondary_DNS_address (double-long-unsigned) as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; attributes `2`-`10` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). Methods `1` `add_mc_IP_address` and `2` `delete_mc_IP_address` return `UnsupportedFeature` (built-in object does not own multicast subscription policy); other method ids return `MethodNotFound`. |
| `43` | MAC Address Setup | Partial | `CosemMacAddressSetupObject` exposes mac_address (octet-string(6)) as an opaque encoded DLMS Data buffer prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; mac_address (`2`) honors a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `44` | PPP Setup | Partial | `CosemPppSetupObject` exposes PHY_reference (octet-string), LCP_options (array of structure), IPCP_options (array of structure) and PPP_authentication (structure of user_name/password) as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; attributes `2`-`5` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `45` | GPRS Modem Setup | Partial | `CosemGprsModemSetupObject` exposes APN (octet-string), PIN code (long-unsigned) and quality_of_service (structure) as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name (`1`) is read-only; attributes `2`-`4` share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). IC defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
| `46` | SMTP Setup | Application-provided | No built-in implementation. |
| `47` | GSM Diagnostic | Application-provided | No built-in implementation. |
| `48` | IPv6 Setup | Application-provided | No built-in implementation. |
| `50`-`59` | PLC / LLC setup classes | Application-provided | No built-in implementation. |
| `61` | Register Table | Partial | `CosemRegisterTableObject` exposes table_cell_values, single_buffer, table_cell_definition and table_entries as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name and table_cell_values (`1`, `2`) are read-only; single_buffer, table_cell_definition and table_entries (`3`-`5`) share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). A setter exposes backend-driven refresh of table_cell_values. Methods `1` `table_entry` and `2` `update_table_entry` dispatch application-defined column selection and update and are surfaced as `UnsupportedFeature`; other method ids report `MethodNotFound`. |
| `62` | Compact Data | Planned | Needed for compact transfer support. |
| `63` | Status Mapping | Planned | Needed for status word mapping. |
| `64` | Security Setup | Partial | `CosemSecuritySetupObject` defaults to class version `1`, allows explicit descriptor version selection up to `MaxSupportedVersion`, gates attribute `6` and methods `3`-`8` to version `1`, and exposes logical name, encoded security policy, security suite, client/server system titles. Attribute `6` `certificates` is encoded as a DLMS Data array of `certificate_info` structures from a pluggable `ICosemCertificateStore` (empty array when no store entries exist). `security_activate` enforces monotonic policy strengthening; suite `0` `global_key_transfer` is implemented through a mutable key store. Methods `6` (`import_certificate`), `7` (`export_certificate`) and `8` (`remove_certificate`) parse the Blue Book selector structures and dispatch to the certificate store backend, returning `UnsupportedFeature` when no store is attached; methods `3`/`4`/`5` (key agreement / generate key pair / generate certificate request) remain `UnsupportedFeature` until an ECDSA/X.509 stack is wired in. |
| `65` | Parameter Monitor | Planned | Needed for parameter monitoring. |
| `67` | Sensor Manager | Application-provided | No built-in implementation. |
| `68` | Arbitrator | Application-provided | No built-in implementation. |
| `70` | Disconnect Control | Partial | `CosemDisconnectControlObject` exposes output_state (boolean) and control_state (enum) as opaque encoded DLMS Data buffers prepared by the caller (read-only with setters for backend-driven refresh), and control_mode (enum) honoring a caller-selected `AttributeAccessMode`. Class version `0`, constructor normalization to `MaxSupportedVersion`. Methods `1` `remote_disconnect` and `2` `remote_reconnect` dispatch application-defined relay switching and state transitions and are surfaced as `UnsupportedFeature`; other method ids report `MethodNotFound`. |
| `71` | Limiter | Partial | `CosemLimiterObject` exposes monitored_value, threshold_active, threshold_normal, threshold_emergency, min_over_threshold_duration, min_under_threshold_duration, emergency_profile, emergency_profile_group_id_list, emergency_profile_active and actions as opaque encoded DLMS Data buffers prepared by the caller. Class version `0`, constructor normalization to `MaxSupportedVersion`. logical_name and monitored_value (`1`, `2`) are read-only; threshold/duration/emergency/actions attributes (`3`-`11`) share a caller-selected `AttributeAccessMode` (writes replace the stored buffer in-place when permitted). Setters expose backend-driven refresh of threshold_active and emergency_profile_active from a future limiter backend. IC v0 defines no methods; `InvokeMethod` reports `MethodNotFound` for all ids. |
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
