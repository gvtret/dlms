# dlms-cosem API

## 1. Public Headers

Planned phase 1 headers:

```text
include/dlms/cosem/cosem_status.hpp
include/dlms/cosem/cosem_types.hpp
include/dlms/cosem/cosem_object.hpp
include/dlms/cosem/object_registry.hpp
include/dlms/cosem/logical_device_interface.hpp
include/dlms/cosem/logical_device.hpp
include/dlms/cosem/simple_objects.hpp
```

No C ABI is planned for the first implementation.

## 2. Status

`CosemStatus` shall be a stable status contract:

- `Ok`
- `InvalidArgument`
- `DuplicateObject`
- `ObjectNotFound`
- `AttributeNotFound`
- `MethodNotFound`
- `AccessDenied`
- `OutputBufferTooSmall`
- `UnsupportedFeature`
- `ObjectError`
- `InternalError`

## 3. Types

`CosemLogicalName` is a six-byte logical-name value.

`CosemObjectKey` contains:

- `classId`
- `logicalName`

`CosemObjectDescriptor` contains:

- `classId`
- `version`
- `logicalName`

`CosemAttributeDescriptor` contains:

- `object`
- `attributeId`

`CosemMethodDescriptor` contains:

- `object`
- `methodId`

`CosemByteBuffer` is the first-phase encoded xDLMS data container.

## 4. Access Rights

`AttributeAccessMode`:

- `NoAccess`
- `ReadOnly`
- `WriteOnly`
- `ReadAndWrite`
- `AuthenticatedReadOnly`
- `AuthenticatedWriteOnly`
- `AuthenticatedReadAndWrite`

`MethodAccessMode`:

- `NoAccess`
- `Access`
- `AuthenticatedAccess`

`CosemAccessRights` contains attribute and method access entries for one
object. The first implementation stores explicit entries only; missing entries
mean no access.

## 5. Object Interface

```cpp
class ICosemObject
{
public:
  virtual ~ICosemObject();
  virtual CosemObjectDescriptor Descriptor() const = 0;
  virtual CosemAccessRights AccessRights() const = 0;
  virtual CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const = 0;
  virtual CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input) = 0;
  virtual CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output) = 0;
};
```

## 6. Registry API

```cpp
ObjectRegistry registry;
registry.Register(object);

const ICosemObject* object = registry.Find(key);
registry.ReadAttribute(attribute, output);
registry.WriteAttribute(attribute, input);
registry.InvokeMethod(method, input, output);
registry.BuildAssociationView(view);
```

## 7. Logical Device Interface

`ILogicalDevice` is the abstract server dispatch boundary for applications
that own their COSEM storage outside the default in-memory `LogicalDevice`:

```cpp
class ILogicalDevice
{
public:
  virtual ~ILogicalDevice();

  virtual CosemStatus ReadAttribute(
    const CosemAttributeDescriptor& descriptor,
    const CosemAccessContext& context,
    CosemByteBuffer& output) const = 0;
  virtual CosemStatus WriteAttribute(
    const CosemAttributeDescriptor& descriptor,
    const CosemAccessContext& context,
    const CosemByteBuffer& input) = 0;
  virtual CosemStatus InvokeMethod(
    const CosemMethodDescriptor& descriptor,
    const CosemAccessContext& context,
    const CosemByteBuffer& input,
    CosemByteBuffer& output) = 0;
};
```

`ILogicalDevice` lives in `logical_device_interface.hpp`. Applications that
only implement custom COSEM storage can include that interface header without
including the default logical/physical device declarations. `LogicalDevice`
lives in `logical_device.hpp`, is the default implementation over
`ObjectRegistry`, and implements `ILogicalDevice`.

## 8. Module Diagram

```mermaid
classDiagram
  class ILogicalDevice {
    <<interface>>
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
  }

  class LogicalDevice {
    -ObjectRegistry registry
    -uint16 sap
    +RegisterObject()
    +Registry()
    +BuildAssociationView()
  }

  class ObjectRegistry {
    -vector objects
    +Register()
    +Find()
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
  }

  class ICosemObject {
    +Descriptor()
    +AccessRights()
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
  }

  class CosemAccessRights {
    +attributeAccess
    +methodAccess
  }

  LogicalDevice --|> ILogicalDevice
  LogicalDevice --> ObjectRegistry
  ObjectRegistry --> ICosemObject
  ICosemObject --> CosemAccessRights
```

## 9. Simple Interface Objects

