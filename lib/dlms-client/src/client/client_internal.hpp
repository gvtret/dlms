#pragma once

// Internal declarations for dlms-client unit tests.
// This header is not part of the public install tree and must not be
// included from outside the dlms-client source/test trees.

#include "dlms/client/client_status.hpp"
#include "dlms/profile/profile_types.hpp"

namespace dlms {
namespace client {
namespace internal {

// Maps a ProfileStatus returned by `HdlcProfileChannel::DisconnectDataLink()`
// into the public `ClientStatus` reported by `DlmsClient::Close()` and
// `DlmsClient::ReleaseAssociation()`.
//
// The mapping preserves the most actionable category for callers; in
// particular `Timeout`, `ConnectionClosed`, malformed UA frames and any
// other receive-side failure collapse into `ReceiveFailed` rather than
// `InternalError`, so that disconnect failures are distinguishable from
// genuine library bugs.
ClientStatus MapDataLinkDisconnectStatus(
  dlms::profile::ProfileStatus status);

} // namespace internal
} // namespace client
} // namespace dlms
