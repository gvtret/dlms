# Changelog

## 0.122.0 - 2026-06-22

### Breaking changes

- **`CosemDisconnectControlObject`** (`class_id=70`, `version=0`,
  IEC 62056-6-2 ED4 §4.5.8 / DLMS UA Blue Book Ed. 12.1 §4.5.8)
  now exposes all three dynamic attributes as typed values
  instead of opaque `CosemByteBuffer` payloads:
  - `output_state` (attribute `2`) → `bool`
  - `control_state` (attribute `3`) → nested enum
    `CosemDisconnectControlObject::ControlState`
    `{Disconnected=0, Connected=1, ReadyForReconnection=2}`
  - `control_mode` (attribute `4`) → nested enum
    `CosemDisconnectControlObject::ControlMode` `{Mode0..Mode6}`
  Both ctors take typed parameters; `OutputState()`,
  `GetControlState()`, `GetControlMode()` return typed values;
  `SetOutputState(bool)` and `SetControlState(ControlState)`
  expose backend-driven refresh (`output_state` and
  `control_state` remain RO via `WriteAttribute`).
  `ReadAttribute` encodes the values as AXDR `boolean` /
  `enum` on every read. `WriteAttribute(4)` parses the AXDR
  enum, validates the range `0..6` and returns
  `InvalidArgument` for malformed payloads (the stored value
  is preserved on rejection).

### Added

- Methods `1` `remote_disconnect` and `2` `remote_reconnect`
  are now **implemented** per §4.5.8.3 directly inside the
  built-in object (no longer `UnsupportedFeature` in
  configured modes):
  - `remote_disconnect`: `UnsupportedFeature` in `Mode0`;
    otherwise sets `control_state = Disconnected` and
    `output_state = false`.
  - `remote_reconnect`: `UnsupportedFeature` in `Mode0`;
    transitions to `Connected` (output `true`) in modes 2/4
    and to `ReadyForReconnection` (output `false`) in modes
    1/3/5/6. The `data` argument is tolerated (real meters
    often invoke with empty input).
- Static helpers `IsValidControlMode(raw)` and
  `IsValidControlState(raw)` for caller-side validation.
- New per-IC test file (rule P2.4)
  `lib/dlms-cosem/test/cosem/test_cosem_disconnect_control_object.cpp`
  with 16 focused tests covering:
  `DescriptorAndAccessRights`,
  `ReadAttributeEncodesTypedAxdr`,
  `OutputStateFalseEncodesAsBooleanZero`,
  `WriteControlModeParsesAndValidatesEnum`,
  `WriteControlModeRejectsMalformedAxdr`,
  `WriteControlModeReadOnlyRejected`,
  `WriteAttributeRejectsReadOnlyAttributes`,
  `RemoteDisconnectInMode0IsUnsupported`,
  `RemoteDisconnectFromConnectedInMode2` (parametric over
  modes 1..6), `RemoteReconnectInMode0IsUnsupported`,
  `RemoteReconnectInModes1356TransitionsToReady`,
  `RemoteReconnectInModes24ClosesRelay`,
  `UnknownMethodIdsReturnMethodNotFound`,
  `SettersUpdateObservableState`, `IsValidStaticHelpers`,
  `NormalizesVersionAboveMax`. Helpers are self-contained.
- Legacy IC 70 tests in `test_simple_objects.cpp` collapsed
  to a `_MovedToPerIcFile` placeholder (rule P2.4).
- `docs/ic_support_matrix.md` IC 70 row promoted to
  **Supported** with full typed-attribute and FSM coverage.

### Verified

- Full ctest: `1236/1236` pass.
- Focused
  `dlms_cosem_tests.exe --gtest_filter='CosemDisconnectControlObject.*'`:
  `17/17` pass (includes the moved-file placeholder).

## 0.121.0 - 2026-06-22

### Breaking changes

- **`CosemRegisterTableObject`** (`class_id=61`, `version=0`,
  IEC 62056-6-2 ED4 §4.3.7 / DLMS UA Blue Book Ed. 12.1 §4.3.7)
  `scaler_unit` (attribute `4`) switched from `CosemByteBuffer` to
  the typed `dlms::cosem::types::ScalerUnit` (`structure { integer
  scaler, enum unit }` per §4.3.2.2.3). Both ctors now take
  `const types::ScalerUnit&` instead of `const CosemByteBuffer&`,
  and the `ScalerUnit()` getter now returns
  `const types::ScalerUnit&`. `ReadAttribute(4)` encodes the AXDR
  wire form on every read via the shared `AppendScalerUnit`
  helper; `WriteAttribute(4)` parses the AXDR structure with
  `DecodeScalerUnit` and returns `InvalidArgument` for empty
  payloads, wrong tags, wrong field counts or trailing garbage
  (the stored value is preserved on rejection).

### Unchanged on purpose

- `table_cell_values` (attribute `2`) and `table_cell_definition`
  (attribute `3`) remain opaque `CosemByteBuffer` payloads because
  their column schema is meter-specific (it mirrors the wired
  Register / Extended Register / Demand Register entries) and a
  proper typed representation requires the discriminated-union
  `value` infrastructure that is still pending on IC 3/4/5.
  `table_cell_values` stays RO (refreshed via
  `SetTableCellValues`); `table_cell_definition` honors the
  caller-selected `AttributeAccessMode`.
- Methods `1` `reset` and `2` `capture` continue to return
  `UnsupportedFeature` (captured-payload lifecycle is owned by
  the backend); other method ids return `MethodNotFound`.

### Added

- New per-IC test file (rule P2.4)
  `lib/dlms-cosem/test/cosem/test_cosem_register_table_object.cpp`
  with 8 focused tests covering:
  `ExposesAllAttributesWithTypedScalerUnit`,
  `WriteScalerUnitParsesAxdrStructure`,
  `WriteScalerUnitRejectsMalformedInput`,
  `WriteTableCellDefinitionAcceptedReadOnlyRejected`,
  `WriteAttributeRejectsLogicalNameAndValues`,
  `SetTableCellValuesUpdatesReadResult`,
  `MethodsReturnUnsupportedFeatureOrNotFound`,
  `NormalizesVersionAboveMax`. Helpers are self-contained.
- Legacy IC 61 tests in `test_simple_objects.cpp` collapsed to a
  `_MovedToPerIcFile` placeholder (rule P2.4).
- `docs/ic_support_matrix.md` IC 61 row updated to reflect the
  typed `scaler_unit` and to call out the remaining opaque fields
  with their rationale.

### Verified

- Full ctest: `1223/1223` pass.
- Focused
  `dlms_cosem_tests.exe --gtest_filter='CosemRegisterTableObject.*'`:
  `9/9` pass.

## 0.120.0 - 2026-06-22

### Changed

- **`CosemProfileGenericObject`** (`class_id=7`, IEC 62056-6-2 ED4
  §4.3.6 / DLMS UA Blue Book Ed. 12.1 §4.3.6) now implements method
  `1` `reset()` with real semantics instead of returning
  `UnsupportedFeature`. Calling `InvokeMethod(1, ...)` clears the
  stored profile buffer (`bufferRows_`) and returns
  `CosemStatus::Ok`; the derived `entries_in_use` attribute (`7`)
  drops to `0` on the next read. Method output is cleared
  regardless of input. The call is idempotent on an already-empty
  buffer.
- Methods `2` `capture()`, `3` `get_buffer_by_range()` and `4`
  `get_buffer_by_index()` still return `UnsupportedFeature` and
  remain backend hooks — the built-in object has no notion of
  "now" or of the captured objects' live values, and selective
  access still relies on the backend wiring the free-standing
  `DecodeProfileGenericRangeDescriptor` /
  `DecodeProfileGenericEntryDescriptor` codecs. All four methods
  keep their `MethodAccessMode::Access` access-right entries; any
  other method id continues to return `MethodNotFound`.

### Added

- New per-IC test file
  `lib/dlms-cosem/test/cosem/test_cosem_profile_generic_object.cpp`
  (rule P2.4) with 11 focused tests covering:
  `ResetClearsBufferAndReportsZeroEntries`,
  `ResetIsIdempotentOnEmptyBuffer`,
  `CaptureMethodIsUnsupportedFeature`,
  `GetBufferByRangeIsUnsupportedFeature`,
  `GetBufferByIndexIsUnsupportedFeature`,
  `UnknownMethodReturnsMethodNotFound`,
  `EntriesInUseTracksBufferSize`,
  `WriteAttributeReturnsAccessDeniedForKnownIds`,
  `ReadAttributeRejectsUnknownIds`. Helpers are self-contained
  (no dependency on the shared `test_simple_objects.cpp` fixtures).
- Existing `RejectsWritesAndReportsUnsupportedMethods` and
  `AcceptsExplicitVersion` legacy tests in `test_simple_objects.cpp`
  updated to expect `Ok` on method `1` and `UnsupportedFeature` on
  methods `2`-`4` (instead of `UnsupportedFeature` across the
  board).
- IC support matrix row `7` updated to reflect the implemented
  `reset()`; AXDR codecs for `capture_objects`, `buffer`,
  range/entry descriptors are still pre-existing and unchanged.

### Verified

- Full ctest: `1218/1218` pass.
- Focused `dlms_cosem_tests.exe --gtest_filter='CosemProfileGenericObject.*'`:
  `23/23` pass.

## 0.119.0 - 2026-06-22

### Breaking changes

