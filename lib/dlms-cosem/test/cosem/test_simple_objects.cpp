#include "dlms/cosem/cosem.hpp"
#include "dlms/security/in_memory_invocation_counter_store.hpp"
#include "dlms/security/in_memory_key_store.hpp"
#include "dlms/security/suite0_key_wrap.hpp"

#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <string>

namespace {

dlms::cosem::CosemLogicalName MakeName(std::uint8_t c)
{
  return dlms::cosem::CosemLogicalName(1u, 0u, c, 8u, 0u, 255u);
}

dlms::cosem::CosemByteBuffer Bytes(
  std::uint8_t first,
  std::uint8_t second)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(first);
  bytes.push_back(second);
  return bytes;
}

dlms::security::SecurityByteView SecurityView(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::security::SecurityByteView view;
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

dlms::security::SecurityKey MakeSecurityKey(
  dlms::security::SecurityKeyRole role,
  const std::uint8_t* bytes,
  std::size_t size)
{
  dlms::security::SecurityKey key = dlms::security::EmptySecurityKey(role);
  key.size = size;
  for (std::size_t i = 0u; i < size; ++i) {
    key.bytes[i] = bytes[i];
  }
  return key;
}

class FailingCounterResetPolicy
  : public dlms::security::IInvocationCounterResetPolicy
{
public:
  FailingCounterResetPolicy()
    : calls_(0u)
  {
  }

  dlms::security::SecurityStatus ResetAfterKeyRotation(
    dlms::security::SecurityKeyRole role)
  {
    (void)role;
    ++calls_;
    return dlms::security::SecurityStatus::AuthenticationFailed;
  }

  std::size_t Calls() const
  {
    return calls_;
  }

private:
  std::size_t calls_;
};

dlms::cosem::CosemByteBuffer EncodedLogicalName(
  const dlms::cosem::CosemLogicalName& name)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x09u);
  bytes.push_back(0x06u);
  for (std::size_t i = 0; i < name.Size(); ++i) {
    bytes.push_back(name[i]);
  }
  return bytes;
}

void AppendLongUnsigned(
  dlms::cosem::CosemByteBuffer& bytes,
  std::uint16_t value)
{
  bytes.push_back(0x12u);
  bytes.push_back(static_cast<std::uint8_t>(value >> 8u));
  bytes.push_back(static_cast<std::uint8_t>(value & 0xffu));
}

void AppendInteger(
  dlms::cosem::CosemByteBuffer& bytes,
  std::uint8_t value)
{
  bytes.push_back(0x0Fu);
  bytes.push_back(value);
}

void AppendDoubleLongUnsigned(
  dlms::cosem::CosemByteBuffer& bytes,
  std::uint32_t value)
{
  bytes.push_back(0x06u);
  bytes.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>(value & 0xffu));
}

void AppendEnum(
  dlms::cosem::CosemByteBuffer& bytes,
  std::uint8_t value)
{
  bytes.push_back(0x16u);
  bytes.push_back(value);
}

void AppendOctetString(
  dlms::cosem::CosemByteBuffer& bytes,
  const dlms::cosem::CosemLogicalName& name)
{
  bytes.push_back(0x09u);
  bytes.push_back(0x06u);
  for (std::size_t i = 0; i < name.Size(); ++i) {
    bytes.push_back(name[i]);
  }
}

dlms::cosem::CosemByteBuffer EncodedCaptureObject(
  const dlms::cosem::CosemCaptureObject& object)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x02u);
  bytes.push_back(0x04u);
  AppendLongUnsigned(bytes, object.object.classId);
  AppendOctetString(bytes, object.object.logicalName);
  AppendInteger(bytes, object.attributeId);
  AppendLongUnsigned(bytes, object.dataIndex);
  return bytes;
}

dlms::cosem::CosemByteBuffer EncodedDateTime(std::uint8_t seed)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x19u);
  for (std::uint8_t i = 0u; i < 12u; ++i) {
    bytes.push_back(static_cast<std::uint8_t>(seed + i));
  }
  return bytes;
}

dlms::cosem::CosemByteBuffer EncodedOctetString(
  const std::string& value)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x09u);
  bytes.push_back(static_cast<std::uint8_t>(value.size()));
  for (std::size_t i = 0; i < value.size(); ++i) {
    bytes.push_back(static_cast<std::uint8_t>(value[i]));
  }
  return bytes;
}

dlms::cosem::CosemByteBuffer EncodedOctetString(
  const dlms::cosem::CosemSecuritySetupObject::SystemTitle& value)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x09u);
  bytes.push_back(static_cast<std::uint8_t>(value.size()));
  for (std::size_t i = 0; i < value.size(); ++i) {
    bytes.push_back(value[i]);
  }
  return bytes;
}

dlms::cosem::CosemByteBuffer EncodedGlobalKeyTransfer(
  std::uint8_t keyId,
  const std::vector<std::uint8_t>& wrapped)
{
  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(keyId);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());
  return input;
}

