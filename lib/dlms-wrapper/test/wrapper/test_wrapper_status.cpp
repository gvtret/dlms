#include "dlms/wrapper/wrapper_error.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::wrapper::WrapperStatus;
using dlms::wrapper::WrapperStatusName;

// Covers every WrapperStatus value defined in wrapper_error.hpp.
TEST(WrapperStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", WrapperStatusName(WrapperStatus::Ok));
  EXPECT_STREQ("NeedMoreData",
               WrapperStatusName(WrapperStatus::NeedMoreData));
  EXPECT_STREQ("OutputBufferTooSmall",
               WrapperStatusName(WrapperStatus::OutputBufferTooSmall));
  EXPECT_STREQ("InvalidArgument",
               WrapperStatusName(WrapperStatus::InvalidArgument));
  EXPECT_STREQ("InvalidVersion",
               WrapperStatusName(WrapperStatus::InvalidVersion));
  EXPECT_STREQ("InvalidLength",
               WrapperStatusName(WrapperStatus::InvalidLength));
  EXPECT_STREQ("InvalidSourcePort",
               WrapperStatusName(WrapperStatus::InvalidSourcePort));
  EXPECT_STREQ("InvalidDestinationPort",
               WrapperStatusName(WrapperStatus::InvalidDestinationPort));
  EXPECT_STREQ("DataTooLarge",
               WrapperStatusName(WrapperStatus::DataTooLarge));
  EXPECT_STREQ("FrameTooLarge",
               WrapperStatusName(WrapperStatus::FrameTooLarge));
  EXPECT_STREQ("UnsupportedFeature",
               WrapperStatusName(WrapperStatus::UnsupportedFeature));
  EXPECT_STREQ("InternalError",
               WrapperStatusName(WrapperStatus::InternalError));
}

TEST(WrapperStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown",
               WrapperStatusName(static_cast<WrapperStatus>(255)));
}

} // namespace
