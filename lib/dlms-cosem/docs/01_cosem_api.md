# dlms-cosem API

## 1. Public Headers

Planned phase 1 headers:

```text
include/dlms/cosem/cosem_status.hpp
include/dlms/cosem/cosem_types.hpp
include/dlms/cosem/cosem_object.hpp
include/dlms/cosem/object_registry.hpp
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

## 7. Module Diagram

```mermaid
classDiagram
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

  LogicalDevice --> ObjectRegistry
  ObjectRegistry --> ICosemObject
  ICosemObject --> CosemAccessRights
```

## 8. Simple Interface Objects

`simple_objects.hpp` adds reusable in-memory implementations for the first
concrete COSEM interface classes:

```cpp
class CosemDataObject : public ICosemObject
{
public:
  CosemDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    AttributeAccessMode valueAccess);

  const CosemByteBuffer& Value() const;
  void SetValue(const CosemByteBuffer& value);
};

class CosemRegisterObject : public ICosemObject
{
public:
  CosemRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode valueAccess);

  const CosemByteBuffer& Value() const;
  const CosemByteBuffer& ScalerUnit() const;
  void SetValue(const CosemByteBuffer& value);
  void SetScalerUnit(const CosemByteBuffer& scalerUnit);
};
```

The constructors create descriptors with class ids `1` and `3`, version `0`.
Attribute `1` is read-only logical name. Attribute `2` is the value. Register
attribute `3` is read-only scaler-unit. Methods are not supported in this
increment.

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
  CosemSapAssignmentObject(
    const CosemLogicalName& logicalName,
    const std::vector<SapAssignment>& assignments);
};

CosemLogicalName CurrentAssociationLnName();
CosemLogicalName SapAssignmentName();
CosemLogicalName LogicalDeviceNameObjectName();
```

Association LN exposes read-only attributes `1` and `2`. SAP Assignment exposes
read-only attributes `1` and `2`. Their list attributes are returned as encoded
xDLMS Data array bytes and methods are not supported in this increment.

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
