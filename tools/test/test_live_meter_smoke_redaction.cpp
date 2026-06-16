// Redaction policy regression test for the live meter smoke
// tool. Verifies that wire-byte hex dumps from the wrapper
// and HDLC trace sinks stay off by default and only emit
// when the operator explicitly opts in via
// DLMS_LIVE_TRACE_WIRE_BYTES=1.
//
// This protects against accidentally re-leaking HLS
// challenges, GMAC tags, and ciphered/clear protected APDU
// payloads on the operator console during a routine live
// smoke run. See P0 \u00a73.6 in
// docs/production_readiness_roadmap.md.

#include "live_meter_smoke_byte_emit.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdint>
#include <sstream>
#include <string>

namespace {

#if defined(_WIN32)
void SetEnv(const char* name, const char* value)
{
  // Empty value clears the variable on Windows.
  _putenv_s(name, value == 0 ? "" : value);
}
#else
void SetEnv(const char* name, const char* value)
{
  if (value == 0) {
    ::unsetenv(name);
  } else {
    ::setenv(name, value, 1);
  }
}
#endif

class LiveMeterSmokeRedactionTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", 0);
  }

  void TearDown() override
  {
    SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", 0);
  }
};

const std::uint8_t kSecretBytes[] = {0xde, 0xad, 0xbe, 0xef};

dlms::profile::WrapperTcpTraceEvent MakeWrapperWireEvent()
{
  dlms::profile::WrapperTcpTraceEvent event;
  event.kind = dlms::profile::WrapperTcpTraceKind::WireWrite;
  event.direction = dlms::profile::WrapperTcpTraceDirection::Outbound;
  event.status = dlms::profile::ProfileStatus::Ok;
  event.sourcePort = 0u;
  event.destinationPort = 0u;
  event.encodedSize = sizeof(kSecretBytes);
  event.apduSize = sizeof(kSecretBytes);
  event.bytes = kSecretBytes;
  event.byteSize = sizeof(kSecretBytes);
  return event;
}

dlms::profile::HdlcProfileTraceEvent MakeHdlcWireEvent()
{
  dlms::profile::HdlcProfileTraceEvent event;
  event.kind = dlms::profile::HdlcProfileTraceKind::WireRead;
  event.direction = dlms::profile::HdlcProfileTraceDirection::Inbound;
  event.status = dlms::profile::ProfileStatus::Ok;
  event.encodedSize = sizeof(kSecretBytes);
  event.apduSize = sizeof(kSecretBytes);
  event.bytes = kSecretBytes;
  event.byteSize = sizeof(kSecretBytes);
  return event;
}

}  // namespace

TEST_F(LiveMeterSmokeRedactionTest, WireBytesTraceEnabledIsFalseByDefault)
{
  EXPECT_FALSE(dlms_live_smoke::WireBytesTraceEnabled());
}

TEST_F(LiveMeterSmokeRedactionTest, WireBytesTraceEnabledIsTrueWhenEnvSetToOne)
{
  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "1");
  EXPECT_TRUE(dlms_live_smoke::WireBytesTraceEnabled());
}

TEST_F(LiveMeterSmokeRedactionTest, WireBytesTraceRejectsOtherValues)
{
  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "0");
  EXPECT_FALSE(dlms_live_smoke::WireBytesTraceEnabled());

  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "true");
  EXPECT_FALSE(dlms_live_smoke::WireBytesTraceEnabled());

  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "");
  EXPECT_FALSE(dlms_live_smoke::WireBytesTraceEnabled());
}

TEST_F(LiveMeterSmokeRedactionTest, WrapperEmitOmitsBytesWhenFlagIsUnset)
{
  std::stringstream out;
  const dlms::profile::WrapperTcpTraceEvent event = MakeWrapperWireEvent();
  dlms_live_smoke::EmitWrapperBytesIfEnabled(out, event);
  EXPECT_EQ(out.str(), std::string());
}

TEST_F(LiveMeterSmokeRedactionTest, WrapperEmitWritesBytesWhenFlagIsSet)
{
  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "1");
  std::stringstream out;
  const dlms::profile::WrapperTcpTraceEvent event = MakeWrapperWireEvent();
  dlms_live_smoke::EmitWrapperBytesIfEnabled(out, event);
  EXPECT_EQ(out.str(), std::string(" bytes=de ad be ef"));
}

TEST_F(LiveMeterSmokeRedactionTest, HdlcEmitOmitsBytesWhenFlagIsUnset)
{
  std::stringstream out;
  const dlms::profile::HdlcProfileTraceEvent event = MakeHdlcWireEvent();
  dlms_live_smoke::EmitHdlcBytesIfEnabled(out, event);
  EXPECT_EQ(out.str(), std::string());
}

TEST_F(LiveMeterSmokeRedactionTest, HdlcEmitWritesBytesWhenFlagIsSet)
{
  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "1");
  std::stringstream out;
  const dlms::profile::HdlcProfileTraceEvent event = MakeHdlcWireEvent();
  dlms_live_smoke::EmitHdlcBytesIfEnabled(out, event);
  EXPECT_EQ(out.str(), std::string(" bytes=de ad be ef"));
}

TEST_F(LiveMeterSmokeRedactionTest, NonWireWrapperEventsAreSilentEvenWithFlag)
{
  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "1");
  std::stringstream out;
  dlms::profile::WrapperTcpTraceEvent event = MakeWrapperWireEvent();
  event.kind = dlms::profile::WrapperTcpTraceKind::ReadStatus;
  dlms_live_smoke::EmitWrapperBytesIfEnabled(out, event);
  EXPECT_EQ(out.str(), std::string());
}

TEST_F(LiveMeterSmokeRedactionTest, NonWireHdlcEventsAreSilentEvenWithFlag)
{
  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "1");
  std::stringstream out;
  dlms::profile::HdlcProfileTraceEvent event = MakeHdlcWireEvent();
  event.kind = dlms::profile::HdlcProfileTraceKind::DecodeStatus;
  dlms_live_smoke::EmitHdlcBytesIfEnabled(out, event);
  EXPECT_EQ(out.str(), std::string());
}

TEST_F(LiveMeterSmokeRedactionTest, EmptyByteSpanProducesNoOutputWhenFlagIsSet)
{
  SetEnv("DLMS_LIVE_TRACE_WIRE_BYTES", "1");
  std::stringstream out;
  dlms::profile::WrapperTcpTraceEvent event = MakeWrapperWireEvent();
  event.bytes = 0;
  event.byteSize = 0u;
  dlms_live_smoke::EmitWrapperBytesIfEnabled(out, event);
  EXPECT_EQ(out.str(), std::string());
}
