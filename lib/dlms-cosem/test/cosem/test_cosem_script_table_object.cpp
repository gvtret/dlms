// Tests for CosemScriptTableObject (IC 9 "Script table"). Per the
// roadmap rule "one test file per IC class". Covers:
//   * typed ctor & getter (incl. safe-fallback on malformed input)
//   * SetScripts validation (per-entry + unique identifier)
//   * AXDR codec round-trip (full attribute + null/non-null parameter)
//   * ReadAttribute: logical_name + scripts + unknown attribute
//   * WriteAttribute: well-formed accept, malformed reject, access
//     mode gate, duplicate-identifier reject, trailing-bytes reject
//   * InvokeMethod: execute(1) surfaces UnsupportedFeature; other
//     method ids return MethodNotFound; both clear output
//   * Version normalization

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

using dlms::cosem::AttributeAccessMode;
using dlms::cosem::CosemByteBuffer;
using dlms::cosem::CosemLogicalName;
using dlms::cosem::CosemScriptTableObject;
using dlms::cosem::CosemStatus;
using dlms::cosem::types::ActionSpecification;
using dlms::cosem::types::ScriptEntry;
using dlms::cosem::types::ScriptServiceId;

CosemLogicalName ScriptTableLn()
{
  return CosemLogicalName(0u, 0u, 10u, 0u, 100u, 255u);
}

CosemLogicalName ClockLn()
{
  return CosemLogicalName(0u, 0u, 1u, 0u, 0u, 255u);
}

CosemByteBuffer BytesOf(std::initializer_list<std::uint8_t> bytes)
{
  return CosemByteBuffer(bytes.begin(), bytes.end());
}

// helper: action_specification(write_attribute, class=8, clock, attr=2,
// parameter=long-unsigned 0x1234)
ActionSpecification SampleWriteAction()
{
  return ActionSpecification(
    ScriptServiceId::WriteAttribute,
    8u,
    ClockLn(),
    2,
    BytesOf({0x12u, 0x12u, 0x34u}));
}

// helper: action_specification(execute_method, class=20, name=ScriptTableLn,
// method=1, parameter empty)
ActionSpecification SampleExecuteAction()
{
  return ActionSpecification(
    ScriptServiceId::ExecuteMethod,
    20u,
    ScriptTableLn(),
    1,
    CosemByteBuffer{});
}

ScriptEntry SampleEntry1()
{
  return ScriptEntry(
    1u,
    std::vector<ActionSpecification>{ SampleWriteAction() });
}

ScriptEntry SampleEntry2()
{
  return ScriptEntry(
    2u,
    std::vector<ActionSpecification>{
      SampleWriteAction(), SampleExecuteAction()
    });
}

std::vector<ScriptEntry> SampleScripts()
{
  return { SampleEntry1(), SampleEntry2() };
}

// Encode the same `scripts` payload the object writes on the wire,
// reusing the object's own encoder to keep the tests honest.
CosemByteBuffer EncodeScripts(const std::vector<ScriptEntry>& scripts)
{
  CosemScriptTableObject tmp(
    ScriptTableLn(), scripts, AttributeAccessMode::ReadAndWrite);
  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, tmp.ReadAttribute(2u, out));
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

} // namespace

TEST(CosemScriptTableObject, DescriptorAndVersion)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(9u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(CosemScriptTableObject::MaxSupportedVersion,
            object.Descriptor().key.version);
  EXPECT_EQ(ScriptTableLn(), object.Descriptor().key.logicalName);
}

TEST(CosemScriptTableObject, NormalizesVersionAboveMax)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(CosemScriptTableObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}

TEST(CosemScriptTableObject, ConstructorRejectsDuplicateIdentifierFallback)
{
  // Two scripts with identifier=1 must collapse to an empty scripts
  // collection via the safe-fallback ctor (no mutation of caller).
  std::vector<ScriptEntry> bad = { SampleEntry1(), SampleEntry1() };
  CosemScriptTableObject object(
    ScriptTableLn(), bad, AttributeAccessMode::ReadAndWrite);
  EXPECT_TRUE(object.Scripts().empty());
}

TEST(CosemScriptTableObject, SetScriptsRejectsDuplicateIdentifier)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);
  const std::vector<ScriptEntry> original = object.Scripts();
  std::vector<ScriptEntry> bad = { SampleEntry1(), SampleEntry1() };
  EXPECT_FALSE(object.SetScripts(bad));
  EXPECT_EQ(original, object.Scripts());
}

TEST(CosemScriptTableObject, SetScriptsAcceptsValidReplacement)
{
  CosemScriptTableObject object(
    ScriptTableLn(), {}, AttributeAccessMode::ReadAndWrite);
  EXPECT_TRUE(object.Scripts().empty());
  EXPECT_TRUE(object.SetScripts(SampleScripts()));
  EXPECT_EQ(SampleScripts(), object.Scripts());
}

TEST(CosemScriptTableObject, IsValidScriptsStatic)
{
  EXPECT_TRUE(CosemScriptTableObject::IsValidScripts({}));
  EXPECT_TRUE(CosemScriptTableObject::IsValidScripts(SampleScripts()));
  EXPECT_FALSE(CosemScriptTableObject::IsValidScripts(
    { SampleEntry1(), SampleEntry1() }));
}

