#pragma once

#include "dlms/association/association_types.hpp"

namespace dlms {
namespace association {

class IAssociationClient
{
public:
  virtual ~IAssociationClient()
  {
  }

  virtual AssociationStatus Open() = 0;
  virtual AssociationStatus Close() = 0;
  virtual AssociationStatus Establish() = 0;
  virtual AssociationStatus Release() = 0;

  virtual AssociationState State() const = 0;
  virtual bool IsAssociated() const = 0;
  virtual const AssociationResult& Result() const = 0;
};

} // namespace association
} // namespace dlms
