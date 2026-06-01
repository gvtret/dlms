#pragma once

#include "dlms/cosem/cosem_access.hpp"
#include "dlms/cosem/cosem_object.hpp"
#include "dlms/cosem/cosem_status.hpp"
#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/object_registry.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace dlms {
namespace cosem {

struct AssociationViewObject
{
  CosemObjectDescriptor descriptor;
  CosemAccessRights accessRights;
};

struct AssociationView
{
  std::vector<AssociationViewObject> objects;
};

struct SapAssignment
{
  std::uint16_t sap;
  std::string logicalDeviceName;
};

struct AssociationLnMetadata
{
  CosemObjectDescriptor descriptor;
  AssociationView objectList;
};

class LogicalDevice
{
public:
  LogicalDevice(std::uint16_t sap, const std::string& name);

  std::uint16_t Sap() const;
  const std::string& Name() const;
  ObjectRegistry& Registry();
  const ObjectRegistry& Registry() const;

  CosemStatus RegisterObject(const std::shared_ptr<ICosemObject>& object);
  AssociationView BuildAssociationView() const;
  SapAssignment BuildSapAssignment() const;
  AssociationLnMetadata BuildAssociationLnMetadata(
    const CosemObjectDescriptor& associationObject) const;

  CosemStatus ReadAttribute(
    const CosemAttributeDescriptor& descriptor,
    CosemByteBuffer& output) const;
  CosemStatus ReadAttribute(
    const CosemAttributeDescriptor& descriptor,
    const CosemAccessContext& context,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    const CosemAttributeDescriptor& descriptor,
    const CosemByteBuffer& input);
  CosemStatus WriteAttribute(
    const CosemAttributeDescriptor& descriptor,
    const CosemAccessContext& context,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    const CosemMethodDescriptor& descriptor,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);
  CosemStatus InvokeMethod(
    const CosemMethodDescriptor& descriptor,
    const CosemAccessContext& context,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

private:
  std::uint16_t sap_;
  std::string name_;
  ObjectRegistry registry_;
};

class PhysicalDevice
{
public:
  CosemStatus AddLogicalDevice(
    const std::shared_ptr<LogicalDevice>& logicalDevice);
  LogicalDevice* FindLogicalDevice(std::uint16_t sap);
  const LogicalDevice* FindLogicalDevice(std::uint16_t sap) const;
  std::vector<SapAssignment> SapAssignments() const;
  std::size_t Size() const;

private:
  std::vector<std::shared_ptr<LogicalDevice> > logicalDevices_;
};

} // namespace cosem
} // namespace dlms
