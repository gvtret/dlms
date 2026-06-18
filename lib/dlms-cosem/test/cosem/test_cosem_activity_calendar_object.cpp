// Tests for CosemActivityCalendarObject (IC 20 "Activity calendar",
// class_id=20, version=0) following IEC 62056-6-2 ED4 (2021) section
// 4.5.5 and DLMS UA Blue Book Ed. 12.1 section 5.1.9.
//
// One file per IC per docs/production_readiness_roadmap.md P2.4.

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

namespace {

using dlms::cosem::AttributeAccessMode;
using dlms::cosem::CosemActivityCalendarObject;
using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::CosemStatus;
using dlms::cosem::types::DateTime;
using dlms::cosem::types::DayProfile;
using dlms::cosem::types::DayProfileAction;
using dlms::cosem::types::SeasonProfile;
using dlms::cosem::types::Time;
using dlms::cosem::types::WeekProfile;

CosemByteBuffer Bytes(std::initializer_list<std::uint8_t> v)
{
  return CosemByteBuffer(v.begin(), v.end());
}

Time MakeTime(std::uint8_t h, std::uint8_t m)
{
  Time t;
  t.SetHour(h);
  t.SetMinute(m);
  t.SetSecond(0u);
  t.SetHundredths(0u);
  return t;
}

DateTime MakeDateTimeAllWildcards()
{
  // Default-constructed DateTime has every field unspecified, which the
  // spec explicitly allows for season_start ("never starts").
  return DateTime();
}

DateTime MakeDateTimeJune21()
{
  DateTime dt;
  dt.SetYear(2021u);
  dt.SetMonth(6u);
  dt.SetDayOfMonth(21u);
  dt.SetHour(0u);
  dt.SetMinute(0u);
  dt.SetSecond(0u);
  dt.SetHundredths(0u);
  dt.SetDeviation(0);
  dt.SetClockStatus(0u);
  return dt;
}

CosemLogicalName ObisName()
{
  return CosemLogicalName(0u, 0u, 13u, 0u, 0u, 255u);
}

CosemLogicalName ScriptName()
{
  // Any valid OBIS; the activity-calendar tests don't dereference it.
  return CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u);
}

DayProfile MakeDayProfile(std::uint8_t dayId)
{
  std::vector<DayProfileAction> sched;
  sched.push_back(DayProfileAction(MakeTime(6u, 0u), ScriptName(), 1u));
  sched.push_back(DayProfileAction(MakeTime(22u, 0u), ScriptName(), 2u));
  return DayProfile(dayId, sched);
}

WeekProfile MakeWeekProfile(
  const CosemByteBuffer& name, std::uint8_t dayId)
{
  return WeekProfile(name, dayId, dayId, dayId, dayId, dayId, dayId, dayId);
}

SeasonProfile MakeSeason(
  const CosemByteBuffer& name,
  const DateTime& start,
  const CosemByteBuffer& weekName)
{
  return SeasonProfile(name, start, weekName);
}

struct ConsistentPassive
{
  CosemByteBuffer calendarName;
  std::vector<SeasonProfile> seasons;
  std::vector<WeekProfile> weeks;
  std::vector<DayProfile> days;
  DateTime activateTime;
};

ConsistentPassive MakeConsistentPassive()
{
  ConsistentPassive p;
  p.calendarName = Bytes({'P', 'A', 'S', 'S', 'I', 'V', 'E'});
  p.days.push_back(MakeDayProfile(1u));
  p.weeks.push_back(MakeWeekProfile(Bytes({'W', 'D'}), 1u));
  p.seasons.push_back(MakeSeason(
    Bytes({'S', 'U', 'M'}),
    MakeDateTimeAllWildcards(),
    Bytes({'W', 'D'})));
  p.activateTime = MakeDateTimeJune21();
  return p;
}

CosemActivityCalendarObject MakeObject(
  AttributeAccessMode passive = AttributeAccessMode::ReadAndWrite)
{
  const ConsistentPassive p = MakeConsistentPassive();
  return CosemActivityCalendarObject(
    ObisName(),
    Bytes({'A', 'C', 'T'}),
    {}, {}, {},  // active snapshots start empty
    p.calendarName, p.seasons, p.weeks, p.days,
    p.activateTime,
    passive);
}

} // namespace