`simple_objects.hpp` adds reusable in-memory implementations for the first
concrete COSEM interface classes:

```cpp
class CosemDataObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    AttributeAccessMode valueAccess);
  CosemDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    AttributeAccessMode valueAccess,
    std::uint8_t version);

  const CosemByteBuffer& Value() const;
  void SetValue(const CosemByteBuffer& value);
};

class CosemRegisterObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode valueAccess);
  CosemRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode valueAccess,
    std::uint8_t version);

  const CosemByteBuffer& Value() const;
  const CosemByteBuffer& ScalerUnit() const;
  void SetValue(const CosemByteBuffer& value);
  void SetScalerUnit(const CosemByteBuffer& scalerUnit);
};

class CosemClockObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemClockObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& time,
    std::int16_t timeZone,
    std::uint8_t status,
    const CosemByteBuffer& daylightSavingsBegin,
    const CosemByteBuffer& daylightSavingsEnd,
    std::int8_t daylightSavingsDeviation,
    bool daylightSavingsEnabled,
    CosemClockBase clockBase);
  CosemClockObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& time,
    std::int16_t timeZone,
    std::uint8_t status,
    const CosemByteBuffer& daylightSavingsBegin,
    const CosemByteBuffer& daylightSavingsEnd,
    std::int8_t daylightSavingsDeviation,
    bool daylightSavingsEnabled,
    CosemClockBase clockBase,
    std::uint8_t version);
};
```

The constructors create descriptors with class ids `1`, `3`, and `8`, version `0`.
Each class exposes `MaxSupportedVersion`; constructors that accept `version`
normalize values above the maximum to `MaxSupportedVersion`.
Attribute `1` is read-only logical name. Attribute `2` is the value. Register
attribute `3` is read-only scaler-unit. Methods are not supported in this
increment.

`simple_objects.hpp` also exposes a partial Extended Register IC `4`
(`CosemExtendedRegisterObject`) with class version `0`. The constructors take
the value, scaler-unit, status and capture_time payloads as encoded DLMS Data
octet buffers and accept a caller-selected value access mode. Attribute `1` is
read-only logical name; attribute `2` is the value; attribute `3` is read-only
scaler-unit; attribute `4` is read-only status; attribute `5` is read-only
capture_time (DLMS date-time octet-string). Method `1` `reset` returns
`UnsupportedFeature` (application-defined semantics); other method ids report
`MethodNotFound`.

`simple_objects.hpp` also exposes a partial Demand Register IC `5`
(`CosemDemandRegisterObject`) with class version `0`. The constructors take
the current_average_value, last_average_value, scaler_unit, status,
capture_time and start_time_current payloads as encoded DLMS Data buffers and
accept the `period` (seconds, encoded as DLMS Data `double-long-unsigned`)
and `number_of_periods` (encoded as DLMS Data `long-unsigned`) numeric values.
Attribute `1` is read-only logical name; attributes `2` and `3` are read-only
current/last average values; attribute `4` is read-only scaler-unit;
attribute `5` is read-only status; attribute `6` is read-only capture_time;
attribute `7` is read-only start_time_current; attribute `8` is read-only
period (encoded on read); attribute `9` is read-only number_of_periods
(encoded on read). Methods `1` `reset` and `2` `next_period` return explicit
`UnsupportedFeature` (application-defined semantics); other method ids report
`MethodNotFound`.

`simple_objects.hpp` also exposes a partial Register Activation IC `6`
(`CosemRegisterActivationObject`) with class version `0`. The constructors
take the register_assignment, mask_list and active_mask payloads as encoded
DLMS Data buffers prepared by the caller, plus the logical name and an
optional explicit version that is normalized to `MaxSupportedVersion` when
out of range. Attribute `1` is read-only logical name; attributes `2`, `3`
and `4` are read-only register_assignment, mask_list and active_mask
respectively. Methods `1` `add_register`, `2` `add_mask` and `3`
`delete_mask` mutate caller-owned assignment state and are surfaced as
`UnsupportedFeature`; other method ids report `MethodNotFound`.

`simple_objects.hpp` also exposes a partial Register Monitor IC `21`
(`CosemRegisterMonitorObject`) with class version `0`. The constructors
take the thresholds, monitored_value and actions payloads as encoded DLMS
Data buffers prepared by the caller, the logical name, a caller-selected
`AttributeAccessMode` for `thresholds`, and an optional explicit version
that is normalized to `MaxSupportedVersion` when out of range. Attribute `1`
is read-only logical name; attribute `2` `thresholds` honors the caller
access mode and replaces the stored buffer in-place when writable;
attribute `3` `monitored_value` and attribute `4` `actions` are read-only.
Register Monitor v0 defines no methods, so `InvokeMethod` reports
`MethodNotFound` for every method id.

