import pathlib

p = pathlib.Path(r'E:\work\dlms\lib\dlms-cosem\src\cosem\simple_objects.cpp')
src = p.read_text()

old_start = '''namespace {
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
} // namespace'''

old_end = '''const CosemByteBuffer&
CosemSensorManagerObject::MeasurementDesc() const
{
  return measurementDesc_;
}'''

i = src.find(old_start)
j = src.find(old_end)
assert i != -1, 'IC67 start not found'
assert j != -1, 'IC67 end not found'
j += len(old_end)
prefix, suffix = src[:i], src[j:]

new_block = r'''namespace {
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
}'''

src = prefix + new_block + suffix
p.write_text(src)
print('OK')
