#include "dlms/cosem/simple_objects.hpp"
#include "dlms/security/suite0_key_wrap.hpp"

namespace dlms {
namespace cosem {
namespace {

constexpr std::uint16_t kDataClassId = 1u;
constexpr std::uint16_t kRegisterClassId = 3u;
constexpr std::uint16_t kAssociationLnClassId = 15u;
constexpr std::uint16_t kSapAssignmentClassId = 17u;
constexpr std::uint16_t kSecuritySetupClassId = 64u;
constexpr std::uint8_t kLogicalNameAttributeId = 1u;
constexpr std::uint8_t kValueAttributeId = 2u;
constexpr std::uint8_t kScalerUnitAttributeId = 3u;
constexpr std::uint8_t kSecurityPolicyAttributeId = 2u;
constexpr std::uint8_t kSecuritySuiteAttributeId = 3u;
constexpr std::uint8_t kClientSystemTitleAttributeId = 4u;
constexpr std::uint8_t kServerSystemTitleAttributeId = 5u;
constexpr std::uint8_t kSecurityActivateMethodId = 1u;
constexpr std::uint8_t kGlobalKeyTransferMethodId = 2u;
constexpr std::uint8_t kVersion0 = 0u;
constexpr std::uint8_t kArrayTag = 0x01u;
constexpr std::uint8_t kStructureTag = 0x02u;
constexpr std::uint8_t kNullDataTag = 0x00u;
constexpr std::uint8_t kDoubleLongUnsignedTag = 0x06u;
constexpr std::uint8_t kDataOctetStringTag = 0x09u;
constexpr std::uint8_t kIntegerTag = 0x0Fu;
constexpr std::uint8_t kUnsignedTag = 0x11u;
constexpr std::uint8_t kLongUnsignedTag = 0x12u;
constexpr std::uint8_t kEnumTag = 0x16u;
constexpr std::uint8_t kLogicalNameSize = 6u;
constexpr std::size_t kSystemTitleSize = 8u;
constexpr std::size_t kSuite0KeySize = 16u;
constexpr std::size_t kSuite0WrappedKeySize = 24u;

bool IsAxdrEnum(
  const CosemByteBuffer& input,
  std::uint8_t& value)
{
  if (input.size() != 2u || input[0] != kEnumTag) {
    return false;
  }
  value = input[1];
  return true;
}

bool ReadAxdrLength(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::size_t& length)
{
  if (offset >= input.size()) {
    return false;
  }

  const std::uint8_t first = input[offset++];
  if ((first & 0x80u) == 0u) {
    length = first;
    return true;
  }

  const std::size_t lengthBytes = first & 0x7fu;
  if (lengthBytes == 0u ||
      lengthBytes > sizeof(std::size_t) ||
      input.size() - offset < lengthBytes) {
    return false;
  }

  length = 0u;
  for (std::size_t i = 0u; i < lengthBytes; ++i) {
    length = (length << 8u) | input[offset++];
  }
  return true;
}

bool StrengthensOrKeepsPolicy(
  std::uint8_t currentPolicy,
  std::uint8_t requestedPolicy)
{
  return (currentPolicy & requestedPolicy) == currentPolicy;
}

bool MapSecuritySetupKeyId(
  std::uint8_t keyId,
  dlms::security::SecurityKeyRole& role)
{
  switch (keyId) {
    case 0u:
      role = dlms::security::SecurityKeyRole::GlobalUnicastEncryption;
      return true;
    case 1u:
      role = dlms::security::SecurityKeyRole::GlobalBroadcastEncryption;
      return true;
    case 2u:
      role = dlms::security::SecurityKeyRole::Authentication;
      return true;
    case 3u:
      role = dlms::security::SecurityKeyRole::KeyEncryption;
      return true;
    default:
      return false;
  }
}

CosemStatus MapSecurityStatus(dlms::security::SecurityStatus status)
{
  switch (status) {
    case dlms::security::SecurityStatus::Ok:
      return CosemStatus::Ok;
    case dlms::security::SecurityStatus::InvalidArgument:
    case dlms::security::SecurityStatus::InvalidKeyLength:
      return CosemStatus::InvalidArgument;
    case dlms::security::SecurityStatus::MissingKey:
    case dlms::security::SecurityStatus::AuthenticationFailed:
      return CosemStatus::AccessDenied;
    case dlms::security::SecurityStatus::UnsupportedFeature:
      return CosemStatus::UnsupportedFeature;
    default:
      return CosemStatus::ObjectError;
  }
}

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

void AppendDoubleLongUnsigned(CosemByteBuffer& output, std::uint32_t value)
{
  output.push_back(kDoubleLongUnsignedTag);
  output.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xffu));
  output.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xffu));
  output.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xffu));
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