TEST(CosemScriptTableObject, ReadAttributeLogicalNameAndScripts)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(ScriptTableLn()), out);

  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_FALSE(out.empty());
  EXPECT_EQ(0x01u, out[0]); // array tag
  EXPECT_EQ(2u, out[1]);    // 2 scripts

  out = BytesOf({0xAAu});
  EXPECT_EQ(CosemStatus::AttributeNotFound, object.ReadAttribute(3u, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemScriptTableObject, ReadAttributeEmptyScriptsEncodesEmptyArray)
{
  CosemScriptTableObject object(
    ScriptTableLn(), {}, AttributeAccessMode::ReadAndWrite);

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesOf({0x01u, 0x00u}), out);
}

TEST(CosemScriptTableObject, WriteAttributeAcceptsRoundTrip)
{
  CosemScriptTableObject object(
    ScriptTableLn(), {}, AttributeAccessMode::ReadAndWrite);
  const CosemByteBuffer wire = EncodeScripts(SampleScripts());

  EXPECT_EQ(CosemStatus::Ok, object.WriteAttribute(2u, wire));
  EXPECT_EQ(SampleScripts(), object.Scripts());

  CosemByteBuffer out;
  EXPECT_EQ(CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(wire, out);
}

TEST(CosemScriptTableObject, WriteAttributeRoundTripsNullParameter)
{
  // SampleExecuteAction has an empty parameter buffer; encoder must
  // emit null-data, decoder must round-trip back to empty.
  const std::vector<ScriptEntry> scripts = {
    ScriptEntry(7u, std::vector<ActionSpecification>{ SampleExecuteAction() })
  };
  CosemScriptTableObject object(
    ScriptTableLn(), {}, AttributeAccessMode::ReadAndWrite);
  const CosemByteBuffer wire = EncodeScripts(scripts);

  EXPECT_EQ(CosemStatus::Ok, object.WriteAttribute(2u, wire));
  ASSERT_EQ(1u, object.Scripts().size());
  ASSERT_EQ(1u, object.Scripts()[0].Actions().size());
  EXPECT_TRUE(object.Scripts()[0].Actions()[0].Parameter().empty());
}

TEST(CosemScriptTableObject, WriteAttributeRejectsMalformed)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);
  const std::vector<ScriptEntry> snapshot = object.Scripts();

  // Not an array.
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesOf({0x02u, 0x00u})));
  // Truncated array length.
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesOf({0x01u})));
  // Trailing garbage after a valid encoding.
  CosemByteBuffer trailing = EncodeScripts(SampleScripts());
  trailing.push_back(0xFFu);
  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, trailing));

  // No mutation on any rejection.
  EXPECT_EQ(snapshot, object.Scripts());
}

TEST(CosemScriptTableObject, WriteAttributeRejectsDuplicateIdentifier)
{
  CosemScriptTableObject object(
    ScriptTableLn(), {}, AttributeAccessMode::ReadAndWrite);

  // Hand-craft array(2) of two identical scripts (id=1) by slicing the
  // wire of a single-entry array — the ctor would have filtered this,
  // so we build it from the encoded form of a 1-entry script.
  const CosemByteBuffer single = EncodeScripts({ SampleEntry1() });
  ASSERT_GE(single.size(), 2u);
  ASSERT_EQ(0x01u, single[0]);
  ASSERT_EQ(0x01u, single[1]);
  CosemByteBuffer wire;
  wire.push_back(0x01u); // array tag
  wire.push_back(0x02u); // 2 entries
  wire.insert(wire.end(), single.begin() + 2, single.end());
  wire.insert(wire.end(), single.begin() + 2, single.end());

  EXPECT_EQ(CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, wire));
  EXPECT_TRUE(object.Scripts().empty());
}

TEST(CosemScriptTableObject, WriteAttributeAccessModeGate)
{
  CosemScriptTableObject readOnly(
    ScriptTableLn(), SampleScripts(), AttributeAccessMode::ReadOnly);
  const CosemByteBuffer wire = EncodeScripts({ SampleEntry1() });

  EXPECT_EQ(CosemStatus::AccessDenied,
            readOnly.WriteAttribute(2u, wire));
  EXPECT_EQ(SampleScripts(), readOnly.Scripts());

  // Writing the logical_name attribute is always denied.
  CosemScriptTableObject writable(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(CosemStatus::AccessDenied,
            writable.WriteAttribute(1u, wire));
  EXPECT_EQ(CosemStatus::AttributeNotFound,
            writable.WriteAttribute(99u, wire));
}

TEST(CosemScriptTableObject, InvokeMethodExecuteIsUnsupportedAndClearsOutput)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);

  CosemByteBuffer in = BytesOf({0x12u, 0x00u, 0x01u});
  CosemByteBuffer out = BytesOf({0xAAu, 0xBBu});
  EXPECT_EQ(CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemScriptTableObject, InvokeUnknownMethodNotFound)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);
  CosemByteBuffer in;
  CosemByteBuffer out = BytesOf({0xCCu});
  EXPECT_EQ(CosemStatus::MethodNotFound,
            object.InvokeMethod(2u, in, out));
  EXPECT_TRUE(out.empty());
}

TEST(CosemScriptTableObject, AccessRightsAdvertiseLogicalNameReadOnly)
{
  CosemScriptTableObject object(
    ScriptTableLn(), SampleScripts(),
    AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(AttributeAccessMode::ReadOnly,
            object.AccessRights().AttributeAccess(1u));
  EXPECT_EQ(AttributeAccessMode::ReadAndWrite,
            object.AccessRights().AttributeAccess(2u));
}
