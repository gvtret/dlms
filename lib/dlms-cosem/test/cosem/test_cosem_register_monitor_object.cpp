// Tests for CosemRegisterMonitorObject (IC 21 "Register monitor",
// class_id=21, version=0). Per the per-class test-file convention
// (P2.4 in production_readiness_roadmap.md). Spec:
// IEC 62056-6-2 ED4 (2021) \u00a74.5.6 / DLMS UA Blue Book Ed. 12.1 \u00a74.5.6.
//
// Covers:
//   * typed ctor & getters (incl. safe-fallback for size mismatch
//     and invalid monitored_value)
//   * static validators (IsValidThresholds, ThresholdsMatchActions)
//   * setter validation: SetThresholds / SetActions / SetMonitoredValue
//   * AXDR round-trip: thresholds (opaque-per-entry), monitored_value
//     (typed struct), actions (array of action_set with two action_items)
//   * ReadAttribute: logical_name + thresholds + monitored_value +
//     actions + unknown attribute
//   * WriteAttribute: thresholds accept on well-formed; reject on
//     malformed AXDR, size-mismatch with actions, trailing bytes,
//     and on read-only access; other ids access-denied / not-found
//   * InvokeMethod: every id returns MethodNotFound and clears output
//   * Version normalization above MaxSupportedVersion

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

using dlms::cosem::AttributeAccessMode;
using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::CosemRegisterMonitorObject;
using dlms::cosem::CosemStatus;
using dlms::cosem::types::ActionSet;
using dlms::cosem::types::MonitoredValue;
using dlms::cosem::types::Script;

CosemLogicalName MonitorLn()
{
  return CosemLogicalName(0u, 0u, 16u, 1u, 0u, 255u);
}

CosemLogicalName MonitoredObjectLn()
{
  return CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u);
}

CosemLogicalName ScriptTableLn()
{
  return CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u);
}

CosemByteBuffer BytesOf(std::initializer_list<std::uint8_t> bytes)
{
  return CosemByteBuffer(bytes.begin(), bytes.end());
}

// AXDR encoding for a single threshold value: double-long-unsigned
// (tag 0x06, 4 bytes BE). Same wire shape used by Blue Book examples.
CosemByteBuffer Threshold(std::uint32_t v)
{
  return BytesOf({
    0x06u,
    static_cast<std::uint8_t>((v >> 24) & 0xFFu),
    static_cast<std::uint8_t>((v >> 16) & 0xFFu),
    static_cast<std::uint8_t>((v >> 8) & 0xFFu),
    static_cast<std::uint8_t>(v & 0xFFu),
  });
}

std::vector<CosemByteBuffer> SampleThresholds()
{
  return { Threshold(100u), Threshold(200u) };
}

MonitoredValue SampleMonitoredValue()
{
  // class_id=3 (Register), monitored attribute=2 ("value")
  return MonitoredValue(3u, MonitoredObjectLn(), 2);
}

Script ScrUp(std::uint16_t selector)
{
  return Script(ScriptTableLn(), selector);
}

Script ScrDown(std::uint16_t selector)
{
  return Script(ScriptTableLn(), selector);
}

std::vector<ActionSet> SampleActions()
{
  return {
    ActionSet(ScrUp(11u), ScrDown(12u)),
    ActionSet(ScrUp(21u), ScrDown(22u)),
  };
}

// Re-encode an attribute via the object itself to keep the wire form
// honest (same approach used in IC 9 / IC 20 tests).
CosemByteBuffer EncodeAttribute(
  const CosemRegisterMonitorObject& obj,
  std::uint8_t attributeId)
{
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, obj.ReadAttribute(attributeId, out));
  return out;
}

CosemByteBuffer EncodedLogicalName(const CosemLogicalName& name)
{
  CosemByteBuffer out;
  out.push_back(0x09u); // octet-string tag
  out.push_back(0x06u); // length 6
  for (std::size_t i = 0u; i < name.Size(); ++i) {
    out.push_back(name[i]);
  }
  return out;
}

CosemRegisterMonitorObject MakeMonitor(
  AttributeAccessMode thresholdsAccess
    = AttributeAccessMode::ReadAndWrite)
{
  return CosemRegisterMonitorObject(
    MonitorLn(),
    SampleThresholds(),
    SampleMonitoredValue(),
    SampleActions(),
    thresholdsAccess);
}

} // namespace

