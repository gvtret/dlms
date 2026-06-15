import pathlib

p = pathlib.Path(r'E:\work\dlms\lib\dlms-cosem\test\cosem\test_simple_objects.cpp')
src = p.read_text()

old_start = '''struct SensorManagerBuffers
{
  dlms::cosem::CosemByteBuffer status;
  dlms::cosem::CosemByteBuffer serialNumber;
  dlms::cosem::CosemByteBuffer deviceType;
  dlms::cosem::CosemByteBuffer manufacturerId;
  dlms::cosem::CosemByteBuffer firmwareVersion;
  dlms::cosem::CosemByteBuffer metrologyFirmwareVersion;
  dlms::cosem::CosemByteBuffer driver;
  dlms::cosem::CosemByteBuffer communicationDesc;
  dlms::cosem::CosemByteBuffer setupDesc;
  dlms::cosem::CosemByteBuffer measurementDesc;
};'''

old_end = '''TEST(CosemSensorManagerObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 96u, 50u, 0u, 255u);
  const SensorManagerBuffers b = MakeSampleSensorManager();
  dlms::cosem::CosemSensorManagerObject object(
    name, b.status, b.serialNumber, b.deviceType, b.manufacturerId,
    b.firmwareVersion, b.metrologyFirmwareVersion, b.driver,
    b.communicationDesc, b.setupDesc, b.measurementDesc,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSensorManagerObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}'''

i = src.find(old_start)
j = src.find(old_end)
assert i != -1, 'IC67 tests start not found'
assert j != -1, 'IC67 tests end not found'
j += len(old_end)