`simple_objects.hpp` also exposes a partial Script Table IC `9`
(`CosemScriptTableObject`) with class version `0`. The constructors take
the scripts payload as an encoded DLMS Data buffer prepared by the caller,
the logical name, a caller-selected `AttributeAccessMode` for `scripts`,
and an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` is read-only logical
name; attribute `2` `scripts` honors the caller access mode and replaces
the stored buffer in-place when writable. Method `1` `execute` dispatches
application-defined script semantics and is surfaced as
`UnsupportedFeature`; other method ids report `MethodNotFound`.

`simple_objects.hpp` also exposes a partial Activity Calendar IC `20`
(`CosemActivityCalendarObject`) with class version `0`. The constructors
take the active and passive calendar payloads
(`calendar_name_active`, `season_profile_active`,
`week_profile_table_active`, `day_profile_table_active`,
`calendar_name_passive`, `season_profile_passive`,
`week_profile_table_passive`, `day_profile_table_passive`) and the
`activate_passive_calendar_time` payload as encoded DLMS Data buffers
prepared by the caller, plus the logical name, a caller-selected
`AttributeAccessMode` shared by the five passive-side attributes (6-10)
and an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` (logical_name) and
attributes `2`-`5` (the active calendar snapshot) are read-only;
attributes `6`-`10` honor the caller passive access mode and replace the
stored buffer in-place when writable. Method `1`
`activate_passive_calendar` dispatches application-defined activation
policy (copying passive into active at meter time) and is surfaced as
`UnsupportedFeature`; other method ids report `MethodNotFound`.

