#include "dlms/cosem/simple_objects.hpp"
#include "dlms/security/suite0_key_wrap.hpp"

namespace dlms {
namespace cosem {
namespace {

constexpr std::uint16_t kDataClassId = 1u;
constexpr std::uint16_t kRegisterClassId = 3u;
constexpr std::uint16_t kClockClassId = 8u;
constexpr std::uint16_t kProfileGenericClassId = 7u;
constexpr std::uint16_t kAssociationLnClassId = 15u;
constexpr std::uint16_t kSapAssignmentClassId = 17u;
constexpr std::uint16_t kSecuritySetupClassId = 64u;
constexpr std::uint8_t kLogicalNameAttributeId = 1u;
constexpr std::uint8_t kValueAttributeId = 2u;
constexpr std::uint8_t kScalerUnitAttributeId = 3u;
constexpr std::uint8_t kClockTimeAttributeId = 2u;
constexpr std::uint8_t kClockTimeZoneAttributeId = 3u;
constexpr std::uint8_t kClockStatusAttributeId = 4u;
constexpr std::uint8_t kClockDaylightSavingsBeginAttributeId = 5u;
constexpr std::uint8_t kClockDaylightSavingsEndAttributeId = 6u;
constexpr std::uint8_t kClockDaylightSavingsDeviationAttributeId = 7u;
constexpr std::uint8_t kClockDaylightSavingsEnabledAttributeId = 8u;
constexpr std::uint8_t kClockBaseAttributeId = 9u;
constexpr std::uint8_t kClockAdjustToQuarterMethodId = 1u;
constexpr std::uint8_t kClockAdjustToMeasuringPeriodMethodId = 2u;
constexpr std::uint8_t kClockAdjustToMinuteMethodId = 3u;
constexpr std::uint8_t kClockAdjustToPresetTimeMethodId = 4u;
constexpr std::uint8_t kClockPresetAdjustingTimeMethodId = 5u;
constexpr std::uint8_t kClockShiftTimeMethodId = 6u;
constexpr std::uint8_t kProfileBufferAttributeId = 2u;
constexpr std::uint8_t kProfileCaptureObjectsAttributeId = 3u;
constexpr std::uint8_t kProfileCapturePeriodAttributeId = 4u;
constexpr std::uint8_t kProfileSortMethodAttributeId = 5u;
constexpr std::uint8_t kProfileSortObjectAttributeId = 6u;
constexpr std::uint8_t kProfileEntriesInUseAttributeId = 7u;
constexpr std::uint8_t kProfileProfileEntriesAttributeId = 8u;
constexpr std::uint8_t kProfileResetMethodId = 1u;
constexpr std::uint8_t kProfileCaptureMethodId = 2u;
constexpr std::uint8_t kProfileGetBufferByRangeMethodId = 3u;
constexpr std::uint8_t kProfileGetBufferByIndexMethodId = 4u;
constexpr std::uint8_t kAssociationStatusAttributeId = 8u;
constexpr std::uint8_t kAssociationSecuritySetupReferenceAttributeId = 9u;
constexpr std::uint8_t kAssociationUserListAttributeId = 10u;
constexpr std::uint8_t kAssociationCurrentUserAttributeId = 11u;
constexpr std::uint8_t kAssociationReplyToHlsAuthenticationMethodId = 1u;
constexpr std::uint8_t kAssociationChangeHlsSecretMethodId = 2u;
constexpr std::uint8_t kAssociationAddObjectMethodId = 3u;
constexpr std::uint8_t kAssociationRemoveObjectMethodId = 4u;
constexpr std::uint8_t kAssociationAddUserMethodId = 5u;
constexpr std::uint8_t kAssociationRemoveUserMethodId = 6u;
constexpr std::uint8_t kSecurityPolicyAttributeId = 2u;
constexpr std::uint8_t kSecuritySuiteAttributeId = 3u;
constexpr std::uint8_t kClientSystemTitleAttributeId = 4u;
constexpr std::uint8_t kServerSystemTitleAttributeId = 5u;
constexpr std::uint8_t kSecurityCertificatesAttributeId = 6u;
constexpr std::uint8_t kSecurityActivateMethodId = 1u;
constexpr std::uint8_t kGlobalKeyTransferMethodId = 2u;
constexpr std::uint8_t kVersion0 = 0u;
constexpr std::uint8_t kAssociationLnMaxVersion = 3u;
constexpr std::uint8_t kProfileGenericVersion = 1u;
constexpr std::uint8_t kSecuritySetupVersion = 1u;
constexpr std::uint8_t kArrayTag = 0x01u;
constexpr std::uint8_t kStructureTag = 0x02u;
constexpr std::uint8_t kNullDataTag = 0x00u;
constexpr std::uint8_t kBooleanTag = 0x03u;
constexpr std::uint8_t kDoubleLongTag = 0x05u;
constexpr std::uint8_t kDoubleLongUnsignedTag = 0x06u;
constexpr std::uint8_t kDataOctetStringTag = 0x09u;
constexpr std::uint8_t kVisibleStringTag = 0x0Au;
constexpr std::uint8_t kUtf8StringTag = 0x0Cu;
constexpr std::uint8_t kIntegerTag = 0x0Fu;
constexpr std::uint8_t kLongTag = 0x10u;
constexpr std::uint8_t kUnsignedTag = 0x11u;
constexpr std::uint8_t kLongUnsignedTag = 0x12u;
constexpr std::uint8_t kLong64Tag = 0x14u;
constexpr std::uint8_t kLong64UnsignedTag = 0x15u;
constexpr std::uint8_t kEnumTag = 0x16u;
constexpr std::uint8_t kFloat32Tag = 0x17u;
constexpr std::uint8_t kFloat64Tag = 0x18u;
constexpr std::uint8_t kDateTimeTag = 0x19u;
constexpr std::uint8_t kDateTag = 0x1Au;
constexpr std::uint8_t kTimeTag = 0x1Bu;
constexpr std::uint8_t kLogicalNameSize = 6u;
constexpr std::size_t kClockDateTimeOctetStringSize = 12u;
constexpr std::uint8_t kProfileGenericRangeSelector = 1u;
constexpr std::uint8_t kProfileGenericEntrySelector = 2u;
constexpr std::size_t kSystemTitleSize = 8u;
constexpr std::size_t kSuite0KeySize = 16u;
constexpr std::size_t kSuite0WrappedKeySize = 24u;
constexpr std::size_t kMaxProfileGenericDecodeDepth = 16u;
constexpr std::size_t kDlmsDateTimeSize = 12u;
constexpr std::size_t kDlmsDateSize = 5u;
constexpr std::size_t kDlmsTimeSize = 4u;

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

bool ReadByte(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint8_t& value)
{
  if (offset >= input.size()) {
    return false;
  }
  value = input[offset++];
  return true;
}

bool ReadExpectedTag(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint8_t expectedTag)
{
  std::uint8_t tag = 0u;
  return ReadByte(input, offset, tag) && tag == expectedTag;
}

bool ReadFixedBytes(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::size_t size,
  const std::uint8_t*& data)
{
  if (input.size() - offset < size) {
    return false;
  }
  data = &input[offset];
  offset += size;
  return true;
}

bool ReadLongUnsignedValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint16_t& value)
{
  if (!ReadExpectedTag(input, offset, kLongUnsignedTag) ||
      input.size() - offset < 2u) {
    return false;
  }

  value = static_cast<std::uint16_t>(
    (static_cast<std::uint16_t>(input[offset]) << 8u) |
    input[offset + 1u]);
  offset += 2u;
  return true;
}

bool ReadLongValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::int16_t& value)
{
  if (!ReadExpectedTag(input, offset, kLongTag) ||
      input.size() - offset < 2u) {
    return false;
  }

  const std::uint16_t raw = static_cast<std::uint16_t>(
    (static_cast<std::uint16_t>(input[offset]) << 8u) |
    input[offset + 1u]);
  value = static_cast<std::int16_t>(raw);
  offset += 2u;
  return true;
}

bool ReadIntegerValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint8_t& value)
{
  return ReadExpectedTag(input, offset, kIntegerTag) &&
         ReadByte(input, offset, value);
}

bool ReadBooleanValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  bool& value)
{
  std::uint8_t raw = 0u;
  if (!ReadExpectedTag(input, offset, kBooleanTag) ||
      !ReadByte(input, offset, raw)) {
    return false;
  }

  value = raw != 0u;
  return true;
}

bool ReadUnsignedValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint8_t& value)
{
  return ReadExpectedTag(input, offset, kUnsignedTag) &&
         ReadByte(input, offset, value);
}

bool ReadEnumValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint8_t& value)
{
  return ReadExpectedTag(input, offset, kEnumTag) &&
         ReadByte(input, offset, value);
}

bool ReadLogicalNameValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemLogicalName& logicalName)
{
  std::size_t length = 0u;
  const std::uint8_t* data = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, length) ||
      length != kLogicalNameSize ||
      !ReadFixedBytes(input, offset, length, data)) {
    return false;
  }

  logicalName = CosemLogicalName(
    data[0],
    data[1],
    data[2],
    data[3],
    data[4],
    data[5]);
  return true;
}

bool SkipDlmsData(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::size_t depth);

bool SkipDlmsDataItems(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::size_t count,
  std::size_t depth)
{
  for (std::size_t i = 0u; i < count; ++i) {
    if (!SkipDlmsData(input, offset, depth + 1u)) {
      return false;
    }
  }
  return true;
}

bool SkipDlmsData(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::size_t depth)
{
  if (depth > kMaxProfileGenericDecodeDepth) {
    return false;
  }

  std::uint8_t tag = 0u;
  if (!ReadByte(input, offset, tag)) {
    return false;
  }

  switch (tag) {
    case kNullDataTag:
      return true;
    case kArrayTag:
    case kStructureTag: {
      std::size_t count = 0u;
      return ReadAxdrLength(input, offset, count) &&
             SkipDlmsDataItems(input, offset, count, depth);
    }
    case kBooleanTag:
    case kIntegerTag:
    case kUnsignedTag:
    case kEnumTag:
      return input.size() - offset >= 1u && (offset += 1u, true);
    case kLongTag:
    case kLongUnsignedTag:
      return input.size() - offset >= 2u && (offset += 2u, true);
    case kDoubleLongTag:
    case kDoubleLongUnsignedTag:
      return input.size() - offset >= 4u && (offset += 4u, true);
    case kLong64Tag:
    case kLong64UnsignedTag:
      return input.size() - offset >= 8u && (offset += 8u, true);
    case kDateTimeTag:
      return input.size() - offset >= kDlmsDateTimeSize &&
             (offset += kDlmsDateTimeSize, true);
    case kDateTag:
      return input.size() - offset >= kDlmsDateSize &&
             (offset += kDlmsDateSize, true);
    case kTimeTag:
      return input.size() - offset >= kDlmsTimeSize &&
             (offset += kDlmsTimeSize, true);
    case kDataOctetStringTag: {
      std::size_t length = 0u;
      if (!ReadAxdrLength(input, offset, length) ||
          input.size() - offset < length) {
        return false;
      }
      offset += length;
      return true;
    }
    default:
      return false;
  }
}

bool SkipProfileGenericRangeValue(
  const CosemByteBuffer& input,
  std::size_t& offset)
{
  std::uint8_t tag = 0u;
  if (!ReadByte(input, offset, tag)) {
    return false;
  }

  switch (tag) {
    case kDoubleLongTag:
    case kDoubleLongUnsignedTag:
    case kFloat32Tag:
      return input.size() - offset >= 4u && (offset += 4u, true);
    case kDataOctetStringTag:
    case kVisibleStringTag:
    case kUtf8StringTag: {
      std::size_t length = 0u;
      if (!ReadAxdrLength(input, offset, length) ||
          input.size() - offset < length) {
        return false;
      }
      offset += length;
      return true;
    }
    case kIntegerTag:
    case kUnsignedTag:
      return input.size() - offset >= 1u && (offset += 1u, true);
    case kLongTag:
    case kLongUnsignedTag:
      return input.size() - offset >= 2u && (offset += 2u, true);
    case kLong64Tag:
    case kLong64UnsignedTag:
    case kFloat64Tag:
      return input.size() - offset >= 8u && (offset += 8u, true);
    case kDateTimeTag:
      return input.size() - offset >= kDlmsDateTimeSize &&
             (offset += kDlmsDateTimeSize, true);
    case kDateTag:
      return input.size() - offset >= kDlmsDateSize &&
             (offset += kDlmsDateSize, true);
    case kTimeTag:
      return input.size() - offset >= kDlmsTimeSize &&
             (offset += kDlmsTimeSize, true);
    default:
      return false;
  }
}

bool DecodeCaptureObjectAt(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemCaptureObject& object)
{
  std::size_t fieldCount = 0u;
  std::uint16_t classId = 0u;
  CosemLogicalName logicalName(0u, 0u, 0u, 0u, 0u, 0u);
  std::uint8_t attributeId = 0u;
  std::uint16_t dataIndex = 0u;

  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 4u ||
      !ReadLongUnsignedValue(input, offset, classId) ||
      !ReadLogicalNameValue(input, offset, logicalName) ||
      !ReadIntegerValue(input, offset, attributeId) ||
      !ReadLongUnsignedValue(input, offset, dataIndex)) {
    return false;
  }

  object.object.classId = classId;
  object.object.version = 0u;
  object.object.logicalName = logicalName;
  object.attributeId = attributeId;
  object.dataIndex = dataIndex;
  return true;
}

bool DecodeCaptureObjectsAt(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::vector<CosemCaptureObject>& objects)
{
  std::size_t count = 0u;
  std::vector<CosemCaptureObject> decoded;

  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return false;
  }

  decoded.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    CosemCaptureObject object;
    if (!DecodeCaptureObjectAt(input, offset, object)) {
      return false;
    }
    decoded.push_back(object);
  }

  objects.swap(decoded);
  return true;
}

bool ReadDoubleLongUnsignedValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint32_t& value)
{
  if (!ReadExpectedTag(input, offset, kDoubleLongUnsignedTag) ||
      input.size() - offset < 4u) {
    return false;
  }

  value =
    (static_cast<std::uint32_t>(input[offset]) << 24u) |
    (static_cast<std::uint32_t>(input[offset + 1u]) << 16u) |
    (static_cast<std::uint32_t>(input[offset + 2u]) << 8u) |
    static_cast<std::uint32_t>(input[offset + 3u]);
  offset += 4u;
  return true;
}

bool MapAttributeAccessMode(
  std::uint8_t value,
  AttributeAccessMode& mode)
{
  switch (value) {
    case 0u:
      mode = AttributeAccessMode::NoAccess;
      return true;
    case 1u:
      mode = AttributeAccessMode::ReadOnly;
      return true;
    case 2u:
      mode = AttributeAccessMode::WriteOnly;
      return true;
    case 3u:
      mode = AttributeAccessMode::ReadAndWrite;
      return true;
    case 4u:
      mode = AttributeAccessMode::AuthenticatedReadOnly;
      return true;
    case 5u:
      mode = AttributeAccessMode::AuthenticatedWriteOnly;
      return true;
    case 6u:
      mode = AttributeAccessMode::AuthenticatedReadAndWrite;
      return true;
    default:
      return false;
  }
}

bool MapMethodAccessMode(
  std::uint8_t value,
  MethodAccessMode& mode)
{
  switch (value) {
    case 0u:
      mode = MethodAccessMode::NoAccess;
      return true;
    case 1u:
      mode = MethodAccessMode::Access;
      return true;
    case 2u:
      mode = MethodAccessMode::AuthenticatedAccess;
      return true;
    default:
      return false;
  }
}

bool SkipAssociationAccessSelectors(
  const CosemByteBuffer& input,
  std::size_t& offset)
{
  if (offset >= input.size()) {
    return false;
  }
  if (input[offset] == kNullDataTag) {
    ++offset;
    return true;
  }

  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return false;
  }

  for (std::size_t i = 0u; i < count; ++i) {
    std::uint8_t selector = 0u;
    if (!ReadIntegerValue(input, offset, selector)) {
      return false;
    }
  }
  return true;
}

bool DecodeAttributeAccessAt(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemAccessRights& rights)
{
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return false;
  }

  for (std::size_t i = 0u; i < count; ++i) {
    std::size_t fieldCount = 0u;
    std::uint8_t attributeId = 0u;
    std::uint8_t modeValue = 0u;
    AttributeAccessMode mode = AttributeAccessMode::NoAccess;
    if (!ReadExpectedTag(input, offset, kStructureTag) ||
        !ReadAxdrLength(input, offset, fieldCount) ||
        fieldCount != 3u ||
        !ReadIntegerValue(input, offset, attributeId) ||
        !ReadEnumValue(input, offset, modeValue) ||
        !MapAttributeAccessMode(modeValue, mode) ||
        !SkipAssociationAccessSelectors(input, offset)) {
      return false;
    }
    rights.SetAttributeAccess(attributeId, mode);
  }
  return true;
}

bool DecodeMethodAccessAt(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemAccessRights& rights)
{
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return false;
  }

  for (std::size_t i = 0u; i < count; ++i) {
    std::size_t fieldCount = 0u;
    std::uint8_t methodId = 0u;
    std::uint8_t modeValue = 0u;
    MethodAccessMode mode = MethodAccessMode::NoAccess;
    if (!ReadExpectedTag(input, offset, kStructureTag) ||
        !ReadAxdrLength(input, offset, fieldCount) ||
        fieldCount != 2u ||
        !ReadIntegerValue(input, offset, methodId) ||
        !ReadEnumValue(input, offset, modeValue) ||
        !MapMethodAccessMode(modeValue, mode)) {
      return false;
    }
    rights.SetMethodAccess(methodId, mode);
  }
  return true;
}

bool DecodeAccessRightsAt(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemAccessRights& rights)
{
  std::size_t fieldCount = 0u;
  CosemAccessRights decoded;
  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 2u ||
      !DecodeAttributeAccessAt(input, offset, decoded) ||
      !DecodeMethodAccessAt(input, offset, decoded)) {
    return false;
  }

  rights = decoded;
  return true;
}

bool DecodeObjectListElementAt(
  const CosemByteBuffer& input,
  std::size_t& offset,
  AssociationViewObject& object)
{
  std::size_t fieldCount = 0u;
  std::uint16_t classId = 0u;
  std::uint8_t version = 0u;
  CosemLogicalName logicalName(0u, 0u, 0u, 0u, 0u, 0u);
  CosemAccessRights accessRights;

  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 4u ||
      !ReadLongUnsignedValue(input, offset, classId) ||
      !ReadUnsignedValue(input, offset, version) ||
      !ReadLogicalNameValue(input, offset, logicalName) ||
      !DecodeAccessRightsAt(input, offset, accessRights)) {
    return false;
  }

  object.descriptor.key.classId = classId;
  object.descriptor.key.version = version;
  object.descriptor.key.logicalName = logicalName;
  object.accessRights = accessRights;
  return true;
}

bool DecodeExactOctetString(
  const CosemByteBuffer& input,
  std::size_t expectedSize,
  CosemByteBuffer& value)
{
  std::size_t offset = 0u;
  std::size_t length = 0u;
  const std::uint8_t* data = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, length) ||
      length != expectedSize ||
      !ReadFixedBytes(input, offset, length, data) ||
      offset != input.size()) {
    return false;
  }

  value.assign(data, data + length);
  return true;
}

std::uint8_t NormalizeAssociationLnVersion(std::uint8_t version)
{
  if (version > kAssociationLnMaxVersion) {
    return kAssociationLnMaxVersion;
  }
  return version;
}

std::uint8_t NormalizeVersion(
  std::uint8_t version,
  std::uint8_t maxSupportedVersion)
{
  if (version > maxSupportedVersion) {
    return maxSupportedVersion;
  }
  return version;
}

CosemAssociationUser DefaultAssociationUser()
{
  CosemAssociationUser user;
  user.userId = 0u;
  return user;
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
  std::uint8_t version,
  const CosemLogicalName& logicalName)
{
  CosemObjectDescriptor descriptor;
  descriptor.key.classId = classId;
  descriptor.key.version = version;
  descriptor.key.logicalName = logicalName;
  return descriptor;
}

CosemObjectDescriptor MakeDescriptor(
  std::uint16_t classId,
  const CosemLogicalName& logicalName)
{
  return MakeDescriptor(classId, kVersion0, logicalName);
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

void AppendLong(CosemByteBuffer& output, std::int16_t value)
{
  const std::uint16_t raw = static_cast<std::uint16_t>(value);
  output.push_back(kLongTag);
  output.push_back(static_cast<std::uint8_t>(raw >> 8u));
  output.push_back(static_cast<std::uint8_t>(raw & 0xffu));
}

void AppendUnsigned(CosemByteBuffer& output, std::uint8_t value)
{
  output.push_back(kUnsignedTag);
  output.push_back(value);
}

void AppendBoolean(CosemByteBuffer& output, bool value)
{
  output.push_back(kBooleanTag);
  output.push_back(value ? 1u : 0u);
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

void AppendVisibleString(
  CosemByteBuffer& output,
  const std::string& value)
{
  output.push_back(kVisibleStringTag);
  AppendLength(output, value.size());
  output.insert(output.end(), value.begin(), value.end());
}

void AppendBufferOctetString(
  CosemByteBuffer& output,
  const CosemByteBuffer& value)
{
  static const std::uint8_t kEmpty = 0u;
  AppendOctetString(
    output,
    value.empty() ? &kEmpty : &value[0],
    value.size());
}

void AppendLogicalName(
  CosemByteBuffer& output,
  const CosemLogicalName& logicalName)
{
  AppendOctetString(output, logicalName.Data(), logicalName.Size());
}

void AppendAssociationUser(
  CosemByteBuffer& output,
  const CosemAssociationUser& user)
{
  AppendStructureHeader(output, 2u);
  AppendUnsigned(output, user.userId);
  AppendVisibleString(output, user.userName);
}

void AppendAssociationUserList(
  CosemByteBuffer& output,
  const std::vector<CosemAssociationUser>& users)
{
  AppendArrayHeader(output, users.size());
  for (std::size_t i = 0u; i < users.size(); ++i) {
    AppendAssociationUser(output, users[i]);
  }
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

void AppendCaptureObject(
  CosemByteBuffer& output,
  const CosemCaptureObject& object)
{
  AppendStructureHeader(output, 4u);
  AppendLongUnsigned(output, object.object.classId);
  AppendLogicalName(output, object.object.logicalName);
  AppendInteger(output, object.attributeId);
  AppendLongUnsigned(output, object.dataIndex);
}

void AppendProfileBuffer(
  CosemByteBuffer& output,
  const std::vector<CosemByteBuffer>& rows)
{
  AppendArrayHeader(output, rows.size());
  for (std::vector<CosemByteBuffer>::const_iterator it = rows.begin();
       it != rows.end();
       ++it) {
    output.insert(output.end(), it->begin(), it->end());
  }
}

void AppendProfileCaptureObjects(
  CosemByteBuffer& output,
  const std::vector<CosemCaptureObject>& objects)
{
  AppendArrayHeader(output, objects.size());
  for (std::vector<CosemCaptureObject>::const_iterator it = objects.begin();
       it != objects.end();
       ++it) {
    AppendCaptureObject(output, *it);
  }
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

std::uint8_t ProfileGenericRangeAccessSelector()
{
  return kProfileGenericRangeSelector;
}

std::uint8_t ProfileGenericEntryAccessSelector()
{
  return kProfileGenericEntrySelector;
}

CosemByteBuffer EncodeProfileGenericCaptureObject(
  const CosemCaptureObject& object)
{
  CosemByteBuffer output;
  AppendCaptureObject(output, object);
  return output;
}

CosemStatus DecodeProfileGenericCaptureObject(
  const CosemByteBuffer& input,
  CosemCaptureObject& object)
{
  CosemCaptureObject decoded;
  std::size_t offset = 0u;
  if (!DecodeCaptureObjectAt(input, offset, decoded) ||
      offset != input.size()) {
    return CosemStatus::InvalidArgument;
  }

  object = decoded;
  return CosemStatus::Ok;
}

CosemByteBuffer EncodeProfileGenericCaptureObjects(
  const std::vector<CosemCaptureObject>& objects)
{
  CosemByteBuffer output;
  AppendProfileCaptureObjects(output, objects);
  return output;
}

CosemStatus DecodeProfileGenericCaptureObjects(
  const CosemByteBuffer& input,
  std::vector<CosemCaptureObject>& objects)
{
  std::size_t offset = 0u;
  std::vector<CosemCaptureObject> decoded;

  if (!DecodeCaptureObjectsAt(input, offset, decoded) ||
      offset != input.size()) {
    return CosemStatus::InvalidArgument;
  }

  objects.swap(decoded);
  return CosemStatus::Ok;
}

CosemByteBuffer EncodeProfileGenericBuffer(
  const std::vector<CosemByteBuffer>& rows)
{
  CosemByteBuffer output;
  AppendProfileBuffer(output, rows);
  return output;
}

CosemStatus DecodeProfileGenericBuffer(
  const CosemByteBuffer& input,
  std::vector<CosemByteBuffer>& rows)
{
  std::size_t offset = 0u;
  std::size_t count = 0u;
  std::vector<CosemByteBuffer> decoded;

  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return CosemStatus::InvalidArgument;
  }

  decoded.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    const std::size_t rowStart = offset;
    if (offset >= input.size() || input[offset] != kStructureTag ||
        !SkipDlmsData(input, offset, 0u)) {
      return CosemStatus::InvalidArgument;
    }
    decoded.push_back(
      CosemByteBuffer(input.begin() + rowStart, input.begin() + offset));
  }

  if (offset != input.size()) {
    return CosemStatus::InvalidArgument;
  }

  rows.swap(decoded);
  return CosemStatus::Ok;
}

CosemByteBuffer EncodeProfileGenericRangeDescriptor(
  const CosemProfileGenericRangeDescriptor& descriptor)
{
  CosemByteBuffer output;
  AppendStructureHeader(output, 4u);
  AppendCaptureObject(output, descriptor.restrictingObject);
  output.insert(
    output.end(),
    descriptor.fromValue.begin(),
    descriptor.fromValue.end());
  output.insert(
    output.end(),
    descriptor.toValue.begin(),
    descriptor.toValue.end());
  AppendProfileCaptureObjects(output, descriptor.selectedValues);
  return output;
}

CosemStatus DecodeProfileGenericRangeDescriptor(
  const CosemByteBuffer& input,
  CosemProfileGenericRangeDescriptor& descriptor)
{
  std::size_t offset = 0u;
  std::size_t fieldCount = 0u;
  CosemProfileGenericRangeDescriptor decoded;

  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 4u ||
      !DecodeCaptureObjectAt(
        input,
        offset,
        decoded.restrictingObject)) {
    return CosemStatus::InvalidArgument;
  }

  std::size_t valueStart = offset;
  if (!SkipProfileGenericRangeValue(input, offset)) {
    return CosemStatus::InvalidArgument;
  }
  decoded.fromValue.assign(input.begin() + valueStart, input.begin() + offset);

  valueStart = offset;
  if (!SkipProfileGenericRangeValue(input, offset)) {
    return CosemStatus::InvalidArgument;
  }
  decoded.toValue.assign(input.begin() + valueStart, input.begin() + offset);

  if (!DecodeCaptureObjectsAt(input, offset, decoded.selectedValues) ||
      offset != input.size()) {
    return CosemStatus::InvalidArgument;
  }

  descriptor = decoded;
  return CosemStatus::Ok;
}

CosemByteBuffer EncodeProfileGenericEntryDescriptor(
  const CosemProfileGenericEntryDescriptor& descriptor)
{
  CosemByteBuffer output;
  AppendStructureHeader(output, 4u);
  AppendDoubleLongUnsigned(output, descriptor.fromEntry);
  AppendDoubleLongUnsigned(output, descriptor.toEntry);
  AppendLongUnsigned(output, descriptor.fromSelectedValue);
  AppendLongUnsigned(output, descriptor.toSelectedValue);
  return output;
}

CosemStatus DecodeProfileGenericEntryDescriptor(
  const CosemByteBuffer& input,
  CosemProfileGenericEntryDescriptor& descriptor)
{
  std::size_t offset = 0u;
  std::size_t fieldCount = 0u;
  CosemProfileGenericEntryDescriptor decoded;

  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 4u ||
      !ReadDoubleLongUnsignedValue(input, offset, decoded.fromEntry) ||
      !ReadDoubleLongUnsignedValue(input, offset, decoded.toEntry) ||
      !ReadLongUnsignedValue(input, offset, decoded.fromSelectedValue) ||
      !ReadLongUnsignedValue(input, offset, decoded.toSelectedValue) ||
      offset != input.size()) {
    return CosemStatus::InvalidArgument;
  }

  descriptor = decoded;
  return CosemStatus::Ok;
}

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

CosemByteBuffer EncodeAssociationAccessRights(
  const CosemAccessRights& rights)
{
  CosemByteBuffer output;
  AppendAccessRights(output, rights);
  return output;
}

CosemStatus DecodeAssociationAccessRights(
  const CosemByteBuffer& input,
  CosemAccessRights& rights)
{
  CosemAccessRights decoded;
  std::size_t offset = 0u;
  if (!DecodeAccessRightsAt(input, offset, decoded) ||
      offset != input.size()) {
    return CosemStatus::InvalidArgument;
  }

  rights = decoded;
  return CosemStatus::Ok;
}

