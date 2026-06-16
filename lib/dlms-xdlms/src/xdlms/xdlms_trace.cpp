#include "dlms/xdlms/xdlms_trace.hpp"

namespace dlms {
namespace xdlms {

IXdlmsTraceSink::~IXdlmsTraceSink()
{
}

XdlmsTraceEvent EmptyXdlmsTraceEvent()
{
  XdlmsTraceEvent event = {};
  event.kind = XdlmsTraceKind::GetRequest;
  event.direction = XdlmsTraceDirection::Outbound;
  event.status = XdlmsStatus::Ok;
  event.invokeId = 0u;
  event.options = DefaultServiceOptions();
  event.classId = 0u;
  event.attributeOrMethodId = 0u;
  for (std::size_t i = 0; i < sizeof(event.logicalName); ++i) {
    event.logicalName[i] = 0u;
  }
  event.hasBlockNumber = false;
  event.blockNumber = 0u;
  event.apduSize = 0u;
  event.payloadSize = 0u;
  event.conversationId = 0u;
  return event;
}

} // namespace xdlms
} // namespace dlms