- **`CosemRegisterActivationObject`** (`class_id=6`, `version=0`,
  IEC 62056-6-2 ED4 §4.3.5 / DLMS UA Blue Book Ed. 12.1 §4.3.5)
  now takes typed structured lists instead of opaque AXDR buffers:
  - `register_assignment` switched from `CosemByteBuffer` to
    `std::vector<dlms::cosem::types::ObjectDefinition>`. Each entry
    is the `object_definition ::= structure { long-unsigned class_id,
    octet-string(6) logical_name }` from the spec. Wire form on
    `ReadAttribute(2)` is encoded by the object on every read.
  - `mask_list` switched from `CosemByteBuffer` to
    `std::vector<dlms::cosem::types::RegisterMask>`. Each entry is
    `structure { octet-string mask_name, array of long-unsigned
    index_list }`; `index_list` items are 1-based indices into
    `register_assignment`. Wire form on `ReadAttribute(3)` is encoded
    by the object on every read.
  - `active_mask` stays an `octet-string` payload (`CosemByteBuffer`),
    but is now encoded as a proper AXDR `octet-string` on every read
    instead of being passed through opaque.
  - Both ctors switched accordingly: `(name, registerAssignment,
    maskList, activeMask[, version])` now takes
    `(name, std::vector<types::ObjectDefinition>,
     std::vector<types::RegisterMask>, CosemByteBuffer activeMask
     [, version])`.
  - `RegisterAssignment()` / `MaskList()` getters now return
    `const std::vector<types::ObjectDefinition>&` /
    `const std::vector<types::RegisterMask>&` (were
    `const CosemByteBuffer&`).
  - `SetRegisterAssignment(std::vector<types::ObjectDefinition>)` /
    `SetMaskList(std::vector<types::RegisterMask>)` replace the
    buffer-taking variants.

  Methods `1` `add_register`, `2` `add_mask` and `3` `delete_mask`
  remain `UnsupportedFeature` (application-defined semantics owned by
  the backend's object catalogue).

### New types

- `dlms::cosem::types::ObjectDefinition` — typed view of the COSEM
  `object_definition` structure `{class_id, logical_name(6)}`,
  reused across ICs that reference COSEM objects by class+LN.
- `dlms::cosem::types::RegisterMask` — typed view of one
  `mask_list` entry `{mask_name, index_list[]}` from IC 6.

  Both types are inline (header-only API with thin `.cpp` for build
  discipline parity); equality / mutator surface matches the other
  COSEM types.

### Tests

- New per-IC test file
  `test/cosem/test_cosem_register_activation_object.cpp` (9 tests)
  covering class-id/version normalisation, empty / non-empty
  `register_assignment` / `mask_list` / `active_mask` wire encoding,
  read-only enforcement across all 4 attributes, `add_register` /
  `add_mask` / `delete_mask` → `UnsupportedFeature`, unknown
  method/attribute handling, and typed setter round-trip.
- New type tests: `test/cosem/types/test_object_definition.cpp` (4
  tests) and `test/cosem/types/test_register_mask.cpp` (4 tests).
- Legacy IC 6 tests in `test_simple_objects.cpp` reduced to a single
  placeholder (`MigratedToPerICFile`) per P2.4.

### Status matrix

- IC 6 (Register Activation) promoted from **Partial** to **Yes** in
  `docs/ic_support_matrix.md` (all three attributes now have a
  typed, spec-aligned representation).

### Test stats

- ctest: **1209 / 1209 pass** (was 1195 → +14 from the new IC 6
  per-IC file plus the two new type-test files, minus the four
  collapsed legacy tests).

## 0.118.0 - 2026-06-21

### Breaking changes

- **`CosemDemandRegisterObject`** (`class_id=5`, `version=0`,
  IEC 62056-6-2 ED4 §4.3.4 / DLMS UA Blue Book Ed. 12.1 §4.3.4)
  now takes typed `scaler_unit`, `capture_time` and
  `start_time_current` instead of opaque `CosemByteBuffer` payloads:
  - `scaler_unit`: `dlms::cosem::types::ScalerUnit` (same
    `scal_unit_type ::= structure { integer scaler, enum unit }` as
    IC 3 / IC 4). The wire form on `ReadAttribute(4)` is encoded by
    the object on every read.
  - `capture_time` and `start_time_current`: both
    `dlms::cosem::types::DateTime`, the 12-byte
    `octet-string(date_time)`. Wire forms on `ReadAttribute(6)` /
    `ReadAttribute(7)` are encoded by the object on every read.
  - Both ctors switched from `(name, currentAvg, lastAvg,
    CosemByteBuffer scalerUnit, CosemByteBuffer status,
    CosemByteBuffer captureTime, CosemByteBuffer startTimeCurrent,
    period, numberOfPeriods[, version])` to `(name, currentAvg,
    lastAvg, types::ScalerUnit scalerUnit, CosemByteBuffer status,
    types::DateTime captureTime, types::DateTime startTimeCurrent,
    period, numberOfPeriods[, version])`.
  - `ScalerUnit()` / `CaptureTime()` / `StartTimeCurrent()` getters
    now return `const types::ScalerUnit&` / `const types::DateTime&`
    (were `const CosemByteBuffer&`).
  - `SetScalerUnit(types::ScalerUnit)` /
    `SetCaptureTime(types::DateTime)` /
    `SetStartTimeCurrent(types::DateTime)` replace the buffer-taking
    variants.
  - `SetCurrentAverageValue` / `SetLastAverageValue` now return
    `bool` and reject empty buffers (safe-fallback parity with
    IC 3 / IC 4 `value`). The constructor likewise drops empty
    `current_average_value` / `last_average_value` arguments rather
    than retaining them; static `IsValidAverageValue` exposes the
    same pre-construction check.

  `current_average_value`, `last_average_value` and `status` remain
  opaque AXDR buffers: their concrete CHOICE alternatives are
  per-instance (per-register), and a typed discriminated-union
  representation will land later.

### Tests

- New per-IC test file `test/cosem/test_cosem_demand_register_object.cpp`
  (14 tests) covering class-id/version normalisation,
  scaler_unit / capture_time / start_time_current wire round-trip,
  safe-fallback construction, setter rejection of empty average
  values, read-only enforcement across all 9 attributes, `reset` /
  `next_period` → `UnsupportedFeature`, and unknown
  method/attribute handling. Legacy IC 5 tests in
  `test_simple_objects.cpp` reduced to a single placeholder
  (`MigratedToPerICFile`) per the one-IC-one-file rule (P2.4).

### Status matrix

- IC 5 (Demand Register) description in `docs/ic_support_matrix.md`
  updated to spell out the typed contract, safe-fallback construction,
  and why the row stays `Partial` (opaque
  `current_average_value` / `last_average_value` / `status` pending
  discriminated-union migration).

### Test stats

- ctest: **1195 / 1195 pass** (was 1185 → +10 from the new IC 5
  per-IC file minus 4 collapsed legacy tests).

## 0.117.0 - 2026-06-20

### Breaking changes

- **`CosemExtendedRegisterObject`** (`class_id=4`, `version=0`,
  IEC 62056-6-2 ED4 §4.3.3 / DLMS UA Blue Book Ed. 12.1 §4.3.3)
  now takes typed `scaler_unit` and `capture_time` instead of opaque
  `CosemByteBuffer` payloads:
  - `scaler_unit`: `dlms::cosem::types::ScalerUnit` (same
    `scal_unit_type ::= structure { integer scaler, enum unit }` as
    IC 3). The wire form on `ReadAttribute(3)` is encoded by the
    object itself on every read.
  - `capture_time`: `dlms::cosem::types::DateTime`, the 12-byte
    `octet-string(date_time)` from Blue Book. The wire form on
    `ReadAttribute(5)` is encoded by the object itself.
  - Both constructors switched from
    `(name, value, CosemByteBuffer scalerUnit, CosemByteBuffer status,
     CosemByteBuffer captureTime, valueAccess[, version])` to
    `(name, value, types::ScalerUnit scalerUnit, CosemByteBuffer
     status, types::DateTime captureTime, valueAccess[, version])`.
  - `ScalerUnit()` / `CaptureTime()` getters now return
    `const types::ScalerUnit&` / `const types::DateTime&` (was
    `const CosemByteBuffer&`).
  - `SetScalerUnit(types::ScalerUnit)` / `SetCaptureTime(types::DateTime)`
    replace the buffer-taking variants.
  - `SetValue(CosemByteBuffer)` now returns `bool` and rejects empty
    buffers (matches IC 3 safe-fallback). Constructor likewise drops
    an empty `value` argument rather than retaining it; static
    `IsValidValue` exposes the same pre-construction check.
  - `WriteAttribute(2, value)` returns `InvalidArgument` for empty
    payloads (was silently accepted).

  `value` and `status` remain opaque AXDR buffers for now: their
  concrete CHOICE alternatives are per-instance (per-register) and
  a typed discriminated-union representation will land jointly with
  IC 5 (Demand Register).

### Tests

- New per-IC test file `test/cosem/test_cosem_extended_register_object.cpp`
  (15 tests) covering class-id/version, scaler_unit/capture_time wire
  round-trip, safe-fallback construction, `SetValue` empty rejection,
  `WriteAttribute` `InvalidArgument`, read-only attribute write
  rejection, `reset` → `UnsupportedFeature`, unknown method/attribute
  handling, and `AccessRights()` carryover. Legacy IC 4 tests in
  `test_simple_objects.cpp` reduced to a single placeholder
  (`MigratedToPerICFile`) per the one-IC-one-file rule (P2.4).

### Status matrix

- IC 4 (Extended Register) description in `docs/ic_support_matrix.md`
  updated to spell out the typed `scaler_unit`/`capture_time`
  contract, safe-fallback construction, and why the row stays
  `Partial` (opaque `value`/`status` pending discriminated-union
  migration).

### Test stats

- ctest: **1185 / 1185 pass** (was 1173 → +12 from the new IC 4
  per-IC file; one redundant test — `ValueWriteHonoursAccessMode`,
  which incorrectly assumed `WriteAttribute` enforces access mode —
  was renamed to `ValueWriteAcceptsValidPayload` to match project
  convention that access-mode enforcement is the caller’s
  responsibility through `AccessRights()`).

## 0.116.0 - 2026-06-19

### Breaking changes

- **`CosemRegisterObject`** (`class_id=3`, `version=0`,
  IEC 62056-6-2 ED4 §4.3.2 / DLMS UA Blue Book Ed. 12.1 §4.3.2) now
  takes a typed `scaler_unit` instead of an opaque
  `CosemByteBuffer`:
  - `scaler_unit`: `dlms::cosem::types::ScalerUnit` — the
    `scal_unit_type ::= structure { integer scaler, enum unit }` per
    §4.3.2.2.3, with `scaler ∈ [-128,127]` and `unit` mapping to the
    Blue Book unit enumeration. The wire form on `ReadAttribute(3)`
    is encoded by the object itself on every read, so call sites no
    longer need to pre-encode the structure (or pass an empty buffer
    placeholder).
  - Constructors signature changed from
    `CosemRegisterObject(name, value, CosemByteBuffer scalerUnit,
    valueAccess[, version])` to
    `CosemRegisterObject(name, value, types::ScalerUnit scalerUnit,
    valueAccess[, version])`. All call sites in the tree
    (`tools/endpoint_*_example.cpp`, `test/integration/*`,
    `lib/dlms-endpoint/test/*`) were migrated to pass
    `dlms::cosem::types::ScalerUnit()` (default = `{scaler=0,
    unit=255 (“no unit”)}`) or an explicit `{scaler, unit}` pair.

- `SetValue` now returns `bool` and refuses empty buffers (the COSEM
  `value` attribute is a CHOICE of concrete DLMS data items — the
  empty buffer is not a valid encoding). `SetScalerUnit` takes
  `types::ScalerUnit` and never fails (every constructible
  `ScalerUnit` is on the wire valid by construction).

- `Value()` is unchanged (`const CosemByteBuffer&`), but `ScalerUnit()`
  now returns `const types::ScalerUnit&`.

### Safe-fallback construction

- The constructor drops an empty `value` argument rather than
  retaining an invalid empty AXDR payload; the attribute starts
  cleared and the backend must publish a real value via `SetValue`
  (which rejects empty buffers symmetrically). `WriteAttribute(2)`
  returns `CosemStatus::InvalidArgument` for empty payloads. A new
  public `static bool IsValidValue(const CosemByteBuffer&)` exposes
  the same check pre-construction.

### Behaviour preserved

- Class version still defaults to `0` and is normalised to
  `MaxSupportedVersion` for higher requests.
- `logical_name` (attr 1) and `scaler_unit` (attr 3) remain
  read-only; `value` (attr 2) honours the constructor’s
  `AttributeAccessMode`.
- Method `1` `reset` still surfaces as `UnsupportedFeature`
  (application-defined semantics); other method ids still return
  `MethodNotFound`.

### Tests / docs

- New per-IC suite `test/cosem/test_cosem_register_object.cpp`
  (16 tests): version normalisation, typed scaler/unit encoding,
  safe-fallback constructor, empty-value rejection on construction
  and write, access-mode propagation, `reset` → `UnsupportedFeature`,
  `IsValidValue` static checker. Legacy IC 3 cases removed from
  `test/cosem/test_simple_objects.cpp` per the one-IC-one-file rule
  (P2.4 in `docs/production_readiness_roadmap.md`).
- `docs/ic_support_matrix.md` row `3`: clarified to spell out the
  typed `ScalerUnit` surface and the safe-fallback behaviour.
- Full suite: **1173 / 1173** tests pass.

## 0.115.0 - 2026-06-19

### Breaking changes

- **`CosemRegisterMonitorObject`** (`class_id=21`, `version=0`,
  IEC 62056-6-2 ED4 §4.5.6 / Blue Book Ed. 12.1 §4.5.6) now models its
  three payload attributes as typed values instead of opaque
  `CosemByteBuffer`s:
  - `thresholds`: `std::vector<dlms::cosem::CosemByteBuffer>` — one
    opaque-per-entry AXDR data item (the per-element type tracks the
    monitored attribute and is intentionally not narrowed).
  - `monitored_value`: `dlms::cosem::types::MonitoredValue` —
    structure `{class_id: long-unsigned, logical_name: octet-string(6),
    attribute_index: integer (>= 1)}`.
  - `actions`: `std::vector<dlms::cosem::types::ActionSet>` —
    `action_set ::= structure{action_up: action_item,
    action_down: action_item}` where `action_item` reuses
    `dlms::cosem::types::Script` (`{script_logical_name,
    script_selector}`).
  - Both constructors now take typed parameters; legacy buffer-based
    constructors removed.
  - `Thresholds()`, `MonitoredValue()` and `Actions()` return typed
    references.
  - Safe-fallback construction: when `|thresholds| != |actions|` or
    any threshold entry is empty AXDR, both collections are dropped
    together rather than holding inconsistent state.
  - `SetThresholds(...)`, `SetActions(...)`, `SetMonitoredValue(...)`
    return `bool` and refuse the mutation on invariant violation
    (`|thresholds|==|actions|`, non-empty threshold entries,
    `MonitoredValue::IsValid`).
  - New static validators: `IsValidThresholds`,
    `ThresholdsMatchActions`.
  - `WriteAttribute(thresholds)` decodes the wire form
    (`array of <opaque AXDR>`), captures each entry verbatim via
    `SkipDlmsData`, rejects trailing bytes / empty entries /
    size-mismatch against current `actions` with
    `CosemStatus::InvalidArgument`, and swaps only on success.
  - `ReadAttribute` re-encodes typed state with new AXDR helpers
    (`AppendThresholds`, `AppendMonitoredValue`, `AppendActions`,
    plus `AppendActionSet`/`AppendActionItem`).
  - Behaviour unchanged: `logical_name` is read-only,
    `monitored_value`/`actions` are read-only, `thresholds` access
    is caller-selected, no methods are defined (all return
    `MethodNotFound`), version normalization to
    `MaxSupportedVersion`.

### Added

- **`dlms::cosem::types::MonitoredValue`** — typed representation of
  the `value_definition` structure (IEC 62056-6-2 ED4 §4.5.6.2.3):
  `class_id`, `logical_name`, `attribute_index`. Constructor clamps
  `attribute_index < 1` to `1` (safe-fallback); `SetAttributeIndex`
  refuses values below the minimum. Static `IsValid` validator.
- **`dlms::cosem::types::ActionSet`** — typed representation of the
  `action_set` structure (IEC 62056-6-2 ED4 §4.5.6.2.4) holding two
  `types::Script` action items (`action_up`, `action_down`).
- Unit tests `test_monitored_value.cpp` and `test_action_set.cpp` for
  the new types; per-class test file
  `test_cosem_register_monitor_object.cpp` for IC 21 typed API
  (18 tests covering ctor safe-fallback, AXDR round-trip,
  WriteAttribute validation paths, access-mode gating, setters,
  static validators, and method/version behaviour).
- IC support matrix row 21 promoted from **Partial** to **Yes**.

## 0.114.0 - 2026-06-18

### Breaking changes

- **`CosemScriptTableObject`** (`class_id=9`, `version=0`,
  IEC 62056-6-2 ED4 §4.5.2 / Blue Book Ed. 12.1 §4.5.2) now models the
  `scripts` attribute as a typed
  `std::vector<dlms::cosem::types::ScriptEntry>` instead of an opaque
  `CosemByteBuffer`. Each entry holds a `script_identifier` plus a
  `std::vector<dlms::cosem::types::ActionSpecification>` (typed
  `service_id`, `class_id`, `logical_name`, `index`, and an opaque
  `parameter` whose raw AXDR bytes are preserved verbatim).
  - Both constructors now take a `std::vector<types::ScriptEntry>`
    instead of a `CosemByteBuffer`.
  - `Scripts()` returns `const std::vector<types::ScriptEntry>&`.
  - `SetScripts(...)` returns `bool` and does not mutate on failure;
    new static `IsValidScripts(...)` exposes the same invariants
    (per-action `IsValid` + unique `script_identifier`).
  - Safe-fallback construction: a malformed `scripts` argument leaves
    the object holding an empty collection rather than invalid state.
  - `WriteAttribute(scripts)` decodes the wire form
    (`array of script`, `script ::= structure(2){long-unsigned id,
    array of action_specification}`,
    `action_specification ::= structure(5){enum, long-unsigned, octet-
    string(6), integer, parameter}`), captures `parameter` as raw
    AXDR via the existing `SkipDlmsData` helper, validates every
    field plus the unique-identifier invariant, and only swaps on
    success; returns `CosemStatus::InvalidArgument` on malformed
    AXDR, wrong tags, wrong field counts, invalid actions, duplicate
    `script_identifier`, or trailing bytes after the last script.
  - `ReadAttribute(scripts)` re-encodes from the typed model; empty
    `parameter` round-trips as AXDR `null-data` (`0x00`).
  - `InvokeMethod(execute=1)` semantics are unchanged: still surfaces
    `CosemStatus::UnsupportedFeature` so a future backend can attach
    script execution without changing the object surface.

### Added

- New AXDR codec helpers `AppendActionSpecification` /
  `DecodeActionSpecification` and `AppendScripts` / `DecodeScripts`
  in `lib/dlms-cosem/src/cosem/simple_objects.cpp`, layered on the
  existing AXDR primitives plus `SkipDlmsData` for opaque
  `parameter` capture.
- Per-class test file `test/cosem/test_cosem_script_table_object.cpp`
  (16 tests) per the per-IC test-file convention
  (`docs/production_readiness_roadmap.md` P2.4). Legacy buffer-based
  IC 9 tests removed from `test/cosem/test_simple_objects.cpp`.

### Docs

- `docs/ic_support_matrix.md`: IC 9 row updated from Partial to Yes.

## 0.113.0 - 2026-06-18

### Breaking changes

- **`CosemActivityCalendarObject`** (`class_id=20`, `version=0`,
  IEC 62056-6-2 ED4 §4.5.5 / Blue Book Ed. 12.1 §5.1.9) now models
  `season_profile` / `week_profile_table` / `day_profile_table` (both
  active and passive) as typed
  `std::vector<dlms::cosem::types::SeasonProfile>` /
  `std::vector<dlms::cosem::types::WeekProfile>` /
  `std::vector<dlms::cosem::types::DayProfile>` instead of opaque
  `CosemByteBuffer`s, and `activate_passive_calendar_time` as a typed
  `dlms::cosem::types::DateTime` (wildcards allowed; an all-wildcard
  value means "never activates" per the spec note).
  `calendar_name_active` and `calendar_name_passive` remain opaque
  octet-strings.
  - Constructors now take typed `std::vector<>` collections and a
    `types::DateTime` activation time instead of `CosemByteBuffer`s.
  - Setters `SetSeasonProfilePassive`, `SetWeekProfileTablePassive`,
    `SetDayProfileTablePassive` return `bool` and do not mutate on
    failure.
  - New static validators expose intra-collection invariants
    (`IsValidSeasonProfile`, `IsValidWeekProfileTable`,
    `IsValidDayProfileTable`) and cross-collection invariants
    (`WeekProfileTableSatisfies`, `SeasonProfileSatisfies`).
  - Safe-fallback construction: passing an inconsistent collection
    yields an empty value for that collection rather than holding
    invalid state.
  - `WriteAttribute` decodes the wire form
    (`array of season`, `array of week_profile`,
    `array of day_profile`, `date_time` octet-string), validates each
    field via `types::DateTime::TryFromBytes` /
    `types::Time::TryFromBytes` and enforces uniqueness and
    cross-reference invariants before swapping; returns
    `CosemStatus::InvalidArgument` on malformed AXDR, unknown tags,
    out-of-range fields, duplicate season/week names, duplicate
    day_ids, or broken cross-references, without touching the
    existing collections.

### Test reorganization

- IC 20 tests live in their own file
  `test/cosem/test_cosem_activity_calendar_object.cpp` per the
  per-class test-file convention adopted in
  `docs/production_readiness_roadmap.md` P2.4. The legacy buffer-based
  fixture (`ActivityCalendarBuffers`) and tests were removed from
  `test/cosem/test_simple_objects.cpp`.

### Documentation

- `docs/ic_support_matrix.md` row `20` Activity Calendar: Partial →
  Yes; description updated to reflect the typed model and the new
  validator surface.

## 0.112.0 - 2026-06-18

### Breaking changes

- **`CosemScheduleObject`** (`class_id=10`, `version=0`,
  IEC 62056-6-2 ED4 §4.5.3 / Blue Book Ed. 12.1 §5.1.7) now models its
  `entries` attribute as a typed
  `std::vector<dlms::cosem::types::ScheduleTableEntry>` instead of an
  opaque `CosemByteBuffer`. Each entry is
  `{ index: long-unsigned, enable: boolean,
     script: { logical_name: octet-string(6), selector: long-unsigned },
     switch_time: octet-string(4)=time,
     validity_window: long-unsigned (0xFFFF=always),
     exec_weekdays: bit-string(7) (bit 0 = Mon … bit 6 = Sun),
     exec_specdays: bit-string(64) (bit i = day_id i, capped to 0..63),
     begin_date: octet-string(5)=date,
     end_date: octet-string(5)=date }`.
  - Constructors now take
    `const std::vector<types::ScheduleTableEntry>&`.
  - `Entries()` returns `const std::vector<types::ScheduleTableEntry>&`.
  - `SetEntries(...)` returns `bool`: validates every entry via
    `types::ScheduleTableEntry::IsValid` and the collection invariant
    (unique `index`) without mutating on failure.
  - On the wire the attribute is still encoded exactly as the spec
    requires (`array of structure(10) { long-unsigned, boolean,
    octet-string(6), long-unsigned, octet-string(4), long-unsigned,
    bit-string(7), bit-string(64), octet-string(5), octet-string(5) }`).
    `ReadAttribute` encodes from the typed collection;
    `WriteAttribute` decodes the wire form, validates each field via
    `types::Time::TryFromBytes` / `types::Date::TryFromBytes` and the
    uniqueness invariant before swapping. Malformed AXDR, wrong
    tags/lengths, bit-string widths > the spec maximum, invalid
    times/dates and duplicate `index` values all return
    `CosemStatus::InvalidArgument` without touching the existing
    entries.
  - Safe-fallback constructor: passing an invalid collection now
    yields an empty `entries_` rather than holding an inconsistent
    state.

### Specific methods

- **Method `1` `enable_disable(structure { first_disable, last_disable,
  first_enable, last_enable })`**, **method `2` `insert(schedule_table_entry)`**
  and **method `3` `delete(structure { first_index, last_index })`** are
  now fully implemented per IEC 62056-6-2 ED4 §4.5.3.3 instead of
  returning `UnsupportedFeature`.
  - `enable_disable` disables range A first, then enables range B, so
    when the ranges overlap the enabling side wins per spec. Ranges
    with `first>last` or `first>9999` are no-ops; `last>9999` is
    capped to 9999.
  - `insert` appends a new entry or overwrites the existing one
    sharing the new `index`.
  - `delete` removes all entries whose `index` falls in the inclusive
    `[first,last]` range; `first>last` is a no-op.
  - All three return `Ok` on success and `InvalidArgument` on
    malformed AXDR (wrong tag, wrong structure arity, EOF, trailing
    bytes, decoded entry that violates `ScheduleTableEntry::IsValid`,
    …); other method ids continue to report `MethodNotFound`.

### `exec_specdays` width — path A (64-day cap)

- `types::ScheduleTableEntry::SpecdaysBitWidth = 64` and the field is
  carried as `uint64_t`. Per spec choice **A** the IC caps usable
  `day_id` values at `0..63`. Values outside this range cannot be
  represented in this implementation; encode/decode round-trips fail
  on wider bit-strings, and a backend that needs the historical
  ED4-era "unbounded" interpretation should either truncate before
  insertion or stick to spec choice B (out of scope here).

### Tests

- New file `lib/dlms-cosem/test/cosem/types/test_schedule_table_entry.cpp`
  (7 tests) covering value/equality, weekday-mask high-bit refusal,
  `IsValid`, and accessor/setter round-trips.
- New file `lib/dlms-cosem/test/cosem/test_cosem_schedule_object.cpp`
  (16 tests) covering descriptor / default rights, version
  normalization, ctor and `SetEntries` validation (per-entry +
  unique-index, fail-no-mutate), AXDR codec round-trip, empty
  collection wire form, `WriteAttribute` accept/reject paths
  (malformed, truncated, duplicate-index), access-mode gating,
  `enable_disable` spec ordering and no-op rules,
  `insert` overwrite-on-index-collision, `delete` inclusive-range
  removal and `first>last` no-op, and `MethodNotFound` for unknown
  ids. The corresponding 4 legacy buffer-based tests in
  `test_simple_objects.cpp` (and their now-unused
  `MakeSampleScheduleEntries` helper) were removed in the same
  commit to honor the new "one test file per IC class" roadmap rule.

### AXDR codec additions

- New shared helpers in `src/cosem/simple_objects.cpp`:
  `AppendBitStringMsbFirst(output, bits, bit_width)` and
  `ReadBitStringMsbFirst(input, offset, expected_bit_width, bits_out)`
  for the standard AXDR bit-string (`tag=0x04`, length in bits,
  MSB-aligned, bit 0 of the bit-string = MSB of the first octet).
  These will be reused by upcoming work on IC 20 (Activity Calendar)
  and any other ICs that carry bit-string fields.

## 0.111.0 - 2026-06-18

### Breaking changes

- **`CosemSpecialDaysTableObject`** (`class_id=11`, `version=0`,
  Blue Book Ed. 12.1 / IEC 62056-6-2 ED4 §4.5.4) now models its
  `entries` attribute as a typed
  `std::vector<dlms::cosem::types::SpecialDayEntry>` instead of an
  opaque `CosemByteBuffer`. Each entry is
  `{ index: long-unsigned, specialday_date: types::Date, day_id:
  unsigned }`.
  - Constructors now take `const std::vector<types::SpecialDayEntry>&`.
  - `Entries()` returns `const std::vector<types::SpecialDayEntry>&`.
  - `SetEntries(...)` returns `bool` and validates the collection
    invariant (unique `index` and unique `specialday_date`) without
    mutating on failure.
  - On the wire the attribute is still encoded exactly as the spec
    requires (`array of structure(3) { long-unsigned, octet-string(5),
    unsigned }`). `ReadAttribute` encodes from the typed collection;
    `WriteAttribute` decodes, validates field-by-field via
    `types::Date::TryFromBytes`, then enforces the uniqueness
    invariant before swapping. Malformed AXDR, wrong tags/lengths,
    invalid dates, and invariant violations all return
    `CosemStatus::InvalidArgument` without touching the existing
    entries.
  - Safe-fallback constructor: passing an invalid collection now
    yields an empty `entries_` rather than holding an inconsistent
    state.
- **Method `1` `insert(spec_day_entry)`** and **method `2`
  `delete(long-unsigned index)`** are now fully implemented per
  §4.5.4.3.1 instead of returning `UnsupportedFeature`. `insert`
  overwrites any existing entry sharing the new `index` *or* the new
  `specialday_date` (so when the incoming entry collides with two
  different stored entries on the two keys at once, both are removed
  before insertion and the post-condition still holds). `delete`
  removes the entry with the requested index, returning `Ok` even if
  no entry was found (the spec does not mandate an error for a
  missing index). Both methods return `InvalidArgument` on malformed
  payloads and clear the output buffer on success.
- Added public helper
  `CosemSpecialDaysTableObject::IsValidEntries(value)` so callers can
  pre-validate a candidate collection before `SetEntries`.

### Migration notes

Prior callers passing AXDR bytes directly must now build the typed
collection:

```cpp
std::vector<dlms::cosem::types::SpecialDayEntry> entries;
dlms::cosem::types::Date d;
d.SetYear(2021u); d.SetMonth(1u); d.SetDayOfMonth(1u);
entries.push_back({1u, d, 1u});

CosemSpecialDaysTableObject obj(
  name, entries, AttributeAccessMode::ReadAndWrite);
```

For wildcard-date entries (e.g. recurring Christmas) leave `year` at
its default (unspecified, `0xFFFF`) — `types::Date` mirrors the
spec sentinels.

### Tests

- 13 new `CosemSpecialDaysTableObject.*` tests covering wire
  round-trip, validation, `Insert` overwrite-by-index /
  overwrite-by-date / both-at-once, `Delete`, `InvokeMethod` for
  `insert`/`delete` (success + invalid payload + missing index), and
  wildcard-date round-trip.
- Full `ctest`: 1078/1078 (the two transient failures
  `dlms_transport_tests` and `dlms_package_artifact_smoke` pass on
  re-run — same flappers observed in prior phases, unrelated to
  IC 11).

## 0.110.0 - 2026-06-18

### Breaking changes

- **`CosemSingleActionScheduleObject`** (`class_id=22`, `version=0`,
  Blue Book Ed. 12.1 / IEC 62056-6-2 ED4 §4.5.7) now models its three
  configurable attributes via typed values instead of opaque
  `CosemByteBuffer`s:
  - `executed_script` → `dlms::cosem::types::Script` (new typed
    struct: `{ logical_name, selector }`).
  - `type` → `dlms::cosem::types::SingleActionScheduleType` (new
    enum wrapper, range 1..5, with predicate helpers
    `RequiresSingleEntry()`, `RequiresUniformTime()`,
    `ForbidsWildcardsInDate()` matching §4.5.7.2.3).
  - `execution_time` → `std::vector<std::pair<types::Time,
    types::Date>>` (typedef
    `CosemSingleActionScheduleObject::ExecutionTimeEntry`).
  Constructors, getters (`ExecutedScript()` / `Type()` /
  `ExecutionTime()`) and the new `SetExecutedScript` /
  `SetType` / `SetExecutionTime` setters all use these typed values.
  On the wire the attributes are still encoded exactly as the spec
  requires (`structure(2) { octet-string(6), long-unsigned }` for the
  script; `enum` for the type; `array of structure(2) { octet-string(4)
  time, octet-string(5) date }` for execution_time); field-level
  validation now enforces both per-field constraints and the
  cross-field invariants from §4.5.7.2.3.
- **Invariants enforced**: `type==1 → exactly 1 entry`; `type∈{2,3} →
  all entries share the same time`; `type∈{2,4} → no wildcard in date`;
  `hundredths_of_second` must be `0` (or unspecified) on every stored
  time. Setters refuse to mutate on violation (`SetType` /
  `SetExecutionTime` return `false`); constructors fall back to a safe
  single-entry, all-wildcard, type=1 schedule rather than holding an
  invalid value. `WriteAttribute` returns `CosemStatus::InvalidArgument`
  for malformed AXDR, out-of-range enum values, and invariant
  violations.
- Added public helper
  `CosemSingleActionScheduleObject::IsValidExecutionTime(type,
  executionTime)` so callers can pre-validate a candidate pair before
  committing it.

No migration shim is provided — callers that were constructing IC 22
from hand-rolled `CosemByteBuffer`s must build the typed values
directly. The wire encoding is unchanged.

### Added

- **`dlms::cosem::types::Script`** — typed wrapper around the IC 22
  executed_script structure (`{ logical_name, selector }`). Header
  `dlms/cosem/types/script.hpp`, 4 dedicated unit tests in
  `TypesScript.*`.
- **`dlms::cosem::types::SingleActionScheduleType`** — typed wrapper
  around the IC 22 type enum (1..5) with spec-driven invariant
  predicates. Header
  `dlms/cosem/types/single_action_schedule_type.hpp`, 6 dedicated
  unit tests in `TypesSingleActionScheduleType.*`.
- **`CosemSingleActionScheduleObject`** — 7 new unit tests covering
  the invariants, the safe-fallback construction path, the
  `WriteAttribute` error reporting, and a round-trip of a type=4
  multi-entry schedule (11 IC 22 tests total).

## 0.109.1 - 2026-06-17

### Added

- **`dlms::cosem::types::Date`** — typed wrapper around the
  Blue Book Ed. 12.1 §4.1.6.1 `date` value (5 bytes:
  `year`/`month`/`day_of_month`/`day_of_week`). Exposed via
  `dlms/cosem/types/date.hpp` with validating setters, sentinel
  constants (year `0xFFFF`, month DST begin/end `0xFE`/`0xFD`,
  day-of-month last/second-last `0xFE`/`0xFD`, reserved range
  `0xE0..0xFC` rejected), `ToBytes` / `TryFromBytes` round-trip
  helpers and equality operators. 11 dedicated unit tests in
  `TypesDate.*`.
- **`dlms::cosem::types::Time`** — typed wrapper around the
  Blue Book Ed. 12.1 §4.1.6.1 `time` value (4 bytes:
  `hour`/`minute`/`second`/`hundredths_of_second`). Exposed via
  `dlms/cosem/types/time.hpp` with validating setters, the
  `0xFF` per-field wildcard, `ToBytes` / `TryFromBytes` round-trip
  helpers and equality operators. 7 dedicated unit tests in
  `TypesTime.*`.

Neither type is consumed by any IC yet; this release only adds the
building blocks. Upcoming phases will migrate IC 19 (Special Days
Table), IC 20 (Activity Calendar) and IC 22 (Single Action
Schedule) onto these typed values.

## 0.109.0 - 2026-06-17

### Breaking changes

- **`CosemClockObject`** (`class_id=8`, `version=0`) now models the
  `date_time` attributes via the new typed value
  `dlms::cosem::types::DateTime` (Blue Book Ed. 12.1 §4.1.6.1)
  instead of opaque 12-byte buffers. The constructors and the
  `Time()` / `DaylightSavingsBegin()` / `DaylightSavingsEnd()`
  accessors take and return `const types::DateTime&`. On the wire
  the attributes are still encoded as `octet-string` of length 12
  (per spec), but field-level validation now rejects values with
  out-of-range months, day-of-month, day-of-week, hour / minute /
  second / hundredths and deviation; wildcard sentinels
  (`0xFF`, `0xFFFF`, `0x8000`, `0xFE`, `0xFD`) and the documented
  DST sentinels (`0xFE`, `0xFD` for the month; `last day`,
  `second-last day`) are accepted. The new
  `SetTime` / `SetStatus` backend hooks let owners republish
  authoritative clock state without going through the
  `WriteAttribute` access-mode gate.

  Migration: construct the value via `types::DateTime` setters
  (`SetYear`, `SetMonth`, ..., `SetDeviation`, `SetClockStatus`)
  or via `types::DateTime::TryFromBytes` if you already have the
  12-byte buffer; pass the resulting `types::DateTime` to the
  `CosemClockObject` constructor. Replace direct buffer access
  with the typed getters (e.g. `object.Time().Year()`).

### Added

- **`dlms::cosem::types::DateTime`** — typed wrapper around the
  Blue Book Ed. 12.1 §4.1.6.1 `date_time` value (12 bytes,
  `year`/`month`/`day_of_month`/`day_of_week`/`hour`/`minute`/
  `second`/`hundredths_of_second`/`deviation`/`clock_status`).
  Exposed via `dlms/cosem/types/date_time.hpp` with validating
  setters, sentinel constants, `ToBytes` / `TryFromBytes` round-trip
  helpers and equality operators. 14 dedicated unit tests in
  `TypesDateTime.*`.

## 0.108.0 - 2026-06-17

### Breaking changes

- **`CosemPrimePlcMacNetworkAdminDataObject`** (`class_id=85`,
  `version=0`) realigned with IEC 62056-6-2 ED4 (2021) §4.12.9 /
  DLMS UA Blue Book Ed. 12.1 §4.12.9. The previous implementation
  exposed four invented counter-shaped attributes
  (`node_registrations`, `node_unregistrations`,
  `processed_alive_msgs`, `handled_promotions`) on attribute ids
  `2`-`5`, which never matched the published IC layout.
  The class now exposes the five spec-defined dynamic array
  attributes:
    - `2 mac_list_multicast_entries`  (array, PIB attribute `0x0052`)
    - `3 mac_list_switch_table`       (array, PIB attribute `0x0053`)
    - `4 mac_list_direct_table`       (array, PIB attribute `0x0055`)
    - `5 mac_list_available_switches` (array, PIB attribute `0x0056`)
    - `6 mac_list_phy_comm`           (array, PIB attribute `0x0057`)
  Both constructors gained `macListMulticastEntries`,
  `macListSwitchTable`, `macListDirectTable`,
  `macListAvailableSwitches` and `macListPhyComm` parameters
  instead of the four old counter buffers. The
  `NodeRegistrations()` / `NodeUnregistrations()` /
  `ProcessedAliveMsgs()` / `HandledPromotions()` accessors are
  replaced with `MacListMulticastEntries()`,
  `MacListSwitchTable()`, `MacListDirectTable()`,
  `MacListAvailableSwitches()` and `MacListPhyComm()`. Attributes
  `2`-`6` share a caller-selected `AttributeAccessMode` so the
  management backend can republish refreshed PRIME MAC network
  state; `logical_name` (`1`) stays hard-coded read-only.
  Specific method `1 reset(data)` (optional per spec) keeps
  returning `UnsupportedFeature` because the backend owns the
  table snapshots; other method ids continue to report
  `MethodNotFound`.
  Callers that previously constructed the object with four
  counter buffers must rebuild it from the five spec-defined
  table buffers and switch to the new getters.

### Misc

- Existing 4 IC 85 gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnUnsupportedFeature`, `NormalizesVersionAboveMax`)
  rewritten against the new attribute layout; no net test count
  change. Full MinGW64 ctest: 1016/1016 passing.
- `docs/ic_support_matrix.md` IC 85 row updated to describe the
  five spec attributes, PIB mappings, RW policy and the optional
  `reset(data)` method semantics.

## 0.107.0 - 2026-06-17

### Breaking changes

- **`CosemPrimePlcApplicationIdentificationObject`** (`class_id=86`,
  `version=0`) realigned with IEC 62056-6-2 ED4 (2021) §4.12.11 /
  DLMS UA Blue Book Ed. 12.1 §4.12.11. The previous implementation
  exposed a single sweeping `application_identifier` attribute on
  attribute id `2`, which never matched the published IC layout.
  The class now exposes the three spec-defined static attributes:
    - `2 firmware_version` (octet-string, max 128 bytes,
      PIB attribute `0x0075`)
    - `3 vendor_Id`        (long-unsigned,
      PIB attribute `0x0076`)
    - `4 product_Id`       (long-unsigned,
      PIB attribute `0x0077`)
  Both constructors gained `firmwareVersion`, `vendorId` and
  `productId` parameters instead of a single
  `applicationIdentifier`. The `ApplicationIdentifier()` accessor
  is replaced with `FirmwareVersion()`, `VendorId()` and
  `ProductId()`. Attributes `2`-`4` share a caller-selected
  `AttributeAccessMode` so the management backend can republish
  refreshed identifiers as the firmware updates; `logical_name`
  (`1`) stays hard-coded read-only. IC v0 still defines no
  specific methods, so `InvokeMethod` keeps reporting
  `MethodNotFound` for every method id.
  Callers that previously constructed the object with a sweeping
  application identifier must split it into the three spec
  attributes and switch their getters.

### Misc

- Existing 4 IC 86 gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`, `NoMethodsDefined`,
  `NormalizesVersionAboveMax`) rewritten against the new
  attribute layout; no net test count change. Full MinGW64
  ctest: 1016/1016 passing.
- `docs/ic_support_matrix.md` IC 86 row updated to describe the
  three spec attributes, PIB mappings, RW policy and lack of
  methods.

## 0.106.18 - 2026-06-17

- New built-in COSEM object `CosemPrimePlcLlcSscsSetupObject`
  (`class_id=80`, `version=0`) implementing **61334-4-32 LLC
  SSCS setup** per IEC 62056-6-2 ED4 (2021) §4.12.3 and DLMS UA
  Blue Book Ed. 12.1 §4.12.3. Previously
  docs/ic_support_matrix.md marked IC 80 as
  "Application-provided"; this change closes that gap so the
  whole PRIME PLC block (ICs 80, 81, 82, 83, 84) now has
  built-in coverage with only IC 85 (MAC network administration
  data) and IC 86 (Application identification) left as
  application-provided.
  The IC holds the addresses provided by the base node during
  the opening of the convergence layer, as a response to the
  service node's establish request. After deregistration the
  spec sets `service_node_address = NEW = 0x0FFE` and
  `base_node_address = 0x0000`.
  Exposes both 432 CL addresses as opaque A-XDR buffers prepared
  by the caller:
    - `2 service_node_address` (long-unsigned)
    - `3 base_node_address`    (long-unsigned)
  Attributes `2`-`3` share a caller-selected
  `AttributeAccessMode` so the PRIME convergence-layer backend
  can republish refreshed addresses as the base/service node
  (de)registers. `logical_name` (`1`) is hard-coded read-only.
  The spec defines one specific method `reset(data)`
  (data ::= integer(0), method id `1`) for deallocating the
  service node address; actual deallocation is owned by the
  PRIME convergence-layer backend, so `InvokeMethod` reports
  `UnsupportedFeature` for method id `1` and `MethodNotFound`
  for every other method id. Constructor normalises `version`
  to `MaxSupportedVersion=0`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `ResetMethodReportsUnsupported`, `NormalizesVersionAboveMax`).
  Full MinGW64 ctest: 1016/1016 passing (was 1012/1012).

## 0.106.17 - 2026-06-17

- New built-in COSEM object `CosemPrimePlcPhyLayerCountersObject`
  (`class_id=81`, `version=0`) implementing **PRIME NB OFDM PLC
  Physical layer counters** per IEC 62056-6-2 ED4 (2021) §4.12.5
  and DLMS UA Blue Book Ed. 12.1 §4.12.4. Previously
  docs/ic_support_matrix.md marked IC 81 as
  "Application-provided"; this change closes that gap and slots
  the class into the existing PRIME PLC block alongside IC 82
  (MAC setup), IC 83 (MAC functional parameters) and IC 84
  (MAC counters).
  Exposes the four PHY statistics counters as opaque A-XDR
  buffers prepared by the caller:
    - `2 phy_stats_crc_incorrect_count` (long-unsigned, PIB
      0x00A0)
    - `3 phy_stats_crc_failed_count`    (long-unsigned, PIB
      0x00A1)
    - `4 phy_stats_tx_drop_count`       (long-unsigned, PIB
      0x00A2)
    - `5 phy_stats_rx_drop_count`       (long-unsigned, PIB
      0x00A3)
  Counters are read-only per spec, but attributes `2`-`5` share a
  caller-selected `AttributeAccessMode` so the PLC stack backend
  can republish refreshed counter buffers as the PRIME node
  updates its PIB.
  `logical_name` (`1`) is hard-coded read-only. The spec defines
  one specific method `reset(data)` (data ::= integer(0), method
  id `1`) for clearing the counters; actual zeroing is owned by
  the PLC backend, so `InvokeMethod` reports `UnsupportedFeature`
  for method id `1` and `MethodNotFound` for every other method
  id. Constructor normalises `version` to `MaxSupportedVersion=0`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `ResetMethodReportsUnsupported`, `NormalizesVersionAboveMax`).
  Full MinGW64 ctest: 1012/1012 passing (was 1008/1008).

