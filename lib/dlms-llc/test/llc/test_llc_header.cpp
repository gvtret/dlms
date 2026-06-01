#include "dlms/llc/llc_header.hpp"

#include "dlms/llc/llc_error.hpp"
#include "dlms/llc/llc_types.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::llc::IsKnownDlmsLlcHeader;
using dlms::llc::kLlcControlDefault;
using dlms::llc::kLlcDsapBroadcast;
using dlms::llc::kLlcDsapDlms;
using dlms::llc::kLlcHeaderSize;
using dlms::llc::kLlcSsapClient;
using dlms::llc::kLlcSsapServer;
using dlms::llc::LlcDirection;
using dlms::llc::LlcHeader;
using dlms::llc::LlcStatus;
using dlms::llc::MakeLlcHeader;
using dlms::llc::ValidateLlcHeader;

TEST(LlcTypesTest, ConstantsMatchDlmsCosemProfile)
{
  EXPECT_EQ(0xe6u, kLlcDsapDlms);
  EXPECT_EQ(0xe6u, kLlcSsapClient);
  EXPECT_EQ(0xe7u, kLlcSsapServer);
  EXPECT_EQ(0xffu, kLlcDsapBroadcast);
  EXPECT_EQ(0x00u, kLlcControlDefault);
  EXPECT_EQ(3u, kLlcHeaderSize);
}

TEST(LlcHeaderTest, MakeClientToServerHeader)
{
  const LlcHeader header = MakeLlcHeader(LlcDirection::ClientToServer);

  EXPECT_EQ(kLlcDsapDlms, header.dsap);
  EXPECT_EQ(kLlcSsapClient, header.ssap);
  EXPECT_EQ(kLlcControlDefault, header.control);
}

TEST(LlcHeaderTest, MakeServerToClientHeader)
{
  const LlcHeader header = MakeLlcHeader(LlcDirection::ServerToClient);

  EXPECT_EQ(kLlcDsapDlms, header.dsap);
  EXPECT_EQ(kLlcSsapServer, header.ssap);
  EXPECT_EQ(kLlcControlDefault, header.control);
}

TEST(LlcHeaderTest, RecognizesKnownDlmsHeaders)
{
  EXPECT_TRUE(IsKnownDlmsLlcHeader(
    MakeLlcHeader(LlcDirection::ClientToServer)));
  EXPECT_TRUE(IsKnownDlmsLlcHeader(
    MakeLlcHeader(LlcDirection::ServerToClient)));
}

TEST(LlcHeaderTest, DoesNotTreatBroadcastAsStandardHeader)
{
  LlcHeader header = MakeLlcHeader(LlcDirection::ClientToServer);
  header.dsap = kLlcDsapBroadcast;

  EXPECT_FALSE(IsKnownDlmsLlcHeader(header));
}

TEST(LlcHeaderTest, ValidatesClientAndServerHeaders)
{
  EXPECT_EQ(LlcStatus::Ok,
            ValidateLlcHeader(MakeLlcHeader(LlcDirection::ClientToServer),
                              false));
  EXPECT_EQ(LlcStatus::Ok,
            ValidateLlcHeader(MakeLlcHeader(LlcDirection::ServerToClient),
                              false));
}

TEST(LlcHeaderTest, RejectsUnknownDestinationLsap)
{
  LlcHeader header = MakeLlcHeader(LlcDirection::ClientToServer);
  header.dsap = 0x01;

  EXPECT_EQ(LlcStatus::InvalidDsap, ValidateLlcHeader(header, false));
}

TEST(LlcHeaderTest, RejectsUnknownSourceLsap)
{
  LlcHeader header = MakeLlcHeader(LlcDirection::ClientToServer);
  header.ssap = 0x01;

  EXPECT_EQ(LlcStatus::InvalidSsap, ValidateLlcHeader(header, false));
}

TEST(LlcHeaderTest, RejectsUnsupportedControl)
{
  LlcHeader header = MakeLlcHeader(LlcDirection::ClientToServer);
  header.control = 0x03;

  EXPECT_EQ(LlcStatus::UnsupportedControl,
            ValidateLlcHeader(header, false));
}

TEST(LlcHeaderTest, BroadcastDestinationRequiresExplicitDecodePolicy)
{
  LlcHeader header = MakeLlcHeader(LlcDirection::ClientToServer);
  header.dsap = kLlcDsapBroadcast;

  EXPECT_EQ(LlcStatus::BroadcastEncodeForbidden,
            ValidateLlcHeader(header, false));
  EXPECT_EQ(LlcStatus::Ok, ValidateLlcHeader(header, true));
}

} // namespace
