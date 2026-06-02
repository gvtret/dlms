#pragma once

#include "dlms/xdlms/xdlms_security_processor_interface.hpp"

namespace dlms {
namespace security {
class CipheredApduProcessor;
}
namespace xdlms {

class CipheredXdlmsSecurityProcessor : public IXdlmsSecurityProcessor
{
public:
  explicit CipheredXdlmsSecurityProcessor(
    dlms::security::CipheredApduProcessor& processor);

  dlms::security::SecurityStatus Protect(
    dlms::security::SecurityByteView plainApdu,
    std::vector<std::uint8_t>& protectedApdu) const;

  dlms::security::SecurityStatus Unprotect(
    dlms::security::SecurityByteView protectedApdu,
    std::vector<std::uint8_t>& plainApdu) const;

private:
  dlms::security::CipheredApduProcessor& processor_;
};

} // namespace xdlms
} // namespace dlms