## 0.106.16 - 2026-06-17

- New built-in COSEM object `CosemWirelessModeQChannelObject`
  (`class_id=73`, `version=1`) implementing **Wireless Mode Q
  channel** per IEC 62056-6-2 ED4 (2021) §4.8.4 and DLMS UA Blue
  Book Ed. 12.1 §4.8.3 (see also EN 13757-5:2015). Closes the
  last M-Bus / EN 13757 short-range wireless gap in the built-in
  IC catalogue: previously `73` was `Application-provided`.
  Exposes the three configuration attributes as opaque A-XDR
  buffers prepared by the caller:
    - `2 addr_state` (enum: 0 not assigned / 1 assigned by
      manual setting or automated method)
    - `3 device_address` (octet-string carrying the currently
      assigned address of the device on the network)
    - `4 address_mask` (octet-string carrying the group address
      the device will respond to when short-form addressing is
      used).
  `logical_name` (`1`) is read-only; attributes `2`-`4` share a
  caller-selected `AttributeAccessMode` so the backend can
  republish refreshed wireless commissioning state after over-
  the-air re-keying performed out-of-band. The class defines no
  specific methods; all method ids report `MethodNotFound`.
  Constructor normalises `version` to `MaxSupportedVersion=1`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnMethodNotFound`, `NormalizesVersionAboveMax`).

## 0.106.15 - 2026-06-17

- New built-in COSEM object `CosemIso8802LlcType3SetupObject`
  (`class_id=59`, `version=0`) implementing **ISO/IEC 8802-2 LLC
  Type 3 setup** (acknowledged connectionless operation) per IEC
  62056-6-2 ED4 (2021) §4.11.4 and DLMS UA Blue Book Ed. 12.1
  §4.11.4. Exposes the five LLC Type 3 parameters as opaque
  A-XDR buffers prepared by the caller:
    - `2 max_octets_acn_pdu_n3` (long-unsigned)
    - `3 max_number_transmissions_n4` (unsigned)
    - `4 acknowledgement_time_t1` (long-unsigned)
    - `5 receive_lifetime_var_t2` (long-unsigned)
    - `6 transmit_lifetime_var_t3` (long-unsigned)
  Parameter semantics per ISO/IEC 8802-2:1998 §8.6.1, §8.6.2 and
  the acknowledged-connectionless timer descriptions in the same
  clause set. `logical_name` is read-only; attributes 2–6 share
  a caller-selected `AttributeAccessMode` so the backend can
  re-tune ACL parameters after commissioning. The class defines
  no specific methods; all method ids report `MethodNotFound`.
  Constructor normalises `version` to `MaxSupportedVersion=0`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnMethodNotFound`, `NormalizesVersionAboveMax`).
  MinGW64 ctest 1004/1004 ✅ (was 1000/1000 in 0.106.14; +4 new
  cases from this IC).
- Docs: `ic_support_matrix.md` row for IC 59 promoted from
  Application-provided to Partial. Closes the ISO/IEC 8802-2 LLC
  family (IC 57/58/59) and the entire IEC 61334-4-32 / ISO/IEC
  8802-2 LLC setup area of the support matrix.
- No public API change to existing types. Patch bump 0.106.14 →
  0.106.15.

## 0.106.14 - 2026-06-17

- New built-in COSEM object `CosemIso8802LlcType2SetupObject`
  (`class_id=58`, `version=0`) implementing **ISO/IEC 8802-2 LLC
  Type 2 setup** (connection-oriented operation) per IEC
  62056-6-2 ED4 (2021) §4.11.3 and DLMS UA Blue Book Ed. 12.1
  §4.11.3. Exposes the nine LLC Type 2 parameters as opaque
  A-XDR buffers prepared by the caller:
    - `2 transmit_window_size_k` (unsigned, 1..127, def 1)
    - `3 receive_window_size_rw` (unsigned, 1..127, def 1)
    - `4 max_octets_i_pdu_n1` (long-unsigned, def 128)
    - `5 max_number_transmissions_n2` (unsigned)
    - `6 acknowledgement_timer` (long-unsigned, seconds)
    - `7 p_bit_timer` (long-unsigned, seconds)
    - `8 reject_timer` (long-unsigned, seconds)
    - `9 busy_state_timer` (long-unsigned, seconds)
  Parameter semantics per ISO/IEC 8802-2:1998 §7.8.1–7.8.4.
  `logical_name` is read-only; attributes 2–9 share a caller-
  selected `AttributeAccessMode` so the backend can re-tune
  connection parameters after commissioning. The class defines
  no specific methods; all method ids report `MethodNotFound`.
  Constructor normalises `version` to `MaxSupportedVersion=0`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnMethodNotFound`, `NormalizesVersionAboveMax`).
  MinGW64 ctest 1000/1000 ✅ (was 996/996 in 0.106.13; +4 new
  cases from this IC).
- Docs: `ic_support_matrix.md` row for IC 58 promoted from
  Application-provided to Partial. Only IC 59 (LLC Type 3,
  acknowledged connectionless) remains in the LLC area.
- No public API change to existing types. Patch bump 0.106.13 →
  0.106.14.

## 0.106.13 - 2026-06-17

- New built-in COSEM object `CosemIso8802LlcType1SetupObject`
  (`class_id=57`, `version=0`) implementing **ISO/IEC 8802-2 LLC
  Type 1 setup** per IEC 62056-6-2 ED4 (2021) §4.11.2 and DLMS UA
  Blue Book Ed. 12.1 §4.11.2. Exposes attribute
  `2 max_octets_ui_pdu` (long-unsigned, default 128) as an
  opaque A-XDR buffer prepared by the caller. Per ISO/IEC
  8802-2:1998 §6.8.1 ("Maximum number of octets in a UI PDU"),
  the LLC sublayer imposes no restriction, but for
  interoperability all MACs must accommodate UI PDUs with
  information fields up to and including 128 octets in length.
  `logical_name` is read-only; attribute 2 honors a caller-
  selected `AttributeAccessMode` so the backend can republish
  the negotiated MAC ceiling after commissioning. The class
  defines no specific methods; all method ids report
  `MethodNotFound`. Constructor normalises `version` to
  `MaxSupportedVersion=0`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributeHonorsAccessMode`,
  `MethodsReturnMethodNotFound`, `NormalizesVersionAboveMax`).
  MinGW64 ctest 996/996 ✅ (was 992/992 in 0.106.12; +4 new
  cases from this IC).
- Docs: `ic_support_matrix.md` row for IC 57 promoted from
  Application-provided to Partial; remaining LLC row narrowed to
  `58`-`59`. Type 2 (connection-oriented) and Type 3
  (acknowledged connectionless) LLC setups remain.
- No public API change to existing types. Patch bump 0.106.12 →
  0.106.13.

## 0.106.12 - 2026-06-17

- New built-in COSEM object `CosemSFskReportingSystemListObject`
  (`class_id=56`, `version=0`) implementing **S-FSK Reporting
  system list** per IEC 62056-6-2 ED4 (2021) §4.10.8 and DLMS UA
  Blue Book Ed. 12.1 §4.10.8. Exposes attribute
  `2 reporting_system_list` (array of system-title where
  system-title ::= octet-string; MIB variable
  reporting-system-list (variable 16) per IEC 61334-4-512:2001
  §5.7 — system-titles of server systems that issued a
  DiscoverReport CI_PDU and have not yet been registered, sorted
  by arrival with the newest first) as an opaque A-XDR buffer
  prepared by the caller. `logical_name` is read-only;
  attribute 2 honors a caller-selected `AttributeAccessMode` so
  the CIASE backend can republish the list after DiscoverReport
  CI_PDUs arrive and registrations purge entries out-of-band.
  The class defines no specific methods; all method ids report
  `MethodNotFound`. Constructor normalises `version` to
  `MaxSupportedVersion=0`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributeHonorsAccessMode`,
  `MethodsReturnMethodNotFound`, `NormalizesVersionAboveMax`).
  MinGW64 ctest 992/992 ✅ (was 988/988 in 0.106.11; +4 new
  cases from this IC).
- Docs: `ic_support_matrix.md` row for IC 56 promoted from
  Application-provided to Partial; remaining LLC row narrowed to
  `57`-`59`. With IC 50/51/52/53/55/56 done, the S-FSK PLC stack
  (IEC 61334-5-1 lower-layer profile, IEC 61334-4-32 LLC, S-FSK
  CIASE reporting) is feature-complete; only the ISO/IEC 8802-2
  LLC setup family (IC 57/58/59) remains in the LLC area.
- No public API change to existing types. Patch bump 0.106.11 →
  0.106.12.

## 0.106.11 - 2026-06-17

- New built-in COSEM object `CosemIec61334432LlcSetupObject`
  (`class_id=55`, `version=1`) implementing **IEC 61334-4-32 LLC
  setup** per IEC 62056-6-2 ED4 (2021) §4.10.7 and DLMS UA Blue
  Book Ed. 12.1 §4.10.7. Exposes attributes
  `2 max_frame_length` (long-unsigned, length of the LLC frame in
  bytes per IEC 61334-4-32:1996 §5.1.4; S-FSK profile
  min/def/max 26/134/242 per IEC 61334-5-1:2001 §4.2.2) and
  `3 reply_status_list` (array of structure {L-SAP-selector,
  length-of-waiting-L-SDU}; MIB variable reply-status-list
  (variable 11) per IEC 61334-4-512:2001 §5.4) as opaque A-XDR
  buffers prepared by the caller. `logical_name` is read-only;
  attributes 2..3 share a caller-selected `AttributeAccessMode`
  so the backend can republish refreshed frame length and reply
  status list after the LLC sublayer accumulates events out-of-
  band. The class defines no specific methods; all method ids
  report `MethodNotFound`. Constructor normalises `version` to
  `MaxSupportedVersion=1` (ED4 deprecates `0`).
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnMethodNotFound`, `NormalizesVersionAboveMax`).
  MinGW64 ctest 988/988 ✅ (was 984/984 in 0.106.10; +4 new
  cases from this IC).
- Docs: `ic_support_matrix.md` row for IC 55 promoted from
  Application-provided to Partial; remaining LLC row narrowed to
  `56`-`59`. With IC 50/51/52/53/55 done, the S-FSK PLC stack is
  feature-complete except for `56 S-FSK Reporting system list`.
- No public API change to existing types. Patch bump 0.106.10 →
  0.106.11.

## 0.106.10 - 2026-06-17

- New built-in COSEM object `CosemSFskMacCountersObject`
  (`class_id=53`, `version=0`) implementing **S-FSK MAC counters**
  per IEC 62056-6-2 ED4 (2021) §4.10.6 and DLMS UA Blue Book
  Ed. 12.1 §4.10.6. Exposes attributes
  `2 synchronization_register` (array of {mac_address,
  synchronizations_counter}), `3 desynchronization_listing`
  (structure), `4 broadcast_frames_counter` (array of
  {mac_address, frames_counter}), `5 repetitions_counter`,
  `6 transmissions_counter`, `7 CRC_OK_frames_counter`,
  `8 CRC_NOK_frames_counter` (all double-long-unsigned) as opaque
  A-XDR buffers prepared by the caller (MIB variables from
  IEC 61334-4-512:2001 §5.8). `logical_name` is read-only;
  attributes 2..8 share a caller-selected `AttributeAccessMode`
  so the backend can republish refreshed counters/listings after
  the S-FSK MAC accumulates events out-of-band. Method `1 reset`
  reports `UnsupportedFeature` and clears method output (counter
  bookkeeping is backend-owned); other method ids report
  `MethodNotFound`. Constructor normalises `version` to
  `MaxSupportedVersion`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnUnsupportedFeature`, `NormalizesVersionAboveMax`).
  MinGW64 ctest 984/984 ✅ (was 980/980 in 0.106.9; +4 new cases
  from this IC).
- Docs: `ic_support_matrix.md` row for IC 53 promoted from
  Application-provided to Partial; remaining LLC row narrowed to
  `55`-`59`. This closes the S-FSK MAC stack alongside the
  already-built IC 50/51/52.
- No public API change to existing types. Patch bump 0.106.9 →
  0.106.10.

## 0.106.9 - 2026-06-17

- New built-in COSEM object `CosemSFskMacSyncTimeoutsObject`
  (`class_id=52`, `version=0`) implementing **S-FSK MAC
  Synchronization Timeouts** per IEC 62056-6-2 ED4 (2021) §4.10.5
  and DLMS UA Blue Book Ed. 12.1 §4.10.5. Exposes attributes
  `2 search_initiator_timeout`, `3 synchronization_confirmation_timeout`,
  `4 time_out_not_addressed`, `5 time_out_frame_not_OK` as opaque
  long-unsigned A-XDR buffers prepared by the caller (MIB variables
  from IEC 61334-4-512:2001 §5.3 and IEC 61334-5-1:2001 §4.3.7.6).
  `logical_name` is read-only; attributes 2..5 share a caller-selected
  `AttributeAccessMode` so the client can re-tune the MAC sub-layer
  timers after commissioning. The class defines no specific methods;
  `InvokeMethod` reports `MethodNotFound` for all method ids and
  clears method output. Constructor normalises `version` to
  `MaxSupportedVersion`.
- 4 new gtests (`ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`, `MethodsReturnMethodNotFound`,
  `NormalizesVersionAboveMax`). MinGW64 ctest 980/980 ✅
  (was 976/976 in 0.106.8; +4 new cases from this IC).
- Docs: `ic_support_matrix.md` row for IC 52 promoted from
  Application-provided to Partial; remaining IC 53..59 row narrowed
  to `53`-`59`.
- No public API change to existing types. Patch bump 0.106.8 → 0.106.9.

## 0.106.8 - 2026-06-17

- Close P1 Transport §4 (buffer reuse). Five per-call hot sites
  converted from local `std::vector<std::uint8_t>` to member
  buffers that are reused across calls (`clear()` + reuse,
  capacity is retained), removing per-frame heap traffic on the
  steady-state read/write path:
  - `WrapperTcpProfileChannel::SendApdu` — send WPDU encode
    buffer now reuses `sendWpdu_`.
  - `WrapperTcpProfileChannel::ReceiveNextFrame` — decoder frame
    list now reuses `decodeFrames_`, decoded frames move into
    `pendingFrames_` (no copy).
  - `WrapperUdpProfileChannel::SendApdu` — reuses `sendWpdu_`.
  - `ServerEndpoint::RunOnce` — request/response APDU buffers
    reuse `requestApdu_` / `responseApdu_`.
  - `GatewayEndpoint::RunOnce` — reuses `requestApdu_` /
    `responseApdu_`.
  - `PushListenerEndpoint::RunOnce` — reuses `apdu_`.
- No public API change. Headers gain one or two private member
  vectors per affected class (PODs append to private section
  only). On-the-wire behaviour and observable trace events are
  unchanged.
- Verified MinGW64 ctest 976/976 ✅ and Linux Release serial
  ctest 1390/1390 ✅. Patch bump 0.106.7 → 0.106.8.

## 0.106.7 - 2026-06-17

- Close P1 Package §4. Package artifact verified on both supported
  release platforms:
  - **Windows / MSYS2 MINGW64** via the existing
    `scripts/verify_release_mingw64.sh`: clean Release build →
    serial ctest 976/976 → CPack tarball.
  - **Linux** via the new `scripts/verify_release_linux.sh`
    (mirror driver, mode 0755): clean Release build → serial
    ctest **1390/1390** → CPack tarball. Toolchain reference:
    Ubuntu 25.10 / WSL2 / gcc 15.2 / cmake 3.31 / ninja 1.12 /
    OpenSSL 3.5.3. Script is location-independent (resolves repo
    root from its own dir) and requires only cmake/ninja/g++/
    libssl-dev.
- Release gate is **serial** ctest on both platforms (matches the
  MinGW64 driver). Parallel `ctest -jN` may surface flakes in a
  small set of TCP loopback / endpoint integration tests under
  kernel-level port-reuse contention (TIME_WAIT pressure). Those
  flakes are test-harness limits, not production regressions —
  documented in the roadmap and reproducible only under heavy
  parallel scheduling. Parallel runs stay dev-only.
- Roadmap (`docs/production_readiness_roadmap.md`) updated: P1
  Package §4 marked ✅ DONE with reference to the new Linux driver
  and the verified test counts.
- No code changes outside scripts and docs. Patch bump 0.106.6 →
  0.106.7 (docs + script only, no public API or behavioural
  change).

## 0.106.6 - 2026-06-17

- Fix `TcpServerTransport.Close()` on POSIX so it actually unblocks a
  concurrent `Accept()`. Previously `Close()` only called `close(fd)`,
  which on Linux does not wake another thread that is already inside
  `select()` / `accept()` on that fd (POSIX leaves the behaviour
  unspecified, and Linux in particular keeps the call blocked). The
  pre-existing `TcpServerTransport.CloseUnblocksAccept` unit test
  surfaced this on the WSL Linux build (subprocess aborted) while
  MinGW64 happened to short-circuit via Winsock semantics.
  - `Close()` now (1) flips `open_ = false` *before* touching the
    socket, so a racing `Accept()` that wakes from `select()` sees
    the closed flag and bails out cleanly instead of calling
    `accept()` on a stale/recycled fd, and (2) issues
    `shutdown(fd, SHUT_RDWR)` before `close(fd)` to force any
    in-flight `select()`/`accept()` to return immediately.
  - New helper `ShutdownNativeSocket()` with POSIX (`SHUT_RDWR`) and
    Win32 (`SD_BOTH`) branches. Win32 path is conservative belt-and-
    braces: Winsock's `closesocket()` already aborts pending
    `accept()`, but the extra `shutdown()` keeps the two platforms
    symmetric and harmless.
  - No public API change. `IByteStream`, `TcpServerTransport`
    surface, options, and TransportStatus enum are untouched.
  - Result: Linux ctest goes 1389/1390 → 1390/1390. MinGW64 stays
    976/976. First time the framework is fully green on both
    platforms in this roadmap pass.
  - Patch bump: behavioural bug fix in a published method, no new
    surface, no removed surface.

## 0.106.5 - 2026-06-17

- Fix latent UB in security unit/integration tests where
  `SecurityContext` was passed as an rvalue temporary to constructors
  that store it by reference (`HlsGmacAuthenticator`,
  `CipheredApduProcessor`). Linux/Clang made the issue visible (9
  fails across `dlms-security`, `dlms-xdlms`, `dlms-client`, and the
  ciphered-get integration); MinGW happened to keep the spilled stack
  slot valid long enough to mask it. The fix is mechanical: hoist
  every `MakeContext(...)` / `MakeSecurityContext(...)` rvalue into a
  named `const SecurityContext` local before passing it to the ctor.
  - Touched test files only: `test_hls_gmac_authenticator.cpp`,
    `test_ciphered_apdu_processor.cpp`, `test_xdlms_security.cpp`,
    `test_client.cpp` (`DlmsClient.InjectedSecurityProtectsGetRequest`
    + `DlmsClient.MapsInjectedSecurityFailure`), and
    `test_ciphered_get_integration.cpp`.
  - Public headers `hls_gmac_authenticator.hpp` and
    `ciphered_apdu_processor.hpp` gain a doc comment on the ctor
    explicitly stating that `context` is stored by reference and
    must outlive the object — and that callers may mutate it
    between calls. This pins the existing endpoint contract
    (`ServerEndpoint` updates `remoteSystemTitle` from the
    incoming AARQ calling-AP-title after the authenticator is
    constructed; the authenticator must observe that update).
  - No ABI change: constructor signatures, member layout, and
    storage-by-reference semantics are unchanged. This is purely
    a test-correctness fix plus a contract clarification.
  - Result: MinGW64 stays 976/976 green; Linux goes from 1381/1390
    to 1389/1390 (the remaining `TcpServerTransport.CloseUnblocksAccept`
    is a pre-existing Linux-only transport issue, unrelated).
  - Patch bump: tests + doc comments only.

## 0.106.4 - 2026-06-17

- P1 «Transport и runtime» §1: TLS-статус зафиксирован как
  documentation-only contract. Framework публикует TLS *adapter slot*,
  но не сам TLS-бекенд — это теперь явный публичный контракт,
  а не побочный эффект отсутствия бекенда.
  - Новый `docs/tls_transport_status.md` — канонический документ:
    что поставляется (`TlsStreamTransportOptions`,
    `ITlsStreamBackend`, `TlsStreamTransport`,
    `UnsupportedTlsStreamBackend`), что не поставляется
    (OpenSSL/mbedTLS/SChannel имплементация, валидация
    сертификатов, cipher policy, IEC 62056-4-7 / Green Book TLS
    profile enforcement), почему такой дизайн (разные целевые
    окружения — от MinGW64 до RTOS+HSM/FIPS модулей), как
    интегрировать свой backend, и failure semantics
    `UnsupportedTlsStreamBackend` (`Open()` → `UnsupportedFeature`,
    lower closed, `ReadSome`/`WriteAll` → `NotOpen`). Явно сказано:
    framework не позиционирует TLS как production-ready surface
    до появления веттед reference backend.
  - `lib/dlms-transport/docs/01_transport_api.md` §7 расширен
    prominent-блоком со ссылкой на новый контракт — чтобы никто
    не выводил production-readiness из самого наличия
    `TlsStreamTransport` в публичных хедерах.
  - Patch bump: docs-only. Код не изменялся, ABI не тронут,
    новых API нет. Контракт выражает реальное состояние репо
    без новых обещаний (AGENTS.md §2).
  - `docs/production_readiness_roadmap.md` P1 Transport §1 marked DONE.

## 0.106.3 - 2026-06-17

- P1 «Transport и runtime» §3: tests для serial edge cases и IEC 62056-21
  Mode E расширены.
  - `lib/dlms-transport/test/transport/test_iec62056_21_mode_e.cpp`: +10
    тестов покрывают ранее непроверенные ветви parser-а identification:
    нет ведущего `/`, отсутствует `\r\n`-терминатор (3 варианта),
    фрейм короче 5 байт, неизвестный baud-код, фрейм без mode-marker,
    маркер не Mode E (`\W1` — Mode B, `\W3` — Mode D), усечённый mode-marker
    `\W` без цифры. Плюс baud-rate negotiation: downgrade к meter
    capability, соблюдение client cap, отклонение ACK build для
    unknown baud (cast из int) с выводом очищенным. Отдельный тест
    эксхаустивно пинит code и value для всех 7 baud-рейтов, чтобы
    будущие расширения enum ломали тест первым.
  - `lib/dlms-transport/test/transport/test_serial_transport.cpp`: +2
    теста. `ZeroSizedIoBeforeOpenStillReturnsNotOpen` пинит контракт
    «NotOpen проверяется до zero-size short-circuit», чтобы
    consumer не был обманут об успехе IO без Open().
    `AcceptsAllLegalDataBitWidths` эксхаустивно проверяет все 4
    легальные ширины 5..8 (валидация проходит, OS-вызов потом
    ожидаемо даёт `OpenFailed`).
  - Полностью pure-unit, без mock и без живого устройства.
  - Patch bump: tests-only. Local MinGW64 ctest — 976/976 зелёных
    (уже без pre-existing red, от 0.106.2).
  - `docs/production_readiness_roadmap.md`: P1 Transport §3 marked DONE.

## 0.106.2 - 2026-06-17

- P1 «Package и consumer experience» §2: exported target audit расширен.
  - Новые asserts в `cmake/PackageInstallSmoke.cmake` пинят
    `DLMSFrameworkConfig.cmake.in` OpenSSL opt-in контракт:
    - Components `protocol`, `cosem_server`, `runtime`, `framework`
      ОБЯЗАНЫ триггерить `find_dependency(OpenSSL)`.
    - Components `codec` и `io` ДОЛЖНЫ оставаться OpenSSL-free.
    - Контракт выводится static-grep-ом по сгенерированному
      Config-файлу, фейлит рано с понятным FATAL_ERROR.
  - Контекст аудита (всё было уже корректным, этот коммит
    фиксирует contracts):
    - Include dirs: 12/12 lib-ов используют
      `$<BUILD_INTERFACE:...>` + `$<INSTALL_INTERFACE:include>`.
    - OpenSSL привязан только к `dlms_security`
      (единственный `find_package(OpenSSL)` в репо).
    - Test-deps (`gtest`, `gmock`, `GTest::`) в export-tree уже
      запрещены smoke-ом.
    - Aggregate `INTERFACE_LINK_LIBRARIES` проверяются smoke-ом с
      `0.4.10`.
    - `codec`-only и `io`-only consumer-билды в smoke фактически
      доказывают положительную часть OpenSSL-free контракта
      (линкуются без OpenSSL на build-хосте).
  - Patch bump: чистый audit + smoke-extension, ни одного API/ABI/
    поведенческого изменения. Local ctest 976/976 зелёные.
  - `docs/production_readiness_roadmap.md`: P1 Package §2 marked ✅ DONE.

## 0.106.1 - 2026-06-17

- P2 «Надежность и оптимизация» §5 (частично): overflow guards для
  size calculations.
  - Аудит выявил 4 сайта в dlms-security, где OpenSSL EVP_*Update/Final
    возвращает два `int`, которые складывались как `int + int`
    перед кастом в `std::size_t`:
    - `hls_high_authenticator.cpp:54` (Aes128EcbEncrypt)
    - `suite0_aes_gcm.cpp:148` (Seal) + `:225` (Open)
    - `suite0_key_wrap.cpp:95` (size-equality check)
  - Формально это signed-integer addition, который UBSan пометил бы
    как `signed-integer-overflow` UB при близких к `INT_MAX` входах
    (практически не достижимо на реальных APDU, но UBSan-чистота
    важна для нового CI job).
  - Fix: каждый `int` сначала кастится в `std::size_t`, и только
    потом складывается. Контракт OpenSSL гарантирует `>= 0`
    для обоих, поэтому семантика не меняется.
  - Остальные size-арифметики в фреймворке уже имеют guards:
    `ber.cpp` `result > size_t::max() >> 8`, `llc_codec::HasRepresentable
    LpduSize`, `apdu_c_api` `payload_size == size_t::max()`,
    `in_memory_invocation_counter_store::nextLocal_ == uint32_t::max()`,
    `suite0_aes_gcm::SizeFitsInt`. Reserve-сайты в association/ciphered/
    hls_gmac/xdlms_client складывают фиксированные мелкие
    константы с валидированными wire-размерами — оверфлоу потребовал
    бы 16 ЭБ памяти.
  - Patch bump: внутренние фиксы, ни одного API/ABI/поведенческого
    изменения. Local ctest 976/976 зелёные. Новый
    `linux-sanitizers` CI job (P2 §2) будет обнаруживать
    регрессии в этой категории.
  - `docs/production_readiness_roadmap.md`: P2 §5 marked ✅ DONE (частично).

## 0.106.0 - 2026-06-17

- P2 «Надежность и оптимизация» §2: sanitizers в CI.
  - New CMake option `DLMS_SANITIZE` (`none` | `address` | `undefined` |
    `address,undefined`, default `none`). When set, applies
    `-fsanitize=<list>` and `-fno-omit-frame-pointer` to every
    compile and link in the build tree; validates that the compiler is
    Clang/AppleClang/GCC and fails early with a readable
    `FATAL_ERROR` otherwise. Default `none` keeps existing builds
    untouched (verified locally on MinGW64: 976/976 ctest still green).
  - New `scripts/verify_sanitizers_linux.sh`: Linux clang, Debug,
    `DLMS_SANITIZE=address,undefined`, `DLMS_INSTALL=OFF`. Sets
    `ASAN_OPTIONS`/`UBSAN_OPTIONS`/`LSAN_OPTIONS` for symbolized
    halt-on-error and runs the full ctest excluding
    `dlms_package_(install|artifact)_smoke` (those configure a separate
    consumer build that does not inherit sanitizer flags; install/
    artifact paths are still validated by the existing MinGW64 release
    job).
  - New GitHub Actions job `linux-sanitizers` (ubuntu-latest, clang +
    cmake + ninja-build + libssl-dev) runs the script on every push
    and pull request. The existing MinGW64 release job is untouched
    and remains responsible for package install/artifact verification
    and tag-driven release publishing.
  - Pure additive: no public API or runtime behavior changes when
    `DLMS_SANITIZE=none` (the default). Minor bump because the public
    CMake option surface gained a new toggle.
  - `docs/production_readiness_roadmap.md`: P2 §2 marked ✅ DONE.

## 0.105.2 - 2026-06-17

- Docs-only: new `docs/package_consumer_minimum.md` documents the
  minimum CMake snippet and `#include` set for every public
  aggregate target (`dlms::codec`, `dlms::io`, `dlms::protocol`,
  `dlms::cosem_server`, `dlms::runtime`, `dlms::framework`). For each
  aggregate the document lists the transitive components, the
  OpenSSL dependency status, a minimum compilable `main.cpp`, and a
  link to the runnable example under `examples/package-consumers/`.
  The snippets are pinned to reality by `dlms_package_install_smoke`,
  which already builds all six aggregate examples on every CI run.
