#pragma once

#include "dlms/association/association_types.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace association {

class IAssociationServer
{
public:
  virtual ~IAssociationServer()
  {
  }

  virtual AssociationStatus Open() = 0;
  virtual AssociationStatus Close() = 0;
  virtual AssociationStatus Accept() = 0;
  virtual AssociationStatus Release() = 0;
  virtual AssociationStatus Release(
    const std::vector<std::uint8_t>& rlrq) = 0;

  virtual AssociationState State() const = 0;
  virtual bool IsAssociated() const = 0;
  virtual const AssociationResult& Result() const = 0;
};

} // namespace association
} // namespace dlms
