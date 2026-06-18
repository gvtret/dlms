// Tests for IC 65 (Parameter Monitor) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.5.10 / DLMS UA Blue Book Ed. 12.1
// §4.5.10.
//
// Typed attributes:
//   capture_time      (id 3) : types::DateTime
//   parameter_list    (id 4) : array of types::MonitoredValue
//   hash_algorithm_id (id 6) : enum
//     {Sha256=0, Sha384=1, Sha256Last16=2, Sha256Last8=3,
//      Sha256Last4=4}
//
// `changed_parameter` (id 2), `parameter_list_name` (id 5),
// `parameter_value_digest` (id 7) and `parameter_values` (id 8)
// remain opaque CosemByteBuffer — they depend on the typed
// discriminated-union (CHOICE) machinery shared with the other
// `value`-bearing ICs and will be migrated when that lands.
//
// Methods 1 `add_parameter` / 2 `delete_parameter` are implemented
// in-place: they decode `parameter_list_element` and append/remove
// from parameter_list. delete with no match returns OtherReason.

#include <cstdint>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"
#include "dlms/cosem/types/date_time.hpp"
#include "dlms/cosem/types/monitored_value.hpp"

#include <gtest/gtest.h>

namespace {

using Object = dlms::cosem::CosemParameterMonitorObject;
using HashId = dlms::cosem::CosemParameterMonitorObject::HashAlgorithmId;

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 16u, 2u, 0u, 255u);
}

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  dlms::cosem::CosemByteBuffer out;
  out.reserve(bytes.size());
  for (std::uint8_t b : bytes) out.push_back(b);
  return out;
}

dlms::cosem::types::DateTime SampleDateTime()
{
  // year=2025 month=06 day=22 weekday=7 hour=12 min=34 sec=56 hsec=00
  // deviation = 0x8000 (unspecified), clock_status = 0xFF.
  const std::uint8_t bytes[12] = {
    0x07, 0xE9, 0x06, 0x16, 0x07, 0x0C, 0x22, 0x38,
    0x00, 0x80, 0x00, 0xFF
  };
  dlms::cosem::types::DateTime dt;
  EXPECT_TRUE(
    dlms::cosem::types::DateTime::TryFromBytes(bytes, 12u, dt));
  return dt;
}

std::vector<dlms::cosem::types::MonitoredValue> SampleParameterList()
{
  std::vector<dlms::cosem::types::MonitoredValue> v;
  v.emplace_back(
    1u,
    dlms::cosem::CosemLogicalName(0u, 0u, 96u, 1u, 0u, 255u),
    static_cast<std::int8_t>(2));
  v.emplace_back(
    3u,
    dlms::cosem::CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u),
    static_cast<std::int8_t>(2));
  return v;
}

Object MakeObject(
  dlms::cosem::AttributeAccessMode mode =
    dlms::cosem::AttributeAccessMode::ReadAndWrite)
{
  return Object(
    MakeName(),
    BytesFromList({0x09, 0x01, 0x00}),  // changedParameter (opaque)
    SampleDateTime(),
    SampleParameterList(),
    BytesFromList({0x09, 0x04, 't', 'e', 's', 't'}),  // listName
    HashId::Sha384,
    BytesFromList({0x09, 0x02, 0xAA, 0xBB}),  // digest
    BytesFromList({0x02, 0x00}),               // values (empty struct)
    mode);
}

} // namespace

TEST(CosemParameterMonitorObject, DescriptorAndAccessRights)
{
  Object o = MakeObject();
  EXPECT_EQ(65u, o.Descriptor().key.classId);
  EXPECT_EQ(
    Object::MaxSupportedVersion, o.Descriptor().key.version);
  const dlms::cosem::CosemAccessRights& r = o.AccessRights();
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadOnly,
    r.AttributeAccess(1u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    r.AttributeAccess(2u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    r.AttributeAccess(3u));
  EXPECT_EQ(
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    r.AttributeAccess(8u));
}

TEST(CosemParameterMonitorObject, ReadCaptureTimeAsOctetString)
{
  Object o = MakeObject();
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, o.ReadAttribute(3u, out));
  ASSERT_EQ(14u, out.size());
  EXPECT_EQ(0x09u, out[0]);
  EXPECT_EQ(0x0Cu, out[1]);  // length = 12
  EXPECT_EQ(0x07u, out[2]);
  EXPECT_EQ(0xE9u, out[3]);  // 2025
}

TEST(CosemParameterMonitorObject, ReadParameterListAsArray)
{
  Object o = MakeObject();
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, o.ReadAttribute(4u, out));
  ASSERT_GE(out.size(), 2u);
  EXPECT_EQ(0x01u, out[0]);  // array tag
  EXPECT_EQ(0x02u, out[1]);  // count = 2
  // First element: structure(3) { LU 0001, OS(6) 0000600100FF, I 02 }
  EXPECT_EQ(0x02u, out[2]);  // structure tag
  EXPECT_EQ(0x03u, out[3]);  // field count
  EXPECT_EQ(0x12u, out[4]);  // long-unsigned tag
}