dlms::cosem::CosemAttributeDescriptor MakeAttribute(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t attributeId)
{
  dlms::cosem::CosemAttributeDescriptor descriptor;
  descriptor.object = key;
  descriptor.attributeId = attributeId;
  return descriptor;
}

dlms::cosem::CosemMethodDescriptor MakeMethod(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t methodId)
{
  dlms::cosem::CosemMethodDescriptor descriptor;
  descriptor.object = key;
  descriptor.methodId = methodId;
  return descriptor;
}

} // namespace

TEST(CosemDataObject, ExposesDescriptorAndAttributes)
{
  const dlms::cosem::CosemLogicalName name = MakeName(1u);
  const dlms::cosem::CosemByteBuffer value = Bytes(0x12u, 0x34u);
  dlms::cosem::CosemDataObject object(
    name,
    value,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(1u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(name, descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(name), output);

  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(value, output);
}

TEST(CosemDataObject, WritesValueAndRejectsUnsupportedMembers)
{
  dlms::cosem::CosemDataObject object(
    MakeName(2u),
    Bytes(0x01u, 0x02u),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer updated = Bytes(0x03u, 0x04u);
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, updated));
  EXPECT_EQ(updated, object.Value());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, updated));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, updated));

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(1u, updated, output));
  EXPECT_TRUE(output.empty());
}

TEST(CosemRegisterObject, ExposesDescriptorValueAndScalerUnit)
{
  const dlms::cosem::CosemLogicalName name = MakeName(3u);
  const dlms::cosem::CosemByteBuffer value = Bytes(0x06u, 0x01u);
  const dlms::cosem::CosemByteBuffer scaler = Bytes(0x02u, 0x03u);
  dlms::cosem::CosemRegisterObject object(
    name,
    value,
    scaler,
    dlms::cosem::AttributeAccessMode::ReadOnly);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(3u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(name, descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(name), output);

  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(value, output);

  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, output));
  EXPECT_EQ(scaler, output);
}

TEST(CosemRegisterObject, WritesValueAndRejectsUnsupportedMembers)
{
  dlms::cosem::CosemRegisterObject object(
    MakeName(4u),
    Bytes(0x01u, 0x02u),
    Bytes(0x03u, 0x04u),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer updated = Bytes(0x05u, 0x06u);
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, updated));
  EXPECT_EQ(updated, object.Value());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, updated));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(3u, updated));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, updated));

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(1u, updated, output));
  EXPECT_TRUE(output.empty());
}

TEST(CosemProfileGenericObject, ExposesReadOnlyProfileAttributes)
{
  const dlms::cosem::CosemLogicalName name = MakeName(7u);

  dlms::cosem::CosemByteBuffer row;
  row.push_back(0x02u);
  row.push_back(0x01u);
  AppendDoubleLongUnsigned(row, 42u);
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  rows.push_back(row);

  dlms::cosem::CosemCaptureObject capture;
  capture.object.classId = 3u;
  capture.object.version = 0u;
  capture.object.logicalName = MakeName(3u);
  capture.attributeId = 2u;
  capture.dataIndex = 0u;
  std::vector<dlms::cosem::CosemCaptureObject> captures;
  captures.push_back(capture);

  dlms::cosem::CosemProfileGenericObject object(
    name,
    rows,
    captures,
    60u,
    100u);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(7u, descriptor.key.classId);
  EXPECT_EQ(1u, descriptor.key.version);
  EXPECT_EQ(name, descriptor.key.logicalName);
  EXPECT_EQ(rows, object.BufferRows());
  ASSERT_EQ(1u, object.CaptureObjects().size());
  EXPECT_EQ(3u, object.CaptureObjects()[0].object.classId);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(name), output);

  dlms::cosem::CosemByteBuffer expectedBuffer;
  expectedBuffer.push_back(0x01u);
  expectedBuffer.push_back(0x01u);
  expectedBuffer.insert(expectedBuffer.end(), row.begin(), row.end());
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(expectedBuffer, output);

  dlms::cosem::CosemByteBuffer expectedCaptureObjects;
  expectedCaptureObjects.push_back(0x01u);
  expectedCaptureObjects.push_back(0x01u);
  const dlms::cosem::CosemByteBuffer encodedCapture =
    EncodedCaptureObject(capture);
  expectedCaptureObjects.insert(
    expectedCaptureObjects.end(),
    encodedCapture.begin(),
    encodedCapture.end());
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, output));
  EXPECT_EQ(expectedCaptureObjects, output);

  dlms::cosem::CosemByteBuffer expectedCapturePeriod;
  AppendDoubleLongUnsigned(expectedCapturePeriod, 60u);
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, output));
  EXPECT_EQ(expectedCapturePeriod, output);

  dlms::cosem::CosemByteBuffer expectedSortMethod;
  AppendEnum(expectedSortMethod, 1u);
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, output));
  EXPECT_EQ(expectedSortMethod, output);

  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, output));
  EXPECT_EQ(encodedCapture, output);

  dlms::cosem::CosemByteBuffer expectedEntriesInUse;
  AppendDoubleLongUnsigned(expectedEntriesInUse, 1u);
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(7u, output));
  EXPECT_EQ(expectedEntriesInUse, output);

  dlms::cosem::CosemByteBuffer expectedProfileEntries;
  AppendDoubleLongUnsigned(expectedProfileEntries, 100u);
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(8u, output));
  EXPECT_EQ(expectedProfileEntries, output);
}