- `docs/production_readiness_roadmap.md`: P1 «Package и consumer
  experience» §3 marked ✅ DONE.

## 0.105.1 - 2026-06-17

- Test harness fix (`cmake/PackageInstallSmoke.cmake`): the install
  smoke check used to hard-require
  `DLMSFrameworkTargets-noconfig.cmake`, which is only emitted by
  multi-config generators (Visual Studio, Xcode) when no build type
  is selected. Single-config generators (Make, Ninja) emit a per-
  config file instead — `DLMSFrameworkTargets-debug.cmake` for Debug,
  `-release.cmake` for Release, etc. The harness now globs for
  `DLMSFrameworkTargets-*.cmake` and accepts the first per-config
  file it finds, asserting only that the produced CMake package
  contains the documented files (`Config`, `Targets`, and at least
  one per-config Targets file). This unblocks `dlms_package_install_smoke`
  on MinGW + Ninja Debug builds without weakening the contract.
- Full ctest: 976/976 passing (first fully green run).

## 0.105.0 - 2026-06-17

- Feature (P1 §7 commit 3c — end-to-end `conversationId` on the
  server-dispatch trace):
  - `TracingXdlmsServerDispatcher` gained an optional back-reference
    to an `dlms::profile::IApduChannel` via
    `SetCorrelationChannel(IApduChannel*)` /
    `CorrelationChannel()`. When set, emitted
    `ServerDispatchTraceEvent`s carry the `conversationId` currently
    latched on the channel (read via `IApduChannel::CurrentConversationId()`
    added in 0.103.0). When unset (default), events keep using
    `kNoConversationId == 0`. The decorator only ever reads the id —
    it never sends or receives APDUs through the channel reference.
  - `ServerEndpoint::ConfigureXdlmsProcessor` and both
    `GatewayEndpoint` constructors now install the inbound APDU
    channel both on the `XdlmsServerApduProcessor` (via
    `SetApduChannel`) and on the `TracingXdlmsServerDispatcher`
    (via `SetCorrelationChannel`). This closes the gap left by
    0.102.0/0.104.0: the GET/SET/ACTION dispatch event now publishes
    the same conversation id that the xDLMS request/response trace
    and the transport-layer trace already see.
- Tests: new integration test
  `test/integration/test_endpoint_trace_correlation.cpp`
  (`dlms_endpoint_trace_correlation_tests`) drives one GET through
  `ServerEndpoint` with a fake `IApduChannel` and asserts that the
  same non-zero `conversationId` appears on:
  - the channel (`SetCorrelation` + `CorrelationAtSend`),
  - both `IXdlmsTraceSink` events (`RequestReceived` + `ResponseSent`),
  - the single `IServerDispatchTraceSink` `GetDispatched` event.
- Full ctest: 975/976 passing (the lone failure,
  `dlms_package_install_smoke`, is a pre-existing MinGW-only
  multi-config artefact unrelated to this commit).

## 0.104.0 - 2026-06-17

- Feature (P1 §7 commit 3b — end-to-end `conversationId` propagation
  through the server-side xDLMS stack):
  - `XdlmsServerApduProcessor` now computes a `conversationId` per
    inbound request from `MakeConversationId(seedSource_->ConversationSeed(),
    invokeId)` immediately after `DecodeXdlmsApdu` succeeds, and stamps
    it on every emitted `IXdlmsTraceSink` event for that request:
    `RequestReceived`, `ResponseSent`, the inner `InvokeIdRejected` /
    `BlockTransferStep` events emitted from `ProcessGetRequest` /
    `ProcessSetRequest` / `ProcessActionRequest`, and the post-decode
    `SecurityFailed` (Protect failure). Events emitted before the
    request invoke id is known (Unprotect-failure `SecurityFailed`,
    `DecodeFailed`) continue to carry `kNoConversationId` (0).
  - When an `IApduChannel*` has been installed via
    `SetApduChannel(...)`, the processor calls
    `channel_->SetCorrelation(conversationId)` right after the id is
    computed, mirroring `XdlmsClient`'s pattern. This gives the
    transport-layer trace sinks (`HdlcProfileChannel`,
    `WrapperTcpProfileChannel`) a stable id to stamp on every
    outbound response APDU emitted later in the request lifecycle.
  - Helper signatures `EmitServerTrace` / `EmitServerSimpleTrace`
    gained a trailing `std::uint64_t conversationId` parameter and the
    three `Process*Request` helpers gained a trailing
    `std::uint64_t conversationId` parameter — both are internal-only,
    no public API impact beyond `XdlmsServerApduProcessor`'s ctor
    initializers (which already shipped in 0.102.0).
- Tests: extended `lib/dlms-xdlms/test/xdlms/test_xdlms_server_trace.cpp`
  with two new cases:
  - `ServerStampsConversationIdOnEventsAndChannel`: installs a
    `FixedSeedAssociation` (seed=0xA5A5…) and a
    `CorrelationCapturingChannel`, sends a Get request with
    `invokeId=0x01`, and asserts that both emitted trace events plus
    the channel's last `SetCorrelation` value all equal
    `MakeConversationId(seed, 0x01)`.
  - `ServerZeroSeedYieldsInvokeIdAsConversationId`: pins the
    documented `MakeConversationId(0, invokeId) == invokeId & 0x0F`
    property for the server path (matching the client-side
    `CrossLayerCorrelation.ZeroSeedYieldsZeroConversationId` test).
  - `lib/dlms-xdlms/test/CMakeLists.txt` now links `dlms_xdlms_tests`
    against `dlms_profile` so the `IApduChannel` stub compiles.
  - Full ctest: 974/975 passing (the lone failure,
    `dlms_package_install_smoke`, is a pre-existing MinGW-only
    multi-config artefact unrelated to this commit).

## 0.103.0 - 2026-06-17

- Feature (P1 §7 follow-up enabler — `IApduChannel::CurrentConversationId()`):
  - New `virtual std::uint64_t CurrentConversationId() const noexcept`
    on `dlms::profile::IApduChannel`, appended at the end of the
    interface (ABI-safe addition, default returns `0u`). Reads back the
    correlation id most recently installed via `SetCorrelation(...)`,
    so server-side trace decorators (notably
    `TracingXdlmsServerDispatcher`, to be wired in a later commit) can
    publish the conversation id that the xDLMS processor seeded for the
    current request without having to thread the id through the
    dispatcher API.
  - Implemented in `HdlcProfileChannel` and `WrapperTcpProfileChannel`
    (returns the stored `conversationId_`). Other channels inherit the
    default `0u` and keep working unchanged.
  - No behavioural change to any existing code path; this is purely a
    new read-only accessor.

## 0.102.0 - 2026-06-17

- Feature (P1 §7 commit 3a — server-side dispatch trace sink in
  `dlms-server`, plus endpoint wiring for xDLMS + dispatch sinks):
  - New header `dlms/server/server_dispatch_trace.hpp` declares
    `IServerDispatchTraceSink` and `ServerDispatchTraceEvent` (with
    `ServerDispatchTraceKind::{GetDispatched, SetDispatched,
    ActionDispatched}`). Kept in `dlms-server` rather than
    `dlms-xdlms` so `dlms-xdlms` does not pick up a dependency on
    server-layer types — matches the "two sinks, not one" decision
    in `docs/xdlms_server_trace_design.md`.
  - New header `dlms/server/tracing_xdlms_server_dispatcher.hpp`
    declares `TracingXdlmsServerDispatcher` — a thin pass-through
    decorator around any `dlms::xdlms::IXdlmsServerDispatcher` that
    emits one `ServerDispatchTraceEvent` per `DispatchGet` /
    `DispatchSet` / `DispatchAction`. Zero-cost when the sink is
    `nullptr` (no event object constructed, no virtual call).
  - `ServerEndpointOptions` gains two opt-in pointer fields:
    `xdlms::IXdlmsTraceSink* xdlmsTraceSink = nullptr` and
    `server::IServerDispatchTraceSink* serverDispatchTraceSink =
    nullptr`. `GatewayEndpointOptions::downstream` (a
    `ServerEndpointOptions`) inherits the same two fields and they
    are now honoured by both `ServerEndpoint::ConfigureXdlmsProcessor`
    and `GatewayEndpoint`'s downstream construction. Defaults
    (`nullptr`) preserve existing behaviour: no events emitted, no
    decorator overhead.
  - `PushListenerEndpointOptions` and `ClientEndpointOptions` are
    deliberately **not** extended in this commit: the push-listener
    side does not run a dispatcher, and the client side already has
    `XdlmsClient::SetTraceSink` available for embedded code (proper
    `DlmsClient`/`ClientEndpoint` plumbing of `xdlmsTraceSink` will
    follow as its own commit once it has a real consumer — AGENTS.md
    §2: no speculative API).
  - Redaction contract identical to existing sinks: events publish
    only sizes and identifiers (invoke id, class id, attribute or
    method id, OBIS LN, request and response payload sizes, xDLMS
    status, conversation id). The `conversationId` field is
    currently always `kNoConversationId` (0); end-to-end correlation
    on the server side is the subject of §7 commit 3b together with
    the integration test.
  - Version bumped 0.101.0 → 0.102.0 (minor — additive public API
    on `dlms-server` and `dlms-endpoint`, no ABI / semantic break).
  - Tests: full suite 971/973 functional pass (the two remaining
    failures, `dlms_changelog_check` and
    `dlms_package_install_smoke`, are closed by this entry).

## 0.101.0 - 2026-06-17

- Feature (P1 §7 commit 2/3 — xDLMS server trace emission):
  `XdlmsServerApduProcessor` gains opt-in
  `SetTraceSink(IXdlmsTraceSink*) noexcept` /
  `TraceSink() const noexcept`, mirroring the client-side surface
  shipped in 0.100.0. The processor reuses the same
  `IXdlmsTraceSink` interface defined in
  `lib/dlms-xdlms/include/dlms/xdlms/xdlms_trace.hpp`, with
  `XdlmsTraceDirection::Inbound` on every server-emitted event. When
  the sink is unset (default), no event object is constructed and no
  virtual call is made — zero-cost on the hot path.

  **Emission sites in `XdlmsServerApduProcessor::ProcessRequest`:**
  - `SecurityFailed` — when `IXdlmsSecurityProcessor::Unprotect` of
    the request or `Protect` of the response returns non-Ok.
  - `DecodeFailed` — when `DecodeXdlmsApdu` rejects the plain
    request.
  - `RequestReceived` — once per successfully decoded APDU, carrying
    invoke id, parsed `ServiceOptions`, class id, attribute or method
    id, and 6-byte OBIS LN for Get / Set / Action.
  - `ResponseSent` — once per successful response, carrying the
    final (post-`Protect`, if ciphered) `apduSize`.
  - `InvokeIdRejected` — emitted from the per-service block
    transfer helpers (Get-next, Set continuation, Action
    continuation) when the inbound APDU's invoke id does not match
    the active block state.
  - `BlockTransferStep` — emitted after each successful encode of
    `SendNextGetResponseBlock` / `EncodeSetBlockAckResponse` /
    `EncodeActionNextPblockResponse`, carrying the block number and
    response APDU size.

  Events publish only sizes and identifiers — never raw APDU bytes,
  plaintext payload, keys, system titles, GMAC tags, or HLS
  challenges, matching the 0.100.0 redaction contract. The
  `conversationId` field on server-side events is currently always
  `kNoConversationId` (0); endpoint composition in §7 commit 3/3
  will publish the correlation id from the listening side.

  **Tests.** New `lib/dlms-xdlms/test/xdlms/test_xdlms_server_trace.cpp`
  adds 3 cases (`ServerEmitsRequestReceivedAndResponseSentForGet`,
  `ServerEmitsDecodeFailedOnGarbageInput`,
  `ServerWithoutSinkProcessesNormally`) that pin the happy-path
  Request/Response pairing, the malformed-input early-emit, and the
  zero-cost null-sink contract. Full ctest: 970 → 973.

  **Minor bump** (0.100.0 → 0.101.0) because two new public members
  (`SetTraceSink`, `TraceSink`) are added to a non-final class. ABI
  is preserved: no virtuals, no member reorder, no removed symbol —
  the new `IXdlmsTraceSink* traceSink_` field is appended last and
  initialized to `0` in all six constructors. Pre-existing callers
  that never touch the sink see no behavioural change.

  Out of scope (deferred to §7 commit 3/3): `IServerDispatchTraceSink`
  for `XdlmsServerDispatcher` and endpoint-level pass-through
  (`EndpointOptions::xdlmsTraceSink` and
  `serverDispatchTraceSink`).

## 0.100.0 - 2026-06-17

- Feature (P1 §7 commit 1/3 — xDLMS client trace + §2 wiring closure):
  new public xDLMS-layer trace surface plus end-to-end cross-layer
  correlation propagation. Two changes that share infrastructure and
  ship together:

  **xDLMS trace sink (§7, client side).** New header
  `lib/dlms-xdlms/include/dlms/xdlms/xdlms_trace.hpp` defines
  `IXdlmsTraceSink`, `XdlmsTraceEvent` (kind, direction, status, invoke
  id, options, class id, attribute/method id, OBIS LN, optional block
  number, APDU size, payload size, `conversationId`), `XdlmsTraceKind`
  (12 kinds covering Get/Set/Action request/response, block transfer
  step, request received, response sent, decode failure, invoke-id
  mismatch, security failure) and `XdlmsTraceDirection`. Events publish
  only sizes and identifiers — never raw APDU bytes, keys, system
  titles, GMAC tags, or HLS challenges. Same lifecycle / re-entrancy /
  no-throw / span-validity contract as the 4 existing sinks (see
  `docs/trace_contracts.md`). `XdlmsClient` gains opt-in
  `SetTraceSink(IXdlmsTraceSink*) noexcept` /
  `TraceSink() const noexcept` (additive, no new ctor overloads); when
  unset, behaviour is identical to before. Client-side emission is wired
  through `Get`, `Set` (normal + block-transfer), `Action` (normal +
  block-transfer): per-service `Request`/`Response`/`DecodeFailed`/
  `SecurityFailed` events from a single `SendAndReceive` site, plus
  `BlockTransferStep` events in the per-service block loops and
  `InvokeIdRejected` events on every invoke-id mismatch check.

  **Cross-layer correlation wiring (§2 closure).** `XdlmsClient::Get/
  Set/Action` now also call
  `channel_.SetCorrelation(MakeConversationId(seed, invokeId))` on the
  bound `IApduChannel` immediately after allocating the invoke id and
  before the first `SendApdu`. The seed comes from a new
  `IXdlmsAssociationState::ConversationSeed() const noexcept` virtual
  whose default returns `0` (ABI-safe: existing association
  implementations continue to return `0`, which yields
  `conversationId == kNoConversationId`, which is exactly the pre-0.99.6
  channel behaviour). Together with the channel-side stamping shipped
  in 0.99.6, this means xDLMS sink and channel sinks see bit-identical
  `conversationId` for the same service call — no manual plumbing in
  consumer code, no "who owns the id" coordination across layers.

  **Tests.** New integration fixture
  `test/integration/test_cross_layer_correlation.cpp` adds 3 cases
  (`GetEmitsMatchingConversationIdOnBothLayers`,
  `ZeroSeedYieldsZeroConversationId`,
  `NoTraceSinkStillCallsSetCorrelation`) that pin the invariant
  end-to-end through a custom `IApduChannel` capturing `SetCorrelation`
  and stamping a `WrapperTcpTraceEvent` with the captured id, alongside
  an `IXdlmsTraceSink` that captures the xDLMS-side events. Full ctest:
  967 → 970.

  Out of scope here (deferred to follow-up commits): server-side xDLMS
  trace emission through `XdlmsServerApduProcessor` (§7 commit 2/3),
  the `IServerDispatchTraceSink` and dispatcher wiring (§7 commit 3/3),
  and endpoint composition (`EndpointOptions::xdlmsTraceSink` +
  `serverDispatchTraceSink` pass-through). Also out of scope:
  `AssociationClient` still returns `ConversationSeed() == 0`, so real
  deployments still see `kNoConversationId` until a future change
  publishes a non-zero seed — at which point the wiring above will
  automatically carry it through every sink without further channel or
  client changes.

## 0.99.7 - 2026-06-17

- Docs (P1 §7 design phase): `docs/xdlms_server_trace_design.md`
  fixes the contracts for two new opt-in trace sinks that close the
  visibility gap called out by `docs/trace_contracts.md`:
  `IXdlmsTraceSink` (xDLMS layer, client + server) and
  `IServerDispatchTraceSink` (server-side dispatch). Two sinks, not
  one, so `dlms-xdlms` doesn't take a dependency on `dlms-server`
  types. Events publish only sizes and identifiers (class id,
  attribute/method id, OBIS LN, invoke id, sizes), never raw APDU or
  payload bytes — no new redaction obligation on consumers. Same
  lifecycle / no-throw / no-reentry / span-validity discipline as the
  4 existing sinks. Both events carry `conversationId` so events
  stitch with the §2 correlation work. 3-commit implementation plan
  laid out. No production code in this commit.

## 0.99.6 - 2026-06-17

- Feature (P1 §2 commit 3/3): `WrapperTcpProfileChannel` and
  `HdlcProfileChannel` now override `SetCorrelation(uint64_t)` and
  stamp the active `conversationId` onto every emitted trace event
  (`WireWrite`, `WireRead`, `ReadStatus`, `DecodeStatus` for both
  channels). Default conversation id stays `kNoConversationId = 0`, so
  consumers that never call `SetCorrelation` see exactly the same
  events as before. End-to-end test fixture extended with 2 new cases
  (`WrapperTcpChannelStampsSendAndReceiveEvents`,
  `HdlcChannelStampsWireWriteEvents`) that drive the channels through
  `FakeByteStream`, flip `SetCorrelation` on and off, and pin every
  event in the capture window to the expected id. Test fixture total:
  9 cases; full ctest unchanged at 967/967.

  Out of scope for this commit (intentionally deferred to a follow-up):
  AssociationClient does not yet generate or propagate a conversation
  id — once it does, the same `SetCorrelation` plumbing will carry it
  into trace consumers without any additional channel changes.

## 0.99.5 - 2026-06-17

- Feature (P1 §2 commit 2/3): wired the cross-layer correlation field
  through the four trace event structs and the apdu channel interface.
  Append-only changes, default `0` everywhere, no ABI break in 0.x:
  - `TransportTraceEvent`: new `conversationId` member, initialised to
    `0` in the existing user-defined default ctor.
  - `WrapperTcpTraceEvent`, `HdlcProfileTraceEvent`,
    `AssociationTraceEvent`: new `conversationId` member as the last
    field, default member initialiser `= 0` so aggregate `Event{}` and
    `Event ev; ev = {};` keep producing zero-context events.
  - `IApduChannel::SetCorrelation(uint64_t) noexcept`: new virtual
    method with default no-op body. Channels that do not emit traces
    do not need to override.
  New test fixture `test_trace_correlation.cpp` (7 cases, lives in
  `dlms-profile/test`) pins: every event default-inits its
  `conversationId` to `kNoConversationId = 0`; assigning the field
  round-trips; default `SetCorrelation` is reachable through the base
  interface, accepts any `uint64_t`, is `noexcept`; overriding channels
  receive the value verbatim. Per-test enumeration: 7 new gtest cases
  inside `dlms_profile_tests.exe` (ctest still sees one aggregate
  entry); ctest summary unchanged at 967/967.

## 0.99.4 - 2026-06-17

- Feature (P1 §2 commit 1/3): added `dlms/xdlms/xdlms_correlation.hpp` —
  header-only `constexpr noexcept MakeConversationId(seed, invokeId)`
  primitive plus `kNoConversationId` sentinel. Formula:
  `(seed & ~0x0F) | (invokeId & 0x0F)` — the low nibble preserves the
  human-readable invoke-id; the high 60 bits separate associations.
  Zero-impact addition: no existing code calls it yet, no ABI change,
  pure header. New test fixture `test_xdlms_correlation.cpp` (9 cases)
  pins the invariants: low-nibble round-trip, high-bits = seed high
  bits, invoke-id high nibble ignored, distinct seeds → distinct ids,
  distinct invoke-ids → distinct ids, seed low nibble cleared not
  mixed, constexpr + noexcept, sentinel = 0, seed=0/invoke=0 collapses
  to sentinel (documented edge case). ctest 958 → 967.

## 0.99.3 - 2026-06-17

- Docs (P1 «Диагностика» §2 design phase): added
  `docs/trace_correlation_design.md` — accepted design for cross-layer
  trace correlation. Introduces a single opaque non-secret 64-bit
  `conversationId` carried by every trace event in all four sinks
  (`TransportTraceEvent`, `WrapperTcpTraceEvent`,
  `HdlcProfileTraceEvent`, `AssociationTraceEvent`). The id originates
  in the xDLMS layer (as `(association_seed & ~0x0F) | invoke_id`,
  preserving the low nibble for human readability) and propagates down
  to the profile/transport sinks through a new no-op
  `IApduChannel::SetCorrelation()` virtual. The seed is a per-association
  logging salt with no security role; nothing about correlation appears
  on the wire. Design respects the existing ABI rules (POD append
  at end of struct, virtual default no-op, zero-init = no correlation
  context). Three follow-up commits planned: primitive +
  `MakeConversationId`; struct extension + default virtual; wiring +
  integration test. Docs-only; no code change in this version.

## 0.99.2 - 2026-06-17

- Docs (P1 «Диагностика» §6 closed): added
  `docs/status_to_string_contract.md` — the single canonical contract
  for every `*StatusName` / `ToString` helper in the stack (13 status
  enum across 13 modules). Documents totality, `"Unknown"` fallback,
  lifetime (static storage), thread-safety, no-allocation, and ABI
  stability guarantees, plus the full catalogue of helpers and the
  matching exhaustive-coverage test files. Each per-module
  `01_*_api.md` now ends with a short "Diagnostic helpers" section
  that names the module’s helper and links back to the central
  contract. Also records the `EndpointStatus::ToString` /
  `TransportStatus::ToString` naming inconsistency (vs the
  `*StatusName` form everywhere else) as P1 §5 follow-up; both names
  will continue to work until a major bump introduces an
  alias-and-deprecate. No code change; docs-only.

## 0.99.1 - 2026-06-17

- Docs (P1 «Диагностика» §1 closed): added `docs/trace_contracts.md`,
  the audit map of every public trace sink in the stack
  (`ITransportTraceSink`, `IWrapperTcpTraceSink`, `IHdlcProfileTraceSink`,
  `IAssociationTraceSink`). For each sink the doc records the layer,
  defining header, event struct, whether raw bytes are carried, the
  security semantics (wrapper/hdlc carry the literal protected APDU —
  treat as secret; association carries only sizes), framework guarantees
  (lifecycle, re-entrancy, byte-span validity, no-exception-escape), and
  consumer responsibilities (redaction, thread safety, backpressure,
  filtering). Also makes the endpoint pass-through pattern explicit and
  records two known gaps (no public xDLMS-layer or server-side trace
  sink) as roadmap §7 follow-ups. No code change; audit-only.

## 0.99.0 - 2026-06-17

- API / diagnostics (P1 “Диагностика” §3 status-to-string
  completeness, etap 2): added public `HdlcStatusName(HdlcStatus)`,
  `LlcStatusName(LlcStatus)`, `WrapperStatusName(WrapperStatus)`, and
  `ProfileStatusName(ProfileStatus)` helpers that return the enum-value
  identifier as a `static`-storage C string (`"Ok"`, `"NeedMoreData"`,
  ...) or `"Unknown"` for out-of-range casts. With this every public
  status enum in `dlms::*` now exposes a stable, exhaustive `Name()`
  helper for diagnostics, logs, and cross-layer error propagation. Each
  implementation is a plain switch with no `default:` arm so future enum
  additions will fire `-Wswitch` until a string is wired. Adds 8 new
  `*StatusName` coverage tests (2 per module) covering every enum value
  plus the Unknown fallback; full ctest 958/958.