CosemLogicalName SecuritySetupName()
{
  return CosemLogicalName(0u, 0u, 43u, 0u, 0u, 255u);
}

CosemLogicalName InvocationCounterObjectName()
{
  return CosemLogicalName(0u, 0u, 43u, 1u, 0u, 255u);
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

CosemDataObject MakeInvocationCounterObject(
  std::uint32_t invocationCounter)
{
  CosemByteBuffer value;
  AppendDoubleLongUnsigned(value, invocationCounter);
  return CosemDataObject(
    InvocationCounterObjectName(),
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

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle)
  : CosemSecuritySetupObject(
      logicalName,
      securityPolicy,
      securitySuite,
      clientSystemTitle,
      serverSystemTitle,
      0)
{
}

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle,
  dlms::security::IMutableKeyStore* keyStore)
  : CosemSecuritySetupObject(
      logicalName,
      securityPolicy,
      securitySuite,
      clientSystemTitle,
      serverSystemTitle,
      keyStore,
      0)
{
}

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle,
  dlms::security::IMutableKeyStore* keyStore,
  dlms::security::IInvocationCounterResetPolicy* counterResetPolicy)
  : descriptor_(MakeDescriptor(kSecuritySetupClassId, logicalName))
  , securityPolicy_(securityPolicy)
  , securitySuite_(securitySuite)
  , clientSystemTitle_(clientSystemTitle)
  , serverSystemTitle_(serverSystemTitle)
  , keyStore_(keyStore)
  , counterResetPolicy_(counterResetPolicy)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSecurityPolicyAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSecuritySuiteAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kClientSystemTitleAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kServerSystemTitleAttributeId,
    AttributeAccessMode::ReadOnly);

  for (std::uint8_t methodId = 1u; methodId <= 8u; ++methodId) {
    rights_.SetMethodAccess(methodId, MethodAccessMode::Access);
  }
}