CosemByteBuffer EncodeAssociationObjectList(
  const AssociationView& objectList)
{
  CosemByteBuffer output;
  AppendArrayHeader(output, objectList.objects.size());
  for (std::vector<AssociationViewObject>::const_iterator it =
         objectList.objects.begin();
       it != objectList.objects.end();
       ++it) {
    AppendObjectListElement(output, *it);
  }
  return output;
}

CosemStatus DecodeAssociationObjectList(
  const CosemByteBuffer& input,
  AssociationView& objectList)
{
  std::size_t offset = 0u;
  std::size_t count = 0u;
  AssociationView decoded;

  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return CosemStatus::InvalidArgument;
  }

  decoded.objects.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    AssociationViewObject object;
    if (!DecodeObjectListElementAt(input, offset, object)) {
      return CosemStatus::InvalidArgument;
    }
    decoded.objects.push_back(object);
  }

  if (offset != input.size()) {
    return CosemStatus::InvalidArgument;
  }

  objectList = decoded;
  return CosemStatus::Ok;
}

const std::uint8_t CosemDataObject::MaxSupportedVersion;

CosemDataObject::CosemDataObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  AttributeAccessMode valueAccess)
  : CosemDataObject(logicalName, value, valueAccess, kVersion0)
{
}

CosemDataObject::CosemDataObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  AttributeAccessMode valueAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kDataClassId,
      NormalizeVersion(version, CosemDataObject::MaxSupportedVersion),
      logicalName))
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
  output.clear();
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
  output.clear();
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

const std::uint8_t CosemRegisterObject::MaxSupportedVersion;

CosemRegisterObject::CosemRegisterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  const CosemByteBuffer& scalerUnit,
  AttributeAccessMode valueAccess)
  : CosemRegisterObject(
      logicalName,
      value,
      scalerUnit,
      valueAccess,
      kVersion0)
{
}

CosemRegisterObject::CosemRegisterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  const CosemByteBuffer& scalerUnit,
  AttributeAccessMode valueAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kRegisterClassId,
      NormalizeVersion(version, CosemRegisterObject::MaxSupportedVersion),
      logicalName))
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
  output.clear();
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
  output.clear();
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

namespace {
constexpr std::uint16_t kExtendedRegisterClassId = 4u;
constexpr std::uint8_t kExtendedRegisterStatusAttributeId = 4u;
constexpr std::uint8_t kExtendedRegisterCaptureTimeAttributeId = 5u;
constexpr std::uint8_t kExtendedRegisterResetMethodId = 1u;
} // namespace

const std::uint8_t CosemExtendedRegisterObject::MaxSupportedVersion;

CosemExtendedRegisterObject::CosemExtendedRegisterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  const CosemByteBuffer& scalerUnit,
  const CosemByteBuffer& status,
  const CosemByteBuffer& captureTime,
  AttributeAccessMode valueAccess)
  : CosemExtendedRegisterObject(
      logicalName,
      value,
      scalerUnit,
      status,
      captureTime,
      valueAccess,
      kVersion0)
{
}

CosemExtendedRegisterObject::CosemExtendedRegisterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& value,
  const CosemByteBuffer& scalerUnit,
  const CosemByteBuffer& status,
  const CosemByteBuffer& captureTime,
  AttributeAccessMode valueAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kExtendedRegisterClassId,
      NormalizeVersion(version, CosemExtendedRegisterObject::MaxSupportedVersion),
      logicalName))
  , value_(value)
  , scalerUnit_(scalerUnit)
  , status_(status)
  , captureTime_(captureTime)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(kValueAttributeId, valueAccess);
  rights_.SetAttributeAccess(
    kScalerUnitAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kExtendedRegisterStatusAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kExtendedRegisterCaptureTimeAttributeId,
    AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemExtendedRegisterObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemExtendedRegisterObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemExtendedRegisterObject::ReadAttribute(
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
  if (attributeId == kExtendedRegisterStatusAttributeId) {
    output = status_;
    return CosemStatus::Ok;
  }
  if (attributeId == kExtendedRegisterCaptureTimeAttributeId) {
    output = captureTime_;
    return CosemStatus::Ok;
  }
  output.clear();
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemExtendedRegisterObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId
      || attributeId == kScalerUnitAttributeId
      || attributeId == kExtendedRegisterStatusAttributeId
      || attributeId == kExtendedRegisterCaptureTimeAttributeId) {
    return CosemStatus::AccessDenied;
  }
  if (attributeId == kValueAttributeId) {
    value_ = input;
    return CosemStatus::Ok;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemExtendedRegisterObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // method 1 = reset. Built-in object exposes it explicitly as
  // UnsupportedFeature: application-defined semantics decide what reset means
  // for an extended register, and the COSEM object does not own that policy.
  if (methodId == kExtendedRegisterResetMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemExtendedRegisterObject::Value() const
{
  return value_;
}

const CosemByteBuffer& CosemExtendedRegisterObject::ScalerUnit() const
{
  return scalerUnit_;
}

const CosemByteBuffer& CosemExtendedRegisterObject::Status() const
{
  return status_;
}

const CosemByteBuffer& CosemExtendedRegisterObject::CaptureTime() const
{
  return captureTime_;
}

void CosemExtendedRegisterObject::SetValue(const CosemByteBuffer& value)
{
  value_ = value;
}

void CosemExtendedRegisterObject::SetScalerUnit(
  const CosemByteBuffer& scalerUnit)
{
  scalerUnit_ = scalerUnit;
}

void CosemExtendedRegisterObject::SetStatus(const CosemByteBuffer& status)
{
  status_ = status;
}

void CosemExtendedRegisterObject::SetCaptureTime(
  const CosemByteBuffer& captureTime)
{
  captureTime_ = captureTime;
}

namespace {
constexpr std::uint16_t kDemandRegisterClassId = 5u;
constexpr std::uint8_t kDemandRegisterCurrentAvgAttributeId = 2u;
constexpr std::uint8_t kDemandRegisterLastAvgAttributeId = 3u;
constexpr std::uint8_t kDemandRegisterScalerUnitAttributeId = 4u;
constexpr std::uint8_t kDemandRegisterStatusAttributeId = 5u;
constexpr std::uint8_t kDemandRegisterCaptureTimeAttributeId = 6u;
constexpr std::uint8_t kDemandRegisterStartTimeCurrentAttributeId = 7u;
constexpr std::uint8_t kDemandRegisterPeriodAttributeId = 8u;
constexpr std::uint8_t kDemandRegisterNumberOfPeriodsAttributeId = 9u;
constexpr std::uint8_t kDemandRegisterResetMethodId = 1u;
constexpr std::uint8_t kDemandRegisterNextPeriodMethodId = 2u;
} // namespace

const std::uint8_t CosemDemandRegisterObject::MaxSupportedVersion;

CosemDemandRegisterObject::CosemDemandRegisterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& currentAverageValue,
  const CosemByteBuffer& lastAverageValue,
  const CosemByteBuffer& scalerUnit,
  const CosemByteBuffer& status,
  const CosemByteBuffer& captureTime,
  const CosemByteBuffer& startTimeCurrent,
  std::uint32_t period,
  std::uint16_t numberOfPeriods)
  : CosemDemandRegisterObject(
      logicalName,
      currentAverageValue,
      lastAverageValue,
      scalerUnit,
      status,
      captureTime,
      startTimeCurrent,
      period,
      numberOfPeriods,
      kVersion0)
{
}

CosemDemandRegisterObject::CosemDemandRegisterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& currentAverageValue,
  const CosemByteBuffer& lastAverageValue,
  const CosemByteBuffer& scalerUnit,
  const CosemByteBuffer& status,
  const CosemByteBuffer& captureTime,
  const CosemByteBuffer& startTimeCurrent,
  std::uint32_t period,
  std::uint16_t numberOfPeriods,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kDemandRegisterClassId,
      NormalizeVersion(version, CosemDemandRegisterObject::MaxSupportedVersion),
      logicalName))
  , currentAverageValue_(currentAverageValue)
  , lastAverageValue_(lastAverageValue)
  , scalerUnit_(scalerUnit)
  , status_(status)
  , captureTime_(captureTime)
  , startTimeCurrent_(startTimeCurrent)
  , period_(period)
  , numberOfPeriods_(numberOfPeriods)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterCurrentAvgAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterLastAvgAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterScalerUnitAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterStatusAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterCaptureTimeAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterStartTimeCurrentAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterPeriodAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDemandRegisterNumberOfPeriodsAttributeId,
    AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemDemandRegisterObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemDemandRegisterObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemDemandRegisterObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterCurrentAvgAttributeId) {
    output = currentAverageValue_;
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterLastAvgAttributeId) {
    output = lastAverageValue_;
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterScalerUnitAttributeId) {
    output = scalerUnit_;
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterStatusAttributeId) {
    output = status_;
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterCaptureTimeAttributeId) {
    output = captureTime_;
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterStartTimeCurrentAttributeId) {
    output = startTimeCurrent_;
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterPeriodAttributeId) {
    output.clear();
    AppendDoubleLongUnsigned(output, period_);
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterNumberOfPeriodsAttributeId) {
    output.clear();
    AppendLongUnsigned(output, numberOfPeriods_);
    return CosemStatus::Ok;
  }
  output.clear();
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemDemandRegisterObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  (void)input;
  if (attributeId == kLogicalNameAttributeId
      || attributeId == kDemandRegisterCurrentAvgAttributeId
      || attributeId == kDemandRegisterLastAvgAttributeId
      || attributeId == kDemandRegisterScalerUnitAttributeId
      || attributeId == kDemandRegisterStatusAttributeId
      || attributeId == kDemandRegisterCaptureTimeAttributeId
      || attributeId == kDemandRegisterStartTimeCurrentAttributeId
      || attributeId == kDemandRegisterPeriodAttributeId
      || attributeId == kDemandRegisterNumberOfPeriodsAttributeId) {
    return CosemStatus::AccessDenied;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemDemandRegisterObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // methods 1 (reset) and 2 (next_period) are application-defined; the
  // built-in object exposes them explicitly as UnsupportedFeature instead of
  // silently ignoring them.
  if (methodId == kDemandRegisterResetMethodId
      || methodId == kDemandRegisterNextPeriodMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemDemandRegisterObject::CurrentAverageValue() const
{
  return currentAverageValue_;
}

const CosemByteBuffer& CosemDemandRegisterObject::LastAverageValue() const
{
  return lastAverageValue_;
}

const CosemByteBuffer& CosemDemandRegisterObject::ScalerUnit() const
{
  return scalerUnit_;
}

const CosemByteBuffer& CosemDemandRegisterObject::Status() const
{
  return status_;
}

const CosemByteBuffer& CosemDemandRegisterObject::CaptureTime() const
{
  return captureTime_;
}

const CosemByteBuffer& CosemDemandRegisterObject::StartTimeCurrent() const
{
  return startTimeCurrent_;
}

std::uint32_t CosemDemandRegisterObject::Period() const
{
  return period_;
}

std::uint16_t CosemDemandRegisterObject::NumberOfPeriods() const
{
  return numberOfPeriods_;
}

void CosemDemandRegisterObject::SetCurrentAverageValue(
  const CosemByteBuffer& value)
{
  currentAverageValue_ = value;
}

void CosemDemandRegisterObject::SetLastAverageValue(
  const CosemByteBuffer& value)
{
  lastAverageValue_ = value;
}

void CosemDemandRegisterObject::SetScalerUnit(
  const CosemByteBuffer& scalerUnit)
{
  scalerUnit_ = scalerUnit;
}

void CosemDemandRegisterObject::SetStatus(const CosemByteBuffer& status)
{
  status_ = status;
}

void CosemDemandRegisterObject::SetCaptureTime(
  const CosemByteBuffer& captureTime)
{
  captureTime_ = captureTime;
}

void CosemDemandRegisterObject::SetStartTimeCurrent(
  const CosemByteBuffer& startTime)
{
  startTimeCurrent_ = startTime;
}

void CosemDemandRegisterObject::SetPeriod(std::uint32_t period)
{
  period_ = period;
}

void CosemDemandRegisterObject::SetNumberOfPeriods(
  std::uint16_t numberOfPeriods)
{
  numberOfPeriods_ = numberOfPeriods;
}

namespace {
constexpr std::uint16_t kRegisterActivationClassId = 6u;
constexpr std::uint8_t kRegisterActivationRegisterAssignmentAttributeId = 2u;
constexpr std::uint8_t kRegisterActivationMaskListAttributeId = 3u;
constexpr std::uint8_t kRegisterActivationActiveMaskAttributeId = 4u;
constexpr std::uint8_t kRegisterActivationAddRegisterMethodId = 1u;
constexpr std::uint8_t kRegisterActivationAddMaskMethodId = 2u;
constexpr std::uint8_t kRegisterActivationDeleteMaskMethodId = 3u;
} // namespace

const std::uint8_t CosemRegisterActivationObject::MaxSupportedVersion;

CosemRegisterActivationObject::CosemRegisterActivationObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& registerAssignment,
  const CosemByteBuffer& maskList,
  const CosemByteBuffer& activeMask)
  : CosemRegisterActivationObject(
      logicalName,
      registerAssignment,
      maskList,
      activeMask,
      kVersion0)
{
}

CosemRegisterActivationObject::CosemRegisterActivationObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& registerAssignment,
  const CosemByteBuffer& maskList,
  const CosemByteBuffer& activeMask,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kRegisterActivationClassId,
      NormalizeVersion(
        version,
        CosemRegisterActivationObject::MaxSupportedVersion),
      logicalName))
  , registerAssignment_(registerAssignment)
  , maskList_(maskList)
  , activeMask_(activeMask)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kRegisterActivationRegisterAssignmentAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kRegisterActivationMaskListAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kRegisterActivationActiveMaskAttributeId,
    AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemRegisterActivationObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemRegisterActivationObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemRegisterActivationObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterActivationRegisterAssignmentAttributeId) {
    output = registerAssignment_;
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterActivationMaskListAttributeId) {
    output = maskList_;
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterActivationActiveMaskAttributeId) {
    output = activeMask_;
    return CosemStatus::Ok;
  }
  output.clear();
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemRegisterActivationObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  (void)input;
  if (attributeId == kLogicalNameAttributeId
      || attributeId == kRegisterActivationRegisterAssignmentAttributeId
      || attributeId == kRegisterActivationMaskListAttributeId
      || attributeId == kRegisterActivationActiveMaskAttributeId) {
    return CosemStatus::AccessDenied;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemRegisterActivationObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // methods 1 (add_register), 2 (add_mask) and 3 (delete_mask) mutate
  // assignment / mask state owned by the application. The built-in object
  // surfaces them explicitly as UnsupportedFeature; a future backend can
  // attach mutating policy without changing the object surface.
  if (methodId == kRegisterActivationAddRegisterMethodId
      || methodId == kRegisterActivationAddMaskMethodId
      || methodId == kRegisterActivationDeleteMaskMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemRegisterActivationObject::RegisterAssignment() const
{
  return registerAssignment_;
}

const CosemByteBuffer& CosemRegisterActivationObject::MaskList() const
{
  return maskList_;
}

const CosemByteBuffer& CosemRegisterActivationObject::ActiveMask() const
{
  return activeMask_;
}

void CosemRegisterActivationObject::SetRegisterAssignment(
  const CosemByteBuffer& assignment)
{
  registerAssignment_ = assignment;
}

void CosemRegisterActivationObject::SetMaskList(
  const CosemByteBuffer& maskList)
{
  maskList_ = maskList;
}

void CosemRegisterActivationObject::SetActiveMask(
  const CosemByteBuffer& activeMask)
{
  activeMask_ = activeMask;
}

namespace {
constexpr std::uint16_t kRegisterMonitorClassId = 21u;
constexpr std::uint8_t kRegisterMonitorThresholdsAttributeId = 2u;
constexpr std::uint8_t kRegisterMonitorMonitoredValueAttributeId = 3u;
constexpr std::uint8_t kRegisterMonitorActionsAttributeId = 4u;
} // namespace

const std::uint8_t CosemRegisterMonitorObject::MaxSupportedVersion;

CosemRegisterMonitorObject::CosemRegisterMonitorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& thresholds,
  const CosemByteBuffer& monitoredValue,
  const CosemByteBuffer& actions,
  AttributeAccessMode thresholdsAccess)
  : CosemRegisterMonitorObject(
      logicalName,
      thresholds,
      monitoredValue,
      actions,
      thresholdsAccess,
      kVersion0)
{
}

CosemRegisterMonitorObject::CosemRegisterMonitorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& thresholds,
  const CosemByteBuffer& monitoredValue,
  const CosemByteBuffer& actions,
  AttributeAccessMode thresholdsAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kRegisterMonitorClassId,
      NormalizeVersion(
        version,
        CosemRegisterMonitorObject::MaxSupportedVersion),
      logicalName))
  , thresholds_(thresholds)
  , monitoredValue_(monitoredValue)
  , actions_(actions)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kRegisterMonitorThresholdsAttributeId,
    thresholdsAccess);
  rights_.SetAttributeAccess(
    kRegisterMonitorMonitoredValueAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kRegisterMonitorActionsAttributeId,
    AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemRegisterMonitorObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemRegisterMonitorObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemRegisterMonitorObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterMonitorThresholdsAttributeId) {
    output = thresholds_;
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterMonitorMonitoredValueAttributeId) {
    output = monitoredValue_;
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterMonitorActionsAttributeId) {
    output = actions_;
    return CosemStatus::Ok;
  }
  output.clear();
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemRegisterMonitorObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kRegisterMonitorThresholdsAttributeId) {
    const AttributeAccessMode mode = rights_.AttributeAccess(
      kRegisterMonitorThresholdsAttributeId);
    if (mode == AttributeAccessMode::WriteOnly
        || mode == AttributeAccessMode::ReadAndWrite
        || mode == AttributeAccessMode::AuthenticatedWriteOnly
        || mode == AttributeAccessMode::AuthenticatedReadAndWrite) {
      thresholds_ = input;
      return CosemStatus::Ok;
    }
    return CosemStatus::AccessDenied;
  }
  if (attributeId == kLogicalNameAttributeId
      || attributeId == kRegisterMonitorMonitoredValueAttributeId
      || attributeId == kRegisterMonitorActionsAttributeId) {
    return CosemStatus::AccessDenied;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemRegisterMonitorObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Register Monitor v0 defines no methods; every method id is
  // unconditionally MethodNotFound.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemRegisterMonitorObject::Thresholds() const
{
  return thresholds_;
}

const CosemByteBuffer& CosemRegisterMonitorObject::MonitoredValue() const
{
  return monitoredValue_;
}

const CosemByteBuffer& CosemRegisterMonitorObject::Actions() const
{
  return actions_;
}

void CosemRegisterMonitorObject::SetThresholds(
  const CosemByteBuffer& thresholds)
{
  thresholds_ = thresholds;
}

void CosemRegisterMonitorObject::SetMonitoredValue(
  const CosemByteBuffer& monitoredValue)
{
  monitoredValue_ = monitoredValue;
}

void CosemRegisterMonitorObject::SetActions(
  const CosemByteBuffer& actions)
{
  actions_ = actions;
}

namespace {
constexpr std::uint16_t kScriptTableClassId = 9u;
constexpr std::uint8_t kScriptTableScriptsAttributeId = 2u;
constexpr std::uint8_t kScriptTableExecuteMethodId = 1u;
} // namespace

const std::uint8_t CosemScriptTableObject::MaxSupportedVersion;

CosemScriptTableObject::CosemScriptTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& scripts,
  AttributeAccessMode scriptsAccess)
  : CosemScriptTableObject(
      logicalName,
      scripts,
      scriptsAccess,
      kVersion0)
{
}

CosemScriptTableObject::CosemScriptTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& scripts,
  AttributeAccessMode scriptsAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kScriptTableClassId,
      NormalizeVersion(
        version,
        CosemScriptTableObject::MaxSupportedVersion),
      logicalName))
  , scripts_(scripts)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kScriptTableScriptsAttributeId,
    scriptsAccess);
}

CosemObjectDescriptor CosemScriptTableObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemScriptTableObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemScriptTableObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kScriptTableScriptsAttributeId) {
    output = scripts_;
    return CosemStatus::Ok;
  }
  output.clear();
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemScriptTableObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kScriptTableScriptsAttributeId) {
    const AttributeAccessMode mode = rights_.AttributeAccess(
      kScriptTableScriptsAttributeId);
    if (mode == AttributeAccessMode::WriteOnly
        || mode == AttributeAccessMode::ReadAndWrite
        || mode == AttributeAccessMode::AuthenticatedWriteOnly
        || mode == AttributeAccessMode::AuthenticatedReadAndWrite) {
      scripts_ = input;
      return CosemStatus::Ok;
    }
    return CosemStatus::AccessDenied;
  }
  if (attributeId == kLogicalNameAttributeId) {
    return CosemStatus::AccessDenied;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemScriptTableObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // method 1 (execute) dispatches application-defined script semantics; the
  // built-in object surfaces it explicitly as UnsupportedFeature so that a
  // future backend can attach script execution without changing the object
  // surface.
  if (methodId == kScriptTableExecuteMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemScriptTableObject::Scripts() const
{
  return scripts_;
}

void CosemScriptTableObject::SetScripts(const CosemByteBuffer& scripts)
{
  scripts_ = scripts;
}

namespace {
constexpr std::uint16_t kActivityCalendarClassId = 20u;
constexpr std::uint8_t kActivityCalendarCalendarNameActiveAttributeId = 2u;
constexpr std::uint8_t kActivityCalendarSeasonProfileActiveAttributeId = 3u;
constexpr std::uint8_t kActivityCalendarWeekProfileTableActiveAttributeId =
  4u;
constexpr std::uint8_t kActivityCalendarDayProfileTableActiveAttributeId =
  5u;
constexpr std::uint8_t kActivityCalendarCalendarNamePassiveAttributeId = 6u;
constexpr std::uint8_t kActivityCalendarSeasonProfilePassiveAttributeId =
  7u;
constexpr std::uint8_t kActivityCalendarWeekProfileTablePassiveAttributeId =
  8u;
constexpr std::uint8_t kActivityCalendarDayProfileTablePassiveAttributeId =
  9u;
constexpr std::uint8_t kActivityCalendarActivatePassiveCalendarTimeAttributeId
  = 10u;
constexpr std::uint8_t kActivityCalendarActivatePassiveCalendarMethodId =
  1u;

bool IsAccessWritable(AttributeAccessMode mode)
{
  return mode == AttributeAccessMode::WriteOnly
      || mode == AttributeAccessMode::ReadAndWrite
      || mode == AttributeAccessMode::AuthenticatedWriteOnly
      || mode == AttributeAccessMode::AuthenticatedReadAndWrite;
}
} // namespace

const std::uint8_t CosemActivityCalendarObject::MaxSupportedVersion;

CosemActivityCalendarObject::CosemActivityCalendarObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& calendarNameActive,
  const CosemByteBuffer& seasonProfileActive,
  const CosemByteBuffer& weekProfileTableActive,
  const CosemByteBuffer& dayProfileTableActive,
  const CosemByteBuffer& calendarNamePassive,
  const CosemByteBuffer& seasonProfilePassive,
  const CosemByteBuffer& weekProfileTablePassive,
  const CosemByteBuffer& dayProfileTablePassive,
  const CosemByteBuffer& activatePassiveCalendarTime,
  AttributeAccessMode passiveAccess)
  : CosemActivityCalendarObject(
      logicalName,
      calendarNameActive,
      seasonProfileActive,
      weekProfileTableActive,
      dayProfileTableActive,
      calendarNamePassive,
      seasonProfilePassive,
      weekProfileTablePassive,
      dayProfileTablePassive,
      activatePassiveCalendarTime,
      passiveAccess,
      kVersion0)
{
}

CosemActivityCalendarObject::CosemActivityCalendarObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& calendarNameActive,
  const CosemByteBuffer& seasonProfileActive,
  const CosemByteBuffer& weekProfileTableActive,
  const CosemByteBuffer& dayProfileTableActive,
  const CosemByteBuffer& calendarNamePassive,
  const CosemByteBuffer& seasonProfilePassive,
  const CosemByteBuffer& weekProfileTablePassive,
  const CosemByteBuffer& dayProfileTablePassive,
  const CosemByteBuffer& activatePassiveCalendarTime,
  AttributeAccessMode passiveAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kActivityCalendarClassId,
      NormalizeVersion(
        version,
        CosemActivityCalendarObject::MaxSupportedVersion),
      logicalName))
  , calendarNameActive_(calendarNameActive)
  , seasonProfileActive_(seasonProfileActive)
  , weekProfileTableActive_(weekProfileTableActive)
  , dayProfileTableActive_(dayProfileTableActive)
  , calendarNamePassive_(calendarNamePassive)
  , seasonProfilePassive_(seasonProfilePassive)
  , weekProfileTablePassive_(weekProfileTablePassive)
  , dayProfileTablePassive_(dayProfileTablePassive)
  , activatePassiveCalendarTime_(activatePassiveCalendarTime)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kActivityCalendarCalendarNameActiveAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kActivityCalendarSeasonProfileActiveAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kActivityCalendarWeekProfileTableActiveAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kActivityCalendarDayProfileTableActiveAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kActivityCalendarCalendarNamePassiveAttributeId, passiveAccess);
  rights_.SetAttributeAccess(
    kActivityCalendarSeasonProfilePassiveAttributeId, passiveAccess);
  rights_.SetAttributeAccess(
    kActivityCalendarWeekProfileTablePassiveAttributeId, passiveAccess);
  rights_.SetAttributeAccess(
    kActivityCalendarDayProfileTablePassiveAttributeId, passiveAccess);
  rights_.SetAttributeAccess(
    kActivityCalendarActivatePassiveCalendarTimeAttributeId,
    passiveAccess);
}

