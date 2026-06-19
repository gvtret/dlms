#include "dlms/cosem/simple_objects.hpp"
#include "dlms/security/suite0_key_wrap.hpp"

namespace dlms {
namespace cosem {
namespace {

constexpr std::uint16_t kDataClassId = 1u;
constexpr std::uint16_t kRegisterClassId = 3u;
// IEC 62056-6-2 ED4 (2021) §4.3.2 / DLMS UA Blue Book Ed. 12.1 §4.3.2:
// class_id=3 defines a single specific method `reset` (data ::= integer(0)).
constexpr std::uint8_t kRegisterResetMethodId = 1u;
constexpr std::uint16_t kClockClassId = 8u;
constexpr std::uint16_t kProfileGenericClassId = 7u;
constexpr std::uint16_t kAssociationLnClassId = 15u;
constexpr std::uint16_t kSapAssignmentClassId = 17u;
// IEC 62056-6-2 ED4 (2021) §4.4.4 / DLMS UA Blue Book Ed. 12.1 §5.3.4:
// class_id=17 defines a single specific method.
constexpr std::uint8_t kSapAssignmentConnectLogicalDeviceMethodId = 1u;
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
constexpr std::uint8_t kBitStringTag = 0x04u;
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

void AppendDateTimeOctetString(
  CosemByteBuffer& output,
  const dlms::cosem::types::DateTime& value)
{
  const std::array<std::uint8_t, dlms::cosem::types::DateTime::WireSize>
    bytes = value.ToBytes();
  AppendOctetString(output, bytes.data(), bytes.size());
}

bool DecodeDateTimeOctetString(
  const CosemByteBuffer& input,
  dlms::cosem::types::DateTime& value)
{
  std::size_t offset = 0u;
  std::size_t length = 0u;
  const std::uint8_t* data = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, length) ||
      length != dlms::cosem::types::DateTime::WireSize ||
      !ReadFixedBytes(input, offset, length, data) ||
      offset != input.size()) {
    return false;
  }
  return dlms::cosem::types::DateTime::TryFromBytes(data, length, value);
}

// Encode the `script` structure used by IC 22.executed_script:
//   structure(2) { octet-string(6) logical_name, long-unsigned selector }
void AppendScript(
  CosemByteBuffer& output,
  const dlms::cosem::types::Script& value)
{
  AppendStructureHeader(output, 2u);
  AppendOctetString(
    output, value.LogicalName().Data(), value.LogicalName().Size());
  AppendLongUnsigned(output, value.Selector());
}

bool DecodeScript(
  const CosemByteBuffer& input,
  dlms::cosem::types::Script& value)
{
  std::size_t offset = 0u;
  std::uint8_t tag = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, count) || count != 2u) {
    return false;
  }

  dlms::cosem::CosemLogicalName logicalName;
  std::uint16_t selector = 0u;
  if (!ReadLogicalNameValue(input, offset, logicalName)) {
    return false;
  }
  if (!ReadLongUnsignedValue(input, offset, selector)) {
    return false;
  }
  if (offset != input.size()) {
    return false;
  }
  (void)tag;
  value = dlms::cosem::types::Script(logicalName, selector);
  return true;
}

// Encode IC 22.execution_time:
//   array(n) of structure(2) { octet-string(4) time, octet-string(5) date }.
void AppendExecutionTime(
  CosemByteBuffer& output,
  const std::vector<std::pair<
    dlms::cosem::types::Time, dlms::cosem::types::Date> >& entries)
{
  AppendArrayHeader(output, entries.size());
  for (std::size_t i = 0u; i < entries.size(); ++i) {
    AppendStructureHeader(output, 2u);
    const std::array<std::uint8_t, dlms::cosem::types::Time::WireSize>
      timeBytes = entries[i].first.ToBytes();
    AppendOctetString(output, timeBytes.data(), timeBytes.size());
    const std::array<std::uint8_t, dlms::cosem::types::Date::WireSize>
      dateBytes = entries[i].second.ToBytes();
    AppendOctetString(output, dateBytes.data(), dateBytes.size());
  }
}

bool DecodeExecutionTime(
  const CosemByteBuffer& input,
  std::vector<std::pair<
    dlms::cosem::types::Time, dlms::cosem::types::Date> >& entries)
{
  entries.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  entries.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    std::size_t fieldCount = 0u;
    if (!ReadExpectedTag(input, offset, kStructureTag) ||
        !ReadAxdrLength(input, offset, fieldCount) ||
        fieldCount != 2u) {
      return false;
    }
    std::size_t timeLen = 0u;
    const std::uint8_t* timeData = 0;
    if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
        !ReadAxdrLength(input, offset, timeLen) ||
        timeLen != dlms::cosem::types::Time::WireSize ||
        !ReadFixedBytes(input, offset, timeLen, timeData)) {
      return false;
    }
    dlms::cosem::types::Time time;
    if (!dlms::cosem::types::Time::TryFromBytes(timeData, timeLen, time)) {
      return false;
    }
    std::size_t dateLen = 0u;
    const std::uint8_t* dateData = 0;
    if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
        !ReadAxdrLength(input, offset, dateLen) ||
        dateLen != dlms::cosem::types::Date::WireSize ||
        !ReadFixedBytes(input, offset, dateLen, dateData)) {
      return false;
    }
    dlms::cosem::types::Date date;
    if (!dlms::cosem::types::Date::TryFromBytes(dateData, dateLen, date)) {
      return false;
    }
    entries.push_back(std::make_pair(time, date));
  }
  if (offset != input.size()) {
    return false;
  }
  return true;
}

// True when any spec wildcard sentinel is set in the date.
bool DateHasAnyWildcard(const dlms::cosem::types::Date& date)
{
  return date.YearUnspecified() || date.MonthUnspecified() ||
         date.DayOfMonthUnspecified() || date.DayOfWeekUnspecified() ||
         date.Month() == dlms::cosem::types::Date::MonthDstBeginValue ||
         date.Month() == dlms::cosem::types::Date::MonthDstEndValue ||
         date.DayOfMonth() ==
           dlms::cosem::types::Date::DayOfMonthLastValue ||
         date.DayOfMonth() ==
           dlms::cosem::types::Date::DayOfMonthSecondLastValue;
}

// AXDR bit-string codec (IEC 61334-6 / Blue Book §4.1.2):
//   tag       = 0x04
//   length    = number of *bits* (length-axdr encoded)
//   payload   = ceil(bits/8) octets, MSB-first inside each octet
//
// Bit numbering: bit index `i` (0-based) lives in octet `i/8` at bit
// position `7 - (i%8)`. This matches DLMS conventions (e.g. weekday
// where Monday = bit 0 = 0x80 of the first octet).
void AppendBitStringMsbFirst(
  CosemByteBuffer& output,
  std::uint64_t bits,
  std::uint8_t bitWidth)
{
  output.push_back(kBitStringTag);
  AppendLength(output, bitWidth);
  const std::size_t octets =
    static_cast<std::size_t>((bitWidth + 7u) / 8u);
  for (std::size_t o = 0u; o < octets; ++o) {
    std::uint8_t byte = 0u;
    for (std::uint8_t b = 0u; b < 8u; ++b) {
      const std::uint32_t bitIndex =
        static_cast<std::uint32_t>(o) * 8u + b;
      if (bitIndex >= bitWidth)
        break;
      if ((bits >> bitIndex) & std::uint64_t{1}) {
        byte = static_cast<std::uint8_t>(
          byte | (0x80u >> b));
      }
    }
    output.push_back(byte);
  }
}

// Reads a bit-string of exactly `expectedBitWidth` bits. Wider strings
// are rejected; narrower strings are accepted and zero-extended on the
// high end. Returns false on tag/length/EOF mismatch.
bool ReadBitStringMsbFirst(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::uint8_t expectedBitWidth,
  std::uint64_t& bitsOut)
{
  if (!ReadExpectedTag(input, offset, kBitStringTag)) {
    return false;
  }
  std::size_t bitLen = 0u;
  if (!ReadAxdrLength(input, offset, bitLen)) {
    return false;
  }
  if (bitLen > expectedBitWidth) {
    return false;
  }
  const std::size_t octets = (bitLen + 7u) / 8u;
  const std::uint8_t* data = 0;
  if (!ReadFixedBytes(input, offset, octets, data)) {
    return false;
  }
  std::uint64_t bits = 0u;
  for (std::size_t o = 0u; o < octets; ++o) {
    for (std::uint8_t b = 0u; b < 8u; ++b) {
      const std::uint32_t bitIndex =
        static_cast<std::uint32_t>(o) * 8u + b;
      if (bitIndex >= bitLen)
        break;
      if (data[o] & (0x80u >> b)) {
        bits |= (std::uint64_t{1} << bitIndex);
      }
    }
  }
  bitsOut = bits;
  return true;
}

// Encode IC 10.entries element:
//   structure(10) {
//     index:               long-unsigned,
//     enable:              boolean,
//     script_logical_name: octet-string(6),
//     script_selector:     long-unsigned,
//     switch_time:         octet-string(4),
//     validity_window:     long-unsigned,
//     exec_weekdays:       bit-string(7),
//     exec_specdays:       bit-string(64),
//     begin_date:          octet-string(5),
//     end_date:            octet-string(5)
//   }
void AppendScheduleTableEntry(
  CosemByteBuffer& output,
  const dlms::cosem::types::ScheduleTableEntry& entry)
{
  AppendStructureHeader(output, 10u);
  AppendLongUnsigned(output, entry.Index());
  AppendBoolean(output, entry.Enable());
  AppendOctetString(
    output,
    entry.GetScript().LogicalName().Data(),
    entry.GetScript().LogicalName().Size());
  AppendLongUnsigned(output, entry.GetScript().Selector());
  const std::array<std::uint8_t, dlms::cosem::types::Time::WireSize>
    timeBytes = entry.SwitchTime().ToBytes();
  AppendOctetString(output, timeBytes.data(), timeBytes.size());
  AppendLongUnsigned(output, entry.ValidityWindow());
  AppendBitStringMsbFirst(
    output,
    static_cast<std::uint64_t>(entry.ExecWeekdays()),
    dlms::cosem::types::ScheduleTableEntry::WeekdaysBitWidth);
  AppendBitStringMsbFirst(
    output,
    entry.ExecSpecdays(),
    dlms::cosem::types::ScheduleTableEntry::SpecdaysBitWidth);
  const std::array<std::uint8_t, dlms::cosem::types::Date::WireSize>
    beginBytes = entry.BeginDate().ToBytes();
  AppendOctetString(output, beginBytes.data(), beginBytes.size());
  const std::array<std::uint8_t, dlms::cosem::types::Date::WireSize>
    endBytes = entry.EndDate().ToBytes();
  AppendOctetString(output, endBytes.data(), endBytes.size());
}

bool DecodeScheduleTableEntry(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::ScheduleTableEntry& entryOut)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 10u) {
    return false;
  }

  std::uint16_t index = 0u;
  if (!ReadLongUnsignedValue(input, offset, index)) return false;

  bool enable = false;
  if (!ReadBooleanValue(input, offset, enable)) return false;

  dlms::cosem::CosemLogicalName logicalName;
  if (!ReadLogicalNameValue(input, offset, logicalName)) return false;

  std::uint16_t selector = 0u;
  if (!ReadLongUnsignedValue(input, offset, selector)) return false;

  std::size_t timeLen = 0u;
  const std::uint8_t* timeData = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, timeLen) ||
      timeLen != dlms::cosem::types::Time::WireSize ||
      !ReadFixedBytes(input, offset, timeLen, timeData)) {
    return false;
  }
  dlms::cosem::types::Time switchTime;
  if (!dlms::cosem::types::Time::TryFromBytes(
        timeData, timeLen, switchTime)) {
    return false;
  }

  std::uint16_t validityWindow = 0u;
  if (!ReadLongUnsignedValue(input, offset, validityWindow)) return false;

  std::uint64_t weekdaysBits = 0u;
  if (!ReadBitStringMsbFirst(
        input, offset,
        dlms::cosem::types::ScheduleTableEntry::WeekdaysBitWidth,
        weekdaysBits)) {
    return false;
  }
  if (weekdaysBits > 0x7Fu) {
    return false;
  }

  std::uint64_t specdaysBits = 0u;
  if (!ReadBitStringMsbFirst(
        input, offset,
        dlms::cosem::types::ScheduleTableEntry::SpecdaysBitWidth,
        specdaysBits)) {
    return false;
  }

  std::size_t beginLen = 0u;
  const std::uint8_t* beginData = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, beginLen) ||
      beginLen != dlms::cosem::types::Date::WireSize ||
      !ReadFixedBytes(input, offset, beginLen, beginData)) {
    return false;
  }
  dlms::cosem::types::Date beginDate;
  if (!dlms::cosem::types::Date::TryFromBytes(
        beginData, beginLen, beginDate)) {
    return false;
  }

  std::size_t endLen = 0u;
  const std::uint8_t* endData = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, endLen) ||
      endLen != dlms::cosem::types::Date::WireSize ||
      !ReadFixedBytes(input, offset, endLen, endData)) {
    return false;
  }
  dlms::cosem::types::Date endDate;
  if (!dlms::cosem::types::Date::TryFromBytes(
        endData, endLen, endDate)) {
    return false;
  }

  entryOut = dlms::cosem::types::ScheduleTableEntry(
    index, enable,
    dlms::cosem::types::Script(logicalName, selector),
    switchTime,
    validityWindow,
    static_cast<std::uint8_t>(weekdaysBits & 0x7Fu),
    specdaysBits,
    beginDate,
    endDate);
  return dlms::cosem::types::ScheduleTableEntry::IsValid(entryOut);
}

void AppendScheduleEntries(
  CosemByteBuffer& output,
  const std::vector<dlms::cosem::types::ScheduleTableEntry>& entries)
{
  AppendArrayHeader(output, entries.size());
  for (std::size_t i = 0u; i < entries.size(); ++i) {
    AppendScheduleTableEntry(output, entries[i]);
  }
}

bool DecodeScheduleEntries(
  const CosemByteBuffer& input,
  std::vector<dlms::cosem::types::ScheduleTableEntry>& entriesOut)
{
  entriesOut.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  entriesOut.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    dlms::cosem::types::ScheduleTableEntry entry;
    if (!DecodeScheduleTableEntry(input, offset, entry)) {
      return false;
    }
    entriesOut.push_back(entry);
  }
  if (offset != input.size()) {
    return false;
  }
  return true;
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

// scal_unit_type ::= structure(2) { integer scaler, enum unit }
// per IEC 62056-6-2 ED4 (2021) §4.3.2.2.3 and DLMS UA Blue Book Ed. 12.1.
void AppendScalerUnit(
  CosemByteBuffer& output,
  const dlms::cosem::types::ScalerUnit& su)
{
  AppendStructureHeader(output, 2u);
  AppendInteger(output, static_cast<std::uint8_t>(su.Scaler()));
  AppendEnum(output, su.Unit());
}

bool DecodeScalerUnit(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::ScalerUnit& out)
{
  if (!ReadExpectedTag(input, offset, kStructureTag)) return false;
  std::size_t fieldCount = 0u;
  if (!ReadAxdrLength(input, offset, fieldCount)) return false;
  if (fieldCount != 2u) return false;
  std::uint8_t scalerRaw = 0u;
  if (!ReadIntegerValue(input, offset, scalerRaw)) return false;
  std::uint8_t unitRaw = 0u;
  if (!ReadEnumValue(input, offset, unitRaw)) return false;
  out = dlms::cosem::types::ScalerUnit(
    static_cast<std::int8_t>(scalerRaw), unitRaw);
  return true;
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
  const dlms::cosem::types::ScalerUnit& scalerUnit,
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
  const dlms::cosem::types::ScalerUnit& scalerUnit,
  AttributeAccessMode valueAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kRegisterClassId,
      NormalizeVersion(version, CosemRegisterObject::MaxSupportedVersion),
      logicalName))
  , value_()
  , scalerUnit_(scalerUnit)
{
  // Safe-fallback: only retain `value` when it is a non-empty AXDR
  // data item. Empty / missing payload leaves the attribute cleared
  // and the backend must publish a real value via SetValue().
  if (IsValidValue(value)) {
    value_ = value;
  }
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
    output.clear();
    AppendScalerUnit(output, scalerUnit_);
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
    if (!IsValidValue(input)) {
      return CosemStatus::InvalidArgument;
    }
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
  (void)input;
  output.clear();
  // method 1 = reset. Built-in object exposes it explicitly as
  // UnsupportedFeature: application-defined semantics decide what reset means
  // for a register, and the COSEM object does not own that policy.
  if (methodId == kRegisterResetMethodId) {
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemRegisterObject::Value() const
{
  return value_;
}

const dlms::cosem::types::ScalerUnit& CosemRegisterObject::ScalerUnit() const
{
  return scalerUnit_;
}

bool CosemRegisterObject::SetValue(const CosemByteBuffer& value)
{
  if (!IsValidValue(value)) {
    return false;
  }
  value_ = value;
  return true;
}

void CosemRegisterObject::SetScalerUnit(
  const dlms::cosem::types::ScalerUnit& scalerUnit)
{
  scalerUnit_ = scalerUnit;
}

bool CosemRegisterObject::IsValidValue(const CosemByteBuffer& value)
{
  // A single non-empty AXDR data item; the concrete type belongs to
  // the instance (see spec §4.3.2.2.2 CHOICE) so we only require that
  // the producer pushed at least the tag byte.
  return !value.empty();
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
  const types::ScalerUnit& scalerUnit,
  const CosemByteBuffer& status,
  const types::DateTime& captureTime,
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
  const types::ScalerUnit& scalerUnit,
  const CosemByteBuffer& status,
  const types::DateTime& captureTime,
  AttributeAccessMode valueAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kExtendedRegisterClassId,
      NormalizeVersion(version, CosemExtendedRegisterObject::MaxSupportedVersion),
      logicalName))
  , value_(CosemExtendedRegisterObject::IsValidValue(value) ? value
                                                            : CosemByteBuffer())
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
    output.clear();
    AppendScalerUnit(output, scalerUnit_);
    return CosemStatus::Ok;
  }
  if (attributeId == kExtendedRegisterStatusAttributeId) {
    output = status_;
    return CosemStatus::Ok;
  }
  if (attributeId == kExtendedRegisterCaptureTimeAttributeId) {
    output.clear();
    AppendDateTimeOctetString(output, captureTime_);
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
    if (!CosemExtendedRegisterObject::IsValidValue(input)) {
      return CosemStatus::InvalidArgument;
    }
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

const types::ScalerUnit& CosemExtendedRegisterObject::ScalerUnit() const
{
  return scalerUnit_;
}

const CosemByteBuffer& CosemExtendedRegisterObject::Status() const
{
  return status_;
}

const types::DateTime& CosemExtendedRegisterObject::CaptureTime() const
{
  return captureTime_;
}

bool CosemExtendedRegisterObject::SetValue(const CosemByteBuffer& value)
{
  if (!CosemExtendedRegisterObject::IsValidValue(value)) {
    return false;
  }
  value_ = value;
  return true;
}

void CosemExtendedRegisterObject::SetScalerUnit(
  const types::ScalerUnit& scalerUnit)
{
  scalerUnit_ = scalerUnit;
}

void CosemExtendedRegisterObject::SetStatus(const CosemByteBuffer& status)
{
  status_ = status;
}

void CosemExtendedRegisterObject::SetCaptureTime(
  const types::DateTime& captureTime)
{
  captureTime_ = captureTime;
}

bool CosemExtendedRegisterObject::IsValidValue(const CosemByteBuffer& value)
{
  return !value.empty();
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
  const types::ScalerUnit& scalerUnit,
  const CosemByteBuffer& status,
  const types::DateTime& captureTime,
  const types::DateTime& startTimeCurrent,
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
  const types::ScalerUnit& scalerUnit,
  const CosemByteBuffer& status,
  const types::DateTime& captureTime,
  const types::DateTime& startTimeCurrent,
  std::uint32_t period,
  std::uint16_t numberOfPeriods,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kDemandRegisterClassId,
      NormalizeVersion(version, CosemDemandRegisterObject::MaxSupportedVersion),
      logicalName))
  , currentAverageValue_(
      CosemDemandRegisterObject::IsValidAverageValue(currentAverageValue)
        ? currentAverageValue
        : CosemByteBuffer())
  , lastAverageValue_(
      CosemDemandRegisterObject::IsValidAverageValue(lastAverageValue)
        ? lastAverageValue
        : CosemByteBuffer())
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
    output.clear();
    AppendScalerUnit(output, scalerUnit_);
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterStatusAttributeId) {
    output = status_;
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterCaptureTimeAttributeId) {
    output.clear();
    AppendDateTimeOctetString(output, captureTime_);
    return CosemStatus::Ok;
  }
  if (attributeId == kDemandRegisterStartTimeCurrentAttributeId) {
    output.clear();
    AppendDateTimeOctetString(output, startTimeCurrent_);
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

const types::ScalerUnit& CosemDemandRegisterObject::ScalerUnit() const
{
  return scalerUnit_;
}

const CosemByteBuffer& CosemDemandRegisterObject::Status() const
{
  return status_;
}

const types::DateTime& CosemDemandRegisterObject::CaptureTime() const
{
  return captureTime_;
}

const types::DateTime& CosemDemandRegisterObject::StartTimeCurrent() const
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

bool CosemDemandRegisterObject::IsValidAverageValue(
  const CosemByteBuffer& value)
{
  return !value.empty();
}

bool CosemDemandRegisterObject::SetCurrentAverageValue(
  const CosemByteBuffer& value)
{
  if (!CosemDemandRegisterObject::IsValidAverageValue(value)) {
    return false;
  }
  currentAverageValue_ = value;
  return true;
}

bool CosemDemandRegisterObject::SetLastAverageValue(
  const CosemByteBuffer& value)
{
  if (!CosemDemandRegisterObject::IsValidAverageValue(value)) {
    return false;
  }
  lastAverageValue_ = value;
  return true;
}

void CosemDemandRegisterObject::SetScalerUnit(
  const types::ScalerUnit& scalerUnit)
{
  scalerUnit_ = scalerUnit;
}

void CosemDemandRegisterObject::SetStatus(const CosemByteBuffer& status)
{
  status_ = status;
}

void CosemDemandRegisterObject::SetCaptureTime(
  const types::DateTime& captureTime)
{
  captureTime_ = captureTime;
}

void CosemDemandRegisterObject::SetStartTimeCurrent(
  const types::DateTime& startTime)
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
  const std::vector<types::ObjectDefinition>& registerAssignment,
  const std::vector<types::RegisterMask>& maskList,
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
  const std::vector<types::ObjectDefinition>& registerAssignment,
  const std::vector<types::RegisterMask>& maskList,
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

namespace {

// register_assignment ::= array of object_definition { class_id LU,
//                                                      logical_name OS(6) }
void AppendRegisterAssignment(
  CosemByteBuffer& out,
  const std::vector<types::ObjectDefinition>& items)
{
  AppendArrayHeader(out, items.size());
  for (const auto& item : items) {
    AppendStructureHeader(out, 2u);
    AppendLongUnsigned(out, item.ClassId());
    AppendLogicalName(out, item.LogicalName());
  }
}

// mask_list ::= array of structure { mask_name OS, index_list array of LU }
void AppendMaskList(
  CosemByteBuffer& out,
  const std::vector<types::RegisterMask>& items)
{
  AppendArrayHeader(out, items.size());
  for (const auto& item : items) {
    AppendStructureHeader(out, 2u);
    AppendOctetString(out, item.MaskName().data(), item.MaskName().size());
    const auto& indices = item.IndexList();
    AppendArrayHeader(out, indices.size());
    for (const auto index : indices) {
      AppendLongUnsigned(out, index);
    }
  }
}

// active_mask ::= octet-string
void AppendActiveMask(CosemByteBuffer& out, const CosemByteBuffer& name)
{
  AppendOctetString(out, name.data(), name.size());
}

}  // namespace

CosemStatus CosemRegisterActivationObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  if (attributeId == kLogicalNameAttributeId) {
    output = EncodeLogicalName(descriptor_.key.logicalName);
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterActivationRegisterAssignmentAttributeId) {
    output.clear();
    AppendRegisterAssignment(output, registerAssignment_);
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterActivationMaskListAttributeId) {
    output.clear();
    AppendMaskList(output, maskList_);
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterActivationActiveMaskAttributeId) {
    output.clear();
    AppendActiveMask(output, activeMask_);
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

const std::vector<types::ObjectDefinition>&
CosemRegisterActivationObject::RegisterAssignment() const
{
  return registerAssignment_;
}

const std::vector<types::RegisterMask>&
CosemRegisterActivationObject::MaskList() const
{
  return maskList_;
}

const CosemByteBuffer& CosemRegisterActivationObject::ActiveMask() const
{
  return activeMask_;
}

void CosemRegisterActivationObject::SetRegisterAssignment(
  const std::vector<types::ObjectDefinition>& assignment)
{
  registerAssignment_ = assignment;
}

void CosemRegisterActivationObject::SetMaskList(
  const std::vector<types::RegisterMask>& maskList)
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

// ---- IC 21 AXDR codec helpers (IEC 62056-6-2 ED4 §4.5.6) ----
//
// action_item ::= structure(2) {
//   octet-string(6) script_logical_name,
//   long-unsigned   script_selector
// }
void AppendActionItem(
  CosemByteBuffer& output,
  const dlms::cosem::types::Script& item)
{
  AppendStructureHeader(output, 2u);
  AppendLogicalName(output, item.LogicalName());
  AppendLongUnsigned(output, item.Selector());
}

bool DecodeActionItem(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::Script& out)
{
  if (!ReadExpectedTag(input, offset, kStructureTag)) return false;
  std::size_t fieldCount = 0u;
  if (!ReadAxdrLength(input, offset, fieldCount)) return false;
  if (fieldCount != 2u) return false;
  CosemLogicalName ln;
  if (!ReadLogicalNameValue(input, offset, ln)) return false;
  std::uint16_t selector = 0u;
  if (!ReadLongUnsignedValue(input, offset, selector)) return false;
  out = dlms::cosem::types::Script(ln, selector);
  return true;
}

// action_set ::= structure(2) { action_item action_up, action_item action_down }
void AppendActionSet(
  CosemByteBuffer& output,
  const dlms::cosem::types::ActionSet& set)
{
  AppendStructureHeader(output, 2u);
  AppendActionItem(output, set.ActionUp());
  AppendActionItem(output, set.ActionDown());
}

bool DecodeActionSet(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::ActionSet& out)
{
  if (!ReadExpectedTag(input, offset, kStructureTag)) return false;
  std::size_t fieldCount = 0u;
  if (!ReadAxdrLength(input, offset, fieldCount)) return false;
  if (fieldCount != 2u) return false;
  dlms::cosem::types::Script up;
  dlms::cosem::types::Script down;
  if (!DecodeActionItem(input, offset, up)) return false;
  if (!DecodeActionItem(input, offset, down)) return false;
  out = dlms::cosem::types::ActionSet(up, down);
  return true;
}

// actions ::= array of action_set
void AppendActions(
  CosemByteBuffer& output,
  const std::vector<dlms::cosem::types::ActionSet>& actions)
{
  AppendArrayHeader(output, actions.size());
  for (const auto& s : actions) {
    AppendActionSet(output, s);
  }
}

bool DecodeActions(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::vector<dlms::cosem::types::ActionSet>& out)
{
  if (!ReadExpectedTag(input, offset, kArrayTag)) return false;
  std::size_t count = 0u;
  if (!ReadAxdrLength(input, offset, count)) return false;
  std::vector<dlms::cosem::types::ActionSet> tmp;
  tmp.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    dlms::cosem::types::ActionSet s;
    if (!DecodeActionSet(input, offset, s)) return false;
    tmp.push_back(s);
  }
  out.swap(tmp);
  return true;
}

// value_definition ::= structure(3) {
//   long-unsigned class_id, octet-string(6) logical_name, integer attribute_index
// }
void AppendMonitoredValue(
  CosemByteBuffer& output,
  const dlms::cosem::types::MonitoredValue& v)
{
  AppendStructureHeader(output, 3u);
  AppendLongUnsigned(output, v.ClassId());
  AppendLogicalName(output, v.LogicalName());
  AppendInteger(output, v.AttributeIndex());
}

bool DecodeMonitoredValue(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::MonitoredValue& out)
{
  if (!ReadExpectedTag(input, offset, kStructureTag)) return false;
  std::size_t fieldCount = 0u;
  if (!ReadAxdrLength(input, offset, fieldCount)) return false;
  if (fieldCount != 3u) return false;
  std::uint16_t classId = 0u;
  if (!ReadLongUnsignedValue(input, offset, classId)) return false;
  CosemLogicalName ln;
  if (!ReadLogicalNameValue(input, offset, ln)) return false;
  std::uint8_t attrRaw = 0u;
  if (!ReadIntegerValue(input, offset, attrRaw)) return false;
  const std::int8_t attrIndex = static_cast<std::int8_t>(attrRaw);
  if (attrIndex < dlms::cosem::types::MonitoredValue::AttributeIndexMin) {
    return false;
  }
  out = dlms::cosem::types::MonitoredValue(classId, ln, attrIndex);
  return true;
}

// thresholds ::= array of <opaque AXDR data item>
// (the per-element type matches the monitored attribute; we capture
// the raw AXDR bytes verbatim via SkipDlmsData)
void AppendThresholds(
  CosemByteBuffer& output,
  const std::vector<CosemByteBuffer>& thresholds)
{
  AppendArrayHeader(output, thresholds.size());
  for (const auto& t : thresholds) {
    output.insert(output.end(), t.begin(), t.end());
  }
}

bool DecodeThresholds(
  const CosemByteBuffer& input,
  std::size_t& offset,
  std::vector<CosemByteBuffer>& out)
{
  if (!ReadExpectedTag(input, offset, kArrayTag)) return false;
  std::size_t count = 0u;
  if (!ReadAxdrLength(input, offset, count)) return false;
  std::vector<CosemByteBuffer> tmp;
  tmp.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    const std::size_t start = offset;
    if (!SkipDlmsData(input, offset, 0u)) return false;
    if (offset <= start) return false;
    tmp.emplace_back(input.begin() + start, input.begin() + offset);
  }
  out.swap(tmp);
  return true;
}
} // namespace

const std::uint8_t CosemRegisterMonitorObject::MaxSupportedVersion;

bool CosemRegisterMonitorObject::IsValidThresholds(
  const std::vector<CosemByteBuffer>& thresholds)
{
  for (const auto& t : thresholds) {
    if (t.empty()) return false;
  }
  return true;
}

bool CosemRegisterMonitorObject::ThresholdsMatchActions(
  const std::vector<CosemByteBuffer>& thresholds,
  const std::vector<types::ActionSet>& actions)
{
  return thresholds.size() == actions.size();
}

