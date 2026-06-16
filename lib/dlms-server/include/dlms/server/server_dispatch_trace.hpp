#pragma once

// IServerDispatchTraceSink: opt-in trace sink for the server-side
// dispatcher boundary in `dlms-server`. Distinct from
// `dlms::xdlms::IXdlmsTraceSink` (which sees raw xDLMS APDU shape and
// lives in `dlms-xdlms`) — this sink sees decoded service
// indications/results crossing the COSEM dispatch boundary. Two sinks,
// not one, so `dlms-xdlms` does not take a dependency on `dlms-server`
// types.
//
// Contract (matches the 5 existing trace sinks):
//   - Lifetime: caller owns the sink and keeps it alive for the
//     lifetime of the decorator / endpoint it is attached to.
//   - Re-entrancy: the sink MUST NOT call back into the dispatcher,
//     endpoint, or any DLMS layer object that is currently emitting.
//   - No-throw: implementations MUST NOT throw; any exception escaping
//     is undefined behaviour (decorator does not install a try/catch).
//   - Span validity: all fields are by-value primitives; no spans or
//     pointers escape the event.
//
// Redaction: events publish only sizes and identifiers (invokeId,
// classId, attribute or method id, OBIS LN, payload size, status).
// Never raw COSEM data, keys, system titles, GMAC tags, or HLS
// challenges.

#include "dlms/server/server_status.hpp"
#include "dlms/xdlms/xdlms_status.hpp"

#include <cstdint>

namespace dlms {
namespace server {

enum class ServerDispatchTraceKind
{
  GetDispatched,
  SetDispatched,
  ActionDispatched
};

struct ServerDispatchTraceEvent
{
  ServerDispatchTraceKind kind;
  // xDLMS-layer status returned by the dispatcher. `Ok` covers both
  // service success and access-result-bearing soft failures (which are
  // reported to the peer through the service response itself).
  dlms::xdlms::XdlmsStatus status;
  std::uint8_t invokeId;
  std::uint16_t classId;
  // For Get/Set this is the attribute id; for Action this is the
  // method id. Distinguish via `kind`.
  std::int8_t attributeOrMethodId;
  std::uint8_t logicalName[6];
  // Inbound payload size at the dispatch boundary: Get has no inbound
  // payload (always 0); Set carries the encoded value; Action carries
  // the encoded invocation parameter (0 when absent).
  std::size_t requestPayloadSize;
  // Outbound payload size at the dispatch boundary: Get/Action result
  // data; Set has no outbound payload (always 0).
  std::size_t responsePayloadSize;
  // Cross-layer correlation id; matches `IXdlmsTraceSink` and channel
  // sinks for the same service call. `kNoConversationId` (0) when the
  // dispatcher does not yet know it (server-side composition will fill
  // it in a future endpoint commit).
  std::uint64_t conversationId;
};

class IServerDispatchTraceSink
{
public:
  virtual ~IServerDispatchTraceSink();

  virtual void OnServerDispatchTrace(
    const ServerDispatchTraceEvent& event) = 0;
};

} // namespace server
} // namespace dlms
