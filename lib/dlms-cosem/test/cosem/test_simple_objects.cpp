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

dlms::cosem::CosemByteBuffer Bytes3(
  std::uint8_t first,
  std::uint8_t second,
  std::uint8_t third)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(first);
  bytes.push_back(second);
  bytes.push_back(third);
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

dlms::cosem::CosemByteBuffer EncodedRawOctetString(
  const dlms::cosem::CosemByteBuffer& value)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x09u);
  bytes.push_back(static_cast<std::uint8_t>(value.size()));
  bytes.insert(bytes.end(), value.begin(), value.end());
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

TEST(CosemDataObject, NormalizesRequestedVersion)
{
  dlms::cosem::CosemDataObject object(
    MakeName(1u),
    Bytes(0x12u, 0x34u),
    dlms::cosem::AttributeAccessMode::ReadOnly,
    7u);

  EXPECT_EQ(dlms::cosem::CosemDataObject::MaxSupportedVersion,
            object.Descriptor().key.version);
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

TEST(CosemRegisterObject, NormalizesRequestedVersion)
{
  dlms::cosem::CosemRegisterObject object(
    MakeName(3u),
    Bytes(0x12u, 0x34u),
    Bytes(0x02u, 0x03u),
    dlms::cosem::AttributeAccessMode::ReadOnly,
    7u);

  EXPECT_EQ(dlms::cosem::CosemRegisterObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemClockObject, ExposesClockAttributes)
{
  dlms::cosem::CosemByteBuffer time;
  for (std::uint8_t i = 0u; i < 12u; ++i) {
    time.push_back(static_cast<std::uint8_t>(0x10u + i));
  }
  dlms::cosem::CosemByteBuffer dstBegin;
  dlms::cosem::CosemByteBuffer dstEnd;
  for (std::uint8_t i = 0u; i < 12u; ++i) {
    dstBegin.push_back(static_cast<std::uint8_t>(0x20u + i));
    dstEnd.push_back(static_cast<std::uint8_t>(0x30u + i));
  }

  dlms::cosem::CosemClockObject object(
    MakeName(8u),
    time,
    180,
    0x80u,
    dstBegin,
    dstEnd,
    -60,
    true,
    dlms::cosem::CosemClockBase::InternalCrystal);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(8u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(MakeName(8u), descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(MakeName(8u)), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(EncodedRawOctetString(time), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, output));
  EXPECT_EQ(Bytes3(0x10u, 0x00u, 0xB4u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, output));
  EXPECT_EQ(Bytes(0x11u, 0x80u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, output));
  EXPECT_EQ(EncodedRawOctetString(dstBegin), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, output));
  EXPECT_EQ(EncodedRawOctetString(dstEnd), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(7u, output));
  EXPECT_EQ(Bytes(0x0Fu, 0xC4u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(8u, output));
  EXPECT_EQ(Bytes(0x03u, 0x01u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(9u, output));
  EXPECT_EQ(Bytes(0x16u, 0x01u), output);
}

TEST(CosemClockObject, WritesMutableClockAttributes)
{
  dlms::cosem::CosemByteBuffer value(12u, 0x11u);
  dlms::cosem::CosemClockObject object(
    MakeName(8u),
    value,
    0,
    0u,
    value,
    value,
    0,
    false,
    dlms::cosem::CosemClockBase::NotDefined);

  dlms::cosem::CosemByteBuffer updated(12u, 0x22u);
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, EncodedRawOctetString(updated)));
  EXPECT_EQ(updated, object.Time());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, Bytes3(0x10u, 0xFFu, 0x4Cu)));
  EXPECT_EQ(-180, object.TimeZone());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(5u, EncodedRawOctetString(updated)));
  EXPECT_EQ(updated, object.DaylightSavingsBegin());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(6u, EncodedRawOctetString(updated)));
  EXPECT_EQ(updated, object.DaylightSavingsEnd());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(7u, Bytes(0x0Fu, 0x3Cu)));
  EXPECT_EQ(60, object.DaylightSavingsDeviation());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(8u, Bytes(0x03u, 0x01u)));
  EXPECT_TRUE(object.DaylightSavingsEnabled());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(9u, Bytes(0x16u, 0x04u)));
  EXPECT_EQ(
    dlms::cosem::CosemClockBase::Gps,
    object.ClockBase());
}

TEST(CosemClockObject, RejectsInvalidWritesAndUnsupportedMethods)
{
  dlms::cosem::CosemByteBuffer value(12u, 0x11u);
  dlms::cosem::CosemClockObject object(
    MakeName(8u),
    value,
    0,
    0u,
    value,
    value,
    0,
    false,
    dlms::cosem::CosemClockBase::NotDefined);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedRawOctetString(value)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(4u, Bytes(0x11u, 0x00u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, Bytes(0x09u, 0x01u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(9u, Bytes(0x16u, 0xFFu)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, Bytes(0x00u, 0x00u)));

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, Bytes(0x0Fu, 0x00u), output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(99u, Bytes(0x0Fu, 0x00u), output));
  EXPECT_TRUE(output.empty());
}

TEST(CosemClockObject, NormalizesRequestedVersion)
{
  dlms::cosem::CosemByteBuffer value(12u, 0x11u);
  dlms::cosem::CosemClockObject object(
    MakeName(8u),
    value,
    0,
    0u,
    value,
    value,
    0,
    false,
    dlms::cosem::CosemClockBase::NotDefined,
    7u);

  EXPECT_EQ(dlms::cosem::CosemClockObject::MaxSupportedVersion,
            object.Descriptor().key.version);
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

  dlms::cosem::CosemCaptureObject emptySort;
  emptySort.object.classId = 0u;
  emptySort.object.version = 0u;
  emptySort.object.logicalName =
    dlms::cosem::CosemLogicalName(0u, 0u, 0u, 0u, 0u, 0u);
  emptySort.attributeId = 0u;
  emptySort.dataIndex = 0u;
  const dlms::cosem::CosemByteBuffer expectedSortObject =
    EncodedCaptureObject(emptySort);
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, output));
  EXPECT_EQ(expectedSortObject, output);

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

  dlms::cosem::CosemByteBuffer thirdRow;
  thirdRow.push_back(0x02u);
  thirdRow.push_back(0x01u);
  thirdRow.push_back(0x19u);
  for (std::uint8_t i = 0u; i < 12u; ++i) {
    thirdRow.push_back(i);
  }

  std::vector<dlms::cosem::CosemByteBuffer> rows;
  rows.push_back(firstRow);
  rows.push_back(secondRow);
  rows.push_back(thirdRow);

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

  for (std::uint8_t methodId = 1u; methodId <= 4u; ++methodId) {
    dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(methodId, input, output))
      << "methodId=" << static_cast<int>(methodId);
    EXPECT_TRUE(output.empty())
      << "methodId=" << static_cast<int>(methodId);
    EXPECT_EQ(dlms::cosem::MethodAccessMode::Access,
              object.AccessRights().MethodAccess(methodId))
      << "methodId=" << static_cast<int>(methodId);
  }

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(5u, input, output));
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

TEST(CosemProfileGenericObject, AcceptsExplicitVersion)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  std::vector<dlms::cosem::CosemCaptureObject> captures;

  dlms::cosem::CosemProfileGenericObject version0(
    MakeName(7u),
    rows,
    captures,
    60u,
    100u,
    0u);
  dlms::cosem::CosemProfileGenericObject capped(
    MakeName(7u),
    rows,
    captures,
    60u,
    100u,
    7u);

  EXPECT_EQ(0u, version0.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::CosemProfileGenericObject::MaxSupportedVersion,
            capped.Descriptor().key.version);

  for (std::uint8_t methodId = 1u; methodId <= 4u; ++methodId) {
    EXPECT_EQ(dlms::cosem::MethodAccessMode::Access,
              version0.AccessRights().MethodAccess(methodId))
      << "v0 methodId=" << static_cast<int>(methodId);
    EXPECT_EQ(dlms::cosem::MethodAccessMode::Access,
              capped.AccessRights().MethodAccess(methodId))
      << "v1 methodId=" << static_cast<int>(methodId);

    dlms::cosem::CosemByteBuffer output;
    EXPECT_EQ(
      dlms::cosem::CosemStatus::UnsupportedFeature,
      version0.InvokeMethod(methodId, dlms::cosem::CosemByteBuffer(), output))
      << "v0 methodId=" << static_cast<int>(methodId);
    EXPECT_EQ(
      dlms::cosem::CosemStatus::UnsupportedFeature,
      capped.InvokeMethod(methodId, dlms::cosem::CosemByteBuffer(), output))
      << "v1 methodId=" << static_cast<int>(methodId);
  }

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            version0.InvokeMethod(5u, dlms::cosem::CosemByteBuffer(), output));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            capped.InvokeMethod(5u, dlms::cosem::CosemByteBuffer(), output));
}

TEST(CosemProfileGenericObject, HonorsConfigurableSortMethodAndSortObject)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  std::vector<dlms::cosem::CosemCaptureObject> captures;

  dlms::cosem::CosemCaptureObject capturedClock;
  capturedClock.object.classId = 8u;
  capturedClock.object.version = 0u;
  capturedClock.object.logicalName = MakeName(8u);
  capturedClock.attributeId = 2u;
  capturedClock.dataIndex = 0u;
  captures.push_back(capturedClock);

  dlms::cosem::CosemCaptureObject capturedDemand;
  capturedDemand.object.classId = 5u;
  capturedDemand.object.version = 0u;
  capturedDemand.object.logicalName = MakeName(5u);
  capturedDemand.attributeId = 3u;
  capturedDemand.dataIndex = 0u;
  captures.push_back(capturedDemand);

  dlms::cosem::CosemProfileGenericObject object(
    MakeName(7u),
    rows,
    captures,
    0u,
    50u,
    dlms::cosem::CosemProfileGenericSortMethod::Largest,
    capturedDemand);

  EXPECT_EQ(
    dlms::cosem::CosemProfileGenericSortMethod::Largest,
    object.SortMethod());
  EXPECT_EQ(capturedDemand.object.classId, object.SortObject().object.classId);
  EXPECT_EQ(
    capturedDemand.object.logicalName,
    object.SortObject().object.logicalName);
  EXPECT_EQ(capturedDemand.attributeId, object.SortObject().attributeId);

  dlms::cosem::CosemByteBuffer expectedSortMethod;
  AppendEnum(expectedSortMethod, static_cast<std::uint8_t>(
    dlms::cosem::CosemProfileGenericSortMethod::Largest));
  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, output));
  EXPECT_EQ(expectedSortMethod, output);

  const dlms::cosem::CosemByteBuffer expectedSortObject =
    EncodedCaptureObject(capturedDemand);
  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, output));
  EXPECT_EQ(expectedSortObject, output);
}

TEST(CosemProfileGenericObject, DefaultSortMethodIsFifoWithEmptySortObject)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  std::vector<dlms::cosem::CosemCaptureObject> captures;

  dlms::cosem::CosemCaptureObject capture;
  capture.object.classId = 3u;
  capture.object.version = 0u;
  capture.object.logicalName = MakeName(3u);
  capture.attributeId = 2u;
  capture.dataIndex = 0u;
  captures.push_back(capture);

  dlms::cosem::CosemProfileGenericObject object(
    MakeName(7u),
    rows,
    captures,
    60u,
    100u);

  EXPECT_EQ(
    dlms::cosem::CosemProfileGenericSortMethod::Fifo,
    object.SortMethod());
  EXPECT_EQ(0u, object.SortObject().object.classId);
  EXPECT_EQ(0u, object.SortObject().attributeId);

  dlms::cosem::CosemCaptureObject empty;
  empty.object.classId = 0u;
  empty.object.version = 0u;
  empty.object.logicalName =
    dlms::cosem::CosemLogicalName(0u, 0u, 0u, 0u, 0u, 0u);
  empty.attributeId = 0u;
  empty.dataIndex = 0u;
  const dlms::cosem::CosemByteBuffer expectedSortObject =
    EncodedCaptureObject(empty);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, output));
  EXPECT_EQ(expectedSortObject, output);
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
  EXPECT_EQ(
    dlms::cosem::CosemAssociationStatus::Associated,
    object.AssociationStatus());
  EXPECT_FALSE(object.HasSecuritySetupReference());

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

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(8u, output));
  EXPECT_EQ(Bytes(0x16u, 0x02u), output);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadOnly,
    rights.AttributeAccess(8u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::Access,
    rights.MethodAccess(1u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::Access,
    rights.MethodAccess(4u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::NoAccess,
    rights.AttributeAccess(9u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::NoAccess,
    rights.MethodAccess(5u));

  ASSERT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(9u, output));
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

TEST(CosemAssociationLnObject, ExposesSecuritySetupReferenceWhenConfigured)
{
  dlms::cosem::AssociationView view;
  dlms::cosem::CosemAssociationLnObject object(
    dlms::cosem::CurrentAssociationLnName(),
    view,
    dlms::cosem::CosemAssociationStatus::AssociationPending,
    dlms::cosem::SecuritySetupName());

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(15u, descriptor.key.classId);
  EXPECT_EQ(1u, descriptor.key.version);
  EXPECT_EQ(
    dlms::cosem::CosemAssociationStatus::AssociationPending,
    object.AssociationStatus());
  EXPECT_TRUE(object.HasSecuritySetupReference());
  EXPECT_EQ(
    dlms::cosem::SecuritySetupName(),
    object.SecuritySetupReference());

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(8u, output));
  EXPECT_EQ(Bytes(0x16u, 0x01u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(9u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::SecuritySetupName()), output);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadOnly,
    rights.AttributeAccess(9u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::NoAccess,
    rights.AttributeAccess(10u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::NoAccess,
    rights.MethodAccess(5u));

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, output));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(5u, dlms::cosem::CosemByteBuffer(), output));
}

TEST(CosemAssociationLnObject, VersionControlsUserAttributesAndMethods)
{
  dlms::cosem::AssociationView view;
  dlms::cosem::CosemAssociationLnConfig config;
  config.version = dlms::cosem::CosemAssociationLnObject::MaxSupportedVersion;
  config.associationStatus = dlms::cosem::CosemAssociationStatus::Associated;
  config.hasSecuritySetupReference = true;
  config.securitySetupReference = dlms::cosem::SecuritySetupName();
  dlms::cosem::CosemAssociationUser operatorUser;
  operatorUser.userId = 7u;
  operatorUser.userName = "operator";
  config.users.push_back(operatorUser);
  config.currentUser = operatorUser;

  dlms::cosem::CosemAssociationLnObject object(
    dlms::cosem::CurrentAssociationLnName(),
    view,
    config);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(3u, descriptor.key.version);
  ASSERT_EQ(1u, object.Users().size());
  EXPECT_EQ(7u, object.Users()[0].userId);
  EXPECT_EQ("operator", object.Users()[0].userName);
  EXPECT_EQ(7u, object.CurrentUser().userId);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(10u, output));
  dlms::cosem::CosemByteBuffer expectedList;
  expectedList.push_back(0x01u);
  expectedList.push_back(0x01u);
  expectedList.push_back(0x02u);
  expectedList.push_back(0x02u);
  expectedList.push_back(0x11u);
  expectedList.push_back(0x07u);
  expectedList.push_back(0x0Au);
  expectedList.push_back(0x08u);
  expectedList.push_back('o');
  expectedList.push_back('p');
  expectedList.push_back('e');
  expectedList.push_back('r');
  expectedList.push_back('a');
  expectedList.push_back('t');
  expectedList.push_back('o');
  expectedList.push_back('r');
  EXPECT_EQ(expectedList, output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(11u, output));
  dlms::cosem::CosemByteBuffer expectedCurrent;
  expectedCurrent.push_back(0x02u);
  expectedCurrent.push_back(0x02u);
  expectedCurrent.push_back(0x11u);
  expectedCurrent.push_back(0x07u);
  expectedCurrent.push_back(0x0Au);
  expectedCurrent.push_back(0x08u);
  expectedCurrent.push_back('o');
  expectedCurrent.push_back('p');
  expectedCurrent.push_back('e');
  expectedCurrent.push_back('r');
  expectedCurrent.push_back('a');
  expectedCurrent.push_back('t');
  expectedCurrent.push_back('o');
  expectedCurrent.push_back('r');
  EXPECT_EQ(expectedCurrent, output);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadOnly,
    rights.AttributeAccess(10u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadOnly,
    rights.AttributeAccess(11u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::Access,
    rights.MethodAccess(5u));
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(5u, dlms::cosem::CosemByteBuffer(), output));
}

TEST(CosemAssociationLnObject, Version2DoesNotExposeUserAttributesOrMethods)
{
  dlms::cosem::AssociationView view;
  dlms::cosem::CosemAssociationLnConfig config;
  config.version = 2u;
  config.associationStatus = dlms::cosem::CosemAssociationStatus::Associated;
  config.hasSecuritySetupReference = true;
  config.securitySetupReference = dlms::cosem::SecuritySetupName();
  dlms::cosem::CosemAssociationUser operatorUser;
  operatorUser.userId = 7u;
  operatorUser.userName = "operator";
  config.users.push_back(operatorUser);
  config.currentUser = operatorUser;

  dlms::cosem::CosemAssociationLnObject object(
    dlms::cosem::CurrentAssociationLnName(),
    view,
    config);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(2u, descriptor.key.version);
  EXPECT_TRUE(object.Users().empty());
  EXPECT_EQ(0u, object.CurrentUser().userId);
  EXPECT_TRUE(object.CurrentUser().userName.empty());

  // security_setup_reference (attr 9) is exposed since v1.
  EXPECT_TRUE(object.HasSecuritySetupReference());
  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(9u, output));

  // user_list (10) and current_user (11) appear only in v3.
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, output));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(11u, output));

  // add_user (5) and remove_user (6) appear only in v3.
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(5u, dlms::cosem::CosemByteBuffer(), output));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(6u, dlms::cosem::CosemByteBuffer(), output));

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::NoAccess,
    rights.AttributeAccess(10u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::NoAccess,
    rights.AttributeAccess(11u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::NoAccess,
    rights.MethodAccess(5u));
  EXPECT_EQ(
    dlms::cosem::MethodAccessMode::NoAccess,
    rights.MethodAccess(6u));
}

TEST(CosemAssociationLnObject, NormalizesUnsupportedVersionToMaximum)
{
  dlms::cosem::AssociationView view;
  dlms::cosem::CosemAssociationLnObject object(
    dlms::cosem::CurrentAssociationLnName(),
    view,
    99u);

  EXPECT_EQ(
    dlms::cosem::CosemAssociationLnObject::MaxSupportedVersion,
    object.Descriptor().key.version);
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

TEST(CosemSapAssignmentObject, NormalizesRequestedVersion)
{
  std::vector<dlms::cosem::SapAssignment> assignments;
  dlms::cosem::CosemSapAssignmentObject object(
    dlms::cosem::SapAssignmentName(),
    assignments,
    7u);

  EXPECT_EQ(dlms::cosem::CosemSapAssignmentObject::MaxSupportedVersion,
            object.Descriptor().key.version);
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
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            association.InvokeMethod(1u, bytes, output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            association.InvokeMethod(4u, bytes, output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            association.InvokeMethod(99u, bytes, output));
  EXPECT_TRUE(output.empty());
  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            sap.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            sap.WriteAttribute(2u, bytes));
  output = Bytes(0xAAu, 0xBBu);
  // IEC 62056-6-2 ED4 (2021) §4.4.4 defines method 1 (connect_logical_device)
  // for class_id=17. The built-in object does not own the SAP-mutation
  // dispatch policy and exposes the spec id explicitly as
  // UnsupportedFeature; unknown ids continue to report MethodNotFound.
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            sap.InvokeMethod(1u, bytes, output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            sap.InvokeMethod(99u, bytes, output));
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
  EXPECT_EQ(dlms::cosem::CosemSecuritySetupObject::MaxSupportedVersion,
            descriptor.key.version);
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

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, output));
  EXPECT_EQ(Bytes(0x01u, 0x00u), output);

  output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, output));
  EXPECT_TRUE(output.empty());
}

TEST(CosemSecuritySetupObject, AcceptsExplicitVersion)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};

  dlms::cosem::CosemSecuritySetupObject version0(
    dlms::cosem::SecuritySetupName(),
    0x30u,
    0x01u,
    client,
    server,
    static_cast<std::uint8_t>(0u));
  dlms::cosem::CosemSecuritySetupObject capped(
    dlms::cosem::SecuritySetupName(),
    0x30u,
    0x01u,
    client,
    server,
    static_cast<std::uint8_t>(7u));

  EXPECT_EQ(0u, version0.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::CosemSecuritySetupObject::MaxSupportedVersion,
            capped.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::MethodAccessMode::Access,
            version0.AccessRights().MethodAccess(1u));
  EXPECT_EQ(dlms::cosem::MethodAccessMode::Access,
            version0.AccessRights().MethodAccess(2u));
  EXPECT_EQ(dlms::cosem::MethodAccessMode::NoAccess,
            version0.AccessRights().MethodAccess(3u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::NoAccess,
            version0.AccessRights().AttributeAccess(6u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            capped.AccessRights().AttributeAccess(6u));
  EXPECT_EQ(dlms::cosem::MethodAccessMode::Access,
            capped.AccessRights().MethodAccess(3u));

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            version0.ReadAttribute(6u, output));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            version0.InvokeMethod(3u, dlms::cosem::CosemByteBuffer(), output));
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            capped.InvokeMethod(3u, dlms::cosem::CosemByteBuffer(), output));
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

namespace {

dlms::cosem::CertificateInfoEntry MakeEntry(
  std::uint8_t entity,
  std::uint8_t type,
  const dlms::cosem::CertificateSystemTitle& systemTitle,
  std::initializer_list<std::uint8_t> serial,
  std::initializer_list<std::uint8_t> issuer,
  std::initializer_list<std::uint8_t> subject,
  std::initializer_list<std::uint8_t> raw)
{
  dlms::cosem::CertificateInfoEntry entry;
  entry.entity = entity;
  entry.type = type;
  entry.systemTitle = systemTitle;
  entry.serialNumber.assign(serial.begin(), serial.end());
  entry.issuer.assign(issuer.begin(), issuer.end());
  entry.subject.assign(subject.begin(), subject.end());
  entry.rawCertificate.assign(raw.begin(), raw.end());
  return entry;
}

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> items)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.insert(bytes.end(), items.begin(), items.end());
  return bytes;
}

} // namespace

TEST(CosemSecuritySetupObject, ReadCertificatesEmptyWhenStoreAttached)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::InMemoryCosemCertificateStore store;
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x00u, 0x00u, client, server,
    static_cast<dlms::security::IMutableKeyStore*>(0),
    static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
    &store);

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, output));
  // array tag (0x01) + length 0.
  ASSERT_EQ(2u, output.size());
  EXPECT_EQ(0x01u, output[0]);
  EXPECT_EQ(0x00u, output[1]);
}

TEST(CosemSecuritySetupObject, ReadCertificatesEncodesStoreEntries)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CertificateSystemTitle st = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};

  dlms::cosem::InMemoryCosemCertificateStore store;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, store.Import(MakeEntry(
    dlms::cosem::CertificateEntity_Server,
    dlms::cosem::CertificateType_DigitalSignature,
    st,
    {0x12u, 0x34u},
    {'C', 'A'},
    {'M', 'E'},
    {0xAAu, 0xBBu, 0xCCu})));

  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x00u, 0x00u, client, server,
    static_cast<dlms::security::IMutableKeyStore*>(0),
    static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
    &store);

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, output));

  // array tag, length 1, structure tag, length 6,
  //   enum 0 (server), enum 0 (digital_signature),
  //   octet-string serial(2), octet-string issuer(2),
  //   octet-string subject(2), octet-string alt-name(0).
  ASSERT_EQ(BytesFromList({
    0x01u, 0x01u,
    0x02u, 0x06u,
    0x16u, 0x00u,
    0x16u, 0x00u,
    0x09u, 0x02u, 0x12u, 0x34u,
    0x09u, 0x02u, 'C', 'A',
    0x09u, 0x02u, 'M', 'E',
    0x09u, 0x00u
  }), output);
}

TEST(CosemSecuritySetupObject, ImportExportRemoveCertificateRoundTrip)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CertificateSystemTitle st = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};

  dlms::cosem::InMemoryCosemCertificateStore store;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, store.Import(MakeEntry(
    dlms::cosem::CertificateEntity_Client,
    dlms::cosem::CertificateType_KeyAgreement,
    st,
    {0xDEu, 0xADu},
    {'C', 'A'}, {'M', 'E'},
    {0x01u, 0x02u, 0x03u, 0x04u})));

  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x00u, 0x00u, client, server,
    static_cast<dlms::security::IMutableKeyStore*>(0),
    static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
    &store);

  // export by entity: structure{enum(0), structure{enum, enum, octet-string(8)}}
  dlms::cosem::CosemByteBuffer exportByEntity = BytesFromList({
    0x02u, 0x02u,
    0x16u, 0x00u,
    0x02u, 0x03u,
    0x16u, dlms::cosem::CertificateEntity_Client,
    0x16u, dlms::cosem::CertificateType_KeyAgreement,
    0x09u, 0x08u,
    'C', 'L', 'I', 'E', 'N', 'T', '0', '1'
  });
  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(7u, exportByEntity, output));
  EXPECT_EQ(BytesFromList({0x09u, 0x04u, 0x01u, 0x02u, 0x03u, 0x04u}), output);

  // export by serial: structure{enum(1), structure{octet-string serial, octet-string issuer}}
  dlms::cosem::CosemByteBuffer exportBySerial = BytesFromList({
    0x02u, 0x02u,
    0x16u, 0x01u,
    0x02u, 0x02u,
    0x09u, 0x02u, 0xDEu, 0xADu,
    0x09u, 0x02u, 'C', 'A'
  });
  output.clear();
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(7u, exportBySerial, output));
  EXPECT_EQ(BytesFromList({0x09u, 0x04u, 0x01u, 0x02u, 0x03u, 0x04u}), output);

  // remove by entity.
  output.clear();
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(8u, exportByEntity, output));
  EXPECT_EQ(0u, store.Size());

  // subsequent export now fails with ObjectError.
  output.clear();
  EXPECT_EQ(dlms::cosem::CosemStatus::ObjectError,
            object.InvokeMethod(7u, exportByEntity, output));
}