CosemRegisterMonitorObject::CosemRegisterMonitorObject(
  const CosemLogicalName& logicalName,
  const std::vector<CosemByteBuffer>& thresholds,
  const types::MonitoredValue& monitoredValue,
  const std::vector<types::ActionSet>& actions,
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
  const std::vector<CosemByteBuffer>& thresholds,
  const types::MonitoredValue& monitoredValue,
  const std::vector<types::ActionSet>& actions,
  AttributeAccessMode thresholdsAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kRegisterMonitorClassId,
      NormalizeVersion(
        version,
        CosemRegisterMonitorObject::MaxSupportedVersion),
      logicalName))
  , thresholds_()
  , monitoredValue_()
  , actions_()
{
  // Safe-fallback: drop any collection that violates IC 21 invariants
  // rather than holding invalid state.
  if (IsValidThresholds(thresholds)
      && ThresholdsMatchActions(thresholds, actions)) {
    thresholds_ = thresholds;
    actions_ = actions;
  }
  if (types::MonitoredValue::IsValid(monitoredValue)) {
    monitoredValue_ = monitoredValue;
  }
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
    output.clear();
    AppendThresholds(output, thresholds_);
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterMonitorMonitoredValueAttributeId) {
    output.clear();
    AppendMonitoredValue(output, monitoredValue_);
    return CosemStatus::Ok;
  }
  if (attributeId == kRegisterMonitorActionsAttributeId) {
    output.clear();
    AppendActions(output, actions_);
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
    if (mode != AttributeAccessMode::WriteOnly
        && mode != AttributeAccessMode::ReadAndWrite
        && mode != AttributeAccessMode::AuthenticatedWriteOnly
        && mode != AttributeAccessMode::AuthenticatedReadAndWrite) {
      return CosemStatus::AccessDenied;
    }
    std::vector<CosemByteBuffer> decoded;
    std::size_t offset = 0u;
    if (!DecodeThresholds(input, offset, decoded)) {
      return CosemStatus::InvalidArgument;
    }
    if (offset != input.size()) {
      return CosemStatus::InvalidArgument;
    }
    if (!IsValidThresholds(decoded)
        || !ThresholdsMatchActions(decoded, actions_)) {
      return CosemStatus::InvalidArgument;
    }
    thresholds_ = std::move(decoded);
    return CosemStatus::Ok;
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

const std::vector<CosemByteBuffer>& CosemRegisterMonitorObject::Thresholds() const
{
  return thresholds_;
}

const dlms::cosem::types::MonitoredValue&
CosemRegisterMonitorObject::MonitoredValue() const
{
  return monitoredValue_;
}

const std::vector<dlms::cosem::types::ActionSet>&
CosemRegisterMonitorObject::Actions() const
{
  return actions_;
}

bool CosemRegisterMonitorObject::SetThresholds(
  const std::vector<CosemByteBuffer>& thresholds)
{
  if (!IsValidThresholds(thresholds)
      || !ThresholdsMatchActions(thresholds, actions_)) {
    return false;
  }
  thresholds_ = thresholds;
  return true;
}

bool CosemRegisterMonitorObject::SetMonitoredValue(
  const types::MonitoredValue& monitoredValue)
{
  if (!types::MonitoredValue::IsValid(monitoredValue)) {
    return false;
  }
  monitoredValue_ = monitoredValue;
  return true;
}

bool CosemRegisterMonitorObject::SetActions(
  const std::vector<types::ActionSet>& actions)
{
  if (!ThresholdsMatchActions(thresholds_, actions)) {
    return false;
  }
  actions_ = actions;
  return true;
}

namespace {
constexpr std::uint16_t kScriptTableClassId = 9u;
constexpr std::uint8_t kScriptTableScriptsAttributeId = 2u;
constexpr std::uint8_t kScriptTableExecuteMethodId = 1u;
constexpr std::size_t kScriptTableLogicalNameSize = 6u;

// Append a single action_specification structure on the wire:
//   structure(5) {
//     enum service_id,
//     long-unsigned class_id,
//     octet-string(6) logical_name,
//     integer index,
//     parameter (raw AXDR; null-data if empty)
//   }
void AppendActionSpecification(
  CosemByteBuffer& output,
  const dlms::cosem::types::ActionSpecification& action)
{
  AppendStructureHeader(output, 5u);
  AppendEnum(
    output,
    static_cast<std::uint8_t>(action.ServiceId()));
  AppendLongUnsigned(output, action.ClassId());
  AppendLogicalName(output, action.LogicalName());
  AppendInteger(
    output,
    static_cast<std::uint8_t>(action.Index()));
  const CosemByteBuffer& parameter = action.Parameter();
  if (parameter.empty()) {
    output.push_back(kNullDataTag);
  } else {
    output.insert(output.end(), parameter.begin(), parameter.end());
  }
}

// Decode a single action_specification structure starting at `offset`.
bool DecodeActionSpecification(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::ActionSpecification& outAction)
{
  std::size_t fields = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fields) ||
      fields != 5u) {
    return false;
  }

  std::uint8_t serviceIdRaw = 0u;
  if (!ReadEnumValue(input, offset, serviceIdRaw)) {
    return false;
  }
  dlms::cosem::types::ScriptServiceId serviceId =
    dlms::cosem::types::ScriptServiceId::Dummy;
  switch (serviceIdRaw) {
    case 0u:
      serviceId = dlms::cosem::types::ScriptServiceId::Dummy;
      break;
    case 1u:
      serviceId = dlms::cosem::types::ScriptServiceId::WriteAttribute;
      break;
    case 2u:
      serviceId = dlms::cosem::types::ScriptServiceId::ExecuteMethod;
      break;
    default:
      return false;
  }

  std::uint16_t classId = 0u;
  if (!ReadLongUnsignedValue(input, offset, classId)) {
    return false;
  }

  if (!ReadExpectedTag(input, offset, kDataOctetStringTag)) {
    return false;
  }
  std::size_t nameLen = 0u;
  if (!ReadAxdrLength(input, offset, nameLen) ||
      nameLen != kScriptTableLogicalNameSize) {
    return false;
  }
  const std::uint8_t* nameBytes = nullptr;
  if (!ReadFixedBytes(input, offset, nameLen, nameBytes)) {
    return false;
  }
  CosemLogicalName logicalName(
    nameBytes[0], nameBytes[1], nameBytes[2],
    nameBytes[3], nameBytes[4], nameBytes[5]);

  std::uint8_t indexRaw = 0u;
  if (!ReadIntegerValue(input, offset, indexRaw)) {
    return false;
  }
  const std::int8_t index = static_cast<std::int8_t>(indexRaw);

  // parameter: capture raw AXDR bytes for the full data item.
  if (offset >= input.size()) {
    return false;
  }
  const std::size_t paramStart = offset;
  const std::uint8_t paramTag = input[paramStart];
  if (!SkipDlmsData(input, offset, 0u)) {
    return false;
  }
  CosemByteBuffer parameter;
  // null-data has zero "payload"; round-trip "absent" as empty buffer.
  if (paramTag != kNullDataTag) {
    parameter.assign(
      input.begin() + paramStart,
      input.begin() + offset);
  }

  dlms::cosem::types::ActionSpecification decoded(
    serviceId, classId, logicalName, index, parameter);
  if (!dlms::cosem::types::ActionSpecification::IsValid(decoded)) {
    return false;
  }
  outAction = decoded;
  return true;
}

// Append the full `scripts` attribute on the wire:
//   array of script
//   script ::= structure(2) {
//     long-unsigned script_identifier,
//     array of action_specification
//   }
void AppendScripts(
  CosemByteBuffer& output,
  const std::vector<dlms::cosem::types::ScriptEntry>& scripts)
{
  AppendArrayHeader(output, scripts.size());
  for (std::size_t i = 0u; i < scripts.size(); ++i) {
    const dlms::cosem::types::ScriptEntry& entry = scripts[i];
    AppendStructureHeader(output, 2u);
    AppendLongUnsigned(output, entry.Identifier());
    const std::vector<dlms::cosem::types::ActionSpecification>&
      actions = entry.Actions();
    AppendArrayHeader(output, actions.size());
    for (std::size_t j = 0u; j < actions.size(); ++j) {
      AppendActionSpecification(output, actions[j]);
    }
  }
}

bool DecodeScripts(
  const CosemByteBuffer& input,
  std::vector<dlms::cosem::types::ScriptEntry>& outScripts)
{
  std::size_t offset = 0u;
  std::size_t scriptCount = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, scriptCount)) {
    return false;
  }

  std::vector<dlms::cosem::types::ScriptEntry> decoded;
  decoded.reserve(scriptCount);

  for (std::size_t i = 0u; i < scriptCount; ++i) {
    std::size_t scriptFields = 0u;
    if (!ReadExpectedTag(input, offset, kStructureTag) ||
        !ReadAxdrLength(input, offset, scriptFields) ||
        scriptFields != 2u) {
      return false;
    }
    std::uint16_t identifier = 0u;
    if (!ReadLongUnsignedValue(input, offset, identifier)) {
      return false;
    }
    std::size_t actionCount = 0u;
    if (!ReadExpectedTag(input, offset, kArrayTag) ||
        !ReadAxdrLength(input, offset, actionCount)) {
      return false;
    }
    std::vector<dlms::cosem::types::ActionSpecification> actions;
    actions.reserve(actionCount);
    for (std::size_t j = 0u; j < actionCount; ++j) {
      dlms::cosem::types::ActionSpecification action;
      if (!DecodeActionSpecification(input, offset, action)) {
        return false;
      }
      actions.push_back(action);
    }
    dlms::cosem::types::ScriptEntry entry(identifier, actions);
    // Constructor drops actions if any fail IsValid; reject here too.
    if (entry.Actions().size() != actions.size()) {
      return false;
    }
    decoded.push_back(entry);
  }

  // Whole-buffer check: no trailing bytes after the last script.
  if (offset != input.size()) {
    return false;
  }
  // Unique script_identifier across the collection.
  for (std::size_t i = 0u; i < decoded.size(); ++i) {
    for (std::size_t k = i + 1u; k < decoded.size(); ++k) {
      if (decoded[i].Identifier() == decoded[k].Identifier()) {
        return false;
      }
    }
  }
  outScripts.swap(decoded);
  return true;
}

} // namespace

const std::uint8_t CosemScriptTableObject::MaxSupportedVersion;

CosemScriptTableObject::CosemScriptTableObject(
  const CosemLogicalName& logicalName,
  const std::vector<types::ScriptEntry>& scripts,
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
  const std::vector<types::ScriptEntry>& scripts,
  AttributeAccessMode scriptsAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kScriptTableClassId,
      NormalizeVersion(
        version,
        CosemScriptTableObject::MaxSupportedVersion),
      logicalName))
  , scripts_()
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kScriptTableScriptsAttributeId,
    scriptsAccess);
  // Safe-fallback construction: hold an empty scripts collection when
  // the caller passes a malformed one (duplicate ids, invalid actions).
  if (IsValidScripts(scripts)) {
    scripts_ = scripts;
  }
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
    output.clear();
    AppendScripts(output, scripts_);
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
      std::vector<types::ScriptEntry> decoded;
      if (!DecodeScripts(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      // DecodeScripts already enforces uniqueness + per-action IsValid;
      // call the public validator for symmetry / future-proofing.
      if (!IsValidScripts(decoded)) {
        return CosemStatus::InvalidArgument;
      }
      scripts_.swap(decoded);
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

const std::vector<types::ScriptEntry>&
CosemScriptTableObject::Scripts() const
{
  return scripts_;
}

bool CosemScriptTableObject::SetScripts(
  const std::vector<types::ScriptEntry>& scripts)
{
  if (!IsValidScripts(scripts)) {
    return false;
  }
  scripts_ = scripts;
  return true;
}

bool CosemScriptTableObject::IsValidScripts(
  const std::vector<types::ScriptEntry>& scripts)
{
  for (std::size_t i = 0u; i < scripts.size(); ++i) {
    if (!types::ScriptEntry::IsValid(scripts[i])) {
      return false;
    }
    for (std::size_t k = i + 1u; k < scripts.size(); ++k) {
      if (scripts[i].Identifier() == scripts[k].Identifier()) {
        return false;
      }
    }
  }
  return true;
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

// AXDR codec helpers for IC 20 "Activity calendar" per IEC 62056-6-2 ED4
// (2021) section 4.5.5 and DLMS UA Blue Book Ed. 12.1 section 5.1.9. All
// helpers are pure functions: they never mutate the IC 20 instance and
// reject malformed input by returning `false` without leaving partial
// state in their out-parameter.

void AppendDayProfileAction(
  CosemByteBuffer& output,
  const dlms::cosem::types::DayProfileAction& action)
{
  // day_profile_action ::= structure
  // { start_time:time, script_logical_name:octet-string(6),
  //   script_selector:long-unsigned }
  AppendStructureHeader(output, 3u);
  const std::array<std::uint8_t, dlms::cosem::types::Time::WireSize>
    timeBytes = action.StartTime().ToBytes();
  AppendOctetString(output, timeBytes.data(), timeBytes.size());
  AppendOctetString(
    output,
    action.ScriptLogicalName().Data(),
    action.ScriptLogicalName().Size());
  AppendLongUnsigned(output, action.ScriptSelector());
}

bool DecodeDayProfileAction(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::DayProfileAction& out)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag)
      || !ReadAxdrLength(input, offset, fieldCount) || fieldCount != 3u) {
    return false;
  }
  std::size_t timeLen = 0u;
  const std::uint8_t* timeData = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag)
      || !ReadAxdrLength(input, offset, timeLen)
      || timeLen != dlms::cosem::types::Time::WireSize
      || !ReadFixedBytes(input, offset, timeLen, timeData)) {
    return false;
  }
  dlms::cosem::types::Time startTime;
  if (!dlms::cosem::types::Time::TryFromBytes(
         timeData, timeLen, startTime)) {
    return false;
  }
  dlms::cosem::CosemLogicalName scriptName;
  if (!ReadLogicalNameValue(input, offset, scriptName)) {
    return false;
  }
  std::uint16_t selector = 0u;
  if (!ReadLongUnsignedValue(input, offset, selector)) {
    return false;
  }
  dlms::cosem::types::DayProfileAction tmp;
  if (!tmp.SetStartTime(startTime)) {
    return false;  // start_time must not be wildcard.
  }
  tmp.SetScriptLogicalName(scriptName);
  tmp.SetScriptSelector(selector);
  if (!tmp.IsValid()) {
    return false;
  }
  out = tmp;
  return true;
}

void AppendDayProfile(
  CosemByteBuffer& output,
  const dlms::cosem::types::DayProfile& profile)
{
  // day_profile ::= structure { day_id:unsigned, day_schedule:array }
  AppendStructureHeader(output, 2u);
  AppendUnsigned(output, profile.DayId());
  AppendArrayHeader(output, profile.DaySchedule().size());
  for (std::size_t i = 0u; i < profile.DaySchedule().size(); ++i) {
    AppendDayProfileAction(output, profile.DaySchedule()[i]);
  }
}

bool DecodeDayProfile(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::DayProfile& out)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag)
      || !ReadAxdrLength(input, offset, fieldCount) || fieldCount != 2u) {
    return false;
  }
  std::uint8_t dayId = 0u;
  if (!ReadUnsignedValue(input, offset, dayId)) {
    return false;
  }
  std::size_t actionCount = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, actionCount)) {
    return false;
  }
  std::vector<dlms::cosem::types::DayProfileAction> schedule;
  schedule.reserve(actionCount);
  for (std::size_t i = 0u; i < actionCount; ++i) {
    dlms::cosem::types::DayProfileAction action;
    if (!DecodeDayProfileAction(input, offset, action)) {
      return false;
    }
    schedule.push_back(action);
  }
  dlms::cosem::types::DayProfile tmp;
  tmp.SetDayId(dayId);
  if (!tmp.SetDaySchedule(schedule)) {
    // Strictly-ascending start_time invariant violated.
    return false;
  }
  out = tmp;
  return true;
}

void AppendDayProfileTable(
  CosemByteBuffer& output,
  const std::vector<dlms::cosem::types::DayProfile>& table)
{
  AppendArrayHeader(output, table.size());
  for (std::size_t i = 0u; i < table.size(); ++i) {
    AppendDayProfile(output, table[i]);
  }
}

bool DecodeDayProfileTablePayload(
  const CosemByteBuffer& input,
  std::vector<dlms::cosem::types::DayProfile>& out)
{
  out.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  out.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    dlms::cosem::types::DayProfile profile;
    if (!DecodeDayProfile(input, offset, profile)) {
      return false;
    }
    out.push_back(profile);
  }
  return offset == input.size();
}

void AppendWeekProfile(
  CosemByteBuffer& output,
  const dlms::cosem::types::WeekProfile& profile)
{
  // week_profile ::= structure(8)
  // { name:octet-string, mon..sun: unsigned day_id }
  AppendStructureHeader(output, 8u);
  AppendBufferOctetString(output, profile.Name());
  AppendUnsigned(output, profile.Monday());
  AppendUnsigned(output, profile.Tuesday());
  AppendUnsigned(output, profile.Wednesday());
  AppendUnsigned(output, profile.Thursday());
  AppendUnsigned(output, profile.Friday());
  AppendUnsigned(output, profile.Saturday());
  AppendUnsigned(output, profile.Sunday());
}

bool ReadBufferOctetString(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemByteBuffer& out)
{
  std::size_t length = 0u;
  const std::uint8_t* data = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag)
      || !ReadAxdrLength(input, offset, length)
      || !ReadFixedBytes(input, offset, length, data)) {
    return false;
  }
  out.assign(data, data + length);
  return true;
}

bool DecodeWeekProfile(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::WeekProfile& out)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag)
      || !ReadAxdrLength(input, offset, fieldCount) || fieldCount != 8u) {
    return false;
  }
  CosemByteBuffer name;
  if (!ReadBufferOctetString(input, offset, name)) {
    return false;
  }
  std::uint8_t days[7] = {0u, 0u, 0u, 0u, 0u, 0u, 0u};
  for (std::size_t i = 0u; i < 7u; ++i) {
    if (!ReadUnsignedValue(input, offset, days[i])) {
      return false;
    }
  }
  out = dlms::cosem::types::WeekProfile(
    name,
    days[0], days[1], days[2], days[3],
    days[4], days[5], days[6]);
  return true;
}

void AppendWeekProfileTable(
  CosemByteBuffer& output,
  const std::vector<dlms::cosem::types::WeekProfile>& table)
{
  AppendArrayHeader(output, table.size());
  for (std::size_t i = 0u; i < table.size(); ++i) {
    AppendWeekProfile(output, table[i]);
  }
}

bool DecodeWeekProfileTablePayload(
  const CosemByteBuffer& input,
  std::vector<dlms::cosem::types::WeekProfile>& out)
{
  out.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  out.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    dlms::cosem::types::WeekProfile profile;
    if (!DecodeWeekProfile(input, offset, profile)) {
      return false;
    }
    out.push_back(profile);
  }
  return offset == input.size();
}

void AppendSeasonProfile(
  CosemByteBuffer& output,
  const dlms::cosem::types::SeasonProfile& profile)
{
  // season ::= structure(3)
  // { name:octet-string, start:date_time octet-string(12),
  //   week_name:octet-string }
  AppendStructureHeader(output, 3u);
  AppendBufferOctetString(output, profile.Name());
  const std::array<std::uint8_t, dlms::cosem::types::DateTime::WireSize>
    dtBytes = profile.Start().ToBytes();
  AppendOctetString(output, dtBytes.data(), dtBytes.size());
  AppendBufferOctetString(output, profile.WeekName());
}

bool DecodeSeasonProfile(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::SeasonProfile& out)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag)
      || !ReadAxdrLength(input, offset, fieldCount) || fieldCount != 3u) {
    return false;
  }
  CosemByteBuffer name;
  if (!ReadBufferOctetString(input, offset, name)) {
    return false;
  }
  std::size_t dtLen = 0u;
  const std::uint8_t* dtData = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag)
      || !ReadAxdrLength(input, offset, dtLen)
      || dtLen != dlms::cosem::types::DateTime::WireSize
      || !ReadFixedBytes(input, offset, dtLen, dtData)) {
    return false;
  }
  dlms::cosem::types::DateTime start;
  if (!dlms::cosem::types::DateTime::TryFromBytes(dtData, dtLen, start)) {
    return false;
  }
  CosemByteBuffer weekName;
  if (!ReadBufferOctetString(input, offset, weekName)) {
    return false;
  }
  out = dlms::cosem::types::SeasonProfile(name, start, weekName);
  return true;
}

void AppendSeasonProfileTable(
  CosemByteBuffer& output,
  const std::vector<dlms::cosem::types::SeasonProfile>& table)
{
  AppendArrayHeader(output, table.size());
  for (std::size_t i = 0u; i < table.size(); ++i) {
    AppendSeasonProfile(output, table[i]);
  }
}

bool DecodeSeasonProfilePayload(
  const CosemByteBuffer& input,
  std::vector<dlms::cosem::types::SeasonProfile>& out)
{
  out.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  out.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    dlms::cosem::types::SeasonProfile profile;
    if (!DecodeSeasonProfile(input, offset, profile)) {
      return false;
    }
    out.push_back(profile);
  }
  return offset == input.size();
}
} // namespace

const std::uint8_t CosemActivityCalendarObject::MaxSupportedVersion;

