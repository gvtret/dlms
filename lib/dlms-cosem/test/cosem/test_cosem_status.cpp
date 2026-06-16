#include "dlms/cosem/cosem_status.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::cosem::CosemStatus;
using dlms::cosem::CosemStatusName;

// Covers every CosemStatus value defined in cosem_status.hpp.
TEST(CosemStatus, NameCoversEveryEnumValue)
{
  EXPECT_STREQ("Ok", CosemStatusName(CosemStatus::Ok));
  EXPECT_STREQ("InvalidArgument",
               CosemStatusName(CosemStatus::InvalidArgument));
  EXPECT_STREQ("DuplicateObject",
               CosemStatusName(CosemStatus::DuplicateObject));
  EXPECT_STREQ("ObjectNotFound",
               CosemStatusName(CosemStatus::ObjectNotFound));
  EXPECT_STREQ("AttributeNotFound",
               CosemStatusName(CosemStatus::AttributeNotFound));
  EXPECT_STREQ("MethodNotFound",
               CosemStatusName(CosemStatus::MethodNotFound));
  EXPECT_STREQ("AccessDenied",
               CosemStatusName(CosemStatus::AccessDenied));
  EXPECT_STREQ("OutputBufferTooSmall",
               CosemStatusName(CosemStatus::OutputBufferTooSmall));
  EXPECT_STREQ("UnsupportedFeature",
               CosemStatusName(CosemStatus::UnsupportedFeature));
  EXPECT_STREQ("ObjectError",
               CosemStatusName(CosemStatus::ObjectError));
  EXPECT_STREQ("InternalError",
               CosemStatusName(CosemStatus::InternalError));
}

TEST(CosemStatus, NameReturnsUnknownForInvalidValue)
{
  EXPECT_STREQ("Unknown", CosemStatusName(static_cast<CosemStatus>(255)));
}

} // namespace
