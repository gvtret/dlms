#pragma once

#include "dlms/association/association_client_interface.hpp"
#include "dlms/xdlms/xdlms_association_state_interface.hpp"

namespace dlms {
namespace association {
class AssociationClient;
}
namespace xdlms {

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