TEST(CosemProfileGenericObject, EncodesAndDecodesCaptureObject)
{
  dlms::cosem::CosemCaptureObject capture;
  capture.object.classId = 3u;
  capture.object.version = 0u;
  capture.object.logicalName = MakeName(3u);
  capture.attributeId = 2u;
  capture.dataIndex = 1u;

  const dlms::cosem::CosemByteBuffer encoded =
    dlms::cosem::EncodeProfileGenericCaptureObject(capture);
  EXPECT_EQ(EncodedCaptureObject(capture), encoded);

  dlms::cosem::CosemCaptureObject decoded;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            dlms::cosem::DecodeProfileGenericCaptureObject(
              encoded,
              decoded));
  EXPECT_EQ(capture.object.classId, decoded.object.classId);
  EXPECT_EQ(0u, decoded.object.version);
  EXPECT_EQ(capture.object.logicalName, decoded.object.logicalName);
  EXPECT_EQ(capture.attributeId, decoded.attributeId);
  EXPECT_EQ(capture.dataIndex, decoded.dataIndex);
}

TEST(CosemProfileGenericObject, EncodesAndDecodesCaptureObjects)
{
  dlms::cosem::CosemCaptureObject first;
  first.object.classId = 3u;
  first.object.version = 0u;
  first.object.logicalName = MakeName(3u);
  first.attributeId = 2u;
  first.dataIndex = 0u;

  dlms::cosem::CosemCaptureObject second;
  second.object.classId = 8u;
  second.object.version = 0u;
  second.object.logicalName = MakeName(8u);
  second.attributeId = 2u;
  second.dataIndex = 0u;

  std::vector<dlms::cosem::CosemCaptureObject> captures;
  captures.push_back(first);
  captures.push_back(second);

  const dlms::cosem::CosemByteBuffer encoded =
    dlms::cosem::EncodeProfileGenericCaptureObjects(captures);
  std::vector<dlms::cosem::CosemCaptureObject> decoded;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            dlms::cosem::DecodeProfileGenericCaptureObjects(
              encoded,
              decoded));

  ASSERT_EQ(2u, decoded.size());
  EXPECT_EQ(first.object.classId, decoded[0].object.classId);
  EXPECT_EQ(first.object.logicalName, decoded[0].object.logicalName);
  EXPECT_EQ(first.attributeId, decoded[0].attributeId);
  EXPECT_EQ(first.dataIndex, decoded[0].dataIndex);
  EXPECT_EQ(second.object.classId, decoded[1].object.classId);
  EXPECT_EQ(second.object.logicalName, decoded[1].object.logicalName);
  EXPECT_EQ(second.attributeId, decoded[1].attributeId);
  EXPECT_EQ(second.dataIndex, decoded[1].dataIndex);
}

TEST(CosemProfileGenericObject, RejectsMalformedCaptureObjects)
{
  std::vector<dlms::cosem::CosemCaptureObject> decoded;
  decoded.resize(1u);

  dlms::cosem::CosemByteBuffer malformed;
  malformed.push_back(0x01u);
  malformed.push_back(0x01u);
  malformed.push_back(0x02u);
  malformed.push_back(0x03u);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::DecodeProfileGenericCaptureObjects(
              malformed,
              decoded));
  EXPECT_EQ(1u, decoded.size());
}

TEST(CosemProfileGenericObject, EncodesAndDecodesBufferRows)
{
  dlms::cosem::CosemByteBuffer firstRow;
  firstRow.push_back(0x02u);
  firstRow.push_back(0x02u);
  AppendDoubleLongUnsigned(firstRow, 42u);
  AppendEnum(firstRow, 7u);

  dlms::cosem::CosemByteBuffer secondRow;
  secondRow.push_back(0x02u);
  secondRow.push_back(0x01u);
  secondRow.push_back(0x09u);
  secondRow.push_back(0x03u);
  secondRow.push_back(0x41u);
  secondRow.push_back(0x42u);
  secondRow.push_back(0x43u);

  std::vector<dlms::cosem::CosemByteBuffer> rows;
  rows.push_back(firstRow);
  rows.push_back(secondRow);

  const dlms::cosem::CosemByteBuffer encoded =
    dlms::cosem::EncodeProfileGenericBuffer(rows);
  std::vector<dlms::cosem::CosemByteBuffer> decoded;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            dlms::cosem::DecodeProfileGenericBuffer(encoded, decoded));
  EXPECT_EQ(rows, decoded);
}

TEST(CosemProfileGenericObject, ExposesSelectiveAccessSelectors)
{
  EXPECT_EQ(1u, dlms::cosem::ProfileGenericRangeAccessSelector());
  EXPECT_EQ(2u, dlms::cosem::ProfileGenericEntryAccessSelector());
}