CosemActivityCalendarObject::CosemActivityCalendarObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& calendarNameActive,
  const std::vector<types::SeasonProfile>& seasonProfileActive,
  const std::vector<types::WeekProfile>& weekProfileTableActive,
  const std::vector<types::DayProfile>& dayProfileTableActive,
  const CosemByteBuffer& calendarNamePassive,
  const std::vector<types::SeasonProfile>& seasonProfilePassive,
  const std::vector<types::WeekProfile>& weekProfileTablePassive,
  const std::vector<types::DayProfile>& dayProfileTablePassive,
  const types::DateTime& activatePassiveCalendarTime,
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
  const std::vector<types::SeasonProfile>& seasonProfileActive,
  const std::vector<types::WeekProfile>& weekProfileTableActive,
  const std::vector<types::DayProfile>& dayProfileTableActive,
  const CosemByteBuffer& calendarNamePassive,
  const std::vector<types::SeasonProfile>& seasonProfilePassive,
  const std::vector<types::WeekProfile>& weekProfileTablePassive,
  const std::vector<types::DayProfile>& dayProfileTablePassive,
  const types::DateTime& activatePassiveCalendarTime,
  AttributeAccessMode passiveAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kActivityCalendarClassId,
      NormalizeVersion(
        version,
        CosemActivityCalendarObject::MaxSupportedVersion),
      logicalName))
  , calendarNameActive_(calendarNameActive)
  , seasonProfileActive_()
  , weekProfileTableActive_()
  , dayProfileTableActive_()
  , calendarNamePassive_(calendarNamePassive)
  , seasonProfilePassive_()
  , weekProfileTablePassive_()
  , dayProfileTablePassive_()
  , activatePassiveCalendarTime_(activatePassiveCalendarTime)
{
  // Safe-fallback construction: silently drop ill-formed collections so
  // the constructed object never holds invalid state. Tests covering the
  // boundary use the explicit Setter() / IsValid*() API to verify the
  // refusal path.
  if (IsValidDayProfileTable(dayProfileTableActive)) {
    dayProfileTableActive_ = dayProfileTableActive;
  }
  if (IsValidWeekProfileTable(weekProfileTableActive)
      && WeekProfileTableSatisfies(
           weekProfileTableActive, dayProfileTableActive_)) {
    weekProfileTableActive_ = weekProfileTableActive;
  }
  if (IsValidSeasonProfile(seasonProfileActive)
      && SeasonProfileSatisfies(
           seasonProfileActive, weekProfileTableActive_)) {
    seasonProfileActive_ = seasonProfileActive;
  }
  if (IsValidDayProfileTable(dayProfileTablePassive)) {
    dayProfileTablePassive_ = dayProfileTablePassive;
  }
  if (IsValidWeekProfileTable(weekProfileTablePassive)
      && WeekProfileTableSatisfies(
           weekProfileTablePassive, dayProfileTablePassive_)) {
    weekProfileTablePassive_ = weekProfileTablePassive;
  }
  if (IsValidSeasonProfile(seasonProfilePassive)
      && SeasonProfileSatisfies(
           seasonProfilePassive, weekProfileTablePassive_)) {
    seasonProfilePassive_ = seasonProfilePassive;
  }

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
      output.clear();
      AppendSeasonProfileTable(output, seasonProfileActive_);
      return CosemStatus::Ok;
    case kActivityCalendarWeekProfileTableActiveAttributeId:
      output.clear();
      AppendWeekProfileTable(output, weekProfileTableActive_);
      return CosemStatus::Ok;
    case kActivityCalendarDayProfileTableActiveAttributeId:
      output.clear();
      AppendDayProfileTable(output, dayProfileTableActive_);
      return CosemStatus::Ok;
    case kActivityCalendarCalendarNamePassiveAttributeId:
      output = calendarNamePassive_;
      return CosemStatus::Ok;
    case kActivityCalendarSeasonProfilePassiveAttributeId:
      output.clear();
      AppendSeasonProfileTable(output, seasonProfilePassive_);
      return CosemStatus::Ok;
    case kActivityCalendarWeekProfileTablePassiveAttributeId:
      output.clear();
      AppendWeekProfileTable(output, weekProfileTablePassive_);
      return CosemStatus::Ok;
    case kActivityCalendarDayProfileTablePassiveAttributeId:
      output.clear();
      AppendDayProfileTable(output, dayProfileTablePassive_);
      return CosemStatus::Ok;
    case kActivityCalendarActivatePassiveCalendarTimeAttributeId:
      output.clear();
      AppendDateTimeOctetString(output, activatePassiveCalendarTime_);
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
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        return CosemStatus::AccessDenied;
      }
      calendarNamePassive_ = input;
      return CosemStatus::Ok;
    }
    case kActivityCalendarSeasonProfilePassiveAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        return CosemStatus::AccessDenied;
      }
      std::vector<types::SeasonProfile> decoded;
      if (!DecodeSeasonProfilePayload(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      if (!SetSeasonProfilePassive(decoded)) {
        return CosemStatus::InvalidArgument;
      }
      return CosemStatus::Ok;
    }
    case kActivityCalendarWeekProfileTablePassiveAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        return CosemStatus::AccessDenied;
      }
      std::vector<types::WeekProfile> decoded;
      if (!DecodeWeekProfileTablePayload(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      if (!SetWeekProfileTablePassive(decoded)) {
        return CosemStatus::InvalidArgument;
      }
      return CosemStatus::Ok;
    }
    case kActivityCalendarDayProfileTablePassiveAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        return CosemStatus::AccessDenied;
      }
      std::vector<types::DayProfile> decoded;
      if (!DecodeDayProfileTablePayload(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      if (!SetDayProfileTablePassive(decoded)) {
        return CosemStatus::InvalidArgument;
      }
      return CosemStatus::Ok;
    }
    case kActivityCalendarActivatePassiveCalendarTimeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId))) {
        return CosemStatus::AccessDenied;
      }
      types::DateTime decoded;
      if (!DecodeDateTimeOctetString(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      activatePassiveCalendarTime_ = decoded;
      return CosemStatus::Ok;
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

const std::vector<types::SeasonProfile>&
CosemActivityCalendarObject::SeasonProfileActive() const
{
  return seasonProfileActive_;
}

const std::vector<types::WeekProfile>&
CosemActivityCalendarObject::WeekProfileTableActive() const
{
  return weekProfileTableActive_;
}

const std::vector<types::DayProfile>&
CosemActivityCalendarObject::DayProfileTableActive() const
{
  return dayProfileTableActive_;
}

const CosemByteBuffer&
CosemActivityCalendarObject::CalendarNamePassive() const
{
  return calendarNamePassive_;
}

const std::vector<types::SeasonProfile>&
CosemActivityCalendarObject::SeasonProfilePassive() const
{
  return seasonProfilePassive_;
}

const std::vector<types::WeekProfile>&
CosemActivityCalendarObject::WeekProfileTablePassive() const
{
  return weekProfileTablePassive_;
}

const std::vector<types::DayProfile>&
CosemActivityCalendarObject::DayProfileTablePassive() const
{
  return dayProfileTablePassive_;
}

const types::DateTime&
CosemActivityCalendarObject::ActivatePassiveCalendarTime() const
{
  return activatePassiveCalendarTime_;
}

void CosemActivityCalendarObject::SetCalendarNamePassive(
  const CosemByteBuffer& value)
{
  calendarNamePassive_ = value;
}

bool CosemActivityCalendarObject::SetSeasonProfilePassive(
  const std::vector<types::SeasonProfile>& value)
{
  if (!IsValidSeasonProfile(value)
      || !SeasonProfileSatisfies(value, weekProfileTablePassive_)) {
    return false;
  }
  seasonProfilePassive_ = value;
  return true;
}

bool CosemActivityCalendarObject::SetWeekProfileTablePassive(
  const std::vector<types::WeekProfile>& value)
{
  if (!IsValidWeekProfileTable(value)
      || !WeekProfileTableSatisfies(value, dayProfileTablePassive_)) {
    return false;
  }
  // Replacing week_profile_table may invalidate the existing
  // season_profile cross-reference. Reject when this would leave the
  // object in an inconsistent state instead of silently dropping seasons.
  if (!SeasonProfileSatisfies(seasonProfilePassive_, value)) {
    return false;
  }
  weekProfileTablePassive_ = value;
  return true;
}

bool CosemActivityCalendarObject::SetDayProfileTablePassive(
  const std::vector<types::DayProfile>& value)
{
  if (!IsValidDayProfileTable(value)) {
    return false;
  }
  // Replacing day_profile_table may break week_profile cross-references.
  if (!WeekProfileTableSatisfies(weekProfileTablePassive_, value)) {
    return false;
  }
  dayProfileTablePassive_ = value;
  return true;
}

void CosemActivityCalendarObject::SetActivatePassiveCalendarTime(
  const types::DateTime& value)
{
  activatePassiveCalendarTime_ = value;
}

bool CosemActivityCalendarObject::IsValidSeasonProfile(
  const std::vector<types::SeasonProfile>& value)
{
  // Per IEC 62056-6-2 ED4 §4.5.5: season_profile entries must have a
  // unique season name (sort key) and the array must be ordered by
  // strictly ascending start date_time. We enforce both invariants here.
  // DateTime has no operator<; compare the canonical wire form bytewise
  // (lexicographic order on the 12-byte representation matches calendar
  // order field-by-field: year-hi, year-lo, month, day, ...).
  for (std::size_t i = 0u; i < value.size(); ++i) {
    for (std::size_t j = i + 1u; j < value.size(); ++j) {
      if (value[i].Name() == value[j].Name()) {
        return false;
      }
    }
    if (i > 0u) {
      const std::array<std::uint8_t, types::DateTime::WireSize> prev
        = value[i - 1u].Start().ToBytes();
      const std::array<std::uint8_t, types::DateTime::WireSize> cur
        = value[i].Start().ToBytes();
      bool strictlyLess = false;
      for (std::size_t k = 0u; k < types::DateTime::WireSize; ++k) {
        if (prev[k] < cur[k]) {
          strictlyLess = true;
          break;
        }
        if (prev[k] > cur[k]) {
          break;
        }
      }
      if (!strictlyLess) {
        return false;
      }
    }
  }
  return true;
}

bool CosemActivityCalendarObject::IsValidWeekProfileTable(
  const std::vector<types::WeekProfile>& value)
{
  // Week-profile names must be unique within the table.
  for (std::size_t i = 0u; i < value.size(); ++i) {
    for (std::size_t j = i + 1u; j < value.size(); ++j) {
      if (value[i].Name() == value[j].Name()) {
        return false;
      }
    }
  }
  return true;
}

bool CosemActivityCalendarObject::IsValidDayProfileTable(
  const std::vector<types::DayProfile>& value)
{
  // day_id values are the keys of the day_profile_table and must be
  // unique. Each entry itself enforces its day_schedule invariants via
  // types::DayProfile::IsValid().
  for (std::size_t i = 0u; i < value.size(); ++i) {
    if (!value[i].IsValid()) {
      return false;
    }
    for (std::size_t j = i + 1u; j < value.size(); ++j) {
      if (value[i].DayId() == value[j].DayId()) {
        return false;
      }
    }
  }
  return true;
}

bool CosemActivityCalendarObject::WeekProfileTableSatisfies(
  const std::vector<types::WeekProfile>& weekTable,
  const std::vector<types::DayProfile>& dayTable)
{
  // Every day_id referenced by a week profile must resolve to a row in
  // the day_profile_table. Pure check, no side effects.
  for (std::size_t i = 0u; i < weekTable.size(); ++i) {
    const std::uint8_t ids[7] = {
      weekTable[i].Monday(), weekTable[i].Tuesday(),
      weekTable[i].Wednesday(), weekTable[i].Thursday(),
      weekTable[i].Friday(), weekTable[i].Saturday(),
      weekTable[i].Sunday()};
    for (std::size_t k = 0u; k < 7u; ++k) {
      bool found = false;
      for (std::size_t d = 0u; d < dayTable.size(); ++d) {
        if (dayTable[d].DayId() == ids[k]) {
          found = true;
          break;
        }
      }
      if (!found) {
        return false;
      }
    }
  }
  return true;
}

bool CosemActivityCalendarObject::SeasonProfileSatisfies(
  const std::vector<types::SeasonProfile>& seasonProfile,
  const std::vector<types::WeekProfile>& weekTable)
{
  // Each season's week_name must reference an existing week profile.
  for (std::size_t i = 0u; i < seasonProfile.size(); ++i) {
    bool found = false;
    for (std::size_t j = 0u; j < weekTable.size(); ++j) {
      if (seasonProfile[i].WeekName() == weekTable[j].Name()) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
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
constexpr std::uint8_t kPushSetupResetMethodId = 2u;
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
  }
  if (descriptor_.key.version >= 2u) {
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
  const bool v2 = descriptor_.key.version >= 2u;
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
      if (!v2) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = pushOperationMethod_;
      return CosemStatus::Ok;
    case kPushSetupConfirmationParametersAttributeId:
      if (!v2) { output.clear(); return CosemStatus::AttributeNotFound; }
      output = confirmationParameters_;
      return CosemStatus::Ok;
    case kPushSetupLastConfirmationDateTimeAttributeId:
      if (!v2) { output.clear(); return CosemStatus::AttributeNotFound; }
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
  const bool v2 = descriptor_.key.version >= 2u;
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
      if (!v2) return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      pushOperationMethod_ = input;
      return CosemStatus::Ok;
    case kPushSetupConfirmationParametersAttributeId:
      if (!v2) return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      confirmationParameters_ = input;
      return CosemStatus::Ok;
    case kPushSetupLastConfirmationDateTimeAttributeId:
      if (!v2) return CosemStatus::AttributeNotFound;
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
  // Method 2 (reset) is defined for v2 only (IEC 62056-6-2 ED4,
  // 4.4.8.2.3.2). It resets the push process to its initial state.
  // Without an active push backend, the only persistent state is the
  // last_confirmation_date_time attribute, which we clear here so the
  // method has an observable effect that matches the spec intent.
  if (methodId == kPushSetupResetMethodId) {
    if (descriptor_.key.version < 2u) {
      return CosemStatus::MethodNotFound;
    }
    lastConfirmationDateTime_.clear();
    return CosemStatus::Ok;
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

bool CosemDisconnectControlObject::IsValidControlMode(std::uint8_t raw)
{
  return raw <= 6u;
}

bool CosemDisconnectControlObject::IsValidControlState(std::uint8_t raw)
{
  return raw <= 2u;
}

CosemDisconnectControlObject::CosemDisconnectControlObject(
  const CosemLogicalName& logicalName,
  bool outputState,
  ControlState controlState,
  ControlMode controlMode,
  AttributeAccessMode controlModeAccess)
  : CosemDisconnectControlObject(
      logicalName, outputState, controlState, controlMode,
      controlModeAccess, kVersion0)
{
}

CosemDisconnectControlObject::CosemDisconnectControlObject(
  const CosemLogicalName& logicalName,
  bool outputState,
  ControlState controlState,
  ControlMode controlMode,
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
      output.clear();
      AppendBoolean(output, outputState_);
      return CosemStatus::Ok;
    case kDisconnectControlControlStateAttributeId:
      output.clear();
      AppendEnum(output, static_cast<std::uint8_t>(controlState_));
      return CosemStatus::Ok;
    case kDisconnectControlControlModeAttributeId:
      output.clear();
      AppendEnum(output, static_cast<std::uint8_t>(controlMode_));
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
    case kDisconnectControlControlModeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t raw = 0u;
      if (!ReadEnumValue(input, offset, raw)
          || offset != input.size()
          || !IsValidControlMode(raw)) {
        return CosemStatus::InvalidArgument;
      }
      controlMode_ = static_cast<ControlMode>(raw);
      return CosemStatus::Ok;
    }
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
  // data ::= integer (0); spec allows tolerance — we don't reject
  // payload mismatches because real meters often invoke with empty input.
  const std::uint8_t mode = static_cast<std::uint8_t>(controlMode_);
  if (methodId == kDisconnectControlRemoteDisconnectMethodId) {
    // §4.5.8.3.1: enabled only when control_mode > 0.
    if (mode == 0u) return CosemStatus::UnsupportedFeature;
    controlState_ = ControlState::Disconnected;
    outputState_ = false;
    return CosemStatus::Ok;
  }
  if (methodId == kDisconnectControlRemoteReconnectMethodId) {
    // §4.5.8.3.2:
    //   modes 1, 3, 5, 6 → ready_for_reconnection (output stays FALSE)
    //   modes 2, 4       → connected directly (output TRUE)
    //   mode  0          → not enabled
    if (mode == 0u) return CosemStatus::UnsupportedFeature;
    if (mode == 2u || mode == 4u) {
      controlState_ = ControlState::Connected;
      outputState_ = true;
    } else {
      controlState_ = ControlState::ReadyForReconnection;
      outputState_ = false;
    }
    return CosemStatus::Ok;
  }
  return CosemStatus::MethodNotFound;
}

bool CosemDisconnectControlObject::OutputState() const
{
  return outputState_;
}

CosemDisconnectControlObject::ControlState
CosemDisconnectControlObject::GetControlState() const
{
  return controlState_;
}

CosemDisconnectControlObject::ControlMode
CosemDisconnectControlObject::GetControlMode() const
{
  return controlMode_;
}

void CosemDisconnectControlObject::SetOutputState(bool value)
{
  outputState_ = value;
}

void CosemDisconnectControlObject::SetControlState(ControlState value)
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

bool CosemIecHdlcSetupObject::IsValidCommSpeed(std::uint8_t raw)
{
  return raw <= 9u;
}

bool CosemIecHdlcSetupObject::IsValidWindowSize(std::uint8_t value)
{
  return value >= 1u && value <= 7u;
}

bool CosemIecHdlcSetupObject::IsValidMaxInfoFieldLength(std::uint16_t value)
{
  return value >= 32u && value <= 2030u;
}

bool CosemIecHdlcSetupObject::IsValidInterOctetTimeOut(std::uint16_t value)
{
  return value >= 20u && value <= 6000u;
}

bool CosemIecHdlcSetupObject::IsValidDeviceAddress(std::uint16_t value)
{
  return value >= 0x0010u && value <= 0x3FFDu;
}

namespace {

CosemIecHdlcSetupObject::CommSpeed NormalizeCommSpeed(
  CosemIecHdlcSetupObject::CommSpeed value)
{
  const std::uint8_t raw = static_cast<std::uint8_t>(value);
  return CosemIecHdlcSetupObject::IsValidCommSpeed(raw)
           ? value
           : CosemIecHdlcSetupObject::CommSpeed::Baud9600;
}

std::uint8_t ClampWindowSize(std::uint8_t value)
{
  if (value < 1u) return 1u;
  if (value > 7u) return 7u;
  return value;
}

std::uint16_t ClampMaxInfoFieldLength(std::uint16_t value)
{
  if (value < 32u) return 32u;
  if (value > 2030u) return 2030u;
  return value;
}

std::uint16_t ClampInterOctetTimeOut(std::uint16_t value)
{
  if (value < 20u) return 20u;
  if (value > 6000u) return 6000u;
  return value;
}

std::uint16_t ClampDeviceAddress(std::uint16_t value)
{
  if (value < 0x0010u) return 0x0010u;
  if (value > 0x3FFDu) return 0x3FFDu;
  return value;
}

}  // namespace

CosemIecHdlcSetupObject::CosemIecHdlcSetupObject(
  const CosemLogicalName& logicalName,
  CommSpeed commSpeed,
  std::uint8_t windowSizeTransmit,
  std::uint8_t windowSizeReceive,
  std::uint16_t maxInfoFieldLengthTransmit,
  std::uint16_t maxInfoFieldLengthReceive,
  std::uint16_t interOctetTimeOut,
  std::uint16_t inactivityTimeOut,
  std::uint16_t deviceAddress,
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
  CommSpeed commSpeed,
  std::uint8_t windowSizeTransmit,
  std::uint8_t windowSizeReceive,
  std::uint16_t maxInfoFieldLengthTransmit,
  std::uint16_t maxInfoFieldLengthReceive,
  std::uint16_t interOctetTimeOut,
  std::uint16_t inactivityTimeOut,
  std::uint16_t deviceAddress,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIecHdlcSetupClassId,
      NormalizeVersion(
        version, CosemIecHdlcSetupObject::MaxSupportedVersion),
      logicalName))
  , commSpeed_(NormalizeCommSpeed(commSpeed))
  , windowSizeTransmit_(ClampWindowSize(windowSizeTransmit))
  , windowSizeReceive_(ClampWindowSize(windowSizeReceive))
  , maxInfoFieldLengthTransmit_(
      ClampMaxInfoFieldLength(maxInfoFieldLengthTransmit))
  , maxInfoFieldLengthReceive_(
      ClampMaxInfoFieldLength(maxInfoFieldLengthReceive))
  , interOctetTimeOut_(ClampInterOctetTimeOut(interOctetTimeOut))
  , inactivityTimeOut_(inactivityTimeOut)
  , deviceAddress_(ClampDeviceAddress(deviceAddress))
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
  // and let the backend refresh it via SetDeviceAddress (validated).
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
  output.clear();
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIecHdlcSetupCommSpeedAttributeId:
      AppendEnum(output, static_cast<std::uint8_t>(commSpeed_));
      return CosemStatus::Ok;
    case kIecHdlcSetupWindowSizeTransmitAttributeId:
      AppendUnsigned(output, windowSizeTransmit_);
      return CosemStatus::Ok;
    case kIecHdlcSetupWindowSizeReceiveAttributeId:
      AppendUnsigned(output, windowSizeReceive_);
      return CosemStatus::Ok;
    case kIecHdlcSetupMaxInfoTxAttributeId:
      AppendLongUnsigned(output, maxInfoFieldLengthTransmit_);
      return CosemStatus::Ok;
    case kIecHdlcSetupMaxInfoRxAttributeId:
      AppendLongUnsigned(output, maxInfoFieldLengthReceive_);
      return CosemStatus::Ok;
    case kIecHdlcSetupInterOctetTimeOutAttributeId:
      AppendLongUnsigned(output, interOctetTimeOut_);
      return CosemStatus::Ok;
    case kIecHdlcSetupInactivityTimeOutAttributeId:
      AppendLongUnsigned(output, inactivityTimeOut_);
      return CosemStatus::Ok;
    case kIecHdlcSetupDeviceAddressAttributeId:
      AppendLongUnsigned(output, deviceAddress_);
      return CosemStatus::Ok;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIecHdlcSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIecHdlcSetupCommSpeedAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t raw = 0u;
      if (!ReadEnumValue(input, offset, raw) || offset != input.size() ||
          !IsValidCommSpeed(raw))
        return CosemStatus::InvalidArgument;
      commSpeed_ = static_cast<CommSpeed>(raw);
      return CosemStatus::Ok;
    }
    case kIecHdlcSetupWindowSizeTransmitAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t raw = 0u;
      if (!ReadUnsignedValue(input, offset, raw) || offset != input.size() ||
          !IsValidWindowSize(raw))
        return CosemStatus::InvalidArgument;
      windowSizeTransmit_ = raw;
      return CosemStatus::Ok;
    }
    case kIecHdlcSetupWindowSizeReceiveAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t raw = 0u;
      if (!ReadUnsignedValue(input, offset, raw) || offset != input.size() ||
          !IsValidWindowSize(raw))
        return CosemStatus::InvalidArgument;
      windowSizeReceive_ = raw;
      return CosemStatus::Ok;
    }
    case kIecHdlcSetupMaxInfoTxAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t raw = 0u;
      if (!ReadLongUnsignedValue(input, offset, raw) ||
          offset != input.size() || !IsValidMaxInfoFieldLength(raw))
        return CosemStatus::InvalidArgument;
      maxInfoFieldLengthTransmit_ = raw;
      return CosemStatus::Ok;
    }
    case kIecHdlcSetupMaxInfoRxAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t raw = 0u;
      if (!ReadLongUnsignedValue(input, offset, raw) ||
          offset != input.size() || !IsValidMaxInfoFieldLength(raw))
        return CosemStatus::InvalidArgument;
      maxInfoFieldLengthReceive_ = raw;
      return CosemStatus::Ok;
    }
    case kIecHdlcSetupInterOctetTimeOutAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t raw = 0u;
      if (!ReadLongUnsignedValue(input, offset, raw) ||
          offset != input.size() || !IsValidInterOctetTimeOut(raw))
        return CosemStatus::InvalidArgument;
      interOctetTimeOut_ = raw;
      return CosemStatus::Ok;
    }
    case kIecHdlcSetupInactivityTimeOutAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t raw = 0u;
      if (!ReadLongUnsignedValue(input, offset, raw) ||
          offset != input.size())
        return CosemStatus::InvalidArgument;
      // inactivity_time_out has no upper bound per Blue Book; 0 disables it.
      inactivityTimeOut_ = raw;
      return CosemStatus::Ok;
    }
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

CosemIecHdlcSetupObject::CommSpeed
CosemIecHdlcSetupObject::GetCommSpeed() const
{
  return commSpeed_;
}

std::uint8_t CosemIecHdlcSetupObject::WindowSizeTransmit() const
{
  return windowSizeTransmit_;
}

std::uint8_t CosemIecHdlcSetupObject::WindowSizeReceive() const
{
  return windowSizeReceive_;
}

std::uint16_t CosemIecHdlcSetupObject::MaxInfoFieldLengthTransmit() const
{
  return maxInfoFieldLengthTransmit_;
}

std::uint16_t CosemIecHdlcSetupObject::MaxInfoFieldLengthReceive() const
{
  return maxInfoFieldLengthReceive_;
}

std::uint16_t CosemIecHdlcSetupObject::InterOctetTimeOut() const
{
  return interOctetTimeOut_;
}

std::uint16_t CosemIecHdlcSetupObject::InactivityTimeOut() const
{
  return inactivityTimeOut_;
}

std::uint16_t CosemIecHdlcSetupObject::DeviceAddress() const
{
  return deviceAddress_;
}

bool CosemIecHdlcSetupObject::SetDeviceAddress(std::uint16_t value)
{
  if (!IsValidDeviceAddress(value)) return false;
  deviceAddress_ = value;
  return true;
}

namespace {
constexpr std::uint16_t kRegisterTableClassId = 61u;
constexpr std::uint8_t kRegisterTableTableCellValuesAttributeId = 2u;
constexpr std::uint8_t kRegisterTableTableCellDefinitionAttributeId = 3u;
constexpr std::uint8_t kRegisterTableScalerUnitAttributeId = 4u;
constexpr std::uint8_t kRegisterTableResetMethodId = 1u;
constexpr std::uint8_t kRegisterTableCaptureMethodId = 2u;
} // namespace

const std::uint8_t CosemRegisterTableObject::MaxSupportedVersion;

CosemRegisterTableObject::CosemRegisterTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tableCellValues,
  const CosemByteBuffer& tableCellDefinition,
  const types::ScalerUnit& scalerUnit,
  AttributeAccessMode mutableAccess)
  : CosemRegisterTableObject(
      logicalName, tableCellValues, tableCellDefinition,
      scalerUnit, mutableAccess, kVersion0)
{
}

CosemRegisterTableObject::CosemRegisterTableObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& tableCellValues,
  const CosemByteBuffer& tableCellDefinition,
  const types::ScalerUnit& scalerUnit,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kRegisterTableClassId,
      NormalizeVersion(
        version, CosemRegisterTableObject::MaxSupportedVersion),
      logicalName))
  , tableCellValues_(tableCellValues)
  , tableCellDefinition_(tableCellDefinition)
  , scalerUnit_(scalerUnit)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  // table_cell_values is the dynamic captured payload; treat as RO and
  // let the backend refresh it via SetTableCellValues.
  rights_.SetAttributeAccess(
    kRegisterTableTableCellValuesAttributeId,
    AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kRegisterTableTableCellDefinitionAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kRegisterTableScalerUnitAttributeId, mutableAccess);
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
    case kRegisterTableTableCellDefinitionAttributeId:
      output = tableCellDefinition_;
      return CosemStatus::Ok;
    case kRegisterTableScalerUnitAttributeId:
      output.clear();
      AppendScalerUnit(output, scalerUnit_);
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
    case kRegisterTableTableCellDefinitionAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      tableCellDefinition_ = input;
      return CosemStatus::Ok;
    case kRegisterTableScalerUnitAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      types::ScalerUnit decoded;
      if (!DecodeScalerUnit(input, offset, decoded)
          || offset != input.size())
        return CosemStatus::InvalidArgument;
      scalerUnit_ = decoded;
      return CosemStatus::Ok;
    }
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
    case kRegisterTableResetMethodId:
    case kRegisterTableCaptureMethodId:
      // Both methods drive the captured payload lifecycle which is
      // owned by the backend; surface as UnsupportedFeature.
      return CosemStatus::UnsupportedFeature;
    default:
      return CosemStatus::MethodNotFound;
  }
}

const CosemByteBuffer& CosemRegisterTableObject::TableCellValues() const
{
  return tableCellValues_;
}

const CosemByteBuffer&
CosemRegisterTableObject::TableCellDefinition() const
{
  return tableCellDefinition_;
}

const types::ScalerUnit& CosemRegisterTableObject::ScalerUnit() const
{
  return scalerUnit_;
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

// IEC 62056-6-2 ED4 (2021) §4.9.1.2.4: MSS range is [40, 65535],
// default 576. Lower bound enforced on construction and on write;
// upper bound is the full u16 range, so no explicit check needed.
constexpr std::uint16_t kTcpUdpSetupMssMin = 40u;
constexpr std::uint16_t kTcpUdpSetupMssDefault = 576u;
// §4.9.1.2.5: nb_of_sim_conn min = 1.
constexpr std::uint8_t kTcpUdpSetupNbOfSimConnMin = 1u;
} // namespace

bool CosemTcpUdpSetupObject::IsValidMss(std::uint16_t value)
{
  return value >= kTcpUdpSetupMssMin;
}

bool CosemTcpUdpSetupObject::IsValidNbOfSimConn(std::uint8_t value)
{
  return value >= kTcpUdpSetupNbOfSimConnMin;
}

const std::uint8_t CosemTcpUdpSetupObject::MaxSupportedVersion;

CosemTcpUdpSetupObject::CosemTcpUdpSetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t tcpUdpPort,
  const CosemLogicalName& ipReference,
  std::uint16_t mss,
  std::uint8_t nbOfSimConn,
  std::uint16_t inactivityTimeOut,
  AttributeAccessMode mutableAccess)
  : CosemTcpUdpSetupObject(
      logicalName, tcpUdpPort, ipReference, mss, nbOfSimConn,
      inactivityTimeOut, mutableAccess, kVersion0)
{
}

CosemTcpUdpSetupObject::CosemTcpUdpSetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t tcpUdpPort,
  const CosemLogicalName& ipReference,
  std::uint16_t mss,
  std::uint8_t nbOfSimConn,
  std::uint16_t inactivityTimeOut,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kTcpUdpSetupClassId,
      NormalizeVersion(
        version, CosemTcpUdpSetupObject::MaxSupportedVersion),
      logicalName))
  , tcpUdpPort_(tcpUdpPort)
  , ipReference_(ipReference)
  , mss_(IsValidMss(mss) ? mss : kTcpUdpSetupMssDefault)
  , nbOfSimConn_(
      IsValidNbOfSimConn(nbOfSimConn) ? nbOfSimConn
                                       : kTcpUdpSetupNbOfSimConnMin)
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
  output.clear();
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kTcpUdpSetupTcpUdpPortAttributeId:
      AppendLongUnsigned(output, tcpUdpPort_);
      return CosemStatus::Ok;
    case kTcpUdpSetupIpReferenceAttributeId:
      AppendLogicalName(output, ipReference_);
      return CosemStatus::Ok;
    case kTcpUdpSetupMssAttributeId:
      AppendLongUnsigned(output, mss_);
      return CosemStatus::Ok;
    case kTcpUdpSetupNbOfSimConnAttributeId:
      AppendUnsigned(output, nbOfSimConn_);
      return CosemStatus::Ok;
    case kTcpUdpSetupInactivityTimeOutAttributeId:
      AppendLongUnsigned(output, inactivityTimeOut_);
      return CosemStatus::Ok;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemTcpUdpSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kTcpUdpSetupTcpUdpPortAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t value = 0u;
      if (!ReadLongUnsignedValue(input, offset, value) ||
          offset != input.size())
        return CosemStatus::InvalidArgument;
      tcpUdpPort_ = value;
      return CosemStatus::Ok;
    }
    case kTcpUdpSetupIpReferenceAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      CosemLogicalName value;
      if (!ReadLogicalNameValue(input, offset, value) ||
          offset != input.size())
        return CosemStatus::InvalidArgument;
      ipReference_ = value;
      return CosemStatus::Ok;
    }
    case kTcpUdpSetupMssAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t value = 0u;
      if (!ReadLongUnsignedValue(input, offset, value) ||
          offset != input.size())
        return CosemStatus::InvalidArgument;
      if (!IsValidMss(value)) return CosemStatus::InvalidArgument;
      mss_ = value;
      return CosemStatus::Ok;
    }
    case kTcpUdpSetupNbOfSimConnAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t value = 0u;
      if (!ReadUnsignedValue(input, offset, value) ||
          offset != input.size())
        return CosemStatus::InvalidArgument;
      if (!IsValidNbOfSimConn(value))
        return CosemStatus::InvalidArgument;
      nbOfSimConn_ = value;
      return CosemStatus::Ok;
    }
    case kTcpUdpSetupInactivityTimeOutAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t value = 0u;
      if (!ReadLongUnsignedValue(input, offset, value) ||
          offset != input.size())
        return CosemStatus::InvalidArgument;
      inactivityTimeOut_ = value;
      return CosemStatus::Ok;
    }
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

std::uint16_t CosemTcpUdpSetupObject::TcpUdpPort() const
{
  return tcpUdpPort_;
}

const CosemLogicalName& CosemTcpUdpSetupObject::IpReference() const
{
  return ipReference_;
}

std::uint16_t CosemTcpUdpSetupObject::Mss() const
{
  return mss_;
}

std::uint8_t CosemTcpUdpSetupObject::NbOfSimConn() const
{
  return nbOfSimConn_;
}

std::uint16_t CosemTcpUdpSetupObject::InactivityTimeOut() const
{
  return inactivityTimeOut_;
}

namespace {
constexpr std::uint16_t kScheduleClassId = 10u;
constexpr std::uint8_t kScheduleEntriesAttributeId = 2u;
// IEC 62056-6-2 ED4 (2021) §4.5.3 / DLMS UA Blue Book Ed. 12.1 §5.1.7:
// class_id=10 defines three specific methods.
constexpr std::uint8_t kScheduleEnableDisableMethodId = 1u;
constexpr std::uint8_t kScheduleInsertMethodId = 2u;
constexpr std::uint8_t kScheduleDeleteMethodId = 3u;

// True when no two entries in `entries` share an `index`.
bool ScheduleEntriesIndicesUnique(
  const std::vector<dlms::cosem::types::ScheduleTableEntry>& entries)
{
  for (std::size_t i = 0u; i < entries.size(); ++i) {
    for (std::size_t j = i + 1u; j < entries.size(); ++j) {
      if (entries[i].Index() == entries[j].Index()) return false;
    }
  }
  return true;
}
} // namespace

const std::uint8_t CosemScheduleObject::MaxSupportedVersion;

bool CosemScheduleObject::IsValidEntries(
  const std::vector<types::ScheduleTableEntry>& value)
{
  for (std::size_t i = 0u; i < value.size(); ++i) {
    if (!types::ScheduleTableEntry::IsValid(value[i])) return false;
  }
  return ScheduleEntriesIndicesUnique(value);
}

CosemScheduleObject::CosemScheduleObject(
  const CosemLogicalName& logicalName,
  const std::vector<types::ScheduleTableEntry>& entries,
  AttributeAccessMode entriesAccess)
  : CosemScheduleObject(logicalName, entries, entriesAccess, kVersion0)
{
}

CosemScheduleObject::CosemScheduleObject(
  const CosemLogicalName& logicalName,
  const std::vector<types::ScheduleTableEntry>& entries,
  AttributeAccessMode entriesAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kScheduleClassId,
      NormalizeVersion(
        version, CosemScheduleObject::MaxSupportedVersion),
      logicalName))
  , entries_(IsValidEntries(entries)
             ? entries
             : std::vector<types::ScheduleTableEntry>())
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
      output.clear();
      AppendScheduleEntries(output, entries_);
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
    case kScheduleEntriesAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<types::ScheduleTableEntry> decoded;
      if (!DecodeScheduleEntries(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      if (!IsValidEntries(decoded)) {
        return CosemStatus::InvalidArgument;
      }
      entries_ = decoded;
      return CosemStatus::Ok;
    }
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
  output.clear();
  switch (methodId) {
    case kScheduleEnableDisableMethodId: {
      // data ::= structure { firstA, lastA, firstB, lastB : long-unsigned }
      // Spec §4.5.3.3.1: ranges with first>last are no-ops; first/last
      // beyond 9999 mean "no entry". Disable range A first, then enable
      // range B — strict spec ordering.
      std::size_t offset = 0u;
      std::size_t fieldCount = 0u;
      if (!ReadExpectedTag(input, offset, kStructureTag) ||
          !ReadAxdrLength(input, offset, fieldCount) ||
          fieldCount != 4u) {
        return CosemStatus::InvalidArgument;
      }
      std::uint16_t firstA = 0u, lastA = 0u, firstB = 0u, lastB = 0u;
      if (!ReadLongUnsignedValue(input, offset, firstA) ||
          !ReadLongUnsignedValue(input, offset, lastA) ||
          !ReadLongUnsignedValue(input, offset, firstB) ||
          !ReadLongUnsignedValue(input, offset, lastB) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      auto applyEnable = [this](std::uint16_t lo, std::uint16_t hi, bool on) {
        if (lo > hi) return;
        if (lo > 9999u) return;
        const std::uint16_t cappedHi = (hi > 9999u) ? 9999u : hi;
        for (std::size_t i = 0u; i < entries_.size(); ++i) {
          const std::uint16_t idx = entries_[i].Index();
          if (idx >= lo && idx <= cappedHi) {
            entries_[i].SetEnable(on);
          }
        }
      };
      applyEnable(firstA, lastA, false);
      applyEnable(firstB, lastB, true);
      return CosemStatus::Ok;
    }
    case kScheduleInsertMethodId: {
      // data ::= schedule_table_entry. If entry.index already exists,
      // the existing entry is overwritten by the new one.
      std::size_t offset = 0u;
      types::ScheduleTableEntry entry;
      if (!DecodeScheduleTableEntry(input, offset, entry) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      for (std::size_t i = 0u; i < entries_.size(); ++i) {
        if (entries_[i].Index() == entry.Index()) {
          entries_[i] = entry;
          return CosemStatus::Ok;
        }
      }
      entries_.push_back(entry);
      return CosemStatus::Ok;
    }
    case kScheduleDeleteMethodId: {
      // data ::= structure { firstIndex, lastIndex : long-unsigned }
      // first>last — nothing deleted (spec also allows first==last for
      // single-entry delete, included in the inclusive range below).
      std::size_t offset = 0u;
      std::size_t fieldCount = 0u;
      if (!ReadExpectedTag(input, offset, kStructureTag) ||
          !ReadAxdrLength(input, offset, fieldCount) ||
          fieldCount != 2u) {
        return CosemStatus::InvalidArgument;
      }
      std::uint16_t firstIndex = 0u, lastIndex = 0u;
      if (!ReadLongUnsignedValue(input, offset, firstIndex) ||
          !ReadLongUnsignedValue(input, offset, lastIndex) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      if (firstIndex > lastIndex) {
        return CosemStatus::Ok;
      }
      std::vector<types::ScheduleTableEntry> kept;
      kept.reserve(entries_.size());
      for (std::size_t i = 0u; i < entries_.size(); ++i) {
        const std::uint16_t idx = entries_[i].Index();
        if (idx >= firstIndex && idx <= lastIndex) continue;
        kept.push_back(entries_[i]);
      }
      entries_.swap(kept);
      return CosemStatus::Ok;
    }
    default:
      return CosemStatus::MethodNotFound;
  }
}

const std::vector<types::ScheduleTableEntry>&
CosemScheduleObject::Entries() const
{
  return entries_;
}

bool CosemScheduleObject::SetEntries(
  const std::vector<types::ScheduleTableEntry>& value)
{
  if (!IsValidEntries(value)) return false;
  entries_ = value;
  return true;
}

namespace {
constexpr std::uint16_t kSpecialDaysTableClassId = 11u;
constexpr std::uint8_t kSpecialDaysTableEntriesAttributeId = 2u;
constexpr std::uint8_t kSpecialDaysTableInsertMethodId = 1u;
constexpr std::uint8_t kSpecialDaysTableDeleteMethodId = 2u;

// Encode a single spec_day_entry as AXDR:
//   structure(3) { long-unsigned(index), octet-string(5)=date, unsigned(day_id) }
void AppendSpecialDayEntry(
  CosemByteBuffer& output, const dlms::cosem::types::SpecialDayEntry& entry)
{
  AppendStructureHeader(output, 3u);
  AppendLongUnsigned(output, entry.Index());
  const std::array<std::uint8_t, dlms::cosem::types::Date::WireSize>
    dateBytes = entry.SpecialDayDate().ToBytes();
  AppendOctetString(output, dateBytes.data(), dateBytes.size());
  AppendUnsigned(output, entry.DayId());
}

// Decode the entries array (array of spec_day_entry). Returns false on
// malformed AXDR, wrong tag, wrong length, or invalid date bytes. The
// uniqueness invariant (unique index/date across the collection) is
// enforced by the caller.
bool DecodeSpecialDayEntries(
  const CosemByteBuffer& input,
  std::vector<dlms::cosem::types::SpecialDayEntry>& entries)
{
  entries.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag) ||
      !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  entries.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    std::size_t fieldCount = 0u;
    if (!ReadExpectedTag(input, offset, kStructureTag) ||
        !ReadAxdrLength(input, offset, fieldCount) || fieldCount != 3u) {
      return false;
    }
    std::uint16_t index = 0u;
    if (!ReadLongUnsignedValue(input, offset, index)) {
      return false;
    }
    std::size_t dateLen = 0u;
    const std::uint8_t* dateData = 0;
    if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
        !ReadAxdrLength(input, offset, dateLen) ||
        dateLen != dlms::cosem::types::Date::WireSize ||
        !ReadFixedBytes(input, offset, dateLen, dateData)) {
      return false;
    }
    dlms::cosem::types::Date date;
    if (!dlms::cosem::types::Date::TryFromBytes(dateData, dateLen, date)) {
      return false;
    }
    std::uint8_t dayId = 0u;
    if (!ReadUnsignedValue(input, offset, dayId)) {
      return false;
    }
    entries.push_back(
      dlms::cosem::types::SpecialDayEntry(index, date, dayId));
  }
  if (offset != input.size()) {
    return false;
  }
  return true;
}

// Decode a single spec_day_entry payload (no array wrapper) used by the
// insert(data) method. Trailing bytes are rejected.
bool DecodeSpecialDayEntryPayload(
  const CosemByteBuffer& input, dlms::cosem::types::SpecialDayEntry& entry)
{
  std::size_t offset = 0u;
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) || fieldCount != 3u) {
    return false;
  }
  std::uint16_t index = 0u;
  if (!ReadLongUnsignedValue(input, offset, index)) {
    return false;
  }
  std::size_t dateLen = 0u;
  const std::uint8_t* dateData = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, dateLen) ||
      dateLen != dlms::cosem::types::Date::WireSize ||
      !ReadFixedBytes(input, offset, dateLen, dateData)) {
    return false;
  }
  dlms::cosem::types::Date date;
  if (!dlms::cosem::types::Date::TryFromBytes(dateData, dateLen, date)) {
    return false;
  }
  std::uint8_t dayId = 0u;
  if (!ReadUnsignedValue(input, offset, dayId)) {
    return false;
  }
  if (offset != input.size()) {
    return false;
  }
  entry = dlms::cosem::types::SpecialDayEntry(index, date, dayId);
  return true;
}

} // namespace

