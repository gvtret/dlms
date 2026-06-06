#pragma once

#include "dlms/security/security_types.hpp"

#include <vector>

namespace dlms {
namespace security {

class Suite0KeyWrap
{
public:
  static const std::size_t kKeySize = 16u;
  static const std::size_t kWrappedKeySize = 24u;

  SecurityStatus Wrap(
    const SecurityKey& keyEncryptionKey,
    SecurityByteView plainKey,
    std::vector<std::uint8_t>& wrappedKey) const;

  SecurityStatus Unwrap(
    const SecurityKey& keyEncryptionKey,
    SecurityByteView wrappedKey,
    std::vector<std::uint8_t>& plainKey) const;
};

} // namespace security
} // namespace dlms
