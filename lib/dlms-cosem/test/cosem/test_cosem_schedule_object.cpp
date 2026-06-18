// Tests for CosemScheduleObject (IC 10 "Schedule"). Per the roadmap
// rule "one test file per IC class". Covers:
//   * typed ctor & getter
//   * SetEntries validation (per-entry + unique-index)
//   * AXDR codec round-trip
//   * WriteAttribute: well-formed accept, malformed reject, access-mode
//     gate, duplicate-index reject
//   * InvokeMethod: enable_disable (1), insert (2), delete (3) per
//     IEC 62056-6-2 ED4 \u00a74.5.3.3

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::AttributeAccessMode;
using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::CosemScheduleObject;
using dlms::cosem::CosemStatus;
using dlms::cosem::types::Date;
using dlms::cosem::types::ScheduleTableEntry;
using dlms::cosem::types::Script;
using dlms::cosem::types::Time;

CosemLogicalName ScheduleLn()
{
  return CosemLogicalName(0u, 0u, 10u, 0u, 0u, 255u);
}

Date MakeDate(std::uint16_t year, std::uint8_t month, std::uint8_t dom)
{
  Date d;
  EXPECT_TRUE(d.SetYear(year));
  EXPECT_TRUE(d.SetMonth(month));
  EXPECT_TRUE(d.SetDayOfMonth(dom));
  return d;
}

Date WildcardDate()
{
  Date d;
  // default-constructed Date already exposes wildcards on every field.
  return d;
}

Time MakeTime(std::uint8_t h, std::uint8_t m, std::uint8_t s)
{
  Time t;
  EXPECT_TRUE(t.SetHour(h));
  EXPECT_TRUE(t.SetMinute(m));
  EXPECT_TRUE(t.SetSecond(s));
  EXPECT_TRUE(t.SetHundredths(0u));
  return t;
}

Script MakeScript(std::uint8_t lnTail, std::uint16_t selector)
{
  return Script(
    CosemLogicalName(0u, 0u, 10u, 100u, 100u, lnTail), selector);
}

ScheduleTableEntry MakeEntry(std::uint16_t index, bool enable = true)
{
  return ScheduleTableEntry(
    index, enable,
    MakeScript(1u, 1u),
    MakeTime(6u, 0u, 0u),
    ScheduleTableEntry::ValidityWindowAlways,
    0x7Fu,                                            // every weekday
    0u,
    WildcardDate(), WildcardDate());
}

std::vector<ScheduleTableEntry> SampleEntries()
{
  std::vector<ScheduleTableEntry> v;
  v.push_back(MakeEntry(1u, true));
  v.push_back(MakeEntry(2u, false));
  v.push_back(MakeEntry(3u, true));
  return v;
}

CosemByteBuffer EncodeEntries(
  const std::vector<ScheduleTableEntry>& entries)
{
  CosemScheduleObject probe(
    ScheduleLn(), entries, AttributeAccessMode::ReadAndWrite);
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, probe.ReadAttribute(2u, out));
  return out;
}

CosemByteBuffer EncodeSingleEntry(const ScheduleTableEntry& e)
{
  return EncodeEntries(std::vector<ScheduleTableEntry>(1u, e));
}

} // namespace

TEST(CosemScheduleObject, DescriptorAndDefaultRights)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(10u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(CosemScheduleObject::MaxSupportedVersion,
            object.Descriptor().key.version);

  EXPECT_EQ(AttributeAccessMode::ReadOnly,
            object.AccessRights().AttributeAccess(1u));
  EXPECT_EQ(AttributeAccessMode::ReadAndWrite,
            object.AccessRights().AttributeAccess(2u));
}

TEST(CosemScheduleObject, NormalizesVersionAboveMax)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(),
    AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(CosemScheduleObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemScheduleObject, ConstructorRejectsDuplicateIndexFallsBackToEmpty)
{
  std::vector<ScheduleTableEntry> dup;
  dup.push_back(MakeEntry(1u));
  dup.push_back(MakeEntry(1u));   // duplicate index
  CosemScheduleObject object(
    ScheduleLn(), dup, AttributeAccessMode::ReadAndWrite);
  EXPECT_TRUE(object.Entries().empty());
}

