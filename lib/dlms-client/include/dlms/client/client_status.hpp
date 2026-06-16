#pragma once

namespace dlms {
namespace client {

enum class ClientStatus
{
  Ok,
  InvalidArgument,
  InvalidState,
  TransportOpenFailed,
  ChannelOpenFailed,
  AssociationFailed,
  NotAssociated,
  SendFailed,
  ReceiveFailed,
  ServiceRejected,
  SecurityFailed,
  UnsupportedFeature,
  // The xDLMS layer asked the caller to switch to block transfer; the
  // request did not fail per se, but the simple non-block path cannot
  // complete it. Distinct from UnsupportedFeature because the feature
  // exists, the client just did not engage it.
  BlockTransferRequired,
  // The xDLMS layer received a response whose invoke-id does not match
  // the outstanding request. Distinct from ReceiveFailed because the
  // response was framed and decoded; the correlation is wrong.
  InvokeIdMismatch,
  // APDU encode or decode failed inside the xDLMS layer. Distinct from
  // InternalError because it implies wire-level corruption or a spec
  // mismatch with the peer, not a library bug.
  CodecFailed,
  InternalError
};

const char* ClientStatusName(ClientStatus status);

} // namespace client
} // namespace dlms
