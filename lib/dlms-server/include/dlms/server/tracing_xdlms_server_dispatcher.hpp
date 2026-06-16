#pragma once

// TracingXdlmsServerDispatcher: thin pass-through decorator around any
// `dlms::xdlms::IXdlmsServerDispatcher` that emits one
// `ServerDispatchTraceEvent` per dispatch call. Constructed inside
// `dlms-server` so that `dlms-xdlms` does not pick up a dependency on
// server-layer types (see `IServerDispatchTraceSink` contract notes).
//
// Behaviour when `sink == nullptr`: dispatch is forwarded verbatim and
// no event object is constructed — zero-cost on the hot path.

#include "dlms/server/server_dispatch_trace.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

namespace dlms {
namespace server {

class TracingXdlmsServerDispatcher : public dlms::xdlms::IXdlmsServerDispatcher
{
public:
  TracingXdlmsServerDispatcher(
    dlms::xdlms::IXdlmsServerDispatcher& inner,
    IServerDispatchTraceSink* sink);

  dlms::xdlms::XdlmsStatus DispatchGet(
    const dlms::xdlms::GetIndication& indication,
    dlms::xdlms::GetResult& result);

  dlms::xdlms::XdlmsStatus DispatchSet(
    const dlms::xdlms::SetIndication& indication,
    dlms::xdlms::SetResult& result);

  dlms::xdlms::XdlmsStatus DispatchAction(
    const dlms::xdlms::ActionIndication& indication,
    dlms::xdlms::ActionResult& result);

  void SetSink(IServerDispatchTraceSink* sink) noexcept { sink_ = sink; }
  IServerDispatchTraceSink* Sink() const noexcept { return sink_; }

private:
  dlms::xdlms::IXdlmsServerDispatcher& inner_;
  IServerDispatchTraceSink* sink_;
};

} // namespace server
} // namespace dlms