TEST(CosemProfileGenericObject, EncodesAndDecodesRangeDescriptor)
{
  dlms::cosem::CosemCaptureObject restrictingObject;
  restrictingObject.object.classId = 8u;
  restrictingObject.object.version = 0u;
  restrictingObject.object.logicalName = MakeName(8u);
  restrictingObject.attributeId = 2u;
  restrictingObject.dataIndex = 0u;

  dlms::cosem::CosemCaptureObject selectedObject;
  selectedObject.object.classId = 3u;
  selectedObject.object.version = 0u;
  selectedObject.object.logicalName = MakeName(3u);
  selectedObject.attributeId = 2u;
  selectedObject.dataIndex = 0u;

  dlms::cosem::CosemProfileGenericRangeDescriptor descriptor;
  descriptor.restrictingObject = restrictingObject;
  descriptor.fromValue = EncodedDateTime(0x10u);
  descriptor.toValue = EncodedDateTime(0x20u);
  descriptor.selectedValues.push_back(selectedObject);

  dlms::cosem::CosemByteBuffer expected;
  expected.push_back(0x02u);
  expected.push_back(0x04u);
  const dlms::cosem::CosemByteBuffer restricting =
    EncodedCaptureObject(restrictingObject);
  expected.insert(expected.end(), restricting.begin(), restricting.end());
  expected.insert(
    expected.end(),
    descriptor.fromValue.begin(),
    descriptor.fromValue.end());
  expected.insert(
    expected.end(),
    descriptor.toValue.begin(),
    descriptor.toValue.end());
  expected.push_back(0x01u);
  expected.push_back(0x01u);
  const dlms::cosem::CosemByteBuffer selected =
    EncodedCaptureObject(selectedObject);
  expected.insert(expected.end(), selected.begin(), selected.end());

  const dlms::cosem::CosemByteBuffer encoded =
    dlms::cosem::EncodeProfileGenericRangeDescriptor(descriptor);
  EXPECT_EQ(expected, encoded);

  dlms::cosem::CosemProfileGenericRangeDescriptor decoded;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            dlms::cosem::DecodeProfileGenericRangeDescriptor(
              encoded,
              decoded));
  EXPECT_EQ(
    descriptor.restrictingObject.object.classId,
    decoded.restrictingObject.object.classId);
  EXPECT_EQ(
    descriptor.restrictingObject.object.logicalName,
    decoded.restrictingObject.object.logicalName);
  EXPECT_EQ(
    descriptor.restrictingObject.attributeId,
    decoded.restrictingObject.attributeId);
  EXPECT_EQ(
    descriptor.restrictingObject.dataIndex,
    decoded.restrictingObject.dataIndex);
  EXPECT_EQ(descriptor.fromValue, decoded.fromValue);
  EXPECT_EQ(descriptor.toValue, decoded.toValue);
  ASSERT_EQ(1u, decoded.selectedValues.size());
  EXPECT_EQ(
    selectedObject.object.classId,
    decoded.selectedValues[0].object.classId);
  EXPECT_EQ(
    selectedObject.object.logicalName,
    decoded.selectedValues[0].object.logicalName);
  EXPECT_EQ(selectedObject.attributeId, decoded.selectedValues[0].attributeId);
  EXPECT_EQ(selectedObject.dataIndex, decoded.selectedValues[0].dataIndex);
}

TEST(CosemProfileGenericObject, EncodesAndDecodesEntryDescriptor)
{
  dlms::cosem::CosemProfileGenericEntryDescriptor descriptor;
  descriptor.fromEntry = 1u;
  descriptor.toEntry = 10u;
  descriptor.fromSelectedValue = 2u;
  descriptor.toSelectedValue = 0u;

  dlms::cosem::CosemByteBuffer expected;
  expected.push_back(0x02u);
  expected.push_back(0x04u);
  AppendDoubleLongUnsigned(expected, 1u);
  AppendDoubleLongUnsigned(expected, 10u);
  AppendLongUnsigned(expected, 2u);
  AppendLongUnsigned(expected, 0u);

  const dlms::cosem::CosemByteBuffer encoded =
    dlms::cosem::EncodeProfileGenericEntryDescriptor(descriptor);
  EXPECT_EQ(expected, encoded);

  dlms::cosem::CosemProfileGenericEntryDescriptor decoded;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            dlms::cosem::DecodeProfileGenericEntryDescriptor(
              encoded,
              decoded));
  EXPECT_EQ(descriptor.fromEntry, decoded.fromEntry);
  EXPECT_EQ(descriptor.toEntry, decoded.toEntry);
  EXPECT_EQ(descriptor.fromSelectedValue, decoded.fromSelectedValue);
  EXPECT_EQ(descriptor.toSelectedValue, decoded.toSelectedValue);
}