CosemObjectDescriptor CosemActivityCalendarObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemActivityCalendarObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemActivityCalendarObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kActivityCalendarCalendarNameActiveAttributeId:
      output = calendarNameActive_;
      return CosemStatus::Ok;
    case kActivityCalendarSeasonProfileActiveAttributeId:
      output = seasonProfileActive_;
      return CosemStatus::Ok;
    case kActivityCalendarWeekProfileTableActiveAttributeId:
      output = weekProfileTableActive_;
      return CosemStatus::Ok;
    case kActivityCalendarDayProfileTableActiveAttributeId:
      output = dayProfileTableActive_;
      return CosemStatus::Ok;
    case kActivityCalendarCalendarNamePassiveAttributeId:
      output = calendarNamePassive_;
      return CosemStatus::Ok;
    case kActivityCalendarSeasonProfilePassiveAttributeId:
      output = seasonProfilePassive_;
      return CosemStatus::Ok;
    case kActivityCalendarWeekProfileTablePassiveAttributeId:
      output = weekProfileTablePassive_;
      return CosemStatus::Ok;
    case kActivityCalendarDayProfileTablePassiveAttributeId:
      output = dayProfileTablePassive_;
      return CosemStatus::Ok;
    case kActivityCalendarActivatePassiveCalendarTimeAttributeId:
      output = activatePassiveCalendarTime_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemActivityCalendarObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kActivityCalendarCalendarNamePassiveAttributeId: {
      if (IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        calendarNamePassive_ = input;
        return CosemStatus::Ok;
      }
      return CosemStatus::AccessDenied;
    }
    case kActivityCalendarSeasonProfilePassiveAttributeId: {
      if (IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        seasonProfilePassive_ = input;
        return CosemStatus::Ok;
      }
      return CosemStatus::AccessDenied;
    }
    case kActivityCalendarWeekProfileTablePassiveAttributeId: {
      if (IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        weekProfileTablePassive_ = input;
        return CosemStatus::Ok;
      }
      return CosemStatus::AccessDenied;
    }
    case kActivityCalendarDayProfileTablePassiveAttributeId: {
      if (IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        dayProfileTablePassive_ = input;
        return CosemStatus::Ok;
      }
      return CosemStatus::AccessDenied;
    }
    case kActivityCalendarActivatePassiveCalendarTimeAttributeId: {
      if (IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        activatePassiveCalendarTime_ = input;
        return CosemStatus::Ok;
      }
      return CosemStatus::AccessDenied;
    }
    case kLogicalNameAttributeId:
    case kActivityCalendarCalendarNameActiveAttributeId:
    case kActivityCalendarSeasonProfileActiveAttributeId:
    case kActivityCalendarWeekProfileTableActiveAttributeId:
    case kActivityCalendarDayProfileTableActiveAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemActivityCalendarObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // method 1 (activate_passive_calendar) is application-defined: it
  // atomically copies passive attributes into the active set at meter
  // time. The built-in object surfaces it explicitly as UnsupportedFeature
  // so that a future backend can attach activation policy without changing
  // the object surface.
  if (methodId == kActivityCalendarActivatePassiveCalendarMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemActivityCalendarObject::CalendarNameActive() const
{
  return calendarNameActive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::SeasonProfileActive() const
{
  return seasonProfileActive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::WeekProfileTableActive() const
{
  return weekProfileTableActive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::DayProfileTableActive() const
{
  return dayProfileTableActive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::CalendarNamePassive() const
{
  return calendarNamePassive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::SeasonProfilePassive() const
{
  return seasonProfilePassive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::WeekProfileTablePassive() const
{
  return weekProfileTablePassive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::DayProfileTablePassive() const
{
  return dayProfileTablePassive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::ActivatePassiveCalendarTime() const
{
  return activatePassiveCalendarTime_;
}

void CosemActivityCalendarObject::SetCalendarNamePassive(
  const CosemByteBuffer& value)
{
  calendarNamePassive_ = value;
}

void CosemActivityCalendarObject::SetSeasonProfilePassive(
  const CosemByteBuffer& value)
{
  seasonProfilePassive_ = value;
}

void CosemActivityCalendarObject::SetWeekProfileTablePassive(
  const CosemByteBuffer& value)
{
  weekProfileTablePassive_ = value;
}

void CosemActivityCalendarObject::SetDayProfileTablePassive(
  const CosemByteBuffer& value)
{
  dayProfileTablePassive_ = value;
}

void CosemActivityCalendarObject::SetActivatePassiveCalendarTime(
  const CosemByteBuffer& value)
{
  activatePassiveCalendarTime_ = value;
}

namespace {
constexpr std::uint16_t kImageTransferClassId = 18u;
constexpr std::uint8_t kImageTransferBlockSizeAttributeId = 2u;
constexpr std::uint8_t kImageTransferTransferredBlocksStatusAttributeId = 3u;
constexpr std::uint8_t kImageTransferFirstNotTransferredBlockAttributeId =
  4u;
constexpr std::uint8_t kImageTransferEnabledAttributeId = 5u;
constexpr std::uint8_t kImageTransferStatusAttributeId = 6u;
constexpr std::uint8_t kImageTransferToActivateInfoAttributeId = 7u;
constexpr std::uint8_t kImageTransferInitiateMethodId = 1u;
constexpr std::uint8_t kImageTransferBlockTransferMethodId = 2u;
constexpr std::uint8_t kImageTransferVerifyMethodId = 3u;
constexpr std::uint8_t kImageTransferActivateMethodId = 4u;
} // namespace

const std::uint8_t CosemImageTransferObject::MaxSupportedVersion;

CosemImageTransferObject::CosemImageTransferObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& imageBlockSize,
  const CosemByteBuffer& imageTransferredBlocksStatus,
  const CosemByteBuffer& imageFirstNotTransferredBlockNumber,
  const CosemByteBuffer& imageTransferEnabled,
  const CosemByteBuffer& imageTransferStatus,
  const CosemByteBuffer& imageToActivateInfo,
  AttributeAccessMode transferEnabledAccess)
  : CosemImageTransferObject(
      logicalName,
      imageBlockSize,
      imageTransferredBlocksStatus,
      imageFirstNotTransferredBlockNumber,
      imageTransferEnabled,
      imageTransferStatus,
      imageToActivateInfo,
      transferEnabledAccess,
      kVersion0)
{
}

CosemImageTransferObject::CosemImageTransferObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& imageBlockSize,
  const CosemByteBuffer& imageTransferredBlocksStatus,
  const CosemByteBuffer& imageFirstNotTransferredBlockNumber,
  const CosemByteBuffer& imageTransferEnabled,
  const CosemByteBuffer& imageTransferStatus,
  const CosemByteBuffer& imageToActivateInfo,
  AttributeAccessMode transferEnabledAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kImageTransferClassId,
      NormalizeVersion(
        version,
        CosemImageTransferObject::MaxSupportedVersion),
      logicalName))
  , imageBlockSize_(imageBlockSize)
  , imageTransferredBlocksStatus_(imageTransferredBlocksStatus)
  , imageFirstNotTransferredBlockNumber_(imageFirstNotTransferredBlockNumber)
  , imageTransferEnabled_(imageTransferEnabled)
  , imageTransferStatus_(imageTransferStatus)
  , imageToActivateInfo_(imageToActivateInfo)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kImageTransferBlockSizeAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kImageTransferTransferredBlocksStatusAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kImageTransferFirstNotTransferredBlockAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kImageTransferEnabledAttributeId, transferEnabledAccess);
  rights_.SetAttributeAccess(
    kImageTransferStatusAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kImageTransferToActivateInfoAttributeId,
    AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemImageTransferObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemImageTransferObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemImageTransferObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kImageTransferBlockSizeAttributeId:
      output = imageBlockSize_;
      return CosemStatus::Ok;
    case kImageTransferTransferredBlocksStatusAttributeId:
      output = imageTransferredBlocksStatus_;
      return CosemStatus::Ok;
    case kImageTransferFirstNotTransferredBlockAttributeId:
      output = imageFirstNotTransferredBlockNumber_;
      return CosemStatus::Ok;
    case kImageTransferEnabledAttributeId:
      output = imageTransferEnabled_;
      return CosemStatus::Ok;
    case kImageTransferStatusAttributeId:
      output = imageTransferStatus_;
      return CosemStatus::Ok;
    case kImageTransferToActivateInfoAttributeId:
      output = imageToActivateInfo_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemImageTransferObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kImageTransferEnabledAttributeId: {
      if (IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        imageTransferEnabled_ = input;
        return CosemStatus::Ok;
      }
      return CosemStatus::AccessDenied;
    }
    case kLogicalNameAttributeId:
    case kImageTransferBlockSizeAttributeId:
    case kImageTransferTransferredBlocksStatusAttributeId:
    case kImageTransferFirstNotTransferredBlockAttributeId:
    case kImageTransferStatusAttributeId:
    case kImageTransferToActivateInfoAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemImageTransferObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // methods 1..4 (image_transfer_initiate, image_block_transfer,
  // image_verify, image_activate) dispatch application-defined firmware
  // transfer/storage semantics. The built-in object surfaces them
  // explicitly as UnsupportedFeature so that a future backend can attach
  // image storage and activation without changing the object surface.
  switch (methodId) {
    case kImageTransferInitiateMethodId:
    case kImageTransferBlockTransferMethodId:
    case kImageTransferVerifyMethodId:
    case kImageTransferActivateMethodId:
      return CosemStatus::UnsupportedFeature;
    default:
      return CosemStatus::MethodNotFound;
  }
}

const CosemByteBuffer& CosemImageTransferObject::ImageBlockSize() const
{
  return imageBlockSize_;
}

const CosemByteBuffer&
CosemImageTransferObject::ImageTransferredBlocksStatus() const
{
  return imageTransferredBlocksStatus_;
}

const CosemByteBuffer&
CosemImageTransferObject::ImageFirstNotTransferredBlockNumber() const
{
  return imageFirstNotTransferredBlockNumber_;
}

const CosemByteBuffer&
CosemImageTransferObject::ImageTransferEnabled() const
{
  return imageTransferEnabled_;
}

const CosemByteBuffer&
CosemImageTransferObject::ImageTransferStatus() const
{
  return imageTransferStatus_;
}

const CosemByteBuffer&
CosemImageTransferObject::ImageToActivateInfo() const
{
  return imageToActivateInfo_;
}

void CosemImageTransferObject::SetImageBlockSize(
  const CosemByteBuffer& value)
{
  imageBlockSize_ = value;
}

void CosemImageTransferObject::SetImageTransferredBlocksStatus(
  const CosemByteBuffer& value)
{
  imageTransferredBlocksStatus_ = value;
}

void CosemImageTransferObject::SetImageFirstNotTransferredBlockNumber(
  const CosemByteBuffer& value)
{
  imageFirstNotTransferredBlockNumber_ = value;
}

void CosemImageTransferObject::SetImageTransferEnabled(
  const CosemByteBuffer& value)
{
  imageTransferEnabled_ = value;
}

void CosemImageTransferObject::SetImageTransferStatus(
  const CosemByteBuffer& value)
{
  imageTransferStatus_ = value;
}

void CosemImageTransferObject::SetImageToActivateInfo(
  const CosemByteBuffer& value)
{
  imageToActivateInfo_ = value;
}

namespace {
constexpr std::uint16_t kPushSetupClassId = 40u;
constexpr std::uint8_t kPushSetupObjectListAttributeId = 2u;
constexpr std::uint8_t kPushSetupSendDestinationAndMethodAttributeId = 3u;
constexpr std::uint8_t kPushSetupCommunicationWindowAttributeId = 4u;
constexpr std::uint8_t kPushSetupRandomisationStartIntervalAttributeId = 5u;
constexpr std::uint8_t kPushSetupNumberOfRetriesAttributeId = 6u;
constexpr std::uint8_t kPushSetupRepetitionDelayAttributeId = 7u;
constexpr std::uint8_t kPushSetupPortReferenceAttributeId = 8u;
constexpr std::uint8_t kPushSetupPushClientSapAttributeId = 9u;
constexpr std::uint8_t kPushSetupPushProtectionParametersAttributeId = 10u;
constexpr std::uint8_t kPushSetupPushOperationMethodAttributeId = 11u;
constexpr std::uint8_t kPushSetupConfirmationParametersAttributeId = 12u;
constexpr std::uint8_t kPushSetupLastConfirmationDateTimeAttributeId = 13u;
constexpr std::uint8_t kPushSetupPushMethodId = 1u;
} // namespace

const std::uint8_t CosemPushSetupObject::MaxSupportedVersion;

CosemPushSetupObject::CosemPushSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& pushObjectList,
  const CosemByteBuffer& sendDestinationAndMethod,
  const CosemByteBuffer& communicationWindow,
  const CosemByteBuffer& randomisationStartInterval,
  const CosemByteBuffer& numberOfRetries,
  const CosemByteBuffer& repetitionDelay,
  const CosemByteBuffer& portReference,
  const CosemByteBuffer& pushClientSap,
  const CosemByteBuffer& pushProtectionParameters,
  const CosemByteBuffer& pushOperationMethod,
  const CosemByteBuffer& confirmationParameters,
  const CosemByteBuffer& lastConfirmationDateTime,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPushSetupClassId,
      NormalizeVersion(version, CosemPushSetupObject::MaxSupportedVersion),
      logicalName))
  , pushObjectList_(pushObjectList)
  , sendDestinationAndMethod_(sendDestinationAndMethod)
  , communicationWindow_(communicationWindow)
  , randomisationStartInterval_(randomisationStartInterval)
  , numberOfRetries_(numberOfRetries)
  , repetitionDelay_(repetitionDelay)
  , portReference_(portReference)
  , pushClientSap_(pushClientSap)
  , pushProtectionParameters_(pushProtectionParameters)
  , pushOperationMethod_(pushOperationMethod)
  , confirmationParameters_(confirmationParameters)
  , lastConfirmationDateTime_(lastConfirmationDateTime)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kPushSetupObjectListAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPushSetupSendDestinationAndMethodAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPushSetupCommunicationWindowAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPushSetupRandomisationStartIntervalAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPushSetupNumberOfRetriesAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPushSetupRepetitionDelayAttributeId, mutableAccess);
  if (descriptor_.key.version >= 1u) {
    rights_.SetAttributeAccess(
      kPushSetupPortReferenceAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kPushSetupPushClientSapAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kPushSetupPushProtectionParametersAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kPushSetupPushOperationMethodAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kPushSetupConfirmationParametersAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kPushSetupLastConfirmationDateTimeAttributeId,
      AttributeAccessMode::ReadOnly);
  }
}

CosemObjectDescriptor CosemPushSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemPushSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemPushSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  const bool v1 = descriptor_.key.version >= 1u;
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPushSetupObjectListAttributeId:
      output = pushObjectList_;
      return CosemStatus::Ok;
    case kPushSetupSendDestinationAndMethodAttributeId:
      output = sendDestinationAndMethod_;
      return CosemStatus::Ok;
    case kPushSetupCommunicationWindowAttributeId:
      output = communicationWindow_;
      return CosemStatus::Ok;
    case kPushSetupRandomisationStartIntervalAttributeId:
      output = randomisationStartInterval_;
      return CosemStatus::Ok;
    case kPushSetupNumberOfRetriesAttributeId:
      output = numberOfRetries_;
      return CosemStatus::Ok;
    case kPushSetupRepetitionDelayAttributeId:
      output = repetitionDelay_;
      return CosemStatus::Ok;
    case kPushSetupPortReferenceAttributeId:
      if (!v1) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = portReference_;
      return CosemStatus::Ok;
    case kPushSetupPushClientSapAttributeId:
      if (!v1) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = pushClientSap_;
      return CosemStatus::Ok;
    case kPushSetupPushProtectionParametersAttributeId:
      if (!v1) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = pushProtectionParameters_;
      return CosemStatus::Ok;
    case kPushSetupPushOperationMethodAttributeId:
      if (!v1) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = pushOperationMethod_;
      return CosemStatus::Ok;
    case kPushSetupConfirmationParametersAttributeId:
      if (!v1) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = confirmationParameters_;
      return CosemStatus::Ok;
    case kPushSetupLastConfirmationDateTimeAttributeId:
      if (!v1) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = lastConfirmationDateTime_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPushSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  const bool v1 = descriptor_.key.version >= 1u;
  switch (attributeId) {
    case kPushSetupObjectListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      pushObjectList_ = input;
      return CosemStatus::Ok;
    case kPushSetupSendDestinationAndMethodAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      sendDestinationAndMethod_ = input;
      return CosemStatus::Ok;
    case kPushSetupCommunicationWindowAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      communicationWindow_ = input;
      return CosemStatus::Ok;
    case kPushSetupRandomisationStartIntervalAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      randomisationStartInterval_ = input;
      return CosemStatus::Ok;
    case kPushSetupNumberOfRetriesAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      numberOfRetries_ = input;
      return CosemStatus::Ok;
    case kPushSetupRepetitionDelayAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      repetitionDelay_ = input;
      return CosemStatus::Ok;
    case kPushSetupPortReferenceAttributeId:
      if (!v1) return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      portReference_ = input;
      return CosemStatus::Ok;
    case kPushSetupPushClientSapAttributeId:
      if (!v1) return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      pushClientSap_ = input;
      return CosemStatus::Ok;
    case kPushSetupPushProtectionParametersAttributeId:
      if (!v1) return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      pushProtectionParameters_ = input;
      return CosemStatus::Ok;
    case kPushSetupPushOperationMethodAttributeId:
      if (!v1) return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      pushOperationMethod_ = input;
      return CosemStatus::Ok;
    case kPushSetupConfirmationParametersAttributeId:
      if (!v1) return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      confirmationParameters_ = input;
      return CosemStatus::Ok;
    case kPushSetupLastConfirmationDateTimeAttributeId:
      if (!v1) return CosemStatus::AttributeNotFound;
      return CosemStatus::AccessDenied;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPushSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // Method 1 (push) triggers the server to emit a DataNotification to
  // the configured destination. The built-in object surfaces it as
  // UnsupportedFeature; a future push backend will own scheduling,
  // transport selection and confirmation tracking.
  if (methodId == kPushSetupPushMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemPushSetupObject::PushObjectList() const
{
  return pushObjectList_;
}

const CosemByteBuffer&
CosemPushSetupObject::SendDestinationAndMethod() const
{
  return sendDestinationAndMethod_;
}

const CosemByteBuffer& CosemPushSetupObject::CommunicationWindow() const
{
  return communicationWindow_;
}

const CosemByteBuffer&
CosemPushSetupObject::RandomisationStartInterval() const
{
  return randomisationStartInterval_;
}

const CosemByteBuffer& CosemPushSetupObject::NumberOfRetries() const
{
  return numberOfRetries_;
}

const CosemByteBuffer& CosemPushSetupObject::RepetitionDelay() const
{
  return repetitionDelay_;
}

const CosemByteBuffer& CosemPushSetupObject::PortReference() const
{
  return portReference_;
}

const CosemByteBuffer& CosemPushSetupObject::PushClientSap() const
{
  return pushClientSap_;
}

const CosemByteBuffer&
CosemPushSetupObject::PushProtectionParameters() const
{
  return pushProtectionParameters_;
}

const CosemByteBuffer& CosemPushSetupObject::PushOperationMethod() const
{
  return pushOperationMethod_;
}

const CosemByteBuffer& CosemPushSetupObject::ConfirmationParameters() const
{
  return confirmationParameters_;
}

const CosemByteBuffer&
CosemPushSetupObject::LastConfirmationDateTime() const
{
  return lastConfirmationDateTime_;
}

void CosemPushSetupObject::SetLastConfirmationDateTime(
  const CosemByteBuffer& value)
{
  lastConfirmationDateTime_ = value;
}

namespace {
constexpr std::uint16_t kDisconnectControlClassId = 70u;
constexpr std::uint8_t kDisconnectControlOutputStateAttributeId = 2u;
constexpr std::uint8_t kDisconnectControlControlStateAttributeId = 3u;
constexpr std::uint8_t kDisconnectControlControlModeAttributeId = 4u;
constexpr std::uint8_t kDisconnectControlRemoteDisconnectMethodId = 1u;
constexpr std::uint8_t kDisconnectControlRemoteReconnectMethodId = 2u;
} // namespace

const std::uint8_t CosemDisconnectControlObject::MaxSupportedVersion;

CosemDisconnectControlObject::CosemDisconnectControlObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& outputState,
  const CosemByteBuffer& controlState,
  const CosemByteBuffer& controlMode,
  AttributeAccessMode controlModeAccess)
  : CosemDisconnectControlObject(
      logicalName, outputState, controlState, controlMode,
      controlModeAccess, kVersion0)
{
}

CosemDisconnectControlObject::CosemDisconnectControlObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& outputState,
  const CosemByteBuffer& controlState,
  const CosemByteBuffer& controlMode,
  AttributeAccessMode controlModeAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kDisconnectControlClassId,
      NormalizeVersion(
        version,
        CosemDisconnectControlObject::MaxSupportedVersion),
      logicalName))
  , outputState_(outputState)
  , controlState_(controlState)
  , controlMode_(controlMode)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDisconnectControlOutputStateAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDisconnectControlControlStateAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDisconnectControlControlModeAttributeId, controlModeAccess);
}

CosemObjectDescriptor CosemDisconnectControlObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemDisconnectControlObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemDisconnectControlObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kDisconnectControlOutputStateAttributeId:
      output = outputState_;
      return CosemStatus::Ok;
    case kDisconnectControlControlStateAttributeId:
      output = controlState_;
      return CosemStatus::Ok;
    case kDisconnectControlControlModeAttributeId:
      output = controlMode_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemDisconnectControlObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kDisconnectControlControlModeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      controlMode_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
    case kDisconnectControlOutputStateAttributeId:
    case kDisconnectControlControlStateAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemDisconnectControlObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // Methods 1 (remote_disconnect) and 2 (remote_reconnect) drive the
  // load relay and update output_state / control_state per the
  // configured control_mode state machine. The built-in object surfaces
  // them as UnsupportedFeature; a future relay backend will own the
  // electrical switching and state transitions.
  if (methodId == kDisconnectControlRemoteDisconnectMethodId ||
      methodId == kDisconnectControlRemoteReconnectMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemDisconnectControlObject::OutputState() const
{
  return outputState_;
}

const CosemByteBuffer& CosemDisconnectControlObject::ControlState() const
{
  return controlState_;
}

const CosemByteBuffer& CosemDisconnectControlObject::ControlMode() const
{
  return controlMode_;
}

void CosemDisconnectControlObject::SetOutputState(
  const CosemByteBuffer& value)
{
  outputState_ = value;
}

void CosemDisconnectControlObject::SetControlState(
  const CosemByteBuffer& value)
{
  controlState_ = value;
}

namespace {
constexpr std::uint16_t kLimiterClassId = 71u;
constexpr std::uint8_t kLimiterMonitoredValueAttributeId = 2u;
constexpr std::uint8_t kLimiterThresholdActiveAttributeId = 3u;
constexpr std::uint8_t kLimiterThresholdNormalAttributeId = 4u;
constexpr std::uint8_t kLimiterThresholdEmergencyAttributeId = 5u;
constexpr std::uint8_t kLimiterMinOverThresholdDurationAttributeId = 6u;
constexpr std::uint8_t kLimiterMinUnderThresholdDurationAttributeId = 7u;
constexpr std::uint8_t kLimiterEmergencyProfileAttributeId = 8u;
constexpr std::uint8_t kLimiterEmergencyProfileGroupIdListAttributeId = 9u;
constexpr std::uint8_t kLimiterEmergencyProfileActiveAttributeId = 10u;
constexpr std::uint8_t kLimiterActionsAttributeId = 11u;
} // namespace

const std::uint8_t CosemLimiterObject::MaxSupportedVersion;

CosemLimiterObject::CosemLimiterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& monitoredValue,
  const CosemByteBuffer& thresholdActive,
  const CosemByteBuffer& thresholdNormal,
  const CosemByteBuffer& thresholdEmergency,
  const CosemByteBuffer& minOverThresholdDuration,
  const CosemByteBuffer& minUnderThresholdDuration,
  const CosemByteBuffer& emergencyProfile,
  const CosemByteBuffer& emergencyProfileGroupIdList,
  const CosemByteBuffer& emergencyProfileActive,
  const CosemByteBuffer& actions,
  AttributeAccessMode mutableAccess)
  : CosemLimiterObject(
      logicalName, monitoredValue, thresholdActive, thresholdNormal,
      thresholdEmergency, minOverThresholdDuration,
      minUnderThresholdDuration, emergencyProfile,
      emergencyProfileGroupIdList, emergencyProfileActive, actions,
      mutableAccess, kVersion0)
{
}

CosemLimiterObject::CosemLimiterObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& monitoredValue,
  const CosemByteBuffer& thresholdActive,
  const CosemByteBuffer& thresholdNormal,
  const CosemByteBuffer& thresholdEmergency,
  const CosemByteBuffer& minOverThresholdDuration,
  const CosemByteBuffer& minUnderThresholdDuration,
  const CosemByteBuffer& emergencyProfile,
  const CosemByteBuffer& emergencyProfileGroupIdList,
  const CosemByteBuffer& emergencyProfileActive,
  const CosemByteBuffer& actions,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kLimiterClassId,
      NormalizeVersion(version, CosemLimiterObject::MaxSupportedVersion),
      logicalName))
  , monitoredValue_(monitoredValue)
  , thresholdActive_(thresholdActive)
  , thresholdNormal_(thresholdNormal)
  , thresholdEmergency_(thresholdEmergency)
  , minOverThresholdDuration_(minOverThresholdDuration)
  , minUnderThresholdDuration_(minUnderThresholdDuration)
  , emergencyProfile_(emergencyProfile)
  , emergencyProfileGroupIdList_(emergencyProfileGroupIdList)
  , emergencyProfileActive_(emergencyProfileActive)
  , actions_(actions)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  // monitored_value identifies the source attribute and is part of the
  // limiter configuration; keep it read-only via the built-in object.
  rights_.SetAttributeAccess(
    kLimiterMonitoredValueAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kLimiterThresholdActiveAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterThresholdNormalAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterThresholdEmergencyAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterMinOverThresholdDurationAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterMinUnderThresholdDurationAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterEmergencyProfileAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterEmergencyProfileGroupIdListAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterEmergencyProfileActiveAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kLimiterActionsAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemLimiterObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemLimiterObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemLimiterObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kLimiterMonitoredValueAttributeId:
      output = monitoredValue_;
      return CosemStatus::Ok;
    case kLimiterThresholdActiveAttributeId:
      output = thresholdActive_;
      return CosemStatus::Ok;
    case kLimiterThresholdNormalAttributeId:
      output = thresholdNormal_;
      return CosemStatus::Ok;
    case kLimiterThresholdEmergencyAttributeId:
      output = thresholdEmergency_;
      return CosemStatus::Ok;
    case kLimiterMinOverThresholdDurationAttributeId:
      output = minOverThresholdDuration_;
      return CosemStatus::Ok;
    case kLimiterMinUnderThresholdDurationAttributeId:
      output = minUnderThresholdDuration_;
      return CosemStatus::Ok;
    case kLimiterEmergencyProfileAttributeId:
      output = emergencyProfile_;
      return CosemStatus::Ok;
    case kLimiterEmergencyProfileGroupIdListAttributeId:
      output = emergencyProfileGroupIdList_;
      return CosemStatus::Ok;
    case kLimiterEmergencyProfileActiveAttributeId:
      output = emergencyProfileActive_;
      return CosemStatus::Ok;
    case kLimiterActionsAttributeId:
      output = actions_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemLimiterObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kLimiterThresholdActiveAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      thresholdActive_ = input;
      return CosemStatus::Ok;
    case kLimiterThresholdNormalAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      thresholdNormal_ = input;
      return CosemStatus::Ok;
    case kLimiterThresholdEmergencyAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      thresholdEmergency_ = input;
      return CosemStatus::Ok;
    case kLimiterMinOverThresholdDurationAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      minOverThresholdDuration_ = input;
      return CosemStatus::Ok;
    case kLimiterMinUnderThresholdDurationAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      minUnderThresholdDuration_ = input;
      return CosemStatus::Ok;
    case kLimiterEmergencyProfileAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      emergencyProfile_ = input;
      return CosemStatus::Ok;
    case kLimiterEmergencyProfileGroupIdListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      emergencyProfileGroupIdList_ = input;
      return CosemStatus::Ok;
    case kLimiterEmergencyProfileActiveAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      emergencyProfileActive_ = input;
      return CosemStatus::Ok;
    case kLimiterActionsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      actions_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
    case kLimiterMonitoredValueAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemLimiterObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Limiter IC v0 defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemLimiterObject::MonitoredValue() const
{
  return monitoredValue_;
}

const CosemByteBuffer& CosemLimiterObject::ThresholdActive() const
{
  return thresholdActive_;
}

const CosemByteBuffer& CosemLimiterObject::ThresholdNormal() const
{
  return thresholdNormal_;
}

const CosemByteBuffer& CosemLimiterObject::ThresholdEmergency() const
{
  return thresholdEmergency_;
}

const CosemByteBuffer&
CosemLimiterObject::MinOverThresholdDuration() const
{
  return minOverThresholdDuration_;
}

const CosemByteBuffer&
CosemLimiterObject::MinUnderThresholdDuration() const
{
  return minUnderThresholdDuration_;
}

const CosemByteBuffer& CosemLimiterObject::EmergencyProfile() const
{
  return emergencyProfile_;
}

const CosemByteBuffer&
CosemLimiterObject::EmergencyProfileGroupIdList() const
{
  return emergencyProfileGroupIdList_;
}

const CosemByteBuffer& CosemLimiterObject::EmergencyProfileActive() const
{
  return emergencyProfileActive_;
}

const CosemByteBuffer& CosemLimiterObject::Actions() const
{
  return actions_;
}

void CosemLimiterObject::SetThresholdActive(const CosemByteBuffer& value)
{
  thresholdActive_ = value;
}

void CosemLimiterObject::SetEmergencyProfileActive(
  const CosemByteBuffer& value)
{
  emergencyProfileActive_ = value;
}

namespace {
constexpr std::uint16_t kIecHdlcSetupClassId = 23u;
constexpr std::uint8_t kIecHdlcSetupCommSpeedAttributeId = 2u;
constexpr std::uint8_t kIecHdlcSetupWindowSizeTransmitAttributeId = 3u;
constexpr std::uint8_t kIecHdlcSetupWindowSizeReceiveAttributeId = 4u;
constexpr std::uint8_t kIecHdlcSetupMaxInfoTxAttributeId = 5u;
constexpr std::uint8_t kIecHdlcSetupMaxInfoRxAttributeId = 6u;
constexpr std::uint8_t kIecHdlcSetupInterOctetTimeOutAttributeId = 7u;
constexpr std::uint8_t kIecHdlcSetupInactivityTimeOutAttributeId = 8u;
constexpr std::uint8_t kIecHdlcSetupDeviceAddressAttributeId = 9u;
} // namespace

const std::uint8_t CosemIecHdlcSetupObject::MaxSupportedVersion;

CosemIecHdlcSetupObject::CosemIecHdlcSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& commSpeed,
  const CosemByteBuffer& windowSizeTransmit,
  const CosemByteBuffer& windowSizeReceive,
  const CosemByteBuffer& maxInfoFieldLengthTransmit,
  const CosemByteBuffer& maxInfoFieldLengthReceive,
  const CosemByteBuffer& interOctetTimeOut,
  const CosemByteBuffer& inactivityTimeOut,
  const CosemByteBuffer& deviceAddress,
  AttributeAccessMode mutableAccess)
  : CosemIecHdlcSetupObject(
      logicalName, commSpeed, windowSizeTransmit, windowSizeReceive,
      maxInfoFieldLengthTransmit, maxInfoFieldLengthReceive,
      interOctetTimeOut, inactivityTimeOut, deviceAddress,
      mutableAccess, CosemIecHdlcSetupObject::MaxSupportedVersion)
{
}

CosemIecHdlcSetupObject::CosemIecHdlcSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& commSpeed,
  const CosemByteBuffer& windowSizeTransmit,
  const CosemByteBuffer& windowSizeReceive,
  const CosemByteBuffer& maxInfoFieldLengthTransmit,
  const CosemByteBuffer& maxInfoFieldLengthReceive,
  const CosemByteBuffer& interOctetTimeOut,
  const CosemByteBuffer& inactivityTimeOut,
  const CosemByteBuffer& deviceAddress,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIecHdlcSetupClassId,
      NormalizeVersion(
        version, CosemIecHdlcSetupObject::MaxSupportedVersion),
      logicalName))
  , commSpeed_(commSpeed)
  , windowSizeTransmit_(windowSizeTransmit)
  , windowSizeReceive_(windowSizeReceive)
  , maxInfoFieldLengthTransmit_(maxInfoFieldLengthTransmit)
  , maxInfoFieldLengthReceive_(maxInfoFieldLengthReceive)
  , interOctetTimeOut_(interOctetTimeOut)
  , inactivityTimeOut_(inactivityTimeOut)
  , deviceAddress_(deviceAddress)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIecHdlcSetupCommSpeedAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecHdlcSetupWindowSizeTransmitAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecHdlcSetupWindowSizeReceiveAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecHdlcSetupMaxInfoTxAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecHdlcSetupMaxInfoRxAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecHdlcSetupInterOctetTimeOutAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecHdlcSetupInactivityTimeOutAttributeId, mutableAccess);
  // device_address is the assigned HDLC address; treat it as read-only
  // and let the backend refresh it via SetDeviceAddress.
  rights_.SetAttributeAccess(
    kIecHdlcSetupDeviceAddressAttributeId, AttributeAccessMode::ReadOnly);
}