TEST(CosemScheduleObject, SetEntriesValidatesAndIsAtomic)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadAndWrite);
  ASSERT_EQ(3u, object.Entries().size());

  // Duplicate index: rejected, no mutation.
  std::vector<ScheduleTableEntry> bad;
  bad.push_back(MakeEntry(5u));
  bad.push_back(MakeEntry(5u));
  EXPECT_FALSE(object.SetEntries(bad));
  EXPECT_EQ(3u, object.Entries().size());

  // Well-formed replacement: accepted.
  std::vector<ScheduleTableEntry> repl;
  repl.push_back(MakeEntry(10u));
  EXPECT_TRUE(object.SetEntries(repl));
  ASSERT_EQ(1u, object.Entries().size());
  EXPECT_EQ(10u, object.Entries()[0].Index());

  // Empty collection is a valid state ("schedule cleared").
  EXPECT_TRUE(object.SetEntries(std::vector<ScheduleTableEntry>()));
  EXPECT_TRUE(object.Entries().empty());
}

TEST(CosemScheduleObject, AxdrRoundTripPreservesEntries)
{
  // Build a heterogeneous set: enabled/disabled, varied weekday mask,
  // varied specdays bits, concrete + wildcard dates.
  std::vector<ScheduleTableEntry> in;
  in.push_back(ScheduleTableEntry(
    1u, true, MakeScript(1u, 1u), MakeTime(6u, 0u, 0u),
    ScheduleTableEntry::ValidityWindowAlways,
    0x1Fu, 0u,
    MakeDate(2024u, 1u, 1u), MakeDate(2024u, 12u, 31u)));
  in.push_back(ScheduleTableEntry(
    2u, false, MakeScript(2u, 5u), MakeTime(22u, 30u, 15u),
    900u,
    0x60u,                                  // Sat+Sun
    (std::uint64_t{1} << 0) | (std::uint64_t{1} << 63),
    WildcardDate(), WildcardDate()));
  in.push_back(ScheduleTableEntry(
    65535u, true, MakeScript(3u, 0u), MakeTime(0u, 0u, 0u),
    1u, 0x7Fu, ~std::uint64_t{0},
    MakeDate(2024u, 6u, 15u), MakeDate(2024u, 6u, 15u)));

  CosemByteBuffer wire = EncodeEntries(in);

  CosemScheduleObject sink(
    ScheduleLn(), std::vector<ScheduleTableEntry>(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(CosemStatus::Ok, sink.WriteAttribute(2u, wire));
  ASSERT_EQ(in.size(), sink.Entries().size());
  for (std::size_t i = 0u; i < in.size(); ++i) {
    EXPECT_TRUE(in[i] == sink.Entries()[i]) << "entry " << i;
  }

  // Encoded form must be stable across the round trip.
  CosemByteBuffer re;
  EXPECT_EQ(CosemStatus::Ok, sink.ReadAttribute(2u, re));
  EXPECT_EQ(wire, re);
}

TEST(CosemScheduleObject, AxdrEncodesEmptyAsArrayLengthZero)
{
  CosemScheduleObject object(
    ScheduleLn(), std::vector<ScheduleTableEntry>(),
    AttributeAccessMode::ReadAndWrite);
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  // array tag (0x01) + length 0.
  ASSERT_EQ(2u, out.size());
  EXPECT_EQ(0x01u, out[0]);
  EXPECT_EQ(0x00u, out[1]);
}

TEST(CosemScheduleObject, WriteAttributeRejectsMalformedAndDuplicateIndex)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadAndWrite);
  const std::vector<ScheduleTableEntry> before = object.Entries();

  // 1. Pure garbage.
  CosemByteBuffer garbage;
  garbage.push_back(0xAAu);
  garbage.push_back(0xBBu);
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, garbage));
  EXPECT_EQ(before, object.Entries());

  // 2. Truncated array (claims 1 entry, payload empty).
  CosemByteBuffer truncated;
  truncated.push_back(0x01u);
  truncated.push_back(0x01u);
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, truncated));
  EXPECT_EQ(before, object.Entries());

  // 3. Wire payload with two entries that share `index` \u2013 codec
  // accepts the bytes per-entry, but the collection invariant must
  // reject and the schedule must stay unchanged.
  std::vector<ScheduleTableEntry> dupSrc;
  dupSrc.push_back(MakeEntry(7u));
  CosemByteBuffer singleWire = EncodeEntries(dupSrc);
  // singleWire = [0x01, 0x01, <entry bytes>] - array(1) header + entry.
  ASSERT_GT(singleWire.size(), 2u);
  CosemByteBuffer entryBytes(singleWire.begin() + 2, singleWire.end());
  CosemByteBuffer dupWire;
  dupWire.push_back(0x01u);     // array tag
  dupWire.push_back(0x02u);     // length = 2
  dupWire.insert(dupWire.end(), entryBytes.begin(), entryBytes.end());
  dupWire.insert(dupWire.end(), entryBytes.begin(), entryBytes.end());
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, dupWire));
  EXPECT_EQ(before, object.Entries());
}

