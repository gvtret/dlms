#pragma once

#include "dlms/security/security_types.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace security {

class IInvocationCounterStore
{
public:
  virtual ~IInvocationCounterStore() {}

  virtual SecurityStatus NextLocal(
    std::uint32_t& invocationCounter) = 0;

  virtual SecurityStatus ValidateRemote(
    std::uint32_t invocationCounter) = 0;

  virtual SecurityStatus ValidateRemoteForSystemTitle(
    const std::uint8_t* systemTitle,
    std::size_t systemTitleSize,
    std::uint32_t invocationCounter)
  {
    (void)systemTitle;
    (void)systemTitleSize;
    return ValidateRemote(invocationCounter);
  }
};

class IInvocationCounterResetPolicy
{
public:
  virtual ~IInvocationCounterResetPolicy() {}

  virtual SecurityStatus ResetAfterKeyRotation(SecurityKeyRole role) = 0;
};

} // namespace security
} // namespace dlms
