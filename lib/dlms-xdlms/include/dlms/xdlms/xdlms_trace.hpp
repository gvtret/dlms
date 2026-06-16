#pragma once

// Public trace surface for the xDLMS layer (P1 §7).
//
// Both XdlmsClient (outbound) and XdlmsServerApduProcessor (inbound)
// emit XdlmsTraceEvent records to an optional consumer-supplied
// IXdlmsTraceSink. The sink is default-null: when not configured,
// no event is constructed, no virtual call is made.
//
// Contract (see docs/trace_contracts.md and
// docs/xdlms_server_trace_design.md):
//   - Synchronous, called on the framework thread that owns the
//     request/response.
//   - Must not throw. Implementations that may throw must catch
//     at the boundary.
//   - The event reference is valid only for the call duration; the
//     event has no pointer/span members, so this is automatic.
//   - Must not call back into the originating XdlmsClient or
//     XdlmsServerApduProcessor from inside the hook.
//   - Carries only sizes and identifiers (class id, attribute /
//     method id, OBIS LN, invoke id, sizes). Never raw APDU bytes
//     or plaintext payload. No keys / system titles / GMAC tags /
//     HLS challenges.
//   - The conversationId field carries the §2 cross-layer
//     correlation id (kNoConversationId == 0 if the consumer has
//     not opted in).

#include "dlms/xdlms/xdlms_status.hpp"
#include "dlms/xdlms/xdlms_types.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace xdlms {

enum class XdlmsTraceKind : std::uint8_t
{
  // Outbound (XdlmsClient)
  GetRequest,
  SetRequest,
  ActionRequest,
  GetResponse,
  SetResponse,
  ActionResponse,
  BlockTransferStep,

  // Inbound (XdlmsServerApduProcessor)
  RequestReceived,
  ResponseSent,

  // Either side
  DecodeFailed,
  InvokeIdRejected,
  SecurityFailed,
};

enum class XdlmsTraceDirection : std::uint8_t
{
  Outbound,
  Inbound,
};

struct XdlmsTraceEvent
{
  XdlmsTraceKind      kind;
  XdlmsTraceDirection direction;
  XdlmsStatus         status;
  std::uint8_t        invokeId;
  ServiceOptions      options;
  std::uint16_t       classId;
  std::uint8_t        attributeOrMethodId;
  std::uint8_t        logicalName[6];
  bool                hasBlockNumber;
  std::uint32_t       blockNumber;
  std::size_t         apduSize;
  std::size_t         payloadSize;
  std::uint64_t       conversationId;
};

class IXdlmsTraceSink
{
public:
  virtual ~IXdlmsTraceSink();
  virtual void OnXdlmsTrace(const XdlmsTraceEvent& event) = 0;
};

XdlmsTraceEvent EmptyXdlmsTraceEvent();

} // namespace xdlms
} // namespace dlms
