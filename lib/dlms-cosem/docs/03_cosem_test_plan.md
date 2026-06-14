# dlms-cosem Test Plan

## 1. Unit Tests

Phase 1 type and registry tests:

- default logical names are zeroed;
- object descriptors compare by class id and logical name;
- invalid descriptors are rejected;
- registry accepts unique object keys;
- registry rejects duplicate object keys;
- registry returns null or `ObjectNotFound` for missing objects;
- access-right helpers distinguish read, write, method, and authenticated
  modes.

Phase 2 object access tests:

- read attribute calls the object only when read access is granted;
- write attribute calls the object only when write access is granted;
- invoke method calls the object only when method access is granted;
- unsupported attributes return `AttributeNotFound`;
- unsupported methods return `MethodNotFound`;
- denied access returns `AccessDenied`;
- object callback errors return `ObjectError`;
- output buffers keep caller-owned data intact on failure.

Phase 3 logical-device tests:

- logical device stores SAP and registry;
- association view includes registered object descriptors and access rights;
- management device can expose SAP assignment metadata;
- association LN metadata can expose visible object list data.

Phase 5 simple interface object tests:

- Data object descriptor uses class id `1`, version `0`, and the configured
  logical name;
- Data explicit version constructor normalizes values above
  `MaxSupportedVersion`;
- Data attribute `1` returns encoded logical-name bytes;
- Data attribute `2` returns and writes the stored encoded value according to
  access rights;
- Register object descriptor uses class id `3`, version `0`, and the
  configured logical name;
- Register explicit version constructor normalizes values above
  `MaxSupportedVersion`;
- Register attribute `2` returns and writes the stored encoded value;
- Register attribute `3` returns the stored scaler-unit bytes;
- unsupported attributes return `AttributeNotFound`;
- methods return `MethodNotFound`;
- invalid logical names are rejected by existing registry descriptor validation.

Phase 7 association and SAP object tests:

- Association LN descriptor uses class id `15`, version `0`, and OBIS
  `0.0.40.0.0.255` for the helper default;
- Association LN descriptor version follows the caller-selected version up to
  `MaxSupportedVersion`;
- Association LN attribute `1` returns encoded logical-name bytes;
- Association LN attribute `2` returns an encoded array of visible objects;
- Association LN attribute `8` returns encoded `association_status` enum;
- Association LN version `1` constructor exposes attribute `9` as encoded
  `security_setup_reference` logical-name bytes;
- Association LN version `2+` exposes `user_list` and `current_user` as
  documented user-entry structures;
- Association LN methods `1` through `4` return `UnsupportedFeature`;
- Association LN version `2+` methods `5` and `6` return
  `UnsupportedFeature`;
- unknown Association LN methods return `MethodNotFound`;
- object-list entries include class id, version, logical name, attribute access
  descriptors, and method access descriptors;
- Association LN object-list helpers encode/decode visible objects and nested
  access rights;
- malformed Association LN object-list bytes return `InvalidArgument` and do
  not publish partial decoded output;
- SAP Assignment descriptor uses class id `17`, version `0`, and OBIS
  `0.0.41.0.0.255` for the helper default;
- SAP Assignment explicit version constructor normalizes values above
  `MaxSupportedVersion`;
- SAP Assignment attribute `2` returns an encoded array of SAP/name pairs;
- logical-device name helper returns OBIS `0.0.42.0.0.255`;
- writes are denied by registry access checks;
- unsupported SAP methods return `MethodNotFound`.

Clock object tests:

- Clock descriptor uses class id `8`, version `0`, and the configured logical
  name;
- Clock explicit version constructor normalizes values above
  `MaxSupportedVersion`;
- Clock attribute `1` returns encoded logical-name bytes;
- Clock attributes `2`, `5`, and `6` return 12-byte date-time values wrapped as
  DLMS Data octet-strings;
- Clock attributes `3`, `4`, `7`, `8`, and `9` return the documented DLMS Data
  scalar tags;
- mutable attributes update object state only after exact tag and length
  validation;
- logical name and status writes are rejected;
- malformed writes leave previous values unchanged;
- methods `1` through `6` return `UnsupportedFeature`;
- unknown methods return `MethodNotFound`.