- Internal cleanup: `tools/live_meter_smoke.cpp` had a private
  `ProfileStatusName` copy from before the public helper existed; it has
  been removed and the two call sites now use the public
  `dlms::profile::ProfileStatusName`.

## 0.98.3 - 2026-06-17

- Tests / status hygiene (P1 "Диагностика" §3 status-to-string
  completeness, etap 1): added exhaustive `*StatusName` /
  `ToString` coverage tests for `ApduStatus`, `AssociationStatus`,
  `CosemStatus`, `ServerStatus`, `XdlmsStatus`, and brought
  `EndpointStatus` test up to exhaustive (covered 12/12 values
  instead of the previous 4/12). Each new test pins the stable
  string form for every enum value plus the `Unknown` fallback.
  Also dropped a stale `default: return "Unknown";` from
  `endpoint_status.cpp` so `-Wswitch` will fire if a future
  `EndpointStatus` value forgets a `ToString` arm (consistent
  with the same hygiene applied to status mappers in 0.97.1 /
  0.97.2). Test-only behavioural change is a single warning-on-
  miss surface; full ctest 952/952 (942 → 952, +10 new cases).

## 0.98.2 - 2026-06-17

- Tests / C ABI: every public `*_c_api.h` header now has a
  dedicated C-only smoke executable that links and runs from a
  pure-C TU, closing P0 §1.4 "Проверить все C headers smoke
  tests". Previously six of the seven `test_*_c_header.c` files
  were just compiled into their C++ gtest binary (compile-only
  canary, never invoked); only `dlms_association_c_header_smoke`
  ran as a real C-only executable. Now each of the seven
  modules (`apdu`, `association`, `hdlc`, `llc`, `profile`,
  `transport`, `wrapper`) gets `dlms_<mod>_c_header_smoke`,
  registered as `<Module>CApi.CHeaderCompilesAsC` in ctest, that
  actually calls the C smoke function at runtime.
  The existing `test_*_c_header.c` files keep their original
  function signatures so the corresponding C++ gtest cases that
  already linked against them continue to work unchanged; a
  thin `test_*_c_header_main.c` companion supplies the `main()`
  for the smoke executable. Test-only change, no library-side
  impact; full ctest 942/942 (936 → 942).

## 0.98.1 - 2026-06-17

- Tests / endpoint: added three idempotency regression tests
  covering `Open()` after a failed `Open()` for the remaining
  server-side endpoints, closing the second half of P0 §2.3
  "regression tests для cleanup при неуспешном
  open/association":
  - `ServerEndpoint::OpenAfterFailedOpenIsIdempotentAndRetries`
  - `PushListenerEndpoint::OpenAfterFailedOpenIsIdempotentAndRetries`
  - `GatewayEndpoint::OpenAfterFailedDownstreamOpenIsIdempotentAndRetries`

  Each test pins: a failed channel `Open()` returns the mapped
  status and leaves the endpoint closed; `Close()` after a failed
  open is a no-op; a second `Open()` actually re-invokes the
  channel (no stale short-circuit); clearing the failure lets the
  next open succeed without any residual state. The gateway test
  additionally pins that the upstream is *not* opened when the
  downstream fails first.
- Test hygiene / endpoint: the three test-only `FakeApduChannel`
  fixtures in `test_server_endpoint.cpp`,
  `test_push_listener_endpoint.cpp`, `test_gateway_endpoint.cpp`
  were setting `open = true` unconditionally in `Open()`, even
  when `openStatus` was non-`Ok`. Now they set
  `open = (openStatus == Ok)` to match real channel semantics,
  aligning with how `test_listener_runtime.cpp`'s fake already
  behaved. Test-only change, no library-side impact; full ctest
  936/936 confirms no existing test depended on the broken
  behaviour.

## 0.98.0 - 2026-06-17 (BREAKING bugfix)

- BREAKING fix / endpoint: `ClientEndpoint::Close()` now always
  drops the owned `DlmsClient` instance, even when the underlying
  `client->Close()` returned a non-`Ok` status. Previously a failed
  close left the stale instance attached to the endpoint, which
  meant the next `Open()` could either short-circuit on stale
  `IsAssociated()` or silently leak the old instance via
  `move`-assignment inside `CreateClient()`. The status returned by
  `Close()` is unchanged; only the post-condition is stricter.
  The behaviour change is observable, hence the minor bump.
- Tests / endpoint: added four regression tests in
  `lib/dlms-endpoint/test/endpoint/test_client_endpoint.cpp`
  covering the cleanup contract for failed `Open()` /
  `Associate()` paths:
  - `OpenAfterFailedOpenIsIdempotentAndRetries` - a failed open
    leaves no stale state and a second open retries.
  - `CloseAfterFailedOpenLeavesNoStateBehind` - repeated
    `Close()` after a failed open is always `Ok`, and services
    return `InvalidState`, not `InternalError`.
  - `OpenIsIdempotentAfterValidationFailure` - validation
    failures (bad host) before any network use are repeatable
    and leave the endpoint closed.
  - `DestructorClosesAfterFailedOpenWithoutLeak` - destructor
    must not throw / abort after a failed open; loop-runs the
    pattern three times for ASan to catch leaks if rerun there.
  Closes the first half of P0 §2.3 "regression tests для
  cleanup при неуспешном open/association".

## 0.97.7 - 2026-06-17

- Docs / client: added §1.1 “Facade Status Mapping Policy”
  to `lib/dlms-client/docs/01_client_api.md` that pins what
  the public `ClientStatus` enum preserves vs. what it
  intentionally drops, after the `0.96.0`–`0.97.3`
  exhaustive-`switch` audit. Documents the
  per-direction send/receive split, the new first-class
  `BlockTransferRequired` / `InvokeIdMismatch` /
  `CodecFailed` values, the deliberate collapse of
  per-layer transport detail / service-rejected reasons /
  security sub-classification / COSEM access-result, and
  reserves `InternalError` for facade-owned invariants only.
  Also lists every mapper site (`client.cpp`,
  `client_data.cpp`, `client_endpoint.cpp`,
  `server_status.cpp`) and the testing entry point in
  `test_client_internal.cpp`. Closes the second bullet of
  P0 §1.3 “status mapping”. Also refreshed the enum listing
  in the same doc to include the three values added in
  `0.97.0`.

## 0.97.6 - 2026-06-17

- Docs / security: added §5.1 “Storage, Ownership and
  Lifetime” to `lib/dlms-security/docs/01_security_api.md`,
  pinning the contract callers must honour for `IKeyStore`,
  `IMutableKeyStore` and `IInvocationCounterStore`. Covers
  ownership (caller owns, store outlives every processor /
  authenticator), lifetime (per-process or per-association
  both acceptable), storage backends (TPM/HSM/keyring/
  encrypted blob; no in-tree production in-RAM key store),
  invocation-counter persistence (monotonic across restarts;
  reserve-window pattern; remote high-water mark per system
  title), reset semantics (`ResetAfterKeyRotation` atomic
  with `SetKey` from the caller’s perspective; matches the
  in-tree `simple_objects` global-key-transfer path), and
  thread safety (caller-side responsibility;
  `InMemoryInvocationCounterStore` is not internally
  synchronised). Closes the last open bullet of P0 §3.6.

## 0.97.5 - 2026-06-17

- Tools / security: extracted the wire-byte hex dump policy
  for `tools/live_meter_smoke` into
  `tools/live_meter_smoke_byte_emit.hpp` so the redaction
  rule (default off, opt-in via
  `DLMS_LIVE_TRACE_WIRE_BYTES=1`) lives in one inline header
  used by both the tool and the new test target.
- Added `dlms_live_meter_smoke_redaction_tests` (10 cases)
  pinning the policy: wire payload omitted by default, only
  emitted on the explicit env flag, non-wire trace kinds and
  empty byte spans stay silent regardless. Test target is
  gated behind `DLMS_BUILD_LIVE_TESTS=ON` to match the
  existing `LiveMeterSmoke` opt-in. Closes the
  belt-and-braces follow-up promised in `0.97.4`.

## 0.97.4 - 2026-06-17

- Tools / security: `tools/live_meter_smoke` no longer dumps
  on-wire bytes from `OnWrapperTcpTrace` / `OnHdlcProfileTrace`
  unless the operator explicitly sets
  `DLMS_LIVE_TRACE_WIRE_BYTES=1`. Previously the smoke tool
  printed raw `event.bytes` as hex whenever
  `DLMS_LIVE_TRACE=1`, which leaked HLS challenges, GMAC tags,
  and any ciphered/clear protected APDU payload to the
  operator console. Trace metadata (kind, direction,
  status, ports, encoded/apdu sizes, byte count) is still
  printed under `DLMS_LIVE_TRACE=1`; only the byte payload
  itself is gated. Closes P0 §3.6 trace-redaction concern for
  the live smoke tool.

## 0.97.3 - 2026-06-17

- Docs / audit: P0 §2.4 “bounded loops do not swallow Timeout /
  Closed / InvalidState” marked as audited. Every `for(;;)` and
  `while(...)` loop in `lib/dlms-*` was reviewed:
  - Transport-facing loops in `dlms-profile` (HDLC and Wrapper TCP
    channel `Receive*` paths) propagate any non-`Ok` `ProfileStatus`
    immediately, including `Timeout`, `ConnectionClosed`, and
    `InvalidState`.
  - xDLMS client block-transfer loops (`Get`, `Set`, `Action`) return
    the underlying `XdlmsStatus` from `SendAndReceive` /
    `ReceiveGetResponse` / `ReceiveActionResponse` verbatim.
  - Stream / decoder loops (`hdlc_stream_decoder`,
    `wrapper_stream_decoder`) surface `NeedMoreData` only when no
    bytes were read; `ReadSome` failures fall through unchanged.
  - Remaining loops are pure buffer/grow / parser loops with no
    transport interaction (`client_data.cpp EncodeData`, BER/AXDR,
    HDLC segmentation, AXDR length decoder).
  No code change; the roadmap entry now records the audit and
  cites the verification scope.

## 0.97.2 - 2026-06-17

- Hygiene (status mapping, continued): Removed remaining
  `default:` arms from five more status mappers so future
  enum extensions trip `-Wswitch` at compile time. No
  observable runtime behaviour changes for any known input.
  Touched mappers:
  - `dlms-server/xdlms_server_adapter.cpp`
    `MapServerStatusToDataAccessResult` and
    `MapServerStatusToActionResult` (now list every
    `ServerStatus` value explicitly; values without a
    dedicated wire code still collapse to `other-reason`
    (250) per IEC 62056-5-3).
  - `dlms-endpoint/gateway_endpoint.cpp`
    `MapAssociationStatus`.
  - `dlms-endpoint/push_listener_endpoint.cpp`
    `MapAssociationStatus`.
  - `dlms-endpoint/server_endpoint.cpp`
    `MapAssociationStatus`.
- Known cleanup target: `MapAssociationStatus` is now
  duplicated across three endpoint files. Extracting it into
  a shared helper is intentionally deferred to keep this
  change surgical; tracked as a future refactor.

## 0.97.1 - 2026-06-17

- Hygiene (status mapping): Removed `default:` arms from six
  status-enum mappers so future additions to the source enum
  trip `-Wswitch` at compile time instead of silently falling
  into `InternalError`. No observable runtime behaviour
  changes for any known input. Touched mappers:
  - `dlms-endpoint/client_endpoint.cpp` `MapClientStatus`
    (added explicit arms for `BlockTransferRequired`,
    `InvokeIdMismatch`, `CodecFailed` collapsing to
    `ServiceFailed`; the endpoint facade keeps coarse
    service-layer granularity by design).
  - `dlms-endpoint/endpoint_factories.cpp` `MapTransportStatus`.
  - `dlms-endpoint/gateway_endpoint.cpp`
    `MapEndpointStatusToXdlmsStatus`.
  - `dlms-endpoint/server_endpoint.cpp` `MapProfileStatus` and
    `MapXdlmsStatus`.
  - `dlms-server/xdlms_server_adapter.cpp`
    `MapServerStatusToXdlmsStatus`.
  Each mapper now ends with a defensive `return InternalError`
  outside the switch to keep ABI-drift behaviour for unknown
  integer values.
- Deferred: extending `dlms::endpoint::EndpointStatus` with
  finer categories (`BlockTransferRequired`, `CodecFailed`,
  `InvokeIdMismatch`) is intentionally not done in this patch.
  Callers that need that granularity should consume
  `dlms::client::DlmsClient` directly; lifting the categories
  to the endpoint layer is a separate BREAKING change.

## 0.97.0 - 2026-06-17

- BREAKING (C++ API): Extended `dlms::client::ClientStatus` with
  three new values that were previously collapsed into other
  categories. Existing switches with `default` branches keep
  building; exhaustive switches over `ClientStatus` need a new
  arm per value. The full list of added values:
  - `BlockTransferRequired` — the peer asked the client to
    switch to block transfer; the simple non-block path cannot
    complete the request. Distinct from `UnsupportedFeature`
    because the protocol feature exists, the client just did
    not engage it.
  - `InvokeIdMismatch` — the response was framed and decoded
    fine, but its invoke-id does not match the outstanding
    request. Distinct from `ReceiveFailed`.
  - `CodecFailed` — APDU encode or decode failed inside the
    xDLMS layer. Distinct from `InternalError` because it
    implies wire-level corruption or a spec mismatch with the
    peer, not a library bug.
- Fix (status mapping): `MapXdlmsStatus()` no longer collapses
  `XdlmsStatus::BlockTransferRequired` into
  `ClientStatus::UnsupportedFeature` and no longer collapses
  `EncodeFailed`, `DecodeFailed`, and `InvokeIdMismatch` into
  `ClientStatus::InternalError`. The mapper now routes each to
  its dedicated public category (P0 §1.3 from
  `docs/production_readiness_roadmap.md`).
- Updated `ClientStatusName()` and `test_client_status.cpp` to
  recognise the three new values.
- Updated two existing regression tests in `test_client.cpp`
  (`GetMapsRealMalformedResponseAndKeepsAssociated`,
  `GetMapsRealInvokeIdMismatchAndKeepsAssociated`) to expect
  the new specific statuses instead of `InternalError`.
- Exposed `internal::MapXdlmsStatus()` via
  `client_internal.hpp` and added 7 unit tests in
  `test_client_internal.cpp` pinning the full mapping table,
  including a defensive fall-through for unknown enum values.

## 0.96.0 - 2026-06-17