TEST(CosemSecuritySetupObject, ImportCertificateStoresRawBytes)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::InMemoryCosemCertificateStore store;
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x00u, 0x00u, client, server,
    static_cast<dlms::security::IMutableKeyStore*>(0),
    static_cast<dlms::security::IInvocationCounterResetPolicy*>(0),
    &store);

  // method 6: data is octet-string with raw cert bytes.
  dlms::cosem::CosemByteBuffer input = BytesFromList({
    0x09u, 0x03u, 0x30u, 0x82u, 0x01u
  });
  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(6u, input, output));
  ASSERT_EQ(1u, store.Size());

  std::vector<dlms::cosem::CertificateInfoEntry> entries;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, store.List(entries));
  ASSERT_EQ(1u, entries.size());
  EXPECT_EQ(3u, entries[0].rawCertificate.size());
  EXPECT_EQ(0x30u, entries[0].rawCertificate[0]);
}

TEST(CosemSecuritySetupObject, CertificateMethodsUnsupportedWithoutStore)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x00u, 0x00u, client, server);

  dlms::cosem::CosemByteBuffer input = BytesFromList({0x09u, 0x01u, 0x00u});
  dlms::cosem::CosemByteBuffer output;
  for (std::uint8_t methodId = 6u; methodId <= 8u; ++methodId) {
    output.clear();
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(methodId, input, output));
  }
}

TEST(CosemExtendedRegisterObject, ExposesValueScalerStatusAndCaptureTime)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer value = BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x2Au});
  const dlms::cosem::CosemByteBuffer scaler = BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu});
  const dlms::cosem::CosemByteBuffer status = BytesFromList({0x11u, 0x05u});
  const dlms::cosem::CosemByteBuffer captureTime = BytesFromList({
    0x09u, 0x0Cu,
    0x07u, 0xE7u, 0x06u, 0x0Fu, 0x05u,
    0x0Au, 0x1Eu, 0x00u, 0x00u,
    0x80u, 0x00u, 0x00u
  });

  dlms::cosem::CosemExtendedRegisterObject object(
    name, value, scaler, status, captureTime,
    dlms::cosem::AttributeAccessMode::ReadOnly);

  EXPECT_EQ(4u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::CosemExtendedRegisterObject::MaxSupportedVersion,
            object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(value, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(scaler, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(captureTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));
}

TEST(CosemExtendedRegisterObject, AllowsValueWriteAndRejectsRest)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u);
  dlms::cosem::CosemExtendedRegisterObject object(
    name,
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu}),
    BytesFromList({0x11u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer updated =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x10u});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.WriteAttribute(2u, updated));
  EXPECT_EQ(updated, object.Value());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, updated));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(3u, updated));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(4u, updated));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(5u, updated));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, updated));
}

TEST(CosemExtendedRegisterObject, ResetIsUnsupportedAndOtherMethodsNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u);
  dlms::cosem::CosemExtendedRegisterObject object(
    name,
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu}),
    BytesFromList({0x11u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadOnly);

  dlms::cosem::CosemByteBuffer in = BytesFromList({0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu, 0xBBu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  out = BytesFromList({0xCCu, 0xDDu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemExtendedRegisterObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u);
  dlms::cosem::CosemExtendedRegisterObject object(
    name,
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu}),
    BytesFromList({0x11u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadOnly,
    99u);
  EXPECT_EQ(dlms::cosem::CosemExtendedRegisterObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemDemandRegisterObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 31u, 4u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer currentAvg =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x01u, 0x00u});
  const dlms::cosem::CosemByteBuffer lastAvg =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0xFFu});
  const dlms::cosem::CosemByteBuffer scaler =
    BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu});
  const dlms::cosem::CosemByteBuffer status = BytesFromList({0x11u, 0x05u});
  const dlms::cosem::CosemByteBuffer captureTime = BytesFromList({
    0x09u, 0x0Cu,
    0x07u, 0xE7u, 0x06u, 0x0Fu, 0x05u,
    0x0Au, 0x1Eu, 0x00u, 0x00u,
    0x80u, 0x00u, 0x00u});
  const dlms::cosem::CosemByteBuffer startTime = BytesFromList({
    0x09u, 0x0Cu,
    0x07u, 0xE7u, 0x06u, 0x0Fu, 0x05u,
    0x0Au, 0x00u, 0x00u, 0x00u,
    0x80u, 0x00u, 0x00u});

  dlms::cosem::CosemDemandRegisterObject object(
    name, currentAvg, lastAvg, scaler, status, captureTime, startTime,
    0x0000003Cu /* 60 seconds */, 0x000Fu /* 15 periods */);

  EXPECT_EQ(5u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::CosemDemandRegisterObject::MaxSupportedVersion,
            object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(currentAvg, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(lastAvg, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(scaler, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(captureTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(startTime, out);

  // attribute 8 = period, double-long-unsigned 60
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x3Cu}), out);

  // attribute 9 = number_of_periods, long-unsigned 15
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x0Fu}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, out));
}

TEST(CosemDemandRegisterObject, RejectsAllAttributeWrites)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 31u, 4u, 0u, 255u);
  dlms::cosem::CosemDemandRegisterObject object(
    name,
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu}),
    BytesFromList({0x11u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    60u,
    15u);

  const dlms::cosem::CosemByteBuffer probe =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x01u});
  for (std::uint8_t attr = 1u; attr <= 9u; ++attr) {
    EXPECT_EQ(
      dlms::cosem::CosemStatus::AccessDenied,
      object.WriteAttribute(attr, probe));
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, probe));
}

TEST(CosemDemandRegisterObject, ResetAndNextPeriodAreUnsupported)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 31u, 4u, 0u, 255u);
  dlms::cosem::CosemDemandRegisterObject object(
    name,
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu}),
    BytesFromList({0x11u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    60u,
    15u);

  dlms::cosem::CosemByteBuffer in = BytesFromList({0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  out = BytesFromList({0xBBu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
  out = BytesFromList({0xCCu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(3u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemDemandRegisterObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(1u, 0u, 31u, 4u, 0u, 255u);
  dlms::cosem::CosemDemandRegisterObject object(
    name,
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x00u}),
    BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu}),
    BytesFromList({0x11u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    60u,
    15u,
    99u);
  EXPECT_EQ(dlms::cosem::CosemDemandRegisterObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemRegisterActivationObject, ExposesAssignmentMaskListAndActiveMask)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 14u, 0u, 1u, 255u);
  const dlms::cosem::CosemByteBuffer assignment = BytesFromList({
    0x01u, 0x01u,
    0x02u, 0x02u,
    0x12u, 0x00u, 0x03u,
    0x09u, 0x06u, 0x01u, 0x00u, 0x01u, 0x08u, 0x00u, 0xFFu});
  const dlms::cosem::CosemByteBuffer maskList = BytesFromList({
    0x01u, 0x01u,
    0x02u, 0x02u,
    0x09u, 0x05u, 0x4Du, 0x41u, 0x49u, 0x4Eu, 0x00u,
    0x01u, 0x01u, 0x12u, 0x00u, 0x01u});
  const dlms::cosem::CosemByteBuffer activeMask = BytesFromList({
    0x09u, 0x05u, 0x4Du, 0x41u, 0x49u, 0x4Eu, 0x00u});

  dlms::cosem::CosemRegisterActivationObject object(
    name, assignment, maskList, activeMask);

  EXPECT_EQ(6u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::CosemRegisterActivationObject::MaxSupportedVersion,
            object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(assignment, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(maskList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(activeMask, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemRegisterActivationObject, RejectsAllAttributeWrites)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 14u, 0u, 1u, 255u);
  dlms::cosem::CosemRegisterActivationObject object(
    name,
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x09u, 0x00u}));

  const dlms::cosem::CosemByteBuffer probe = BytesFromList({0x09u, 0x00u});
  for (std::uint8_t attr = 1u; attr <= 4u; ++attr) {
    EXPECT_EQ(
      dlms::cosem::CosemStatus::AccessDenied,
      object.WriteAttribute(attr, probe));
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, probe));
}

TEST(CosemRegisterActivationObject, MethodsAreUnsupportedAndOthersNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 14u, 0u, 1u, 255u);
  dlms::cosem::CosemRegisterActivationObject object(
    name,
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x09u, 0x00u}));

  dlms::cosem::CosemByteBuffer in = BytesFromList({0x00u});
  for (std::uint8_t method = 1u; method <= 3u; ++method) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu, 0xBBu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(method, in, out));
    EXPECT_TRUE(out.empty());
  }
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xCCu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(4u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemRegisterActivationObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 14u, 0u, 1u, 255u);
  dlms::cosem::CosemRegisterActivationObject object(
    name,
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x09u, 0x00u}),
    99u);
  EXPECT_EQ(dlms::cosem::CosemRegisterActivationObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemRegisterMonitorObject, ExposesThresholdsMonitoredValueAndActions)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer thresholds = BytesFromList({
    0x01u, 0x02u,
    0x06u, 0x00u, 0x00u, 0x00u, 0x64u,
    0x06u, 0x00u, 0x00u, 0x00u, 0xC8u});
  const dlms::cosem::CosemByteBuffer monitoredValue = BytesFromList({
    0x02u, 0x03u,
    0x12u, 0x00u, 0x03u,
    0x09u, 0x06u, 0x01u, 0x00u, 0x01u, 0x08u, 0x00u, 0xFFu,
    0x0Fu, 0x02u});
  const dlms::cosem::CosemByteBuffer actions = BytesFromList({
    0x01u, 0x01u,
    0x02u, 0x02u,
    0x02u, 0x02u,
      0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
      0x12u, 0x00u, 0x01u,
    0x02u, 0x02u,
      0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
      0x12u, 0x00u, 0x02u});

  dlms::cosem::CosemRegisterMonitorObject object(
    name, thresholds, monitoredValue, actions,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(21u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::CosemRegisterMonitorObject::MaxSupportedVersion,
            object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(thresholds, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(monitoredValue, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(actions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemRegisterMonitorObject, ThresholdsHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer initial =
    BytesFromList({0x01u, 0x00u});
  const dlms::cosem::CosemByteBuffer replacement = BytesFromList({
    0x01u, 0x01u,
    0x06u, 0x00u, 0x00u, 0x00u, 0xFFu});

  dlms::cosem::CosemRegisterMonitorObject writable(
    name, initial,
    BytesFromList({0x02u, 0x00u}),
    BytesFromList({0x01u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.Thresholds());

  dlms::cosem::CosemRegisterMonitorObject readOnly(
    name, initial,
    BytesFromList({0x02u, 0x00u}),
    BytesFromList({0x01u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(initial, readOnly.Thresholds());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(3u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(4u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));
}

TEST(CosemRegisterMonitorObject, NoMethodsAreDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  dlms::cosem::CosemRegisterMonitorObject object(
    name,
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x02u, 0x00u}),
    BytesFromList({0x01u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer in = BytesFromList({0x00u});
  for (std::uint8_t method = 1u; method <= 4u; ++method) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu, 0xBBu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(method, in, out));
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemRegisterMonitorObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  dlms::cosem::CosemRegisterMonitorObject object(
    name,
    BytesFromList({0x01u, 0x00u}),
    BytesFromList({0x02u, 0x00u}),
    BytesFromList({0x01u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(dlms::cosem::CosemRegisterMonitorObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemScriptTableObject, ExposesScriptsAttribute)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u);
  const dlms::cosem::CosemByteBuffer scripts = BytesFromList({
    0x01u, 0x01u,
    0x02u, 0x02u,
      0x12u, 0x00u, 0x01u,
      0x01u, 0x01u,
        0x02u, 0x05u,
          0x16u, 0x01u,
          0x12u, 0x00u, 0x08u,
          0x09u, 0x06u, 0x00u, 0x00u, 0x01u, 0x00u, 0x00u, 0xFFu,
          0x0Fu, 0x01u,
          0x09u, 0x00u});

  dlms::cosem::CosemScriptTableObject object(
    name, scripts, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(9u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::CosemScriptTableObject::MaxSupportedVersion,
            object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(scripts, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemScriptTableObject, ScriptsHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u);
  const dlms::cosem::CosemByteBuffer initial =
    BytesFromList({0x01u, 0x00u});
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x01u, 0x01u, 0x02u, 0x01u, 0x12u, 0x00u, 0x07u});

  dlms::cosem::CosemScriptTableObject writable(
    name, initial, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.Scripts());

  dlms::cosem::CosemScriptTableObject readOnly(
    name, initial, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(initial, readOnly.Scripts());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));
}

TEST(CosemScriptTableObject, ExecuteIsUnsupportedAndOthersNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u);
  dlms::cosem::CosemScriptTableObject object(
    name,
    BytesFromList({0x01u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer in =
    BytesFromList({0x12u, 0x00u, 0x01u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu, 0xBBu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  out = BytesFromList({0xCCu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemScriptTableObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u);
  dlms::cosem::CosemScriptTableObject object(
    name,
    BytesFromList({0x01u, 0x00u}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(dlms::cosem::CosemScriptTableObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

namespace {

struct ActivityCalendarBuffers
{
  dlms::cosem::CosemByteBuffer calendarNameActive;
  dlms::cosem::CosemByteBuffer seasonProfileActive;
  dlms::cosem::CosemByteBuffer weekProfileTableActive;
  dlms::cosem::CosemByteBuffer dayProfileTableActive;
  dlms::cosem::CosemByteBuffer calendarNamePassive;
  dlms::cosem::CosemByteBuffer seasonProfilePassive;
  dlms::cosem::CosemByteBuffer weekProfileTablePassive;
  dlms::cosem::CosemByteBuffer dayProfileTablePassive;
  dlms::cosem::CosemByteBuffer activatePassiveCalendarTime;
};

ActivityCalendarBuffers MakeSampleActivityCalendar()
{
  ActivityCalendarBuffers b;
  b.calendarNameActive = BytesFromList({
    0x09u, 0x06u, 'A', 'C', 'T', 'I', 'V', 'E'});
  b.seasonProfileActive = BytesFromList({0x01u, 0x00u});
  b.weekProfileTableActive = BytesFromList({0x01u, 0x00u});
  b.dayProfileTableActive = BytesFromList({0x01u, 0x00u});
  b.calendarNamePassive = BytesFromList({
    0x09u, 0x07u, 'P', 'A', 'S', 'S', 'I', 'V', 'E'});
  b.seasonProfilePassive = BytesFromList({
    0x01u, 0x01u,
    0x02u, 0x03u,
      0x09u, 0x06u, 'S', 'U', 'M', 'M', 'E', 'R',
      0x09u, 0x05u, 0x07u, 0xE5u, 0x06u, 0x15u, 0x00u,
      0x09u, 0x06u, 'W', 'E', 'E', 'K', 'D', 'Y'});
  b.weekProfileTablePassive = BytesFromList({0x01u, 0x00u});
  b.dayProfileTablePassive = BytesFromList({0x01u, 0x00u});
  b.activatePassiveCalendarTime = BytesFromList({
    0x09u, 0x0Cu,
    0x07u, 0xE5u, 0x07u, 0x01u, 0xFFu,
    0x00u, 0x00u, 0x00u, 0x00u,
    0x80u, 0x00u, 0x00u});
  return b;
}

} // namespace

TEST(CosemActivityCalendarObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 13u, 0u, 0u, 255u);
  const ActivityCalendarBuffers b = MakeSampleActivityCalendar();
  dlms::cosem::CosemActivityCalendarObject object(
    name,
    b.calendarNameActive,
    b.seasonProfileActive,
    b.weekProfileTableActive,
    b.dayProfileTableActive,
    b.calendarNamePassive,
    b.seasonProfilePassive,
    b.weekProfileTablePassive,
    b.dayProfileTablePassive,
    b.activatePassiveCalendarTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(20u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemActivityCalendarObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.calendarNameActive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.seasonProfileActive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.weekProfileTableActive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.dayProfileTableActive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.calendarNamePassive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.seasonProfilePassive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.weekProfileTablePassive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.dayProfileTablePassive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(10u, out));
  EXPECT_EQ(b.activatePassiveCalendarTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(11u, out));
}

TEST(CosemActivityCalendarObject, PassiveAttributesHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 13u, 0u, 0u, 255u);
  const ActivityCalendarBuffers b = MakeSampleActivityCalendar();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x09u, 0x03u, 'N', 'E', 'W'});

  dlms::cosem::CosemActivityCalendarObject writable(
    name,
    b.calendarNameActive, b.seasonProfileActive,
    b.weekProfileTableActive, b.dayProfileTableActive,
    b.calendarNamePassive, b.seasonProfilePassive,
    b.weekProfileTablePassive, b.dayProfileTablePassive,
    b.activatePassiveCalendarTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(6u, replacement));
  EXPECT_EQ(replacement, writable.CalendarNamePassive());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(7u, replacement));
  EXPECT_EQ(replacement, writable.SeasonProfilePassive());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(8u, replacement));
  EXPECT_EQ(replacement, writable.WeekProfileTablePassive());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(9u, replacement));
  EXPECT_EQ(replacement, writable.DayProfileTablePassive());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(10u, replacement));
  EXPECT_EQ(replacement, writable.ActivatePassiveCalendarTime());

  dlms::cosem::CosemActivityCalendarObject readOnly(
    name,
    b.calendarNameActive, b.seasonProfileActive,
    b.weekProfileTableActive, b.dayProfileTableActive,
    b.calendarNamePassive, b.seasonProfilePassive,
    b.weekProfileTablePassive, b.dayProfileTablePassive,
    b.activatePassiveCalendarTime,
    dlms::cosem::AttributeAccessMode::ReadOnly);

  for (std::uint8_t id = 6u; id <= 10u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(id, replacement));
  }
  EXPECT_EQ(b.calendarNamePassive, readOnly.CalendarNamePassive());
  EXPECT_EQ(b.activatePassiveCalendarTime,
            readOnly.ActivatePassiveCalendarTime());
}

TEST(CosemActivityCalendarObject, ActiveAttributesAndLogicalNameAreReadOnly)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 13u, 0u, 0u, 255u);
  const ActivityCalendarBuffers b = MakeSampleActivityCalendar();
  dlms::cosem::CosemActivityCalendarObject object(
    name,
    b.calendarNameActive, b.seasonProfileActive,
    b.weekProfileTableActive, b.dayProfileTableActive,
    b.calendarNamePassive, b.seasonProfilePassive,
    b.weekProfileTablePassive, b.dayProfileTablePassive,
    b.activatePassiveCalendarTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer payload =
    BytesFromList({0x09u, 0x01u, 0x00u});
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, payload));
  for (std::uint8_t id = 2u; id <= 5u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              object.WriteAttribute(id, payload));
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, payload));
}

TEST(CosemActivityCalendarObject,
     ActivatePassiveCalendarIsUnsupportedAndOthersNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 13u, 0u, 0u, 255u);
  const ActivityCalendarBuffers b = MakeSampleActivityCalendar();
  dlms::cosem::CosemActivityCalendarObject object(
    name,
    b.calendarNameActive, b.seasonProfileActive,
    b.weekProfileTableActive, b.dayProfileTableActive,
    b.calendarNamePassive, b.seasonProfilePassive,
    b.weekProfileTablePassive, b.dayProfileTablePassive,
    b.activatePassiveCalendarTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu, 0xBBu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());

  out = BytesFromList({0xCCu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemActivityCalendarObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 13u, 0u, 0u, 255u);
  const ActivityCalendarBuffers b = MakeSampleActivityCalendar();
  dlms::cosem::CosemActivityCalendarObject object(
    name,
    b.calendarNameActive, b.seasonProfileActive,
    b.weekProfileTableActive, b.dayProfileTableActive,
    b.calendarNamePassive, b.seasonProfilePassive,
    b.weekProfileTablePassive, b.dayProfileTablePassive,
    b.activatePassiveCalendarTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    dlms::cosem::CosemActivityCalendarObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct ImageTransferBuffers
{
  dlms::cosem::CosemByteBuffer imageBlockSize;
  dlms::cosem::CosemByteBuffer imageTransferredBlocksStatus;
  dlms::cosem::CosemByteBuffer imageFirstNotTransferredBlockNumber;
  dlms::cosem::CosemByteBuffer imageTransferEnabled;
  dlms::cosem::CosemByteBuffer imageTransferStatus;
  dlms::cosem::CosemByteBuffer imageToActivateInfo;
};

ImageTransferBuffers MakeSampleImageTransfer()
{
  ImageTransferBuffers b;
  // double-long-unsigned 256
  b.imageBlockSize = BytesFromList({0x06u, 0x00u, 0x00u, 0x01u, 0x00u});
  // bit-string length 8 = 0xAA
  b.imageTransferredBlocksStatus =
    BytesFromList({0x04u, 0x08u, 0xAAu});
  // double-long-unsigned 7
  b.imageFirstNotTransferredBlockNumber =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x07u});
  // boolean true
  b.imageTransferEnabled = BytesFromList({0x03u, 0x01u});
  // enum 0 (image_transfer_not_initiated)
  b.imageTransferStatus = BytesFromList({0x16u, 0x00u});
  // array of structure { size, identification, signature }
  b.imageToActivateInfo = BytesFromList({
    0x01u, 0x01u,
    0x02u, 0x03u,
      0x06u, 0x00u, 0x00u, 0x10u, 0x00u,
      0x09u, 0x04u, 'F', 'W', '0', '1',
      0x09u, 0x08u, 0x01u, 0x02u, 0x03u, 0x04u,
                    0x05u, 0x06u, 0x07u, 0x08u});
  return b;
}

} // namespace

TEST(CosemImageTransferObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 44u, 0u, 0u, 255u);
  const ImageTransferBuffers b = MakeSampleImageTransfer();
  dlms::cosem::CosemImageTransferObject object(
    name,
    b.imageBlockSize,
    b.imageTransferredBlocksStatus,
    b.imageFirstNotTransferredBlockNumber,
    b.imageTransferEnabled,
    b.imageTransferStatus,
    b.imageToActivateInfo,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(18u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemImageTransferObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.imageBlockSize, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.imageTransferredBlocksStatus, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.imageFirstNotTransferredBlockNumber, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.imageTransferEnabled, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.imageTransferStatus, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.imageToActivateInfo, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(8u, out));
}

TEST(CosemImageTransferObject, TransferEnabledHonorsCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 44u, 0u, 0u, 255u);
  const ImageTransferBuffers b = MakeSampleImageTransfer();
  const dlms::cosem::CosemByteBuffer disabled =
    BytesFromList({0x03u, 0x00u});

  dlms::cosem::CosemImageTransferObject writable(
    name,
    b.imageBlockSize, b.imageTransferredBlocksStatus,
    b.imageFirstNotTransferredBlockNumber, b.imageTransferEnabled,
    b.imageTransferStatus, b.imageToActivateInfo,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(5u, disabled));
  EXPECT_EQ(disabled, writable.ImageTransferEnabled());

  dlms::cosem::CosemImageTransferObject readOnly(
    name,
    b.imageBlockSize, b.imageTransferredBlocksStatus,
    b.imageFirstNotTransferredBlockNumber, b.imageTransferEnabled,
    b.imageTransferStatus, b.imageToActivateInfo,
    dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(5u, disabled));
  EXPECT_EQ(b.imageTransferEnabled, readOnly.ImageTransferEnabled());

  for (std::uint8_t id : {1u, 2u, 3u, 4u, 6u, 7u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), disabled))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, disabled));
}

TEST(CosemImageTransferObject, MethodsAreUnsupportedAndOthersNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 44u, 0u, 0u, 255u);
  const ImageTransferBuffers b = MakeSampleImageTransfer();
  dlms::cosem::CosemImageTransferObject object(
    name,
    b.imageBlockSize, b.imageTransferredBlocksStatus,
    b.imageFirstNotTransferredBlockNumber, b.imageTransferEnabled,
    b.imageTransferStatus, b.imageToActivateInfo,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x00u});
  for (std::uint8_t method = 1u; method <= 4u; ++method) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu, 0xBBu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(method, in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xCCu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(5u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemImageTransferObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 44u, 0u, 0u, 255u);
  const ImageTransferBuffers b = MakeSampleImageTransfer();
  dlms::cosem::CosemImageTransferObject object(
    name,
    b.imageBlockSize, b.imageTransferredBlocksStatus,
    b.imageFirstNotTransferredBlockNumber, b.imageTransferEnabled,
    b.imageTransferStatus, b.imageToActivateInfo,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    dlms::cosem::CosemImageTransferObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct PushSetupBuffers
{
  dlms::cosem::CosemByteBuffer pushObjectList;
  dlms::cosem::CosemByteBuffer sendDestinationAndMethod;
  dlms::cosem::CosemByteBuffer communicationWindow;
  dlms::cosem::CosemByteBuffer randomisationStartInterval;
  dlms::cosem::CosemByteBuffer numberOfRetries;
  dlms::cosem::CosemByteBuffer repetitionDelay;
  dlms::cosem::CosemByteBuffer portReference;
  dlms::cosem::CosemByteBuffer pushClientSap;
  dlms::cosem::CosemByteBuffer pushProtectionParameters;
  dlms::cosem::CosemByteBuffer pushOperationMethod;
  dlms::cosem::CosemByteBuffer confirmationParameters;
  dlms::cosem::CosemByteBuffer lastConfirmationDateTime;
};

PushSetupBuffers MakeSamplePushSetup()
{
  PushSetupBuffers b;
  b.pushObjectList = BytesFromList({0x01u, 0x00u});
  b.sendDestinationAndMethod = BytesFromList({
    0x02u, 0x03u,
      0x16u, 0x03u,
      0x09u, 0x07u, 'h', 'o', 's', 't', ':', '8', '0',
      0x16u, 0x00u});
  b.communicationWindow = BytesFromList({0x01u, 0x00u});
  // long-unsigned 100
  b.randomisationStartInterval = BytesFromList({0x12u, 0x00u, 0x64u});
  // unsigned 3
  b.numberOfRetries = BytesFromList({0x11u, 0x03u});
  // long-unsigned 60
  b.repetitionDelay = BytesFromList({0x12u, 0x00u, 0x3Cu});
  // long-unsigned 1
  b.portReference = BytesFromList({0x12u, 0x00u, 0x01u});
  // unsigned 1
  b.pushClientSap = BytesFromList({0x11u, 0x01u});
  b.pushProtectionParameters = BytesFromList({0x01u, 0x00u});
  // enum 0
  b.pushOperationMethod = BytesFromList({0x16u, 0x00u});
  b.confirmationParameters = BytesFromList({0x01u, 0x00u});
  b.lastConfirmationDateTime = BytesFromList({
    0x09u, 0x0Cu,
    0x07u, 0xE5u, 0x06u, 0x15u, 0xFFu,
    0x12u, 0x00u, 0x00u, 0x00u,
    0x80u, 0x00u, 0x00u});
  return b;
}

dlms::cosem::CosemPushSetupObject MakePushSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const PushSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access,
  std::uint8_t version)
{
  return dlms::cosem::CosemPushSetupObject(
    name,
    b.pushObjectList,
    b.sendDestinationAndMethod,
    b.communicationWindow,
    b.randomisationStartInterval,
    b.numberOfRetries,
    b.repetitionDelay,
    b.portReference,
    b.pushClientSap,
    b.pushProtectionParameters,
    b.pushOperationMethod,
    b.confirmationParameters,
    b.lastConfirmationDateTime,
    access,
    version);
}

} // namespace

