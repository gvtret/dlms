#include "dlms/cosem/logical_device.hpp"

namespace dlms {
namespace cosem {

LogicalDevice::LogicalDevice(
  std::uint16_t sap,
  const std::string& name)
  : sap_(sap)
  , name_(name)
{
}

std::uint16_t LogicalDevice::Sap() const
{
  return sap_;
}

const std::string& LogicalDevice::Name() const
{
  return name_;
}

ObjectRegistry& LogicalDevice::Registry()
{
  return registry_;
}

const ObjectRegistry& LogicalDevice::Registry() const
{
  return registry_;
}

CosemStatus LogicalDevice::RegisterObject(
  const std::shared_ptr<ICosemObject>& object)
{
  return registry_.Register(object);
}

AssociationView LogicalDevice::BuildAssociationView() const
{
  AssociationView view;
  const std::vector<CosemObjectDescriptor> descriptors =
    registry_.Descriptors();
  for (std::vector<CosemObjectDescriptor>::const_iterator it =
         descriptors.begin();
       it != descriptors.end();
       ++it) {
    const ICosemObject* object = registry_.Find(it->key);
    if (object) {
      AssociationViewObject entry;
      entry.descriptor = *it;
      entry.accessRights = object->AccessRights();
      view.objects.push_back(entry);
    }
  }
  return view;
}

SapAssignment LogicalDevice::BuildSapAssignment() const
{
  SapAssignment assignment;
  assignment.sap = sap_;
  assignment.logicalDeviceName = name_;
  return assignment;
}

AssociationLnMetadata LogicalDevice::BuildAssociationLnMetadata(
  const CosemObjectDescriptor& associationObject) const
{
  AssociationLnMetadata metadata;
  metadata.descriptor = associationObject;
  metadata.objectList = BuildAssociationView();
  return metadata;
}

CosemStatus LogicalDevice::ReadAttribute(
  const CosemAttributeDescriptor& descriptor,
  CosemByteBuffer& output) const
{
  return registry_.ReadAttribute(descriptor, output);
}

CosemStatus LogicalDevice::ReadAttribute(
  const CosemAttributeDescriptor& descriptor,
  const CosemAccessContext& context,
  CosemByteBuffer& output) const
{
  return registry_.ReadAttribute(descriptor, context, output);
}

CosemStatus LogicalDevice::WriteAttribute(
  const CosemAttributeDescriptor& descriptor,
  const CosemByteBuffer& input)
{
  return registry_.WriteAttribute(descriptor, input);
}

CosemStatus LogicalDevice::WriteAttribute(
  const CosemAttributeDescriptor& descriptor,
  const CosemAccessContext& context,
  const CosemByteBuffer& input)
{
  return registry_.WriteAttribute(descriptor, context, input);
}

CosemStatus LogicalDevice::InvokeMethod(
  const CosemMethodDescriptor& descriptor,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  return registry_.InvokeMethod(descriptor, input, output);
}

CosemStatus LogicalDevice::InvokeMethod(
  const CosemMethodDescriptor& descriptor,
  const CosemAccessContext& context,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  return registry_.InvokeMethod(descriptor, context, input, output);
}

CosemStatus PhysicalDevice::AddLogicalDevice(
  const std::shared_ptr<LogicalDevice>& logicalDevice)
{
  if (!logicalDevice || logicalDevice->Sap() == 0u) {
    return CosemStatus::InvalidArgument;
  }

  if (FindLogicalDevice(logicalDevice->Sap())) {
    return CosemStatus::DuplicateObject;
  }

  logicalDevices_.push_back(logicalDevice);
  return CosemStatus::Ok;
}

LogicalDevice* PhysicalDevice::FindLogicalDevice(std::uint16_t sap)
{
  for (std::vector<std::shared_ptr<LogicalDevice> >::iterator it =
         logicalDevices_.begin();
       it != logicalDevices_.end();
       ++it) {
    if ((*it)->Sap() == sap) {
      return it->get();
    }
  }
  return 0;
}

const LogicalDevice* PhysicalDevice::FindLogicalDevice(
  std::uint16_t sap) const
{
  for (std::vector<std::shared_ptr<LogicalDevice> >::const_iterator it =
         logicalDevices_.begin();
       it != logicalDevices_.end();
       ++it) {
    if ((*it)->Sap() == sap) {
      return it->get();
    }
  }
  return 0;
}

std::vector<SapAssignment> PhysicalDevice::SapAssignments() const
{
  std::vector<SapAssignment> assignments;
  for (std::vector<std::shared_ptr<LogicalDevice> >::const_iterator it =
         logicalDevices_.begin();
       it != logicalDevices_.end();
       ++it) {
    assignments.push_back((*it)->BuildSapAssignment());
  }
  return assignments;
}

std::size_t PhysicalDevice::Size() const
{
  return logicalDevices_.size();
}

} // namespace cosem
} // namespace dlms
