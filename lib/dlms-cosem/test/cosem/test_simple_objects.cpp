#include "dlms/cosem/cosem.hpp"
#include "dlms/security/in_memory_invocation_counter_store.hpp"
#include "dlms/security/in_memory_key_store.hpp"
#include "dlms/security/suite0_key_wrap.hpp"

#include <gtest/gtest.h>

#include <array>
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

dlms::security::SecurityByteView SecurityView(
  const std::vector<std::uint8_t>& bytes)
{
  dlms::security::SecurityByteView view;
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

dlms::security::SecurityKey MakeSecurityKey(
  dlms::security::SecurityKeyRole role,
  const std::uint8_t* bytes,
  std::size_t size)
{
  dlms::security::SecurityKey key = dlms::security::EmptySecurityKey(role);
  key.size = size;
  for (std::size_t i = 0u; i < size; ++i) {
    key.bytes[i] = bytes[i];
  }
  return key;
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

void AppendDoubleLongUnsigned(
  dlms::cosem::CosemByteBuffer& bytes,
  std::uint32_t value)
{
  bytes.push_back(0x06u);
  bytes.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xffu));
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

dlms::cosem::CosemByteBuffer EncodedOctetString(
  const dlms::cosem::CosemSecuritySetupObject::SystemTitle& value)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x09u);
  bytes.push_back(static_cast<std::uint8_t>(value.size()));
  for (std::size_t i = 0; i < value.size(); ++i) {
    bytes.push_back(value[i]);
  }
  return bytes;
}

dlms::cosem::CosemByteBuffer EncodedGlobalKeyTransfer(
  std::uint8_t keyId,
  const std::vector<std::uint8_t>& wrapped)
{
  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(keyId);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());
  return input;
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

dlms::cosem::CosemMethodDescriptor MakeMethod(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t methodId)
{
  dlms::cosem::CosemMethodDescriptor descriptor;
  descriptor.object = key;
  descriptor.methodId = methodId;
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
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 43u, 0u, 0u, 255u),
            dlms::cosem::SecuritySetupName());
  EXPECT_EQ(dlms::cosem::CosemLogicalName(0u, 0u, 43u, 1u, 0u, 255u),
            dlms::cosem::InvocationCounterObjectName());
}

TEST(InvocationCounterObject, BuildsReadOnlyDataObject)
{
  dlms::cosem::CosemDataObject object =
    dlms::cosem::MakeInvocationCounterObject(0x01020304u);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(1u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(dlms::cosem::InvocationCounterObjectName(),
            descriptor.key.logicalName);

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::InvocationCounterObjectName()),
            output);

  dlms::cosem::CosemByteBuffer expected;
  AppendDoubleLongUnsigned(expected, 0x01020304u);
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(expected, output);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(2u));
}

TEST(CosemSecuritySetupObject, ExposesDescriptorAndSecurityAttributes)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x30u,
    0x01u,
    client,
    server);

  const dlms::cosem::CosemObjectDescriptor descriptor =
    object.Descriptor();
  EXPECT_EQ(64u, descriptor.key.classId);
  EXPECT_EQ(0u, descriptor.key.version);
  EXPECT_EQ(dlms::cosem::SecuritySetupName(), descriptor.key.logicalName);
  EXPECT_EQ(0x30u, object.SecurityPolicy());
  EXPECT_EQ(0x01u, object.SecuritySuite());
  EXPECT_EQ(client, object.ClientSystemTitle());
  EXPECT_EQ(server, object.ServerSystemTitle());

  dlms::cosem::CosemByteBuffer output;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(1u, output));
  EXPECT_EQ(EncodedLogicalName(dlms::cosem::SecuritySetupName()), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(Bytes(0x16u, 0x30u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(3u, output));
  EXPECT_EQ(Bytes(0x16u, 0x01u), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(4u, output));
  EXPECT_EQ(EncodedOctetString(client), output);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(5u, output));
  EXPECT_EQ(EncodedOctetString(server), output);
}

TEST(CosemSecuritySetupObject, ActivatesOnlyMonotonicSecurityPolicy)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x01u,
    0x00u,
    client,
    server);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(1u, Bytes(0x16u, 0x03u), output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(0x03u, object.SecurityPolicy());

  output.clear();
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(2u, output));
  EXPECT_EQ(Bytes(0x16u, 0x03u), output);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.InvokeMethod(1u, Bytes(0x16u, 0x01u), output));
  EXPECT_EQ(0x03u, object.SecurityPolicy());

  dlms::cosem::CosemByteBuffer invalid = Bytes(0x11u, 0x03u);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.InvokeMethod(1u, invalid, output));
  EXPECT_EQ(0x03u, object.SecurityPolicy());
}

TEST(CosemSecuritySetupObject, RejectsWritesAndReportsUnsupportedMethods)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x00u,
    0x00u,
    client,
    server);

  dlms::cosem::CosemByteBuffer bytes = Bytes(0x01u, 0x02u);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, bytes));
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, bytes, bytes));
  for (std::uint8_t methodId = 3u; methodId <= 8u; ++methodId) {
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(methodId, bytes, bytes));
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(99u, bytes, bytes));
}

