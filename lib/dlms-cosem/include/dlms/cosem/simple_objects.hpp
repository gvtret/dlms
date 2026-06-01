#pragma once

#include "dlms/cosem/logical_device.hpp"

namespace dlms {
namespace cosem {

class CosemDataObject : public ICosemObject
{
public:
  CosemDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    AttributeAccessMode valueAccess);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Value() const;
  void SetValue(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer value_;
  CosemAccessRights rights_;
};

class CosemRegisterObject : public ICosemObject
{
public:
  CosemRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode valueAccess);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Value() const;
  const CosemByteBuffer& ScalerUnit() const;
  void SetValue(const CosemByteBuffer& value);
  void SetScalerUnit(const CosemByteBuffer& scalerUnit);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer value_;
  CosemByteBuffer scalerUnit_;
  CosemAccessRights rights_;
};

CosemLogicalName CurrentAssociationLnName();
CosemLogicalName SapAssignmentName();
CosemLogicalName LogicalDeviceNameObjectName();
CosemDataObject MakeLogicalDeviceNameObject(
  const std::string& logicalDeviceName);

class CosemAssociationLnObject : public ICosemObject
{
public:
  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  AssociationView ObjectList() const;

private:
  CosemObjectDescriptor descriptor_;
  AssociationView objectList_;
  CosemAccessRights rights_;
};

class CosemSapAssignmentObject : public ICosemObject
{
public:
  CosemSapAssignmentObject(
    const CosemLogicalName& logicalName,
    const std::vector<SapAssignment>& assignments);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  std::vector<SapAssignment> Assignments() const;

private:
  CosemObjectDescriptor descriptor_;
  std::vector<SapAssignment> assignments_;
  CosemAccessRights rights_;
};

} // namespace cosem
} // namespace dlms
