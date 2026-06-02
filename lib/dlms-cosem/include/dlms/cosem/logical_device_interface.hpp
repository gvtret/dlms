#pragma once

#include "dlms/cosem/cosem_access.hpp"
#include "dlms/cosem/cosem_status.hpp"
#include "dlms/cosem/cosem_types.hpp"

namespace dlms {
namespace cosem {

class ILogicalDevice
{
public:
  virtual ~ILogicalDevice()
  {
  }

  virtual CosemStatus ReadAttribute(
    const CosemAttributeDescriptor& descriptor,
    const CosemAccessContext& context,
    CosemByteBuffer& output) const = 0;
  virtual CosemStatus WriteAttribute(
    const CosemAttributeDescriptor& descriptor,
    const CosemAccessContext& context,
    const CosemByteBuffer& input) = 0;
  virtual CosemStatus InvokeMethod(
    const CosemMethodDescriptor& descriptor,
    const CosemAccessContext& context,
    const CosemByteBuffer& input,
    CosemByteBuffer& output) = 0;
};

} // namespace cosem
} // namespace dlms