- Fix (status mapping): `DlmsClient::Close()` and
  `DlmsClient::ReleaseAssociation()` no longer collapse every
  HDLC DataLink `DisconnectDataLink()` failure into
  `ClientStatus::InternalError`. The previous mapper used a
  catch-all `default` branch that hid `Timeout`,
  `ConnectionClosed`, malformed UA frames and IO errors behind a
  generic library-bug status, contradicting the production
  readiness roadmap rule P0 §1.3 ("don't collapse useful errors
  to InternalError when a more specific public status exists").
  The mapper now enumerates every `ProfileStatus` explicitly so
  drift in the source enum becomes a compile error and surfaces
  the most actionable category to callers:
  - `Ok`, `NotOpen` → `Ok` (idempotent disconnect).
  - `InvalidArgument` → `InvalidArgument`.
  - `AlreadyOpen` → `InvalidState`.
  - `OpenFailed`, `WriteFailed` → `SendFailed` (DISC could not
    be transmitted to the meter).
  - `ReadFailed`, `Timeout`, `ConnectionClosed`, `WouldBlock`,
    `NeedMoreData`, `OutputBufferTooSmall`, `InvalidFrame`,
    `InvalidLength`, `InvalidAddress`, `PayloadTooLarge` →
    `ReceiveFailed` (no usable UA response from the meter).
  - `UnsupportedFeature` → `UnsupportedFeature`.
  - `InternalError` → `InternalError`.
- Added `lib/dlms-client/src/client/client_internal.hpp` to
  expose `internal::MapDataLinkDisconnectStatus` for unit
  testing without spinning up a real `HdlcProfileChannel`. The
  header is source-tree-only and not part of the install
  surface.
- Added `test_client_internal.cpp` with 8 cases pinning every
  `ProfileStatus` value (including a defensive fall-through for
  unknown integer values).

## 0.95.0 - 2026-06-17

- Fix (C++ API semantics): `CosemRegisterObject::InvokeMethod` now
  recognises method `1` `reset` (data ::= integer(0)) per IEC
  62056-6-2 ED4 (2021) §4.3.2 / DLMS UA Blue Book Ed. 12.1 and
  surfaces it as `CosemStatus::UnsupportedFeature`. Previously
  all method ids returned `CosemStatus::MethodNotFound`, which
  hid `reset` from clients enumerating IC 3 methods. Other
  method ids still report `MethodNotFound`. The built-in object
  remains application-agnostic: reset semantics are decided by
  the backend that owns the underlying register storage.
- Added `kRegisterResetMethodId` to `simple_objects.cpp` and a
  dedicated `CosemRegisterObject, ResetMethodIsUnsupportedAndOtherIdsNotFound`
  unit test. Adjusted the existing `WritesValueAndRejectsUnsupportedMembers`
  test to probe an unrelated method id for the `MethodNotFound`
  branch.
- Refreshed the COSEM IC support matrix row `3` to document the
  new method status mapping.

## 0.94.0 - 2026-06-17

- BREAKING (C++ API and wire semantics): Completed the
  `CosemPrimePlcMacFunctionalParametersObject` (PRIME NB OFDM PLC
  MAC functional parameters, class_id `83`, version `0`) to the
  full 14-attribute spec form per IEC 62056-6-2 ED4 (2021)
  §4.12.7 / DLMS UA Blue Book Ed. 12.1. Previously the object
  exposed only 9 attributes with `mac_capabilities` placed on
  attribute id `9`; per spec `mac_capabilities` lives on
  attribute id `14` and ids `9..13` belong to the beacon family.
  The constructor signature has been replaced and now takes 13
  buffers in spec order: `lnid, lsid, sid, sna, state,
  scpLength, nodeHierarchyLevel, beaconSlotCount, beaconRxSlot,
  beaconTxSlot, beaconRxFrequency, beaconTxFrequency,
  capabilities`. The previous parameters `sct` and `scd` have
  been renamed to `scpLength` (attribute `7`,
  `mac_scp_length`, type `long`) and `nodeHierarchyLevel`
  (attribute `8`, `mac_node_hierarchy_level`, type `unsigned`,
  range `0..63`). Five new attributes were added:
  `mac_beacon_slot_count` (`9`, `unsigned`, `0..7`),
  `mac_beacon_rx_slot` (`10`, `unsigned`, `0..7`),
  `mac_beacon_tx_slot` (`11`, `unsigned`, `0..7`),
  `mac_beacon_rx_frequency` (`12`, `unsigned`, `0..31`),
  `mac_beacon_tx_frequency` (`13`, `unsigned`, `0..31`).
  Internal constant identifiers `kPrimePlcMacFunctionalParamsSctId`
  and `kPrimePlcMacFunctionalParamsScdId` have been renamed to
  `kPrimePlcMacFunctionalParamsScpLengthId` and
  `kPrimePlcMacFunctionalParamsNodeHierarchyLevelId`, the value of
  `kPrimePlcMacFunctionalParamsCapabilitiesId` moved from `9` to
  `14`, and new identifiers were added for the beacon block.
  Public attribute accessors `Sct()` / `Scd()` were renamed to
  `ScpLength()` / `NodeHierarchyLevel()` and accessors
  `BeaconSlotCount()`, `BeaconRxSlot()`, `BeaconTxSlot()`,
  `BeaconRxFrequency()`, `BeaconTxFrequency()` were added.
  Downstream callers must update their constructor argument
  lists and any direct attribute-id reads.
- Refreshed unit tests, COSEM API guide and COSEM IC support
  matrix accordingly. The support matrix row `83` now lists 14
  attributes and notes that the beacon block was completed.

## 0.93.0 - 2026-06-17

- BREAKING (C++ API): Renamed the PRIME PLC MAC Network
  Statistics built-in object from
  `CosemPrimePlcMacNetworkStatisticsObject` to
  `CosemPrimePlcMacNetworkAdminDataObject` to match the
  IEC 62056-6-2 ED4 (2021) §4.12.9 / DLMS UA Blue Book Ed. 12.1
  spec name `PRIME NB OFDM PLC MAC network administration data`
  (class_id `85`, version `0`). The class id, attribute layout,
  access semantics and method behavior are unchanged; only the
  C++ type name and its internal constant identifiers
  (`kPrimePlcMacNetworkAdminData*`) were renamed. Public
  attribute accessors (`NodeRegistrations()`,
  `NodeUnregistrations()`, `ProcessedAliveMsgs()`,
  `HandledPromotions()`) are unchanged.
- Downstream code referencing the old type name must rename to
  the new one; no header path or include changes are required.
- Refreshed unit tests, COSEM API guide and COSEM IC support
  matrix accordingly. The support matrix row `85` now reads
  `PRIME PLC MAC Network Administration Data` and notes the
  rename.

## 0.92.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC Application
  identification built-in object
  (`CosemPrimePlcApplicationIdentificationObject`) from `85`
  to `86` per IEC 62056-6-2 ED4 (2021) §4.12.11 and DLMS UA
  Blue Book Ed. 12.1. Class_id `85` is reserved by the spec
  for the unrelated `PRIME NB OFDM PLC MAC network
  administration data` IC (already corrected in 0.90.0).
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly.

## 0.91.0 - 2026-06-17

- Removed the duplicate `CosemPrimePlcMacAddressSetupObject`
  built-in object (class_id `84`). IEC 62056-6-2 ED4 (2021)
  §4.12.10 places the PRIME NB OFDM PLC MAC address setup
  IC at class_id `43`, which is already covered by the
  existing built-in `CosemMacAddressSetupObject`. The
  duplicate carried the wrong class_id (`84`) and would not
  interoperate with a spec-conformant client/server.
- Refreshed unit tests (4 removed), COSEM IC support matrix,
  COSEM API guide and COSEM test plan accordingly. The
  support matrix now lists row `84` as reserved/
  application-provided and points callers at row `43`.

## 0.90.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC network
  administration data (a.k.a. network statistics) built-in
  object (`CosemPrimePlcMacNetworkStatisticsObject`) from
  `83` to `85` per IEC 62056-6-2 ED4 (2021) §4.12.9 and
  DLMS UA Blue Book Ed. 12.1. Class_id `83` is reserved by
  the spec for the unrelated `PRIME NB OFDM PLC MAC
  functional parameters` IC (already corrected in 0.88.0).
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly.

## 0.89.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC counters
  built-in object (`CosemPrimePlcMacCountersObject`) from
  `82` to `84` per IEC 62056-6-2 ED4 (2021) §4.12.8 and
  DLMS UA Blue Book Ed. 12.1. Class_id `82` is reserved by
  the spec for the unrelated `PRIME NB OFDM PLC MAC setup`
  IC (already corrected in 0.87.0).
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly.

## 0.88.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC functional
  parameters built-in object
  (`CosemPrimePlcMacFunctionalParametersObject`) from `81`
  to `83` per IEC 62056-6-2 ED4 (2021) §4.12.7 and DLMS UA
  Blue Book Ed. 12.1. Class_id `81` is reserved by the spec
  for the unrelated `PRIME NB OFDM PLC Physical layer
  counters` IC.
- The built-in still exposes only the legacy 8-attribute
  surface (`mac_LNID`/`mac_LSID`/`mac_SID`/`mac_SNA`/
  `mac_state`/`sct`/`scd`/`capabilities`). The spec defines
  14 attributes; bringing the remaining ones online
  (`mac_node_hierarchy_level`, `mac_beacon_*`,
  `mac_capabilities` at id `14`) is queued for a follow-up
  rebuild.
- Refreshed unit test, COSEM IC support matrix and COSEM
  API guide accordingly. The matrix now lists IC `81`
  separately as `Application-provided`.

## 0.87.0 - 2026-06-17

- Fixed the class_id of the PRIME NB OFDM PLC MAC setup
  built-in object (`CosemPrimePlcMacSetupObject`) from `80`
  to `82` per IEC 62056-6-2 ED4 (2021) §4.12.6 and DLMS UA
  Blue Book Ed. 12.1. Class_id `80` is reserved by the spec
  for the unrelated `61334-4-32 LLC SSCS setup` IC and was
  never the right id for PRIME PLC MAC setup.
- Refreshed unit test, COSEM IC support matrix and COSEM API
  guide accordingly. The IC support matrix now lists IC `80`
  separately as `Application-provided` (no built-in
  implementation).

## 0.86.0 - 2026-06-17

- Rebuilt the M-Bus Diagnostic built-in object
  (`CosemMBusDiagnosticObject`) to match IEC 62056-6-2 ED4
  (2021) §4.8.7 (class_id 77, version 0):
  - Replaced the prior 6-attribute layout
    (`received_signal_quality`, `transmitter_signal_quality`,
    `bbc`, `fcs_ok_frames_counter`, `fcs_nok_frames_counter`,
    `capture_time`) with the spec-defined 8 dynamic attributes:
    `2 received_signal_strength` (unsigned, dBm),
    `3 channel_id` (unsigned), `4 link_status` (enum),
    `5 broadcast_frames_counter` (array),
    `6 transmissions_counter` (double-long-unsigned),
    `7 fcs_ok_frames_counter` (double-long-unsigned),
    `8 fcs_nok_frames_counter` (double-long-unsigned),
    `9 capture_time` (date-time octet-string(12)).
  - Renamed accessors accordingly
    (`ReceivedSignalStrength`, `ChannelId`, `LinkStatus`,
    `BroadcastFramesCounter`, `TransmissionsCounter`,
    plus the existing FCS counters and `CaptureTime`).
  - Surfaced the optional `1 reset` method as
    `UnsupportedFeature` (built-in object does not own the
    counter sources); undefined method ids still return
    `MethodNotFound`.
- Refreshed unit tests, COSEM IC support matrix, COSEM API
  guide and COSEM test plan.

## 0.85.0 - 2026-06-17

- Fixed the class_id of the M-Bus Master Port Setup built-in
  object (`CosemMBusMasterPortSetupObject`) from `73` to `74`
  to match IEC 62056-6-2 ED4 (2021) §4.8.5 (and the
  DLMS UA Blue Book Ed. 12.1 §4.8.4). class_id `73` is
  reserved for the Wireless Mode Q channel IC, which is not
  shipped as a built-in object.
- Updated the `ExposesAllAttributes` unit test to assert
  `classId == 74u`.
- Refreshed the COSEM IC support matrix (split the previous
  `74`-`76` row, added an explicit Wireless Mode Q channel
  entry for `73`, moved the M-Bus Master Port Setup row to
  `74`) and the COSEM API guide.

## 0.84.0 - 2026-06-17

- Rebuilt the Sensor Manager IC `67` built-in object
  (`CosemSensorManagerObject`) to match the current
  IEC 62056-6-2 ED4 (2021) §4.5.11 layout: class_id `67`,
  version `0` with fifteen attributes (`1 logical_name`,
  `2 serial_number`, `3 metrological_identification`,
  `4 output_type`, `5 adjustment_method`, `6 sealing_method`,
  `7 raw_value`, `8 scaler_unit`, `9 status`, `10 capture_time`,
  `11 raw_value_thresholds`, `12 raw_value_actions`,
  `13 processed_value`, `14 processed_value_thresholds`,
  `15 processed_value_actions`) and one optional specific method
  (`1 reset(data)`). Previously the object exposed an ad-hoc
  layout of eleven attributes with vendor-style names
  (`status`, `serial_number`, `device_type`, `manufacturer_id`,
  `firmware_version`, `metrology_firmware_version`, `driver`,
  `communication_desc`, `setup_desc`, `measurement_desc`) that
  did not match any DLMS UA Blue Book / IEC 62056-6-2 edition
  and declared no methods.
- Method `1` `reset(data)` is surfaced as `UnsupportedFeature`
  (the built-in object does not own the sensor lifecycle); other
  method ids remain `MethodNotFound`.
- Constructor signature reworked to take the fourteen mutable
  payload buffers in spec attribute order followed by the shared
  `AttributeAccessMode` (and the optional version).
- Renamed C++ accessors to follow the spec names
  (`SerialNumber`, `MetrologicalIdentification`, `OutputType`,
  `AdjustmentMethod`, `SealingMethod`, `RawValue`, `ScalerUnit`,
  `Status`, `CaptureTime`, `RawValueThresholds`,
  `RawValueActions`, `ProcessedValue`,
  `ProcessedValueThresholds`, `ProcessedValueActions`); removed
  the old `DeviceType`, `ManufacturerId`, `FirmwareVersion`,
  `MetrologyFirmwareVersion`, `Driver`, `CommunicationDesc`,
  `SetupDesc`, `MeasurementDesc` accessors.
- Updated unit tests to cover the new fifteen-attribute layout,
  the renamed accessors and the new
  `ResetMethodIsUnsupportedFeature` expectation.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the ED4 Sensor Manager layout.

## 0.83.0 - 2026-06-17

- Rebuilt the Parameter Monitor IC `65` built-in object
  (`CosemParameterMonitorObject`) to match the current
  IEC 62056-6-2 ED4 (2021) §4.5.10 layout: class_id `65`,
  version `1` with eight attributes (`1 logical_name`,
  `2 changed_parameter`, `3 capture_time`, `4 parameter_list`,
  `5 parameter_list_name`, `6 hash_algorithm_id`,
  `7 parameter_value_digest`, `8 parameter_values`) and two
  specific methods (`1 add_parameter`, `2 delete_parameter`).
  Previously the object was pinned to legacy version `0`
  (Blue Book Ed. 12.1 §5.4.1) with only four attributes.
- Bumped `CosemParameterMonitorObject::MaxSupportedVersion` from
  `0` to `1`; the short constructor now defaults to version `1`.
  The constructor signature gained four new octet-string buffer
  parameters for attributes `5`-`8`. Passing version `0` keeps
  the legacy behaviour: the four extended buffers are cleared at
  construction time and attributes `5`-`8` return
  `AttributeNotFound` on both read and write.
- Methods `1 add_parameter` and `2 delete_parameter` continue to
  surface as `UnsupportedFeature` (the built-in object does not
  manage the monitored-parameters table; backend is expected to
  republish the buffers after evaluating parameter changes
  out-of-band).
- Updated unit tests to cover the new attributes, the renamed
  methods (`add_parameter`/`delete_parameter`) and the legacy
  version-0 fallback (`LegacyVersion0RejectsExtendedAttrs`).
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the ED4 Parameter Monitor layout.

## 0.82.0 - 2026-06-17

- Aligned the Compact Data IC `62` built-in object
  (`CosemCompactDataObject`) class version with
  IEC 62056-6-2 ED4 (2021) §4.3.10 / DLMS UA Blue Book Ed. 12.1.
  The Compact Data IC is defined as class_id `62`, version `1`
  with five attributes (`1 logical_name`, `2 buffer`,
  `3 capture_objects`, `4 template_id`, `5 template_description`,
  `6 capture_method`) and two specific methods (`1 reset`,
  `2 capture`). The attribute and method layout already matched
  the spec; only `MaxSupportedVersion` was wrong (`0` instead of
  `1`), so the descriptor advertised an obsolete legacy version.
- Bumped `CosemCompactDataObject::MaxSupportedVersion` from `0`
  to `1` and changed the default constructor to pass `1` so
  freshly built objects advertise the spec version. Reads and
  writes of attributes `2`-`6` and the `UnsupportedFeature`
  dispatch for methods `1 reset` / `2 capture` are unchanged.
- Updated `CosemCompactDataObject.ExposesAllAttributes` to
  expect `Descriptor().key.version == 1`.
- Refreshed the COSEM IC support matrix and COSEM API guide to
  describe Compact Data as class version `1` per
  IEC 62056-6-2 ED4 §4.3.10.

## 0.81.0 - 2026-06-17

- Rebuilt the Register Table IC `61` built-in object
  (`CosemRegisterTableObject`) to match IEC 62056-6-2 ED4 (2021)
  §4.3.8 and DLMS UA Blue Book Ed. 12.1. The spec defines four
  attributes (`1 logical_name`, `2 table_cell_values`,
  `3 table_cell_definition`, `4 scaler_unit`) and two specific
  methods (`1 reset`, `2 capture`). The previous implementation had
  five attributes with a phantom `single_buffer` (id 3) and a
  phantom `table_entries` (id 5), no `scaler_unit`, and surfaced
  the methods as `table_entry` / `update_table_entry` rather than
  the spec-defined `reset` / `capture`.
- Replaced the four-buffer constructor pair with the spec
  three-buffer signature `(logical_name, table_cell_values,
  table_cell_definition, scaler_unit, access[, version])` and
  renamed accessors accordingly
  (`SingleBuffer/TableEntries` removed, `ScalerUnit` added).
- `InvokeMethod` now version-gates the spec-defined methods: both
  `1 reset` and `2 capture` return `UnsupportedFeature` because the
  captured payload lifecycle is owned by the backend. Every other
  method id continues to report `MethodNotFound`.
- Updated `CosemRegisterTableObject` tests to drive the new
  three-buffer ctor, the renamed accessors, the four-attribute
  layout (probe `5` -> `AttributeNotFound`) and the `scaler_unit`
  round-trip; kept the existing `MethodsReturnUnsupportedFeature`
  test for spec methods `1` / `2`.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the four-attribute spec layout,
  the spec-defined `reset` / `capture` methods and the dropped
  `single_buffer` / `table_entries` fields.

## 0.80.0 - 2026-06-17

- Aligned the Auto Connect IC `29` built-in object
  (`CosemAutoConnectObject`) with IEC 62056-6-2 ED4 (2021)
  §4.7.6. The spec defines class version `2` ("Auto connect")
  with attributes `1..6` and one specific method
  `1 connect (data)`; the legacy class version `0`
  ("PSTN auto dial") keeps the same six attributes but defines
  no methods. The previous implementation was pinned to
  `MaxSupportedVersion = 0` and unconditionally reported
  `MethodNotFound` for every id, hiding the v2 method.
- Raised `MaxSupportedVersion` to `2` and made the short
  constructor default to class version `2`; the longer ctor that
  accepts an explicit version still allows v0 ("PSTN auto dial")
  for backward compatibility, and any out-of-range version is
  normalized to `MaxSupportedVersion = 2`.
- `InvokeMethod` now version-gates the spec-defined method:
  `method id 1` returns `UnsupportedFeature` for instances at
  class version `>= 2` (the built-in object does not own the
  dialler / radio stack), and `MethodNotFound` for legacy v0
  instances. Every other method id continues to report
  `MethodNotFound`.
- Replaced the `CosemAutoConnectObject.NoMethodsDefined` test
  with `ConnectMethodIsUnsupportedFeature` and
  `LegacyVersion0ReportsMethodNotFound`; updated
  `ExposesAllAttributes` to assert the new default version `2`.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe the new default version, the
  spec-defined `connect` method and the legacy-v0 escape hatch.

## 0.79.0 - 2026-06-17

- Rebuilt the SMTP Setup IC `46` built-in object
  (`CosemSmtpSetupObject`) to match IEC 62056-6-2 ED4 (2021)
  §4.9.6 and DLMS UA Blue Book Ed. 12.1 §4.9.6. The spec defines
  six attributes: `1 logical_name`, `2 server_port`,
  `3 user_name`, `4 login_password`, `5 server_address`,
  `6 sender_address`. The previous implementation invented a
  non-spec `smtp_server` octet-string at id 2, shifted every
  subsequent attribute by one, used different names
  (`sender` vs `sender_address`), and added a non-existent
  seventh `receivers` array attribute. Constants, ctor parameter
  list, access-rights table, read/write switches and accessors
  are now `kSmtpSetup{ServerPort,UserName,LoginPassword,
  ServerAddress,SenderAddress}AttributeId` with attribute ids
  `2..6`. `ReadAttribute(7, ...)` now reports `AttributeNotFound`.
  Methods remain absent (the IC defines none); `InvokeMethod`
  continues to report `MethodNotFound` for every id.
- Updated `CosemSmtpSetupObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`, `NoMethodsDefined` and
  `NormalizesVersionAboveMax` to match the spec layout. The
  writable id set in the mutable-attribute test is now
  `{2, 3, 4, 5, 6}`; the `AttributeNotFound` probe shifts from
  id `8` to id `7`. The sample `SmtpSetupBuffers` helper now
  encodes a `long-unsigned 587` `server_port`, an octet-string
  `"smtp.example.com"` `server_address` and an octet-string
  `"a@b.c"` `sender_address` (no more `smtpServer`/`receivers`
  fields).
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to describe IC 46 attributes `2..6` and the
  new mutable set `{2, 3, 4, 5, 6}`.

## 0.78.0 - 2026-06-17

- Added the missing `7 list_of_allowed_callers` attribute to the
  Auto Answer IC `28` (`CosemAutoAnswerObject`). IEC 62056-6-2
  ED4 (2021) §4.7.5 and DLMS UA Blue Book Ed. 12.1 §4.6.4 define
  this attribute as an `array of allowed_caller_element`
  structures listing the caller identifications that the meter
  accepts; the built-in object previously stopped at
  attribute `6` and reported `AttributeNotFound` for the spec-
  defined attribute `7`. The constructors now take an extra
  `listOfAllowedCallers` content buffer (positioned after
  `numberOfRings`), the new attribute joins the writable set
  governed by the caller-selected `AttributeAccessMode`, and a
  `ListOfAllowedCallers()` accessor exposes the stored buffer.
  Methods remain absent (the IC defines none); `InvokeMethod`
  continues to report `MethodNotFound` for every id.
- Updated `CosemAutoAnswerObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode` and
  `NormalizesVersionAboveMax` to round-trip the new attribute, and
  expanded the writable id set in the mutable-attribute test to
  `{2, 3, 5, 6, 7}` so both writable and read-only paths assert
  the new behavior. `AttributeNotFound` is now expected at id `8`
  (previously `7`). The sample `AutoAnswerBuffers` helper now
  carries an empty `array(0)` payload as the
  `list_of_allowed_callers` default.
- Refreshed the COSEM IC support matrix, COSEM API guide and
  COSEM test plan to document attribute `7 list_of_allowed_callers`
  and the new mutable set `{2, 3, 5, 6, 7}` for IC 28.

## 0.77.0 - 2026-06-17

- Rebuilt the M-Bus slave port setup IC `25`
  (`CosemMBusSlavePortSetupObject`) to match the five-attribute,
  zero-method spec layout defined in IEC 62056-6-2 ED4 (2021)
  §4.8.2 and DLMS UA Blue Book Ed. 12.1 §4.8.1. The class
  definition lists: `1 logical_name` (octet-string, static,
  read-only), `2 default_baud` (enum, static), `3 avail_baud`
  (enum, static), `4 addr_state` (enum, static, indicating whether
  the slave has been assigned a bus address) and `5 bus_address`
  (unsigned, static). The built-in object previously used the
  non-spec attribute names `status` and `mbus_port_reference` for
  attributes `4` and `5`, mismodelling them as a status enum and an
  octet-string reference to another object respectively. The
  constructors and accessor signatures have been rewritten to take
  the four content buffers (`default_baud`, `avail_baud`,
  `addr_state`, `bus_address`) as encoded DLMS Data buffers
  prepared by the caller; attributes `2`-`5` share the
  caller-selected `AttributeAccessMode` and accept in-place writes
  when permitted. The IC defines no specific methods
  (`Specific methods | m/o` column is empty in both editions);
  `InvokeMethod` now returns `MethodNotFound` for every id and
  clears method output. Earlier revisions surfaced a phantom
  `reset` method (id `1`) as `UnsupportedFeature`; that method is
  not defined by any published edition of the spec and has been
  removed along with the `kMBusSlavePortSetupResetMethodId`
  constant.
- Replaced the existing
  `CosemMBusSlavePortSetupObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode`,
  `MethodsReturnUnsupportedFeature` and `NormalizesVersionAboveMax`
  regression tests with versions that exercise the new attribute
  layout (sample `MBusSlavePortSetupBuffers` now carries
  enum-encoded `default_baud` 9 600 bps, `avail_baud` 38 400 bps,
  `addr_state` 1 "assigned" and unsigned-encoded `bus_address`
  0x42). The method test has been renamed `NoMethodsDefined` and
  asserts `MethodNotFound` plus cleared method output for every
  probed method id (`1`, `2`, `3`, `99`), matching the
  zero-method spec surface.
- Updated the COSEM IC support matrix and COSEM API guide to
  document the new spec-shaped attribute layout
  (`default_baud`, `avail_baud`, `addr_state`, `bus_address`),
  call out the removed `reset` method, and note that
  `InvokeMethod` now reports `MethodNotFound` for every id. The
  COSEM test plan's M-Bus slave port setup checklist has been
  refreshed to reflect the new attribute names and the
  zero-method `MethodNotFound` semantics.

## 0.76.0 - 2026-06-17

- Rebuilt the IEC twisted pair (1) Setup IC `24`
  (`CosemIecTwistedPairSetupObject`) to match the five-attribute
  spec layout defined in IEC 62056-6-2 ED4 (2021) §4.7.3 and DLMS
  UA Blue Book Ed. 12.1 §4.7.3. The class definition lists:
  `1 logical_name` (octet-string, static, read-only),
  `2 secondary_address` (octet-string of size 6, static),
  `3 primary_address_list` (array of octet-string, static),
  `4 tabi_list` (array of integer, static) and
  `5 fatal_error` (enum, dynamic) carrying the latest occurrence of
  one of the IEC 62056-31 protocol fatal errors. The built-in
  object previously exposed only two attributes using a mismatched
  naming and layout (`primary_address` long-unsigned at id `2` and
  `tabis` array of long-unsigned at id `3`) which did not match
  any published edition of the spec. The constructors and
  accessor signatures have been rewritten to take the four content
  buffers (`secondary_address`, `primary_address_list`,
  `tabi_list`, `fatal_error`) as encoded DLMS Data buffers prepared
  by the caller. Attributes `2`, `3` and `4` share the
  caller-selected `AttributeAccessMode` and accept in-place writes
  when permitted; `5 fatal_error` is server-managed and remains
  read-only at the wire surface regardless of the caller-selected
  mode (backends republish the buffer when the underlying stack
  observes a new fatal error). The IC defines no specific methods
  (`Specific methods | m/o` column is empty in both editions);
  `InvokeMethod` continues to return `MethodNotFound` for every id
  and clears method output.
- Replaced the existing
  `CosemIecTwistedPairSetupObject.ExposesAllAttributes`,
  `MutableAttributesHonorAccessMode` and `NormalizesVersionAboveMax`
  regression tests with versions that exercise the new five-attribute
  surface (sample `IecTwistedPairSetupBuffers` now carries the
  spec-shaped payloads, including a six-octet secondary address, an
  array of one-octet primary addresses, an array of TAB(i) integers
  and an enum-encoded fatal error). `MutableAttributesHonorAccessMode`
  additionally pins fatal_error's read-only semantics on writable
  instances. `NoMethodsDefined` keeps asserting `MethodNotFound` for
  every method id.
- Updated the IC support matrix (`docs/ic_support_matrix.md`) and
  the COSEM API guide (`lib/dlms-cosem/docs/01_cosem_api.md`) to
  describe the spec-aligned five-attribute layout and the
  read-only-by-design fatal_error semantics.

  BREAKING CHANGE: `CosemIecTwistedPairSetupObject`'s constructors
  and accessors have changed shape. The constructors now take
  four content buffers in the order
  `(secondary_address, primary_address_list, tabi_list, fatal_error)`
  instead of the previous two
  `(primary_address, tabis)`, and the accessor pair
  `PrimaryAddress()`/`Tabis()` has been replaced by
  `SecondaryAddress()`/`PrimaryAddressList()`/`TabiList()`/
  `FatalError()`. Callers constructing the IC at runtime or reading
  back its content must update their call sites and the encoded
  payloads they pass in (note the spec data-type changes: primary
  addresses are now array elements of octet-string of size 1, and
  TAB(i) entries are integers rather than long-unsigned values).
  The on-wire attribute count also grows from `3` to `5`; clients
  iterating over attributes `2..N` based on the previous
  implementation now observe ids `2..5` and must be prepared for
  the additional payloads.

## 0.75.0 - 2026-06-17

- Aligned the IPv4 Setup IC `42` (`CosemIpv4SetupObject`) method
  surface with IEC 62056-6-2 ED4 (2021) clause 4.9.2.3. The spec
  defines three specific methods: `1` `add_mc_IP_address`,
  `2` `delete_mc_IP_address` and `3` `get_nbof_mc_IP_addresses`.
  `InvokeMethod` previously dispatched only ids `1` and `2`,
  silently returning `MethodNotFound` for id `3` and hiding the
  spec method. The built-in object does not own multicast
  subscription policy nor expose the runtime
  `multicast_IP_address` array size, so method `3` is now surfaced
  explicitly as `UnsupportedFeature` alongside `1` and `2`; unknown
  method ids still report `MethodNotFound`. Not a breaking change
  at the public surface: any caller that was already receiving
  `MethodNotFound` for method `3` was out of spec, and callers
  that respect the documented status set handle the new status
  without changes.
- Removed the phantom `reset` method (id `1`) from the GSM
  Diagnostic IC `47` (`CosemGsmDiagnosticObject`). IEC 62056-6-2
  ED4 (2021) §5.6.8 and DLMS UA Blue Book Ed. 12.1 §5.6.8 define
  class_id `47`, version `0` with **no** specific methods (the
  "Specific methods | m/o" column is empty in both editions). The
  built-in object previously surfaced a fabricated `reset` method
  that the spec never defines; `InvokeMethod` now returns
  `MethodNotFound` for every method id, matching the spec. The
  associated `kGsmDiagnosticResetMethodId` constant has been
  removed.
- Updated the `CosemIpv4SetupObject.MulticastMethodsReturnUnsupportedFeature`
  regression test to assert all three spec methods return
  `UnsupportedFeature` and only ids `4`+ return `MethodNotFound`.
- Renamed `CosemGsmDiagnosticObject.MethodsReturnUnsupportedFeature`
  to `AllMethodsReturnMethodNotFound` and rewrote it to assert the
  new spec-aligned behaviour (every id, including the former `1`,
  returns `MethodNotFound`).
- Updated `docs/ic_support_matrix.md` and
  `lib/dlms-cosem/docs/01_cosem_api.md` to document the corrected
  IPv4 Setup and GSM Diagnostic method surfaces with spec
  citations.
- Bumped `VERSION` to `0.75.0`.

## 0.74.0 - 2026-06-17

- Surfaced the SAP Assignment IC `17` `connect_logical_device` method
  (id `1`) per IEC 62056-6-2 ED4 (2021) clause 4.4.4 and DLMS UA Blue
  Book Ed. 12.1 clause 5.3.4. `CosemSapAssignmentObject::InvokeMethod`
  previously returned `MethodNotFound` for every method id, silently
  hiding the only spec-defined method. The built-in object does not
  own the SAP / logical-device attachment policy, so method `1` is now
  surfaced explicitly as `UnsupportedFeature` (matching the pattern
  used by IC 5, 9, 10, 11 and 22); unknown method ids still report
  `MethodNotFound`. Not a breaking change at the public surface: any
  caller that was already receiving `MethodNotFound` for method 1 was
  out of spec, and callers that respect the documented status set
  (Ok / AccessDenied / UnsupportedFeature / MethodNotFound) handle
  the new status without changes.
- Updated the `DiscoveryObjects.RejectUnsupportedAttributesWritesAndMethods`
  regression test to assert the new SAP Assignment method semantics
  (id `1` -> `UnsupportedFeature`, id `99` -> `MethodNotFound`).
- Updated `docs/ic_support_matrix.md` and
  `lib/dlms-cosem/docs/01_cosem_api.md` to document the SAP Assignment
  method surface.
- Bumped `VERSION` to `0.74.0`.

## 0.73.0 - 2026-06-17

- Aligned the Schedule IC `10` built-in object (`CosemScheduleObject`)
  method ids with IEC 62056-6-2 ED4 (2021) clause 4.5.3 and DLMS UA
  Blue Book Ed. 12.1 clause 5.1.7. The spec defines three specific
  methods (`1` `enable_disable`, `2` `insert`, `3` `delete`); the
  implementation previously only recognised two methods and used the
  wrong ids (`1` `insert`, `2` `delete`), so clients targeting the
  spec id for `insert` actually triggered `enable_disable`, clients
  targeting the spec id for `delete` triggered `insert`, and method
  id `3` (`delete`) silently returned `MethodNotFound`. All three
  spec ids now return `UnsupportedFeature` (the built-in object does
  not own schedule-entry mutation policy); method id `0` and ids
  `>= 4` continue to return `MethodNotFound`. This is a breaking
  change for any caller that hardcoded the old (non-spec) method
  ids; callers passing spec ids are unaffected (they now reach the
  correct branch).
- Updated the `CosemScheduleObject.MethodsReturnUnsupportedFeature`
  regression test to cover the new id set (`{1, 2, 3}` ->
  `UnsupportedFeature`, `{0, 4, 5}` -> `MethodNotFound`).
- Updated `docs/ic_support_matrix.md` and
  `lib/dlms-cosem/docs/01_cosem_api.md` to list the spec-aligned
  method ids.
- Bumped `VERSION` to `0.73.0`.

## 0.72.0 - 2026-06-16

- Aligned the M-Bus Client IC `72` built-in object
  (`CosemMBusClientObject`) with IEC 62056-6-2 ED4 (2021) clause
  4.8.3 and DLMS UA Blue Book Ed. 12.1 clause 5.7.1. Attributes
  `13` (`configuration`) and `14` (`encryption_key_status`) are now
  exposed only when the requested class version is `>=1`; v0
  instances (Blue Book table for class_id=72, version=0) report
  `NoAccess` for these attribute ids in their `CosemAccessRights`
  and return `AttributeNotFound` from `ReadAttribute` and
  `WriteAttribute`, matching the spec which stops the v0 attribute
  table at attribute `12`. Previously both attributes leaked onto
  v0 instances and were freely readable and writable. The
  constructor signatures, supported method ids (`1`..`8`) and
  `MaxSupportedVersion = 1` are unchanged; the default constructors
  continue to instantiate v1 objects.
- Added a `CosemMBusClientObject.Version0DoesNotExposeConfigurationOrEncryptionKeyStatus`
  regression test covering the new v0 gating (access rights,
  reads, writes, and that attributes `1`..`12` remain available).
- Bumped `VERSION` to `0.72.0`.

## 0.71.0 - 2026-06-15

- Aligned the Profile Generic IC `7` built-in object
  (`CosemProfileGenericObject`) with IEC 62056-6-2 ED4 (2021) clause
  4.3.6 and DLMS UA Blue Book Ed. 12.1 clause 5.2.1. Both class
  version `0` (legacy) and version `1` (current) expose the full
  method set defined by the standard: `reset` (1), `capture` (2),
  `get_buffer_by_range` (3), and `get_buffer_by_index` (4). Previously
  methods `3` and `4` were silently hidden on the default v1
  instances (the gating predicate was also inverted relative to the
  spec, which does not gate the methods by version at all) and
  returned `MethodNotFound` instead of `UnsupportedFeature`. All four
  ids are now advertised in access rights as `Access` and return
  `UnsupportedFeature` pending the capture and journal execution
  policy.
- Made `sort_method` (attribute `5`) and `sort_object` (attribute
  `6`) explicit static properties of the IC instead of synthesising
  them. The basic constructor defaults to
  `CosemProfileGenericSortMethod::Fifo` with an empty
  `ObjectDefinition` (`class_id = 0`, zeroed logical name,
  `attribute_index = 0`, `data_index = 0`). New constructor
  overloads accept a `CosemProfileGenericSortMethod` and a
  `CosemCaptureObject` (combined with or without an explicit
  version) for callers that publish a sorted profile, and the new
  `SortMethod()` / `SortObject()` getters return the published
  values. The previous behaviour of returning the hardcoded `Fifo`
  enum for attribute `5` and deriving attribute `6` from the first
  capture object was incorrect: the standard treats them as
  independently configurable static attributes.
- Added regression tests
  `CosemProfileGenericObject.HonorsConfigurableSortMethodAndSortObject`
  and `CosemProfileGenericObject.DefaultSortMethodIsFifoWithEmptySortObject`
  that lock the new sort surface, extended
  `CosemProfileGenericObject.RejectsWritesAndReportsUnsupportedMethods`
  to assert all four method ids report `Access` /
  `UnsupportedFeature`, and rewrote
  `CosemProfileGenericObject.AcceptsExplicitVersion` so it exercises
  the corrected v0 and v1 method tables instead of the inverted
  gating.

## 0.70.0 - 2026-06-15

- Re-aligned the Association SN IC `12` built-in object
  (`CosemAssociationSnObject`) with IEC 62056-6-2 ED4 (2021) clause
  4.4.3 and DLMS UA Blue Book Ed. 12.1 clause 5.4.5. The specific
  method ids were renumbered to match the standard: `3`
  `read_by_logicalname`, `5` `change_secret`, `8`
  `reply_to_HLS_authentication`, `9` `add_user` (v3+) and `10`
  `remove_user` (v3+); ids `1`, `2`, `4`, `6`, `7` and `11+` are
  reserved or undefined and now report `MethodNotFound` instead of
  responding to the previously fabricated `add_object` /
  `remove_object` / `change_HLS_secret` mapping. `MaxSupportedVersion`
  was raised from `3` to `4` to match the current ED4 ceiling.
- Gated the version-dependent surface: `security_setup_reference`
  (attribute `4`) is exposed for v>=2, and `user_list` (attribute
  `5`) plus `current_user` (attribute `6`) for v>=3. Constructing a
  lower-version instance clears the gated buffers, reports
  `NoAccess` for them in the access-rights list and returns
  `AttributeNotFound` on reads or writes; `add_user` / `remove_user`
  return `MethodNotFound` outside v3+.
- Added regression tests
  `CosemAssociationSnObject.Version0DoesNotExposeSecuritySetupOrUserAttributes`
  and `CosemAssociationSnObject.Version2ExposesSecuritySetupButNotUserAttributes`
  that lock the new per-version gating, and updated the existing
  `MethodsReturnUnsupportedFeature` test to exercise the corrected
  method ids.

## 0.69.0 - 2026-06-15

- Tightened the per-version surface of the Association LN IC `15`
  built-in object (`CosemAssociationLnObject`) to match IEC 62056-6-2
  ED4 (2021) clause 4.4.4 and DLMS UA Blue Book Ed. 12.1 clause 4.4.4:
  the `user_list` (attribute `10`), `current_user` (attribute `11`),
  `add_user` (method `5`) and `remove_user` (method `6`) elements are
  defined for class version `3` and were previously surfaced on
  version `2` instances as well. Construction of a v2 instance now
  clears any caller-supplied `users`/`currentUser` payload, reports
  `NoAccess` for attributes `10`-`11` and methods `5`-`6` in the
  access-rights list, returns `AttributeNotFound` for reads of `10`
  or `11`, and returns `MethodNotFound` for invocations of `5` or
  `6`. The v0, v1 and v3 surfaces and the access-rights/object-list
  encoders are unchanged. `MaxSupportedVersion` stays at `3`.
- Added regression test
  `CosemAssociationLnObject.Version2DoesNotExposeUserAttributesOrMethods`
  that locks the v2 surface.

## 0.68.0 - 2026-06-15

- Removed phantom `port_speed` attribute (`10`) from the IEC Local
  Port Setup IC `19` built-in object (`CosemIecLocalPortSetupObject`).
  IEC 62056-6-2 ED4 (2021) clauses 4.7.1 and 5.6.1, and DLMS UA
  Blue Book Ed. 12.1 clause 4.7.1, define IC 19 with exactly nine
  attributes (`1`-`9`) in both versions `0` and `1` and no specific
  methods. The previous implementation surfaced an extra `port_speed`
  enum at attribute id `10`, gated to v1, which is not part of the
  standard.
- **Breaking change**: both `CosemIecLocalPortSetupObject` constructors
  drop the `portSpeed` parameter, and the `PortSpeed()` getter is
  removed. Callers must drop the `port_speed` buffer from construction
  sites. Reads of attribute `10` now report `AttributeNotFound`;
  attributes `2`-`9` continue to honor the caller-selected
  `AttributeAccessMode`. `MaxSupportedVersion` stays at `1`, and
  `InvokeMethod` still reports `MethodNotFound` for all method ids.

## 0.67.0 - 2026-06-15

- Bumped Push Setup IC `40` built-in object
  (`CosemPushSetupObject`) from `MaxSupportedVersion = 1` to
  `MaxSupportedVersion = 2`, aligning the per-version surface with
  IEC 62056-6-2 ED4 (2021):
  - v0 exposes attributes `1`-`7` (logical_name plus the v0 surface);
  - v1 adds attributes `8` port_reference, `9` push_client_SAP and
    `10` push_protection_parameters;
  - v2 adds attributes `11` push_operation_method,
    `12` confirmation_parameters and
    `13` last_confirmation_date_time.
  Reads and writes to attributes that are not part of the negotiated
  class version report `AttributeNotFound`; this fixes a regression
  in `0.26.0` that surfaced attributes `8`-`13` as if they all
  belonged to v1.
- Added method `2` `reset` (defined for v2 only): on a v2 instance
  it clears the stored `last_confirmation_date_time` buffer (the
  only persistent state owned by the built-in object) and returns
  `Ok`; on v0/v1 instances it returns `MethodNotFound`. Method `1`
  `push` continues to report `UnsupportedFeature` and undefined
  method ids continue to report `MethodNotFound`; method output is
  always cleared.
- Documented that the `repetition_delay` attribute (`7`) is
  long-unsigned in v0/v1 and a `{repetition_delay_min,
  repetition_delay_exponent, repetition_delay_max}` structure in
  v2. The buffer remains opaque; the caller is responsible for
  encoding the value that matches the negotiated version.

## 0.66.0 - 2026-06-15

- Added S-FSK Active Initiator IC `51` built-in object
  (`CosemSFskActiveInitiatorObject`) with class version `0`,
  exposing `active_initiator` (structure {system_title:
  octet-string(8), MAC_address: long-unsigned,
  L_SAP_selector: unsigned}, per IEC 62056-6-2 ED4 4.10.4.2.2
  and DLMS UA Blue Book IC 51) as an opaque encoded DLMS Data
  buffer prepared by the caller.
- Attribute `2` honours a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset_new_not_synchronized` returns
  `UnsupportedFeature`; undefined method ids return
  `MethodNotFound`; method output is always cleared.

## 0.65.0 - 2026-06-15

- Added S-FSK PLC PHY & MAC Setup IC `50` built-in object
  (`CosemSFskPlcPhyMacSetupObject`) with class version `1`,
  exposing initiator/delta electrical phase, max
  receiving/transmitting gain, search_initiator_threshold,
  frequencies (structure { mark_frequency: double-long-unsigned,
  space_frequency: double-long-unsigned }), mac_address,
  mac_group_addresses, repeater, repeater_status,
  min_delta_credit, initiator_mac_address,
  synchronization_locked and transmission_speed as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`15` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature`; undefined
  method ids return `MethodNotFound`; method output is always
  cleared.

## 0.64.0 - 2026-06-15

- Added PRIME PLC Application Identification IC `85` built-in
  object (`CosemPrimePlcApplicationIdentificationObject`) with
  class version `0`, exposing `application_identifier`
  (octet-string) as an opaque encoded DLMS Data buffer prepared
  by the caller.
- Attribute `2` uses a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.
- With Application Identification the PRIME PLC family is now
  fully covered (ICs `81`-`85` all have built-in opaque-buffer
  objects).

## 0.63.0 - 2026-06-15

- Added PRIME PLC MAC Address Setup IC `84` built-in object
  (`CosemPrimePlcMacAddressSetupObject`) with class version
  `0`, exposing `mac_address` (long-unsigned) as an opaque
  encoded DLMS Data buffer prepared by the caller.
- Attribute `2` uses a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.

## 0.62.0 - 2026-06-15

- Added PRIME PLC MAC Network Statistics IC `83` built-in
  object (`CosemPrimePlcMacNetworkStatisticsObject`) with class
  version `0`, exposing `node_registrations`,
  `node_unregistrations`, `processed_alive_msgs` and
  `handled_promotions` (double-long-unsigned) as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`5` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature`; undefined
  method ids return `MethodNotFound`; method output is always
  cleared.

## 0.61.0 - 2026-06-15

- Added PRIME PLC MAC Counters IC `82` built-in object
  (`CosemPrimePlcMacCountersObject`) with class version `0`,
  exposing `txdatapkt_count`, `rxdatapkt_count`,
  `txctrlpkt_count`, `rxctrlpkt_count`, `csmafail_count` and
  `csmachbusy_count` (double-long-unsigned) as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`7` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature`; undefined
  method ids return `MethodNotFound`; method output is always
  cleared.

## 0.60.0 - 2026-06-15

- Added PRIME PLC MAC Functional Parameters IC `81` built-in
  object (`CosemPrimePlcMacFunctionalParametersObject`) with
  class version `0`, exposing `lnid` (long-unsigned), `lsid`,
  `sid` (unsigned), `sna` (octet-string EUI-48), `state` (enum),
  `sct`, `scd` (long-unsigned) and `capabilities` (bit-string)
  as opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`9` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.

## 0.59.0 - 2026-06-15

- Added PRIME PLC MAC Setup IC `80` built-in object
  (`CosemPrimePlcMacSetupObject`) with class version `0`,
  exposing `mac_min_con_window`, `mac_max_con_window`
  (long-unsigned), `mac_channel_access_fairness_limit`
  (unsigned), `mac_EMA`, `mac_SAR_size`, `mac_max_PDU_size`,
  `mac_min_switch_search_time`, `mac_max_promotion_PDU`,
  `mac_promotion_PDU_TX_period` (long-unsigned),
  `mac_beacons_per_frame`, `mac_scp_max_TX_attempts`,
  `mac_CTL_re_TX_timer` (unsigned) and `mac_max_LNID`
  (long-unsigned) as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`14` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer
  in-place when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id and clears method output.

## 0.58.0 - 2026-06-15

- Added M-Bus Diagnostic IC `77` built-in object
  (`CosemMBusDiagnosticObject`) with class version `0`, exposing
  `received_signal_quality` (unsigned),
  `transmitter_signal_quality` (unsigned), `bbc` (long-unsigned),
  `fcs_ok_frames_counter` (double-long-unsigned),
  `fcs_nok_frames_counter` (double-long-unsigned) and
  `capture_time` (date-time octet-string(12)) as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`7` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so
  the backend can republish refreshed signal quality, frame
  counters and capture time after polling the M-Bus link
  out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports `MethodNotFound`
  for every method id and clears method output.

## 0.57.0 - 2026-06-15

- Added M-Bus Master Port Setup IC `73` built-in object
  (`CosemMBusMasterPortSetupObject`) with class version `0`,
  exposing `comm_speed` (enum) as an opaque encoded DLMS Data
  buffer prepared by the caller.
- Attribute `2` `comm_speed` honors a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC v0 defines no methods; `InvokeMethod` reports `MethodNotFound`
  for every method id and clears method output.

## 0.56.0 - 2026-06-15

- Added M-Bus Client IC `72` built-in object
  (`CosemMBusClientObject`) with class version `1`, exposing
  `mbus_port_reference` (octet-string(6) LN to IEC HDLC Setup),
  `capture_definition` (array of structure {`data_link_reference`:
  octet-string, `value_information_block`: octet-string}),
  `capture_period` (double-long-unsigned, seconds),
  `primary_address` (unsigned), `identification_number`
  (double-long-unsigned), `manufacturer_id` (long-unsigned),
  `version` (unsigned, M-Bus device version), `device_type`
  (unsigned), `access_number` (unsigned), `status` (unsigned),
  `alarm` (unsigned), `configuration` (long-unsigned, v1) and
  `encryption_key_status` (enum, v1) as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `2`-`14` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted, so the backend can republish refreshed
  identification, status, alarm, configuration and key-status
  payloads after driving the M-Bus slave out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `slave_install`, `2` `slave_deinstall`,
  `3` `capture`, `4` `reset_alarm`, `5` `synchronise_clock`,
  `6` `send_data`, `7` `set_encryption_key` and
  `8` `transfer_key` return `UnsupportedFeature` and clear method
  output (the built-in object does not drive the M-Bus slave);
  other method ids return `MethodNotFound`.

## 0.55.0 - 2026-06-15

- Added Association SN IC `12` built-in object
  (`CosemAssociationSnObject`) with class version `3`, exposing
  `object_list` (array of structure {`base_name`: long-int,
  `class_id`: long-unsigned, `version`: unsigned, `logical_name`:
  octet-string(6), `access_rights`: structure}),
  `access_rights_list` (array of structure),
  `security_setup_reference` (octet-string(6) LN to Security
  Setup), `user_list` (array of structure {`user_id`: unsigned,
  `user_name`: visible-string}) and `current_user` (structure
  {`user_id`: unsigned, `user_name`: visible-string}) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed object list, access rights list,
  security setup reference and user lists after HLS authentication,
  HLS secret rotation or list mutations performed out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `reply_to_HLS_authentication`, `2`
  `change_HLS_secret`, `3` `add_object`, `4` `remove_object`,
  `5` `add_user` and `6` `remove_user` return `UnsupportedFeature`
  and clear method output (the built-in object does not perform
  authentication or list mutations); other method ids return
  `MethodNotFound`.

## 0.54.0 - 2026-06-15

- Added IEC Local Port Setup IC `19` built-in object
  (`CosemIecLocalPortSetupObject`) with class version `1`,
  exposing `default_mode` (enum), `default_baud` (enum),
  `proposed_baud` (enum), `response_time` (enum),
  `device_address` (octet-string with the device-address logical
  name) and `password_1` / `password_2` / `password_5` (octet-strings
  carrying the level-1, level-2 and level-5 passwords) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`9` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed mode, baud, response time, device
  address and passwords after configuration changes out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IC defines no methods; `InvokeMethod` reports `MethodNotFound`
  for all method ids and clears method output.

## 0.53.0 - 2026-06-15

- Added Data Protection IC `30` built-in object
  (`CosemDataProtectionObject`) with class version `0`, exposing
  `protection_buffer` (octet-string carrying the protected
  payload), `protection_object_list` (array of structure
  {`protection_type`: enum, `protection_options`: structure,
  `protection_parameters_id`: octet-string}),
  `protection_parameters_get` (array of structure
  {`protection_type`: enum, `protection_options`: structure}),
  `protection_parameters_set` (array of structure
  {`protection_type`: enum, `protection_options`: structure}) and
  `required_protection` (bit-string with authentication /
  encryption / digital-signature bits) as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed protection buffer, object list,
  parameter tables and required-protection mask after performing
  the protected operations out-of-band); logical_name (`1`) is
  read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `get_protected_attributes`, `2`
  `set_protected_attributes` and `3` `invoke_protected_method`
  return `UnsupportedFeature` and clear method output (the
  built-in object does not perform protected operations); other
  method ids return `MethodNotFound`.

