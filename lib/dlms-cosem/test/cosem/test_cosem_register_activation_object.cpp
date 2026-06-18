// SPDX-License-Identifier: BSD-2-Clause
//
// Tests for `dlms::cosem::CosemRegisterActivationObject` (IC
// "Register Activation", class_id=6, version=0) per IEC 62056-6-2
// ED4 (2021) §4.3.5 / DLMS UA Blue Book Ed. 12.1 §4.3.5.
//
// One IC = one test file (see docs/production_readiness_roadmap.md P2.4).
#include "dlms/cosem/simple_objects.hpp"
#include "dlms/cosem/types/object_definition.hpp"
#include "dlms/cosem/types/register_mask.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

namespace dlms::cosem {
namespace {

CosemLogicalName MakeName()
{
  return CosemLogicalName(0u, 0u, 14u, 0u, 1u, 255u);
}

CosemByteBuffer Bytes(std::initializer_list<std::uint8_t> bytes)
{
  return CosemByteBuffer(bytes.begin(), bytes.end());
}

CosemByteBuffer EncodeLogicalNameAsOctetString(const CosemLogicalName& n)
{
  CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(0x06u);
  out.insert(out.end(), n.Data(), n.Data() + n.Size());
  return out;
}

// expected wire form of `register_assignment` array
CosemByteBuffer EncodeAssignment(
  const std::vector<types::ObjectDefinition>& items)
{
  CosemByteBuffer out;
  out.push_back(0x01u);  // array
  out.push_back(static_cast<std::uint8_t>(items.size()));
  for (const auto& it : items) {
    out.push_back(0x02u);  // structure
    out.push_back(0x02u);  // 2 fields
    out.push_back(0x12u);  // long-unsigned
    out.push_back(static_cast<std::uint8_t>((it.ClassId() >> 8) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>(it.ClassId() & 0xFFu));
    out.push_back(0x09u);  // octet-string
    out.push_back(0x06u);
    const auto& ln = it.LogicalName();
    out.insert(out.end(), ln.Data(), ln.Data() + ln.Size());
  }
  return out;
}

CosemByteBuffer EncodeMaskList(const std::vector<types::RegisterMask>& items)
{
  CosemByteBuffer out;
  out.push_back(0x01u);  // array
  out.push_back(static_cast<std::uint8_t>(items.size()));
  for (const auto& it : items) {
    out.push_back(0x02u);  // structure
    out.push_back(0x02u);  // 2 fields
    out.push_back(0x09u);  // octet-string mask_name
    out.push_back(static_cast<std::uint8_t>(it.MaskName().size()));
    out.insert(out.end(), it.MaskName().begin(), it.MaskName().end());
    out.push_back(0x01u);  // array of long-unsigned indices
    out.push_back(static_cast<std::uint8_t>(it.IndexList().size()));
    for (const auto idx : it.IndexList()) {
      out.push_back(0x12u);
      out.push_back(static_cast<std::uint8_t>((idx >> 8) & 0xFFu));
      out.push_back(static_cast<std::uint8_t>(idx & 0xFFu));
    }
  }
  return out;
}

CosemByteBuffer EncodeActiveMask(const CosemByteBuffer& name)
{
  CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(static_cast<std::uint8_t>(name.size()));
  out.insert(out.end(), name.begin(), name.end());
  return out;
}

TEST(CosemRegisterActivationObject, DefaultsToClassId6Version0)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer());
  EXPECT_EQ(obj.Descriptor().key.classId, 6u);
  EXPECT_EQ(obj.Descriptor().key.version, 0u);
  EXPECT_EQ(
    obj.Descriptor().key.version,
    CosemRegisterActivationObject::MaxSupportedVersion);
}

TEST(CosemRegisterActivationObject, NormalisesOverlongVersionToMaxSupported)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer(), 99u);
  EXPECT_EQ(
    obj.Descriptor().key.version,
    CosemRegisterActivationObject::MaxSupportedVersion);
}

TEST(CosemRegisterActivationObject, EncodesEmptyAssignmentAndMaskList)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer());
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(2u, out), CosemStatus::Ok);
  EXPECT_EQ(out, Bytes({0x01u, 0x00u}));  // empty array
  EXPECT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);
  EXPECT_EQ(out, Bytes({0x01u, 0x00u}));
  EXPECT_EQ(obj.ReadAttribute(4u, out), CosemStatus::Ok);
  EXPECT_EQ(out, Bytes({0x09u, 0x00u}));  // empty octet-string
}

