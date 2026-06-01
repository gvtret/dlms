#include "dlms/cosem/simple_objects.hpp"

namespace dlms {
namespace cosem {
namespace {

constexpr std::uint16_t kDataClassId = 1u;
constexpr std::uint16_t kRegisterClassId = 3u;
constexpr std::uint16_t kAssociationLnClassId = 15u;
constexpr std::uint16_t kSapAssignmentClassId = 17u;
constexpr std::uint8_t kLogicalNameAttributeId = 1u;
constexpr std::uint8_t kValueAttributeId = 2u;
constexpr std::uint8_t kScalerUnitAttributeId = 3u;
constexpr std::uint8_t kVersion0 = 0u;
constexpr std::uint8_t kArrayTag = 0x01u;
constexpr std::uint8_t kStructureTag = 0x02u;
constexpr std::uint8_t kNullDataTag = 0x00u;
constexpr std::uint8_t kDataOctetStringTag = 0x09u;
constexpr std::uint8_t kIntegerTag = 0x0Fu;
constexpr std::uint8_t kUnsignedTag = 0x11u;
constexpr std::uint8_t kLongUnsignedTag = 0x12u;
constexpr std::uint8_t kEnumTag = 0x16u;
constexpr std::uint8_t kLogicalNameSize = 6u;

CosemObjectDescriptor MakeDescriptor(
  std::uint16_t classId,
  const CosemLogicalName& logicalName)
{
  CosemObjectDescriptor descriptor;
  descriptor.key.classId = classId;
  descriptor.key.version = kVersion0;
  descriptor.key.logicalName = logicalName;
  return descriptor;
}

CosemByteBuffer EncodeLogicalName(const CosemLogicalName& logicalName)
{
  CosemByteBuffer output;
  output.push_back(kDataOctetStringTag);
  output.push_back(kLogicalNameSize);
  for (std::size_t i = 0; i < logicalName.Size(); ++i) {
    output.push_back(logicalName[i]);
  }
  return output;
}

void AppendLength(CosemByteBuffer& output, std::size_t length)
{
  if (length < 0x80u) {
    output.push_back(static_cast<std::uint8_t>(length));
    return;
  }

  CosemByteBuffer bytes;
  std::size_t value = length;
  while (value != 0u) {
    bytes.insert(bytes.begin(), static_cast<std::uint8_t>(value & 0xffu));
    value >>= 8u;
  }
  output.push_back(static_cast<std::uint8_t>(0x80u | bytes.size()));
  output.insert(output.end(), bytes.begin(), bytes.end());
}

void AppendArrayHeader(CosemByteBuffer& output, std::size_t count)
{
  output.push_back(kArrayTag);
  AppendLength(output, count);
}

void AppendStructureHeader(CosemByteBuffer& output, std::size_t count)
{
  output.push_back(kStructureTag);
  AppendLength(output, count);
}

void AppendLongUnsigned(CosemByteBuffer& output, std::uint16_t value)
{
  output.push_back(kLongUnsignedTag);
  output.push_back(static_cast<std::uint8_t>(value >> 8u));
  output.push_back(static_cast<std::uint8_t>(value & 0xffu));
}

void AppendUnsigned(CosemByteBuffer& output, std::uint8_t value)
{
  output.push_back(kUnsignedTag);
  output.push_back(value);
}

void AppendInteger(CosemByteBuffer& output, std::uint8_t value)
{
  output.push_back(kIntegerTag);
  output.push_back(value);
}

void AppendEnum(CosemByteBuffer& output, std::uint8_t value)
{
  output.push_back(kEnumTag);
  output.push_back(value);
}

void AppendOctetString(
  CosemByteBuffer& output,
  const std::uint8_t* data,
  std::size_t size)
{
  output.push_back(kDataOctetStringTag);
  AppendLength(output, size);
  output.insert(output.end(), data, data + size);
}

void AppendLogicalName(
  CosemByteBuffer& output,
  const CosemLogicalName& logicalName)
{
  AppendOctetString(output, logicalName.Data(), logicalName.Size());
}

void AppendAttributeAccess(
  CosemByteBuffer& output,
  const CosemAccessRights& rights)
{
  const std::vector<AttributeAccessEntry> entries =
    rights.AttributeAccessEntries();
  AppendArrayHeader(output, entries.size());
  for (std::vector<AttributeAccessEntry>::const_iterator it =
         entries.begin();
       it != entries.end();
       ++it) {
    AppendStructureHeader(output, 3u);
    AppendInteger(output, it->attributeId);
    AppendEnum(output, static_cast<std::uint8_t>(it->mode));
    output.push_back(kNullDataTag);
  }
}

void AppendMethodAccess(
  CosemByteBuffer& output,
  const CosemAccessRights& rights)
{
  const std::vector<MethodAccessEntry> entries =
    rights.MethodAccessEntries();
  AppendArrayHeader(output, entries.size());
  for (std::vector<MethodAccessEntry>::const_iterator it = entries.begin();
       it != entries.end();
       ++it) {
    AppendStructureHeader(output, 2u);
    AppendInteger(output, it->methodId);
    AppendEnum(output, static_cast<std::uint8_t>(it->mode));
  }
}

void AppendAccessRights(
  CosemByteBuffer& output,
  const CosemAccessRights& rights)
{
  AppendStructureHeader(output, 2u);
  AppendAttributeAccess(output, rights);
  AppendMethodAccess(output, rights);
}

void AppendObjectListElement(
  CosemByteBuffer& output,
  const AssociationViewObject& object)
{
  AppendStructureHeader(output, 4u);
  AppendLongUnsigned(output, object.descriptor.key.classId);
  AppendUnsigned(output, object.descriptor.key.version);
  AppendLogicalName(output, object.descriptor.key.logicalName);
  AppendAccessRights(output, object.accessRights);
}

void AppendSapAssignment(
  CosemByteBuffer& output,
  const SapAssignment& assignment)
{
  AppendStructureHeader(output, 2u);
  AppendLongUnsigned(output, assignment.sap);
  AppendOctetString(
    output,
    reinterpret_cast<const std::uint8_t*>(
      assignment.logicalDeviceName.data()),
    assignment.logicalDeviceName.size());
}

} // namespace

CosemLogicalName CurrentAssociationLnName()
{
  return CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u);
}

CosemLogicalName SapAssignmentName()
{
  return CosemLogicalName(0u, 0u, 41u, 0u, 0u, 255u);
}

CosemLogicalName LogicalDeviceNameObjectName()
{
  return CosemLogicalName(0u, 0u, 42u, 0u, 0u, 255u);
}

CosemDataObject MakeLogicalDeviceNameObject(
  const std::string& logicalDeviceName)
{
  CosemByteBuffer value;
  AppendOctetString(
    value,
    reinterpret_cast<const std::uint8_t*>(logicalDeviceName.data()),
    logicalDeviceName.size());
  return CosemDataObject(
    LogicalDeviceNameObjectName(),
    value,
    AttributeAccessMode::ReadOnly);
}

CosemDataObject::CosemDataObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  AttributeAccessMode valueAccess)
  : descriptor_(MakeDescriptor(kDataClassId, logicalName))
  , value_(value)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(kValueAttributeId, valueAccess);
}

