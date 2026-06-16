// Coverage for `internal::MapDataLinkDisconnectStatus`, exercised by
// `DlmsClient::Close()` / `DlmsClient::ReleaseAssociation()` when the
// client owns the HDLC data-link session and the meter-side DISC handshake
// produces non-OK statuses. The map is exposed through the
// `dlms::client::internal` namespace so we can pin every ProfileStatus
// without spinning up a real `HdlcProfileChannel`.

#include "../../src/client/client_internal.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::client::ClientStatus;
using dlms::client::internal::MapDataLinkDisconnectStatus;
using dlms::profile::ProfileStatus;

TEST(MapDataLinkDisconnectStatus, OkAndNotOpenAreIdempotentSuccess)
{
  EXPECT_EQ(ClientStatus::Ok,
            MapDataLinkDisconnectStatus(ProfileStatus::Ok));
  EXPECT_EQ(ClientStatus::Ok,
            MapDataLinkDisconnectStatus(ProfileStatus::NotOpen));
}

TEST(MapDataLinkDisconnectStatus, InvalidArgumentSurvives)
{
  EXPECT_EQ(ClientStatus::InvalidArgument,
            MapDataLinkDisconnectStatus(ProfileStatus::InvalidArgument));
}

TEST(MapDataLinkDisconnectStatus, AlreadyOpenIsInvalidState)
{
  EXPECT_EQ(ClientStatus::InvalidState,
            MapDataLinkDisconnectStatus(ProfileStatus::AlreadyOpen));
}

TEST(MapDataLinkDisconnectStatus, WriteAndOpenFailuresMapToSendFailed)
{
  EXPECT_EQ(ClientStatus::SendFailed,
            MapDataLinkDisconnectStatus(ProfileStatus::OpenFailed));
  EXPECT_EQ(ClientStatus::SendFailed,
            MapDataLinkDisconnectStatus(ProfileStatus::WriteFailed));
}

TEST(MapDataLinkDisconnectStatus, MeterSideReceiveFailuresMapToReceiveFailed)
{
  const ProfileStatus receiveFailures[] = {
    ProfileStatus::ReadFailed,
    ProfileStatus::Timeout,
    ProfileStatus::ConnectionClosed,
    ProfileStatus::WouldBlock,
    ProfileStatus::NeedMoreData,
    ProfileStatus::OutputBufferTooSmall,
    ProfileStatus::InvalidFrame,
    ProfileStatus::InvalidLength,
    ProfileStatus::InvalidAddress,
    ProfileStatus::PayloadTooLarge,
  };

  for (const ProfileStatus s : receiveFailures) {
    SCOPED_TRACE(static_cast<int>(s));
    EXPECT_EQ(ClientStatus::ReceiveFailed,
              MapDataLinkDisconnectStatus(s));
  }
}

TEST(MapDataLinkDisconnectStatus, UnsupportedFeatureSurvives)
{
  EXPECT_EQ(ClientStatus::UnsupportedFeature,
            MapDataLinkDisconnectStatus(ProfileStatus::UnsupportedFeature));
}

TEST(MapDataLinkDisconnectStatus, InternalErrorMapsToInternalError)
{
  EXPECT_EQ(ClientStatus::InternalError,
            MapDataLinkDisconnectStatus(ProfileStatus::InternalError));
}

TEST(MapDataLinkDisconnectStatus, UnknownEnumValueFallsThroughToInternalError)
{
  // Defensive default for ABI drift / forward-compatibility.
  EXPECT_EQ(ClientStatus::InternalError,
            MapDataLinkDisconnectStatus(static_cast<ProfileStatus>(0x7Fu)));
}

using dlms::client::internal::MapXdlmsStatus;
using dlms::xdlms::XdlmsStatus;

TEST(MapXdlmsStatus, OkPassesThrough)
{
  EXPECT_EQ(ClientStatus::Ok, MapXdlmsStatus(XdlmsStatus::Ok));
}

TEST(MapXdlmsStatus, DirectCategoriesPassThrough)
{
  EXPECT_EQ(ClientStatus::InvalidArgument,
            MapXdlmsStatus(XdlmsStatus::InvalidArgument));
  EXPECT_EQ(ClientStatus::InvalidState,
            MapXdlmsStatus(XdlmsStatus::InvalidState));
  EXPECT_EQ(ClientStatus::NotAssociated,
            MapXdlmsStatus(XdlmsStatus::NotAssociated));
  EXPECT_EQ(ClientStatus::SendFailed,
            MapXdlmsStatus(XdlmsStatus::SendFailed));
  EXPECT_EQ(ClientStatus::ReceiveFailed,
            MapXdlmsStatus(XdlmsStatus::ReceiveFailed));
  EXPECT_EQ(ClientStatus::ServiceRejected,
            MapXdlmsStatus(XdlmsStatus::ServiceRejected));
  EXPECT_EQ(ClientStatus::SecurityFailed,
            MapXdlmsStatus(XdlmsStatus::SecurityFailed));
  EXPECT_EQ(ClientStatus::UnsupportedFeature,
            MapXdlmsStatus(XdlmsStatus::UnsupportedFeature));
  EXPECT_EQ(ClientStatus::InternalError,
            MapXdlmsStatus(XdlmsStatus::InternalError));
}

TEST(MapXdlmsStatus, BlockTransferRequiredIsDistinct)
{
  // Previously collapsed into UnsupportedFeature; the protocol feature
  // exists but the simple non-block client path did not engage it.
  EXPECT_EQ(ClientStatus::BlockTransferRequired,
            MapXdlmsStatus(XdlmsStatus::BlockTransferRequired));
}

TEST(MapXdlmsStatus, EncodeAndDecodeFailuresMapToCodecFailed)
{
  // Previously collapsed into InternalError; both signal wire-level
  // corruption or a spec mismatch with the peer, not a library bug.
  EXPECT_EQ(ClientStatus::CodecFailed,
            MapXdlmsStatus(XdlmsStatus::EncodeFailed));
  EXPECT_EQ(ClientStatus::CodecFailed,
            MapXdlmsStatus(XdlmsStatus::DecodeFailed));
}

TEST(MapXdlmsStatus, InvokeIdMismatchIsDistinct)
{
  // Previously collapsed into InternalError; the response was framed
  // and decoded fine, the correlation id is the problem.
  EXPECT_EQ(ClientStatus::InvokeIdMismatch,
            MapXdlmsStatus(XdlmsStatus::InvokeIdMismatch));
}

TEST(MapXdlmsStatus, UnknownEnumValueFallsThroughToInternalError)
{
  EXPECT_EQ(ClientStatus::InternalError,
            MapXdlmsStatus(static_cast<XdlmsStatus>(0x7Fu)));
}

} // namespace
