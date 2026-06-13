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
constexpr std::uint8_t kSecurityPolicyAttributeId = 2u;
constexpr std::uint8_t kSecuritySuiteAttributeId = 3u;
constexpr std::uint8_t kClientSystemTitleAttributeId = 4u;
constexpr std::uint8_t kServerSystemTitleAttributeId = 5u;
constexpr std::uint8_t kSecurityActivateMethodId = 1u;
constexpr std::uint8_t kGlobalKeyTransferMethodId = 2u;
constexpr std::uint8_t kVersion0 = 0u;
constexpr std::uint8_t kProfileGenericVersion = 1u;
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
  : descriptor_(MakeDescriptor(kClockClassId, logicalName))
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

CosemProfileGenericObject::CosemProfileGenericObject(
  const CosemLogicalName& logicalName,
  const std::vector<CosemByteBuffer>& bufferRows,
  const std::vector<CosemCaptureObject>& captureObjects,
  std::uint32_t capturePeriod,
  std::uint32_t profileEntries)
  : descriptor_(MakeDescriptor(
      kProfileGenericClassId,
      kProfileGenericVersion,
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
    output = EncodeAssociationObjectList(objectList_);
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
  (void)methodId;
  (void)input;
  output.clear();
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
  if (methodId > kSecurityActivateMethodId && methodId <= 8u) {
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
