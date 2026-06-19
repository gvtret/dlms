// SPDX-License-Identifier: Apache-2.0
//
// Per-IC tests for CosemAutoConnectObject (class_id=29, version=2)
// per IEC 62056-6-2 ED4 (2021) §4.4.6 and DLMS UA Blue Book Ed. 12.1
// §4.4.6 (Auto connect). The typed surface exposes:
//   - mode            : enum
//   - repetitions     : unsigned
//   - repetition_delay: long-unsigned
//   - calling_window  : array of structure { start: date-time,
//                                            end:   date-time }
//   - destination_list: array of octet-string
// Specific method 1 "connect" (data) is defined from version 2 onward
// but is not driven by the built-in object — it returns
// UnsupportedFeature; every other method id reports MethodNotFound.

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include "dlms/cosem/cosem_status.hpp"
#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/simple_objects.hpp"
#include "dlms/cosem/types/date_time.hpp"

namespace {

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  return dlms::cosem::CosemByteBuffer(bytes.begin(), bytes.end());
}

dlms::cosem::CosemByteBuffer EncodedLogicalName(
  const dlms::cosem::CosemLogicalName& name)
{
  return BytesFromList({
    0x09u, 0x06u,
    name[0], name[1], name[2], name[3], name[4], name[5]});
}

dlms::cosem::CosemLogicalName SampleName()
{
  // 0-0:2.1.0.255 — Auto connect, channel 1
  return dlms::cosem::CosemLogicalName(0u, 0u, 2u, 1u, 0u, 255u);
}

// Build a date-time via the typed setters (default ctor = all "unspecified",
// so we explicitly assign every field we care about and clear DST/clock-status
// to zero for a deterministic, parser-friendly value).
dlms::cosem::types::DateTime MakeDateTime(
  std::uint16_t year,
  std::uint8_t month,
  std::uint8_t day,
  std::uint8_t hour,
  std::uint8_t minute,
  std::uint8_t second)
{
  dlms::cosem::types::DateTime dt;
  EXPECT_TRUE(dt.SetYear(year));
  EXPECT_TRUE(dt.SetMonth(month));
  EXPECT_TRUE(dt.SetDayOfMonth(day));
  EXPECT_TRUE(dt.SetHour(hour));
  EXPECT_TRUE(dt.SetMinute(minute));
  EXPECT_TRUE(dt.SetSecond(second));
  EXPECT_TRUE(dt.SetHundredths(0u));
  EXPECT_TRUE(dt.SetDeviation(0));
  dt.SetClockStatus(0u);
  return dt;
}

std::vector<std::uint8_t> EncodedDateTime(
  const dlms::cosem::types::DateTime& dt)
{
  std::array<std::uint8_t, dlms::cosem::types::DateTime::WireSize>
    bytes = dt.ToBytes();
  std::vector<std::uint8_t> out;
  out.push_back(0x09u);  // octet-string tag
  out.push_back(static_cast<std::uint8_t>(bytes.size()));
  out.insert(out.end(), bytes.begin(), bytes.end());
  return out;
}

dlms::cosem::CosemAutoConnectObject::CallingWindowEntry
SampleWindow()
{
  dlms::cosem::CosemAutoConnectObject::CallingWindowEntry e;
  e.start = MakeDateTime(2026u, 6u, 22u, 0u, 0u, 0u);
  e.end   = MakeDateTime(2026u, 6u, 22u, 6u, 0u, 0u);
  return e;
}

dlms::cosem::CosemAutoConnectObject MakeObject(
  dlms::cosem::AttributeAccessMode access,
  std::uint8_t mode = 1u,
  std::uint8_t repetitions = 3u,
  std::uint16_t repetitionDelay = 60u)
{
  std::vector<dlms::cosem::CosemAutoConnectObject::CallingWindowEntry>
    windows{SampleWindow()};
  std::vector<std::vector<std::uint8_t>> dests{
    {'+', '7', '1', '2', '3'}};
  return dlms::cosem::CosemAutoConnectObject(
    SampleName(), mode, repetitions, repetitionDelay,
    windows, dests, access);
}

} // namespace

