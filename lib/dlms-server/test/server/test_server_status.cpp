#include "dlms/server/server_status.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::server::ServerStatus;
using dlms::server::ServerStatusName;

// Covers every ServerStatus value defined in server_status.hpp.
TEST(ServerStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", ServerStatusName(ServerStatus::Ok));
  EXPECT_STREQ("InvalidArgument",
               ServerStatusName(ServerStatus::InvalidArgument));
  EXPECT_STREQ("NotAssociated",
               ServerStatusName(ServerStatus::NotAssociated));
  EXPECT_STREQ("NoLogicalDevice",
               ServerStatusName(ServerStatus::NoLogicalDevice));
  EXPECT_STREQ("ObjectNotFound",
               ServerStatusName(ServerStatus::ObjectNotFound));
  EXPECT_STREQ("AccessDenied",
               ServerStatusName(ServerStatus::AccessDenied));
  EXPECT_STREQ("AttributeNotFound",
               ServerStatusName(ServerStatus::AttributeNotFound));
  EXPECT_STREQ("MethodNotFound",
               ServerStatusName(ServerStatus::MethodNotFound));
  EXPECT_STREQ("ObjectError",
               ServerStatusName(ServerStatus::ObjectError));
  EXPECT_STREQ("UnsupportedFeature",
               ServerStatusName(ServerStatus::UnsupportedFeature));
  EXPECT_STREQ("EncodeRequired",
               ServerStatusName(ServerStatus::EncodeRequired));
  EXPECT_STREQ("InternalError",
               ServerStatusName(ServerStatus::InternalError));
}

TEST(ServerStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown", ServerStatusName(static_cast<ServerStatus>(255)));
}

} // namespace
