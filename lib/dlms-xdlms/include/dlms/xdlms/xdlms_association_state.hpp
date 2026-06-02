#pragma once

#include "dlms/association/association_client_interface.hpp"

namespace dlms {
namespace association {
class AssociationClient;
}
namespace xdlms {

class IXdlmsAssociationState
{
public:
  virtual ~IXdlmsAssociationState();

  virtual bool IsAssociated() const = 0;
};

class AssociationClientXdlmsAssociationState : public IXdlmsAssociationState
{
public:
  explicit AssociationClientXdlmsAssociationState(
    dlms::association::AssociationClient& association);

  explicit AssociationClientXdlmsAssociationState(
    dlms::association::IAssociationClient& association);

  bool IsAssociated() const;

private:
  dlms::association::IAssociationClient& association_;
};

} // namespace xdlms
} // namespace dlms