TEST(CosemPushSetupObject, V2ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 9u, 0u, 255u);
  const PushSetupBuffers b = MakeSamplePushSetup();
  dlms::cosem::CosemPushSetupObject object = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 2u);

  EXPECT_EQ(40u, object.Descriptor().key.classId);
  EXPECT_EQ(2u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPushSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.pushObjectList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.sendDestinationAndMethod, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.communicationWindow, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.randomisationStartInterval, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.numberOfRetries, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.repetitionDelay, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.portReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.pushClientSap, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(10u, out));
  EXPECT_EQ(b.pushProtectionParameters, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(11u, out));
  EXPECT_EQ(b.pushOperationMethod, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(12u, out));
  EXPECT_EQ(b.confirmationParameters, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(13u, out));
  EXPECT_EQ(b.lastConfirmationDateTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(14u, out));
}

TEST(CosemPushSetupObject, V1ExposesV1AttributesAndHidesV2Extensions)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 9u, 0u, 255u);
  const PushSetupBuffers b = MakeSamplePushSetup();
  dlms::cosem::CosemPushSetupObject object = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 1u);

  EXPECT_EQ(1u, object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  for (std::uint8_t id = 1u; id <= 10u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(id, out))
      << "attribute id " << static_cast<unsigned>(id);
  }
  for (std::uint8_t id = 11u; id <= 13u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
              object.ReadAttribute(id, out))
      << "attribute id " << static_cast<unsigned>(id);
    EXPECT_TRUE(out.empty());
  }
  const dlms::cosem::CosemByteBuffer payload =
    BytesFromList({0x01u, 0x00u});
  for (std::uint8_t id = 11u; id <= 13u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
              object.WriteAttribute(id, payload))
      << "attribute id " << static_cast<unsigned>(id);
  }
}

TEST(CosemPushSetupObject, V0HidesV1AttributesAsAttributeNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 9u, 0u, 255u);
  const PushSetupBuffers b = MakeSamplePushSetup();
  dlms::cosem::CosemPushSetupObject object = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 0u);

  EXPECT_EQ(0u, object.Descriptor().key.version);
  dlms::cosem::CosemByteBuffer out;
  for (std::uint8_t id = 1u; id <= 7u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(id, out))
      << "attribute id " << static_cast<unsigned>(id);
  }
  for (std::uint8_t id = 8u; id <= 13u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
              object.ReadAttribute(id, out))
      << "attribute id " << static_cast<unsigned>(id);
    EXPECT_TRUE(out.empty());
  }
  const dlms::cosem::CosemByteBuffer payload =
    BytesFromList({0x01u, 0x00u});
  for (std::uint8_t id = 8u; id <= 13u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
              object.WriteAttribute(id, payload))
      << "attribute id " << static_cast<unsigned>(id);
  }
}

TEST(CosemPushSetupObject, MutableAttributesHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 9u, 0u, 255u);
  const PushSetupBuffers b = MakeSamplePushSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x09u, 0x02u, 0xAAu, 0xBBu});

  dlms::cosem::CosemPushSetupObject writable = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 2u);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.PushObjectList());
  EXPECT_EQ(replacement, writable.ConfirmationParameters());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(13u, replacement));
  EXPECT_EQ(b.lastConfirmationDateTime, writable.LastConfirmationDateTime());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPushSetupObject readOnly = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadOnly, 2u);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.pushObjectList, readOnly.PushObjectList());
  EXPECT_EQ(b.confirmationParameters, readOnly.ConfirmationParameters());

  // v1: attributes 11..13 are not part of the class at all and stay
  // AttributeNotFound regardless of the caller-supplied access mode.
  dlms::cosem::CosemPushSetupObject v1 = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 1u);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              v1.WriteAttribute(static_cast<std::uint8_t>(id), replacement))
      << "v1 attribute id " << static_cast<unsigned>(id);
  }
  for (std::uint8_t id : {11u, 12u, 13u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
              v1.WriteAttribute(static_cast<std::uint8_t>(id), replacement))
      << "v1 attribute id " << static_cast<unsigned>(id);
  }
}

TEST(CosemPushSetupObject, PushIsUnsupportedAndOthersNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 9u, 0u, 255u);
  const PushSetupBuffers b = MakeSamplePushSetup();
  dlms::cosem::CosemPushSetupObject object = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 2u);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  out = BytesFromList({0xCCu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(3u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemPushSetupObject, ResetMethodClearsLastConfirmationDateTimeInV2)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 9u, 0u, 255u);
  const PushSetupBuffers b = MakeSamplePushSetup();
  dlms::cosem::CosemPushSetupObject object = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 2u);

  EXPECT_FALSE(object.LastConfirmationDateTime().empty());
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
  EXPECT_TRUE(object.LastConfirmationDateTime().empty());

  // v1 does not define method 2 at all.
  dlms::cosem::CosemPushSetupObject v1 = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 1u);
  out = BytesFromList({0xBBu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            v1.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemPushSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 9u, 0u, 255u);
  const PushSetupBuffers b = MakeSamplePushSetup();
  dlms::cosem::CosemPushSetupObject object = MakePushSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPushSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(2u, object.Descriptor().key.version);
}

namespace {

struct DisconnectControlBuffers
{
  dlms::cosem::CosemByteBuffer outputState;
  dlms::cosem::CosemByteBuffer controlState;
  dlms::cosem::CosemByteBuffer controlMode;
};

DisconnectControlBuffers MakeSampleDisconnectControl()
{
  DisconnectControlBuffers b;
  // boolean true (connected)
  b.outputState = BytesFromList({0x03u, 0x01u});
  // enum 1 (connected)
  b.controlState = BytesFromList({0x16u, 0x01u});
  // enum 2 (configured control mode)
  b.controlMode = BytesFromList({0x16u, 0x02u});
  return b;
}

} // namespace

TEST(CosemDisconnectControlObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 3u, 10u, 255u);
  const DisconnectControlBuffers b = MakeSampleDisconnectControl();
  dlms::cosem::CosemDisconnectControlObject object(
    name, b.outputState, b.controlState, b.controlMode,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(70u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemDisconnectControlObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.outputState, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.controlState, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.controlMode, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemDisconnectControlObject, ControlModeHonorsCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 3u, 10u, 255u);
  const DisconnectControlBuffers b = MakeSampleDisconnectControl();
  const dlms::cosem::CosemByteBuffer newMode =
    BytesFromList({0x16u, 0x05u});

  dlms::cosem::CosemDisconnectControlObject writable(
    name, b.outputState, b.controlState, b.controlMode,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(4u, newMode));
  EXPECT_EQ(newMode, writable.ControlMode());

  dlms::cosem::CosemDisconnectControlObject readOnly(
    name, b.outputState, b.controlState, b.controlMode,
    dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(4u, newMode));
  EXPECT_EQ(b.controlMode, readOnly.ControlMode());

  for (std::uint8_t id : {1u, 2u, 3u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), newMode))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, newMode));
}

TEST(CosemDisconnectControlObject, MethodsUnsupportedAndOthersNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 3u, 10u, 255u);
  const DisconnectControlBuffers b = MakeSampleDisconnectControl();
  dlms::cosem::CosemDisconnectControlObject object(
    name, b.outputState, b.controlState, b.controlMode,
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xBBu});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(3u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemDisconnectControlObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 3u, 10u, 255u);
  const DisconnectControlBuffers b = MakeSampleDisconnectControl();
  dlms::cosem::CosemDisconnectControlObject object(
    name, b.outputState, b.controlState, b.controlMode,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    dlms::cosem::CosemDisconnectControlObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct LimiterBuffers
{
  dlms::cosem::CosemByteBuffer monitoredValue;
  dlms::cosem::CosemByteBuffer thresholdActive;
  dlms::cosem::CosemByteBuffer thresholdNormal;
  dlms::cosem::CosemByteBuffer thresholdEmergency;
  dlms::cosem::CosemByteBuffer minOverThresholdDuration;
  dlms::cosem::CosemByteBuffer minUnderThresholdDuration;
  dlms::cosem::CosemByteBuffer emergencyProfile;
  dlms::cosem::CosemByteBuffer emergencyProfileGroupIdList;
  dlms::cosem::CosemByteBuffer emergencyProfileActive;
  dlms::cosem::CosemByteBuffer actions;
};

LimiterBuffers MakeSampleLimiter()
{
  LimiterBuffers b;
  // structure(3){ long-unsigned 3 (class_id), octet-string 6, integer 2 }
  b.monitoredValue = BytesFromList({
    0x02u, 0x03u,
      0x12u, 0x00u, 0x03u,
      0x09u, 0x06u, 0x01u, 0x00u, 0x01u, 0x08u, 0x00u, 0xFFu,
      0x0Fu, 0x02u});
  // long-unsigned 100
  b.thresholdActive = BytesFromList({0x12u, 0x00u, 0x64u});
  b.thresholdNormal = BytesFromList({0x12u, 0x00u, 0x64u});
  b.thresholdEmergency = BytesFromList({0x12u, 0x00u, 0xC8u});
  // double-long-unsigned 60
  b.minOverThresholdDuration =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x3Cu});
  b.minUnderThresholdDuration =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x3Cu});
  // structure(3){ long-unsigned id, date_time, double-long-unsigned dur }
  b.emergencyProfile = BytesFromList({
    0x02u, 0x03u,
      0x12u, 0x00u, 0x01u,
      0x09u, 0x0Cu, 0x07u, 0xE5u, 0x06u, 0x15u, 0xFFu,
        0x12u, 0x00u, 0x00u, 0x00u, 0x80u, 0x00u, 0x00u,
      0x06u, 0x00u, 0x00u, 0x01u, 0x2Cu});
  // array(1){ long-unsigned 1 }
  b.emergencyProfileGroupIdList = BytesFromList({
    0x01u, 0x01u, 0x12u, 0x00u, 0x01u});
  // boolean false
  b.emergencyProfileActive = BytesFromList({0x03u, 0x00u});
  // structure(2){ structure(2){class_id, ln}, structure(2){class_id, ln} }
  b.actions = BytesFromList({
    0x02u, 0x02u,
      0x02u, 0x02u,
        0x12u, 0x00u, 0x09u,
        0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
      0x02u, 0x02u,
        0x12u, 0x00u, 0x09u,
        0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x65u, 0xFFu});
  return b;
}

dlms::cosem::CosemLimiterObject MakeLimiterObject(
  const dlms::cosem::CosemLogicalName& name,
  const LimiterBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemLimiterObject(
    name,
    b.monitoredValue, b.thresholdActive, b.thresholdNormal,
    b.thresholdEmergency, b.minOverThresholdDuration,
    b.minUnderThresholdDuration, b.emergencyProfile,
    b.emergencyProfileGroupIdList, b.emergencyProfileActive,
    b.actions, access);
}

} // namespace

TEST(CosemLimiterObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 17u, 0u, 0u, 255u);
  const LimiterBuffers b = MakeSampleLimiter();
  dlms::cosem::CosemLimiterObject object = MakeLimiterObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(71u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemLimiterObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.monitoredValue, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.thresholdActive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.thresholdNormal, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.thresholdEmergency, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.minOverThresholdDuration, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.minUnderThresholdDuration, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.emergencyProfile, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.emergencyProfileGroupIdList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(10u, out));
  EXPECT_EQ(b.emergencyProfileActive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(11u, out));
  EXPECT_EQ(b.actions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(12u, out));
}

TEST(CosemLimiterObject, MutableAttributesHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 17u, 0u, 0u, 255u);
  const LimiterBuffers b = MakeSampleLimiter();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x01u, 0x2Cu});

  dlms::cosem::CosemLimiterObject writable = MakeLimiterObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.ThresholdActive());
  EXPECT_EQ(replacement, writable.Actions());
  // Read-only attributes always reject writes.
  for (std::uint8_t id : {1u, 2u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemLimiterObject readOnly = MakeLimiterObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.thresholdActive, readOnly.ThresholdActive());
  EXPECT_EQ(b.actions, readOnly.Actions());
}

TEST(CosemLimiterObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 17u, 0u, 0u, 255u);
  const LimiterBuffers b = MakeSampleLimiter();
  dlms::cosem::CosemLimiterObject object = MakeLimiterObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemLimiterObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 17u, 0u, 0u, 255u);
  const LimiterBuffers b = MakeSampleLimiter();
  dlms::cosem::CosemLimiterObject object(
    name,
    b.monitoredValue, b.thresholdActive, b.thresholdNormal,
    b.thresholdEmergency, b.minOverThresholdDuration,
    b.minUnderThresholdDuration, b.emergencyProfile,
    b.emergencyProfileGroupIdList, b.emergencyProfileActive,
    b.actions, dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    dlms::cosem::CosemLimiterObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct IecHdlcSetupBuffers
{
  dlms::cosem::CosemByteBuffer commSpeed;
  dlms::cosem::CosemByteBuffer windowSizeTransmit;
  dlms::cosem::CosemByteBuffer windowSizeReceive;
  dlms::cosem::CosemByteBuffer maxInfoFieldLengthTransmit;
  dlms::cosem::CosemByteBuffer maxInfoFieldLengthReceive;
  dlms::cosem::CosemByteBuffer interOctetTimeOut;
  dlms::cosem::CosemByteBuffer inactivityTimeOut;
  dlms::cosem::CosemByteBuffer deviceAddress;
};

IecHdlcSetupBuffers MakeSampleIecHdlcSetup()
{
  IecHdlcSetupBuffers b;
  // enum 5 (9600 baud)
  b.commSpeed = BytesFromList({0x16u, 0x05u});
  // unsigned 1
  b.windowSizeTransmit = BytesFromList({0x11u, 0x01u});
  b.windowSizeReceive = BytesFromList({0x11u, 0x01u});
  // long-unsigned 128
  b.maxInfoFieldLengthTransmit = BytesFromList({0x12u, 0x00u, 0x80u});
  b.maxInfoFieldLengthReceive = BytesFromList({0x12u, 0x00u, 0x80u});
  // long-unsigned 25 (ms)
  b.interOctetTimeOut = BytesFromList({0x12u, 0x00u, 0x19u});
  // long-unsigned 120 (s)
  b.inactivityTimeOut = BytesFromList({0x12u, 0x00u, 0x78u});
  // long-unsigned 16 (assigned HDLC address)
  b.deviceAddress = BytesFromList({0x12u, 0x00u, 0x10u});
  return b;
}

dlms::cosem::CosemIecHdlcSetupObject MakeIecHdlcSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const IecHdlcSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIecHdlcSetupObject(
    name,
    b.commSpeed, b.windowSizeTransmit, b.windowSizeReceive,
    b.maxInfoFieldLengthTransmit, b.maxInfoFieldLengthReceive,
    b.interOctetTimeOut, b.inactivityTimeOut, b.deviceAddress,
    access);
}

} // namespace

TEST(CosemIecHdlcSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 0u, 255u);
  const IecHdlcSetupBuffers b = MakeSampleIecHdlcSetup();
  dlms::cosem::CosemIecHdlcSetupObject object = MakeIecHdlcSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(23u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIecHdlcSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.commSpeed, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.windowSizeTransmit, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.windowSizeReceive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.maxInfoFieldLengthTransmit, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.maxInfoFieldLengthReceive, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.interOctetTimeOut, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.inactivityTimeOut, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.deviceAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, out));
}

TEST(CosemIecHdlcSetupObject, MutableAttributesHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 0u, 255u);
  const IecHdlcSetupBuffers b = MakeSampleIecHdlcSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x01u, 0x00u});

  dlms::cosem::CosemIecHdlcSetupObject writable = MakeIecHdlcSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.CommSpeed());
  EXPECT_EQ(replacement, writable.InactivityTimeOut());
  // Read-only attributes always reject writes.
  for (std::uint8_t id : {1u, 9u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemIecHdlcSetupObject readOnly = MakeIecHdlcSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.commSpeed, readOnly.CommSpeed());
  EXPECT_EQ(b.inactivityTimeOut, readOnly.InactivityTimeOut());
}

TEST(CosemIecHdlcSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 0u, 255u);
  const IecHdlcSetupBuffers b = MakeSampleIecHdlcSetup();
  dlms::cosem::CosemIecHdlcSetupObject object = MakeIecHdlcSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemIecHdlcSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 0u, 255u);
  const IecHdlcSetupBuffers b = MakeSampleIecHdlcSetup();
  dlms::cosem::CosemIecHdlcSetupObject object(
    name,
    b.commSpeed, b.windowSizeTransmit, b.windowSizeReceive,
    b.maxInfoFieldLengthTransmit, b.maxInfoFieldLengthReceive,
    b.interOctetTimeOut, b.inactivityTimeOut, b.deviceAddress,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemIecHdlcSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct RegisterTableBuffers
{
  dlms::cosem::CosemByteBuffer tableCellValues;
  dlms::cosem::CosemByteBuffer singleBuffer;
  dlms::cosem::CosemByteBuffer tableCellDefinition;
  dlms::cosem::CosemByteBuffer tableEntries;
};

RegisterTableBuffers MakeSampleRegisterTable()
{
  RegisterTableBuffers b;
  // array(2){ array(1){long-unsigned 10}, array(1){long-unsigned 20} }
  b.tableCellValues = BytesFromList({
    0x01u, 0x02u,
      0x01u, 0x01u, 0x12u, 0x00u, 0x0Au,
      0x01u, 0x01u, 0x12u, 0x00u, 0x14u});
  // boolean true
  b.singleBuffer = BytesFromList({0x03u, 0x01u});
  // structure(3){ long-unsigned 3 (Register), octet-string 6, integer 2 }
  b.tableCellDefinition = BytesFromList({
    0x02u, 0x03u,
      0x12u, 0x00u, 0x03u,
      0x09u, 0x06u, 0x01u, 0x00u, 0x01u, 0x08u, 0x00u, 0xFFu,
      0x0Fu, 0x02u});
  // long-unsigned 2
  b.tableEntries = BytesFromList({0x12u, 0x00u, 0x02u});
  return b;
}

dlms::cosem::CosemRegisterTableObject MakeRegisterTableObject(
  const dlms::cosem::CosemLogicalName& name,
  const RegisterTableBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemRegisterTableObject(
    name,
    b.tableCellValues, b.singleBuffer, b.tableCellDefinition,
    b.tableEntries, access);
}

} // namespace

TEST(CosemRegisterTableObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 61u, 0u, 0u, 255u);
  const RegisterTableBuffers b = MakeSampleRegisterTable();
  dlms::cosem::CosemRegisterTableObject object = MakeRegisterTableObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(61u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemRegisterTableObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.tableCellValues, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.singleBuffer, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.tableCellDefinition, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.tableEntries, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));
}

TEST(CosemRegisterTableObject, MutableAttributesHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 61u, 0u, 0u, 255u);
  const RegisterTableBuffers b = MakeSampleRegisterTable();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x00u, 0x05u});

  dlms::cosem::CosemRegisterTableObject writable = MakeRegisterTableObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.SingleBuffer());
  EXPECT_EQ(replacement, writable.TableEntries());
  // Read-only attributes always reject writes.
  for (std::uint8_t id : {1u, 2u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemRegisterTableObject readOnly = MakeRegisterTableObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.singleBuffer, readOnly.SingleBuffer());
  EXPECT_EQ(b.tableEntries, readOnly.TableEntries());
}

TEST(CosemRegisterTableObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 61u, 0u, 0u, 255u);
  const RegisterTableBuffers b = MakeSampleRegisterTable();
  dlms::cosem::CosemRegisterTableObject object = MakeRegisterTableObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x12u, 0x00u, 0x01u});
  for (std::uint8_t method : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {3u, 4u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemRegisterTableObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 61u, 0u, 0u, 255u);
  const RegisterTableBuffers b = MakeSampleRegisterTable();
  dlms::cosem::CosemRegisterTableObject object(
    name,
    b.tableCellValues, b.singleBuffer, b.tableCellDefinition,
    b.tableEntries, dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    dlms::cosem::CosemRegisterTableObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct TcpUdpSetupBuffers
{
  dlms::cosem::CosemByteBuffer tcpUdpPort;
  dlms::cosem::CosemByteBuffer ipReference;
  dlms::cosem::CosemByteBuffer mss;
  dlms::cosem::CosemByteBuffer nbOfSimConn;
  dlms::cosem::CosemByteBuffer inactivityTimeOut;
};

TcpUdpSetupBuffers MakeSampleTcpUdpSetup()
{
  TcpUdpSetupBuffers b;
  // long-unsigned 4059 (DLMS port)
  b.tcpUdpPort = BytesFromList({0x12u, 0x0Fu, 0xDBu});
  // octet-string 6: 0.0.25.1.0.255 (IPv4 setup)
  b.ipReference = BytesFromList({
    0x09u, 0x06u, 0x00u, 0x00u, 0x19u, 0x01u, 0x00u, 0xFFu});
  // long-unsigned 1024
  b.mss = BytesFromList({0x12u, 0x04u, 0x00u});
  // unsigned 1
  b.nbOfSimConn = BytesFromList({0x11u, 0x01u});
  // long-unsigned 180 (s)
  b.inactivityTimeOut = BytesFromList({0x12u, 0x00u, 0xB4u});
  return b;
}

dlms::cosem::CosemTcpUdpSetupObject MakeTcpUdpSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const TcpUdpSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemTcpUdpSetupObject(
    name,
    b.tcpUdpPort, b.ipReference, b.mss, b.nbOfSimConn,
    b.inactivityTimeOut, access);
}

} // namespace

TEST(CosemTcpUdpSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 0u, 0u, 255u);
  const TcpUdpSetupBuffers b = MakeSampleTcpUdpSetup();
  dlms::cosem::CosemTcpUdpSetupObject object = MakeTcpUdpSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(41u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemTcpUdpSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.tcpUdpPort, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.ipReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.mss, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.nbOfSimConn, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.inactivityTimeOut, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemTcpUdpSetupObject, MutableAttributesHonorCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 0u, 0u, 255u);
  const TcpUdpSetupBuffers b = MakeSampleTcpUdpSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x01u, 0x00u});

  dlms::cosem::CosemTcpUdpSetupObject writable = MakeTcpUdpSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.TcpUdpPort());
  EXPECT_EQ(replacement, writable.InactivityTimeOut());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemTcpUdpSetupObject readOnly = MakeTcpUdpSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.tcpUdpPort, readOnly.TcpUdpPort());
  EXPECT_EQ(b.inactivityTimeOut, readOnly.InactivityTimeOut());
}

TEST(CosemTcpUdpSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 0u, 0u, 255u);
  const TcpUdpSetupBuffers b = MakeSampleTcpUdpSetup();
  dlms::cosem::CosemTcpUdpSetupObject object = MakeTcpUdpSetupObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemTcpUdpSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 0u, 0u, 255u);
  const TcpUdpSetupBuffers b = MakeSampleTcpUdpSetup();
  dlms::cosem::CosemTcpUdpSetupObject object(
    name,
    b.tcpUdpPort, b.ipReference, b.mss, b.nbOfSimConn,
    b.inactivityTimeOut, dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    dlms::cosem::CosemTcpUdpSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

dlms::cosem::CosemByteBuffer MakeSampleScheduleEntries()
{
  // array(1) of structure(10): a single tariff-switch schedule entry.
  return BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x0Au,
        0x12u, 0x00u, 0x01u,                      // index long-unsigned 1
        0x03u, 0x01u,                              // enable boolean true
        0x09u, 0x06u, 0x00u, 0x00u, 0x0Au,         // script_logical_name
          0x00u, 0x64u, 0xFFu,
        0x12u, 0x00u, 0x01u,                      // script_selector
        0x09u, 0x04u, 0x06u, 0x00u, 0x00u, 0x00u,  // switch_time time
        0x12u, 0x00u, 0x00u,                      // validity_window
        0x04u, 0x07u, 0x7Fu,                      // exec_weekdays bit-string
        0x04u, 0x08u, 0x00u,                      // exec_specdays bit-string
        0x09u, 0x05u, 0xFFu, 0xFFu, 0xFFu, 0xFFu, // begin_date date
          0xFFu,
        0x09u, 0x05u, 0xFFu, 0xFFu, 0xFFu, 0xFFu, // end_date date
          0xFFu});
}

} // namespace