`simple_objects.hpp` also exposes a partial Image Transfer IC `18`
(`CosemImageTransferObject`) with class version `0`. The constructors
take the image_block_size, image_transferred_blocks_status,
image_first_not_transferred_block_number, image_transfer_enabled,
image_transfer_status and image_to_activate_info payloads as encoded
DLMS Data buffers prepared by the caller, the logical name, a
caller-selected `AttributeAccessMode` for `image_transfer_enabled`
(attr 5), and an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` (logical_name)
and attributes `2`, `3`, `4`, `6` and `7` are read-only; attribute `5`
honors the caller access mode and replaces the stored buffer in-place
when writable. Setters are provided so a backend can refresh the
dynamic attributes (transferred-blocks bitmap, first-not-transferred
block counter, transfer status, image-to-activate info) without
changing the object surface. Methods `1` `image_transfer_initiate`,
`2` `image_block_transfer`, `3` `image_verify` and `4` `image_activate`
dispatch application-defined firmware transfer and activation semantics
and are surfaced as `UnsupportedFeature`; other method ids report
`MethodNotFound`.

`simple_objects.hpp` also exposes a partial Push Setup IC `40`
(`CosemPushSetupObject`) with `MaxSupportedVersion = 1`. The single
constructor takes the full v1 attribute payload
(`push_object_list`, `send_destination_and_method`,
`communication_window`, `randomisation_start_interval`,
`number_of_retries`, `repetition_delay`, `port_reference`,
`push_client_SAP`, `push_protection_parameters`,
`push_operation_method`, `confirmation_parameters`,
`last_confirmation_date_time`) as encoded DLMS Data buffers prepared by
the caller, the logical name, a caller-selected `AttributeAccessMode`
shared by the mutable attributes, and an explicit version that is
normalized to `MaxSupportedVersion` when out of range. On a v0 object
the v1-only attributes (`8`-`13`) are hidden: reads and writes report
`AttributeNotFound`. On a v1 object the mutable attributes (`2`-`12`)
honor the caller access mode and replace the stored buffer in-place
when writable; logical_name and `last_confirmation_date_time` (`13`)
are read-only, and a setter exposes backend-driven refresh of the
last-confirmation timestamp. Method `1` `push` dispatches
application-defined scheduling, transport selection and confirmation
tracking and is surfaced as `UnsupportedFeature`; other method ids
report `MethodNotFound`.

`simple_objects.hpp` also exposes a partial Disconnect Control IC `70`
(`CosemDisconnectControlObject`) with class version `0`. The
constructors take the `output_state`, `control_state` and
`control_mode` payloads as encoded DLMS Data buffers prepared by the
caller, the logical name, a caller-selected `AttributeAccessMode` for
`control_mode` (attr `4`), and an optional explicit version that is
normalized to `MaxSupportedVersion` when out of range. Attribute `1`
(logical_name), `2` (output_state) and `3` (control_state) are
read-only; attribute `4` honors the caller access mode and replaces the
stored buffer in-place when writable. Setters expose backend-driven
refresh of `output_state` and `control_state` from a future relay
backend. Methods `1` `remote_disconnect` and `2` `remote_reconnect`
dispatch application-defined relay switching and state transitions and
are surfaced as `UnsupportedFeature`; other method ids report
`MethodNotFound`.

`simple_objects.hpp` also exposes a partial Limiter IC `71`
(`CosemLimiterObject`) with class version `0`. The constructors take
the `monitored_value`, `threshold_active`, `threshold_normal`,
`threshold_emergency`, `min_over_threshold_duration`,
`min_under_threshold_duration`, `emergency_profile`,
`emergency_profile_group_id_list`, `emergency_profile_active` and
`actions` payloads as encoded DLMS Data buffers prepared by the caller,
the logical name, a caller-selected `AttributeAccessMode` shared by the
mutable attributes (`3`-`11`), and an optional explicit version that is
normalized to `MaxSupportedVersion` when out of range. Attribute `1`
(logical_name) and attribute `2` (monitored_value) are read-only;
attributes `3`-`11` honor the caller access mode and replace the stored
buffer in-place when writable. Setters expose backend-driven refresh of
`threshold_active` and `emergency_profile_active` from a future limiter
backend. IC v0 defines no methods; `InvokeMethod` reports
`MethodNotFound` for all method ids.

`simple_objects.hpp` also exposes a partial IEC HDLC Setup IC `23`
(`CosemIecHdlcSetupObject`) with `MaxSupportedVersion = 1`. The
constructors take the `comm_speed`, `window_size_transmit`,
`window_size_receive`, `max_info_field_length_transmit`,
`max_info_field_length_receive`, `inter_octet_time_out`,
`inactivity_time_out` and `device_address` payloads as encoded DLMS
Data buffers prepared by the caller, the logical name, a
caller-selected `AttributeAccessMode` shared by the mutable attributes
(`2`-`8`), and an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` (logical_name)
and attribute `9` (device_address) are read-only; attributes `2`-`8`
honor the caller access mode and replace the stored buffer in-place
when writable. A setter exposes backend-driven refresh of the assigned
HDLC `device_address`. IC defines no methods; `InvokeMethod` reports
`MethodNotFound` for all method ids.

`simple_objects.hpp` also exposes a partial Register Table IC `61`
(`CosemRegisterTableObject`) with class version `0`. The constructors
take the `table_cell_values`, `single_buffer`,
`table_cell_definition` and `table_entries` payloads as encoded DLMS
Data buffers prepared by the caller, the logical name, a
caller-selected `AttributeAccessMode` shared by the mutable attributes
(`3`-`5`), and an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` (logical_name)
and attribute `2` (table_cell_values) are read-only; attributes `3`-`5`
honor the caller access mode and replace the stored buffer in-place
when writable. A setter exposes backend-driven refresh of
`table_cell_values` from a future register-table backend. Methods `1`
`table_entry` and `2` `update_table_entry` dispatch application-defined
column selection and update and are surfaced as `UnsupportedFeature`;
other method ids report `MethodNotFound`.

`simple_objects.hpp` also exposes a partial TCP-UDP Setup IC `41`
(`CosemTcpUdpSetupObject`) with class version `0`. The constructors
take the `tcp_udp_port`, `ip_reference`, `mss`, `nb_of_sim_conn` and
`inactivity_time_out` payloads as encoded DLMS Data buffers prepared
by the caller, the logical name, a caller-selected
`AttributeAccessMode` shared by the mutable attributes (`2`-`6`), and
an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` (logical_name)
is read-only; attributes `2`-`6` honor the caller access mode and
replace the stored buffer in-place when writable. IC defines no
methods; `InvokeMethod` reports `MethodNotFound` for all method ids.