CosemObjectDescriptor CosemSecuritySetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSecuritySetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSecuritySetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kSecurityPolicyAttributeId) {
    output.clear();
    AppendEnum(output, securityPolicy_);
    return CosemStatus::Ok;
  }
  if (attributeId == kSecuritySuiteAttributeId) {
    output.clear();
    AppendEnum(output, securitySuite_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClientSystemTitleAttributeId) {
    output.clear();
    AppendOctetString(
      output,
      clientSystemTitle_.data(),
      kSystemTitleSize);
    return CosemStatus::Ok;
  }
  if (attributeId == kServerSystemTitleAttributeId) {
    output.clear();
    AppendOctetString(
      output,
      serverSystemTitle_.data(),
      kSystemTitleSize);
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemSecuritySetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  (void)input;
  if (attributeId == kLogicalNameAttributeId
      || attributeId == kSecurityPolicyAttributeId
      || attributeId == kSecuritySuiteAttributeId
      || attributeId == kClientSystemTitleAttributeId
      || attributeId == kServerSystemTitleAttributeId) {
    return CosemStatus::AccessDenied;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemSecuritySetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  if (methodId == kSecurityActivateMethodId) {
    std::uint8_t requestedPolicy = 0u;
    if (!IsAxdrEnum(input, requestedPolicy)) {
      return CosemStatus::InvalidArgument;
    }
    if (!StrengthensOrKeepsPolicy(securityPolicy_, requestedPolicy)) {
      return CosemStatus::AccessDenied;
    }
    securityPolicy_ = requestedPolicy;
    output.clear();
    return CosemStatus::Ok;
  }
  if (methodId == kGlobalKeyTransferMethodId) {
    output.clear();
    if (keyStore_ == 0) {
      return CosemStatus::UnsupportedFeature;
    }
    if (securitySuite_ != 0u) {
      return CosemStatus::UnsupportedFeature;
    }

    dlms::security::SecurityKey kek =
      dlms::security::EmptySecurityKey(
        dlms::security::SecurityKeyRole::KeyEncryption);
    CosemStatus status = MapSecurityStatus(
      keyStore_->GetKey(
        dlms::security::SecurityKeyRole::KeyEncryption,
        kek));
    if (status != CosemStatus::Ok) {
      return status;
    }

    std::size_t offset = 0u;
    if (input.size() < 2u || input[offset++] != kArrayTag) {
      return CosemStatus::InvalidArgument;
    }
    std::size_t count = 0u;
    if (!ReadAxdrLength(input, offset, count)) {
      return CosemStatus::InvalidArgument;
    }

    dlms::security::Suite0KeyWrap keyWrap;
    std::vector<dlms::security::SecurityKey> transferredKeys;
    for (std::size_t entry = 0u; entry < count; ++entry) {
      if (input.size() - offset < 6u ||
          input[offset++] != kStructureTag) {
        return CosemStatus::InvalidArgument;
      }
      std::size_t fieldCount = 0u;
      if (!ReadAxdrLength(input, offset, fieldCount) ||
          fieldCount != 2u ||
          input.size() - offset < 2u ||
          input[offset++] != kEnumTag) {
        return CosemStatus::InvalidArgument;
      }

      dlms::security::SecurityKeyRole role =
        dlms::security::SecurityKeyRole::Authentication;
      if (!MapSecuritySetupKeyId(input[offset++], role) ||
          offset >= input.size() ||
          input[offset++] != kDataOctetStringTag) {
        return CosemStatus::InvalidArgument;
      }

      std::size_t wrappedSize = 0u;
      if (!ReadAxdrLength(input, offset, wrappedSize) ||
          wrappedSize != kSuite0WrappedKeySize ||
          input.size() - offset < wrappedSize) {
        return CosemStatus::InvalidArgument;
      }

      dlms::security::SecurityByteView wrapped;
      wrapped.data = &input[offset];
      wrapped.size = wrappedSize;
      offset += wrappedSize;

      std::vector<std::uint8_t> plain;
      status = MapSecurityStatus(keyWrap.Unwrap(kek, wrapped, plain));
      if (status != CosemStatus::Ok) {
        return status;
      }
      if (plain.size() != kSuite0KeySize) {
        return CosemStatus::InvalidArgument;
      }

      dlms::security::SecurityKey transferred =
        dlms::security::EmptySecurityKey(role);
      transferred.size = plain.size();
      for (std::size_t i = 0u; i < plain.size(); ++i) {
        transferred.bytes[i] = plain[i];
      }
      transferredKeys.push_back(transferred);
    }

    if (offset != input.size()) {
      return CosemStatus::InvalidArgument;
    }

    if (counterResetPolicy_ != 0) {
      for (std::size_t i = 0u; i < transferredKeys.size(); ++i) {
        status = MapSecurityStatus(
          counterResetPolicy_->ResetAfterKeyRotation(
            transferredKeys[i].role));
        if (status != CosemStatus::Ok) {
          return status;
        }
      }
    }
    for (std::size_t i = 0u; i < transferredKeys.size(); ++i) {
      status = MapSecurityStatus(keyStore_->SetKey(transferredKeys[i]));
      if (status != CosemStatus::Ok) {
        return status;
      }
    }
    return CosemStatus::Ok;
  }
  if (methodId > kSecurityActivateMethodId && methodId <= 8u) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

std::uint8_t CosemSecuritySetupObject::SecurityPolicy() const
{
  return securityPolicy_;
}

std::uint8_t CosemSecuritySetupObject::SecuritySuite() const
{
  return securitySuite_;
}

const CosemSecuritySetupObject::SystemTitle&
CosemSecuritySetupObject::ClientSystemTitle() const
{
  return clientSystemTitle_;
}

const CosemSecuritySetupObject::SystemTitle&
CosemSecuritySetupObject::ServerSystemTitle() const
{
  return serverSystemTitle_;
}

} // namespace cosem
} // namespace dlms