TEST(CosemParameterMonitorObject, ReadHashAlgorithmIdAsEnum)
{
  Object o = MakeObject();
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, o.ReadAttribute(6u, out));
  ASSERT_EQ(2u, out.size());
  EXPECT_EQ(0x16u, out[0]);  // enum tag
  EXPECT_EQ(0x01u, out[1]);  // Sha384
}

TEST(CosemParameterMonitorObject, WriteCaptureTimeRoundTrips)
{
  Object o = MakeObject();
  // New time: 2024-01-15 08:00:00 hsec=00 dev=unspec status=0xFF
  const dlms::cosem::CosemByteBuffer payload = BytesFromList({
    0x09, 0x0C,
    0x07, 0xE8, 0x01, 0x0F, 0xFF, 0x08, 0x00, 0x00,
    0x00, 0x80, 0x00, 0xFF
  });
  EXPECT_EQ(
    dlms::cosem::CosemStatus::Ok, o.WriteAttribute(3u, payload));
  EXPECT_EQ(2024u, o.CaptureTime().Year());
  EXPECT_EQ(1u, o.CaptureTime().Month());
  EXPECT_EQ(15u, o.CaptureTime().DayOfMonth());
}

TEST(CosemParameterMonitorObject, WriteCaptureTimeRejectsMalformed)
{
  Object o = MakeObject();
  // Wrong tag (boolean instead of octet-string).
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(3u, BytesFromList({0x03, 0x01, 0x01})));
  // Wrong length (11 bytes).
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(3u, BytesFromList({
      0x09, 0x0B,
      0x07, 0xE8, 0x01, 0x0F, 0xFF, 0x08, 0x00, 0x00,
      0x00, 0x80, 0x00
    })));
  // Truncated payload.
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(3u, BytesFromList({0x09, 0x0C, 0x07})));
}

TEST(CosemParameterMonitorObject, WriteParameterListReplacesContents)
{
  Object o = MakeObject();
  // Replace with single-element array.
  // array(1) { structure(3) { LU 0050, OS(6) 0000020000FF, I 01 } }
  const dlms::cosem::CosemByteBuffer payload = BytesFromList({
    0x01, 0x01,
    0x02, 0x03,
    0x12, 0x00, 0x50,
    0x09, 0x06, 0x00, 0x00, 0x02, 0x00, 0x00, 0xFF,
    0x0F, 0x01
  });
  EXPECT_EQ(
    dlms::cosem::CosemStatus::Ok, o.WriteAttribute(4u, payload));
  ASSERT_EQ(1u, o.ParameterList().size());
  EXPECT_EQ(0x50u, o.ParameterList()[0].ClassId());
  EXPECT_EQ(1, o.ParameterList()[0].AttributeIndex());
}

TEST(CosemParameterMonitorObject, WriteParameterListRejectsMalformed)
{
  Object o = MakeObject();
  // Wrong tag (structure instead of array).
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(4u, BytesFromList({0x02, 0x00})));
  // Trailing garbage after array.
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(4u, BytesFromList({0x01, 0x00, 0xAA})));
}

TEST(CosemParameterMonitorObject, WriteHashAlgorithmIdValidatesRange)
{
  Object o = MakeObject();
  // Valid value: Sha256Last16 = 2.
  EXPECT_EQ(
    dlms::cosem::CosemStatus::Ok,
    o.WriteAttribute(6u, BytesFromList({0x16, 0x02})));
  EXPECT_EQ(HashId::Sha256Last16, o.GetHashAlgorithmId());
  // Out-of-range raw value (5).
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(6u, BytesFromList({0x16, 0x05})));
  EXPECT_EQ(HashId::Sha256Last16, o.GetHashAlgorithmId());  // unchanged
  // Wrong tag.
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(6u, BytesFromList({0x0F, 0x02})));
  // Trailing garbage.
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.WriteAttribute(6u, BytesFromList({0x16, 0x02, 0xFF})));
}

TEST(CosemParameterMonitorObject, AddParameterAppendsEntry)
{
  Object o = MakeObject();
  const std::size_t before = o.ParameterList().size();
  // parameter_list_element: structure(3) { LU 0070, OS(6) ..., I 03 }
  const dlms::cosem::CosemByteBuffer payload = BytesFromList({
    0x02, 0x03,
    0x12, 0x00, 0x46,
    0x09, 0x06, 0x01, 0x00, 0x00, 0x05, 0x00, 0xFF,
    0x0F, 0x03
  });
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(
    dlms::cosem::CosemStatus::Ok,
    o.InvokeMethod(1u, payload, out));
  EXPECT_EQ(before + 1u, o.ParameterList().size());
  EXPECT_EQ(0x46u, o.ParameterList().back().ClassId());
  EXPECT_EQ(3, o.ParameterList().back().AttributeIndex());
}