TEST(CosemProfileGenericObject, RejectsMalformedRangeDescriptor)
{
  dlms::cosem::CosemCaptureObject restrictingObject;
  restrictingObject.object.classId = 8u;
  restrictingObject.object.version = 0u;
  restrictingObject.object.logicalName = MakeName(8u);
  restrictingObject.attributeId = 2u;
  restrictingObject.dataIndex = 0u;

  dlms::cosem::CosemByteBuffer malformed;
  malformed.push_back(0x02u);
  malformed.push_back(0x04u);
  const dlms::cosem::CosemByteBuffer restricting =
    EncodedCaptureObject(restrictingObject);
  malformed.insert(malformed.end(), restricting.begin(), restricting.end());
  malformed.push_back(0x00u);
  const dlms::cosem::CosemByteBuffer toValue = EncodedDateTime(0x20u);
  malformed.insert(malformed.end(), toValue.begin(), toValue.end());
  malformed.push_back(0x01u);
  malformed.push_back(0x00u);

  dlms::cosem::CosemProfileGenericRangeDescriptor decoded;
  decoded.selectedValues.resize(1u);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::DecodeProfileGenericRangeDescriptor(
              malformed,
              decoded));
  EXPECT_EQ(1u, decoded.selectedValues.size());
}

TEST(CosemProfileGenericObject, RejectsMalformedBufferRows)
{
  std::vector<dlms::cosem::CosemByteBuffer> decoded;
  decoded.push_back(Bytes(0xAAu, 0xBBu));

  dlms::cosem::CosemByteBuffer malformed;
  malformed.push_back(0x01u);
  malformed.push_back(0x01u);
  malformed.push_back(0x06u);
  malformed.push_back(0x00u);
  malformed.push_back(0x00u);
  malformed.push_back(0x00u);
  malformed.push_back(0x01u);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::DecodeProfileGenericBuffer(
              malformed,
              decoded));
  ASSERT_EQ(1u, decoded.size());
  EXPECT_EQ(Bytes(0xAAu, 0xBBu), decoded[0]);
}

TEST(CosemProfileGenericObject, RejectsWritesAndReportsUnsupportedMethods)
{
  dlms::cosem::CosemProfileGenericObject object(
    MakeName(7u),
    std::vector<dlms::cosem::CosemByteBuffer>(),
    std::vector<dlms::cosem::CosemCaptureObject>(),
    0u,
    0u);

  const dlms::cosem::CosemByteBuffer input = Bytes(0x01u, 0x02u);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, input));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, input));

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, input, output));
  EXPECT_TRUE(output.empty());

  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(99u, input, output));
  EXPECT_TRUE(output.empty());

  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
}

TEST(SimpleObjects, WorkThroughObjectRegistryAccessChecks)
{
  const dlms::cosem::CosemLogicalName name = MakeName(5u);
  std::shared_ptr<dlms::cosem::CosemDataObject> object(
    new dlms::cosem::CosemDataObject(
      name,
      Bytes(0x01u, 0x02u),
      dlms::cosem::AttributeAccessMode::ReadAndWrite));

  dlms::cosem::ObjectRegistry registry;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.ReadAttribute(
              MakeAttribute(object->Descriptor().key, 1u),
              output));
  EXPECT_EQ(EncodedLogicalName(name), output);

  const dlms::cosem::CosemByteBuffer updated = Bytes(0x03u, 0x04u);
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.WriteAttribute(
              MakeAttribute(object->Descriptor().key, 2u),
              updated));
  EXPECT_EQ(updated, object->Value());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            registry.WriteAttribute(
              MakeAttribute(object->Descriptor().key, 1u),
              updated));
}

TEST(SimpleObjects, RegistryRejectsInvalidLogicalName)
{
  std::shared_ptr<dlms::cosem::CosemDataObject> object(
    new dlms::cosem::CosemDataObject(
      dlms::cosem::CosemLogicalName(),
      Bytes(0x01u, 0x02u),
      dlms::cosem::AttributeAccessMode::ReadOnly));

  dlms::cosem::ObjectRegistry registry;
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            registry.Register(object));
}

TEST(CosemAccessRights, ExposesEntriesInAttributeAndMethodOrder)
{
  dlms::cosem::CosemAccessRights rights;
  rights.SetAttributeAccess(3u, dlms::cosem::AttributeAccessMode::ReadOnly);
  rights.SetAttributeAccess(
    1u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  rights.SetMethodAccess(2u, dlms::cosem::MethodAccessMode::Access);
  rights.SetMethodAccess(
    1u,
    dlms::cosem::MethodAccessMode::AuthenticatedAccess);

  const std::vector<dlms::cosem::AttributeAccessEntry> attributes =
    rights.AttributeAccessEntries();
  ASSERT_EQ(2u, attributes.size());
  EXPECT_EQ(1u, attributes[0].attributeId);
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            attributes[0].mode);
  EXPECT_EQ(3u, attributes[1].attributeId);
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly, attributes[1].mode);

  const std::vector<dlms::cosem::MethodAccessEntry> methods =
    rights.MethodAccessEntries();
  ASSERT_EQ(2u, methods.size());
  EXPECT_EQ(1u, methods[0].methodId);
  EXPECT_EQ(dlms::cosem::MethodAccessMode::AuthenticatedAccess,
            methods[0].mode);
  EXPECT_EQ(2u, methods[1].methodId);
  EXPECT_EQ(dlms::cosem::MethodAccessMode::Access, methods[1].mode);
}