const std::uint8_t CosemSpecialDaysTableObject::MaxSupportedVersion;

CosemSpecialDaysTableObject::CosemSpecialDaysTableObject(
  const CosemLogicalName& logicalName,
  const std::vector<types::SpecialDayEntry>& entries,
  AttributeAccessMode entriesAccess)
  : CosemSpecialDaysTableObject(
      logicalName, entries, entriesAccess, kVersion0)
{
}

CosemSpecialDaysTableObject::CosemSpecialDaysTableObject(
  const CosemLogicalName& logicalName,
  const std::vector<types::SpecialDayEntry>& entries,
  AttributeAccessMode entriesAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSpecialDaysTableClassId,
      NormalizeVersion(
        version, CosemSpecialDaysTableObject::MaxSupportedVersion),
      logicalName))
  , entries_()
{
  // Safe fallback: if the caller passes a collection that violates
  // the uniqueness invariant, start empty rather than holding an
  // invalid state. Per spec the table may legitimately be empty.
  if (IsValidEntries(entries)) {
    entries_ = entries;
  }
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
      output.clear();
      AppendArrayHeader(output, entries_.size());
      for (std::size_t i = 0u; i < entries_.size(); ++i) {
        AppendSpecialDayEntry(output, entries_[i]);
      }
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
    case kSpecialDaysTableEntriesAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<types::SpecialDayEntry> decoded;
      if (!DecodeSpecialDayEntries(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      if (!IsValidEntries(decoded)) {
        return CosemStatus::InvalidArgument;
      }
      entries_.swap(decoded);
      return CosemStatus::Ok;
    }
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
  output.clear();
  switch (methodId) {
    case kSpecialDaysTableInsertMethodId: {
      types::SpecialDayEntry entry;
      if (!DecodeSpecialDayEntryPayload(input, entry)) {
        return CosemStatus::InvalidArgument;
      }
      // Per spec: overwrite-by-index or overwrite-by-date semantics.
      // Insert() handles both.
      Insert(entry);
      return CosemStatus::Ok;
    }
    case kSpecialDaysTableDeleteMethodId: {
      std::size_t offset = 0u;
      std::uint16_t index = 0u;
      if (!ReadLongUnsignedValue(input, offset, index) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      // Spec doesn't mandate an error for missing index, so a no-op
      // delete still returns Ok (Delete() reports the bool to local
      // callers, but the method just acknowledges).
      (void)Delete(index);
      return CosemStatus::Ok;
    }
    default:
      return CosemStatus::MethodNotFound;
  }
}

const std::vector<types::SpecialDayEntry>&
CosemSpecialDaysTableObject::Entries() const
{
  return entries_;
}

bool CosemSpecialDaysTableObject::SetEntries(
  const std::vector<types::SpecialDayEntry>& value)
{
  if (!IsValidEntries(value)) {
    return false;
  }
  entries_ = value;
  return true;
}

bool CosemSpecialDaysTableObject::Insert(const types::SpecialDayEntry& entry)
{
  // Spec §4.5.4.3.1: if an entry with the same index OR the same date
  // already exists, the old entry is overwritten. We resolve both keys
  // independently — if the new entry collides with two distinct old
  // entries (one by index, one by date), both old entries are removed
  // before insertion so the post-condition (unique index, unique date)
  // still holds.
  for (std::size_t i = entries_.size(); i > 0u; --i) {
    const std::size_t idx = i - 1u;
    if (entries_[idx].Index() == entry.Index() ||
        entries_[idx].SpecialDayDate() == entry.SpecialDayDate()) {
      entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(idx));
    }
  }
  entries_.push_back(entry);
  return true;
}

bool CosemSpecialDaysTableObject::Delete(std::uint16_t index)
{
  for (std::size_t i = 0u; i < entries_.size(); ++i) {
    if (entries_[i].Index() == index) {
      entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(i));
      return true;
    }
  }
  return false;
}

bool CosemSpecialDaysTableObject::IsValidEntries(
  const std::vector<types::SpecialDayEntry>& value)
{
  for (std::size_t i = 0u; i < value.size(); ++i) {
    for (std::size_t j = i + 1u; j < value.size(); ++j) {
      if (value[i].Index() == value[j].Index()) {
        return false;
      }
      if (value[i].SpecialDayDate() == value[j].SpecialDayDate()) {
        return false;
      }
    }
  }
  return true;
}

namespace {
constexpr std::uint16_t kSingleActionScheduleClassId = 22u;
constexpr std::uint8_t kSingleActionScheduleExecutedScriptAttributeId = 2u;
constexpr std::uint8_t kSingleActionScheduleTypeAttributeId = 3u;
constexpr std::uint8_t kSingleActionScheduleExecutionTimeAttributeId = 4u;

std::vector<CosemSingleActionScheduleObject::ExecutionTimeEntry>
MakeFallbackExecutionTime()
{
  // type=1 requires exactly one entry. Use an all-wildcard time + date,
  // which is always a legal value: wildcards in date are allowed for
  // type=1 and the per-field 0xFF time wildcard satisfies the
  // hundredths==0xFF (== unspecified, not the numeric 0) sanity check
  // below.
  std::vector<CosemSingleActionScheduleObject::ExecutionTimeEntry> out;
  out.push_back(std::make_pair(
    dlms::cosem::types::Time(), dlms::cosem::types::Date()));
  return out;
}
} // namespace

const std::uint8_t CosemSingleActionScheduleObject::MaxSupportedVersion;

bool CosemSingleActionScheduleObject::IsValidExecutionTime(
  const types::SingleActionScheduleType& type,
  const std::vector<ExecutionTimeEntry>& executionTime)
{
  if (executionTime.empty()) {
    return false;
  }
  if (type.RequiresSingleEntry() && executionTime.size() != 1u) {
    return false;
  }

  for (std::size_t i = 0u; i < executionTime.size(); ++i) {
    const types::Time& t = executionTime[i].first;
    // Spec §4.5.7.2.4: "Hundredths of seconds shall be zero." Accept
    // either the literal 0 or the per-field 0xFF wildcard (which still
    // round-trips through the wire untouched and is the only other
    // legitimate way to express "no value").
    if (!t.HundredthsUnspecified() && t.Hundredths() != 0u) {
      return false;
    }
  }

  if (type.RequiresUniformTime()) {
    const types::Time& first = executionTime[0].first;
    for (std::size_t i = 1u; i < executionTime.size(); ++i) {
      if (!(executionTime[i].first == first)) {
        return false;
      }
    }
  }

  if (type.ForbidsWildcardsInDate()) {
    for (std::size_t i = 0u; i < executionTime.size(); ++i) {
      if (DateHasAnyWildcard(executionTime[i].second)) {
        return false;
      }
    }
  }

  return true;
}

CosemSingleActionScheduleObject::CosemSingleActionScheduleObject(
  const CosemLogicalName& logicalName,
  const types::Script& executedScript,
  const types::SingleActionScheduleType& type,
  const std::vector<ExecutionTimeEntry>& executionTime,
  AttributeAccessMode mutableAccess)
  : CosemSingleActionScheduleObject(
      logicalName, executedScript, type, executionTime,
      mutableAccess, kVersion0)
{
}

CosemSingleActionScheduleObject::CosemSingleActionScheduleObject(
  const CosemLogicalName& logicalName,
  const types::Script& executedScript,
  const types::SingleActionScheduleType& type,
  const std::vector<ExecutionTimeEntry>& executionTime,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSingleActionScheduleClassId,
      NormalizeVersion(
        version, CosemSingleActionScheduleObject::MaxSupportedVersion),
      logicalName))
  , executedScript_(executedScript)
  , type_(type)
  , executionTime_(
      IsValidExecutionTime(type, executionTime)
        ? executionTime
        : MakeFallbackExecutionTime())
{
  // If the caller-supplied execution_time violated the invariants we
  // fell back to a safe empty single-entry schedule, which forces type
  // back to 1 so the stored pair stays consistent.
  if (!IsValidExecutionTime(type, executionTime)) {
    type_ = types::SingleActionScheduleType(1u);
  }

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
      output.clear();
      AppendScript(output, executedScript_);
      return CosemStatus::Ok;
    case kSingleActionScheduleTypeAttributeId:
      output.clear();
      AppendEnum(output, type_.Value());
      return CosemStatus::Ok;
    case kSingleActionScheduleExecutionTimeAttributeId:
      output.clear();
      AppendExecutionTime(output, executionTime_);
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
    case kSingleActionScheduleExecutedScriptAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      types::Script decoded;
      if (!DecodeScript(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      executedScript_ = decoded;
      return CosemStatus::Ok;
    }
    case kSingleActionScheduleTypeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t raw = 0u;
      if (!ReadEnumValue(input, offset, raw) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      if (!types::SingleActionScheduleType::IsValid(raw)) {
        return CosemStatus::InvalidArgument;
      }
      const types::SingleActionScheduleType candidate(raw);
      if (!IsValidExecutionTime(candidate, executionTime_)) {
        return CosemStatus::InvalidArgument;
      }
      type_ = candidate;
      return CosemStatus::Ok;
    }
    case kSingleActionScheduleExecutionTimeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<ExecutionTimeEntry> decoded;
      if (!DecodeExecutionTime(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      if (!IsValidExecutionTime(type_, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      executionTime_ = decoded;
      return CosemStatus::Ok;
    }
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

const types::Script&
CosemSingleActionScheduleObject::ExecutedScript() const
{
  return executedScript_;
}

const types::SingleActionScheduleType&
CosemSingleActionScheduleObject::Type() const
{
  return type_;
}

const std::vector<
  CosemSingleActionScheduleObject::ExecutionTimeEntry>&
CosemSingleActionScheduleObject::ExecutionTime() const
{
  return executionTime_;
}

void CosemSingleActionScheduleObject::SetExecutedScript(
  const types::Script& value)
{
  executedScript_ = value;
}

bool CosemSingleActionScheduleObject::SetType(
  const types::SingleActionScheduleType& value)
{
  if (!IsValidExecutionTime(value, executionTime_)) {
    return false;
  }
  type_ = value;
  return true;
}

bool CosemSingleActionScheduleObject::SetExecutionTime(
  const std::vector<ExecutionTimeEntry>& value)
{
  if (!IsValidExecutionTime(type_, value)) {
    return false;
  }
  executionTime_ = value;
  return true;
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
constexpr std::uint8_t kAutoConnectConnectMethodId = 1u;
} // namespace

namespace {

void AppendCallingWindowEntry(
  CosemByteBuffer& output,
  const CosemAutoConnectObject::CallingWindowEntry& entry)
{
  AppendStructureHeader(output, 2u);
  AppendDateTimeOctetString(output, entry.start);
  AppendDateTimeOctetString(output, entry.end);
}

bool ReadStreamingDateTime(
  const CosemByteBuffer& input,
  std::size_t& offset,
  dlms::cosem::types::DateTime& out)
{
  std::size_t length = 0u;
  const std::uint8_t* data = 0;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag)
      || !ReadAxdrLength(input, offset, length)
      || length != dlms::cosem::types::DateTime::WireSize
      || !ReadFixedBytes(input, offset, length, data)) {
    return false;
  }
  return dlms::cosem::types::DateTime::TryFromBytes(data, length, out);
}

bool DecodeCallingWindowEntry(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemAutoConnectObject::CallingWindowEntry& out)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag)
      || !ReadAxdrLength(input, offset, fieldCount)
      || fieldCount != 2u) {
    return false;
  }
  return ReadStreamingDateTime(input, offset, out.start)
    && ReadStreamingDateTime(input, offset, out.end);
}

bool DecodeCallingWindowPayload(
  const CosemByteBuffer& input,
  std::vector<CosemAutoConnectObject::CallingWindowEntry>& out)
{
  out.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  out.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    CosemAutoConnectObject::CallingWindowEntry entry;
    if (!DecodeCallingWindowEntry(input, offset, entry)) {
      return false;
    }
    out.push_back(entry);
  }
  return offset == input.size();
}

void AppendDestinationList(
  CosemByteBuffer& output,
  const std::vector<std::vector<std::uint8_t>>& list)
{
  AppendArrayHeader(output, list.size());
  for (std::size_t i = 0u; i < list.size(); ++i) {
    const std::vector<std::uint8_t>& entry = list[i];
    static const std::uint8_t kEmpty = 0u;
    AppendOctetString(
      output,
      entry.empty() ? &kEmpty : &entry[0],
      entry.size());
  }
}

bool DecodeDestinationList(
  const CosemByteBuffer& input,
  std::vector<std::vector<std::uint8_t>>& out)
{
  out.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  out.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    std::size_t length = 0u;
    const std::uint8_t* data = 0;
    if (!ReadExpectedTag(input, offset, kDataOctetStringTag)
        || !ReadAxdrLength(input, offset, length)
        || !ReadFixedBytes(input, offset, length, data)) {
      return false;
    }
    out.push_back(std::vector<std::uint8_t>(data, data + length));
  }
  return offset == input.size();
}

} // namespace

const std::uint8_t CosemAutoConnectObject::MaxSupportedVersion;

CosemAutoConnectObject::CosemAutoConnectObject(
  const CosemLogicalName& logicalName,
  std::uint8_t mode,
  std::uint8_t repetitions,
  std::uint16_t repetitionDelay,
  const std::vector<CallingWindowEntry>& callingWindow,
  const std::vector<std::vector<std::uint8_t>>& destinationList,
  AttributeAccessMode mutableAccess)
  : CosemAutoConnectObject(
      logicalName, mode, repetitions, repetitionDelay,
      callingWindow, destinationList, mutableAccess,
      static_cast<std::uint8_t>(2u))
{
}

CosemAutoConnectObject::CosemAutoConnectObject(
  const CosemLogicalName& logicalName,
  std::uint8_t mode,
  std::uint8_t repetitions,
  std::uint16_t repetitionDelay,
  const std::vector<CallingWindowEntry>& callingWindow,
  const std::vector<std::vector<std::uint8_t>>& destinationList,
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
  output.clear();
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kAutoConnectModeAttributeId:
      AppendEnum(output, mode_);
      return CosemStatus::Ok;
    case kAutoConnectRepetitionsAttributeId:
      AppendUnsigned(output, repetitions_);
      return CosemStatus::Ok;
    case kAutoConnectRepetitionDelayAttributeId:
      AppendLongUnsigned(output, repetitionDelay_);
      return CosemStatus::Ok;
    case kAutoConnectCallingWindowAttributeId:
      AppendArrayHeader(output, callingWindow_.size());
      for (std::size_t i = 0u; i < callingWindow_.size(); ++i) {
        AppendCallingWindowEntry(output, callingWindow_[i]);
      }
      return CosemStatus::Ok;
    case kAutoConnectDestinationListAttributeId:
      AppendDestinationList(output, destinationList_);
      return CosemStatus::Ok;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAutoConnectObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kAutoConnectModeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t value = 0u;
      if (!ReadEnumValue(input, offset, value) || offset != input.size())
        return CosemStatus::InvalidArgument;
      mode_ = value;
      return CosemStatus::Ok;
    }
    case kAutoConnectRepetitionsAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t value = 0u;
      if (!ReadUnsignedValue(input, offset, value) || offset != input.size())
        return CosemStatus::InvalidArgument;
      repetitions_ = value;
      return CosemStatus::Ok;
    }
    case kAutoConnectRepetitionDelayAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t value = 0u;
      if (!ReadLongUnsignedValue(input, offset, value) || offset != input.size())
        return CosemStatus::InvalidArgument;
      repetitionDelay_ = value;
      return CosemStatus::Ok;
    }
    case kAutoConnectCallingWindowAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<CallingWindowEntry> next;
      if (!DecodeCallingWindowPayload(input, next))
        return CosemStatus::InvalidArgument;
      callingWindow_ = next;
      return CosemStatus::Ok;
    }
    case kAutoConnectDestinationListAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<std::vector<std::uint8_t>> next;
      if (!DecodeDestinationList(input, next))
        return CosemStatus::InvalidArgument;
      destinationList_ = next;
      return CosemStatus::Ok;
    }
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
  (void)input;
  output.clear();
  // The "connect" method (id 1) is only defined from class
  // version 2 onward (Auto connect). The legacy v0 PSTN auto
  // dial IC defines no methods.
  if (methodId == kAutoConnectConnectMethodId &&
      descriptor_.key.version >= static_cast<std::uint8_t>(2u)) {
    // Built-in object does not own the dialler / radio stack;
    // the backend is expected to drive the connect attempt
    // out-of-band.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

namespace {
constexpr std::uint16_t kGprsModemSetupClassId = 45u;
constexpr std::uint8_t kGprsModemSetupApnAttributeId = 2u;
constexpr std::uint8_t kGprsModemSetupPinCodeAttributeId = 3u;
constexpr std::uint8_t kGprsModemSetupQualityOfServiceAttributeId = 4u;
} // namespace

const std::uint8_t CosemGprsModemSetupObject::MaxSupportedVersion;

namespace {

void AppendQosElement(
  CosemByteBuffer& output, const types::QosElement& element)
{
  AppendStructureHeader(output, 5u);
  AppendUnsigned(output, element.Precedence());
  AppendUnsigned(output, element.Delay());
  AppendUnsigned(output, element.Reliability());
  AppendUnsigned(output, element.PeakThroughput());
  AppendUnsigned(output, element.MeanThroughput());
}

void AppendQualityOfService(
  CosemByteBuffer& output, const types::QualityOfService& qos)
{
  AppendStructureHeader(output, 2u);
  AppendQosElement(output, qos.Default());
  AppendQosElement(output, qos.Requested());
}

bool DecodeQosElement(
  const CosemByteBuffer& input, std::size_t& offset,
  types::QosElement& element)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 5u) {
    return false;
  }
  std::uint8_t precedence = 0u;
  std::uint8_t delay = 0u;
  std::uint8_t reliability = 0u;
  std::uint8_t peak = 0u;
  std::uint8_t mean = 0u;
  if (!ReadUnsignedValue(input, offset, precedence) ||
      !ReadUnsignedValue(input, offset, delay) ||
      !ReadUnsignedValue(input, offset, reliability) ||
      !ReadUnsignedValue(input, offset, peak) ||
      !ReadUnsignedValue(input, offset, mean)) {
    return false;
  }
  element = types::QosElement(precedence, delay, reliability, peak, mean);
  return true;
}

bool DecodeQualityOfService(
  const CosemByteBuffer& input, types::QualityOfService& qos)
{
  std::size_t offset = 0u;
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag) ||
      !ReadAxdrLength(input, offset, fieldCount) ||
      fieldCount != 2u) {
    return false;
  }
  types::QosElement dflt;
  types::QosElement requested;
  if (!DecodeQosElement(input, offset, dflt) ||
      !DecodeQosElement(input, offset, requested)) {
    return false;
  }
  if (offset != input.size()) {
    return false;
  }
  qos = types::QualityOfService(dflt, requested);
  return true;
}

bool DecodeApn(
  const CosemByteBuffer& input, std::vector<std::uint8_t>& apn)
{
  std::size_t offset = 0u;
  std::size_t length = 0u;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, length)) {
    return false;
  }
  if (input.size() - offset != length) {
    return false;
  }
  apn.assign(
    input.begin() + static_cast<std::ptrdiff_t>(offset),
    input.begin() + static_cast<std::ptrdiff_t>(offset + length));
  return true;
}

bool DecodePinCode(
  const CosemByteBuffer& input, std::uint16_t& pin)
{
  std::size_t offset = 0u;
  if (!ReadLongUnsignedValue(input, offset, pin)) {
    return false;
  }
  return offset == input.size();
}

}  // namespace

CosemGprsModemSetupObject::CosemGprsModemSetupObject(
  const CosemLogicalName& logicalName,
  const std::vector<std::uint8_t>& apn,
  std::uint16_t pinCode,
  const types::QualityOfService& qualityOfService,
  AttributeAccessMode mutableAccess)
  : CosemGprsModemSetupObject(
      logicalName, apn, pinCode, qualityOfService, mutableAccess, kVersion0)
{
}

CosemGprsModemSetupObject::CosemGprsModemSetupObject(
  const CosemLogicalName& logicalName,
  const std::vector<std::uint8_t>& apn,
  std::uint16_t pinCode,
  const types::QualityOfService& qualityOfService,
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
      output.clear();
      AppendOctetString(output, apn_.data(), apn_.size());
      return CosemStatus::Ok;
    case kGprsModemSetupPinCodeAttributeId:
      output.clear();
      AppendLongUnsigned(output, pinCode_);
      return CosemStatus::Ok;
    case kGprsModemSetupQualityOfServiceAttributeId:
      output.clear();
      AppendQualityOfService(output, qualityOfService_);
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
    case kGprsModemSetupApnAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<std::uint8_t> decoded;
      if (!DecodeApn(input, decoded))
        return CosemStatus::InvalidArgument;
      apn_ = std::move(decoded);
      return CosemStatus::Ok;
    }
    case kGprsModemSetupPinCodeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::uint16_t value = 0u;
      if (!DecodePinCode(input, value))
        return CosemStatus::InvalidArgument;
      pinCode_ = value;
      return CosemStatus::Ok;
    }
    case kGprsModemSetupQualityOfServiceAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      types::QualityOfService decoded;
      if (!DecodeQualityOfService(input, decoded))
        return CosemStatus::InvalidArgument;
      qualityOfService_ = decoded;
      return CosemStatus::Ok;
    }
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

namespace {
constexpr std::uint16_t kAutoAnswerClassId = 28u;
constexpr std::uint8_t kAutoAnswerModeAttributeId = 2u;
constexpr std::uint8_t kAutoAnswerListeningWindowAttributeId = 3u;
constexpr std::uint8_t kAutoAnswerStatusAttributeId = 4u;
constexpr std::uint8_t kAutoAnswerNumberOfCallsAttributeId = 5u;
constexpr std::uint8_t kAutoAnswerNumberOfRingsAttributeId = 6u;
constexpr std::uint8_t kAutoAnswerListOfAllowedCallersAttributeId = 7u;
} // namespace

namespace {

void AppendListeningWindowEntry(
  CosemByteBuffer& output,
  const CosemAutoAnswerObject::ListeningWindowEntry& entry)
{
  AppendStructureHeader(output, 2u);
  AppendDateTimeOctetString(output, entry.start);
  AppendDateTimeOctetString(output, entry.end);
}

bool DecodeListeningWindowEntry(
  const CosemByteBuffer& input,
  std::size_t& offset,
  CosemAutoAnswerObject::ListeningWindowEntry& out)
{
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag)
      || !ReadAxdrLength(input, offset, fieldCount)
      || fieldCount != 2u) {
    return false;
  }
  return ReadStreamingDateTime(input, offset, out.start)
    && ReadStreamingDateTime(input, offset, out.end);
}

bool DecodeListeningWindowPayload(
  const CosemByteBuffer& input,
  std::vector<CosemAutoAnswerObject::ListeningWindowEntry>& out)
{
  out.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  out.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    CosemAutoAnswerObject::ListeningWindowEntry entry;
    if (!DecodeListeningWindowEntry(input, offset, entry)) {
      return false;
    }
    out.push_back(entry);
  }
  return offset == input.size();
}

void AppendNumberOfRings(
  CosemByteBuffer& output,
  const CosemAutoAnswerObject::NumberOfRings& value)
{
  AppendStructureHeader(output, 2u);
  AppendUnsigned(output, value.inWindow);
  AppendUnsigned(output, value.outOfWindow);
}

bool DecodeNumberOfRings(
  const CosemByteBuffer& input,
  CosemAutoAnswerObject::NumberOfRings& out)
{
  std::size_t offset = 0u;
  std::size_t fieldCount = 0u;
  if (!ReadExpectedTag(input, offset, kStructureTag)
      || !ReadAxdrLength(input, offset, fieldCount)
      || fieldCount != 2u
      || !ReadUnsignedValue(input, offset, out.inWindow)
      || !ReadUnsignedValue(input, offset, out.outOfWindow)) {
    return false;
  }
  return offset == input.size();
}

void AppendAllowedCaller(
  CosemByteBuffer& output,
  const CosemAutoAnswerObject::AllowedCaller& caller)
{
  AppendStructureHeader(output, 2u);
  static const std::uint8_t kEmpty = 0u;
  AppendOctetString(
    output,
    caller.callerId.empty() ? &kEmpty : &caller.callerId[0],
    caller.callerId.size());
  AppendEnum(output, caller.callType);
}

bool DecodeAllowedCallersPayload(
  const CosemByteBuffer& input,
  std::vector<CosemAutoAnswerObject::AllowedCaller>& out)
{
  out.clear();
  std::size_t offset = 0u;
  std::size_t count = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag)
      || !ReadAxdrLength(input, offset, count)) {
    return false;
  }
  out.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    std::size_t fieldCount = 0u;
    if (!ReadExpectedTag(input, offset, kStructureTag)
        || !ReadAxdrLength(input, offset, fieldCount)
        || fieldCount != 2u) {
      return false;
    }
    CosemAutoAnswerObject::AllowedCaller caller;
    std::size_t length = 0u;
    const std::uint8_t* data = 0;
    if (!ReadExpectedTag(input, offset, kDataOctetStringTag)
        || !ReadAxdrLength(input, offset, length)
        || !ReadFixedBytes(input, offset, length, data)
        || !ReadEnumValue(input, offset, caller.callType)) {
      return false;
    }
    caller.callerId.assign(data, data + length);
    out.push_back(caller);
  }
  return offset == input.size();
}

} // namespace

const std::uint8_t CosemAutoAnswerObject::MaxSupportedVersion;

CosemAutoAnswerObject::CosemAutoAnswerObject(
  const CosemLogicalName& logicalName,
  std::uint8_t mode,
  const std::vector<ListeningWindowEntry>& listeningWindow,
  std::uint8_t status,
  std::uint8_t numberOfCalls,
  NumberOfRings numberOfRings,
  const std::vector<AllowedCaller>& listOfAllowedCallers,
  AttributeAccessMode mutableAccess)
  : CosemAutoAnswerObject(
      logicalName, mode, listeningWindow, status, numberOfCalls,
      numberOfRings, listOfAllowedCallers, mutableAccess, kVersion0)
{
}

CosemAutoAnswerObject::CosemAutoAnswerObject(
  const CosemLogicalName& logicalName,
  std::uint8_t mode,
  const std::vector<ListeningWindowEntry>& listeningWindow,
  std::uint8_t status,
  std::uint8_t numberOfCalls,
  NumberOfRings numberOfRings,
  const std::vector<AllowedCaller>& listOfAllowedCallers,
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
  , listOfAllowedCallers_(listOfAllowedCallers)
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
  rights_.SetAttributeAccess(
    kAutoAnswerListOfAllowedCallersAttributeId, mutableAccess);
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
  output.clear();
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kAutoAnswerModeAttributeId:
      AppendEnum(output, mode_);
      return CosemStatus::Ok;
    case kAutoAnswerListeningWindowAttributeId:
      AppendArrayHeader(output, listeningWindow_.size());
      for (std::size_t i = 0u; i < listeningWindow_.size(); ++i) {
        AppendListeningWindowEntry(output, listeningWindow_[i]);
      }
      return CosemStatus::Ok;
    case kAutoAnswerStatusAttributeId:
      AppendEnum(output, status_);
      return CosemStatus::Ok;
    case kAutoAnswerNumberOfCallsAttributeId:
      AppendUnsigned(output, numberOfCalls_);
      return CosemStatus::Ok;
    case kAutoAnswerNumberOfRingsAttributeId:
      AppendNumberOfRings(output, numberOfRings_);
      return CosemStatus::Ok;
    case kAutoAnswerListOfAllowedCallersAttributeId:
      AppendArrayHeader(output, listOfAllowedCallers_.size());
      for (std::size_t i = 0u; i < listOfAllowedCallers_.size(); ++i) {
        AppendAllowedCaller(output, listOfAllowedCallers_[i]);
      }
      return CosemStatus::Ok;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemAutoAnswerObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kAutoAnswerModeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t value = 0u;
      if (!ReadEnumValue(input, offset, value) || offset != input.size())
        return CosemStatus::InvalidArgument;
      mode_ = value;
      return CosemStatus::Ok;
    }
    case kAutoAnswerListeningWindowAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<ListeningWindowEntry> next;
      if (!DecodeListeningWindowPayload(input, next))
        return CosemStatus::InvalidArgument;
      listeningWindow_.swap(next);
      return CosemStatus::Ok;
    }
    case kAutoAnswerNumberOfCallsAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t value = 0u;
      if (!ReadUnsignedValue(input, offset, value) || offset != input.size())
        return CosemStatus::InvalidArgument;
      numberOfCalls_ = value;
      return CosemStatus::Ok;
    }
    case kAutoAnswerNumberOfRingsAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      NumberOfRings next{0u, 0u};
      if (!DecodeNumberOfRings(input, next))
        return CosemStatus::InvalidArgument;
      numberOfRings_ = next;
      return CosemStatus::Ok;
    }
    case kAutoAnswerListOfAllowedCallersAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<AllowedCaller> next;
      if (!DecodeAllowedCallersPayload(input, next))
        return CosemStatus::InvalidArgument;
      listOfAllowedCallers_.swap(next);
      return CosemStatus::Ok;
    }
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

std::uint8_t CosemAutoAnswerObject::Mode() const
{
  return mode_;
}

const std::vector<CosemAutoAnswerObject::ListeningWindowEntry>&
CosemAutoAnswerObject::ListeningWindow() const
{
  return listeningWindow_;
}

std::uint8_t CosemAutoAnswerObject::Status() const
{
  return status_;
}

std::uint8_t CosemAutoAnswerObject::NumberOfCalls() const
{
  return numberOfCalls_;
}

CosemAutoAnswerObject::NumberOfRings
CosemAutoAnswerObject::GetNumberOfRings() const
{
  return numberOfRings_;
}

const std::vector<CosemAutoAnswerObject::AllowedCaller>&
CosemAutoAnswerObject::ListOfAllowedCallers() const
{
  return listOfAllowedCallers_;
}

