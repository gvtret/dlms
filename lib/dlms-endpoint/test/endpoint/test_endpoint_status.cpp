#include "dlms/endpoint/endpoint.hpp"

#include <gtest/gtest.h>

TEST(EndpointStatus, NamesStableValues)
{
  EXPECT_STREQ(
    "Ok",
    dlms::endpoint::ToString(dlms::endpoint::EndpointStatus::Ok));
  EXPECT_STREQ(
    "InvalidArgument",
    dlms::endpoint::ToString(
      dlms::endpoint::EndpointStatus::InvalidArgument));
  EXPECT_STREQ(
    "UnsupportedProfile",
    dlms::endpoint::ToString(
      dlms::endpoint::EndpointStatus::UnsupportedProfile));
  EXPECT_STREQ(
    "InternalError",
    dlms::endpoint::ToString(
      dlms::endpoint::EndpointStatus::InternalError));
  EXPECT_STREQ(
    "Unknown",
    dlms::endpoint::ToString(
      static_cast<dlms::endpoint::EndpointStatus>(255)));
}

