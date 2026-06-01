#include "dlms/cosem/cosem.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace {

class DeviceTestObject : public dlms::cosem::ICosemObject
{
public:
  DeviceTestObject(
    const dlms::cosem::CosemObjectKey& key,
    dlms::cosem::AttributeAccessMode attributeMode)
  {
    descriptor_.key = key;
    rights_.SetAttributeAccess(2u, attributeMode);
  }

  dlms::cosem::CosemObjectDescriptor Descriptor() const
  {
    return descriptor_;
  }

  dlms::cosem::CosemAccessRights AccessRights() const
  {
    return rights_;
  }

  dlms::cosem::CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    dlms::cosem::CosemByteBuffer& output) const
  {
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }
    output.push_back(0x42u);
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus WriteAttribute(
    std::uint8_t,
    const dlms::cosem::CosemByteBuffer&)
  {
    return dlms::cosem::CosemStatus::UnsupportedFeature;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t,
    const dlms::cosem::CosemByteBuffer&,
    dlms::cosem::CosemByteBuffer&)
  {
    return dlms::cosem::CosemStatus::UnsupportedFeature;
  }

private:
  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
};

dlms::cosem::CosemObjectKey MakeKey(
  std::uint16_t classId,
  std::uint8_t logicalNamePart)
{
  dlms::cosem::CosemObjectKey key;
  key.classId = classId;
  key.version = 0u;
  key.logicalName =
    dlms::cosem::CosemLogicalName(0, 0, logicalNamePart, 0, 0, 255);
  return key;
}

dlms::cosem::CosemAttributeDescriptor MakeAttribute(
  const dlms::cosem::CosemObjectKey& key)
{
  dlms::cosem::CosemAttributeDescriptor descriptor;
  descriptor.object = key;
  descriptor.attributeId = 2u;
  return descriptor;
}

std::shared_ptr<dlms::cosem::ICosemObject> MakeObject(
  const dlms::cosem::CosemObjectKey& key,
  dlms::cosem::AttributeAccessMode attributeMode)
{
  return std::shared_ptr<dlms::cosem::ICosemObject>(
    new DeviceTestObject(key, attributeMode));
}

TEST(LogicalDevice, StoresSapNameAndDelegatesRegistry)
{
  dlms::cosem::LogicalDevice device(16u, "management");
  const dlms::cosem::CosemObjectKey key = MakeKey(1u, 1u);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            device.RegisterObject(
              MakeObject(key, dlms::cosem::AttributeAccessMode::ReadOnly)));

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            device.ReadAttribute(MakeAttribute(key), output));
  ASSERT_EQ(1u, output.size());
  EXPECT_EQ(0x42u, output[0]);
  EXPECT_EQ(16u, device.Sap());
  EXPECT_EQ("management", device.Name());
  EXPECT_EQ(1u, device.Registry().Size());
}

TEST(LogicalDevice, BuildsAssociationViewWithDescriptorsAndRights)
{
  dlms::cosem::LogicalDevice device(1u, "logical-device");
  const dlms::cosem::CosemObjectKey first = MakeKey(1u, 1u);
  const dlms::cosem::CosemObjectKey second = MakeKey(7u, 2u);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            device.RegisterObject(MakeObject(
              second, dlms::cosem::AttributeAccessMode::NoAccess)));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            device.RegisterObject(MakeObject(
              first, dlms::cosem::AttributeAccessMode::ReadOnly)));

  const dlms::cosem::AssociationView view =
    device.BuildAssociationView();

  ASSERT_EQ(2u, view.objects.size());
  EXPECT_EQ(first, view.objects[0].descriptor.key);
  EXPECT_EQ(second, view.objects[1].descriptor.key);
  EXPECT_TRUE(view.objects[0].accessRights.CanReadAttribute(
    2u, dlms::cosem::PublicAccessContext()));
  EXPECT_FALSE(view.objects[1].accessRights.CanReadAttribute(
    2u, dlms::cosem::PublicAccessContext()));
}

TEST(LogicalDevice, BuildsSapAndAssociationLnMetadata)
{
  dlms::cosem::LogicalDevice device(17u, "ld-17");
  const dlms::cosem::CosemObjectKey associationKey = MakeKey(15u, 40u);
  dlms::cosem::CosemObjectDescriptor associationObject;
  associationObject.key = associationKey;

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            device.RegisterObject(MakeObject(
              associationKey, dlms::cosem::AttributeAccessMode::ReadOnly)));

  const dlms::cosem::SapAssignment sap = device.BuildSapAssignment();
  const dlms::cosem::AssociationLnMetadata metadata =
    device.BuildAssociationLnMetadata(associationObject);

  EXPECT_EQ(17u, sap.sap);
  EXPECT_EQ("ld-17", sap.logicalDeviceName);
  EXPECT_EQ(associationKey, metadata.descriptor.key);
  ASSERT_EQ(1u, metadata.objectList.objects.size());
  EXPECT_EQ(associationKey, metadata.objectList.objects[0].descriptor.key);
}

TEST(PhysicalDevice, StoresAndFindsLogicalDevices)
{
  dlms::cosem::PhysicalDevice physicalDevice;
  const std::shared_ptr<dlms::cosem::LogicalDevice> first(
    new dlms::cosem::LogicalDevice(1u, "ld-1"));
  const std::shared_ptr<dlms::cosem::LogicalDevice> duplicate(
    new dlms::cosem::LogicalDevice(1u, "duplicate"));
  const std::shared_ptr<dlms::cosem::LogicalDevice> invalid(
    new dlms::cosem::LogicalDevice(0u, "invalid"));

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            physicalDevice.AddLogicalDevice(
              std::shared_ptr<dlms::cosem::LogicalDevice>()));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            physicalDevice.AddLogicalDevice(invalid));
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            physicalDevice.AddLogicalDevice(first));
  EXPECT_EQ(dlms::cosem::CosemStatus::DuplicateObject,
            physicalDevice.AddLogicalDevice(duplicate));

  EXPECT_EQ(first.get(), physicalDevice.FindLogicalDevice(1u));
  EXPECT_EQ(0, physicalDevice.FindLogicalDevice(2u));
  EXPECT_EQ(1u, physicalDevice.Size());
}

TEST(PhysicalDevice, EnumeratesSapAssignments)
{
  dlms::cosem::PhysicalDevice physicalDevice;

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            physicalDevice.AddLogicalDevice(
              std::shared_ptr<dlms::cosem::LogicalDevice>(
                new dlms::cosem::LogicalDevice(1u, "ld-1"))));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            physicalDevice.AddLogicalDevice(
              std::shared_ptr<dlms::cosem::LogicalDevice>(
                new dlms::cosem::LogicalDevice(16u, "management"))));

  const std::vector<dlms::cosem::SapAssignment> assignments =
    physicalDevice.SapAssignments();

  ASSERT_EQ(2u, assignments.size());
  EXPECT_EQ(1u, assignments[0].sap);
  EXPECT_EQ("ld-1", assignments[0].logicalDeviceName);
  EXPECT_EQ(16u, assignments[1].sap);
  EXPECT_EQ("management", assignments[1].logicalDeviceName);
}

} // namespace
