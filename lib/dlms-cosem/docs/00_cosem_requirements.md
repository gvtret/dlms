# dlms-cosem Requirements

## 1. Scope

The COSEM layer owns the object model and server resource registry. It is used
by future server-side xDLMS dispatch to resolve logical-name attribute and
method requests to registered COSEM objects.

The layer shall:

- model a physical device containing one or more logical devices;
- model each logical device as a collection of COSEM objects;
- identify COSEM objects by class id, version, and six-byte logical name;
- provide an object registry lookup by logical name and class id;
- expose attribute read/write and method invoke contracts;
- model per-association attribute and method access rights;
- provide association-view metadata containing visible objects and access
  rights;
- model the mandatory Association LN and SAP Assignment concepts enough for the
  minimal server path.

## 2. Normative Model

Document RAG references:

- Blue Book Association LN defines an `object_list` containing visible COSEM
  objects with class id, version, logical name, and access rights.
- Attribute access modes include no access, read only, write only, and read and
  write; newer editions also define authenticated variants.
- Method access modes include no access, access, and authenticated access.
- A COSEM server contains logical devices; each logical device contains one or
  more Association objects.
- Each logical device shall expose a logical device name object and the current
  Association object.
- The management logical device contains a SAP Assignment object listing the SAP
  of logical devices in the physical device; this object is readable at least by
  the public client.

## 3. First Implementation

Version 1 implements a minimal in-memory LN object model:

1. The caller creates a `LogicalDevice` with a logical-device SAP.
2. The caller registers objects implementing `ICosemObject`.
3. The caller reads attributes through the registry and receives encoded xDLMS
   data bytes.
4. The caller may write attributes or invoke methods if the object supports
   those operations.
5. The caller queries an association view containing object identifiers and
   access rights.

## 4. State Requirements

The first implementation shall keep object ownership outside the registry.
Objects are passed by reference and must outlive the registry/logical device.

Rules:

- duplicate object keys are rejected;
- lookups are deterministic and do not allocate on failure;
- access checks happen before object callback invocation;
- unsupported attributes return `AttributeNotFound`;
- unsupported methods return `MethodNotFound`;
- access denial returns `AccessDenied`;
- object callback failures are propagated as `ObjectError`;
- no method mutates the registry while iterating association-view data.

## 5. Data Boundary

`dlms-cosem` stores and returns encoded xDLMS data bytes. It does not own typed
projection of arbitrary COSEM data values yet. Typed value helpers may be added
later when interface-class implementations need them.

## 6. Simple Interface Objects

The next implementation increment shall add reusable in-memory COSEM interface
objects for the common MVP object model:

- Data, class id `1`, version `0`;
- Register, class id `3`, version `0`.

Rules:

- both objects expose attribute `1` as the logical name encoded as xDLMS Data
  octet-string bytes;
- Data exposes attribute `2` as the stored encoded value;
- Register exposes attribute `2` as the stored encoded value;
- Register exposes attribute `3` as stored encoded scaler-unit bytes;
- writes to value attributes update the stored encoded bytes when access rights
  permit writes;
- writes to logical name are rejected;
- methods are not implemented in this increment and return `MethodNotFound`;
- constructors initialize descriptors and explicit access rights; existing
  registry descriptor validation rejects invalid logical names during
  registration.

This phase intentionally stores caller-provided encoded xDLMS Data bytes. It
does not introduce a typed COSEM value hierarchy.

## 7. Association And SAP Objects

The next implementation increment shall add minimal read-only concrete objects
for mandatory server model discovery:

- Association LN, class id `15`, version `0`;
- SAP Assignment, class id `17`, version `0`;
- Logical Device Name as a Data object at OBIS `0.0.42.0.0.255`.

Rules:

- Association LN attribute `1` returns its logical name as encoded xDLMS Data
  octet-string bytes;
- Association LN attribute `2` returns `object_list` as encoded xDLMS Data
  array bytes;
- each object-list element contains class id, version, logical name, and
  access-right descriptors for one visible object;
- SAP Assignment attribute `1` returns its logical name as encoded xDLMS Data
  octet-string bytes;
- SAP Assignment attribute `2` returns `SAP_assignment_list` as encoded xDLMS
  Data array bytes;
- each SAP assignment element contains the logical-device SAP and logical-device
  name as an octet-string;
- the objects are read-only in this increment;
- methods such as Association LN HLS methods, add/remove object, and SAP
  `connect_logical_device` remain out of scope.

The encoded byte layout is intentionally produced inside `dlms-cosem` without
depending on `dlms-apdu`; it follows the same xDLMS Data tag values used by the
APDU layer.

## 8. Out Of Scope

- xDLMS APDU encode/decode;
- GET/SET/ACTION request orchestration;
- association opening, release, and authentication;
- transport and profile channels;
- cryptographic protection and invocation counters;
- persistent storage;
- complete Blue Book interface-class catalog;
- typed COSEM value model;
- Profile Generic capture objects;
- Association LN selective access and methods;
- SAP Assignment connect method;
- short-name referencing;
- public client and server facades.
