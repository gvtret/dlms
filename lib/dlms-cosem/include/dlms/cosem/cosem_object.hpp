#pragma once

#include "dlms/cosem/cosem_access.hpp"
#include "dlms/cosem/cosem_types.hpp"

namespace dlms {
namespace cosem {

class ICosemObject
{
public:
  virtual ~ICosemObject();

  virtual CosemObjectDescriptor Descriptor() const = 0;
  virtual CosemAccessRights AccessRights() const = 0;
  virtual CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const = 0;
  virtual CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input) = 0;
  virtual CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output) = 0;
};

} // namespace cosem
} // namespace dlms
