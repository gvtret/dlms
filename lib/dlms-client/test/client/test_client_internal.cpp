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

} // namespace