CosemObjectDescriptor CosemDataObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemDataObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemDataObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kValueAttributeId) {
    output = value_;
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemDataObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId) {
    return CosemStatus::AccessDenied;
  }
  if (attributeId == kValueAttributeId) {
    value_ = input;
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemDataObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  (void)output;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemDataObject::Value() const
{
  return value_;
}

void CosemDataObject::SetValue(const CosemByteBuffer& value)
{
  value_ = value;
}

CosemRegisterObject::CosemRegisterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  const CosemByteBuffer& scalerUnit,
  AttributeAccessMode valueAccess)
  : descriptor_(MakeDescriptor(kRegisterClassId, logicalName))
  , value_(value)
  , scalerUnit_(scalerUnit)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(kValueAttributeId, valueAccess);
  rights_.SetAttributeAccess(
    kScalerUnitAttributeId,
    AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemRegisterObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemRegisterObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemRegisterObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kValueAttributeId) {
    output = value_;
    return CosemStatus::Ok;
  }
  if (attributeId == kScalerUnitAttributeId) {
    output = scalerUnit_;
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemRegisterObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId
      || attributeId == kScalerUnitAttributeId) {
    return CosemStatus::AccessDenied;
  }
  if (attributeId == kValueAttributeId) {
    value_ = input;
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemRegisterObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  (void)output;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemRegisterObject::Value() const
{
  return value_;
}

const CosemByteBuffer& CosemRegisterObject::ScalerUnit() const
{
  return scalerUnit_;
}

void CosemRegisterObject::SetValue(const CosemByteBuffer& value)
{
  value_ = value;
}

void CosemRegisterObject::SetScalerUnit(
  const CosemByteBuffer& scalerUnit)
{
  scalerUnit_ = scalerUnit;
}

CosemAssociationLnObject::CosemAssociationLnObject(
  const CosemLogicalName& logicalName,
  const AssociationView& objectList)
  : descriptor_(MakeDescriptor(kAssociationLnClassId, logicalName))
  , objectList_(objectList)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(kValueAttributeId, AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemAssociationLnObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemAssociationLnObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemAssociationLnObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kValueAttributeId) {
    output.clear();
    AppendArrayHeader(output, objectList_.objects.size());
    for (std::vector<AssociationViewObject>::const_iterator it =
           objectList_.objects.begin();
         it != objectList_.objects.end();
         ++it) {
      AppendObjectListElement(output, *it);
    }
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemAssociationLnObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  (void)attributeId;
  (void)input;
  return CosemStatus::AccessDenied;
}

CosemStatus CosemAssociationLnObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  (void)output;
  return CosemStatus::MethodNotFound;
}

AssociationView CosemAssociationLnObject::ObjectList() const
{
  return objectList_;
}

CosemSapAssignmentObject::CosemSapAssignmentObject(
  const CosemLogicalName& logicalName,
  const std::vector<SapAssignment>& assignments)
  : descriptor_(MakeDescriptor(kSapAssignmentClassId, logicalName))
  , assignments_(assignments)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(kValueAttributeId, AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemSapAssignmentObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSapAssignmentObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSapAssignmentObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kValueAttributeId) {
    output.clear();
    AppendArrayHeader(output, assignments_.size());
    for (std::vector<SapAssignment>::const_iterator it = assignments_.begin();
         it != assignments_.end();
         ++it) {
      AppendSapAssignment(output, *it);
    }
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemSapAssignmentObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  (void)attributeId;
  (void)input;
  return CosemStatus::AccessDenied;
}

CosemStatus CosemSapAssignmentObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  (void)output;
  return CosemStatus::MethodNotFound;
}

std::vector<SapAssignment> CosemSapAssignmentObject::Assignments() const
{
  return assignments_;
}

} // namespace cosem
} // namespace dlms