TEST(CosemSecuritySetupObject, TransfersGlobalKeyThroughMutableKeyStore)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(0x02u);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
  EXPECT_EQ(dlms::security::SecurityKeyRole::Authentication, installed.role);
  ASSERT_EQ(sizeof(authenticationBytes), installed.size);
  for (std::size_t i = 0u; i < installed.size; ++i) {
    EXPECT_EQ(authenticationBytes[i], installed.bytes[i]);
  }
}

TEST(CosemSecuritySetupObject, RejectsTamperedGlobalKeyTransfer)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));
  wrapped[0] ^= 0x01u;

  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(0x02u);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, RejectsUnsupportedGlobalKeyTransferKeyId)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  const dlms::cosem::CosemByteBuffer input =
    EncodedGlobalKeyTransfer(0x7Fu, wrapped);

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, RejectsGlobalKeyTransferTrailingBytes)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  dlms::cosem::CosemByteBuffer input =
    EncodedGlobalKeyTransfer(0x02u, wrapped);
  input.push_back(0x00u);

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, RejectsGlobalKeyTransferForUnsupportedSuite)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  const dlms::cosem::CosemByteBuffer input =
    EncodedGlobalKeyTransfer(0x02u, wrapped);

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x01u,
    client,
    server,
    &keyStore);

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, input, output));
  EXPECT_TRUE(output.empty());

  dlms::security::SecurityKey installed =
    dlms::security::EmptySecurityKey(
      dlms::security::SecurityKeyRole::Authentication);
  EXPECT_EQ(dlms::security::SecurityStatus::MissingKey,
            keyStore.GetKey(
              dlms::security::SecurityKeyRole::Authentication,
              installed));
}

TEST(CosemSecuritySetupObject, KeyTransferResetsInvocationCounters)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t authenticationBytes[] = {
    0x10u, 0x11u, 0x12u, 0x13u,
    0x14u, 0x15u, 0x16u, 0x17u,
    0x18u, 0x19u, 0x1Au, 0x1Bu,
    0x1Cu, 0x1Du, 0x1Eu, 0x1Fu};
  dlms::security::InMemoryKeyStore keyStore;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyStore.SetKey(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes))));

  dlms::security::InMemoryInvocationCounterStore counters;
  const std::uint8_t title[8] =
    {0x53u, 0x54u, 0x31u, 0u, 0u, 0u, 0u, 1u};
  counters.SetLocalCounter(10u);
  ASSERT_EQ(
    dlms::security::SecurityStatus::Ok,
    counters.ValidateRemoteForSystemTitle(title, sizeof(title), 20u));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  const std::vector<std::uint8_t> plain(
    authenticationBytes,
    authenticationBytes + sizeof(authenticationBytes));
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(
              MakeSecurityKey(
                dlms::security::SecurityKeyRole::KeyEncryption,
                kekBytes,
                sizeof(kekBytes)),
              SecurityView(plain),
              wrapped));

  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x01u);
  input.push_back(0x01u);
  input.push_back(0x02u);
  input.push_back(0x02u);
  input.push_back(0x16u);
  input.push_back(0x02u);
  input.push_back(0x09u);
  input.push_back(static_cast<std::uint8_t>(wrapped.size()));
  input.insert(input.end(), wrapped.begin(), wrapped.end());

  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject object(
    dlms::cosem::SecuritySetupName(),
    0x03u,
    0x00u,
    client,
    server,
    &keyStore,
    &counters);

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(2u, input, output));

  std::uint32_t local = 0u;
  EXPECT_EQ(dlms::security::SecurityStatus::Ok, counters.NextLocal(local));
  EXPECT_EQ(1u, local);
  EXPECT_EQ(
    dlms::security::SecurityStatus::Ok,
    counters.ValidateRemoteForSystemTitle(title, sizeof(title), 20u));
}

TEST(CosemSecuritySetupObject, RegistryActivatesSecurityPolicy)
{
  dlms::cosem::CosemSecuritySetupObject::SystemTitle client = {
    {'C', 'L', 'I', 'E', 'N', 'T', '0', '1'}};
  dlms::cosem::CosemSecuritySetupObject::SystemTitle server = {
    {'S', 'E', 'R', 'V', 'E', 'R', '0', '1'}};
  std::shared_ptr<dlms::cosem::CosemSecuritySetupObject> object(
    new dlms::cosem::CosemSecuritySetupObject(
      dlms::cosem::SecuritySetupName(),
      0x00u,
      0x00u,
      client,
      server));
  dlms::cosem::ObjectRegistry registry;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer output = Bytes(0xAAu, 0xBBu);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.InvokeMethod(
              MakeMethod(object->Descriptor().key, 1u),
              Bytes(0x16u, 0x03u),
              output));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(0x03u, object->SecurityPolicy());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            registry.InvokeMethod(
              MakeMethod(object->Descriptor().key, 1u),
              Bytes(0x16u, 0x01u),
              output));
  EXPECT_EQ(0x03u, object->SecurityPolicy());

  dlms::cosem::CosemByteBuffer bytes;
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            registry.WriteAttribute(
              MakeAttribute(object->Descriptor().key, 2u),
              bytes));
}