`simple_objects.hpp` also exposes a partial Schedule IC `10`
(`CosemScheduleObject`) with class version `0`. The constructors take
the `entries` payload (array of Schedule_table_entry) as an encoded
DLMS Data buffer prepared by the caller, the logical name, a
caller-selected `AttributeAccessMode` for `entries`, and an optional
explicit version that is normalized to `MaxSupportedVersion` when out
of range. Attribute `1` (logical_name) is read-only; attribute `2`
(entries) honors the caller access mode and replaces the stored buffer
in-place when writable. A setter exposes backend-driven refresh of
entries regardless of access mode. Methods `1` `insert` and `2`
`delete` dispatch application-defined schedule-entry mutation and are
surfaced as `UnsupportedFeature`; other method ids report
`MethodNotFound`.

`simple_objects.hpp` also exposes a partial Special Days Table IC `11`
(`CosemSpecialDaysTableObject`) with class version `0`. The
constructors take the `entries` payload (array of special_day_entry)
as an encoded DLMS Data buffer prepared by the caller, the logical
name, a caller-selected `AttributeAccessMode` for `entries`, and an
optional explicit version that is normalized to `MaxSupportedVersion`
when out of range. Attribute `1` (logical_name) is read-only;
attribute `2` (entries) honors the caller access mode and replaces
the stored buffer in-place when writable. A setter exposes
backend-driven refresh of entries regardless of access mode. Methods
`1` `insert` and `2` `delete` dispatch application-defined
special-day entry mutation and are surfaced as `UnsupportedFeature`;
other method ids report `MethodNotFound`.

`simple_objects.hpp` also exposes a partial Single Action Schedule IC
`22` (`CosemSingleActionScheduleObject`) with class version `0`. The
constructors take the `executed_script`, `type` and `execution_time`
payloads as encoded DLMS Data buffers prepared by the caller, the
logical name, a caller-selected `AttributeAccessMode` shared by the
mutable attributes (`2`-`4`), and an optional explicit version that is
normalized to `MaxSupportedVersion` when out of range. Attribute `1`
(logical_name) is read-only; attributes `2`-`4` honor the caller
access mode and replace the stored buffer in-place when writable. IC
defines no methods; `InvokeMethod` reports `MethodNotFound` for all
method ids.

`simple_objects.hpp` also exposes a partial Modem Configuration IC
`27` (`CosemModemConfigurationObject`) with class version `1`. The
constructors take the `communication_speed`, `initialisation_strings`
and `modem_profile` payloads as encoded DLMS Data buffers prepared by
the caller, the logical name, a caller-selected `AttributeAccessMode`
shared by the mutable attributes (`2`-`4`), and an optional explicit
version that is normalized to `MaxSupportedVersion` when out of
range. Attribute `1` (logical_name) is read-only; attributes `2`-`4`
honor the caller access mode and replace the stored buffer in-place
when writable. IC defines no methods; `InvokeMethod` reports
`MethodNotFound` for all method ids.