TEST(CosemAutoConnectObject, DescriptorAndAccessRights)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(29u, obj.Descriptor().key.classId);
  EXPECT_EQ(2u, obj.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemAutoConnectObject::MaxSupportedVersion,
    obj.Descriptor().key.version);

  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            obj.AccessRights().AttributeAccess(1u));
  for (std::uint8_t id = 2u; id <= 6u; ++id) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              obj.AccessRights().AttributeAccess(id))
      << "attr id " << static_cast<unsigned>(id);
  }
}

TEST(CosemAutoConnectObject, TypedGettersReflectCtor)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite,
               101u, 5u, 30u);
  EXPECT_EQ(101u, obj.Mode());
  EXPECT_EQ(5u, obj.Repetitions());
  EXPECT_EQ(30u, obj.RepetitionDelay());
  ASSERT_EQ(1u, obj.CallingWindow().size());
  EXPECT_EQ(SampleWindow(), obj.CallingWindow().front());
  ASSERT_EQ(1u, obj.DestinationList().size());
  const std::vector<std::uint8_t> expectedDest{'+', '7', '1', '2', '3'};
  EXPECT_EQ(expectedDest, obj.DestinationList().front());
}

TEST(CosemAutoConnectObject, ReadAttributeEmitsAxdr)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(SampleName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x01u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x11u, 0x03u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x3Cu}), out);

  // calling_window: array(1) { structure(2) { dt(12), dt(12) } }
  std::vector<std::uint8_t> expectedCw;
  expectedCw.push_back(0x01u);  // array tag
  expectedCw.push_back(0x01u);  // count
  expectedCw.push_back(0x02u);  // structure tag
  expectedCw.push_back(0x02u);  // field count
  const std::vector<std::uint8_t> s = EncodedDateTime(SampleWindow().start);
  const std::vector<std::uint8_t> e = EncodedDateTime(SampleWindow().end);
  expectedCw.insert(expectedCw.end(), s.begin(), s.end());
  expectedCw.insert(expectedCw.end(), e.begin(), e.end());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(5u, out));
  EXPECT_EQ(expectedCw, out);

  // destination_list: array(1) { octet-string "+7123" }
  std::vector<std::uint8_t> expectedDl{
    0x01u, 0x01u, 0x09u, 0x05u, '+', '7', '1', '2', '3'};
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(6u, out));
  EXPECT_EQ(expectedDl, out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(7u, out));
}

TEST(CosemAutoConnectObject, WriteAttributeRoundTripsTypedValues)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // mode <- enum 2
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(2u, BytesFromList({0x16u, 0x02u})));
  EXPECT_EQ(2u, obj.Mode());

  // repetitions <- unsigned 7
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(3u, BytesFromList({0x11u, 0x07u})));
  EXPECT_EQ(7u, obj.Repetitions());

  // repetition_delay <- long-unsigned 0x1234
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(4u, BytesFromList({0x12u, 0x12u, 0x34u})));
  EXPECT_EQ(0x1234u, obj.RepetitionDelay());

  // calling_window <- empty array
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(5u, BytesFromList({0x01u, 0x00u})));
  EXPECT_TRUE(obj.CallingWindow().empty());

  // destination_list <- array(2) { "A", "BC" }
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(6u, BytesFromList({
              0x01u, 0x02u,
              0x09u, 0x01u, 'A',
              0x09u, 0x02u, 'B', 'C'})));
  ASSERT_EQ(2u, obj.DestinationList().size());
  EXPECT_EQ((std::vector<std::uint8_t>{'A'}), obj.DestinationList()[0]);
  EXPECT_EQ((std::vector<std::uint8_t>{'B', 'C'}), obj.DestinationList()[1]);
}

