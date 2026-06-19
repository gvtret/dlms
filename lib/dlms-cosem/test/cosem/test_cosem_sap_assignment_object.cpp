// SPDX-License-Identifier: Apache-2.0
//
// Per-IC tests for CosemSapAssignmentObject (class_id=17, version=0)
// per IEC 62056-6-2 ED4 (2021) §4.4.5 and DLMS UA Blue Book Ed. 12.1
// §4.4.5 (SAP assignment). The typed surface exposes:
//   - sap_assignment_list (attr 2):
//       std::vector<SapAssignment> where each entry is
//       { sap: long-unsigned, logical_device_name: octet-string }
//
// Method 1 "connect_logical_device" is recognised but reported as
// UnsupportedFeature because the built-in object does not own the
// SAP-mutation dispatch policy; every other id is MethodNotFound.

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include "dlms/cosem/cosem_status.hpp"
#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/logical_device.hpp"
#include "dlms/cosem/simple_objects.hpp"

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

dlms::cosem::SapAssignment MakeAssignment(
  std::uint16_t sap,
  const std::string& name)
{
  dlms::cosem::SapAssignment a;
  a.sap = sap;
  a.logicalDeviceName = name;
  return a;
}

std::vector<dlms::cosem::SapAssignment> SampleAssignments()
{
  std::vector<dlms::cosem::SapAssignment> v;
  v.push_back(MakeAssignment(1u, "ld-1"));
  v.push_back(MakeAssignment(16u, "public"));
  return v;
}

dlms::cosem::CosemByteBuffer EncodeSampleAssignments()
{
  // array(2) { struct(2){ long-unsigned, octet-string } * 2 }
  return BytesFromList({
    0x01u, 0x02u,
      0x02u, 0x02u,
        0x12u, 0x00u, 0x01u,
        0x09u, 0x04u, 'l', 'd', '-', '1',
      0x02u, 0x02u,
        0x12u, 0x00u, 0x10u,
        0x09u, 0x06u, 'p', 'u', 'b', 'l', 'i', 'c'});
}

} // namespace

TEST(CosemSapAssignmentObject, DescriptorAndDefaultAccessRights)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(), SampleAssignments());
  EXPECT_EQ(17u, obj.Descriptor().key.classId);
  EXPECT_EQ(0u, obj.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemSapAssignmentObject::MaxSupportedVersion,
    obj.Descriptor().key.version);
  EXPECT_EQ(dlms::cosem::SapAssignmentName(),
            obj.Descriptor().key.logicalName);

  // Default ctor keeps the list read-only per spec §4.4.5.4.
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            obj.AccessRights().AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            obj.AccessRights().AttributeAccess(2u));
}

TEST(CosemSapAssignmentObject, TypedGetterReflectsCtor)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(), SampleAssignments());
  const std::vector<dlms::cosem::SapAssignment> got = obj.Assignments();
  ASSERT_EQ(2u, got.size());
  EXPECT_EQ(1u, got[0].sap);
  EXPECT_EQ("ld-1", got[0].logicalDeviceName);
  EXPECT_EQ(16u, got[1].sap);
  EXPECT_EQ("public", got[1].logicalDeviceName);
}

TEST(CosemSapAssignmentObject, ReadAttributeEmitsAxdr)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(), SampleAssignments());
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::SapAssignmentName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(2u, out));
  EXPECT_EQ(EncodeSampleAssignments(), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.ReadAttribute(99u, out));
}

TEST(CosemSapAssignmentObject, EmptyListEncodesAsEmptyArray)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(),
    std::vector<dlms::cosem::SapAssignment>());
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x01u, 0x00u}), out);
}

TEST(CosemSapAssignmentObject, WriteAttributeDeniedByDefault)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(), SampleAssignments());
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(2u, EncodeSampleAssignments()));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            obj.WriteAttribute(1u, BytesFromList({0x00u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            obj.WriteAttribute(99u, BytesFromList({0x00u})));
}

TEST(CosemSapAssignmentObject, WriteAttributeRoundTripsWhenWritable)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(),
    std::vector<dlms::cosem::SapAssignment>(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            obj.AccessRights().AttributeAccess(2u));

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(2u, EncodeSampleAssignments()));

  const std::vector<dlms::cosem::SapAssignment> got = obj.Assignments();
  ASSERT_EQ(2u, got.size());
  EXPECT_EQ(1u, got[0].sap);
  EXPECT_EQ("ld-1", got[0].logicalDeviceName);
  EXPECT_EQ(16u, got[1].sap);
  EXPECT_EQ("public", got[1].logicalDeviceName);

  // Re-read must emit the same A-XDR.
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, obj.ReadAttribute(2u, out));
  EXPECT_EQ(EncodeSampleAssignments(), out);

  // Empty array clears the table.
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            obj.WriteAttribute(2u, BytesFromList({0x01u, 0x00u})));
  EXPECT_TRUE(obj.Assignments().empty());
}

TEST(CosemSapAssignmentObject, WriteAttributeRejectsMalformedPayload)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(), SampleAssignments(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // Wrong outer tag (structure where array expected).
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x02u, 0x00u})));
  // Empty input.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, dlms::cosem::CosemByteBuffer()));
  // Truncated: declared 1 entry, none provided.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({0x01u, 0x01u})));
  // Entry struct has 3 fields instead of 2.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x03u,
              0x12u, 0x00u, 0x01u,
              0x09u, 0x01u, 'x',
              0x11u, 0x00u})));
  // SAP encoded as unsigned (wrong tag) instead of long-unsigned.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x02u,
              0x11u, 0x01u,
              0x09u, 0x01u, 'x'})));
  // LDN length larger than payload.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x02u,
              0x12u, 0x00u, 0x01u,
              0x09u, 0x05u, 'x'})));
  // Trailing garbage after a valid entry.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            obj.WriteAttribute(2u, BytesFromList({
              0x01u, 0x01u,
              0x02u, 0x02u,
              0x12u, 0x00u, 0x01u,
              0x09u, 0x01u, 'x',
              0xFFu})));

  // Original list preserved on every failed write.
  const std::vector<dlms::cosem::SapAssignment> got = obj.Assignments();
  ASSERT_EQ(2u, got.size());
  EXPECT_EQ(1u, got[0].sap);
  EXPECT_EQ("ld-1", got[0].logicalDeviceName);
}

TEST(CosemSapAssignmentObject, SetAssignmentsBypassesAccessChecks)
{
  // Backend setter must mutate even when the protocol surface is
  // read-only (cold provisioning / offline configuration).
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(),
    std::vector<dlms::cosem::SapAssignment>());
  EXPECT_TRUE(obj.Assignments().empty());
  obj.SetAssignments(SampleAssignments());
  ASSERT_EQ(2u, obj.Assignments().size());
  EXPECT_EQ(16u, obj.Assignments()[1].sap);
}

TEST(CosemSapAssignmentObject, ConnectMethodIsUnsupportedFeature)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(), SampleAssignments());
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            obj.InvokeMethod(1u, in, out));
  EXPECT_TRUE(out.empty());

  for (std::uint8_t method : {2u, 3u, 99u}) {
    out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              obj.InvokeMethod(method, in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemSapAssignmentObject, NormalizesVersionAboveMax)
{
  dlms::cosem::CosemSapAssignmentObject obj(
    dlms::cosem::SapAssignmentName(),
    SampleAssignments(),
    static_cast<std::uint8_t>(99u));
  EXPECT_EQ(
    dlms::cosem::CosemSapAssignmentObject::MaxSupportedVersion,
    obj.Descriptor().key.version);
}