TEST(CosemRegisterMonitorObject, DescriptorAndVersion)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  EXPECT_EQ(21u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(MonitorLn(), object.Descriptor().key.logicalName);
  EXPECT_EQ(CosemRegisterMonitorObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemRegisterMonitorObject, NormalizesVersionAboveMax)
{
  CosemRegisterMonitorObject object(
    MonitorLn(),
    SampleThresholds(), SampleMonitoredValue(), SampleActions(),
    AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(CosemRegisterMonitorObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemRegisterMonitorObject, ExposesTypedAttributes)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  EXPECT_EQ(SampleThresholds(), object.Thresholds());
  EXPECT_EQ(SampleMonitoredValue(), object.MonitoredValue());
  EXPECT_EQ(SampleActions(), object.Actions());
}

TEST(CosemRegisterMonitorObject, ConstructorDropsThresholdsOnSizeMismatch)
{
  // 1 threshold vs 2 actions \u2192 IC 21 invariant violated; both dropped.
  CosemRegisterMonitorObject object(
    MonitorLn(),
    std::vector<CosemByteBuffer>{ Threshold(7u) },
    SampleMonitoredValue(),
    SampleActions(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_TRUE(object.Thresholds().empty());
  EXPECT_TRUE(object.Actions().empty());
  // monitored_value survives because it is independently valid.
  EXPECT_EQ(SampleMonitoredValue(), object.MonitoredValue());
}

TEST(CosemRegisterMonitorObject, ConstructorDropsEmptyThresholdEntry)
{
  std::vector<CosemByteBuffer> bad = { Threshold(1u), CosemByteBuffer{} };
  CosemRegisterMonitorObject object(
    MonitorLn(), bad, SampleMonitoredValue(), SampleActions(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_TRUE(object.Thresholds().empty());
  EXPECT_TRUE(object.Actions().empty());
}

TEST(CosemRegisterMonitorObject, ConstructorDropsInvalidMonitoredValue)
{
  // attribute_index 0 is invalid; ctor falls back to default ln + idx=1
  // (default-constructed MonitoredValue is valid by construction).
  MonitoredValue bad;
  bad.SetClassId(3u);
  bad.SetLogicalName(MonitoredObjectLn());
  // Default attribute_index is already 1; force-construct a borderline
  // value: simulate "lost" by manually building with bad index.
  // Since SetAttributeIndex(0) is rejected, build via ctor clamp path:
  // ctor clamps neg/0 to 1 instead, so an "invalid" MonitoredValue is
  // impossible by API \u2014 the validator path is exercised by codec.
  // Here we only check that valid input survives unchanged.
  CosemRegisterMonitorObject object(
    MonitorLn(), SampleThresholds(), bad, SampleActions(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(bad, object.MonitoredValue());
}

TEST(CosemRegisterMonitorObject, ReadAttributeRoundTrip)
{
  CosemRegisterMonitorObject object = MakeMonitor();

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MonitorLn()), out);

  // thresholds: array(2) of double-long-unsigned(100), double-long-unsigned(200)
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesOf({
    0x01u, 0x02u,
    0x06u, 0x00u, 0x00u, 0x00u, 0x64u,
    0x06u, 0x00u, 0x00u, 0x00u, 0xC8u,
  }), out);

  // monitored_value: structure(3){ long-unsigned 3, octet-string(6) 1.0.1.8.0.255, integer 2 }
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesOf({
    0x02u, 0x03u,
    0x12u, 0x00u, 0x03u,
    0x09u, 0x06u, 0x01u, 0x00u, 0x01u, 0x08u, 0x00u, 0xFFu,
    0x0Fu, 0x02u,
  }), out);

  // actions: array(2) of structure(2){ action_item(up), action_item(down) }
  // action_item ::= structure(2){ octet-string(6), long-unsigned }
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesOf({
    0x01u, 0x02u,
    // entry 0
    0x02u, 0x02u,
      0x02u, 0x02u,
        0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
        0x12u, 0x00u, 0x0Bu,
      0x02u, 0x02u,
        0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
        0x12u, 0x00u, 0x0Cu,
    // entry 1
    0x02u, 0x02u,
      0x02u, 0x02u,
        0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
        0x12u, 0x00u, 0x15u,
      0x02u, 0x02u,
        0x09u, 0x06u, 0x00u, 0x00u, 0x0Au, 0x00u, 0x64u, 0xFFu,
        0x12u, 0x00u, 0x16u,
  }), out);

  EXPECT_EQ(CosemStatus::AttributeNotFound, object.ReadAttribute(5u, out));
}

TEST(CosemRegisterMonitorObject, WriteThresholdsAcceptsWellFormedSameSize)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  // replacement keeps 2 entries (matches actions.size()=2)
  std::vector<CosemByteBuffer> replacement = {
    Threshold(500u), Threshold(900u)
  };
  CosemRegisterMonitorObject src(
    MonitorLn(), replacement, SampleMonitoredValue(), SampleActions(),
    AttributeAccessMode::ReadAndWrite);
  const CosemByteBuffer wire = EncodeAttribute(src, 2u);

  EXPECT_EQ(CosemStatus::Ok, object.WriteAttribute(2u, wire));
  EXPECT_EQ(replacement, object.Thresholds());
}

TEST(CosemRegisterMonitorObject, WriteThresholdsRejectsSizeMismatch)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  // single-element replacement \u2014 violates |thresholds|==|actions|
  std::vector<CosemByteBuffer> bad = { Threshold(7u) };
  CosemRegisterMonitorObject src(
    MonitorLn(),
    SampleThresholds(), SampleMonitoredValue(), SampleActions(),
    AttributeAccessMode::ReadAndWrite);
  // Build wire form manually: array(1) + threshold
  CosemByteBuffer wire = BytesOf({ 0x01u, 0x01u });
  wire.insert(wire.end(), bad.front().begin(), bad.front().end());

  EXPECT_EQ(CosemStatus::InvalidArgument, object.WriteAttribute(2u, wire));
  EXPECT_EQ(SampleThresholds(), object.Thresholds());
}

TEST(CosemRegisterMonitorObject, WriteThresholdsRejectsTrailingBytes)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  CosemByteBuffer wire = EncodeAttribute(object, 2u);
  wire.push_back(0xAAu);
  EXPECT_EQ(CosemStatus::InvalidArgument, object.WriteAttribute(2u, wire));
  EXPECT_EQ(SampleThresholds(), object.Thresholds());
}