## 0.52.0 - 2026-06-15

- Added Compact Data IC `62` built-in object
  (`CosemCompactDataObject`) with class version `0`, exposing
  `buffer` (octet-string carrying the compact-encoded data),
  `capture_objects` (array of structure {`class_id`: long-unsigned,
  `logical_name`: octet-string(6), `attribute_index`: integer,
  `data_index`: long-unsigned}), `template_id` (unsigned),
  `template_description` (octet-string with the A-XDR template)
  and `capture_method` (enum, `1` invoke / `2` implicit) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can refresh the compact buffer, template metadata and
  capture method after acquiring fresh capture data out-of-band);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `reset` and `2` `capture` return
  `UnsupportedFeature` and clear method output (the built-in object
  does not manage capture); other method ids return
  `MethodNotFound`.

## 0.51.0 - 2026-06-15

- Added Parameter Monitor IC `65` built-in object
  (`CosemParameterMonitorObject`) with class version `0`, exposing
  `changed_parameter` (structure {`class_id`: long-unsigned,
  `logical_name`: octet-string(6), `attribute_index`: integer,
  `value`: data}), `capture_time` (date-time octet-string(12)) and
  `parameters` (array of structure {`class_id`, `logical_name`,
  `attribute_index`}) as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish the most recent changed parameter, capture
  time and monitored-parameters table after evaluating parameter
  changes out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `insert` and `2` `delete` return `UnsupportedFeature`
  and clear method output (the built-in object does not manage the
  monitored-parameters table); other method ids return
  `MethodNotFound`.

## 0.50.0 - 2026-06-15

- Added Status Mapping IC `63` built-in object
  (`CosemStatusMappingObject`) with class version `0`, exposing
  `status_word` (bit-string carrying the raw status value) and
  `mappings` (array of structure {`status_value`: bit-string,
  `mapped_value`: bit-string}) as opaque encoded DLMS Data buffers
  prepared by the caller.
- Attributes `2`-`3` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed status and mapping tables after
  evaluating the status word out-of-band); logical_name (`1`) is
  read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Status Mapping IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.49.0 - 2026-06-15

- Added Arbitrator IC `68` built-in object
  (`CosemArbitratorObject`) with class version `0`, exposing
  `actions` (array of structure {`script_logical_name`,
  `script_selector`}), `permissions_table` (array of bit-string,
  one row per actor), `weightings_table` (array of array of
  long-unsigned), `most_recent_requests_table` (array of
  bit-string) and `last_outcome` (unsigned, index of the winning
  script) as opaque encoded DLMS Data buffers prepared by the
  caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed arbitration state after driving
  arbitration out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `request_action` and `2` `reset` return
  `UnsupportedFeature` and clear method output (the built-in object
  does not own arbitration semantics); other method ids return
  `MethodNotFound`.

## 0.48.0 - 2026-06-15

- Added Sensor Manager IC `67` built-in object
  (`CosemSensorManagerObject`) with class version `0`, exposing
  `status` (enum), `serial_number` (octet-string), `device_type`
  (octet-string), `manufacturer_id` (long-unsigned),
  `firmware_version` (octet-string), `metrology_firmware_version`
  (octet-string), `driver` (octet-string), `communication_desc`,
  `setup_desc` and `measurement_desc` (arrays of structure) as
  opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`11` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed sensor metadata after polling the
  slave out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Sensor Manager IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.47.0 - 2026-06-15

- Added Utility Tables IC `26` built-in object
  (`CosemUtilityTablesObject`) with class version `0`, exposing
  `table_id` (long-unsigned), `length` (double-long-unsigned) and
  `buffer` (octet-string carrying the raw table payload) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Utility Tables IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.46.0 - 2026-06-15

- Added IPv6 Setup IC `48` built-in object (`CosemIpv6SetupObject`)
  with class version `0`, exposing `data_link_layer_reference`
  (octet-string LN), `address_config_mode` (enum),
  `unicast_ip_address` and `multicast_ip_address` (arrays of
   16-byte octet-string), `gateway_ip_address`,
  `primary_dns_address`, `secondary_dns_address` (16-byte
  octet-string), `traffic_class` (unsigned) and
  `neighbor_discovery_setup` (array of structure) as opaque
  encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`10` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can republish refreshed addressing after running the
  network stack out-of-band); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `add_address` and `2` `remove_address` return
  `UnsupportedFeature` and clear method output (the built-in object
  does not own network-stack semantics); other method ids return
  `MethodNotFound`.

## 0.45.0 - 2026-06-15

- Added M-Bus slave port setup IC `25` built-in object
  (`CosemMBusSlavePortSetupObject`) with class version `0`, exposing
  `default_baud` (enum), `available_baud` (enum), `status` (enum)
  and `mbus_port_reference` (octet-string referencing an IEC HDLC
  Setup logical name) as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`5` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature` and clears method
  output (the built-in object does not own slave-port reset
  semantics); other method ids return `MethodNotFound`.

## 0.44.0 - 2026-06-15

- Added IEC twisted pair (1) Setup IC `24` built-in object
  (`CosemIecTwistedPairSetupObject`) with class version `0`,
  exposing `primary_address` (long-unsigned) and `tabis` (array of
  long-unsigned listing the registered secondary addresses) as
  opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2` and `3` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- IEC twisted pair (1) Setup IC defines no methods; `InvokeMethod`
  reports `MethodNotFound` for all method ids and clears method
  output.

## 0.43.0 - 2026-06-15

- Added GSM Diagnostic IC `47` built-in object
  (`CosemGsmDiagnosticObject`) with class version `0`, exposing
  `operator` (octet-string), `status` (enum),
  `circuit_switched_status` (enum), `packet_switched_status` (enum),
  `cell_info` (structure of cell_id/location_id/signal_quality/ber/
  mcc/mnc/channel_number), `adjacent_cells` (array of cell_id/
  signal_quality structures) and `capture_time` (date_time
  octet-string) as opaque encoded DLMS Data buffers prepared by the
  caller.
- Attributes `2`-`8` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted, so the
  backend can publish refreshed diagnostic snapshots when read-write
  is granted); logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Method `1` `reset` returns `UnsupportedFeature` and clears method
  output (the built-in object does not own modem reset semantics);
  other method ids return `MethodNotFound`.

## 0.42.0 - 2026-06-15

- Added SMTP Setup IC `46` built-in object (`CosemSmtpSetupObject`)
  with class version `0`, exposing `SMTP_server` (octet-string),
  `SMTP_server_port` (long-unsigned), `user_name` (octet-string),
  `login_password` (octet-string), `sender` (octet-string) and
  `receivers` (array of octet-string) as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `2`-`7` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- SMTP Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.41.0 - 2026-06-15

- Added PPP Setup IC `44` built-in object (`CosemPppSetupObject`)
  with class version `0`, exposing `PHY_reference` (octet-string),
  `LCP_options` (array of structure), `IPCP_options` (array of
  structure) and `PPP_authentication` (structure of
  user_name/password) as opaque encoded DLMS Data buffers prepared by
  the caller.
- Attributes `2`-`5` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- PPP Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.40.0 - 2026-06-15

- Added MAC Address Setup IC `43` built-in object
  (`CosemMacAddressSetupObject`) with class version `0`, exposing
  `mac_address` (octet-string(6)) as an opaque encoded DLMS Data
  buffer prepared by the caller.
- Attribute `2` honors a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- MAC Address Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.39.0 - 2026-06-15

- Added IPv4 Setup IC `42` built-in object (`CosemIpv4SetupObject`)
  with class version `0`, exposing `DL_reference`, `IP_address`,
  `multicast_IP_address`, `IP_options`, `subnet_mask`,
  `gateway_IP_address`, `use_DHCP_flag`, `primary_DNS_address` and
  `secondary_DNS_address` as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`10` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Methods `1` `add_mc_IP_address` and `2` `delete_mc_IP_address`
  return `UnsupportedFeature` with cleared output (built-in object
  does not own multicast subscription policy); other method ids return
  `MethodNotFound` with cleared output.

## 0.38.0 - 2026-06-15

