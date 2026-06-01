#include "dlms/cosem/cosem_types.hpp"

namespace dlms {
namespace cosem {

CosemLogicalName::CosemLogicalName()
{
  for (std::size_t i = 0; i < Size(); ++i) {
    bytes_[i] = 0u;
  }
}

CosemLogicalName::CosemLogicalName(
  std::uint8_t a,
  std::uint8_t b,
  std::uint8_t c,
  std::uint8_t d,
  std::uint8_t e,
  std::uint8_t f)
{
  bytes_[0] = a;
  bytes_[1] = b;
  bytes_[2] = c;
  bytes_[3] = d;
  bytes_[4] = e;
  bytes_[5] = f;
}

const std::uint8_t* CosemLogicalName::Data() const
{
  return bytes_;
}

std::size_t CosemLogicalName::Size() const
{
  return 6u;
}

bool CosemLogicalName::IsEmpty() const
{
  for (std::size_t i = 0; i < Size(); ++i) {
    if (bytes_[i] != 0u) {
      return false;
    }
  }
  return true;
}

std::uint8_t CosemLogicalName::operator[](std::size_t index) const
{
  return index < Size() ? bytes_[index] : 0u;
}

CosemObjectKey EmptyCosemObjectKey()
{
  CosemObjectKey key;
  key.classId = 0u;
  key.version = 0u;
  key.logicalName = CosemLogicalName();
  return key;
}

CosemObjectDescriptor EmptyCosemObjectDescriptor()
{
  CosemObjectDescriptor descriptor;
  descriptor.key = EmptyCosemObjectKey();
  return descriptor;
}

CosemAttributeDescriptor EmptyCosemAttributeDescriptor()
{
  CosemAttributeDescriptor descriptor;
  descriptor.object = EmptyCosemObjectKey();
  descriptor.attributeId = 0u;
  return descriptor;
}

CosemMethodDescriptor EmptyCosemMethodDescriptor()
{
  CosemMethodDescriptor descriptor;
  descriptor.object = EmptyCosemObjectKey();
  descriptor.methodId = 0u;
  return descriptor;
}

CosemStatus ValidateObjectKey(const CosemObjectKey& key)
{
  if (key.classId == 0u || key.logicalName.IsEmpty()) {
    return CosemStatus::InvalidArgument;
  }

  return CosemStatus::Ok;
}

CosemStatus ValidateObjectDescriptor(
  const CosemObjectDescriptor& descriptor)
{
  return ValidateObjectKey(descriptor.key);
}

CosemStatus ValidateAttributeDescriptor(
  const CosemAttributeDescriptor& descriptor)
{
  const CosemStatus status = ValidateObjectKey(descriptor.object);
  if (status != CosemStatus::Ok) {
    return status;
  }
  return descriptor.attributeId == 0u
    ? CosemStatus::InvalidArgument
    : CosemStatus::Ok;
}

CosemStatus ValidateMethodDescriptor(
  const CosemMethodDescriptor& descriptor)
{
  const CosemStatus status = ValidateObjectKey(descriptor.object);
  if (status != CosemStatus::Ok) {
    return status;
  }
  return descriptor.methodId == 0u
    ? CosemStatus::InvalidArgument
    : CosemStatus::Ok;
}

bool operator==(
  const CosemLogicalName& lhs,
  const CosemLogicalName& rhs)
{
  for (std::size_t i = 0; i < lhs.Size(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(
  const CosemLogicalName& lhs,
  const CosemLogicalName& rhs)
{
  return !(lhs == rhs);
}

bool operator<(
  const CosemLogicalName& lhs,
  const CosemLogicalName& rhs)
{
  for (std::size_t i = 0; i < lhs.Size(); ++i) {
    if (lhs[i] < rhs[i]) {
      return true;
    }
    if (rhs[i] < lhs[i]) {
      return false;
    }
  }
  return false;
}

bool operator==(
  const CosemObjectKey& lhs,
  const CosemObjectKey& rhs)
{
  return lhs.classId == rhs.classId
    && lhs.version == rhs.version
    && lhs.logicalName == rhs.logicalName;
}

bool operator!=(
  const CosemObjectKey& lhs,
  const CosemObjectKey& rhs)
{
  return !(lhs == rhs);
}

bool operator<(
  const CosemObjectKey& lhs,
  const CosemObjectKey& rhs)
{
  if (lhs.classId != rhs.classId) {
    return lhs.classId < rhs.classId;
  }
  if (lhs.version != rhs.version) {
    return lhs.version < rhs.version;
  }
  return lhs.logicalName < rhs.logicalName;
}

} // namespace cosem
} // namespace dlms