Profile Generic composite attribute tests:

- `capture_object_definition` encodes as a four-field structure containing
  class id, logical name, attribute index, and data index;
- `capture_objects` encodes as an array of capture-object structures and
  decodes back to typed `CosemCaptureObject` entries;
- malformed capture-object arrays return `InvalidArgument` and do not publish
  partial decoded output;
- `buffer` encodes as an array of row structures;
- `buffer` decode validates each row as a DLMS Data structure and returns the
  original encoded row bytes for schema-specific application decoding;
- malformed buffer rows return `InvalidArgument` and do not publish partial
  decoded output;
- Profile Generic selector constants expose `1` for range descriptor and `2`
  for entry descriptor;
- range descriptor helpers encode/decode restricting object, boundary values,
  and selected columns;
- entry descriptor helpers encode/decode entry and selected-value ranges;
- malformed range descriptors return `InvalidArgument` and do not publish
  partial decoded output.
- Profile Generic default descriptor version is `1`;
- Profile Generic explicit version constructor can publish version `0` and
  normalizes values above `MaxSupportedVersion`;
- Profile Generic version `0` exposes legacy methods `3` and `4` as
  `UnsupportedFeature`;
- Profile Generic version `1` does not expose legacy methods `3` and `4`;

Extended Register tests:

- Extended Register exposes attribute `1` logical name, attribute `2` value,
  attribute `3` scaler_unit, attribute `4` status, attribute `5` capture_time;
- Extended Register accepts caller-selected access mode for attribute `2`
  and rejects writes to attributes `1`, `3`, `4`, `5` with `AccessDenied`;
- Extended Register method `1` `reset` reports `UnsupportedFeature` and
  clears method output; other method ids report `MethodNotFound`;
- Extended Register normalizes versions above `MaxSupportedVersion`.

Demand Register tests:

- Demand Register exposes attributes `1` logical_name, `2` current_average_value,
  `3` last_average_value, `4` scaler_unit, `5` status, `6` capture_time,
  `7` start_time_current, `8` period (encoded as DLMS Data
  `double-long-unsigned`) and `9` number_of_periods (encoded as DLMS Data
  `long-unsigned`);
- Demand Register rejects writes to every defined attribute with
  `AccessDenied` and returns `AttributeNotFound` for undefined attribute ids;
- Demand Register methods `1` `reset` and `2` `next_period` report
  `UnsupportedFeature` and clear method output; other method ids report
  `MethodNotFound`;
- Demand Register normalizes versions above `MaxSupportedVersion`.

Register Activation tests:

- Register Activation exposes attributes `1` logical_name,
  `2` register_assignment, `3` mask_list and `4` active_mask as the encoded
  DLMS Data buffers supplied by the caller, and reports `AttributeNotFound`
  for undefined attribute ids;
- Register Activation rejects writes to every defined attribute with
  `AccessDenied` and reports `AttributeNotFound` for undefined attribute ids;
- Register Activation methods `1` `add_register`, `2` `add_mask` and
  `3` `delete_mask` report `UnsupportedFeature` and clear method output;
  other method ids report `MethodNotFound`;
- Register Activation normalizes versions above `MaxSupportedVersion`.

Register Monitor tests:

- Register Monitor exposes attributes `1` logical_name, `2` thresholds,
  `3` monitored_value and `4` actions as the encoded DLMS Data buffers
  supplied by the caller, and reports `AttributeNotFound` for undefined
  attribute ids;
- Register Monitor `thresholds` writes succeed when the caller-selected
  access mode permits writes and replace the stored buffer in-place;
- Register Monitor `thresholds` writes report `AccessDenied` when the
  caller-selected access mode is read-only, leaving the stored buffer
  unchanged;
- Register Monitor rejects writes to `logical_name`, `monitored_value` and
  `actions` with `AccessDenied`, and reports `AttributeNotFound` for
  undefined attribute ids;
- Register Monitor reports `MethodNotFound` for every method id and clears
  method output;
- Register Monitor normalizes versions above `MaxSupportedVersion`.

Script Table tests:

