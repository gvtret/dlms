#pragma once

#include "dlms/endpoint/apdu_channel_listener.hpp"
#include "dlms/endpoint/endpoint_options.hpp"
#include "dlms/endpoint/endpoint_status.hpp"
#include "dlms/endpoint/gateway_endpoint.hpp"
#include "dlms/endpoint/push_indication_handler.hpp"
#include "dlms/endpoint/server_endpoint.hpp"

#include "dlms/cosem/cosem.hpp"

namespace dlms {
namespace endpoint {

class ServerListenerRuntime
{
public:
  ServerListenerRuntime(
    IApduChannelListener& listener,
    dlms::cosem::ILogicalDevice& logicalDevice);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    dlms::server::IServerService& server);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    const ServerEndpointOptions& options,
    dlms::cosem::ILogicalDevice& logicalDevice);

  ServerListenerRuntime(
    IApduChannelListener& listener,
    const ServerEndpointOptions& options,
    dlms::server::IServerService& server);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;

private:
  ServerListenerRuntime(const ServerListenerRuntime&);
  ServerListenerRuntime& operator=(const ServerListenerRuntime&);

  IApduChannelListener& listener_;
  ServerEndpointOptions options_;
  dlms::cosem::ILogicalDevice* logicalDevice_;
  dlms::server::IServerService* server_;
  bool open_;
};

class PushListenerRuntime
{
public:
  PushListenerRuntime(
    IApduChannelListener& listener,
    IPushIndicationHandler& handler);

  PushListenerRuntime(
    IApduChannelListener& listener,
    const PushListenerEndpointOptions& options,
    IPushIndicationHandler& handler);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;

private:
  PushListenerRuntime(const PushListenerRuntime&);
  PushListenerRuntime& operator=(const PushListenerRuntime&);

  IApduChannelListener& listener_;
  PushListenerEndpointOptions options_;
  IPushIndicationHandler& handler_;
  bool open_;
};

class GatewayListenerRuntime
{
public:
  GatewayListenerRuntime(
    IApduChannelListener& downstreamListener,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);

  GatewayListenerRuntime(
    IApduChannelListener& downstreamListener,
    const GatewayEndpointOptions& options,
    IGatewayUpstream& upstream,
    IGatewayPolicy& policy);

  EndpointStatus Open();
  EndpointStatus RunOnce();
  EndpointStatus Close();

  bool IsOpen() const;

private:
  GatewayListenerRuntime(const GatewayListenerRuntime&);
  GatewayListenerRuntime& operator=(const GatewayListenerRuntime&);

  IApduChannelListener& downstreamListener_;
  GatewayEndpointOptions options_;
  IGatewayUpstream& upstream_;
  IGatewayPolicy& policy_;
  bool open_;
};

} // namespace endpoint
} // namespace dlms
