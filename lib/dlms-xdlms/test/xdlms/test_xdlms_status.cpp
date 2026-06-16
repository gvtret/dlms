#include "dlms/xdlms/xdlms_status.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::xdlms::XdlmsStatus;
using dlms::xdlms::XdlmsStatusName;

// Covers every XdlmsStatus value defined in xdlms_status.hpp.
TEST(XdlmsStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", XdlmsStatusName(XdlmsStatus::Ok));
  EXPECT_STREQ("InvalidArgument",
               XdlmsStatusName(XdlmsStatus::InvalidArgument));
  EXPECT_STREQ("InvalidState",
               XdlmsStatusName(XdlmsStatus::InvalidState));
  EXPECT_STREQ("NotAssociated",
               XdlmsStatusName(XdlmsStatus::NotAssociated));
  EXPECT_STREQ("SendFailed",
               XdlmsStatusName(XdlmsStatus::SendFailed));
  EXPECT_STREQ("ReceiveFailed",
               XdlmsStatusName(XdlmsStatus::ReceiveFailed));
  EXPECT_STREQ("EncodeFailed",
               XdlmsStatusName(XdlmsStatus::EncodeFailed));
  EXPECT_STREQ("DecodeFailed",
               XdlmsStatusName(XdlmsStatus::DecodeFailed));
  EXPECT_STREQ("InvokeIdMismatch",
               XdlmsStatusName(XdlmsStatus::InvokeIdMismatch));
  EXPECT_STREQ("ServiceRejected",
               XdlmsStatusName(XdlmsStatus::ServiceRejected));
  EXPECT_STREQ("BlockTransferRequired",
               XdlmsStatusName(XdlmsStatus::BlockTransferRequired));
  EXPECT_STREQ("UnsupportedFeature",
               XdlmsStatusName(XdlmsStatus::UnsupportedFeature));
  EXPECT_STREQ("SecurityFailed",
               XdlmsStatusName(XdlmsStatus::SecurityFailed));
  EXPECT_STREQ("InternalError",
               XdlmsStatusName(XdlmsStatus::InternalError));
}

TEST(XdlmsStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown", XdlmsStatusName(static_cast<XdlmsStatus>(255)));
}

} // namespace