- Script Table exposes attributes `1` logical_name and `2` scripts as the
  encoded DLMS Data buffer supplied by the caller, and reports
  `AttributeNotFound` for undefined attribute ids;
- Script Table `scripts` writes succeed when the caller-selected access
  mode permits writes and replace the stored buffer in-place;
- Script Table `scripts` writes report `AccessDenied` when the
  caller-selected access mode is read-only, leaving the stored buffer
  unchanged;
- Script Table rejects writes to `logical_name` with `AccessDenied` and
  reports `AttributeNotFound` for undefined attribute ids;
- Script Table method `1` `execute` reports `UnsupportedFeature` and
  clears method output; other method ids report `MethodNotFound`;
- Script Table normalizes versions above `MaxSupportedVersion`.

Activity Calendar tests:

- Activity Calendar exposes attributes `1` logical_name, `2`-`5` active
  calendar snapshot (calendar_name_active, season_profile_active,
  week_profile_table_active, day_profile_table_active), `6`-`9` passive
  calendar (calendar_name_passive, season_profile_passive,
  week_profile_table_passive, day_profile_table_passive) and `10`
  activate_passive_calendar_time as the encoded DLMS Data buffers
  supplied by the caller, and reports `AttributeNotFound` for undefined
  attribute ids;
- Activity Calendar passive attributes (`6`-`10`) writes succeed when the
  caller-selected access mode permits writes and replace the stored
  buffer in-place;
- Activity Calendar passive attributes (`6`-`10`) writes report
  `AccessDenied` when the caller-selected access mode is read-only,
  leaving the stored buffers unchanged;
- Activity Calendar rejects writes to `logical_name` and the active
  snapshot attributes (`2`-`5`) with `AccessDenied`, and reports
  `AttributeNotFound` for undefined attribute ids;
- Activity Calendar method `1` `activate_passive_calendar` reports
  `UnsupportedFeature` and clears method output; other method ids report
  `MethodNotFound`;
- Activity Calendar normalizes versions above `MaxSupportedVersion`.

Image Transfer tests:

- Image Transfer exposes attributes `1` logical_name, `2`
  image_block_size, `3` image_transferred_blocks_status, `4`
  image_first_not_transferred_block_number, `5` image_transfer_enabled,
  `6` image_transfer_status and `7` image_to_activate_info as the
  encoded DLMS Data buffers supplied by the caller, and reports
  `AttributeNotFound` for undefined attribute ids;
- Image Transfer `image_transfer_enabled` writes succeed when the
  caller-selected access mode permits writes and replace the stored
  buffer in-place; writes report `AccessDenied` when the access mode is
  read-only, leaving the stored buffer unchanged;
- Image Transfer rejects writes to logical_name and to the dynamic
  read-only attributes (`2`, `3`, `4`, `6`, `7`) with `AccessDenied`,
  and reports `AttributeNotFound` for undefined attribute ids;
- Image Transfer methods `1` `image_transfer_initiate`, `2`
  `image_block_transfer`, `3` `image_verify` and `4` `image_activate`
  report `UnsupportedFeature` and clear method output; other method
  ids report `MethodNotFound`;
- Image Transfer normalizes versions above `MaxSupportedVersion`.

Push Setup tests:

- Push Setup v1 exposes attributes `1` logical_name and `2`-`13`
  (push_object_list, send_destination_and_method,
  communication_window, randomisation_start_interval,
  number_of_retries, repetition_delay, port_reference,
  push_client_SAP, push_protection_parameters, push_operation_method,
  confirmation_parameters, last_confirmation_date_time) as the encoded
  DLMS Data buffers supplied by the caller, and reports
  `AttributeNotFound` for undefined attribute ids;
- Push Setup v0 reports `AttributeNotFound` on both read and write for
  the v1-only attributes (`8`-`13`), and still exposes `1`-`7` from the
  caller-supplied buffers;
- Push Setup mutable attributes (`2`-`12` on v1) writes succeed when
  the caller-selected access mode permits writes and replace the
  stored buffer in-place; writes report `AccessDenied` when the access
  mode is read-only, leaving the stored buffers unchanged;