`simple_objects.hpp` also exposes a partial Auto Connect IC `29`
(`CosemAutoConnectObject`) with class version `0`. The constructors
take the `mode`, `repetitions`, `repetition_delay`, `calling_window`
and `destination_list` payloads as encoded DLMS Data buffers prepared
by the caller, the logical name, a caller-selected
`AttributeAccessMode` shared by the mutable attributes (`2`-`6`), and
an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` (logical_name)
is read-only; attributes `2`-`6` honor the caller access mode and
replace the stored buffer in-place when writable. IC defines no
methods; `InvokeMethod` reports `MethodNotFound` for all method ids.

`simple_objects.hpp` also exposes a partial GPRS Modem Setup IC `45`
(`CosemGprsModemSetupObject`) with class version `0`. The
constructors take the `APN` (octet-string), `PIN code` (long-unsigned)
and `quality_of_service` (structure of precedence, delay, reliability,
peak_throughput, mean_throughput) payloads as encoded DLMS Data
buffers prepared by the caller, the logical name, a caller-selected
`AttributeAccessMode` shared by the mutable attributes (`2`-`4`), and
an optional explicit version that is normalized to
`MaxSupportedVersion` when out of range. Attribute `1` (logical_name)
is read-only; attributes `2`-`4` honor the caller access mode and
replace the stored buffer in-place when writable. IC defines no
methods; `InvokeMethod` reports `MethodNotFound` for all method ids.

`simple_objects.hpp` also exposes a partial Auto Answer IC `28`
(`CosemAutoAnswerObject`) with class version `0`. The constructors
take the `mode` (enum), `listening_window` (array of structure of
start/end time), `status` (enum), `number_of_calls` (unsigned) and
`number_of_rings` (structure of in/out-of-window ring counts)
payloads as encoded DLMS Data buffers prepared by the caller, the
logical name, a caller-selected `AttributeAccessMode` shared by the
mutable attributes (`2`, `3`, `5`, `6`), and an optional explicit
version that is normalized to `MaxSupportedVersion` when out of
range. Attribute `1` (logical_name) and attribute `4` (status) are
read-only; the mutable attributes honor the caller access mode and
replace the stored buffer in-place when writable. A `SetStatus`
setter exposes backend-driven refresh of status regardless of access
mode. IC defines no methods; `InvokeMethod` reports `MethodNotFound`
for all method ids.

`simple_objects.hpp` also exposes a partial IPv4 Setup IC `42`
(`CosemIpv4SetupObject`) with class version `0`. The constructors
take the `DL_reference` (octet-string), `IP_address`
(double-long-unsigned), `multicast_IP_address` (array of
double-long-unsigned), `IP_options` (array of structure),
`subnet_mask` (double-long-unsigned), `gateway_IP_address`
(double-long-unsigned), `use_DHCP_flag` (boolean),
`primary_DNS_address` (double-long-unsigned) and
`secondary_DNS_address` (double-long-unsigned) payloads as encoded
DLMS Data buffers prepared by the caller, the logical name, a
caller-selected `AttributeAccessMode` shared by the mutable
attributes (`2`-`10`), and an optional explicit version that is
normalized to `MaxSupportedVersion` when out of range. Attribute `1`
(logical_name) is read-only; the mutable attributes honor the caller
access mode and replace the stored buffer in-place when writable.
Methods `1` `add_mc_IP_address` and `2` `delete_mc_IP_address`
return `UnsupportedFeature` and clear method output (built-in object
does not own multicast subscription policy); other method ids return
`MethodNotFound`.

Clock attribute `2`, `5`, and `6` are DLMS Data `octet-string` values
formatted as 12-byte DLMS date-time octets, as defined by the Clock IC. This is
different from the generic DLMS Data `date-time` tag. Clock methods
`adjust_to_quarter`, `adjust_to_measuring_period`, `adjust_to_minute`,
`adjust_to_preset_time`, `preset_adjusting_time`, and `shift_time` are exposed
in access rights but return `UnsupportedFeature` until a time-adjustment policy
is added.

`simple_objects.hpp` also exposes a partial Profile Generic IC `7` object and
class-level helpers for its current composite attributes:

```cpp
class CosemProfileGenericObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemProfileGenericObject(
    const CosemLogicalName& logicalName,
    const std::vector<CosemByteBuffer>& bufferRows,
    const std::vector<CosemCaptureObject>& captureObjects,
    std::uint32_t capturePeriod,
    std::uint32_t profileEntries);
  CosemProfileGenericObject(
    const CosemLogicalName& logicalName,
    const std::vector<CosemByteBuffer>& bufferRows,
    const std::vector<CosemCaptureObject>& captureObjects,
    std::uint32_t capturePeriod,
    std::uint32_t profileEntries,
    std::uint8_t version);
};

CosemByteBuffer EncodeProfileGenericCaptureObject(
  const CosemCaptureObject& object);
CosemStatus DecodeProfileGenericCaptureObject(
  const CosemByteBuffer& input,
  CosemCaptureObject& object);
CosemByteBuffer EncodeProfileGenericCaptureObjects(
  const std::vector<CosemCaptureObject>& objects);
CosemStatus DecodeProfileGenericCaptureObjects(
  const CosemByteBuffer& input,
  std::vector<CosemCaptureObject>& objects);
CosemByteBuffer EncodeProfileGenericBuffer(
  const std::vector<CosemByteBuffer>& rows);
CosemStatus DecodeProfileGenericBuffer(
  const CosemByteBuffer& input,
  std::vector<CosemByteBuffer>& rows);
std::uint8_t ProfileGenericRangeAccessSelector();
std::uint8_t ProfileGenericEntryAccessSelector();
CosemByteBuffer EncodeProfileGenericRangeDescriptor(
  const CosemProfileGenericRangeDescriptor& descriptor);
CosemStatus DecodeProfileGenericRangeDescriptor(
  const CosemByteBuffer& input,
  CosemProfileGenericRangeDescriptor& descriptor);
CosemByteBuffer EncodeProfileGenericEntryDescriptor(
  const CosemProfileGenericEntryDescriptor& descriptor);
