# dlms-cosem Architecture

## 1. Layer Position

```mermaid
flowchart TD
  Server["Future dlms-server"]
  XDlms["lib/dlms-xdlms"]
  Cosem["lib/dlms-cosem"]
  Security["Future dlms-security"]

  Server --> XDlms
  Server --> Cosem
  XDlms --> Cosem
  Cosem -. access policy may consult .-> Security
```

`dlms-cosem` is below the server dispatcher and the xDLMS service layer. It
does not decode APDUs. It receives already resolved COSEM descriptors and
returns encoded xDLMS data bytes plus COSEM status values.

## 2. Read Attribute Flow

```mermaid
sequenceDiagram
  participant Server as dlms-server or dlms-xdlms
  participant Device as LogicalDevice
  participant Registry as ObjectRegistry
  participant Object as ICosemObject

  Server->>Device: ReadAttribute(descriptor)
  Device->>Registry: ReadAttribute(descriptor)
  Registry->>Registry: Find object by class id + logical name
  Registry->>Registry: Check attribute access
  Registry->>Object: ReadAttribute(attributeId)
  Object-->>Registry: encoded xDLMS data
  Registry-->>Device: CosemStatus + data
  Device-->>Server: CosemStatus + data
```

## 3. Association View Flow

```mermaid
sequenceDiagram
  participant Server as dlms-server
  participant Device as LogicalDevice
  participant Registry as ObjectRegistry

  Server->>Device: BuildAssociationView()
  Device->>Registry: Enumerate visible objects
  Registry-->>Device: descriptors + access rights
  Device-->>Server: association view
```

## 4. Class Interaction

```mermaid
classDiagram
  class PhysicalDevice {
    -vector logicalDevices
    +AddLogicalDevice()
    +FindLogicalDevice()
  }

  class LogicalDevice {
    -uint16 sap
    -ObjectRegistry registry
    +RegisterObject()
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
    +BuildAssociationView()
  }

  class ObjectRegistry {
    -vector objectRefs
    +Register()
    +Find()
    +ForEach()
  }

  class ICosemObject {
    +Descriptor()
    +AccessRights()
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
  }

  class AssociationView {
    +objects
  }

  PhysicalDevice --> LogicalDevice
  LogicalDevice --> ObjectRegistry
  LogicalDevice --> AssociationView
  ObjectRegistry --> ICosemObject
```

## 5. Ownership

The first implementation uses non-owning object references. Application or
server code owns concrete object instances. This keeps the registry simple and
avoids prescribing allocation policy before persistent object storage exists.

## 6. Error Model

The layer returns status codes only. Runtime API paths do not throw exceptions.
Object implementation failures are normalized to `ObjectError` unless they map
to a specific COSEM status.

## 7. Simple Interface Objects

```mermaid
sequenceDiagram
  participant Server as dlms-server
  participant Registry as ObjectRegistry
  participant Object as CosemDataObject or CosemRegisterObject

  Server->>Registry: ReadAttribute(attribute descriptor)
  Registry->>Registry: Check object and access rights
  Registry->>Object: ReadAttribute(attributeId)
  Object-->>Registry: encoded xDLMS Data bytes
  Registry-->>Server: CosemStatus::Ok and bytes
```

The concrete objects stay inside `dlms-cosem` and implement only stable COSEM
interface-class behavior. They do not depend on `dlms-apdu`, `dlms-xdlms`, or
`dlms-server`; all attribute values remain encoded xDLMS Data byte vectors at
the layer boundary.

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
    +ReadAttribute(1 logical_name)
    +ReadAttribute(2 value)
    +WriteAttribute(2 value)
  }

  class CosemRegisterObject {
    +ReadAttribute(1 logical_name)
    +ReadAttribute(2 value)
    +ReadAttribute(3 scaler_unit)
    +WriteAttribute(2 value)
  }

  ICosemObject <|-- CosemDataObject
  ICosemObject <|-- CosemRegisterObject
```

## 8. Association And SAP Discovery Objects

```mermaid
sequenceDiagram
  participant Server as dlms-server
  participant Registry as ObjectRegistry
  participant Association as CosemAssociationLnObject
  participant Sap as CosemSapAssignmentObject

  Server->>Registry: Read Association LN attribute 2
  Registry->>Association: ReadAttribute(object_list)
  Association-->>Registry: encoded object_list array
  Registry-->>Server: CosemStatus::Ok and bytes

  Server->>Registry: Read SAP Assignment attribute 2
  Registry->>Sap: ReadAttribute(SAP_assignment_list)
  Sap-->>Registry: encoded SAP assignment array
  Registry-->>Server: CosemStatus::Ok and bytes
```

The discovery objects are snapshots. They receive `AssociationView` or
`SapAssignment` data from the owning logical or physical device and expose those
bytes through normal `ICosemObject` reads. Updating the object list after
registration requires constructing or refreshing the object explicitly; the
minimal object does not hold a mutable back-reference to the registry.