- Push Setup rejects writes to `logical_name` and to
  `last_confirmation_date_time` (`13`) with `AccessDenied`, and reports
  `AttributeNotFound` for undefined attribute ids;
- Push Setup method `1` `push` reports `UnsupportedFeature` and clears
  method output; other method ids report `MethodNotFound`;
- Push Setup normalizes versions above `MaxSupportedVersion`.

Disconnect Control tests:

- Disconnect Control exposes attributes `1` logical_name, `2`
  output_state, `3` control_state and `4` control_mode as the encoded
  DLMS Data buffers supplied by the caller, and reports
  `AttributeNotFound` for undefined attribute ids;
- Disconnect Control `control_mode` writes succeed when the
  caller-selected access mode permits writes and replace the stored
  buffer in-place; writes report `AccessDenied` when the access mode is
  read-only, leaving the stored buffer unchanged;
- Disconnect Control rejects writes to logical_name, output_state and
  control_state (`1`, `2`, `3`) with `AccessDenied`, and reports
  `AttributeNotFound` for undefined attribute ids;
- Disconnect Control methods `1` `remote_disconnect` and `2`
  `remote_reconnect` report `UnsupportedFeature` and clear method
  output; other method ids report `MethodNotFound`;
- Disconnect Control normalizes versions above `MaxSupportedVersion`.

Limiter tests:

- Limiter exposes attributes `1` logical_name and `2`-`11`
  (monitored_value, threshold_active, threshold_normal,
  threshold_emergency, min_over_threshold_duration,
  min_under_threshold_duration, emergency_profile,
  emergency_profile_group_id_list, emergency_profile_active, actions)
  as the encoded DLMS Data buffers supplied by the caller, and reports
  `AttributeNotFound` for undefined attribute ids;
- Limiter mutable attributes (`3`-`11`) writes succeed when the
  caller-selected access mode permits writes and replace the stored
  buffer in-place; writes report `AccessDenied` when the access mode is
  read-only, leaving the stored buffers unchanged;
- Limiter rejects writes to logical_name and monitored_value (`1`, `2`)
  with `AccessDenied`, and reports `AttributeNotFound` for undefined
  attribute ids;
- Limiter `InvokeMethod` reports `MethodNotFound` for all method ids
  and clears method output (IC v0 defines no methods);
- Limiter normalizes versions above `MaxSupportedVersion`.

Security Setup tests:

- Security Setup default descriptor version is
  `CosemSecuritySetupObject::MaxSupportedVersion`;
- Security Setup explicit version constructor can publish version `0` and
  normalizes values above `MaxSupportedVersion`;
- Security Setup version `1` exposes attribute `6`, `certificates`, as an
  encoded DLMS Data array;
- Security Setup version `0` does not expose attribute `6`;
- Security Setup version `0` exposes methods `1` and `2` only;
- Security Setup version `1` exposes methods `1` through `8`;
- Security Setup `security_activate` enforces monotonic policy strengthening;
- Security Setup suite `0` `global_key_transfer` unwraps and installs keys
  through a mutable key store;
- unsupported Security Setup methods report `UnsupportedFeature`;
- Security Setup attribute `6` `certificates` encodes attached
  `ICosemCertificateStore` entries as a DLMS Data array of `certificate_info`
  structures (empty array when the store is empty or absent);
- Security Setup method `6` `import_certificate` stores raw octet-string
  payloads through the certificate store;
- Security Setup methods `7` `export_certificate` and `8` `remove_certificate`
  parse Blue Book `by_entity` and `by_serial` selector structures and dispatch
  to the certificate store; lookup failures map to `ObjectError`;
- Security Setup methods `6`-`8` report `UnsupportedFeature` when no
  certificate store backend is attached.

## 2. Integration Tests

Root integration is deferred until `dlms-server` exists. The first integration
test should verify a server-side normal GET path:

```text
dlms-server -> dlms-xdlms -> dlms-cosem -> ICosemObject
```

## 3. Verification Commands

Standalone:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Root:

```text
cmake -S . -B build-mingw64 -G "MinGW Makefiles"
cmake --build build-mingw64
ctest --test-dir build-mingw64 --output-on-failure
```