TEST(CosemScheduleObject, ExposesLogicalNameAndEntries)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries = MakeSampleScheduleEntries();
  dlms::cosem::CosemScheduleObject object(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(10u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemScheduleObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(entries, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemScheduleObject, EntriesHonorsCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries = MakeSampleScheduleEntries();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x01u, 0x00u});

  dlms::cosem::CosemScheduleObject writable(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.Entries());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemScheduleObject readOnly(
    name, entries, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(entries, readOnly.Entries());

  // Setter exposes backend-driven refresh regardless of access mode.
  readOnly.SetEntries(replacement);
  EXPECT_EQ(replacement, readOnly.Entries());
}

TEST(CosemScheduleObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries = MakeSampleScheduleEntries();
  dlms::cosem::CosemScheduleObject object(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x12u, 0x00u, 0x01u});
  // IEC 62056-6-2 ED4 (2021) §4.5.3 / DLMS UA Blue Book Ed. 12.1 §5.1.7
  // defines three specific methods for class_id=10:
  //   1 = enable_disable
  //   2 = insert
  //   3 = delete
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {0u, 4u, 5u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemScheduleObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 10u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries = MakeSampleScheduleEntries();
  dlms::cosem::CosemScheduleObject object(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemScheduleObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

dlms::cosem::CosemByteBuffer MakeSampleSpecialDaysTableEntries()
{
  // array(2) of structure(3): two special-day entries (New Year, May 1).
  return BytesFromList({
    0x01u, 0x02u,
      0x02u, 0x03u,
        0x12u, 0x00u, 0x01u,                            // index 1
        0x09u, 0x05u, 0x07u, 0xE5u, 0x01u, 0x01u, 0xFFu, // 2021-01-01
        0x11u, 0x01u,                                    // day_id 1
      0x02u, 0x03u,
        0x12u, 0x00u, 0x02u,                            // index 2
        0x09u, 0x05u, 0x07u, 0xE5u, 0x05u, 0x01u, 0xFFu, // 2021-05-01
        0x11u, 0x02u});                                  // day_id 2
}

} // namespace

TEST(CosemSpecialDaysTableObject, ExposesLogicalNameAndEntries)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 11u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries =
    MakeSampleSpecialDaysTableEntries();
  dlms::cosem::CosemSpecialDaysTableObject object(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(11u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSpecialDaysTableObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(entries, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemSpecialDaysTableObject, EntriesHonorsCallerAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 11u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries =
    MakeSampleSpecialDaysTableEntries();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x01u, 0x00u});

  dlms::cosem::CosemSpecialDaysTableObject writable(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.Entries());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemSpecialDaysTableObject readOnly(
    name, entries, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(entries, readOnly.Entries());

  // Setter exposes backend-driven refresh regardless of access mode.
  readOnly.SetEntries(replacement);
  EXPECT_EQ(replacement, readOnly.Entries());
}

TEST(CosemSpecialDaysTableObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 11u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries =
    MakeSampleSpecialDaysTableEntries();
  dlms::cosem::CosemSpecialDaysTableObject object(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in =
    BytesFromList({0x12u, 0x00u, 0x01u});
  for (std::uint8_t method : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {3u, 4u, 5u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSpecialDaysTableObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 11u, 0u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer entries =
    MakeSampleSpecialDaysTableEntries();
  dlms::cosem::CosemSpecialDaysTableObject object(
    name, entries, dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSpecialDaysTableObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct SingleActionScheduleBuffers
{
  dlms::cosem::CosemByteBuffer executedScript;
  dlms::cosem::CosemByteBuffer type;
  dlms::cosem::CosemByteBuffer executionTime;
};

SingleActionScheduleBuffers MakeSampleSingleActionSchedule()
{
  SingleActionScheduleBuffers b;
  // structure(2): script_logical_name + script_selector
  b.executedScript = BytesFromList({
    0x02u, 0x02u,
      0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
      0x12u, 0x00u, 0x01u});
  // enum 1 (SingleAction)
  b.type = BytesFromList({0x16u, 0x01u});
  // array(1) of structure(2): time + date
  b.executionTime = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x09u, 0x04u, 0x06u, 0x00u, 0x00u, 0x00u,        // 06:00:00.00
        0x09u, 0x05u, 0x07u, 0xE5u, 0x01u, 0x01u, 0xFFu});// 2021-01-01
  return b;
}

dlms::cosem::CosemSingleActionScheduleObject MakeSingleActionScheduleObject(
  const dlms::cosem::CosemLogicalName& name,
  const SingleActionScheduleBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemSingleActionScheduleObject(
    name, b.executedScript, b.type, b.executionTime, access);
}

} // namespace

TEST(CosemSingleActionScheduleObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 15u, 0u, 0u, 255u);
  const SingleActionScheduleBuffers b = MakeSampleSingleActionSchedule();
  dlms::cosem::CosemSingleActionScheduleObject object =
    MakeSingleActionScheduleObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(22u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSingleActionScheduleObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.executedScript, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.type, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.executionTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemSingleActionScheduleObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 15u, 0u, 0u, 255u);
  const SingleActionScheduleBuffers b = MakeSampleSingleActionSchedule();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x16u, 0x02u});

  dlms::cosem::CosemSingleActionScheduleObject writable =
    MakeSingleActionScheduleObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.ExecutedScript());
  EXPECT_EQ(replacement, writable.Type());
  EXPECT_EQ(replacement, writable.ExecutionTime());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemSingleActionScheduleObject readOnly =
    MakeSingleActionScheduleObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.executedScript, readOnly.ExecutedScript());
  EXPECT_EQ(b.type, readOnly.Type());
  EXPECT_EQ(b.executionTime, readOnly.ExecutionTime());
}

TEST(CosemSingleActionScheduleObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 15u, 0u, 0u, 255u);
  const SingleActionScheduleBuffers b = MakeSampleSingleActionSchedule();
  dlms::cosem::CosemSingleActionScheduleObject object =
    MakeSingleActionScheduleObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSingleActionScheduleObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 15u, 0u, 0u, 255u);
  const SingleActionScheduleBuffers b = MakeSampleSingleActionSchedule();
  dlms::cosem::CosemSingleActionScheduleObject object(
    name, b.executedScript, b.type, b.executionTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSingleActionScheduleObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct ModemConfigurationBuffers
{
  dlms::cosem::CosemByteBuffer communicationSpeed;
  dlms::cosem::CosemByteBuffer initialisationStrings;
  dlms::cosem::CosemByteBuffer modemProfile;
};

ModemConfigurationBuffers MakeSampleModemConfiguration()
{
  ModemConfigurationBuffers b;
  // enum 5 == 9600 bps
  b.communicationSpeed = BytesFromList({0x16u, 0x05u});
  // array(1) of structure(3): request, response, delay
  b.initialisationStrings = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x03u,
        0x09u, 0x04u, 'A', 'T', 'Z', 0x0Du,            // request "ATZ\r"
        0x09u, 0x02u, 'O', 'K',                          // response "OK"
        0x12u, 0x00u, 0x64u});                           // delay 100
  // array(1) of octet-string: a single profile element
  b.modemProfile = BytesFromList({
    0x01u, 0x01u,
      0x09u, 0x05u, 'D', 'E', 'F', 'L', 'T'});           // "DEFLT"
  return b;
}

dlms::cosem::CosemModemConfigurationObject MakeModemConfigurationObject(
  const dlms::cosem::CosemLogicalName& name,
  const ModemConfigurationBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemModemConfigurationObject(
    name, b.communicationSpeed, b.initialisationStrings, b.modemProfile,
    access);
}

} // namespace

TEST(CosemModemConfigurationObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 0u, 0u, 255u);
  const ModemConfigurationBuffers b = MakeSampleModemConfiguration();
  dlms::cosem::CosemModemConfigurationObject object =
    MakeModemConfigurationObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(27u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemModemConfigurationObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.communicationSpeed, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.initialisationStrings, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.modemProfile, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemModemConfigurationObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 0u, 0u, 255u);
  const ModemConfigurationBuffers b = MakeSampleModemConfiguration();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x16u, 0x07u});

  dlms::cosem::CosemModemConfigurationObject writable =
    MakeModemConfigurationObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.CommunicationSpeed());
  EXPECT_EQ(replacement, writable.InitialisationStrings());
  EXPECT_EQ(replacement, writable.ModemProfile());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemModemConfigurationObject readOnly =
    MakeModemConfigurationObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.communicationSpeed, readOnly.CommunicationSpeed());
  EXPECT_EQ(b.initialisationStrings, readOnly.InitialisationStrings());
  EXPECT_EQ(b.modemProfile, readOnly.ModemProfile());
}

TEST(CosemModemConfigurationObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 0u, 0u, 255u);
  const ModemConfigurationBuffers b = MakeSampleModemConfiguration();
  dlms::cosem::CosemModemConfigurationObject object =
    MakeModemConfigurationObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemModemConfigurationObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 0u, 0u, 255u);
  const ModemConfigurationBuffers b = MakeSampleModemConfiguration();
  dlms::cosem::CosemModemConfigurationObject object(
    name, b.communicationSpeed, b.initialisationStrings, b.modemProfile,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemModemConfigurationObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct AutoConnectBuffers
{
  dlms::cosem::CosemByteBuffer mode;
  dlms::cosem::CosemByteBuffer repetitions;
  dlms::cosem::CosemByteBuffer repetitionDelay;
  dlms::cosem::CosemByteBuffer callingWindow;
  dlms::cosem::CosemByteBuffer destinationList;
};

AutoConnectBuffers MakeSampleAutoConnect()
{
  AutoConnectBuffers b;
  // enum 1 (auto dialling allowed in calling window)
  b.mode = BytesFromList({0x16u, 0x01u});
  // unsigned 3 retries
  b.repetitions = BytesFromList({0x11u, 0x03u});
  // long-unsigned 60 seconds
  b.repetitionDelay = BytesFromList({0x12u, 0x00u, 0x3Cu});
  // array(1) of structure(2): start/end time
  b.callingWindow = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x09u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u,        // start 00:00
        0x09u, 0x04u, 0x06u, 0x00u, 0x00u, 0x00u});      // end 06:00
  // array(1) of octet-string "+7"
  b.destinationList = BytesFromList({
    0x01u, 0x01u,
      0x09u, 0x02u, '+', '7'});
  return b;
}

dlms::cosem::CosemAutoConnectObject MakeAutoConnectObject(
  const dlms::cosem::CosemLogicalName& name,
  const AutoConnectBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemAutoConnectObject(
    name, b.mode, b.repetitions, b.repetitionDelay,
    b.callingWindow, b.destinationList, access);
}

} // namespace

TEST(CosemAutoConnectObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 1u, 0u, 255u);
  const AutoConnectBuffers b = MakeSampleAutoConnect();
  dlms::cosem::CosemAutoConnectObject object =
    MakeAutoConnectObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(29u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemAutoConnectObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.mode, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.repetitions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.repetitionDelay, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.callingWindow, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.destinationList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemAutoConnectObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 1u, 0u, 255u);
  const AutoConnectBuffers b = MakeSampleAutoConnect();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x16u, 0x02u});

  dlms::cosem::CosemAutoConnectObject writable =
    MakeAutoConnectObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.Mode());
  EXPECT_EQ(replacement, writable.Repetitions());
  EXPECT_EQ(replacement, writable.RepetitionDelay());
  EXPECT_EQ(replacement, writable.CallingWindow());
  EXPECT_EQ(replacement, writable.DestinationList());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemAutoConnectObject readOnly =
    MakeAutoConnectObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.mode, readOnly.Mode());
  EXPECT_EQ(b.repetitions, readOnly.Repetitions());
  EXPECT_EQ(b.repetitionDelay, readOnly.RepetitionDelay());
  EXPECT_EQ(b.callingWindow, readOnly.CallingWindow());
  EXPECT_EQ(b.destinationList, readOnly.DestinationList());
}

TEST(CosemAutoConnectObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 1u, 0u, 255u);
  const AutoConnectBuffers b = MakeSampleAutoConnect();
  dlms::cosem::CosemAutoConnectObject object =
    MakeAutoConnectObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemAutoConnectObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 1u, 0u, 255u);
  const AutoConnectBuffers b = MakeSampleAutoConnect();
  dlms::cosem::CosemAutoConnectObject object(
    name, b.mode, b.repetitions, b.repetitionDelay,
    b.callingWindow, b.destinationList,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemAutoConnectObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct GprsModemSetupBuffers
{
  dlms::cosem::CosemByteBuffer apn;
  dlms::cosem::CosemByteBuffer pinCode;
  dlms::cosem::CosemByteBuffer qualityOfService;
};

GprsModemSetupBuffers MakeSampleGprsModemSetup()
{
  GprsModemSetupBuffers b;
  // octet-string "internet"
  b.apn = BytesFromList({
    0x09u, 0x08u, 'i', 'n', 't', 'e', 'r', 'n', 'e', 't'});
  // long-unsigned 1234
  b.pinCode = BytesFromList({0x12u, 0x04u, 0xD2u});
  // structure(5): precedence, delay, reliability, peak, mean
  b.qualityOfService = BytesFromList({
    0x02u, 0x05u,
      0x11u, 0x02u,           // precedence 2
      0x11u, 0x03u,           // delay 3
      0x11u, 0x02u,           // reliability 2
      0x11u, 0x04u,           // peak throughput 4
      0x11u, 0x10u});         // mean throughput 16
  return b;
}

dlms::cosem::CosemGprsModemSetupObject MakeGprsModemSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const GprsModemSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemGprsModemSetupObject(
    name, b.apn, b.pinCode, b.qualityOfService, access);
}

} // namespace

TEST(CosemGprsModemSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const GprsModemSetupBuffers b = MakeSampleGprsModemSetup();
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeGprsModemSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(45u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemGprsModemSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.apn, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.pinCode, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.qualityOfService, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemGprsModemSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const GprsModemSetupBuffers b = MakeSampleGprsModemSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x00u, 0x01u});

  dlms::cosem::CosemGprsModemSetupObject writable =
    MakeGprsModemSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.Apn());
  EXPECT_EQ(replacement, writable.PinCode());
  EXPECT_EQ(replacement, writable.QualityOfService());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemGprsModemSetupObject readOnly =
    MakeGprsModemSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.apn, readOnly.Apn());
  EXPECT_EQ(b.pinCode, readOnly.PinCode());
  EXPECT_EQ(b.qualityOfService, readOnly.QualityOfService());
}

TEST(CosemGprsModemSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const GprsModemSetupBuffers b = MakeSampleGprsModemSetup();
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeGprsModemSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemGprsModemSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const GprsModemSetupBuffers b = MakeSampleGprsModemSetup();
  dlms::cosem::CosemGprsModemSetupObject object(
    name, b.apn, b.pinCode, b.qualityOfService,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemGprsModemSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct AutoAnswerBuffers
{
  dlms::cosem::CosemByteBuffer mode;
  dlms::cosem::CosemByteBuffer listeningWindow;
  dlms::cosem::CosemByteBuffer status;
  dlms::cosem::CosemByteBuffer numberOfCalls;
  dlms::cosem::CosemByteBuffer numberOfRings;
};

AutoAnswerBuffers MakeSampleAutoAnswer()
{
  AutoAnswerBuffers b;
  // enum 1 (auto answering enabled)
  b.mode = BytesFromList({0x16u, 0x01u});
  // array(1) of structure(2): start/end time
  b.listeningWindow = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x09u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u,        // start 00:00
        0x09u, 0x04u, 0x06u, 0x00u, 0x00u, 0x00u});      // end 06:00
  // enum 2 (locked status)
  b.status = BytesFromList({0x16u, 0x02u});
  // unsigned 5 calls
  b.numberOfCalls = BytesFromList({0x11u, 0x05u});
  // structure(2): rings_in_window, rings_out_of_window
  b.numberOfRings = BytesFromList({
    0x02u, 0x02u,
      0x11u, 0x02u,           // rings in listening window
      0x11u, 0x04u});         // rings out of listening window
  return b;
}

dlms::cosem::CosemAutoAnswerObject MakeAutoAnswerObject(
  const dlms::cosem::CosemLogicalName& name,
  const AutoAnswerBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemAutoAnswerObject(
    name, b.mode, b.listeningWindow, b.status, b.numberOfCalls,
    b.numberOfRings, access);
}

} // namespace

TEST(CosemAutoAnswerObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const AutoAnswerBuffers b = MakeSampleAutoAnswer();
  dlms::cosem::CosemAutoAnswerObject object =
    MakeAutoAnswerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(28u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemAutoAnswerObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.mode, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.listeningWindow, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.numberOfCalls, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.numberOfRings, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemAutoAnswerObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const AutoAnswerBuffers b = MakeSampleAutoAnswer();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x16u, 0x03u});

  dlms::cosem::CosemAutoAnswerObject writable =
    MakeAutoAnswerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.Mode());
  EXPECT_EQ(replacement, writable.ListeningWindow());
  EXPECT_EQ(replacement, writable.NumberOfCalls());
  EXPECT_EQ(replacement, writable.NumberOfRings());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(4u, replacement));
  EXPECT_EQ(b.status, writable.Status());
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  // SetStatus refreshes status regardless of access mode.
  const dlms::cosem::CosemByteBuffer newStatus =
    BytesFromList({0x16u, 0x03u});
  writable.SetStatus(newStatus);
  EXPECT_EQ(newStatus, writable.Status());

  dlms::cosem::CosemAutoAnswerObject readOnly =
    MakeAutoAnswerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.mode, readOnly.Mode());
  EXPECT_EQ(b.listeningWindow, readOnly.ListeningWindow());
  EXPECT_EQ(b.numberOfCalls, readOnly.NumberOfCalls());
  EXPECT_EQ(b.numberOfRings, readOnly.NumberOfRings());
  readOnly.SetStatus(newStatus);
  EXPECT_EQ(newStatus, readOnly.Status());
}

TEST(CosemAutoAnswerObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const AutoAnswerBuffers b = MakeSampleAutoAnswer();
  dlms::cosem::CosemAutoAnswerObject object =
    MakeAutoAnswerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemAutoAnswerObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
  const AutoAnswerBuffers b = MakeSampleAutoAnswer();
  dlms::cosem::CosemAutoAnswerObject object(
    name, b.mode, b.listeningWindow, b.status, b.numberOfCalls,
    b.numberOfRings,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemAutoAnswerObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct Ipv4SetupBuffers
{
  dlms::cosem::CosemByteBuffer dlReference;
  dlms::cosem::CosemByteBuffer ipAddress;
  dlms::cosem::CosemByteBuffer multicastIpAddress;
  dlms::cosem::CosemByteBuffer ipOptions;
  dlms::cosem::CosemByteBuffer subnetMask;
  dlms::cosem::CosemByteBuffer gatewayIpAddress;
  dlms::cosem::CosemByteBuffer useDhcpFlag;
  dlms::cosem::CosemByteBuffer primaryDnsAddress;
  dlms::cosem::CosemByteBuffer secondaryDnsAddress;
};

Ipv4SetupBuffers MakeSampleIpv4Setup()
{
  Ipv4SetupBuffers b;
  // octet-string "0.0.27.0.0.255" (placeholder LN reference)
  b.dlReference = BytesFromList({
    0x09u, 0x06u, 0x00u, 0x00u, 0x1Bu, 0x00u, 0x00u, 0xFFu});
  // double-long-unsigned 192.168.1.100 -> 0xC0A80164
  b.ipAddress = BytesFromList({
    0x06u, 0xC0u, 0xA8u, 0x01u, 0x64u});
  // array(1) of double-long-unsigned 239.0.0.1
  b.multicastIpAddress = BytesFromList({
    0x01u, 0x01u,
      0x06u, 0xEFu, 0x00u, 0x00u, 0x01u});
  // array(0) - empty IP options
  b.ipOptions = BytesFromList({0x01u, 0x00u});
  // double-long-unsigned 255.255.255.0 -> 0xFFFFFF00
  b.subnetMask = BytesFromList({
    0x06u, 0xFFu, 0xFFu, 0xFFu, 0x00u});
  // double-long-unsigned 192.168.1.1 -> 0xC0A80101
  b.gatewayIpAddress = BytesFromList({
    0x06u, 0xC0u, 0xA8u, 0x01u, 0x01u});
  // boolean true
  b.useDhcpFlag = BytesFromList({0x03u, 0x01u});
  // double-long-unsigned 8.8.8.8
  b.primaryDnsAddress = BytesFromList({
    0x06u, 0x08u, 0x08u, 0x08u, 0x08u});
  // double-long-unsigned 8.8.4.4
  b.secondaryDnsAddress = BytesFromList({
    0x06u, 0x08u, 0x08u, 0x04u, 0x04u});
  return b;
}

dlms::cosem::CosemIpv4SetupObject MakeIpv4SetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const Ipv4SetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIpv4SetupObject(
    name, b.dlReference, b.ipAddress, b.multicastIpAddress,
    b.ipOptions, b.subnetMask, b.gatewayIpAddress, b.useDhcpFlag,
    b.primaryDnsAddress, b.secondaryDnsAddress, access);
}

} // namespace

TEST(CosemIpv4SetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 1u, 0u, 255u);
  const Ipv4SetupBuffers b = MakeSampleIpv4Setup();
  dlms::cosem::CosemIpv4SetupObject object =
    MakeIpv4SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(42u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIpv4SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.dlReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.ipAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.multicastIpAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.ipOptions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.subnetMask, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.gatewayIpAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.useDhcpFlag, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.primaryDnsAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(10u, out));
  EXPECT_EQ(b.secondaryDnsAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(11u, out));
}

TEST(CosemIpv4SetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 1u, 0u, 255u);
  const Ipv4SetupBuffers b = MakeSampleIpv4Setup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x06u, 0x0Au, 0x00u, 0x00u, 0x01u});

  dlms::cosem::CosemIpv4SetupObject writable =
    MakeIpv4SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.DlReference());
  EXPECT_EQ(replacement, writable.IpAddress());
  EXPECT_EQ(replacement, writable.MulticastIpAddress());
  EXPECT_EQ(replacement, writable.IpOptions());
  EXPECT_EQ(replacement, writable.SubnetMask());
  EXPECT_EQ(replacement, writable.GatewayIpAddress());
  EXPECT_EQ(replacement, writable.UseDhcpFlag());
  EXPECT_EQ(replacement, writable.PrimaryDnsAddress());
  EXPECT_EQ(replacement, writable.SecondaryDnsAddress());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemIpv4SetupObject readOnly =
    MakeIpv4SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.dlReference, readOnly.DlReference());
  EXPECT_EQ(b.ipAddress, readOnly.IpAddress());
  EXPECT_EQ(b.multicastIpAddress, readOnly.MulticastIpAddress());
  EXPECT_EQ(b.ipOptions, readOnly.IpOptions());
  EXPECT_EQ(b.subnetMask, readOnly.SubnetMask());
  EXPECT_EQ(b.gatewayIpAddress, readOnly.GatewayIpAddress());
  EXPECT_EQ(b.useDhcpFlag, readOnly.UseDhcpFlag());
  EXPECT_EQ(b.primaryDnsAddress, readOnly.PrimaryDnsAddress());
  EXPECT_EQ(b.secondaryDnsAddress, readOnly.SecondaryDnsAddress());
}

TEST(CosemIpv4SetupObject, MulticastMethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 1u, 0u, 255u);
  const Ipv4SetupBuffers b = MakeSampleIpv4Setup();
  dlms::cosem::CosemIpv4SetupObject object =
    MakeIpv4SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer addr =
    BytesFromList({0x06u, 0xEFu, 0x01u, 0x02u, 0x03u});
  // Per IEC 62056-6-2 ED4 (2021) §4.9.2.3 the IPv4 Setup IC defines three
  // methods: 1 add_mc_IP_address, 2 delete_mc_IP_address,
  // 3 get_nbof_mc_IP_addresses. All three are surfaced as
  // UnsupportedFeature; ids outside that set remain MethodNotFound.
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), addr, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {4u, 5u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), addr, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemIpv4SetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 1u, 0u, 255u);
  const Ipv4SetupBuffers b = MakeSampleIpv4Setup();
  dlms::cosem::CosemIpv4SetupObject object(
    name, b.dlReference, b.ipAddress, b.multicastIpAddress,
    b.ipOptions, b.subnetMask, b.gatewayIpAddress, b.useDhcpFlag,
    b.primaryDnsAddress, b.secondaryDnsAddress,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemIpv4SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

dlms::cosem::CosemByteBuffer MakeSampleMacAddress()
{
  // octet-string(6) 00:1A:2B:3C:4D:5E
  return BytesFromList({
    0x09u, 0x06u, 0x00u, 0x1Au, 0x2Bu, 0x3Cu, 0x4Du, 0x5Eu});
}

} // namespace

TEST(CosemMacAddressSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 2u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer mac = MakeSampleMacAddress();
  dlms::cosem::CosemMacAddressSetupObject object(
    name, mac, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(43u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemMacAddressSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(mac, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemMacAddressSetupObject, MutableAttributeHonorsAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 2u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer mac = MakeSampleMacAddress();
  const dlms::cosem::CosemByteBuffer replacement = BytesFromList({
    0x09u, 0x06u, 0xAAu, 0xBBu, 0xCCu, 0xDDu, 0xEEu, 0xFFu});

  dlms::cosem::CosemMacAddressSetupObject writable(
    name, mac, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.MacAddress());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemMacAddressSetupObject readOnly(
    name, mac, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(mac, readOnly.MacAddress());
}

TEST(CosemMacAddressSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 2u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer mac = MakeSampleMacAddress();
  dlms::cosem::CosemMacAddressSetupObject object(
    name, mac, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemMacAddressSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 2u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer mac = MakeSampleMacAddress();
  dlms::cosem::CosemMacAddressSetupObject object(
    name, mac, dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemMacAddressSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct PppSetupBuffers
{
  dlms::cosem::CosemByteBuffer phyReference;
  dlms::cosem::CosemByteBuffer lcpOptions;
  dlms::cosem::CosemByteBuffer ipcpOptions;
  dlms::cosem::CosemByteBuffer pppAuthentication;
};

PppSetupBuffers MakeSamplePppSetup()
{
  PppSetupBuffers b;
  // octet-string "0.0.27.0.0.255"
  b.phyReference = BytesFromList({
    0x09u, 0x06u, 0x00u, 0x00u, 0x1Bu, 0x00u, 0x00u, 0xFFu});
  // array(1) of structure(3): option(MRU=1) length(2) value(05DC)
  b.lcpOptions = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x03u,
        0x11u, 0x01u,                                    // option id
        0x11u, 0x02u,                                    // length
        0x09u, 0x02u, 0x05u, 0xDCu});                    // value (MRU=1500)
  // array(1) of structure(3): option(IP-Addr=3) length(4) 0.0.0.0
  b.ipcpOptions = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x03u,
        0x11u, 0x03u,
        0x11u, 0x04u,
        0x09u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u});
  // structure(2): user_name "user", password "pass"
  b.pppAuthentication = BytesFromList({
    0x02u, 0x02u,
      0x09u, 0x04u, 0x75u, 0x73u, 0x65u, 0x72u,          // "user"
      0x09u, 0x04u, 0x70u, 0x61u, 0x73u, 0x73u});        // "pass"
  return b;
}

dlms::cosem::CosemPppSetupObject MakePppSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const PppSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPppSetupObject(
    name, b.phyReference, b.lcpOptions, b.ipcpOptions,
    b.pppAuthentication, access);
}

} // namespace

TEST(CosemPppSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 3u, 0u, 255u);
  const PppSetupBuffers b = MakeSamplePppSetup();
  dlms::cosem::CosemPppSetupObject object =
    MakePppSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(44u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPppSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.phyReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.lcpOptions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.ipcpOptions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.pppAuthentication, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));
}

