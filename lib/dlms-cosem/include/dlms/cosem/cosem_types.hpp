#pragma once

#include "dlms/cosem/cosem_status.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dlms {
namespace cosem {

typedef std::vector<std::uint8_t> CosemByteBuffer;

class CosemLogicalName
{
public:
  CosemLogicalName();
  CosemLogicalName(
    std::uint8_t a,
    std::uint8_t b,
    std::uint8_t c,
    std::uint8_t d,
    std::uint8_t e,
    std::uint8_t f);

  const std::uint8_t* Data() const;
  std::size_t Size() const;
  bool IsEmpty() const;
  std::uint8_t operator[](std::size_t index) const;

private:
  std::uint8_t bytes_[6];
};

struct CosemObjectKey
{
  std::uint16_t classId;
  std::uint8_t version;
  CosemLogicalName logicalName;
};

struct CosemObjectDescriptor
{
  CosemObjectKey key;
};

struct CosemAttributeDescriptor
{
  CosemObjectKey object;
  std::uint8_t attributeId;
};

struct CosemMethodDescriptor
{
  CosemObjectKey object;
  std::uint8_t methodId;
};

CosemObjectKey EmptyCosemObjectKey();
CosemObjectDescriptor EmptyCosemObjectDescriptor();
CosemAttributeDescriptor EmptyCosemAttributeDescriptor();
CosemMethodDescriptor EmptyCosemMethodDescriptor();
CosemStatus ValidateObjectKey(const CosemObjectKey& key);
CosemStatus ValidateObjectDescriptor(
  const CosemObjectDescriptor& descriptor);
CosemStatus ValidateAttributeDescriptor(
  const CosemAttributeDescriptor& descriptor);
CosemStatus ValidateMethodDescriptor(
  const CosemMethodDescriptor& descriptor);

bool operator==(
  const CosemLogicalName& lhs,
  const CosemLogicalName& rhs);
bool operator!=(
  const CosemLogicalName& lhs,
  const CosemLogicalName& rhs);
bool operator<(
  const CosemLogicalName& lhs,
  const CosemLogicalName& rhs);
bool operator==(
  const CosemObjectKey& lhs,
  const CosemObjectKey& rhs);
bool operator!=(
  const CosemObjectKey& lhs,
  const CosemObjectKey& rhs);
bool operator<(
  const CosemObjectKey& lhs,
  const CosemObjectKey& rhs);

} // namespace cosem
} // namespace dlms