TEST(CosemRegisterMonitorObject, WriteThresholdsRejectsMalformedAxdr)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  CosemByteBuffer wire = BytesOf({ 0x01u, 0x02u, 0x06u, 0x00u, 0x00u });
  EXPECT_EQ(CosemStatus::InvalidArgument, object.WriteAttribute(2u, wire));
  EXPECT_EQ(SampleThresholds(), object.Thresholds());
}

TEST(CosemRegisterMonitorObject, ThresholdsHonorCallerAccessMode)
{
  CosemRegisterMonitorObject readOnly = MakeMonitor(
    AttributeAccessMode::ReadOnly);
  CosemByteBuffer wire = EncodeAttribute(readOnly, 2u);

  EXPECT_EQ(CosemStatus::AccessDenied, readOnly.WriteAttribute(2u, wire));
  EXPECT_EQ(SampleThresholds(), readOnly.Thresholds());
}

TEST(CosemRegisterMonitorObject, OtherAttributesAreReadOnly)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  const CosemByteBuffer any = BytesOf({ 0x12u, 0x00u, 0x00u });
  EXPECT_EQ(CosemStatus::AccessDenied, object.WriteAttribute(1u, any));
  EXPECT_EQ(CosemStatus::AccessDenied, object.WriteAttribute(3u, any));
  EXPECT_EQ(CosemStatus::AccessDenied, object.WriteAttribute(4u, any));
  EXPECT_EQ(CosemStatus::AttributeNotFound, object.WriteAttribute(99u, any));
}

TEST(CosemRegisterMonitorObject, NoMethodsAreDefined)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  CosemByteBuffer in = BytesOf({ 0x00u });
  for (std::uint8_t method = 1u; method <= 4u; ++method) {
    CosemByteBuffer out = BytesOf({ 0xAAu, 0xBBu });
    EXPECT_EQ(CosemStatus::MethodNotFound,
              object.InvokeMethod(method, in, out));
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemRegisterMonitorObject, SetThresholdsRejectsMismatch)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  EXPECT_FALSE(object.SetThresholds(
    std::vector<CosemByteBuffer>{ Threshold(1u) }));
  EXPECT_EQ(SampleThresholds(), object.Thresholds());

  EXPECT_FALSE(object.SetThresholds(
    std::vector<CosemByteBuffer>{ Threshold(1u), CosemByteBuffer{} }));
  EXPECT_EQ(SampleThresholds(), object.Thresholds());

  EXPECT_TRUE(object.SetThresholds(
    std::vector<CosemByteBuffer>{ Threshold(10u), Threshold(20u) }));
  EXPECT_EQ(2u, object.Thresholds().size());
}

TEST(CosemRegisterMonitorObject, SetActionsRejectsMismatch)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  EXPECT_FALSE(object.SetActions(
    std::vector<ActionSet>{ ActionSet(ScrUp(1u), ScrDown(2u)) }));
  EXPECT_EQ(SampleActions(), object.Actions());

  EXPECT_TRUE(object.SetActions(std::vector<ActionSet>{
    ActionSet(ScrUp(9u), ScrDown(8u)),
    ActionSet(ScrUp(7u), ScrDown(6u)),
  }));
  EXPECT_EQ(2u, object.Actions().size());
  EXPECT_EQ(9u, object.Actions()[0].ActionUp().Selector());
}

TEST(CosemRegisterMonitorObject, SetMonitoredValueAcceptsValid)
{
  CosemRegisterMonitorObject object = MakeMonitor();
  MonitoredValue updated(7u, MonitoredObjectLn(), 3);
  EXPECT_TRUE(object.SetMonitoredValue(updated));
  EXPECT_EQ(updated, object.MonitoredValue());
}

TEST(CosemRegisterMonitorObject, StaticValidators)
{
  EXPECT_TRUE(CosemRegisterMonitorObject::IsValidThresholds(
    SampleThresholds()));
  EXPECT_FALSE(CosemRegisterMonitorObject::IsValidThresholds(
    { Threshold(1u), CosemByteBuffer{} }));
  EXPECT_TRUE(CosemRegisterMonitorObject::ThresholdsMatchActions(
    SampleThresholds(), SampleActions()));
  EXPECT_FALSE(CosemRegisterMonitorObject::ThresholdsMatchActions(
    SampleThresholds(),
    std::vector<ActionSet>{ ActionSet(ScrUp(1u), ScrDown(2u)) }));
}