TEST(CosemAssociationLnObject, ExposesDescriptorAndObjectList)
{
  dlms::cosem::AssociationView view;
  dlms::cosem::AssociationViewObject entry;
  entry.descriptor.key.classId = 3u;
  entry.descriptor.key.version = 0u;
  entry.descriptor.key.logicalName = MakeName(7u);
  entry.accessRights.SetAttributeAccess(
    1u,
    dlms::cosem::AttributeAccessMode::ReadOnly);
  entry.accessRights.SetAttributeAccess(
    2u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  entry.accessRights.SetMethodAccess(
    1u,
    dlms::cosem::MethodAccessMode::Access);
  view.objects.push_back(entry);

  dlms::cosem::CosemAssociationLnObject object(
    dlms::cosem::CurrentAssociationLnName(),
    view);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(15u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(dlms::cosem::CurrentAssociationLnName(),
            descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::CurrentAssociationLnName()),
            output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  dlms::cosem::CosemByteBuffer expected;
  expected.push_back(0x01u);
  expected.push_back(0x01u);
  expected.push_back(0x02u);
  expected.push_back(0x04u);
  AppendLongUnsigned(expected, 3u);
  expected.push_back(0x11u);
  expected.push_back(0x00u);
  AppendOctetString(expected, MakeName(7u));
  expected.push_back(0x02u);
  expected.push_back(0x02u);
  expected.push_back(0x01u);
  expected.push_back(0x02u);
  expected.push_back(0x02u);
  expected.push_back(0x03u);
  expected.push_back(0x0Fu);
  expected.push_back(0x01u);
  expected.push_back(0x16u);
  expected.push_back(0x01u);
  expected.push_back(0x00u);
  expected.push_back(0x02u);
  expected.push_back(0x03u);
  expected.push_back(0x0Fu);
  expected.push_back(0x02u);
  expected.push_back(0x16u);
  expected.push_back(0x03u);
  expected.push_back(0x00u);
  expected.push_back(0x01u);
  expected.push_back(0x01u);
  expected.push_back(0x02u);
  expected.push_back(0x02u);
  expected.push_back(0x0Fu);
  expected.push_back(0x01u);
  expected.push_back(0x16u);
  expected.push_back(0x01u);
  EXPECT_EQ(expected, output);
}

TEST(CosemAssociationLnObject, EncodesAndDecodesAccessRights)
{
  dlms::cosem::CosemAccessRights rights;
  rights.SetAttributeAccess(
    1u,
    dlms::cosem::AttributeAccessMode::ReadOnly);
  rights.SetAttributeAccess(
    2u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  rights.SetMethodAccess(
    1u,
    dlms::cosem::MethodAccessMode::Access);

  const dlms::cosem::CosemByteBuffer encoded =
    dlms::cosem::EncodeAssociationAccessRights(rights);

  dlms::cosem::CosemAccessRights decoded;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            dlms::cosem::DecodeAssociationAccessRights(
              encoded,
              decoded));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadOnly,
    decoded.AttributeAccess(1u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    decoded.AttributeAccess(2u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::Access,
    decoded.MethodAccess(1u));
}

TEST(CosemAssociationLnObject, EncodesAndDecodesObjectList)
{
  dlms::cosem::AssociationView view;
  dlms::cosem::AssociationViewObject first;
  first.descriptor.key.classId = 1u;
  first.descriptor.key.version = 0u;
  first.descriptor.key.logicalName = MakeName(1u);
  first.accessRights.SetAttributeAccess(
    1u,
    dlms::cosem::AttributeAccessMode::ReadOnly);
  view.objects.push_back(first);

  dlms::cosem::AssociationViewObject second;
  second.descriptor.key.classId = 3u;
  second.descriptor.key.version = 0u;
  second.descriptor.key.logicalName = MakeName(3u);
  second.accessRights.SetAttributeAccess(
    2u,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  second.accessRights.SetMethodAccess(
    1u,
    dlms::cosem::MethodAccessMode::AuthenticatedAccess);
  view.objects.push_back(second);

  const dlms::cosem::CosemByteBuffer encoded =
    dlms::cosem::EncodeAssociationObjectList(view);

  dlms::cosem::AssociationView decoded;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            dlms::cosem::DecodeAssociationObjectList(
              encoded,
              decoded));
  ASSERT_EQ(2u, decoded.objects.size());
  EXPECT_EQ(1u, decoded.objects[0].descriptor.key.classId);
  EXPECT_EQ(MakeName(1u), decoded.objects[0].descriptor.key.logicalName);
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadOnly,
    decoded.objects[0].accessRights.AttributeAccess(1u));
  EXPECT_EQ(3u, decoded.objects[1].descriptor.key.classId);
  EXPECT_EQ(MakeName(3u), decoded.objects[1].descriptor.key.logicalName);
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    decoded.objects[1].accessRights.AttributeAccess(2u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::AuthenticatedAccess,
    decoded.objects[1].accessRights.MethodAccess(1u));
}

TEST(CosemAssociationLnObject, RejectsMalformedObjectList)
{
  dlms::cosem::AssociationView decoded;
  decoded.objects.resize(1u);

  dlms::cosem::CosemByteBuffer malformed;
  malformed.push_back(0x01u);
  malformed.push_back(0x01u);
  malformed.push_back(0x02u);
  malformed.push_back(0x03u);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::DecodeAssociationObjectList(
              malformed,
              decoded));
  EXPECT_EQ(1u, decoded.objects.size());
}

TEST(LogicalDeviceNameObject, BuildsReadOnlyDataObject)
{
  dlms::cosem::CosemDataObject object =
    dlms::cosem::MakeLogicalDeviceNameObject("ld-1");

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(1u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(dlms::cosem::LogicalDeviceNameObjectName(),
            descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::LogicalDeviceNameObjectName()),
            output);

  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(EncodedOctetString("ld-1"), output);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(2u));
}