new_block = '''struct SensorManagerBuffers
{
  dlms::cosem::CosemByteBuffer serialNumber;
  dlms::cosem::CosemByteBuffer metrologicalIdentification;
  dlms::cosem::CosemByteBuffer outputType;
  dlms::cosem::CosemByteBuffer adjustmentMethod;
  dlms::cosem::CosemByteBuffer sealingMethod;
  dlms::cosem::CosemByteBuffer rawValue;
  dlms::cosem::CosemByteBuffer scalerUnit;
  dlms::cosem::CosemByteBuffer status;
  dlms::cosem::CosemByteBuffer captureTime;
  dlms::cosem::CosemByteBuffer rawValueThresholds;
  dlms::cosem::CosemByteBuffer rawValueActions;
  dlms::cosem::CosemByteBuffer processedValue;
  dlms::cosem::CosemByteBuffer processedValueThresholds;
  dlms::cosem::CosemByteBuffer processedValueActions;
};

SensorManagerBuffers MakeSampleSensorManager()
{
  SensorManagerBuffers b;
  // octet-string(8) serial number
  b.serialNumber = BytesFromList({
    0x09u, 0x08u,
      0x53u, 0x4Eu, 0x30u, 0x30u, 0x30u, 0x30u, 0x30u, 0x31u});
  // octet-string(4) metrological id 'MID1'
  b.metrologicalIdentification = BytesFromList({
    0x09u, 0x04u, 0x4Du, 0x49u, 0x44u, 0x31u});
  // enum 1 (placeholder output_type)
  b.outputType = BytesFromList({0x16u, 0x01u});
  // octet-string(4) adjustment method 'ADJ1'
  b.adjustmentMethod = BytesFromList({
    0x09u, 0x04u, 0x41u, 0x44u, 0x4Au, 0x31u});
  // enum 1 (placeholder sealing_method)
  b.sealingMethod = BytesFromList({0x16u, 0x01u});
  // double-long-unsigned 0x12345678 raw_value
  b.rawValue = BytesFromList({
    0x06u, 0x12u, 0x34u, 0x56u, 0x78u});
  // structure(2) scaler_unit { integer 0, enum 30 (V) }
  b.scalerUnit = BytesFromList({
    0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu});
  // bit-string(8) 0x00 status
  b.status = BytesFromList({0x04u, 0x08u, 0x00u});
  // date-time octet-string(12) placeholder capture_time
  b.captureTime = BytesFromList({
    0x09u, 0x0Cu,
      0x07u, 0xE5u, 0x01u, 0x02u, 0x03u,
      0x04u, 0x05u, 0x06u,
      0x00u, 0x00u, 0x00u, 0x00u});
  // array(0) raw_value_thresholds
  b.rawValueThresholds = BytesFromList({0x01u, 0x00u});
  // array(0) raw_value_actions
  b.rawValueActions = BytesFromList({0x01u, 0x00u});
  // structure(0) processed_value
  b.processedValue = BytesFromList({0x02u, 0x00u});
  // array(0) processed_value_thresholds
  b.processedValueThresholds = BytesFromList({0x01u, 0x00u});
  // array(0) processed_value_actions
  b.processedValueActions = BytesFromList({0x01u, 0x00u});
  return b;
}

dlms::cosem::CosemSensorManagerObject MakeSensorManagerObject(
  const dlms::cosem::CosemLogicalName& name,
  const SensorManagerBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemSensorManagerObject(
    name, b.serialNumber, b.metrologicalIdentification,
    b.outputType, b.adjustmentMethod, b.sealingMethod,
    b.rawValue, b.scalerUnit, b.status, b.captureTime,
    b.rawValueThresholds, b.rawValueActions, b.processedValue,
    b.processedValueThresholds, b.processedValueActions, access);
}

} // namespace

TEST(CosemSensorManagerObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 96u, 50u, 0u, 255u);
  const SensorManagerBuffers b = MakeSampleSensorManager();
  dlms::cosem::CosemSensorManagerObject object =
    MakeSensorManagerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(67u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSensorManagerObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.serialNumber, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.metrologicalIdentification, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.outputType, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.adjustmentMethod, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.sealingMethod, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.rawValue, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.scalerUnit, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(10u, out));
  EXPECT_EQ(b.captureTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(11u, out));
  EXPECT_EQ(b.rawValueThresholds, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(12u, out));
  EXPECT_EQ(b.rawValueActions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(13u, out));
  EXPECT_EQ(b.processedValue, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(14u, out));
  EXPECT_EQ(b.processedValueThresholds, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(15u, out));
  EXPECT_EQ(b.processedValueActions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(16u, out));
}

TEST(CosemSensorManagerObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 96u, 50u, 0u, 255u);
  const SensorManagerBuffers b = MakeSampleSensorManager();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemSensorManagerObject writable =
    MakeSensorManagerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id :
       {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.SerialNumber());
  EXPECT_EQ(replacement, writable.MetrologicalIdentification());
  EXPECT_EQ(replacement, writable.OutputType());
  EXPECT_EQ(replacement, writable.AdjustmentMethod());
  EXPECT_EQ(replacement, writable.SealingMethod());
  EXPECT_EQ(replacement, writable.RawValue());
  EXPECT_EQ(replacement, writable.ScalerUnit());
  EXPECT_EQ(replacement, writable.Status());
  EXPECT_EQ(replacement, writable.CaptureTime());
  EXPECT_EQ(replacement, writable.RawValueThresholds());
  EXPECT_EQ(replacement, writable.RawValueActions());
  EXPECT_EQ(replacement, writable.ProcessedValue());
  EXPECT_EQ(replacement, writable.ProcessedValueThresholds());
  EXPECT_EQ(replacement, writable.ProcessedValueActions());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemSensorManagerObject readOnly =
    MakeSensorManagerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id :
       {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.serialNumber, readOnly.SerialNumber());
  EXPECT_EQ(b.metrologicalIdentification,
            readOnly.MetrologicalIdentification());
  EXPECT_EQ(b.outputType, readOnly.OutputType());
  EXPECT_EQ(b.adjustmentMethod, readOnly.AdjustmentMethod());
  EXPECT_EQ(b.sealingMethod, readOnly.SealingMethod());
  EXPECT_EQ(b.rawValue, readOnly.RawValue());
  EXPECT_EQ(b.scalerUnit, readOnly.ScalerUnit());
  EXPECT_EQ(b.status, readOnly.Status());
  EXPECT_EQ(b.captureTime, readOnly.CaptureTime());
  EXPECT_EQ(b.rawValueThresholds, readOnly.RawValueThresholds());
  EXPECT_EQ(b.rawValueActions, readOnly.RawValueActions());
  EXPECT_EQ(b.processedValue, readOnly.ProcessedValue());
  EXPECT_EQ(b.processedValueThresholds,
            readOnly.ProcessedValueThresholds());
  EXPECT_EQ(b.processedValueActions,
            readOnly.ProcessedValueActions());
}

TEST(CosemSensorManagerObject, ResetMethodIsUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 96u, 50u, 0u, 255u);
  const SensorManagerBuffers b = MakeSampleSensorManager();
  dlms::cosem::CosemSensorManagerObject object =
    MakeSensorManagerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  for (std::uint8_t method : {2u, 3u, 99u}) {
    out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSensorManagerObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 96u, 50u, 0u, 255u);
  const SensorManagerBuffers b = MakeSampleSensorManager();
  dlms::cosem::CosemSensorManagerObject object(
    name, b.serialNumber, b.metrologicalIdentification,
    b.outputType, b.adjustmentMethod, b.sealingMethod,
    b.rawValue, b.scalerUnit, b.status, b.captureTime,
    b.rawValueThresholds, b.rawValueActions, b.processedValue,
    b.processedValueThresholds, b.processedValueActions,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSensorManagerObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}'''

src = src[:i] + new_block + src[j:]
p.write_text(src)
print('OK')