CosemObjectDescriptor CosemIecHdlcSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIecHdlcSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIecHdlcSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIecHdlcSetupCommSpeedAttributeId:
      output = commSpeed_;
      return CosemStatus::Ok;
    case kIecHdlcSetupWindowSizeTransmitAttributeId:
      output = windowSizeTransmit_;
      return CosemStatus::Ok;
    case kIecHdlcSetupWindowSizeReceiveAttributeId:
      output = windowSizeReceive_;
      return CosemStatus::Ok;
    case kIecHdlcSetupMaxInfoTxAttributeId:
      output = maxInfoFieldLengthTransmit_;
      return CosemStatus::Ok;
    case kIecHdlcSetupMaxInfoRxAttributeId:
      output = maxInfoFieldLengthReceive_;
      return CosemStatus::Ok;
    case kIecHdlcSetupInterOctetTimeOutAttributeId:
      output = interOctetTimeOut_;
      return CosemStatus::Ok;
    case kIecHdlcSetupInactivityTimeOutAttributeId:
      output = inactivityTimeOut_;
      return CosemStatus::Ok;
    case kIecHdlcSetupDeviceAddressAttributeId:
      output = deviceAddress_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIecHdlcSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIecHdlcSetupCommSpeedAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      commSpeed_ = input;
      return CosemStatus::Ok;
    case kIecHdlcSetupWindowSizeTransmitAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      windowSizeTransmit_ = input;
      return CosemStatus::Ok;
    case kIecHdlcSetupWindowSizeReceiveAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      windowSizeReceive_ = input;
      return CosemStatus::Ok;
    case kIecHdlcSetupMaxInfoTxAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      maxInfoFieldLengthTransmit_ = input;
      return CosemStatus::Ok;
    case kIecHdlcSetupMaxInfoRxAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      maxInfoFieldLengthReceive_ = input;
      return CosemStatus::Ok;
    case kIecHdlcSetupInterOctetTimeOutAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      interOctetTimeOut_ = input;
      return CosemStatus::Ok;
    case kIecHdlcSetupInactivityTimeOutAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      inactivityTimeOut_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
    case kIecHdlcSetupDeviceAddressAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIecHdlcSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // IEC HDLC Setup IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemIecHdlcSetupObject::CommSpeed() const
{
  return commSpeed_;
}

const CosemByteBuffer& CosemIecHdlcSetupObject::WindowSizeTransmit() const
{
  return windowSizeTransmit_;
}

const CosemByteBuffer& CosemIecHdlcSetupObject::WindowSizeReceive() const
{
  return windowSizeReceive_;
}

const CosemByteBuffer&
CosemIecHdlcSetupObject::MaxInfoFieldLengthTransmit() const
{
  return maxInfoFieldLengthTransmit_;
}

const CosemByteBuffer&
CosemIecHdlcSetupObject::MaxInfoFieldLengthReceive() const
{
  return maxInfoFieldLengthReceive_;
}

const CosemByteBuffer& CosemIecHdlcSetupObject::InterOctetTimeOut() const
{
  return interOctetTimeOut_;
}

const CosemByteBuffer& CosemIecHdlcSetupObject::InactivityTimeOut() const
{
  return inactivityTimeOut_;
}

const CosemByteBuffer& CosemIecHdlcSetupObject::DeviceAddress() const
{
  return deviceAddress_;
}

void CosemIecHdlcSetupObject::SetDeviceAddress(const CosemByteBuffer& value)
{
  deviceAddress_ = value;
}

namespace {
constexpr std::uint16_t kRegisterTableClassId = 61u;
constexpr std::uint8_t kRegisterTableTableCellValuesAttributeId = 2u;
constexpr std::uint8_t kRegisterTableSingleBufferAttributeId = 3u;
constexpr std::uint8_t kRegisterTableTableCellDefinitionAttributeId = 4u;
constexpr std::uint8_t kRegisterTableTableEntriesAttributeId = 5u;
constexpr std::uint8_t kRegisterTableTableEntryMethodId = 1u;
constexpr std::uint8_t kRegisterTableUpdateTableEntryMethodId = 2u;
} // namespace

const std::uint8_t CosemRegisterTableObject::MaxSupportedVersion;

CosemRegisterTableObject::CosemRegisterTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tableCellValues,
  const CosemByteBuffer& singleBuffer,
  const CosemByteBuffer& tableCellDefinition,
  const CosemByteBuffer& tableEntries,
  AttributeAccessMode mutableAccess)
  : CosemRegisterTableObject(
      logicalName, tableCellValues, singleBuffer, tableCellDefinition,
      tableEntries, mutableAccess, kVersion0)
{
}

CosemRegisterTableObject::CosemRegisterTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tableCellValues,
  const CosemByteBuffer& singleBuffer,
  const CosemByteBuffer& tableCellDefinition,
  const CosemByteBuffer& tableEntries,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kRegisterTableClassId,
      NormalizeVersion(
        version, CosemRegisterTableObject::MaxSupportedVersion),
      logicalName))
  , tableCellValues_(tableCellValues)
  , singleBuffer_(singleBuffer)
  , tableCellDefinition_(tableCellDefinition)
  , tableEntries_(tableEntries)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  // table_cell_values is the measured/computed data; treat as RO and
  // let the backend refresh it via SetTableCellValues.
  rights_.SetAttributeAccess(
    kRegisterTableTableCellValuesAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kRegisterTableSingleBufferAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kRegisterTableTableCellDefinitionAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kRegisterTableTableEntriesAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemRegisterTableObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemRegisterTableObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemRegisterTableObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kRegisterTableTableCellValuesAttributeId:
      output = tableCellValues_;
      return CosemStatus::Ok;
    case kRegisterTableSingleBufferAttributeId:
      output = singleBuffer_;
      return CosemStatus::Ok;
    case kRegisterTableTableCellDefinitionAttributeId:
      output = tableCellDefinition_;
      return CosemStatus::Ok;
    case kRegisterTableTableEntriesAttributeId:
      output = tableEntries_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemRegisterTableObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kRegisterTableSingleBufferAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      singleBuffer_ = input;
      return CosemStatus::Ok;
    case kRegisterTableTableCellDefinitionAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      tableCellDefinition_ = input;
      return CosemStatus::Ok;
    case kRegisterTableTableEntriesAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      tableEntries_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
    case kRegisterTableTableCellValuesAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemRegisterTableObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  switch (methodId) {
    case kRegisterTableTableEntryMethodId:
    case kRegisterTableUpdateTableEntryMethodId:
      // Application-defined column selection / update.
      return CosemStatus::UnsupportedFeature;
    default:
      return CosemStatus::MethodNotFound;
  }
}

const CosemByteBuffer& CosemRegisterTableObject::TableCellValues() const
{
  return tableCellValues_;
}

const CosemByteBuffer& CosemRegisterTableObject::SingleBuffer() const
{
  return singleBuffer_;
}

const CosemByteBuffer&
CosemRegisterTableObject::TableCellDefinition() const
{
  return tableCellDefinition_;
}

const CosemByteBuffer& CosemRegisterTableObject::TableEntries() const
{
  return tableEntries_;
}

void CosemRegisterTableObject::SetTableCellValues(
  const CosemByteBuffer& value)
{
  tableCellValues_ = value;
}

namespace {
constexpr std::uint16_t kTcpUdpSetupClassId = 41u;
constexpr std::uint8_t kTcpUdpSetupTcpUdpPortAttributeId = 2u;
constexpr std::uint8_t kTcpUdpSetupIpReferenceAttributeId = 3u;
constexpr std::uint8_t kTcpUdpSetupMssAttributeId = 4u;
constexpr std::uint8_t kTcpUdpSetupNbOfSimConnAttributeId = 5u;
constexpr std::uint8_t kTcpUdpSetupInactivityTimeOutAttributeId = 6u;
} // namespace

const std::uint8_t CosemTcpUdpSetupObject::MaxSupportedVersion;

CosemTcpUdpSetupObject::CosemTcpUdpSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tcpUdpPort,
  const CosemByteBuffer& ipReference,
  const CosemByteBuffer& mss,
  const CosemByteBuffer& nbOfSimConn,
  const CosemByteBuffer& inactivityTimeOut,
  AttributeAccessMode mutableAccess)
  : CosemTcpUdpSetupObject(
      logicalName, tcpUdpPort, ipReference, mss, nbOfSimConn,
      inactivityTimeOut, mutableAccess, kVersion0)
{
}

CosemTcpUdpSetupObject::CosemTcpUdpSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tcpUdpPort,
  const CosemByteBuffer& ipReference,
  const CosemByteBuffer& mss,
  const CosemByteBuffer& nbOfSimConn,
  const CosemByteBuffer& inactivityTimeOut,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kTcpUdpSetupClassId,
      NormalizeVersion(
        version, CosemTcpUdpSetupObject::MaxSupportedVersion),
      logicalName))
  , tcpUdpPort_(tcpUdpPort)
  , ipReference_(ipReference)
  , mss_(mss)
  , nbOfSimConn_(nbOfSimConn)
  , inactivityTimeOut_(inactivityTimeOut)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kTcpUdpSetupTcpUdpPortAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kTcpUdpSetupIpReferenceAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kTcpUdpSetupMssAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kTcpUdpSetupNbOfSimConnAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kTcpUdpSetupInactivityTimeOutAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemTcpUdpSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemTcpUdpSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemTcpUdpSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kTcpUdpSetupTcpUdpPortAttributeId:
      output = tcpUdpPort_;
      return CosemStatus::Ok;
    case kTcpUdpSetupIpReferenceAttributeId:
      output = ipReference_;
      return CosemStatus::Ok;
    case kTcpUdpSetupMssAttributeId:
      output = mss_;
      return CosemStatus::Ok;
    case kTcpUdpSetupNbOfSimConnAttributeId:
      output = nbOfSimConn_;
      return CosemStatus::Ok;
    case kTcpUdpSetupInactivityTimeOutAttributeId:
      output = inactivityTimeOut_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemTcpUdpSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kTcpUdpSetupTcpUdpPortAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      tcpUdpPort_ = input;
      return CosemStatus::Ok;
    case kTcpUdpSetupIpReferenceAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      ipReference_ = input;
      return CosemStatus::Ok;
    case kTcpUdpSetupMssAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      mss_ = input;
      return CosemStatus::Ok;
    case kTcpUdpSetupNbOfSimConnAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      nbOfSimConn_ = input;
      return CosemStatus::Ok;
    case kTcpUdpSetupInactivityTimeOutAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      inactivityTimeOut_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemTcpUdpSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // TCP-UDP Setup IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemTcpUdpSetupObject::TcpUdpPort() const
{
  return tcpUdpPort_;
}

const CosemByteBuffer& CosemTcpUdpSetupObject::IpReference() const
{
  return ipReference_;
}

const CosemByteBuffer& CosemTcpUdpSetupObject::Mss() const
{
  return mss_;
}

const CosemByteBuffer& CosemTcpUdpSetupObject::NbOfSimConn() const
{
  return nbOfSimConn_;
}

const CosemByteBuffer& CosemTcpUdpSetupObject::InactivityTimeOut() const
{
  return inactivityTimeOut_;
}

namespace {
constexpr std::uint16_t kScheduleClassId = 10u;
constexpr std::uint8_t kScheduleEntriesAttributeId = 2u;
constexpr std::uint8_t kScheduleInsertMethodId = 1u;
constexpr std::uint8_t kScheduleDeleteMethodId = 2u;
} // namespace

const std::uint8_t CosemScheduleObject::MaxSupportedVersion;

CosemScheduleObject::CosemScheduleObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& entries,
  AttributeAccessMode entriesAccess)
  : CosemScheduleObject(logicalName, entries, entriesAccess, kVersion0)
{
}

CosemScheduleObject::CosemScheduleObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& entries,
  AttributeAccessMode entriesAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kScheduleClassId,
      NormalizeVersion(
        version, CosemScheduleObject::MaxSupportedVersion),
      logicalName))
  , entries_(entries)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kScheduleEntriesAttributeId, entriesAccess);
}

CosemObjectDescriptor CosemScheduleObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemScheduleObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemScheduleObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kScheduleEntriesAttributeId:
      output = entries_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemScheduleObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kScheduleEntriesAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      entries_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemScheduleObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  switch (methodId) {
    case kScheduleInsertMethodId:
    case kScheduleDeleteMethodId:
      // Application-defined schedule entry mutation.
      return CosemStatus::UnsupportedFeature;
    default:
      return CosemStatus::MethodNotFound;
  }
}

const CosemByteBuffer& CosemScheduleObject::Entries() const
{
  return entries_;
}

void CosemScheduleObject::SetEntries(const CosemByteBuffer& value)
{
  entries_ = value;
}

namespace {
constexpr std::uint16_t kSpecialDaysTableClassId = 11u;
constexpr std::uint8_t kSpecialDaysTableEntriesAttributeId = 2u;
constexpr std::uint8_t kSpecialDaysTableInsertMethodId = 1u;
constexpr std::uint8_t kSpecialDaysTableDeleteMethodId = 2u;
} // namespace

const std::uint8_t CosemSpecialDaysTableObject::MaxSupportedVersion;

CosemSpecialDaysTableObject::CosemSpecialDaysTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& entries,
  AttributeAccessMode entriesAccess)
  : CosemSpecialDaysTableObject(
      logicalName, entries, entriesAccess, kVersion0)
{
}

CosemSpecialDaysTableObject::CosemSpecialDaysTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& entries,
  AttributeAccessMode entriesAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSpecialDaysTableClassId,
      NormalizeVersion(
        version, CosemSpecialDaysTableObject::MaxSupportedVersion),
      logicalName))
  , entries_(entries)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSpecialDaysTableEntriesAttributeId, entriesAccess);
}

CosemObjectDescriptor CosemSpecialDaysTableObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSpecialDaysTableObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSpecialDaysTableObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSpecialDaysTableEntriesAttributeId:
      output = entries_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSpecialDaysTableObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSpecialDaysTableEntriesAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      entries_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSpecialDaysTableObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  switch (methodId) {
    case kSpecialDaysTableInsertMethodId:
    case kSpecialDaysTableDeleteMethodId:
      // Application-defined special-day entry mutation.
      return CosemStatus::UnsupportedFeature;
    default:
      return CosemStatus::MethodNotFound;
  }
}

const CosemByteBuffer& CosemSpecialDaysTableObject::Entries() const
{
  return entries_;
}

void CosemSpecialDaysTableObject::SetEntries(const CosemByteBuffer& value)
{
  entries_ = value;
}

namespace {
constexpr std::uint16_t kSingleActionScheduleClassId = 22u;
constexpr std::uint8_t kSingleActionScheduleExecutedScriptAttributeId = 2u;
constexpr std::uint8_t kSingleActionScheduleTypeAttributeId = 3u;
constexpr std::uint8_t kSingleActionScheduleExecutionTimeAttributeId = 4u;
} // namespace

const std::uint8_t CosemSingleActionScheduleObject::MaxSupportedVersion;

CosemSingleActionScheduleObject::CosemSingleActionScheduleObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& executedScript,
  const CosemByteBuffer& type,
  const CosemByteBuffer& executionTime,
  AttributeAccessMode mutableAccess)
  : CosemSingleActionScheduleObject(
      logicalName, executedScript, type, executionTime,
      mutableAccess, kVersion0)
{
}

CosemSingleActionScheduleObject::CosemSingleActionScheduleObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& executedScript,
  const CosemByteBuffer& type,
  const CosemByteBuffer& executionTime,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSingleActionScheduleClassId,
      NormalizeVersion(
        version, CosemSingleActionScheduleObject::MaxSupportedVersion),
      logicalName))
  , executedScript_(executedScript)
  , type_(type)
  , executionTime_(executionTime)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSingleActionScheduleExecutedScriptAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSingleActionScheduleTypeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSingleActionScheduleExecutionTimeAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemSingleActionScheduleObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSingleActionScheduleObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSingleActionScheduleObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSingleActionScheduleExecutedScriptAttributeId:
      output = executedScript_;
      return CosemStatus::Ok;
    case kSingleActionScheduleTypeAttributeId:
      output = type_;
      return CosemStatus::Ok;
    case kSingleActionScheduleExecutionTimeAttributeId:
      output = executionTime_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSingleActionScheduleObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSingleActionScheduleExecutedScriptAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      executedScript_ = input;
      return CosemStatus::Ok;
    case kSingleActionScheduleTypeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      type_ = input;
      return CosemStatus::Ok;
    case kSingleActionScheduleExecutionTimeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      executionTime_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSingleActionScheduleObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Single Action Schedule IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemSingleActionScheduleObject::ExecutedScript() const
{
  return executedScript_;
}

const CosemByteBuffer& CosemSingleActionScheduleObject::Type() const
{
  return type_;
}

const CosemByteBuffer& CosemSingleActionScheduleObject::ExecutionTime() const
{
  return executionTime_;
}

namespace {
constexpr std::uint16_t kModemConfigurationClassId = 27u;
constexpr std::uint8_t kModemConfigurationCommunicationSpeedAttributeId = 2u;
constexpr std::uint8_t kModemConfigurationInitialisationStringsAttributeId = 3u;
constexpr std::uint8_t kModemConfigurationModemProfileAttributeId = 4u;
} // namespace

const std::uint8_t CosemModemConfigurationObject::MaxSupportedVersion;

CosemModemConfigurationObject::CosemModemConfigurationObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& communicationSpeed,
  const CosemByteBuffer& initialisationStrings,
  const CosemByteBuffer& modemProfile,
  AttributeAccessMode mutableAccess)
  : CosemModemConfigurationObject(
      logicalName, communicationSpeed, initialisationStrings,
      modemProfile, mutableAccess,
      CosemModemConfigurationObject::MaxSupportedVersion)
{
}

CosemModemConfigurationObject::CosemModemConfigurationObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& communicationSpeed,
  const CosemByteBuffer& initialisationStrings,
  const CosemByteBuffer& modemProfile,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kModemConfigurationClassId,
      NormalizeVersion(
        version, CosemModemConfigurationObject::MaxSupportedVersion),
      logicalName))
  , communicationSpeed_(communicationSpeed)
  , initialisationStrings_(initialisationStrings)
  , modemProfile_(modemProfile)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kModemConfigurationCommunicationSpeedAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kModemConfigurationInitialisationStringsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kModemConfigurationModemProfileAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemModemConfigurationObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemModemConfigurationObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemModemConfigurationObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kModemConfigurationCommunicationSpeedAttributeId:
      output = communicationSpeed_;
      return CosemStatus::Ok;
    case kModemConfigurationInitialisationStringsAttributeId:
      output = initialisationStrings_;
      return CosemStatus::Ok;
    case kModemConfigurationModemProfileAttributeId:
      output = modemProfile_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemModemConfigurationObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kModemConfigurationCommunicationSpeedAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      communicationSpeed_ = input;
      return CosemStatus::Ok;
    case kModemConfigurationInitialisationStringsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      initialisationStrings_ = input;
      return CosemStatus::Ok;
    case kModemConfigurationModemProfileAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      modemProfile_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemModemConfigurationObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Modem Configuration IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemModemConfigurationObject::CommunicationSpeed() const
{
  return communicationSpeed_;
}

const CosemByteBuffer&
CosemModemConfigurationObject::InitialisationStrings() const
{
  return initialisationStrings_;
}

const CosemByteBuffer& CosemModemConfigurationObject::ModemProfile() const
{
  return modemProfile_;
}

namespace {
constexpr std::uint16_t kAutoConnectClassId = 29u;
constexpr std::uint8_t kAutoConnectModeAttributeId = 2u;
constexpr std::uint8_t kAutoConnectRepetitionsAttributeId = 3u;
constexpr std::uint8_t kAutoConnectRepetitionDelayAttributeId = 4u;
constexpr std::uint8_t kAutoConnectCallingWindowAttributeId = 5u;
constexpr std::uint8_t kAutoConnectDestinationListAttributeId = 6u;
} // namespace

const std::uint8_t CosemAutoConnectObject::MaxSupportedVersion;

CosemAutoConnectObject::CosemAutoConnectObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& mode,
  const CosemByteBuffer& repetitions,
  const CosemByteBuffer& repetitionDelay,
  const CosemByteBuffer& callingWindow,
  const CosemByteBuffer& destinationList,
  AttributeAccessMode mutableAccess)
  : CosemAutoConnectObject(
      logicalName, mode, repetitions, repetitionDelay,
      callingWindow, destinationList, mutableAccess, kVersion0)
{
}

CosemAutoConnectObject::CosemAutoConnectObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& mode,
  const CosemByteBuffer& repetitions,
  const CosemByteBuffer& repetitionDelay,
  const CosemByteBuffer& callingWindow,
  const CosemByteBuffer& destinationList,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kAutoConnectClassId,
      NormalizeVersion(
        version, CosemAutoConnectObject::MaxSupportedVersion),
      logicalName))
  , mode_(mode)
  , repetitions_(repetitions)
  , repetitionDelay_(repetitionDelay)
  , callingWindow_(callingWindow)
  , destinationList_(destinationList)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kAutoConnectModeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAutoConnectRepetitionsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAutoConnectRepetitionDelayAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAutoConnectCallingWindowAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAutoConnectDestinationListAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemAutoConnectObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemAutoConnectObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemAutoConnectObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kAutoConnectModeAttributeId:
      output = mode_;
      return CosemStatus::Ok;
    case kAutoConnectRepetitionsAttributeId:
      output = repetitions_;
      return CosemStatus::Ok;
    case kAutoConnectRepetitionDelayAttributeId:
      output = repetitionDelay_;
      return CosemStatus::Ok;
    case kAutoConnectCallingWindowAttributeId:
      output = callingWindow_;
      return CosemStatus::Ok;
    case kAutoConnectDestinationListAttributeId:
      output = destinationList_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAutoConnectObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kAutoConnectModeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      mode_ = input;
      return CosemStatus::Ok;
    case kAutoConnectRepetitionsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      repetitions_ = input;
      return CosemStatus::Ok;
    case kAutoConnectRepetitionDelayAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      repetitionDelay_ = input;
      return CosemStatus::Ok;
    case kAutoConnectCallingWindowAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      callingWindow_ = input;
      return CosemStatus::Ok;
    case kAutoConnectDestinationListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      destinationList_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAutoConnectObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Auto Connect IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemAutoConnectObject::Mode() const
{
  return mode_;
}

const CosemByteBuffer& CosemAutoConnectObject::Repetitions() const
{
  return repetitions_;
}

const CosemByteBuffer& CosemAutoConnectObject::RepetitionDelay() const
{
  return repetitionDelay_;
}

const CosemByteBuffer& CosemAutoConnectObject::CallingWindow() const
{
  return callingWindow_;
}

const CosemByteBuffer& CosemAutoConnectObject::DestinationList() const
{
  return destinationList_;
}

namespace {
constexpr std::uint16_t kGprsModemSetupClassId = 45u;
constexpr std::uint8_t kGprsModemSetupApnAttributeId = 2u;
constexpr std::uint8_t kGprsModemSetupPinCodeAttributeId = 3u;
constexpr std::uint8_t kGprsModemSetupQualityOfServiceAttributeId = 4u;
} // namespace

const std::uint8_t CosemGprsModemSetupObject::MaxSupportedVersion;

CosemGprsModemSetupObject::CosemGprsModemSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& apn,
  const CosemByteBuffer& pinCode,
  const CosemByteBuffer& qualityOfService,
  AttributeAccessMode mutableAccess)
  : CosemGprsModemSetupObject(
      logicalName, apn, pinCode, qualityOfService, mutableAccess, kVersion0)
{
}

CosemGprsModemSetupObject::CosemGprsModemSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& apn,
  const CosemByteBuffer& pinCode,
  const CosemByteBuffer& qualityOfService,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kGprsModemSetupClassId,
      NormalizeVersion(
        version, CosemGprsModemSetupObject::MaxSupportedVersion),
      logicalName))
  , apn_(apn)
  , pinCode_(pinCode)
  , qualityOfService_(qualityOfService)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kGprsModemSetupApnAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGprsModemSetupPinCodeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGprsModemSetupQualityOfServiceAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemGprsModemSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemGprsModemSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemGprsModemSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kGprsModemSetupApnAttributeId:
      output = apn_;
      return CosemStatus::Ok;
    case kGprsModemSetupPinCodeAttributeId:
      output = pinCode_;
      return CosemStatus::Ok;
    case kGprsModemSetupQualityOfServiceAttributeId:
      output = qualityOfService_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemGprsModemSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kGprsModemSetupApnAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      apn_ = input;
      return CosemStatus::Ok;
    case kGprsModemSetupPinCodeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      pinCode_ = input;
      return CosemStatus::Ok;
    case kGprsModemSetupQualityOfServiceAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      qualityOfService_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemGprsModemSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // GPRS Modem Setup IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemGprsModemSetupObject::Apn() const
{
  return apn_;
}

const CosemByteBuffer& CosemGprsModemSetupObject::PinCode() const
{
  return pinCode_;
}

const CosemByteBuffer& CosemGprsModemSetupObject::QualityOfService() const
{
  return qualityOfService_;
}

namespace {
constexpr std::uint16_t kAutoAnswerClassId = 28u;
constexpr std::uint8_t kAutoAnswerModeAttributeId = 2u;
constexpr std::uint8_t kAutoAnswerListeningWindowAttributeId = 3u;
constexpr std::uint8_t kAutoAnswerStatusAttributeId = 4u;
constexpr std::uint8_t kAutoAnswerNumberOfCallsAttributeId = 5u;
constexpr std::uint8_t kAutoAnswerNumberOfRingsAttributeId = 6u;
} // namespace

const std::uint8_t CosemAutoAnswerObject::MaxSupportedVersion;

CosemAutoAnswerObject::CosemAutoAnswerObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& mode,
  const CosemByteBuffer& listeningWindow,
  const CosemByteBuffer& status,
  const CosemByteBuffer& numberOfCalls,
  const CosemByteBuffer& numberOfRings,
  AttributeAccessMode mutableAccess)
  : CosemAutoAnswerObject(
      logicalName, mode, listeningWindow, status, numberOfCalls,
      numberOfRings, mutableAccess, kVersion0)
{
}

CosemAutoAnswerObject::CosemAutoAnswerObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& mode,
  const CosemByteBuffer& listeningWindow,
  const CosemByteBuffer& status,
  const CosemByteBuffer& numberOfCalls,
  const CosemByteBuffer& numberOfRings,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kAutoAnswerClassId,
      NormalizeVersion(
        version, CosemAutoAnswerObject::MaxSupportedVersion),
      logicalName))
  , mode_(mode)
  , listeningWindow_(listeningWindow)
  , status_(status)
  , numberOfCalls_(numberOfCalls)
  , numberOfRings_(numberOfRings)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kAutoAnswerModeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAutoAnswerListeningWindowAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAutoAnswerStatusAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kAutoAnswerNumberOfCallsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAutoAnswerNumberOfRingsAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemAutoAnswerObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemAutoAnswerObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemAutoAnswerObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kAutoAnswerModeAttributeId:
      output = mode_;
      return CosemStatus::Ok;
    case kAutoAnswerListeningWindowAttributeId:
      output = listeningWindow_;
      return CosemStatus::Ok;
    case kAutoAnswerStatusAttributeId:
      output = status_;
      return CosemStatus::Ok;
    case kAutoAnswerNumberOfCallsAttributeId:
      output = numberOfCalls_;
      return CosemStatus::Ok;
    case kAutoAnswerNumberOfRingsAttributeId:
      output = numberOfRings_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAutoAnswerObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kAutoAnswerModeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      mode_ = input;
      return CosemStatus::Ok;
    case kAutoAnswerListeningWindowAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      listeningWindow_ = input;
      return CosemStatus::Ok;
    case kAutoAnswerNumberOfCallsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      numberOfCalls_ = input;
      return CosemStatus::Ok;
    case kAutoAnswerNumberOfRingsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      numberOfRings_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
    case kAutoAnswerStatusAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAutoAnswerObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Auto Answer IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemAutoAnswerObject::Mode() const
{
  return mode_;
}

const CosemByteBuffer& CosemAutoAnswerObject::ListeningWindow() const
{
  return listeningWindow_;
}

const CosemByteBuffer& CosemAutoAnswerObject::Status() const
{
  return status_;
}

const CosemByteBuffer& CosemAutoAnswerObject::NumberOfCalls() const
{
  return numberOfCalls_;
}

const CosemByteBuffer& CosemAutoAnswerObject::NumberOfRings() const
{
  return numberOfRings_;
}

void CosemAutoAnswerObject::SetStatus(const CosemByteBuffer& status)
{
  status_ = status;
}

