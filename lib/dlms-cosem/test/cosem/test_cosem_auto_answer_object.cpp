// SPDX-License-Identifier: Apache-2.0
//
// Per-IC tests for CosemAutoAnswerObject (class_id=28, version=0)
// per IEC 62056-6-2 ED4 (2021) sec.4.4.5 and DLMS UA Blue Book Ed. 12.1
// sec.4.4.5 (Auto answer). The typed surface exposes:
//   - mode                    : enum
//   - listening_window        : array of structure { start: date-time,
//                                                    end:   date-time }
//   - status                  : enum (read-only, dynamic)
//   - number_of_calls         : unsigned
//   - number_of_rings         : structure { in_window:     unsigned,
//                                            out_of_window: unsigned }
//   - list_of_allowed_callers : array of structure { caller_id: octet-string,
//                                                     call_type: enum }
// The IC defines no specific methods, so every method id reports
// MethodNotFound.

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
  return dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
}

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
  out.push_back(0x09u);
  out.push_back(static_cast<std::uint8_t>(bytes.size()));
  out.insert(out.end(), bytes.begin(), bytes.end());
  return out;
}

dlms::cosem::CosemAutoAnswerObject::ListeningWindowEntry SampleWindow()
{
  dlms::cosem::CosemAutoAnswerObject::ListeningWindowEntry e;
  e.start = MakeDateTime(2026u, 6u, 22u, 0u, 0u, 0u);
  e.end   = MakeDateTime(2026u, 6u, 22u, 6u, 0u, 0u);
  return e;
}

dlms::cosem::CosemAutoAnswerObject MakeObject(
  dlms::cosem::AttributeAccessMode access,
  std::uint8_t mode = 1u,
  std::uint8_t status = 2u,
  std::uint8_t numberOfCalls = 5u)
{
  std::vector<dlms::cosem::CosemAutoAnswerObject::ListeningWindowEntry>
    windows{SampleWindow()};
  dlms::cosem::CosemAutoAnswerObject::NumberOfRings rings{2u, 4u};
  std::vector<dlms::cosem::CosemAutoAnswerObject::AllowedCaller> callers;
  return dlms::cosem::CosemAutoAnswerObject(
    SampleName(), mode, windows, status, numberOfCalls, rings, callers, access);
}

} // namespace

TEST(CosemAutoAnswerObject, DescriptorAndAccessRights)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(28u, obj.Descriptor().key.classId);
  EXPECT_EQ(0u, obj.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemAutoAnswerObject::MaxSupportedVersion,
    obj.Descriptor().key.version);

  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            obj.AccessRights().AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            obj.AccessRights().AttributeAccess(2u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            obj.AccessRights().AttributeAccess(3u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            obj.AccessRights().AttributeAccess(4u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            obj.AccessRights().AttributeAccess(5u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            obj.AccessRights().AttributeAccess(6u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            obj.AccessRights().AttributeAccess(7u));
}

TEST(CosemAutoAnswerObject, TypedGettersReflectCtor)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite,
               101u, 7u, 9u);
  EXPECT_EQ(101u, obj.Mode());
  EXPECT_EQ(7u, obj.Status());
  EXPECT_EQ(9u, obj.NumberOfCalls());
  EXPECT_EQ(2u, obj.GetNumberOfRings().inWindow);
  EXPECT_EQ(4u, obj.GetNumberOfRings().outOfWindow);
  ASSERT_EQ(1u, obj.ListeningWindow().size());
  EXPECT_EQ(SampleWindow().start.ToBytes(),
            obj.ListeningWindow().front().start.ToBytes());
  EXPECT_EQ(SampleWindow().end.ToBytes(),
            obj.ListeningWindow().front().end.ToBytes());
  EXPECT_TRUE(obj.ListOfAllowedCallers().empty());
}

TEST(CosemAutoAnswerObject, ReadAttributeEmitsAxdr)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(SampleName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x01u}), out);

  std::vector<std::uint8_t> expectedLw;
  expectedLw.push_back(0x01u);
  expectedLw.push_back(0x01u);
  expectedLw.push_back(0x02u);
  expectedLw.push_back(0x02u);
  const std::vector<std::uint8_t> s = EncodedDateTime(SampleWindow().start);
  const std::vector<std::uint8_t> e = EncodedDateTime(SampleWindow().end);
  expectedLw.insert(expectedLw.end(), s.begin(), s.end());
  expectedLw.insert(expectedLw.end(), e.begin(), e.end());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(3u, out));
  EXPECT_EQ(expectedLw, out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x02u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(5u, out));
  EXPECT_EQ(BytesFromList({0x11u, 0x05u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(6u, out));
  EXPECT_EQ(BytesFromList({
    0x02u, 0x02u,
    0x11u, 0x02u,
    0x11u, 0x04u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(7u, out));
  EXPECT_EQ(BytesFromList({0x01u, 0x00u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(8u, out));
}

TEST(CosemAutoAnswerObject, WriteAttributeRoundTripsTypedValues)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(2u, BytesFromList({0x16u, 0x03u})));
  EXPECT_EQ(3u, obj.Mode());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(3u, BytesFromList({0x01u, 0x00u})));
  EXPECT_TRUE(obj.ListeningWindow().empty());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(5u, BytesFromList({0x11u, 0x0Cu})));
  EXPECT_EQ(12u, obj.NumberOfCalls());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(6u, BytesFromList({
              0x02u, 0x02u,
              0x11u, 0x07u,
              0x11u, 0x0Bu})));
  EXPECT_EQ(7u, obj.GetNumberOfRings().inWindow);
  EXPECT_EQ(11u, obj.GetNumberOfRings().outOfWindow);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(7u, BytesFromList({
              0x01u, 0x02u,
              0x02u, 0x02u, 0x09u, 0x03u, '1', '2', '3', 0x16u, 0x00u,
              0x02u, 0x02u, 0x09u, 0x02u, 'A', 'B', 0x16u, 0x01u})));
  ASSERT_EQ(2u, obj.ListOfAllowedCallers().size());
  EXPECT_EQ((std::vector<std::uint8_t>{'1', '2', '3'}),
            obj.ListOfAllowedCallers()[0].callerId);
  EXPECT_EQ(0u, obj.ListOfAllowedCallers()[0].callType);
  EXPECT_EQ((std::vector<std::uint8_t>{'A', 'B'}),
            obj.ListOfAllowedCallers()[1].callerId);
  EXPECT_EQ(1u, obj.ListOfAllowedCallers()[1].callType);
}

