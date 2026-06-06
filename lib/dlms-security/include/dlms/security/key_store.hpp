#pragma once

#include "dlms/security/security_types.hpp"

namespace dlms {
namespace security {

class IKeyStore
{
public:
  virtual ~IKeyStore() {}

  virtual SecurityStatus GetKey(
    SecurityKeyRole role,
    SecurityKey& output) const = 0;
};

class IMutableKeyStore : public IKeyStore
{
public:
  virtual SecurityStatus SetKey(const SecurityKey& key) = 0;
};

} // namespace security
} // namespace dlms
