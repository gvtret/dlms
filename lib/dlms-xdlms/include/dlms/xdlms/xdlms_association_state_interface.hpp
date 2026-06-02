#pragma once

namespace dlms {
namespace xdlms {

class IXdlmsAssociationState
{
public:
  virtual ~IXdlmsAssociationState();

  virtual bool IsAssociated() const = 0;
};

} // namespace xdlms
} // namespace dlms
