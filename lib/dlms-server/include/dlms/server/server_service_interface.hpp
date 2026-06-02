#pragma once

#include "dlms/server/server_types.hpp"

namespace dlms {
namespace server {

class IServerService
{
public:
  virtual ~IServerService();

  virtual ServerGetResponse HandleGet(const ServerGetRequest& request) = 0;
  virtual ServerSetResponse HandleSet(const ServerSetRequest& request) = 0;
  virtual ServerActionResponse HandleAction(
    const ServerActionRequest& request) = 0;
};

} // namespace server
} // namespace dlms