namespace {
constexpr std::uint16_t kIpv4SetupClassId = 42u;
constexpr std::uint8_t kIpv4SetupDlReferenceAttributeId = 2u;
constexpr std::uint8_t kIpv4SetupIpAddressAttributeId = 3u;
constexpr std::uint8_t kIpv4SetupMulticastIpAddressAttributeId = 4u;
constexpr std::uint8_t kIpv4SetupIpOptionsAttributeId = 5u;
constexpr std::uint8_t kIpv4SetupSubnetMaskAttributeId = 6u;
constexpr std::uint8_t kIpv4SetupGatewayIpAddressAttributeId = 7u;
constexpr std::uint8_t kIpv4SetupUseDhcpFlagAttributeId = 8u;
constexpr std::uint8_t kIpv4SetupPrimaryDnsAddressAttributeId = 9u;
constexpr std::uint8_t kIpv4SetupSecondaryDnsAddressAttributeId = 10u;
constexpr std::uint8_t kIpv4SetupAddMcIpAddressMethodId = 1u;
constexpr std::uint8_t kIpv4SetupDeleteMcIpAddressMethodId = 2u;
} // namespace

const std::uint8_t CosemIpv4SetupObject::MaxSupportedVersion;

CosemIpv4SetupObject::CosemIpv4SetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& dlReference,
  const CosemByteBuffer& ipAddress,
  const CosemByteBuffer& multicastIpAddress,
  const CosemByteBuffer& ipOptions,
  const CosemByteBuffer& subnetMask,
  const CosemByteBuffer& gatewayIpAddress,
  const CosemByteBuffer& useDhcpFlag,
  const CosemByteBuffer& primaryDnsAddress,
  const CosemByteBuffer& secondaryDnsAddress,
  AttributeAccessMode mutableAccess)
  : CosemIpv4SetupObject(
      logicalName, dlReference, ipAddress, multicastIpAddress,
      ipOptions, subnetMask, gatewayIpAddress, useDhcpFlag,
      primaryDnsAddress, secondaryDnsAddress, mutableAccess, kVersion0)
{
}

CosemIpv4SetupObject::CosemIpv4SetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& dlReference,
  const CosemByteBuffer& ipAddress,
  const CosemByteBuffer& multicastIpAddress,
  const CosemByteBuffer& ipOptions,
  const CosemByteBuffer& subnetMask,
  const CosemByteBuffer& gatewayIpAddress,
  const CosemByteBuffer& useDhcpFlag,
  const CosemByteBuffer& primaryDnsAddress,
  const CosemByteBuffer& secondaryDnsAddress,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIpv4SetupClassId,
      NormalizeVersion(
        version, CosemIpv4SetupObject::MaxSupportedVersion),
      logicalName))
  , dlReference_(dlReference)
  , ipAddress_(ipAddress)
  , multicastIpAddress_(multicastIpAddress)
  , ipOptions_(ipOptions)
  , subnetMask_(subnetMask)
  , gatewayIpAddress_(gatewayIpAddress)
  , useDhcpFlag_(useDhcpFlag)
  , primaryDnsAddress_(primaryDnsAddress)
  , secondaryDnsAddress_(secondaryDnsAddress)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIpv4SetupDlReferenceAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupIpAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupMulticastIpAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupIpOptionsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupSubnetMaskAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupGatewayIpAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupUseDhcpFlagAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupPrimaryDnsAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv4SetupSecondaryDnsAddressAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemIpv4SetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIpv4SetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIpv4SetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIpv4SetupDlReferenceAttributeId:
      output = dlReference_;
      return CosemStatus::Ok;
    case kIpv4SetupIpAddressAttributeId:
      output = ipAddress_;
      return CosemStatus::Ok;
    case kIpv4SetupMulticastIpAddressAttributeId:
      output = multicastIpAddress_;
      return CosemStatus::Ok;
    case kIpv4SetupIpOptionsAttributeId:
      output = ipOptions_;
      return CosemStatus::Ok;
    case kIpv4SetupSubnetMaskAttributeId:
      output = subnetMask_;
      return CosemStatus::Ok;
    case kIpv4SetupGatewayIpAddressAttributeId:
      output = gatewayIpAddress_;
      return CosemStatus::Ok;
    case kIpv4SetupUseDhcpFlagAttributeId:
      output = useDhcpFlag_;
      return CosemStatus::Ok;
    case kIpv4SetupPrimaryDnsAddressAttributeId:
      output = primaryDnsAddress_;
      return CosemStatus::Ok;
    case kIpv4SetupSecondaryDnsAddressAttributeId:
      output = secondaryDnsAddress_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIpv4SetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIpv4SetupDlReferenceAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      dlReference_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupIpAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      ipAddress_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupMulticastIpAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      multicastIpAddress_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupIpOptionsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      ipOptions_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupSubnetMaskAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      subnetMask_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupGatewayIpAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      gatewayIpAddress_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupUseDhcpFlagAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      useDhcpFlag_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupPrimaryDnsAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      primaryDnsAddress_ = input;
      return CosemStatus::Ok;
    case kIpv4SetupSecondaryDnsAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      secondaryDnsAddress_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIpv4SetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  switch (methodId) {
    case kIpv4SetupAddMcIpAddressMethodId:
    case kIpv4SetupDeleteMcIpAddressMethodId:
      // Built-in object does not own the multicast subscription policy.
      return CosemStatus::UnsupportedFeature;
    default:
      return CosemStatus::MethodNotFound;
  }
}

const CosemByteBuffer& CosemIpv4SetupObject::DlReference() const
{
  return dlReference_;
}

const CosemByteBuffer& CosemIpv4SetupObject::IpAddress() const
{
  return ipAddress_;
}

const CosemByteBuffer& CosemIpv4SetupObject::MulticastIpAddress() const
{
  return multicastIpAddress_;
}

const CosemByteBuffer& CosemIpv4SetupObject::IpOptions() const
{
  return ipOptions_;
}

const CosemByteBuffer& CosemIpv4SetupObject::SubnetMask() const
{
  return subnetMask_;
}

const CosemByteBuffer& CosemIpv4SetupObject::GatewayIpAddress() const
{
  return gatewayIpAddress_;
}

const CosemByteBuffer& CosemIpv4SetupObject::UseDhcpFlag() const
{
  return useDhcpFlag_;
}

const CosemByteBuffer& CosemIpv4SetupObject::PrimaryDnsAddress() const
{
  return primaryDnsAddress_;
}

const CosemByteBuffer& CosemIpv4SetupObject::SecondaryDnsAddress() const
{
  return secondaryDnsAddress_;
}

namespace {
constexpr std::uint16_t kMacAddressSetupClassId = 43u;
constexpr std::uint8_t kMacAddressSetupMacAddressAttributeId = 2u;
} // namespace

const std::uint8_t CosemMacAddressSetupObject::MaxSupportedVersion;

CosemMacAddressSetupObject::CosemMacAddressSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& macAddress,
  AttributeAccessMode mutableAccess)
  : CosemMacAddressSetupObject(
      logicalName, macAddress, mutableAccess, kVersion0)
{
}

CosemMacAddressSetupObject::CosemMacAddressSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& macAddress,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kMacAddressSetupClassId,
      NormalizeVersion(
        version, CosemMacAddressSetupObject::MaxSupportedVersion),
      logicalName))
  , macAddress_(macAddress)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kMacAddressSetupMacAddressAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemMacAddressSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemMacAddressSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemMacAddressSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kMacAddressSetupMacAddressAttributeId:
      output = macAddress_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemMacAddressSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kMacAddressSetupMacAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      macAddress_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemMacAddressSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // MAC Address Setup IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemMacAddressSetupObject::MacAddress() const
{
  return macAddress_;
}

namespace {
constexpr std::uint16_t kPppSetupClassId = 44u;
constexpr std::uint8_t kPppSetupPhyReferenceAttributeId = 2u;
constexpr std::uint8_t kPppSetupLcpOptionsAttributeId = 3u;
constexpr std::uint8_t kPppSetupIpcpOptionsAttributeId = 4u;
constexpr std::uint8_t kPppSetupPppAuthenticationAttributeId = 5u;
} // namespace

const std::uint8_t CosemPppSetupObject::MaxSupportedVersion;

CosemPppSetupObject::CosemPppSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& phyReference,
  const CosemByteBuffer& lcpOptions,
  const CosemByteBuffer& ipcpOptions,
  const CosemByteBuffer& pppAuthentication,
  AttributeAccessMode mutableAccess)
  : CosemPppSetupObject(
      logicalName, phyReference, lcpOptions, ipcpOptions,
      pppAuthentication, mutableAccess, kVersion0)
{
}

CosemPppSetupObject::CosemPppSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& phyReference,
  const CosemByteBuffer& lcpOptions,
  const CosemByteBuffer& ipcpOptions,
  const CosemByteBuffer& pppAuthentication,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPppSetupClassId,
      NormalizeVersion(
        version, CosemPppSetupObject::MaxSupportedVersion),
      logicalName))
  , phyReference_(phyReference)
  , lcpOptions_(lcpOptions)
  , ipcpOptions_(ipcpOptions)
  , pppAuthentication_(pppAuthentication)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kPppSetupPhyReferenceAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPppSetupLcpOptionsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPppSetupIpcpOptionsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kPppSetupPppAuthenticationAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemPppSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemPppSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemPppSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPppSetupPhyReferenceAttributeId:
      output = phyReference_;
      return CosemStatus::Ok;
    case kPppSetupLcpOptionsAttributeId:
      output = lcpOptions_;
      return CosemStatus::Ok;
    case kPppSetupIpcpOptionsAttributeId:
      output = ipcpOptions_;
      return CosemStatus::Ok;
    case kPppSetupPppAuthenticationAttributeId:
      output = pppAuthentication_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPppSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kPppSetupPhyReferenceAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      phyReference_ = input;
      return CosemStatus::Ok;
    case kPppSetupLcpOptionsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      lcpOptions_ = input;
      return CosemStatus::Ok;
    case kPppSetupIpcpOptionsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      ipcpOptions_ = input;
      return CosemStatus::Ok;
    case kPppSetupPppAuthenticationAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      pppAuthentication_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPppSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // PPP Setup IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemPppSetupObject::PhyReference() const
{
  return phyReference_;
}

const CosemByteBuffer& CosemPppSetupObject::LcpOptions() const
{
  return lcpOptions_;
}

const CosemByteBuffer& CosemPppSetupObject::IpcpOptions() const
{
  return ipcpOptions_;
}

const CosemByteBuffer& CosemPppSetupObject::PppAuthentication() const
{
  return pppAuthentication_;
}

namespace {
constexpr std::uint16_t kSmtpSetupClassId = 46u;
constexpr std::uint8_t kSmtpSetupSmtpServerAttributeId = 2u;
constexpr std::uint8_t kSmtpSetupSmtpServerPortAttributeId = 3u;
constexpr std::uint8_t kSmtpSetupUserNameAttributeId = 4u;
constexpr std::uint8_t kSmtpSetupLoginPasswordAttributeId = 5u;
constexpr std::uint8_t kSmtpSetupSenderAttributeId = 6u;
constexpr std::uint8_t kSmtpSetupReceiversAttributeId = 7u;
} // namespace

const std::uint8_t CosemSmtpSetupObject::MaxSupportedVersion;

CosemSmtpSetupObject::CosemSmtpSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& smtpServer,
  const CosemByteBuffer& smtpServerPort,
  const CosemByteBuffer& userName,
  const CosemByteBuffer& loginPassword,
  const CosemByteBuffer& sender,
  const CosemByteBuffer& receivers,
  AttributeAccessMode mutableAccess)
  : CosemSmtpSetupObject(
      logicalName, smtpServer, smtpServerPort, userName,
      loginPassword, sender, receivers, mutableAccess, kVersion0)
{
}

CosemSmtpSetupObject::CosemSmtpSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& smtpServer,
  const CosemByteBuffer& smtpServerPort,
  const CosemByteBuffer& userName,
  const CosemByteBuffer& loginPassword,
  const CosemByteBuffer& sender,
  const CosemByteBuffer& receivers,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSmtpSetupClassId,
      NormalizeVersion(
        version, CosemSmtpSetupObject::MaxSupportedVersion),
      logicalName))
  , smtpServer_(smtpServer)
  , smtpServerPort_(smtpServerPort)
  , userName_(userName)
  , loginPassword_(loginPassword)
  , sender_(sender)
  , receivers_(receivers)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSmtpSetupSmtpServerAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupSmtpServerPortAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupUserNameAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupLoginPasswordAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupSenderAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupReceiversAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemSmtpSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSmtpSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSmtpSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSmtpSetupSmtpServerAttributeId:
      output = smtpServer_;
      return CosemStatus::Ok;
    case kSmtpSetupSmtpServerPortAttributeId:
      output = smtpServerPort_;
      return CosemStatus::Ok;
    case kSmtpSetupUserNameAttributeId:
      output = userName_;
      return CosemStatus::Ok;
    case kSmtpSetupLoginPasswordAttributeId:
      output = loginPassword_;
      return CosemStatus::Ok;
    case kSmtpSetupSenderAttributeId:
      output = sender_;
      return CosemStatus::Ok;
    case kSmtpSetupReceiversAttributeId:
      output = receivers_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSmtpSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSmtpSetupSmtpServerAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      smtpServer_ = input;
      return CosemStatus::Ok;
    case kSmtpSetupSmtpServerPortAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      smtpServerPort_ = input;
      return CosemStatus::Ok;
    case kSmtpSetupUserNameAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      userName_ = input;
      return CosemStatus::Ok;
    case kSmtpSetupLoginPasswordAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      loginPassword_ = input;
      return CosemStatus::Ok;
    case kSmtpSetupSenderAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      sender_ = input;
      return CosemStatus::Ok;
    case kSmtpSetupReceiversAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      receivers_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSmtpSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // SMTP Setup IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemSmtpSetupObject::SmtpServer() const
{
  return smtpServer_;
}

const CosemByteBuffer& CosemSmtpSetupObject::SmtpServerPort() const
{
  return smtpServerPort_;
}

const CosemByteBuffer& CosemSmtpSetupObject::UserName() const
{
  return userName_;
}

const CosemByteBuffer& CosemSmtpSetupObject::LoginPassword() const
{
  return loginPassword_;
}

const CosemByteBuffer& CosemSmtpSetupObject::Sender() const
{
  return sender_;
}

const CosemByteBuffer& CosemSmtpSetupObject::Receivers() const
{
  return receivers_;
}

namespace {
constexpr std::uint16_t kGsmDiagnosticClassId = 47u;
constexpr std::uint8_t kGsmDiagnosticOperatorAttributeId = 2u;
constexpr std::uint8_t kGsmDiagnosticStatusAttributeId = 3u;
constexpr std::uint8_t kGsmDiagnosticCsStatusAttributeId = 4u;
constexpr std::uint8_t kGsmDiagnosticPsStatusAttributeId = 5u;
constexpr std::uint8_t kGsmDiagnosticCellInfoAttributeId = 6u;
constexpr std::uint8_t kGsmDiagnosticAdjacentCellsAttributeId = 7u;
constexpr std::uint8_t kGsmDiagnosticCaptureTimeAttributeId = 8u;
constexpr std::uint8_t kGsmDiagnosticResetMethodId = 1u;
} // namespace

const std::uint8_t CosemGsmDiagnosticObject::MaxSupportedVersion;

CosemGsmDiagnosticObject::CosemGsmDiagnosticObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& operatorName,
  const CosemByteBuffer& status,
  const CosemByteBuffer& circuitSwitchedStatus,
  const CosemByteBuffer& packetSwitchedStatus,
  const CosemByteBuffer& cellInfo,
  const CosemByteBuffer& adjacentCells,
  const CosemByteBuffer& captureTime,
  AttributeAccessMode mutableAccess)
  : CosemGsmDiagnosticObject(
      logicalName, operatorName, status, circuitSwitchedStatus,
      packetSwitchedStatus, cellInfo, adjacentCells, captureTime,
      mutableAccess, kVersion0)
{
}

CosemGsmDiagnosticObject::CosemGsmDiagnosticObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& operatorName,
  const CosemByteBuffer& status,
  const CosemByteBuffer& circuitSwitchedStatus,
  const CosemByteBuffer& packetSwitchedStatus,
  const CosemByteBuffer& cellInfo,
  const CosemByteBuffer& adjacentCells,
  const CosemByteBuffer& captureTime,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kGsmDiagnosticClassId,
      NormalizeVersion(
        version, CosemGsmDiagnosticObject::MaxSupportedVersion),
      logicalName))
  , operatorName_(operatorName)
  , status_(status)
  , circuitSwitchedStatus_(circuitSwitchedStatus)
  , packetSwitchedStatus_(packetSwitchedStatus)
  , cellInfo_(cellInfo)
  , adjacentCells_(adjacentCells)
  , captureTime_(captureTime)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kGsmDiagnosticOperatorAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGsmDiagnosticStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGsmDiagnosticCsStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGsmDiagnosticPsStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGsmDiagnosticCellInfoAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGsmDiagnosticAdjacentCellsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kGsmDiagnosticCaptureTimeAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemGsmDiagnosticObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemGsmDiagnosticObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemGsmDiagnosticObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kGsmDiagnosticOperatorAttributeId:
      output = operatorName_;
      return CosemStatus::Ok;
    case kGsmDiagnosticStatusAttributeId:
      output = status_;
      return CosemStatus::Ok;
    case kGsmDiagnosticCsStatusAttributeId:
      output = circuitSwitchedStatus_;
      return CosemStatus::Ok;
    case kGsmDiagnosticPsStatusAttributeId:
      output = packetSwitchedStatus_;
      return CosemStatus::Ok;
    case kGsmDiagnosticCellInfoAttributeId:
      output = cellInfo_;
      return CosemStatus::Ok;
    case kGsmDiagnosticAdjacentCellsAttributeId:
      output = adjacentCells_;
      return CosemStatus::Ok;
    case kGsmDiagnosticCaptureTimeAttributeId:
      output = captureTime_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemGsmDiagnosticObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kGsmDiagnosticOperatorAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      operatorName_ = input;
      return CosemStatus::Ok;
    case kGsmDiagnosticStatusAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      status_ = input;
      return CosemStatus::Ok;
    case kGsmDiagnosticCsStatusAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      circuitSwitchedStatus_ = input;
      return CosemStatus::Ok;
    case kGsmDiagnosticPsStatusAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      packetSwitchedStatus_ = input;
      return CosemStatus::Ok;
    case kGsmDiagnosticCellInfoAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      cellInfo_ = input;
      return CosemStatus::Ok;
    case kGsmDiagnosticAdjacentCellsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      adjacentCells_ = input;
      return CosemStatus::Ok;
    case kGsmDiagnosticCaptureTimeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      captureTime_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemGsmDiagnosticObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kGsmDiagnosticResetMethodId) {
    // GSM Diagnostic reset is not exposed by the built-in object;
    // backend is expected to refresh status fields out-of-band.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemGsmDiagnosticObject::OperatorName() const
{
  return operatorName_;
}

const CosemByteBuffer& CosemGsmDiagnosticObject::Status() const
{
  return status_;
}

const CosemByteBuffer&
CosemGsmDiagnosticObject::CircuitSwitchedStatus() const
{
  return circuitSwitchedStatus_;
}

const CosemByteBuffer&
CosemGsmDiagnosticObject::PacketSwitchedStatus() const
{
  return packetSwitchedStatus_;
}

const CosemByteBuffer& CosemGsmDiagnosticObject::CellInfo() const
{
  return cellInfo_;
}

const CosemByteBuffer& CosemGsmDiagnosticObject::AdjacentCells() const
{
  return adjacentCells_;
}

const CosemByteBuffer& CosemGsmDiagnosticObject::CaptureTime() const
{
  return captureTime_;
}

namespace {
constexpr std::uint16_t kIecTwistedPairSetupClassId = 24u;
constexpr std::uint8_t kIecTwistedPairSetupPrimaryAddressAttributeId = 2u;
constexpr std::uint8_t kIecTwistedPairSetupTabisAttributeId = 3u;
} // namespace

const std::uint8_t CosemIecTwistedPairSetupObject::MaxSupportedVersion;

CosemIecTwistedPairSetupObject::CosemIecTwistedPairSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& primaryAddress,
  const CosemByteBuffer& tabis,
  AttributeAccessMode mutableAccess)
  : CosemIecTwistedPairSetupObject(
      logicalName, primaryAddress, tabis, mutableAccess, kVersion0)
{
}

CosemIecTwistedPairSetupObject::CosemIecTwistedPairSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& primaryAddress,
  const CosemByteBuffer& tabis,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIecTwistedPairSetupClassId,
      NormalizeVersion(
        version, CosemIecTwistedPairSetupObject::MaxSupportedVersion),
      logicalName))
  , primaryAddress_(primaryAddress)
  , tabis_(tabis)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIecTwistedPairSetupPrimaryAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecTwistedPairSetupTabisAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemIecTwistedPairSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIecTwistedPairSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIecTwistedPairSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIecTwistedPairSetupPrimaryAddressAttributeId:
      output = primaryAddress_;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupTabisAttributeId:
      output = tabis_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIecTwistedPairSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIecTwistedPairSetupPrimaryAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      primaryAddress_ = input;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupTabisAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      tabis_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIecTwistedPairSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // IEC twisted pair (1) Setup IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemIecTwistedPairSetupObject::PrimaryAddress() const
{
  return primaryAddress_;
}

const CosemByteBuffer& CosemIecTwistedPairSetupObject::Tabis() const
{
  return tabis_;
}

namespace {
constexpr std::uint16_t kMBusSlavePortSetupClassId = 25u;
constexpr std::uint8_t kMBusSlavePortSetupDefaultBaudAttributeId = 2u;
constexpr std::uint8_t kMBusSlavePortSetupAvailableBaudAttributeId = 3u;
constexpr std::uint8_t kMBusSlavePortSetupStatusAttributeId = 4u;
constexpr std::uint8_t kMBusSlavePortSetupMBusPortReferenceAttributeId = 5u;
constexpr std::uint8_t kMBusSlavePortSetupResetMethodId = 1u;
} // namespace

const std::uint8_t CosemMBusSlavePortSetupObject::MaxSupportedVersion;

CosemMBusSlavePortSetupObject::CosemMBusSlavePortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& defaultBaud,
  const CosemByteBuffer& availableBaud,
  const CosemByteBuffer& status,
  const CosemByteBuffer& mbusPortReference,
  AttributeAccessMode mutableAccess)
  : CosemMBusSlavePortSetupObject(
      logicalName, defaultBaud, availableBaud, status,
      mbusPortReference, mutableAccess, kVersion0)
{
}

CosemMBusSlavePortSetupObject::CosemMBusSlavePortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& defaultBaud,
  const CosemByteBuffer& availableBaud,
  const CosemByteBuffer& status,
  const CosemByteBuffer& mbusPortReference,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kMBusSlavePortSetupClassId,
      NormalizeVersion(
        version, CosemMBusSlavePortSetupObject::MaxSupportedVersion),
      logicalName))
  , defaultBaud_(defaultBaud)
  , availableBaud_(availableBaud)
  , status_(status)
  , mbusPortReference_(mbusPortReference)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupDefaultBaudAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupAvailableBaudAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupMBusPortReferenceAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemMBusSlavePortSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemMBusSlavePortSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemMBusSlavePortSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kMBusSlavePortSetupDefaultBaudAttributeId:
      output = defaultBaud_;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupAvailableBaudAttributeId:
      output = availableBaud_;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupStatusAttributeId:
      output = status_;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupMBusPortReferenceAttributeId:
      output = mbusPortReference_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemMBusSlavePortSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kMBusSlavePortSetupDefaultBaudAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      defaultBaud_ = input;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupAvailableBaudAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      availableBaud_ = input;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupStatusAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      status_ = input;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupMBusPortReferenceAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      mbusPortReference_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemMBusSlavePortSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kMBusSlavePortSetupResetMethodId) {
    // M-Bus slave port reset is not exposed by the built-in object;
    // backend is expected to drive the slave port out-of-band.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemMBusSlavePortSetupObject::DefaultBaud() const
{
  return defaultBaud_;
}

const CosemByteBuffer&
CosemMBusSlavePortSetupObject::AvailableBaud() const
{
  return availableBaud_;
}

const CosemByteBuffer& CosemMBusSlavePortSetupObject::Status() const
{
  return status_;
}

const CosemByteBuffer&
CosemMBusSlavePortSetupObject::MBusPortReference() const
{
  return mbusPortReference_;
}

namespace {
constexpr std::uint16_t kIpv6SetupClassId = 48u;
constexpr std::uint8_t kIpv6SetupDataLinkLayerReferenceAttributeId = 2u;
constexpr std::uint8_t kIpv6SetupAddressConfigModeAttributeId = 3u;
constexpr std::uint8_t kIpv6SetupUnicastIpAddressAttributeId = 4u;
constexpr std::uint8_t kIpv6SetupMulticastIpAddressAttributeId = 5u;
constexpr std::uint8_t kIpv6SetupGatewayIpAddressAttributeId = 6u;
constexpr std::uint8_t kIpv6SetupPrimaryDnsAddressAttributeId = 7u;
constexpr std::uint8_t kIpv6SetupSecondaryDnsAddressAttributeId = 8u;
constexpr std::uint8_t kIpv6SetupTrafficClassAttributeId = 9u;
constexpr std::uint8_t kIpv6SetupNeighborDiscoverySetupAttributeId = 10u;
constexpr std::uint8_t kIpv6SetupAddAddressMethodId = 1u;
constexpr std::uint8_t kIpv6SetupRemoveAddressMethodId = 2u;
} // namespace

const std::uint8_t CosemIpv6SetupObject::MaxSupportedVersion;

CosemIpv6SetupObject::CosemIpv6SetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& dataLinkLayerReference,
  const CosemByteBuffer& addressConfigMode,
  const CosemByteBuffer& unicastIpAddress,
  const CosemByteBuffer& multicastIpAddress,
  const CosemByteBuffer& gatewayIpAddress,
  const CosemByteBuffer& primaryDnsAddress,
  const CosemByteBuffer& secondaryDnsAddress,
  const CosemByteBuffer& trafficClass,
  const CosemByteBuffer& neighborDiscoverySetup,
  AttributeAccessMode mutableAccess)
  : CosemIpv6SetupObject(
      logicalName, dataLinkLayerReference, addressConfigMode,
      unicastIpAddress, multicastIpAddress, gatewayIpAddress,
      primaryDnsAddress, secondaryDnsAddress, trafficClass,
      neighborDiscoverySetup, mutableAccess, kVersion0)
{
}

CosemIpv6SetupObject::CosemIpv6SetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& dataLinkLayerReference,
  const CosemByteBuffer& addressConfigMode,
  const CosemByteBuffer& unicastIpAddress,
  const CosemByteBuffer& multicastIpAddress,
  const CosemByteBuffer& gatewayIpAddress,
  const CosemByteBuffer& primaryDnsAddress,
  const CosemByteBuffer& secondaryDnsAddress,
  const CosemByteBuffer& trafficClass,
  const CosemByteBuffer& neighborDiscoverySetup,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIpv6SetupClassId,
      NormalizeVersion(
        version, CosemIpv6SetupObject::MaxSupportedVersion),
      logicalName))
  , dataLinkLayerReference_(dataLinkLayerReference)
  , addressConfigMode_(addressConfigMode)
  , unicastIpAddress_(unicastIpAddress)
  , multicastIpAddress_(multicastIpAddress)
  , gatewayIpAddress_(gatewayIpAddress)
  , primaryDnsAddress_(primaryDnsAddress)
  , secondaryDnsAddress_(secondaryDnsAddress)
  , trafficClass_(trafficClass)
  , neighborDiscoverySetup_(neighborDiscoverySetup)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIpv6SetupDataLinkLayerReferenceAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupAddressConfigModeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupUnicastIpAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupMulticastIpAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupGatewayIpAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupPrimaryDnsAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupSecondaryDnsAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupTrafficClassAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIpv6SetupNeighborDiscoverySetupAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemIpv6SetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIpv6SetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIpv6SetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIpv6SetupDataLinkLayerReferenceAttributeId:
      output = dataLinkLayerReference_;
      return CosemStatus::Ok;
    case kIpv6SetupAddressConfigModeAttributeId:
      output = addressConfigMode_;
      return CosemStatus::Ok;
    case kIpv6SetupUnicastIpAddressAttributeId:
      output = unicastIpAddress_;
      return CosemStatus::Ok;
    case kIpv6SetupMulticastIpAddressAttributeId:
      output = multicastIpAddress_;
      return CosemStatus::Ok;
    case kIpv6SetupGatewayIpAddressAttributeId:
      output = gatewayIpAddress_;
      return CosemStatus::Ok;
    case kIpv6SetupPrimaryDnsAddressAttributeId:
      output = primaryDnsAddress_;
      return CosemStatus::Ok;
    case kIpv6SetupSecondaryDnsAddressAttributeId:
      output = secondaryDnsAddress_;
      return CosemStatus::Ok;
    case kIpv6SetupTrafficClassAttributeId:
      output = trafficClass_;
      return CosemStatus::Ok;
    case kIpv6SetupNeighborDiscoverySetupAttributeId:
      output = neighborDiscoverySetup_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIpv6SetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIpv6SetupDataLinkLayerReferenceAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      dataLinkLayerReference_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupAddressConfigModeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      addressConfigMode_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupUnicastIpAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      unicastIpAddress_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupMulticastIpAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      multicastIpAddress_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupGatewayIpAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      gatewayIpAddress_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupPrimaryDnsAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      primaryDnsAddress_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupSecondaryDnsAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      secondaryDnsAddress_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupTrafficClassAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      trafficClass_ = input;
      return CosemStatus::Ok;
    case kIpv6SetupNeighborDiscoverySetupAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      neighborDiscoverySetup_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIpv6SetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kIpv6SetupAddAddressMethodId ||
      methodId == kIpv6SetupRemoveAddressMethodId) {
    // IPv6 Setup add/remove address methods are not exposed by the
    // built-in object; backend is expected to drive the network
    // stack out-of-band and republish the stored buffers.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemIpv6SetupObject::DataLinkLayerReference() const
{
  return dataLinkLayerReference_;
}

const CosemByteBuffer& CosemIpv6SetupObject::AddressConfigMode() const
{
  return addressConfigMode_;
}

const CosemByteBuffer& CosemIpv6SetupObject::UnicastIpAddress() const
{
  return unicastIpAddress_;
}

const CosemByteBuffer& CosemIpv6SetupObject::MulticastIpAddress() const
{
  return multicastIpAddress_;
}

const CosemByteBuffer& CosemIpv6SetupObject::GatewayIpAddress() const
{
  return gatewayIpAddress_;
}

const CosemByteBuffer& CosemIpv6SetupObject::PrimaryDnsAddress() const
{
  return primaryDnsAddress_;
}

const CosemByteBuffer& CosemIpv6SetupObject::SecondaryDnsAddress() const
{
  return secondaryDnsAddress_;
}

const CosemByteBuffer& CosemIpv6SetupObject::TrafficClass() const
{
  return trafficClass_;
}

const CosemByteBuffer&
CosemIpv6SetupObject::NeighborDiscoverySetup() const
{
  return neighborDiscoverySetup_;
}

namespace {
constexpr std::uint16_t kUtilityTablesClassId = 26u;
constexpr std::uint8_t kUtilityTablesTableIdAttributeId = 2u;
constexpr std::uint8_t kUtilityTablesLengthAttributeId = 3u;
constexpr std::uint8_t kUtilityTablesBufferAttributeId = 4u;
} // namespace

const std::uint8_t CosemUtilityTablesObject::MaxSupportedVersion;

CosemUtilityTablesObject::CosemUtilityTablesObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tableId,
  const CosemByteBuffer& length,
  const CosemByteBuffer& buffer,
  AttributeAccessMode mutableAccess)
  : CosemUtilityTablesObject(
      logicalName, tableId, length, buffer, mutableAccess, kVersion0)
{
}

