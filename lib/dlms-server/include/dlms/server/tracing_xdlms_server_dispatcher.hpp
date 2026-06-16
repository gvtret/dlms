#pragma once

// TracingXdlmsServerDispatcher: thin pass-through decorator around any
// `dlms::xdlms::IXdlmsServerDispatcher` that emits one
// `ServerDispatchTraceEvent` per dispatch call. Constructed inside
// `dlms-server` so that `dlms-xdlms` does not pick up a dependency on
// server-layer types (see `IServerDispatchTraceSink` contract notes).
//
// Behaviour when `sink == nullptr`: dispatch is forwarded verbatim and
// no event object is constructed — zero-cost on the hot path.

#include "dlms/profile/apdu_channel.hpp"
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

  // Optional: install a back-reference to the APDU channel so emitted
  // dispatch events carry the same `conversationId` that the xDLMS
  // processor already stamped on the channel via
  // `IApduChannel::SetCorrelation(...)`. When `channel == nullptr`
  // (default), events keep using `kNoConversationId == 0`. The
  // decorator only ever calls `channel->CurrentConversationId()` —
  // it does not send/receive APDUs. See
  // `docs/trace_correlation_design.md`.
  void SetCorrelationChannel(dlms::profile::IApduChannel* channel) noexcept
  {
    channel_ = channel;
  }
  dlms::profile::IApduChannel* CorrelationChannel() const noexcept
  {
    return channel_;
  }

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
  dlms::profile::IApduChannel* channel_;
};

} // namespace server
} // namespace dlms
