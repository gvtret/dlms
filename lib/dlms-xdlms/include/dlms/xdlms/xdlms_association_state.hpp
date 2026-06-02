#pragma once

namespace dlms {
namespace association {
class AssociationClient;
class IAssociationClient;
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