TEST(CosemAutoConnectObject, WriteAttributeRejectsMalformedPayload)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // mode: wrong tag (unsigned where enum expected)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x11u, 0x02u})));
  // mode: truncated
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x16u})));
  // mode: trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x16u, 0x01u, 0xFFu})));
  // mode: empty
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, dlms::cosem::CosemByteBuffer()));

  // repetition_delay: wrong tag (unsigned where long-unsigned expected)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(4u, BytesFromList({0x11u, 0x05u})));

  // calling_window: structure with wrong field count
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(5u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x01u,
              0x09u, 0x0Cu,
              0x07u, 0xEEu, 0x06u, 0x16u, 0xFFu, 0x00u, 0x00u, 0x00u,
              0x00u, 0x80u, 0x00u, 0x00u})));

  // calling_window: date-time octet-string with wrong size
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(5u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x02u,
              0x09u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u,
              0x09u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u})));

  // destination_list: declared count larger than payload
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(6u, BytesFromList({0x01u, 0x02u, 0x09u, 0x01u, 'A'})));

  // destination_list: trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(6u, BytesFromList({
              0x01u, 0x01u, 0x09u, 0x01u, 'A', 0xFFu})));

  // Original values preserved on every failed write.
  EXPECT_EQ(1u, obj.Mode());
  EXPECT_EQ(60u, obj.RepetitionDelay());
  ASSERT_EQ(1u, obj.CallingWindow().size());
  EXPECT_EQ(SampleWindow(), obj.CallingWindow().front());
  ASSERT_EQ(1u, obj.DestinationList().size());
}

TEST(CosemAutoConnectObject, ReadOnlyDeniesWrites)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 6u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              obj.WriteAttribute(id, BytesFromList({0x00u})))
      << "attr id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(1u, obj.Mode());
  EXPECT_EQ(3u, obj.Repetitions());
  EXPECT_EQ(60u, obj.RepetitionDelay());
}

TEST(CosemAutoConnectObject, LogicalNameWriteDenied)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(1u, BytesFromList({0x09u, 0x06u,
              0u, 0u, 2u, 1u, 0u, 255u})));
}

TEST(CosemAutoConnectObject, UnknownAttributeIsAttributeNotFound)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(99u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.WriteAttribute(99u, BytesFromList({0x00u})));
}

TEST(CosemAutoConnectObject, ConnectMethodIsUnsupportedFeature)
{
  dlms::cosem::CosemAutoConnectObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            obj.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());

  for (std::uint8_t method : {2u, 3u}) {
    out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              obj.InvokeMethod(method, in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemAutoConnectObject, LegacyVersion0ReportsMethodNotFound)
{
  // PSTN auto dial (v0) defines no specific methods at all; the
  // built-in object must surface MethodNotFound for every id.
  std::vector<dlms::cosem::CosemAutoConnectObject::CallingWindowEntry>
    windows{SampleWindow()};
  std::vector<std::vector<std::uint8_t>> dests{{'+', '7'}};
  dlms::cosem::CosemAutoConnectObject legacy(
    SampleName(), 1u, 3u, 60u, windows, dests,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    static_cast<std::uint8_t>(0u));
  EXPECT_EQ(0u, legacy.Descriptor().key.version);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              legacy.InvokeMethod(method, in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemAutoConnectObject, NormalizesVersionAboveMax)
{
  std::vector<dlms::cosem::CosemAutoConnectObject::CallingWindowEntry>
    windows{SampleWindow()};
  std::vector<std::vector<std::uint8_t>> dests{{'+', '7'}};
  dlms::cosem::CosemAutoConnectObject obj(
    SampleName(), 1u, 3u, 60u, windows, dests,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    static_cast<std::uint8_t>(99u));
  EXPECT_EQ(
    dlms::cosem::CosemAutoConnectObject::MaxSupportedVersion,
    obj.Descriptor().key.version);
}
