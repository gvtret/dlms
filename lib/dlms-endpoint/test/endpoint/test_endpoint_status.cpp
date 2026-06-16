#include "dlms/endpoint/endpoint.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::endpoint::EndpointStatus;
using dlms::endpoint::ToString;

// Covers every EndpointStatus value defined in endpoint_status.hpp.
// `ToString` no longer has a `default:` arm (mirrors Phase 240 hygiene),
// so a new enum value lacking a string would trigger -Wswitch.
TEST(EndpointStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", ToString(EndpointStatus::Ok));
  EXPECT_STREQ("InvalidArgument", ToString(EndpointStatus::InvalidArgument));
  EXPECT_STREQ("InvalidState", ToString(EndpointStatus::InvalidState));
  EXPECT_STREQ("UnsupportedProfile",
               ToString(EndpointStatus::UnsupportedProfile));
  EXPECT_STREQ("TransportFailed",
               ToString(EndpointStatus::TransportFailed));
  EXPECT_STREQ("ProfileFailed", ToString(EndpointStatus::ProfileFailed));
  EXPECT_STREQ("AssociationFailed",
               ToString(EndpointStatus::AssociationFailed));
  EXPECT_STREQ("SecurityFailed",
               ToString(EndpointStatus::SecurityFailed));
  EXPECT_STREQ("ServiceFailed", ToString(EndpointStatus::ServiceFailed));
  EXPECT_STREQ("Timeout", ToString(EndpointStatus::Timeout));
  EXPECT_STREQ("Closed", ToString(EndpointStatus::Closed));
  EXPECT_STREQ("InternalError", ToString(EndpointStatus::InternalError));
}

TEST(EndpointStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown", ToString(static_cast<EndpointStatus>(255)));
}

} // namespace
