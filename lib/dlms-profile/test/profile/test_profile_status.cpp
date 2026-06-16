#include "dlms/profile/profile_types.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::profile::ProfileStatus;
using dlms::profile::ProfileStatusName;

// Covers every ProfileStatus value defined in profile_types.hpp.
TEST(ProfileStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", ProfileStatusName(ProfileStatus::Ok));
  EXPECT_STREQ("NeedMoreData",
               ProfileStatusName(ProfileStatus::NeedMoreData));
  EXPECT_STREQ("OutputBufferTooSmall",
               ProfileStatusName(ProfileStatus::OutputBufferTooSmall));
  EXPECT_STREQ("InvalidArgument",
               ProfileStatusName(ProfileStatus::InvalidArgument));
  EXPECT_STREQ("NotOpen", ProfileStatusName(ProfileStatus::NotOpen));
  EXPECT_STREQ("AlreadyOpen",
               ProfileStatusName(ProfileStatus::AlreadyOpen));
  EXPECT_STREQ("OpenFailed", ProfileStatusName(ProfileStatus::OpenFailed));
  EXPECT_STREQ("ReadFailed", ProfileStatusName(ProfileStatus::ReadFailed));
  EXPECT_STREQ("WriteFailed",
               ProfileStatusName(ProfileStatus::WriteFailed));
  EXPECT_STREQ("Timeout", ProfileStatusName(ProfileStatus::Timeout));
  EXPECT_STREQ("ConnectionClosed",
               ProfileStatusName(ProfileStatus::ConnectionClosed));
  EXPECT_STREQ("WouldBlock", ProfileStatusName(ProfileStatus::WouldBlock));
  EXPECT_STREQ("InvalidFrame",
               ProfileStatusName(ProfileStatus::InvalidFrame));
  EXPECT_STREQ("InvalidLength",
               ProfileStatusName(ProfileStatus::InvalidLength));
  EXPECT_STREQ("InvalidAddress",
               ProfileStatusName(ProfileStatus::InvalidAddress));
  EXPECT_STREQ("PayloadTooLarge",
               ProfileStatusName(ProfileStatus::PayloadTooLarge));
  EXPECT_STREQ("UnsupportedFeature",
               ProfileStatusName(ProfileStatus::UnsupportedFeature));
  EXPECT_STREQ("InternalError",
               ProfileStatusName(ProfileStatus::InternalError));
}

TEST(ProfileStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown",
               ProfileStatusName(static_cast<ProfileStatus>(255)));
}

} // namespace
