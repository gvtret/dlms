import pathlib

p = pathlib.Path(r'E:\work\dlms\lib\dlms-cosem\test\cosem\test_simple_objects.cpp')
src = p.read_text()

old = '''struct ParameterMonitorBuffers
{
  dlms::cosem::CosemByteBuffer changedParameter;
  dlms::cosem::CosemByteBuffer captureTime;
  dlms::cosem::CosemByteBuffer parameters;
};

ParameterMonitorBuffers MakeSampleParameterMonitor()
{
  ParameterMonitorBuffers b;'''
new = '''struct ParameterMonitorBuffers
{
  dlms::cosem::CosemByteBuffer changedParameter;
  dlms::cosem::CosemByteBuffer captureTime;
  dlms::cosem::CosemByteBuffer parameters;
  dlms::cosem::CosemByteBuffer parameterListName;
  dlms::cosem::CosemByteBuffer hashAlgorithmId;
  dlms::cosem::CosemByteBuffer parameterValueDigest;
  dlms::cosem::CosemByteBuffer parameterValues;
};

ParameterMonitorBuffers MakeSampleParameterMonitor()
{
  ParameterMonitorBuffers b;'''
assert old in src, 'PM buffers block not found'
src = src.replace(old, new, 1)

old2 = '''        0x0Fu, 0x02u});
  return b;
}

dlms::cosem::CosemParameterMonitorObject MakeParameterMonitorObject(
  const dlms::cosem::CosemLogicalName& name,
  const ParameterMonitorBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemParameterMonitorObject(
    name, b.changedParameter, b.captureTime, b.parameters, access);
}'''
new2 = '''        0x0Fu, 0x02u});
  // octet-string "main"
  b.parameterListName = BytesFromList({0x09u, 0x04u,
    0x6Du, 0x61u, 0x69u, 0x6Eu});
  // enum 1 (placeholder hash algorithm id)
  b.hashAlgorithmId = BytesFromList({0x16u, 0x01u});
  // octet-string(4) placeholder digest
  b.parameterValueDigest = BytesFromList({0x09u, 0x04u,
    0xDEu, 0xADu, 0xBEu, 0xEFu});
  // structure(0) placeholder for parameter_values
  b.parameterValues = BytesFromList({0x02u, 0x00u});
  return b;
}

dlms::cosem::CosemParameterMonitorObject MakeParameterMonitorObject(
  const dlms::cosem::CosemLogicalName& name,
  const ParameterMonitorBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemParameterMonitorObject(
    name, b.changedParameter, b.captureTime, b.parameters,
    b.parameterListName, b.hashAlgorithmId,
    b.parameterValueDigest, b.parameterValues, access);
}'''
assert old2 in src, 'PM factory block not found'
src = src.replace(old2, new2, 1)

old3 = '''  EXPECT_EQ(65u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemParameterMonitorObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.changedParameter, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.captureTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.parameters, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}'''
new3 = '''  EXPECT_EQ(65u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemParameterMonitorObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.changedParameter, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.captureTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.parameters, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.parameterListName, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.hashAlgorithmId, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.parameterValueDigest, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.parameterValues, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(9u, out));
}'''
assert old3 in src, 'PM ExposesAllAttributes block not found'
src = src.replace(old3, new3, 1)

old4 = '''  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.ChangedParameter());
  EXPECT_EQ(replacement, writable.CaptureTime());
  EXPECT_EQ(replacement, writable.Parameters());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemParameterMonitorObject readOnly =
    MakeParameterMonitorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.changedParameter, readOnly.ChangedParameter());
  EXPECT_EQ(b.captureTime, readOnly.CaptureTime());
  EXPECT_EQ(b.parameters, readOnly.Parameters());
}'''
new4 = '''  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.ChangedParameter());
  EXPECT_EQ(replacement, writable.CaptureTime());
  EXPECT_EQ(replacement, writable.Parameters());
  EXPECT_EQ(replacement, writable.ParameterListName());
  EXPECT_EQ(replacement, writable.HashAlgorithmId());
  EXPECT_EQ(replacement, writable.ParameterValueDigest());
  EXPECT_EQ(replacement, writable.ParameterValues());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemParameterMonitorObject readOnly =
    MakeParameterMonitorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.changedParameter, readOnly.ChangedParameter());
  EXPECT_EQ(b.captureTime, readOnly.CaptureTime());
  EXPECT_EQ(b.parameters, readOnly.Parameters());
  EXPECT_EQ(b.parameterListName, readOnly.ParameterListName());
  EXPECT_EQ(b.hashAlgorithmId, readOnly.HashAlgorithmId());
  EXPECT_EQ(b.parameterValueDigest, readOnly.ParameterValueDigest());
  EXPECT_EQ(b.parameterValues, readOnly.ParameterValues());
}

TEST(CosemParameterMonitorObject, LegacyVersion0RejectsExtendedAttrs)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 2u, 0u, 255u);
  const ParameterMonitorBuffers b = MakeSampleParameterMonitor();
  dlms::cosem::CosemParameterMonitorObject legacy(
    name, b.changedParameter, b.captureTime, b.parameters,
    b.parameterListName, b.hashAlgorithmId,
    b.parameterValueDigest, b.parameterValues,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    static_cast<std::uint8_t>(0u));
  EXPECT_EQ(0u, legacy.Descriptor().key.version);
  EXPECT_TRUE(legacy.ParameterListName().empty());
  EXPECT_TRUE(legacy.HashAlgorithmId().empty());
  EXPECT_TRUE(legacy.ParameterValueDigest().empty());
  EXPECT_TRUE(legacy.ParameterValues().empty());
  dlms::cosem::CosemByteBuffer out;
  for (std::uint8_t id : {5u, 6u, 7u, 8u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
              legacy.ReadAttribute(
                static_cast<std::uint8_t>(id), out));
    EXPECT_TRUE(out.empty());
    EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
              legacy.WriteAttribute(
                static_cast<std::uint8_t>(id),
                BytesFromList({0x11u, 0x2Au})));
  }
}'''
assert old4 in src, 'PM Mutable test block not found'
src = src.replace(old4, new4, 1)

old5 = '''  dlms::cosem::CosemParameterMonitorObject object(
    name, b.changedParameter, b.captureTime, b.parameters,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);'''
new5 = '''  dlms::cosem::CosemParameterMonitorObject object(
    name, b.changedParameter, b.captureTime, b.parameters,
    b.parameterListName, b.hashAlgorithmId,
    b.parameterValueDigest, b.parameterValues,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);'''
assert old5 in src, 'PM NormalizesVersionAboveMax ctor not found'
src = src.replace(old5, new5, 1)

p.write_text(src)
print('OK')
