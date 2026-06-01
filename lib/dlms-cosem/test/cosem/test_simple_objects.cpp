#include "dlms/cosem/cosem.hpp"

#include <gtest/gtest.h>

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

dlms::cosem::CosemAttributeDescriptor MakeAttribute(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t attributeId)
{
  dlms::cosem::CosemAttributeDescriptor descriptor;
  descriptor.object = key;
  descriptor.attributeId = attributeId;
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

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, output));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(1u, updated, output));
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

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, output));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(1u, updated, output));
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

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            association.ReadAttribute(99u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            association.WriteAttribute(2u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            association.InvokeMethod(1u, bytes, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            sap.ReadAttribute(99u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            sap.WriteAttribute(2u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            sap.InvokeMethod(1u, bytes, bytes));
}

TEST(DiscoveryObjects, DefaultLogicalNamesUseStandardObisValues)
{
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 40u, 0u, 0u, 255u),
            dlms::cosem::CurrentAssociationLnName());
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 41u, 0u, 0u, 255u),
            dlms::cosem::SapAssignmentName());
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 42u, 0u, 0u, 255u),
            dlms::cosem::LogicalDeviceNameObjectName());
}