void CosemAutoAnswerObject::SetStatus(std::uint8_t status)
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
// IEC 62056-6-2 ED4 (2021) §4.9.2.3.3 defines a third method on class_id=42.
constexpr std::uint8_t kIpv4SetupGetNbofMcIpAddressesMethodId = 3u;
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
    case kIpv4SetupGetNbofMcIpAddressesMethodId:
      // Built-in object does not own multicast subscription policy nor
      // expose the runtime multicast_IP_address array size. All three
      // spec methods are surfaced as UnsupportedFeature so a backend
      // can attach the IGMP/array-size semantics without changing the
      // object surface.
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
  const MacAddressBytes& macAddress,
  AttributeAccessMode mutableAccess)
  : CosemMacAddressSetupObject(
      logicalName, macAddress, mutableAccess, kVersion0)
{
}

CosemMacAddressSetupObject::CosemMacAddressSetupObject(
  const CosemLogicalName& logicalName,
  const MacAddressBytes& macAddress,
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
      output.clear();
      AppendOctetString(output, macAddress_.data(), macAddress_.size());
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
    case kMacAddressSetupMacAddressAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      // Expect tag 0x09 (octet-string), length 0x06, six MAC bytes
      // and nothing else.
      if (input.size() != 8u
          || input[0] != kDataOctetStringTag
          || input[1] != 0x06u) {
        return CosemStatus::InvalidArgument;
      }
      MacAddressBytes next{};
      for (std::size_t i = 0u; i < next.size(); ++i)
        next[i] = input[2u + i];
      macAddress_ = next;
      return CosemStatus::Ok;
    }
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

const CosemMacAddressSetupObject::MacAddressBytes&
CosemMacAddressSetupObject::MacAddress() const
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
constexpr std::uint8_t kSmtpSetupServerPortAttributeId = 2u;
constexpr std::uint8_t kSmtpSetupUserNameAttributeId = 3u;
constexpr std::uint8_t kSmtpSetupLoginPasswordAttributeId = 4u;
constexpr std::uint8_t kSmtpSetupServerAddressAttributeId = 5u;
constexpr std::uint8_t kSmtpSetupSenderAddressAttributeId = 6u;
// IEC 62056-6-2 ED4 (2021) §4.9.6 / DLMS UA Blue Book Ed. 12.1 §4.4.7
// define class_id=46, version=0 with six attributes:
//   1 logical_name      octet-string(6)  (RO)
//   2 server_port       long-unsigned    (default 25, IANA SMTP)
//   3 user_name         octet-string
//   4 login_password    octet-string     (empty = no auth)
//   5 server_address    octet-string     (DNS name or dotted IP)
//   6 sender_address    octet-string
// No specific methods are defined.

bool DecodeSmtpOctetString(
  const CosemByteBuffer& input,
  std::vector<std::uint8_t>& value)
{
  std::size_t offset = 0u;
  std::size_t length = 0u;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, length) ||
      input.size() - offset < length) {
    return false;
  }
  value.assign(input.begin() + offset, input.begin() + offset + length);
  offset += length;
  if (offset != input.size()) {
    return false;
  }
  return true;
}
} // namespace

const std::uint8_t CosemSmtpSetupObject::MaxSupportedVersion;

CosemSmtpSetupObject::CosemSmtpSetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t serverPort,
  const std::vector<std::uint8_t>& userName,
  const std::vector<std::uint8_t>& loginPassword,
  const std::vector<std::uint8_t>& serverAddress,
  const std::vector<std::uint8_t>& senderAddress,
  AttributeAccessMode mutableAccess)
  : CosemSmtpSetupObject(
      logicalName, serverPort, userName, loginPassword,
      serverAddress, senderAddress, mutableAccess, kVersion0)
{
}

CosemSmtpSetupObject::CosemSmtpSetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t serverPort,
  const std::vector<std::uint8_t>& userName,
  const std::vector<std::uint8_t>& loginPassword,
  const std::vector<std::uint8_t>& serverAddress,
  const std::vector<std::uint8_t>& senderAddress,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSmtpSetupClassId,
      NormalizeVersion(
        version, CosemSmtpSetupObject::MaxSupportedVersion),
      logicalName))
  , serverPort_(serverPort)
  , userName_(userName)
  , loginPassword_(loginPassword)
  , serverAddress_(serverAddress)
  , senderAddress_(senderAddress)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSmtpSetupServerPortAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupUserNameAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupLoginPasswordAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupServerAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSmtpSetupSenderAddressAttributeId, mutableAccess);
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
  output.clear();
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSmtpSetupServerPortAttributeId:
      AppendLongUnsigned(output, serverPort_);
      return CosemStatus::Ok;
    case kSmtpSetupUserNameAttributeId: {
      static const std::uint8_t kEmpty = 0u;
      AppendOctetString(
        output,
        userName_.empty() ? &kEmpty : &userName_[0],
        userName_.size());
      return CosemStatus::Ok;
    }
    case kSmtpSetupLoginPasswordAttributeId: {
      static const std::uint8_t kEmpty = 0u;
      AppendOctetString(
        output,
        loginPassword_.empty() ? &kEmpty : &loginPassword_[0],
        loginPassword_.size());
      return CosemStatus::Ok;
    }
    case kSmtpSetupServerAddressAttributeId: {
      static const std::uint8_t kEmpty = 0u;
      AppendOctetString(
        output,
        serverAddress_.empty() ? &kEmpty : &serverAddress_[0],
        serverAddress_.size());
      return CosemStatus::Ok;
    }
    case kSmtpSetupSenderAddressAttributeId: {
      static const std::uint8_t kEmpty = 0u;
      AppendOctetString(
        output,
        senderAddress_.empty() ? &kEmpty : &senderAddress_[0],
        senderAddress_.size());
      return CosemStatus::Ok;
    }
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSmtpSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSmtpSetupServerPortAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t value = 0u;
      if (!ReadLongUnsignedValue(input, offset, value) ||
          offset != input.size())
        return CosemStatus::InvalidArgument;
      serverPort_ = value;
      return CosemStatus::Ok;
    }
    case kSmtpSetupUserNameAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<std::uint8_t> decoded;
      if (!DecodeSmtpOctetString(input, decoded))
        return CosemStatus::InvalidArgument;
      userName_.swap(decoded);
      return CosemStatus::Ok;
    }
    case kSmtpSetupLoginPasswordAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<std::uint8_t> decoded;
      if (!DecodeSmtpOctetString(input, decoded))
        return CosemStatus::InvalidArgument;
      loginPassword_.swap(decoded);
      return CosemStatus::Ok;
    }
    case kSmtpSetupServerAddressAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<std::uint8_t> decoded;
      if (!DecodeSmtpOctetString(input, decoded))
        return CosemStatus::InvalidArgument;
      serverAddress_.swap(decoded);
      return CosemStatus::Ok;
    }
    case kSmtpSetupSenderAddressAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<std::uint8_t> decoded;
      if (!DecodeSmtpOctetString(input, decoded))
        return CosemStatus::InvalidArgument;
      senderAddress_.swap(decoded);
      return CosemStatus::Ok;
    }
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

std::uint16_t CosemSmtpSetupObject::ServerPort() const
{
  return serverPort_;
}

const std::vector<std::uint8_t>& CosemSmtpSetupObject::UserName() const
{
  return userName_;
}

const std::vector<std::uint8_t>& CosemSmtpSetupObject::LoginPassword() const
{
  return loginPassword_;
}

const std::vector<std::uint8_t>& CosemSmtpSetupObject::ServerAddress() const
{
  return serverAddress_;
}

const std::vector<std::uint8_t>& CosemSmtpSetupObject::SenderAddress() const
{
  return senderAddress_;
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
// IEC 62056-6-2 ED4 (2021) §5.6.8 / DLMS UA Blue Book Ed. 12.1 §5.6.8 define
// class_id=47, version=0 with NO specific methods (the "Specific methods
// m/o" column is empty). Earlier revisions of this implementation
// surfaced a phantom "reset" method (id 1) that the spec never
// defines; it is intentionally removed.
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
  (void)methodId;
  (void)input;
  output.clear();
  // GSM Diagnostic IC v0 defines no methods (see ED4 §5.6.8 /
  // Blue Book Ed. 12.1 §5.6.8); every method id is MethodNotFound.
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
constexpr std::uint8_t kIecTwistedPairSetupSecondaryAddressAttributeId = 2u;
constexpr std::uint8_t kIecTwistedPairSetupPrimaryAddressListAttributeId = 3u;
constexpr std::uint8_t kIecTwistedPairSetupTabiListAttributeId = 4u;
constexpr std::uint8_t kIecTwistedPairSetupFatalErrorAttributeId = 5u;
} // namespace

const std::uint8_t CosemIecTwistedPairSetupObject::MaxSupportedVersion;

CosemIecTwistedPairSetupObject::CosemIecTwistedPairSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& secondaryAddress,
  const CosemByteBuffer& primaryAddressList,
  const CosemByteBuffer& tabiList,
  const CosemByteBuffer& fatalError,
  AttributeAccessMode mutableAccess)
  : CosemIecTwistedPairSetupObject(
      logicalName,
      secondaryAddress,
      primaryAddressList,
      tabiList,
      fatalError,
      mutableAccess,
      kVersion0)
{
}

CosemIecTwistedPairSetupObject::CosemIecTwistedPairSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& secondaryAddress,
  const CosemByteBuffer& primaryAddressList,
  const CosemByteBuffer& tabiList,
  const CosemByteBuffer& fatalError,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIecTwistedPairSetupClassId,
      NormalizeVersion(
        version, CosemIecTwistedPairSetupObject::MaxSupportedVersion),
      logicalName))
  , secondaryAddress_(secondaryAddress)
  , primaryAddressList_(primaryAddressList)
  , tabiList_(tabiList)
  , fatalError_(fatalError)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIecTwistedPairSetupSecondaryAddressAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecTwistedPairSetupPrimaryAddressListAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIecTwistedPairSetupTabiListAttributeId, mutableAccess);
  // fatal_error is server-managed (latest protocol fatal error
  // observed by the IEC 62056-31 stack); it is read-only at the
  // wire surface regardless of the caller-selected mutableAccess.
  rights_.SetAttributeAccess(
    kIecTwistedPairSetupFatalErrorAttributeId,
    AttributeAccessMode::ReadOnly);
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
    case kIecTwistedPairSetupSecondaryAddressAttributeId:
      output = secondaryAddress_;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupPrimaryAddressListAttributeId:
      output = primaryAddressList_;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupTabiListAttributeId:
      output = tabiList_;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupFatalErrorAttributeId:
      output = fatalError_;
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
    case kIecTwistedPairSetupSecondaryAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      secondaryAddress_ = input;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupPrimaryAddressListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      primaryAddressList_ = input;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupTabiListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      tabiList_ = input;
      return CosemStatus::Ok;
    case kIecTwistedPairSetupFatalErrorAttributeId:
      // fatal_error is server-managed; reject writes regardless of
      // the caller-selected mutableAccess.
      return CosemStatus::AccessDenied;
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
  // IEC 62056-6-2 ED4 (2021) §4.7.3 and DLMS UA Blue Book Ed. 12.1
  // §4.7.3 define class_id 24, version 0 with no specific methods.
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemIecTwistedPairSetupObject::SecondaryAddress() const
{
  return secondaryAddress_;
}

const CosemByteBuffer&
CosemIecTwistedPairSetupObject::PrimaryAddressList() const
{
  return primaryAddressList_;
}

const CosemByteBuffer& CosemIecTwistedPairSetupObject::TabiList() const
{
  return tabiList_;
}

const CosemByteBuffer& CosemIecTwistedPairSetupObject::FatalError() const
{
  return fatalError_;
}

namespace {
constexpr std::uint16_t kMBusSlavePortSetupClassId = 25u;
constexpr std::uint8_t kMBusSlavePortSetupDefaultBaudAttributeId = 2u;
constexpr std::uint8_t kMBusSlavePortSetupAvailBaudAttributeId = 3u;
constexpr std::uint8_t kMBusSlavePortSetupAddrStateAttributeId = 4u;
constexpr std::uint8_t kMBusSlavePortSetupBusAddressAttributeId = 5u;
} // namespace

const std::uint8_t CosemMBusSlavePortSetupObject::MaxSupportedVersion;

CosemMBusSlavePortSetupObject::CosemMBusSlavePortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& defaultBaud,
  const CosemByteBuffer& availBaud,
  const CosemByteBuffer& addrState,
  const CosemByteBuffer& busAddress,
  AttributeAccessMode mutableAccess)
  : CosemMBusSlavePortSetupObject(
      logicalName, defaultBaud, availBaud, addrState,
      busAddress, mutableAccess, kVersion0)
{
}

CosemMBusSlavePortSetupObject::CosemMBusSlavePortSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& defaultBaud,
  const CosemByteBuffer& availBaud,
  const CosemByteBuffer& addrState,
  const CosemByteBuffer& busAddress,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kMBusSlavePortSetupClassId,
      NormalizeVersion(
        version, CosemMBusSlavePortSetupObject::MaxSupportedVersion),
      logicalName))
  , defaultBaud_(defaultBaud)
  , availBaud_(availBaud)
  , addrState_(addrState)
  , busAddress_(busAddress)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupDefaultBaudAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupAvailBaudAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupAddrStateAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusSlavePortSetupBusAddressAttributeId, mutableAccess);
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
    case kMBusSlavePortSetupAvailBaudAttributeId:
      output = availBaud_;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupAddrStateAttributeId:
      output = addrState_;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupBusAddressAttributeId:
      output = busAddress_;
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
    case kMBusSlavePortSetupAvailBaudAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      availBaud_ = input;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupAddrStateAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      addrState_ = input;
      return CosemStatus::Ok;
    case kMBusSlavePortSetupBusAddressAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      busAddress_ = input;
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
  (void)methodId;
  (void)input;
  // IEC 62056-6-2 ED4 (2021) §4.8.2 and DLMS UA Blue Book Ed. 12.1
  // §4.8.1 leave the "Specific methods | m/o" column empty for
  // M-Bus slave port setup IC 25; the built-in object therefore
  // exposes no methods. Earlier revisions surfaced a phantom
  // `reset` method (id 1) that the spec never defines; it has
  // been removed.
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemMBusSlavePortSetupObject::DefaultBaud() const
{
  return defaultBaud_;
}

const CosemByteBuffer&
CosemMBusSlavePortSetupObject::AvailBaud() const
{
  return availBaud_;
}

const CosemByteBuffer& CosemMBusSlavePortSetupObject::AddrState() const
{
  return addrState_;
}

const CosemByteBuffer&
CosemMBusSlavePortSetupObject::BusAddress() const
{
  return busAddress_;
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

bool DecodeUtilityTablesBuffer(
  const CosemByteBuffer& input,
  std::vector<std::uint8_t>& value)
{
  std::size_t offset = 0u;
  std::size_t length = 0u;
  if (!ReadExpectedTag(input, offset, kDataOctetStringTag) ||
      !ReadAxdrLength(input, offset, length) ||
      input.size() - offset < length) {
    return false;
  }
  value.assign(input.begin() + offset, input.begin() + offset + length);
  offset += length;
  if (offset != input.size()) {
    return false;
  }
  return true;
}
} // namespace

const std::uint8_t CosemUtilityTablesObject::MaxSupportedVersion;

CosemUtilityTablesObject::CosemUtilityTablesObject(
  const CosemLogicalName& logicalName,
  std::uint16_t tableId,
  std::uint32_t length,
  const std::vector<std::uint8_t>& buffer,
  AttributeAccessMode mutableAccess)
  : CosemUtilityTablesObject(
      logicalName, tableId, length, buffer, mutableAccess, kVersion0)
{
}

CosemUtilityTablesObject::CosemUtilityTablesObject(
  const CosemLogicalName& logicalName,
  std::uint16_t tableId,
  std::uint32_t length,
  const std::vector<std::uint8_t>& buffer,
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
  output.clear();
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kUtilityTablesTableIdAttributeId:
      AppendLongUnsigned(output, tableId_);
      return CosemStatus::Ok;
    case kUtilityTablesLengthAttributeId:
      AppendDoubleLongUnsigned(output, length_);
      return CosemStatus::Ok;
    case kUtilityTablesBufferAttributeId:
      AppendOctetString(output, buffer_.data(), buffer_.size());
      return CosemStatus::Ok;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemUtilityTablesObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kUtilityTablesTableIdAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t value = 0u;
      if (!ReadLongUnsignedValue(input, offset, value) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      tableId_ = value;
      return CosemStatus::Ok;
    }
    case kUtilityTablesLengthAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint32_t value = 0u;
      if (!ReadDoubleLongUnsignedValue(input, offset, value) ||
          offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      length_ = value;
      return CosemStatus::Ok;
    }
    case kUtilityTablesBufferAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::vector<std::uint8_t> decoded;
      if (!DecodeUtilityTablesBuffer(input, decoded)) {
        return CosemStatus::InvalidArgument;
      }
      buffer_ = std::move(decoded);
      return CosemStatus::Ok;
    }
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

std::uint16_t CosemUtilityTablesObject::TableId() const
{
  return tableId_;
}

std::uint32_t CosemUtilityTablesObject::Length() const
{
  return length_;
}

const std::vector<std::uint8_t>& CosemUtilityTablesObject::Buffer() const
{
  return buffer_;
}

namespace {
constexpr std::uint16_t kSensorManagerClassId = 67u;
constexpr std::uint8_t kSensorManagerSerialNumberAttributeId = 2u;
constexpr std::uint8_t
  kSensorManagerMetrologicalIdentificationAttributeId = 3u;
constexpr std::uint8_t kSensorManagerOutputTypeAttributeId = 4u;
constexpr std::uint8_t
  kSensorManagerAdjustmentMethodAttributeId = 5u;
constexpr std::uint8_t kSensorManagerSealingMethodAttributeId = 6u;
constexpr std::uint8_t kSensorManagerRawValueAttributeId = 7u;
constexpr std::uint8_t kSensorManagerScalerUnitAttributeId = 8u;
constexpr std::uint8_t kSensorManagerStatusAttributeId = 9u;
constexpr std::uint8_t kSensorManagerCaptureTimeAttributeId = 10u;
constexpr std::uint8_t
  kSensorManagerRawValueThresholdsAttributeId = 11u;
constexpr std::uint8_t
  kSensorManagerRawValueActionsAttributeId = 12u;
constexpr std::uint8_t
  kSensorManagerProcessedValueAttributeId = 13u;
constexpr std::uint8_t
  kSensorManagerProcessedValueThresholdsAttributeId = 14u;
constexpr std::uint8_t
  kSensorManagerProcessedValueActionsAttributeId = 15u;
constexpr std::uint8_t kSensorManagerResetMethodId = 1u;
} // namespace

const std::uint8_t CosemSensorManagerObject::MaxSupportedVersion;

CosemSensorManagerObject::CosemSensorManagerObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& serialNumber,
  const CosemByteBuffer& metrologicalIdentification,
  const CosemByteBuffer& outputType,
  const CosemByteBuffer& adjustmentMethod,
  const CosemByteBuffer& sealingMethod,
  const CosemByteBuffer& rawValue,
  const CosemByteBuffer& scalerUnit,
  const CosemByteBuffer& status,
  const CosemByteBuffer& captureTime,
  const CosemByteBuffer& rawValueThresholds,
  const CosemByteBuffer& rawValueActions,
  const CosemByteBuffer& processedValue,
  const CosemByteBuffer& processedValueThresholds,
  const CosemByteBuffer& processedValueActions,
  AttributeAccessMode mutableAccess)
  : CosemSensorManagerObject(
      logicalName, serialNumber, metrologicalIdentification,
      outputType, adjustmentMethod, sealingMethod, rawValue,
      scalerUnit, status, captureTime, rawValueThresholds,
      rawValueActions, processedValue, processedValueThresholds,
      processedValueActions, mutableAccess, kVersion0)
{
}

CosemSensorManagerObject::CosemSensorManagerObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& serialNumber,
  const CosemByteBuffer& metrologicalIdentification,
  const CosemByteBuffer& outputType,
  const CosemByteBuffer& adjustmentMethod,
  const CosemByteBuffer& sealingMethod,
  const CosemByteBuffer& rawValue,
  const CosemByteBuffer& scalerUnit,
  const CosemByteBuffer& status,
  const CosemByteBuffer& captureTime,
  const CosemByteBuffer& rawValueThresholds,
  const CosemByteBuffer& rawValueActions,
  const CosemByteBuffer& processedValue,
  const CosemByteBuffer& processedValueThresholds,
  const CosemByteBuffer& processedValueActions,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSensorManagerClassId,
      NormalizeVersion(
        version, CosemSensorManagerObject::MaxSupportedVersion),
      logicalName))
  , serialNumber_(serialNumber)
  , metrologicalIdentification_(metrologicalIdentification)
  , outputType_(outputType)
  , adjustmentMethod_(adjustmentMethod)
  , sealingMethod_(sealingMethod)
  , rawValue_(rawValue)
  , scalerUnit_(scalerUnit)
  , status_(status)
  , captureTime_(captureTime)
  , rawValueThresholds_(rawValueThresholds)
  , rawValueActions_(rawValueActions)
  , processedValue_(processedValue)
  , processedValueThresholds_(processedValueThresholds)
  , processedValueActions_(processedValueActions)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSensorManagerSerialNumberAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerMetrologicalIdentificationAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerOutputTypeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerAdjustmentMethodAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerSealingMethodAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerRawValueAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerScalerUnitAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerCaptureTimeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerRawValueThresholdsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerRawValueActionsAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerProcessedValueAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerProcessedValueThresholdsAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kSensorManagerProcessedValueActionsAttributeId, mutableAccess);
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
    case kSensorManagerSerialNumberAttributeId:
      output = serialNumber_;
      return CosemStatus::Ok;
    case kSensorManagerMetrologicalIdentificationAttributeId:
      output = metrologicalIdentification_;
      return CosemStatus::Ok;
    case kSensorManagerOutputTypeAttributeId:
      output = outputType_;
      return CosemStatus::Ok;
    case kSensorManagerAdjustmentMethodAttributeId:
      output = adjustmentMethod_;
      return CosemStatus::Ok;
    case kSensorManagerSealingMethodAttributeId:
      output = sealingMethod_;
      return CosemStatus::Ok;
    case kSensorManagerRawValueAttributeId:
      output = rawValue_;
      return CosemStatus::Ok;
    case kSensorManagerScalerUnitAttributeId:
      output = scalerUnit_;
      return CosemStatus::Ok;
    case kSensorManagerStatusAttributeId:
      output = status_;
      return CosemStatus::Ok;
    case kSensorManagerCaptureTimeAttributeId:
      output = captureTime_;
      return CosemStatus::Ok;
    case kSensorManagerRawValueThresholdsAttributeId:
      output = rawValueThresholds_;
      return CosemStatus::Ok;
    case kSensorManagerRawValueActionsAttributeId:
      output = rawValueActions_;
      return CosemStatus::Ok;
    case kSensorManagerProcessedValueAttributeId:
      output = processedValue_;
      return CosemStatus::Ok;
    case kSensorManagerProcessedValueThresholdsAttributeId:
      output = processedValueThresholds_;
      return CosemStatus::Ok;
    case kSensorManagerProcessedValueActionsAttributeId:
      output = processedValueActions_;
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
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kSensorManagerSerialNumberAttributeId:
      target = &serialNumber_;
      break;
    case kSensorManagerMetrologicalIdentificationAttributeId:
      target = &metrologicalIdentification_;
      break;
    case kSensorManagerOutputTypeAttributeId:
      target = &outputType_;
      break;
    case kSensorManagerAdjustmentMethodAttributeId:
      target = &adjustmentMethod_;
      break;
    case kSensorManagerSealingMethodAttributeId:
      target = &sealingMethod_;
      break;
    case kSensorManagerRawValueAttributeId:
      target = &rawValue_;
      break;
    case kSensorManagerScalerUnitAttributeId:
      target = &scalerUnit_;
      break;
    case kSensorManagerStatusAttributeId:
      target = &status_;
      break;
    case kSensorManagerCaptureTimeAttributeId:
      target = &captureTime_;
      break;
    case kSensorManagerRawValueThresholdsAttributeId:
      target = &rawValueThresholds_;
      break;
    case kSensorManagerRawValueActionsAttributeId:
      target = &rawValueActions_;
      break;
    case kSensorManagerProcessedValueAttributeId:
      target = &processedValue_;
      break;
    case kSensorManagerProcessedValueThresholdsAttributeId:
      target = &processedValueThresholds_;
      break;
    case kSensorManagerProcessedValueActionsAttributeId:
      target = &processedValueActions_;
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

CosemStatus CosemSensorManagerObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kSensorManagerResetMethodId) {
    // IEC 62056-6-2 ED4 (2021) defines the optional `reset(data)`
    // method for the Sensor manager IC. The built-in object does
    // not own the sensor lifecycle, so surface the request as
    // UnsupportedFeature instead of silently reporting
    // MethodNotFound.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemSensorManagerObject::SerialNumber() const
{
  return serialNumber_;
}

const CosemByteBuffer&
CosemSensorManagerObject::MetrologicalIdentification() const
{
  return metrologicalIdentification_;
}

const CosemByteBuffer& CosemSensorManagerObject::OutputType() const
{
  return outputType_;
}

const CosemByteBuffer&
CosemSensorManagerObject::AdjustmentMethod() const
{
  return adjustmentMethod_;
}

const CosemByteBuffer&
CosemSensorManagerObject::SealingMethod() const
{
  return sealingMethod_;
}

const CosemByteBuffer& CosemSensorManagerObject::RawValue() const
{
  return rawValue_;
}

const CosemByteBuffer& CosemSensorManagerObject::ScalerUnit() const
{
  return scalerUnit_;
}

const CosemByteBuffer& CosemSensorManagerObject::Status() const
{
  return status_;
}

const CosemByteBuffer& CosemSensorManagerObject::CaptureTime() const
{
  return captureTime_;
}

const CosemByteBuffer&
CosemSensorManagerObject::RawValueThresholds() const
{
  return rawValueThresholds_;
}

const CosemByteBuffer&
CosemSensorManagerObject::RawValueActions() const
{
  return rawValueActions_;
}

const CosemByteBuffer&
CosemSensorManagerObject::ProcessedValue() const
{
  return processedValue_;
}

const CosemByteBuffer&
CosemSensorManagerObject::ProcessedValueThresholds() const
{
  return processedValueThresholds_;
}

const CosemByteBuffer&
CosemSensorManagerObject::ProcessedValueActions() const
{
  return processedValueActions_;
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
constexpr std::uint8_t
  kParameterMonitorParameterListNameAttributeId = 5u;
constexpr std::uint8_t
  kParameterMonitorHashAlgorithmIdAttributeId = 6u;
constexpr std::uint8_t
  kParameterMonitorParameterValueDigestAttributeId = 7u;
constexpr std::uint8_t
  kParameterMonitorParameterValuesAttributeId = 8u;
constexpr std::uint8_t kParameterMonitorInsertMethodId = 1u;
constexpr std::uint8_t kParameterMonitorDeleteMethodId = 2u;
} // namespace

const std::uint8_t CosemParameterMonitorObject::MaxSupportedVersion;

bool CosemParameterMonitorObject::IsValidHashAlgorithmId(std::uint8_t raw)
{
  return raw <= static_cast<std::uint8_t>(
                  CosemParameterMonitorObject::HashAlgorithmId::Sha256Last4);
}

CosemParameterMonitorObject::CosemParameterMonitorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& changedParameter,
  const dlms::cosem::types::DateTime& captureTime,
  const std::vector<dlms::cosem::types::MonitoredValue>& parameterList,
  const CosemByteBuffer& parameterListName,
  HashAlgorithmId hashAlgorithmId,
  const CosemByteBuffer& parameterValueDigest,
  const CosemByteBuffer& parameterValues,
  AttributeAccessMode mutableAccess)
  : CosemParameterMonitorObject(
      logicalName, changedParameter, captureTime, parameterList,
      parameterListName, hashAlgorithmId, parameterValueDigest,
      parameterValues, mutableAccess,
      CosemParameterMonitorObject::MaxSupportedVersion)
{
}