TEST(CosemPppSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 3u, 0u, 255u);
  const PppSetupBuffers b = MakeSamplePppSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x01u, 0x00u});

  dlms::cosem::CosemPppSetupObject writable =
    MakePppSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.PhyReference());
  EXPECT_EQ(replacement, writable.LcpOptions());
  EXPECT_EQ(replacement, writable.IpcpOptions());
  EXPECT_EQ(replacement, writable.PppAuthentication());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPppSetupObject readOnly =
    MakePppSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.phyReference, readOnly.PhyReference());
  EXPECT_EQ(b.lcpOptions, readOnly.LcpOptions());
  EXPECT_EQ(b.ipcpOptions, readOnly.IpcpOptions());
  EXPECT_EQ(b.pppAuthentication, readOnly.PppAuthentication());
}

TEST(CosemPppSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 3u, 0u, 255u);
  const PppSetupBuffers b = MakeSamplePppSetup();
  dlms::cosem::CosemPppSetupObject object =
    MakePppSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPppSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 3u, 0u, 255u);
  const PppSetupBuffers b = MakeSamplePppSetup();
  dlms::cosem::CosemPppSetupObject object(
    name, b.phyReference, b.lcpOptions, b.ipcpOptions,
    b.pppAuthentication,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPppSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct SmtpSetupBuffers
{
  dlms::cosem::CosemByteBuffer smtpServer;
  dlms::cosem::CosemByteBuffer smtpServerPort;
  dlms::cosem::CosemByteBuffer userName;
  dlms::cosem::CosemByteBuffer loginPassword;
  dlms::cosem::CosemByteBuffer sender;
  dlms::cosem::CosemByteBuffer receivers;
};

SmtpSetupBuffers MakeSampleSmtpSetup()
{
  SmtpSetupBuffers b;
  // octet-string "smtp.example.com" (16 chars)
  b.smtpServer = BytesFromList({
    0x09u, 0x10u,
      0x73u, 0x6Du, 0x74u, 0x70u, 0x2Eu, 0x65u, 0x78u, 0x61u,
      0x6Du, 0x70u, 0x6Cu, 0x65u, 0x2Eu, 0x63u, 0x6Fu, 0x6Du});
  // long-unsigned 587
  b.smtpServerPort = BytesFromList({0x12u, 0x02u, 0x4Bu});
  // octet-string "meter"
  b.userName = BytesFromList({
    0x09u, 0x05u, 0x6Du, 0x65u, 0x74u, 0x65u, 0x72u});
  // octet-string "secret"
  b.loginPassword = BytesFromList({
    0x09u, 0x06u, 0x73u, 0x65u, 0x63u, 0x72u, 0x65u, 0x74u});
  // octet-string "a@b.c"
  b.sender = BytesFromList({
    0x09u, 0x05u, 0x61u, 0x40u, 0x62u, 0x2Eu, 0x63u});
  // array(1) of octet-string "x@y.z"
  b.receivers = BytesFromList({
    0x01u, 0x01u,
      0x09u, 0x05u, 0x78u, 0x40u, 0x79u, 0x2Eu, 0x7Au});
  return b;
}

dlms::cosem::CosemSmtpSetupObject MakeSmtpSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const SmtpSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemSmtpSetupObject(
    name, b.smtpServer, b.smtpServerPort, b.userName,
    b.loginPassword, b.sender, b.receivers, access);
}

} // namespace

TEST(CosemSmtpSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 4u, 0u, 255u);
  const SmtpSetupBuffers b = MakeSampleSmtpSetup();
  dlms::cosem::CosemSmtpSetupObject object =
    MakeSmtpSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(46u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSmtpSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.smtpServer, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.smtpServerPort, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.userName, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.loginPassword, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.sender, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.receivers, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(8u, out));
}

TEST(CosemSmtpSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 4u, 0u, 255u);
  const SmtpSetupBuffers b = MakeSampleSmtpSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x09u, 0x02u, 0xDEu, 0xADu});

  dlms::cosem::CosemSmtpSetupObject writable =
    MakeSmtpSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.SmtpServer());
  EXPECT_EQ(replacement, writable.SmtpServerPort());
  EXPECT_EQ(replacement, writable.UserName());
  EXPECT_EQ(replacement, writable.LoginPassword());
  EXPECT_EQ(replacement, writable.Sender());
  EXPECT_EQ(replacement, writable.Receivers());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemSmtpSetupObject readOnly =
    MakeSmtpSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.smtpServer, readOnly.SmtpServer());
  EXPECT_EQ(b.smtpServerPort, readOnly.SmtpServerPort());
  EXPECT_EQ(b.userName, readOnly.UserName());
  EXPECT_EQ(b.loginPassword, readOnly.LoginPassword());
  EXPECT_EQ(b.sender, readOnly.Sender());
  EXPECT_EQ(b.receivers, readOnly.Receivers());
}

TEST(CosemSmtpSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 4u, 0u, 255u);
  const SmtpSetupBuffers b = MakeSampleSmtpSetup();
  dlms::cosem::CosemSmtpSetupObject object =
    MakeSmtpSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSmtpSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 4u, 0u, 255u);
  const SmtpSetupBuffers b = MakeSampleSmtpSetup();
  dlms::cosem::CosemSmtpSetupObject object(
    name, b.smtpServer, b.smtpServerPort, b.userName,
    b.loginPassword, b.sender, b.receivers,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSmtpSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct GsmDiagnosticBuffers
{
  dlms::cosem::CosemByteBuffer operatorName;
  dlms::cosem::CosemByteBuffer status;
  dlms::cosem::CosemByteBuffer circuitSwitchedStatus;
  dlms::cosem::CosemByteBuffer packetSwitchedStatus;
  dlms::cosem::CosemByteBuffer cellInfo;
  dlms::cosem::CosemByteBuffer adjacentCells;
  dlms::cosem::CosemByteBuffer captureTime;
};

GsmDiagnosticBuffers MakeSampleGsmDiagnostic()
{
  GsmDiagnosticBuffers b;
  // octet-string "MTS" (operator name)
  b.operatorName = BytesFromList({
    0x09u, 0x03u, 0x4Du, 0x54u, 0x53u});
  // enum 1 (registered_home)
  b.status = BytesFromList({0x16u, 0x01u});
  // enum 0 (inactive)
  b.circuitSwitchedStatus = BytesFromList({0x16u, 0x00u});
  // enum 1 (gprs)
  b.packetSwitchedStatus = BytesFromList({0x16u, 0x01u});
  // structure(7): cell_id=double-long-unsigned 0x00012345, location_id=long-unsigned 0x1234,
  //               signal_quality=unsigned 12, ber=unsigned 0,
  //               mcc=long-unsigned 250, mnc=long-unsigned 1,
  //               channel_number=double-long-unsigned 1900
  b.cellInfo = BytesFromList({
    0x02u, 0x07u,
      0x06u, 0x00u, 0x01u, 0x23u, 0x45u,                 // cell_id
      0x12u, 0x12u, 0x34u,                               // location_id
      0x11u, 0x0Cu,                                      // signal_quality
      0x11u, 0x00u,                                      // ber
      0x12u, 0x00u, 0xFAu,                               // mcc (250)
      0x12u, 0x00u, 0x01u,                               // mnc
      0x06u, 0x00u, 0x00u, 0x07u, 0x6Cu});               // channel_number (1900)
  // array(1) of structure(2): cell_id, signal_quality
  b.adjacentCells = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x06u, 0x00u, 0x01u, 0x23u, 0x46u,
        0x11u, 0x0Au});
  // octet-string(12) date_time placeholder
  b.captureTime = BytesFromList({
    0x09u, 0x0Cu,
      0x07u, 0xE7u, 0x06u, 0x0Fu, 0x05u,                 // year/month/day/dow
      0x0Cu, 0x00u, 0x00u, 0x00u,                        // hh:mm:ss:hundredths
      0x80u, 0x00u, 0x00u});                             // deviation, clock_status
  return b;
}

dlms::cosem::CosemGsmDiagnosticObject MakeGsmDiagnosticObject(
  const dlms::cosem::CosemLogicalName& name,
  const GsmDiagnosticBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemGsmDiagnosticObject(
    name, b.operatorName, b.status, b.circuitSwitchedStatus,
    b.packetSwitchedStatus, b.cellInfo, b.adjacentCells,
    b.captureTime, access);
}

} // namespace

TEST(CosemGsmDiagnosticObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 6u, 0u, 255u);
  const GsmDiagnosticBuffers b = MakeSampleGsmDiagnostic();
  dlms::cosem::CosemGsmDiagnosticObject object =
    MakeGsmDiagnosticObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(47u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemGsmDiagnosticObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.operatorName, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.circuitSwitchedStatus, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.packetSwitchedStatus, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.cellInfo, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.adjacentCells, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.captureTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(9u, out));
}

TEST(CosemGsmDiagnosticObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 6u, 0u, 255u);
  const GsmDiagnosticBuffers b = MakeSampleGsmDiagnostic();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x16u, 0x05u});

  dlms::cosem::CosemGsmDiagnosticObject writable =
    MakeGsmDiagnosticObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.OperatorName());
  EXPECT_EQ(replacement, writable.Status());
  EXPECT_EQ(replacement, writable.CircuitSwitchedStatus());
  EXPECT_EQ(replacement, writable.PacketSwitchedStatus());
  EXPECT_EQ(replacement, writable.CellInfo());
  EXPECT_EQ(replacement, writable.AdjacentCells());
  EXPECT_EQ(replacement, writable.CaptureTime());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemGsmDiagnosticObject readOnly =
    MakeGsmDiagnosticObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.operatorName, readOnly.OperatorName());
  EXPECT_EQ(b.status, readOnly.Status());
  EXPECT_EQ(b.circuitSwitchedStatus, readOnly.CircuitSwitchedStatus());
  EXPECT_EQ(b.packetSwitchedStatus, readOnly.PacketSwitchedStatus());
  EXPECT_EQ(b.cellInfo, readOnly.CellInfo());
  EXPECT_EQ(b.adjacentCells, readOnly.AdjacentCells());
  EXPECT_EQ(b.captureTime, readOnly.CaptureTime());
}

TEST(CosemGsmDiagnosticObject, AllMethodsReturnMethodNotFound)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 6u, 0u, 255u);
  const GsmDiagnosticBuffers b = MakeSampleGsmDiagnostic();
  dlms::cosem::CosemGsmDiagnosticObject object =
    MakeGsmDiagnosticObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // IEC 62056-6-2 ED4 (2021) §5.6.8 and DLMS UA Blue Book Ed. 12.1
  // §5.6.8 define class_id=47, version=0 with NO specific methods
  // (the "Specific methods | m/o" column is empty). Earlier revisions
  // of this implementation surfaced a phantom "reset" method (id 1)
  // that the spec never defines; every method id is MethodNotFound.
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 5u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemGsmDiagnosticObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 6u, 0u, 255u);
  const GsmDiagnosticBuffers b = MakeSampleGsmDiagnostic();
  dlms::cosem::CosemGsmDiagnosticObject object(
    name, b.operatorName, b.status, b.circuitSwitchedStatus,
    b.packetSwitchedStatus, b.cellInfo, b.adjacentCells,
    b.captureTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemGsmDiagnosticObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct IecTwistedPairSetupBuffers
{
  dlms::cosem::CosemByteBuffer secondaryAddress;
  dlms::cosem::CosemByteBuffer primaryAddressList;
  dlms::cosem::CosemByteBuffer tabiList;
  dlms::cosem::CosemByteBuffer fatalError;
};

IecTwistedPairSetupBuffers MakeSampleIecTwistedPairSetup()
{
  IecTwistedPairSetupBuffers b;
  // octet-string(6): ADS of the secondary station
  b.secondaryAddress = BytesFromList({
    0x09u, 0x06u,
      0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u});
  // primary_address_list: array(2) of octet-string(1)
  b.primaryAddressList = BytesFromList({
    0x01u, 0x02u,
      0x09u, 0x01u, 0x11u,
      0x09u, 0x01u, 0x12u});
  // tabi_list: array(2) of integer
  b.tabiList = BytesFromList({
    0x01u, 0x02u,
      0x0Fu, 0x01u,
      0x0Fu, 0x02u});
  // fatal_error: enum (0 = No-error)
  b.fatalError = BytesFromList({0x16u, 0x00u});
  return b;
}

dlms::cosem::CosemIecTwistedPairSetupObject
MakeIecTwistedPairSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const IecTwistedPairSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIecTwistedPairSetupObject(
    name,
    b.secondaryAddress,
    b.primaryAddressList,
    b.tabiList,
    b.fatalError,
    access);
}

} // namespace

TEST(CosemIecTwistedPairSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 1u, 255u);
  const IecTwistedPairSetupBuffers b = MakeSampleIecTwistedPairSetup();
  dlms::cosem::CosemIecTwistedPairSetupObject object =
    MakeIecTwistedPairSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(24u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIecTwistedPairSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  // IEC 62056-6-2 ED4 (2021) §4.7.3 and DLMS UA Blue Book Ed. 12.1
  // §4.7.3 define five attributes: logical_name, secondary_address,
  // primary_address_list, tabi_list, fatal_error.
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.secondaryAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.primaryAddressList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.tabiList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.fatalError, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));
}

TEST(CosemIecTwistedPairSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 1u, 255u);
  const IecTwistedPairSetupBuffers b = MakeSampleIecTwistedPairSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x09u, 0x01u, 0x21u});
  const dlms::cosem::CosemByteBuffer fatalReplacement =
    BytesFromList({0x16u, 0x01u});

  dlms::cosem::CosemIecTwistedPairSetupObject writable =
    MakeIecTwistedPairSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.SecondaryAddress());
  EXPECT_EQ(replacement, writable.PrimaryAddressList());
  EXPECT_EQ(replacement, writable.TabiList());
  // fatal_error is server-managed; writes are always rejected even
  // when the caller asked for ReadAndWrite.
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(5u, fatalReplacement));
  EXPECT_EQ(b.fatalError, writable.FatalError());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemIecTwistedPairSetupObject readOnly =
    MakeIecTwistedPairSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(5u, fatalReplacement));
  EXPECT_EQ(b.secondaryAddress, readOnly.SecondaryAddress());
  EXPECT_EQ(b.primaryAddressList, readOnly.PrimaryAddressList());
  EXPECT_EQ(b.tabiList, readOnly.TabiList());
  EXPECT_EQ(b.fatalError, readOnly.FatalError());
}

TEST(CosemIecTwistedPairSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 1u, 255u);
  const IecTwistedPairSetupBuffers b = MakeSampleIecTwistedPairSetup();
  dlms::cosem::CosemIecTwistedPairSetupObject object =
    MakeIecTwistedPairSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemIecTwistedPairSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 1u, 255u);
  const IecTwistedPairSetupBuffers b = MakeSampleIecTwistedPairSetup();
  dlms::cosem::CosemIecTwistedPairSetupObject object(
    name,
    b.secondaryAddress,
    b.primaryAddressList,
    b.tabiList,
    b.fatalError,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    dlms::cosem::CosemIecTwistedPairSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct MBusSlavePortSetupBuffers
{
  dlms::cosem::CosemByteBuffer defaultBaud;
  dlms::cosem::CosemByteBuffer availableBaud;
  dlms::cosem::CosemByteBuffer status;
  dlms::cosem::CosemByteBuffer mbusPortReference;
};

MBusSlavePortSetupBuffers MakeSampleMBusSlavePortSetup()
{
  MBusSlavePortSetupBuffers b;
  // enum 5 (9600 bps)
  b.defaultBaud = BytesFromList({0x16u, 0x05u});
  // enum 7 (19200 bps)
  b.availableBaud = BytesFromList({0x16u, 0x07u});
  // enum 0 (synchronized)
  b.status = BytesFromList({0x16u, 0x00u});
  // octet-string(6) logical name 0.0.22.0.0.255 (IEC HDLC setup)
  b.mbusPortReference = BytesFromList({
    0x09u, 0x06u,
      0x00u, 0x00u, 0x16u, 0x00u, 0x00u, 0xFFu});
  return b;
}

dlms::cosem::CosemMBusSlavePortSetupObject
MakeMBusSlavePortSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const MBusSlavePortSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemMBusSlavePortSetupObject(
    name, b.defaultBaud, b.availableBaud, b.status,
    b.mbusPortReference, access);
}

} // namespace

TEST(CosemMBusSlavePortSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 0u, 0u, 255u);
  const MBusSlavePortSetupBuffers b = MakeSampleMBusSlavePortSetup();
  dlms::cosem::CosemMBusSlavePortSetupObject object =
    MakeMBusSlavePortSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(25u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemMBusSlavePortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.defaultBaud, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.availableBaud, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.mbusPortReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));
}

TEST(CosemMBusSlavePortSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 0u, 0u, 255u);
  const MBusSlavePortSetupBuffers b = MakeSampleMBusSlavePortSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x16u, 0x03u});

  dlms::cosem::CosemMBusSlavePortSetupObject writable =
    MakeMBusSlavePortSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.DefaultBaud());
  EXPECT_EQ(replacement, writable.AvailableBaud());
  EXPECT_EQ(replacement, writable.Status());
  EXPECT_EQ(replacement, writable.MBusPortReference());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemMBusSlavePortSetupObject readOnly =
    MakeMBusSlavePortSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.defaultBaud, readOnly.DefaultBaud());
  EXPECT_EQ(b.availableBaud, readOnly.AvailableBaud());
  EXPECT_EQ(b.status, readOnly.Status());
  EXPECT_EQ(b.mbusPortReference, readOnly.MBusPortReference());
}

TEST(CosemMBusSlavePortSetupObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 0u, 0u, 255u);
  const MBusSlavePortSetupBuffers b = MakeSampleMBusSlavePortSetup();
  dlms::cosem::CosemMBusSlavePortSetupObject object =
    MakeMBusSlavePortSetupObject(
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

TEST(CosemMBusSlavePortSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 0u, 0u, 255u);
  const MBusSlavePortSetupBuffers b = MakeSampleMBusSlavePortSetup();
  dlms::cosem::CosemMBusSlavePortSetupObject object(
    name, b.defaultBaud, b.availableBaud, b.status,
    b.mbusPortReference,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemMBusSlavePortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct Ipv6SetupBuffers
{
  dlms::cosem::CosemByteBuffer dataLinkLayerReference;
  dlms::cosem::CosemByteBuffer addressConfigMode;
  dlms::cosem::CosemByteBuffer unicastIpAddress;
  dlms::cosem::CosemByteBuffer multicastIpAddress;
  dlms::cosem::CosemByteBuffer gatewayIpAddress;
  dlms::cosem::CosemByteBuffer primaryDnsAddress;
  dlms::cosem::CosemByteBuffer secondaryDnsAddress;
  dlms::cosem::CosemByteBuffer trafficClass;
  dlms::cosem::CosemByteBuffer neighborDiscoverySetup;
};

Ipv6SetupBuffers MakeSampleIpv6Setup()
{
  Ipv6SetupBuffers b;
  // octet-string(6) LN 0.0.27.0.0.255 (TCP-UDP setup as link layer)
  b.dataLinkLayerReference = BytesFromList({
    0x09u, 0x06u,
      0x00u, 0x00u, 0x1Bu, 0x00u, 0x00u, 0xFFu});
  // enum 2 (auto-config DHCPv6)
  b.addressConfigMode = BytesFromList({0x16u, 0x02u});
  // array(1) of octet-string(16): fe80::1
  b.unicastIpAddress = BytesFromList({
    0x01u, 0x01u,
      0x09u, 0x10u,
        0xFEu, 0x80u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x01u});
  // array(1) of octet-string(16): ff02::1 (all-nodes multicast)
  b.multicastIpAddress = BytesFromList({
    0x01u, 0x01u,
      0x09u, 0x10u,
        0xFFu, 0x02u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x01u});
  // octet-string(16) gateway fe80::2
  b.gatewayIpAddress = BytesFromList({
    0x09u, 0x10u,
      0xFEu, 0x80u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
      0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x02u});
  // octet-string(16) primary DNS 2001:4860:4860::8888
  b.primaryDnsAddress = BytesFromList({
    0x09u, 0x10u,
      0x20u, 0x01u, 0x48u, 0x60u, 0x48u, 0x60u, 0x00u, 0x00u,
      0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x88u, 0x88u});
  // octet-string(16) secondary DNS 2001:4860:4860::8844
  b.secondaryDnsAddress = BytesFromList({
    0x09u, 0x10u,
      0x20u, 0x01u, 0x48u, 0x60u, 0x48u, 0x60u, 0x00u, 0x00u,
      0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x88u, 0x44u});
  // unsigned 0 (default traffic class)
  b.trafficClass = BytesFromList({0x11u, 0x00u});
  // array(1) of structure(4): {max_retry=3, retry_timer=1000,
  //                            send_period=60, server_address=fe80::2}
  b.neighborDiscoverySetup = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x04u,
        0x11u, 0x03u,                                     // max_retry
        0x12u, 0x03u, 0xE8u,                              // retry_timer
        0x06u, 0x00u, 0x00u, 0x00u, 0x3Cu,                // send_period
        0x09u, 0x10u,
          0xFEu, 0x80u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
          0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x02u});
  return b;
}

dlms::cosem::CosemIpv6SetupObject MakeIpv6SetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const Ipv6SetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIpv6SetupObject(
    name, b.dataLinkLayerReference, b.addressConfigMode,
    b.unicastIpAddress, b.multicastIpAddress, b.gatewayIpAddress,
    b.primaryDnsAddress, b.secondaryDnsAddress, b.trafficClass,
    b.neighborDiscoverySetup, access);
}

} // namespace

TEST(CosemIpv6SetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 7u, 0u, 255u);
  const Ipv6SetupBuffers b = MakeSampleIpv6Setup();
  dlms::cosem::CosemIpv6SetupObject object =
    MakeIpv6SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(48u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIpv6SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.dataLinkLayerReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.addressConfigMode, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.unicastIpAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.multicastIpAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.gatewayIpAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.primaryDnsAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.secondaryDnsAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.trafficClass, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(10u, out));
  EXPECT_EQ(b.neighborDiscoverySetup, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(11u, out));
}

TEST(CosemIpv6SetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 7u, 0u, 255u);
  const Ipv6SetupBuffers b = MakeSampleIpv6Setup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemIpv6SetupObject writable =
    MakeIpv6SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.DataLinkLayerReference());
  EXPECT_EQ(replacement, writable.AddressConfigMode());
  EXPECT_EQ(replacement, writable.UnicastIpAddress());
  EXPECT_EQ(replacement, writable.MulticastIpAddress());
  EXPECT_EQ(replacement, writable.GatewayIpAddress());
  EXPECT_EQ(replacement, writable.PrimaryDnsAddress());
  EXPECT_EQ(replacement, writable.SecondaryDnsAddress());
  EXPECT_EQ(replacement, writable.TrafficClass());
  EXPECT_EQ(replacement, writable.NeighborDiscoverySetup());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemIpv6SetupObject readOnly =
    MakeIpv6SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.dataLinkLayerReference, readOnly.DataLinkLayerReference());
  EXPECT_EQ(b.addressConfigMode, readOnly.AddressConfigMode());
  EXPECT_EQ(b.unicastIpAddress, readOnly.UnicastIpAddress());
  EXPECT_EQ(b.multicastIpAddress, readOnly.MulticastIpAddress());
  EXPECT_EQ(b.gatewayIpAddress, readOnly.GatewayIpAddress());
  EXPECT_EQ(b.primaryDnsAddress, readOnly.PrimaryDnsAddress());
  EXPECT_EQ(b.secondaryDnsAddress, readOnly.SecondaryDnsAddress());
  EXPECT_EQ(b.trafficClass, readOnly.TrafficClass());
  EXPECT_EQ(b.neighborDiscoverySetup, readOnly.NeighborDiscoverySetup());
}

TEST(CosemIpv6SetupObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 7u, 0u, 255u);
  const Ipv6SetupBuffers b = MakeSampleIpv6Setup();
  dlms::cosem::CosemIpv6SetupObject object =
    MakeIpv6SetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {3u, 4u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemIpv6SetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 25u, 7u, 0u, 255u);
  const Ipv6SetupBuffers b = MakeSampleIpv6Setup();
  dlms::cosem::CosemIpv6SetupObject object(
    name, b.dataLinkLayerReference, b.addressConfigMode,
    b.unicastIpAddress, b.multicastIpAddress, b.gatewayIpAddress,
    b.primaryDnsAddress, b.secondaryDnsAddress, b.trafficClass,
    b.neighborDiscoverySetup,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemIpv6SetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct UtilityTablesBuffers
{
  dlms::cosem::CosemByteBuffer tableId;
  dlms::cosem::CosemByteBuffer length;
  dlms::cosem::CosemByteBuffer buffer;
};

UtilityTablesBuffers MakeSampleUtilityTables()
{
  UtilityTablesBuffers b;
  // long-unsigned 0x0010 (table id 16)
  b.tableId = BytesFromList({0x12u, 0x00u, 0x10u});
  // double-long-unsigned 0x00000020 (length 32 bytes)
  b.length = BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x20u});
  // octet-string(4) sample table payload
  b.buffer = BytesFromList({
    0x09u, 0x04u,
      0xDEu, 0xADu, 0xBEu, 0xEFu});
  return b;
}