TEST(CosemSapAssignmentObject, ExposesDescriptorAndAssignments)
{
  std::vector<dlms::cosem::SapAssignment> assignments;
  dlms::cosem::SapAssignment first;
  first.sap = 1u;
  first.logicalDeviceName = "ld-1";
  assignments.push_back(first);
  dlms::cosem::SapAssignment second;
  second.sap = 16u;
  second.logicalDeviceName = "public";
  assignments.push_back(second);

  dlms::cosem::CosemSapAssignmentObject object(
    dlms::cosem::SapAssignmentName(),
    assignments);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(17u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(dlms::cosem::SapAssignmentName(), descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::SapAssignmentName()), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  dlms::cosem::CosemByteBuffer expected;
  expected.push_back(0x01u);
  expected.push_back(0x02u);
  expected.push_back(0x02u);
  expected.push_back(0x02u);
  AppendLongUnsigned(expected, 1u);
  expected.push_back(0x09u);
  expected.push_back(0x04u);
  expected.push_back('l');
  expected.push_back('d');
  expected.push_back('-');
  expected.push_back('1');
  expected.push_back(0x02u);
  expected.push_back(0x02u);
  AppendLongUnsigned(expected, 16u);
  expected.push_back(0x09u);
  expected.push_back(0x06u);
  expected.push_back('p');
  expected.push_back('u');
  expected.push_back('b');
  expected.push_back('l');
  expected.push_back('i');
  expected.push_back('c');
  EXPECT_EQ(expected, output);
}

TEST(DiscoveryObjects, RejectUnsupportedAttributesWritesAndMethods)
{
  dlms::cosem::AssociationView view;
  dlms::cosem::CosemAssociationLnObject association(
    dlms::cosem::CurrentAssociationLnName(),
    view);
  std::vector<dlms::cosem::SapAssignment> assignments;
  dlms::cosem::CosemSapAssignmentObject sap(
    dlms::cosem::SapAssignmentName(),
    assignments);
  dlms::cosem::CosemByteBuffer bytes;
  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            association.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            association.WriteAttribute(2u, bytes));
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            association.InvokeMethod(1u, bytes, output));
  EXPECT_TRUE(output.empty());
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            sap.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            sap.WriteAttribute(2u, bytes));
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            sap.InvokeMethod(1u, bytes, output));
  EXPECT_TRUE(output.empty());
}

TEST(DiscoveryObjects, DefaultLogicalNamesUseStandardObisValues)
{
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u),
            dlms::cosem::CurrentAssociationLnName());
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 41u, 0u, 0u, 255u),
            dlms::cosem::SapAssignmentName());
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 42u, 0u, 0u, 255u),
            dlms::cosem::LogicalDeviceNameObjectName());
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 43u, 0u, 0u, 255u),
            dlms::cosem::SecuritySetupName());
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 43u, 1u, 0u, 255u),
            dlms::cosem::InvocationCounterObjectName());
}

TEST(InvocationCounterObject, BuildsReadOnlyDataObject)
{
  dlms::cosem::CosemDataObject object =
    dlms::cosem::MakeInvocationCounterObject(0x01020304u);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(1u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(dlms::cosem::InvocationCounterObjectName(),
            descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::InvocationCounterObjectName()),
            output);

  dlms::cosem::CosemByteBuffer expected;
  AppendDoubleLongUnsigned(expected, 0x01020304u);
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(expected, output);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(2u));
}

TEST(CosemSecuritySetupObject, ExposesDescriptorAndSecurityAttributes)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x30u,
    0x01u,
    client,
    server);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(64u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(dlms::cosem::SecuritySetupName(), descriptor.key.logicalName);
  EXPECT_EQ(0x30u, object.SecurityPolicy());
  EXPECT_EQ(0x01u, object.SecuritySuite());
  EXPECT_EQ(client, object.ClientSystemTitle());
  EXPECT_EQ(server, object.ServerSystemTitle());

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::SecuritySetupName()), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(Bytes(0x16u, 0x30u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, output));
  EXPECT_EQ(Bytes(0x16u, 0x01u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, output));
  EXPECT_EQ(EncodedOctetString(client), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, output));
  EXPECT_EQ(EncodedOctetString(server), output);

  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
}