TEST(CosemScheduleObject, WriteAttributeHonorsAccessMode)
{
  CosemScheduleObject readOnly(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadOnly);
  const auto before = readOnly.Entries();
  CosemByteBuffer empty;
  empty.push_back(0x01u);
  empty.push_back(0x00u);
  EXPECT_EQ(CosemStatus::AccessDenied, readOnly.WriteAttribute(2u, empty));
  EXPECT_EQ(before, readOnly.Entries());

  // logical_name is always read-only by spec.
  EXPECT_EQ(CosemStatus::AccessDenied, readOnly.WriteAttribute(1u, empty));
  // Unknown attribute id.
  EXPECT_EQ(CosemStatus::AttributeNotFound,
            readOnly.WriteAttribute(99u, empty));
}

TEST(CosemScheduleObject, EnableDisableAppliesRangesInSpecOrder)
{
  // Per \u00a74.5.3.3.1: disable range A first, then enable range B,
  // so when ranges overlap, B wins.
  std::vector<ScheduleTableEntry> seed;
  seed.push_back(MakeEntry(1u, true));
  seed.push_back(MakeEntry(2u, true));
  seed.push_back(MakeEntry(3u, true));
  seed.push_back(MakeEntry(4u, true));
  seed.push_back(MakeEntry(5u, true));
  CosemScheduleObject object(
    ScheduleLn(), seed, AttributeAccessMode::ReadAndWrite);

  // structure(4) { long-unsigned first_disable=2, last_disable=4,
  //                first_enable=3,  last_enable=3 }
  CosemByteBuffer in;
  in.push_back(0x02u); in.push_back(0x04u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x02u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x04u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x03u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x03u);

  CosemByteBuffer out = CosemByteBuffer(1u, 0xAAu);
  EXPECT_EQ(CosemStatus::Ok, object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());

  EXPECT_TRUE(object.Entries()[0].Enable());     // index 1 untouched
  EXPECT_FALSE(object.Entries()[1].Enable());    // index 2 disabled
  EXPECT_TRUE(object.Entries()[2].Enable());     // index 3 re-enabled
  EXPECT_FALSE(object.Entries()[3].Enable());    // index 4 disabled
  EXPECT_TRUE(object.Entries()[4].Enable());     // index 5 untouched
}

TEST(CosemScheduleObject, EnableDisableEmptyRangesAreNoOp)
{
  std::vector<ScheduleTableEntry> seed;
  seed.push_back(MakeEntry(1u, true));
  seed.push_back(MakeEntry(2u, false));
  CosemScheduleObject object(
    ScheduleLn(), seed, AttributeAccessMode::ReadAndWrite);

  // first > last on both ranges \u2192 no-op; entries unchanged.
  CosemByteBuffer in;
  in.push_back(0x02u); in.push_back(0x04u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x05u);  // 5
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x03u);  // 3
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x05u);  // 5
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x03u);  // 3
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(object.Entries()[0].Enable());
  EXPECT_FALSE(object.Entries()[1].Enable());
}

TEST(CosemScheduleObject, EnableDisableRejectsMalformedInput)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadAndWrite);
  CosemByteBuffer junk;
  junk.push_back(0x00u);
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.InvokeMethod(1u, junk, out));
  // Wrong structure arity (expects 4, gives 2).
  CosemByteBuffer wrongArity;
  wrongArity.push_back(0x02u); wrongArity.push_back(0x02u);
  wrongArity.push_back(0x12u); wrongArity.push_back(0x00u);
  wrongArity.push_back(0x01u);
  wrongArity.push_back(0x12u); wrongArity.push_back(0x00u);
  wrongArity.push_back(0x02u);
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.InvokeMethod(1u, wrongArity, out));
}