CosemParameterMonitorObject::CosemParameterMonitorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& changedParameter,
  const dlms::cosem::types::DateTime& captureTime,
  const std::vector<dlms::cosem::types::MonitoredValue>& parameterList,
  const CosemByteBuffer& parameterListName,
  HashAlgorithmId hashAlgorithmId,
  const CosemByteBuffer& parameterValueDigest,
  const CosemByteBuffer& parameterValues,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kParameterMonitorClassId,
      NormalizeVersion(
        version, CosemParameterMonitorObject::MaxSupportedVersion),
      logicalName))
  , changedParameter_(changedParameter)
  , captureTime_(captureTime)
  , parameterList_(parameterList)
  , parameterListName_(parameterListName)
  , hashAlgorithmId_(
      IsValidHashAlgorithmId(static_cast<std::uint8_t>(hashAlgorithmId))
        ? hashAlgorithmId
        : HashAlgorithmId::Sha256)
  , parameterValueDigest_(parameterValueDigest)
  , parameterValues_(parameterValues)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kParameterMonitorChangedParameterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kParameterMonitorCaptureTimeAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kParameterMonitorParametersAttributeId, mutableAccess);
  // Attributes 5..8 are only defined for class_id = 65,
  // version = 1 per IEC 62056-6-2 ED4 (2021) §4.5.10. Legacy
  // version 0 (Blue Book 12.1 5.4.1) stops at attribute 4.
  if (descriptor_.key.version >= 1u) {
    rights_.SetAttributeAccess(
      kParameterMonitorParameterListNameAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kParameterMonitorHashAlgorithmIdAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kParameterMonitorParameterValueDigestAttributeId,
      mutableAccess);
    rights_.SetAttributeAccess(
      kParameterMonitorParameterValuesAttributeId, mutableAccess);
  } else {
    parameterListName_.clear();
    parameterValueDigest_.clear();
    parameterValues_.clear();
  }
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
      output.clear();
      AppendDateTimeOctetString(output, captureTime_);
      return CosemStatus::Ok;
    case kParameterMonitorParametersAttributeId:
      output.clear();
      AppendArrayHeader(output, parameterList_.size());
      for (const auto& v : parameterList_) {
        AppendMonitoredValue(output, v);
      }
      return CosemStatus::Ok;
    case kParameterMonitorParameterListNameAttributeId:
      if (descriptor_.key.version < 1u) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
      output = parameterListName_;
      return CosemStatus::Ok;
    case kParameterMonitorHashAlgorithmIdAttributeId:
      if (descriptor_.key.version < 1u) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
      output.clear();
      AppendEnum(output, static_cast<std::uint8_t>(hashAlgorithmId_));
      return CosemStatus::Ok;
    case kParameterMonitorParameterValueDigestAttributeId:
      if (descriptor_.key.version < 1u) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
      output = parameterValueDigest_;
      return CosemStatus::Ok;
    case kParameterMonitorParameterValuesAttributeId:
      if (descriptor_.key.version < 1u) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
      output = parameterValues_;
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
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    case kParameterMonitorChangedParameterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      changedParameter_ = input;
      return CosemStatus::Ok;
    case kParameterMonitorCaptureTimeAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      dlms::cosem::types::DateTime decoded;
      if (!DecodeDateTimeOctetString(input, decoded))
        return CosemStatus::InvalidArgument;
      captureTime_ = decoded;
      return CosemStatus::Ok;
    }
    case kParameterMonitorParametersAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      if (!ReadExpectedTag(input, offset, kArrayTag))
        return CosemStatus::InvalidArgument;
      std::size_t count = 0u;
      if (!ReadAxdrLength(input, offset, count))
        return CosemStatus::InvalidArgument;
      std::vector<dlms::cosem::types::MonitoredValue> decoded;
      decoded.reserve(count);
      for (std::size_t i = 0u; i < count; ++i) {
        dlms::cosem::types::MonitoredValue mv;
        if (!DecodeMonitoredValue(input, offset, mv))
          return CosemStatus::InvalidArgument;
        decoded.push_back(mv);
      }
      if (offset != input.size())
        return CosemStatus::InvalidArgument;
      parameterList_ = std::move(decoded);
      return CosemStatus::Ok;
    }
    case kParameterMonitorParameterListNameAttributeId:
      if (descriptor_.key.version < 1u)
        return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      parameterListName_ = input;
      return CosemStatus::Ok;
    case kParameterMonitorHashAlgorithmIdAttributeId: {
      if (descriptor_.key.version < 1u)
        return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::uint8_t raw = 0u;
      std::size_t offset = 0u;
      if (!ReadEnumValue(input, offset, raw) || offset != input.size())
        return CosemStatus::InvalidArgument;
      if (!IsValidHashAlgorithmId(raw))
        return CosemStatus::InvalidArgument;
      hashAlgorithmId_ = static_cast<HashAlgorithmId>(raw);
      return CosemStatus::Ok;
    }
    case kParameterMonitorParameterValueDigestAttributeId:
      if (descriptor_.key.version < 1u)
        return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      parameterValueDigest_ = input;
      return CosemStatus::Ok;
    case kParameterMonitorParameterValuesAttributeId:
      if (descriptor_.key.version < 1u)
        return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      parameterValues_ = input;
      return CosemStatus::Ok;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemParameterMonitorObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  output.clear();
  if (methodId == kParameterMonitorInsertMethodId) {
    // add_parameter (data): data ::= parameter_list_element.
    // Per IEC 62056-6-2 ED4 §4.5.10.2.1 we append the element to
    // parameter_list. Duplicate entries are silently accepted —
    // the spec does not require de-duplication and the backend
    // is free to enforce its own policy.
    if (descriptor_.key.version < 1u)
      return CosemStatus::UnsupportedFeature;
    std::size_t offset = 0u;
    dlms::cosem::types::MonitoredValue mv;
    if (!DecodeMonitoredValue(input, offset, mv) ||
        offset != input.size()) {
      return CosemStatus::InvalidArgument;
    }
    parameterList_.push_back(mv);
    return CosemStatus::Ok;
  }
  if (methodId == kParameterMonitorDeleteMethodId) {
    // delete_parameter (data): data ::= parameter_list_element.
    // Per §4.5.10.2.2 we drop the first matching entry (matching
    // class_id + logical_name + attribute_index). Missing target
    // returns ObjectError — DLMS lacks a precise "not found"
    // status, and ObjectError best signals a semantic failure on
    // an otherwise well-formed request.
    if (descriptor_.key.version < 1u)
      return CosemStatus::UnsupportedFeature;
    std::size_t offset = 0u;
    dlms::cosem::types::MonitoredValue mv;
    if (!DecodeMonitoredValue(input, offset, mv) ||
        offset != input.size()) {
      return CosemStatus::InvalidArgument;
    }
    for (auto it = parameterList_.begin();
         it != parameterList_.end(); ++it) {
      if (it->ClassId() == mv.ClassId() &&
          it->LogicalName() == mv.LogicalName() &&
          it->AttributeIndex() == mv.AttributeIndex()) {
        parameterList_.erase(it);
        return CosemStatus::Ok;
      }
    }
    return CosemStatus::ObjectError;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemParameterMonitorObject::ChangedParameter() const
{
  return changedParameter_;
}

const dlms::cosem::types::DateTime&
CosemParameterMonitorObject::CaptureTime() const
{
  return captureTime_;
}

const std::vector<dlms::cosem::types::MonitoredValue>&
CosemParameterMonitorObject::ParameterList() const
{
  return parameterList_;
}

const CosemByteBuffer&
CosemParameterMonitorObject::ParameterListName() const
{
  return parameterListName_;
}

CosemParameterMonitorObject::HashAlgorithmId
CosemParameterMonitorObject::GetHashAlgorithmId() const
{
  return hashAlgorithmId_;
}

const CosemByteBuffer&
CosemParameterMonitorObject::ParameterValueDigest() const
{
  return parameterValueDigest_;
}

const CosemByteBuffer&
CosemParameterMonitorObject::ParameterValues() const
{
  return parameterValues_;
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

bool CosemCompactDataObject::IsValidCaptureMethod(std::uint8_t raw)
{
  return raw <= 2u;
}

CosemCompactDataObject::CosemCompactDataObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& buffer,
  const CosemByteBuffer& captureObjects,
  std::uint8_t templateId,
  const CosemByteBuffer& templateDescription,
  CaptureMethod captureMethod,
  AttributeAccessMode mutableAccess)
  : CosemCompactDataObject(
      logicalName, buffer, captureObjects, templateId,
      templateDescription, captureMethod, mutableAccess,
      static_cast<std::uint8_t>(1u))
{
}

CosemCompactDataObject::CosemCompactDataObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& buffer,
  const CosemByteBuffer& captureObjects,
  std::uint8_t templateId,
  const CosemByteBuffer& templateDescription,
  CaptureMethod captureMethod,
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
  , captureMethod_(
      IsValidCaptureMethod(static_cast<std::uint8_t>(captureMethod))
        ? captureMethod
        : CaptureMethod::Inactive)
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
      output.clear();
      AppendUnsigned(output, templateId_);
      return CosemStatus::Ok;
    case kCompactDataTemplateDescriptionAttributeId:
      output = templateDescription_;
      return CosemStatus::Ok;
    case kCompactDataCaptureMethodAttributeId:
      output.clear();
      AppendEnum(output, static_cast<std::uint8_t>(captureMethod_));
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
    case kCompactDataTemplateIdAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t raw = 0u;
      if (!ReadUnsignedValue(input, offset, raw)
          || offset != input.size()) {
        return CosemStatus::InvalidArgument;
      }
      templateId_ = raw;
      return CosemStatus::Ok;
    }
    case kCompactDataTemplateDescriptionAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      templateDescription_ = input;
      return CosemStatus::Ok;
    case kCompactDataCaptureMethodAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint8_t raw = 0u;
      if (!ReadEnumValue(input, offset, raw)
          || offset != input.size()
          || !IsValidCaptureMethod(raw)) {
        return CosemStatus::InvalidArgument;
      }
      captureMethod_ = static_cast<CaptureMethod>(raw);
      return CosemStatus::Ok;
    }
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
  if (methodId == kCompactDataResetMethodId) {
    // §5.2.2.3.1: reset clears the compact_buffer. capture_objects /
    // template_id / template_description are configuration and remain.
    buffer_.clear();
    return CosemStatus::Ok;
  }
  if (methodId == kCompactDataCaptureMethodId) {
    // capture() requires evaluating capture_objects against live
    // attribute values and encoding them per template_description.
    // This needs the typed capture_object_definition machinery shared
    // with IC 7 — surfaced as UnsupportedFeature until that lands.
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

std::uint8_t CosemCompactDataObject::TemplateId() const
{
  return templateId_;
}

const CosemByteBuffer&
CosemCompactDataObject::TemplateDescription() const
{
  return templateDescription_;
}

CosemCompactDataObject::CaptureMethod
CosemCompactDataObject::GetCaptureMethod() const
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
  AttributeAccessMode mutableAccess)
  : CosemIecLocalPortSetupObject(
      logicalName, defaultMode, defaultBaud, proposedBaud,
      responseTime, deviceAddress, password1, password2, password5,
      mutableAccess, kVersion1)
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

namespace {
constexpr std::uint16_t kAssociationSnClassId = 12u;
constexpr std::uint8_t kAssociationSnObjectListAttributeId = 2u;
constexpr std::uint8_t kAssociationSnAccessRightsListAttributeId = 3u;
constexpr std::uint8_t
  kAssociationSnSecuritySetupReferenceAttributeId = 4u;
constexpr std::uint8_t kAssociationSnUserListAttributeId = 5u;
constexpr std::uint8_t kAssociationSnCurrentUserAttributeId = 6u;
constexpr std::uint8_t
  kAssociationSnReadByLogicalNameMethodId = 3u;
constexpr std::uint8_t kAssociationSnChangeSecretMethodId = 5u;
constexpr std::uint8_t
  kAssociationSnReplyToHlsAuthenticationMethodId = 8u;
constexpr std::uint8_t kAssociationSnAddUserMethodId = 9u;
constexpr std::uint8_t kAssociationSnRemoveUserMethodId = 10u;
constexpr std::uint8_t kVersion2 = 2u;
constexpr std::uint8_t kVersion3 = 3u;
constexpr std::uint8_t kVersion4 = 4u;
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
      mutableAccess, kVersion4)
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
  if (descriptor_.key.version >= kVersion2) {
    rights_.SetAttributeAccess(
      kAssociationSnSecuritySetupReferenceAttributeId, mutableAccess);
  }
  if (descriptor_.key.version >= kVersion3) {
    rights_.SetAttributeAccess(
      kAssociationSnUserListAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kAssociationSnCurrentUserAttributeId, mutableAccess);
  } else {
    userList_.clear();
    currentUser_.clear();
  }
  if (descriptor_.key.version < kVersion2) {
    securitySetupReference_.clear();
  }
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
      if (descriptor_.key.version < kVersion2) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
      output = securitySetupReference_;
      return CosemStatus::Ok;
    case kAssociationSnUserListAttributeId:
      if (descriptor_.key.version < kVersion3) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
      output = userList_;
      return CosemStatus::Ok;
    case kAssociationSnCurrentUserAttributeId:
      if (descriptor_.key.version < kVersion3) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
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
      if (descriptor_.key.version < kVersion2)
        return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      securitySetupReference_ = input;
      return CosemStatus::Ok;
    case kAssociationSnUserListAttributeId:
      if (descriptor_.key.version < kVersion3)
        return CosemStatus::AttributeNotFound;
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      userList_ = input;
      return CosemStatus::Ok;
    case kAssociationSnCurrentUserAttributeId:
      if (descriptor_.key.version < kVersion3)
        return CosemStatus::AttributeNotFound;
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
  if (methodId == kAssociationSnReadByLogicalNameMethodId ||
      methodId == kAssociationSnChangeSecretMethodId ||
      methodId == kAssociationSnReplyToHlsAuthenticationMethodId) {
    // IEC 62056-6-2 ED4 4.4.3 / Blue Book 5.4 define methods
    // read_by_logicalname (3), change_secret (5) and
    // reply_to_HLS_authentication (8) on every Association SN
    // version. The built-in object does not execute SN-mode
    // authentication or secret rotation; backend handles them
    // out-of-band and republishes the stored buffers.
    return CosemStatus::UnsupportedFeature;
  }
  if (descriptor_.key.version >= kVersion3 &&
      (methodId == kAssociationSnAddUserMethodId ||
       methodId == kAssociationSnRemoveUserMethodId)) {
    // add_user (9) and remove_user (10) appear in v3 and later.
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
  // Attributes 13 (configuration) and 14 (encryption_key_status) are
  // only defined for class_id = 72, version = 1 per IEC 62056-6-2:2021
  // 4.8.3. Version 0 (Blue Book 12.1 5.7.1) stops at attribute 12.
  if (descriptor_.key.version >= 1u) {
    rights_.SetAttributeAccess(
      kMBusClientConfigurationAttributeId, mutableAccess);
    rights_.SetAttributeAccess(
      kMBusClientEncryptionKeyStatusAttributeId, mutableAccess);
  }
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
      if (descriptor_.key.version < 1u) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
      output = configuration_;
      return CosemStatus::Ok;
    case kMBusClientEncryptionKeyStatusAttributeId:
      if (descriptor_.key.version < 1u) {
        output.clear();
        return CosemStatus::AttributeNotFound;
      }
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
      if (descriptor_.key.version < 1u) {
        return CosemStatus::AttributeNotFound;
      }
      target = &configuration_;
      break;
    case kMBusClientEncryptionKeyStatusAttributeId:
      if (descriptor_.key.version < 1u) {
        return CosemStatus::AttributeNotFound;
      }
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
constexpr std::uint16_t kWirelessModeQChannelClassId = 73u;
} // namespace

const std::uint8_t CosemWirelessModeQChannelObject::MaxSupportedVersion;

CosemWirelessModeQChannelObject::CosemWirelessModeQChannelObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& addrState,
  const CosemByteBuffer& deviceAddress,
  const CosemByteBuffer& addressMask,
  AttributeAccessMode mutableAccess)
  : CosemWirelessModeQChannelObject(
      logicalName,
      addrState,
      deviceAddress,
      addressMask,
      mutableAccess,
      CosemWirelessModeQChannelObject::MaxSupportedVersion)
{
}

CosemWirelessModeQChannelObject::CosemWirelessModeQChannelObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& addrState,
  const CosemByteBuffer& deviceAddress,
  const CosemByteBuffer& addressMask,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kWirelessModeQChannelClassId,
      NormalizeVersion(
        version, CosemWirelessModeQChannelObject::MaxSupportedVersion),
      logicalName))
  , addrState_(addrState)
  , deviceAddress_(deviceAddress)
  , addressMask_(addressMask)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 4u; ++id)
    rights_.SetAttributeAccess(id, mutableAccess);
}

CosemObjectDescriptor CosemWirelessModeQChannelObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemWirelessModeQChannelObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemWirelessModeQChannelObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case 2u: output = addrState_; return CosemStatus::Ok;
    case 3u: output = deviceAddress_; return CosemStatus::Ok;
    case 4u: output = addressMask_; return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemWirelessModeQChannelObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId)
    return CosemStatus::AccessDenied;
  if (attributeId < 2u || attributeId > 4u)
    return CosemStatus::AttributeNotFound;
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  switch (attributeId) {
    case 2u: addrState_ = input; break;
    case 3u: deviceAddress_ = input; break;
    case 4u: addressMask_ = input; break;
  }
  return CosemStatus::Ok;
}

CosemStatus CosemWirelessModeQChannelObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IEC 62056-6-2 ED4 (2021) §4.8.4 and DLMS UA Blue Book Ed. 12.1
  // §4.8.3 define no specific methods for IC 73 v1.
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer& CosemWirelessModeQChannelObject::AddrState() const
{
  return addrState_;
}

const CosemByteBuffer& CosemWirelessModeQChannelObject::DeviceAddress() const
{
  return deviceAddress_;
}

const CosemByteBuffer& CosemWirelessModeQChannelObject::AddressMask() const
{
  return addressMask_;
}

namespace {
constexpr std::uint16_t kMBusMasterPortSetupClassId = 74u;
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
  kMBusDiagnosticReceivedSignalStrengthAttributeId = 2u;
constexpr std::uint8_t kMBusDiagnosticChannelIdAttributeId = 3u;
constexpr std::uint8_t kMBusDiagnosticLinkStatusAttributeId = 4u;
constexpr std::uint8_t
  kMBusDiagnosticBroadcastFramesCounterAttributeId = 5u;
constexpr std::uint8_t
  kMBusDiagnosticTransmissionsCounterAttributeId = 6u;
constexpr std::uint8_t
  kMBusDiagnosticFcsOkFramesCounterAttributeId = 7u;
constexpr std::uint8_t
  kMBusDiagnosticFcsNokFramesCounterAttributeId = 8u;
constexpr std::uint8_t kMBusDiagnosticCaptureTimeAttributeId = 9u;
constexpr std::uint8_t kMBusDiagnosticResetMethodId = 1u;
} // namespace

const std::uint8_t CosemMBusDiagnosticObject::MaxSupportedVersion;

CosemMBusDiagnosticObject::CosemMBusDiagnosticObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& receivedSignalStrength,
  const CosemByteBuffer& channelId,
  const CosemByteBuffer& linkStatus,
  const CosemByteBuffer& broadcastFramesCounter,
  const CosemByteBuffer& transmissionsCounter,
  const CosemByteBuffer& fcsOkFramesCounter,
  const CosemByteBuffer& fcsNokFramesCounter,
  const CosemByteBuffer& captureTime,
  AttributeAccessMode mutableAccess)
  : CosemMBusDiagnosticObject(
      logicalName, receivedSignalStrength, channelId, linkStatus,
      broadcastFramesCounter, transmissionsCounter,
      fcsOkFramesCounter, fcsNokFramesCounter, captureTime,
      mutableAccess,
      CosemMBusDiagnosticObject::MaxSupportedVersion)
{
}

CosemMBusDiagnosticObject::CosemMBusDiagnosticObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& receivedSignalStrength,
  const CosemByteBuffer& channelId,
  const CosemByteBuffer& linkStatus,
  const CosemByteBuffer& broadcastFramesCounter,
  const CosemByteBuffer& transmissionsCounter,
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
  , receivedSignalStrength_(receivedSignalStrength)
  , channelId_(channelId)
  , linkStatus_(linkStatus)
  , broadcastFramesCounter_(broadcastFramesCounter)
  , transmissionsCounter_(transmissionsCounter)
  , fcsOkFramesCounter_(fcsOkFramesCounter)
  , fcsNokFramesCounter_(fcsNokFramesCounter)
  , captureTime_(captureTime)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kMBusDiagnosticReceivedSignalStrengthAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticChannelIdAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticLinkStatusAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticBroadcastFramesCounterAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kMBusDiagnosticTransmissionsCounterAttributeId, mutableAccess);
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
    case kMBusDiagnosticReceivedSignalStrengthAttributeId:
      output = receivedSignalStrength_;
      return CosemStatus::Ok;
    case kMBusDiagnosticChannelIdAttributeId:
      output = channelId_;
      return CosemStatus::Ok;
    case kMBusDiagnosticLinkStatusAttributeId:
      output = linkStatus_;
      return CosemStatus::Ok;
    case kMBusDiagnosticBroadcastFramesCounterAttributeId:
      output = broadcastFramesCounter_;
      return CosemStatus::Ok;
    case kMBusDiagnosticTransmissionsCounterAttributeId:
      output = transmissionsCounter_;
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
    case kMBusDiagnosticReceivedSignalStrengthAttributeId:
      target = &receivedSignalStrength_;
      break;
    case kMBusDiagnosticChannelIdAttributeId:
      target = &channelId_;
      break;
    case kMBusDiagnosticLinkStatusAttributeId:
      target = &linkStatus_;
      break;
    case kMBusDiagnosticBroadcastFramesCounterAttributeId:
      target = &broadcastFramesCounter_;
      break;
    case kMBusDiagnosticTransmissionsCounterAttributeId:
      target = &transmissionsCounter_;
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
  (void)input;
  output.clear();
  if (methodId == kMBusDiagnosticResetMethodId) {
    // Optional `reset` clears the dynamic counters and the
    // capture_time; the built-in object does not own those
    // counter sources, so the backend must republish refreshed
    // values via the attribute setters.
    return CosemStatus::UnsupportedFeature;
  }
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::ReceivedSignalStrength() const
{
  return receivedSignalStrength_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::ChannelId() const
{
  return channelId_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::LinkStatus() const
{
  return linkStatus_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::BroadcastFramesCounter() const
{
  return broadcastFramesCounter_;
}

const CosemByteBuffer&
CosemMBusDiagnosticObject::TransmissionsCounter() const
{
  return transmissionsCounter_;
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
constexpr std::uint16_t kPrimePlcLlcSscsSetupClassId = 80u;
constexpr std::uint8_t kPrimePlcLlcSscsSetupServiceNodeAddressId = 2u;
constexpr std::uint8_t kPrimePlcLlcSscsSetupBaseNodeAddressId = 3u;
constexpr std::uint8_t kPrimePlcLlcSscsSetupResetMethodId = 1u;
} // namespace

const std::uint8_t CosemPrimePlcLlcSscsSetupObject::MaxSupportedVersion;

CosemPrimePlcLlcSscsSetupObject::CosemPrimePlcLlcSscsSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& serviceNodeAddress,
  const CosemByteBuffer& baseNodeAddress,
  AttributeAccessMode mutableAccess)
  : CosemPrimePlcLlcSscsSetupObject(
      logicalName,
      serviceNodeAddress,
      baseNodeAddress,
      mutableAccess,
      CosemPrimePlcLlcSscsSetupObject::MaxSupportedVersion)
{
}

CosemPrimePlcLlcSscsSetupObject::CosemPrimePlcLlcSscsSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& serviceNodeAddress,
  const CosemByteBuffer& baseNodeAddress,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPrimePlcLlcSscsSetupClassId,
      NormalizeVersion(
        version,
        CosemPrimePlcLlcSscsSetupObject::MaxSupportedVersion),
      logicalName))
  , serviceNodeAddress_(serviceNodeAddress)
  , baseNodeAddress_(baseNodeAddress)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr :
       {kPrimePlcLlcSscsSetupServiceNodeAddressId,
        kPrimePlcLlcSscsSetupBaseNodeAddressId}) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor
CosemPrimePlcLlcSscsSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemPrimePlcLlcSscsSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemPrimePlcLlcSscsSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPrimePlcLlcSscsSetupServiceNodeAddressId:
      output = serviceNodeAddress_;
      return CosemStatus::Ok;
    case kPrimePlcLlcSscsSetupBaseNodeAddressId:
      output = baseNodeAddress_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPrimePlcLlcSscsSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kPrimePlcLlcSscsSetupServiceNodeAddressId:
      target = &serviceNodeAddress_;
      break;
    case kPrimePlcLlcSscsSetupBaseNodeAddressId:
      target = &baseNodeAddress_;
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

CosemStatus CosemPrimePlcLlcSscsSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // Spec defines reset(data) (data ::= integer(0)) as the only
  // method: it sets service_node_address to NEW (0xFFE) and
  // base_node_address to 0. Actual deallocation is owned by the
  // PRIME convergence-layer backend, so the built-in object
  // surfaces UnsupportedFeature just like IC 81/IC 84.
  if (methodId == kPrimePlcLlcSscsSetupResetMethodId)
    return CosemStatus::UnsupportedFeature;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemPrimePlcLlcSscsSetupObject::ServiceNodeAddress() const
{
  return serviceNodeAddress_;
}

const CosemByteBuffer&
CosemPrimePlcLlcSscsSetupObject::BaseNodeAddress() const
{
  return baseNodeAddress_;
}

namespace {
constexpr std::uint16_t kPrimePlcPhyLayerCountersClassId = 81u;
constexpr std::uint8_t kPrimePlcPhyLayerCountersCrcIncorrectId = 2u;
constexpr std::uint8_t kPrimePlcPhyLayerCountersCrcFailedId = 3u;
constexpr std::uint8_t kPrimePlcPhyLayerCountersTxDropId = 4u;
constexpr std::uint8_t kPrimePlcPhyLayerCountersRxDropId = 5u;
constexpr std::uint8_t kPrimePlcPhyLayerCountersResetMethodId = 1u;
} // namespace

const std::uint8_t CosemPrimePlcPhyLayerCountersObject::MaxSupportedVersion;

CosemPrimePlcPhyLayerCountersObject::CosemPrimePlcPhyLayerCountersObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& crcIncorrectCount,
  const CosemByteBuffer& crcFailedCount,
  const CosemByteBuffer& txDropCount,
  const CosemByteBuffer& rxDropCount,
  AttributeAccessMode mutableAccess)
  : CosemPrimePlcPhyLayerCountersObject(
      logicalName,
      crcIncorrectCount,
      crcFailedCount,
      txDropCount,
      rxDropCount,
      mutableAccess,
      CosemPrimePlcPhyLayerCountersObject::MaxSupportedVersion)
{
}

CosemPrimePlcPhyLayerCountersObject::CosemPrimePlcPhyLayerCountersObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& crcIncorrectCount,
  const CosemByteBuffer& crcFailedCount,
  const CosemByteBuffer& txDropCount,
  const CosemByteBuffer& rxDropCount,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPrimePlcPhyLayerCountersClassId,
      NormalizeVersion(
        version,
        CosemPrimePlcPhyLayerCountersObject::MaxSupportedVersion),
      logicalName))
  , crcIncorrectCount_(crcIncorrectCount)
  , crcFailedCount_(crcFailedCount)
  , txDropCount_(txDropCount)
  , rxDropCount_(rxDropCount)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr :
       {kPrimePlcPhyLayerCountersCrcIncorrectId,
        kPrimePlcPhyLayerCountersCrcFailedId,
        kPrimePlcPhyLayerCountersTxDropId,
        kPrimePlcPhyLayerCountersRxDropId}) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor
CosemPrimePlcPhyLayerCountersObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemPrimePlcPhyLayerCountersObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemPrimePlcPhyLayerCountersObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPrimePlcPhyLayerCountersCrcIncorrectId:
      output = crcIncorrectCount_;
      return CosemStatus::Ok;
    case kPrimePlcPhyLayerCountersCrcFailedId:
      output = crcFailedCount_;
      return CosemStatus::Ok;
    case kPrimePlcPhyLayerCountersTxDropId:
      output = txDropCount_;
      return CosemStatus::Ok;
    case kPrimePlcPhyLayerCountersRxDropId:
      output = rxDropCount_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPrimePlcPhyLayerCountersObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kPrimePlcPhyLayerCountersCrcIncorrectId:
      target = &crcIncorrectCount_;
      break;
    case kPrimePlcPhyLayerCountersCrcFailedId:
      target = &crcFailedCount_;
      break;
    case kPrimePlcPhyLayerCountersTxDropId:
      target = &txDropCount_;
      break;
    case kPrimePlcPhyLayerCountersRxDropId:
      target = &rxDropCount_;
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

CosemStatus CosemPrimePlcPhyLayerCountersObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // Spec defines reset(data) as the only method: actual zeroing
  // of PHY counters is owned by the PLC stack backend, so the
  // built-in object surfaces UnsupportedFeature like IC 84.
  if (methodId == kPrimePlcPhyLayerCountersResetMethodId)
    return CosemStatus::UnsupportedFeature;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemPrimePlcPhyLayerCountersObject::CrcIncorrectCount() const
{
  return crcIncorrectCount_;
}

const CosemByteBuffer&
CosemPrimePlcPhyLayerCountersObject::CrcFailedCount() const
{
  return crcFailedCount_;
}

const CosemByteBuffer&
CosemPrimePlcPhyLayerCountersObject::TxDropCount() const
{
  return txDropCount_;
}

const CosemByteBuffer&
CosemPrimePlcPhyLayerCountersObject::RxDropCount() const
{
  return rxDropCount_;
}

namespace {
constexpr std::uint16_t kPrimePlcMacSetupClassId = 82u;
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

namespace {
constexpr std::uint16_t kPrimePlcMacFunctionalParamsClassId = 83u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsLnidId = 2u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsLsidId = 3u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsSidId = 4u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsSnaId = 5u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsStateId = 6u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsScpLengthId = 7u;
constexpr std::uint8_t
  kPrimePlcMacFunctionalParamsNodeHierarchyLevelId = 8u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsBeaconSlotCountId = 9u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsBeaconRxSlotId = 10u;
constexpr std::uint8_t kPrimePlcMacFunctionalParamsBeaconTxSlotId = 11u;
constexpr std::uint8_t
  kPrimePlcMacFunctionalParamsBeaconRxFrequencyId = 12u;
constexpr std::uint8_t
  kPrimePlcMacFunctionalParamsBeaconTxFrequencyId = 13u;
constexpr std::uint8_t
  kPrimePlcMacFunctionalParamsCapabilitiesId = 14u;
} // namespace

const std::uint8_t
  CosemPrimePlcMacFunctionalParametersObject::MaxSupportedVersion;

CosemPrimePlcMacFunctionalParametersObject::
  CosemPrimePlcMacFunctionalParametersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& lnid,
    const CosemByteBuffer& lsid,
    const CosemByteBuffer& sid,
    const CosemByteBuffer& sna,
    const CosemByteBuffer& state,
    const CosemByteBuffer& scpLength,
    const CosemByteBuffer& nodeHierarchyLevel,
    const CosemByteBuffer& beaconSlotCount,
    const CosemByteBuffer& beaconRxSlot,
    const CosemByteBuffer& beaconTxSlot,
    const CosemByteBuffer& beaconRxFrequency,
    const CosemByteBuffer& beaconTxFrequency,
    const CosemByteBuffer& capabilities,
    AttributeAccessMode mutableAccess)
  : CosemPrimePlcMacFunctionalParametersObject(
      logicalName, lnid, lsid, sid, sna, state, scpLength,
      nodeHierarchyLevel, beaconSlotCount, beaconRxSlot,
      beaconTxSlot, beaconRxFrequency, beaconTxFrequency,
      capabilities, mutableAccess,
      CosemPrimePlcMacFunctionalParametersObject::
        MaxSupportedVersion)
{
}

CosemPrimePlcMacFunctionalParametersObject::
  CosemPrimePlcMacFunctionalParametersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& lnid,
    const CosemByteBuffer& lsid,
    const CosemByteBuffer& sid,
    const CosemByteBuffer& sna,
    const CosemByteBuffer& state,
    const CosemByteBuffer& scpLength,
    const CosemByteBuffer& nodeHierarchyLevel,
    const CosemByteBuffer& beaconSlotCount,
    const CosemByteBuffer& beaconRxSlot,
    const CosemByteBuffer& beaconTxSlot,
    const CosemByteBuffer& beaconRxFrequency,
    const CosemByteBuffer& beaconTxFrequency,
    const CosemByteBuffer& capabilities,
    AttributeAccessMode mutableAccess,
    std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPrimePlcMacFunctionalParamsClassId,
      NormalizeVersion(
        version,
        CosemPrimePlcMacFunctionalParametersObject::
          MaxSupportedVersion),
      logicalName))
  , lnid_(lnid)
  , lsid_(lsid)
  , sid_(sid)
  , sna_(sna)
  , state_(state)
  , scpLength_(scpLength)
  , nodeHierarchyLevel_(nodeHierarchyLevel)
  , beaconSlotCount_(beaconSlotCount)
  , beaconRxSlot_(beaconRxSlot)
  , beaconTxSlot_(beaconTxSlot)
  , beaconRxFrequency_(beaconRxFrequency)
  , beaconTxFrequency_(beaconTxFrequency)
  , capabilities_(capabilities)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr :
       {kPrimePlcMacFunctionalParamsLnidId,
        kPrimePlcMacFunctionalParamsLsidId,
        kPrimePlcMacFunctionalParamsSidId,
        kPrimePlcMacFunctionalParamsSnaId,
        kPrimePlcMacFunctionalParamsStateId,
        kPrimePlcMacFunctionalParamsScpLengthId,
        kPrimePlcMacFunctionalParamsNodeHierarchyLevelId,
        kPrimePlcMacFunctionalParamsBeaconSlotCountId,
        kPrimePlcMacFunctionalParamsBeaconRxSlotId,
        kPrimePlcMacFunctionalParamsBeaconTxSlotId,
        kPrimePlcMacFunctionalParamsBeaconRxFrequencyId,
        kPrimePlcMacFunctionalParamsBeaconTxFrequencyId,
        kPrimePlcMacFunctionalParamsCapabilitiesId}) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor
CosemPrimePlcMacFunctionalParametersObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemPrimePlcMacFunctionalParametersObject::AccessRights() const
{
  return rights_;
}

CosemStatus
CosemPrimePlcMacFunctionalParametersObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsLnidId:
      output = lnid_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsLsidId:
      output = lsid_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsSidId:
      output = sid_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsSnaId:
      output = sna_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsStateId:
      output = state_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsScpLengthId:
      output = scpLength_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsNodeHierarchyLevelId:
      output = nodeHierarchyLevel_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsBeaconSlotCountId:
      output = beaconSlotCount_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsBeaconRxSlotId:
      output = beaconRxSlot_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsBeaconTxSlotId:
      output = beaconTxSlot_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsBeaconRxFrequencyId:
      output = beaconRxFrequency_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsBeaconTxFrequencyId:
      output = beaconTxFrequency_;
      return CosemStatus::Ok;
    case kPrimePlcMacFunctionalParamsCapabilitiesId:
      output = capabilities_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus
CosemPrimePlcMacFunctionalParametersObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kPrimePlcMacFunctionalParamsLnidId:
      target = &lnid_;
      break;
    case kPrimePlcMacFunctionalParamsLsidId:
      target = &lsid_;
      break;
    case kPrimePlcMacFunctionalParamsSidId:
      target = &sid_;
      break;
    case kPrimePlcMacFunctionalParamsSnaId:
      target = &sna_;
      break;
    case kPrimePlcMacFunctionalParamsStateId:
      target = &state_;
      break;
    case kPrimePlcMacFunctionalParamsScpLengthId:
      target = &scpLength_;
      break;
    case kPrimePlcMacFunctionalParamsNodeHierarchyLevelId:
      target = &nodeHierarchyLevel_;
      break;
    case kPrimePlcMacFunctionalParamsBeaconSlotCountId:
      target = &beaconSlotCount_;
      break;
    case kPrimePlcMacFunctionalParamsBeaconRxSlotId:
      target = &beaconRxSlot_;
      break;
    case kPrimePlcMacFunctionalParamsBeaconTxSlotId:
      target = &beaconTxSlot_;
      break;
    case kPrimePlcMacFunctionalParamsBeaconRxFrequencyId:
      target = &beaconRxFrequency_;
      break;
    case kPrimePlcMacFunctionalParamsBeaconTxFrequencyId:
      target = &beaconTxFrequency_;
      break;
    case kPrimePlcMacFunctionalParamsCapabilitiesId:
      target = &capabilities_;
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

CosemStatus
CosemPrimePlcMacFunctionalParametersObject::InvokeMethod(
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
CosemPrimePlcMacFunctionalParametersObject::Lnid() const
{
  return lnid_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::Lsid() const
{
  return lsid_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::Sid() const
{
  return sid_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::Sna() const
{
  return sna_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::State() const
{
  return state_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::ScpLength() const
{
  return scpLength_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::NodeHierarchyLevel() const
{
  return nodeHierarchyLevel_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::BeaconSlotCount() const
{
  return beaconSlotCount_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::BeaconRxSlot() const
{
  return beaconRxSlot_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::BeaconTxSlot() const
{
  return beaconTxSlot_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::BeaconRxFrequency() const
{
  return beaconRxFrequency_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::BeaconTxFrequency() const
{
  return beaconTxFrequency_;
}

const CosemByteBuffer&
CosemPrimePlcMacFunctionalParametersObject::Capabilities() const
{
  return capabilities_;
}

namespace {
constexpr std::uint16_t kPrimePlcMacCountersClassId = 84u;
constexpr std::uint8_t kPrimePlcMacCountersTxDataPktCountId = 2u;
constexpr std::uint8_t kPrimePlcMacCountersRxDataPktCountId = 3u;
constexpr std::uint8_t kPrimePlcMacCountersTxCtrlPktCountId = 4u;
constexpr std::uint8_t kPrimePlcMacCountersRxCtrlPktCountId = 5u;
constexpr std::uint8_t kPrimePlcMacCountersCsmaFailCountId = 6u;
constexpr std::uint8_t kPrimePlcMacCountersCsmaChBusyCountId = 7u;
constexpr std::uint8_t kPrimePlcMacCountersResetMethodId = 1u;
} // namespace

const std::uint8_t CosemPrimePlcMacCountersObject::MaxSupportedVersion;

CosemPrimePlcMacCountersObject::CosemPrimePlcMacCountersObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& txDataPktCount,
  const CosemByteBuffer& rxDataPktCount,
  const CosemByteBuffer& txCtrlPktCount,
  const CosemByteBuffer& rxCtrlPktCount,
  const CosemByteBuffer& csmaFailCount,
  const CosemByteBuffer& csmaChBusyCount,
  AttributeAccessMode mutableAccess)
  : CosemPrimePlcMacCountersObject(
      logicalName, txDataPktCount, rxDataPktCount,
      txCtrlPktCount, rxCtrlPktCount, csmaFailCount,
      csmaChBusyCount, mutableAccess,
      CosemPrimePlcMacCountersObject::MaxSupportedVersion)
{
}

CosemPrimePlcMacCountersObject::CosemPrimePlcMacCountersObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& txDataPktCount,
  const CosemByteBuffer& rxDataPktCount,
  const CosemByteBuffer& txCtrlPktCount,
  const CosemByteBuffer& rxCtrlPktCount,
  const CosemByteBuffer& csmaFailCount,
  const CosemByteBuffer& csmaChBusyCount,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPrimePlcMacCountersClassId,
      NormalizeVersion(
        version,
        CosemPrimePlcMacCountersObject::MaxSupportedVersion),
      logicalName))
  , txDataPktCount_(txDataPktCount)
  , rxDataPktCount_(rxDataPktCount)
  , txCtrlPktCount_(txCtrlPktCount)
  , rxCtrlPktCount_(rxCtrlPktCount)
  , csmaFailCount_(csmaFailCount)
  , csmaChBusyCount_(csmaChBusyCount)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr :
       {kPrimePlcMacCountersTxDataPktCountId,
        kPrimePlcMacCountersRxDataPktCountId,
        kPrimePlcMacCountersTxCtrlPktCountId,
        kPrimePlcMacCountersRxCtrlPktCountId,
        kPrimePlcMacCountersCsmaFailCountId,
        kPrimePlcMacCountersCsmaChBusyCountId}) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor
CosemPrimePlcMacCountersObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemPrimePlcMacCountersObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemPrimePlcMacCountersObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPrimePlcMacCountersTxDataPktCountId:
      output = txDataPktCount_;
      return CosemStatus::Ok;
    case kPrimePlcMacCountersRxDataPktCountId:
      output = rxDataPktCount_;
      return CosemStatus::Ok;
    case kPrimePlcMacCountersTxCtrlPktCountId:
      output = txCtrlPktCount_;
      return CosemStatus::Ok;
    case kPrimePlcMacCountersRxCtrlPktCountId:
      output = rxCtrlPktCount_;
      return CosemStatus::Ok;
    case kPrimePlcMacCountersCsmaFailCountId:
      output = csmaFailCount_;
      return CosemStatus::Ok;
    case kPrimePlcMacCountersCsmaChBusyCountId:
      output = csmaChBusyCount_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemPrimePlcMacCountersObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kPrimePlcMacCountersTxDataPktCountId:
      target = &txDataPktCount_;
      break;
    case kPrimePlcMacCountersRxDataPktCountId:
      target = &rxDataPktCount_;
      break;
    case kPrimePlcMacCountersTxCtrlPktCountId:
      target = &txCtrlPktCount_;
      break;
    case kPrimePlcMacCountersRxCtrlPktCountId:
      target = &rxCtrlPktCount_;
      break;
    case kPrimePlcMacCountersCsmaFailCountId:
      target = &csmaFailCount_;
      break;
    case kPrimePlcMacCountersCsmaChBusyCountId:
      target = &csmaChBusyCount_;
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

CosemStatus CosemPrimePlcMacCountersObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kPrimePlcMacCountersResetMethodId)
    return CosemStatus::UnsupportedFeature;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemPrimePlcMacCountersObject::TxDataPktCount() const
{
  return txDataPktCount_;
}

const CosemByteBuffer&
CosemPrimePlcMacCountersObject::RxDataPktCount() const
{
  return rxDataPktCount_;
}

const CosemByteBuffer&
CosemPrimePlcMacCountersObject::TxCtrlPktCount() const
{
  return txCtrlPktCount_;
}

const CosemByteBuffer&
CosemPrimePlcMacCountersObject::RxCtrlPktCount() const
{
  return rxCtrlPktCount_;
}

const CosemByteBuffer&
CosemPrimePlcMacCountersObject::CsmaFailCount() const
{
  return csmaFailCount_;
}

const CosemByteBuffer&
CosemPrimePlcMacCountersObject::CsmaChBusyCount() const
{
  return csmaChBusyCount_;
}

namespace {
constexpr std::uint16_t kPrimePlcMacNetworkAdminDataClassId = 85u;
constexpr std::uint8_t kPrimePlcMacNetworkAdminDataMulticastEntriesId = 2u;
constexpr std::uint8_t kPrimePlcMacNetworkAdminDataSwitchTableId = 3u;
constexpr std::uint8_t kPrimePlcMacNetworkAdminDataDirectTableId = 4u;
constexpr std::uint8_t kPrimePlcMacNetworkAdminDataAvailableSwitchesId = 5u;
constexpr std::uint8_t kPrimePlcMacNetworkAdminDataPhyCommId = 6u;
constexpr std::uint8_t kPrimePlcMacNetworkAdminDataResetMethodId = 1u;
} // namespace

const std::uint8_t
  CosemPrimePlcMacNetworkAdminDataObject::MaxSupportedVersion;

CosemPrimePlcMacNetworkAdminDataObject::
  CosemPrimePlcMacNetworkAdminDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& macListMulticastEntries,
    const CosemByteBuffer& macListSwitchTable,
    const CosemByteBuffer& macListDirectTable,
    const CosemByteBuffer& macListAvailableSwitches,
    const CosemByteBuffer& macListPhyComm,
    AttributeAccessMode mutableAccess)
  : CosemPrimePlcMacNetworkAdminDataObject(
      logicalName, macListMulticastEntries, macListSwitchTable,
      macListDirectTable, macListAvailableSwitches,
      macListPhyComm, mutableAccess,
      CosemPrimePlcMacNetworkAdminDataObject::MaxSupportedVersion)
{
}

CosemPrimePlcMacNetworkAdminDataObject::
  CosemPrimePlcMacNetworkAdminDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& macListMulticastEntries,
    const CosemByteBuffer& macListSwitchTable,
    const CosemByteBuffer& macListDirectTable,
    const CosemByteBuffer& macListAvailableSwitches,
    const CosemByteBuffer& macListPhyComm,
    AttributeAccessMode mutableAccess,
    std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPrimePlcMacNetworkAdminDataClassId,
      NormalizeVersion(
        version,
        CosemPrimePlcMacNetworkAdminDataObject::
          MaxSupportedVersion),
      logicalName))
  , macListMulticastEntries_(macListMulticastEntries)
  , macListSwitchTable_(macListSwitchTable)
  , macListDirectTable_(macListDirectTable)
  , macListAvailableSwitches_(macListAvailableSwitches)
  , macListPhyComm_(macListPhyComm)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr :
       {kPrimePlcMacNetworkAdminDataMulticastEntriesId,
        kPrimePlcMacNetworkAdminDataSwitchTableId,
        kPrimePlcMacNetworkAdminDataDirectTableId,
        kPrimePlcMacNetworkAdminDataAvailableSwitchesId,
        kPrimePlcMacNetworkAdminDataPhyCommId}) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor
CosemPrimePlcMacNetworkAdminDataObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemPrimePlcMacNetworkAdminDataObject::AccessRights() const
{
  return rights_;
}

CosemStatus
CosemPrimePlcMacNetworkAdminDataObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPrimePlcMacNetworkAdminDataMulticastEntriesId:
      output = macListMulticastEntries_;
      return CosemStatus::Ok;
    case kPrimePlcMacNetworkAdminDataSwitchTableId:
      output = macListSwitchTable_;
      return CosemStatus::Ok;
    case kPrimePlcMacNetworkAdminDataDirectTableId:
      output = macListDirectTable_;
      return CosemStatus::Ok;
    case kPrimePlcMacNetworkAdminDataAvailableSwitchesId:
      output = macListAvailableSwitches_;
      return CosemStatus::Ok;
    case kPrimePlcMacNetworkAdminDataPhyCommId:
      output = macListPhyComm_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus
CosemPrimePlcMacNetworkAdminDataObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kPrimePlcMacNetworkAdminDataMulticastEntriesId:
      target = &macListMulticastEntries_;
      break;
    case kPrimePlcMacNetworkAdminDataSwitchTableId:
      target = &macListSwitchTable_;
      break;
    case kPrimePlcMacNetworkAdminDataDirectTableId:
      target = &macListDirectTable_;
      break;
    case kPrimePlcMacNetworkAdminDataAvailableSwitchesId:
      target = &macListAvailableSwitches_;
      break;
    case kPrimePlcMacNetworkAdminDataPhyCommId:
      target = &macListPhyComm_;
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

CosemStatus
CosemPrimePlcMacNetworkAdminDataObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kPrimePlcMacNetworkAdminDataResetMethodId)
    return CosemStatus::UnsupportedFeature;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemPrimePlcMacNetworkAdminDataObject::MacListMulticastEntries() const
{
  return macListMulticastEntries_;
}

const CosemByteBuffer&
CosemPrimePlcMacNetworkAdminDataObject::MacListSwitchTable() const
{
  return macListSwitchTable_;
}

const CosemByteBuffer&
CosemPrimePlcMacNetworkAdminDataObject::MacListDirectTable() const
{
  return macListDirectTable_;
}

const CosemByteBuffer&
CosemPrimePlcMacNetworkAdminDataObject::MacListAvailableSwitches() const
{
  return macListAvailableSwitches_;
}

const CosemByteBuffer&
CosemPrimePlcMacNetworkAdminDataObject::MacListPhyComm() const
{
  return macListPhyComm_;
}

namespace {
constexpr std::uint16_t kPrimePlcApplicationIdentificationClassId = 86u;
constexpr std::uint8_t kPrimePlcApplicationIdentificationFirmwareVersionId = 2u;
constexpr std::uint8_t kPrimePlcApplicationIdentificationVendorIdId = 3u;
constexpr std::uint8_t kPrimePlcApplicationIdentificationProductIdId = 4u;
} // namespace

const std::uint8_t
  CosemPrimePlcApplicationIdentificationObject::MaxSupportedVersion;

CosemPrimePlcApplicationIdentificationObject::
  CosemPrimePlcApplicationIdentificationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& firmwareVersion,
    const CosemByteBuffer& vendorId,
    const CosemByteBuffer& productId,
    AttributeAccessMode mutableAccess)
  : CosemPrimePlcApplicationIdentificationObject(
      logicalName, firmwareVersion, vendorId, productId,
      mutableAccess,
      CosemPrimePlcApplicationIdentificationObject::
        MaxSupportedVersion)
{
}

CosemPrimePlcApplicationIdentificationObject::
  CosemPrimePlcApplicationIdentificationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& firmwareVersion,
    const CosemByteBuffer& vendorId,
    const CosemByteBuffer& productId,
    AttributeAccessMode mutableAccess,
    std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kPrimePlcApplicationIdentificationClassId,
      NormalizeVersion(
        version,
        CosemPrimePlcApplicationIdentificationObject::
          MaxSupportedVersion),
      logicalName))
  , firmwareVersion_(firmwareVersion)
  , vendorId_(vendorId)
  , productId_(productId)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr :
       {kPrimePlcApplicationIdentificationFirmwareVersionId,
        kPrimePlcApplicationIdentificationVendorIdId,
        kPrimePlcApplicationIdentificationProductIdId}) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor
CosemPrimePlcApplicationIdentificationObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights
CosemPrimePlcApplicationIdentificationObject::AccessRights() const
{
  return rights_;
}

CosemStatus
CosemPrimePlcApplicationIdentificationObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kPrimePlcApplicationIdentificationFirmwareVersionId:
      output = firmwareVersion_;
      return CosemStatus::Ok;
    case kPrimePlcApplicationIdentificationVendorIdId:
      output = vendorId_;
      return CosemStatus::Ok;
    case kPrimePlcApplicationIdentificationProductIdId:
      output = productId_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus
CosemPrimePlcApplicationIdentificationObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case kPrimePlcApplicationIdentificationFirmwareVersionId:
      target = &firmwareVersion_;
      break;
    case kPrimePlcApplicationIdentificationVendorIdId:
      target = &vendorId_;
      break;
    case kPrimePlcApplicationIdentificationProductIdId:
      target = &productId_;
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

CosemStatus
CosemPrimePlcApplicationIdentificationObject::InvokeMethod(
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
CosemPrimePlcApplicationIdentificationObject::FirmwareVersion() const
{
  return firmwareVersion_;
}

const CosemByteBuffer&
CosemPrimePlcApplicationIdentificationObject::VendorId() const
{
  return vendorId_;
}

const CosemByteBuffer&
CosemPrimePlcApplicationIdentificationObject::ProductId() const
{
  return productId_;
}

namespace {
constexpr std::uint16_t kSFskPlcPhyMacSetupClassId = 50u;
constexpr std::uint8_t kSFskPlcPhyMacSetupResetMethodId = 1u;
constexpr std::uint8_t kSFskPlcPhyMacSetupFirstMutableAttributeId = 2u;
constexpr std::uint8_t kSFskPlcPhyMacSetupLastMutableAttributeId = 15u;
} // namespace

const std::uint8_t CosemSFskPlcPhyMacSetupObject::MaxSupportedVersion;

CosemSFskPlcPhyMacSetupObject::CosemSFskPlcPhyMacSetupObject(
  const CosemLogicalName& logicalName,
  const Attributes& attributes,
  AttributeAccessMode mutableAccess)
  : CosemSFskPlcPhyMacSetupObject(
      logicalName, attributes, mutableAccess,
      CosemSFskPlcPhyMacSetupObject::MaxSupportedVersion)
{
}

CosemSFskPlcPhyMacSetupObject::CosemSFskPlcPhyMacSetupObject(
  const CosemLogicalName& logicalName,
  const Attributes& attributes,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSFskPlcPhyMacSetupClassId,
      NormalizeVersion(
        version,
        CosemSFskPlcPhyMacSetupObject::MaxSupportedVersion),
      logicalName))
  , attributes_(attributes)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t attr =
         kSFskPlcPhyMacSetupFirstMutableAttributeId;
       attr <= kSFskPlcPhyMacSetupLastMutableAttributeId; ++attr) {
    rights_.SetAttributeAccess(attr, mutableAccess);
  }
}

CosemObjectDescriptor CosemSFskPlcPhyMacSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSFskPlcPhyMacSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSFskPlcPhyMacSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case 2u: output = attributes_.initiatorElectricalPhase; return CosemStatus::Ok;
    case 3u: output = attributes_.deltaElectricalPhase; return CosemStatus::Ok;
    case 4u: output = attributes_.maxReceivingGain; return CosemStatus::Ok;
    case 5u: output = attributes_.maxTransmittingGain; return CosemStatus::Ok;
    case 6u: output = attributes_.searchInitiatorThreshold; return CosemStatus::Ok;
    case 7u: output = attributes_.frequencies; return CosemStatus::Ok;
    case 8u: output = attributes_.macAddress; return CosemStatus::Ok;
    case 9u: output = attributes_.macGroupAddresses; return CosemStatus::Ok;
    case 10u: output = attributes_.repeater; return CosemStatus::Ok;
    case 11u: output = attributes_.repeaterStatus; return CosemStatus::Ok;
    case 12u: output = attributes_.minDeltaCredit; return CosemStatus::Ok;
    case 13u: output = attributes_.initiatorMacAddress; return CosemStatus::Ok;
    case 14u: output = attributes_.synchronizationLocked; return CosemStatus::Ok;
    case 15u: output = attributes_.transmissionSpeed; return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskPlcPhyMacSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  CosemByteBuffer* target = nullptr;
  switch (attributeId) {
    case 2u: target = &attributes_.initiatorElectricalPhase; break;
    case 3u: target = &attributes_.deltaElectricalPhase; break;
    case 4u: target = &attributes_.maxReceivingGain; break;
    case 5u: target = &attributes_.maxTransmittingGain; break;
    case 6u: target = &attributes_.searchInitiatorThreshold; break;
    case 7u: target = &attributes_.frequencies; break;
    case 8u: target = &attributes_.macAddress; break;
    case 9u: target = &attributes_.macGroupAddresses; break;
    case 10u: target = &attributes_.repeater; break;
    case 11u: target = &attributes_.repeaterStatus; break;
    case 12u: target = &attributes_.minDeltaCredit; break;
    case 13u: target = &attributes_.initiatorMacAddress; break;
    case 14u: target = &attributes_.synchronizationLocked; break;
    case 15u: target = &attributes_.transmissionSpeed; break;
    case kLogicalNameAttributeId: return CosemStatus::AccessDenied;
    default: return CosemStatus::AttributeNotFound;
  }
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  *target = input;
  return CosemStatus::Ok;
}

CosemStatus CosemSFskPlcPhyMacSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kSFskPlcPhyMacSetupResetMethodId)
    return CosemStatus::UnsupportedFeature;
  return CosemStatus::MethodNotFound;
}

const CosemSFskPlcPhyMacSetupObject::Attributes&
CosemSFskPlcPhyMacSetupObject::AttributeData() const
{
  return attributes_;
}

namespace {
constexpr std::uint16_t kSFskActiveInitiatorClassId = 51u;
constexpr std::uint8_t kSFskActiveInitiatorAttributeId = 2u;
constexpr std::uint8_t kSFskActiveInitiatorResetMethodId = 1u;
} // namespace

const std::uint8_t
  CosemSFskActiveInitiatorObject::MaxSupportedVersion;

CosemSFskActiveInitiatorObject::CosemSFskActiveInitiatorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& activeInitiator,
  AttributeAccessMode mutableAccess)
  : CosemSFskActiveInitiatorObject(
      logicalName, activeInitiator, mutableAccess,
      CosemSFskActiveInitiatorObject::MaxSupportedVersion)
{
}

CosemSFskActiveInitiatorObject::CosemSFskActiveInitiatorObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& activeInitiator,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSFskActiveInitiatorClassId,
      NormalizeVersion(
        version,
        CosemSFskActiveInitiatorObject::MaxSupportedVersion),
      logicalName))
  , activeInitiator_(activeInitiator)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSFskActiveInitiatorAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemSFskActiveInitiatorObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSFskActiveInitiatorObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSFskActiveInitiatorObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSFskActiveInitiatorAttributeId:
      output = activeInitiator_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskActiveInitiatorObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSFskActiveInitiatorAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      activeInitiator_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskActiveInitiatorObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kSFskActiveInitiatorResetMethodId)
    return CosemStatus::UnsupportedFeature;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemSFskActiveInitiatorObject::ActiveInitiator() const
{
  return activeInitiator_;
}

namespace {
constexpr std::uint16_t kSFskMacSyncTimeoutsClassId = 52u;
constexpr std::uint8_t kSFskMacSyncTimeoutsSearchInitiatorTimeoutAttributeId = 2u;
constexpr std::uint8_t kSFskMacSyncTimeoutsSyncConfirmationTimeoutAttributeId = 3u;
constexpr std::uint8_t kSFskMacSyncTimeoutsTimeOutNotAddressedAttributeId = 4u;
constexpr std::uint8_t kSFskMacSyncTimeoutsTimeOutFrameNotOkAttributeId = 5u;
} // namespace

const std::uint8_t
  CosemSFskMacSyncTimeoutsObject::MaxSupportedVersion;

CosemSFskMacSyncTimeoutsObject::CosemSFskMacSyncTimeoutsObject(
  const CosemLogicalName& logicalName,
  std::uint16_t searchInitiatorTimeout,
  std::uint16_t synchronizationConfirmationTimeout,
  std::uint16_t timeOutNotAddressed,
  std::uint16_t timeOutFrameNotOk,
  AttributeAccessMode mutableAccess)
  : CosemSFskMacSyncTimeoutsObject(
      logicalName,
      searchInitiatorTimeout,
      synchronizationConfirmationTimeout,
      timeOutNotAddressed,
      timeOutFrameNotOk,
      mutableAccess,
      CosemSFskMacSyncTimeoutsObject::MaxSupportedVersion)
{
}

CosemSFskMacSyncTimeoutsObject::CosemSFskMacSyncTimeoutsObject(
  const CosemLogicalName& logicalName,
  std::uint16_t searchInitiatorTimeout,
  std::uint16_t synchronizationConfirmationTimeout,
  std::uint16_t timeOutNotAddressed,
  std::uint16_t timeOutFrameNotOk,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSFskMacSyncTimeoutsClassId,
      NormalizeVersion(
        version,
        CosemSFskMacSyncTimeoutsObject::MaxSupportedVersion),
      logicalName))
  , searchInitiatorTimeout_(searchInitiatorTimeout)
  , synchronizationConfirmationTimeout_(
      synchronizationConfirmationTimeout)
  , timeOutNotAddressed_(timeOutNotAddressed)
  , timeOutFrameNotOk_(timeOutFrameNotOk)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSFskMacSyncTimeoutsSearchInitiatorTimeoutAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacSyncTimeoutsSyncConfirmationTimeoutAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacSyncTimeoutsTimeOutNotAddressedAttributeId,
    mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacSyncTimeoutsTimeOutFrameNotOkAttributeId,
    mutableAccess);
}

CosemObjectDescriptor CosemSFskMacSyncTimeoutsObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSFskMacSyncTimeoutsObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSFskMacSyncTimeoutsObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSFskMacSyncTimeoutsSearchInitiatorTimeoutAttributeId:
      output.clear();
      AppendLongUnsigned(output, searchInitiatorTimeout_);
      return CosemStatus::Ok;
    case kSFskMacSyncTimeoutsSyncConfirmationTimeoutAttributeId:
      output.clear();
      AppendLongUnsigned(output, synchronizationConfirmationTimeout_);
      return CosemStatus::Ok;
    case kSFskMacSyncTimeoutsTimeOutNotAddressedAttributeId:
      output.clear();
      AppendLongUnsigned(output, timeOutNotAddressed_);
      return CosemStatus::Ok;
    case kSFskMacSyncTimeoutsTimeOutFrameNotOkAttributeId:
      output.clear();
      AppendLongUnsigned(output, timeOutFrameNotOk_);
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskMacSyncTimeoutsObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  auto decodeLU = [&](std::uint16_t& target) -> CosemStatus {
    if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
      return CosemStatus::AccessDenied;
    std::size_t offset = 0u;
    std::uint16_t value = 0u;
    if (!ReadLongUnsignedValue(input, offset, value))
      return CosemStatus::InvalidArgument;
    if (offset != input.size())
      return CosemStatus::InvalidArgument;
    target = value;
    return CosemStatus::Ok;
  };
  switch (attributeId) {
    case kSFskMacSyncTimeoutsSearchInitiatorTimeoutAttributeId:
      return decodeLU(searchInitiatorTimeout_);
    case kSFskMacSyncTimeoutsSyncConfirmationTimeoutAttributeId:
      return decodeLU(synchronizationConfirmationTimeout_);
    case kSFskMacSyncTimeoutsTimeOutNotAddressedAttributeId:
      return decodeLU(timeOutNotAddressed_);
    case kSFskMacSyncTimeoutsTimeOutFrameNotOkAttributeId:
      return decodeLU(timeOutFrameNotOk_);
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskMacSyncTimeoutsObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  output.clear();
  // IEC 62056-6-2 ED4 (2021) §4.10.5 and DLMS UA Blue Book Ed. 12.1
  // §4.10.5 define class_id 52, version 0 with no specific methods.
  return CosemStatus::MethodNotFound;
}

std::uint16_t
CosemSFskMacSyncTimeoutsObject::SearchInitiatorTimeout() const
{
  return searchInitiatorTimeout_;
}

std::uint16_t
CosemSFskMacSyncTimeoutsObject::SynchronizationConfirmationTimeout() const
{
  return synchronizationConfirmationTimeout_;
}

std::uint16_t
CosemSFskMacSyncTimeoutsObject::TimeOutNotAddressed() const
{
  return timeOutNotAddressed_;
}

std::uint16_t
CosemSFskMacSyncTimeoutsObject::TimeOutFrameNotOk() const
{
  return timeOutFrameNotOk_;
}

namespace {
constexpr std::uint16_t kSFskMacCountersClassId = 53u;
constexpr std::uint8_t kSFskMacCountersSynchronizationRegisterAttributeId = 2u;
constexpr std::uint8_t kSFskMacCountersDesynchronizationListingAttributeId = 3u;
constexpr std::uint8_t kSFskMacCountersBroadcastFramesCounterAttributeId = 4u;
constexpr std::uint8_t kSFskMacCountersRepetitionsCounterAttributeId = 5u;
constexpr std::uint8_t kSFskMacCountersTransmissionsCounterAttributeId = 6u;
constexpr std::uint8_t kSFskMacCountersCrcOkFramesCounterAttributeId = 7u;
constexpr std::uint8_t kSFskMacCountersCrcNokFramesCounterAttributeId = 8u;
constexpr std::uint8_t kSFskMacCountersResetMethodId = 1u;
} // namespace

const std::uint8_t CosemSFskMacCountersObject::MaxSupportedVersion;

CosemSFskMacCountersObject::CosemSFskMacCountersObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& synchronizationRegister,
  const CosemByteBuffer& desynchronizationListing,
  const CosemByteBuffer& broadcastFramesCounter,
  const CosemByteBuffer& repetitionsCounter,
  const CosemByteBuffer& transmissionsCounter,
  const CosemByteBuffer& crcOkFramesCounter,
  const CosemByteBuffer& crcNokFramesCounter,
  AttributeAccessMode mutableAccess)
  : CosemSFskMacCountersObject(
      logicalName,
      synchronizationRegister,
      desynchronizationListing,
      broadcastFramesCounter,
      repetitionsCounter,
      transmissionsCounter,
      crcOkFramesCounter,
      crcNokFramesCounter,
      mutableAccess,
      CosemSFskMacCountersObject::MaxSupportedVersion)
{
}

CosemSFskMacCountersObject::CosemSFskMacCountersObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& synchronizationRegister,
  const CosemByteBuffer& desynchronizationListing,
  const CosemByteBuffer& broadcastFramesCounter,
  const CosemByteBuffer& repetitionsCounter,
  const CosemByteBuffer& transmissionsCounter,
  const CosemByteBuffer& crcOkFramesCounter,
  const CosemByteBuffer& crcNokFramesCounter,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSFskMacCountersClassId,
      NormalizeVersion(
        version, CosemSFskMacCountersObject::MaxSupportedVersion),
      logicalName))
  , synchronizationRegister_(synchronizationRegister)
  , desynchronizationListing_(desynchronizationListing)
  , broadcastFramesCounter_(broadcastFramesCounter)
  , repetitionsCounter_(repetitionsCounter)
  , transmissionsCounter_(transmissionsCounter)
  , crcOkFramesCounter_(crcOkFramesCounter)
  , crcNokFramesCounter_(crcNokFramesCounter)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSFskMacCountersSynchronizationRegisterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacCountersDesynchronizationListingAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacCountersBroadcastFramesCounterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacCountersRepetitionsCounterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacCountersTransmissionsCounterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacCountersCrcOkFramesCounterAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kSFskMacCountersCrcNokFramesCounterAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemSFskMacCountersObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSFskMacCountersObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSFskMacCountersObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSFskMacCountersSynchronizationRegisterAttributeId:
      output = synchronizationRegister_;
      return CosemStatus::Ok;
    case kSFskMacCountersDesynchronizationListingAttributeId:
      output = desynchronizationListing_;
      return CosemStatus::Ok;
    case kSFskMacCountersBroadcastFramesCounterAttributeId:
      output = broadcastFramesCounter_;
      return CosemStatus::Ok;
    case kSFskMacCountersRepetitionsCounterAttributeId:
      output = repetitionsCounter_;
      return CosemStatus::Ok;
    case kSFskMacCountersTransmissionsCounterAttributeId:
      output = transmissionsCounter_;
      return CosemStatus::Ok;
    case kSFskMacCountersCrcOkFramesCounterAttributeId:
      output = crcOkFramesCounter_;
      return CosemStatus::Ok;
    case kSFskMacCountersCrcNokFramesCounterAttributeId:
      output = crcNokFramesCounter_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskMacCountersObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSFskMacCountersSynchronizationRegisterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      synchronizationRegister_ = input;
      return CosemStatus::Ok;
    case kSFskMacCountersDesynchronizationListingAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      desynchronizationListing_ = input;
      return CosemStatus::Ok;
    case kSFskMacCountersBroadcastFramesCounterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      broadcastFramesCounter_ = input;
      return CosemStatus::Ok;
    case kSFskMacCountersRepetitionsCounterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      repetitionsCounter_ = input;
      return CosemStatus::Ok;
    case kSFskMacCountersTransmissionsCounterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      transmissionsCounter_ = input;
      return CosemStatus::Ok;
    case kSFskMacCountersCrcOkFramesCounterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      crcOkFramesCounter_ = input;
      return CosemStatus::Ok;
    case kSFskMacCountersCrcNokFramesCounterAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      crcNokFramesCounter_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskMacCountersObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  // IEC 62056-6-2 ED4 (2021) §4.10.6 / DLMS UA Blue Book Ed. 12.1
  // §4.10.6 define method 1 reset(data) which clears the dynamic
  // counters. Counter bookkeeping is backend-owned, so the built-in
  // object surfaces it as UnsupportedFeature.
  if (methodId == kSFskMacCountersResetMethodId)
    return CosemStatus::UnsupportedFeature;
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemSFskMacCountersObject::SynchronizationRegister() const
{
  return synchronizationRegister_;
}