dlms::cosem::CosemUtilityTablesObject MakeUtilityTablesObject(
  const dlms::cosem::CosemLogicalName& name,
  const UtilityTablesBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemUtilityTablesObject(
    name, b.tableId, b.length, b.buffer, access);
}

} // namespace

TEST(CosemUtilityTablesObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 65u, 1u, 0u, 255u);
  const UtilityTablesBuffers b = MakeSampleUtilityTables();
  dlms::cosem::CosemUtilityTablesObject object =
    MakeUtilityTablesObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(26u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemUtilityTablesObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.tableId, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.length, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.buffer, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
}

TEST(CosemUtilityTablesObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 65u, 1u, 0u, 255u);
  const UtilityTablesBuffers b = MakeSampleUtilityTables();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemUtilityTablesObject writable =
    MakeUtilityTablesObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.TableId());
  EXPECT_EQ(replacement, writable.Length());
  EXPECT_EQ(replacement, writable.Buffer());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemUtilityTablesObject readOnly =
    MakeUtilityTablesObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.tableId, readOnly.TableId());
  EXPECT_EQ(b.length, readOnly.Length());
  EXPECT_EQ(b.buffer, readOnly.Buffer());
}

TEST(CosemUtilityTablesObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 65u, 1u, 0u, 255u);
  const UtilityTablesBuffers b = MakeSampleUtilityTables();
  dlms::cosem::CosemUtilityTablesObject object =
    MakeUtilityTablesObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemUtilityTablesObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 65u, 1u, 0u, 255u);
  const UtilityTablesBuffers b = MakeSampleUtilityTables();
  dlms::cosem::CosemUtilityTablesObject object(
    name, b.tableId, b.length, b.buffer,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemUtilityTablesObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct SensorManagerBuffers
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
};

SensorManagerBuffers MakeSampleSensorManager()
{
  SensorManagerBuffers b;
  // enum 1 (operational)
  b.status = BytesFromList({0x16u, 0x01u});
  // octet-string(8) serial number
  b.serialNumber = BytesFromList({
    0x09u, 0x08u,
      0x53u, 0x4Eu, 0x30u, 0x30u, 0x30u, 0x30u, 0x30u, 0x31u});
  // octet-string(3) device type 'WAT'
  b.deviceType = BytesFromList({0x09u, 0x03u, 0x57u, 0x41u, 0x54u});
  // long-unsigned 0x1234 (manufacturer id)
  b.manufacturerId = BytesFromList({0x12u, 0x12u, 0x34u});
  // octet-string(4) fw v1.0
  b.firmwareVersion = BytesFromList({
    0x09u, 0x04u, 0x31u, 0x2Eu, 0x30u, 0x00u});
  // octet-string(4) metrology fw v2.0
  b.metrologyFirmwareVersion = BytesFromList({
    0x09u, 0x04u, 0x32u, 0x2Eu, 0x30u, 0x00u});
  // octet-string(6) driver id
  b.driver = BytesFromList({
    0x09u, 0x06u, 0x44u, 0x52u, 0x49u, 0x56u, 0x45u, 0x52u});
  // array(0) communication descriptor
  b.communicationDesc = BytesFromList({0x01u, 0x00u});
  // array(0) setup descriptor
  b.setupDesc = BytesFromList({0x01u, 0x00u});
  // array(0) measurement descriptor
  b.measurementDesc = BytesFromList({0x01u, 0x00u});
  return b;
}

dlms::cosem::CosemSensorManagerObject MakeSensorManagerObject(
  const dlms::cosem::CosemLogicalName& name,
  const SensorManagerBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemSensorManagerObject(
    name, b.status, b.serialNumber, b.deviceType, b.manufacturerId,
    b.firmwareVersion, b.metrologyFirmwareVersion, b.driver,
    b.communicationDesc, b.setupDesc, b.measurementDesc, access);
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
  EXPECT_EQ(b.status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.serialNumber, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.deviceType, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.manufacturerId, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.firmwareVersion, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.metrologyFirmwareVersion, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.driver, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.communicationDesc, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(10u, out));
  EXPECT_EQ(b.setupDesc, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(11u, out));
  EXPECT_EQ(b.measurementDesc, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(12u, out));
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
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.Status());
  EXPECT_EQ(replacement, writable.SerialNumber());
  EXPECT_EQ(replacement, writable.DeviceType());
  EXPECT_EQ(replacement, writable.ManufacturerId());
  EXPECT_EQ(replacement, writable.FirmwareVersion());
  EXPECT_EQ(replacement, writable.MetrologyFirmwareVersion());
  EXPECT_EQ(replacement, writable.Driver());
  EXPECT_EQ(replacement, writable.CommunicationDesc());
  EXPECT_EQ(replacement, writable.SetupDesc());
  EXPECT_EQ(replacement, writable.MeasurementDesc());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemSensorManagerObject readOnly =
    MakeSensorManagerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.status, readOnly.Status());
  EXPECT_EQ(b.serialNumber, readOnly.SerialNumber());
  EXPECT_EQ(b.deviceType, readOnly.DeviceType());
  EXPECT_EQ(b.manufacturerId, readOnly.ManufacturerId());
  EXPECT_EQ(b.firmwareVersion, readOnly.FirmwareVersion());
  EXPECT_EQ(b.metrologyFirmwareVersion,
            readOnly.MetrologyFirmwareVersion());
  EXPECT_EQ(b.driver, readOnly.Driver());
  EXPECT_EQ(b.communicationDesc, readOnly.CommunicationDesc());
  EXPECT_EQ(b.setupDesc, readOnly.SetupDesc());
  EXPECT_EQ(b.measurementDesc, readOnly.MeasurementDesc());
}

TEST(CosemSensorManagerObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 96u, 50u, 0u, 255u);
  const SensorManagerBuffers b = MakeSampleSensorManager();
  dlms::cosem::CosemSensorManagerObject object =
    MakeSensorManagerObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
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
    name, b.status, b.serialNumber, b.deviceType, b.manufacturerId,
    b.firmwareVersion, b.metrologyFirmwareVersion, b.driver,
    b.communicationDesc, b.setupDesc, b.measurementDesc,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSensorManagerObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct ArbitratorBuffers
{
  dlms::cosem::CosemByteBuffer actions;
  dlms::cosem::CosemByteBuffer permissionsTable;
  dlms::cosem::CosemByteBuffer weightingsTable;
  dlms::cosem::CosemByteBuffer mostRecentRequestsTable;
  dlms::cosem::CosemByteBuffer lastOutcome;
};

ArbitratorBuffers MakeSampleArbitrator()
{
  ArbitratorBuffers b;
  // array(1) of structure(2): {script_logical_name 0.0.10.0.100.255,
  //                            script_selector long-unsigned 1}
  b.actions = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x09u, 0x06u,
          0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
        0x12u, 0x00u, 0x01u});
  // array(1) of bit-string(8): 0b10000000 (actor 0 may run script 0)
  b.permissionsTable = BytesFromList({
    0x01u, 0x01u,
      0x04u, 0x08u, 0x80u});
  // array(1) of array(1) of long-unsigned: {{ 0x0001 }}
  b.weightingsTable = BytesFromList({
    0x01u, 0x01u,
      0x01u, 0x01u,
        0x12u, 0x00u, 0x01u});
  // array(1) of bit-string(8): 0b00000000 (no recent requests)
  b.mostRecentRequestsTable = BytesFromList({
    0x01u, 0x01u,
      0x04u, 0x08u, 0x00u});
  // unsigned 0 (no script ran yet)
  b.lastOutcome = BytesFromList({0x11u, 0x00u});
  return b;
}

dlms::cosem::CosemArbitratorObject MakeArbitratorObject(
  const dlms::cosem::CosemLogicalName& name,
  const ArbitratorBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemArbitratorObject(
    name, b.actions, b.permissionsTable, b.weightingsTable,
    b.mostRecentRequestsTable, b.lastOutcome, access);
}

} // namespace

TEST(CosemArbitratorObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  const ArbitratorBuffers b = MakeSampleArbitrator();
  dlms::cosem::CosemArbitratorObject object =
    MakeArbitratorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(68u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemArbitratorObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.actions, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.permissionsTable, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.weightingsTable, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.mostRecentRequestsTable, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.lastOutcome, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemArbitratorObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  const ArbitratorBuffers b = MakeSampleArbitrator();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemArbitratorObject writable =
    MakeArbitratorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.Actions());
  EXPECT_EQ(replacement, writable.PermissionsTable());
  EXPECT_EQ(replacement, writable.WeightingsTable());
  EXPECT_EQ(replacement, writable.MostRecentRequestsTable());
  EXPECT_EQ(replacement, writable.LastOutcome());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemArbitratorObject readOnly =
    MakeArbitratorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.actions, readOnly.Actions());
  EXPECT_EQ(b.permissionsTable, readOnly.PermissionsTable());
  EXPECT_EQ(b.weightingsTable, readOnly.WeightingsTable());
  EXPECT_EQ(b.mostRecentRequestsTable,
            readOnly.MostRecentRequestsTable());
  EXPECT_EQ(b.lastOutcome, readOnly.LastOutcome());
}

TEST(CosemArbitratorObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  const ArbitratorBuffers b = MakeSampleArbitrator();
  dlms::cosem::CosemArbitratorObject object =
    MakeArbitratorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {3u, 4u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemArbitratorObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
  const ArbitratorBuffers b = MakeSampleArbitrator();
  dlms::cosem::CosemArbitratorObject object(
    name, b.actions, b.permissionsTable, b.weightingsTable,
    b.mostRecentRequestsTable, b.lastOutcome,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemArbitratorObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct StatusMappingBuffers
{
  dlms::cosem::CosemByteBuffer statusWord;
  dlms::cosem::CosemByteBuffer mappings;
};

StatusMappingBuffers MakeSampleStatusMapping()
{
  StatusMappingBuffers b;
  // bit-string(8) 0b10100000 (raw status word)
  b.statusWord = BytesFromList({0x04u, 0x08u, 0xA0u});
  // array(2) of structure(2): {status_value bit-string(8), mapped_value bit-string(8)}
  b.mappings = BytesFromList({
    0x01u, 0x02u,
      0x02u, 0x02u,
        0x04u, 0x08u, 0x80u,
        0x04u, 0x08u, 0x01u,
      0x02u, 0x02u,
        0x04u, 0x08u, 0x20u,
        0x04u, 0x08u, 0x02u});
  return b;
}

dlms::cosem::CosemStatusMappingObject MakeStatusMappingObject(
  const dlms::cosem::CosemLogicalName& name,
  const StatusMappingBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemStatusMappingObject(
    name, b.statusWord, b.mappings, access);
}

} // namespace

TEST(CosemStatusMappingObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 5u, 0u, 255u);
  const StatusMappingBuffers b = MakeSampleStatusMapping();
  dlms::cosem::CosemStatusMappingObject object =
    MakeStatusMappingObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(63u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemStatusMappingObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.statusWord, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.mappings, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(4u, out));
}

TEST(CosemStatusMappingObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 5u, 0u, 255u);
  const StatusMappingBuffers b = MakeSampleStatusMapping();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemStatusMappingObject writable =
    MakeStatusMappingObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.StatusWord());
  EXPECT_EQ(replacement, writable.Mappings());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemStatusMappingObject readOnly =
    MakeStatusMappingObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.statusWord, readOnly.StatusWord());
  EXPECT_EQ(b.mappings, readOnly.Mappings());
}

TEST(CosemStatusMappingObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 5u, 0u, 255u);
  const StatusMappingBuffers b = MakeSampleStatusMapping();
  dlms::cosem::CosemStatusMappingObject object =
    MakeStatusMappingObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemStatusMappingObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 5u, 0u, 255u);
  const StatusMappingBuffers b = MakeSampleStatusMapping();
  dlms::cosem::CosemStatusMappingObject object(
    name, b.statusWord, b.mappings,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemStatusMappingObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct ParameterMonitorBuffers
{
  dlms::cosem::CosemByteBuffer changedParameter;
  dlms::cosem::CosemByteBuffer captureTime;
  dlms::cosem::CosemByteBuffer parameters;
};

ParameterMonitorBuffers MakeSampleParameterMonitor()
{
  ParameterMonitorBuffers b;
  // structure(4):
  //   long-unsigned 3 (Register class id),
  //   octet-string(6) 1.0.32.7.0.255,
  //   integer 2 (value attribute),
  //   double-long-unsigned 230 (changed value)
  b.changedParameter = BytesFromList({
    0x02u, 0x04u,
      0x12u, 0x00u, 0x03u,
      0x09u, 0x06u,
        0x01u, 0x00u, 0x20u, 0x07u, 0x00u, 0xFFu,
      0x0Fu, 0x02u,
      0x06u, 0x00u, 0x00u, 0x00u, 0xE6u});
  // octet-string(12) date-time 2026-06-15 12:00:00 (no deviation)
  b.captureTime = BytesFromList({
    0x09u, 0x0Cu,
      0x07u, 0xEAu, 0x06u, 0x0Fu, 0x01u,
      0x0Cu, 0x00u, 0x00u, 0x00u,
      0x80u, 0x00u, 0x00u});
  // array(1) of structure(3):
  //   long-unsigned 3, octet-string(6) 1.0.32.7.0.255, integer 2
  b.parameters = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x03u,
        0x12u, 0x00u, 0x03u,
        0x09u, 0x06u,
          0x01u, 0x00u, 0x20u, 0x07u, 0x00u, 0xFFu,
        0x0Fu, 0x02u});
  return b;
}

dlms::cosem::CosemParameterMonitorObject MakeParameterMonitorObject(
  const dlms::cosem::CosemLogicalName& name,
  const ParameterMonitorBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemParameterMonitorObject(
    name, b.changedParameter, b.captureTime, b.parameters, access);
}

} // namespace

TEST(CosemParameterMonitorObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 2u, 0u, 255u);
  const ParameterMonitorBuffers b = MakeSampleParameterMonitor();
  dlms::cosem::CosemParameterMonitorObject object =
    MakeParameterMonitorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(65u, object.Descriptor().key.classId);
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
}

TEST(CosemParameterMonitorObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 2u, 0u, 255u);
  const ParameterMonitorBuffers b = MakeSampleParameterMonitor();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemParameterMonitorObject writable =
    MakeParameterMonitorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u}) {
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
}

TEST(CosemParameterMonitorObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 2u, 0u, 255u);
  const ParameterMonitorBuffers b = MakeSampleParameterMonitor();
  dlms::cosem::CosemParameterMonitorObject object =
    MakeParameterMonitorObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {3u, 4u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemParameterMonitorObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 16u, 2u, 0u, 255u);
  const ParameterMonitorBuffers b = MakeSampleParameterMonitor();
  dlms::cosem::CosemParameterMonitorObject object(
    name, b.changedParameter, b.captureTime, b.parameters,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemParameterMonitorObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct CompactDataBuffers
{
  dlms::cosem::CosemByteBuffer buffer;
  dlms::cosem::CosemByteBuffer captureObjects;
  dlms::cosem::CosemByteBuffer templateId;
  dlms::cosem::CosemByteBuffer templateDescription;
  dlms::cosem::CosemByteBuffer captureMethod;
};

CompactDataBuffers MakeSampleCompactData()
{
  CompactDataBuffers b;
  // octet-string(4) raw compact buffer
  b.buffer = BytesFromList({0x09u, 0x04u, 0xDEu, 0xADu, 0xBEu, 0xEFu});
  // array(1) of structure(4):
  //   long-unsigned 3 (Register), octet-string(6) 1.0.32.7.0.255,
  //   integer 2, long-unsigned 0
  b.captureObjects = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x04u,
        0x12u, 0x00u, 0x03u,
        0x09u, 0x06u,
          0x01u, 0x00u, 0x20u, 0x07u, 0x00u, 0xFFu,
        0x0Fu, 0x02u,
        0x12u, 0x00u, 0x00u});
  // unsigned 1
  b.templateId = BytesFromList({0x11u, 0x01u});
  // octet-string(2) A-XDR template description (long-unsigned tag)
  b.templateDescription = BytesFromList({0x09u, 0x02u, 0x12u, 0x00u});
  // enum 1 (invoke)
  b.captureMethod = BytesFromList({0x16u, 0x01u});
  return b;
}

dlms::cosem::CosemCompactDataObject MakeCompactDataObject(
  const dlms::cosem::CosemLogicalName& name,
  const CompactDataBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemCompactDataObject(
    name, b.buffer, b.captureObjects, b.templateId,
    b.templateDescription, b.captureMethod, access);
}

} // namespace

TEST(CosemCompactDataObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 66u, 0u, 1u, 255u);
  const CompactDataBuffers b = MakeSampleCompactData();
  dlms::cosem::CosemCompactDataObject object =
    MakeCompactDataObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(62u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemCompactDataObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.buffer, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.captureObjects, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.templateId, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.templateDescription, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.captureMethod, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemCompactDataObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 66u, 0u, 1u, 255u);
  const CompactDataBuffers b = MakeSampleCompactData();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemCompactDataObject writable =
    MakeCompactDataObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.Buffer());
  EXPECT_EQ(replacement, writable.CaptureObjects());
  EXPECT_EQ(replacement, writable.TemplateId());
  EXPECT_EQ(replacement, writable.TemplateDescription());
  EXPECT_EQ(replacement, writable.CaptureMethod());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemCompactDataObject readOnly =
    MakeCompactDataObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.buffer, readOnly.Buffer());
  EXPECT_EQ(b.captureObjects, readOnly.CaptureObjects());
  EXPECT_EQ(b.templateId, readOnly.TemplateId());
  EXPECT_EQ(b.templateDescription, readOnly.TemplateDescription());
  EXPECT_EQ(b.captureMethod, readOnly.CaptureMethod());
}

TEST(CosemCompactDataObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 66u, 0u, 1u, 255u);
  const CompactDataBuffers b = MakeSampleCompactData();
  dlms::cosem::CosemCompactDataObject object =
    MakeCompactDataObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {3u, 4u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemCompactDataObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 66u, 0u, 1u, 255u);
  const CompactDataBuffers b = MakeSampleCompactData();
  dlms::cosem::CosemCompactDataObject object(
    name, b.buffer, b.captureObjects, b.templateId,
    b.templateDescription, b.captureMethod,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemCompactDataObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct DataProtectionBuffers
{
  dlms::cosem::CosemByteBuffer protectionBuffer;
  dlms::cosem::CosemByteBuffer protectionObjectList;
  dlms::cosem::CosemByteBuffer protectionParametersGet;
  dlms::cosem::CosemByteBuffer protectionParametersSet;
  dlms::cosem::CosemByteBuffer requiredProtection;
};

DataProtectionBuffers MakeSampleDataProtection()
{
  DataProtectionBuffers b;
  // octet-string(4) protected payload
  b.protectionBuffer = BytesFromList({0x09u, 0x04u, 0xCAu, 0xFEu, 0xBAu, 0xBEu});
  // array(1) of structure(3): {enum 1, structure(0) {}, octet-string(2) 0xAABB}
  b.protectionObjectList = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x03u,
        0x16u, 0x01u,
        0x02u, 0x00u,
        0x09u, 0x02u, 0xAAu, 0xBBu});
  // array(1) of structure(2): {enum 1, structure(0) {}}
  b.protectionParametersGet = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x16u, 0x01u,
        0x02u, 0x00u});
  // array(1) of structure(2): {enum 2, structure(0) {}}
  b.protectionParametersSet = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x16u, 0x02u,
        0x02u, 0x00u});
  // bit-string(3) 0b111 (authentication + encryption + digital signature)
  b.requiredProtection = BytesFromList({0x04u, 0x03u, 0xE0u});
  return b;
}

dlms::cosem::CosemDataProtectionObject MakeDataProtectionObject(
  const dlms::cosem::CosemLogicalName& name,
  const DataProtectionBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemDataProtectionObject(
    name, b.protectionBuffer, b.protectionObjectList,
    b.protectionParametersGet, b.protectionParametersSet,
    b.requiredProtection, access);
}

} // namespace

TEST(CosemDataProtectionObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 43u, 1u, 0u, 255u);
  const DataProtectionBuffers b = MakeSampleDataProtection();
  dlms::cosem::CosemDataProtectionObject object =
    MakeDataProtectionObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(30u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemDataProtectionObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.protectionBuffer, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.protectionObjectList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.protectionParametersGet, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.protectionParametersSet, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.requiredProtection, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemDataProtectionObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 43u, 1u, 0u, 255u);
  const DataProtectionBuffers b = MakeSampleDataProtection();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemDataProtectionObject writable =
    MakeDataProtectionObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.ProtectionBuffer());
  EXPECT_EQ(replacement, writable.ProtectionObjectList());
  EXPECT_EQ(replacement, writable.ProtectionParametersGet());
  EXPECT_EQ(replacement, writable.ProtectionParametersSet());
  EXPECT_EQ(replacement, writable.RequiredProtection());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemDataProtectionObject readOnly =
    MakeDataProtectionObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.protectionBuffer, readOnly.ProtectionBuffer());
  EXPECT_EQ(b.protectionObjectList, readOnly.ProtectionObjectList());
  EXPECT_EQ(b.protectionParametersGet,
            readOnly.ProtectionParametersGet());
  EXPECT_EQ(b.protectionParametersSet,
            readOnly.ProtectionParametersSet());
  EXPECT_EQ(b.requiredProtection, readOnly.RequiredProtection());
}

TEST(CosemDataProtectionObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 43u, 1u, 0u, 255u);
  const DataProtectionBuffers b = MakeSampleDataProtection();
  dlms::cosem::CosemDataProtectionObject object =
    MakeDataProtectionObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {4u, 5u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemDataProtectionObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 43u, 1u, 0u, 255u);
  const DataProtectionBuffers b = MakeSampleDataProtection();
  dlms::cosem::CosemDataProtectionObject object(
    name, b.protectionBuffer, b.protectionObjectList,
    b.protectionParametersGet, b.protectionParametersSet,
    b.requiredProtection,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemDataProtectionObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct IecLocalPortSetupBuffers
{
  dlms::cosem::CosemByteBuffer defaultMode;
  dlms::cosem::CosemByteBuffer defaultBaud;
  dlms::cosem::CosemByteBuffer proposedBaud;
  dlms::cosem::CosemByteBuffer responseTime;
  dlms::cosem::CosemByteBuffer deviceAddress;
  dlms::cosem::CosemByteBuffer password1;
  dlms::cosem::CosemByteBuffer password2;
  dlms::cosem::CosemByteBuffer password5;
};

IecLocalPortSetupBuffers MakeSampleIecLocalPortSetup()
{
  IecLocalPortSetupBuffers b;
  // enum 0 (mode HDLC)
  b.defaultMode = BytesFromList({0x16u, 0x00u});
  // enum 5 (9600)
  b.defaultBaud = BytesFromList({0x16u, 0x05u});
  // enum 5 (9600)
  b.proposedBaud = BytesFromList({0x16u, 0x05u});
  // enum 0 (20 ms)
  b.responseTime = BytesFromList({0x16u, 0x00u});
  // octet-string(6) device address 0.0.96.1.0.255
  b.deviceAddress = BytesFromList({
    0x09u, 0x06u, 0x00u, 0x00u, 0x60u, 0x01u, 0x00u, 0xFFu});
  // octet-string(8) password 1
  b.password1 = BytesFromList({
    0x09u, 0x08u, 0x31u, 0x31u, 0x31u, 0x31u, 0x31u, 0x31u, 0x31u, 0x31u});
  // octet-string(8) password 2
  b.password2 = BytesFromList({
    0x09u, 0x08u, 0x32u, 0x32u, 0x32u, 0x32u, 0x32u, 0x32u, 0x32u, 0x32u});
  // octet-string(8) password 5
  b.password5 = BytesFromList({
    0x09u, 0x08u, 0x35u, 0x35u, 0x35u, 0x35u, 0x35u, 0x35u, 0x35u, 0x35u});
  return b;
}

dlms::cosem::CosemIecLocalPortSetupObject MakeIecLocalPortSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const IecLocalPortSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemIecLocalPortSetupObject(
    name, b.defaultMode, b.defaultBaud, b.proposedBaud,
    b.responseTime, b.deviceAddress, b.password1, b.password2,
    b.password5, access);
}

} // namespace

TEST(CosemIecLocalPortSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 20u, 0u, 0u, 255u);
  const IecLocalPortSetupBuffers b = MakeSampleIecLocalPortSetup();
  dlms::cosem::CosemIecLocalPortSetupObject object =
    MakeIecLocalPortSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(19u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemIecLocalPortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.defaultMode, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.defaultBaud, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.proposedBaud, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.responseTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.deviceAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.password1, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.password2, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.password5, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, out));
}

TEST(CosemIecLocalPortSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 20u, 0u, 0u, 255u);
  const IecLocalPortSetupBuffers b = MakeSampleIecLocalPortSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemIecLocalPortSetupObject writable =
    MakeIecLocalPortSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.DefaultMode());
  EXPECT_EQ(replacement, writable.DefaultBaud());
  EXPECT_EQ(replacement, writable.ProposedBaud());
  EXPECT_EQ(replacement, writable.ResponseTime());
  EXPECT_EQ(replacement, writable.DeviceAddress());
  EXPECT_EQ(replacement, writable.Password1());
  EXPECT_EQ(replacement, writable.Password2());
  EXPECT_EQ(replacement, writable.Password5());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemIecLocalPortSetupObject readOnly =
    MakeIecLocalPortSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.defaultMode, readOnly.DefaultMode());
  EXPECT_EQ(b.defaultBaud, readOnly.DefaultBaud());
  EXPECT_EQ(b.proposedBaud, readOnly.ProposedBaud());
  EXPECT_EQ(b.responseTime, readOnly.ResponseTime());
  EXPECT_EQ(b.deviceAddress, readOnly.DeviceAddress());
  EXPECT_EQ(b.password1, readOnly.Password1());
  EXPECT_EQ(b.password2, readOnly.Password2());
  EXPECT_EQ(b.password5, readOnly.Password5());
}

