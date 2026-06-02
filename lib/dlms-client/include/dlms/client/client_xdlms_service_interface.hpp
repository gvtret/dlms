#pragma once

#include "dlms/xdlms/xdlms_status.hpp"
#include "dlms/xdlms/xdlms_types.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace client {

using CosemAttributeDescriptor = dlms::xdlms::CosemAttributeDescriptor;
using CosemMethodDescriptor = dlms::xdlms::CosemMethodDescriptor;

class IClientXdlmsService
{
public:
  virtual ~IClientXdlmsService();

  virtual dlms::xdlms::XdlmsStatus Get(
    const CosemAttributeDescriptor& descriptor,
    dlms::xdlms::GetResult& result) = 0;

  virtual dlms::xdlms::XdlmsStatus Set(
    const CosemAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData,
    dlms::xdlms::SetResult& result) = 0;

  virtual dlms::xdlms::XdlmsStatus Action(
    const CosemMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    dlms::xdlms::ActionResult& result) = 0;
};

} // namespace client
} // namespace dlms
