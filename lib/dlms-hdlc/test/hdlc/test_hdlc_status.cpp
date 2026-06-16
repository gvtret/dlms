#include "dlms/hdlc/hdlc_error.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::hdlc::HdlcStatus;
using dlms::hdlc::HdlcStatusName;

// Covers every HdlcStatus value defined in hdlc_error.hpp.
TEST(HdlcStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", HdlcStatusName(HdlcStatus::Ok));
  EXPECT_STREQ("NeedMoreData", HdlcStatusName(HdlcStatus::NeedMoreData));
  EXPECT_STREQ("OutputBufferTooSmall",
               HdlcStatusName(HdlcStatus::OutputBufferTooSmall));
  EXPECT_STREQ("InvalidArgument",
               HdlcStatusName(HdlcStatus::InvalidArgument));
  EXPECT_STREQ("InvalidFlag", HdlcStatusName(HdlcStatus::InvalidFlag));
  EXPECT_STREQ("InvalidFrameFormat",
               HdlcStatusName(HdlcStatus::InvalidFrameFormat));
  EXPECT_STREQ("InvalidFrameType",
               HdlcStatusName(HdlcStatus::InvalidFrameType));
  EXPECT_STREQ("InvalidFrameLength",
               HdlcStatusName(HdlcStatus::InvalidFrameLength));
  EXPECT_STREQ("InvalidAddress",
               HdlcStatusName(HdlcStatus::InvalidAddress));
  EXPECT_STREQ("InvalidControlField",
               HdlcStatusName(HdlcStatus::InvalidControlField));
  EXPECT_STREQ("InvalidHeaderChecksum",
               HdlcStatusName(HdlcStatus::InvalidHeaderChecksum));
  EXPECT_STREQ("InvalidFrameChecksum",
               HdlcStatusName(HdlcStatus::InvalidFrameChecksum));
  EXPECT_STREQ("FrameTooLarge", HdlcStatusName(HdlcStatus::FrameTooLarge));
  EXPECT_STREQ("InformationFieldTooLarge",
               HdlcStatusName(HdlcStatus::InformationFieldTooLarge));
  EXPECT_STREQ("SegmentationError",
               HdlcStatusName(HdlcStatus::SegmentationError));
  EXPECT_STREQ("SegmentationIncomplete",
               HdlcStatusName(HdlcStatus::SegmentationIncomplete));
  EXPECT_STREQ("SegmentationOverflow",
               HdlcStatusName(HdlcStatus::SegmentationOverflow));
  EXPECT_STREQ("UnsupportedFrame",
               HdlcStatusName(HdlcStatus::UnsupportedFrame));
  EXPECT_STREQ("UnsupportedAddress",
               HdlcStatusName(HdlcStatus::UnsupportedAddress));
  EXPECT_STREQ("UnsupportedFeature",
               HdlcStatusName(HdlcStatus::UnsupportedFeature));
  EXPECT_STREQ("InternalError", HdlcStatusName(HdlcStatus::InternalError));
}

TEST(HdlcStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown", HdlcStatusName(static_cast<HdlcStatus>(255)));
}

} // namespace