- Added Auto Answer IC `28` built-in object (`CosemAutoAnswerObject`)
  with class version `0`, exposing `mode`, `listening_window`,
  `status`, `number_of_calls` and `number_of_rings` as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`, `3`, `5` and `6` share a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) and status (`4`) are read-only.
- `SetStatus` exposes backend-driven refresh of status regardless of
  the access mode used for the mutable attributes.
- Constructors normalize versions above `MaxSupportedVersion`.
- Auto Answer IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.37.0 - 2026-06-15

- Added GPRS Modem Setup IC `45` built-in object
  (`CosemGprsModemSetupObject`) with class version `0`, exposing
  `APN`, `PIN code` and `quality_of_service` as opaque encoded DLMS
  Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- GPRS Modem Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.36.0 - 2026-06-15

- Added Auto Connect IC `29` built-in object (`CosemAutoConnectObject`)
  with class version `0`, exposing `mode`, `repetitions`,
  `repetition_delay`, `calling_window` and `destination_list` as
  opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Auto Connect IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.35.0 - 2026-06-15

- Added Modem Configuration IC `27` built-in object
  (`CosemModemConfigurationObject`) with class version `1`, exposing
  `communication_speed`, `initialisation_strings` and `modem_profile`
  as opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Modem Configuration IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.34.0 - 2026-06-15

- Added Single Action Schedule IC `22` built-in object
  (`CosemSingleActionScheduleObject`) with class version `0`, exposing
  `executed_script`, `type` and `execution_time` as opaque encoded
  DLMS Data buffers prepared by the caller.
- Attributes `2`-`4` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- Single Action Schedule IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.33.0 - 2026-06-15

- Added Special Days Table IC `11` built-in object
  (`CosemSpecialDaysTableObject`) with class version `0`, exposing
  `entries` (array of special_day_entry) as an opaque encoded DLMS
  Data buffer prepared by the caller.
- Attribute `2` (entries) honors a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only. A setter exposes
  backend-driven refresh of entries regardless of access mode.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Special Days Table methods `1` `insert` and `2` `delete` as
  explicit `UnsupportedFeature` (application-defined special-day
  entry mutation); other method ids report `MethodNotFound`.

## 0.32.0 - 2026-06-15

- Added Schedule IC `10` built-in object (`CosemScheduleObject`) with
  class version `0`, exposing `entries` (array of
  Schedule_table_entry) as an opaque encoded DLMS Data buffer prepared
  by the caller.
- Attribute `2` (entries) honors a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place
  when permitted); logical_name (`1`) is read-only. A setter exposes
  backend-driven refresh of entries regardless of access mode.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Schedule methods `1` `insert` and `2` `delete` as explicit
  `UnsupportedFeature` (application-defined schedule-entry mutation);
  other method ids report `MethodNotFound`.

## 0.31.0 - 2026-06-15

- Added TCP-UDP Setup IC `41` built-in object
  (`CosemTcpUdpSetupObject`) with class version `0`, exposing
  `tcp_udp_port`, `ip_reference`, `mss`, `nb_of_sim_conn` and
  `inactivity_time_out` as opaque encoded DLMS Data buffers prepared
  by the caller.
- Attributes `2`-`6` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name (`1`) is read-only.
- Constructors normalize versions above `MaxSupportedVersion`.
- TCP-UDP Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.30.0 - 2026-06-15

- Added Register Table IC `61` built-in object
  (`CosemRegisterTableObject`) with class version `0`, exposing
  `table_cell_values`, `single_buffer`, `table_cell_definition` and
  `table_entries` as opaque encoded DLMS Data buffers prepared by the
  caller.
- Attributes `3`-`5` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name and table_cell_values (`1`, `2`) are read-only with a
  setter that exposes backend-driven refresh of `table_cell_values`
  from a future register-table backend.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Register Table methods `1` `table_entry` and `2`
  `update_table_entry` as explicit `UnsupportedFeature`
  (application-defined column selection and update); other method ids
  report `MethodNotFound`.

## 0.29.0 - 2026-06-15

- Added IEC HDLC Setup IC `23` built-in object
  (`CosemIecHdlcSetupObject`) with `MaxSupportedVersion = 1`, exposing
  `comm_speed`, `window_size_transmit`, `window_size_receive`,
  `max_info_field_length_transmit`, `max_info_field_length_receive`,
  `inter_octet_time_out`, `inactivity_time_out` and `device_address`
  as opaque encoded DLMS Data buffers prepared by the caller.
- Attributes `2`-`8` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name and device_address (`1`, `9`) are read-only with a
  setter that exposes backend-driven refresh of the assigned HDLC
  address.
- Constructors normalize versions above `MaxSupportedVersion`.
- IEC HDLC Setup IC defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.28.0 - 2026-06-15

- Added Limiter IC `71` built-in object (`CosemLimiterObject`) with
  class version `0`, exposing `monitored_value`, `threshold_active`,
  `threshold_normal`, `threshold_emergency`,
  `min_over_threshold_duration`, `min_under_threshold_duration`,
  `emergency_profile`, `emergency_profile_group_id_list`,
  `emergency_profile_active` and `actions` as opaque encoded DLMS Data
  buffers prepared by the caller.
- Attributes `3`-`11` share a caller-selected `AttributeAccessMode`
  (writes replace the stored buffer in-place when permitted);
  logical_name and monitored_value (`1`, `2`) are read-only with
  setters that expose backend-driven refresh of `threshold_active` and
  `emergency_profile_active` from a future limiter backend.
- Constructors normalize versions above `MaxSupportedVersion`.
- Limiter IC v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for all method ids and clears method output.

## 0.27.0 - 2026-06-15

- Added Disconnect Control IC `70` built-in object
  (`CosemDisconnectControlObject`) with class version `0`, exposing
  `output_state`, `control_state` and `control_mode` as opaque encoded
  DLMS Data buffers prepared by the caller.
- `control_mode` (attribute 4) shares a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place when
  permitted); logical_name, `output_state` and `control_state` are
  read-only with setters that expose backend-driven refresh of the
  output/control state from a future relay backend.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Disconnect Control methods `1` `remote_disconnect` and `2`
  `remote_reconnect` as explicit `UnsupportedFeature`
  (application-defined relay switching and state transitions); other
  method ids report `MethodNotFound`.

## 0.26.0 - 2026-06-15

- Added Push Setup IC `40` built-in object (`CosemPushSetupObject`) with
  `MaxSupportedVersion = 1`, exposing the v0 surface
  (`push_object_list`, `send_destination_and_method`,
  `communication_window`, `randomisation_start_interval`,
  `number_of_retries`, `repetition_delay`) and the v1 surface
  (`port_reference`, `push_client_SAP`, `push_protection_parameters`,
  `push_operation_method`, `confirmation_parameters`,
  `last_confirmation_date_time`) as opaque encoded DLMS Data buffers
  prepared by the caller.
- v0 objects hide attributes 8-13 (`AttributeNotFound` on read and
  write). v1 objects expose attributes 2-12 with a caller-selected
  `AttributeAccessMode`; logical_name and `last_confirmation_date_time`
  (`13`) are read-only, and a setter exposes backend-driven refresh of
  the last-confirmation timestamp.
- Constructors normalize versions above `MaxSupportedVersion`.
- Added Push Setup method `1` `push` as explicit `UnsupportedFeature`
  (application-defined scheduling, transport selection and
  confirmation tracking); other method ids report `MethodNotFound`.

## 0.25.0 - 2026-06-15

- Added Image Transfer IC `18` built-in object
  (`CosemImageTransferObject`) with class version `0`, exposing
  attributes `1` logical_name, `2` image_block_size, `3`
  image_transferred_blocks_status, `4`
  image_first_not_transferred_block_number, `5`
  image_transfer_enabled, `6` image_transfer_status and `7`
  image_to_activate_info as opaque encoded DLMS Data buffers prepared
  by the caller.
- `image_transfer_enabled` (attribute 5) shares a caller-selected
  `AttributeAccessMode` (writes replace the stored buffer in-place when
  permitted); logical_name and all other attributes (2-4, 6-7) are
  read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Image Transfer; constructors normalize versions
  above the maximum. Setters expose dynamic refresh of
  transferred-blocks status, first-not-transferred counter,
  image-transfer status and image-to-activate info.
- Added Image Transfer methods `1` `image_transfer_initiate`, `2`
  `image_block_transfer`, `3` `image_verify` and `4` `image_activate`
  as explicit `UnsupportedFeature` (application-defined firmware
  transfer/storage semantics); other method ids report
  `MethodNotFound`.

## 0.24.0 - 2026-06-15

- Added Activity Calendar IC `20` built-in object
  (`CosemActivityCalendarObject`) with class version `0`, exposing
  attributes `1` logical_name, `2`-`5` active calendar snapshot
  (calendar_name_active, season_profile_active,
  week_profile_table_active, day_profile_table_active), `6`-`9` passive
  calendar (calendar_name_passive, season_profile_passive,
  week_profile_table_passive, day_profile_table_passive) and `10`
  activate_passive_calendar_time as opaque encoded DLMS Data buffers
  prepared by the caller.
- Passive attributes (`6`-`10`) share a caller-selected access mode
  (writes replace the stored buffer in-place when permitted); logical_name
  and active snapshot attributes (`2`-`5`) are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Activity Calendar; constructors normalize versions
  above the maximum.
- Added Activity Calendar method `1` `activate_passive_calendar` as
  explicit `UnsupportedFeature` (application-defined activation policy);
  other method ids report `MethodNotFound`.

## 0.23.0 - 2026-06-15

- Added Script Table IC `9` built-in object
  (`CosemScriptTableObject`) with class version `0`, exposing attributes
  `1` logical_name and `2` scripts as an opaque encoded DLMS Data buffer
  prepared by the caller. `scripts` access mode is caller-selected.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Script Table; constructors normalize versions above the
  maximum.
- Added Script Table method `1` `execute` as explicit `UnsupportedFeature`
  (application-defined script semantics); other method ids report
  `MethodNotFound`.

## 0.22.0 - 2026-06-15

- Added Register Monitor IC `21` built-in object
  (`CosemRegisterMonitorObject`) with class version `0`, exposing
  attributes `1` logical_name, `2` thresholds, `3` monitored_value and
  `4` actions as opaque encoded DLMS Data buffers prepared by the caller.
  `thresholds` access mode is caller-selected; `monitored_value` and
  `actions` are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Register Monitor; constructors normalize versions above
  the maximum.
- Register Monitor v0 defines no methods; `InvokeMethod` reports
  `MethodNotFound` for every method id.

## 0.21.0 - 2026-06-15

- Added Register Activation IC `6` built-in object
  (`CosemRegisterActivationObject`) with class version `0`, exposing
  attributes `1` logical_name, `2` register_assignment, `3` mask_list and
  `4` active_mask as opaque encoded DLMS Data buffers prepared by the caller.
  All attributes are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking
  constructor for Register Activation; constructors normalize versions above
  the maximum.
- Added Register Activation methods `1` `add_register`, `2` `add_mask` and
  `3` `delete_mask` as explicit `UnsupportedFeature` (application-defined
  semantics); other method ids report `MethodNotFound`.

## 0.20.0 - 2026-06-15

- Added Demand Register IC `5` built-in object
  (`CosemDemandRegisterObject`) with class version `0`, exposing attributes
  `1` logical_name, `2` current_average_value, `3` last_average_value,
  `4` scaler_unit, `5` status, `6` capture_time, `7` start_time_current,
  `8` period (encoded as DLMS Data `double-long-unsigned`) and
  `9` number_of_periods (encoded as DLMS Data `long-unsigned`). All
  attributes are read-only.
- Added explicit `MaxSupportedVersion` constant and version-taking constructor
  for Demand Register; constructors normalize versions above the maximum.
- Added Demand Register methods `1` `reset` and `2` `next_period` as explicit
  `UnsupportedFeature` (application-defined semantics); other method ids
  report `MethodNotFound`.

## 0.19.0 - 2026-06-15

- Added Extended Register IC `4` built-in object
  (`CosemExtendedRegisterObject`) with class version `0`, exposing attributes
  `1` logical_name (read-only), `2` value (caller-selected access mode),
  `3` scaler_unit (read-only), `4` status (read-only) and `5` capture_time
  (read-only DLMS date-time octet-string).
- Added explicit `MaxSupportedVersion` constant and version-taking constructor
  for Extended Register; constructors normalize versions above the maximum.
- Added Extended Register method `1` `reset` as explicit `UnsupportedFeature`
  (application-defined semantics); other method ids report `MethodNotFound`.

## 0.18.0 - 2026-06-15

- Added pluggable `ICosemCertificateStore` interface and
  `InMemoryCosemCertificateStore` reference backend for Security Setup IC `64`
  version `1`.
- Changed Security Setup attribute `6` `certificates` to encode the attached
  certificate store entries as a DLMS Data array of `certificate_info`
  structures (entity enum, type enum, serial / issuer / subject /
  subject-alt-name octet-strings).
- Added Security Setup method `6` `import_certificate` (octet-string payload),
  method `7` `export_certificate` and method `8` `remove_certificate` with
  Blue Book `by_entity` / `by_serial` selector parsing dispatched to the
  certificate store backend. Without an attached store these methods return
  `UnsupportedFeature` and clear method output. Methods `3`/`4`/`5` (key
  agreement, generate key pair, generate certificate request) remain
  `UnsupportedFeature` until an ECDSA / X.509 stack is wired in.

## 0.17.0 - 2026-06-14

- Added caller-selected descriptor version constructors and
  `MaxSupportedVersion` constants for built-in Data, Register, Clock,
  Profile Generic, SAP Assignment, and Security Setup COSEM objects.
- Changed built-in Security Setup to publish class version `1` by default,
  matching the implemented security attributes and key-transfer method surface.
- Added version-gated method exposure for Profile Generic version `0` legacy
  buffer methods and Security Setup version `0`/`1` method surfaces.
- Added version-gated Security Setup version `1` certificates attribute support
  as an encoded empty DLMS Data array when no certificate store is configured.
- Fixed HDLC session-mode segmented inbound APDU handling so RR frames
  acknowledge each accepted I-frame segment with the updated `N(R)` value.

## 0.16.2 - 2026-06-13

- Fixed HDLC session teardown so owned HDLC/TCP clients perform the mandatory
  DISC/UA data-link disconnect after application association release and before
  closing the underlying transport.
- Fixed confirmed release against meters that return `RLRE` with
  `user-information` and non-canonical BER length fields around the embedded
  xDLMS `InitiateResponse`.

## 0.16.1 - 2026-06-13

- Fixed HDLC session-mode APDU exchange so a final client I-frame no longer
  waits for a separate RR acknowledgement, and piggybacked server I-frame
  responses are queued for `ReceiveApdu()` instead of being consumed as control
  acknowledgements.

## 0.16.0 - 2026-06-13

- Added explicit HDLC client, logical-device, and physical-device addressing
  fields to endpoint/profile options, so COSEM SAPs are no longer reused as
  HDLC link-layer addresses.
- Added HDLC profile wire trace hooks and changed Wrapper/TCP trace event names
  to `WireWrite` and `WireRead` for wire-level diagnostics.

## 0.15.0 - 2026-06-07

- Added version-gated Association LN configuration up to class version `3`,
  including user-list/current-user attributes for version `2+`.

## 0.14.0 - 2026-06-07

- Added Association LN status and optional security setup reference attributes,
  and exposed documented Association LN methods as explicit unsupported
  features.

## 0.13.0 - 2026-06-07

- Added a built-in partial Clock IC `8` object with read/write support for
  documented clock attributes and explicit unsupported clock methods.

## 0.12.1 - 2026-06-07

- Added a GUI-oriented СПОДЭС OBIS read example and allowed Profile Generic
  buffer row decoding to validate `date-time`, `date`, and `time` values.

## 0.12.0 - 2026-06-07

- Added DLMS `date-time`, `date`, and `time` Data codec support plus
  GUI-facing typed client helpers.

## 0.11.0 - 2026-06-07

- Added Association LN object-list and access-rights encode/decode helpers for
  the built-in COSEM discovery model.

## 0.10.0 - 2026-06-07

- Added Profile Generic selective access range and entry descriptor helpers for
  building and validating selector `1` and selector `2` parameters.

## 0.9.0 - 2026-06-07

- Added Profile Generic composite attribute encode/decode helpers for
  `capture_objects`, `sort_object`, and `buffer` row framing.

## 0.8.0 - 2026-06-07

- Added xDLMS and client facade support for GET selective access requests with
  encoded DLMS Data selection parameters.

## 0.7.1 - 2026-06-07

- Added a GUI-oriented client read example that uses `ReadAttribute` and typed
  DLMS Data decoding without requiring application code to parse A-XDR bytes.

## 0.7.0 - 2026-06-07

- Added `dlms-client` typed DLMS Data encode/decode helpers for GUI consumers
  that should not manipulate A-XDR bytes directly for common scalar values.

## 0.6.0 - 2026-06-07

- Added detailed `DlmsClient` attribute/method helpers for GUI consumers that
  need class-id/OBIS calls plus access/action result details.

## 0.5.0 - 2026-06-07

- Added a built-in partial Profile Generic IC `7` object with read-only
  profile attributes and explicit unsupported reset/capture methods.

## 0.4.11 - 2026-06-07

- Reconciled the IC and security production-readiness documentation against
  the knowledge-base class lists and current built-in COSEM coverage.

## 0.4.10 - 2026-06-07

- Extended the package install smoke audit to verify aggregate target
  `INTERFACE_LINK_LIBRARIES` for all documented DLMSFramework components.

## 0.4.9 - 2026-06-07

- Added profile C API regression coverage for datagram receive callbacks that
  return a non-OK status after reporting bytes.

## 0.4.8 - 2026-06-07

- Hardened transport C API read and receive entry points so `bytes_read`
  remains zero for non-OK backend statuses.

## 0.4.7 - 2026-06-07

- Hardened the association C API callback APDU adapter so receive callbacks
  cannot leave stale output or accept over-reported received sizes.

## 0.4.6 - 2026-06-07

- Documented the complete `DLMSFramework` component-to-target package mapping
  in the README and release versioning notes.

## 0.4.5 - 2026-06-07

- Added install-tree consumer examples for the `io`, `cosem_server`, and
  `framework` package components and included them in package smoke coverage.

## 0.4.4 - 2026-06-07

- Added an io-only install-tree smoke test with OpenSSL disabled to verify that
  `find_package(DLMSFramework COMPONENTS io)` stays independent from security
  dependencies.

## 0.4.3 - 2026-06-07

- Scoped the package OpenSSL dependency to components that transitively use
  security and added a codec-only install-tree smoke test with OpenSSL disabled.

## 0.4.2 - 2026-06-07

- Cleaned up exported CMake targets so installed concrete targets expose the
  package include directory only once.

## 0.4.1 - 2026-06-07

- Hardened profile mutable receive small-buffer failures so Wrapper TCP,
  Wrapper UDP and HDLC channels report the required APDU size.

## 0.4.0 - 2026-06-07

- Added the Wrapper C API stream decoder push entry point so C consumers can
  incrementally decode WPDU streams and drain pending frames.

## 0.3.48 - 2026-06-07

- Hardened HDLC C stream decoder and reassembler small-buffer failures so the
  information size output reports the required payload size.

## 0.3.47 - 2026-06-07

- Hardened endpoint security bundle creation so validation failures reset the
  caller-provided security bundle output.

## 0.3.46 - 2026-06-07

- Hardened xDLMS server dispatcher failures so stale get, set and action
  result objects are cleared on validation and handler errors.

## 0.3.45 - 2026-06-07

- Hardened xDLMS server adapter status-level failures so stale get, set and
  action result objects are cleared before returning an error status.

## 0.3.44 - 2026-06-07

- Hardened COSEM object registry read and method invocation failures so stale
  caller output buffers are cleared on missing objects, denied access and
  object-level errors.

## 0.3.43 - 2026-06-07

- Hardened built-in simple COSEM objects so missing attributes and methods
  clear stale output buffers on public object API failures.

## 0.3.42 - 2026-06-07

- Hardened Security Setup method invocation failures so stale COSEM output
  data is cleared for invalid activation requests and unsupported methods.

## 0.3.41 - 2026-06-07

- Hardened Security Setup key transfer so invocation-counter reset policy
  failures reject key rotation before installing transferred keys.

## 0.3.40 - 2026-06-07

- Hardened HDLC C decode buffer validation to allow a null information buffer
  only when its size is zero.

## 0.3.39 - 2026-06-06

- Added Security Setup regression coverage for the currently unsupported
  method id range.

## 0.3.38 - 2026-06-06

- Added Security Setup key-transfer regression coverage for unsupported
  security suites.

## 0.3.37 - 2026-06-06

- Hardened transport C write/send entry points to reject null input with a
  non-zero size at the C ABI boundary.

## 0.3.36 - 2026-06-06

- Hardened profile C callback adapters so failed read/receive callbacks do not
  propagate non-zero byte counts.

## 0.3.35 - 2026-06-06

- Added Security Setup key-transfer malformed input regression coverage for
  unsupported key identifiers and trailing bytes.

## 0.3.34 - 2026-06-06

- Added invocation-counter reset policy hook for key rotation.
- Updated `InMemoryInvocationCounterStore` to reset local and remote replay
  state after key rotation.
- Wired Security Setup key transfer to invoke the reset hook after successful
  key installation.

## 0.3.33 - 2026-06-06

- Wired Security Setup IC `64` method `global_key_transfer` for Suite 0 key
  transfer through the mutable key store abstraction.
- Added AXDR key-data parsing for wrapped global unicast, broadcast,
  authentication and key-encryption keys.
- Added COSEM regression coverage for installing an unwrapped authentication
  key through Security Setup method 2.

## 0.3.32 - 2026-06-06

- Added `IMutableKeyStore` as the public mutable key sink contract for
  Security Setup key transfer.
- Updated `InMemoryKeyStore` to implement the mutable key store abstraction.
- Added regression coverage for writing keys through the abstract interface.

## 0.3.31 - 2026-06-06

- Added Suite 0 AES key wrap/unwrap primitives for Security Setup key transfer.
- Added RFC 3394 regression coverage for wrapping and unwrapping 128-bit keys
  with a 128-bit key encryption key.
- Updated the security roadmap and support matrix for the new key wrapping
  primitive.

## 0.3.30 - 2026-06-06

- Added endpoint lifecycle regression coverage for retrying HLS GMAC
  authentication after an invalid client reply.
- Verified `ServerEndpoint` remains open, unassociated and able to accept a
  later valid GMAC HLS reply after the first verification failure.
- Updated the production-ready roadmap for HLS GMAC negotiated failure
  coverage.

## 0.3.29 - 2026-06-06

- Added endpoint lifecycle regression coverage for retrying high-password HLS
  authentication after an invalid client reply.
- Verified `ServerEndpoint` remains open, unassociated and able to accept a
  later valid HLS reply after the first HLS verification failure.
- Updated the production-ready roadmap for high-authentication negotiated
  failure coverage.

## 0.3.28 - 2026-06-06

- Added endpoint lifecycle regression coverage for retrying negotiated server
  and push listener opens after malformed association requests.
- Verified failed negotiated opens close the APDU channel and do not leave
  endpoints logically open.
- Updated the production-ready roadmap for negotiated open-failure cleanup
  coverage.

## 0.3.27 - 2026-06-06

- Added endpoint lifecycle regression coverage for gateway downstream close
  failures.
- Verified `GatewayEndpoint` remains logically open when the downstream APDU
  channel reports a close failure.
- Updated the production-ready roadmap for gateway close-failure lifecycle
  coverage.

## 0.3.26 - 2026-06-06

- Added endpoint lifecycle regression coverage for close failures on server and
  push listener endpoints.
- Verified endpoints remain logically open when their underlying APDU channel
  reports a close failure.
- Updated the production-ready roadmap for close-failure lifecycle coverage.

## 0.3.25 - 2026-06-06

- Added endpoint lifecycle regression coverage for idempotent `Open()` on
  server, push listener and gateway endpoints.
- Verified repeated `Open()` calls do not reopen downstream channels or
  gateway upstreams when endpoints are already open.
- Updated the production-ready roadmap for endpoint lifecycle idempotency
  coverage.

## 0.3.24 - 2026-06-06

- Added sender-aware invocation counter validation to the invocation counter
  store abstraction.
- Updated `InMemoryInvocationCounterStore` to track remote replay state per
  remote system title.
- Updated protected APDU and HLS GMAC verification paths to validate remote
  invocation counters against the remote system title.
- Added regression coverage for independent replay state across remote system
  titles.

## 0.3.23 - 2026-06-06

- Added `InvocationCounterObjectName()` for the public invocation counter OBIS
  `0.0.43.1.0.255`.
- Added `MakeInvocationCounterObject()` to expose the current invocation
  counter as a read-only COSEM Data object encoded as AXDR
  `double-long-unsigned`.
- Added deterministic COSEM coverage for the public invocation counter object.

## 0.3.22 - 2026-06-06

- Added regression coverage for invocation counter exhaustion through
  `CipheredApduProcessor::Protect`.
- Added regression coverage for invocation counter exhaustion through HLS GMAC
  response generation.
- Updated security documentation to mark local invocation counter exhaustion
  refusal as covered at the protected APDU and HLS GMAC boundaries.

## 0.3.21 - 2026-06-06

- Implemented Security Setup IC `64` `security_activate` method for encoded
  security policy activation.
- Enforced monotonic security policy activation so active policy bits cannot be
  weakened through `security_activate`.
- Added direct and registry regression coverage for Security Setup activation,
  malformed activation input and policy weakening rejection.

## 0.3.20 - 2026-06-06

- Added `CosemSecuritySetupObject` for Security Setup IC `64` as a read-only
  COSEM extension point exposing security policy, security suite and
  client/server system titles.
- Added explicit unsupported method behavior for Security Setup activation,
  key transfer, key agreement and certificate operation slots.
- Updated security, IC and production-ready roadmap documentation for the
  current Security Setup support level.

## 0.3.19 - 2026-06-06

- Fixed Profile C API receive validation so invalid caller output buffers and
  null `written_size` are rejected at the C ABI boundary before the underlying
  APDU channel is called.
- Documented the Profile receive output-buffer validation contract.

## 0.3.18 - 2026-06-06

- Fixed HDLC strict encode and C API encode small-buffer handling so
  `written_size` reports the required encoded frame size on
  `OutputBufferTooSmall` without writing a partial frame.
- Documented the HDLC small-output-buffer size contract.

## 0.3.17 - 2026-06-06

- Fixed Wrapper strict encode and C API encode small-buffer handling so
  `written_size` reports the required WPDU size on `OutputBufferTooSmall`
  without writing a partial WPDU.
- Documented the Wrapper small-output-buffer size contract.

## 0.3.16 - 2026-06-06

- Added the production-ready roadmap based on code review findings.
- Fixed APDU raw xDLMS C API small-buffer handling so `written_size` reports
  the required encoded APDU size on `OutputBufferTooSmall` without writing a
  partial APDU.
- Documented the APDU small-output-buffer size contract.

## 0.3.15 - 2026-06-06

- Fixed LLC strict encode and C API encode small-buffer handling so
  `written_size` reports the required LPDU size on `OutputBufferTooSmall`
  without writing a partial LPDU.
- Documented the LLC small-output-buffer size contract.

## 0.3.14 - 2026-06-06

- Refreshed root, package, release, architecture, and component documentation
  for the monorepo component model and `DLMSFramework` CMake components.
- Documented current aggregate targets, install-tree consumer examples, and
  public extension points after layer modernization.
- Removed obsolete submodule and per-layer repository wording from active
  documentation and implementation plans.

## 0.3.13 - 2026-06-06

- Fixed LLC C API decode validation so null input with zero size follows the
  C++ codec contract and returns `NeedMoreData`.
- Added LLC C API regression coverage for empty null-input decode cleanup.

## 0.3.12 - 2026-06-06

- Fixed Wrapper C API decode validation so each provided output pointer is
  cleared before null output-pointer validation errors are returned.
- Added Wrapper C API regression coverage for partial output cleanup.

## 0.3.11 - 2026-06-06

- Fixed APDU C API encode validation so provided `written_size` outputs are
  cleared before null input validation errors.
- Added APDU C API regression coverage for null input output-size cleanup.

## 0.3.10 - 2026-06-06

- Fixed Association C API option validation so invalid application contexts,
  authentication modes, and null low-level credentials with non-zero sizes are
  rejected during handle creation.
- Added Association C API regression coverage for invalid option rejection.

## 0.3.9 - 2026-06-06

- Fixed HDLC C API decode, stream decoder, and reassembler entry points so
  provided output frame structures are cleared before validation errors.
- Added HDLC C API regression coverage for output frame cleanup.

## 0.3.8 - 2026-06-06

- Fixed Association C API result accessors so provided result outputs are
  cleared before validation errors are returned.
- Added Association C API regression coverage for get-result output cleanup.

## 0.3.7 - 2026-06-06

- Fixed Wrapper C API decode validation so null data output buffers are rejected
  before `data_size` is written for non-empty decoded payloads.
- Added Wrapper C API regression coverage for decode output cleanup.

## 0.3.6 - 2026-06-06

- Fixed Transport C API receive validation so null output buffers are rejected
  before lower transport calls when a non-zero output size is requested.
- Added Transport C API regression coverage for null receive output buffers.

## 0.3.5 - 2026-06-06

- Fixed Profile C API receive validation so a provided `written_size` output is
  cleared before `dlms_profile_receive_apdu()` returns validation errors.
- Added Profile C API regression coverage for receive output-size cleanup.

## 0.3.4 - 2026-06-06

- Fixed APDU C API xDLMS encode validation so null output buffers return
  `INVALID_ARGUMENT` while still clearing `written_size` when it is provided.
- Added an APDU C API guard for payload sizes that would overflow the encoded
  APDU size calculation.
- Added APDU C API regression coverage for those edge cases.

## 0.3.3 - 2026-06-06

- Fixed HDLC C API limit conversion so zero fields in `dlms_hdlc_limits_t`
  preserve the documented default limits.
- Fixed HDLC C API stream decoder error handling to reset the decoder after
  codec errors.
- Fixed HDLC C API stream decoder and reassembler payload output validation to
  reject null information buffers when a non-empty Information field is
  returned.
- Added HDLC C API regression coverage for those edge cases.

## 0.3.2 - 2026-06-05

- Fixed listener runtimes to close accepted APDU channels when constructing or
  opening the per-connection server, push, or gateway endpoint fails.
- Added listener runtime regression coverage for accepted-channel cleanup on
  endpoint open failures.

## 0.3.1 - 2026-06-05

- Added install-tree package consumer examples for `dlms::codec`,
  `dlms::protocol`, and `dlms::runtime`.
- Extended package install smoke verification to build those examples against
  the installed `DLMSFramework` package and audit exported CMake target files.
- Documented CMake components in the versioning and release checklist docs.

## 0.3.0 - 2026-06-05

- Added CMake package component support for the documented aggregate targets:
  `codec`, `io`, `protocol`, `cosem_server`, `runtime`, and `framework`.
- Extended the install package smoke test to require those components through
  `find_package(DLMSFramework COMPONENTS ...)`.

## 0.2.2 - 2026-06-05

- Added `README.md`, `CHANGELOG.md`, and `VERSION` to the installed
  `DLMSFramework` package metadata under `share/doc/DLMSFramework`.
- Extended the package artifact smoke test to require those release metadata
  files in generated ZIP artifacts.

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