TEST(CosemParameterMonitorObject, AddParameterRejectsMalformed)
{
  Object o = MakeObject();
  const std::size_t before = o.ParameterList().size();
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(
    dlms::cosem::CosemStatus::InvalidArgument,
    o.InvokeMethod(1u, BytesFromList({0x01, 0x00}), out));
  EXPECT_EQ(before, o.ParameterList().size());
}

TEST(CosemParameterMonitorObject, DeleteParameterRemovesMatch)
{
  Object o = MakeObject();
  const std::size_t before = o.ParameterList().size();
  ASSERT_GE(before, 1u);
  // Encode the first existing entry: class_id=1, OBIS 0000600100FF, attr=2.
  const dlms::cosem::CosemByteBuffer payload = BytesFromList({
    0x02, 0x03,
    0x12, 0x00, 0x01,
    0x09, 0x06, 0x00, 0x00, 0x60, 0x01, 0x00, 0xFF,
    0x0F, 0x02
  });
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(
    dlms::cosem::CosemStatus::Ok,
    o.InvokeMethod(2u, payload, out));
  EXPECT_EQ(before - 1u, o.ParameterList().size());
}

TEST(CosemParameterMonitorObject, DeleteParameterMissingReturnsObjectError)
{
  Object o = MakeObject();
  const std::size_t before = o.ParameterList().size();
  const dlms::cosem::CosemByteBuffer payload = BytesFromList({
    0x02, 0x03,
    0x12, 0xFF, 0xFF,
    0x09, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x0F, 0x7F
  });
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(
    dlms::cosem::CosemStatus::ObjectError,
    o.InvokeMethod(2u, payload, out));
  EXPECT_EQ(before, o.ParameterList().size());
}

TEST(CosemParameterMonitorObject, UnknownMethodReturnsMethodNotFound)
{
  Object o = MakeObject();
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(
    dlms::cosem::CosemStatus::MethodNotFound,
    o.InvokeMethod(99u, BytesFromList({}), out));
}

TEST(CosemParameterMonitorObject, ReadOnlyModeRejectsWrites)
{
  Object o = MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(
    dlms::cosem::CosemStatus::AccessDenied,
    o.WriteAttribute(3u, BytesFromList({
      0x09, 0x0C,
      0x07, 0xE8, 0x01, 0x0F, 0xFF, 0x08, 0x00, 0x00,
      0x00, 0x80, 0x00, 0xFF
    })));
  EXPECT_EQ(
    dlms::cosem::CosemStatus::AccessDenied,
    o.WriteAttribute(6u, BytesFromList({0x16, 0x00})));
  EXPECT_EQ(
    dlms::cosem::CosemStatus::AccessDenied,
    o.WriteAttribute(1u, BytesFromList({0x09, 0x06, 0,0,0,0,0,0})));
}

TEST(CosemParameterMonitorObject, IsValidHashAlgorithmIdBoundary)
{
  EXPECT_TRUE(Object::IsValidHashAlgorithmId(0u));
  EXPECT_TRUE(Object::IsValidHashAlgorithmId(4u));
  EXPECT_FALSE(Object::IsValidHashAlgorithmId(5u));
  EXPECT_FALSE(Object::IsValidHashAlgorithmId(255u));
}

TEST(CosemParameterMonitorObject, CtorNormalizesInvalidHashAlgorithm)
{
  Object o(
    MakeName(),
    BytesFromList({0x09, 0x00}),
    SampleDateTime(),
    SampleParameterList(),
    BytesFromList({}),
    static_cast<HashId>(99u),
    BytesFromList({}),
    BytesFromList({}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(HashId::Sha256, o.GetHashAlgorithmId());
}

TEST(CosemParameterMonitorObject, LegacyVersion0HidesExtendedAttrs)
{
  Object legacy(
    MakeName(),
    BytesFromList({0x09, 0x00}),
    SampleDateTime(),
    SampleParameterList(),
    BytesFromList({0x09, 0x04, 't', 'e', 's', 't'}),
    HashId::Sha384,
    BytesFromList({0x09, 0x00}),
    BytesFromList({0x02, 0x00}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    0u);
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(
    dlms::cosem::CosemStatus::AttributeNotFound,
    legacy.ReadAttribute(5u, out));
  EXPECT_EQ(
    dlms::cosem::CosemStatus::AttributeNotFound,
    legacy.ReadAttribute(6u, out));
  EXPECT_EQ(
    dlms::cosem::CosemStatus::AttributeNotFound,
    legacy.ReadAttribute(8u, out));
  dlms::cosem::CosemByteBuffer mout;
  EXPECT_EQ(
    dlms::cosem::CosemStatus::UnsupportedFeature,
    legacy.InvokeMethod(1u, BytesFromList({}), mout));
}

TEST(CosemParameterMonitorObject, NormalizesVersionAboveMax)
{
  Object o(
    MakeName(),
    BytesFromList({0x09, 0x00}),
    SampleDateTime(),
    SampleParameterList(),
    BytesFromList({}),
    HashId::Sha256,
    BytesFromList({}),
    BytesFromList({}),
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    Object::MaxSupportedVersion, o.Descriptor().key.version);
}