const CosemByteBuffer&
CosemSFskMacCountersObject::DesynchronizationListing() const
{
  return desynchronizationListing_;
}

const CosemByteBuffer&
CosemSFskMacCountersObject::BroadcastFramesCounter() const
{
  return broadcastFramesCounter_;
}

const CosemByteBuffer&
CosemSFskMacCountersObject::RepetitionsCounter() const
{
  return repetitionsCounter_;
}

const CosemByteBuffer&
CosemSFskMacCountersObject::TransmissionsCounter() const
{
  return transmissionsCounter_;
}

const CosemByteBuffer&
CosemSFskMacCountersObject::CrcOkFramesCounter() const
{
  return crcOkFramesCounter_;
}

const CosemByteBuffer&
CosemSFskMacCountersObject::CrcNokFramesCounter() const
{
  return crcNokFramesCounter_;
}

namespace {
constexpr std::uint16_t kIec61334432LlcSetupClassId = 55u;
constexpr std::uint8_t kIec61334432LlcSetupMaxFrameLengthAttributeId = 2u;
constexpr std::uint8_t kIec61334432LlcSetupReplyStatusListAttributeId = 3u;
} // namespace

const std::uint8_t CosemIec61334432LlcSetupObject::MaxSupportedVersion;

CosemIec61334432LlcSetupObject::CosemIec61334432LlcSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& maxFrameLength,
  const CosemByteBuffer& replyStatusList,
  AttributeAccessMode mutableAccess)
  : CosemIec61334432LlcSetupObject(
      logicalName,
      maxFrameLength,
      replyStatusList,
      mutableAccess,
      CosemIec61334432LlcSetupObject::MaxSupportedVersion)
{
}

CosemIec61334432LlcSetupObject::CosemIec61334432LlcSetupObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& maxFrameLength,
  const CosemByteBuffer& replyStatusList,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIec61334432LlcSetupClassId,
      NormalizeVersion(
        version, CosemIec61334432LlcSetupObject::MaxSupportedVersion),
      logicalName))
  , maxFrameLength_(maxFrameLength)
  , replyStatusList_(replyStatusList)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIec61334432LlcSetupMaxFrameLengthAttributeId, mutableAccess);
  rights_.SetAttributeAccess(
    kIec61334432LlcSetupReplyStatusListAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemIec61334432LlcSetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIec61334432LlcSetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIec61334432LlcSetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIec61334432LlcSetupMaxFrameLengthAttributeId:
      output = maxFrameLength_;
      return CosemStatus::Ok;
    case kIec61334432LlcSetupReplyStatusListAttributeId:
      output = replyStatusList_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIec61334432LlcSetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIec61334432LlcSetupMaxFrameLengthAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      maxFrameLength_ = input;
      return CosemStatus::Ok;
    case kIec61334432LlcSetupReplyStatusListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      replyStatusList_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIec61334432LlcSetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IEC 62056-6-2 ED4 (2021) §4.10.7 and DLMS UA Blue Book Ed. 12.1
  // §4.10.7 define no specific methods for IC 55 v1.
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemIec61334432LlcSetupObject::MaxFrameLength() const
{
  return maxFrameLength_;
}

const CosemByteBuffer&
CosemIec61334432LlcSetupObject::ReplyStatusList() const
{
  return replyStatusList_;
}

namespace {
constexpr std::uint16_t kSFskReportingSystemListClassId = 56u;
constexpr std::uint8_t kSFskReportingSystemListAttributeId = 2u;
} // namespace

const std::uint8_t CosemSFskReportingSystemListObject::MaxSupportedVersion;

CosemSFskReportingSystemListObject::CosemSFskReportingSystemListObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& reportingSystemList,
  AttributeAccessMode mutableAccess)
  : CosemSFskReportingSystemListObject(
      logicalName,
      reportingSystemList,
      mutableAccess,
      CosemSFskReportingSystemListObject::MaxSupportedVersion)
{
}

CosemSFskReportingSystemListObject::CosemSFskReportingSystemListObject(
  const CosemLogicalName& logicalName,
  const CosemByteBuffer& reportingSystemList,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kSFskReportingSystemListClassId,
      NormalizeVersion(
        version, CosemSFskReportingSystemListObject::MaxSupportedVersion),
      logicalName))
  , reportingSystemList_(reportingSystemList)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kSFskReportingSystemListAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemSFskReportingSystemListObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemSFskReportingSystemListObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemSFskReportingSystemListObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kSFskReportingSystemListAttributeId:
      output = reportingSystemList_;
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskReportingSystemListObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kSFskReportingSystemListAttributeId:
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      reportingSystemList_ = input;
      return CosemStatus::Ok;
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemSFskReportingSystemListObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IEC 62056-6-2 ED4 (2021) §4.10.8 and DLMS UA Blue Book Ed. 12.1
  // §4.10.8 define no specific methods for IC 56 v0.
  output.clear();
  return CosemStatus::MethodNotFound;
}

const CosemByteBuffer&
CosemSFskReportingSystemListObject::ReportingSystemList() const
{
  return reportingSystemList_;
}

namespace {
constexpr std::uint16_t kIso8802LlcType1SetupClassId = 57u;
constexpr std::uint8_t kIso8802LlcType1SetupMaxOctetsUiPduAttributeId = 2u;
} // namespace

const std::uint8_t CosemIso8802LlcType1SetupObject::MaxSupportedVersion;

CosemIso8802LlcType1SetupObject::CosemIso8802LlcType1SetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t maxOctetsUiPdu,
  AttributeAccessMode mutableAccess)
  : CosemIso8802LlcType1SetupObject(
      logicalName,
      maxOctetsUiPdu,
      mutableAccess,
      CosemIso8802LlcType1SetupObject::MaxSupportedVersion)
{
}

CosemIso8802LlcType1SetupObject::CosemIso8802LlcType1SetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t maxOctetsUiPdu,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIso8802LlcType1SetupClassId,
      NormalizeVersion(
        version, CosemIso8802LlcType1SetupObject::MaxSupportedVersion),
      logicalName))
  , maxOctetsUiPdu_(maxOctetsUiPdu)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  rights_.SetAttributeAccess(
    kIso8802LlcType1SetupMaxOctetsUiPduAttributeId, mutableAccess);
}

CosemObjectDescriptor CosemIso8802LlcType1SetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIso8802LlcType1SetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIso8802LlcType1SetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case kIso8802LlcType1SetupMaxOctetsUiPduAttributeId:
      output.clear();
      AppendLongUnsigned(output, maxOctetsUiPdu_);
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIso8802LlcType1SetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  switch (attributeId) {
    case kIso8802LlcType1SetupMaxOctetsUiPduAttributeId: {
      if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
        return CosemStatus::AccessDenied;
      std::size_t offset = 0u;
      std::uint16_t decoded = 0u;
      if (!ReadLongUnsignedValue(input, offset, decoded))
        return CosemStatus::InvalidArgument;
      if (offset != input.size())
        return CosemStatus::InvalidArgument;
      maxOctetsUiPdu_ = decoded;
      return CosemStatus::Ok;
    }
    case kLogicalNameAttributeId:
      return CosemStatus::AccessDenied;
    default:
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIso8802LlcType1SetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IEC 62056-6-2 ED4 (2021) §4.11.2 and DLMS UA Blue Book Ed. 12.1
  // §4.11.2 define no specific methods for IC 57 v0.
  output.clear();
  return CosemStatus::MethodNotFound;
}

std::uint16_t
CosemIso8802LlcType1SetupObject::MaxOctetsUiPdu() const
{
  return maxOctetsUiPdu_;
}

namespace {
constexpr std::uint16_t kIso8802LlcType2SetupClassId = 58u;
} // namespace

const std::uint8_t CosemIso8802LlcType2SetupObject::MaxSupportedVersion;

bool CosemIso8802LlcType2SetupObject::IsValidWindowSize(std::uint8_t value)
{
  return value >= 1u && value <= 127u;
}

namespace {
std::uint8_t NormalizeWindowSize(std::uint8_t value)
{
  return CosemIso8802LlcType2SetupObject::IsValidWindowSize(value)
           ? value
           : static_cast<std::uint8_t>(1u);
}
} // namespace

CosemIso8802LlcType2SetupObject::CosemIso8802LlcType2SetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t transmitWindowSizeK,
  std::uint8_t receiveWindowSizeRw,
  std::uint16_t maxOctetsIPduN1,
  std::uint8_t maxNumberTransmissionsN2,
  std::uint16_t acknowledgementTimer,
  std::uint16_t pBitTimer,
  std::uint16_t rejectTimer,
  std::uint16_t busyStateTimer,
  AttributeAccessMode mutableAccess)
  : CosemIso8802LlcType2SetupObject(
      logicalName,
      transmitWindowSizeK,
      receiveWindowSizeRw,
      maxOctetsIPduN1,
      maxNumberTransmissionsN2,
      acknowledgementTimer,
      pBitTimer,
      rejectTimer,
      busyStateTimer,
      mutableAccess,
      CosemIso8802LlcType2SetupObject::MaxSupportedVersion)
{
}

CosemIso8802LlcType2SetupObject::CosemIso8802LlcType2SetupObject(
  const CosemLogicalName& logicalName,
  std::uint8_t transmitWindowSizeK,
  std::uint8_t receiveWindowSizeRw,
  std::uint16_t maxOctetsIPduN1,
  std::uint8_t maxNumberTransmissionsN2,
  std::uint16_t acknowledgementTimer,
  std::uint16_t pBitTimer,
  std::uint16_t rejectTimer,
  std::uint16_t busyStateTimer,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIso8802LlcType2SetupClassId,
      NormalizeVersion(
        version, CosemIso8802LlcType2SetupObject::MaxSupportedVersion),
      logicalName))
  , transmitWindowSizeK_(NormalizeWindowSize(transmitWindowSizeK))
  , receiveWindowSizeRw_(NormalizeWindowSize(receiveWindowSizeRw))
  , maxOctetsIPduN1_(maxOctetsIPduN1)
  , maxNumberTransmissionsN2_(maxNumberTransmissionsN2)
  , acknowledgementTimer_(acknowledgementTimer)
  , pBitTimer_(pBitTimer)
  , rejectTimer_(rejectTimer)
  , busyStateTimer_(busyStateTimer)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 9u; ++id)
    rights_.SetAttributeAccess(id, mutableAccess);
}

CosemObjectDescriptor CosemIso8802LlcType2SetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIso8802LlcType2SetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIso8802LlcType2SetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case 2u:
      output.clear();
      AppendUnsigned(output, transmitWindowSizeK_);
      return CosemStatus::Ok;
    case 3u:
      output.clear();
      AppendUnsigned(output, receiveWindowSizeRw_);
      return CosemStatus::Ok;
    case 4u:
      output.clear();
      AppendLongUnsigned(output, maxOctetsIPduN1_);
      return CosemStatus::Ok;
    case 5u:
      output.clear();
      AppendUnsigned(output, maxNumberTransmissionsN2_);
      return CosemStatus::Ok;
    case 6u:
      output.clear();
      AppendLongUnsigned(output, acknowledgementTimer_);
      return CosemStatus::Ok;
    case 7u:
      output.clear();
      AppendLongUnsigned(output, pBitTimer_);
      return CosemStatus::Ok;
    case 8u:
      output.clear();
      AppendLongUnsigned(output, rejectTimer_);
      return CosemStatus::Ok;
    case 9u:
      output.clear();
      AppendLongUnsigned(output, busyStateTimer_);
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIso8802LlcType2SetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId)
    return CosemStatus::AccessDenied;
  if (attributeId < 2u || attributeId > 9u)
    return CosemStatus::AttributeNotFound;
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  std::size_t offset = 0u;
  // attributes 2 (k), 3 (Rw), 5 (n2) are unsigned (0x11);
  // attributes 4 (n1), 6 (ack), 7 (P-bit), 8 (reject),
  // 9 (busy) are long-unsigned (0x12).
  if (attributeId == 2u || attributeId == 3u || attributeId == 5u) {
    std::uint8_t decoded = 0u;
    if (!ReadUnsignedValue(input, offset, decoded))
      return CosemStatus::InvalidArgument;
    if (offset != input.size())
      return CosemStatus::InvalidArgument;
    if ((attributeId == 2u || attributeId == 3u)
        && !IsValidWindowSize(decoded))
      return CosemStatus::InvalidArgument;
    switch (attributeId) {
      case 2u: transmitWindowSizeK_ = decoded; break;
      case 3u: receiveWindowSizeRw_ = decoded; break;
      case 5u: maxNumberTransmissionsN2_ = decoded; break;
    }
    return CosemStatus::Ok;
  }
  std::uint16_t decoded = 0u;
  if (!ReadLongUnsignedValue(input, offset, decoded))
    return CosemStatus::InvalidArgument;
  if (offset != input.size())
    return CosemStatus::InvalidArgument;
  switch (attributeId) {
    case 4u: maxOctetsIPduN1_ = decoded; break;
    case 6u: acknowledgementTimer_ = decoded; break;
    case 7u: pBitTimer_ = decoded; break;
    case 8u: rejectTimer_ = decoded; break;
    case 9u: busyStateTimer_ = decoded; break;
  }
  return CosemStatus::Ok;
}

CosemStatus CosemIso8802LlcType2SetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IEC 62056-6-2 ED4 (2021) §4.11.3 and DLMS UA Blue Book Ed. 12.1
  // §4.11.3 define no specific methods for IC 58 v0.
  output.clear();
  return CosemStatus::MethodNotFound;
}

std::uint8_t CosemIso8802LlcType2SetupObject::TransmitWindowSizeK() const
{
  return transmitWindowSizeK_;
}

std::uint8_t CosemIso8802LlcType2SetupObject::ReceiveWindowSizeRw() const
{
  return receiveWindowSizeRw_;
}

std::uint16_t CosemIso8802LlcType2SetupObject::MaxOctetsIPduN1() const
{
  return maxOctetsIPduN1_;
}

std::uint8_t
CosemIso8802LlcType2SetupObject::MaxNumberTransmissionsN2() const
{
  return maxNumberTransmissionsN2_;
}

std::uint16_t CosemIso8802LlcType2SetupObject::AcknowledgementTimer() const
{
  return acknowledgementTimer_;
}

std::uint16_t CosemIso8802LlcType2SetupObject::PBitTimer() const
{
  return pBitTimer_;
}

std::uint16_t CosemIso8802LlcType2SetupObject::RejectTimer() const
{
  return rejectTimer_;
}

std::uint16_t CosemIso8802LlcType2SetupObject::BusyStateTimer() const
{
  return busyStateTimer_;
}

namespace {
constexpr std::uint16_t kIso8802LlcType3SetupClassId = 59u;
} // namespace

const std::uint8_t CosemIso8802LlcType3SetupObject::MaxSupportedVersion;

CosemIso8802LlcType3SetupObject::CosemIso8802LlcType3SetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t maxOctetsAcnPduN3,
  std::uint8_t maxNumberTransmissionsN4,
  std::uint16_t acknowledgementTimeT1,
  std::uint16_t receiveLifetimeVarT2,
  std::uint16_t transmitLifetimeVarT3,
  AttributeAccessMode mutableAccess)
  : CosemIso8802LlcType3SetupObject(
      logicalName,
      maxOctetsAcnPduN3,
      maxNumberTransmissionsN4,
      acknowledgementTimeT1,
      receiveLifetimeVarT2,
      transmitLifetimeVarT3,
      mutableAccess,
      CosemIso8802LlcType3SetupObject::MaxSupportedVersion)
{
}

CosemIso8802LlcType3SetupObject::CosemIso8802LlcType3SetupObject(
  const CosemLogicalName& logicalName,
  std::uint16_t maxOctetsAcnPduN3,
  std::uint8_t maxNumberTransmissionsN4,
  std::uint16_t acknowledgementTimeT1,
  std::uint16_t receiveLifetimeVarT2,
  std::uint16_t transmitLifetimeVarT3,
  AttributeAccessMode mutableAccess,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kIso8802LlcType3SetupClassId,
      NormalizeVersion(
        version, CosemIso8802LlcType3SetupObject::MaxSupportedVersion),
      logicalName))
  , maxOctetsAcnPduN3_(maxOctetsAcnPduN3)
  , maxNumberTransmissionsN4_(maxNumberTransmissionsN4)
  , acknowledgementTimeT1_(acknowledgementTimeT1)
  , receiveLifetimeVarT2_(receiveLifetimeVarT2)
  , transmitLifetimeVarT3_(transmitLifetimeVarT3)
{
  rights_.SetAttributeAccess(
    kLogicalNameAttributeId, AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 6u; ++id)
    rights_.SetAttributeAccess(id, mutableAccess);
}

CosemObjectDescriptor CosemIso8802LlcType3SetupObject::Descriptor() const
{
  return descriptor_;
}

CosemAccessRights CosemIso8802LlcType3SetupObject::AccessRights() const
{
  return rights_;
}

CosemStatus CosemIso8802LlcType3SetupObject::ReadAttribute(
  std::uint8_t attributeId,
  CosemByteBuffer& output) const
{
  switch (attributeId) {
    case kLogicalNameAttributeId:
      output = EncodeLogicalName(descriptor_.key.logicalName);
      return CosemStatus::Ok;
    case 2u:
      output.clear();
      AppendLongUnsigned(output, maxOctetsAcnPduN3_);
      return CosemStatus::Ok;
    case 3u:
      output.clear();
      AppendUnsigned(output, maxNumberTransmissionsN4_);
      return CosemStatus::Ok;
    case 4u:
      output.clear();
      AppendLongUnsigned(output, acknowledgementTimeT1_);
      return CosemStatus::Ok;
    case 5u:
      output.clear();
      AppendLongUnsigned(output, receiveLifetimeVarT2_);
      return CosemStatus::Ok;
    case 6u:
      output.clear();
      AppendLongUnsigned(output, transmitLifetimeVarT3_);
      return CosemStatus::Ok;
    default:
      output.clear();
      return CosemStatus::AttributeNotFound;
  }
}

CosemStatus CosemIso8802LlcType3SetupObject::WriteAttribute(
  std::uint8_t attributeId,
  const CosemByteBuffer& input)
{
  if (attributeId == kLogicalNameAttributeId)
    return CosemStatus::AccessDenied;
  if (attributeId < 2u || attributeId > 6u)
    return CosemStatus::AttributeNotFound;
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;
  std::size_t offset = 0u;
  if (attributeId == 3u) {
    std::uint8_t decoded = 0u;
    if (!ReadUnsignedValue(input, offset, decoded))
      return CosemStatus::InvalidArgument;
    if (offset != input.size())
      return CosemStatus::InvalidArgument;
    maxNumberTransmissionsN4_ = decoded;
    return CosemStatus::Ok;
  }
  std::uint16_t decoded = 0u;
  if (!ReadLongUnsignedValue(input, offset, decoded))
    return CosemStatus::InvalidArgument;
  if (offset != input.size())
    return CosemStatus::InvalidArgument;
  switch (attributeId) {
    case 2u: maxOctetsAcnPduN3_ = decoded; break;
    case 4u: acknowledgementTimeT1_ = decoded; break;
    case 5u: receiveLifetimeVarT2_ = decoded; break;
    case 6u: transmitLifetimeVarT3_ = decoded; break;
  }
  return CosemStatus::Ok;
}

CosemStatus CosemIso8802LlcType3SetupObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)methodId;
  (void)input;
  // IEC 62056-6-2 ED4 (2021) §4.11.4 and DLMS UA Blue Book Ed. 12.1
  // §4.11.4 define no specific methods for IC 59 v0.
  output.clear();
  return CosemStatus::MethodNotFound;
}

std::uint16_t
CosemIso8802LlcType3SetupObject::MaxOctetsAcnPduN3() const
{
  return maxOctetsAcnPduN3_;
}

std::uint8_t
CosemIso8802LlcType3SetupObject::MaxNumberTransmissionsN4() const
{
  return maxNumberTransmissionsN4_;
}

std::uint16_t
CosemIso8802LlcType3SetupObject::AcknowledgementTimeT1() const
{
  return acknowledgementTimeT1_;
}

std::uint16_t
CosemIso8802LlcType3SetupObject::ReceiveLifetimeVarT2() const
{
  return receiveLifetimeVarT2_;
}

std::uint16_t
CosemIso8802LlcType3SetupObject::TransmitLifetimeVarT3() const
{
  return transmitLifetimeVarT3_;
}


const std::uint8_t CosemClockObject::MaxSupportedVersion;

CosemClockObject::CosemClockObject(
  const CosemLogicalName& logicalName,
  const types::DateTime& time,
  std::int16_t timeZone,
  std::uint8_t status,
  const types::DateTime& daylightSavingsBegin,
  const types::DateTime& daylightSavingsEnd,
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
  const types::DateTime& time,
  std::int16_t timeZone,
  std::uint8_t status,
  const types::DateTime& daylightSavingsBegin,
  const types::DateTime& daylightSavingsEnd,
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
    AppendDateTimeOctetString(output, time_);
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
    AppendDateTimeOctetString(output, daylightSavingsBegin_);
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsEndAttributeId) {
    AppendDateTimeOctetString(output, daylightSavingsEnd_);
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
    types::DateTime value;
    if (!DecodeDateTimeOctetString(input, value)) {
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
    types::DateTime value;
    if (!DecodeDateTimeOctetString(input, value)) {
      return CosemStatus::InvalidArgument;
    }
    daylightSavingsBegin_ = value;
    return CosemStatus::Ok;
  }
  if (attributeId == kClockDaylightSavingsEndAttributeId) {
    types::DateTime value;
    if (!DecodeDateTimeOctetString(input, value)) {
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

const types::DateTime& CosemClockObject::Time() const
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

const types::DateTime& CosemClockObject::DaylightSavingsBegin() const
{
  return daylightSavingsBegin_;
}

const types::DateTime& CosemClockObject::DaylightSavingsEnd() const
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

void CosemClockObject::SetTime(const types::DateTime& value)
{
  time_ = value;
}

void CosemClockObject::SetStatus(std::uint8_t value)
{
  status_ = value;
}

const std::uint8_t CosemProfileGenericObject::MaxSupportedVersion;

namespace {

CosemCaptureObject MakeEmptyProfileGenericSortObject()
{
  CosemCaptureObject empty;
  empty.object.classId = 0u;
  empty.object.version = 0u;
  empty.object.logicalName = CosemLogicalName(0u, 0u, 0u, 0u, 0u, 0u);
  empty.attributeId = 0u;
  empty.dataIndex = 0u;
  return empty;
}

}  // namespace

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
      CosemProfileGenericSortMethod::Fifo,
      MakeEmptyProfileGenericSortObject(),
      kProfileGenericVersion)
{
}

CosemProfileGenericObject::CosemProfileGenericObject(
  const CosemLogicalName& logicalName,
  const std::vector<CosemByteBuffer>& bufferRows,
  const std::vector<CosemCaptureObject>& captureObjects,
  std::uint32_t capturePeriod,
  std::uint32_t profileEntries,
  CosemProfileGenericSortMethod sortMethod,
  const CosemCaptureObject& sortObject)
  : CosemProfileGenericObject(
      logicalName,
      bufferRows,
      captureObjects,
      capturePeriod,
      profileEntries,
      sortMethod,
      sortObject,
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
  : CosemProfileGenericObject(
      logicalName,
      bufferRows,
      captureObjects,
      capturePeriod,
      profileEntries,
      CosemProfileGenericSortMethod::Fifo,
      MakeEmptyProfileGenericSortObject(),
      version)
{
}

CosemProfileGenericObject::CosemProfileGenericObject(
  const CosemLogicalName& logicalName,
  const std::vector<CosemByteBuffer>& bufferRows,
  const std::vector<CosemCaptureObject>& captureObjects,
  std::uint32_t capturePeriod,
  std::uint32_t profileEntries,
  CosemProfileGenericSortMethod sortMethod,
  const CosemCaptureObject& sortObject,
  std::uint8_t version)
  : descriptor_(MakeDescriptor(
      kProfileGenericClassId,
      NormalizeVersion(version, CosemProfileGenericObject::MaxSupportedVersion),
      logicalName))
  , bufferRows_(bufferRows)
  , captureObjects_(captureObjects)
  , capturePeriod_(capturePeriod)
  , profileEntries_(profileEntries)
  , sortMethod_(sortMethod)
  , sortObject_(sortObject)
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
  rights_.SetMethodAccess(
    kProfileGetBufferByRangeMethodId,
    MethodAccessMode::Access);
  rights_.SetMethodAccess(
    kProfileGetBufferByIndexMethodId,
    MethodAccessMode::Access);
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
    AppendEnum(output, static_cast<std::uint8_t>(sortMethod_));
    return CosemStatus::Ok;
  }
  if (attributeId == kProfileSortObjectAttributeId) {
    output = EncodeProfileGenericCaptureObject(sortObject_);
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
  if (methodId == kProfileResetMethodId) {
    // IEC 62056-6-2 ED4 4.3.6.3.1 / Blue Book Ed. 12.1 IC 7:
    // reset() clears the profile buffer. entries_in_use derives
    // from bufferRows_.size() and therefore returns to 0 on the
    // next read.
    bufferRows_.clear();
    return CosemStatus::Ok;
  }
  if (methodId == kProfileCaptureMethodId ||
      methodId == kProfileGetBufferByRangeMethodId ||
      methodId == kProfileGetBufferByIndexMethodId) {
    // capture() and get_buffer_by_{range,index}() are application
    // hooks: the built-in object has no notion of "now" or of the
    // captured objects' live values. Backends own this semantics.
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

CosemProfileGenericSortMethod CosemProfileGenericObject::SortMethod() const
{
  return sortMethod_;
}

const CosemCaptureObject& CosemProfileGenericObject::SortObject() const
{
  return sortObject_;
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
  if (descriptor_.key.version >= 3u) {
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
  if (descriptor_.key.version < 3u) {
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
      descriptor_.key.version >= 3u) {
    output.clear();
    AppendAssociationUserList(output, users_);
    return CosemStatus::Ok;
  }
  if (attributeId == kAssociationCurrentUserAttributeId &&
      descriptor_.key.version >= 3u) {
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
  if (descriptor_.key.version >= 3u &&
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
  : CosemSapAssignmentObject(
      logicalName,
      assignments,
      AttributeAccessMode::ReadOnly,
      kVersion0)
{
}

CosemSapAssignmentObject::CosemSapAssignmentObject(
  const CosemLogicalName& logicalName,
  const std::vector<SapAssignment>& assignments,
  AttributeAccessMode listAccess)
  : CosemSapAssignmentObject(
      logicalName,
      assignments,
      listAccess,
      kVersion0)
{
}

CosemSapAssignmentObject::CosemSapAssignmentObject(
  const CosemLogicalName& logicalName,
  const std::vector<SapAssignment>& assignments,
  std::uint8_t version)
  : CosemSapAssignmentObject(
      logicalName,
      assignments,
      AttributeAccessMode::ReadOnly,
      version)
{
}

CosemSapAssignmentObject::CosemSapAssignmentObject(
  const CosemLogicalName& logicalName,
  const std::vector<SapAssignment>& assignments,
  AttributeAccessMode listAccess,
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
  rights_.SetAttributeAccess(kValueAttributeId, listAccess);
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
  if (attributeId == kLogicalNameAttributeId)
    return CosemStatus::AccessDenied;
  if (attributeId != kValueAttributeId)
    return CosemStatus::AttributeNotFound;
  if (!IsAccessWritable(rights_.AttributeAccess(attributeId)))
    return CosemStatus::AccessDenied;

  // sap_assignment_list ::= array of structure {
  //     long-unsigned sap,
  //     octet-string  logical_device_name }
  // per IEC 62056-6-2 ED4 (2021) §4.4.5.2.2 / DLMS UA Blue Book
  // Ed. 12.1 §4.4.5.2.2.
  std::size_t offset = 0u;
  if (!ReadExpectedTag(input, offset, kArrayTag))
    return CosemStatus::InvalidArgument;
  std::size_t count = 0u;
  if (!ReadAxdrLength(input, offset, count))
    return CosemStatus::InvalidArgument;

  std::vector<SapAssignment> decoded;
  decoded.reserve(count);
  for (std::size_t i = 0u; i < count; ++i) {
    if (!ReadExpectedTag(input, offset, kStructureTag))
      return CosemStatus::InvalidArgument;
    std::size_t fieldCount = 0u;
    if (!ReadAxdrLength(input, offset, fieldCount) || fieldCount != 2u)
      return CosemStatus::InvalidArgument;

    std::uint16_t sap = 0u;
    if (!ReadLongUnsignedValue(input, offset, sap))
      return CosemStatus::InvalidArgument;

    if (!ReadExpectedTag(input, offset, kDataOctetStringTag))
      return CosemStatus::InvalidArgument;
    std::size_t ldnLen = 0u;
    if (!ReadAxdrLength(input, offset, ldnLen))
      return CosemStatus::InvalidArgument;
    if (offset + ldnLen > input.size())
      return CosemStatus::InvalidArgument;
    SapAssignment entry;
    entry.sap = sap;
    entry.logicalDeviceName.assign(
      reinterpret_cast<const char*>(input.data() + offset), ldnLen);
    offset += ldnLen;
    decoded.push_back(entry);
  }
  if (offset != input.size())
    return CosemStatus::InvalidArgument;

  assignments_.swap(decoded);
  return CosemStatus::Ok;
}

void CosemSapAssignmentObject::SetAssignments(
  const std::vector<SapAssignment>& assignments)
{
  assignments_ = assignments;
}

CosemStatus CosemSapAssignmentObject::InvokeMethod(
  std::uint8_t methodId,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  (void)input;
  output.clear();
  if (methodId == kSapAssignmentConnectLogicalDeviceMethodId) {
    // method 1 (connect_logical_device) attaches or detaches a logical
    // device to/from a SAP. The built-in object does not own that
    // dispatch policy and surfaces the spec method explicitly as
    // UnsupportedFeature instead of silently returning MethodNotFound.
    return CosemStatus::UnsupportedFeature;
  }
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
