#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms
git add -A
git status -s
echo ---
git commit -m "fix(client): surface HDLC DataLink disconnect failures with specific status

P0 \u00a71.3 from docs/production_readiness_roadmap.md: do not collapse
useful errors to InternalError when a more specific public status exists.

MapDataLinkDisconnectStatus() used a catch-all default branch that
mapped every non-trivial ProfileStatus to ClientStatus::InternalError.
DlmsClient::Close() and DlmsClient::ReleaseAssociation() therefore
hid Timeout, ConnectionClosed, malformed UA frames and IO errors
behind a generic library-bug status.

Replaced with an exhaustive switch over every ProfileStatus:
- Ok / NotOpen -> Ok (idempotent disconnect).
- InvalidArgument -> InvalidArgument.
- AlreadyOpen -> InvalidState.
- OpenFailed / WriteFailed -> SendFailed (DISC could not be sent).
- ReadFailed, Timeout, ConnectionClosed, WouldBlock, NeedMoreData,
  OutputBufferTooSmall, InvalidFrame, InvalidLength, InvalidAddress,
  PayloadTooLarge -> ReceiveFailed (no usable UA from the meter).
- UnsupportedFeature -> UnsupportedFeature.
- InternalError -> InternalError.

Removing the default keeps the mapper compile-checked against future
ProfileStatus additions; an unconditional trailing return preserves a
defensive fall-through for ABI drift / unknown integer values.

Added client_internal.hpp (source-tree-only, not installed) exposing
internal::MapDataLinkDisconnectStatus, delegating to the existing
anonymous-namespace mapper. Added test_client_internal.cpp with 8
cases pinning every ProfileStatus value.

Verification:
- ninja dlms_client_tests: clean rebuild.
- MapDataLinkDisconnectStatus.* gtest filter: 8/8 pass.
- dlms_client_tests: 54/54, no regression in existing Close() /
  ReleaseAssociation() coverage.
- Full root ctest: 935/935 pass in 59.52 s.

VERSION bumped 0.95.0 -> 0.96.0; CHANGELOG and handoff updated."
git log --oneline -5