CosemUtilityTablesObject::CosemUtilityTablesObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tableId,
  const CosemByteBuffer& length,
  const CosemByteBuffer& buffer,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kUtilityTablesClassId,
      NormalizeVersion(
        version, CosemUtilityTablesObject::MaxSupportedVersion),
      logicalName))
  , tableId_(tableId)
  , length_(length)
  , buffer_(buffer)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kUtilityTablesTableIdAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kUtilityTablesLengthAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kUtilityTablesBufferAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemUtilityTablesObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemUtilityTablesObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemUtilityTablesObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kUtilityTablesTableIdAttributeId:
      output = tableId_;
      return CosemStatus::Ok;
    case kUtilityTablesLengthAttributeId:
      output = length_;
      return CosemStatus::Ok;
    case kUtilityTablesBufferAttributeId:
      output = buffer_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemUtilityTablesObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kUtilityTablesTableIdAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      tableId_ = input;
      return CosemStatus::Ok;
    case kUtilityTablesLengthAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      length_ = input;
      return CosemStatus::Ok;
    case kUtilityTablesBufferAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      buffer_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemUtilityTablesObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Utility Tables IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemUtilityTablesObject::TableId() const
{
  return tableId_;
}

const CosemByteBuffer& CosemUtilityTablesObject::Length() const
{
  return length_;
}

const CosemByteBuffer& CosemUtilityTablesObject::Buffer() const
{
  return buffer_;
}

namespace {
constexpr std::uint16_t kSensorManagerClassId = 67u;
constexpr std::uint8_t kSensorManagerStatusAttributeId = 2u;
constexpr std::uint8_t kSensorManagerSerialNumberAttributeId = 3u;
constexpr std::uint8_t kSensorManagerDeviceTypeAttributeId = 4u;
constexpr std::uint8_t kSensorManagerManufacturerIdAttributeId = 5u;
constexpr std::uint8_t kSensorManagerFirmwareVersionAttributeId = 6u;
constexpr std::uint8_t
  kSensorManagerMetrologyFirmwareVersionAttributeId = 7u;
constexpr std::uint8_t kSensorManagerDriverAttributeId = 8u;
constexpr std::uint8_t
  kSensorManagerCommunicationDescAttributeId = 9u;
constexpr std::uint8_t kSensorManagerSetupDescAttributeId = 10u;
constexpr std::uint8_t
  kSensorManagerMeasurementDescAttributeId = 11u;
} // namespace

const std::uint8_t CosemSensorManagerObject::MaxSupportedVersion;

CosemSensorManagerObject::CosemSensorManagerObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& status,
  const CosemByteBuffer& serialNumber,
  const CosemByteBuffer& deviceType,
  const CosemByteBuffer& manufacturerId,
  const CosemByteBuffer& firmwareVersion,
  const CosemByteBuffer& metrologyFirmwareVersion,
  const CosemByteBuffer& driver,
  const CosemByteBuffer& communicationDesc,
  const CosemByteBuffer& setupDesc,
  const CosemByteBuffer& measurementDesc,
  AttributeAccessMode mutableAccess)
  : CosemSensorManagerObject(
      logicalName, status, serialNumber, deviceType, manufacturerId,
      firmwareVersion, metrologyFirmwareVersion, driver,
      communicationDesc, setupDesc, measurementDesc, mutableAccess,
      kVersion0)
{
}

CosemSensorManagerObject::CosemSensorManagerObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& status,
  const CosemByteBuffer& serialNumber,
  const CosemByteBuffer& deviceType,
  const CosemByteBuffer& manufacturerId,
  const CosemByteBuffer& firmwareVersion,
  const CosemByteBuffer& metrologyFirmwareVersion,
  const CosemByteBuffer& driver,
  const CosemByteBuffer& communicationDesc,
  const CosemByteBuffer& setupDesc,
  const CosemByteBuffer& measurementDesc,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSensorManagerClassId,
      NormalizeVersion(
        version, CosemSensorManagerObject::MaxSupportedVersion),
      logicalName))
  , status_(status)
  , serialNumber_(serialNumber)
  , deviceType_(deviceType)
  , manufacturerId_(manufacturerId)
  , firmwareVersion_(firmwareVersion)
  , metrologyFirmwareVersion_(metrologyFirmwareVersion)
  , driver_(driver)
  , communicationDesc_(communicationDesc)
  , setupDesc_(setupDesc)
  , measurementDesc_(measurementDesc)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSensorManagerStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerSerialNumberAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerDeviceTypeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerManufacturerIdAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerFirmwareVersionAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerMetrologyFirmwareVersionAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerDriverAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerCommunicationDescAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerSetupDescAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerMeasurementDescAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemSensorManagerObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSensorManagerObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSensorManagerObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSensorManagerStatusAttributeId:
      output = status_;
      return CosemStatus::Ok;
    case kSensorManagerSerialNumberAttributeId:
      output = serialNumber_;
      return CosemStatus::Ok;
    case kSensorManagerDeviceTypeAttributeId:
      output = deviceType_;
      return CosemStatus::Ok;
    case kSensorManagerManufacturerIdAttributeId:
      output = manufacturerId_;
      return CosemStatus::Ok;
    case kSensorManagerFirmwareVersionAttributeId:
      output = firmwareVersion_;
      return CosemStatus::Ok;
    case kSensorManagerMetrologyFirmwareVersionAttributeId:
      output = metrologyFirmwareVersion_;
      return CosemStatus::Ok;
    case kSensorManagerDriverAttributeId:
      output = driver_;
      return CosemStatus::Ok;
    case kSensorManagerCommunicationDescAttributeId:
      output = communicationDesc_;
      return CosemStatus::Ok;
    case kSensorManagerSetupDescAttributeId:
      output = setupDesc_;
      return CosemStatus::Ok;
    case kSensorManagerMeasurementDescAttributeId:
      output = measurementDesc_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSensorManagerObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSensorManagerStatusAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      status_ = input;
      return CosemStatus::Ok;
    case kSensorManagerSerialNumberAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      serialNumber_ = input;
      return CosemStatus::Ok;
    case kSensorManagerDeviceTypeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      deviceType_ = input;
      return CosemStatus::Ok;
    case kSensorManagerManufacturerIdAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      manufacturerId_ = input;
      return CosemStatus::Ok;
    case kSensorManagerFirmwareVersionAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      firmwareVersion_ = input;
      return CosemStatus::Ok;
    case kSensorManagerMetrologyFirmwareVersionAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      metrologyFirmwareVersion_ = input;
      return CosemStatus::Ok;
    case kSensorManagerDriverAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      driver_ = input;
      return CosemStatus::Ok;
    case kSensorManagerCommunicationDescAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      communicationDesc_ = input;
      return CosemStatus::Ok;
    case kSensorManagerSetupDescAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      setupDesc_ = input;
      return CosemStatus::Ok;
    case kSensorManagerMeasurementDescAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      measurementDesc_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSensorManagerObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Sensor Manager IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemSensorManagerObject::Status() const
{
  return status_;
}

const CosemByteBuffer& CosemSensorManagerObject::SerialNumber() const
{
  return serialNumber_;
}

const CosemByteBuffer& CosemSensorManagerObject::DeviceType() const
{
  return deviceType_;
}

const CosemByteBuffer&
CosemSensorManagerObject::ManufacturerId() const
{
  return manufacturerId_;
}

const CosemByteBuffer&
CosemSensorManagerObject::FirmwareVersion() const
{
  return firmwareVersion_;
}

const CosemByteBuffer&
CosemSensorManagerObject::MetrologyFirmwareVersion() const
{
  return metrologyFirmwareVersion_;
}

const CosemByteBuffer& CosemSensorManagerObject::Driver() const
{
  return driver_;
}

const CosemByteBuffer&
CosemSensorManagerObject::CommunicationDesc() const
{
  return communicationDesc_;
}

const CosemByteBuffer& CosemSensorManagerObject::SetupDesc() const
{
  return setupDesc_;
}

const CosemByteBuffer&
CosemSensorManagerObject::MeasurementDesc() const
{
  return measurementDesc_;
}

namespace {
constexpr std::uint16_t kArbitratorClassId = 68u;
constexpr std::uint8_t kArbitratorActionsAttributeId = 2u;
constexpr std::uint8_t kArbitratorPermissionsTableAttributeId = 3u;
constexpr std::uint8_t kArbitratorWeightingsTableAttributeId = 4u;
constexpr std::uint8_t
  kArbitratorMostRecentRequestsTableAttributeId = 5u;
constexpr std::uint8_t kArbitratorLastOutcomeAttributeId = 6u;
constexpr std::uint8_t kArbitratorRequestActionMethodId = 1u;
constexpr std::uint8_t kArbitratorResetMethodId = 2u;
} // namespace

const std::uint8_t CosemArbitratorObject::MaxSupportedVersion;

CosemArbitratorObject::CosemArbitratorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& actions,
  const CosemByteBuffer& permissionsTable,
  const CosemByteBuffer& weightingsTable,
  const CosemByteBuffer& mostRecentRequestsTable,
  const CosemByteBuffer& lastOutcome,
  AttributeAccessMode mutableAccess)
  : CosemArbitratorObject(
      logicalName, actions, permissionsTable, weightingsTable,
      mostRecentRequestsTable, lastOutcome, mutableAccess,
      kVersion0)
{
}

CosemArbitratorObject::CosemArbitratorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& actions,
  const CosemByteBuffer& permissionsTable,
  const CosemByteBuffer& weightingsTable,
  const CosemByteBuffer& mostRecentRequestsTable,
  const CosemByteBuffer& lastOutcome,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kArbitratorClassId,
      NormalizeVersion(
        version, CosemArbitratorObject::MaxSupportedVersion),
      logicalName))
  , actions_(actions)
  , permissionsTable_(permissionsTable)
  , weightingsTable_(weightingsTable)
  , mostRecentRequestsTable_(mostRecentRequestsTable)
  , lastOutcome_(lastOutcome)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kArbitratorActionsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kArbitratorPermissionsTableAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kArbitratorWeightingsTableAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kArbitratorMostRecentRequestsTableAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kArbitratorLastOutcomeAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemArbitratorObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemArbitratorObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemArbitratorObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kArbitratorActionsAttributeId:
      output = actions_;
      return CosemStatus::Ok;
    case kArbitratorPermissionsTableAttributeId:
      output = permissionsTable_;
      return CosemStatus::Ok;
    case kArbitratorWeightingsTableAttributeId:
      output = weightingsTable_;
      return CosemStatus::Ok;
    case kArbitratorMostRecentRequestsTableAttributeId:
      output = mostRecentRequestsTable_;
      return CosemStatus::Ok;
    case kArbitratorLastOutcomeAttributeId:
      output = lastOutcome_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemArbitratorObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kArbitratorActionsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      actions_ = input;
      return CosemStatus::Ok;
    case kArbitratorPermissionsTableAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      permissionsTable_ = input;
      return CosemStatus::Ok;
    case kArbitratorWeightingsTableAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      weightingsTable_ = input;
      return CosemStatus::Ok;
    case kArbitratorMostRecentRequestsTableAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      mostRecentRequestsTable_ = input;
      return CosemStatus::Ok;
    case kArbitratorLastOutcomeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      lastOutcome_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemArbitratorObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kArbitratorRequestActionMethodId ||
      methodId == kArbitratorResetMethodId) {
    // Arbitrator request_action / reset are not exposed by the
    // built-in object; backend is expected to drive arbitration
    // out-of-band and republish the stored buffers.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemArbitratorObject::Actions() const
{
  return actions_;
}

const CosemByteBuffer& CosemArbitratorObject::PermissionsTable() const
{
  return permissionsTable_;
}

const CosemByteBuffer& CosemArbitratorObject::WeightingsTable() const
{
  return weightingsTable_;
}

const CosemByteBuffer&
CosemArbitratorObject::MostRecentRequestsTable() const
{
  return mostRecentRequestsTable_;
}

const CosemByteBuffer& CosemArbitratorObject::LastOutcome() const
{
  return lastOutcome_;
}

namespace {
constexpr std::uint16_t kStatusMappingClassId = 63u;
constexpr std::uint8_t kStatusMappingStatusWordAttributeId = 2u;
constexpr std::uint8_t kStatusMappingMappingsAttributeId = 3u;
} // namespace

const std::uint8_t CosemStatusMappingObject::MaxSupportedVersion;

CosemStatusMappingObject::CosemStatusMappingObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& statusWord,
  const CosemByteBuffer& mappings,
  AttributeAccessMode mutableAccess)
  : CosemStatusMappingObject(
      logicalName, statusWord, mappings, mutableAccess, kVersion0)
{
}

CosemStatusMappingObject::CosemStatusMappingObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& statusWord,
  const CosemByteBuffer& mappings,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kStatusMappingClassId,
      NormalizeVersion(
        version, CosemStatusMappingObject::MaxSupportedVersion),
      logicalName))
  , statusWord_(statusWord)
  , mappings_(mappings)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kStatusMappingStatusWordAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kStatusMappingMappingsAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemStatusMappingObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemStatusMappingObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemStatusMappingObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kStatusMappingStatusWordAttributeId:
      output = statusWord_;
      return CosemStatus::Ok;
    case kStatusMappingMappingsAttributeId:
      output = mappings_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemStatusMappingObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kStatusMappingStatusWordAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      statusWord_ = input;
      return CosemStatus::Ok;
    case kStatusMappingMappingsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      mappings_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemStatusMappingObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // Status Mapping IC defines no methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemStatusMappingObject::StatusWord() const
{
  return statusWord_;
}

const CosemByteBuffer& CosemStatusMappingObject::Mappings() const
{
  return mappings_;
}

namespace {
constexpr std::uint16_t kParameterMonitorClassId = 65u;
constexpr std::uint8_t
  kParameterMonitorChangedParameterAttributeId = 2u;
constexpr std::uint8_t
  kParameterMonitorCaptureTimeAttributeId = 3u;
constexpr std::uint8_t
  kParameterMonitorParametersAttributeId = 4u;
constexpr std::uint8_t kParameterMonitorInsertMethodId = 1u;
constexpr std::uint8_t kParameterMonitorDeleteMethodId = 2u;
} // namespace

const std::uint8_t CosemParameterMonitorObject::MaxSupportedVersion;

CosemParameterMonitorObject::CosemParameterMonitorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& changedParameter,
  const CosemByteBuffer& captureTime,
  const CosemByteBuffer& parameters,
  AttributeAccessMode mutableAccess)
  : CosemParameterMonitorObject(
      logicalName, changedParameter, captureTime, parameters,
      mutableAccess, kVersion0)
{
}

CosemParameterMonitorObject::CosemParameterMonitorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& changedParameter,
  const CosemByteBuffer& captureTime,
  const CosemByteBuffer& parameters,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kParameterMonitorClassId,
      NormalizeVersion(
        version, CosemParameterMonitorObject::MaxSupportedVersion),
      logicalName))
  , changedParameter_(changedParameter)
  , captureTime_(captureTime)
  , parameters_(parameters)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kParameterMonitorChangedParameterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kParameterMonitorCaptureTimeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kParameterMonitorParametersAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemParameterMonitorObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemParameterMonitorObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemParameterMonitorObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kParameterMonitorChangedParameterAttributeId:
      output = changedParameter_;
      return CosemStatus::Ok;
    case kParameterMonitorCaptureTimeAttributeId:
      output = captureTime_;
      return CosemStatus::Ok;
    case kParameterMonitorParametersAttributeId:
      output = parameters_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemParameterMonitorObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kParameterMonitorChangedParameterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      changedParameter_ = input;
      return CosemStatus::Ok;
    case kParameterMonitorCaptureTimeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      captureTime_ = input;
      return CosemStatus::Ok;
    case kParameterMonitorParametersAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      parameters_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemParameterMonitorObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kParameterMonitorInsertMethodId ||
      methodId == kParameterMonitorDeleteMethodId) {
    // Parameter Monitor insert / delete are not exposed by the
    // built-in object; backend is expected to manage the
    // monitored-parameters table out-of-band and republish the
    // stored buffers.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemParameterMonitorObject::ChangedParameter() const
{
  return changedParameter_;
}

const CosemByteBuffer&
CosemParameterMonitorObject::CaptureTime() const
{
  return captureTime_;
}

const CosemByteBuffer&
CosemParameterMonitorObject::Parameters() const
{
  return parameters_;
}

namespace {
constexpr std::uint16_t kCompactDataClassId = 62u;
constexpr std::uint8_t kCompactDataBufferAttributeId = 2u;
constexpr std::uint8_t kCompactDataCaptureObjectsAttributeId = 3u;
constexpr std::uint8_t kCompactDataTemplateIdAttributeId = 4u;
constexpr std::uint8_t
  kCompactDataTemplateDescriptionAttributeId = 5u;
constexpr std::uint8_t kCompactDataCaptureMethodAttributeId = 6u;
constexpr std::uint8_t kCompactDataResetMethodId = 1u;
constexpr std::uint8_t kCompactDataCaptureMethodId = 2u;
} // namespace

const std::uint8_t CosemCompactDataObject::MaxSupportedVersion;

CosemCompactDataObject::CosemCompactDataObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& buffer,
  const CosemByteBuffer& captureObjects,
  const CosemByteBuffer& templateId,
  const CosemByteBuffer& templateDescription,
  const CosemByteBuffer& captureMethod,
  AttributeAccessMode mutableAccess)
  : CosemCompactDataObject(
      logicalName, buffer, captureObjects, templateId,
      templateDescription, captureMethod, mutableAccess, kVersion0)
{
}

CosemCompactDataObject::CosemCompactDataObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& buffer,
  const CosemByteBuffer& captureObjects,
  const CosemByteBuffer& templateId,
  const CosemByteBuffer& templateDescription,
  const CosemByteBuffer& captureMethod,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kCompactDataClassId,
      NormalizeVersion(
        version, CosemCompactDataObject::MaxSupportedVersion),
      logicalName))
  , buffer_(buffer)
  , captureObjects_(captureObjects)
  , templateId_(templateId)
  , templateDescription_(templateDescription)
  , captureMethod_(captureMethod)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kCompactDataBufferAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kCompactDataCaptureObjectsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kCompactDataTemplateIdAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kCompactDataTemplateDescriptionAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kCompactDataCaptureMethodAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemCompactDataObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemCompactDataObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemCompactDataObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kCompactDataBufferAttributeId:
      output = buffer_;
      return CosemStatus::Ok;
    case kCompactDataCaptureObjectsAttributeId:
      output = captureObjects_;
      return CosemStatus::Ok;
    case kCompactDataTemplateIdAttributeId:
      output = templateId_;
      return CosemStatus::Ok;
    case kCompactDataTemplateDescriptionAttributeId:
      output = templateDescription_;
      return CosemStatus::Ok;
    case kCompactDataCaptureMethodAttributeId:
      output = captureMethod_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemCompactDataObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kCompactDataBufferAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      buffer_ = input;
      return CosemStatus::Ok;
    case kCompactDataCaptureObjectsAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      captureObjects_ = input;
      return CosemStatus::Ok;
    case kCompactDataTemplateIdAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      templateId_ = input;
      return CosemStatus::Ok;
    case kCompactDataTemplateDescriptionAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      templateDescription_ = input;
      return CosemStatus::Ok;
    case kCompactDataCaptureMethodAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      captureMethod_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemCompactDataObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kCompactDataResetMethodId ||
      methodId == kCompactDataCaptureMethodId) {
    // Compact Data reset / capture are not exposed by the
    // built-in object; backend is expected to refresh the buffer
    // and capture metadata out-of-band and republish the stored
    // buffers.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemCompactDataObject::Buffer() const
{
  return buffer_;
}

const CosemByteBuffer& CosemCompactDataObject::CaptureObjects() const
{
  return captureObjects_;
}

const CosemByteBuffer& CosemCompactDataObject::TemplateId() const
{
  return templateId_;
}

const CosemByteBuffer&
CosemCompactDataObject::TemplateDescription() const
{
  return templateDescription_;
}

const CosemByteBuffer& CosemCompactDataObject::CaptureMethod() const
{
  return captureMethod_;
}

namespace {
constexpr std::uint16_t kDataProtectionClassId = 30u;
constexpr std::uint8_t
  kDataProtectionProtectionBufferAttributeId = 2u;
constexpr std::uint8_t
  kDataProtectionProtectionObjectListAttributeId = 3u;
constexpr std::uint8_t
  kDataProtectionProtectionParametersGetAttributeId = 4u;
constexpr std::uint8_t
  kDataProtectionProtectionParametersSetAttributeId = 5u;
constexpr std::uint8_t
  kDataProtectionRequiredProtectionAttributeId = 6u;
constexpr std::uint8_t
  kDataProtectionGetProtectedAttributesMethodId = 1u;
constexpr std::uint8_t
  kDataProtectionSetProtectedAttributesMethodId = 2u;
constexpr std::uint8_t
  kDataProtectionInvokeProtectedMethodMethodId = 3u;
} // namespace

const std::uint8_t CosemDataProtectionObject::MaxSupportedVersion;

CosemDataProtectionObject::CosemDataProtectionObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& protectionBuffer,
  const CosemByteBuffer& protectionObjectList,
  const CosemByteBuffer& protectionParametersGet,
  const CosemByteBuffer& protectionParametersSet,
  const CosemByteBuffer& requiredProtection,
  AttributeAccessMode mutableAccess)
  : CosemDataProtectionObject(
      logicalName, protectionBuffer, protectionObjectList,
      protectionParametersGet, protectionParametersSet,
      requiredProtection, mutableAccess, kVersion0)
{
}

CosemDataProtectionObject::CosemDataProtectionObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& protectionBuffer,
  const CosemByteBuffer& protectionObjectList,
  const CosemByteBuffer& protectionParametersGet,
  const CosemByteBuffer& protectionParametersSet,
  const CosemByteBuffer& requiredProtection,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kDataProtectionClassId,
      NormalizeVersion(
        version, CosemDataProtectionObject::MaxSupportedVersion),
      logicalName))
  , protectionBuffer_(protectionBuffer)
  , protectionObjectList_(protectionObjectList)
  , protectionParametersGet_(protectionParametersGet)
  , protectionParametersSet_(protectionParametersSet)
  , requiredProtection_(requiredProtection)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kDataProtectionProtectionBufferAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kDataProtectionProtectionObjectListAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kDataProtectionProtectionParametersGetAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kDataProtectionProtectionParametersSetAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kDataProtectionRequiredProtectionAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemDataProtectionObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemDataProtectionObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemDataProtectionObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kDataProtectionProtectionBufferAttributeId:
      output = protectionBuffer_;
      return CosemStatus::Ok;
    case kDataProtectionProtectionObjectListAttributeId:
      output = protectionObjectList_;
      return CosemStatus::Ok;
    case kDataProtectionProtectionParametersGetAttributeId:
      output = protectionParametersGet_;
      return CosemStatus::Ok;
    case kDataProtectionProtectionParametersSetAttributeId:
      output = protectionParametersSet_;
      return CosemStatus::Ok;
    case kDataProtectionRequiredProtectionAttributeId:
      output = requiredProtection_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemDataProtectionObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kDataProtectionProtectionBufferAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      protectionBuffer_ = input;
      return CosemStatus::Ok;
    case kDataProtectionProtectionObjectListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      protectionObjectList_ = input;
      return CosemStatus::Ok;
    case kDataProtectionProtectionParametersGetAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      protectionParametersGet_ = input;
      return CosemStatus::Ok;
    case kDataProtectionProtectionParametersSetAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      protectionParametersSet_ = input;
      return CosemStatus::Ok;
    case kDataProtectionRequiredProtectionAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      requiredProtection_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemDataProtectionObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kDataProtectionGetProtectedAttributesMethodId ||
      methodId == kDataProtectionSetProtectedAttributesMethodId ||
      methodId == kDataProtectionInvokeProtectedMethodMethodId) {
    // Data Protection get/set/invoke are not exposed by the
    // built-in object; backend is expected to perform the
    // protected operations out-of-band and republish the stored
    // buffers.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemDataProtectionObject::ProtectionBuffer() const
{
  return protectionBuffer_;
}

const CosemByteBuffer&
CosemDataProtectionObject::ProtectionObjectList() const
{
  return protectionObjectList_;
}

const CosemByteBuffer&
CosemDataProtectionObject::ProtectionParametersGet() const
{
  return protectionParametersGet_;
}

const CosemByteBuffer&
CosemDataProtectionObject::ProtectionParametersSet() const
{
  return protectionParametersSet_;
}

const CosemByteBuffer&
CosemDataProtectionObject::RequiredProtection() const
{
  return requiredProtection_;
}

namespace {
constexpr std::uint16_t kIecLocalPortSetupClassId = 19u;
constexpr std::uint8_t kIecLocalPortSetupDefaultModeAttributeId = 2u;
constexpr std::uint8_t kIecLocalPortSetupDefaultBaudAttributeId = 3u;
constexpr std::uint8_t kIecLocalPortSetupProposedBaudAttributeId = 4u;
constexpr std::uint8_t kIecLocalPortSetupResponseTimeAttributeId = 5u;
constexpr std::uint8_t kIecLocalPortSetupDeviceAddressAttributeId = 6u;
constexpr std::uint8_t kIecLocalPortSetupPassword1AttributeId = 7u;
constexpr std::uint8_t kIecLocalPortSetupPassword2AttributeId = 8u;
constexpr std::uint8_t kIecLocalPortSetupPassword5AttributeId = 9u;
constexpr std::uint8_t kIecLocalPortSetupPortSpeedAttributeId = 10u;
constexpr std::uint8_t kVersion1 = 1u;
} // namespace

const std::uint8_t CosemIecLocalPortSetupObject::MaxSupportedVersion;

CosemIecLocalPortSetupObject::CosemIecLocalPortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& defaultMode,
  const CosemByteBuffer& defaultBaud,
  const CosemByteBuffer& proposedBaud,
  const CosemByteBuffer& responseTime,
  const CosemByteBuffer& deviceAddress,
  const CosemByteBuffer& password1,
  const CosemByteBuffer& password2,
  const CosemByteBuffer& password5,
  const CosemByteBuffer& portSpeed,
  AttributeAccessMode mutableAccess)
  : CosemIecLocalPortSetupObject(
      logicalName, defaultMode, defaultBaud, proposedBaud,
      responseTime, deviceAddress, password1, password2, password5,
      portSpeed, mutableAccess, kVersion1)
{
}