TEST(CosemSecuritySetupObject, ActivatesOnlyMonotonicSecurityPolicy)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x01u,
    0x00u,
    client,
    server);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(1u, Bytes(0x16u, 0x03u), output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(0x03u, object.SecurityPolicy());

  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(Bytes(0x16u, 0x03u), output);

  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.InvokeMethod(1u, Bytes(0x16u, 0x01u), output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(0x03u, object.SecurityPolicy());

  dlms::cosem::CosemByteBuffer invalid = Bytes(0x11u, 0x03u);
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.InvokeMethod(1u, invalid, output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(0x03u, object.SecurityPolicy());
}

TEST(CosemSecuritySetupObject, RejectsWritesAndReportsUnsupportedMethods)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x00u,
    0x00u,
    client,
    server);

  dlms::cosem::CosemByteBuffer bytes = Bytes(0x01u, 0x02u);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, bytes));

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, bytes, output));
  EXPECT_TRUE(output.empty());

  for (std::uint8_t methodId = 3u; methodId <= 8u; ++methodId) {
    output = Bytes(0xAAu, 0xBBu);
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(methodId, bytes, output));
    EXPECT_TRUE(output.empty());
  }
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(99u, bytes, output));
  EXPECT_TRUE(output.empty());
}

TEST(CosemSecuritySetupObject, TransfersGlobalKeyThroughMutableKeyStore)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(0x02u);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
  EXPECT_EQ(dlms::security::SecurityKeyRole::Authentication, installed.role);
  ASSERT_EQ(sizeof(authenticationBytes), installed.size);
  for (std::size_t i = 0u; i < installed.size; ++i) {
    EXPECT_EQ(authenticationBytes[i], installed.bytes[i]);
  }
}

TEST(CosemSecuritySetupObject, RejectsTamperedGlobalKeyTransfer)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));
  wrapped[0] ^= 0x01u;

  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(0x02u);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, RejectsUnsupportedGlobalKeyTransferKeyId)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  const dlms::cosem::CosemByteBuffer input =
    EncodedGlobalKeyTransfer(0x7Fu, wrapped);

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, RejectsGlobalKeyTransferTrailingBytes)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  dlms::cosem::CosemByteBuffer input =
    EncodedGlobalKeyTransfer(0x02u, wrapped);
  input.push_back(0x00u);

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, RejectsGlobalKeyTransferForUnsupportedSuite)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  const dlms::cosem::CosemByteBuffer input =
    EncodedGlobalKeyTransfer(0x02u, wrapped);

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x01u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, KeyTransferResetsInvocationCounters)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::InMemoryInvocationCounterStore counters;
  const std::uint8_t title[8] =
    {0x53u, 0x54u, 0x31u, 0u, 0u, 0u, 0u, 1u};
  counters.SetLocalCounter(10u);
  ASSERT_EQ(
    dlms::security::SecurityStatus::Ok,
    counters.ValidateRemoteForSystemTitle(title, sizeof(title), 20u));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(0x02u);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore,
    &counters);

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(2u, input, output));

  std::uint32_t local = 0u;
  EXPECT_EQ(dlms::security::SecurityStatus::Ok, counters.NextLocal(local));
  EXPECT_EQ(1u, local);
  EXPECT_EQ(
    dlms::security::SecurityStatus::Ok,
    counters.ValidateRemoteForSystemTitle(title, sizeof(title), 20u));
}

TEST(CosemSecuritySetupObject, KeyTransferResetFailureDoesNotInstallKey)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  const dlms::cosem::CosemByteBuffer input =
    EncodedGlobalKeyTransfer(0x02u, wrapped);

  FailingCounterResetPolicy resetPolicy;
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore,
    &resetPolicy);

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.InvokeMethod(2u, input, output));
  EXPECT_EQ(1u, resetPolicy.Calls());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, RegistryActivatesSecurityPolicy)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  std::shared_ptr<dlms::cosem::CosemSecuritySetupObject> object(
    new dlms::cosem::CosemSecuritySetupObject(
      dlms::cosem::SecuritySetupName(),
      0x00u,
      0x00u,
      client,
      server));
  dlms::cosem::ObjectRegistry registry;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.InvokeMethod(
              MakeMethod(object->Descriptor().key, 1u),
              Bytes(0x16u, 0x03u),
              output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(0x03u, object->SecurityPolicy());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            registry.InvokeMethod(
              MakeMethod(object->Descriptor().key, 1u),
              Bytes(0x16u, 0x01u),
              output));
  EXPECT_EQ(0x03u, object->SecurityPolicy());

  dlms::cosem::CosemByteBuffer bytes;
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            registry.WriteAttribute(
              MakeAttribute(object->Descriptor().key, 2u),
              bytes));
}