TEST(CosemRegisterActivationObject, ReadsAllAttributesWithTypedWireForm)
{
  const std::vector<types::ObjectDefinition> assignment = {
    types::ObjectDefinition(3u, CosemLogicalName(1u, 0u, 1u, 8u, 0u, 255u)),
    types::ObjectDefinition(3u, CosemLogicalName(1u, 0u, 2u, 8u, 0u, 255u)),
  };
  const std::vector<types::RegisterMask> masks = {
    types::RegisterMask(Bytes({0x4Du, 0x41u, 0x49u, 0x4Eu, 0x00u}),
                        {1u, 2u}),
  };
  const CosemByteBuffer activeName =
    Bytes({0x4Du, 0x41u, 0x49u, 0x4Eu, 0x00u});

  CosemRegisterActivationObject obj(
    MakeName(), assignment, masks, activeName);

  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(1u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeLogicalNameAsOctetString(MakeName()));
  EXPECT_EQ(obj.ReadAttribute(2u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeAssignment(assignment));
  EXPECT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeMaskList(masks));
  EXPECT_EQ(obj.ReadAttribute(4u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeActiveMask(activeName));
}

TEST(CosemRegisterActivationObject, UnknownAttributeReportsAttributeNotFound)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer());
  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(5u, out), CosemStatus::AttributeNotFound);
  EXPECT_TRUE(out.empty());
  EXPECT_EQ(obj.ReadAttribute(99u, out), CosemStatus::AttributeNotFound);
}

TEST(CosemRegisterActivationObject, AllAttributesAreReadOnly)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer());
  const auto probe = Bytes({0x09u, 0x00u});
  for (std::uint8_t attr = 1u; attr <= 4u; ++attr) {
    EXPECT_EQ(obj.WriteAttribute(attr, probe), CosemStatus::AccessDenied);
    EXPECT_EQ(obj.AccessRights().AttributeAccess(attr),
              AttributeAccessMode::ReadOnly);
  }
  EXPECT_EQ(obj.WriteAttribute(99u, probe), CosemStatus::AttributeNotFound);
}

TEST(CosemRegisterActivationObject, AddRegisterAddMaskDeleteMaskUnsupported)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer());
  CosemByteBuffer in = Bytes({0x00u});
  for (std::uint8_t method = 1u; method <= 3u; ++method) {
    CosemByteBuffer out = Bytes({0xAAu, 0xBBu});
    EXPECT_EQ(obj.InvokeMethod(method, in, out),
              CosemStatus::UnsupportedFeature);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemRegisterActivationObject, UnknownMethodReportsMethodNotFound)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer());
  CosemByteBuffer in;
  CosemByteBuffer out = Bytes({0xCCu});
  EXPECT_EQ(obj.InvokeMethod(4u, in, out), CosemStatus::MethodNotFound);
  EXPECT_TRUE(out.empty());
}

TEST(CosemRegisterActivationObject, SettersReplaceTypedLists)
{
  CosemRegisterActivationObject obj(
    MakeName(), {}, {}, CosemByteBuffer());
  const std::vector<types::ObjectDefinition> assignment = {
    types::ObjectDefinition(8u, CosemLogicalName(0u, 0u, 1u, 0u, 0u, 255u)),
  };
  const std::vector<types::RegisterMask> masks = {
    types::RegisterMask(Bytes({0xAAu}), {1u}),
  };
  obj.SetRegisterAssignment(assignment);
  obj.SetMaskList(masks);
  obj.SetActiveMask(Bytes({0xBBu, 0xCCu}));

  EXPECT_EQ(obj.RegisterAssignment(), assignment);
  EXPECT_EQ(obj.MaskList(), masks);
  EXPECT_EQ(obj.ActiveMask(), Bytes({0xBBu, 0xCCu}));

  CosemByteBuffer out;
  EXPECT_EQ(obj.ReadAttribute(2u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeAssignment(assignment));
  EXPECT_EQ(obj.ReadAttribute(3u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeMaskList(masks));
  EXPECT_EQ(obj.ReadAttribute(4u, out), CosemStatus::Ok);
  EXPECT_EQ(out, EncodeActiveMask(Bytes({0xBBu, 0xCCu})));
}

}  // namespace
}  // namespace dlms::cosem
