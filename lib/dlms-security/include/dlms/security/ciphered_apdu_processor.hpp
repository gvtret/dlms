#pragma once

#include "dlms/security/invocation_counter_store.hpp"
#include "dlms/security/key_store.hpp"
#include "dlms/security/security_types.hpp"

#include <vector>

namespace dlms {
namespace security {

class CipheredApduProcessor
{
public:
  // NOTE: `context` is stored by reference and must outlive this object.
  // The referenced SecurityContext may be mutated between calls (e.g. by the
  // endpoint when it learns the peer system title); the processor reads the
  // live values per call.
  CipheredApduProcessor(
    const SecurityContext& context,
    const IKeyStore& keys,
    IInvocationCounterStore& counters);

  SecurityStatus Protect(
    SecurityByteView plainApdu,
    std::vector<std::uint8_t>& protectedApdu) const;

  SecurityStatus Unprotect(
    SecurityByteView protectedApdu,
    std::vector<std::uint8_t>& plainApdu) const;

private:
  const SecurityContext& context_;
  const IKeyStore& keys_;
  IInvocationCounterStore& counters_;
};

} // namespace security
} // namespace dlms
