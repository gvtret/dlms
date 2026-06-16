#include "dlms/llc/llc_error.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::llc::LlcStatus;
using dlms::llc::LlcStatusName;

// Covers every LlcStatus value defined in llc_error.hpp.
TEST(LlcStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", LlcStatusName(LlcStatus::Ok));
  EXPECT_STREQ("NeedMoreData", LlcStatusName(LlcStatus::NeedMoreData));
  EXPECT_STREQ("OutputBufferTooSmall",
               LlcStatusName(LlcStatus::OutputBufferTooSmall));
  EXPECT_STREQ("InvalidArgument",
               LlcStatusName(LlcStatus::InvalidArgument));
  EXPECT_STREQ("InvalidHeader", LlcStatusName(LlcStatus::InvalidHeader));
  EXPECT_STREQ("InvalidDsap", LlcStatusName(LlcStatus::InvalidDsap));
  EXPECT_STREQ("InvalidSsap", LlcStatusName(LlcStatus::InvalidSsap));
  EXPECT_STREQ("InvalidControl", LlcStatusName(LlcStatus::InvalidControl));
  EXPECT_STREQ("InvalidLpduLength",
               LlcStatusName(LlcStatus::InvalidLpduLength));
  EXPECT_STREQ("LsduTooLarge", LlcStatusName(LlcStatus::LsduTooLarge));
  EXPECT_STREQ("BroadcastEncodeForbidden",
               LlcStatusName(LlcStatus::BroadcastEncodeForbidden));
  EXPECT_STREQ("UnsupportedAddress",
               LlcStatusName(LlcStatus::UnsupportedAddress));
  EXPECT_STREQ("UnsupportedControl",
               LlcStatusName(LlcStatus::UnsupportedControl));
  EXPECT_STREQ("UnsupportedFeature",
               LlcStatusName(LlcStatus::UnsupportedFeature));
  EXPECT_STREQ("InternalError", LlcStatusName(LlcStatus::InternalError));
}

TEST(LlcStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown", LlcStatusName(static_cast<LlcStatus>(255)));
}

} // namespace