TEST(CosemIecLocalPortSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 20u, 0u, 0u, 255u);
  const IecLocalPortSetupBuffers b = MakeSampleIecLocalPortSetup();
  dlms::cosem::CosemIecLocalPortSetupObject object =
    MakeIecLocalPortSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemIecLocalPortSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 20u, 0u, 0u, 255u);
  const IecLocalPortSetupBuffers b = MakeSampleIecLocalPortSetup();
  dlms::cosem::CosemIecLocalPortSetupObject object(
    name, b.defaultMode, b.defaultBaud, b.proposedBaud,
    b.responseTime, b.deviceAddress, b.password1, b.password2,
    b.password5,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemIecLocalPortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct AssociationSnBuffers
{
  dlms::cosem::CosemByteBuffer objectList;
  dlms::cosem::CosemByteBuffer accessRightsList;
  dlms::cosem::CosemByteBuffer securitySetupReference;
  dlms::cosem::CosemByteBuffer userList;
  dlms::cosem::CosemByteBuffer currentUser;
};

AssociationSnBuffers MakeSampleAssociationSn()
{
  AssociationSnBuffers b;
  // array(1) of structure(5): {long-int 0xFA00, long-unsigned 8,
  // unsigned 0, octet-string(6) 0.0.1.0.0.255, structure(0) {}}
  b.objectList = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x05u,
        0x10u, 0xFAu, 0x00u,
        0x12u, 0x00u, 0x08u,
        0x11u, 0x00u,
        0x09u, 0x06u, 0x00u, 0x00u, 0x01u, 0x00u, 0x00u, 0xFFu,
        0x02u, 0x00u});
  // array(0)
  b.accessRightsList = BytesFromList({0x01u, 0x00u});
  // octet-string(6) security setup LN 0.0.43.0.0.255
  b.securitySetupReference = BytesFromList({
    0x09u, 0x06u, 0x00u, 0x00u, 0x2Bu, 0x00u, 0x00u, 0xFFu});
  // array(1) of structure(2): {unsigned 1, visible-string "PUBLIC"}
  b.userList = BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x02u,
        0x11u, 0x01u,
        0x0Au, 0x06u, 0x50u, 0x55u, 0x42u, 0x4Cu, 0x49u, 0x43u});
  // structure(2): {unsigned 1, visible-string "PUBLIC"}
  b.currentUser = BytesFromList({
    0x02u, 0x02u,
      0x11u, 0x01u,
      0x0Au, 0x06u, 0x50u, 0x55u, 0x42u, 0x4Cu, 0x49u, 0x43u});
  return b;
}

dlms::cosem::CosemAssociationSnObject MakeAssociationSnObject(
  const dlms::cosem::CosemLogicalName& name,
  const AssociationSnBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemAssociationSnObject(
    name, b.objectList, b.accessRightsList,
    b.securitySetupReference, b.userList, b.currentUser, access);
}

} // namespace

TEST(CosemAssociationSnObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u);
  const AssociationSnBuffers b = MakeSampleAssociationSn();
  dlms::cosem::CosemAssociationSnObject object =
    MakeAssociationSnObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(12u, object.Descriptor().key.classId);
  EXPECT_EQ(4u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemAssociationSnObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.objectList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.accessRightsList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.securitySetupReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.userList, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.currentUser, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemAssociationSnObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u);
  const AssociationSnBuffers b = MakeSampleAssociationSn();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x2Au});

  dlms::cosem::CosemAssociationSnObject writable =
    MakeAssociationSnObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.ObjectList());
  EXPECT_EQ(replacement, writable.AccessRightsList());
  EXPECT_EQ(replacement, writable.SecuritySetupReference());
  EXPECT_EQ(replacement, writable.UserList());
  EXPECT_EQ(replacement, writable.CurrentUser());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemAssociationSnObject readOnly =
    MakeAssociationSnObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.objectList, readOnly.ObjectList());
  EXPECT_EQ(b.accessRightsList, readOnly.AccessRightsList());
  EXPECT_EQ(b.securitySetupReference,
            readOnly.SecuritySetupReference());
  EXPECT_EQ(b.userList, readOnly.UserList());
  EXPECT_EQ(b.currentUser, readOnly.CurrentUser());
}

TEST(CosemAssociationSnObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u);
  const AssociationSnBuffers b = MakeSampleAssociationSn();
  dlms::cosem::CosemAssociationSnObject object =
    MakeAssociationSnObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  // IEC 62056-6-2 ED4 4.4.3 / Blue Book Ed. 12.1 5.4.5 specific
  // methods: 3 read_by_logicalname, 5 change_secret,
  // 8 reply_to_HLS_authentication, 9 add_user (v3+),
  // 10 remove_user (v3+). Method ids 1, 2, 4, 6, 7 are reserved.
  for (std::uint8_t method : {3u, 5u, 8u, 9u, 10u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {1u, 2u, 4u, 6u, 7u, 11u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemAssociationSnObject, Version0DoesNotExposeSecuritySetupOrUserAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u);
  const AssociationSnBuffers b = MakeSampleAssociationSn();
  dlms::cosem::CosemAssociationSnObject object(
    name, b.objectList, b.accessRightsList,
    b.securitySetupReference, b.userList, b.currentUser,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 0u);

  EXPECT_EQ(0u, object.Descriptor().key.version);

  // attrs 4, 5, 6 unavailable in v0
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(4u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));

  // add_user / remove_user (9, 10) only in v3+
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(9u, in, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(10u, in, out));

  // read_by_logicalname (3), change_secret (5),
  // reply_to_HLS_authentication (8) still apply in v0
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(3u, in, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(5u, in, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(8u, in, out));

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::NoAccess,
            rights.AttributeAccess(4u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::NoAccess,
            rights.AttributeAccess(5u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::NoAccess,
            rights.AttributeAccess(6u));
}

TEST(CosemAssociationSnObject, Version2ExposesSecuritySetupButNotUserAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u);
  const AssociationSnBuffers b = MakeSampleAssociationSn();
  dlms::cosem::CosemAssociationSnObject object(
    name, b.objectList, b.accessRightsList,
    b.securitySetupReference, b.userList, b.currentUser,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 2u);

  EXPECT_EQ(2u, object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.securitySetupReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(9u, in, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(10u, in, out));
}

TEST(CosemAssociationSnObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u);
  const AssociationSnBuffers b = MakeSampleAssociationSn();
  dlms::cosem::CosemAssociationSnObject object(
    name, b.objectList, b.accessRightsList,
    b.securitySetupReference, b.userList, b.currentUser,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemAssociationSnObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct MBusClientBuffers
{
  dlms::cosem::CosemByteBuffer mbusPortReference;
  dlms::cosem::CosemByteBuffer captureDefinition;
  dlms::cosem::CosemByteBuffer capturePeriod;
  dlms::cosem::CosemByteBuffer primaryAddress;
  dlms::cosem::CosemByteBuffer identificationNumber;
  dlms::cosem::CosemByteBuffer manufacturerId;
  dlms::cosem::CosemByteBuffer version;
  dlms::cosem::CosemByteBuffer deviceType;
  dlms::cosem::CosemByteBuffer accessNumber;
  dlms::cosem::CosemByteBuffer status;
  dlms::cosem::CosemByteBuffer alarm;
  dlms::cosem::CosemByteBuffer configuration;
  dlms::cosem::CosemByteBuffer encryptionKeyStatus;
};

MBusClientBuffers MakeSampleMBusClient()
{
  MBusClientBuffers b;
  // octet-string(6): IEC HDLC Setup LN 0.0.22.0.0.255
  b.mbusPortReference = BytesFromList({
    0x09u, 0x06u, 0x00u, 0x00u, 0x16u, 0x00u, 0x00u, 0xFFu});
  // array(0)
  b.captureDefinition = BytesFromList({0x01u, 0x00u});
  // double-long-unsigned 3600
  b.capturePeriod = BytesFromList({0x06u, 0x00u, 0x00u, 0x0Eu, 0x10u});
  // unsigned 7
  b.primaryAddress = BytesFromList({0x11u, 0x07u});
  // double-long-unsigned 0x01234567
  b.identificationNumber = BytesFromList({
    0x06u, 0x01u, 0x23u, 0x45u, 0x67u});
  // long-unsigned 0x1234
  b.manufacturerId = BytesFromList({0x12u, 0x12u, 0x34u});
  // unsigned 0x21
  b.version = BytesFromList({0x11u, 0x21u});
  // unsigned 0x07
  b.deviceType = BytesFromList({0x11u, 0x07u});
  // unsigned 0x42
  b.accessNumber = BytesFromList({0x11u, 0x42u});
  // unsigned 0x00
  b.status = BytesFromList({0x11u, 0x00u});
  // unsigned 0x00
  b.alarm = BytesFromList({0x11u, 0x00u});
  // long-unsigned 0x0001
  b.configuration = BytesFromList({0x12u, 0x00u, 0x01u});
  // enum 0 (no encryption key)
  b.encryptionKeyStatus = BytesFromList({0x16u, 0x00u});
  return b;
}

dlms::cosem::CosemMBusClientObject MakeMBusClientObject(
  const dlms::cosem::CosemLogicalName& name,
  const MBusClientBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemMBusClientObject(
    name, b.mbusPortReference, b.captureDefinition, b.capturePeriod,
    b.primaryAddress, b.identificationNumber, b.manufacturerId,
    b.version, b.deviceType, b.accessNumber, b.status, b.alarm,
    b.configuration, b.encryptionKeyStatus, access);
}

} // namespace

TEST(CosemMBusClientObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 24u, 0u, 0u, 255u);
  const MBusClientBuffers b = MakeSampleMBusClient();
  dlms::cosem::CosemMBusClientObject object = MakeMBusClientObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(72u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemMBusClientObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(b.mbusPortReference, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(b.captureDefinition, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(b.capturePeriod, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(b.primaryAddress, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(b.identificationNumber, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(b.manufacturerId, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(b.version, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(b.deviceType, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(10u, out));
  EXPECT_EQ(b.accessNumber, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(11u, out));
  EXPECT_EQ(b.status, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(12u, out));
  EXPECT_EQ(b.alarm, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(13u, out));
  EXPECT_EQ(b.configuration, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(14u, out));
  EXPECT_EQ(b.encryptionKeyStatus, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(15u, out));
}

TEST(CosemMBusClientObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 24u, 0u, 0u, 255u);
  const MBusClientBuffers b = MakeSampleMBusClient();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x55u});

  dlms::cosem::CosemMBusClientObject writable = MakeMBusClientObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u,
                          11u, 12u, 13u, 14u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(replacement, writable.MBusPortReference());
  EXPECT_EQ(replacement, writable.CaptureDefinition());
  EXPECT_EQ(replacement, writable.CapturePeriod());
  EXPECT_EQ(replacement, writable.PrimaryAddress());
  EXPECT_EQ(replacement, writable.IdentificationNumber());
  EXPECT_EQ(replacement, writable.ManufacturerId());
  EXPECT_EQ(replacement, writable.Version());
  EXPECT_EQ(replacement, writable.DeviceType());
  EXPECT_EQ(replacement, writable.AccessNumber());
  EXPECT_EQ(replacement, writable.Status());
  EXPECT_EQ(replacement, writable.Alarm());
  EXPECT_EQ(replacement, writable.Configuration());
  EXPECT_EQ(replacement, writable.EncryptionKeyStatus());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemMBusClientObject readOnly = MakeMBusClientObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u,
                          11u, 12u, 13u, 14u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(
                static_cast<std::uint8_t>(id), replacement))
      << "attribute id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(b.mbusPortReference, readOnly.MBusPortReference());
  EXPECT_EQ(b.captureDefinition, readOnly.CaptureDefinition());
  EXPECT_EQ(b.capturePeriod, readOnly.CapturePeriod());
  EXPECT_EQ(b.primaryAddress, readOnly.PrimaryAddress());
  EXPECT_EQ(b.identificationNumber, readOnly.IdentificationNumber());
  EXPECT_EQ(b.manufacturerId, readOnly.ManufacturerId());
  EXPECT_EQ(b.version, readOnly.Version());
  EXPECT_EQ(b.deviceType, readOnly.DeviceType());
  EXPECT_EQ(b.accessNumber, readOnly.AccessNumber());
  EXPECT_EQ(b.status, readOnly.Status());
  EXPECT_EQ(b.alarm, readOnly.Alarm());
  EXPECT_EQ(b.configuration, readOnly.Configuration());
  EXPECT_EQ(b.encryptionKeyStatus, readOnly.EncryptionKeyStatus());
}

TEST(CosemMBusClientObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 24u, 0u, 0u, 255u);
  const MBusClientBuffers b = MakeSampleMBusClient();
  dlms::cosem::CosemMBusClientObject object = MakeMBusClientObject(
    name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t method : {9u, 10u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemMBusClientObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 24u, 0u, 0u, 255u);
  const MBusClientBuffers b = MakeSampleMBusClient();
  dlms::cosem::CosemMBusClientObject object(
    name, b.mbusPortReference, b.captureDefinition, b.capturePeriod,
    b.primaryAddress, b.identificationNumber, b.manufacturerId,
    b.version, b.deviceType, b.accessNumber, b.status, b.alarm,
    b.configuration, b.encryptionKeyStatus,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemMBusClientObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemMBusClientObject,
     Version0DoesNotExposeConfigurationOrEncryptionKeyStatus)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 1u, 24u, 0u, 0u, 255u);
  const MBusClientBuffers b = MakeSampleMBusClient();
  dlms::cosem::CosemMBusClientObject object(
    name, b.mbusPortReference, b.captureDefinition, b.capturePeriod,
    b.primaryAddress, b.identificationNumber, b.manufacturerId,
    b.version, b.deviceType, b.accessNumber, b.status, b.alarm,
    b.configuration, b.encryptionKeyStatus,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 0u);

  EXPECT_EQ(0u, object.Descriptor().key.version);

  dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::NoAccess,
            rights.AttributeAccess(13u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::NoAccess,
            rights.AttributeAccess(14u));

  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(13u, out));
  EXPECT_TRUE(out.empty());
  out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(14u, out));
  EXPECT_TRUE(out.empty());

  // Attributes 1..12 must still be readable on v0.
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(12u, out));
  EXPECT_EQ(b.alarm, out);

  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x55u});
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(13u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(14u, replacement));
}

namespace {

dlms::cosem::CosemByteBuffer SampleMBusMasterCommSpeed()
{
  // enum 5
  return BytesFromList({0x16u, 0x05u});
}

dlms::cosem::CosemMBusMasterPortSetupObject
MakeMBusMasterPortSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const dlms::cosem::CosemByteBuffer& commSpeed,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemMBusMasterPortSetupObject(
    name, commSpeed, access);
}

} // namespace

TEST(CosemMBusMasterPortSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 6u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer commSpeed =
    SampleMBusMasterCommSpeed();
  dlms::cosem::CosemMBusMasterPortSetupObject object =
    MakeMBusMasterPortSetupObject(
      name, commSpeed,
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(73u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemMBusMasterPortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(commSpeed, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemMBusMasterPortSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 6u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer commSpeed =
    SampleMBusMasterCommSpeed();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x16u, 0x06u});

  dlms::cosem::CosemMBusMasterPortSetupObject writable =
    MakeMBusMasterPortSetupObject(
      name, commSpeed,
      dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.CommSpeed());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemMBusMasterPortSetupObject readOnly =
    MakeMBusMasterPortSetupObject(
      name, commSpeed, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(commSpeed, readOnly.CommSpeed());
}

TEST(CosemMBusMasterPortSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 6u, 0u, 255u);
  dlms::cosem::CosemMBusMasterPortSetupObject object =
    MakeMBusMasterPortSetupObject(
      name, SampleMBusMasterCommSpeed(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemMBusMasterPortSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 6u, 0u, 255u);
  dlms::cosem::CosemMBusMasterPortSetupObject object(
    name, SampleMBusMasterCommSpeed(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemMBusMasterPortSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct MBusDiagnosticBuffers
{
  dlms::cosem::CosemByteBuffer receivedSignalQuality;
  dlms::cosem::CosemByteBuffer transmitterSignalQuality;
  dlms::cosem::CosemByteBuffer bbc;
  dlms::cosem::CosemByteBuffer fcsOkFramesCounter;
  dlms::cosem::CosemByteBuffer fcsNokFramesCounter;
  dlms::cosem::CosemByteBuffer captureTime;
};

MBusDiagnosticBuffers MakeSampleMBusDiagnostic()
{
  MBusDiagnosticBuffers b;
  // unsigned 75 (RSSI quality)
  b.receivedSignalQuality = BytesFromList({0x11u, 0x4Bu});
  // unsigned 80
  b.transmitterSignalQuality = BytesFromList({0x11u, 0x50u});
  // long-unsigned 0x1234 (BBC)
  b.bbc = BytesFromList({0x12u, 0x12u, 0x34u});
  // double-long-unsigned 12345
  b.fcsOkFramesCounter =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x30u, 0x39u});
  // double-long-unsigned 7
  b.fcsNokFramesCounter =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x07u});
  // octet-string(12) date-time
  b.captureTime = BytesFromList({0x09u, 0x0Cu,
    0x07u, 0xE9u, 0x06u, 0x0Fu, 0x04u, 0x0Cu, 0x00u, 0x00u,
    0x00u, 0x00u, 0x80u, 0x00u});
  return b;
}

dlms::cosem::CosemMBusDiagnosticObject
MakeMBusDiagnosticObject(
  const dlms::cosem::CosemLogicalName& name,
  const MBusDiagnosticBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemMBusDiagnosticObject(
    name, b.receivedSignalQuality, b.transmitterSignalQuality,
    b.bbc, b.fcsOkFramesCounter, b.fcsNokFramesCounter,
    b.captureTime, access);
}

} // namespace

TEST(CosemMBusDiagnosticObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 9u, 0u, 255u);
  const MBusDiagnosticBuffers b = MakeSampleMBusDiagnostic();
  dlms::cosem::CosemMBusDiagnosticObject object =
    MakeMBusDiagnosticObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(77u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemMBusDiagnosticObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(b.receivedSignalQuality, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, out));
  EXPECT_EQ(b.transmitterSignalQuality, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, out));
  EXPECT_EQ(b.bbc, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, out));
  EXPECT_EQ(b.fcsOkFramesCounter, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(6u, out));
  EXPECT_EQ(b.fcsNokFramesCounter, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(7u, out));
  EXPECT_EQ(b.captureTime, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(8u, out));
}

TEST(CosemMBusDiagnosticObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 9u, 0u, 255u);
  const MBusDiagnosticBuffers b = MakeSampleMBusDiagnostic();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x11u, 0x00u});

  dlms::cosem::CosemMBusDiagnosticObject writable =
    MakeMBusDiagnosticObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t attr : {2u, 3u, 4u, 5u, 6u, 7u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(attr, replacement))
      << "attr " << static_cast<unsigned>(attr);
  }
  EXPECT_EQ(replacement, writable.ReceivedSignalQuality());
  EXPECT_EQ(replacement, writable.TransmitterSignalQuality());
  EXPECT_EQ(replacement, writable.Bbc());
  EXPECT_EQ(replacement, writable.FcsOkFramesCounter());
  EXPECT_EQ(replacement, writable.FcsNokFramesCounter());
  EXPECT_EQ(replacement, writable.CaptureTime());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemMBusDiagnosticObject readOnly =
    MakeMBusDiagnosticObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(b.receivedSignalQuality, readOnly.ReceivedSignalQuality());
}

TEST(CosemMBusDiagnosticObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 9u, 0u, 255u);
  dlms::cosem::CosemMBusDiagnosticObject object =
    MakeMBusDiagnosticObject(
      name, MakeSampleMBusDiagnostic(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemMBusDiagnosticObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 24u, 9u, 0u, 255u);
  const MBusDiagnosticBuffers b = MakeSampleMBusDiagnostic();
  dlms::cosem::CosemMBusDiagnosticObject object(
    name, b.receivedSignalQuality, b.transmitterSignalQuality,
    b.bbc, b.fcsOkFramesCounter, b.fcsNokFramesCounter,
    b.captureTime,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemMBusDiagnosticObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct PrimePlcMacSetupBuffers
{
  dlms::cosem::CosemByteBuffer macMinConWindow;
  dlms::cosem::CosemByteBuffer macMaxConWindow;
  dlms::cosem::CosemByteBuffer macChannelAccessFairnessLimit;
  dlms::cosem::CosemByteBuffer macEma;
  dlms::cosem::CosemByteBuffer macSarSize;
  dlms::cosem::CosemByteBuffer macMaxPduSize;
  dlms::cosem::CosemByteBuffer macMinSwitchSearchTime;
  dlms::cosem::CosemByteBuffer macMaxPromotionPdu;
  dlms::cosem::CosemByteBuffer macPromotionPduTxPeriod;
  dlms::cosem::CosemByteBuffer macBeaconsPerFrame;
  dlms::cosem::CosemByteBuffer macScpMaxTxAttempts;
  dlms::cosem::CosemByteBuffer macCtlReTxTimer;
  dlms::cosem::CosemByteBuffer macMaxLnid;
};

PrimePlcMacSetupBuffers MakeSamplePrimePlcMacSetup()
{
  PrimePlcMacSetupBuffers b;
  // long-unsigned 1
  b.macMinConWindow = BytesFromList({0x12u, 0x00u, 0x01u});
  // long-unsigned 8
  b.macMaxConWindow = BytesFromList({0x12u, 0x00u, 0x08u});
  // unsigned 50
  b.macChannelAccessFairnessLimit = BytesFromList({0x11u, 0x32u});
  // long-unsigned 16
  b.macEma = BytesFromList({0x12u, 0x00u, 0x10u});
  // long-unsigned 128
  b.macSarSize = BytesFromList({0x12u, 0x00u, 0x80u});
  // long-unsigned 364
  b.macMaxPduSize = BytesFromList({0x12u, 0x01u, 0x6Cu});
  // long-unsigned 0x000F
  b.macMinSwitchSearchTime = BytesFromList({0x12u, 0x00u, 0x0Fu});
  // long-unsigned 4
  b.macMaxPromotionPdu = BytesFromList({0x12u, 0x00u, 0x04u});
  // long-unsigned 0x0020
  b.macPromotionPduTxPeriod = BytesFromList({0x12u, 0x00u, 0x20u});
  // unsigned 4
  b.macBeaconsPerFrame = BytesFromList({0x11u, 0x04u});
  // unsigned 3
  b.macScpMaxTxAttempts = BytesFromList({0x11u, 0x03u});
  // unsigned 5
  b.macCtlReTxTimer = BytesFromList({0x11u, 0x05u});
  // long-unsigned 0x00FF
  b.macMaxLnid = BytesFromList({0x12u, 0x00u, 0xFFu});
  return b;
}

dlms::cosem::CosemPrimePlcMacSetupObject
MakePrimePlcMacSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const PrimePlcMacSetupBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPrimePlcMacSetupObject(
    name, b.macMinConWindow, b.macMaxConWindow,
    b.macChannelAccessFairnessLimit, b.macEma, b.macSarSize,
    b.macMaxPduSize, b.macMinSwitchSearchTime,
    b.macMaxPromotionPdu, b.macPromotionPduTxPeriod,
    b.macBeaconsPerFrame, b.macScpMaxTxAttempts,
    b.macCtlReTxTimer, b.macMaxLnid, access);
}

} // namespace

TEST(CosemPrimePlcMacSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 1u, 0u, 255u);
  const PrimePlcMacSetupBuffers b = MakeSamplePrimePlcMacSetup();
  dlms::cosem::CosemPrimePlcMacSetupObject object =
    MakePrimePlcMacSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(80u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  const dlms::cosem::CosemByteBuffer* expected[] = {
    &b.macMinConWindow, &b.macMaxConWindow,
    &b.macChannelAccessFairnessLimit, &b.macEma, &b.macSarSize,
    &b.macMaxPduSize, &b.macMinSwitchSearchTime,
    &b.macMaxPromotionPdu, &b.macPromotionPduTxPeriod,
    &b.macBeaconsPerFrame, &b.macScpMaxTxAttempts,
    &b.macCtlReTxTimer, &b.macMaxLnid};
  std::uint8_t attrId = 2u;
  for (const auto* exp : expected) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.ReadAttribute(attrId, out))
      << "attr " << static_cast<unsigned>(attrId);
    EXPECT_EQ(*exp, out)
      << "attr " << static_cast<unsigned>(attrId);
    ++attrId;
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(15u, out));
}

TEST(CosemPrimePlcMacSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 1u, 0u, 255u);
  const PrimePlcMacSetupBuffers b = MakeSamplePrimePlcMacSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x07u, 0xFFu});

  dlms::cosem::CosemPrimePlcMacSetupObject writable =
    MakePrimePlcMacSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t attr :
       {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(attr, replacement))
      << "attr " << static_cast<unsigned>(attr);
  }
  EXPECT_EQ(replacement, writable.MacMinConWindow());
  EXPECT_EQ(replacement, writable.MacMaxConWindow());
  EXPECT_EQ(replacement, writable.MacChannelAccessFairnessLimit());
  EXPECT_EQ(replacement, writable.MacEma());
  EXPECT_EQ(replacement, writable.MacSarSize());
  EXPECT_EQ(replacement, writable.MacMaxPduSize());
  EXPECT_EQ(replacement, writable.MacMinSwitchSearchTime());
  EXPECT_EQ(replacement, writable.MacMaxPromotionPdu());
  EXPECT_EQ(replacement, writable.MacPromotionPduTxPeriod());
  EXPECT_EQ(replacement, writable.MacBeaconsPerFrame());
  EXPECT_EQ(replacement, writable.MacScpMaxTxAttempts());
  EXPECT_EQ(replacement, writable.MacCtlReTxTimer());
  EXPECT_EQ(replacement, writable.MacMaxLnid());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPrimePlcMacSetupObject readOnly =
    MakePrimePlcMacSetupObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(b.macMinConWindow, readOnly.MacMinConWindow());
}