TEST(CosemActivityCalendarObject, DescriptorReportsClassIdVersionAndName)
{
  const auto object = MakeObject();
  EXPECT_EQ(20u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    CosemActivityCalendarObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(ObisName(), object.Descriptor().key.logicalName);
}

TEST(CosemActivityCalendarObject, NormalizesVersionAboveMax)
{
  const ConsistentPassive p = MakeConsistentPassive();
  CosemActivityCalendarObject object(
    ObisName(),
    Bytes({'A', 'C', 'T'}),
    {}, {}, {},
    p.calendarName, p.seasons, p.weeks, p.days,
    p.activateTime,
    AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(
    CosemActivityCalendarObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}

TEST(CosemActivityCalendarObject, ConstructorAcceptsConsistentPassive)
{
  const auto object = MakeObject();
  EXPECT_EQ(1u, object.SeasonProfilePassive().size());
  EXPECT_EQ(1u, object.WeekProfileTablePassive().size());
  EXPECT_EQ(1u, object.DayProfileTablePassive().size());
}

TEST(CosemActivityCalendarObject,
     ConstructorDropsInvariantViolatingCollections)
{
  // Season references a week name that does not exist → ctor must drop
  // the whole season vector (safe-fallback construction).
  const ConsistentPassive p = MakeConsistentPassive();
  std::vector<SeasonProfile> badSeasons;
  badSeasons.push_back(MakeSeason(
    Bytes({'S', 'U', 'M'}),
    MakeDateTimeAllWildcards(),
    Bytes({'M', 'I', 'S', 'S'})));
  CosemActivityCalendarObject object(
    ObisName(),
    Bytes({'A', 'C', 'T'}),
    {}, {}, {},
    p.calendarName, badSeasons, p.weeks, p.days,
    p.activateTime,
    AttributeAccessMode::ReadAndWrite);
  EXPECT_TRUE(object.SeasonProfilePassive().empty());
  // Week and day tables are still consistent on their own, so they
  // survive the ctor.
  EXPECT_EQ(1u, object.WeekProfileTablePassive().size());
  EXPECT_EQ(1u, object.DayProfileTablePassive().size());
}

TEST(CosemActivityCalendarObject, ActiveAttributesAreReadOnly)
{
  auto object = MakeObject();
  const CosemByteBuffer anything = Bytes({0x09u, 0x01u, 0x00u});
  EXPECT_EQ(CosemStatus::AccessDenied,
            object.WriteAttribute(1u, anything));   // logical_name
  for (std::uint8_t id = 2u; id <= 5u; ++id) {
    EXPECT_EQ(CosemStatus::AccessDenied,
              object.WriteAttribute(id, anything));
  }
}

TEST(CosemActivityCalendarObject, PassiveCalendarNameRoundTrips)
{
  auto object = MakeObject();
  const CosemByteBuffer payload = Bytes({0x09u, 0x03u, 'N', 'E', 'W'});
  EXPECT_EQ(CosemStatus::Ok, object.WriteAttribute(6u, payload));
  EXPECT_EQ(payload, object.CalendarNamePassive());
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(payload, out);
}

TEST(CosemActivityCalendarObject, ReadOnlyPassiveAccessRejectsWrites)
{
  auto object = MakeObject(AttributeAccessMode::ReadOnly);
  const CosemByteBuffer anything = Bytes({0x09u, 0x01u, 0x00u});
  for (std::uint8_t id = 6u; id <= 10u; ++id) {
    EXPECT_EQ(CosemStatus::AccessDenied,
              object.WriteAttribute(id, anything));
  }
}

TEST(CosemActivityCalendarObject,
     SetSeasonProfilePassiveRejectsDanglingWeekName)
{
  auto object = MakeObject();
  std::vector<SeasonProfile> bad;
  bad.push_back(MakeSeason(
    Bytes({'X'}),
    MakeDateTimeAllWildcards(),
    Bytes({'D', 'O', 'E', 'S', '_', 'N', 'O', 'T'})));
  EXPECT_FALSE(object.SetSeasonProfilePassive(bad));
  // Original value preserved.
  EXPECT_EQ(1u, object.SeasonProfilePassive().size());
  EXPECT_EQ(Bytes({'S', 'U', 'M'}),
            object.SeasonProfilePassive()[0].Name());
}

TEST(CosemActivityCalendarObject,
     SetSeasonProfilePassiveRejectsDuplicateSeasonName)
{
  auto object = MakeObject();
  std::vector<SeasonProfile> bad;
  bad.push_back(MakeSeason(
    Bytes({'A'}), MakeDateTimeAllWildcards(), Bytes({'W', 'D'})));
  bad.push_back(MakeSeason(
    Bytes({'A'}), MakeDateTimeAllWildcards(), Bytes({'W', 'D'})));
  EXPECT_FALSE(object.SetSeasonProfilePassive(bad));
}

TEST(CosemActivityCalendarObject,
     SetWeekProfileTablePassiveRejectsDanglingDayId)
{
  auto object = MakeObject();
  std::vector<WeekProfile> bad;
  // day_id 99 is not in the day_profile_table.
  bad.push_back(MakeWeekProfile(Bytes({'W', 'D'}), 99u));
  EXPECT_FALSE(object.SetWeekProfileTablePassive(bad));
  EXPECT_EQ(1u, object.WeekProfileTablePassive().size());
}

TEST(CosemActivityCalendarObject,
     SetDayProfileTablePassiveRejectsDanglingReferenceFromWeek)
{
  auto object = MakeObject();
  // Replacing the day_profile_table with a table that does NOT contain
  // day_id 1 would orphan the existing week_profile_table → reject.
  std::vector<DayProfile> bad;
  bad.push_back(MakeDayProfile(7u));
  EXPECT_FALSE(object.SetDayProfileTablePassive(bad));
  EXPECT_EQ(1u, object.DayProfileTablePassive().size());
  EXPECT_EQ(1u, object.DayProfileTablePassive()[0].DayId());
}

TEST(CosemActivityCalendarObject, ActivatePassiveCalendarReturnsUnsupported)
{
  auto object = MakeObject();
  const CosemByteBuffer in = Bytes({0x0Fu, 0x00u});  // integer 0
  CosemByteBuffer out = Bytes({0xAAu});
  EXPECT_EQ(CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemActivityCalendarObject, UnknownMethodReturnsMethodNotFound)
{
  auto object = MakeObject();
  const CosemByteBuffer in = Bytes({0x00u});
  CosemByteBuffer out = Bytes({0xBBu});
  EXPECT_EQ(CosemStatus::MethodNotFound,
            object.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemActivityCalendarObject, UnknownAttributeReturnsAttributeNotFound)
{
  const auto object = MakeObject();
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::AttributeNotFound,
            object.ReadAttribute(11u, out));
}

TEST(CosemActivityCalendarObject, WriteAttributeRejectsMalformedSeasonProfile)
{
  auto object = MakeObject();
  // Empty payload — not a valid array → InvalidArgument.
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(7u, CosemByteBuffer()));
  // Original still intact.
  EXPECT_EQ(1u, object.SeasonProfilePassive().size());
}

TEST(CosemActivityCalendarObject, ValidatorsAreSelfConsistent)
{
  EXPECT_TRUE(CosemActivityCalendarObject::IsValidDayProfileTable({}));
  EXPECT_TRUE(CosemActivityCalendarObject::IsValidWeekProfileTable({}));
  EXPECT_TRUE(CosemActivityCalendarObject::IsValidSeasonProfile({}));

  // Duplicate day_id → invalid.
  std::vector<DayProfile> dupDays;
  dupDays.push_back(MakeDayProfile(1u));
  dupDays.push_back(MakeDayProfile(1u));
  EXPECT_FALSE(CosemActivityCalendarObject::IsValidDayProfileTable(dupDays));

  // Duplicate week name → invalid.
  std::vector<WeekProfile> dupWeeks;
  dupWeeks.push_back(MakeWeekProfile(Bytes({'X'}), 1u));
  dupWeeks.push_back(MakeWeekProfile(Bytes({'X'}), 1u));
  EXPECT_FALSE(
    CosemActivityCalendarObject::IsValidWeekProfileTable(dupWeeks));
}