CosemStatus DecodeProfileGenericEntryDescriptor(
  const CosemByteBuffer& input,
  CosemProfileGenericEntryDescriptor& descriptor);
```

`CosemCaptureObject` follows the Profile Generic
`capture_object_definition` structure order from the knowledge base:
`class_id`, `logical_name`, `attribute_index`, `data_index`. The buffer helpers
encode and decode the top-level `array`; each decoded row is validated as a
DLMS Data `structure` and returned as its original encoded bytes because row
field types are defined by the matching `capture_objects` schema.

Selective access helpers build the `buffer` access parameters documented for
Profile Generic:

- selector `1` uses `CosemProfileGenericRangeDescriptor`, encoded as
  `restricting_object`, `from_value`, `to_value`, and `selected_values`;
- selector `2` uses `CosemProfileGenericEntryDescriptor`, encoded as
  `from_entry`, `to_entry`, `from_selected_value`, and `to_selected_value`.

Range descriptor boundary values are stored as encoded simple DLMS Data bytes.
The decoder validates the allowed simple data tags, including `date-time`,
`date`, and `time`, but keeps the exact encoded value for the caller.

Profile Generic descriptors use class version `1` by default. The explicit
version constructor can publish version `0` when required by a specific meter
model; values above `MaxSupportedVersion` are normalized. Version `0` exposes
legacy methods `3` and `4`, `get_buffer_by_range` and `get_buffer_by_index`,
as `UnsupportedFeature`. Version `1` keeps those method ids unavailable because
the same behavior is represented by selective access.

The same header also adds minimal discovery objects:

```cpp
class CosemAssociationLnObject : public ICosemObject
{
public:
  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList);
};

class CosemSapAssignmentObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemSapAssignmentObject(
    const CosemLogicalName& logicalName,
    const std::vector<SapAssignment>& assignments);
  CosemSapAssignmentObject(
    const CosemLogicalName& logicalName,
    const std::vector<SapAssignment>& assignments,
    std::uint8_t version);
};

CosemLogicalName CurrentAssociationLnName();
CosemLogicalName SapAssignmentName();
CosemLogicalName LogicalDeviceNameObjectName();
CosemByteBuffer EncodeAssociationAccessRights(
  const CosemAccessRights& rights);
CosemStatus DecodeAssociationAccessRights(
  const CosemByteBuffer& input,
  CosemAccessRights& rights);
CosemByteBuffer EncodeAssociationObjectList(
  const AssociationView& objectList);
CosemStatus DecodeAssociationObjectList(
  const CosemByteBuffer& input,
  AssociationView& objectList);

enum class CosemAssociationStatus
{
  NonAssociated = 0,
  AssociationPending = 1,
  Associated = 2
};

struct CosemAssociationUser
{
  std::uint8_t userId;
  std::string userName;
};

struct CosemAssociationLnConfig
{
  std::uint8_t version;
  CosemAssociationStatus associationStatus;
  bool hasSecuritySetupReference;
  CosemLogicalName securitySetupReference;
  std::vector<CosemAssociationUser> users;
  CosemAssociationUser currentUser;
};
```

Association LN supports caller-selected class versions up to
`CosemAssociationLnObject::MaxSupportedVersion`, currently `3`. Attribute and
method access rights are derived from the selected version: version `0`
exposes attributes `1`, `2`, `8` and methods `1`-`4`; version `1+` may expose
attribute `9`, `security_setup_reference`, when configured; version `2+`
exposes attributes `10`, `11` and methods `5`, `6` for user-list handling.
Unimplemented Association LN methods return `UnsupportedFeature`.

SAP Assignment uses class version `0`; its explicit version constructor
normalizes values above `MaxSupportedVersion`.

Security setup is exposed by `CosemSecuritySetupObject`:

```cpp
class CosemSecuritySetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle);
  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    std::uint8_t version);
};
```

The default constructor publishes class version `1`, matching the implemented
attribute and method surface. The explicit version constructors can publish an
older descriptor version when required; values above `MaxSupportedVersion` are
normalized. Version `0` exposes attributes `1` through `5` and methods `1`
and `2`; version `1` also exposes attribute `6`, `certificates`, as a DLMS
Data array of `certificate_info` structures sourced from an attached
`ICosemCertificateStore` (empty array when no entries exist or no store is
attached). Version `1` exposes methods `1` through `8`.
Implemented methods keep their existing runtime behavior: `security_activate`
validates monotonic policy strengthening and `global_key_transfer` supports
suite `0` key transfer through an installed mutable key store. Methods `6`
(`import_certificate`), `7` (`export_certificate`) and `8`
(`remove_certificate`) parse Blue Book payloads (raw octet-string for import;
`structure{enum, structure{...}}` selector for export/remove with `by_entity`
and `by_serial` variants) and dispatch to the certificate store backend.
Without a certificate store attached, methods `6`-`8` return
`UnsupportedFeature` and clear method output. Methods `3` (key agreement),
`4` (generate key pair) and `5` (generate certificate request) remain
`UnsupportedFeature` until an ECDSA / X.509 stack is wired in.

The certificate store contract is declared in
`dlms/cosem/certificate_store.hpp`:

```
struct CertificateInfoEntry {
  std::uint8_t entity;
  std::uint8_t type;
  CertificateSystemTitle systemTitle;
  std::vector<std::uint8_t> serialNumber;
  std::vector<std::uint8_t> issuer;
  std::vector<std::uint8_t> subject;
  std::vector<std::uint8_t> subjectAltName;
  std::vector<std::uint8_t> rawCertificate;
};