TEST(CosemAutoAnswerObject, WriteAttributeRejectsMalformedPayload)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x11u, 0x02u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x16u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x16u, 0x01u, 0xFFu})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, dlms::cosem::CosemByteBuffer()));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(5u, BytesFromList({0x16u, 0x05u})));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(6u, BytesFromList({
              0x02u, 0x03u,
              0x11u, 0x01u, 0x11u, 0x02u, 0x11u, 0x03u})));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(6u, BytesFromList({
              0x02u, 0x02u,
              0x11u, 0x01u, 0x11u, 0x02u, 0xFFu})));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(3u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x01u,
              0x09u, 0x0Cu,
              0x07u, 0xEEu, 0x06u, 0x16u, 0xFFu, 0x00u, 0x00u, 0x00u,
              0x00u, 0x80u, 0x00u, 0x00u})));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(3u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x02u,
              0x09u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u,
              0x09u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u})));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(7u, BytesFromList({
              0x01u, 0x02u,
              0x02u, 0x02u, 0x09u, 0x01u, 'X', 0x16u, 0x00u})));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(7u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x02u, 0x09u, 0x01u, 'X'})));

  EXPECT_EQ(1u, obj.Mode());
  EXPECT_EQ(5u, obj.NumberOfCalls());
  EXPECT_EQ(2u, obj.GetNumberOfRings().inWindow);
  EXPECT_EQ(4u, obj.GetNumberOfRings().outOfWindow);
  ASSERT_EQ(1u, obj.ListeningWindow().size());
}

TEST(CosemAutoAnswerObject, ReadOnlyDeniesWrites)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id : {2u, 3u, 5u, 6u, 7u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              obj.WriteAttribute(
                static_cast<std::uint8_t>(id), BytesFromList({0x00u})))
      << "attr id " << static_cast<unsigned>(id);
  }
  EXPECT_EQ(1u, obj.Mode());
  EXPECT_EQ(5u, obj.NumberOfCalls());
}

TEST(CosemAutoAnswerObject, LogicalNameAndStatusWritesDenied)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(1u, BytesFromList({0x09u, 0x06u,
              0u, 0u, 2u, 2u, 0u, 255u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(4u, BytesFromList({0x16u, 0x07u})));
}

TEST(CosemAutoAnswerObject, SetStatusUpdatesDynamicField)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  obj.SetStatus(9u);
  EXPECT_EQ(9u, obj.Status());
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x09u}), out);
}

TEST(CosemAutoAnswerObject, UnknownAttributeIsAttributeNotFound)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(99u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.WriteAttribute(99u, BytesFromList({0x00u})));
}

TEST(CosemAutoAnswerObject, NoMethodsDefined)
{
  dlms::cosem::CosemAutoAnswerObject obj =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              obj.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemAutoAnswerObject, NormalizesVersionAboveMax)
{
  std::vector<dlms::cosem::CosemAutoAnswerObject::ListeningWindowEntry>
    windows{SampleWindow()};
  dlms::cosem::CosemAutoAnswerObject::NumberOfRings rings{2u, 4u};
  std::vector<dlms::cosem::CosemAutoAnswerObject::AllowedCaller> callers;
  dlms::cosem::CosemAutoAnswerObject obj(
    SampleName(), 1u, windows, 2u, 5u, rings, callers,
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    static_cast<std::uint8_t>(99u));
  EXPECT_EQ(
    dlms::cosem::CosemAutoAnswerObject::MaxSupportedVersion,
    obj.Descriptor().key.version);
}
