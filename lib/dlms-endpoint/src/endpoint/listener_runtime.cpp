#include "dlms/endpoint/listener_runtime.hpp"

#include "dlms/endpoint/gateway_endpoint.hpp"
#include "dlms/endpoint/push_listener_endpoint.hpp"

namespace dlms {
namespace endpoint {

IApduChannelListener::~IApduChannelListener()
{
}

namespace {

EndpointStatus EnsureAcceptedChannel(
  const std::unique_ptr<dlms::profile::IApduChannel>& channel)
{
  return channel ? EndpointStatus::Ok : EndpointStatus::InternalError;
}

} // namespace

ServerListenerRuntime::ServerListenerRuntime(
  IApduChannelListener& listener,
  dlms::cosem::ILogicalDevice& logicalDevice)
  : listener_(listener)
  , options_(DefaultServerEndpointOptions())
  , logicalDevice_(&logicalDevice)
  , server_(0)
  , open_(false)
{
}

ServerListenerRuntime::ServerListenerRuntime(
  IApduChannelListener& listener,
  dlms::server::IServerService& server)
  : listener_(listener)
  , options_(DefaultServerEndpointOptions())
  , logicalDevice_(0)
  , server_(&server)
  , open_(false)
{
}

ServerListenerRuntime::ServerListenerRuntime(
  IApduChannelListener& listener,
  const ServerEndpointOptions& options,
  dlms::cosem::ILogicalDevice& logicalDevice)
  : listener_(listener)
  , options_(options)
  , logicalDevice_(&logicalDevice)
  , server_(0)
  , open_(false)
{
}

ServerListenerRuntime::ServerListenerRuntime(
  IApduChannelListener& listener,
  const ServerEndpointOptions& options,
  dlms::server::IServerService& server)
  : listener_(listener)
  , options_(options)
  , logicalDevice_(0)
  , server_(&server)
  , open_(false)
{
}

EndpointStatus ServerListenerRuntime::Open()
{
  if (open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = listener_.Open();
  if (status == EndpointStatus::Ok) {
    open_ = true;
  }
  return status;
}

EndpointStatus ServerListenerRuntime::RunOnce()
{
  if (!open_) {
    return EndpointStatus::InvalidState;
  }

  std::unique_ptr<dlms::profile::IApduChannel> channel;
  EndpointStatus status = listener_.Accept(channel);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = EnsureAcceptedChannel(channel);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  std::unique_ptr<ServerEndpoint> endpoint;
  if (server_ != 0) {
    endpoint.reset(new ServerEndpoint(*channel, options_, *server_));
  } else {
    endpoint.reset(new ServerEndpoint(*channel, options_, *logicalDevice_));
  }
  status = endpoint->Open();
  if (status != EndpointStatus::Ok) {
    return status;
  }

  const EndpointStatus runStatus = endpoint->RunOnce();
  const EndpointStatus closeStatus = endpoint->Close();
  return runStatus != EndpointStatus::Ok ? runStatus : closeStatus;
}

EndpointStatus ServerListenerRuntime::Close()
{
  if (!open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = listener_.Close();
  if (status == EndpointStatus::Ok) {
    open_ = false;
  }
  return status;
}

bool ServerListenerRuntime::IsOpen() const
{
  return open_;
}

PushListenerRuntime::PushListenerRuntime(
  IApduChannelListener& listener,
  IPushIndicationHandler& handler)
  : listener_(listener)
  , options_(DefaultPushListenerEndpointOptions())
  , handler_(handler)
  , open_(false)
{
}

PushListenerRuntime::PushListenerRuntime(
  IApduChannelListener& listener,
  const PushListenerEndpointOptions& options,
  IPushIndicationHandler& handler)
  : listener_(listener)
  , options_(options)
  , handler_(handler)
  , open_(false)
{
}

EndpointStatus PushListenerRuntime::Open()
{
  if (open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = listener_.Open();
  if (status == EndpointStatus::Ok) {
    open_ = true;
  }
  return status;
}

EndpointStatus PushListenerRuntime::RunOnce()
{
  if (!open_) {
    return EndpointStatus::InvalidState;
  }

  std::unique_ptr<dlms::profile::IApduChannel> channel;
  EndpointStatus status = listener_.Accept(channel);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = EnsureAcceptedChannel(channel);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  PushListenerEndpoint endpoint(*channel, options_, handler_);
  status = endpoint.Open();
  if (status != EndpointStatus::Ok) {
    return status;
  }

  const EndpointStatus runStatus = endpoint.RunOnce();
  const EndpointStatus closeStatus = endpoint.Close();
  return runStatus != EndpointStatus::Ok ? runStatus : closeStatus;
}

EndpointStatus PushListenerRuntime::Close()
{
  if (!open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = listener_.Close();
  if (status == EndpointStatus::Ok) {
    open_ = false;
  }
  return status;
}

bool PushListenerRuntime::IsOpen() const
{
  return open_;
}

GatewayListenerRuntime::GatewayListenerRuntime(
  IApduChannelListener& downstreamListener,
  IGatewayUpstream& upstream,
  IGatewayPolicy& policy)
  : downstreamListener_(downstreamListener)
  , options_(DefaultGatewayEndpointOptions())
  , upstream_(upstream)
  , policy_(policy)
  , open_(false)
{
}

GatewayListenerRuntime::GatewayListenerRuntime(
  IApduChannelListener& downstreamListener,
  const GatewayEndpointOptions& options,
  IGatewayUpstream& upstream,
  IGatewayPolicy& policy)
  : downstreamListener_(downstreamListener)
  , options_(options)
  , upstream_(upstream)
  , policy_(policy)
  , open_(false)
{
}

EndpointStatus GatewayListenerRuntime::Open()
{
  if (open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = downstreamListener_.Open();
  if (status == EndpointStatus::Ok) {
    open_ = true;
  }
  return status;
}

EndpointStatus GatewayListenerRuntime::RunOnce()
{
  if (!open_) {
    return EndpointStatus::InvalidState;
  }

  std::unique_ptr<dlms::profile::IApduChannel> channel;
  EndpointStatus status = downstreamListener_.Accept(channel);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  status = EnsureAcceptedChannel(channel);
  if (status != EndpointStatus::Ok) {
    return status;
  }

  GatewayEndpoint endpoint(*channel, options_, upstream_, policy_);
  status = endpoint.Open();
  if (status != EndpointStatus::Ok) {
    return status;
  }

  const EndpointStatus runStatus = endpoint.RunOnce();
  const EndpointStatus closeStatus = endpoint.Close();
  return runStatus != EndpointStatus::Ok ? runStatus : closeStatus;
}

EndpointStatus GatewayListenerRuntime::Close()
{
  if (!open_) {
    return EndpointStatus::Ok;
  }

  const EndpointStatus status = downstreamListener_.Close();
  if (status == EndpointStatus::Ok) {
    open_ = false;
  }
  return status;
}

bool GatewayListenerRuntime::IsOpen() const
{
  return open_;
}

} // namespace endpoint
} // namespace dlms