CosemIecLocalPortSetupObject::CosemIecLocalPortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& defaultMode,
  const CosemByteBuffer& defaultBaud,
  const CosemByteBuffer& proposedBaud,
  const CosemByteBuffer& responseTime,
  const CosemByteBuffer& deviceAddress,
  const CosemByteBuffer& password1,
  const CosemByteBuffer& password2,
  const CosemByteBuffer& password5,
  const CosemByteBuffer& portSpeed,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIecLocalPortSetupClassId,
      NormalizeVersion(
        version,
        CosemIecLocalPortSetupObject::MaxSupportedVersion),
      logicalName))
  , defaultMode_(defaultMode)
  , defaultBaud_(defaultBaud)
  , proposedBaud_(proposedBaud)
  , responseTime_(responseTime)
  , deviceAddress_(deviceAddress)
  , password1_(password1)
  , password2_(password2)
  , password5_(password5)
  , portSpeed_(portSpeed)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupDefaultModeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupDefaultBaudAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupProposedBaudAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupResponseTimeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupDeviceAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupPassword1AttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupPassword2AttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupPassword5AttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecLocalPortSetupPortSpeedAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemIecLocalPortSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIecLocalPortSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIecLocalPortSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIecLocalPortSetupDefaultModeAttributeId:
      output = defaultMode_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupDefaultBaudAttributeId:
      output = defaultBaud_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupProposedBaudAttributeId:
      output = proposedBaud_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupResponseTimeAttributeId:
      output = responseTime_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupDeviceAddressAttributeId:
      output = deviceAddress_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPassword1AttributeId:
      output = password1_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPassword2AttributeId:
      output = password2_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPassword5AttributeId:
      output = password5_;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPortSpeedAttributeId:
      output = portSpeed_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIecLocalPortSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIecLocalPortSetupDefaultModeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      defaultMode_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupDefaultBaudAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      defaultBaud_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupProposedBaudAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      proposedBaud_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupResponseTimeAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      responseTime_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupDeviceAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      deviceAddress_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPassword1AttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      password1_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPassword2AttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      password2_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPassword5AttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      password5_ = input;
      return CosemStatus::Ok;
    case kIecLocalPortSetupPortSpeedAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      portSpeed_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIecLocalPortSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::DefaultMode() const
{
  return defaultMode_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::DefaultBaud() const
{
  return defaultBaud_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::ProposedBaud() const
{
  return proposedBaud_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::ResponseTime() const
{
  return responseTime_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::DeviceAddress() const
{
  return deviceAddress_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::Password1() const
{
  return password1_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::Password2() const
{
  return password2_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::Password5() const
{
  return password5_;
}

const CosemByteBuffer&
CosemIecLocalPortSetupObject::PortSpeed() const
{
  return portSpeed_;
}

namespace {
constexpr std::uint16_t kAssociationSnClassId = 12u;
constexpr std::uint8_t kAssociationSnObjectListAttributeId = 2u;
constexpr std::uint8_t kAssociationSnAccessRightsListAttributeId = 3u;
constexpr std::uint8_t
  kAssociationSnSecuritySetupReferenceAttributeId = 4u;
constexpr std::uint8_t kAssociationSnUserListAttributeId = 5u;
constexpr std::uint8_t kAssociationSnCurrentUserAttributeId = 6u;
constexpr std::uint8_t
  kAssociationSnReplyToHlsAuthenticationMethodId = 1u;
constexpr std::uint8_t kAssociationSnChangeHlsSecretMethodId = 2u;
constexpr std::uint8_t kAssociationSnAddObjectMethodId = 3u;
constexpr std::uint8_t kAssociationSnRemoveObjectMethodId = 4u;
constexpr std::uint8_t kAssociationSnAddUserMethodId = 5u;
constexpr std::uint8_t kAssociationSnRemoveUserMethodId = 6u;
constexpr std::uint8_t kVersion3 = 3u;
} // namespace

const std::uint8_t CosemAssociationSnObject::MaxSupportedVersion;

CosemAssociationSnObject::CosemAssociationSnObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& objectList,
  const CosemByteBuffer& accessRightsList,
  const CosemByteBuffer& securitySetupReference,
  const CosemByteBuffer& userList,
  const CosemByteBuffer& currentUser,
  AttributeAccessMode mutableAccess)
  : CosemAssociationSnObject(
      logicalName, objectList, accessRightsList,
      securitySetupReference, userList, currentUser,
      mutableAccess, kVersion3)
{
}

CosemAssociationSnObject::CosemAssociationSnObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& objectList,
  const CosemByteBuffer& accessRightsList,
  const CosemByteBuffer& securitySetupReference,
  const CosemByteBuffer& userList,
  const CosemByteBuffer& currentUser,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kAssociationSnClassId,
      NormalizeVersion(
        version,
        CosemAssociationSnObject::MaxSupportedVersion),
      logicalName))
  , objectList_(objectList)
  , accessRightsList_(accessRightsList)
  , securitySetupReference_(securitySetupReference)
  , userList_(userList)
  , currentUser_(currentUser)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kAssociationSnObjectListAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAssociationSnAccessRightsListAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAssociationSnSecuritySetupReferenceAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAssociationSnUserListAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kAssociationSnCurrentUserAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemAssociationSnObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemAssociationSnObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemAssociationSnObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kAssociationSnObjectListAttributeId:
      output = objectList_;
      return CosemStatus::Ok;
    case kAssociationSnAccessRightsListAttributeId:
      output = accessRightsList_;
      return CosemStatus::Ok;
    case kAssociationSnSecuritySetupReferenceAttributeId:
      output = securitySetupReference_;
      return CosemStatus::Ok;
    case kAssociationSnUserListAttributeId:
      output = userList_;
      return CosemStatus::Ok;
    case kAssociationSnCurrentUserAttributeId:
      output = currentUser_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAssociationSnObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kAssociationSnObjectListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      objectList_ = input;
      return CosemStatus::Ok;
    case kAssociationSnAccessRightsListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      accessRightsList_ = input;
      return CosemStatus::Ok;
    case kAssociationSnSecuritySetupReferenceAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      securitySetupReference_ = input;
      return CosemStatus::Ok;
    case kAssociationSnUserListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      userList_ = input;
      return CosemStatus::Ok;
    case kAssociationSnCurrentUserAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      currentUser_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAssociationSnObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kAssociationSnReplyToHlsAuthenticationMethodId ||
      methodId == kAssociationSnChangeHlsSecretMethodId ||
      methodId == kAssociationSnAddObjectMethodId ||
      methodId == kAssociationSnRemoveObjectMethodId ||
      methodId == kAssociationSnAddUserMethodId ||
      methodId == kAssociationSnRemoveUserMethodId) {
    // Association SN HLS authentication, HLS secret rotation and
    // object/user list mutations are not exposed by the built-in
    // object; backend is expected to perform those operations
    // out-of-band and republish the stored buffers.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemAssociationSnObject::ObjectList() const
{
  return objectList_;
}

const CosemByteBuffer&
CosemAssociationSnObject::AccessRightsList() const
{
  return accessRightsList_;
}

const CosemByteBuffer&
CosemAssociationSnObject::SecuritySetupReference() const
{
  return securitySetupReference_;
}

const CosemByteBuffer& CosemAssociationSnObject::UserList() const
{
  return userList_;
}

const CosemByteBuffer& CosemAssociationSnObject::CurrentUser() const
{
  return currentUser_;
}

namespace {
constexpr std::uint16_t kMBusClientClassId = 72u;
constexpr std::uint8_t kMBusClientMbusPortReferenceAttributeId = 2u;
constexpr std::uint8_t kMBusClientCaptureDefinitionAttributeId = 3u;
constexpr std::uint8_t kMBusClientCapturePeriodAttributeId = 4u;
constexpr std::uint8_t kMBusClientPrimaryAddressAttributeId = 5u;
constexpr std::uint8_t
  kMBusClientIdentificationNumberAttributeId = 6u;
constexpr std::uint8_t kMBusClientManufacturerIdAttributeId = 7u;
constexpr std::uint8_t kMBusClientVersionAttributeId = 8u;
constexpr std::uint8_t kMBusClientDeviceTypeAttributeId = 9u;
constexpr std::uint8_t kMBusClientAccessNumberAttributeId = 10u;
constexpr std::uint8_t kMBusClientStatusAttributeId = 11u;
constexpr std::uint8_t kMBusClientAlarmAttributeId = 12u;
constexpr std::uint8_t kMBusClientConfigurationAttributeId = 13u;
constexpr std::uint8_t kMBusClientEncryptionKeyStatusAttributeId = 14u;
constexpr std::uint8_t kMBusClientSlaveInstallMethodId = 1u;
constexpr std::uint8_t kMBusClientSlaveDeinstallMethodId = 2u;
constexpr std::uint8_t kMBusClientCaptureMethodId = 3u;
constexpr std::uint8_t kMBusClientResetAlarmMethodId = 4u;
constexpr std::uint8_t kMBusClientSynchroniseClockMethodId = 5u;
constexpr std::uint8_t kMBusClientSendDataMethodId = 6u;
constexpr std::uint8_t kMBusClientSetEncryptionKeyMethodId = 7u;
constexpr std::uint8_t kMBusClientTransferKeyMethodId = 8u;
} // namespace

const std::uint8_t CosemMBusClientObject::MaxSupportedVersion;

CosemMBusClientObject::CosemMBusClientObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& mbusPortReference,
  const CosemByteBuffer& captureDefinition,
  const CosemByteBuffer& capturePeriod,
  const CosemByteBuffer& primaryAddress,
  const CosemByteBuffer& identificationNumber,
  const CosemByteBuffer& manufacturerId,
  const CosemByteBuffer& version,
  const CosemByteBuffer& deviceType,
  const CosemByteBuffer& accessNumber,
  const CosemByteBuffer& status,
  const CosemByteBuffer& alarm,
  const CosemByteBuffer& configuration,
  const CosemByteBuffer& encryptionKeyStatus,
  AttributeAccessMode mutableAccess)
  : CosemMBusClientObject(
      logicalName, mbusPortReference, captureDefinition,
      capturePeriod, primaryAddress, identificationNumber,
      manufacturerId, version, deviceType, accessNumber, status,
      alarm, configuration, encryptionKeyStatus, mutableAccess,
      CosemMBusClientObject::MaxSupportedVersion)
{
}

CosemMBusClientObject::CosemMBusClientObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& mbusPortReference,
  const CosemByteBuffer& captureDefinition,
  const CosemByteBuffer& capturePeriod,
  const CosemByteBuffer& primaryAddress,
  const CosemByteBuffer& identificationNumber,
  const CosemByteBuffer& manufacturerId,
  const CosemByteBuffer& version,
  const CosemByteBuffer& deviceType,
  const CosemByteBuffer& accessNumber,
  const CosemByteBuffer& status,
  const CosemByteBuffer& alarm,
  const CosemByteBuffer& configuration,
  const CosemByteBuffer& encryptionKeyStatus,
  AttributeAccessMode mutableAccess,
  std::uint8_t version_)
  : descriptor_(MakeDescriptor(
      kMBusClientClassId,
      NormalizeVersion(
        version_, CosemMBusClientObject::MaxSupportedVersion),
      logicalName))
  , mbusPortReference_(mbusPortReference)
  , captureDefinition_(captureDefinition)
  , capturePeriod_(capturePeriod)
  , primaryAddress_(primaryAddress)
  , identificationNumber_(identificationNumber)
  , manufacturerId_(manufacturerId)
  , version_(version)
  , deviceType_(deviceType)
  , accessNumber_(accessNumber)
  , status_(status)
  , alarm_(alarm)
  , configuration_(configuration)
  , encryptionKeyStatus_(encryptionKeyStatus)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kMBusClientMbusPortReferenceAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientCaptureDefinitionAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientCapturePeriodAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientPrimaryAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientIdentificationNumberAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientManufacturerIdAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientVersionAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientDeviceTypeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientAccessNumberAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientAlarmAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientConfigurationAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusClientEncryptionKeyStatusAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemMBusClientObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemMBusClientObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemMBusClientObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kMBusClientMbusPortReferenceAttributeId:
      output = mbusPortReference_;
      return CosemStatus::Ok;
    case kMBusClientCaptureDefinitionAttributeId:
      output = captureDefinition_;
      return CosemStatus::Ok;
    case kMBusClientCapturePeriodAttributeId:
      output = capturePeriod_;
      return CosemStatus::Ok;
    case kMBusClientPrimaryAddressAttributeId:
      output = primaryAddress_;
      return CosemStatus::Ok;
    case kMBusClientIdentificationNumberAttributeId:
      output = identificationNumber_;
      return CosemStatus::Ok;
    case kMBusClientManufacturerIdAttributeId:
      output = manufacturerId_;
      return CosemStatus::Ok;
    case kMBusClientVersionAttributeId:
      output = version_;
      return CosemStatus::Ok;
    case kMBusClientDeviceTypeAttributeId:
      output = deviceType_;
      return CosemStatus::Ok;
    case kMBusClientAccessNumberAttributeId:
      output = accessNumber_;
      return CosemStatus::Ok;
    case kMBusClientStatusAttributeId:
      output = status_;
      return CosemStatus::Ok;
    case kMBusClientAlarmAttributeId:
      output = alarm_;
      return CosemStatus::Ok;
    case kMBusClientConfigurationAttributeId:
      output = configuration_;
      return CosemStatus::Ok;
    case kMBusClientEncryptionKeyStatusAttributeId:
      output = encryptionKeyStatus_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemMBusClientObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kMBusClientMbusPortReferenceAttributeId:
      target = &mbusPortReference_;
      break;
    case kMBusClientCaptureDefinitionAttributeId:
      target = &captureDefinition_;
      break;
    case kMBusClientCapturePeriodAttributeId:
      target = &capturePeriod_;
      break;
    case kMBusClientPrimaryAddressAttributeId:
      target = &primaryAddress_;
      break;
    case kMBusClientIdentificationNumberAttributeId:
      target = &identificationNumber_;
      break;
    case kMBusClientManufacturerIdAttributeId:
      target = &manufacturerId_;
      break;
    case kMBusClientVersionAttributeId:
      target = &version_;
      break;
    case kMBusClientDeviceTypeAttributeId:
      target = &deviceType_;
      break;
    case kMBusClientAccessNumberAttributeId:
      target = &accessNumber_;
      break;
    case kMBusClientStatusAttributeId:
      target = &status_;
      break;
    case kMBusClientAlarmAttributeId:
      target = &alarm_;
      break;
    case kMBusClientConfigurationAttributeId:
      target = &configuration_;
      break;
    case kMBusClientEncryptionKeyStatusAttributeId:
      target = &encryptionKeyStatus_;
      break;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  *target = input;
  return CosemStatus::Ok;
}

CosemStatus CosemMBusClientObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kMBusClientSlaveInstallMethodId ||
      methodId == kMBusClientSlaveDeinstallMethodId ||
      methodId == kMBusClientCaptureMethodId ||
      methodId == kMBusClientResetAlarmMethodId ||
      methodId == kMBusClientSynchroniseClockMethodId ||
      methodId == kMBusClientSendDataMethodId ||
      methodId == kMBusClientSetEncryptionKeyMethodId ||
      methodId == kMBusClientTransferKeyMethodId) {
    // M-Bus client slave install/deinstall, capture, reset alarm,
    // clock synchronisation, data send and encryption key
    // distribution are not exposed by the built-in object; the
    // backend is expected to perform those operations out-of-band
    // and republish stored buffers (identification, status,
    // alarm, configuration, key status).
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemMBusClientObject::MBusPortReference() const
{
  return mbusPortReference_;
}

const CosemByteBuffer& CosemMBusClientObject::CaptureDefinition() const
{
  return captureDefinition_;
}

const CosemByteBuffer& CosemMBusClientObject::CapturePeriod() const
{
  return capturePeriod_;
}

const CosemByteBuffer& CosemMBusClientObject::PrimaryAddress() const
{
  return primaryAddress_;
}

const CosemByteBuffer&
CosemMBusClientObject::IdentificationNumber() const
{
  return identificationNumber_;
}

const CosemByteBuffer& CosemMBusClientObject::ManufacturerId() const
{
  return manufacturerId_;
}

const CosemByteBuffer& CosemMBusClientObject::Version() const
{
  return version_;
}

const CosemByteBuffer& CosemMBusClientObject::DeviceType() const
{
  return deviceType_;
}

const CosemByteBuffer& CosemMBusClientObject::AccessNumber() const
{
  return accessNumber_;
}

const CosemByteBuffer& CosemMBusClientObject::Status() const
{
  return status_;
}

const CosemByteBuffer& CosemMBusClientObject::Alarm() const
{
  return alarm_;
}

const CosemByteBuffer& CosemMBusClientObject::Configuration() const
{
  return configuration_;
}

const CosemByteBuffer&
CosemMBusClientObject::EncryptionKeyStatus() const
{
  return encryptionKeyStatus_;
}

namespace {
constexpr std::uint16_t kMBusMasterPortSetupClassId = 73u;
constexpr std::uint8_t kMBusMasterPortSetupCommSpeedAttributeId = 2u;
} // namespace

const std::uint8_t CosemMBusMasterPortSetupObject::MaxSupportedVersion;

CosemMBusMasterPortSetupObject::CosemMBusMasterPortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& commSpeed,
  AttributeAccessMode mutableAccess)
  : CosemMBusMasterPortSetupObject(
      logicalName, commSpeed, mutableAccess,
      CosemMBusMasterPortSetupObject::MaxSupportedVersion)
{
}

CosemMBusMasterPortSetupObject::CosemMBusMasterPortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& commSpeed,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kMBusMasterPortSetupClassId,
      NormalizeVersion(
        version,
        CosemMBusMasterPortSetupObject::MaxSupportedVersion),
      logicalName))
  , commSpeed_(commSpeed)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kMBusMasterPortSetupCommSpeedAttributeId, mutableAccess);
}

CosemObjectDescriptor
CosemMBusMasterPortSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemMBusMasterPortSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemMBusMasterPortSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kMBusMasterPortSetupCommSpeedAttributeId:
      output = commSpeed_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemMBusMasterPortSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId)
    return CosemStatus::AccessDenied;
  if (attributeId != kMBusMasterPortSetupCommSpeedAttributeId)
    return CosemStatus::AttributeNotFound;
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  commSpeed_ = input;
  return CosemStatus::Ok;
}

CosemStatus CosemMBusMasterPortSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IC v0 defines no methods.
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemMBusMasterPortSetupObject::CommSpeed() const
{
  return commSpeed_;
}

namespace {
constexpr std::uint16_t kMBusDiagnosticClassId = 77u;
constexpr std::uint8_t
  kMBusDiagnosticReceivedSignalQualityAttributeId = 2u;
constexpr std::uint8_t
  kMBusDiagnosticTransmitterSignalQualityAttributeId = 3u;
constexpr std::uint8_t kMBusDiagnosticBbcAttributeId = 4u;
constexpr std::uint8_t
  kMBusDiagnosticFcsOkFramesCounterAttributeId = 5u;
constexpr std::uint8_t
  kMBusDiagnosticFcsNokFramesCounterAttributeId = 6u;
constexpr std::uint8_t kMBusDiagnosticCaptureTimeAttributeId = 7u;
} // namespace

const std::uint8_t CosemMBusDiagnosticObject::MaxSupportedVersion;

CosemMBusDiagnosticObject::CosemMBusDiagnosticObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& receivedSignalQuality,
  const CosemByteBuffer& transmitterSignalQuality,
  const CosemByteBuffer& bbc,
  const CosemByteBuffer& fcsOkFramesCounter,
  const CosemByteBuffer& fcsNokFramesCounter,
  const CosemByteBuffer& captureTime,
  AttributeAccessMode mutableAccess)
  : CosemMBusDiagnosticObject(
      logicalName, receivedSignalQuality, transmitterSignalQuality,
      bbc, fcsOkFramesCounter, fcsNokFramesCounter, captureTime,
      mutableAccess,
      CosemMBusDiagnosticObject::MaxSupportedVersion)
{
}

CosemMBusDiagnosticObject::CosemMBusDiagnosticObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& receivedSignalQuality,
  const CosemByteBuffer& transmitterSignalQuality,
  const CosemByteBuffer& bbc,
  const CosemByteBuffer& fcsOkFramesCounter,
  const CosemByteBuffer& fcsNokFramesCounter,
  const CosemByteBuffer& captureTime,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kMBusDiagnosticClassId,
      NormalizeVersion(
        version, CosemMBusDiagnosticObject::MaxSupportedVersion),
      logicalName))
  , receivedSignalQuality_(receivedSignalQuality)
  , transmitterSignalQuality_(transmitterSignalQuality)
  , bbc_(bbc)
  , fcsOkFramesCounter_(fcsOkFramesCounter)
  , fcsNokFramesCounter_(fcsNokFramesCounter)
  , captureTime_(captureTime)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kMBusDiagnosticReceivedSignalQualityAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticTransmitterSignalQualityAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticBbcAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticFcsOkFramesCounterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticFcsNokFramesCounterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticCaptureTimeAttributeId, mutableAccess);
}

CosemObjectDescriptor
CosemMBusDiagnosticObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemMBusDiagnosticObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemMBusDiagnosticObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kMBusDiagnosticReceivedSignalQualityAttributeId:
      output = receivedSignalQuality_;
      return CosemStatus::Ok;
    case kMBusDiagnosticTransmitterSignalQualityAttributeId:
      output = transmitterSignalQuality_;
      return CosemStatus::Ok;
    case kMBusDiagnosticBbcAttributeId:
      output = bbc_;
      return CosemStatus::Ok;
    case kMBusDiagnosticFcsOkFramesCounterAttributeId:
      output = fcsOkFramesCounter_;
      return CosemStatus::Ok;
    case kMBusDiagnosticFcsNokFramesCounterAttributeId:
      output = fcsNokFramesCounter_;
      return CosemStatus::Ok;
    case kMBusDiagnosticCaptureTimeAttributeId:
      output = captureTime_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemMBusDiagnosticObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kMBusDiagnosticReceivedSignalQualityAttributeId:
      target = &receivedSignalQuality_;
      break;
    case kMBusDiagnosticTransmitterSignalQualityAttributeId:
      target = &transmitterSignalQuality_;
      break;
    case kMBusDiagnosticBbcAttributeId:
      target = &bbc_;
      break;
    case kMBusDiagnosticFcsOkFramesCounterAttributeId:
      target = &fcsOkFramesCounter_;
      break;
    case kMBusDiagnosticFcsNokFramesCounterAttributeId:
      target = &fcsNokFramesCounter_;
      break;
    case kMBusDiagnosticCaptureTimeAttributeId:
      target = &captureTime_;
      break;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  *target = input;
  return CosemStatus::Ok;
}

CosemStatus CosemMBusDiagnosticObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IC v0 defines no methods.
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::ReceivedSignalQuality() const
{
  return receivedSignalQuality_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::TransmitterSignalQuality() const
{
  return transmitterSignalQuality_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::Bbc() const
{
  return bbc_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::FcsOkFramesCounter() const
{
  return fcsOkFramesCounter_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::FcsNokFramesCounter() const
{
  return fcsNokFramesCounter_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::CaptureTime() const
{
  return captureTime_;
}

namespace {
constexpr std::uint16_t kPrimePlcMacSetupClassId = 80u;
constexpr std::uint8_t kPrimePlcMacSetupMacMinConWindowId = 2u;
constexpr std::uint8_t kPrimePlcMacSetupMacMaxConWindowId = 3u;
constexpr std::uint8_t
  kPrimePlcMacSetupMacChannelAccessFairnessLimitId = 4u;
constexpr std::uint8_t kPrimePlcMacSetupMacEmaId = 5u;
constexpr std::uint8_t kPrimePlcMacSetupMacSarSizeId = 6u;
constexpr std::uint8_t kPrimePlcMacSetupMacMaxPduSizeId = 7u;
constexpr std::uint8_t
  kPrimePlcMacSetupMacMinSwitchSearchTimeId = 8u;
constexpr std::uint8_t kPrimePlcMacSetupMacMaxPromotionPduId = 9u;
constexpr std::uint8_t
  kPrimePlcMacSetupMacPromotionPduTxPeriodId = 10u;
constexpr std::uint8_t kPrimePlcMacSetupMacBeaconsPerFrameId = 11u;
constexpr std::uint8_t kPrimePlcMacSetupMacScpMaxTxAttemptsId = 12u;
constexpr std::uint8_t kPrimePlcMacSetupMacCtlReTxTimerId = 13u;
constexpr std::uint8_t kPrimePlcMacSetupMacMaxLnidId = 14u;
} // namespace

const std::uint8_t CosemPrimePlcMacSetupObject::MaxSupportedVersion;

CosemPrimePlcMacSetupObject::CosemPrimePlcMacSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& macMinConWindow,
  const CosemByteBuffer& macMaxConWindow,
  const CosemByteBuffer& macChannelAccessFairnessLimit,
  const CosemByteBuffer& macEma,
  const CosemByteBuffer& macSarSize,
  const CosemByteBuffer& macMaxPduSize,
  const CosemByteBuffer& macMinSwitchSearchTime,
  const CosemByteBuffer& macMaxPromotionPdu,
  const CosemByteBuffer& macPromotionPduTxPeriod,
  const CosemByteBuffer& macBeaconsPerFrame,
  const CosemByteBuffer& macScpMaxTxAttempts,
  const CosemByteBuffer& macCtlReTxTimer,
  const CosemByteBuffer& macMaxLnid,
  AttributeAccessMode mutableAccess)
  : CosemPrimePlcMacSetupObject(
      logicalName, macMinConWindow, macMaxConWindow,
      macChannelAccessFairnessLimit, macEma, macSarSize,
      macMaxPduSize, macMinSwitchSearchTime, macMaxPromotionPdu,
      macPromotionPduTxPeriod, macBeaconsPerFrame,
      macScpMaxTxAttempts, macCtlReTxTimer, macMaxLnid,
      mutableAccess,
      CosemPrimePlcMacSetupObject::MaxSupportedVersion)
{
}

CosemPrimePlcMacSetupObject::CosemPrimePlcMacSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& macMinConWindow,
  const CosemByteBuffer& macMaxConWindow,
  const CosemByteBuffer& macChannelAccessFairnessLimit,
  const CosemByteBuffer& macEma,
  const CosemByteBuffer& macSarSize,
  const CosemByteBuffer& macMaxPduSize,
  const CosemByteBuffer& macMinSwitchSearchTime,
  const CosemByteBuffer& macMaxPromotionPdu,
  const CosemByteBuffer& macPromotionPduTxPeriod,
  const CosemByteBuffer& macBeaconsPerFrame,
  const CosemByteBuffer& macScpMaxTxAttempts,
  const CosemByteBuffer& macCtlReTxTimer,
  const CosemByteBuffer& macMaxLnid,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPrimePlcMacSetupClassId,
      NormalizeVersion(
        version,
        CosemPrimePlcMacSetupObject::MaxSupportedVersion),
      logicalName))
  , macMinConWindow_(macMinConWindow)
  , macMaxConWindow_(macMaxConWindow)
  , macChannelAccessFairnessLimit_(macChannelAccessFairnessLimit)
  , macEma_(macEma)
  , macSarSize_(macSarSize)
  , macMaxPduSize_(macMaxPduSize)
  , macMinSwitchSearchTime_(macMinSwitchSearchTime)
  , macMaxPromotionPdu_(macMaxPromotionPdu)
  , macPromotionPduTxPeriod_(macPromotionPduTxPeriod)
  , macBeaconsPerFrame_(macBeaconsPerFrame)
  , macScpMaxTxAttempts_(macScpMaxTxAttempts)
  , macCtlReTxTimer_(macCtlReTxTimer)
  , macMaxLnid_(macMaxLnid)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr :
       {kPrimePlcMacSetupMacMinConWindowId,
        kPrimePlcMacSetupMacMaxConWindowId,
        kPrimePlcMacSetupMacChannelAccessFairnessLimitId,
        kPrimePlcMacSetupMacEmaId,
        kPrimePlcMacSetupMacSarSizeId,
        kPrimePlcMacSetupMacMaxPduSizeId,
        kPrimePlcMacSetupMacMinSwitchSearchTimeId,
        kPrimePlcMacSetupMacMaxPromotionPduId,
        kPrimePlcMacSetupMacPromotionPduTxPeriodId,
        kPrimePlcMacSetupMacBeaconsPerFrameId,
        kPrimePlcMacSetupMacScpMaxTxAttemptsId,
        kPrimePlcMacSetupMacCtlReTxTimerId,
        kPrimePlcMacSetupMacMaxLnidId}) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor
CosemPrimePlcMacSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemPrimePlcMacSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemPrimePlcMacSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacMinConWindowId:
      output = macMinConWindow_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacMaxConWindowId:
      output = macMaxConWindow_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacChannelAccessFairnessLimitId:
      output = macChannelAccessFairnessLimit_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacEmaId:
      output = macEma_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacSarSizeId:
      output = macSarSize_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacMaxPduSizeId:
      output = macMaxPduSize_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacMinSwitchSearchTimeId:
      output = macMinSwitchSearchTime_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacMaxPromotionPduId:
      output = macMaxPromotionPdu_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacPromotionPduTxPeriodId:
      output = macPromotionPduTxPeriod_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacBeaconsPerFrameId:
      output = macBeaconsPerFrame_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacScpMaxTxAttemptsId:
      output = macScpMaxTxAttempts_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacCtlReTxTimerId:
      output = macCtlReTxTimer_;
      return CosemStatus::Ok;
    case kPrimePlcMacSetupMacMaxLnidId:
      output = macMaxLnid_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPrimePlcMacSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kPrimePlcMacSetupMacMinConWindowId:
      target = &macMinConWindow_;
      break;
    case kPrimePlcMacSetupMacMaxConWindowId:
      target = &macMaxConWindow_;
      break;
    case kPrimePlcMacSetupMacChannelAccessFairnessLimitId:
      target = &macChannelAccessFairnessLimit_;
      break;
    case kPrimePlcMacSetupMacEmaId:
      target = &macEma_;
      break;
    case kPrimePlcMacSetupMacSarSizeId:
      target = &macSarSize_;
      break;
    case kPrimePlcMacSetupMacMaxPduSizeId:
      target = &macMaxPduSize_;
      break;
    case kPrimePlcMacSetupMacMinSwitchSearchTimeId:
      target = &macMinSwitchSearchTime_;
      break;
    case kPrimePlcMacSetupMacMaxPromotionPduId:
      target = &macMaxPromotionPdu_;
      break;
    case kPrimePlcMacSetupMacPromotionPduTxPeriodId:
      target = &macPromotionPduTxPeriod_;
      break;
    case kPrimePlcMacSetupMacBeaconsPerFrameId:
      target = &macBeaconsPerFrame_;
      break;
    case kPrimePlcMacSetupMacScpMaxTxAttemptsId:
      target = &macScpMaxTxAttempts_;
      break;
    case kPrimePlcMacSetupMacCtlReTxTimerId:
      target = &macCtlReTxTimer_;
      break;
    case kPrimePlcMacSetupMacMaxLnidId:
      target = &macMaxLnid_;
      break;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  *target = input;
  return CosemStatus::Ok;
}