TEST(CosemScheduleObject, InsertAppendsNewIndexAndOverwritesExisting)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadAndWrite);
  const std::size_t initial = object.Entries().size();

  // Append new index 7.
  ScheduleTableEntry newEntry = MakeEntry(7u);
  CosemByteBuffer wire7 = EncodeSingleEntry(newEntry);
  // wire7 is array(1){struct(10){...}}; strip the array header for the
  // method's `entry` payload (\u00a74.5.3.3.2 takes a single
  // schedule_table_entry, not an array).
  CosemByteBuffer entryOnly(wire7.begin() + 2, wire7.end());
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.InvokeMethod(2u, entryOnly, out));
  EXPECT_TRUE(out.empty());
  EXPECT_EQ(initial + 1u, object.Entries().size());
  EXPECT_EQ(7u, object.Entries().back().Index());

  // Overwrite existing index 2 with a disabled variant.
  ScheduleTableEntry replaceEntry = MakeEntry(2u, false);
  CosemByteBuffer wire2 = EncodeSingleEntry(replaceEntry);
  CosemByteBuffer entryOnly2(wire2.begin() + 2, wire2.end());
  EXPECT_EQ(CosemStatus::Ok, object.InvokeMethod(2u, entryOnly2, out));
  EXPECT_EQ(initial + 1u, object.Entries().size());
  for (const auto& e : object.Entries()) {
    if (e.Index() == 2u) {
      EXPECT_FALSE(e.Enable());
    }
  }
}

TEST(CosemScheduleObject, InsertRejectsMalformedEntry)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadAndWrite);
  const auto before = object.Entries();
  CosemByteBuffer junk;
  junk.push_back(0xFFu);
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.InvokeMethod(2u, junk, out));
  EXPECT_EQ(before, object.Entries());
}

TEST(CosemScheduleObject, DeleteRemovesInclusiveRange)
{
  std::vector<ScheduleTableEntry> seed;
  for (std::uint16_t i = 1u; i <= 5u; ++i) seed.push_back(MakeEntry(i));
  CosemScheduleObject object(
    ScheduleLn(), seed, AttributeAccessMode::ReadAndWrite);

  // structure(2) { first=2, last=4 }
  CosemByteBuffer in;
  in.push_back(0x02u); in.push_back(0x02u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x02u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x04u);
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.InvokeMethod(3u, in, out));
  ASSERT_EQ(2u, object.Entries().size());
  EXPECT_EQ(1u, object.Entries()[0].Index());
  EXPECT_EQ(5u, object.Entries()[1].Index());
}

TEST(CosemScheduleObject, DeleteFirstAboveLastIsNoOp)
{
  std::vector<ScheduleTableEntry> seed;
  seed.push_back(MakeEntry(1u));
  seed.push_back(MakeEntry(2u));
  CosemScheduleObject object(
    ScheduleLn(), seed, AttributeAccessMode::ReadAndWrite);

  CosemByteBuffer in;
  in.push_back(0x02u); in.push_back(0x02u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x05u);
  in.push_back(0x12u); in.push_back(0x00u); in.push_back(0x03u);
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.InvokeMethod(3u, in, out));
  EXPECT_EQ(2u, object.Entries().size());
}

TEST(CosemScheduleObject, UnknownMethodReturnsMethodNotFound)
{
  CosemScheduleObject object(
    ScheduleLn(), SampleEntries(), AttributeAccessMode::ReadAndWrite);
  CosemByteBuffer in;
  in.push_back(0x00u);
  CosemByteBuffer out = CosemByteBuffer(1u, 0xAAu);
  for (std::uint8_t m : {0u, 4u, 99u}) {
    EXPECT_EQ(CosemStatus::MethodNotFound,
              object.InvokeMethod(static_cast<std::uint8_t>(m), in, out))
      << "method id " << static_cast<unsigned>(m);
    EXPECT_TRUE(out.empty());
  }
}
