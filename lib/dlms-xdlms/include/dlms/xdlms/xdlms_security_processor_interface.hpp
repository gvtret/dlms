#pragma once

#include "dlms/security/security_types.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace xdlms {

class IXdlmsSecurityProcessor
{
public:
  virtual ~IXdlmsSecurityProcessor();

  virtual dlms::security::SecurityStatus Protect(
    dlms::security::SecurityByteView plainApdu,
    std::vector<std::uint8_t>& protectedApdu) const = 0;

  virtual dlms::security::SecurityStatus Unprotect(
    dlms::security::SecurityByteView protectedApdu,
    std::vector<std::uint8_t>& plainApdu) const = 0;
};

} // namespace xdlms
} // namespace dlms