CosemStatus CosemPrimePlcMacSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IC v0 defines no methods.
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacMinConWindow() const
{
  return macMinConWindow_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacMaxConWindow() const
{
  return macMaxConWindow_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacChannelAccessFairnessLimit() const
{
  return macChannelAccessFairnessLimit_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacEma() const
{
  return macEma_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacSarSize() const
{
  return macSarSize_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacMaxPduSize() const
{
  return macMaxPduSize_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacMinSwitchSearchTime() const
{
  return macMinSwitchSearchTime_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacMaxPromotionPdu() const
{
  return macMaxPromotionPdu_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacPromotionPduTxPeriod() const
{
  return macPromotionPduTxPeriod_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacBeaconsPerFrame() const
{
  return macBeaconsPerFrame_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacScpMaxTxAttempts() const
{
  return macScpMaxTxAttempts_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacCtlReTxTimer() const
{
  return macCtlReTxTimer_;
}

const CosemByteBuffer&
CosemPrimePlcMacSetupObject::MacMaxLnid() const
{
  return macMaxLnid_;
}


const std::uint8_t CosemClockObject::MaxSupportedVersion;

CosemClockObject::CosemClockObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& time,
  std::int16_t timeZone,
  std::uint8_t status,
  const CosemByteBuffer& daylightSavingsBegin,
  const CosemByteBuffer& daylightSavingsEnd,
  std::int8_t daylightSavingsDeviation,
  bool daylightSavingsEnabled,
  CosemClockBase clockBase)
  : CosemClockObject(
      logicalName,
      time,
      timeZone,
      status,
      daylightSavingsBegin,
      daylightSavingsEnd,
      daylightSavingsDeviation,
      daylightSavingsEnabled,
      clockBase,
      kVersion0)
{
}

CosemClockObject::CosemClockObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& time,
  std::int16_t timeZone,
  std::uint8_t status,
  const CosemByteBuffer& daylightSavingsBegin,
  const CosemByteBuffer& daylightSavingsEnd,
  std::int8_t daylightSavingsDeviation,
  bool daylightSavingsEnabled,
  CosemClockBase clockBase,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kClockClassId,
      NormalizeVersion(version, CosemClockObject::MaxSupportedVersion),
      logicalName))
  , time_(time)
  , timeZone_(timeZone)
  , status_(status)
  , daylightSavingsBegin_(daylightSavingsBegin)
  , daylightSavingsEnd_(daylightSavingsEnd)
  , daylightSavingsDeviation_(daylightSavingsDeviation)
  , daylightSavingsEnabled_(daylightSavingsEnabled)
  , clockBase_(clockBase)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kClockTimeAttributeId,
    AttributeAccessMode::ReadAndWrite);
  rights_.SetAttributeAccess(
    kClockTimeZoneAttributeId,
    AttributeAccessMode::ReadAndWrite);
  rights_.SetAttributeAccess(
    kClockStatusAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kClockDaylightSavingsBeginAttributeId,
    AttributeAccessMode::ReadAndWrite);
  rights_.SetAttributeAccess(
    kClockDaylightSavingsEndAttributeId,
    AttributeAccessMode::ReadAndWrite);
  rights_.SetAttributeAccess(
    kClockDaylightSavingsDeviationAttributeId,
    AttributeAccessMode::ReadAndWrite);
  rights_.SetAttributeAccess(
    kClockDaylightSavingsEnabledAttributeId,
    AttributeAccessMode::ReadAndWrite);
  rights_.SetAttributeAccess(
    kClockBaseAttributeId,
    AttributeAccessMode::ReadAndWrite);
  rights_.SetMethodAccess(
    kClockAdjustToQuarterMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kClockAdjustToMeasuringPeriodMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kClockAdjustToMinuteMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kClockAdjustToPresetTimeMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kClockPresetAdjustingTimeMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(kClockShiftTimeMethodId, MethodAccessMode::Access);
}

CosemObjectDescriptor CosemClockObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemClockObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemClockObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  output.clear();
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockTimeAttributeId) {
    AppendBufferOctetString(output, time_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockTimeZoneAttributeId) {
    AppendLong(output, timeZone_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockStatusAttributeId) {
    AppendUnsigned(output, status_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsBeginAttributeId) {
    AppendBufferOctetString(output, daylightSavingsBegin_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsEndAttributeId) {
    AppendBufferOctetString(output, daylightSavingsEnd_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsDeviationAttributeId) {
    AppendInteger(
      output,
      static_cast<std::uint8_t>(daylightSavingsDeviation_));
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsEnabledAttributeId) {
    AppendBoolean(output, daylightSavingsEnabled_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockBaseAttributeId) {
    AppendEnum(output, static_cast<std::uint8_t>(clockBase_));
    return CosemStatus::Ok;
  }

  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemClockObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId ||
      attributeId == kClockStatusAttributeId) {
    return CosemStatus::AccessDenied;
  }
  if (attributeId == kClockTimeAttributeId) {
    CosemByteBuffer value;
    if (!DecodeExactOctetString(input, kClockDateTimeOctetStringSize, value)) {
      return CosemStatus::InvalidArgument;
    }
    time_ = value;
    return CosemStatus::Ok;
  }
  if (attributeId == kClockTimeZoneAttributeId) {
    std::int16_t value = 0;
    std::size_t offset = 0u;
    if (!ReadLongValue(input, offset, value) || offset != input.size()) {
      return CosemStatus::InvalidArgument;
    }
    timeZone_ = value;
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsBeginAttributeId) {
    CosemByteBuffer value;
    if (!DecodeExactOctetString(input, kClockDateTimeOctetStringSize, value)) {
      return CosemStatus::InvalidArgument;
    }
    daylightSavingsBegin_ = value;
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsEndAttributeId) {
    CosemByteBuffer value;
    if (!DecodeExactOctetString(input, kClockDateTimeOctetStringSize, value)) {
      return CosemStatus::InvalidArgument;
    }
    daylightSavingsEnd_ = value;
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsDeviationAttributeId) {
    std::uint8_t value = 0u;
    std::size_t offset = 0u;
    if (!ReadIntegerValue(input, offset, value) || offset != input.size()) {
      return CosemStatus::InvalidArgument;
    }
    daylightSavingsDeviation_ = static_cast<std::int8_t>(value);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsEnabledAttributeId) {
    bool value = false;
    std::size_t offset = 0u;
    if (!ReadBooleanValue(input, offset, value) || offset != input.size()) {
      return CosemStatus::InvalidArgument;
    }
    daylightSavingsEnabled_ = value;
    return CosemStatus::Ok;
  }
  if (attributeId == kClockBaseAttributeId) {
    std::uint8_t value = 0u;
    std::size_t offset = 0u;
    if (!ReadEnumValue(input, offset, value) ||
        offset != input.size() ||
        value > static_cast<std::uint8_t>(CosemClockBase::RadioControlled)) {
      return CosemStatus::InvalidArgument;
    }
    clockBase_ = static_cast<CosemClockBase>(value);
    return CosemStatus::Ok;
  }

  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemClockObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId >= kClockAdjustToQuarterMethodId &&
      methodId <= kClockShiftTimeMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemClockObject::Time() const
{
  return time_;
}

std::int16_t CosemClockObject::TimeZone() const
{
  return timeZone_;
}

std::uint8_t CosemClockObject::Status() const
{
  return status_;
}

const CosemByteBuffer& CosemClockObject::DaylightSavingsBegin() const
{
  return daylightSavingsBegin_;
}

const CosemByteBuffer& CosemClockObject::DaylightSavingsEnd() const
{
  return daylightSavingsEnd_;
}

std::int8_t CosemClockObject::DaylightSavingsDeviation() const
{
  return daylightSavingsDeviation_;
}

bool CosemClockObject::DaylightSavingsEnabled() const
{
  return daylightSavingsEnabled_;
}

CosemClockBase CosemClockObject::ClockBase() const
{
  return clockBase_;
}

const std::uint8_t CosemProfileGenericObject::MaxSupportedVersion;

CosemProfileGenericObject::CosemProfileGenericObject(
  const CosemLogicalName& logicalName,
  const std::vector<CosemByteBuffer>& bufferRows,
  const std::vector<CosemCaptureObject>& captureObjects,
  std::uint32_t capturePeriod,
  std::uint32_t profileEntries)
  : CosemProfileGenericObject(
      logicalName,
      bufferRows,
      captureObjects,
      capturePeriod,
      profileEntries,
      kProfileGenericVersion)
{
}

CosemProfileGenericObject::CosemProfileGenericObject(
  const CosemLogicalName& logicalName,
  const std::vector<CosemByteBuffer>& bufferRows,
  const std::vector<CosemCaptureObject>& captureObjects,
  std::uint32_t capturePeriod,
  std::uint32_t profileEntries,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kProfileGenericClassId,
      NormalizeVersion(version, CosemProfileGenericObject::MaxSupportedVersion),
      logicalName))
  , bufferRows_(bufferRows)
  , captureObjects_(captureObjects)
  , capturePeriod_(capturePeriod)
  , profileEntries_(profileEntries)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kProfileBufferAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kProfileCaptureObjectsAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kProfileCapturePeriodAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kProfileSortMethodAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kProfileSortObjectAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kProfileEntriesInUseAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kProfileProfileEntriesAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetMethodAccess(kProfileResetMethodId, MethodAccessMode::Access);
  rights_.SetMethodAccess(kProfileCaptureMethodId, MethodAccessMode::Access);
  if (descriptor_.key.version == 0u) {
    rights_.SetMethodAccess(
      kProfileGetBufferByRangeMethodId,
      MethodAccessMode::Access);
    rights_.SetMethodAccess(
      kProfileGetBufferByIndexMethodId,
      MethodAccessMode::Access);
  }
}

CosemObjectDescriptor CosemProfileGenericObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemProfileGenericObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemProfileGenericObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileBufferAttributeId) {
    output = EncodeProfileGenericBuffer(bufferRows_);
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileCaptureObjectsAttributeId) {
    output = EncodeProfileGenericCaptureObjects(captureObjects_);
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileCapturePeriodAttributeId) {
    output.clear();
    AppendDoubleLongUnsigned(output, capturePeriod_);
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileSortMethodAttributeId) {
    output.clear();
    AppendEnum(
      output,
      static_cast<std::uint8_t>(CosemProfileGenericSortMethod::Fifo));
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileSortObjectAttributeId) {
    output.clear();
    if (captureObjects_.empty()) {
      CosemCaptureObject empty;
      empty.object.classId = 0u;
      empty.object.version = 0u;
      empty.object.logicalName = CosemLogicalName(0u, 0u, 0u, 0u, 0u, 0u);
      empty.attributeId = 0u;
      empty.dataIndex = 0u;
      output = EncodeProfileGenericCaptureObject(empty);
    } else {
      output = EncodeProfileGenericCaptureObject(captureObjects_[0]);
    }
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileEntriesInUseAttributeId) {
    output.clear();
    AppendDoubleLongUnsigned(
      output,
      static_cast<std::uint32_t>(bufferRows_.size()));
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileProfileEntriesAttributeId) {
    output.clear();
    AppendDoubleLongUnsigned(output, profileEntries_);
    return CosemStatus::Ok;
  }

  output.clear();
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemProfileGenericObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  (void)input;
  if (attributeId >= kLogicalNameAttributeId &&
      attributeId <= kProfileProfileEntriesAttributeId) {
    return CosemStatus::AccessDenied;
  }
  return CosemStatus::AttributeNotFound;
}

CosemStatus CosemProfileGenericObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kProfileResetMethodId ||
      methodId == kProfileCaptureMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  if (descriptor_.key.version == 0u &&
      methodId >= kProfileGetBufferByRangeMethodId &&
      methodId <= kProfileGetBufferByIndexMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const std::vector<CosemByteBuffer>&
CosemProfileGenericObject::BufferRows() const
{
  return bufferRows_;
}

const std::vector<CosemCaptureObject>&
CosemProfileGenericObject::CaptureObjects() const
{
  return captureObjects_;
}

const std::uint8_t CosemAssociationLnObject::MaxSupportedVersion;

CosemAssociationLnObject::CosemAssociationLnObject(
  const CosemLogicalName& logicalName,
  const AssociationView& objectList)
  : CosemAssociationLnObject(
      logicalName,
      objectList,
      kVersion0,
      CosemAssociationStatus::Associated)
{
}

CosemAssociationLnObject::CosemAssociationLnObject(
  const CosemLogicalName& logicalName,
  const AssociationView& objectList,
  std::uint8_t version)
  : CosemAssociationLnObject(
      logicalName,
      objectList,
      version,
      CosemAssociationStatus::Associated)
{
}

CosemAssociationLnObject::CosemAssociationLnObject(
  const CosemLogicalName& logicalName,
  const AssociationView& objectList,
  CosemAssociationStatus associationStatus)
  : CosemAssociationLnObject(
      logicalName,
      objectList,
      kVersion0,
      associationStatus)
{
}

CosemAssociationLnObject::CosemAssociationLnObject(
  const CosemLogicalName& logicalName,
  const AssociationView& objectList,
  std::uint8_t version,
  CosemAssociationStatus associationStatus)
  : CosemAssociationLnObject(
      logicalName,
      objectList,
      CosemAssociationLnConfig{
        version,
        associationStatus,
        false,
        CosemLogicalName(),
        std::vector<CosemAssociationUser>(),
        DefaultAssociationUser()})
{
}

CosemAssociationLnObject::CosemAssociationLnObject(
  const CosemLogicalName& logicalName,
  const AssociationView& objectList,
  CosemAssociationStatus associationStatus,
  const CosemLogicalName& securitySetupReference)
  : CosemAssociationLnObject(
      logicalName,
      objectList,
      CosemAssociationLnConfig{
        1u,
        associationStatus,
        true,
        securitySetupReference,
        std::vector<CosemAssociationUser>(),
        DefaultAssociationUser()})
{
}

CosemAssociationLnObject::CosemAssociationLnObject(
  const CosemLogicalName& logicalName,
  const AssociationView& objectList,
  const CosemAssociationLnConfig& config)
  : descriptor_(
      MakeDescriptor(
        kAssociationLnClassId,
        NormalizeAssociationLnVersion(config.version),
        logicalName))
  , objectList_(objectList)
  , associationStatus_(config.associationStatus)
  , hasSecuritySetupReference_(config.hasSecuritySetupReference)
  , securitySetupReference_(config.securitySetupReference)
  , users_(config.users)
  , currentUser_(config.currentUser)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(kValueAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kAssociationStatusAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetMethodAccess(
    kAssociationReplyToHlsAuthenticationMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kAssociationChangeHlsSecretMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kAssociationAddObjectMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kAssociationRemoveObjectMethodId,
    MethodAccessMode::Access);
  if (descriptor_.key.version >= 1u && hasSecuritySetupReference_) {
    rights_.SetAttributeAccess(
      kAssociationSecuritySetupReferenceAttributeId,
      AttributeAccessMode::ReadOnly);
  }
  if (descriptor_.key.version >= 2u) {
    rights_.SetAttributeAccess(
      kAssociationUserListAttributeId,
      AttributeAccessMode::ReadOnly);
    rights_.SetAttributeAccess(
      kAssociationCurrentUserAttributeId,
      AttributeAccessMode::ReadOnly);
    rights_.SetMethodAccess(
      kAssociationAddUserMethodId,
      MethodAccessMode::Access);
    rights_.SetMethodAccess(
      kAssociationRemoveUserMethodId,
      MethodAccessMode::Access);
  }
  if (descriptor_.key.version == 0u && hasSecuritySetupReference_) {
    hasSecuritySetupReference_ = false;
    securitySetupReference_ = CosemLogicalName();
  }
  if (descriptor_.key.version < 2u) {
    users_.clear();
    currentUser_ = DefaultAssociationUser();
  }
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
    output = EncodeAssociationObjectList(objectList_);
    return CosemStatus::Ok;
  }
  if (attributeId == kAssociationStatusAttributeId) {
    output.clear();
    AppendEnum(output, static_cast<std::uint8_t>(associationStatus_));
    return CosemStatus::Ok;
  }
  if (attributeId == kAssociationSecuritySetupReferenceAttributeId &&
      descriptor_.key.version >= 1u &&
      hasSecuritySetupReference_) {
    output = EncodeLogicalName(securitySetupReference_);
    return CosemStatus::Ok;
  }
  if (attributeId == kAssociationUserListAttributeId &&
      descriptor_.key.version >= 2u) {
    output.clear();
    AppendAssociationUserList(output, users_);
    return CosemStatus::Ok;
  }
  if (attributeId == kAssociationCurrentUserAttributeId &&
      descriptor_.key.version >= 2u) {
    output.clear();
    AppendAssociationUser(output, currentUser_);
    return CosemStatus::Ok;
  }
  output.clear();
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
  (void)input;
  output.clear();
  if (methodId >= kAssociationReplyToHlsAuthenticationMethodId &&
      methodId <= kAssociationRemoveObjectMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  if (descriptor_.key.version >= 2u &&
      methodId >= kAssociationAddUserMethodId &&
      methodId <= kAssociationRemoveUserMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

AssociationView CosemAssociationLnObject::ObjectList() const
{
  return objectList_;
}

CosemAssociationStatus CosemAssociationLnObject::AssociationStatus() const
{
  return associationStatus_;
}

bool CosemAssociationLnObject::HasSecuritySetupReference() const
{
  return hasSecuritySetupReference_;
}

CosemLogicalName CosemAssociationLnObject::SecuritySetupReference() const
{
  return securitySetupReference_;
}

std::vector<CosemAssociationUser> CosemAssociationLnObject::Users() const
{
  return users_;
}

CosemAssociationUser CosemAssociationLnObject::CurrentUser() const
{
  return currentUser_;
}

const std::uint8_t CosemSapAssignmentObject::MaxSupportedVersion;

CosemSapAssignmentObject::CosemSapAssignmentObject(
  const CosemLogicalName& logicalName,
  const std::vector<SapAssignment>& assignments)
  : CosemSapAssignmentObject(logicalName, assignments, kVersion0)
{
}

CosemSapAssignmentObject::CosemSapAssignmentObject(
  const CosemLogicalName& logicalName,
  const std::vector<SapAssignment>& assignments,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSapAssignmentClassId,
      NormalizeVersion(version, CosemSapAssignmentObject::MaxSupportedVersion),
      logicalName))
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
  output.clear();
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
  output.clear();
  return CosemStatus::MethodNotFound;
}

std::vector<SapAssignment> CosemSapAssignmentObject::Assignments() const
{
  return assignments_;
}

const std::uint8_t CosemSecuritySetupObject::MaxSupportedVersion;

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
      static_cast<dlms::security::IMutableKeyStore*>(0),
      static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
      static_cast<ICosemCertificateStore*>(0),
      kSecuritySetupVersion)
{
}

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle,
  std::uint8_t version)
  : CosemSecuritySetupObject(
      logicalName,
      securityPolicy,
      securitySuite,
      clientSystemTitle,
      serverSystemTitle,
      static_cast<dlms::security::IMutableKeyStore*>(0),
      static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
      static_cast<ICosemCertificateStore*>(0),
      version)
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
      static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
      static_cast<ICosemCertificateStore*>(0),
      kSecuritySetupVersion)
{
}

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle,
  dlms::security::IMutableKeyStore* keyStore,
  std::uint8_t version)
  : CosemSecuritySetupObject(
      logicalName,
      securityPolicy,
      securitySuite,
      clientSystemTitle,
      serverSystemTitle,
      keyStore,
      static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
      static_cast<ICosemCertificateStore*>(0),
      version)
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
  : CosemSecuritySetupObject(
      logicalName,
      securityPolicy,
      securitySuite,
      clientSystemTitle,
      serverSystemTitle,
      keyStore,
      counterResetPolicy,
      static_cast<ICosemCertificateStore*>(0),
      kSecuritySetupVersion)
{
}

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle,
  dlms::security::IMutableKeyStore* keyStore,
  dlms::security::IInvocationCounterResetPolicy* counterResetPolicy,
  std::uint8_t version)
  : CosemSecuritySetupObject(
      logicalName,
      securityPolicy,
      securitySuite,
      clientSystemTitle,
      serverSystemTitle,
      keyStore,
      counterResetPolicy,
      static_cast<ICosemCertificateStore*>(0),
      version)
{
}

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle,
  dlms::security::IMutableKeyStore* keyStore,
  dlms::security::IInvocationCounterResetPolicy* counterResetPolicy,
  ICosemCertificateStore* certificateStore)
  : CosemSecuritySetupObject(
      logicalName,
      securityPolicy,
      securitySuite,
      clientSystemTitle,
      serverSystemTitle,
      keyStore,
      counterResetPolicy,
      certificateStore,
      kSecuritySetupVersion)
{
}

CosemSecuritySetupObject::CosemSecuritySetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t securityPolicy,
  std::uint8_t securitySuite,
  const SystemTitle& clientSystemTitle,
  const SystemTitle& serverSystemTitle,
  dlms::security::IMutableKeyStore* keyStore,
  dlms::security::IInvocationCounterResetPolicy* counterResetPolicy,
  ICosemCertificateStore* certificateStore,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSecuritySetupClassId,
      NormalizeVersion(version, CosemSecuritySetupObject::MaxSupportedVersion),
      logicalName))
  , securityPolicy_(securityPolicy)
  , securitySuite_(securitySuite)
  , clientSystemTitle_(clientSystemTitle)
  , serverSystemTitle_(serverSystemTitle)
  , keyStore_(keyStore)
  , counterResetPolicy_(counterResetPolicy)
  , certificateStore_(certificateStore)
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
  if (descriptor_.key.version >= 1u) {
    rights_.SetAttributeAccess(
      kSecurityCertificatesAttributeId,
      AttributeAccessMode::ReadOnly);
  }

  const std::uint8_t lastMethodId =
    descriptor_.key.version == 0u ? kGlobalKeyTransferMethodId : 8u;
  for (std::uint8_t methodId = 1u; methodId <= lastMethodId; ++methodId) {
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
  if (attributeId == kSecurityCertificatesAttributeId &&
      descriptor_.key.version >= 1u) {
    output.clear();
    std::vector<CertificateInfoEntry> entries;
    if (certificateStore_ != 0) {
      CosemStatus listStatus = certificateStore_->List(entries);
      if (listStatus != CosemStatus::Ok) {
        return listStatus;
      }
    }
    AppendArrayHeader(output, entries.size());
    static const std::uint8_t kEmptyByte = 0u;
    for (std::size_t i = 0u; i < entries.size(); ++i) {
      const CertificateInfoEntry& entry = entries[i];
      AppendStructureHeader(output, 6u);
      AppendEnum(output, entry.entity);
      AppendEnum(output, entry.type);
      AppendOctetString(
        output,
        entry.serialNumber.empty() ? &kEmptyByte : &entry.serialNumber[0],
        entry.serialNumber.size());
      AppendOctetString(
        output,
        entry.issuer.empty() ? &kEmptyByte : &entry.issuer[0],
        entry.issuer.size());
      AppendOctetString(
        output,
        entry.subject.empty() ? &kEmptyByte : &entry.subject[0],
        entry.subject.size());
      AppendOctetString(
        output,
        entry.subjectAltName.empty() ? &kEmptyByte : &entry.subjectAltName[0],
        entry.subjectAltName.size());
    }
    return CosemStatus::Ok;
  }
  output.clear();
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
      || attributeId == kServerSystemTitleAttributeId
      || (descriptor_.key.version >= 1u &&
          attributeId == kSecurityCertificatesAttributeId)) {
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
      output.clear();
      return CosemStatus::InvalidArgument;
    }
    if (!StrengthensOrKeepsPolicy(securityPolicy_, requestedPolicy)) {
      output.clear();
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
  if (descriptor_.key.version >= 1u &&
      methodId >= 6u && methodId <= 8u) {
    output.clear();
    if (certificateStore_ == 0) {
      return CosemStatus::UnsupportedFeature;
    }

    if (methodId == 6u) {
      // import_certificate: data is octet-string with raw X.509.
      std::size_t offset = 0u;
      std::size_t length = 0u;
      const std::uint8_t* data = 0;
      if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
          !ReadAxdrLength(input, offset, length) ||
          !ReadFixedBytes(input, offset, length, data) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      CertificateInfoEntry entry;
      entry.entity = CertificateEntity_Other;
      entry.type = CertificateType_Other;
      entry.rawCertificate.assign(data, data + length);
      // We do not parse X.509 here; subject/issuer/serial stay empty.
      return certificateStore_->Import(entry);
    }

    // methods 7 (export) and 8 (remove) share the same selector structure:
    //   structure(2) {
    //     enum kind,                              // 0 = by-entity, 1 = by-serial
    //     structure(N) { ... }                    // selector payload
    //   }
    std::size_t offset = 0u;
    std::size_t outerFields = 0u;
    std::uint8_t kind = 0u;
    if (!ReadExpectedTag(input, offset, kStructureTag) ||
        !ReadAxdrLength(input, offset, outerFields) ||
        outerFields != 2u ||
        !ReadEnumValue(input, offset, kind)) {
      return CosemStatus::InvalidArgument;
    }

    std::size_t selectorFields = 0u;
    if (!ReadExpectedTag(input, offset, kStructureTag) ||
        !ReadAxdrLength(input, offset, selectorFields)) {
      return CosemStatus::InvalidArgument;
    }

    if (kind == 0u) {
      // by-entity: { entity:enum, type:enum, system_title:octet-string(8) }
      std::uint8_t entity = 0u;
      std::uint8_t type = 0u;
      std::size_t stLength = 0u;
      const std::uint8_t* stData = 0;
      if (selectorFields != 3u ||
          !ReadEnumValue(input, offset, entity) ||
          !ReadEnumValue(input, offset, type) ||
          !ReadExpectedTag(input, offset, kDataOctetStringTag) ||
          !ReadAxdrLength(input, offset, stLength) ||
          stLength != kSystemTitleSize ||
          !ReadFixedBytes(input, offset, stLength, stData) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      CertificateSystemTitle systemTitle;
      for (std::size_t i = 0u; i < kSystemTitleSize; ++i) {
        systemTitle[i] = stData[i];
      }
      if (methodId == 7u) {
        std::vector<std::uint8_t> raw;
        CosemStatus status = certificateStore_->ExportByEntity(
          entity, type, systemTitle, raw);
        if (status != CosemStatus::Ok) {
          return status;
        }
        static const std::uint8_t kEmpty = 0u;
        AppendOctetString(
          output,
          raw.empty() ? &kEmpty : &raw[0],
          raw.size());
        return CosemStatus::Ok;
      }
      return certificateStore_->RemoveByEntity(entity, type, systemTitle);
    }

    if (kind == 1u) {
      // by-serial: { serial_number:octet-string, issuer:octet-string }
      std::size_t snLength = 0u;
      const std::uint8_t* snData = 0;
      std::size_t issLength = 0u;
      const std::uint8_t* issData = 0;
      if (selectorFields != 2u ||
          !ReadExpectedTag(input, offset, kDataOctetStringTag) ||
          !ReadAxdrLength(input, offset, snLength) ||
          !ReadFixedBytes(input, offset, snLength, snData) ||
          !ReadExpectedTag(input, offset, kDataOctetStringTag) ||
          !ReadAxdrLength(input, offset, issLength) ||
          !ReadFixedBytes(input, offset, issLength, issData) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      std::vector<std::uint8_t> serial(snData, snData + snLength);
      std::vector<std::uint8_t> issuer(issData, issData + issLength);
      if (methodId == 7u) {
        std::vector<std::uint8_t> raw;
        CosemStatus status = certificateStore_->ExportBySerial(
          serial, issuer, raw);
        if (status != CosemStatus::Ok) {
          return status;
        }
        static const std::uint8_t kEmpty = 0u;
        AppendOctetString(
          output,
          raw.empty() ? &kEmpty : &raw[0],
          raw.size());
        return CosemStatus::Ok;
      }
      return certificateStore_->RemoveBySerial(serial, issuer);
    }

    return CosemStatus::InvalidArgument;
  }
  if (descriptor_.key.version >= 1u &&
      methodId > kGlobalKeyTransferMethodId &&
      methodId <= 8u) {
    // methods 3 (key_agreement), 4 (generate_key_pair),
    // 5 (generate_certificate_request) require an X.509 / ECDSA stack that is
    // not provided by dlms-cosem.
    output.clear();
    return CosemStatus::UnsupportedFeature;
  }
  output.clear();
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
