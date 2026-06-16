#include "dlms/server/tracing_xdlms_server_dispatcher.hpp"

#include "dlms/xdlms/xdlms_correlation.hpp"

namespace {

constexpr std::uint64_t kNoConversationId =
  dlms::xdlms::kNoConversationId;

void FillLogicalName(
  std::uint8_t (&output)[6],
  const dlms::xdlms::CosemLogicalName& logicalName)
{
  for (std::size_t i = 0u; i < 6u; ++i) {
    output[i] = logicalName[i];
  }
}

} // namespace

namespace dlms {
namespace server {

TracingXdlmsServerDispatcher::TracingXdlmsServerDispatcher(
  dlms::xdlms::IXdlmsServerDispatcher& inner,
  IServerDispatchTraceSink* sink)
  : inner_(inner)
  , sink_(sink)
{
}

dlms::xdlms::XdlmsStatus TracingXdlmsServerDispatcher::DispatchGet(
  const dlms::xdlms::GetIndication& indication,
  dlms::xdlms::GetResult& result)
{
  const dlms::xdlms::XdlmsStatus status = inner_.DispatchGet(indication, result);

  if (sink_ != 0) {
    ServerDispatchTraceEvent event = {};
    event.kind = ServerDispatchTraceKind::GetDispatched;
    event.status = status;
    event.invokeId = indication.invokeId;
    event.classId = indication.descriptor.classId;
    event.attributeOrMethodId =
      static_cast<std::int8_t>(indication.descriptor.attributeId);
    FillLogicalName(event.logicalName, indication.descriptor.instanceId);
    event.requestPayloadSize = 0u;
    event.responsePayloadSize = result.hasData ? result.data.size() : 0u;
    event.conversationId = kNoConversationId;
    sink_->OnServerDispatchTrace(event);
  }

  return status;
}

dlms::xdlms::XdlmsStatus TracingXdlmsServerDispatcher::DispatchSet(
  const dlms::xdlms::SetIndication& indication,
  dlms::xdlms::SetResult& result)
{
  const dlms::xdlms::XdlmsStatus status = inner_.DispatchSet(indication, result);

  if (sink_ != 0) {
    ServerDispatchTraceEvent event = {};
    event.kind = ServerDispatchTraceKind::SetDispatched;
    event.status = status;
    event.invokeId = indication.invokeId;
    event.classId = indication.descriptor.classId;
    event.attributeOrMethodId =
      static_cast<std::int8_t>(indication.descriptor.attributeId);
    FillLogicalName(event.logicalName, indication.descriptor.instanceId);
    event.requestPayloadSize = indication.data.size();
    event.responsePayloadSize = 0u;
    event.conversationId = kNoConversationId;
    sink_->OnServerDispatchTrace(event);
  }

  return status;
}

dlms::xdlms::XdlmsStatus TracingXdlmsServerDispatcher::DispatchAction(
  const dlms::xdlms::ActionIndication& indication,
  dlms::xdlms::ActionResult& result)
{
  const dlms::xdlms::XdlmsStatus status = inner_.DispatchAction(indication, result);

  if (sink_ != 0) {
    ServerDispatchTraceEvent event = {};
    event.kind = ServerDispatchTraceKind::ActionDispatched;
    event.status = status;
    event.invokeId = indication.invokeId;
    event.classId = indication.descriptor.classId;
    event.attributeOrMethodId =
      static_cast<std::int8_t>(indication.descriptor.methodId);
    FillLogicalName(event.logicalName, indication.descriptor.instanceId);
    event.requestPayloadSize = indication.parameter.size();
    event.responsePayloadSize = result.hasData ? result.data.size() : 0u;
    event.conversationId = kNoConversationId;
    sink_->OnServerDispatchTrace(event);
  }

  return status;
}

} // namespace server
} // namespace dlms