class ICosemCertificateStore {
 public:
  virtual CosemStatus List(std::vector<CertificateInfoEntry>& out) const = 0;
  virtual CosemStatus Import(const CertificateInfoEntry& entry) = 0;
  virtual CosemStatus ExportByEntity(std::uint8_t entity,
                                     std::uint8_t type,
                                     const CertificateSystemTitle& systemTitle,
                                     std::vector<std::uint8_t>& raw) const = 0;
  virtual CosemStatus ExportBySerial(const std::vector<std::uint8_t>& serial,
                                     const std::vector<std::uint8_t>& issuer,
                                     std::vector<std::uint8_t>& raw) const = 0;
  virtual CosemStatus RemoveByEntity(std::uint8_t entity,
                                     std::uint8_t type,
                                     const CertificateSystemTitle& systemTitle) = 0;
  virtual CosemStatus RemoveBySerial(const std::vector<std::uint8_t>& serial,
                                     const std::vector<std::uint8_t>& issuer) = 0;
};
```

Lookups that miss return `CosemStatus::ObjectError`. An
`InMemoryCosemCertificateStore` reference backend is provided.

Attribute `8` is `association_status`, encoded as DLMS Data `enum`. Attribute
`9` is encoded as an xDLMS Data octet-string logical name. Attribute `10` is an
array of `{ user_id: unsigned, user_name: visible-string }` structures, and
attribute `11` is one such structure. SAP Assignment exposes read-only
attributes `1` and `2`; its methods are not supported in this increment. Their
list attributes are returned as encoded xDLMS Data array bytes.
Association LN object-list helpers follow the LN `object_list` structure:
`class_id`, `version`, `logical_name`, and `access_rights`. Access rights decode
validates attribute access items, method access items, and the documented
`access_selectors` choice. Selector values are currently validated but not
stored because `CosemAccessRights` models access modes only.

```mermaid
classDiagram
  class ICosemObject {
    +Descriptor()
    +AccessRights()
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
  }

  class CosemDataObject {
    -CosemObjectDescriptor descriptor
    -CosemByteBuffer value
    -CosemAccessRights rights
    +Value()
    +SetValue()
  }

  class CosemRegisterObject {
    -CosemObjectDescriptor descriptor
    -CosemByteBuffer value
    -CosemByteBuffer scalerUnit
    -CosemAccessRights rights
    +Value()
    +ScalerUnit()
    +SetValue()
    +SetScalerUnit()
  }

  class CosemAssociationLnObject {
    -CosemObjectDescriptor descriptor
    -AssociationView objectList
    -CosemAccessRights rights
    +ReadAttribute(1 logical_name)
    +ReadAttribute(2 object_list)
  }

  class CosemSapAssignmentObject {
    -CosemObjectDescriptor descriptor
    -vector assignments
    -CosemAccessRights rights
    +ReadAttribute(1 logical_name)
    +ReadAttribute(2 SAP_assignment_list)
  }

  ICosemObject <|-- CosemDataObject
  ICosemObject <|-- CosemRegisterObject
  ICosemObject <|-- CosemAssociationLnObject
  ICosemObject <|-- CosemSapAssignmentObject
  CosemDataObject --> CosemAccessRights
  CosemRegisterObject --> CosemAccessRights
  CosemAssociationLnObject --> AssociationView
  CosemSapAssignmentObject --> SapAssignment
```