TEST(CosemPrimePlcMacSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 1u, 0u, 255u);
  dlms::cosem::CosemPrimePlcMacSetupObject object =
    MakePrimePlcMacSetupObject(
      name, MakeSamplePrimePlcMacSetup(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPrimePlcMacSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 1u, 0u, 255u);
  const PrimePlcMacSetupBuffers b = MakeSamplePrimePlcMacSetup();
  dlms::cosem::CosemPrimePlcMacSetupObject object(
    name, b.macMinConWindow, b.macMaxConWindow,
    b.macChannelAccessFairnessLimit, b.macEma, b.macSarSize,
    b.macMaxPduSize, b.macMinSwitchSearchTime,
    b.macMaxPromotionPdu, b.macPromotionPduTxPeriod,
    b.macBeaconsPerFrame, b.macScpMaxTxAttempts,
    b.macCtlReTxTimer, b.macMaxLnid,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct PrimePlcMacFunctionalParamsBuffers
{
  dlms::cosem::CosemByteBuffer lnid;
  dlms::cosem::CosemByteBuffer lsid;
  dlms::cosem::CosemByteBuffer sid;
  dlms::cosem::CosemByteBuffer sna;
  dlms::cosem::CosemByteBuffer state;
  dlms::cosem::CosemByteBuffer sct;
  dlms::cosem::CosemByteBuffer scd;
  dlms::cosem::CosemByteBuffer capabilities;
};

PrimePlcMacFunctionalParamsBuffers
MakeSamplePrimePlcMacFunctionalParams()
{
  PrimePlcMacFunctionalParamsBuffers b;
  // long-unsigned 0x0042
  b.lnid = BytesFromList({0x12u, 0x00u, 0x42u});
  // unsigned 1
  b.lsid = BytesFromList({0x11u, 0x01u});
  // unsigned 2
  b.sid = BytesFromList({0x11u, 0x02u});
  // octet-string(6) EUI-48 12:34:56:78:9A:BC
  b.sna = BytesFromList(
    {0x09u, 0x06u, 0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu});
  // enum 3 (Terminal connected)
  b.state = BytesFromList({0x16u, 0x03u});
  // long-unsigned 0x0007
  b.sct = BytesFromList({0x12u, 0x00u, 0x07u});
  // long-unsigned 0x0005
  b.scd = BytesFromList({0x12u, 0x00u, 0x05u});
  // bit-string(8) 0b10100101
  b.capabilities = BytesFromList({0x04u, 0x08u, 0xA5u});
  return b;
}

dlms::cosem::CosemPrimePlcMacFunctionalParametersObject
MakePrimePlcMacFunctionalParamsObject(
  const dlms::cosem::CosemLogicalName& name,
  const PrimePlcMacFunctionalParamsBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPrimePlcMacFunctionalParametersObject(
    name, b.lnid, b.lsid, b.sid, b.sna, b.state, b.sct, b.scd,
    b.capabilities, access);
}

} // namespace

TEST(CosemPrimePlcMacFunctionalParametersObject,
     ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 2u, 0u, 255u);
  const PrimePlcMacFunctionalParamsBuffers b =
    MakeSamplePrimePlcMacFunctionalParams();
  dlms::cosem::CosemPrimePlcMacFunctionalParametersObject object =
    MakePrimePlcMacFunctionalParamsObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(81u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacFunctionalParametersObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  const dlms::cosem::CosemByteBuffer* expected[] = {
    &b.lnid, &b.lsid, &b.sid, &b.sna, &b.state, &b.sct, &b.scd,
    &b.capabilities};
  std::uint8_t attrId = 2u;
  for (const auto* exp : expected) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.ReadAttribute(attrId, out))
      << "attr " << static_cast<unsigned>(attrId);
    EXPECT_EQ(*exp, out)
      << "attr " << static_cast<unsigned>(attrId);
    ++attrId;
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, out));
}

TEST(CosemPrimePlcMacFunctionalParametersObject,
     MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 2u, 0u, 255u);
  const PrimePlcMacFunctionalParamsBuffers b =
    MakeSamplePrimePlcMacFunctionalParams();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x10u, 0x00u});

  dlms::cosem::CosemPrimePlcMacFunctionalParametersObject writable =
    MakePrimePlcMacFunctionalParamsObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t attr : {2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(attr, replacement))
      << "attr " << static_cast<unsigned>(attr);
  }
  EXPECT_EQ(replacement, writable.Lnid());
  EXPECT_EQ(replacement, writable.Lsid());
  EXPECT_EQ(replacement, writable.Sid());
  EXPECT_EQ(replacement, writable.Sna());
  EXPECT_EQ(replacement, writable.State());
  EXPECT_EQ(replacement, writable.Sct());
  EXPECT_EQ(replacement, writable.Scd());
  EXPECT_EQ(replacement, writable.Capabilities());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPrimePlcMacFunctionalParametersObject readOnly =
    MakePrimePlcMacFunctionalParamsObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(b.lnid, readOnly.Lnid());
}

TEST(CosemPrimePlcMacFunctionalParametersObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 2u, 0u, 255u);
  dlms::cosem::CosemPrimePlcMacFunctionalParametersObject object =
    MakePrimePlcMacFunctionalParamsObject(
      name, MakeSamplePrimePlcMacFunctionalParams(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPrimePlcMacFunctionalParametersObject,
     NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 2u, 0u, 255u);
  const PrimePlcMacFunctionalParamsBuffers b =
    MakeSamplePrimePlcMacFunctionalParams();
  dlms::cosem::CosemPrimePlcMacFunctionalParametersObject object(
    name, b.lnid, b.lsid, b.sid, b.sna, b.state, b.sct, b.scd,
    b.capabilities,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacFunctionalParametersObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct PrimePlcMacCountersBuffers
{
  dlms::cosem::CosemByteBuffer txDataPktCount;
  dlms::cosem::CosemByteBuffer rxDataPktCount;
  dlms::cosem::CosemByteBuffer txCtrlPktCount;
  dlms::cosem::CosemByteBuffer rxCtrlPktCount;
  dlms::cosem::CosemByteBuffer csmaFailCount;
  dlms::cosem::CosemByteBuffer csmaChBusyCount;
};

PrimePlcMacCountersBuffers MakeSamplePrimePlcMacCounters()
{
  PrimePlcMacCountersBuffers b;
  // double-long-unsigned 0x00001234
  b.txDataPktCount =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x12u, 0x34u});
  // double-long-unsigned 0x00005678
  b.rxDataPktCount =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x56u, 0x78u});
  // double-long-unsigned 0x000000FF
  b.txCtrlPktCount =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0xFFu});
  // double-long-unsigned 0x000000AA
  b.rxCtrlPktCount =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0xAAu});
  // double-long-unsigned 0x00000010
  b.csmaFailCount =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x10u});
  // double-long-unsigned 0x00000020
  b.csmaChBusyCount =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x20u});
  return b;
}

dlms::cosem::CosemPrimePlcMacCountersObject
MakePrimePlcMacCountersObject(
  const dlms::cosem::CosemLogicalName& name,
  const PrimePlcMacCountersBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPrimePlcMacCountersObject(
    name, b.txDataPktCount, b.rxDataPktCount,
    b.txCtrlPktCount, b.rxCtrlPktCount, b.csmaFailCount,
    b.csmaChBusyCount, access);
}

} // namespace

TEST(CosemPrimePlcMacCountersObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 3u, 0u, 255u);
  const PrimePlcMacCountersBuffers b =
    MakeSamplePrimePlcMacCounters();
  dlms::cosem::CosemPrimePlcMacCountersObject object =
    MakePrimePlcMacCountersObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(82u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacCountersObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  const dlms::cosem::CosemByteBuffer* expected[] = {
    &b.txDataPktCount, &b.rxDataPktCount, &b.txCtrlPktCount,
    &b.rxCtrlPktCount, &b.csmaFailCount, &b.csmaChBusyCount};
  std::uint8_t attrId = 2u;
  for (const auto* exp : expected) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.ReadAttribute(attrId, out))
      << "attr " << static_cast<unsigned>(attrId);
    EXPECT_EQ(*exp, out)
      << "attr " << static_cast<unsigned>(attrId);
    ++attrId;
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(8u, out));
}

TEST(CosemPrimePlcMacCountersObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 3u, 0u, 255u);
  const PrimePlcMacCountersBuffers b =
    MakeSamplePrimePlcMacCounters();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x06u, 0xDEu, 0xADu, 0xBEu, 0xEFu});

  dlms::cosem::CosemPrimePlcMacCountersObject writable =
    MakePrimePlcMacCountersObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t attr : {2u, 3u, 4u, 5u, 6u, 7u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(attr, replacement))
      << "attr " << static_cast<unsigned>(attr);
  }
  EXPECT_EQ(replacement, writable.TxDataPktCount());
  EXPECT_EQ(replacement, writable.RxDataPktCount());
  EXPECT_EQ(replacement, writable.TxCtrlPktCount());
  EXPECT_EQ(replacement, writable.RxCtrlPktCount());
  EXPECT_EQ(replacement, writable.CsmaFailCount());
  EXPECT_EQ(replacement, writable.CsmaChBusyCount());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPrimePlcMacCountersObject readOnly =
    MakePrimePlcMacCountersObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(b.txDataPktCount, readOnly.TxDataPktCount());
}

TEST(CosemPrimePlcMacCountersObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 3u, 0u, 255u);
  dlms::cosem::CosemPrimePlcMacCountersObject object =
    MakePrimePlcMacCountersObject(
      name, MakeSamplePrimePlcMacCounters(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  for (std::uint8_t method : {0u, 2u, 99u, 255u}) {
    out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPrimePlcMacCountersObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 3u, 0u, 255u);
  const PrimePlcMacCountersBuffers b =
    MakeSamplePrimePlcMacCounters();
  dlms::cosem::CosemPrimePlcMacCountersObject object(
    name, b.txDataPktCount, b.rxDataPktCount,
    b.txCtrlPktCount, b.rxCtrlPktCount, b.csmaFailCount,
    b.csmaChBusyCount,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacCountersObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

struct PrimePlcMacNetStatsBuffers
{
  dlms::cosem::CosemByteBuffer nodeRegistrations;
  dlms::cosem::CosemByteBuffer nodeUnregistrations;
  dlms::cosem::CosemByteBuffer processedAliveMsgs;
  dlms::cosem::CosemByteBuffer handledPromotions;
};

PrimePlcMacNetStatsBuffers MakeSamplePrimePlcMacNetStats()
{
  PrimePlcMacNetStatsBuffers b;
  // double-long-unsigned 0x00000011
  b.nodeRegistrations =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x11u});
  // double-long-unsigned 0x00000022
  b.nodeUnregistrations =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x22u});
  // double-long-unsigned 0x00000033
  b.processedAliveMsgs =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x33u});
  // double-long-unsigned 0x00000044
  b.handledPromotions =
    BytesFromList({0x06u, 0x00u, 0x00u, 0x00u, 0x44u});
  return b;
}

dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject
MakePrimePlcMacNetStatsObject(
  const dlms::cosem::CosemLogicalName& name,
  const PrimePlcMacNetStatsBuffers& b,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject(
    name, b.nodeRegistrations, b.nodeUnregistrations,
    b.processedAliveMsgs, b.handledPromotions, access);
}

} // namespace

TEST(CosemPrimePlcMacNetworkStatisticsObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 4u, 0u, 255u);
  const PrimePlcMacNetStatsBuffers b =
    MakeSamplePrimePlcMacNetStats();
  dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject object =
    MakePrimePlcMacNetStatsObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(83u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  const dlms::cosem::CosemByteBuffer* expected[] = {
    &b.nodeRegistrations, &b.nodeUnregistrations,
    &b.processedAliveMsgs, &b.handledPromotions};
  std::uint8_t attrId = 2u;
  for (const auto* exp : expected) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.ReadAttribute(attrId, out))
      << "attr " << static_cast<unsigned>(attrId);
    EXPECT_EQ(*exp, out)
      << "attr " << static_cast<unsigned>(attrId);
    ++attrId;
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(6u, out));
}

TEST(CosemPrimePlcMacNetworkStatisticsObject,
     MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 4u, 0u, 255u);
  const PrimePlcMacNetStatsBuffers b =
    MakeSamplePrimePlcMacNetStats();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x06u, 0xCAu, 0xFEu, 0xBAu, 0xBEu});

  dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject writable =
    MakePrimePlcMacNetStatsObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t attr : {2u, 3u, 4u, 5u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(attr, replacement))
      << "attr " << static_cast<unsigned>(attr);
  }
  EXPECT_EQ(replacement, writable.NodeRegistrations());
  EXPECT_EQ(replacement, writable.NodeUnregistrations());
  EXPECT_EQ(replacement, writable.ProcessedAliveMsgs());
  EXPECT_EQ(replacement, writable.HandledPromotions());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject readOnly =
    MakePrimePlcMacNetStatsObject(
      name, b, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(b.nodeRegistrations, readOnly.NodeRegistrations());
}

TEST(CosemPrimePlcMacNetworkStatisticsObject,
     MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 4u, 0u, 255u);
  dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject object =
    MakePrimePlcMacNetStatsObject(
      name, MakeSamplePrimePlcMacNetStats(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  for (std::uint8_t method : {0u, 2u, 99u, 255u}) {
    out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPrimePlcMacNetworkStatisticsObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 4u, 0u, 255u);
  const PrimePlcMacNetStatsBuffers b =
    MakeSamplePrimePlcMacNetStats();
  dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject object(
    name, b.nodeRegistrations, b.nodeUnregistrations,
    b.processedAliveMsgs, b.handledPromotions,
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacNetworkStatisticsObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

dlms::cosem::CosemByteBuffer SamplePrimePlcMacAddress()
{
  // long-unsigned 0x1234
  return BytesFromList({0x12u, 0x12u, 0x34u});
}

dlms::cosem::CosemPrimePlcMacAddressSetupObject
MakePrimePlcMacAddressSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const dlms::cosem::CosemByteBuffer& macAddress,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPrimePlcMacAddressSetupObject(
    name, macAddress, access);
}

} // namespace

TEST(CosemPrimePlcMacAddressSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 5u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer mac = SamplePrimePlcMacAddress();
  dlms::cosem::CosemPrimePlcMacAddressSetupObject object =
    MakePrimePlcMacAddressSetupObject(
      name, mac, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(84u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacAddressSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(mac, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemPrimePlcMacAddressSetupObject,
     MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 5u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer mac = SamplePrimePlcMacAddress();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0xABu, 0xCDu});

  dlms::cosem::CosemPrimePlcMacAddressSetupObject writable =
    MakePrimePlcMacAddressSetupObject(
      name, mac, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.MacAddress());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPrimePlcMacAddressSetupObject readOnly =
    MakePrimePlcMacAddressSetupObject(
      name, mac, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(mac, readOnly.MacAddress());
}

TEST(CosemPrimePlcMacAddressSetupObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 5u, 0u, 255u);
  dlms::cosem::CosemPrimePlcMacAddressSetupObject object =
    MakePrimePlcMacAddressSetupObject(
      name, SamplePrimePlcMacAddress(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPrimePlcMacAddressSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 5u, 0u, 255u);
  dlms::cosem::CosemPrimePlcMacAddressSetupObject object(
    name, SamplePrimePlcMacAddress(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcMacAddressSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

dlms::cosem::CosemByteBuffer SamplePrimePlcApplicationIdentifier()
{
  // octet-string of length 8 (PRIME application identifier)
  return BytesFromList({0x09u, 0x08u, 0x01u, 0x02u, 0x03u, 0x04u,
                        0x05u, 0x06u, 0x07u, 0x08u});
}

dlms::cosem::CosemPrimePlcApplicationIdentificationObject
MakePrimePlcApplicationIdentificationObject(
  const dlms::cosem::CosemLogicalName& name,
  const dlms::cosem::CosemByteBuffer& appId,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemPrimePlcApplicationIdentificationObject(
    name, appId, access);
}

} // namespace

TEST(CosemPrimePlcApplicationIdentificationObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 6u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer appId =
    SamplePrimePlcApplicationIdentifier();
  dlms::cosem::CosemPrimePlcApplicationIdentificationObject object =
    MakePrimePlcApplicationIdentificationObject(
      name, appId, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(85u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcApplicationIdentificationObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(appId, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemPrimePlcApplicationIdentificationObject,
     MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 6u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer appId =
    SamplePrimePlcApplicationIdentifier();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x09u, 0x04u, 0xDEu, 0xADu, 0xBEu, 0xEFu});

  dlms::cosem::CosemPrimePlcApplicationIdentificationObject writable =
    MakePrimePlcApplicationIdentificationObject(
      name, appId, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.ApplicationIdentifier());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemPrimePlcApplicationIdentificationObject readOnly =
    MakePrimePlcApplicationIdentificationObject(
      name, appId, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(appId, readOnly.ApplicationIdentifier());
}

TEST(CosemPrimePlcApplicationIdentificationObject, NoMethodsDefined)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 6u, 0u, 255u);
  dlms::cosem::CosemPrimePlcApplicationIdentificationObject object =
    MakePrimePlcApplicationIdentificationObject(
      name, SamplePrimePlcApplicationIdentifier(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemPrimePlcApplicationIdentificationObject,
     NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 28u, 6u, 0u, 255u);
  dlms::cosem::CosemPrimePlcApplicationIdentificationObject object(
    name, SamplePrimePlcApplicationIdentifier(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemPrimePlcApplicationIdentificationObject::
      MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

dlms::cosem::CosemSFskPlcPhyMacSetupObject::Attributes
MakeSampleSFskPlcPhyMacSetup()
{
  dlms::cosem::CosemSFskPlcPhyMacSetupObject::Attributes a;
  // enum 0x01
  a.initiatorElectricalPhase = BytesFromList({0x16u, 0x01u});
  a.deltaElectricalPhase = BytesFromList({0x16u, 0x02u});
  // unsigned
  a.maxReceivingGain = BytesFromList({0x11u, 0x10u});
  a.maxTransmittingGain = BytesFromList({0x11u, 0x20u});
  // unsigned 0x62 (98, the Blue Book default for v1)
  a.searchInitiatorThreshold = BytesFromList({0x11u, 0x62u});
  // frequencies = structure { double-long-unsigned mark_frequency,
  //                           double-long-unsigned space_frequency }
  // mark = 0x000012C0 (4800 Hz), space = 0x00001800 (6144 Hz).
  a.frequencies = BytesFromList({
    0x02u, 0x02u,
    0x06u, 0x00u, 0x00u, 0x12u, 0xC0u,
    0x06u, 0x00u, 0x00u, 0x18u, 0x00u});
  // long-unsigned 0x1234
  a.macAddress = BytesFromList({0x12u, 0x12u, 0x34u});
  // empty array of long-unsigned
  a.macGroupAddresses = BytesFromList({0x01u, 0x00u});
  // enum 0x00
  a.repeater = BytesFromList({0x16u, 0x00u});
  // boolean false
  a.repeaterStatus = BytesFromList({0x03u, 0x00u});
  a.minDeltaCredit = BytesFromList({0x11u, 0x05u});
  a.initiatorMacAddress = BytesFromList({0x12u, 0x56u, 0x78u});
  a.synchronizationLocked = BytesFromList({0x03u, 0xFFu});
  // enum 0x03 (default per Blue Book v1)
  a.transmissionSpeed = BytesFromList({0x16u, 0x03u});
  return a;
}

dlms::cosem::CosemSFskPlcPhyMacSetupObject
MakeSFskPlcPhyMacSetupObject(
  const dlms::cosem::CosemLogicalName& name,
  const dlms::cosem::CosemSFskPlcPhyMacSetupObject::Attributes& a,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemSFskPlcPhyMacSetupObject(name, a, access);
}

} // namespace

TEST(CosemSFskPlcPhyMacSetupObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 0u, 0u, 255u);
  const auto attrs = MakeSampleSFskPlcPhyMacSetup();
  dlms::cosem::CosemSFskPlcPhyMacSetupObject object =
    MakeSFskPlcPhyMacSetupObject(
      name, attrs, dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(50u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSFskPlcPhyMacSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  const dlms::cosem::CosemByteBuffer* expected[] = {
    &attrs.initiatorElectricalPhase, &attrs.deltaElectricalPhase,
    &attrs.maxReceivingGain, &attrs.maxTransmittingGain,
    &attrs.searchInitiatorThreshold, &attrs.frequencies,
    &attrs.macAddress, &attrs.macGroupAddresses,
    &attrs.repeater, &attrs.repeaterStatus,
    &attrs.minDeltaCredit, &attrs.initiatorMacAddress,
    &attrs.synchronizationLocked, &attrs.transmissionSpeed};
  std::uint8_t attrId = 2u;
  for (const auto* exp : expected) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.ReadAttribute(attrId, out))
      << "attr " << static_cast<unsigned>(attrId);
    EXPECT_EQ(*exp, out)
      << "attr " << static_cast<unsigned>(attrId);
    ++attrId;
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(16u, out));
}

TEST(CosemSFskPlcPhyMacSetupObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 0u, 0u, 255u);
  const auto attrs = MakeSampleSFskPlcPhyMacSetup();
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x09u, 0x02u, 0xAAu, 0xBBu});

  dlms::cosem::CosemSFskPlcPhyMacSetupObject writable =
    MakeSFskPlcPhyMacSetupObject(
      name, attrs, dlms::cosem::AttributeAccessMode::ReadAndWrite);
  for (std::uint8_t attr = 2u; attr <= 15u; ++attr) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              writable.WriteAttribute(attr, replacement))
      << "attr " << static_cast<unsigned>(attr);
  }
  EXPECT_EQ(replacement,
            writable.AttributeData().transmissionSpeed);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemSFskPlcPhyMacSetupObject readOnly =
    MakeSFskPlcPhyMacSetupObject(
      name, attrs, dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(7u, replacement));
  EXPECT_EQ(attrs.frequencies,
            readOnly.AttributeData().frequencies);
}

TEST(CosemSFskPlcPhyMacSetupObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 0u, 0u, 255u);
  dlms::cosem::CosemSFskPlcPhyMacSetupObject object =
    MakeSFskPlcPhyMacSetupObject(
      name, MakeSampleSFskPlcPhyMacSetup(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  for (std::uint8_t method : {0u, 2u, 99u, 255u}) {
    out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSFskPlcPhyMacSetupObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 0u, 0u, 255u);
  dlms::cosem::CosemSFskPlcPhyMacSetupObject object(
    name, MakeSampleSFskPlcPhyMacSetup(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSFskPlcPhyMacSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

namespace {

dlms::cosem::CosemByteBuffer SampleSFskActiveInitiator()
{
  // structure { octet-string(8) system_title,
  //             long-unsigned MAC_address,
  //             unsigned L_SAP_selector }
  // See IEC 62056-6-2 ED4 4.10.4.2.2 and DLMS UA Blue Book IC 51.
  return BytesFromList({
    0x02u, 0x03u,
    0x09u, 0x08u, 0x53u, 0x4Du, 0x54u, 0x01u,
      0x02u, 0x03u, 0x04u, 0x05u,
    0x12u, 0x01u, 0xF4u,
    0x11u, 0x7Fu});
}

dlms::cosem::CosemSFskActiveInitiatorObject
MakeSFskActiveInitiatorObject(
  const dlms::cosem::CosemLogicalName& name,
  const dlms::cosem::CosemByteBuffer& initiator,
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemSFskActiveInitiatorObject(
    name, initiator, access);
}

} // namespace

TEST(CosemSFskActiveInitiatorObject, ExposesAllAttributes)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 1u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer initiator =
    SampleSFskActiveInitiator();
  dlms::cosem::CosemSFskActiveInitiatorObject object =
    MakeSFskActiveInitiatorObject(
      name, initiator,
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(51u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSFskActiveInitiatorObject::MaxSupportedVersion,
    object.Descriptor().key.version);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(name), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, out));
  EXPECT_EQ(initiator, out);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(3u, out));
}

TEST(CosemSFskActiveInitiatorObject, MutableAttributesHonorAccessMode)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 1u, 0u, 255u);
  const dlms::cosem::CosemByteBuffer initiator =
    SampleSFskActiveInitiator();
  // Default "not registered" descriptor per IEC 62056-6-2 ED4 4.10.4.2.2:
  // system_title = octet-string of 0s, MAC_address = NO-BODY (0x0FFE
  // is the IC 50 default; 0x0000 is the standard "no body" value),
  // L_SAP_selector = 0.
  const dlms::cosem::CosemByteBuffer replacement = BytesFromList({
    0x02u, 0x03u,
    0x09u, 0x08u, 0x00u, 0x00u, 0x00u, 0x00u,
      0x00u, 0x00u, 0x00u, 0x00u,
    0x12u, 0x00u, 0x00u,
    0x11u, 0x00u});

  dlms::cosem::CosemSFskActiveInitiatorObject writable =
    MakeSFskActiveInitiatorObject(
      name, initiator,
      dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(2u, replacement));
  EXPECT_EQ(replacement, writable.ActiveInitiator());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, replacement));

  dlms::cosem::CosemSFskActiveInitiatorObject readOnly =
    MakeSFskActiveInitiatorObject(
      name, initiator,
      dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, replacement));
  EXPECT_EQ(initiator, readOnly.ActiveInitiator());
}

TEST(CosemSFskActiveInitiatorObject, MethodsReturnUnsupportedFeature)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 1u, 0u, 255u);
  dlms::cosem::CosemSFskActiveInitiatorObject object =
    MakeSFskActiveInitiatorObject(
      name, SampleSFskActiveInitiator(),
      dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
  for (std::uint8_t method : {0u, 2u, 99u, 255u}) {
    out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSFskActiveInitiatorObject, NormalizesVersionAboveMax)
{
  const dlms::cosem::CosemLogicalName name =
    dlms::cosem::CosemLogicalName(0u, 0u, 26u, 1u, 0u, 255u);
  dlms::cosem::CosemSFskActiveInitiatorObject object(
    name, SampleSFskActiveInitiator(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(
    dlms::cosem::CosemSFskActiveInitiatorObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}


