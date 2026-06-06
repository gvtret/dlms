#pragma once

#include "dlms/security/invocation_counter_store.hpp"

#include <map>
#include <vector>

namespace dlms {
namespace security {

class InMemoryInvocationCounterStore : public IInvocationCounterStore
{
public:
  InMemoryInvocationCounterStore();

  void SetLocalCounter(std::uint32_t invocationCounter);
  void SetHighestRemoteCounter(std::uint32_t invocationCounter);

  SecurityStatus NextLocal(std::uint32_t& invocationCounter);
  SecurityStatus ValidateRemote(std::uint32_t invocationCounter);
  SecurityStatus ValidateRemoteForSystemTitle(
    const std::uint8_t* systemTitle,
    std::size_t systemTitleSize,
    std::uint32_t invocationCounter);

private:
  std::uint32_t nextLocal_;
  std::uint32_t highestRemote_;
  std::map<std::vector<std::uint8_t>, std::uint32_t>
    highestRemoteBySystemTitle_;
};

} // namespace security
} // namespace dlms
