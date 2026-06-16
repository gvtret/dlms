#include "dlms/apdu/apdu_error.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduStatusName;

// Covers every ApduStatus value defined in apdu_error.hpp.
// If a new enum value is added without extending ApduStatusName(),
// the corresponding switch in the implementation triggers -Wswitch
// (no default:); this test additionally pins the stable string form.
TEST(ApduStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", ApduStatusName(ApduStatus::Ok));
  EXPECT_STREQ("NeedMoreData", ApduStatusName(ApduStatus::NeedMoreData));
  EXPECT_STREQ("OutputBufferTooSmall",
               ApduStatusName(ApduStatus::OutputBufferTooSmall));
  EXPECT_STREQ("InvalidArgument",
               ApduStatusName(ApduStatus::InvalidArgument));
  EXPECT_STREQ("InvalidTag", ApduStatusName(ApduStatus::InvalidTag));
  EXPECT_STREQ("InvalidLength", ApduStatusName(ApduStatus::InvalidLength));
  EXPECT_STREQ("InvalidBer", ApduStatusName(ApduStatus::InvalidBer));
  EXPECT_STREQ("InvalidAxdr", ApduStatusName(ApduStatus::InvalidAxdr));
  EXPECT_STREQ("InvalidChoice", ApduStatusName(ApduStatus::InvalidChoice));
  EXPECT_STREQ("InvalidData", ApduStatusName(ApduStatus::InvalidData));
  EXPECT_STREQ("InvalidInvokeId",
               ApduStatusName(ApduStatus::InvalidInvokeId));
  EXPECT_STREQ("InvalidDescriptor",
               ApduStatusName(ApduStatus::InvalidDescriptor));
  EXPECT_STREQ("InvalidConformance",
               ApduStatusName(ApduStatus::InvalidConformance));
  EXPECT_STREQ("UnsupportedApdu",
               ApduStatusName(ApduStatus::UnsupportedApdu));
  EXPECT_STREQ("UnsupportedAcseField",
               ApduStatusName(ApduStatus::UnsupportedAcseField));
  EXPECT_STREQ("UnsupportedXdlmsService",
               ApduStatusName(ApduStatus::UnsupportedXdlmsService));
  EXPECT_STREQ("UnsupportedDataType",
               ApduStatusName(ApduStatus::UnsupportedDataType));
  EXPECT_STREQ("UnsupportedFeature",
               ApduStatusName(ApduStatus::UnsupportedFeature));
  EXPECT_STREQ("PduTooLarge", ApduStatusName(ApduStatus::PduTooLarge));
  EXPECT_STREQ("InternalError", ApduStatusName(ApduStatus::InternalError));
}

TEST(ApduStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown", ApduStatusName(static_cast<ApduStatus>(255)));
}

} // namespace
