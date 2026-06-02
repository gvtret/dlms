#pragma once

#include "dlms/profile/profile_types.hpp"

namespace dlms {
namespace profile {

class IHdlcDataLinkSession
{
public:
  virtual ~IHdlcDataLinkSession() {}

  virtual ProfileStatus ConnectDataLink() = 0;
  virtual ProfileStatus AcceptDataLink() = 0;
  virtual ProfileStatus DisconnectDataLink() = 0;
};

} // namespace profile
} // namespace dlms
