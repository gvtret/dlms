#pragma once

#include "dlms/server/service_dispatcher.hpp"
#include "dlms/server/server_context.hpp"
#include "dlms/server/server_service_interface.hpp"

namespace dlms {
namespace server {

class DlmsServer : public IServerService
{
public:
  explicit DlmsServer(ServerContext& context);

  ServerGetResponse HandleGet(const ServerGetRequest& request);
  ServerSetResponse HandleSet(const ServerSetRequest& request);
  ServerActionResponse HandleAction(const ServerActionRequest& request);

private:
  CosemServiceDispatcher dispatcher_;
};

} // namespace server
} // namespace dlms
