#include "dlms/association/association_status.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::association::AssociationStatus;
using dlms::association::AssociationStatusName;

// Covers every AssociationStatus value defined in association_status.hpp.
TEST(AssociationStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", AssociationStatusName(AssociationStatus::Ok));
  EXPECT_STREQ("InvalidArgument",
               AssociationStatusName(AssociationStatus::InvalidArgument));
  EXPECT_STREQ("InvalidState",
               AssociationStatusName(AssociationStatus::InvalidState));
  EXPECT_STREQ("AlreadyAssociated",
               AssociationStatusName(AssociationStatus::AlreadyAssociated));
  EXPECT_STREQ(
    "UnsupportedApplicationContext",
    AssociationStatusName(
      AssociationStatus::UnsupportedApplicationContext));
  EXPECT_STREQ(
    "UnsupportedAuthentication",
    AssociationStatusName(AssociationStatus::UnsupportedAuthentication));
  EXPECT_STREQ("ChannelOpenFailed",
               AssociationStatusName(AssociationStatus::ChannelOpenFailed));
  EXPECT_STREQ("ChannelCloseFailed",
               AssociationStatusName(AssociationStatus::ChannelCloseFailed));
  EXPECT_STREQ("SendFailed",
               AssociationStatusName(AssociationStatus::SendFailed));
  EXPECT_STREQ("ReceiveFailed",
               AssociationStatusName(AssociationStatus::ReceiveFailed));
  EXPECT_STREQ("EncodeFailed",
               AssociationStatusName(AssociationStatus::EncodeFailed));
  EXPECT_STREQ("DecodeFailed",
               AssociationStatusName(AssociationStatus::DecodeFailed));
  EXPECT_STREQ("AssociationRejected",
               AssociationStatusName(AssociationStatus::AssociationRejected));
  EXPECT_STREQ("NegotiationFailed",
               AssociationStatusName(AssociationStatus::NegotiationFailed));
  EXPECT_STREQ("InternalError",
               AssociationStatusName(AssociationStatus::InternalError));
}

TEST(AssociationStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ(
    "Unknown",
    AssociationStatusName(static_cast<AssociationStatus>(255)));
}

} // namespace
